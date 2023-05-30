#ifndef Patch_h
#define Patch_h

#include "Envelope.h"
#include "Lfo.h"
#include <Arduino.h>

struct PatchVelocityConfig {
  /** What velocity has no attenuation applied. This is typically 64 or 72. */
  unsigned velocity_center;
  /** At what velocity difference is the attenuation changed. A smaller number
   * means a smaller change in velocity will have a more dramatic effect.
   * e.g. if interval=16, then we have:
   * attentuation = (actual_velocity - velocity_center) / interval
   * so for every 16 lower than velocity_center, attenuation is increased by 1.
   */
  signed interval;
};

struct PatchDelayConfig {
  bool enable;
  unsigned long delay_ticks;
  signed detune_cents;
  /** Number from 0-15 to be subtracted from loudness. */
  unsigned attenuation;
};

struct Patch {
  EnvelopeShape amplitude_envelope;
  EnvelopeShape frequency_envelope;

  Lfo amplitude_lfo;
  Lfo frequency_lfo;
  PatchDelayConfig delay_config;
  PatchVelocityConfig velocity_config;
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
