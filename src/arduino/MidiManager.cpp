#include "MidiManager.h"
#include "NoteMappings.h"
#include "Sysex.h"
#include <MIDI.h>
#include <midi_Defs.h>

#define LED_PIN 13

/**
 * @param channel this must be zero indexed! in other words "MIDI channel 1" is
 * 0, "MIDI channel 16" is 15. The channel from Arduino MIDI Library should be
 * normalized outside this function.
 */
void MidiManager::handleNoteOn(byte channel, byte pitch, byte velocity) {
  if (velocity == 0) {
    // per MIDI spec, noteOn with velocity=0 is equivalent to noteOff
    handleNoteOff(channel, pitch, velocity);
    return;
  }

  _synth->noteOn(channel, pitch, velocity);

  PatchDelayConfig *delay_config = _synth->getDelayConfig(channel % 16);
  if (_delay && channel < 16 && delay_config != nullptr &&
      delay_config->enable) {
    _delay->enqueue(midi::MidiType::NoteOn, channel + 16, pitch, velocity,
                    getDelayTicks(delay_config->delay_time));
  }
}

/**
 * @param channel this must be zero indexed! in other words "MIDI channel 1" is
 * 0, "MIDI channel 16" is 15. The channel from Arduino MIDI Library should be
 * normalized outside this function.
 */
void MidiManager::handleNoteOff(byte channel, byte pitch, byte velocity) {
  _synth->noteOff(channel, pitch, velocity);

  PatchDelayConfig *delay_config = _synth->getDelayConfig(channel % 16);
  if (_delay && channel < 16 && delay_config != nullptr &&
      delay_config->enable) {
    _delay->enqueue(midi::MidiType::NoteOff, channel + 16, pitch, velocity,
                    getDelayTicks(delay_config->delay_time));
  }
}

void MidiManager::handlePitchBend(byte channel, int bend) {
  _synth->setPitchBend(channel, bend);
}

void applyPsgControlChange(PsgPatch *patch, byte cc_number, byte data) {
  switch (cc_number) {
  // envelopes
  case 70:
    // patch->amplitude_envelope.steps[0].value = data >> 3;
    break;
  case 72:
    patch->amplitude_envelope.attack = data;
    break;
  case 73:
    patch->amplitude_envelope.decay = data;
    break;
  case 75:
    patch->amplitude_envelope.sustain = data;
    break;
  case 76:
    patch->amplitude_envelope.release = data;
    break;

  // pitch envelope
  case 77:
    patch->pitch_envelope.scaling = data - 63;
    break;
  case 78:
    patch->pitch_envelope.envelope_shape.attack = data;
    break;
  case 79:
    patch->pitch_envelope.envelope_shape.decay = data;
    break;
  case 80:
    patch->pitch_envelope.envelope_shape.sustain = data;
    break;
  case 81:
    patch->pitch_envelope.envelope_shape.release = data;
    break;
  // pitch LFO
  case 84:
    patch->pitch_lfo.depth = data;
    break;
  case 85:
    patch->pitch_lfo.speed = data;
    break;
  case 86:
    patch->pitch_lfo.waveform = LfoWaveform(data >> 5);
    break;
  case 87:
    patch->pitch_lfo.start_delay_time = data;
    break;
  // delay voice
  case 91:
    patch->delay_config.attenuation = 15 - (data >> 3);
    break;
  case 93:
    patch->delay_config.detune_cents = data - 63;
    break;
  case 95:
    patch->delay_config.delay_time = data;
    break;
  // voice-level parameters
  case 94:
    patch->detune_cents = data - 63;
    break;
  }
}

/**
 * Note: for many parameters (total level, and all envelope parameters), they
 * are flipped. The patch format keeps the numbers the exact same as the values
 * sent to the chip (e.g. if operator.total_level is 127, it is silent, if
 * attack is 31, the sound is instantaneous), while for the CC values they are
 * scaled to 127 and flipped to normal values (127 is the maximium total level,
 * and 127 is the slowest possible attack rate).
 */
void applyFmControlChange(FmPatch *patch, byte cc_number, byte data) {
  switch (cc_number) {
  // voice-level parameters
  case 14:
    patch->core_parameters.algorithm = data >> 4;
    break;
  case 15:
    patch->core_parameters.feedback = data >> 4;
    break;
  case 75:
    patch->core_parameters.lfo_frequency_sensitivity = data >> 4;
    break;
  case 76:
    patch->core_parameters.lfo_amplitude_sensitivity = data >> 5;
    break;
  // case 77:
  //   patch->core_parameters.panning = FmPanningMode(data >> 5);
  //   break;

  // per-operator parameters

  // tl
  case 16:
  case 17:
  case 18:
  case 19:
    patch->operators[cc_number - 16].total_level = 127 - data;
    break;
  // mult
  case 20:
  case 21:
  case 22:
  case 23:
    patch->operators[cc_number - 20].multiple = data >> 3;
    break;
  // dt1
  case 24:
  case 25:
  case 26:
  case 27:
    patch->operators[cc_number - 24].detune = data >> 4;
    break;
  // rs
  case 39:
  case 40:
  case 41:
  case 42:
    patch->operators[cc_number - 39].key_scale = data >> 5;
    break;
  // ar
  case 43:
  case 44:
  case 45:
  case 46:
    patch->operators[cc_number - 43].attack_rate = (127 - data) >> 2;
    break;
  // d1r
  case 47:
  case 48:
  case 49:
  case 50:
    patch->operators[cc_number - 47].decay_rate = (127 - data) >> 2;
    break;
  // d2r
  case 51:
  case 52:
  case 53:
  case 54:
    patch->operators[cc_number - 51].sustain_rate = (127 - data) >> 2;
    break;
  // dl
  case 55:
  case 56:
  case 57:
  case 58:
    patch->operators[cc_number - 55].sustain_level = (127 - data) >> 3;
    break;
  // rr
  case 59:
  case 60:
  case 61:
  case 62:
    patch->operators[cc_number - 59].release_rate = (127 - data) >> 3;
    break;
  // am
  case 70:
  case 71:
  case 72:
  case 73:
    patch->operators[cc_number - 70].lfo_amplitude_enable = data >> 7;
    break;
  // pitch envelope
  case 77:
    patch->pitch_envelope.scaling = data - 63;
    break;
  case 78:
    patch->pitch_envelope.envelope_shape.attack = data;
    break;
  case 79:
    patch->pitch_envelope.envelope_shape.decay = data;
    break;
  case 80:
    patch->pitch_envelope.envelope_shape.sustain = data;
    break;
  case 81:
    patch->pitch_envelope.envelope_shape.release = data;
    break;
  // TODO: implement SSG_EG
  // case 90:
  // case 91:
  // case 92:
  // case 93:
  //   patch->operators[cc_number - 90].ssg_eg = data;
  //   break;

  // software pitch LFO
  case 84:
    patch->pitch_lfo.depth = data;
    break;
  case 85:
    patch->pitch_lfo.speed = data;
    break;
  case 86:
    patch->pitch_lfo.waveform = LfoWaveform(data >> 5);
    break;
  case 87:
    patch->pitch_lfo.start_delay_time = data;
    break;
  // delay voice
  case 91:
    patch->delay_config.attenuation = 15 - (data >> 3);
    break;
  case 93:
    patch->delay_config.detune_cents = data - 63;
    break;
  case 95:
    patch->delay_config.delay_time = data;
    break;
  // voice-level parameters
  case 94:
    // patch->detune_cents = data - 63;
    break;
  }
}

void MidiManager::handleProgramChange(byte channel, byte program) {
  digitalWrite(13, HIGH);

  _synth->programChange(channel, program);

  digitalWrite(13, LOW);
}

/**
 * @param channel this must be zero indexed! in other words "MIDI channel 1" is
 * 0, "MIDI channel 16" is 15. The channel from Arduino MIDI Library should be
 * normalized outside this function.
 */
void MidiManager::handleControlChange(byte channel, byte cc_number, byte data) {
  // bank select
  if (cc_number == 0) {
    _synth->bankMsbChange(channel, data);
    return;
  }
  if (cc_number == 32) {
    _synth->bankLsbChange(channel, data);
    return;
  }

  _synth->controlChange(channel, cc_number, data);

  // if (cc_number == 38) {
  //   _synth->saveMulti();
  //   return;
  // }

  SynthChannel *synth_channel = _synth->getChannel(channel);
  if (synth_channel->mode == MULTI_CHANNEL_MODE_PSG) {
    applyPsgControlChange(
        _synth->getPsgPatchManager()->getChannelPatch(channel), cc_number,
        data);
  } else if (synth_channel->mode == MULTI_CHANNEL_MODE_FM) {
    applyFmControlChange(_synth->getFmPatchManager()->getChannelPatch(channel),
                         cc_number, data);
  }
}

/**
 * Common:
 *  array[0]: F0
 *  array[1]: Manufacturer ID
 *  array[2]: Message type
 *
 * FM patch dump:
 *  array[3]: program number
 *  array[4]: bank (MSB)
 *  array[5]: bank (LSB)
 *  array[6]: update buffered patch?
 *  array[7..n]: data
 *  array[n+1]: F7
 */
void MidiManager::handleSysex(byte *array, unsigned size) {
  Serial.println("Received Sysex message");
  Serial.print(size, DEC);
  Serial.println(" bytes");

  byte manufacturer_id = array[1];
  SysexMessageType message_type = (SysexMessageType)array[2];

  if (manufacturer_id != MANUFACTURER_ID) {
    Serial.println("incorrect manufacturer ID, aborting");
    return;
  }

  FmPatch *patch;
  PatchId target_patch_id;
  bool should_update_buffered_patches;

  switch (message_type) {
  case SYSEX_MESSAGE_TYPE_FM_SINGLE_VOICE_DUMP:
    target_patch_id.program_number = array[3];
    target_patch_id.bank_number_msb = array[4];
    target_patch_id.bank_number_lsb = array[5];
    should_update_buffered_patches = array[6] > 0;

    if (size - 8 != sizeof *patch) {
      Serial.print("incorrect packet size. Got ");
      Serial.print(size - 2);
      Serial.print(", expected ");
      Serial.print(sizeof *patch);
      return;
    }

    patch = (FmPatch *)&array[7];

    _synth->getFmPatchManager()->writePatch(&target_patch_id, patch);

    if (should_update_buffered_patches) {

      for (byte channel = 0; channel < 16; channel++) {
        SynthChannel *synth_channel = _synth->getChannel(channel);
        if (synth_channel->patch_id.bank_number_msb ==
                target_patch_id.bank_number_msb &&
            synth_channel->patch_id.bank_number_lsb ==
                target_patch_id.bank_number_lsb &&
            synth_channel->patch_id.program_number ==
                target_patch_id.program_number) {
          Serial.print("Updating synth channel ");
          Serial.println(channel + 1, DEC);

          _synth->getFmPatchManager()->loadPatch(&synth_channel->patch_id,
                                                 channel);
        }
      }
    }
    return;
  default:
    break;
  }
  Serial.println("Done");
  Serial.println();
}
