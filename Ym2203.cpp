#include "Ym2203.h"
#include "IoUtils.h"
#include "NoteMappings.h"
#include <Arduino.h>

Ym2203Instance::Ym2203Instance(unsigned clock_division) {
  _clock_division = clock_division;
}

void Ym2203Instance::setup() {
  setWriteEnable(false);

  // set internal clock division to 1/2 (normal for 4MHz clock, I think)
  // This system is poorly documented for the YM2203, but based on them YM2149
  // & YM2608 data sheets, by just writing one of the addresses 2D, 2E, and/or
  // 2F, the clock division is set to:
  //   2D:    1/4  -> F = (4MHz/4) / (16 * N)  = 125kHz/N
  //   2D,2E: 1/2  -> F = (4MHz/2) / (16 * N)  = 250kHz/N  <- this is default
  //   2F:    1/1  -> F =   4MHz   / (16 * N)  = 500kHz/N
  _writeAddress(0x2d, YM2203_ADDRESS_WRITE_WAIT_SSG);
  _writeAddress(0x2e, YM2203_ADDRESS_WRITE_WAIT_SSG);

  // TODO: figure out how to manage noise enable per-channel in a sane way
  write(YM2203_ADDRESS_PSG_NOISE_TONE_ENABLE, 0b11111000);

  psg_channels[0].setChannel(0);
  psg_channels[1].setChannel(1);
  psg_channels[2].setChannel(2);
  psg_channels[0].initialize();
  psg_channels[1].initialize();
  psg_channels[2].initialize();

  fm_channels[0].setChannel(0);
  fm_channels[1].setChannel(1);
  fm_channels[2].setChannel(2);
  fm_channels[0].initialize();
  fm_channels[1].initialize();
  fm_channels[2].initialize();
}

void Ym2203Instance::_writeAddress(byte address, unsigned delay_cycles) {
  setByteOut(address);
  setA0(false);
  clockDelay(4);
  setWriteEnable(true);
  clockDelay(delay_cycles);
  setWriteEnable(false);
  clockDelay(4);
}

void Ym2203Instance::_writeData(byte data, unsigned delay_cycles) {
  setByteOut(data);
  setA0(true);
  clockDelay(4);
  setWriteEnable(true);
  clockDelay(delay_cycles);
  setWriteEnable(false);
  clockDelay(4);
}

void Ym2203Instance::write(byte address, byte data) {
  _writeAddress(address, YM2203_ADDRESS_WRITE_WAIT_SSG);

  clockDelay(4);
  _writeData(data, YM2203_DATA_WRITE_WAIT_SSG);
  clockDelay(40);
}

// PSG

Ym2203PsgChannel::Ym2203PsgChannel() {
  _channel = 0;
  _last_frequency_written = 0;
  _last_level_written = 0;
}
Ym2203PsgChannel::Ym2203PsgChannel(unsigned channel) { _channel = channel; }

void Ym2203PsgChannel::setChannel(unsigned channel) { _channel = channel; }

void Ym2203PsgChannel::initialize() { writeLevel(0, true); }

/**
 * @param n is a 12-bit number to be divided by CLOCK_FREQUENCY / 16 to get PSG
 * channel output.
 */
void Ym2203PsgChannel::_writeFrequencyN(unsigned frequency_n) {
  if (frequency_n > YM2203_MAX_FREQUENCY_N) {
    // not the most elegant thing, but if the frequency N is too high (i.e.
    // pitch is too low), just return early and do nothing
    return;
  }
  if (_last_frequency_written == frequency_n) {
    return;
  }
  unsigned n_top_4_bits = (frequency_n >> 8) & YM2203_PSG_FREQUENCY_COARSE_MASK;
  unsigned n_lower_8_bits = frequency_n & YM2203_PSG_FREQUENCY_FINE_MASK;

  // When making a shift in frequency that flips every bit in the fine tune
  // register, (e.g changing N from 0b0010_11111111 to 0b0011_00000000), the
  // order that the two registers get written is important. Taking the above
  // example, if the fine tuning register is written first, for a few clock
  // cycles, N will dip much lower than it should (in this example it would
  // briefly drop from 0b0010_11111111 to 0b0010_00000000). When this happens,
  // the YM2203 often forces the tone generator to end its cycle early (per the
  // new, much lower N -> shorter wavelength), but then as soon as the coarse
  // pitch is written, the wavelength is back to where it should be and the tone
  // generator resets again, causing popping sounds. If written in the reverse
  // order, the N value will temporarily become higher briefly, not interrupting
  // the current cycle (the cycle won't be reset if the wavelength briefly
  // lengthens). If changing from a higher to a lower N value, the opposite is
  // true (coarse must be written first to avoid pops), hence the two cases.
  if (frequency_n < _last_frequency_written) {
    _ym2203_instance->write(YM2203_ADDRESS_PSG_FREQUENCY_FINE(_channel),
                            n_lower_8_bits);
    _ym2203_instance->write(YM2203_ADDRESS_PSG_FREQUENCY_COARSE(_channel),
                            n_top_4_bits);
  } else {
    _ym2203_instance->write(YM2203_ADDRESS_PSG_FREQUENCY_COARSE(_channel),
                            n_top_4_bits);
    _ym2203_instance->write(YM2203_ADDRESS_PSG_FREQUENCY_FINE(_channel),
                            n_lower_8_bits);
  }

  _last_frequency_written = frequency_n;
}

/**
 * @param frequency_cents is the number of cents from MIDI note 0, in other
 * words it is 100 * midi_note + offset_cents.
 */
void Ym2203PsgChannel::writePitch(unsigned frequency_cents) {
  _writeFrequencyN(getFrequencyN(frequency_cents, NOTES_250KHZ));
}

/**
 * @param level: a 4-bit number, where 15 is loudest and 0 is silent, in steps
 * of 3dB (i.e. level=15 is twice as loud as level=13).
 */
void Ym2203PsgChannel::writeLevel(unsigned level, bool force) {
  byte normalized_level = level;
  if (level >= 15) {
    normalized_level = 15;
  }

  if (!force && _last_level_written == normalized_level) {
    return;
  }

  _ym2203_instance->write(YM2203_ADDRESS_PSG_LEVEL(_channel),
                          normalized_level & YM2203_PSG_LEVEL_MASK);
  _last_level_written = normalized_level;
}

// FM

Ym2203FmChannel::Ym2203FmChannel() {
  _channel = 0;
  _last_f_number_written = 0;
  _last_f_block_written = 0;
}

void Ym2203FmChannel::setChannel(unsigned channel) { _channel = channel; }

void Ym2203FmChannel::initialize() {
  for (unsigned op = 0; op < 4; op++) {
    writeKeyOnOff(false, true);
    writeTotalLevel(op, 127);
  }
}

void Ym2203FmChannel::writeFeedbackAlgorithm(unsigned feedback,
                                             unsigned algorithm) {
  byte data =
      ((feedback & YM2203_FM_FEEDBACK_MASK) << YM2203_FM_FEEDBACK_OFFSET) |
      (algorithm & YM2203_FM_ALGORITHM_MASK);

  _ym2203_instance->write(YM2203_ADDRESS_FM_FEEDBACK_ALGORITHM(_channel), data);
}

void Ym2203FmChannel::writeDetuneMultiple(unsigned op, unsigned detune,
                                          unsigned multiple) {
  byte data = ((detune & YM2203_FM_DETUNE_MASK) << YM2203_FM_DETUNE_OFFSET) |
              (multiple & YM2203_FM_MULTIPLE_MASK);

  _ym2203_instance->write(YM2203_ADDRESS_FM_DETUNE_MULTIPLE(_channel, op),
                          data);
}

void Ym2203FmChannel::writeTotalLevel(unsigned op, unsigned total_level) {
  byte data = (total_level & YM2203_FM_TOTAL_LEVEL_MASK);

  _ym2203_instance->write(YM2203_ADDRESS_FM_TOTAL_LEVEL(_channel, op), data);
}

void Ym2203FmChannel::writeKeyScaleAttackRate(unsigned op, unsigned key_scale,
                                              unsigned attack_rate) {
  byte data =
      ((key_scale & YM2203_FM_KEY_SCALE_MASK) << YM2203_FM_KEY_SCALE_MASK) |
      (attack_rate & YM2203_FM_ATTACK_RATE_MASK);

  _ym2203_instance->write(YM2203_ADDRESS_FM_KEY_SCALE_ATTACK_RATE(_channel, op),
                          data);
}

void Ym2203FmChannel::writeDecayRate(unsigned op, unsigned decay_rate) {
  byte data = (decay_rate & YM2203_FM_DECAY_RATE_MASK);

  _ym2203_instance->write(YM2203_ADDRESS_FM_DECAY_RATE(_channel, op), data);
}

void Ym2203FmChannel::writeSustainRate(unsigned op, unsigned sustain_rate) {
  byte data = (sustain_rate & YM2203_FM_SUSTAIN_RATE_MASK);

  _ym2203_instance->write(YM2203_ADDRESS_FM_SUSTAIN_RATE(_channel, op), data);
}

void Ym2203FmChannel::writeSustainLevelRelease(unsigned op,
                                               unsigned sustain_level,
                                               unsigned release_rate) {
  byte data = ((sustain_level & YM2203_FM_SUSTAIN_LEVEL_MASK)
               << YM2203_FM_SUSTAIN_LEVEL_OFFSET) |
              (release_rate & YM2203_FM_RELEASE_RATE_MASK);

  _ym2203_instance->write(YM2203_ADDRESS_FM_SUSTAIN_LEVEL_RELEASE(_channel, op),
                          data);
}

void Ym2203FmChannel::_writeOperatorParameters(
    unsigned op, const FmOperator *operator_params) {
  writeDetuneMultiple(op, operator_params->detune, operator_params->multiple);
  writeTotalLevel(op, operator_params->total_level);
  writeKeyScaleAttackRate(op, operator_params->key_scale,
                          operator_params->attack_rate);
  writeDecayRate(op, operator_params->decay_rate);
  writeSustainRate(op, operator_params->sustain_rate);
  writeSustainLevelRelease(op, operator_params->sustain_level,
                           operator_params->release_rate);
}

void Ym2203FmChannel::writeAllPatchParameters(const FmPatch *patch) {
  writeFeedbackAlgorithm(patch->core_parameters.feedback,
                         patch->core_parameters.algorithm);
  for (unsigned op = 0; op < 4; op++) {
    _writeOperatorParameters(op, &patch->core_parameters.operators[op]);
  }
}

/**
 * @param block is a 3-bit number indicating the note's octave.
 * @param f_number is a 11-bit F-Number per data sheet specification.
 */
void Ym2203FmChannel::_writeFNumberBlock(unsigned block, unsigned f_number) {
  if (f_number > YM2203_MAX_F_NUMBER) {
    // this needs to get caught upstream. if the pitch is being set to an
    // invalid f_number, good chance the other parameters are being synced, so
    // it will lead to weird behavior no matter what gets done here.
    _ym2203_instance->write(
        YM2203_ADDRESS_FM_F_NUMBER_COARSE_AND_BLOCK(_channel), 0x00);
    _ym2203_instance->write(YM2203_ADDRESS_FM_F_NUMBER_FINE(_channel), 0x00);
    return;
  }
  if (_last_f_number_written == f_number && _last_f_block_written == block) {
    return;
  }

  unsigned f_number_upper_3_bits =
      (f_number >> 8) & YM2203_FM_F_NUMBER_COARSE_MASK;
  unsigned f_number_lower_8_bits = f_number & YM2203_FM_F_NUMBER_FINE_MASK;

  byte block_and_coarse = (YM2203_FM_F_BLOCK_MASK & block)
                              << YM2203_FM_F_BLOCK_OFFSET |
                          f_number_upper_3_bits;

  _ym2203_instance->write(YM2203_ADDRESS_FM_F_NUMBER_COARSE_AND_BLOCK(_channel),
                          block_and_coarse);
  _ym2203_instance->write(YM2203_ADDRESS_FM_F_NUMBER_FINE(_channel),
                          f_number_lower_8_bits);

  _last_f_number_written = f_number;
  _last_f_block_written = block;
}

/**
 * @param frequency_cents is the number of cents from MIDI note 0, in other
 * words it is 100 * midi_note + offset_cents.
 */
void Ym2203FmChannel::writePitch(unsigned frequency_cents) {
  _writeFNumberBlock(getBlock(frequency_cents),
                     getFNumber(frequency_cents, F_NUMBERS_4MHZ));
}

void Ym2203FmChannel::writeKeyOnOff(bool key_on, bool force) {
  if (!force && _last_key_on_written == key_on) {
    return;
  }
  // each bit of "slot" is the state of a single operator for the given channel
  unsigned slot = key_on ? 0b1111 : 0b0000;

  byte data = slot << YM2203_FM_SLOT_OFFSET | (_channel & YM2203_CHANNEL_MASK);

  _ym2203_instance->write(YM2203_ADDRESS_FM_SLOT_KEY_ON, data);

  _last_key_on_written = key_on;
}
