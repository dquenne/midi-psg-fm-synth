#include "MidiManager.h"
#include "NoteMappings.h"
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
  if (channel < 16) {
    state.channels[channel].notes[pitch].velocity = velocity;
  }

  _synth->noteOn(channel, pitch, velocity);

  if (_delay && channel < 16 && _synth->getDelayConfig(channel)->enable) {
    _delay->enqueue(
        midi::MidiType::NoteOn, channel + 16, pitch, velocity,
        getDelayTicks(_synth->getDelayConfig(channel % 16)->delay_time));
  }
}

/**
 * @param channel this must be zero indexed! in other words "MIDI channel 1" is
 * 0, "MIDI channel 16" is 15. The channel from Arduino MIDI Library should be
 * normalized outside this function.
 */
void MidiManager::handleNoteOff(byte channel, byte pitch, byte velocity) {
  if (channel < 16) {
    state.channels[channel].notes[pitch].velocity = 0;
  }
  _synth->noteOff(channel, pitch, velocity);

  if (_delay && channel < 16 && _synth->getDelayConfig(channel % 16)->enable) {
    _delay->enqueue(
        midi::MidiType::NoteOff, channel + 16, pitch, velocity,
        getDelayTicks(_synth->getDelayConfig(channel % 16)->delay_time));
  }
}

void MidiManager::handlePitchBend(byte channel, int bend) {
  _synth->setPitchBend(channel, bend);
}

void applyPsgControlChange(PsgPatch *patch, byte cc_number, byte data) {
  switch (cc_number) {
  // envelopes
  case 70:
    patch->amplitude_envelope.steps[0].value = data >> 3;
    break;
  case 72:
    patch->amplitude_envelope.steps[1].value = data >> 3;
    break;
  case 73:
    patch->amplitude_envelope.steps[2].value = data >> 3;
    break;
  case 75:
    patch->amplitude_envelope.steps[3].value = data >> 3;
    break;
  case 76:
    patch->amplitude_envelope.steps[4].value = data >> 3;
    break;
  case 77:
    patch->amplitude_envelope.steps[0].hold_ticks = data << 2;
    break;
  case 78:
    patch->amplitude_envelope.steps[1].hold_ticks = data << 2;
    break;
  case 79:
    patch->amplitude_envelope.steps[2].hold_ticks = data << 2;
    break;
  case 80:
    patch->amplitude_envelope.steps[3].hold_ticks = data << 2;
    break;
  case 81:
    patch->amplitude_envelope.steps[4].hold_ticks = data << 2;
    break;
  // frequency LFO
  case 84:
    patch->frequency_lfo.depth = data;
    break;
  case 85:
    patch->frequency_lfo.speed = data;
    break;
  case 86:
    patch->frequency_lfo.waveform = LfoWaveform(data >> 5);
    break;
  case 87:
    patch->frequency_lfo.start_delay_time = data;
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
  case 77:
    patch->core_parameters.panning = FmPanningMode(data >> 5);
    break;

  // per-operator parameters

  // tl
  case 16:
  case 17:
  case 18:
  case 19:
    patch->core_parameters.operators[cc_number - 16].total_level = 127 - data;
    break;
  // mult
  case 20:
  case 21:
  case 22:
  case 23:
    patch->core_parameters.operators[cc_number - 20].multiple = data >> 3;
    break;
  // dt1
  case 24:
  case 25:
  case 26:
  case 27:
    patch->core_parameters.operators[cc_number - 24].detune = data >> 4;
    break;
  // rs
  case 39:
  case 40:
  case 41:
  case 42:
    patch->core_parameters.operators[cc_number - 39].key_scale = data >> 5;
    break;
  // ar
  case 43:
  case 44:
  case 45:
  case 46:
    patch->core_parameters.operators[cc_number - 43].attack_rate =
        (127 - data) >> 2;
    break;
  // d1r
  case 47:
  case 48:
  case 49:
  case 50:
    patch->core_parameters.operators[cc_number - 47].decay_rate =
        (127 - data) >> 2;
    break;
  // d2r
  case 51:
  case 52:
  case 53:
  case 54:
    patch->core_parameters.operators[cc_number - 51].sustain_rate =
        (127 - data) >> 2;
    break;
  // dl
  case 55:
  case 56:
  case 57:
  case 58:
    patch->core_parameters.operators[cc_number - 55].sustain_level =
        (127 - data) >> 3;
    break;
  // rr
  case 59:
  case 60:
  case 61:
  case 62:
    patch->core_parameters.operators[cc_number - 59].release_rate =
        (127 - data) >> 3;
    break;
  // am
  case 70:
  case 71:
  case 72:
  case 73:
    patch->core_parameters.operators[cc_number - 70].lfo_amplitude_enable =
        data >> 7;
    break;
  // TODO: implement SSG_EG
  // case 90:
  // case 91:
  // case 92:
  // case 93:
  //   patch->core_parameters.operators[cc_number - 90].ssg_eg = data;
  //   break;

  // software frequency LFO
  case 84:
    patch->frequency_lfo.depth = data;
    break;
  case 85:
    patch->frequency_lfo.speed = data;
    break;
  case 86:
    patch->frequency_lfo.waveform = LfoWaveform(data >> 5);
    break;
  case 87:
    patch->frequency_lfo.start_delay_time = data;
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

  SynthChannel *synth_channel = _synth->getChannel(channel);
  synth_channel->patch_id = {program, synth_channel->patch_id.bank_number};

  digitalWrite(13, LOW);
}

/**
 * @param channel this must be zero indexed! in other words "MIDI channel 1" is
 * 0, "MIDI channel 16" is 15. The channel from Arduino MIDI Library should be
 * normalized outside this function.
 */
void MidiManager::handleControlChange(byte channel, byte cc_number, byte data) {
  state.channels[channel].cc[cc_number] = data;

  // bank select
  if (cc_number == 0) {
    _synth->getChannel(channel)->mode =
        (data >> 6 == 0 ? MULTI_CHANNEL_MODE_FM : MULTI_CHANNEL_MODE_PSG);
    return;
  }

  // if (cc_number == 38) {
  //   _synth->saveMulti();
  //   return;
  // }

  SynthChannel *synth_channel = _synth->getChannel(channel);
  if (synth_channel->mode == MULTI_CHANNEL_MODE_PSG) {
    applyPsgControlChange(
        _synth->getPsgPatchManager()->getPatch(synth_channel->patch_id),
        cc_number, data);
  } else if (synth_channel->mode == MULTI_CHANNEL_MODE_FM) {
    applyFmControlChange(
        _synth->getFmPatchManager()->getPatch(synth_channel->patch_id),
        cc_number, data);
  }
}
