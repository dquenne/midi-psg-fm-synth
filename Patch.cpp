#include "Patch.h"
#include "NoteMappings.h"

PatchState::PatchState() { _patch_set = false; }

void PatchState::initialize() {
  amplitude_envelope_state.initialize();
  frequency_envelope_state.initialize();
  amplitude_lfo_state.initialize();
  frequency_lfo_state.initialize();
}

void PatchState::setPatch(const Patch *patch) {
  _patch = patch;
  initialize();
  amplitude_envelope_state.setEnvelopeShape(_patch->amplitude_envelope);
  frequency_envelope_state.setEnvelopeShape(_patch->frequency_envelope);
  amplitude_lfo_state.setLfo(_patch->amplitude_lfo);
  frequency_lfo_state.setLfo(_patch->frequency_lfo);
  _patch_set = true;
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
  return (_pitch * 100) + frequency_lfo_state.getValue();
}

unsigned PatchState::getLevel() {
  if (!isActive()) {
    return 0;
  }
  return amplitude_envelope_state.getValue();
}

bool PatchState::isActive() { return !!_patch; }

static const Patch PRESET_PATCH_0 = {ENVELOPE_SHAPES[1], ENVELOPE_SHAPES[0],
                                     LFO_PRESETS[1], LFO_PRESETS[1]};

const Patch *PRESET_PATCHES[] = {&PRESET_PATCH_0};
