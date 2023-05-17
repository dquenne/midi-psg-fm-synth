#ifndef Envelope_h
#define Envelope_h

#include <Arduino.h>

// Keeping this relatively small because too many changes in attenuation seems
// to sound bad on the SN76489AN - there's clicks every time it changes.
#define MAX_ENVELOPE_STEP_COUNT 5

typedef struct EnvelopeStep EnvelopeStep;

struct EnvelopeStep {
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

#endif
