#ifndef MidiManager_h
#define MidiManager_h

#include "Delay.h"
#include "Multi.h"
#include "Synth.h"
#include "VoiceManager.h"

class MidiManager {
public:
  MidiManager(Synth *synth, MidiDelay *delay) : _synth(synth), _delay(delay) {}
  void handleNoteOn(byte channel, byte pitch, byte velocity);
  void handleNoteOff(byte channel, byte pitch, byte velocity);
  void handlePitchBend(byte channel, int bend);
  void handleControlChange(byte channel, byte cc_number, byte data);
  void handleProgramChange(byte channel, byte program);
  void handleSysex(byte *array, unsigned size);

private:
  Synth *_synth;
  MidiDelay *_delay;
};

#endif
