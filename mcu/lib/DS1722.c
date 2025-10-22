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
    char configStatus = 0xE0;

    if (resolution == 8)   configStatus = 0xE0;
    if (resolution == 9)   configStatus = 0xE2;
    if (resolution == 10)  configStatus = 0xE4;
    if (resolution == 11)  configStatus = 0xE6;
    if (resolution == 12)  configStatus = 0xE8;

    // SPI write to configuration register
    digitalWrite(SPI1_CE, PIO_HIGH);
    spiSendReceive(0x80);                   // Table 4: Configuration (Write Address) is 0x80
    spiSendReceive(configStatus);           // Write the selected configuration status
    digitalWrite(SPI1_CE, PIO_LOW);         // End transaction

    delay_millis(TIM15, 1000);
}

// readTemperature reads the current temperature in ºC.
double readTemperature(void) {
    uint8_t LSB = 0;    // Least significant bit
    uint8_t MSB = 0;    // Most significant bit

    // Enable SPI
    digitalWrite(SPI1_CE, PIO_HIGH);        // Enabling Chip Enable 

    // Read MSB (Table 3: 0x02)
    spiSendReceive(0x02);
    MSB = spiSendReceive(0x00);
    digitalWrite(SPI1_CE, PIO_LOW);         // Pulse CE for next address

    // Read LSB (Table 3: 0x01)
    digitalWrite(SPI1_CE, PIO_HIGH);
    spiSendReceive(0x01);
    LSB = spiSendReceive(0x00);
    digitalWrite(SPI1_CE, PIO_LOW);         // End SPI transaction

    // Concatenate bytes and calculate temperature
    return calcTemperature(MSB, LSB);
}

// calcTemperature calculates +/- temperature values
double calcTemperature(uint8_t MSB, uint8_t LSB) {
    double temperature = 0.0;

    // Concatenate MSB and LSB into a signed 16-bit rawTemp
    uint16_t rawTemp = ((uint16_t)MSB << 8) | LSB;

    // Check if the sign bit (bit 15) is set
    if (rawTemp & 0x8000) {

        // NEGATIVE TEMPERATURE: 
        int16_t signedValue = (int16_t)rawTemp;       // Convert from two's complement

        // Table 3: 0.5ºC = 128d, so 1ºC maps to 256d. Thus 1 LSB = 1/256ºC
        temperature = (double)signedValue / 256.0;
    }

    else {

        // POSITIVE TEMPERATURE:
        temperature = (double)MSB + ((double)LSB / 256.0);
    }

    return temperature;
}
