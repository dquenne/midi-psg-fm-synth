#ifndef Chip_h
#define Chip_h

class PsgChannel {
public:
  virtual void setChannel(unsigned channel_number) = 0;
  virtual void initialize() = 0;
  // low-level write methods
  virtual void writeFrequencyN(unsigned frequency_n) = 0;
  virtual void writeAttenuation(unsigned attenuation, bool force = false) = 0;

  // write methods in terms of pitch & level
  virtual void writePitch(unsigned frequency_cents) = 0;
  virtual void writeLevel(unsigned level, bool force = false) = 0;
};

class Chip {
public:
  virtual void setup() = 0;
  virtual PsgChannel *getPsgChannel(unsigned channel_number) = 0;
};

#endif
