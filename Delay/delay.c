#include "delay.h"
#include <intrins.h>

void Delay_xms(uint xms)	//@11.0592MHz
{
	unsigned char data i, j;
  while(xms)
	{
		_nop_();
	i = 2;
	j = 199;
	do
	{
		while (--j);
	} while (--i);
	xms--;
  }
	
}
void Delay5us(void)	//@11.0592MHz
{
}

void Delay30us(void)	//@11.0592MHz
{
	unsigned char data i;

	i = 11;
	while (--i);
}

