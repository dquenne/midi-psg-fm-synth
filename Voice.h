#ifndef Voice_h
#define Voice_h

#include "Patch.h"
#include <Arduino.h>

class Voice {
public:
  Voice();
  void setPatch(const Patch *patch);
  void noteOn(byte note, byte velocity);
  void noteOff();
  void tick();

  // state management
  uint16_t frequency_cents;
  byte level;
  byte pitch;

private:
  PatchState _patch_state;
  bool _on;
};

#endif
