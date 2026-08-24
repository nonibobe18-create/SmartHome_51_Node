# SmartHome 51 Node

## 项目简介

STC89C52 作为智能家居环境监测节点，负责：

- 读取 DHT11 温湿度；
- OLED 显示本地温湿度和状态；
- 通过 UART 向 STM32 网关发送环境数据；
- 接收 STM32 的温度和烟雾报警命令；
- 控制 LED 和蜂鸣器；
- 控制步进电机实现窗帘 OPEN、CLOSE、STOP；
- 使用 KEY1、KEY2 实现窗帘开关限位保护。

项目只使用一个 51 节点。

## 51 引脚连接

| 引脚 | 功能 | 状态 |
|---|---|---|
| P1.0 | LED1 报警指示灯 | 已使用 |
| P1.2 | DHT11 数据线 | 已使用 |
| P1.4 | OLED SDA | 已使用 |
| P1.5 | OLED SCL | 已使用 |
| P1.6 | 蜂鸣器 BEEP | 已使用 |
| P2.0 | ULN2003 IN1 | 已使用 |
| P2.1 | ULN2003 IN2 | 已使用 |
| P2.2 | ULN2003 IN3 | 已使用 |
| P2.3 | ULN2003 IN4 | 已使用 |
| P3.0 | UART RXD，连接 STM32 PA9 | 已使用 |
| P3.1 | UART TXD，连接 STM32 PA10 | 已使用 |
| P3.2 | KEY1，开限位 | 已使用 |
| P3.3 | KEY2，关限位 | 已使用 |

## 51 与 STM32 串口连接

```text
51 P3.1/TXD -> STM32 PA10/RX
51 P3.0/RXD <- STM32 PA9/TX
51 GND      -> STM32 GND

```

## 串口参数

```text
波特率：9600
数据位：8
停止位：1
校验位：None
```

## 窗帘控制命令

```text
@G1,CURTAIN=OPEN,C=校验值\r\n
@G1,CURTAIN=CLOSE,C=校验值\r\n
@G1,CURTAIN=STOP,C=41\r\n
```

## 限位保护

- KEY1 为开限位；
- KEY2 为关限位；
- 限位信号连续稳定检测后停止电机；
- OPEN、CLOSE、STOP 命令均已验证；
- 步进电机运行过程中发送 STOP 可立即停止。

## 已完成测试

- DHT11 读取正常；
- OLED 显示正常；
- UART 双向通信正常；
- LED 和蜂鸣器报警正常；
- 步进电机正转、反转、停止正常；
- KEY1、KEY2 限位停止正常；
- Keil 编译通过。
