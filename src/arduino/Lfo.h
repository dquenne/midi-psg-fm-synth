#ifndef Lfo_h
#define Lfo_h

#include <Arduino.h>

enum LfoWaveform : byte { triangle, square, positive_square, none };

/** LFO wavelengths for different LFO speed values (0-127). Exact calculation
 * is given by this formula:
 *   F = 6 + 0.00009 * (speed - 40)^3
 * giving:
 *   wavelength_ms = 1000 / F = 1000 / (6 + 0.00009 * (speed - 40)^3)
 * i.e. if speed = 64:
 *  F = 6 + 0.00009 * (64-40)^3 = 7.24416Hz
 * giving
 *  wavelength_ms = 1000/7.24416Hz = 128ms
 *
 * This formula is pretty arbitrary and just gave a good range of flexible
 * values, with a higher resolution around 6Hz, since that's a common sweet spot
 * for LFO frequencies. This draws some inspiration from the OPNA/OPN2 built-in
 * LFO values (5 of the 8 values are within about 1Hz of 6Hz).
 *
 * One possible future consideration: remove duplicate values (e.g. 11 different
 * speed values all map to a wavelength of 167ms). Could be accomplished by
 * massaging the formula used to create these, or just skipped duplicates and
 * doing some manual adjustments.
 */
static const uint16_t LFO_WAVELENGTHS[128] = {
    4167, 1512, 942, 694, 555, 467, 406, 362, 328, 301, 280, 263, 248, 236, 226,
    218,  210,  204, 198, 194, 189, 186, 183, 180, 178, 176, 174, 172, 171, 170,
    169,  169,  168, 168, 167, 167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
    166,  166,  166, 165, 165, 164, 163, 162, 161, 160, 159, 157, 155, 153, 151,
    149,  146,  144, 141, 138, 135, 132, 129, 125, 122, 119, 115, 112, 108, 105,
    101,  98,   95,  91,  88,  85,  82,  79,  76,  73,  70,  68,  65,  63,  60,
    58,   56,   54,  52,  50,  48,  46,  44,  42,  41,  39,  38,  36,  35,  34,
    33,   31,   30,  29,  28,  27,  26,  25,  24,  24,  23,  22,  21,  21,  20,
    19,   19,   18,  17,  17,  16,  16,  15,
};

typedef struct Lfo Lfo;

struct Lfo {
  /** If depth is negative, the wave will be flipped. If depth is zero, LFO is
   * disabled. */
  int8_t depth;
  /** LFO speed, from 0.24Hz to 65.3Hz. See LFO_WAVELENGTHS for full details.*/
  byte speed;
  LfoWaveform waveform;

  /** Delay time (0-127), in increments of 16ms (0ms - 2032ms). */
  byte start_delay_time;
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
  unsigned _wavelength;
  unsigned _wavelength_ticks_pased;
  unsigned _start_delay_ticks;
};

#endif
