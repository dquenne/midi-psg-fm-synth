#include "Patch.h"
#include "NoteMappings.h"

unsigned getDelayTicks(byte delay_time) { return 4 * delay_time; }

void applyPsgPreset(PsgPatch *target, const PsgPatch *preset) {
  memcpy(target, preset, sizeof(*preset));
}

void applyFmPreset(FmPatch *target, const FmPatch *preset) {
  memcpy(target, preset, sizeof(*preset));
}
