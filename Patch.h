#ifndef Patch_h
#define Patch_h

#include "Envelope.h"
#include "Lfo.h"
#include <Arduino.h>

/** will extend this to standard 128 when EEPROM storage is implemented */
#define PATCH_BANK_SIZE 16

struct PatchId {
  byte program_number;  // 7-bit MIDI program number
  uint16_t bank_number; // 14-bit MIDI bank number
};

struct PatchDelayConfig {
  bool enable;
  unsigned long delay_ticks;
  signed detune_cents;
  /** Number from 0-15 to be subtracted from loudness. */
  unsigned attenuation;
};

struct Patch {
  PatchDelayConfig delay_config;
};

class PatchState {
public:
  virtual void initialize() = 0;
  virtual void noteOn(byte pitch, byte velocity) = 0;
  virtual void noteOff() = 0;
  virtual void tick() = 0;
  virtual unsigned getFrequencyCents() = 0;
  virtual bool isActive() = 0;

protected:
  byte _pitch;
  byte _velocity;
  bool _held;
  bool _is_delay;
};

// PSG

struct PsgPatchVelocityConfig {
  /** What velocity has no attenuation applied. This is typically 64 or 72. */
  unsigned velocity_center;
  /** At what velocity difference is the attenuation changed. A smaller number
   * means a smaller change in velocity will have a more dramatic effect.
   * e.g. if interval=16, then we have:
   * attentuation = (actual_velocity - velocity_center) / interval
   * so for every 16 lower than velocity_center, attenuation is increased by 1.
   * 0 = no scaling.
   */
  signed interval;
};

struct PsgPatch {
  PatchDelayConfig delay_config;
  EnvelopeShape amplitude_envelope;
  EnvelopeShape frequency_envelope;

  Lfo amplitude_lfo;
  Lfo frequency_lfo;
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
};

// FM

/** YM chips represent panning with a 2-bit number, where the least-significant
 * bit is the right channel, and the most-significant bit is the left channel. 1
 * is on, 0 is off.*/
enum FmPanningMode : byte {
  FM_PANNING_OFF = 0b00,
  FM_PANNING_RIGHT = 0b01,
  FM_PANNING_LEFT = 0b10,
  FM_PANNING_CENTER = 0b11
};

struct FmOperator {

  /** Attack speed. 0-31 (5 bit): 31 is instantaneous, at 0 the sound will never
   * start. */
  byte attack_rate;

  /** 0-31 (5 bit): 31 is instantaneous, at 0 the sound will not decay from the
   * maximum level. */
  byte decay_rate;

  /** 0-15 (4 bit): 15 is instantaneous, at 0 the sound will not decay after
   * first decay stage. */
  byte sustain_rate;

  /** 0-31 (5 bit): 31 is instantaneous, at 0 the sound will be held as long as
   * possible. */
  byte release_rate;

  /** Attenuation after decay rate. 0-15 (4 bit): 0 is no volume drop, 15 means
   * the first decay stage will decay to silent. */
  byte sustain_level;

  /** Attenuation. 0-127 (7 bit): 127 is off, 0 is maximum level.*/
  byte total_level;

  byte key_scale;

  /** Note frequency multiplier. 0-15 (5-bit): At 0, this will be half the
   * actual frequency of the note played. For 1-15 this is the multiplier times
   * the original note's frequency.*/
  byte multiple;

  byte detune;

  bool lfo_amplitude_enable;
};

/** These are parameters common to YMxxxx chips. */
struct FmParameters {
  FmPanningMode panning;
  byte feedback;
  byte algorithm;
  byte lfo_amplitude_sensitivity;
  byte lfo_frequency_sensitivity;
  FmOperator operators[4];
};

// WIP, not implemented
struct FmPatchVelocityConfig {
  /** What velocity has no attenuation/amplification applied. This is typically
   * 64 or 72. */
  unsigned velocity_center;

  /** At what velocity difference is the total level for the operator changed. A
   * smaller number means a smaller change in velocity will have a more dramatic
   * effect. e.g. if interval=16, then we have: attentuation = (actual_velocity
   * - velocity_center) / interval so for every 16 lower than velocity_center,
   * attenuation is increased by 1.
   * One value per operator.
   * 0 = no scaling.
   */
  signed interval[4];
};

/** A superset of the parameters sent to YMxxxx chips, including software-driven
 * parameters like velocity sensitivity and software LFOs.
 */
struct FmPatch {
  PatchDelayConfig delay_config;
  FmParameters core_parameters;
  FmPatchVelocityConfig velocity_level_scaling;
};

class FmPatchState : public PatchState {
public:
  void initialize() {
    _is_patch_set = false;
    _is_delay = false;
  }
  void setPatch(const FmPatch *patch, bool is_delay) {
    initialize();
    _patch = patch;
    _is_patch_set = true;
    _is_delay = is_delay;
  }
  const FmPatch *getPatch() { return _patch; }
  void noteOn(byte pitch, byte velocity) {
    _pitch = pitch;
    _velocity = velocity;
    _held = true;
  }
  void noteOff() { _held = false; }
  void tick() {}
  unsigned getFrequencyCents() { return 100 * _pitch; }
  bool isActive() { return _held; }

private:
  const FmPatch *_patch;
  bool _is_patch_set;
};

void applyFmPreset(FmPatch *target, const FmPatch *preset);

// storage stuff

struct PsgPatchBank {
  PsgPatch patches[16];
};

struct FmPatchBank {
  FmPatch patches[16];
};

#endif
