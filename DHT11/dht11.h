#ifndef _DHT_H_
#define _DHT_H_
#include "main.h"

#define DHT11_OK                 0U
#define DHT11_ERR_RESPONSE_LOW   1U
#define DHT11_ERR_RESPONSE_HIGH  2U
#define DHT11_ERR_FIRST_BIT_LOW  3U
#define DHT11_ERR_BIT_RISE       4U
#define DHT11_ERR_BIT_FALL       5U
#define DHT11_ERR_CHECKSUM       6U

uchar DHT11_Start(void);
uchar DHT11_Read(unsigned char *humi,unsigned char *temp);


#endif 