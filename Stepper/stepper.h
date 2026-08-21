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
 * @brief 控制步进电机转动指定半步数
 * @param steps 需要转动的半步数量；28BYJ?48半步模式完整一圈需要512个半步
 * @param direction 转动方向：STEPPER_DIRECTION_FORWARD正转 / STEPPER_DIRECTION_REVERSE反转
 * @retval None 无返回值
 */
void Stepper_RotateSteps(unsigned int steps, unsigned char direction);

/**
 * @brief 关闭步进电机所有线圈输出
 * @param None 无输入参数
 * @retval None 无返回值
 * @note 断电后电机锁止力矩消失，可手动旋转电机轴
 */
void Stepper_Stop(void);

#endif