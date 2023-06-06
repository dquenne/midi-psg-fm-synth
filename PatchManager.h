#ifndef PatchManager_h
#define PatchManager_h

#include "Patch.h"

template <typename PatchType> class PatchManager {
public:
  PatchType *getPatch(PatchId patch_id) {
    if (patch_id.bank_number != 0) {
      return &null_patch;
    }
    return &bank_0[patch_id.program_number];
  }

  void loadPatchBanks(){};

protected:
  PatchType bank_0[PATCH_BANK_SIZE];
  PatchType null_patch;
};

#endif
