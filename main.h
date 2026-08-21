#ifndef _MAIN_H_
#define _MAIN_H_
#include "reg52.h"


typedef unsigned int uint;
typedef unsigned char uchar;

sbit LED1 = P1^0;
sbit LED2 = P1^1;
//sbit LED3 = P1^2;
sbit LED4 = P1^3;
sbit BEEP = P1^6;

/* P2.0 to P2.3 are reserved for the stepper motor driver. */

sbit SUN = P3^7;

sbit KEY1 = P3^2;
sbit KEY2 = P3^3;

sbit IIC_SCL = P1^5;
sbit IIC_SDA = P1^4;


sbit DHT11_IO = P1^2;

#endif