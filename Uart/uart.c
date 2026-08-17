#include "uart.h"

/* 存储从STM32网关收到的一整包完整数据 */
char UART_RxPacket[UART_RX_PACKET_SIZE];

/* 收到完整数据包后置1 */
volatile uchar UART_RxFlag = 0;

/* 串口发送中断使用，标记发送是否正在进行 */
static volatile uchar UART_TxBusy = 0;

/**
 * @brief 51单片机串口发送单个字节
 * @param send_byte 需要发送的字节
 * @retval None
 */
void Send_UART_Byte(unsigned char send_byte)
{
	UART_TxBusy = 1;
	
	SBUF = send_byte;
	
	/* 等待串口发送中断通知字节发送完成 */
	while(UART_TxBusy == 1);
	
}

/**
 * @brief 发送以'\0'结尾的字符串
 * @param send_str 待发送字符串
 * @retval None
 */
void Send_UART_Str(uchar *send_str)
{
	while(*send_str != '\0')
	{
	  Send_UART_Byte(*send_str);
		send_str ++;
	}
}

/**
 * @brief 串口初始化：模式1，波特率9600
 * @param None
 * @retval None
 */
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
	
	UART_RxPacket[0] = '\0';
	UART_RxFlag = 0;
	
	ES = 1;
	EA = 1;
}

/**
 * @brief 51单片机串口中断服务函数
 * @param None
 * @retval None
 */
void UART_ISR(void) interrupt 4
{
	static uchar rxState = 0;
	static uchar packetIndex = 0;
	uchar rxData;
	
	if(RI == 1)
	{
		/* 读取接收字节，清除接收中断标志RI */
		rxData = SBUF;
		RI = 0;
		
		if(rxState == 0)
		{
			/* 等待帧起始字符 '@' */
			if((rxData == '@') && (UART_RxFlag == 0))
			{
				rxState = 1;
				packetIndex = 0;
      }
		}
		else if(rxState == 1)
		{
			/* 接收报文主体，直到收到 '\r' */
			if(rxData == '\r')
			{
				rxState = 2;
      }
			 else if(packetIndex < (UART_RX_PACKET_SIZE - 1U))
			 {
				 UART_RxPacket[packetIndex] = rxData;
				 packetIndex ++;
       }
			 else
			 {
				 /* 数据包超长，丢弃本帧 */
				 rxState = 0;
				 packetIndex = 0;
       }
     }
		 else
		 {
			 /* 校验帧结束符 '\n' */
			 if(rxData == '\n')
			 {
				 UART_RxPacket[packetIndex] = '\0';
				 UART_RxFlag = 1;
       }
			 
			 rxState = 0;
			 packetIndex = 0;
     }
  }
	
	if(TI == 1)
	{
		/* 单字节发送完成，清除发送标志，释放阻塞的发送函数 */
		TI = 0;
		UART_TxBusy =0;
  }
}
