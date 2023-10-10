#ifndef IoUtils_h
#define IoUtils_h

#include <Arduino.h>

#define NOP asm volatile("nop\n\t")
#define NOP4 asm volatile("nop\n\tnop\n\tnop\n\tnop\n\t")

#define LED_PIN 13

#define IO_PIN_WRITE_ENABLE 9 // WE write enable (low = enable)
#define IO_PIN_YM_CHIPS_A0 19 // control writing an address vs data (YM chips)

void setupPinModes();

/**
 * @param[in] division is the factor that 8MHz is divided by. Should be 2 or
 * greater for SN76489, which has a maximum of 4MHz input clock.
 */
void setClockOut(unsigned division);

void clockDelay(unsigned count);

void setA0(bool value);

void setWriteEnable(bool enabled);

void setByteOut(byte data);

#endif
