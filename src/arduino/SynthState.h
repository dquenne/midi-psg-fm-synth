#ifndef SynthState_h
#define SynthState_h

#include <Arduino.h>

typedef struct SynthNoteState SynthNoteState;

struct SynthNoteState {
  byte velocity = 0;
  byte aftertouch = 0;
};

typedef struct SynthChannelState SynthChannelState;

struct SynthChannelState {
  SynthNoteState notes[128];
  byte cc[128];
  signed long pitch_bend = 0;
  byte channel_aftertouch = 0;
};

typedef struct SynthControlState SynthControlState;

struct SynthControlState {
  SynthChannelState channels[32];
};

#endif
