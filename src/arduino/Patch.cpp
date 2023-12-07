#include "Patch.h"
#include "NoteMappings.h"

// TODO: extract to common utils file
/** a - b, except never go lower than 0 (for unsigned values) */
#define FLOOR_MINUS(a, b) (a < b ? 0 : a - b)

unsigned getDelayTicks(byte delay_time) { return 4 * delay_time; }

// PSG

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
}

void PsgPatchState::noteOn(byte channel, byte pitch, byte velocity,
                           bool retrigger) {
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

// FM

void applyFmPreset(FmPatch *target, const FmPatch *preset) {
  memcpy(target, preset, sizeof(*preset));
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
}

void FmPatchState::noteOn(byte channel, byte pitch, byte velocity,
                          bool retrigger) {
  if (retrigger) {
    pitch_envelope_state.initialize();
    pitch_envelope_state.start();
    pitch_lfo_state.initialize();
    pitch_lfo_state.start();
  }
}

void FmPatchState::noteOff() {
  pitch_envelope_state.noteOff();
  pitch_lfo_state.noteOff();
}

void FmPatchState::tick() {
  pitch_envelope_state.tick();
  pitch_lfo_state.tick();
}
