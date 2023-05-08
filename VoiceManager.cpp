#include "VoiceManager.h"

VoiceManager::VoiceManager(byte voice_count) {
  _voice_count = voice_count;
  _voice_round_robin = 0;
}

Voice nullVoice;

Voice *VoiceManager::getVoice(byte channel, byte pitch) {
  // if there is already an active voice that matches the channel + pitch
  // requested, provide that. This covers to primary scenarios:
  //  - hitting a note twice while holding the sustain pedal
  for (byte voice_index = 0; voice_index < _voice_count; voice_index++) {
    if (voices[voice_index].getStatus() != voice_off &&
        voices[voice_index].pitch == pitch &&
        voices[voice_index].channel == channel) {
      return &voices[voice_index];
    }
  }

  for (byte voice_index = 0; voice_index < _voice_count; voice_index++) {
    if (voices[voice_index].getStatus() == voice_off) {
      return &voices[voice_index];
    }
  }

  for (byte voice_index = 0; voice_index < _voice_count; voice_index++) {
    if (voices[voice_index].getStatus() == voice_decay) {
      return &voices[voice_index];
    }
  }

  if (channel > 15) {
    // channel 16+ is reserved for echos & chorus effects. if all voices are
    // currently being held, echo/chorus should never steal from held voices.
    // TODO: it's okay if an echo voice steals from another live echo voice!
    return &nullVoice;
  }

  return &(voices[(++_voice_round_robin) % _voice_count]);
}

/**
 * getExactVoice is intended for handling note off messages. It will only yield
 * voices that match the channel and pitch exactly, and are either in the
 * held or decaying state.
 */
Voice *VoiceManager::getExactVoice(byte channel, byte pitch) {
  for (byte voice_index = 0; voice_index < _voice_count; voice_index++) {
    if ( // voices[voice_index].getStatus() != voice_off &&
        voices[voice_index].pitch == pitch &&
        voices[voice_index].channel == channel) {
      return &voices[voice_index];
    }
  }

  return &nullVoice;
}

void VoiceManager::tick() {
  for (byte voice_index = 0; voice_index < _voice_count; voice_index++) {
    voices[voice_index].tick();
  }
}
