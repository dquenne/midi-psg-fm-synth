#ifndef Ym2203_h
#define Ym2203_h

#include "Chip.h"
#include <Arduino.h>

// The YM2203 data sheet doesn't have clear guidance on how many cycles to wait
// when writing to registers. These numbers are guesses based on the YM2149 and
// YM2608 data sheets. If writes seem to be missing, it's worth trying
// increasing these.
#define YM2203_ADDRESS_WRITE_WAIT_FM 40
#define YM2203_ADDRESS_WRITE_WAIT_SSG 4 // 40
#define YM2203_DATA_WRITE_WAIT_FM 83
#define YM2203_DATA_WRITE_WAIT_SSG 4 // 40

#define YM2203_CHANNEL_MASK 0b11 // note: this is 0b111 for YM2608

// PSG
#define YM2203_MAX_FREQUENCY_N 4095 // max possible 12-bit unsigned int

#define YM2203_ADDRESS_PSG_FREQUENCY_FINE(channel) (0x00 + channel * 2)
#define YM2203_ADDRESS_PSG_FREQUENCY_COARSE(channel) (0x01 + channel * 2)
#define YM2203_ADDRESS_PSG_NOISE_PERIOD 0x06
#define YM2203_ADDRESS_PSG_NOISE_TONE_ENABLE 0x07
#define YM2203_ADDRESS_PSG_LEVEL(channel) (0x08 + channel)
#define YM2203_ADDRESS_PSG_ENVELOPE_PERIOD_FINE 0x0B
#define YM2203_ADDRESS_PSG_ENVELOPE_PERIOD_COARSE 0x0C
#define YM2203_ADDRESS_PSG_ENVELOPE_SHAPE_CYCLE 0x0D
// note: I/O port ADDRESSs are omitted as they are not used

/* Masks & offsets for building messages */
#define YM2203_PSG_FREQUENCY_COARSE_MASK 0b1111
#define YM2203_PSG_FREQUENCY_FINE_MASK 0b11111111

#define YM2203_PSG_LEVEL_MASK 0b1111
#define YM2203_PSG_LEVEL_OFFSET 0
#define YM2203_PSG_LEVEL_ENVELOPE_ENABLE_MASK 0b1
#define YM2203_PSG_LEVEL_ENVELOPE_ENABLE_OFFSET 4

#define YM2203_PSG_NOISE_FREQUENCY_MASK 0b11111

#define YM2203_PSG_MIXER_TONE_OFFSET(channel) (0 + channel)
#define YM2203_PSG_MIXER_NOISE_OFFSET(channel) (3 + channel)

class Ym2203Instance;

struct Ym2203PsgAddressSet {
  byte frequency_fine;
  byte frequency_coarse;
  byte level;
  unsigned mixer_tone_offset;
  unsigned mixer_noise_offset;
};

class Ym2203PsgChannel : public PsgChannel {
public:
  Ym2203PsgChannel();
  Ym2203PsgChannel(unsigned channel);

  void setChannel(unsigned channel);
  void initialize();

  // write methods in terms of pitch & level
  void writePitch(unsigned frequency_cents);
  void writeLevel(unsigned level, bool force = false);

private:
  void _writeFrequencyN(unsigned frequency_n);

  Ym2203Instance *_ym2203_instance;
  unsigned _channel;
  unsigned _last_frequency_written;
  unsigned _last_level_written;
};

class Ym2203Instance : public Chip {
public:
  Ym2203Instance(unsigned clock_division);
  Ym2203PsgChannel psg_channels[3];
  PsgChannel *getPsgChannel(unsigned channel_number) {
    return &psg_channels[channel_number];
  };
  void setup();
  void write(byte address, byte data);

private:
  void _writeAddress(byte address, unsigned delay_cycles);
  void _writeData(byte data, unsigned delay_cycles);
  unsigned _clock_division;
};

#endif
