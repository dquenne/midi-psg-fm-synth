#include "Synth.h"
#include "NoteMappings.h"

void Synth::noteOn(byte channel, byte pitch, byte velocity) {
  // FIXME: make this dependant on target chip.
  // if (NOTES_125KHZ[pitch] > 1023) {
  //   return;
  // }
  if (NOTES_250KHZ[pitch] > 4095) {
    return;
  }

  PsgVoice *voice = _psg_voice_manager.getVoice(channel, pitch);
  PsgPatch *active_patch = &_active_multi->channels[channel % 16];
  voice->setPatch(&_active_multi->channels[channel % 16], channel >= 16);
  voice->noteOn(channel, pitch, velocity);

  voice->detune_cents = 0;
}

void Synth::noteOff(byte channel, byte pitch, byte velocity) {
  Voice *voice = _psg_voice_manager.getExactVoice(channel, pitch);
  voice->noteOff();
}

void syncPsgChannel(PsgChannel *channel, PsgVoice *voice) {
  channel->writeLevel(voice->level);
  channel->writePitch(voice->frequency_cents);
}

void Synth::tick() {
  _psg_voice_manager.tick();

  syncPsgChannel(_chip->getPsgChannel(0),
                 _psg_voice_manager.getVoiceByIndex(0));
  syncPsgChannel(_chip->getPsgChannel(1),
                 _psg_voice_manager.getVoiceByIndex(1));
  syncPsgChannel(_chip->getPsgChannel(2),
                 _psg_voice_manager.getVoiceByIndex(2));
}
