#ifndef Synth_h
#define Synth_h

#include "Chip.h"
#include "InternalStorage.h"
#include "Multi.h"
#include "PatchManager.h"
#include "SynthState.h"
#include "VoiceManager.h"

#define SYNTH_CHANNEL_COUNT 16

// 2 most-significant bits of bank MSB define the synth mode
enum BankMsbSynthMode : byte {
  BANK_MSB_SYNTH_MODE_FM = 0b00,
  BANK_MSB_SYNTH_MODE_RHYTHM = 0b01,
  BANK_MSB_SYNTH_MODE_PSG = 0b10,
};

struct NoteSwapNote {
  byte pitch;
  byte velocity;
  NoteSwapNote *next_priority_note;
  NoteSwapNote(byte _pitch, byte _velocity)
      : pitch(_pitch), velocity(_velocity) {
    next_priority_note = nullptr;
  }
};

struct NoteSwap {
  NoteSwapNote *old_note;
  NoteSwapNote *new_note;
};

class NoteManager {
public:
  NoteSwap noteOn(byte pitch, byte velocity);
  NoteSwap noteOff(byte pitch, byte velocity);
  void noteStolen(byte pitch);
  void setPolyphonyConfig(const PatchPolyphonyConfig *_polyphony_config);

  NoteSwapNote *top_priority_note;

private:
  signed _compareNotePriority(NoteSwapNote *a, NoteSwapNote *b);
  const PatchPolyphonyConfig *polyphony_config;
};

class SynthChannel {
public:
  SynthChannel() {
    patch_id = {0, 0};
    mode = MULTI_CHANNEL_MODE_PSG;
    pitch_bend = 0;
  }

  void setProgram(byte program_number) {
    patch_id.program_number = program_number;
  }

  void setBank(int bank_number) {
    mode = (bank_number >> 6 == 0 ? MULTI_CHANNEL_MODE_FM
                                  : MULTI_CHANNEL_MODE_PSG);
  }

  PatchId patch_id;
  MultiChannelMode mode;

  int pitch_bend;
};

class Synth {
public:
  Synth(Chip *chip, Multi *active_multi)
      : _psg_voice_manager{3}, _fm_voice_manager{3}, _chip(chip),
        _active_multi(active_multi), _psg_patch_manager{INTERNAL_STORAGE_PSG},
        _fm_patch_manager{INTERNAL_STORAGE_FM} {}
  void initialize();
  void tick();
  void noteOn(byte _channel, byte _pitch, byte velocity);
  void noteOff(byte _channel, byte _pitch, byte velocity);
  void setPitchBend(byte _channel, int bend);
  void programChange(byte channel, byte program);
  void bankMsbChange(byte channel, byte bank);
  void bankLsbChange(byte channel, byte bank);
  void controlChange(byte channel, byte cc_number, byte data);

  /* Would like to remove this. This is just to support querying MIDI delay
   * functionality. */
  PatchDelayConfig *getDelayConfig(unsigned channel);

  PatchManager<PsgPatch> *getPsgPatchManager();
  PatchManager<FmPatch> *getFmPatchManager();
  void loadMulti(Multi *multi);
  SynthChannel *getChannel(byte _channel);

  byte getTotalPsgLevel();
  byte getTotalFmLevel();

  // Patch and multi storage needs to be pretty fully re-thought, this is just a
  // stop-gap.
  void saveMulti() {}

private:
  void _fmNoteOn(byte channel, byte pitch, byte velocity, FmPatch *active_patch,
                 NoteSwapNote *note_swap_old_note);
  void stealNote(byte channel, byte pitch);
  void syncPsgChannel(PsgChannel *channel, PsgVoice *voice);
  void syncFmChannel(FmChannel *channel, FmVoice *voice);

  VoiceManager<PsgVoice> _psg_voice_manager;
  VoiceManager<FmVoice> _fm_voice_manager;
  PatchManager<PsgPatch> _psg_patch_manager;
  PatchManager<FmPatch> _fm_patch_manager;
  RhythmPatchManager _rhythm_patch_manager;
  Chip *_chip;
  Multi *_active_multi;
  SynthControlState _control_state;
  SynthChannel _synth_channels[16];
  NoteManager _note_managers[32];
};

#endif
