#include "VoiceManager.h"

template <typename VoiceType>
unsigned getAvailableVoiceIndex(byte channel, byte note,
                                VoiceType voice_options[], unsigned voice_count,
                                unsigned last_voice_used) {
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

template unsigned getAvailableVoiceIndex<PsgVoice>(byte channel, byte note,
                                                   PsgVoice voice_options[],
                                                   unsigned voice_count,
                                                   unsigned last_voice_used);

template unsigned getAvailableVoiceIndex<FmVoice>(byte channel, byte note,
                                                  FmVoice voice_options[],
                                                  unsigned voice_count,
                                                  unsigned last_voice_used);

template <typename VoiceType> void VoiceManager<VoiceType>::tick() {
  for (byte voice_index = 0; voice_index < _voice_count; voice_index++) {
    voices[voice_index].tick();
  }
};

template <typename VoiceType>
VoiceType *VoiceManager<VoiceType>::getVoice(byte channel, byte note) {
  unsigned index = getAvailableVoiceIndex<VoiceType>(
      channel, note, voices, _voice_count, _last_voice_used);

  if (index == NO_AVAILABLE_VOICE_INDEX) {
    return nullptr;
  }
  _last_voice_used = index;
  return getVoiceByIndex(index);
}

/**
 * getExactVoice is intended for handling note off messages. It will only
 * yield voices that match the channel and pitch exactly, and are either in
 * the held or decaying state.
 */
template <typename VoiceType>
VoiceType *VoiceManager<VoiceType>::getExactVoice(byte channel, byte note) {
  VoiceType *voice_hypothesis;
  for (byte voice_index = 0; voice_index < _voice_count; voice_index++) {
    voice_hypothesis = getVoiceByIndex(voice_index);
    if (voice_hypothesis->getStatus() == voice_held &&
        voice_hypothesis->pitch == note &&
        voice_hypothesis->channel == channel) {
      return voice_hypothesis;
    }
  }

  return nullptr;
}

template <typename VoiceType>
VoiceType *VoiceManager<VoiceType>::getVoiceByIndex(unsigned index) {
  return &voices[index];
};

template class VoiceManager<FmVoice>;
template class VoiceManager<PsgVoice>;
