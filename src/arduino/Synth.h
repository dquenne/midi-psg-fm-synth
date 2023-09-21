#ifndef Synth_h
#define Synth_h

#include "Chip.h"
#include "Multi.h"
#include "PatchManager.h"
#include "Storage.h"
#include "VoiceManager.h"

#define SYNTH_CHANNEL_COUNT 16

class SynthChannel {
public:
  SynthChannel() {
    patch_id = {0, 0};
    mode = MULTI_CHANNEL_MODE_PSG;
    pitch_bend = 0;
  }

  PatchId patch_id;
  MultiChannelMode mode;

  int pitch_bend;
};

class Synth {
public:
  Synth(Chip *chip, Multi *active_multi)
      : _psg_voice_manager{3}, _fm_voice_manager{3}, _chip(chip),
        _active_multi(active_multi) {}
  void tick();
  void noteOn(byte _channel, byte _pitch, byte velocity);
  void noteOff(byte _channel, byte _pitch, byte velocity);
  void setPitchBend(byte _channel, int bend) {
    _synth_channels[_channel].pitch_bend = bend;
  }

  /* Would like to remove this. This is just to support querying MIDI delay
   * functionality. */
  PatchDelayConfig *getDelayConfig(unsigned channel) {
    SynthChannel *synth_channel = &_synth_channels[channel];
    if (synth_channel->mode == MULTI_CHANNEL_MODE_FM) {
      return &_fm_patch_manager.getPatch(synth_channel->patch_id)->delay_config;
    }
    return &_psg_patch_manager.getPatch(synth_channel->patch_id)->delay_config;
  }

  PatchManager<PsgPatch> *getPsgPatchManager() { return &_psg_patch_manager; }
  PatchManager<FmPatch> *getFmPatchManager() { return &_fm_patch_manager; }
  void loadMulti(Multi *multi) {
    for (byte channel_index = 0; channel_index < SYNTH_CHANNEL_COUNT;
         channel_index++) {
      _synth_channels[channel_index].mode = multi->channels[channel_index].mode;
      _synth_channels[channel_index].patch_id =
          multi->channels[channel_index].patch_id;
    }
  }
  SynthChannel *getChannel(byte _channel) { return &_synth_channels[_channel]; }

  // Patch and multi storage needs to be pretty fully re-thought, this is just a
  // stop-gap.
  void saveMulti() {} // storeMulti(_active_multi); }

private:
  void syncPsgChannel(PsgChannel *channel, PsgVoice *voice);
  void syncFmChannel(FmChannel *channel, FmVoice *voice);

  VoiceManager<PsgVoice> _psg_voice_manager;
  VoiceManager<FmVoice> _fm_voice_manager;
  PatchManager<PsgPatch> _psg_patch_manager;
  PatchManager<FmPatch> _fm_patch_manager;
  Chip *_chip;
  Multi *_active_multi;
  SynthChannel _synth_channels[16];
};

#endif
