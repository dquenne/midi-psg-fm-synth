#include "Storage.h"
#include "Patch.h"

FlashStorage(psg_bank_0_storage, PsgPatchBank);
FlashStorage(fm_bank_0_storage, FmPatchBank);

void storePsgBank(PsgPatchBank *bank) {
  digitalWrite(13, HIGH);
  psg_bank_0_storage.write(*bank);
  delay(500);
  digitalWrite(13, LOW);
}

void retrievePsgBank(PsgPatchBank *target_bank) {
  digitalWrite(13, HIGH);
  PsgPatchBank stored_bank = psg_bank_0_storage.read();
  memcpy(target_bank, &stored_bank, sizeof stored_bank);
  delay(500);
  digitalWrite(13, LOW);
}

void storeFmBank(FmPatchBank *bank) {
  digitalWrite(13, HIGH);
  fm_bank_0_storage.write(*bank);
  delay(500);
  digitalWrite(13, LOW);
}

void retrieveFmBank(FmPatchBank *target_bank) {
  digitalWrite(13, HIGH);
  FmPatchBank stored_bank = fm_bank_0_storage.read();
  memcpy(target_bank, &stored_bank, sizeof stored_bank);
  delay(500);
  digitalWrite(13, LOW);
}
