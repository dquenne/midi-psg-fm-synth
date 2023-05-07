#ifndef Lfo_h
#define Lfo_h

#include <Arduino.h>

enum LfoWaveform { triangle, square, positive_square };

typedef struct Lfo Lfo;

struct Lfo {
  bool lfo_enable;
  unsigned start_delay_ticks;
  unsigned off_ticks;

  /** If depth is negative, the wave will be flipped.*/
  signed depth;
  unsigned wavelength;
  LfoWaveform waveform;
};

class LfoState {
public:
  LfoState();
  void setLfo(const Lfo *lfo);
  void initialize();
  void start();
  void noteOff();
  void done();
  bool tick();
  signed getValue();

  unsigned amplitude_lfo_ticks_passed;
  unsigned frequency_lfo_ticks_passed;

private:
  const Lfo *_lfo;
  bool _active;
  bool _held;
  unsigned _ticks_passed;
  unsigned _wavelength_ticks_pased;
};

extern const Lfo *LFO_PRESETS[];

#endif
