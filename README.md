# DHT22
DHT22 Sensor code for STM32F100RB &amp; Serial USART 
Hello Everyone

in this small project we have 7 files
1-main.c
2-dht22.c  3-dht22.h
4-delay.c  5-delay.h
6-serial_uart.c 7-serial_uart.h // FOR PUTTY SERIAL OUTPUT 

starting with the main file

in line 22 we are init the uart to be able to use the serial
using rs232 as shown in the begining of this video

then the delay init which it's going to be used inside the dht22 file
the delay helps us to send an revice the wanted data from the sensor
sice we only have one pin that transmit the data from the sensor

every time we need to read data from the sensor we have to init the dht22

if you are looking to have one more sensor you need to copy the init function and rename it

and you have to copy all of the other functions to be able to read from two differnet sensors

remeber to change the pin the dht22_init 

DHT22_GPIO_PIN is PB0 AND DHT22_GPIO_PIN_OUTSIDE IS PB1

WITH DHT22_Init AND DHT22_Init_outside you can have two sensors in the same file 

for the serial file you can use any usart port that you need 

look inside serial_uart.h

I'm using port 3 which is PIN PB10 AND PB11 

to change it just change line 1 of the code as shown in the left side



this code can run two sensors in same time using port B:
PB0 -- 1ST SENSOR

PB1 --2ND SENSOR





























