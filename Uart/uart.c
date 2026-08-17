#include "uart.h"

void Send_UART_Byte(unsigned char send_byte)
{
		SBUF =send_byte;
		while(TI == 0);
		TI = 0;
}

void Send_UART_Str(uchar *send_str)
{
	while(*send_str != '\0')
	{
	   Send_UART_Byte(*send_str++);
	}
}

void UART_Init(void)
{
		//初始化串口中断
	SCON = 0x50;
	PCON &= 0x7f;
	TMOD &= 0x0f;
	TMOD |= 0x20;
	TH1 = 0xFD;
	TL1 = 0xFD;
	TR1 = 1;
	ES = 1;
	EA = 1;
}
