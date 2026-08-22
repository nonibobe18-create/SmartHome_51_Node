#ifndef _STEPPER_H_
#define _STEPPER_H_

// 步进电机方向定义
#define STEPPER_DIRECTION_REVERSE 0U    // 电机反转
#define STEPPER_DIRECTION_FORWARD 1U    // 电机正转

/**
 * @brief 步进电机驱动引脚初始化
 * @param None 无输入参数
 * @retval None 无返回值
 * @note 初始化电机相位，关闭全部线圈，上电电机不保持锁止
 */
void Stepper_Init(void);

/**
 * @brief 启动非阻塞式步进电机运动
 * @param steps 半步步数
 * @param direction 旋转方向
 * @retval 无
 */
void Stepper_Start(unsigned int steps,unsigned char direction);

/**
 * @brief 执行一次步进电机控制周期（放在主循环/定时器中调用）
 * @param 无
 * @retval 无
 */
void Stepper_Task(void);
 
 /**
 * @brief 判断步进电机是否正在运转
 * @param 无
 * @retval 1：正在运动；0：停止
 */
unsigned char Stepper_IsBusy(void);

/**
 * @brief 关闭步进电机所有线圈输出
 * @param None 无输入参数
 * @retval None 无返回值
 * @note 断电后电机锁止力矩消失，可手动旋转电机轴
 */
void Stepper_Stop(void);

#endif