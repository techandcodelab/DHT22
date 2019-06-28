#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>
#include "delay.h"
#include "dht22.h"
#include "stm32f10x.h"


uint16_t bits[40];

uint8_t  hMSB = 0;
uint8_t  hLSB = 0;
uint8_t  tMSB = 0;
uint8_t  tLSB = 0;
uint8_t  parity_rcv = 0;


uint16_t bits_out[40];

uint8_t  hMSB_out = 0;
uint8_t  hLSB_out = 0;
uint8_t  tMSB_out = 0;
uint8_t  tLSB_out = 0;
uint8_t  parity_rcv_out = 0;
static GPIO_InitTypeDef PORT;
static GPIO_InitTypeDef PORT_out;


void DHT22_Init(void) {
	RCC_APB2PeriphClockCmd(DHT22_GPIO_CLOCK,ENABLE);
	PORT.GPIO_Mode = GPIO_Mode_Out_PP;
	PORT.GPIO_Pin = DHT22_GPIO_PIN;
	PORT.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DHT22_GPIO_PORT,&PORT);
}
void DHT22_Init_outside(void) {
	RCC_APB2PeriphClockCmd(DHT22_GPIO_CLOCK,ENABLE);
	PORT_out.GPIO_Mode = GPIO_Mode_Out_PP;
	PORT_out.GPIO_Pin = DHT22_GPIO_PIN_OUTSIDE;
	PORT_out.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DHT22_GPIO_PORT,&PORT_out);
}
uint32_t DHT22_GetReadings(void) {
	uint32_t wait;
	uint8_t i;

	// Generate start impulse for sensor
	DHT22_GPIO_PORT->BRR = DHT22_GPIO_PIN; // Pull down SDA (Bit_SET)
	DelayMs(2); // Host start signal at least 800us
	DHT22_GPIO_PORT->BSRR = DHT22_GPIO_PIN; // Release SDA (Bit_RESET)

	// Switch pin to input with Pull-Up
	PORT.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(DHT22_GPIO_PORT,&PORT);

	// Wait for AM2302 to start communicate
	wait = 0;

	while ((DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN) && (wait++ < 200)) DelayUs(2);
	if (wait > 50) return DHT22_RCV_NO_RESPONSE;

	// Check ACK strobe from sensor
	wait = 0;
	while (!(DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN) && (wait++ < 100)) DelayUs(1);
	if ((wait < 8) || (wait > 15)) return DHT22_RCV_BAD_ACK1;

	wait = 0;
	while ((DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN) && (wait++ < 100)) DelayUs(1);
	if ((wait < 8) || (wait > 15)) return DHT22_RCV_BAD_ACK2;

	// ACK strobe received --> receive 40 bits
	i = 0;
	while (i < 40) {
		// Measure bit start impulse (T_low = 50us)
		wait = 0;
		while (!(DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN) && (wait++ < 20)) DelayUs(1);
		if (wait > 16) {
			// invalid bit start impulse length
			bits[i] = 0xffff;
			while ((DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN) && (wait++ < 20)) DelayUs(1);
		} else {
			// Measure bit impulse length (T_h0 = 25us, T_h1 = 70us)
			wait = 0;
			while ((DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN) && (wait++ < 20)) DelayUs(1);
			bits[i] = (wait < 16) ? wait : 0xffff;
		}

		i++;
	}

	for (i = 0; i < 40; i++) if (bits[i] == 0xffff) return DHT22_RCV_RCV_TIMEOUT;

	return DHT22_RCV_OK;
}

uint32_t DHT22_GetReadings_outside(void) {
	uint32_t wait;
	uint8_t i;

	// Generate start impulse for sensor
	DHT22_GPIO_PORT->BRR = DHT22_GPIO_PIN_OUTSIDE; // Pull down SDA (Bit_SET)
	DelayMs(2); // Host start signal at least 800us
	DHT22_GPIO_PORT->BSRR = DHT22_GPIO_PIN_OUTSIDE; // Release SDA (Bit_RESET)

	// Switch pin to input with Pull-Up
	PORT_out.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(DHT22_GPIO_PORT,&PORT_out);

	// Wait for AM2302 to start communicate
	wait = 0;

	while ((DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN_OUTSIDE) && (wait++ < 200)) DelayUs(2);
	if (wait > 50) return DHT22_RCV_NO_RESPONSE;

	// Check ACK strobe from sensor
	wait = 0;
	while (!(DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN_OUTSIDE) && (wait++ < 100)) DelayUs(1);
	if ((wait < 8) || (wait > 15)) return DHT22_RCV_BAD_ACK1;

	wait = 0;
	while ((DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN_OUTSIDE) && (wait++ < 100)) DelayUs(1);
	if ((wait < 8) || (wait > 15)) return DHT22_RCV_BAD_ACK2;

	// ACK strobe received --> receive 40 bits
	i = 0;
	while (i < 40) {
		// Measure bit start impulse (T_low = 50us)
		wait = 0;
		while (!(DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN_OUTSIDE) && (wait++ < 20)) DelayUs(1);
		if (wait > 16) {
			// invalid bit start impulse length
			bits_out[i] = 0xffff;
			while ((DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN_OUTSIDE) && (wait++ < 20)) DelayUs(1);
		} else {
			// Measure bit impulse length (T_h0 = 25us, T_h1 = 70us)
			wait = 0;
			while ((DHT22_GPIO_PORT->IDR & DHT22_GPIO_PIN_OUTSIDE) && (wait++ < 20)) DelayUs(1);
			bits_out[i] = (wait < 16) ? wait : 0xffff;
		}

		i++;
	}

	for (i = 0; i < 40; i++) if (bits_out[i] == 0xffff) return DHT22_RCV_RCV_TIMEOUT;

	return DHT22_RCV_OK;
}

uint16_t DHT22_DecodeReadings(void) {
	uint8_t parity;
	uint8_t  i = 0;

	hMSB = 0;
	for (; i < 8; i++) {
		hMSB <<= 1;
		if (bits[i] > 7) hMSB |= 1;
	}
	hLSB = 0;
	for (; i < 16; i++) {
		hLSB <<= 1;
		if (bits[i] > 7) hLSB |= 1;
	}
	tMSB = 0;
	for (; i < 24; i++) {
		tMSB <<= 1;
		if (bits[i] > 7) tMSB |= 1;
	}
	tLSB = 0;
	for (; i < 32; i++) {
		tLSB <<= 1;
		if (bits[i] > 7) tLSB |= 1;
	}
	for (; i < 40; i++) {
		parity_rcv <<= 1;
		if (bits[i] > 7) parity_rcv |= 1;
	}

	parity  = hMSB + hLSB + tMSB + tLSB;

	return (parity_rcv << 8) | parity;
}


uint16_t DHT22_GetHumidity(void) {
	return (hMSB << 8) + hLSB;
}

uint16_t DHT22_GetTemperature(void) {
	return (tMSB << 8) + tLSB;
}


uint16_t DHT22_DecodeReadings_outside(void) {
	uint8_t parity2;
	uint8_t  i = 0;

	hMSB_out = 0;
	for (; i < 8; i++) {
		hMSB_out <<= 1;
		if (bits_out[i] > 7) hMSB_out |= 1;
	}
	hLSB_out = 0;
	for (; i < 16; i++) {
		hLSB_out <<= 1;
		if (bits_out[i] > 7) hLSB_out |= 1;
	}
	tMSB_out = 0;
	for (; i < 24; i++) {
		tMSB_out <<= 1;
		if (bits_out[i] > 7) tMSB_out |= 1;
	}
	tLSB_out = 0;
	for (; i < 32; i++) {
		tLSB_out <<= 1;
		if (bits_out[i] > 7) tLSB_out |= 1;
	}
	for (; i < 40; i++) {
		parity_rcv_out <<= 1;
		if (bits_out[i] > 7) parity_rcv_out |= 1;
	}

	parity2  = hMSB_out + hLSB_out + tMSB_out + tLSB_out;

	return (parity_rcv_out << 8) | parity2;
}

uint16_t DHT22_GetHumidity_outside(void) {
	return (hMSB_out << 8) + hLSB_out;
}

uint16_t DHT22_GetTemperature_outside(void) {
	return (tMSB_out << 8) + tLSB_out;
}
