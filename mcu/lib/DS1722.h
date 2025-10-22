// Sebastian Heredia
// dheredia@g.hmc.edu
// October 17, 2025

// DS1722.h contains header file code for DS1722.h to interface with the MCU.

#ifndef DS1722_H
#define DS1722_H

#include <stdint.h>
#include "stm32l432xx.h"
#include "STM32L432KC_SPI.h"

// Sets the DS1722 temperature resolution (8-12 bits)
void writeResolution(int resolution);

// Reads the current temperature in ºC, including negative values
double readTemperature(void);

// Helper function to calculate temperature from MSB and LSB (handles negative temps)
double calcTemperature(uint8_t msb, uint8_t lsb);

#endif
