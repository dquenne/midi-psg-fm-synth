#ifndef Envelope_h
#define Envelope_h

#include <Arduino.h>

#define ADSR_ENVELOPE_MAX_VALUE 1024

enum EnvelopeStatus { not_started, active, done };

/**
 * Each value should be 0-127. For timing values (attack, sustain, release),
 * a larger value is slower. For sustain, a larger value is louder; 0 is silent.
 */
struct AdsrEnvelopeShape {
  byte attack;
  byte decay;
  byte sustain;
  byte release;
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
