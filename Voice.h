#ifndef Voice_h
#define Voice_h

#include "Patch.h"
#include <Arduino.h>

enum VoiceStatus { voice_off, voice_held, voice_decay };

class Voice {
public:
  virtual void noteOn(byte _channel, byte _pitch, byte velocity) = 0;
  virtual void noteOff() = 0;
  virtual void tick() = 0;
  VoiceStatus getStatus();
  bool getIsDelay();
  byte pitch;
  byte channel;

protected:
  bool _on;
  bool _held;
  bool _is_delay;
};

class PsgVoice : public Voice {
public:
  PsgVoice();
  void setPatch(PsgPatch *patch, bool is_delay);
  void noteOn(byte _channel, byte _pitch, byte velocity);
  void noteOff();
  void tick();

  byte detune_cents;
  // state management
  uint16_t frequency_cents;
  byte level;

protected:
  PsgPatchState _patch_state;
};

#endif
