#include "IoUtils.h"

#include <Arduino.h>

void setupPinModes() {
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

  // control pins
  pinMode(IO_PIN_WRITE_ENABLE, OUTPUT);
  pinMode(IO_PIN_YM_CHIPS_A0, OUTPUT);
}

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

void clockDelay(unsigned count) {
  unsigned elapsed;
  for (elapsed = 0; elapsed < count; elapsed++) {
    NOP4;
    NOP4;
  }
}

uint32_t CUSTOM_PORT_OUT_MASK =
    ((0b11 << 22) | (0b11 << 18) | (0b11 << 15) | (0b11 << 4));

void setA0(bool value) { digitalWrite(IO_PIN_YM_CHIPS_A0, value); }

void setWriteEnable(bool enabled) {
  if (enabled) {
    digitalWrite(IO_PIN_WRITE_ENABLE, LOW);
  } else {
    digitalWrite(IO_PIN_WRITE_ENABLE, HIGH);
  }
}

void setByteOut(byte data) {
  int bit_pairs[] = {
      (0b11000000 & data) >> 6,
      (0b00110000 & data) >> 4,
      (0b00001100 & data) >> 2,
      0b00000011 & data,
  };

  PORT->Group[0].OUT.reg &= ~CUSTOM_PORT_OUT_MASK;
  PORT->Group[0].OUT.reg |=
      (CUSTOM_PORT_OUT_MASK & ((bit_pairs[0] << 22) | (bit_pairs[1] << 18) |
                               (bit_pairs[2] << 15) | (bit_pairs[3] << 4)));
}
