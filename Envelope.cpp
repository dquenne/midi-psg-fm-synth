#include "Envelope.h"

EnvelopeState::EnvelopeState() { initialize(); }
void EnvelopeState::setEnvelopeShape(EnvelopeShape *envelope_shape) {
  _envelope_shape = envelope_shape;
  initialize();
}

void EnvelopeState::initialize() {
  _step = _envelope_shape->end_after_step + 1;
  _held = false;
  _step_ticks_passed = 0;
  _started = false;
}

void EnvelopeState::start() {
  _started = true;
  _held = true;
  _step = 0;
}

void EnvelopeState::noteOff() {
  if (_envelope_shape->on_off_jump_to_step) {
    _step = _envelope_shape->on_off_jump_to_step;
    _step_ticks_passed = 0;
  }
  _held = false;
}

void EnvelopeState::step() {
  // check if step forward or step back to loop point
  if (_step > _envelope_shape->end_after_step) {
    return;
  } else if (_held && _step == _envelope_shape->loop_after_step) {
    _step = _envelope_shape->loop_to_step;
  } else {
    _step++;
  }
}

/** tick envelope forward one frame. if the envelope stepped, returns true. */
bool EnvelopeState::tick() {
  if (_started && _step <= _envelope_shape->end_after_step) {
    if (_step_ticks_passed > _envelope_shape->steps[_step].hold_ticks) {
      step();
      _step_ticks_passed = 0;
      return true;
    }
    _step_ticks_passed++;
  }
  return false;
}

unsigned EnvelopeState::getValue() {
  if (_step > _envelope_shape->end_after_step) {
    return 0;
  }
  return _envelope_shape->steps[_step].value;
}

EnvelopeStatus EnvelopeState::getStatus() {
  if (!_started) {
    return not_started;
  }
  if (_step > _envelope_shape->end_after_step) {
    return done;
  }
  return active;
}
