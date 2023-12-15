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

#endif
