#ifndef VoiceManager_h
#define VoiceManager_h

#include "Voice.h"
#include <Arduino.h>

// can updated to 12 if multi-chip version is implemented
#define MAX_VOICE_COUNT 3

class VoiceManager {
public:
  VoiceManager(byte voice_count);
  void tick();
  Voice *getVoice(byte channel, byte note);
  Voice *getExactVoice(byte channel, byte note);

  Voice voices[MAX_VOICE_COUNT];

private:
  byte _voice_count;
  byte _voice_round_robin;
};

#endif
