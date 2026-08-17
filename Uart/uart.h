#ifndef _UART_H_
#define _UART_H_

#include "main.h"

/* 单条串口接收数据包最大长度 */
#define UART_RX_PACKET_SIZE 32U

extern char UART_RxPacket[UART_RX_PACKET_SIZE];
extern volatile uchar UART_RxFlag;

void Send_UART_Str(uchar *send_str);
void Send_UART_Byte(uchar send_byte);
void UART_Init(void);

#endif

