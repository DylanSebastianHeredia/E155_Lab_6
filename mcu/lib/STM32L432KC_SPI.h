// Sebastian Heredia
// dheredia@g.hmc.edu
// October 17, 2025

// STM32L432KC_SPI.h contains header file information for STM32L432KC_SPI.c.


#ifndef STM32L4_SPI_H
#define STM32L4_SPI_H

#include <stdint.h>
#include <stm32l432xx.h>

// Defining macros for STM32L432KC.c & main.c
#define SPI1_SCK    PA5     // SCK = Serial Clock
#define SPI1_MOSI   PB5     // MOSI (COPI): Controller Out Peripheral In
#define SPI1_MISO   PA6     // MISO (CIPO): Controller In Peripheral Out
#define SPI1_CE     PA8    // CE = Chip Enable 

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

/* Enables the SPI peripheral and intializes its clock speed (baud rate), polarity, and phase.
 *    -- br: (0b000 - 0b111). The SPI clk will be the master clock / 2^(BR+1).
 *    -- cpol: clock polarity (0: inactive state is logical 0, 1: inactive state is logical 1).
 *    -- cpha: clock phase (0: data captured on leading edge of clk and changed on next edge, 
 *          1: data changed on leading edge of clk and captured on next edge)
 * Refer to the datasheet for more low-level details. */ 
void initSPI(int br, int cpol, int cpha);

/* Transmits a character (1 byte) over SPI and returns the received character.
 *    -- send: the character to send over SPI
 *    -- return: the character received over SPI */
char spiSendReceive(char send);

#endif