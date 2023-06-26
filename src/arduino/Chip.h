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

  /** Write patch parameters that only change when a patch is changed (i.e. they
   * are not controlled by things like pitch, velocity, etc.). */
  virtual void writeStaticPatchParameters(const FmPatch *patch) = 0;
  virtual void writePitch(unsigned frequency_cents) = 0;
  virtual void writeTotalLevel(unsigned op, unsigned total_level) = 0;
  virtual void writeKeyOnOff(bool key_on, bool force = false) = 0;
};

class Chip {
public:
  virtual void setup() = 0;
  virtual PsgChannel *getPsgChannel(unsigned channel_number) = 0;
  virtual FmChannel *getFmChannel(unsigned channel_number) = 0;
};

#endif
