#include "App_Motor_Ctrl.h"

/* 2 * PI 的预计算值，避免每次计算都调用库函数 */
#define FOC_2PI  6.28318531f

/**
 * @brief 初始化电机控制器
 * 
 * @param ctrl 电机控制器句柄
 * @param v_bus 电机总电压 (V)
 * @param arr 电机PWM数组 (用于计算PWM占空比)
 */
 
void App_Motor_Init(Motor_Ctrl_t *ctrl, float v_bus, uint32_t arr) 
{
    // 1. 初始化 FOC 数学对象
    Alg_FOC_Init(&ctrl->MathParam, v_bus, arr);
    
    // 2. 设置默认运行模式和目标速度
    ctrl->Mode = MOTOR_MODE_OPEN_LOOP; // 默认先设为开环调试模式
    ctrl->Target_Speed = 0.0f;
}




/**
 * @brief 设置电机目标速度
 * 
 * @param ctrl 电机控制器句柄
 * @param speed 目标速度 (rad/s)
 */
void App_Motor_Set_Speed(Motor_Ctrl_t *ctrl, float speed)
{
    ctrl->Target_Speed = speed;
}



/**
 * @brief 运行电机控制器
 * 
 * @param ctrl 电机控制器句柄
 */
void App_Motor_Run_Task(Motor_Ctrl_t *ctrl)
{
    if(ctrl->Mode == MOTOR_MODE_OPEN_LOOP)
    {
        ctrl->MathParam.Theta += ctrl->Target_Speed;

        if(ctrl->MathParam.Theta > FOC_2PI)
        {
            ctrl->MathParam.Theta -= FOC_2PI; // 保持 Theta 在 0 ~ 2π 之间
        }
        else if(ctrl->MathParam.Theta < 0.0f)
        {
            ctrl->MathParam.Theta += FOC_2PI; // 保持 Theta 在 0 ~ 2π 之间
        }

        Alg_FOC_Step_OpenLoop(&ctrl->MathParam);
    }
    else if(ctrl->Mode == MOTOR_MODE_CLOSED_LOOP)
    {
        /* 预留：闭环行为 */
        // 1. 采集电流 ADC
        // 2. 采集编码器/观测器角度，赋值给 ctrl->MathParam.Theta
        // 3. 运行 PID 计算
        // 4. 调用 Alg_FOC_Step_ClosedLoop(&ctrl->MathParam);
    }
}
