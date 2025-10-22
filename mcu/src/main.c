// Sebastian Heredia
// dheredia@g.hmc.edu
// October 17, 2025

// main.c contains code to generate a webpage using the ESP8266 via USART and 
// process temperature data from the DS1722 over SPI. 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "DS1722.h"

/////////////////////////////////////////////////////////////////
// Provided Constants and Functions
/////////////////////////////////////////////////////////////////

//Defining the web page in two chunks: everything before the current time, and everything after the current time
char* webpageStart = "<!DOCTYPE html><html><head><title>HEREDIA: E155 Lab 6 IoT Webserver</title>\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
    </head>\
    <body><h1>HEREDIA: E155 Lab 6 IoT Webserver</h1>";
  
char* ledStr = "<p>LED Control:</p><form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
    <form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>"; // Copied this line for Temperature

char* tempStr = "<h2>Temperature</h2>\
        <p>Temperature Resolution:</p>\
        <form action=\"8\"><input type=\"submit\" value=\"8 bits: 0.0000\"></form>\
        <form action=\"9\"><input type=\"submit\" value=\"9 bits: 0.5000\"></form>\
        <form action=\"10\"><input type=\"submit\" value=\"10 bits: 0.2500\"></form>\
        <form action=\"11\"><input type=\"submit\" value=\"11 bits: 0.1250\"></form>\
        <form action=\"12\"><input type=\"submit\" value=\"12 bits: 0.0625\"></form>";

char* webpageEnd   = "</body></html>";

/////////////////////////////////////////////////////////////////
// Helper Functions
/////////////////////////////////////////////////////////////////

// inString determines whether a given character sequence is in a char array request, returns 1 if present, -1 if not
int inString(char request[], char des[]) {
    if (strstr(request, des) != NULL) {return 1;}
    return -1;
}

int led_status = 0;

// updateLEDStatus controls the LED based on the request
int updateLEDStatus(char request[])
{
    // The request has been received. now process to determine whether to turn the LED on or off
    if (inString(request, "ledoff")==1) {
        digitalWrite(LED_PIN, PIO_LOW);
        led_status = 0;
        return led_status;    // Ensures LED status is accurate when bit resolution is selected
    }
    else if (inString(request, "ledon")==1) {
        digitalWrite(LED_PIN, PIO_HIGH);
        led_status = 1;
        return led_status;
    }
    return led_status;
}

// selectResolution returns the temperature resolution (8-12 bits) based on the user request
int selectResolution(char request[]) {
    if (inString(request, "8") == 1)   return 8;
    if (inString(request, "9") == 1)   return 9;
    if (inString(request, "10") == 1)  return 10;
    if (inString(request, "11") == 1)  return 11;
    if (inString(request, "12") == 1)  return 12;
    return 9; // default resolution
}

/////////////////////////////////////////////////////////////////
// Solution Functions
/////////////////////////////////////////////////////////////////

// Main function
int main(void) {
    configureFlash();
    configureClock();

    gpioEnable(GPIO_PORT_A);
    gpioEnable(GPIO_PORT_B);
    gpioEnable(GPIO_PORT_C);

    pinMode(LED_PIN, GPIO_OUTPUT);
    
    RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
    initTIM(TIM15);
    
    USART_TypeDef * USART = initUSART(USART1_ID, 125000);

    // TODO: Add SPI initialization code
    initSPI(0b111, 0, 1);             // 8-bit width, SPI Mode 0

    // Default DS1722 resolution 
    writeResolution(9);               // Initialize at 9-bit default per datasheet default

    while(1) {
        /* Wait for ESP8266 to send a request.
        Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
        Therefore the request[] array must be able to contain 18 characters.
        */

        // Receive web request from the ESP
        char request[BUFF_LEN] = "                  "; // initialize to known value
        int charIndex = 0;
      
        // Keep going until you get end of line character
        while(inString(request, "\n") == -1) {
            // Wait for a complete request to be transmitted before processing
            while(!(USART->ISR & USART_ISR_RXNE));
            request[charIndex++] = readChar(USART);
        }

        // Apply resolution helper function and write to DS1722
        int res = selectResolution(request);
        writeResolution(res);

        // Read temperature from DS1722 using SPI
        double tempC = readTemperature();      // COMMENT for negative temp verification

        // Verifying negative temperature range (-10.125°C) - Setting MSB and LSB according to datasheet
        /*
        uint8_t MSB = 0xF5;    
        uint8_t LSB = 0xE0;
        double tempC = calcTemperature(MSB, LSB);
        printf("DEBUG: Testing -10.125°C\n");
        printf("MSB = 0x%02X, LSB = 0x%02X -> Temperature = %.4f C\n", MSB, LSB, tempC);
        */

        // Update string with current LED state
        int led_status = updateLEDStatus(request);

        char ledStatusStr[20];
        if (led_status == 1)
            sprintf(ledStatusStr,"LED is on!");
        else if (led_status == 0)
            sprintf(ledStatusStr,"LED is off!");

        char tempStatusStr[64];
        sprintf(tempStatusStr, "Current Temperature: %.4f C | Resolution: %d-bit", tempC, res);

        // finally, transmit the webpage over UART
        sendString(USART, webpageStart);      // webpage header code
        sendString(USART, ledStr);            // button for controlling LED
        sendString(USART, tempStr);           // buttons for setting temperature resolution

        // LED STRING
        sendString(USART, "<h2>LED Status</h2>");
        sendString(USART, "<p>");
        sendString(USART, ledStatusStr);
        sendString(USART, "</p>");

        // TEMPERATURE STRING
        sendString(USART, "<h2>Temperature</h2>");
        sendString(USART, "<p>");
        sendString(USART, tempStatusStr);
        sendString(USART, "</p>");

        sendString(USART, webpageEnd);
    }
}
