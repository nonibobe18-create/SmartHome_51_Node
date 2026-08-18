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

/* DHT11 reading and node reporting interval. */
#define NODE_REPORT_INTERVAL_MS 2000U

/* UART packet buffer sent to the STM32 gateway. */
static char NodeTxBuffer[24];

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
 * @brief 解析处理来自STM32网关下发的报警控制命令
 * @param None
 * @retval None
 */
static void App_ProcessGatewayCommand(void)
{
    unsigned int commandValue;   // 解析得到ALARM命令值 0/1
    unsigned int receivedChecksum; // 报文中收到的校验码
    uchar expectedChecksum;      // 本地计算出来的预期XOR校验和
    int parseResult;             // sscanf解析返回成功字段个数

    // 没有收到完整一帧，直接返回
    if (UART_RxFlag == 0)
    {
        return;
    }

    /*
     * UART_RxPacket：已经剥离帧头@、帧结束\r\n，报文样例："G1,ALARM=1,C=86"
     * sscanf解析出ALARM数值 和 C后面收到的校验码
     */
    parseResult = sscanf(UART_RxPacket,
                         "G1,ALARM=%u,C=%u",
                         &commandValue,
                         &receivedChecksum);

    // 解析字段不足2个 / 命令值不是0或1 / 收到校验超过1字节范围 → 非法报文
    if ((parseResult != 2) ||
        (commandValue > 1U) ||
        (receivedChecksum > 255U))
    {
        OLED_ShowString(0, 6, (u8 *)"Cmd:INVALID    ", 16);
    }
    else if (commandValue == 1U)
    {
        // 本地对原始载荷 "G1,ALARM=1" 计算XOR校验
        expectedChecksum = Protocol_CalculateChecksum("G1,ALARM=1");

        // 校验比对成功，执行报警开启
        if (receivedChecksum == expectedChecksum)
        {
            LED1 = 0;
            BEEP = 0;
            OLED_ShowString(0, 6, (u8 *)"Cmd:ALARM ON   ", 16);
        }
        else
        {
            OLED_ShowString(0, 6, (u8 *)"Cmd:INVALID    ", 16);
        }
    }
    else
    {
        // commandValue == 0，计算载荷"G1,ALARM=0"预期校验
        expectedChecksum = Protocol_CalculateChecksum("G1,ALARM=0");

        // 校验比对成功，关闭报警
        if (receivedChecksum == expectedChecksum)
        {
            LED1 = 1;
            BEEP = 1;
            OLED_ShowString(0, 6, (u8 *)"Cmd:ALARM OFF  ", 16);
        }
        else
        {
            OLED_ShowString(0, 6, (u8 *)"Cmd:INVALID    ", 16);
        }
    }

    UART_RxFlag = 0; // 清除接收完成标志，等待下一帧报文
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
