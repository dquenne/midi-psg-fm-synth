#include "Lfo.h"

unsigned getLfoWavelength(byte speed) { return LFO_WAVELENGTHS[speed]; }
unsigned getLfoDelayTicks(byte delay_time) { return 16 * delay_time; }

LfoState::LfoState() { initialize(); }
void LfoState::setLfo(const Lfo *lfo) {
  _lfo = lfo;
  initialize();
}

void LfoState::initialize() {
  _held = false;
  _ticks_passed = 0;
  _wavelength = getLfoWavelength(_lfo->speed);
  _start_delay_ticks = getLfoDelayTicks(_lfo->start_delay_time);
  _wavelength_ticks_pased = 0;
  _active = false;
}

void LfoState::start() {
  _active = true;
  _held = true;
  _ticks_passed = 0;
}

void LfoState::noteOff() {
  _held = false;
  digitalWrite(13, LOW);
}

/** Calling done() just signals to the LfoState that it can ignore any further
 * tick() calls until start() is called again. */
void LfoState::done() { _active = false; }

/** tick LFO forward one frame */
bool LfoState::tick() {
  _wavelength = getLfoWavelength(_lfo->speed);
  _start_delay_ticks = getLfoDelayTicks(_lfo->start_delay_time);
  _ticks_passed++;
  _wavelength_ticks_pased++;
  if (_wavelength_ticks_pased >= _wavelength) {
    _wavelength_ticks_pased = 0;
  }

  return false;
}

/**
 * @returns a value between -1024 and 1024
 */
signed getSquareLfoMangitude(unsigned wavelength,
                             unsigned wavelength_progress) {
  if (wavelength_progress < wavelength / 2) {
    return 1024;
  } else {
    return -1024;
  }
}

/**
 * @returns a value between 0 and 1024
 */
signed getPositiveSquareLfoMangitude(unsigned wavelength,
                                     unsigned wavelength_progress) {

  if (wavelength_progress < wavelength / 2) {
    return 1024;
  } else {
    return 0;
  }
}

/**
 * @returns a value between -1024 and 1024
 */
signed getTriangleLfoMagnitude(unsigned wavelength,
                               unsigned wavelength_progress) {

  unsigned stage = wavelength_progress * 4 / wavelength;

  switch (stage) {
  case 0:
    return (wavelength_progress)*1024 / (wavelength / 4);
  case 1:
    return ((wavelength / 2) - wavelength_progress) * 1024 / (wavelength / 4);
  case 2:
    return signed((wavelength * 3 / 4 - wavelength_progress) * 1024 /
                  (wavelength / 4)) -
           1024;
  case 3:
    return signed((wavelength_progress - (wavelength * 3 / 4)) * 1024 /
                  (wavelength / 4)) -
           1024;
  }
  return 0;
}

signed getLfoMagnitude(LfoWaveform waveform, unsigned wavelength,
                       unsigned wavelength_ticks_passed) {
  switch (waveform) {
  case triangle:
    return getTriangleLfoMagnitude(wavelength, wavelength_ticks_passed);
  case square:
    return getSquareLfoMangitude(wavelength, wavelength_ticks_passed);
  case positive_square:
    return getPositiveSquareLfoMangitude(wavelength, wavelength_ticks_passed);
  }
  return 0;
}

/**
 * @returns Returns the amount of cents to be added to pitch
 */
signed LfoState::getValue() {
  if (!_active || _lfo->depth == 0) {
    return 0;
  }

  // number from 0-1024, where 1024 is 100%
  signed start_delay_coefficient = 1024;
  if (_ticks_passed < _start_delay_ticks) {
    start_delay_coefficient = _ticks_passed * 1024 / _start_delay_ticks;
  }

  signed magnitude =
      getLfoMagnitude(_lfo->waveform, _wavelength, _wavelength_ticks_pased);

  signed value = magnitude * _lfo->depth / 1024;

  // number from 0-1024, where 1024 is 100%
  if (_ticks_passed < _start_delay_ticks) {
    signed start_delay_coefficient = _ticks_passed * 1024 / _start_delay_ticks;
    return value * start_delay_coefficient / 1024;
  }
  return value;
}
