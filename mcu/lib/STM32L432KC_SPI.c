// Sebastian Heredia 
// dheredia@g.hmc.edu
// October 17, 2025

// STM32L432KC_SPI.c contains source code for SPI functions.

#include "STM32L432KC.h"
#include "STM32L432KC_SPI.h"
#include "STM32L432KC_GPIO.h"
#include "STM32L432KC_RCC.h"

void initSPI(int br, int cpol, int cpha) {
    // Turn on GPIOA and GPIOB clock domains (GPIOAEN and GPIOBEN bits in AHB1ENR)
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN);
    
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // Turn on SPI1 clock domain (SPI1EN bit in APB2ENR)

    // Enable A,B, GPIO pins
    gpioEnable(GPIO_PORT_A);
    gpioEnable(GPIO_PORT_B);
    gpioEnable(GPIO_PORT_C);

    // Assing SPI pins
    pinMode(SPI1_SCK, GPIO_ALT);
    pinMode(SPI1_MOSI, GPIO_ALT);  
    pinMode(SPI1_MISO, GPIO_ALT);  
    pinMode(SPI1_CE, GPIO_OUTPUT);

    // Configure SPI
    GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED3); // Set output speed type to high for SCK

    // Set to AF05 for SPI alternate functions
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL3, 5);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL4, 5);
    GPIOB->AFR[0] |= _VAL2FLD(GPIO_AFRL_AFSEL5, 5);
    
    // CR1 CONFIGURATION
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, br);    // Set baud rate divider

    // Set MCU as Master
    SPI1->CR1 |= (SPI_CR1_MSTR);
    SPI1->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_LSBFIRST | SPI_CR1_SSM);

    // Set chpa & cpol
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha);
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol);

    // CR2 CONFIGURATION
    SPI1->CR2 |= _VAL2FLD(SPI_CR2_DS, 0b0111);

    SPI1->CR2 |= (SPI_CR2_FRXTH | SPI_CR2_SSOE);

    // Enable SPI
    SPI1->CR1 |= (SPI_CR1_SPE); 
}

char spiSendReceive(char send) {
    while(!(SPI1->SR & SPI_SR_TXE));        // Wait until the transmit buffer is empty
    *(volatile char *) (&SPI1->DR) = send;  // Transmit the character over SPI
    while(!(SPI1->SR & SPI_SR_RXNE));       // Wait until data has been received
    char rec = (volatile char) SPI1->DR;    // rec = received data
    return rec;                             // Return received character
}
