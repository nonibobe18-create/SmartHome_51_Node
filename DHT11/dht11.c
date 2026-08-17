#include "dht11.h"
#include "delay.h"

/* DHT11信号异常时防止死循环的超时计数阈值 */
#define DHT11_TIMEOUT_COUNT 10000U

/**
 * @brief  发送DHT11起始信号，并检测DHT11应答
 * @param  None
 * @retval DHT11状态码，DHT11_OK代表应答正常
 */
static uchar DHT11_Start(void)
{
    uint timeout = 0;

    DHT11_IO = 0;        // 主机拉低数据线，发送起始信号
    Delay_xms(20);       // 保持20ms，满足DHT11起始时序

    DHT11_IO = 1;        // 主机释放总线，拉高
    Delay30us();

    /*
     * 时序流程：等待DHT11应答拉低结束（等待变为高）
     * 应答脉冲：DHT11会先拉低总线，再拉高总线
     */
    while (DHT11_IO == 0)
    {
        timeout++;
        if (timeout >= DHT11_TIMEOUT_COUNT)
        {
            return DHT11_ERR_RESPONSE_HIGH;
        }
    }

    timeout = 0;

    /* 等待DHT11应答高电平结束，重新拉低，准备输出第一位数据 */
    while (DHT11_IO == 1)
    {
        timeout++;
        if (timeout >= DHT11_TIMEOUT_COUNT)
        {
            return DHT11_ERR_FIRST_BIT_LOW;
        }
    }

    return DHT11_OK;
}

/**
 * @brief  DHT11初始化，数据线置为释放高电平
 * @param  None
 * @retval None
 */
void DHT11_Init(void)
{
    DHT11_IO = 1;
}

/**
 * @brief  读取DHT11温湿度
 * @param  temperature：温度输出指针，单位℃
 * @param  humidity：湿度输出指针，单位%
 * @retval DHT11状态码
 */
uchar DHT11_Read(uchar *temperature, uchar *humidity)
{
    uchar dhtData[5] = {0};   // DHT11返回5字节数据缓冲区
    uchar byteIndex;          // 字节索引0?4，共5字节
    uchar bitIndex;           // 位索引0?7，1字节8bit
    uchar bitValue;           // 保存当前读到bit的值0/1
    uchar dataByte;           // 保存正在接收的单个字节
    uint timeout;             // 超时计数器
    uint checksum;            // 校验和

    // 发送起始信号，检测应答
    if (DHT11_Start() != DHT11_OK)
    {
        return DHT11_ERR_RESPONSE_HIGH;
    }

    for (byteIndex = 0; byteIndex < 5; byteIndex++)
    {
        /* 接收每一个字节前清零 */
        dataByte = 0;

        for (bitIndex = 0; bitIndex < 8; bitIndex++)
        {
            timeout = 0;

            /* 等待bit的低电平周期结束，等待上升沿 */
            while (DHT11_IO == 0)
            {
                timeout++;
                if (timeout >= DHT11_TIMEOUT_COUNT)
                {
                    return DHT11_ERR_BIT_RISE;
                }
            }

            /*
             * 延时约30us之后采样电平：
             * 低电平代表bit0；高电平代表bit1
             */
            Delay30us();

            if (DHT11_IO == 1)
            {
                bitValue = 1;

                timeout = 0;
                /*
                 * 只有bit=1时，需要等待高电平结束（下降沿）
                 * bit0高电平很短，读完直接进入下一位低电平，不需要额外等待
                 */
                while (DHT11_IO == 1)
                {
                    timeout++;
                    if (timeout >= DHT11_TIMEOUT_COUNT)
                    {
                        return DHT11_ERR_BIT_FALL;
                    }
                }
            }
            else
            {
                bitValue = 0;
            }

            dataByte <<= 1;        // 字节左移，腾出最低位
            dataByte |= bitValue;  // 将当前bit写入最低位
        }

        dhtData[byteIndex] = dataByte;
    }

    // 计算校验和：前4字节相加
    checksum = dhtData[0] + dhtData[1] +
               dhtData[2] + dhtData[3];

    // 强制转为uchar，和接收的校验字节对比
    if ((uchar)checksum != dhtData[4])
    {
        return DHT11_ERR_CHECKSUM;
    }

    *humidity = dhtData[0];     //湿度整数
    *temperature = dhtData[2];  //温度整数

    return DHT11_OK;
}
