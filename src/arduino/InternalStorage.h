
#ifndef InternalStorage_h
#define InternalStorage_h
#include "Patch.h"

#include "Multi.h"
// #include <FlashStorage.h>

#include <SPI.h>

#include "SdFat.h"

#include "Adafruit_SPIFlash.h"

#define FLASH_SS SS1        // Flash chip SS pin.
#define FLASH_SPI_PORT SPI1 // What SPI port is Flash on?

#define BANK_DIR "/banks"
#define BANK_PREFIX_PSG "/banks/psg"
#define BANK_PREFIX_FM "/banks/fm"
#define BANK_PREFIX(bank_type)                                                 \
  (bank_type == INTERNAL_STORAGE_FM ? BANK_PREFIX_FM : BANK_PREFIX_PSG)

#define BANK_EXTENSION_PSG ".psgpatch"
#define BANK_EXTENSION_FM ".fmpatch"
#define BANK_EXTENSION(bank_type)                                              \
  (bank_type == INTERNAL_STORAGE_FM ? BANK_EXTENSION_FM : BANK_EXTENSION_PSG)

enum InternalStorageBankType {
  INTERNAL_STORAGE_PSG,
  INTERNAL_STORAGE_FM,
};

void beginStorage();

template <typename PatchType> class PatchStorage {
public:
  PatchStorage(InternalStorageBankType patch_type) : _patch_type(patch_type) {}

  void initializeBank(uint16_t bank_number, const PatchType *default_patch);
  void writePatch(PatchId *patch_id, const PatchType *patch);
  void readPatch(PatchId *patch_id, PatchType *target_patch);

protected:
  InternalStorageBankType _patch_type;
};

#endif
