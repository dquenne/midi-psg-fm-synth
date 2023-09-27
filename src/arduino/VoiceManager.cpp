#include "VoiceManager.h"

template <typename VoiceType>
unsigned getAvailableVoiceIndex(byte channel, byte note,
                                VoiceType voice_options[], unsigned voice_count,
                                unsigned last_voice_used,
                                PatchPolyphonyConfig *patch_polyphony_config) {
  VoiceType *voice_hypothesis;
  unsigned voice_index;

  // if there is already an active voice that matches the channel + pitch
  // requested, provide that. This covers to primary scenarios:
  //  - hitting a note twice while holding the sustain pedal
  for (voice_index = 0; voice_index < voice_count; voice_index++) {
    voice_hypothesis = &voice_options[voice_index];
    if (voice_hypothesis->getStatus() != voice_off &&
        voice_hypothesis->pitch == note &&
        voice_hypothesis->channel == channel) {
      return voice_index;
    }
  }

  if (patch_polyphony_config->max_polyphony != MAX_POLYPHONY_UNLIMITED) {
    unsigned channel_used_voice_count = 0;
    unsigned channel_lowest_priority_voice_index = voice_count;

    for (voice_index = 0; voice_index < voice_count; voice_index++) {
      voice_hypothesis = &voice_options[voice_index];
      if (voice_hypothesis->channel == channel &
          voice_hypothesis->getStatus() != voice_off) {
        channel_used_voice_count += 1;

        // if current note being analyzed is same channel and older, make that
        // new lowest-priority hypothesis
        if (channel_lowest_priority_voice_index >= voice_count ||
            voice_hypothesis->triggered_at <
                voice_options[channel_lowest_priority_voice_index]
                    .triggered_at) {
          channel_lowest_priority_voice_index = voice_index;
        }
      }
    }
    if (channel_used_voice_count >= patch_polyphony_config->max_polyphony) {
      // at polyphony maximum, need to steal voice
      return channel_lowest_priority_voice_index;
    }
    // else, just take next available voice from normal stealing logic
  }

  for (voice_index = 0; voice_index < voice_count; voice_index++) {
    unsigned offset_voice_index =
        (last_voice_used + 1 + voice_index) % voice_count;
    voice_hypothesis = &voice_options[offset_voice_index];
    if (voice_hypothesis->getStatus() == voice_off) {
      return offset_voice_index;
    }
  }

  for (voice_index = 0; voice_index < voice_count; voice_index++) {
    unsigned offset_voice_index =
        (last_voice_used + 1 + voice_index) % voice_count;
    voice_hypothesis = &voice_options[offset_voice_index];
    if (voice_hypothesis->getStatus() == voice_decay) {
      return offset_voice_index;
    }
  }

  // always prefer to steal an echo voice rather than a lead voice
  for (voice_index = 0; voice_index < voice_count; voice_index++) {
    unsigned offset_voice_index =
        (last_voice_used + 1 + voice_index) % voice_count;
    voice_hypothesis = &voice_options[offset_voice_index];
    if (voice_hypothesis->getIsDelay()) {
      return offset_voice_index;
    }
  }

  if (channel > 15) {
    // channel 16+ is reserved for echos & chorus effects. if all voices are
    // currently being held, echo/chorus should never steal from held voices.
    return NO_AVAILABLE_VOICE_INDEX;
  }

  return ((last_voice_used + 1) % voice_count);
}

template unsigned getAvailableVoiceIndex<PsgVoice>(
    byte channel, byte note, PsgVoice voice_options[], unsigned voice_count,
    unsigned last_voice_used, PatchPolyphonyConfig *patch_polyphony_config);

template unsigned getAvailableVoiceIndex<FmVoice>(
    byte channel, byte note, FmVoice voice_options[], unsigned voice_count,
    unsigned last_voice_used, PatchPolyphonyConfig *patch_polyphony_config);
