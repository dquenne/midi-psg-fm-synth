#ifndef Sysex_h
#define Sysex_h

#include "Patch.h"
#include <Arduino.h>

#define MANUFACTURER_ID 0x7d // educational use manufacturer ID

enum SysexMessageType : byte {
  SYSEX_MESSAGE_TYPE_FM_SINGLE_VOICE_DUMP,
  SYSEX_MESSAGE_TYPE_FM_VOICE_BANK_DUMP,
  SYSEX_MESSAGE_TYPE_FM_TFI_SINGLE_VOICE_DUMP,
  SYSEX_MESSAGE_TYPE_PSG_SINGLE_VOICE_DUMP = 0b1000,
};

struct FmPatchDump {
  PatchId target_patch_id;
  bool update_buffered_patch;
  FmPatch patch_data;
};

#endif
