#ifndef Voice_h
#define Voice_h

#include "Patch.h"
#include <Arduino.h>

enum VoiceStatus { voice_off, voice_held, voice_decay };

class Voice {
public:
  virtual void noteOn(byte _channel, byte _pitch, byte velocity) = 0;
  virtual void noteOff() = 0;
  virtual void tick() = 0;
  VoiceStatus getStatus() {
    if (_on) {
      if (_held) {
        return voice_held;
      }
      return voice_decay;
    }
    return voice_off;
  }
  bool getIsDelay() { return _is_delay; }
  byte channel;

  byte pitch;
  unsigned frequency_cents;
  byte detune_cents;

protected:
  bool _on;
  bool _held;
  bool _is_delay;
};

class PsgVoice : public Voice {
public:
  PsgVoice();
  void setPatch(PsgPatch *patch, bool is_delay);
  void noteOn(byte _channel, byte _pitch, byte velocity);
  void noteOff();
  void tick();

  // state management
  byte level;

protected:
  PsgPatchState _patch_state;
};

class FmVoice : public Voice {
public:
  FmVoice();
  void setPatch(FmPatch *patch, bool is_delay) {
    _patch_state.setPatch(patch, is_delay);
    for (unsigned op = 0; op < 4; op++) {
      operator_levels[op] = patch->core_parameters.operators[op].total_level;
    }
    _is_delay = is_delay;
  }
  const FmPatch *getPatch() { return _patch_state.getPatch(); }

  void noteOn(byte _channel, byte _pitch, byte velocity) {
    pitch = _pitch;
    channel = _channel;
    _patch_state.noteOn(_pitch, velocity);
    _trigger = true;
    _on = true;
    _held = true;
  }
  void noteOff() {
    _patch_state.noteOff();
    _held = false;
    _on = false; // no way to know if a voice that's not held is still decaying
  }
  void setSynced() { _trigger = false; }
  bool getIsSynced() { return _trigger == false; }
  void tick() {
    _patch_state.tick();
    frequency_cents = _patch_state.getFrequencyCents() + detune_cents;
    for (unsigned op = 0; op < 4; op++) {
      operator_levels[op] = _patch_state.getOperatorLevel(op);
    }
  }
  byte operator_levels[4];

private:
  FmPatchState _patch_state;
  bool _trigger;
};

#endif
