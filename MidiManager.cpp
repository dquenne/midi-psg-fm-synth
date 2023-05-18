#include "MidiManager.h"
#include "NoteMappings.h"
#include <MIDI.h>
#include <midi_Defs.h>

#define LED_PIN 13

MidiManager::MidiManager(VoiceManager *voice_manager, Multi *active_multi,
                         MidiDelay *delay) {
  _voice_manager = voice_manager;
  _active_multi = active_multi;
  _delay = delay;
}

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

  if (NOTES_4MHZ[pitch] > 1023) {
    return;
  }

  Voice *voice = _voice_manager->getVoice(channel, pitch);
  Patch *active_patch = &_active_multi->channels[channel % 16];
  voice->setPatch(&_active_multi->channels[channel % 16], channel >= 16);
  voice->noteOn(channel, pitch, velocity);

  voice->detune_cents = 0;

  if (_delay && channel < 16 && active_patch->delay_config.enable) {
    _delay->enqueue(midi::MidiType::NoteOn, channel + 16, pitch, velocity,
                    active_patch->delay_config.delay_ticks);
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

  Patch *active_patch = &_active_multi->channels[channel % 16];
  Voice *voice = _voice_manager->getExactVoice(channel, pitch);
  voice->noteOff();

  if (_delay && channel < 16 && active_patch->delay_config.enable) {
    _delay->enqueue(midi::MidiType::NoteOff, channel + 16, pitch, velocity,
                    active_patch->delay_config.delay_ticks);
  }
}

void applyControlChange(Patch *patch, byte cc_number, byte data) {
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
    patch->frequency_lfo.depth = data << 1;
    break;
  case 85:
    patch->frequency_lfo.wavelength = ((128 / (data + 1)) << 4) + 80;
    break;
  case 86:
    patch->frequency_lfo.waveform = LfoWaveform(data >> 5);
    break;
  case 87:
    patch->frequency_lfo.start_delay_ticks = data << 4;
    break;
  // delay voice
  case 91:
    patch->delay_config.attenuation = 15 - (data >> 3);
    break;
  case 93:
    patch->delay_config.detune_cents = data - 63;
    break;
  case 95:
    patch->delay_config.delay_ticks = data << 2;
    break;
  // voice-level parameters
  case 94:
    patch->detune_cents = data - 63;
    break;
  }
}

/**
 * @param channel this must be zero indexed! in other words "MIDI channel 1" is
 * 0, "MIDI channel 16" is 15. The channel from Arduino MIDI Library should be
 * normalized outside this function.
 */
void MidiManager::handleControlChange(byte channel, byte cc_number, byte data) {
  state.channels[channel].cc[cc_number] = data;

  applyControlChange(&_active_multi->channels[channel], cc_number, data);
}
