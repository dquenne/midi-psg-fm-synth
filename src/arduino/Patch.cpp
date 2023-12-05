#include "Patch.h"
#include "NoteMappings.h"

// TODO: extract to common utils file
/** a - b, except never go lower than 0 (for unsigned values) */
#define FLOOR_MINUS(a, b) (a < b ? 0 : a - b)

unsigned getDelayTicks(byte delay_time) { return 4 * delay_time; }

void PatchState::setSynthControlState(SynthControlState *synth_control_state) {
  _synth_control_state = synth_control_state;
}

byte PatchState::getVelocity() { return _initial_velocity; }

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

const PsgPatch *PsgPatchState::getPatch() { return _patch; }

void PsgPatchState::noteOn(byte channel, byte pitch, byte velocity,
                           bool retrigger) {
  _channel = channel;
  _initial_velocity = velocity;
  _pitch = pitch;
  _held = true;
  if (retrigger) {
    amplitude_envelope_state.initialize();
    pitch_envelope_state.initialize();
    amplitude_lfo_state.initialize();
    pitch_lfo_state.initialize();

    amplitude_envelope_state.start();
    pitch_envelope_state.start();
    amplitude_lfo_state.start();
    pitch_lfo_state.start();
  }
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
  unsigned envelope_amplitude = amplitude_envelope_state.getValue();

  unsigned scaled_level =
      envelope_amplitude / 32 * (480 + VELOCITY_SCALING[getVelocity()]) / 1024;

  if (_is_delay) {
    return FLOOR_MINUS(scaled_level, _patch->delay_config.attenuation);
  }
  return MIN(scaled_level, 15);
}

bool PsgPatchState::isActive() {
  return amplitude_envelope_state.getStatus() != done;
}

// FM

void applyFmPreset(FmPatch *target, const FmPatch *preset) {
  memcpy(target, preset, sizeof(*preset));
}

FmPatchState::FmPatchState() {
  _is_patch_set = false;
  _is_delay = false;
}

void FmPatchState::initialize() {
  pitch_envelope_state.initialize();
  pitch_lfo_state.initialize();
}

void FmPatchState::setPatch(const FmPatch *patch, bool is_delay) {
  _patch = patch;
  initialize();
  pitch_envelope_state.setEnvelopeShape(&_patch->pitch_envelope.envelope_shape);
  pitch_lfo_state.setLfo(&_patch->pitch_lfo);
  _is_patch_set = true;
  _is_delay = is_delay;
}

const FmPatch *FmPatchState::getPatch() { return _patch; }

void FmPatchState::noteOn(byte channel, byte pitch, byte velocity,
                          bool retrigger) {
  _channel = channel;
  _initial_velocity = velocity;
  _pitch = pitch;
  _held = true;
  if (retrigger) {
    pitch_envelope_state.initialize();
    pitch_envelope_state.start();
    pitch_lfo_state.initialize();
    pitch_lfo_state.start();
  }
}

void FmPatchState::noteOff() {
  _held = false;
  pitch_envelope_state.noteOff();
  pitch_lfo_state.noteOff();
}

void FmPatchState::tick() {
  pitch_envelope_state.tick();
  pitch_lfo_state.tick();
}

unsigned FmPatchState::getPitchCents() {
  signed pitch_cents = (100 * _pitch) + pitch_lfo_state.getValue();

  pitch_cents =
      (signed)pitch_cents + _patch->pitch_envelope.scaling * 25 *
                                (signed)pitch_envelope_state.getValue() / 1024;

  return MAX(0, pitch_cents);
}

unsigned FmPatchState::getOperatorLevel(unsigned op) {
  unsigned total_level = _patch->core_parameters.operators[op].total_level;
  unsigned scaled_level = total_level;

  if (_patch->operator_scaling_config[op].scaling_mode !=
      FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING) {
    unsigned scalar =
        getModLevel(_patch->operator_scaling_config[op].scaling_mode);
    unsigned alternative_level =
        _patch->operator_scaling_config[op].alternative_value;

    scaled_level =
        scaled_level + ((signed)alternative_level - (signed)scaled_level) *
                           (signed)(scalar) / 127;
  }

  if (_is_delay &&
      FM_CARRIERS_BY_ALGORITHM[_patch->core_parameters.algorithm][op]) {
    return LIMIT(scaled_level + _patch->delay_config.attenuation * 8, 0, 127);
  }
  return scaled_level;
}

unsigned FmPatchState::getModLevel(FmPatchOperatorScalingMode scaling_mode) {
  switch (scaling_mode) {
  case FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL:
    return _synth_control_state->channels[_channel].cc[1];
  case FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY:
    return 127 - _initial_velocity;
  default:
    return 127;
  }
}

bool FmPatchState::isActive() { return _held; }
