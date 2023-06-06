#ifndef Synth_h
#define Synth_h

#include "Chip.h"
#include "Multi.h"
#include "PatchManager.h"
#include "Storage.h"
#include "VoiceManager.h"

class Synth {
public:
  Synth(Chip *chip, Multi *active_multi)
      : _psg_voice_manager{3}, _fm_voice_manager{3}, _chip(chip),
        _active_multi(active_multi) {}
  void tick();
  void noteOn(byte _channel, byte _pitch, byte velocity);
  void noteOff(byte _channel, byte _pitch, byte velocity);

  /* Would like to remove this. This is just to support querying MIDI delay
   * functionality. */
  PatchDelayConfig *getDelayConfig(unsigned channel) {
    MultiChannel *multi_channel = &_active_multi->channels[channel];
    if (multi_channel->mode == MULTI_CHANNEL_MODE_FM) {
      return &_fm_patch_manager.getPatch(multi_channel->patch_id)->delay_config;
    }
    return &_psg_patch_manager.getPatch(multi_channel->patch_id)->delay_config;
  }

  PatchManager<PsgPatch> *getPsgPatchManager() { return &_psg_patch_manager; }
  PatchManager<FmPatch> *getFmPatchManager() { return &_fm_patch_manager; }

  // Patch and multi storage needs to be pretty fully re-thought, this is just a
  // stop-gap.
  void saveMulti() {} // storeMulti(_active_multi); }

private:
  VoiceManager<PsgVoice> _psg_voice_manager;
  VoiceManager<FmVoice> _fm_voice_manager;
  PatchManager<PsgPatch> _psg_patch_manager;
  PatchManager<FmPatch> _fm_patch_manager;
  Chip *_chip;
  Multi *_active_multi;
};

#endif
