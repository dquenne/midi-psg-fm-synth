#include "Ym2203.h"
#include "IoUtils.h"
#include "NoteMappings.h"
#include <Arduino.h>

Ym2203Instance::Ym2203Instance(unsigned clock_division) {
  _clock_division = clock_division;
}

void Ym2203Instance::setup() {
  setWriteEnable(false);

  psg_channels[0].setChannel(0);
  psg_channels[1].setChannel(1);
  psg_channels[2].setChannel(2);

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
  psg_channels[0].initialize();
  psg_channels[1].initialize();
  psg_channels[2].initialize();
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

  _ym2203_instance->write(
      // 0x01,
      YM2203_PSG_ADDRESS_SETS_BY_CHANNEL[_channel]->frequency_coarse,
      n_top_4_bits);
  _ym2203_instance->write(
      // 0x00,
      YM2203_PSG_ADDRESS_SETS_BY_CHANNEL[_channel]->frequency_fine,
      n_lower_8_bits);
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

  _ym2203_instance->write(YM2203_PSG_ADDRESS_SETS_BY_CHANNEL[_channel]->level,
                          normalized_level & YM2203_PSG_LEVEL_MASK);
  _last_level_written = normalized_level;
}
