#ifndef Envelope_h
#define Envelope_h

#include <Arduino.h>

// Keeping this relatively small because too many changes in attenuation seems
// to sound bad on the SN76489AN - there's clicks every time it changes.
#define MAX_ENVELOPE_STEP_COUNT 5

#define ADSR_ENVELOPE_MAX_VALUE 1024

typedef struct EnvelopeStep EnvelopeStep;

struct EnvelopeStep {
  /** Amplitude value between 0 and 15, inclusive, where 0 is silent and 15 is
   * maximum loudness.
   */
  unsigned value;
  unsigned hold_ticks;
};

typedef struct EnvelopeShape EnvelopeShape;

struct EnvelopeShape {
  EnvelopeStep steps[MAX_ENVELOPE_STEP_COUNT];

  bool loop_enable;

  // default values have 2 attack stages, 1 hold stage, and 2 release stages
  unsigned loop_after_step;
  unsigned loop_to_step;

  /* if 0, continue from current step without jump */
  unsigned on_off_jump_to_step;

  unsigned end_after_step;
};

enum EnvelopeStatus { not_started, active, done };

class EnvelopeState {
public:
  EnvelopeState();
  void setEnvelopeShape(const EnvelopeShape *envelope_shape);
  void initialize();
  void start();
  void noteOff();
  void step();
  bool tick();
  unsigned getValue();
  EnvelopeStatus getStatus();

private:
  const EnvelopeShape *_envelope_shape;
  bool _started;
  bool _held;
  unsigned _step;
  unsigned _step_ticks_passed;
};

/**
 * Each value should be 0-127. For timing values (attack, sustain, release),
 * a larger value is slower. For sustain, a larger value is louder; 0 is silent.
 */
struct AdsrEnvelopeShape {
  unsigned attack;
  unsigned decay;
  unsigned sustain;
  unsigned release;
};

enum AdsrEnvelopeStage {
  ADSR_STAGE_PRE_START,
  ADSR_STAGE_ATTACK,
  ADSR_STAGE_DECAY,
  ADSR_STAGE_RELEASE,
  ADSR_STAGE_DONE
};

static const uint16_t ADSR_INCREMENTS_PER_STEP[128] = {
    512, 330, 256, 180, 150, 120, 100, 70, 62, 58, 54, 50, 46, 42, 38, 34,
    30,  29,  28,  27,  26,  25,  24,  23, 22, 21, 20, 19, 18, 17, 16, 15,
    57,  56,  55,  54,  53,  52,  51,  50, 49, 48, 47, 46, 45, 44, 43, 42,
    41,  40,  39,  38,  37,  36,  35,  34, 33, 32, 31, 30, 29, 28, 27, 26,
    51,  50,  49,  48,  47,  46,  45,  44, 43, 42, 41, 40, 39, 38, 37, 36,
    35,  34,  33,  32,  31,  30,  29,  28, 27, 26, 25, 24, 23, 22, 21, 20,
    38,  37,  36,  35,  34,  33,  32,  31, 30, 29, 28, 27, 26, 25, 24, 23,
    22,  21,  20,  19,  18,  17,  16,  15, 14, 13, 12, 11, 10, 9,  8,  7,
};

static const uint16_t ADSR_STEP_TICKS[128] = {
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
    4,  4,  4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
    8,  16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

class AdsrEnvelopeState {
public:
  AdsrEnvelopeState() { initialize(); };
  void setEnvelopeShape(const AdsrEnvelopeShape *envelope_shape) {
    _envelope_shape = envelope_shape;
    initialize();
  };
  void initialize();
  void start();
  void noteOff();
  void step();
  bool tick();
  unsigned getValue();
  EnvelopeStatus getStatus();

private:
  void _setStage(AdsrEnvelopeStage stage);
  void _stageStep();
  unsigned _getStageRate();
  unsigned _getStageTicksPerStep();
  unsigned _getStageIncrementPerStep();
  unsigned _getStageTargetValue();

  const AdsrEnvelopeShape *_envelope_shape;

  AdsrEnvelopeStage _stage;
  unsigned _stage_start_value;
  unsigned _stage_ticks_passed;

  unsigned _stage_step;

  unsigned _value;
};

#endif
