#ifndef __APP_MOTOR_CTRL_H__
#define __APP_MOTOR_CTRL_H__


#include "Alg_FOC_Math.h"

/**
 * @brief  电机运行模式枚举
 * @note    定义了电机的三种运行模式：停机、开环强拖、闭环。
 */
typedef enum {
    MOTOR_MODE_STOP = 0,    // 停机模式
    MOTOR_MODE_OPEN_LOOP,   // 开环强拖模式
    MOTOR_MODE_CLOSED_LOOP  // 闭环模式 (预留)
} Motor_Mode_e;


// 电机控制器句柄 (将数学对象和控制状态打包)
typedef struct {
    FOC_Motor_t MathParam;     // FOC 底层数学计算对象 (组合模式)
    
    Motor_Mode_e Mode;         // 当前运行模式
    float Target_Speed;        // 目标转速 (开环步长或闭环目标速度)
    
    // 定时器硬件相关 (用于底层更新)
    // TIM_HandleTypeDef *htim; // 如果未来你想在这个文件里直接更新PWM，可以把硬件句柄传进来
} Motor_Ctrl_t;





/**
 * @brief 初始化电机控制器
 * 
 * @param ctrl 电机控制器句柄
 * @param v_bus 电机总电压 (V)
 * @param arr 电机PWM数组 (用于计算PWM占空比)
 */
 
void App_Motor_Init(Motor_Ctrl_t *ctrl, float v_bus, uint32_t arr);





/**
 * @brief 设置电机目标速度
 * 
 * @param ctrl 电机控制器句柄
 * @param speed 目标速度 (rad/s)
 */
void App_Motor_Set_Speed(Motor_Ctrl_t *ctrl, float speed);




/**
 * @brief 运行电机控制器
 * 
 * @param ctrl 电机控制器句柄
 */
void App_Motor_Run_Task(Motor_Ctrl_t *ctrl);


#endif
