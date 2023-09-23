#include "Sn76489.h"
#include "IoUtils.h"
#include "NoteMappings.h"
#include <Arduino.h>

#define NOP asm volatile("nop\n\t")
#define NOP4 asm volatile("nop\n\tnop\n\tnop\n\tnop\n\t")

#define MAX_FREQUENCY_N 1023 // max possible 10-bit unsigned int

byte buildPsgMessage(unsigned channel, bool is_attenuation, unsigned data) {
  return (
      (0b1 << 7) | // first bit indicates this is a latch byte
      ((channel & PSG_CHANNEL_MASK) << PSG_CHANNEL_OFFSET) |
      ((is_attenuation & PSG_REGISTER_TYPE_MASK) << PSG_REGISTER_TYPE_OFFSET) |
      (data & 0b1111));
}

Sn76489Instance::Sn76489Instance(unsigned clock_division) {
  _clock_division = clock_division;
}

void Sn76489Instance::setup() {
  setWriteEnable(false);

  tone_channels[0].setChannel(0);
  tone_channels[1].setChannel(1);
  tone_channels[2].setChannel(2);
  tone_channels[0].initialize();
  tone_channels[1].initialize();
  tone_channels[2].initialize();

  // placeholder to silence noise channel on startup until the channel is
  // fully implemented
  write(buildPsgMessage(3, PSG_REGISTER_TYPE_ATTENUATION, 15));
}

void Sn76489Instance::write(byte data) {
  setByteOut(data);
  setWriteEnable(true);
  clockDelay(40);
  setWriteEnable(false);
}
Sn76489ToneChannel::Sn76489ToneChannel() {
  _channel = PSG_CHANNEL_UNSPECIFIED;
  _last_frequency_written = 0;
  _last_attenuation_written = 0;
}
Sn76489ToneChannel::Sn76489ToneChannel(unsigned channel) { _channel = channel; }

void Sn76489ToneChannel::setChannel(unsigned channel) { _channel = channel; }

void Sn76489ToneChannel::initialize() { writeLevel(0, true); }

/**
 * @param n is a 10-bit number to be divided by CLOCK_FREQUENCY / 32 to get PSG
 * channel output.
 */
void Sn76489ToneChannel::writeFrequencyN(unsigned frequency_n) {
  if (frequency_n > MAX_FREQUENCY_N) {
    // not the most elegant thing, but if the frequency N is too high (i.e.
    // pitch is too low), just return early and do nothing
    return;
  }
  if (_last_frequency_written == frequency_n) {
    return;
  }
  unsigned n_top_6_bits = (frequency_n >> 4) & 0b111111;
  unsigned n_lower_4_bits = frequency_n & 0b1111;

  _sn76489_instance->write(
      buildPsgMessage(_channel, PSG_REGISTER_TYPE_FREQUENCY, n_lower_4_bits));
  _sn76489_instance->write(n_top_6_bits);
  _last_frequency_written = frequency_n;
}

/**
 * @param attenuation is a 4-bit number specifying the attenuation dB, in steps
 * of -2dB. i.e. -2*attenuation = out dB.
 */
void Sn76489ToneChannel::writeAttenuation(unsigned attenuation, bool force) {
  if (!force && _last_attenuation_written == attenuation) {
    return;
  }
  _sn76489_instance->write(
      buildPsgMessage(_channel, PSG_REGISTER_TYPE_ATTENUATION, attenuation));
  _last_attenuation_written = attenuation;
}

/**
 * @param pitch_cents is the number of cents from MIDI note 0, in other
 * words it is 100 * midi_note + offset_cents.
 */
void Sn76489ToneChannel::writePitch(unsigned pitch_cents) {
  writeFrequencyN(getFrequencyN(pitch_cents, NOTES_125KHZ));
}

/**
 * @param level: a value from 0 to 15, where 15 is loudest and 0 is silent
 */
void Sn76489ToneChannel::writeLevel(unsigned level, bool force) {
  if (level >= 15) {
    writeAttenuation(0, force);
    return;
  }
  if (level <= 0) {
    writeAttenuation(15, force);
    return;
  }
  writeAttenuation(15 - level, force);
}
