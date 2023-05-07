#include <Arduino.h>

#define PSG_CHANNEL_TONE_1 0b00
#define PSG_CHANNEL_TONE_2 0b01
#define PSG_CHANNEL_TONE_3 0b10
#define PSG_CHANNEL_NOISE 0b11
#define PSG_CHANNEL_UNSPECIFIED 0b100

#define PSG_REGISTER_TYPE_FREQUENCY 0b0
#define PSG_REGISTER_TYPE_NOISE_CONTROL 0b0
#define PSG_REGISTER_TYPE_ATTENUATION 0b1

/* Masks & offsets for building messages */
#define PSG_CHANNEL_MASK 0b11
#define PSG_CHANNEL_OFFSET 5

#define PSG_REGISTER_TYPE_MASK 0b1
#define PSG_REGISTER_TYPE_OFFSET 4

#define PSG_FREQUENCY_MASK 0b1111
#define PSG_FREQUENCY_OFFSET 0

#define PSG_ATTENUATION_MASK 0b1111
#define PSG_ATTENUATION_OFFSET 0

#define PSG_NOISE_FREQUENCY_MASK 0b11
#define PSG_NOISE_FREQUENCY_OFFSET 0

#define PSG_NOISE_FEEDBACK_MASK 0b1
#define PSG_NOISE_FEEDBACK_OFFSET 2

class Sn76489Instance;

class Sn76489ToneChannel {
public:
  Sn76489ToneChannel();
  Sn76489ToneChannel(unsigned channel);

  void setChannel(unsigned channel);
  void initialize();
  // low-level write methods
  void writeFrequencyN(unsigned frequency_n);
  void writeAttenuation(unsigned attenuation, bool force = false);

  // write methods in terms of pitch & level
  void writePitch(unsigned frequency_cents);
  void writeLevel(unsigned level, bool force = false);

private:
  Sn76489Instance *_sn76489_instance;
  unsigned _channel;
  unsigned _last_frequency_written;
  unsigned _last_attenuation_written;
};

class Sn76489Instance {
public:
  Sn76489Instance(unsigned clock_division);
  Sn76489ToneChannel tone_channels[3];
  void setup();
  void write(byte data);

private:
  void _writeEnable(bool enabled);
  void _setByteOut(byte data);
  void clockDelay(unsigned count);
  unsigned _clock_division;
};
