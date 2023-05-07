#include "Sn76489.h"
#include "NoteMappings.h"
#include <Arduino.h>

#define NOP asm volatile("nop\n\t")
#define NOP4 asm volatile("nop\n\tnop\n\tnop\n\tnop\n\t")

#define MAX_FREQUENCY_N 1023 // max possible 10-bit unsigned int

byte buildPsgMessage(unsigned channel, bool is_attenuation, unsigned data) {
  return (
      (0b1 << 7) | // first bit indicates this is a register write
      ((channel & PSG_CHANNEL_MASK) << PSG_CHANNEL_OFFSET) |
      ((is_attenuation & PSG_REGISTER_TYPE_MASK) << PSG_REGISTER_TYPE_OFFSET) |
      (data & 0b1111));
}

Sn76489Instance::Sn76489Instance(unsigned clock_division) {
  _clock_division = clock_division;
}

void Sn76489Instance::setup() {
  _writeEnable(false);

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

void Sn76489Instance::clockDelay(unsigned count) {
  unsigned elapsed;
  for (elapsed = 0; elapsed < count; elapsed++) {
    NOP4;
    NOP4;
  }
}

uint32_t CUSTOM_PORT_OUT_MASK =
    ((0b11 << 22) | (0b11 << 18) | (0b11 << 15) | (0b11 << 4));

void Sn76489Instance::_writeEnable(bool enabled) {
  if (enabled) {
    digitalWrite(9, LOW);
  } else {
    digitalWrite(9, HIGH);
  }
}

void Sn76489Instance::_setByteOut(byte data) {
  int bit_pairs[] = {
      (0b11000000 & data) >> 6,
      (0b00110000 & data) >> 4,
      (0b00001100 & data) >> 2,
      0b00000011 & data,
  };

  PORT->Group[0].OUT.reg &= ~CUSTOM_PORT_OUT_MASK;
  PORT->Group[0].OUT.reg |=
      (CUSTOM_PORT_OUT_MASK & ((bit_pairs[0] << 22) | (bit_pairs[1] << 18) |
                               (bit_pairs[2] << 15) | (bit_pairs[3] << 4)));
}

void Sn76489Instance::write(byte data) {
  _setByteOut(data);
  _writeEnable(true);
  clockDelay(40);
  _writeEnable(false);
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

/** @returns The number to subtract from the note's N to get note + cents */
unsigned getFrequencyNForCents(unsigned note, unsigned cents) {
  if (note == 127) {
    return NOTES_4MHZ[127];
  }
  // the difference in units of N between note and its next highest note
  unsigned n_diff = NOTES_4MHZ[note] - NOTES_4MHZ[note + 1];

  return (n_diff * cents) / 100;
}

/**
 * @param frequency_cents is the number of cents from MIDI note 0, in other
 * words it is 100 * midi_note + offset_cents.
 */
void Sn76489ToneChannel::writePitch(unsigned frequency_cents) {
  unsigned note = frequency_cents / 100;
  unsigned cents = frequency_cents % 100;

  unsigned frequency_n = NOTES_4MHZ[note] - getFrequencyNForCents(note, cents);
  writeFrequencyN(frequency_n);
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
