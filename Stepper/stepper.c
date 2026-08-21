#include "reg52.h"
#include "delay.h"
#include "stepper.h"

// 步进电机控制引脚，ULN2003驱动板 IN1~IN4
sbit STEPPER_IN1 = P2^0;
sbit STEPPER_IN2 = P2^1;
sbit STEPPER_IN3 = P2^2;
sbit STEPPER_IN4 = P2^3;

static unsigned char stepperPhase;   // 电机当前相位索引 0~7

/* 28BYJ-48 + ULN2003 半步八拍序列表
 * 位定义：bit0=IN1，bit1=IN2，bit2=IN3，bit3=IN4
 * 半步步进模式，力矩更大，振动更小
 * code关键字：表格存储在ROM程序存储器，不占用RAM
 */
static unsigned char code StepperHalfStepTable[8] =
{
	0x01U, // 0: IN1 单独通电
	0x03U,  // 1: IN1+IN2 同时通电
	0x02U,  // 2: IN2 单独通电
	0x06U,  // 3: IN2+IN3 同时通电
	0x04U,  // 4: IN3 单独通电
	0x0CU,  // 5: IN3+IN4 同时通电
	0x08U,  // 6: IN4 单独通电
	0x09U,  // 7: IN4+IN1 同时通电
};

/**
 * @brief 输出半步相位控制电平到4路线圈
 * @param pattern 4bit线圈控制码，低4位有效
 * @retval 无
 */
static void Stepper_Output(unsigned char pattern)
{
	STEPPER_IN1 = (pattern & 0x01U) ? 1 : 0;  // bit0输出IN1
	STEPPER_IN2 = (pattern & 0x02U) ? 1 : 0;  // bit1输出IN2
	STEPPER_IN3 = (pattern & 0x04U) ? 1 : 0;  // bit2输出IN3
	STEPPER_IN4 = (pattern & 0x08U) ? 1 : 0;  // bit3输出IN4
}

/**
 * @brief 步进电机初始化
 * @param 无
 * @retval 无
 * @note 设置相位从0开始，关闭所有线圈，电机上电不锁死
 */
void Stepper_Init(void)
{
	stepperPhase = 0U;
	Stepper_Stop();
}

/**
 * @brief 电机转动指定半步数量
 * @param steps 需要转动的半步数；28BYJ?48半步模式一圈需要512步
 * @param direction STEPPER_DIRECTION_FORWARD正转 / STEPPER_DIRECTION_REVERSE反转
 * @retval 无
 * @note Delay_xms(3U) 控制转速；延时越小速度越快，过小会丢步
 */
void Stepper_RotateSteps(unsigned int steps, unsigned char direction)
{
	unsigned int stepIndex;
	
	// 循环执行对应步数
	for(stepIndex = 0U; stepIndex < steps; stepIndex ++)
	{
		Stepper_Output(StepperHalfStepTable[stepperPhase]);
		
		if(direction == STEPPER_DIRECTION_FORWARD)
		{
			// 正转：相位索引递增，到7之后回到0
			stepperPhase ++;
			if(stepperPhase >= 8U)
			{
				stepperPhase = 0U;
      }
    }
		else
		{
			// 反转：相位索引递减，到0跳回7
			if(stepperPhase == 0U)
			{
				stepperPhase = 7U;
      }
			else
			{
				stepperPhase --;
      }
    }
		Delay_xms(30);  // 每步保持延时，建立力矩
  }
}

/**
 * @brief 关闭电机全部线圈，电机断电，取消锁止力矩
 * @param 无
 * @retval 无
 * @note 调用Stop后电机可以手转动；想要保持锁止不要调用此函数
 */
void Stepper_Stop(void)
{
	STEPPER_IN1 = 0;
	STEPPER_IN2 = 0;
	STEPPER_IN3 = 0;
	STEPPER_IN4 = 0;
}

