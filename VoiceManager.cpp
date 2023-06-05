#include "VoiceManager.h"

template <typename VoiceType>
unsigned getAvailableVoiceIndex(byte channel, byte note,
                                VoiceType voice_options[], unsigned voice_count,
                                unsigned last_voice_used) {
  VoiceType *voice_hypothesis;
  // if there is already an active voice that matches the channel + pitch
  // requested, provide that. This covers to primary scenarios:
  //  - hitting a note twice while holding the sustain pedal
  for (unsigned voice_index = 0; voice_index < voice_count; voice_index++) {
    voice_hypothesis = &voice_options[voice_index];
    if (voice_hypothesis->getStatus() != voice_off &&
        voice_hypothesis->pitch == note &&
        voice_hypothesis->channel == channel) {
      return voice_index;
    }
  }

  for (unsigned voice_index = 0; voice_index < voice_count; voice_index++) {
    voice_hypothesis = &voice_options[voice_index];
    if (voice_hypothesis->getStatus() == voice_off) {
      return voice_index;
    }
  }

  for (unsigned voice_index = 0; voice_index < voice_count; voice_index++) {
    voice_hypothesis = &voice_options[voice_index];
    if (voice_hypothesis->getStatus() == voice_decay) {
      return voice_index;
    }
  }

  // always prefer to steal an echo voice rather than a lead voice
  for (unsigned voice_index = 0; voice_index < voice_count; voice_index++) {
    voice_hypothesis = &voice_options[voice_index];
    if (voice_hypothesis->getIsDelay()) {
      return voice_index;
    }
  }

  if (channel > 15) {
    // channel 16+ is reserved for echos & chorus effects. if all voices are
    // currently being held, echo/chorus should never steal from held voices.
    return NO_AVAILABLE_VOICE_INDEX;
  }

  return ((last_voice_used + 1) % voice_count);
}

template unsigned getAvailableVoiceIndex<PsgVoice>(byte channel, byte note,
                                                   PsgVoice voice_options[],
                                                   unsigned voice_count,
                                                   unsigned last_voice_used);
