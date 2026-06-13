/**
  ******************************************************************************
  * @file    Alg_FOC_Math.c
  * @brief   FOC 核心数学转换算法库实现
  ******************************************************************************
  */
#include "Alg_FOC_Math.h"


/**
 * @brief  初始化 FOC 电机对象
 * @param  motor: 电机对象指针
 * @param  v_bus: 电池母线电压 (例如 12V 电池则传入 12.0f)
 * @param  arr:   PWM 定时器的自动重装载值 (决定了 100% 占空比对应的数值)
 * @retval 无
 */
void Alg_FOC_Init(FOC_Motor_t *motor, float v_bus, uint32_t arr) 
{
    // 清空内存
    // memset(motor, 0, sizeof(FOC_Motor_t)); 
    // 1. 初始化基础配置参数
    motor->V_Bus = v_bus;
    motor->PWM_ARR = arr;

    // 2. 初始化状态/中间变量 
    // (开环下 Theta 是个不断累加的中间状态，闭环下它是从传感器读来的反馈)
    motor->Theta = 0.0f;

    // 3. 初始化控制目标变量 (你想让电机出多大力)
    motor->V_d_target = 0.0f;
    motor->V_q_target = 0.0f;
}




/**
 * @brief  Clarke 变换 (3相电流 -> 2相静止正交电流)
 * @note   利用基尔霍夫电流定律 Ia + Ib + Ic = 0，省去了对 Ic 的采样。
 * 公式：I_alpha = I_a;  I_beta = (I_a + 2*I_b) / sqrt(3)
 * @param  motor: 电机对象指针
 */
void Alg_FOC_Clarke(FOC_Motor_t *motor) 
{
    motor->I_alpha = motor->I_a;
    // 使用预先算好的倒数 FOC_INV_SQRT3，把除法变成乘法，提速！
    motor->I_beta  = FOC_INV_SQRT3 * (motor->I_a + 2.0f * motor->I_b);
}




/**
 * @brief  Park 变换 (2相静止 -> 2相旋转)
 * @note   将交流量 (I_alpha, I_beta) 变成相对于转子静止的直流量 (I_d, I_q)。
 * 这是闭环 PID 能否稳定控制的关键。
 * @param  motor: 电机对象指针
 */
void Alg_FOC_Park(FOC_Motor_t *motor) 
{
    // 提取三角函数计算结果，避免重复计算耗时
    float sin_theta = sinf(motor->Theta);
    float cos_theta = cosf(motor->Theta);

    // 旋转矩阵乘法
    motor->I_d =  motor->I_alpha * cos_theta + motor->I_beta * sin_theta;
    motor->I_q = -motor->I_alpha * sin_theta + motor->I_beta * cos_theta;
}




/**
 * @brief  逆 Park 变换 (2相旋转目标电压 -> 2相静止目标电压)
 * @note   把 PID 算出来的目标直流量 (Vd, Vq) 结合当前角度，还原成要输出的交流量。
 * @param  motor: 电机对象指针
 */
void Alg_FOC_InvPark(FOC_Motor_t *motor) 
{
    float sin_theta = sinf(motor->Theta);
    float cos_theta = cosf(motor->Theta);

    // 逆旋转矩阵乘法
    motor->V_alpha = motor->V_d_target * cos_theta - motor->V_q_target * sin_theta;
    motor->V_beta  = motor->V_d_target * sin_theta + motor->V_q_target * cos_theta;
}




/**
 * @brief  SVPWM (空间矢量脉宽调制)
 * @note   这是 FOC 的灵魂。它包含了逆 Clarke 变换，并注入了零序分量。
 * 作用是生成能最大化利用电池电压的“马鞍波”，同时算出给定时器的占空比。
 * @param  motor: 电机对象指针
 */
void Alg_FOC_SVPWM(FOC_Motor_t *motor) 
{
    /* 1. 逆 Clarke 变换 (把 Alpha-Beta 变为三相虚拟电压 va, vb, vc) */
    float va = motor->V_alpha;
    float vb = (-motor->V_alpha + FOC_SQRT3 * motor->V_beta) / 2.0f;
    float vc = (-motor->V_alpha - FOC_SQRT3 * motor->V_beta) / 2.0f;

    /* 2. 注入零序分量 (产生马鞍波的精髓) */
    // 找出三相虚拟电压中的最大值和最小值
    float v_min = fminf(va, fminf(vb, vc));
    float v_max = fmaxf(va, fmaxf(vb, vc));
    // 计算共模偏移量 (把波形居中)
    float v_offset = (v_min + v_max) / 2.0f;

    /* 3. 计算最终的三相真实相电压 */
    motor->U_a = va - v_offset;
    motor->U_b = vb - v_offset;
    motor->U_c = vc - v_offset;

    /* 4. 归一化并映射到 PWM 占空比比例 (0.0 ~ 1.0) */
    // 除以母线电压得到百分比；加上 0.5 是为了让交流波形在正值范围内摆动 (中心对齐模式的要求)
    float duty_a = (motor->U_a / motor->V_Bus) + 0.5f;
    float duty_b = (motor->U_b / motor->V_Bus) + 0.5f;
    float duty_c = (motor->U_c / motor->V_Bus) + 0.5f;

    // 安全限制：防止占空比超过 1.0 或低于 0.0，否则定时器会错乱导致炸管
    duty_a = fmaxf(0.0f, fminf(1.0f, duty_a));
    duty_b = fmaxf(0.0f, fminf(1.0f, duty_b));
    duty_c = fmaxf(0.0f, fminf(1.0f, duty_c));

    /* 5. 转换为实际定时器 Compare 寄存器 (CCR) 的数值 */
    motor->CCR_a = (uint32_t)(duty_a * motor->PWM_ARR);
    motor->CCR_b = (uint32_t)(duty_b * motor->PWM_ARR);
    motor->CCR_c = (uint32_t)(duty_c * motor->PWM_ARR);
}




/**
 * @brief  开环步进流水线 (用于前期不带电流反馈的调试)
 * @note   在主循环/中断中，每次给定 Theta 和 V_q_target 后调用此函数即可。
 * @param  motor: 电机对象指针
 */
void Alg_FOC_Step_OpenLoop(FOC_Motor_t *motor) 
{
    // 开环只需要 FOC 的后半段 (2变3)
    Alg_FOC_InvPark(motor);
    Alg_FOC_SVPWM(motor);
}






/**
 * @brief  闭环步进流水线 (用于后期带 ADC 电流反馈的完整运行)
 * @note   ADC 采样完成后，在中断中调用此函数。
 * @param  motor: 电机对象指针
 */
void Alg_FOC_Step_ClosedLoop(FOC_Motor_t *motor) 
{
    // 1. 采集到的相电流进入数学变换 (3变2)
    Alg_FOC_Clarke(motor);
    Alg_FOC_Park(motor);
    
    // ----------------------------------------------------
    // 注意：在这里未来会插入 PID 环路代码！
    // 比如：
    // motor->V_q_target = PID_Calc(&PID_Iq, Target_Iq, motor->I_q);
    // motor->V_d_target = PID_Calc(&PID_Id, 0.0f, motor->I_d); // 期望 Id 为 0
    // ----------------------------------------------------

    // 2. 将 PID 算出的目标电压进行逆变换并生成 PWM (2变3)
    Alg_FOC_InvPark(motor);
    Alg_FOC_SVPWM(motor);
}
