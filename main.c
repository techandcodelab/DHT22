// DHT22 sensor
// tech and code lab 
//PB0 SENSOR INPUT 1
//PB1 SENSOR INPUT 2
//pin PB10 IS THE USART OUTPUT TX TO RS232

#include "stm32f10x.h"


#include "delay.h"
#include "dht22.h"
#include "serial_uart.h"



#include <stdio.h>


uint32_t response;
uint16_t humidity,temperature;
int main (void) { 
	UART_Init();
	//DHT22_Init_outside();
	DelayInit();

	while (1){

	DHT22_Init();

		response = DHT22_GetReadings();
					//	UART_SendStr("TEST");

		if (response != DHT22_RCV_OK) {
			UART_SendStr("DHT22_GetReadings() error = ");
			UART_SendInt(response); UART_SendChar('\n');
		} else {
			response = DHT22_DecodeReadings();
			UART_SendChar('\r');

			UART_SendChar('\n');
			if ((response & 0xff) != (response >> 8)) {
				UART_SendStr("Wrong data received.\n");
			} else {
				temperature = DHT22_GetTemperature();
				humidity = DHT22_GetHumidity();
				UART_SendChar('\r');
				UART_SendStr("Humidity: ");
				UART_SendInt(humidity / 10); UART_SendChar('.');
				UART_SendInt(humidity % 10); UART_SendStr("%RH");
				UART_SendChar('\n');
				UART_SendChar('\r');

				UART_SendStr("Temperature: ");
				if ((temperature & 0x8000) != 0) UART_SendChar('-');
				UART_SendInt((temperature & 0x7fff) / 10); UART_SendChar('.');
				UART_SendInt((temperature & 0x7fff) % 10); UART_SendStr("C");
				UART_SendChar('\n');
			}
		}

		DelayMs(1000);
}
}
