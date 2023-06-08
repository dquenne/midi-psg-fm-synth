#include "Patch.h"
#include "NoteMappings.h"

#define LIMIT(val, min, max) MIN(MAX(val, min), max)
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

/** a - b, except never go lower than 0 (for unsigned values) */
#define FLOOR_MINUS(a, b) (a < b ? 0 : a - b)

// PSG

PsgPatchState::PsgPatchState() {
  _patch_set = false;
  _is_delay = false;
}

void PsgPatchState::initialize() {
  amplitude_envelope_state.initialize();
  frequency_envelope_state.initialize();
  amplitude_lfo_state.initialize();
  frequency_lfo_state.initialize();
}

void applyPresetEnvelope(EnvelopeShape *target, const EnvelopeShape *preset) {
  target->steps[0] = preset->steps[0];
  target->steps[1] = preset->steps[1];
  target->steps[2] = preset->steps[2];
  target->steps[3] = preset->steps[3];
  target->steps[4] = preset->steps[4];
  target->loop_enable = preset->loop_enable;
  target->loop_after_step = preset->loop_after_step;
  target->loop_to_step = preset->loop_to_step;
  target->on_off_jump_to_step = preset->on_off_jump_to_step;
  target->end_after_step = preset->end_after_step;
}

void applyPresetLfo(Lfo *target, const Lfo *preset) {
  target->depth = preset->depth;
  target->waveform = preset->waveform;
  target->wavelength = preset->wavelength;
  target->start_delay_ticks = preset->start_delay_ticks;
  target->off_ticks = preset->off_ticks;
}

void applyPsgPreset(PsgPatch *target, const PsgPatch *preset) {
  applyPresetEnvelope(&target->amplitude_envelope, &preset->amplitude_envelope);
  applyPresetEnvelope(&target->frequency_envelope, &preset->frequency_envelope);
  applyPresetLfo(&target->amplitude_lfo, &preset->amplitude_lfo);
  applyPresetLfo(&target->frequency_lfo, &preset->frequency_lfo);
  target->velocity_config.velocity_center =
      preset->velocity_config.velocity_center;
  target->velocity_config.interval = preset->velocity_config.interval;
  target->delay_config.enable = preset->delay_config.enable;
  target->delay_config.delay_ticks = preset->delay_config.delay_ticks;
  target->delay_config.detune_cents = preset->delay_config.detune_cents;
  target->delay_config.attenuation = preset->delay_config.attenuation;
}

void PsgPatchState::setPatch(const PsgPatch *patch, bool is_delay) {
  _patch = patch;
  initialize();
  amplitude_envelope_state.setEnvelopeShape(&_patch->amplitude_envelope);
  frequency_envelope_state.setEnvelopeShape(&_patch->frequency_envelope);
  amplitude_lfo_state.setLfo(&_patch->amplitude_lfo);
  frequency_lfo_state.setLfo(&_patch->frequency_lfo);
  _patch_set = true;
  _is_delay = is_delay;
}

void PsgPatchState::noteOn(byte pitch, byte velocity) {
  _pitch = pitch;
  _velocity = velocity;
  _held = true;
  amplitude_envelope_state.start();
  frequency_envelope_state.start();
  amplitude_lfo_state.start();
  frequency_lfo_state.start();
}

void PsgPatchState::noteOff() {
  _held = false;
  amplitude_envelope_state.noteOff();
  frequency_envelope_state.noteOff();
  amplitude_lfo_state.noteOff();
  frequency_lfo_state.noteOff();
}

void PsgPatchState::tick() {
  if (amplitude_envelope_state.getStatus() == done) {
    return;
  }
  amplitude_envelope_state.tick();
  frequency_envelope_state.tick();
  amplitude_lfo_state.tick();
  frequency_lfo_state.tick();
}

unsigned PsgPatchState::getFrequencyCents() {
  unsigned frequency_cents =
      (_pitch * 100) + frequency_lfo_state.getValue() + _patch->detune_cents;
  if (_is_delay) {
    return frequency_cents + _patch->delay_config.detune_cents;
  }
  return frequency_cents;
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

void applyPresetOperator(FmOperator *target, const FmOperator *preset) {
  target->attack_rate = preset->attack_rate;
  target->decay_rate = preset->decay_rate;
  target->sustain_rate = preset->sustain_rate;
  target->release_rate = preset->release_rate;
  target->sustain_level = preset->sustain_level;
  target->total_level = preset->total_level;
  target->key_scale = preset->key_scale;
  target->multiple = preset->multiple;
  target->detune = preset->detune;
  target->lfo_amplitude_enable = preset->lfo_amplitude_enable;
}

void applyPresetFMCoreParameters(FmParameters *target,
                                 const FmParameters *preset) {
  applyPresetOperator(&target->operators[0], &preset->operators[0]);
  applyPresetOperator(&target->operators[1], &preset->operators[1]);
  applyPresetOperator(&target->operators[2], &preset->operators[2]);
  applyPresetOperator(&target->operators[3], &preset->operators[3]);
  target->panning = preset->panning;
  target->feedback = preset->feedback;
  target->algorithm = preset->algorithm;
  target->lfo_amplitude_sensitivity = preset->lfo_amplitude_sensitivity;
  target->lfo_frequency_sensitivity = preset->lfo_frequency_sensitivity;
}

void applyFmPreset(FmPatch *target, const FmPatch *preset) {
  applyPresetFMCoreParameters(&target->core_parameters,
                              &preset->core_parameters);
  target->delay_config.enable = preset->delay_config.enable;
  target->delay_config.delay_ticks = preset->delay_config.delay_ticks;
  target->delay_config.detune_cents = preset->delay_config.detune_cents;
  target->delay_config.attenuation = preset->delay_config.attenuation;
}
