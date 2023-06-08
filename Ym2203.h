#ifndef Ym2203_h
#define Ym2203_h

#include "Chip.h"
#include "Patch.h"
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

// FM
#define YM2203_MAX_F_NUMBER 2047 // max possible 11-bit unsigned int

#define PER_CHAN(start, channel) (start + channel)
#define PER_OP(start, channel, op) (start + (op * 4) + channel)

#define YM2203_ADDRESS_FM_SLOT_KEY_ON 0x28

#define YM2203_ADDRESS_FM_DETUNE_MULTIPLE(chan, op) PER_OP(0x30, chan, op)
#define YM2203_ADDRESS_FM_TOTAL_LEVEL(chan, op) PER_OP(0x40, chan, op)
#define YM2203_ADDRESS_FM_KEY_SCALE_ATTACK_RATE(chan, op) PER_OP(0x50, chan, op)
#define YM2203_ADDRESS_FM_DECAY_RATE(chan, op) PER_OP(0x60, chan, op)
#define YM2203_ADDRESS_FM_SUSTAIN_RATE(chan, op) PER_OP(0x70, chan, op)
#define YM2203_ADDRESS_FM_SUSTAIN_LEVEL_RELEASE(chan, op) PER_OP(0x80, chan, op)
#define YM2203_ADDRESS_FM_SSG_EG(chan, op) PER_OP(0x90, chan, op)

#define YM2203_ADDRESS_FM_F_NUMBER_FINE(chan) PER_CHAN(0xA0, chan)
#define YM2203_ADDRESS_FM_F_NUMBER_COARSE_AND_BLOCK(chan) PER_CHAN(0xA4, chan)
#define YM2203_ADDRESS_FM_FEEDBACK_ALGORITHM(chan) PER_CHAN(0xB0, chan)

/* Masks & offsets for building messages */
#define YM2203_FM_SLOT_OFFSET 4

#define YM2203_FM_F_BLOCK_MASK 0b111
#define YM2203_FM_F_NUMBER_COARSE_MASK 0b111
#define YM2203_FM_F_NUMBER_FINE_MASK 0b11111111
#define YM2203_FM_F_BLOCK_OFFSET 3

#define YM2203_FM_DETUNE_MASK 0b111
#define YM2203_FM_DETUNE_OFFSET 4
#define YM2203_FM_MULTIPLE_MASK 0b1111

#define YM2203_FM_TOTAL_LEVEL_MASK 0b01111111

#define YM2203_FM_KEY_SCALE_MASK 0b11
#define YM2203_FM_KEY_SCALE_OFFSET 6
#define YM2203_FM_ATTACK_RATE_MASK 0b11111

#define YM2203_FM_DECAY_RATE_MASK 0b11111
#define YM2203_FM_SUSTAIN_RATE_MASK 0b11111

#define YM2203_FM_SUSTAIN_LEVEL_MASK 0b1111
#define YM2203_FM_SUSTAIN_LEVEL_OFFSET 4
#define YM2203_FM_RELEASE_RATE_MASK 0b1111

#define YM2203_FM_FEEDBACK_MASK 0b111
#define YM2203_FM_FEEDBACK_OFFSET 3
#define YM2203_FM_ALGORITHM_MASK 0b111

class Ym2203Instance;

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

class Ym2203FmChannel : public FmChannel {
public:
  Ym2203FmChannel();

  void setChannel(unsigned channel);
  void initialize();

  // write methods in terms of pitch & level
  void writeStaticPatchParameters(const FmPatch *patch);
  void writePitch(unsigned frequency_cents);
  void writeKeyOnOff(bool key_on, bool force = false);

  void writeFeedbackAlgorithm(unsigned feedback, unsigned algorithm);
  void writeDetuneMultiple(unsigned op, unsigned detune, unsigned multiple);
  void writeTotalLevel(unsigned op, unsigned total_level);
  void writeKeyScaleAttackRate(unsigned op, unsigned key_scale,
                               unsigned attack_rate);
  void writeDecayRate(unsigned op, unsigned decay_rate);
  void writeSustainRate(unsigned op, unsigned sustain_rate);
  void writeSustainLevelRelease(unsigned op, unsigned sustain_level,
                                unsigned release_rate);

private:
  void _writeOperatorParameters(unsigned op, const FmOperator *operator_params);
  void _writeFNumberBlock(unsigned block, unsigned f_number);

  Ym2203Instance *_ym2203_instance;
  unsigned _channel;
  unsigned _last_f_number_written;
  unsigned _last_f_block_written;
  bool _last_key_on_written;
};

class Ym2203Instance : public Chip {
public:
  Ym2203Instance(unsigned clock_division);
  Ym2203PsgChannel psg_channels[3];
  Ym2203FmChannel fm_channels[3];

  Ym2203PsgChannel *getPsgChannel(unsigned channel_number) {
    return &psg_channels[channel_number];
  };
  Ym2203FmChannel *getFmChannel(unsigned channel_number) {
    return &fm_channels[channel_number];
  };
  void setup();
  void write(byte address, byte data);

private:
  void _writeAddress(byte address, unsigned delay_cycles);
  void _writeData(byte data, unsigned delay_cycles);
  unsigned _clock_division;
};

#endif
