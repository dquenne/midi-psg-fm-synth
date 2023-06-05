#ifndef Synth_h
#define Synth_h

#include "Chip.h"
#include "Multi.h"
#include "Storage.h"
#include "VoiceManager.h"

class Synth {
public:
  Synth(Chip *chip, Multi *active_multi)
      : _psg_voice_manager{3}, _chip(chip), _active_multi(active_multi) {}
  void tick();
  void noteOn(byte _channel, byte _pitch, byte velocity);
  void noteOff(byte _channel, byte _pitch, byte velocity);
  Patch *getPatch(unsigned channel) {
    return &_active_multi->channels[channel];
  }

  // Patch and multi storage needs to be pretty fully re-thought, this is just a
  // stop-gap.
  void saveMulti() { storeMulti(_active_multi); }

private:
  VoiceManager _psg_voice_manager;
  Chip *_chip;
  Multi *_active_multi;
};

#endif
