#include "Chip.h"
#include "Delay.h"
#include "InternalStorage.h"
#include "IoUtils.h"
#include "MidiManager.h"
#include "Multi.h"
#include "NoteMappings.h"
#include "Presets.h"
#include "Synth.h"
#include "VoiceManager.h"
#include "Ym2203.h"
#include "sn76489.h"
#include <Adafruit_DotStar.h>
#include <Arduino.h>
#include <MIDI.h>
#include <SPI.h>
#include <USB-MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();
USBMIDI_CREATE_INSTANCE(0, MIDI_USB);

Adafruit_DotStar dotstar(1, 41, 40, DOTSTAR_BGR);

// Chip *sound_chip = new Sn76489Instance(2);
Chip *sound_chip = new Ym2203Instance(2);

Multi main_multi;
Synth synth(sound_chip, &main_multi);

MidiDelay midi_delay;
MidiManager midi_manager(&synth, &midi_delay);

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  // Arduino MIDI Library adds 1 to the channel so that it's 1-indexed, which
  // is not ideal, and hard to keep track of.
  byte zero_indexed_channel = channel - 1;

  midi_manager.handleNoteOn(zero_indexed_channel, pitch, velocity);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  // Arduino MIDI Library adds 1 to the channel so that it's 1-indexed, which
  // is not ideal, and hard to keep track of.
  byte zero_indexed_channel = channel - 1;

  midi_manager.handleNoteOff(zero_indexed_channel, pitch, velocity);
}

void handlePitchBend(byte channel, int bend) {
  // Arduino MIDI Library adds 1 to the channel so that it's 1-indexed, which
  // is not ideal, and hard to keep track of.
  byte zero_indexed_channel = channel - 1;

  midi_manager.handlePitchBend(zero_indexed_channel, bend);
}

void handleControlChange(byte channel, byte pitch, byte velocity) {
  // Arduino MIDI Library adds 1 to the channel so that it's 1-indexed, which
  // is not ideal, and hard to keep track of.
  byte zero_indexed_channel = channel - 1;

  midi_manager.handleControlChange(zero_indexed_channel, pitch, velocity);
}

void handleProgramChange(byte channel, byte program_number) {
  // Arduino MIDI Library adds 1 to the channel so that it's 1-indexed, which
  // is not ideal, and hard to keep track of.
  byte zero_indexed_channel = channel - 1;

  midi_manager.handleProgramChange(zero_indexed_channel, program_number);
}

void handleSystemExclusive(byte *array, unsigned size) {
  midi_manager.handleSysex(array, size);
}

void initializeMainMulti() {
  digitalWrite(13, HIGH);
  Serial.println("Initializing patch banks");
  synth.getFmPatchManager()->initializePatchBanks(FM_PRESETS[0]);
  synth.getPsgPatchManager()->initializePatchBanks(PSG_PRESETS[0]);
  digitalWrite(13, LOW);

  for (byte channel = 0; channel < 16; channel++) {
    PatchId patch_id = {channel, 0};

    synth.getPsgPatchManager()->loadPatch(&patch_id, channel);
    synth.getFmPatchManager()->loadPatch(&patch_id, channel);
  }

  for (byte channel = 0; channel < 16; channel++) {
    main_multi.channels[channel].mode = MULTI_CHANNEL_MODE_PSG;
    main_multi.channels[channel].patch_id = {channel, 0};
  }
  main_multi._is_valid = true;

  main_multi.channels[1].patch_id = {3, 0};
  main_multi.channels[4].patch_id = {2, 0};

  main_multi.channels[1].mode = MULTI_CHANNEL_MODE_FM;
  main_multi.channels[1].patch_id = {0, 0};

  main_multi.channels[4].mode = MULTI_CHANNEL_MODE_FM;
  main_multi.channels[4].patch_id = {1, 0};
}

void setup() {
  dotstar.begin();
  dotstar.show();

  midi_delay.handleNoteOn = handleNoteOn;
  midi_delay.handleNoteOff = handleNoteOff;

  setClockOut(2);
  setupPinModes();

  digitalWrite(LED_PIN, HIGH);

  sound_chip->setup();

  delay(10);
  digitalWrite(LED_PIN, LOW);

  MIDI.turnThruOff();
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandlePitchBend(handlePitchBend);
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.setHandleProgramChange(handleProgramChange);

  MIDI_USB.turnThruOff();
  MIDI_USB.begin(MIDI_CHANNEL_OMNI);
  MIDI_USB.setHandleControlChange(handleControlChange);

  beginStorage();

  initializeMainMulti();

  synth.initialize();
}

unsigned long last_millis = millis();

void loop() {
  MIDI.read();
  MIDI_USB.read();

  // silly way to get 1ms ticks
  while (last_millis >= millis())
    ;

  if (millis() - last_millis > 1) {
    Serial.print("Lost frame(s): ");
    Serial.println(millis() - last_millis, DEC);
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  last_millis = millis();

  synth.tick();
  midi_delay.tick();

  dotstar.setPixelColor(
      0, synth.getTotalPsgLevel(), synth.getTotalFmLevel() / 4,
      synth.getTotalPsgLevel() / 2 + synth.getTotalFmLevel() / 2);
  dotstar.show();
}
