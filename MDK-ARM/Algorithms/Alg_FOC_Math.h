#ifndef __ALG_FOC_MATH_H__
#define __ALG_FOC_MATH_H__

#include <stdint.h>
#include <math.h>

/* ============================================================================
 * 硬件与数学常数定义 (预先计算好，用空间换取 CPU 算力)
 * ============================================================================ */
#define FOC_SQRT3       1.73205081f  // 根号 3 (用于 Clarke 和 SVPWM 变换)
#define FOC_INV_SQRT3   0.57735027f  // 根号 3 的倒数 (把除法变乘法，极大提升运行速度)

/**
 * @brief FOC 核心状态结构体 (面向对象思想的体现)
 * @note  把一个电机所需的所有变量打包。如果要控制 4 个电机，只需定义 4 个这个结构体
 */
typedef struct {
    /* --- 1. 基础配置参数 (硬件物理约束，初始化后基本不变) --- */
    float V_Bus;        // 母线电压 (例如: 12.0f V)。用于计算占空比比例
    uint32_t PWM_ARR;   // 定时器自动重装载值 (例如: 8400)。决定了 PWM 的最大满偏值

    /* --- 2. 反馈输入变量 (闭环时，由 ADC 和传感器实时更新) --- */
    float I_a;          // 实时采样的 A 相相电流 (安培)
    float I_b;          // 实时采样的 B 相相电流 (安培)
    float I_c;          // 实时采样的 C 相相电流 (安培)
    float Theta;        // 电机的实时电角度 (弧度制，0 ~ 2π)

    /* --- 3. 坐标变换中间变量 (诊断和 PID 计算的核心) --- */
    float I_alpha;      // Clarke 变换结果：静止直角坐标系下的 Alpha 轴电流
    float I_beta;       // Clarke 变换结果：静止直角坐标系下的 Beta  轴电流
    float I_d;          // Park 变换结果：旋转坐标系下的 D 轴电流 (励磁电流，通常期望值为 0)
    float I_q;          // Park 变换结果：旋转坐标系下的 Q 轴电流 (转矩电流，决定了电机的力气)

    /* --- 4. 控制目标变量 (由飞控 PID 计算给出，或者开环时手动给出) --- */
    float V_d_target;   // 目标 D 轴电压 (期望的励磁电压，无弱磁需求时设为 0)
    float V_q_target;   // 目标 Q 轴电压 (期望的推力电压，相当于“油门”大小)

    /* --- 5. 逆变换中间变量 (用于生成 PWM 的数学准备) --- */
    float V_alpha;      // 逆 Park 变换结果：静止坐标系 Alpha 轴目标电压
    float V_beta;       // 逆 Park 变换结果：静止坐标系 Beta  轴目标电压
    float U_a;          // SVPWM 算出的 A 相目标电压 (未归一化)
    float U_b;          // SVPWM 算出的 B 相目标电压 (未归一化)
    float U_c;          // SVPWM 算出的 C 相目标电压 (未归一化)
    
    /* --- 6. 最终输出变量 (直接丢给底层定时器寄存器执行) --- */
    uint32_t CCR_a;     // 换算后的 A 相 PWM 占空比数值 (直接写入 TIMx->CCR1)
    uint32_t CCR_b;     // 换算后的 B 相 PWM 占空比数值 (直接写入 TIMx->CCR2)
    uint32_t CCR_c;     // 换算后的 C 相 PWM 占空比数值 (直接写入 TIMx->CCR3)

} FOC_Motor_t;

/* ============================================================================
 * 核心算法 API 接口声明
 * ============================================================================ */

/**
 * @brief  初始化 FOC 电机对象
 * @param  motor: 电机对象指针
 * @param  v_bus: 电池母线电压 (例如 12V 电池则传入 12.0f)
 * @param  arr:   PWM 定时器的自动重装载值 (决定了 100% 占空比对应的数值)
 * @retval 无
 */
void Alg_FOC_Init(FOC_Motor_t *motor, float v_bus, uint32_t arr);



/**
 * @brief  Clarke 变换 (3相电流 -> 2相静止正交电流)
 * @note   利用基尔霍夫电流定律 Ia + Ib + Ic = 0，省去了对 Ic 的采样。
 * 公式：I_alpha = I_a;  I_beta = (I_a + 2*I_b) / sqrt(3)
 * @param  motor: 电机对象指针
 */
void Alg_FOC_Clarke(FOC_Motor_t *motor);

/**
 * @brief  Park 变换 (2相静止 -> 2相旋转)
 * @note   将交流量 (I_alpha, I_beta) 变成相对于转子静止的直流量 (I_d, I_q)。
 * 这是闭环 PID 能否稳定控制的关键。
 * @param  motor: 电机对象指针
 */
void Alg_FOC_Park(FOC_Motor_t *motor);


/**
 * @brief  逆 Park 变换 (2相旋转目标电压 -> 2相静止目标电压)
 * @note   把 PID 算出来的目标直流量 (Vd, Vq) 结合当前角度，还原成要输出的交流量。
 * @param  motor: 电机对象指针
 */
void Alg_FOC_InvPark(FOC_Motor_t *motor);


/**
 * @brief  SVPWM (空间矢量脉宽调制)
 * @note   这是 FOC 的灵魂。它包含了逆 Clarke 变换，并注入了零序分量。
 * 作用是生成能最大化利用电池电压的“马鞍波”，同时算出给定时器的占空比。
 * @param  motor: 电机对象指针
 */
void Alg_FOC_SVPWM(FOC_Motor_t *motor);




/**
 * @brief  开环步进流水线 (用于前期不带电流反馈的调试)
 * @note   在主循环/中断中，每次给定 Theta 和 V_q_target 后调用此函数即可。
 * @param  motor: 电机对象指针
 */
void Alg_FOC_Step_OpenLoop(FOC_Motor_t *motor);



/**
 * @brief  闭环步进流水线 (用于后期带 ADC 电流反馈的完整运行)
 * @note   ADC 采样完成后，在中断中调用此函数。
 * @param  motor: 电机对象指针
 */
void Alg_FOC_Step_ClosedLoop(FOC_Motor_t *motor);



#endif
