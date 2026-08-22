#include "reg52.h"
#include "delay.h"
#include "stepper.h"

// 步进电机控制引脚，ULN2003驱动板 IN1~IN4
sbit STEPPER_IN1 = P2^0;
sbit STEPPER_IN2 = P2^1;
sbit STEPPER_IN3 = P2^2;
sbit STEPPER_IN4 = P2^3;

static unsigned char stepperPhase;         // 半步相位索引 0~7
static unsigned char stepperDirection;     // 电机转动方向
static unsigned char stepperBusy;          // 忙标志：1正在运动，0空闲
static unsigned int stepperRemainingSteps;// 剩余需要走的半步步数

// 半步通电时序表（28BYJ-48 8拍半步模式）
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
 * @brief 输出一组半步线圈驱动电平
 * @param pattern 4位绕组控制码
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
 * @brief 初始化步进电机驱动
 * @param 无
 * @retval 无
 */
//相位从0开始,默认正转,初始状态空闲,剩余步数清零,关闭所有线圈输出
void Stepper_Init(void)
{
	stepperPhase = 0U;
	stepperDirection = STEPPER_DIRECTION_FORWARD;
	stepperBusy = 0U;
	stepperRemainingSteps = 0U;
	Stepper_Stop();
}

/**
 * @brief 启动非阻塞步进电机运动
 * @param steps 需要走的半步步数
 * @param direction 转动方向：STEPPER_DIRECTION_FORWARD正转 / STEPPER_DIRECTION_REVERSE反转
 * @retval 无
 * @note 若电机正在忙或者步数为0，直接忽略本次请求；函数立刻返回不会阻塞
 */
void Stepper_Start(unsigned int steps,unsigned char direction)
{
	if((steps == 0U) || (stepperBusy != 0U))
	{
		return;
  }
	
	stepperDirection = direction;
	stepperRemainingSteps = steps;
	stepperBusy = 1U;                    //置位标志，开始运行
}

/**
 * @brief 执行一次半步控制周期，必须定时循环调用
 * @param 无
 * @retval 无
 * @note 建议放在定时器中断或者主while循环，控制调用间隔来调节转速；每调用一次走1个半步
 */
void Stepper_Task(void)
{
	if(stepperBusy == 0U)
	{
		return;                         //电机空闲，直接退出，不做处理
  }
	
	//根据当前相位输出绕组电平
	Stepper_Output(StepperHalfStepTable[stepperPhase]);
	
	//根据方向更新相位索引
	if(stepperDirection == STEPPER_DIRECTION_FORWARD)
	{
		stepperPhase ++;
		if(stepperPhase >= 8U)
		{
			stepperPhase = 0U;           //8拍到头回到0
    }
  }
	else
	{
		if(stepperPhase == 0U)
		{
			stepperPhase = 7U;           //反向，0相位跳会7
    }
		else
		{
			stepperPhase --;
    }
  }
	
	stepperRemainingSteps --;        //剩余步数减1
	
	//步数走完，停止电机释放线圈
	if(stepperRemainingSteps == 0U)
	{
		Stepper_Stop();
  }
}

/**
 * @brief 查询电机是否正在运动
 * @param 无
 * @retval 1：正在运动；0：空闲停止
 */
unsigned char Stepper_IsBusy(void)
{
	return stepperBusy;
}

/**
 * @brief 停止步进电机，切断全部绕组电流，电机松锁
 * @param 无
 * @retval 无
 */
void Stepper_Stop(void)
{
	STEPPER_IN1 = 0;
	STEPPER_IN2 = 0;
	STEPPER_IN3 = 0;
	STEPPER_IN4 = 0;
	
	stepperBusy = 0U;
	stepperRemainingSteps = 0U;
}

