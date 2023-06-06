#ifndef Chip_h
#define Chip_h

#include "Patch.h"

class PsgChannel {
public:
  virtual void setChannel(unsigned channel_number) = 0;
  virtual void initialize() = 0;

  // write methods in terms of pitch & level
  virtual void writePitch(unsigned frequency_cents) = 0;
  virtual void writeLevel(unsigned level, bool force = false) = 0;
};

class FmChannel {
public:
  virtual void setChannel(unsigned channel_number) = 0;
  virtual void initialize() = 0;

  virtual void writeAllPatchParameters(const FmPatch *patch) = 0;
  virtual void writePitch(unsigned frequency_cents) = 0;
  virtual void writeKeyOnOff(bool key_on, bool force = false) = 0;
};

class Chip {
public:
  virtual void setup() = 0;
  virtual PsgChannel *getPsgChannel(unsigned channel_number) = 0;
  virtual FmChannel *getFmChannel(unsigned channel_number) = 0;
};

#endif
