#include "reg52.h"
#include <intrins.h>
#include "delay.h"
#include <stdio.h>
#include "delay.h"
#include "dht11.h"
#include "main.h"
#include "oled.h"
#include "uart.h"
#include <string.h>
#include "stepper.h"

/* DHT11 reading and node reporting interval. */
#define NODE_REPORT_INTERVAL_MS 2000U

/* UART packet buffer sent to the STM32 gateway. */
static char NodeTxBuffer[24];

/*
 * @brief Number of half steps used for one curtain movement.
 * @note 512 half steps is approximately one revolution for 28BYJ?48.
 * @note 28BYJ-48半步驱动模式，512个半步对应输出轴旋转完整1圈
 * @note 该宏定义窗帘执行一次开合动作的步进电机半步数
 */
#define CURTAIN_TRAVEL_STEPS 512U

/**
 * @brief 为协议载荷计算XOR异或校验和
 * @param text 去除帧头、帧尾标记之后的协议有效载荷数据字符串
 * @retval 返回计算得到的XOR校验字节
 */
static uchar Protocol_CalculateChecksum(const char *text)
{
	uchar checksum;                //保存XOR校验和结果
	uchar index;                   //数组遍历索引
	
	checksum = 0;
	index = 0;
	
	// 遍历字符串，直到遇到字符串结束符'\0'
	while(text[index] != '\0')
	{
		// 将字符强制转为无符号字节，逐字节做(异或)累积运算
		checksum ^= (uchar)text[index];
		index ++;
  }
	
	return checksum;               // 返回最终XOR校验和
}

/**
 * @brief 对载荷做XOR校验，并按照协议帧格式发送出去
 * @param None
 * @retval None
 */
static void Protocol_SendPayloadWithChecksum(void)
{
    uchar checksum;

    // 对 NodeTxBuffer 里面存放的原始有效载荷计算XOR校验和
    checksum = Protocol_CalculateChecksum(NodeTxBuffer);

    Send_UART_Byte('@');                 // 发送帧起始标记 @
    Send_UART_Str((uchar *)NodeTxBuffer);// 发送原始载荷字符串

    // 在缓冲区拼接校验字段与帧结束符 ,C=xx\r\n
    sprintf(NodeTxBuffer,
            ",C=%u\r\n",
            (uint)checksum);

    Send_UART_Str((uchar *)NodeTxBuffer);// 发送校验字段 + 帧结束 \r\n
}

/**
 * @brief Display fixed OLED labels.
 * @param None
 * @retval None
 */
static void App_ShowStaticText(void)
{
    OLED_Clear();

    OLED_ShowString(0, 0, (u8 *)"Temp:", 16);
    OLED_ShowString(0, 2, (u8 *)"Humi:", 16);
    OLED_ShowString(0, 4, (u8 *)"Status:", 16);
}

/**
 * @brief Display valid DHT11 data on the local OLED.
 * @param temperature Temperature in degrees Celsius.
 * @param humidity Relative humidity in percent.
 * @retval None
 */
static void App_ShowData(uchar temperature, uchar humidity)
{
    OLED_ShowNum(48, 0, temperature, 2, 16);
    OLED_ShowString(64, 0, (u8 *)"C", 16);

    OLED_ShowNum(48, 2, humidity, 2, 16);
    OLED_ShowString(64, 2, (u8 *)"%", 16);

    OLED_ShowString(64, 4, (u8 *)"OK    ", 16);
    OLED_ShowString(0, 6, (u8 *)"                ", 16);
}

/**
 * @brief Display and report a DHT11 error code.
 * @param status DHT11 status code.
 * @retval None
 */
static void App_ReportError(uchar status)
{
	OLED_ShowString(64, 4, (u8 *)"ERROR ", 16);
	OLED_ShowString(0, 6, (u8 *)"Code:", 16);
	OLED_ShowNum(48, 6, status, 2, 16);
	
	sprintf(NodeTxBuffer,"N1,ERROR=%u",(uint)status);

  Protocol_SendPayloadWithChecksum();
}

/**
 * @brief 发送带XOR校验的环境温湿度数据包
 * @param temperature 温度，单位：摄氏度
 * @param humidity    相对湿度，单位：%RH
 * @retval 无
 */
static void Node_SendEnvironmentPacket(uchar temperature, uchar humidity)
{
    // 将温湿度格式化写入发送缓冲区，仅填充业务载荷，不含帧头、校验、结尾
    sprintf(NodeTxBuffer,
            "N1,T=%u,H=%u",
            (uint)temperature,
            (uint)humidity);

    // 调用通用协议函数：自动计算XOR校验、添加@帧头、追加,C=xx\r\n并完成串口发送
    Protocol_SendPayloadWithChecksum();
}

/**
 * @brief Process validated alarm and curtain commands from the STM32 gateway.
 * @param None
 * @retval None
 * @note 处理来自STM32网关的窗帘、报警器串口指令
 * @note 通过strcmp完整比对接收缓冲区与拼装后的标准命令报文实现校验
 */
static void App_ProcessGatewayCommand(void)
{
    uchar expectedChecksum;   // 存储本地计算得到的预期校验和

    // 没有收到串口数据包，直接退出函数
    if (UART_RxFlag == 0)
    {
        return;
    }

    /*
     * @brief Validate and execute the curtain OPEN command.
     * @param None
     * @retval None
     * @note 窗帘打开指令：G1,CURTAIN=OPEN,C=校验和
     */
    // 计算"G1,CURTAIN=OPEN"报文对应的校验和
    expectedChecksum =
        Protocol_CalculateChecksum("G1,CURTAIN=OPEN");

    // 将命令+校验和拼装完整报文存入NodeTxBuffer缓冲区
    sprintf(NodeTxBuffer,
            "G1,CURTAIN=OPEN,C=%u",
            (uint)expectedChecksum);

    // 将收到的串口报文和拼装好的标准报文完整比对
    if (strcmp(UART_RxPacket, NodeTxBuffer) == 0)
    {
        // 电机正转，执行窗帘打开动作
        Stepper_RotateSteps(CURTAIN_TRAVEL_STEPS,
                            STEPPER_DIRECTION_FORWARD);
        Stepper_Stop();         // 动作完成，关闭步进电机线圈，释放锁止力矩

        OLED_ShowString(0, 6,
                        (u8 *)"Cmd:CURTAIN OPEN",
                        16); // OLED屏幕显示：窗帘打开命令

        UART_RxFlag = 0;        // 清除串口接收标志位
        return;                 // 命令处理完毕，退出
    }

    /*
     * @brief Validate and execute the curtain CLOSE command.
     * @param None
     * @retval None
     * @note 窗帘关闭指令：G1,CURTAIN=CLOSE,C=校验和
     */
    expectedChecksum =
        Protocol_CalculateChecksum("G1,CURTAIN=CLOSE");

    sprintf(NodeTxBuffer,
            "G1,CURTAIN=CLOSE,C=%u",
            (uint)expectedChecksum);

    if (strcmp(UART_RxPacket, NodeTxBuffer) == 0)
    {
        // 电机反转，执行窗帘关闭动作
        Stepper_RotateSteps(CURTAIN_TRAVEL_STEPS,
                            STEPPER_DIRECTION_REVERSE);
        Stepper_Stop();

        OLED_ShowString(0, 6,
                        (u8 *)"Cmd:CURTAIN CLS ",
                        16); // OLED屏幕显示：窗帘关闭命令

        UART_RxFlag = 0;
        return;
    }

    /*
     * @brief Validate and execute the alarm ON command.
     * @param None
     * @retval None
     * @note 报警器开启指令：G1,ALARM=1,C=校验和；LED低电平点亮，蜂鸣器低电平鸣响
     */
    expectedChecksum =
        Protocol_CalculateChecksum("G1,ALARM=1");

    sprintf(NodeTxBuffer,
            "G1,ALARM=1,C=%u",
            (uint)expectedChecksum);

    if (strcmp(UART_RxPacket, NodeTxBuffer) == 0)
    {
        LED1 = 0;   // 报警LED点亮
        BEEP = 0;   // 蜂鸣器开启报警

        OLED_ShowString(0, 6,
                        (u8 *)"Cmd:ALARM ON   ",
                        16); // OLED屏幕显示：报警开启

        UART_RxFlag = 0;
        return;
    }

    /*
     * @brief Validate and execute the alarm OFF command.
     * @param None
     * @retval None
     * @note 报警器关闭指令：G1,ALARM=0,C=校验和；LED高电平熄灭，蜂鸣器高电平关闭
     */
    expectedChecksum =
        Protocol_CalculateChecksum("G1,ALARM=0");

    sprintf(NodeTxBuffer,
            "G1,ALARM=0,C=%u",
            (uint)expectedChecksum);

    if (strcmp(UART_RxPacket, NodeTxBuffer) == 0)
    {
        LED1 = 1;   // 报警LED熄灭
        BEEP = 1;   // 蜂鸣器关闭

        OLED_ShowString(0, 6,
                        (u8 *)"Cmd:ALARM OFF  ",
                        16); // OLED屏幕显示：报警关闭

        UART_RxFlag = 0;
        return;
    }

    // 所有命令都不匹配，判定为无效指令
    OLED_ShowString(0, 6,
                    (u8 *)"Cmd:INVALID    ",
                    16);

    UART_RxFlag = 0;   // 清除接收标志，等待下一条指令
}


/**
 * @brief 51 remote-node application entry point.
 * @param None
 * @retval None
 */
void main(void)
{
	uchar temperature;
	uchar humidity;
	uchar status;

	OLED_Init();
	DHT11_Init();
	UART_Init();
	Stepper_Init();

	App_ShowStaticText();
	
	while (1)
	{
			status = DHT11_Read(&temperature, &humidity);

			if (status == DHT11_OK)
			{
					App_ShowData(temperature, humidity);
					Node_SendEnvironmentPacket(temperature, humidity);
			}
			else
			{
					App_ReportError(status);
			}
			
			/* 处理来自STM32网关下发的命令 */
			App_ProcessGatewayCommand();


			Delay_xms(NODE_REPORT_INTERVAL_MS);
	}
}
