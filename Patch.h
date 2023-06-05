#ifndef Patch_h
#define Patch_h

#include "Envelope.h"
#include "Lfo.h"
#include <Arduino.h>

class PatchState {
public:
  virtual void initialize() = 0;
  virtual void noteOn(byte pitch, byte velocity) = 0;
  virtual void noteOff() = 0;
  virtual void tick() = 0;
  virtual unsigned getFrequencyCents() = 0;
  virtual bool isActive() = 0;

private:
  byte _pitch;
  byte _velocity;
  bool _held;
  bool _is_delay;
};

struct PsgPatchVelocityConfig {
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

struct PsgPatchDelayConfig {
  bool enable;
  unsigned long delay_ticks;
  signed detune_cents;
  /** Number from 0-15 to be subtracted from loudness. */
  unsigned attenuation;
};

struct PsgPatch {
  EnvelopeShape amplitude_envelope;
  EnvelopeShape frequency_envelope;

  Lfo amplitude_lfo;
  Lfo frequency_lfo;
  PsgPatchDelayConfig delay_config;
  PsgPatchVelocityConfig velocity_config;
  signed detune_cents;
};

void applyPsgPreset(PsgPatch *target, const PsgPatch *preset);

class PsgPatchState : public PatchState {
public:
  PsgPatchState();
  void setPatch(const PsgPatch *patch, bool is_delay);
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
  const PsgPatch *_patch;
  byte _pitch;
  byte _velocity;
  bool _held;
  bool _is_delay;
};

#endif
