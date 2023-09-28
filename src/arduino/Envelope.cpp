#include "Envelope.h"

// TODO: extract to common utils file
/** a - b, except never go lower than 0 (for unsigned values) */
#define FLOOR_MINUS(a, b) (a < b ? 0 : a - b)

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
    return 8 * (int)_envelope_shape->sustain;
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
