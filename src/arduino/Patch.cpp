#include "Patch.h"
#include "NoteMappings.h"

// TODO: extract to common utils file
/** a - b, except never go lower than 0 (for unsigned values) */
#define FLOOR_MINUS(a, b) (a < b ? 0 : a - b)

unsigned getDelayTicks(byte delay_time) { return 4 * delay_time; }

// PSG

PsgPatchState::PsgPatchState() {
  _patch_set = false;
  _is_delay = false;
}

void PsgPatchState::initialize() {
  amplitude_envelope_state.initialize();
  pitch_envelope_state.initialize();
  amplitude_lfo_state.initialize();
  pitch_lfo_state.initialize();
}

void applyPsgPreset(PsgPatch *target, const PsgPatch *preset) {
  memcpy(target, preset, sizeof(*preset));
}

void PsgPatchState::setPatch(const PsgPatch *patch, bool is_delay) {
  _patch = patch;
  initialize();
  amplitude_envelope_state.setEnvelopeShape(&_patch->amplitude_envelope);
  pitch_envelope_state.setEnvelopeShape(&_patch->pitch_envelope.envelope_shape);
  amplitude_lfo_state.setLfo(&_patch->amplitude_lfo);
  pitch_lfo_state.setLfo(&_patch->pitch_lfo);
  _patch_set = true;
  _is_delay = is_delay;
}

void PsgPatchState::noteOn(byte pitch, byte velocity) {
  _pitch = pitch;
  _velocity = velocity;
  _held = true;
  amplitude_envelope_state.start();
  pitch_envelope_state.start();
  amplitude_lfo_state.start();
  pitch_lfo_state.start();
}

void PsgPatchState::noteOff() {
  _held = false;
  amplitude_envelope_state.noteOff();
  pitch_envelope_state.noteOff();
  amplitude_lfo_state.noteOff();
  pitch_lfo_state.noteOff();
}

void PsgPatchState::tick() {
  if (amplitude_envelope_state.getStatus() == done) {
    return;
  }
  amplitude_envelope_state.tick();
  pitch_envelope_state.tick();
  amplitude_lfo_state.tick();
  pitch_lfo_state.tick();
}

unsigned PsgPatchState::getPitchCents() {
  signed pitch_cents =
      (_pitch * 100) + pitch_lfo_state.getValue() + _patch->detune_cents;

  pitch_cents =
      (signed)pitch_cents + _patch->pitch_envelope.scaling * 25 *
                                (signed)pitch_envelope_state.getValue() / 1024;

  if (_is_delay) {
    return pitch_cents + _patch->delay_config.detune_cents;
  }
  return MAX(0, pitch_cents);
}

unsigned PsgPatchState::getLevel() {
  if (!isActive()) {
    return 0;
  }
  signed envelope_amplitude = amplitude_envelope_state.getValue();

  // positive velocity scaling should never bring it louder than 0
  if (envelope_amplitude == 0) {
    return 0;
  }

  signed velocity_attenuation =
      (signed(_patch->velocity_config.velocity_center) - _velocity) /
      _patch->velocity_config.interval;

  unsigned scaled_level =
      LIMIT(envelope_amplitude - velocity_attenuation, 0, 15);
  if (_is_delay) {
    return FLOOR_MINUS(scaled_level, _patch->delay_config.attenuation);
  }
  return scaled_level;
}

bool PsgPatchState::isActive() {
  return amplitude_envelope_state.getStatus() != done;
}

// FM

unsigned FmPatchState::getOperatorLevel(unsigned op) {
  if (_patch->velocity_level_scaling.operator_scaling[op] == 0) {
    return _patch->core_parameters.operators[op].total_level;
  }
  signed velocity_magnitude =
      (signed(_patch->velocity_level_scaling.velocity_center) - _velocity);
  signed velocity_scaling = _patch->velocity_level_scaling.operator_scaling[op];

  signed attenuation = velocity_magnitude * velocity_scaling / 64;
  unsigned velocity_scaled_level = LIMIT(
      _patch->core_parameters.operators[op].total_level + attenuation, 0, 127);

  if (_is_delay &&
      FM_CARRIERS_BY_ALGORITHM[_patch->core_parameters.algorithm][op]) {
    return LIMIT(velocity_scaled_level + _patch->delay_config.attenuation * 8,
                 0, 127);
  }
  return velocity_scaled_level;
}

void applyFmPreset(FmPatch *target, const FmPatch *preset) {
  memcpy(target, preset, sizeof(*preset));
}
