\# SmartHome 51 Node



\## Project Description



The STC89C52 works as the sensor node of the smart-home project.



It reads DHT11 temperature and humidity data, displays local data on OLED, and sends environment packets to the STM32 gateway through UART.



\## Hardware



\- STC89C52

\- DHT11

\- OLED

\- LED

\- Buzzer



\## Pin Definition



```text

P1.0  LED1

P1.2  DHT11

P1.4  OLED SDA

P1.5  OLED SCL

P1.6  BEEP

P3.0  UART RXD

P3.1  UART TXD

