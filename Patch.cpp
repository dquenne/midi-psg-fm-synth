#include "Patch.h"
#include "NoteMappings.h"

#define LIMIT(val, min, max) MIN(MAX(val, min), max)
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

PatchState::PatchState() {
  _patch_set = false;
  _is_delay = false;
}

void PatchState::initialize() {
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

void applyPreset(Patch *target, const Patch *preset) {
  applyPresetEnvelope(&target->amplitude_envelope, &preset->amplitude_envelope);
  applyPresetEnvelope(&target->frequency_envelope, &preset->frequency_envelope);
  applyPresetLfo(&target->amplitude_lfo, &preset->amplitude_lfo);
  applyPresetLfo(&target->frequency_lfo, &preset->frequency_lfo);
  target->velocity_scaling = preset->velocity_scaling;
  target->delay_config.enable = preset->delay_config.enable;
  target->delay_config.delay_ticks = preset->delay_config.delay_ticks;
  target->delay_config.detune_cents = preset->delay_config.detune_cents;
}

void PatchState::setPreset(const Patch *preset, bool is_delay) {
  applyPreset(&_patch, preset);
  initialize();
  amplitude_envelope_state.setEnvelopeShape(&_patch.amplitude_envelope);
  frequency_envelope_state.setEnvelopeShape(&_patch.frequency_envelope);
  amplitude_lfo_state.setLfo(&_patch.amplitude_lfo);
  frequency_lfo_state.setLfo(&_patch.frequency_lfo);
  _patch_set = true;
  _is_delay = is_delay;
}

void PatchState::noteOn(byte pitch, byte velocity) {
  _pitch = pitch;
  _velocity = velocity;
  _held = true;
  amplitude_envelope_state.start();
  frequency_envelope_state.start();
  amplitude_lfo_state.start();
  frequency_lfo_state.start();
}

void PatchState::noteOff() {
  _held = false;
  amplitude_envelope_state.noteOff();
  frequency_envelope_state.noteOff();
  amplitude_lfo_state.noteOff();
  frequency_lfo_state.noteOff();
}

void PatchState::tick() {
  if (amplitude_envelope_state.getStatus() == done) {
    return;
  }
  amplitude_envelope_state.tick();
  frequency_envelope_state.tick();
  amplitude_lfo_state.tick();
  frequency_lfo_state.tick();
}

unsigned PatchState::getFrequencyCents() {
  unsigned frequency_cents =
      (_pitch * 100) + frequency_lfo_state.getValue() + _patch.detune_cents;
  if (_is_delay) {
    return frequency_cents + _patch.delay_config.detune_cents;
  }
  return frequency_cents;
}

float velocity_center_point = 64.0f;

unsigned PatchState::getLevel() {
  if (!isActive()) {
    return 0;
  }
  signed envelope_amplitude = amplitude_envelope_state.getValue();
  float velocity_scale = (float(_velocity - velocity_center_point) *
                          _patch.velocity_scaling / velocity_center_point) +
                         1.0f;

  return LIMIT(int(float(envelope_amplitude) * velocity_scale), 0, 15);
}

bool PatchState::isActive() {
  return amplitude_envelope_state.getStatus() != done;
}
