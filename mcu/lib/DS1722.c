// Sebastian Heredia
// dheredia@g.hmc.edu
// October 17, 2025

// DS1722.c contains code to interface the DS1722 temperature sensor. 

#include "DS1722.h"
#include <stdint.h>
#include <stdio.h>
#include "STM32L432KC_GPIO.h"
#include "STM32L432KC_TIM.h"
#include "STM32L432KC_SPI.h"

// writeResolution sets the DS1722 temperature resolution (8-12 bits)
void writeResolution(int resolution){
    uint8_t configStatus = 0xE0;  // Table 2: Cofiguration for continuous conversion starts w/ base: 1110_0000b = 0xE0

    // Table 5: Generate case statements for selecting resolution
    switch (resolution) {
        case 8:     configStatus |= (0b000 << 1); break;    // Shifting bits left by one space
        case 9:     configStatus |= (0b001 << 1); break;    // Bitwise OR and assign: configStatus = configStatus | (something)
        case 10:    configStatus |= (0b010 << 1); break;    
        case 11:    configStatus |= (0b011 << 1); break;
        case 12:    configStatus |= (0b111 << 1); break;

        default:    configStatus |= (0b001 << 1); break;    // Default case to 9-bits based on datasheet default
    }

    // SPI write to configuration register
    digitalWrite(SPI1_CE, PIO_HIGH);        // Enabling Chip Enable
    spiSendReceive(0x80);                   // Table 4: Configuration (Write Address) is 0x80
    spiSendReceive(configStatus);           // Write the selected configuration status
    digitalWrite(SPI1_CE, PIO_LOW);         // End transaction

    delay_millis(TIM15, 100);               // Allow time for request to settle
}

// readTemperature reads the current temperature in ºC.
float readTemperature(void) {
    uint8_t LSB = 0;    // Least significant bit
    uint8_t MSB = 0;    // Most signiticant bit

    // Enable SPI
    digitalWrite(SPI1_CE, PIO_HIGH);        // Enabling Chip Enable 

    // Read MSB (Table 3: 0x02)
    spiSendReceive(0x02);
    MSB = spiSendReceive(0x00);             // Sending dummy byte
    digitalWrite(SPI1_CE, PIO_LOW);         // Pulse CE for next address
    digitalWrite(SPI1_CE, PIO_HIGH);

    // Read LSB (Table 3: 0x01)
    spiSendReceive(0x01);
    LSB = spiSendReceive(0x00);
    digitalWrite(SPI1_CE, PIO_LOW);         // End SPI transaction

    // Concatenate bytes into signed 16-bit
    int16_t rawTemp = ((int16_t)MSB << 8) | LSB;

    // Table 3: 0.5ºC = 128d, so 1ºC maps to 256d. Thus 1 LSB = 1/256ºC
    float temperature = (float)rawTemp / 256.0f;

    return temperature;
}