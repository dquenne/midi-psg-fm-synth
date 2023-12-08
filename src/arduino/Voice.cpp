#include "Voice.h"
#include "Patch.h"
#include <Arduino.h>

// Common voice parameters

// TODO: extract to common utils file
/** a - b, except never go lower than 0 (for unsigned values) */
#define FLOOR_MINUS(a, b) (a < b ? 0 : a - b)

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

void Voice::setSynthControlState(SynthControlState *synth_control_state) {
  _synth_control_state = synth_control_state;
}
// PSG

PsgVoice::PsgVoice() {
  pitch_cents = 0;
  level = 0;
  pitch = 0;
  channel = 0;
  _on = false;
  _held = false;
  initialize();
}

void PsgVoice::initialize() {
  _amplitude_envelope_state.initialize();
  _pitch_envelope_state.initialize();
  _amplitude_lfo_state.initialize();
  _pitch_lfo_state.initialize();
}

void PsgVoice::setPatch(PsgPatch *patch, bool is_delay) {
  _patch = patch;
  initialize();
  _amplitude_envelope_state.setEnvelopeShape(&_patch->amplitude_envelope);
  _pitch_envelope_state.setEnvelopeShape(
      &_patch->pitch_envelope.envelope_shape);
  _amplitude_lfo_state.setLfo(&_patch->amplitude_lfo);
  _pitch_lfo_state.setLfo(&_patch->pitch_lfo);
  _is_delay = is_delay;
}

const PsgPatch *PsgVoice::getPatch() { return _patch; }

void PsgVoice::noteOn(byte _channel, byte _pitch, byte velocity) {
  pitch = _pitch;
  _initial_velocity = velocity;
  _previous_channel = channel;
  channel = _channel;
  triggered_at = millis();

  if (!_held || _previous_channel != channel ||
      _patch->polyphony_config.retrigger_mode != RETRIGGER_MODE_OFF) {
    _amplitude_envelope_state.initialize();
    _pitch_envelope_state.initialize();
    _amplitude_lfo_state.initialize();
    _pitch_lfo_state.initialize();

    _amplitude_envelope_state.start();
    _pitch_envelope_state.start();
    _amplitude_lfo_state.start();
    _pitch_lfo_state.start();
  }
  _on = true;
  _held = true;
}

void PsgVoice::noteOff() {
  _amplitude_envelope_state.noteOff();
  _pitch_envelope_state.noteOff();
  _amplitude_lfo_state.noteOff();
  _pitch_lfo_state.noteOff();
  _held = false;
}

void PsgVoice::tick() {
  if (!_on) {
    return;
  }
  if (_amplitude_envelope_state.getStatus() != done) {
    _amplitude_envelope_state.tick();
    _pitch_envelope_state.tick();
    _amplitude_lfo_state.tick();
    _pitch_lfo_state.tick();
  }
  level = _getLevel();
  pitch_cents = _getPitchCents();

  if (_amplitude_envelope_state.getStatus() == done) {
    _on = false;
  }
}

unsigned PsgVoice::_getPitchCents() {
  signed _pitch_cents =
      (pitch * 100) + _pitch_lfo_state.getValue() + _patch->detune_cents;

  _pitch_cents = (signed)_pitch_cents +
                 _patch->pitch_envelope.scaling * 25 *
                     (signed)_pitch_envelope_state.getValue() / 1024;

  if (_is_delay) {
    return _pitch_cents + _patch->delay_config.detune_cents;
  }
  return MAX(0, _pitch_cents);
}

unsigned PsgVoice::_getLevel() {
  if (!_isActive()) {
    return 0;
  }
  unsigned envelope_amplitude = _amplitude_envelope_state.getValue();

  unsigned scaled_level = envelope_amplitude / 32 *
                          (480 + VELOCITY_SCALING[_initial_velocity]) / 1024;

  if (_is_delay) {
    return FLOOR_MINUS(scaled_level, _patch->delay_config.attenuation);
  }
  return MIN(scaled_level, 15);
}
bool PsgVoice::_isActive() {
  return _amplitude_envelope_state.getStatus() != done;
}

// FM

FmVoice::FmVoice() {
  _previous_channel = 0;
  channel = 0;
  pitch_cents = 0;
  pitch = 0;
  _on = false;
  _held = false;
  initialize();
}

void FmVoice::initialize() {
  _pitch_envelope_state.initialize();
  _pitch_lfo_state.initialize();
}

void FmVoice::setPatch(FmPatch *patch, bool is_delay) {
  _patch = patch;
  initialize();
  _pitch_envelope_state.setEnvelopeShape(
      &_patch->pitch_envelope.envelope_shape);
  _pitch_lfo_state.setLfo(&_patch->pitch_lfo);

  for (unsigned op = 0; op < 4; op++) {
    operator_levels[op] = patch->operators[op].total_level;
  }
  _is_delay = is_delay;
}

const FmPatch *FmVoice::getPatch() { return _patch; }

void FmVoice::noteOn(byte _channel, byte _pitch, byte velocity) {
  pitch = _pitch;
  _initial_velocity = velocity;
  _previous_channel = channel;
  channel = _channel;
  triggered_at = millis();
  _trigger = !_held || _previous_channel != channel ||
             _patch->polyphony_config.retrigger_mode != RETRIGGER_MODE_OFF;

  if (_trigger) {
    _pitch_envelope_state.initialize();
    _pitch_envelope_state.start();
    _pitch_lfo_state.initialize();
    _pitch_lfo_state.start();
  }

  _on = true;
  _held = true;
}

void FmVoice::noteOff() {
  _pitch_envelope_state.noteOff();
  _pitch_lfo_state.noteOff();

  _held = false;
  _on = false; // no way to know if a voice that's not held is still decaying
}

void FmVoice::setSynced() { _trigger = false; }

bool FmVoice::getIsSynced() { return _trigger == false; }

void FmVoice::tick() {
  _pitch_envelope_state.tick();
  _pitch_lfo_state.tick();

  _updateModMatrixAccumlators();

  pitch_cents = _getPitchCents();
  for (unsigned op = 0; op < 4; op++) {
    operator_levels[op] = _getOperatorLevel(op);
  }
}

unsigned FmVoice::_getPitchCents() {
  signed _pitch_cents = (100 * pitch) + _pitch_lfo_state.getValue();

  _pitch_cents = (signed)_pitch_cents +
                 _patch->pitch_envelope.scaling * 25 *
                     (signed)_pitch_envelope_state.getValue() / 1024;

  return MAX(0, _pitch_cents);
}

unsigned FmVoice::_getOperatorLevel(unsigned op) {
  unsigned total_level = _patch->operators[op].total_level;
  unsigned scaled_level = FLOOR_MINUS(
      (signed)total_level, _mod_matrix_accumlators[MOD_DEST_TL_OP0 + op]);

  if (FM_CARRIERS_BY_ALGORITHM[_patch->core_parameters.algorithm][op]) {
    scaled_level = FLOOR_MINUS((signed)scaled_level,
                               _mod_matrix_accumlators[MOD_DEST_CARRIER_LEVEL]);
  }

  if (_is_delay &&
      FM_CARRIERS_BY_ALGORITHM[_patch->core_parameters.algorithm][op]) {
    return LIMIT(scaled_level + _patch->delay_config.attenuation * 8, 0, 127);
  }
  return scaled_level;
}

/**
 * Right now this is just linear scaling with velocity, which doesn't sound
 * super natural.
 */
signed FmVoice::_getCarrierVelocityScaling() {
  return ((signed)_initial_velocity - 127) *
         (signed)_patch->velocity_sensitivity / 128;
}

/** @returns mod source level from 0 to 255*/
byte FmVoice::_getModSourceLevel(ModSource source) {
  // for CC 1-127, they're sequential so this is a shorthand for writing them
  // all out. Note that CC 0 is not available as a mod source because it is
  // reserved for bank select.
  if (source >= MOD_SRC_CC1_MOD_WHEEL) {
    return _synth_control_state->channels[channel]
               .cc[source - MOD_SRC_CC1_MOD_WHEEL + 1] *
           2;
  }

  switch (source) {
  case MOD_SRC_VELOCITY:
    return _initial_velocity * 2;
  default:
    return 0;
  }
}

void FmVoice::_updateModMatrixAccumlators() {
  for (byte destination_index = 0; destination_index < MOD_DESTINATION_COUNT;
       destination_index++) {
    _mod_matrix_accumlators[destination_index] = 0;
  }

  _mod_matrix_accumlators[MOD_DEST_CARRIER_LEVEL] +=
      _getCarrierVelocityScaling();

  const ModMatrixEntry *current_entry;
  for (byte mod_matrix_index = 0; mod_matrix_index < MOD_MATRIX_ENTRY_COUNT;
       mod_matrix_index++) {
    current_entry = &_patch->mod_matrix[mod_matrix_index];

    if (current_entry->destination == MOD_DEST_NONE ||
        current_entry->source == MOD_SRC_NONE || current_entry->amount == 0) {
      continue;
    }

    _mod_matrix_accumlators[current_entry->destination] +=
        ((signed)_getModSourceLevel(current_entry->source) -
         2 * current_entry->center) *
        current_entry->amount / 256;
  }
}
