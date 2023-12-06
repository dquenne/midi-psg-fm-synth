#include "Voice.h"
#include "Patch.h"
#include <Arduino.h>

// Common voice parameters

VoiceStatus Voice::getStatus() {
  if (_on) {
    if (_held) {
      return voice_held;
    }
    return voice_decay;
  }
  return voice_off;
}

bool Voice::getIsDelay() { return _is_delay; }

bool Voice::getChangedChannel() { return channel != _previous_channel; }

// PSG

PsgVoice::PsgVoice() {
  pitch_cents = 0;
  level = 0;
  pitch = 0;
  channel = 0;
  _on = false;
  _held = false;
  _patch_state.initialize();
}

void PsgVoice::setSynthControlState(SynthControlState *synth_control_state) {
  _patch_state.setSynthControlState(synth_control_state);
}

void PsgVoice::setPatch(PsgPatch *patch, bool is_delay) {
  _patch_state.setPatch(patch, is_delay);
  _is_delay = is_delay;
}

void PsgVoice::noteOn(byte _channel, byte _pitch, byte velocity) {
  pitch = _pitch;
  _previous_channel = channel;
  channel = _channel;
  triggered_at = millis();
  _patch_state.noteOn(
      _channel, _pitch, velocity,
      !_held || _previous_channel != channel ||
          _patch_state.getPatch()->polyphony_config.retrigger_mode !=
              RETRIGGER_MODE_OFF);
  _on = true;
  _held = true;
}

void PsgVoice::noteOff() {
  _patch_state.noteOff();
  _held = false;
}

void PsgVoice::tick() {
  if (!_on) {
    return;
  }
  _patch_state.tick();
  level = _patch_state.getLevel();
  pitch_cents = _patch_state.getPitchCents();

  if (_patch_state.amplitude_envelope_state.getStatus() == done) {
    _on = false;
  }
}

// FM

FmVoice::FmVoice() {
  _previous_channel = 0;
  channel = 0;
  pitch_cents = 0;
  pitch = 0;
  _on = false;
  _held = false;
  _patch_state.initialize();
}

void FmVoice::setSynthControlState(SynthControlState *synth_control_state) {
  _synth_control_state = synth_control_state;
  _patch_state.setSynthControlState(synth_control_state);
}

void FmVoice::setPatch(FmPatch *patch, bool is_delay) {
  _patch = patch;
  _patch_state.setPatch(patch, is_delay);
  for (unsigned op = 0; op < 4; op++) {
    operator_levels[op] = patch->core_parameters.operators[op].total_level;
  }
  _is_delay = is_delay;
}
const FmPatch *FmVoice::getPatch() { return _patch_state.getPatch(); }

void FmVoice::noteOn(byte _channel, byte _pitch, byte velocity) {
  pitch = _pitch;
  _initial_velocity = velocity;
  _previous_channel = channel;
  channel = _channel;
  triggered_at = millis();
  _trigger = !_held || _previous_channel != channel ||
             _patch_state.getPatch()->polyphony_config.retrigger_mode !=
                 RETRIGGER_MODE_OFF;
  _patch_state.noteOn(_channel, _pitch, velocity, _trigger);
  _on = true;
  _held = true;
}

void FmVoice::noteOff() {
  _patch_state.noteOff();
  _held = false;
  _on = false; // no way to know if a voice that's not held is still decaying
}

void FmVoice::setSynced() { _trigger = false; }

bool FmVoice::getIsSynced() { return _trigger == false; }

void FmVoice::tick() {
  _patch_state.tick();
  pitch_cents = _getPitchCents();
  for (unsigned op = 0; op < 4; op++) {
    operator_levels[op] = _getOperatorLevel(op);
  }
}

unsigned FmVoice::_getPitchCents() {
  signed _pitch_cents = (100 * pitch) + _patch_state.pitch_lfo_state.getValue();

  _pitch_cents = (signed)_pitch_cents +
                 _patch->pitch_envelope.scaling * 25 *
                     (signed)_patch_state.pitch_envelope_state.getValue() /
                     1024;

  return MAX(0, _pitch_cents);
}

unsigned FmVoice::_getOperatorLevel(unsigned op) {
  unsigned total_level = _patch->core_parameters.operators[op].total_level;
  unsigned scaled_level = total_level;

  if (_patch->operator_scaling_config[op].scaling_mode !=
      FM_PATCH_OPERATOR_SCALING_MODE_NO_SCALING) {
    unsigned scalar =
        _getModLevel(_patch->operator_scaling_config[op].scaling_mode);
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

unsigned FmVoice::_getModLevel(FmPatchOperatorScalingMode scaling_mode) {
  switch (scaling_mode) {
  case FM_PATCH_OPERATOR_SCALING_MODE_MOD_WHEEL:
    return _synth_control_state->channels[channel].cc[1];
  case FM_PATCH_OPERATOR_SCALING_MODE_VELOCITY:
    return 127 - _initial_velocity;
  default:
    return 127;
  }
}
