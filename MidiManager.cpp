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

void MidiManager::handleNoteOn(byte channel, byte pitch, byte velocity) {
  if (NOTES_4MHZ[pitch] > 1023) {
    return;
  }
  // digitalWrite(LED_PIN, HIGH);

  Voice *voice = _voice_manager->getVoice(channel, pitch);
  Patch *active_patch = &_active_multi->channels[(channel - 1) % 16];
  voice->setPatch(&_active_multi->channels[(channel - 1) % 16]);
  voice->noteOn(channel, pitch, velocity);
  if (channel > 16) {

    voice->detune_cents = active_patch->delay_config.detune_cents;
  } else {
    voice->detune_cents = 0;
  }

  if (channel <= 16 && active_patch->delay_config.enable) {
    _delay->enqueue(midi::MidiType::NoteOn, channel + 16, pitch,
                    velocity * 0.5f, active_patch->delay_config.delay_ticks);
  }

  // digitalWrite(LED_PIN, LOW);
}

void MidiManager::handleNoteOff(byte channel, byte pitch, byte velocity) {
  Patch *active_patch = &_active_multi->channels[(channel - 1) % 16];
  Voice *voice = _voice_manager->getExactVoice(channel, pitch);
  voice->noteOff();

  if (channel <= 16 && active_patch->delay_config.enable) {
    _delay->enqueue(midi::MidiType::NoteOff, channel + 16, pitch,
                    velocity * 0.5f, active_patch->delay_config.delay_ticks);
  }
}

void MidiManager::tick() { _delay->tick(); }
