#ifndef Patch_h
#define Patch_h

#include "Envelope.h"
#include "Lfo.h"
#include <Arduino.h>

struct Patch {
  const EnvelopeShape *amplitude_envelope;
  const EnvelopeShape *frequency_envelope;

  const Lfo *amplitude_lfo;
  const Lfo *frequency_lfo;
};

class PatchState {
public:
  PatchState();
  void setPatch(const Patch *patch);
  void initialize();
  void noteOn(byte pitch, byte velocity);
  void noteOff();
  void tick();
  unsigned getFrequencyCents();
  unsigned getLevel();
  bool isActive();
  EnvelopeState amplitude_envelope_state;
  EnvelopeState frequency_envelope_state;

  LfoState amplitude_lfo_state;
  LfoState frequency_lfo_state;

private:
  bool _patch_set;
  const Patch *_patch;
  byte _pitch;
  byte _velocity;
  bool _held;
};

extern const Patch *PRESET_PATCHES[];

#endif
