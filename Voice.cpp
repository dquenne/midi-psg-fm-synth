#include "Voice.h"

Voice::Voice() {
  frequency_cents = 0;
  level = 0;
  pitch = 0;
  _on = false;
  _patch_state.initialize();
}

void Voice::setPatch(const Patch *patch) { _patch_state.setPatch(patch); }

void Voice::noteOn(byte _pitch, byte velocity) {
  pitch = _pitch;
  _patch_state.noteOn(_pitch, velocity);
  _on = true;
}

void Voice::noteOff() { _patch_state.noteOff(); }

void Voice::tick() {
  if (!_on) {
    return;
  }
  _patch_state.tick();
  level = _patch_state.getLevel();
  frequency_cents = _patch_state.getFrequencyCents();

  if (_patch_state.amplitude_envelope_state.getStatus() == done) {
    _on = false;
  }
}
