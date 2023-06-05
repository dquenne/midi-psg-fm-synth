#include "Chip.h"
#include "Delay.h"
#include "IoUtils.h"
#include "MidiManager.h"
#include "Multi.h"
#include "NoteMappings.h"
#include "Presets.h"
#include "Storage.h"
#include "Synth.h"
#include "VoiceManager.h"
#include "Ym2203.h"
#include "sn76489.h"
#include <Arduino.h>
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

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

void handleControlChange(byte channel, byte pitch, byte velocity) {
  // Arduino MIDI Library adds 1 to the channel so that it's 1-indexed, which
  // is not ideal, and hard to keep track of.
  byte zero_indexed_channel = channel - 1;

  midi_manager.handleControlChange(zero_indexed_channel, pitch, velocity);
}

void initializeMainMulti() {
  retrieveMulti(&main_multi);
  if (main_multi._is_valid) {
    // main_multi already recorded in memory, don't initialize
    return;
  }
  for (byte channel = 0; channel < 16; channel++) {
    applyPsgPreset(&main_multi.channels[channel], PRESETS[channel]);
  }
  main_multi._is_valid = true;
  applyPsgPreset(&main_multi.channels[4], PRESETS[2]);
  applyPsgPreset(&main_multi.channels[1], PRESETS[3]);
}

void setup() {
  midi_delay.handleNoteOn = handleNoteOn;
  midi_delay.handleNoteOff = handleNoteOff;
  setClockOut(2);

  setupPinModes();

  delay(1);

  sound_chip->setup();

  delay(10);

  MIDI.turnThruOff();
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);

  initializeMainMulti();
}

unsigned long last_millis = millis();

void loop() {
  MIDI.read();

  // silly way to get 1ms ticks
  while (last_millis >= millis())
    ;

  last_millis = millis();

  synth.tick();
  midi_delay.tick();
}
