#include "InternalStorage.h"

Adafruit_FlashTransport_SPI
    internalFlashTransport(FLASH_SS,
                           &FLASH_SPI_PORT); // Use hardware SPI

Adafruit_SPIFlash flash(&internalFlashTransport);

FatVolume fatfs;
File32 temp_file_buffer;

void beginStorage() {
  Serial.println("Initializing interla flash storage");
  digitalWrite(13, HIGH);
  flash.begin();
  if (!fatfs.begin(&flash)) {
    Serial.println("Error: failed to initialize filesystem.");
    while (1) {
      yield();
      delay(1);
    }
  }
  digitalWrite(13, LOW);
}

String getDirectory(PatchId *patch_id, InternalStorageBankType bank_type) {
  String directory = "";
  directory += BANK_PREFIX(bank_type);
  directory += patch_id->bank_number_lsb;
  return directory;
}

String getFilename(PatchId *patch_id, InternalStorageBankType bank_type) {
  String filename = getDirectory(patch_id, bank_type);
  filename += "/patch";
  filename += patch_id->program_number;
  filename += BANK_EXTENSION(bank_type);
  return filename;
}

template <typename PatchType>
void PatchStorage<PatchType>::initializeBank(uint16_t bank_number,
                                             const PatchType *default_patch) {
  PatchId patch_id = {0, bank_number};

  for (byte program_number = 0; program_number < 16; program_number++) {
    patch_id.program_number = program_number;
    String filename = getFilename(&patch_id, _patch_type);

    if (!fatfs.exists(filename.c_str())) {
      writePatch(&patch_id, default_patch);
    }
  }
}

unsigned getPatchSize(InternalStorageBankType bank_type) {
  if (bank_type == INTERNAL_STORAGE_FM) {
    FmPatch dummy_patch;
    return sizeof(dummy_patch);
  } else {
    PsgPatch dummy_patch;
    return sizeof(dummy_patch);
  }
}

template <typename PatchType>
void PatchStorage<PatchType>::writePatch(PatchId *patch_id,
                                         const PatchType *patch) {
  String directory = getDirectory(patch_id, _patch_type);

  fatfs.mkdir(directory.c_str(), true);

  String filename = getFilename(patch_id, _patch_type);

  temp_file_buffer = fatfs.open(filename, O_RDWR | O_CREAT | O_TRUNC);
  temp_file_buffer.seekSet(0);

  int wrote_bytes =
      temp_file_buffer.write((byte *)patch, getPatchSize(_patch_type));

  Serial.print("wrote ");
  Serial.print(wrote_bytes, DEC);
  Serial.print(" bytes to ");
  Serial.println(filename);

  temp_file_buffer.close();
}

template <typename PatchType>
void PatchStorage<PatchType>::readPatch(PatchId *patch_id,
                                        PatchType *target_patch) {
  byte read_buffer[getPatchSize(_patch_type)];

  String filename = getFilename(patch_id, _patch_type);

  temp_file_buffer = fatfs.open(filename, O_RDONLY);

  temp_file_buffer.seekSet(0);

  int read_bytes =
      temp_file_buffer.read(read_buffer, getPatchSize(_patch_type));
  Serial.print("read ");
  Serial.print(read_bytes, DEC);
  Serial.print(" bytes from ");
  Serial.println(filename);

  temp_file_buffer.close();

  if (read_bytes < 0) {
    // failed to read patch
    return;
  }

  memcpy(target_patch, read_buffer, getPatchSize(_patch_type));
}

template class PatchStorage<PsgPatch>;
template class PatchStorage<FmPatch>;
