#ifndef PatchManager_h
#define PatchManager_h

#include "InternalStorage.h"
#include "Patch.h"

#define PATCH_BUFFER_SIZE 16

template <typename PatchType> class PatchManager {
public:
  PatchManager(InternalStorageBankType patch_type)
      : _patch_storage{patch_type} {}
  void loadPatch(PatchId *patch_id, byte channel_number) {
    _patch_storage.readPatch(patch_id, &patch_buffer[channel_number]);
  }

  PatchType *getChannelPatch(byte channel_number) {
    return &patch_buffer[channel_number];
  }

  void writeChannelPatch(PatchId *patch_id, byte channel_number) {
    _patch_storage.writePatch(patch_id, _patch_storage[channel_number]);
  }

  void initializePatchBanks(const PatchType *default_patch) {
    _patch_storage.initializeBank(0, 0, default_patch);
  };

  void writePatch(PatchId *patch_id, PatchType *patch_data) {
    _patch_storage.writePatch(patch_id, patch_data);
  };

protected:
  PatchStorage<PatchType> _patch_storage;
  PatchType patch_buffer[PATCH_BUFFER_SIZE];
  PatchType null_patch;
};

const RhythmPatch RHYTHM_PRESET = {
    {1, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_HARD},
    {
        {{1, 0, 127}, 5},   // electric kick
        {{64, 0, 127}, 38}, // side stick
        {{17, 0, 127}, 59}, // acoustic snare
        {{16, 0, 127}, 57}, // electric snare
        {{32, 0, 127}, 78}, // closed hat
        {{33, 0, 127}, 83}, // open hat
        {{1, 0, 127}, 30},  // tom
        {{65, 0, 127}, 66}, // tambourine
    },
};

const PatchPolyphonyConfig RHYTHM_POLYPHONY_CONFIG = {
    1, NOTE_PRIORITY_MODE_LATEST, RETRIGGER_MODE_SOFT};

class RhythmPatchManager {
public:
  RhythmPatchManager() : _fm_patch_storage{INTERNAL_STORAGE_FM} {}
  void loadPatch(PatchId *rhythm_patch_id) {
    memcpy(&_rhythm_patch_buffer, &RHYTHM_PRESET, sizeof RHYTHM_PRESET);
    PatchId *note_patch_id;
    for (byte note_index = 0; note_index < RHYTHM_PATCH_NOTE_COUNT;
         note_index++) {
      note_patch_id = &_rhythm_patch_buffer.notes[note_index].patch_id;
      _fm_patch_storage.readPatch(note_patch_id, &fm_patch_buffer[note_index]);
    }
  };

  FmPatch *getNotePatch(byte note) {
    switch (note) {
    case RHYTHM_NOTE_VOICE_ACOUSTIC_KICK:
    case RHYTHM_NOTE_VOICE_ELECTRIC_KICK:
      return &fm_patch_buffer[RHYTHM_PATCH_VOICE_INDEX_KICK];
    case RHYTHM_NOTE_VOICE_SIDE_STICK:
      return &fm_patch_buffer[RHYTHM_PATCH_VOICE_INDEX_SIDE_STICK];
    case RHYTHM_NOTE_VOICE_ACOUSTIC_SNARE:
      return &fm_patch_buffer[RHYTHM_PATCH_VOICE_INDEX_SNARE_1];
    case RHYTHM_NOTE_VOICE_ELECTRIC_SNARE:
      return &fm_patch_buffer[RHYTHM_PATCH_VOICE_INDEX_SNARE_2];
    case RHYTHM_NOTE_VOICE_LOW_FLOOR_TOM:
    case RHYTHM_NOTE_VOICE_HIGH_FLOOR_TOM:
    case RHYTHM_NOTE_VOICE_LOW_TOM:
    case RHYTHM_NOTE_VOICE_LOW_MID_TOM:
    case RHYTHM_NOTE_VOICE_HIGH_MID_TOM:
    case RHYTHM_NOTE_VOICE_HIGH_TOM:
      return &fm_patch_buffer[RHYTHM_PATCH_VOICE_INDEX_TOM];
    case RHYTHM_NOTE_VOICE_CLOSED_HAT:
    case RHYTHM_NOTE_VOICE_PEDAL_HAT:
      return &fm_patch_buffer[RHYTHM_PATCH_VOICE_INDEX_CLOSED_HAT];
    case RHYTHM_NOTE_VOICE_OPEN_HAT:
      return &fm_patch_buffer[RHYTHM_PATCH_VOICE_INDEX_OPEN_HAT];
    case RHYTHM_NOTE_VOICE_TAMBOURINE:
      return &fm_patch_buffer[RHYTHM_PATCH_VOICE_INDEX_TAMBOURINE];

    default:
      return nullptr;
    }
  };

  byte getNotePitch(byte note) {
    switch (note) {
    case RHYTHM_NOTE_VOICE_ACOUSTIC_KICK:
    case RHYTHM_NOTE_VOICE_ELECTRIC_KICK:
      return _rhythm_patch_buffer.notes[RHYTHM_PATCH_VOICE_INDEX_KICK].pitch;
    case RHYTHM_NOTE_VOICE_SIDE_STICK:
      return _rhythm_patch_buffer.notes[RHYTHM_PATCH_VOICE_INDEX_SIDE_STICK]
          .pitch;
    case RHYTHM_NOTE_VOICE_ACOUSTIC_SNARE:
      return _rhythm_patch_buffer.notes[RHYTHM_PATCH_VOICE_INDEX_SNARE_1].pitch;
    case RHYTHM_NOTE_VOICE_ELECTRIC_SNARE:
      return _rhythm_patch_buffer.notes[RHYTHM_PATCH_VOICE_INDEX_SNARE_2].pitch;
    case RHYTHM_NOTE_VOICE_LOW_FLOOR_TOM:
    case RHYTHM_NOTE_VOICE_HIGH_FLOOR_TOM:
    case RHYTHM_NOTE_VOICE_LOW_TOM:
    case RHYTHM_NOTE_VOICE_LOW_MID_TOM:
    case RHYTHM_NOTE_VOICE_HIGH_MID_TOM:
      return _rhythm_patch_buffer.notes[RHYTHM_PATCH_VOICE_INDEX_TOM].pitch +
             4 * ((signed)note - 47);
    case RHYTHM_NOTE_VOICE_CLOSED_HAT:
    case RHYTHM_NOTE_VOICE_PEDAL_HAT:
      return _rhythm_patch_buffer.notes[RHYTHM_PATCH_VOICE_INDEX_CLOSED_HAT]
          .pitch;
    case RHYTHM_NOTE_VOICE_OPEN_HAT:
      return _rhythm_patch_buffer.notes[RHYTHM_PATCH_VOICE_INDEX_OPEN_HAT]
          .pitch;
    case RHYTHM_NOTE_VOICE_TAMBOURINE:
      return _rhythm_patch_buffer.notes[RHYTHM_PATCH_VOICE_INDEX_TAMBOURINE]
          .pitch;
    default:
      return 0;
    }
  };

protected:
  RhythmPatch _rhythm_patch_buffer;
  PatchStorage<FmPatch> _fm_patch_storage;
  FmPatch fm_patch_buffer[8]; // buffering more than this seems to exceed the
                              // limits of the M0's 32KB memory. TBD what the
                              // best approach is moving forward.
};

#endif
