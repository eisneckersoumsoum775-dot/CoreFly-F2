#ifndef __ALG_MPU6050_DATA_H__
#define __ALG_MPU6050_DATA_H__

#include <stdint.h>
#include <math.h>

/**
 * @brief MPU6050 姿态角数据结构体
 */
typedef struct {
    float roll;     // 横滚角 (°)
    float pitch;    // 俯仰角 (°)
    float yaw;      // 偏航角 (°)  — 陀螺积分，会漂
    float dt;       // 采样间隔 (秒)

    /* 零偏校准值 */
    float acc_offset_x, acc_offset_y, acc_offset_z;     // 加速度计零偏 (g)
    float gyro_offset_x, gyro_offset_y, gyro_offset_z;  // 陀螺仪零偏 (dps)

    /* 内部累计 (用于自动校准) */
    float _sum_ax, _sum_ay, _sum_az;
    float _sum_gx, _sum_gy, _sum_gz;
    int   _count;

    /* 平滑用 */
    float _roll_prev, _pitch_prev;
} MPU6050_Attitude_t;

/**
 * @brief  初始化
 * @param  att: 姿态结构体指针
 * @param  dt:  采样间隔 (秒)
 */
void Alg_MPU6050_Init(MPU6050_Attitude_t *att, float dt);

/**
 * @brief  喂入一帧数据并累计 (用于自动校准)
 *         调用 N 次 (推荐 200) 后校准完成
 * @retval 1=校准完成, 0=继续采集
 */
int Alg_MPU6050_Calibrate(MPU6050_Attitude_t *att,
    float ax, float ay, float az,
    float gx, float gy, float gz);

/**
 * @brief  6轴原始数据 → Roll / Pitch / Yaw
 */
void Alg_MPU6050_Update(MPU6050_Attitude_t *att,
    float ax, float ay, float az,
    float gx, float gy, float gz);

#endif