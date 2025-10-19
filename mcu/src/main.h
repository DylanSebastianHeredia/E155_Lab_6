// Sebastian Heredia
// dheredia@g.hmc.edu
// October 17, 2025

// main.h contains header file code to set up main.c. 

#ifndef MAIN_H
#define MAIN_H

#include "STM32L432KC.h"
#include "DS1722.h"         // Contains SPI1 macros

#define LED_PIN     PB3     // LED pin for blinking on Port B pin 3
#define BUFF_LEN    32      // Buffer length (ms)

#endif // MAIN_H