#ifndef MidiManager_h
#define MidiManager_h

#include "Delay.h"
#include "Multi.h"
#include "Synth.h"
#include "VoiceManager.h"

typedef struct MidiNoteState MidiNoteState;

struct MidiNoteState {
  byte velocity = 0;
  byte aftertouch = 0;
};

typedef struct MidiChannelState MidiChannelState;

struct MidiChannelState {
  MidiNoteState notes[128];
  byte cc[128];
  signed long pitch_bend;
  byte channel_aftertouch;
};

typedef struct MidiState MidiState;

struct MidiState {
  MidiChannelState channels[16];
};

class MidiManager {
public:
  MidiManager(Synth *synth, MidiDelay *delay) : _synth(synth), _delay(delay) {}
  void handleNoteOn(byte channel, byte pitch, byte velocity);
  void handleNoteOff(byte channel, byte pitch, byte velocity);
  void handlePitchBend(byte channel, int bend);
  void handleControlChange(byte channel, byte cc_number, byte data);
  void handleProgramChange(byte channel, byte program);
  MidiState state;

private:
  Synth *_synth;
  MidiDelay *_delay;
};

#endif
