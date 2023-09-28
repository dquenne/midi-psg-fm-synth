#ifndef VoiceManager_h
#define VoiceManager_h

#include "Voice.h"
#include <Arduino.h>

/*This is the max number supported by any chip, just needs to be at least as big
 * as the chip with the most number of voices of a common type. The actual
 * bounds of a VoiceManager are separate - this is just for memory allocation.
 * Can updated if multi-chip version is implemented or chips like YM2608 are
 * supported (since it has 6 FM channels).*/
#define MAX_VOICE_COUNT 3

#define NO_AVAILABLE_VOICE_INDEX 999

template <typename VoiceType>
unsigned getAvailableVoiceIndex(byte channel, byte pitch,
                                VoiceType voice_options[], unsigned voice_count,
                                unsigned last_voice_used,
                                PatchPolyphonyConfig *patch_polyphony_config);

template <typename VoiceType> class VoiceManager {
public:
  VoiceManager(byte voice_count)
      : _voice_count(voice_count), _last_voice_used{0} {}

  void tick();

  VoiceType *getVoice(byte channel, byte note,
                      PatchPolyphonyConfig *patch_polyphony_config);

  /**
   * getExactVoice is intended for handling note off messages. It will only
   * yield voices that match the channel and pitch exactly, and are either in
   * the held or decaying state.
   */
  VoiceType *getExactVoice(byte channel, byte note);

  VoiceType *getVoiceByIndex(unsigned index);

private:
  VoiceType voices[MAX_VOICE_COUNT];
  VoiceType _null_voice;
  unsigned _voice_count;
  unsigned _last_voice_used;
};

#endif
