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
    _delay->enqueue(midi::MidiType::NoteOn, channel + 16, pitch,
                    velocity * 0.5f, active_patch->delay_config.delay_ticks);
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
    _delay->enqueue(midi::MidiType::NoteOff, channel + 16, pitch,
                    velocity * 0.5f, active_patch->delay_config.delay_ticks);
  }
}

/**
 * @param channel this must be zero indexed! in other words "MIDI channel 1" is
 * 0, "MIDI channel 16" is 15. The channel from Arduino MIDI Library should be
 * normalized outside this function.
 */
void MidiManager::handleControlChange(byte channel, byte cc_number, byte data) {
  state.channels[channel].cc[cc_number] = data;

  switch (cc_number) {
  case 94:
    _active_multi->channels[channel].detune_cents = signed(data) - 63;
    return;
  }
}
