#ifndef _UART_H_
#define _UART_H_
#include "main.h"


void Send_UART_Str(uchar *send_str);
void Send_UART_Byte(uchar send_byte);
void UART_Init(void);

#endif

