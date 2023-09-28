#include "Synth.h"
#include "NoteMappings.h"

void Synth::noteOn(byte channel, byte pitch, byte velocity) {

  SynthChannel *synth_channel = &_synth_channels[channel % 16];
  if (synth_channel->mode == MULTI_CHANNEL_MODE_FM) {
    FmPatch *active_patch =
        _fm_patch_manager.getPatch(_synth_channels[channel % 16].patch_id);
    FmVoice *voice = _fm_voice_manager.getVoice(
        channel, pitch, &active_patch->polyphony_config);
    if (voice->channel != channel || voice->getPatch() != active_patch) {
      voice->setPatch(active_patch, channel >= 16);
    }
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
    PsgPatch *active_patch =
        _psg_patch_manager.getPatch(_synth_channels[channel % 16].patch_id);
    PsgVoice *voice = _psg_voice_manager.getVoice(
        channel, pitch, &active_patch->polyphony_config);

    if (voice->channel != channel || voice->getPatch() != active_patch) {
      voice->setPatch(active_patch, channel >= 16);
    }
    voice->noteOn(channel, pitch, velocity);

    voice->detune_cents = 0;
  }
}

// hardcoded at 2 semitone pitch bend range for now, but could be made
// configurable as part of the patch
signed getPitchBendCents(int pitch_bend) { return pitch_bend * 2 * 100 / 8192; }

void Synth::noteOff(byte channel, byte pitch, byte velocity) {
  Voice *voice;
  if (_synth_channels[channel % 16].mode == MULTI_CHANNEL_MODE_FM) {
    voice = _fm_voice_manager.getExactVoice(channel, pitch);
  } else {
    voice = _psg_voice_manager.getExactVoice(channel, pitch);
  }
  voice->noteOff();
}

void Synth::setPitchBend(byte _channel, int bend) {
  _synth_channels[_channel].pitch_bend = bend;
}

PatchDelayConfig *Synth::getDelayConfig(unsigned channel) {
  SynthChannel *synth_channel = &_synth_channels[channel];
  if (synth_channel->mode == MULTI_CHANNEL_MODE_FM) {
    return &_fm_patch_manager.getPatch(synth_channel->patch_id)->delay_config;
  }
  return &_psg_patch_manager.getPatch(synth_channel->patch_id)->delay_config;
}

PatchManager<PsgPatch> *Synth::getPsgPatchManager() {
  return &_psg_patch_manager;
}

PatchManager<FmPatch> *Synth::getFmPatchManager() { return &_fm_patch_manager; }

void Synth::loadMulti(Multi *multi) {
  for (byte channel_index = 0; channel_index < SYNTH_CHANNEL_COUNT;
       channel_index++) {
    _synth_channels[channel_index].mode = multi->channels[channel_index].mode;
    _synth_channels[channel_index].patch_id =
        multi->channels[channel_index].patch_id;
  }
}

SynthChannel *Synth::getChannel(byte _channel) {
  return &_synth_channels[_channel];
}

void Synth::syncPsgChannel(PsgChannel *channel, PsgVoice *voice) {
  channel->writeLevel(voice->level);
  channel->writePitch(
      voice->pitch_cents +
      getPitchBendCents(_synth_channels[voice->channel % 16].pitch_bend));
}

void Synth::syncFmChannel(FmChannel *channel, FmVoice *voice) {
  if (!voice->getIsSynced() && voice->getStatus() == voice_held) {
    RetriggerMode retrigger_mode =
        voice->getPatch()->polyphony_config.retrigger_mode;
    if (retrigger_mode == RETRIGGER_MODE_SOFT) {
      channel->writeKeyOnOff(false);
    } else if (retrigger_mode == RETRIGGER_MODE_HARD ||
               voice->getChangedChannel()) {
      channel->writeReleaseZero();
      channel->writeKeyOnOff(false);

      // Might need a better approach here. This will fully stop the synth for
      // 2ms, which will mess up timing for any software LFOs, envelopes, etc.
      // 2ms was the smallest amount that seemed sufficient to let the YM2203
      // envelopes completely discharge at the fastest release rate.
      delay(2);
    } else if (retrigger_mode == RETRIGGER_MODE_OFF) {
      // Do not set the previous note off. This creates a hammer-on / glissando
      // effect.
    }
    channel->writeStaticPatchParameters(voice->getPatch());
    voice->setSynced();
  }
  for (unsigned op = 0; op < 4; op++) {
    channel->writeTotalLevel(op, voice->operator_levels[op]);
  }
  channel->writePitch(
      voice->pitch_cents +
      getPitchBendCents(_synth_channels[voice->channel % 16].pitch_bend));
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
