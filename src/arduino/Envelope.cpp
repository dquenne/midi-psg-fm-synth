#include "Envelope.h"

// TODO: extract to common utils file
/** a - b, except never go lower than 0 (for unsigned values) */
#define FLOOR_MINUS(a, b) (a < b ? 0 : a - b)

EnvelopeState::EnvelopeState() { initialize(); }
void EnvelopeState::setEnvelopeShape(const EnvelopeShape *envelope_shape) {
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

void AdsrEnvelopeState::initialize() {
  _stage = ADSR_STAGE_PRE_START;
  _stage_ticks_passed = 0;
  _value = 0;
}

void AdsrEnvelopeState::start() {
  _value = 0;
  _setStage(ADSR_STAGE_ATTACK);
}

void AdsrEnvelopeState::_setStage(AdsrEnvelopeStage stage) {
  _stage = stage;
  _stage_start_value = _value;
  unsigned rate = _getStageRate();
}

void AdsrEnvelopeState::noteOff() { _setStage(ADSR_STAGE_RELEASE); }

void AdsrEnvelopeState::step() {
  switch (_stage) {
  case ADSR_STAGE_PRE_START:
    _setStage(ADSR_STAGE_ATTACK);
    break;
  case ADSR_STAGE_ATTACK:
    _setStage(ADSR_STAGE_DECAY);
    break;
  case ADSR_STAGE_DECAY:
    // The sustain stage is actually just the decay stage. This allows realtime
    // editing of the sustain level / decay rate to modify currently-playing
    // notes.
    break;
  case ADSR_STAGE_RELEASE:
    _setStage(ADSR_STAGE_DONE);
    break;
  default:
    _setStage(ADSR_STAGE_DONE);
    break;
  }
}

unsigned AdsrEnvelopeState::_getStageRate() {
  switch (_stage) {
  case ADSR_STAGE_PRE_START:
    return 0;
  case ADSR_STAGE_ATTACK:
    return _envelope_shape->attack;
  case ADSR_STAGE_DECAY:
    return _envelope_shape->decay;
  case ADSR_STAGE_RELEASE:
    return _envelope_shape->release;
  default:
    return 0;
  }
}

unsigned AdsrEnvelopeState::_getStageTicksPerStep() {
  return ADSR_STEP_TICKS[_getStageRate()];
}
unsigned AdsrEnvelopeState::_getStageIncrementPerStep() {
  return ADSR_INCREMENTS_PER_STEP[_getStageRate()];
}

unsigned AdsrEnvelopeState::_getStageTargetValue() {
  switch (_stage) {
  case ADSR_STAGE_ATTACK:
    return ADSR_ENVELOPE_MAX_VALUE;
  case ADSR_STAGE_DECAY:
    return _envelope_shape->sustain * 8;
  case ADSR_STAGE_RELEASE:
    return 0;
  default:
    return 0;
  }
}

void AdsrEnvelopeState::_stageStep() {
  unsigned stage_increment = _getStageIncrementPerStep();
  unsigned stage_target_value = _getStageTargetValue();

  if (_value < stage_target_value) {
    _value += stage_increment;
    if (_value >= stage_target_value) {
      _value = stage_target_value;
      step();
    }
  } else if (_value > stage_target_value) {
    _value = FLOOR_MINUS(_value, stage_increment);
    if (_value <= stage_target_value) {
      _value = stage_target_value;
      step();
    }
  }
}

/** tick envelope forward one frame. if the envelope stepped, returns true. */
bool AdsrEnvelopeState::tick() {
  if (_stage == ADSR_STAGE_PRE_START || _stage == ADSR_STAGE_DONE) {
    return false;
  }

  if (_stage_ticks_passed > _getStageTicksPerStep()) {
    _stage_ticks_passed = 0;
    _stageStep();
    return true;
  }
  _stage_ticks_passed++;

  return false;
}

unsigned AdsrEnvelopeState::getValue() { return _value; }

EnvelopeStatus AdsrEnvelopeState::getStatus() {
  if (_stage == ADSR_STAGE_PRE_START) {
    return not_started;
  }
  if (_stage == ADSR_STAGE_DONE) {
    return done;
  }
  return active;
}
