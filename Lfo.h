#ifndef Lfo_h
#define Lfo_h

#include <Arduino.h>

enum LfoWaveform { triangle, square, positive_square };

typedef struct Lfo Lfo;

struct Lfo {
  /** If depth is negative, the wave will be flipped. If depth is zero, LFO is
   * disabled. */
  signed depth;
  /** LFO wavelength in number of ticks. */
  unsigned wavelength;
  LfoWaveform waveform;

  unsigned start_delay_ticks;
  unsigned off_ticks;
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

#endif
