#include "Multi.h"
#include "NoteMappings.h"
#include "Presets.h"
#include "VoiceManager.h"
#include "notes.h"
#include "sn76489.h"
#include <Arduino.h>
#include <MIDI.h>

#define NOP asm volatile("nop\n\t")
#define NOP4 asm volatile("nop\n\tnop\n\tnop\n\tnop\n\t")

#define LED_PIN 13

MIDI_CREATE_DEFAULT_INSTANCE();

byte echo_pitch = 0;
unsigned echo_ticks_remaining = 0;
unsigned echo_note_off_ticks_remaining = 0;

/**
 * @param[in] division is the factor that 8MHz is divided by. Should be 2 or
 * greater for SN76489, which has a maximum of 4MHz input clock.
 */
void setClockOut(unsigned division) {
  PORT->Group[PORTA].PINCFG[20].bit.PMUXEN =
      1; // Switch on port pin PA20's multiplexer
  PORT->Group[PORTA].PMUX[20 >> 1].reg |=
      PORT_PMUX_PMUXE_H; // Switch the PA20's port multiplexer to GCLK IO

  GCLK->GENCTRL.reg = GCLK_GENCTRL_OE |        // Enable GCLK4 output
                      GCLK_GENCTRL_IDC |       // Improve duty-cycle to 50%
                      GCLK_GENCTRL_GENEN |     // Enable generic clock
                      GCLK_GENCTRL_SRC_OSC8M | // Set the clock source to the
                                               // internal 8MHz oscillator
                      GCLK_GENCTRL_ID(4);      // Set the GCLK ID to GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY)
    ; // Wait for synchronization

  GCLK->GENDIV.reg =
      GCLK_GENDIV_DIV(division) |
      GCLK_GENDIV_ID(4); // Set GLCK4 division to 2 (8MHz / 2 = 4MHz)
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;
}

Sn76489Instance sn76489_1(2);

VoiceManager voice_manager(3);

Multi main_multi;

void handleNoteOn(byte channel, byte pitch, byte velocity) {
  if (NOTES_4MHZ[pitch] > 1023) {
    return;
  }
  digitalWrite(LED_PIN, HIGH);

  // this approach to a delay/echo is super naive and broken, but accomplishes
  // the effect if you play notes slowly. real implementation of a delay to
  // come after voice mapping is better fleshed out.
  echo_pitch = pitch;
  echo_ticks_remaining = 170;
  Voice *voice = voice_manager.getVoice(channel, pitch);
  voice->setPatch(&main_multi.channels[channel - 1]);
  voice->noteOn(channel, pitch, velocity);

  digitalWrite(LED_PIN, LOW);
}

void handleNoteOff(byte channel, byte pitch, byte velocity) {
  Voice *voice = voice_manager.getExactVoice(channel, pitch);
  voice->noteOff();
  echo_note_off_ticks_remaining = 170;
}

void setup() {
  setClockOut(2);

  pinMode(LED_PIN, OUTPUT); // LED output

  // pins connected to D0-D7 on SN76489AN chip
  pinMode(A3, OUTPUT); // PA04
  pinMode(A4, OUTPUT); // PA05
  pinMode(5, OUTPUT);  // PA15
  pinMode(11, OUTPUT); // PA16
  pinMode(10, OUTPUT); // PA18
  pinMode(12, OUTPUT); // PA19
  pinMode(20, OUTPUT); // PA22
  pinMode(21, OUTPUT); // PA23

  // control pin
  pinMode(9, OUTPUT); // WE write enable (low = enable)

  delay(1);

  sn76489_1.setup();

  delay(10);

  MIDI.turnThruOff();
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);

  for (byte channel = 0; channel < 16; channel++) {
    applyPreset(&main_multi.channels[channel], PRESETS[channel]);
  }
}

unsigned long last_millis = millis();
byte last_echo_pitch = 0;

void loop() {
  MIDI.read();

  // silly way to get 1ms ticks
  while (last_millis >= millis())
    ;

  // if (echo_ticks_remaining == 1) {

  //   Voice *lastEchoVoice = voice_manager.getVoice(17, last_echo_pitch);
  //   lastEchoVoice->noteOff();

  //   Voice *echoVoice = voice_manager.getVoice(17, echo_pitch);
  //   echoVoice->setPatch(PRESETS[3]);
  //   echoVoice->noteOn(17, echo_pitch, 40);
  //   last_echo_pitch = echo_pitch;
  // }
  // if (echo_note_off_ticks_remaining == 1) {
  //   Voice *echoVoice = voice_manager.getVoice(17, echo_pitch);
  //   echoVoice->noteOff();
  // }
  // if (echo_ticks_remaining > 0) {
  //   echo_ticks_remaining--;
  // }

  // if (echo_note_off_ticks_remaining > 0) {
  //   echo_note_off_ticks_remaining--;
  // }

  last_millis = millis();

  voice_manager.tick();

  // TODO: clean up syncing from VoiceManager to actual chip tone channels
  sn76489_1.tone_channels[0].writeLevel(voice_manager.voices[0].level);
  sn76489_1.tone_channels[0].writePitch(
      voice_manager.voices[0].frequency_cents);
  sn76489_1.tone_channels[1].writeLevel(voice_manager.voices[1].level);
  sn76489_1.tone_channels[1].writePitch(
      voice_manager.voices[1].frequency_cents);
  sn76489_1.tone_channels[2].writeLevel(voice_manager.voices[2].level);
  sn76489_1.tone_channels[2].writePitch(
      voice_manager.voices[2].frequency_cents);
  return;
}
