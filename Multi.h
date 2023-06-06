#ifndef Multi_h
#define Multi_h

#include "Patch.h"
#include "VoiceManager.h"

enum MultiChannelMode { MULTI_CHANNEL_MODE_PSG, MULTI_CHANNEL_MODE_FM };

struct MultiChannel {
  MultiChannelMode mode;
  PatchId patch_id;
};

struct Multi {
  bool _is_valid;
  MultiChannel channels[16];
};

#endif
