#include "reg52.h"
#include <intrins.h>
#include "delay.h"
#include <stdio.h>
#include "delay.h"
#include "dht11.h"
#include "main.h"
#include "oled.h"
#include "uart.h"

/* DHT11 reading and node reporting interval. */
#define NODE_REPORT_INTERVAL_MS 2000U

/* UART packet buffer sent to the STM32 gateway. */
static char NodeTxBuffer[32];

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

    sprintf(NodeTxBuffer, "@N1,ERROR=%u\r\n", (uint)status);
    Send_UART_Str((uchar *)NodeTxBuffer);
}

/**
 * @brief Send valid environment data to the STM32 gateway.
 * @param temperature Temperature in degrees Celsius.
 * @param humidity Relative humidity in percent.
 * @retval None
 */
static void Node_SendEnvironmentPacket(uchar temperature, uchar humidity)
{
    sprintf(NodeTxBuffer,
            "@N1,T=%u,H=%u\r\n",
            (uint)temperature,
            (uint)humidity);

    Send_UART_Str((uchar *)NodeTxBuffer);
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

    /* The current node only sends data and does not receive commands yet. */
    ES = 0;

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

        Delay_xms(NODE_REPORT_INTERVAL_MS);
    }
}
