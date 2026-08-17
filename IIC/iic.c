#include "iic.h"
#include "main.h"
#include "delay.h"

void IIC_Start(void)
{
	IIC_SCL = 1;
	IIC_SDA = 1;
	Delay5us();
	IIC_SDA = 0;
	Delay5us();
	IIC_SCL = 0;
	
}

void IIC_Stop(void)
{
	IIC_SCL = 1;
	IIC_SDA = 0;
	Delay5us();
	IIC_SDA = 1;
	
}

void IIC_Send_Byte(unsigned char IIC_Byte)
{
	char i;
	for(i = 0;i<8;i++)
	{
		IIC_SDA = IIC_Byte & (0x80 >> i);
		IIC_SCL = 1;
		Delay5us();
		IIC_SCL = 0;
		Delay5us();
  }
	
}

unsigned char IIC_Recv_Ack(void)
{
	unsigned char ack;
	IIC_SDA = 1;
	IIC_SCL = 1;
	Delay5us();
	ack = IIC_SDA;
	IIC_SCL = 0;
	Delay5us();
	return ack;
	
}

