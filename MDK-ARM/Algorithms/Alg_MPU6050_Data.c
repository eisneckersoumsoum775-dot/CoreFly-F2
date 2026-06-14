#include "Alg_MPU6050_Data.h"

#define RAD_TO_DEG          57.2957795131f
#define CALIB_SAMPLES       200
#define SMOOTH_FACTOR       0.7f        // 平滑系数, 越小越平滑

void Alg_MPU6050_Init(MPU6050_Attitude_t *att, float dt)
{
    att->roll  = 0.0f;
    att->pitch = 0.0f;
    att->yaw   = 0.0f;
    att->dt    = dt;

    att->acc_offset_x = att->acc_offset_y = att->acc_offset_z = 0.0f;
    att->gyro_offset_x = att->gyro_offset_y = att->gyro_offset_z = 0.0f;
    att->_sum_ax = att->_sum_ay = att->_sum_az = 0.0f;
    att->_sum_gx = att->_sum_gy = att->_sum_gz = 0.0f;
    att->_count  = 0;
    att->_roll_prev  = 0.0f;
    att->_pitch_prev = 0.0f;
}

int Alg_MPU6050_Calibrate(MPU6050_Attitude_t *att,
    float ax, float ay, float az,
    float gx, float gy, float gz)
{
    att->_sum_ax += ax;
    att->_sum_ay += ay;
    att->_sum_az += az;
    att->_sum_gx += gx;
    att->_sum_gy += gy;
    att->_sum_gz += gz;
    att->_count++;

    if (att->_count >= CALIB_SAMPLES)
    {
        float n = (float)att->_count;
        att->acc_offset_x  = att->_sum_ax / n;
        att->acc_offset_y  = att->_sum_ay / n;
        att->acc_offset_z  = att->_sum_az / n - 1.0f;
        att->gyro_offset_x = att->_sum_gx / n;
        att->gyro_offset_y = att->_sum_gy / n;
        att->gyro_offset_z = att->_sum_gz / n;
        return 1;
    }
    return 0;
}

void Alg_MPU6050_Update(MPU6050_Attitude_t *att,
    float ax, float ay, float az,
    float gx, float gy, float gz)
{
    float roll_raw, pitch_raw;

    /* 去零偏 */
    ax -= att->acc_offset_x;
    ay -= att->acc_offset_y;
    az -= att->acc_offset_z;
    gx -= att->gyro_offset_x;
    gy -= att->gyro_offset_y;
    gz -= att->gyro_offset_z;

    /* Roll / Pitch 由加速度计算 */
    roll_raw  = atan2f(ay, az) * RAD_TO_DEG;
    pitch_raw = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG;

    /* 一阶平滑: out = k*prev + (1-k)*raw */
    att->roll  = SMOOTH_FACTOR * att->_roll_prev  + (1.0f - SMOOTH_FACTOR) * roll_raw;
    att->pitch = SMOOTH_FACTOR * att->_pitch_prev + (1.0f - SMOOTH_FACTOR) * pitch_raw;

    att->_roll_prev  = att->roll;
    att->_pitch_prev = att->pitch;

    /* Yaw 陀螺积分 */
    att->yaw += gz * att->dt;
}