#include "Storage.h"

FlashStorage(active_multi_storage, Multi);

void storeMulti(Multi *multi) {
  digitalWrite(13, HIGH);
  active_multi_storage.write(*multi);
  delay(500);
  digitalWrite(13, LOW);
}

void retrieveMulti(Multi *target_multi) {
  digitalWrite(13, HIGH);
  Multi stored_multi = active_multi_storage.read();
  memcpy(target_multi, &stored_multi, sizeof stored_multi);
  delay(500);
  digitalWrite(13, LOW);
}
