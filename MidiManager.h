#ifndef MidiManager_h
#define MidiManager_h

#include "Delay.h"
#include "Multi.h"
#include "VoiceManager.h"

class MidiManager {
public:
  MidiManager(VoiceManager *voice_manager, Multi *active_multi,
              MidiDelay *delay);
  void handleNoteOn(byte channel, byte pitch, byte velocity);
  void handleNoteOff(byte channel, byte pitch, byte velocity);
  void tick();

private:
  VoiceManager *_voice_manager;
  Multi *_active_multi;
  MidiDelay *_delay;
};

#endif
