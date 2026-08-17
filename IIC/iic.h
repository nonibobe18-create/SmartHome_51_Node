#ifndef _IIC_H_
#define _IIC_H_

void IIC_Start(void);
void IIC_Stop(void);
void IIC_Send_Byte(unsigned char IIC_Byte);
unsigned char IIC_Recv_Ack(void);


#endif 