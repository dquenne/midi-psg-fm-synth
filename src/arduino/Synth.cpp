#include "Synth.h"
#include "NoteMappings.h"
#include <Arduino.h>

NoteSwap NoteManager::noteOn(byte pitch, byte velocity) {
  NoteSwapNote *new_note_state = new NoteSwapNote(pitch, velocity);
  NoteSwapNote *current_note = top_priority_note;

  if (!top_priority_note) {
    top_priority_note = new_note_state;
  } else if (_compareNotePriority(top_priority_note, new_note_state) > 0) {
    new_note_state->next_priority_note = top_priority_note;
    top_priority_note = new_note_state;
  } else {
    while (current_note) {
      if (!current_note->next_priority_note) {
        current_note->next_priority_note = new_note_state;
        break;
      }
      if (_compareNotePriority(current_note->next_priority_note,
                               new_note_state) > 0) {

        new_note_state->next_priority_note = current_note->next_priority_note;
        current_note->next_priority_note = new_note_state;
        break;
      }
      current_note = current_note->next_priority_note;
    }
  }

  current_note = top_priority_note;
  unsigned active_note_count = 0;
  bool new_note_is_sounded = false;
  while (current_note) {
    active_note_count++;
    if (current_note->pitch == new_note_state->pitch) {
      new_note_is_sounded = true;
    }
    if (polyphony_config->max_polyphony != MAX_POLYPHONY_UNLIMITED &&
        active_note_count >= polyphony_config->max_polyphony) {
      if (!new_note_is_sounded) {

        return {nullptr, nullptr};
      }
      return {current_note->next_priority_note, new_note_state};
    }
    current_note = current_note->next_priority_note;
  }
  if (new_note_is_sounded) {
    return {nullptr, new_note_state};
  }
  return {nullptr, new_note_state};
}

NoteSwap NoteManager::noteOff(byte pitch, byte velocity) {
  if (top_priority_note->pitch == pitch) {
    NoteSwapNote *old_note = top_priority_note;
    top_priority_note = top_priority_note->next_priority_note;
    delete old_note;
  }

  NoteSwapNote *current_note = top_priority_note;

  while (current_note) {
    if (current_note->next_priority_note->pitch == pitch) {
      NoteSwapNote *old_note = current_note->next_priority_note;
      current_note->next_priority_note =
          current_note->next_priority_note->next_priority_note;
      delete old_note;
    }
    current_note = current_note->next_priority_note;
  }

  current_note = top_priority_note;
  unsigned active_note_count = 0;
  while (current_note) {
    active_note_count++;
    if (active_note_count >= polyphony_config->max_polyphony) {
      return {nullptr, current_note};
    }
    current_note = current_note->next_priority_note;
  }
  return {nullptr, nullptr};
}

void NoteManager::noteStolen(byte pitch) {}

void NoteManager::setPolyphonyConfig(
    const PatchPolyphonyConfig *_polyphony_config) {
  polyphony_config = _polyphony_config;
}

signed NoteManager::_compareNotePriority(NoteSwapNote *older_note,
                                         NoteSwapNote *newer_note) {
  switch (polyphony_config->note_priority_mode) {
  case (NOTE_PRIORITY_MODE_LATEST):
    return 1;
    break;
  case (NOTE_PRIORITY_MODE_HIGHEST):
    return newer_note->pitch > older_note->pitch ? 1 : -1;
    break;
  case (NOTE_PRIORITY_MODE_LOWEST):
    return newer_note->pitch < older_note->pitch ? 1 : -1;
    break;
  }
  return 0;
}
// Synth

void Synth::initialize() {
  _psg_voice_manager.getVoiceByIndex(0)->setSynthControlState(&_control_state);
  _psg_voice_manager.getVoiceByIndex(1)->setSynthControlState(&_control_state);
  _psg_voice_manager.getVoiceByIndex(2)->setSynthControlState(&_control_state);
  _fm_voice_manager.getVoiceByIndex(0)->setSynthControlState(&_control_state);
  _fm_voice_manager.getVoiceByIndex(1)->setSynthControlState(&_control_state);
  _fm_voice_manager.getVoiceByIndex(2)->setSynthControlState(&_control_state);
}

void Synth::stealNote(byte channel, byte pitch) { noteOff(channel, pitch, 0); }

void Synth::noteOn(byte channel, byte pitch, byte velocity) {
  if (channel < 16) {
    _control_state.channels[channel].notes[pitch].velocity = velocity;
  }

  SynthChannel *synth_channel = &_synth_channels[channel % 16];

  NoteSwap note_swap = _note_managers[channel].noteOn(pitch, velocity);

  if (!note_swap.new_note) {
    // if the newly pressed note is too low a priority to be sounded, do nothing
    return;
  }

  if (synth_channel->mode == MULTI_CHANNEL_MODE_FM) {
    FmPatch *active_patch =
        _fm_patch_manager.getPatch(_synth_channels[channel % 16].patch_id);

    FmVoice *voice = nullptr;
    if (note_swap.old_note) {
      voice =
          _fm_voice_manager.getExactVoice(channel, note_swap.old_note->pitch);
    }
    if (!voice) {
      voice = _fm_voice_manager.getVoice(channel, pitch);
    }
    if (!voice) {
      return;
    }

    // if voice was stolen from another channel, or this channel does not track
    // maximum polyphony, force the stolen note off so that it is not
    // retriggered when this note is released.
    if ((voice->channel != channel ||
         active_patch->polyphony_config.max_polyphony ==
             MAX_POLYPHONY_UNLIMITED) &&
        voice->getStatus() == voice_held && !voice->getIsDelay()) {
      stealNote(voice->channel, voice->pitch);
    }

    if (voice->channel != channel || voice->getPatch() != active_patch) {
      voice->setPatch(active_patch, channel >= 16);
    }
    voice->noteOn(channel, pitch, velocity);

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

    PsgVoice *voice = nullptr;

    if (note_swap.old_note) {
      voice =
          _psg_voice_manager.getExactVoice(channel, note_swap.old_note->pitch);
    }
    if (!voice) {
      voice = _psg_voice_manager.getVoice(channel, pitch);
    }
    if (!voice) {
      return;
    }

    // if voice was stolen from another channel, or this channel does not track
    // maximum polyphony, force the stolen note off so that it is not
    // retriggered when this note is released.
    if ((voice->channel != channel ||
         active_patch->polyphony_config.max_polyphony ==
             MAX_POLYPHONY_UNLIMITED) &&
        voice->getStatus() == voice_held && !voice->getIsDelay()) {
      stealNote(voice->channel, voice->pitch);
    }

    if (voice->channel != channel || voice->getPatch() != active_patch) {
      voice->setPatch(active_patch, channel >= 16);
    }
    voice->noteOn(channel, pitch, velocity);
  }
}

// hardcoded at 2 semitone pitch bend range for now, but could be made
// configurable as part of the patch
signed getPitchBendCents(int pitch_bend) { return pitch_bend * 2 * 100 / 8192; }

void Synth::noteOff(byte channel, byte pitch, byte velocity) {
  if (channel < 16) {
    _control_state.channels[channel].notes[pitch].velocity = 0;
  }

  SynthChannel *synth_channel = &_synth_channels[channel % 16];

  Voice *voice;
  if (_synth_channels[channel % 16].mode == MULTI_CHANNEL_MODE_FM) {
    voice = _fm_voice_manager.getExactVoice(channel, pitch);
  } else {
    voice = _psg_voice_manager.getExactVoice(channel, pitch);
  }

  NoteSwap note_swap = _note_managers[channel].noteOff(pitch, velocity);

  if (!voice) {
    // can't find matching voice -> voice has probably already been reassigned
    return;
  }
  if (note_swap.new_note) {
    Voice *new_note_existing_voice;
    if (_synth_channels[channel % 16].mode == MULTI_CHANNEL_MODE_FM) {
      new_note_existing_voice =
          _fm_voice_manager.getExactVoice(channel, note_swap.new_note->pitch);
    } else {
      new_note_existing_voice =
          _psg_voice_manager.getExactVoice(channel, note_swap.new_note->pitch);
    }
    if (!new_note_existing_voice) {
      voice->noteOn(channel, note_swap.new_note->pitch,
                    note_swap.new_note->velocity);
      return;
    }
  }

  voice->noteOff();
}

void Synth::setPitchBend(byte _channel, int bend) {
  _synth_channels[_channel].pitch_bend = bend;
}

void Synth::programChange(byte channel, byte program) {
  SynthChannel *synth_channel = &_synth_channels[channel];
  synth_channel->setProgram(program);

  PatchPolyphonyConfig *polyphony_config;
  if (synth_channel->mode == MULTI_CHANNEL_MODE_FM) {
    polyphony_config =
        &_fm_patch_manager.getPatch(synth_channel->patch_id)->polyphony_config;
  } else {
    polyphony_config =
        &_psg_patch_manager.getPatch(synth_channel->patch_id)->polyphony_config;
  }

  _note_managers[channel].setPolyphonyConfig(polyphony_config);
  _note_managers[channel + 16].setPolyphonyConfig(polyphony_config);
}

void Synth::bankChange(byte channel, byte bank_number) {
  SynthChannel *synth_channel = &_synth_channels[channel];

  synth_channel->mode =
      (bank_number >> 6 == 0 ? MULTI_CHANNEL_MODE_FM : MULTI_CHANNEL_MODE_PSG);

  programChange(channel, synth_channel->patch_id.program_number);
}

void Synth::controlChange(byte channel, byte cc_number, byte data) {
  _control_state.channels[channel].cc[cc_number] = data;
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

    bankChange(channel_index,
               multi->channels[channel_index].patch_id.bank_number);
    programChange(channel_index,
                  multi->channels[channel_index].patch_id.program_number);
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

/** Used for visualization */
byte Synth::getTotalPsgLevel() {
  return (_psg_voice_manager.getVoiceByIndex(0)->level +
          _psg_voice_manager.getVoiceByIndex(1)->level +
          _psg_voice_manager.getVoiceByIndex(2)->level) /
         2;
}

/** Used for visualization */
byte Synth::getTotalFmLevel() {
  byte total = 0;
  for (unsigned channel = 0; channel < 3; channel++) {
    FmVoice *voice = _fm_voice_manager.getVoiceByIndex(channel);
    if (voice->getStatus() == voice_held) {
      total += voice->operator_levels[3] / 8;
    }
  }
  return total;
}
