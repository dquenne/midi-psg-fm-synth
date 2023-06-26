#include "Synth.h"
#include "NoteMappings.h"

void Synth::noteOn(byte channel, byte pitch, byte velocity) {

  MultiChannel *multi_channel = &_active_multi->channels[channel % 16];
  if (multi_channel->mode == MULTI_CHANNEL_MODE_FM) {
    FmPatch *active_patch = _fm_patch_manager.getPatch(
        _active_multi->channels[channel % 16].patch_id);
    FmVoice *voice = _fm_voice_manager.getVoice(channel, pitch);
    voice->setPatch(active_patch, channel >= 16);
    voice->noteOn(channel, pitch, velocity);

    voice->detune_cents = 0;

  } else {
    // FIXME: make this dependant on target chip.
    // if (NOTES_125KHZ[pitch] > 1023) {
    //   return;
    // }
    if (NOTES_250KHZ[pitch] > 4095) {
      return;
    }
    PsgPatch *active_patch = _psg_patch_manager.getPatch(
        _active_multi->channels[channel % 16].patch_id);
    PsgVoice *voice = _psg_voice_manager.getVoice(channel, pitch);
    voice->setPatch(active_patch, channel >= 16);
    voice->noteOn(channel, pitch, velocity);

    voice->detune_cents = 0;
  }
}

void Synth::noteOff(byte channel, byte pitch, byte velocity) {
  Voice *voice;
  if (_active_multi->channels[channel % 16].mode == MULTI_CHANNEL_MODE_FM) {
    voice = _fm_voice_manager.getExactVoice(channel, pitch);
  } else {
    voice = _psg_voice_manager.getExactVoice(channel, pitch);
  }
  voice->noteOff();
}

void syncPsgChannel(PsgChannel *channel, PsgVoice *voice) {
  channel->writeLevel(voice->level);
  channel->writePitch(voice->frequency_cents);
}

void syncFmChannel(FmChannel *channel, FmVoice *voice) {
  if (!voice->getIsSynced() && voice->getStatus() == voice_held) {
    channel->writeKeyOnOff(false);
    channel->writeStaticPatchParameters(voice->getPatch());
    voice->setSynced();
    channel->writeKeyOnOff(voice->getStatus() == voice_held);
  }
  for (unsigned op = 0; op < 4; op++) {
    channel->writeTotalLevel(op, voice->operator_levels[op]);
  }
  channel->writePitch(voice->frequency_cents);
  channel->writeKeyOnOff(voice->getStatus() == voice_held);
}

void Synth::tick() {
  _psg_voice_manager.tick();
  _fm_voice_manager.tick();

  syncPsgChannel(_chip->getPsgChannel(0),
                 _psg_voice_manager.getVoiceByIndex(0));
  syncPsgChannel(_chip->getPsgChannel(1),
                 _psg_voice_manager.getVoiceByIndex(1));
  syncPsgChannel(_chip->getPsgChannel(2),
                 _psg_voice_manager.getVoiceByIndex(2));

  syncFmChannel(_chip->getFmChannel(0), _fm_voice_manager.getVoiceByIndex(0));
  syncFmChannel(_chip->getFmChannel(1), _fm_voice_manager.getVoiceByIndex(1));
  syncFmChannel(_chip->getFmChannel(2), _fm_voice_manager.getVoiceByIndex(2));
}
