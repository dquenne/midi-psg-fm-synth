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
  VoiceStatus getStatus();
  void setSynthControlState(SynthControlState *synth_control_state);
  bool getIsDelay();
  bool getChangedChannel();
  byte channel;

  byte pitch;
  unsigned pitch_cents;
  unsigned long triggered_at;

protected:
  byte _initial_velocity;
  bool _on;
  bool _held;
  bool _is_delay;
  byte _previous_channel;
  SynthControlState *_synth_control_state;
};

class PsgVoice : public Voice {
public:
  PsgVoice();
  void setPatch(PsgPatch *patch, bool is_delay);
  const PsgPatch *getPatch();
  void noteOn(byte _channel, byte _pitch, byte velocity);
  void noteOff();
  void tick();

  // state management
  byte level;

protected:
  unsigned _getPitchCents();
  unsigned _getLevel();
  bool _isActive();

  const PsgPatch *_patch;
  PsgPatchState _patch_state;
};

class FmVoice : public Voice {
public:
  FmVoice();
  void setPatch(FmPatch *patch, bool is_delay);
  const FmPatch *getPatch();

  void noteOn(byte _channel, byte _pitch, byte velocity);
  void noteOff();
  void setSynced();
  bool getIsSynced();
  void tick();

  byte operator_levels[4];

private:
  unsigned _getPitchCents();
  unsigned _getOperatorLevel(unsigned op);
  unsigned _getModLevel(FmPatchOperatorScalingMode scaling_mode);

  const FmPatch *_patch;
  FmPatchState _patch_state;
  bool _trigger;
};

#endif
