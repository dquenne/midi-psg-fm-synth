#ifndef Patch_h
#define Patch_h

#include "Envelope.h"
#include "Lfo.h"
#include <Arduino.h>

struct PatchDelayConfig {
  bool enable;
  unsigned long delay_ticks;
  signed detune_cents;
  /** Number from 0-15 to be subtracked from loudness. */
  unsigned attenuation;
};

struct Patch {
  EnvelopeShape amplitude_envelope;
  EnvelopeShape frequency_envelope;

  Lfo amplitude_lfo;
  Lfo frequency_lfo;
  PatchDelayConfig delay_config;
  float velocity_scaling;
  signed detune_cents;
};

void applyPreset(Patch *target, const Patch *preset);

class PatchState {
public:
  PatchState();
  void setPatch(const Patch *patch, bool is_delay);
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
  bool _is_delay;
};

extern const Patch *PRESET_PATCHES[];

#endif
