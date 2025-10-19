// Sebastian Heredia
// dheredia@g.hmc.edu
// October 17, 2025

// main.c

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "DS1722.h"     // Explicitly include DS1722 SPI info

/////////////////////////////////////////////////////////////////
// Provided Constants and Functions
/////////////////////////////////////////////////////////////////

//Defining the web page in two chunks: everything before the current time, and everything after the current time
char* webpageStart = "<!DOCTYPE html><html><head><title>E155 Web Server Demo Webpage</title>\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	</head>\
	<body><h1>E155 Web Server Demo Webpage</h1>";
  
char* ledStr = "<p>LED Control:</p><form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
	<form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>";

char* tempStr = "<h1>Temperature</h1>\
  <p>Temperature Resolution:</p>\
  <form action=\"precision8\"><input type=\"submit\" value=\"8 bits\"></form>\
  <form action=\"precision9\"><input type=\"submit\" value=\"9 bits\"></form>\
  <form action=\"precision10\"><input type=\"submit\" value=\"10 bits\"></form>\
  <form action=\"precision11\"><input type=\"submit\" value=\"11 bits\"></form>\
  <form action=\"precision12\"><input type=\"submit\" value=\"12 bits\"></form>";

char* webpageEnd   = "</body></html>";

/////////////////////////////////////////////////////////////////
// Helper Functions
/////////////////////////////////////////////////////////////////

// inString determines whether a given character sequence is in a char array request, returns 1 if present, -1 if not
int inString(char request[], char des[]) {
	if (strstr(request, des) != NULL) {return 1;}
	return -1;
}

// updateLEDStatus controls the LED based on the request
int updateLEDStatus(char request[])
{
	int led_status = 0;
	// The request has been received. now process to determine whether to turn the LED on or off
	if (inString(request, "ledoff")==1) {
		digitalWrite(LED_PIN, PIO_LOW);
		led_status = 0;
	}
	else if (inString(request, "ledon")==1) {
		digitalWrite(LED_PIN, PIO_HIGH);
		led_status = 1;
	}

	return led_status;
}

// selectResolution selects the temperature resolution (8-12 bits) based on the user request
void selectResolution( char request[]) {
  int res = 0;

  if      (inString(request, "8") == 1)   res = 8;
  else if (inString(request, "9") == 1)   res = 9;
  else if (inString(request, "10") == 1)  res = 10;
  else if (inString(request, "11") == 1)  res = 11;
  else if (inString(request, "12") == 1)  res = 12;
  else res = 9;    // Default resolution from  DS1722 datasheet

  writeResolution(res);
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

  pinMode(LED_PIN, GPIO_OUTPUT);    // Recall: LED_PIN = PB3
  
  RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
  initTIM(TIM15);
  
  USART_TypeDef * USART = initUSART(USART1_ID, 125000);

  // TODO: Add SPI initialization code
  initSPI(0b111, 0, 1);             // 8-bit width, SPI Mode 0

  // Default DS1722 resolution 
  writeResolution(9);

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

    // TODO: Add SPI code here for reading temperature
    selectResolution(request);            // Apply resolution helper function
    float tempC = readTemperature();      // Read temperature from DS1722 using SPI

    // Update string with current LED state
    int led_status = updateLEDStatus(request);

    char ledStatusStr[20];
    if (led_status == 1)
      sprintf(ledStatusStr,"LED is on!");
    else if (led_status == 0)
      sprintf(ledStatusStr,"LED is off!");

    char tempStatusStr[32];
    sprintf(tempStatusStr, "Current Temperature: %.2f °C", tempC);  // Printing temperature reading (ºC)

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