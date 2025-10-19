// Sebastian Heredia
// dheredia@g.hmc.edu
// October 17, 2025

// DS1722.h contains header file code for DS1722.h to interface with the MCU.

#ifndef DS1722_H
#define DS1722_H

#include <stdint.h>
#include "stm32l432xx.h"
#include "STM32L432KC_SPI.h"

void writeResolution(int resolution);

float readTemperature(void);

#endif