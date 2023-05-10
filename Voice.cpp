#include "Voice.h"

Voice::Voice() {
  frequency_cents = 0;
  level = 0;
  pitch = 0;
  channel = 0;
  detune_cents = 0;
  _on = false;
  _held = false;
  _patch_state.initialize();
}

void Voice::setPatch(const Patch *patch) { _patch_state.setPreset(patch); }

void Voice::noteOn(byte _channel, byte _pitch, byte velocity) {
  pitch = _pitch;
  channel = _channel;
  _patch_state.noteOn(_pitch, velocity);
  _on = true;
  _held = true;
}

void Voice::noteOff() {
  _patch_state.noteOff();
  _held = false;
}

void Voice::tick() {
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

VoiceStatus Voice::getStatus() {
  if (_on) {
    if (_held) {
      return voice_held;
    }
    return voice_decay;
  }

  return voice_off;
}
