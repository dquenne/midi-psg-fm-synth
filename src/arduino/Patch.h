#ifndef Patch_h
#define Patch_h

#include "Envelope.h"
#include "Lfo.h"
#include <Arduino.h>

/** will extend this to standard 128 when EEPROM storage is implemented */
#define PATCH_BANK_SIZE 16

#define MAX_POLYPHONY_UNLIMITED 0

#define LIMIT(val, min, max) MIN(MAX(val, min), max)
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

struct PatchId {
  byte program_number;  // 7-bit MIDI program number
  uint16_t bank_number; // 14-bit MIDI bank number
};

/** Convert delay time (0-127) to ticks (ms) */
unsigned getDelayTicks(byte delay_time);

struct PatchDelayConfig {
  bool enable;
  /** Delay time (0-127), in increments of 4ms (0ms - 508ms). */
  byte delay_time;
  /** Detune in cents, from -64 to +63. */
  int8_t detune_cents;
  /** Number from 0-15 to be subtracted from loudness. */
  byte attenuation;
};

enum NotePriorityMode : byte {
  NOTE_PRIORITY_MODE_LATEST,
  NOTE_PRIORITY_MODE_HIGHEST,
  NOTE_PRIORITY_MODE_LOWEST
};

enum RetriggerMode : byte {
  /** If voice is stolen, reset envelope to the beginning and reset envelope
     levels to zero. */
  RETRIGGER_MODE_HARD,
  /** If voice is stolen, reset envelope to the beginning but retain envelope
     levels. */
  RETRIGGER_MODE_SOFT,
  /** If voice is stolen, just update params without resetting envelopes. */
  RETRIGGER_MODE_OFF,
};

struct PatchPolyphonyConfig {
  /** 0 = no limit. */
  byte max_polyphony;
  NotePriorityMode note_priority_mode;
  RetriggerMode retrigger_mode;
};

struct Patch {
  PatchDelayConfig delay_config;
  PatchPolyphonyConfig polyphony_config;
};

struct PitchEnvelope {
  signed scaling;
  AdsrEnvelopeShape envelope_shape;
};

class PatchState {
public:
  virtual void initialize() = 0;
  virtual void noteOn(byte pitch, byte velocity, bool retrigger) = 0;
  virtual void noteOff() = 0;
  virtual void tick() = 0;
  virtual unsigned getPitchCents() = 0;
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
  PatchPolyphonyConfig polyphony_config;
  EnvelopeShape amplitude_envelope;
  PitchEnvelope pitch_envelope;

  Lfo amplitude_lfo;
  Lfo pitch_lfo;
  PsgPatchVelocityConfig velocity_config;
  signed detune_cents;
};

void applyPsgPreset(PsgPatch *target, const PsgPatch *preset);

class PsgPatchState : public PatchState {
public:
  PsgPatchState();
  void setPatch(const PsgPatch *patch, bool is_delay);
  const PsgPatch *getPatch() { return _patch; }
  void initialize();
  void noteOn(byte pitch, byte velocity, bool retrigger);
  void noteOff();
  void tick();
  unsigned getPitchCents();
  unsigned getLevel();
  bool isActive();
  EnvelopeState amplitude_envelope_state;
  AdsrEnvelopeState pitch_envelope_state;

  LfoState amplitude_lfo_state;
  LfoState pitch_lfo_state;

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

static const bool FM_CARRIERS_BY_ALGORITHM[8][4] = {
    {false, false, false, true}, {false, false, false, true},
    {false, false, false, true}, {false, false, false, true},
    {false, true, false, true},  {false, true, true, true},
    {false, true, true, true},   {true, true, true, true},
};

struct FmOperator {

  /** Attack speed. 0-31 (5 bit): 31 is instantaneous, at 0 the sound will never
   * start. */
  byte attack_rate;

  /** 0-31 (5 bit): 31 is instantaneous, at 0 the sound will not decay from the
   * maximum level. */
  byte decay_rate;

  /** 0-31 (5 bit): 31 is instantaneous, at 0 the sound will not decay after
   * first decay stage. */
  byte sustain_rate;

  /** 0-15 (4 bit): 15 is instantaneous, at 0 the sound will be held as long as
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

struct FmPatchVelocityConfig {
  /** What velocity has no attenuation/amplification applied. This is typically
   * 64 or 72. */
  byte velocity_center;

  /** Degree of scaling. -64 - 63: Negative numbers mean higher velocity is
   * quieter. 0 means no scaling is applied.
   */
  int8_t operator_scaling[4];
};

/** A superset of the parameters sent to YMxxxx chips, including software-driven
 * parameters like velocity sensitivity and software LFOs.
 */
struct FmPatch {
  FmParameters core_parameters;
  FmPatchVelocityConfig velocity_level_scaling;

  PitchEnvelope pitch_envelope;
  /** This LFO is distinct from some YM chips' built-in global LFO and
   * calculated at a per voice level. */
  Lfo pitch_lfo;
  PatchDelayConfig delay_config;
  PatchPolyphonyConfig polyphony_config;
};

class FmPatchState : public PatchState {
public:
  FmPatchState() {
    _is_patch_set = false;
    _is_delay = false;
  }
  void initialize() {
    pitch_envelope_state.initialize();
    pitch_lfo_state.initialize();
  }
  void setPatch(const FmPatch *patch, bool is_delay) {
    _patch = patch;
    initialize();
    pitch_envelope_state.setEnvelopeShape(
        &_patch->pitch_envelope.envelope_shape);
    pitch_lfo_state.setLfo(&_patch->pitch_lfo);
    _is_patch_set = true;
    _is_delay = is_delay;
  }
  const FmPatch *getPatch() { return _patch; }
  void noteOn(byte pitch, byte velocity, bool retrigger) {
    _pitch = pitch;
    _velocity = velocity;
    _held = true;
    if (retrigger) {
      pitch_envelope_state.initialize();
      pitch_envelope_state.start();
      pitch_lfo_state.initialize();
      pitch_lfo_state.start();
    }
  }
  void noteOff() {
    _held = false;
    pitch_envelope_state.noteOff();
    pitch_lfo_state.noteOff();
  }
  void tick() {
    pitch_envelope_state.tick();
    pitch_lfo_state.tick();
  }
  unsigned getPitchCents() {
    signed pitch_cents = (100 * _pitch) + pitch_lfo_state.getValue();

    pitch_cents = (signed)pitch_cents +
                  _patch->pitch_envelope.scaling * 25 *
                      (signed)pitch_envelope_state.getValue() / 1024;

    return MAX(0, pitch_cents);
  }
  unsigned getOperatorLevel(unsigned op);
  bool isActive() { return _held; }

  AdsrEnvelopeState pitch_envelope_state;
  LfoState pitch_lfo_state;

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

// A curve that seems to give natural / pleasing velocity amplitude scaling.
// Subject to change, but generated from the formula:
//   scaling = sqrt(velocity - 10) * 32 - 10
const static signed VELOCITY_SCALING[] = {
    -219, -214, -210, -205, -201, -197, -192, -189, -185, -181, -177, -174,
    -170, -167, -164, -160, -157, -154, -151, -148, -145, -142, -139, -137,
    -134, -131, -128, -126, -123, -121, -118, -116, -113, -111, -108, -106,
    -103, -101, -99,  -96,  -94,  -92,  -90,  -88,  -85,  -83,  -81,  -79,
    -77,  -75,  -73,  -71,  -69,  -67,  -64,  -63,  -61,  -59,  -57,  -55,
    -53,  -51,  -49,  -47,  -45,  -43,  -42,  -40,  -38,  -36,  -34,  -32,
    -31,  -29,  -27,  -25,  -24,  -22,  -20,  -19,  -17,  -15,  -14,  -12,
    -10,  -9,   -7,   -5,   -4,   -2,   0,    1,    3,    4,    6,    7,
    9,    11,   12,   14,   15,   17,   18,   20,   21,   23,   24,   26,
    27,   29,   30,   32,   33,   34,   36,   37,   39,   40,   42,   43,
    44,   46,   47,   49,   50,   51,   53,   54,
};

#endif
