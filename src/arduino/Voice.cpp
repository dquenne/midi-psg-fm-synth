#include "Voice.h"

// PSG

PsgVoice::PsgVoice() {
  frequency_cents = 0;
  level = 0;
  pitch = 0;
  channel = 0;
  detune_cents = 0;
  _on = false;
  _held = false;
  _patch_state.initialize();
}

void PsgVoice::setPatch(PsgPatch *patch, bool is_delay) {
  _patch_state.setPatch(patch, is_delay);
  _is_delay = is_delay;
}

void PsgVoice::noteOn(byte _channel, byte _pitch, byte velocity) {
  pitch = _pitch;
  channel = _channel;
  _patch_state.noteOn(_pitch, velocity);
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
  frequency_cents = _patch_state.getFrequencyCents() + detune_cents;

  if (_patch_state.amplitude_envelope_state.getStatus() == done) {
    _on = false;
  }
}

// FM

FmVoice::FmVoice() {
  channel = 0;
  frequency_cents = 0;
  pitch = 0;
  detune_cents = 0;
  _on = false;
  _held = false;
  _patch_state.initialize();
}
