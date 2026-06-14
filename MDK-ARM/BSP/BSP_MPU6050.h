#ifndef __BSP_MPU6050_H__
#define __BSP_MPU6050_H__

#include "main.h"
#include "i2c.h"

#include "App_FreeRTOS.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/**
 * @brief MPU6050 I2C 设备地址 (AD0=0, 7位地址 0x68, 左移 1 位适配 HAL)
 */
#define MPU6050_I2C_ADDR        (0x68 << 1)

/**
 * @brief I2C 通信超时 (ms)
 */
#define MPU6050_I2C_TIMEOUT     10

/**
 * @brief 读取 IMU_INT 引脚电平 (低电平有效中断)
 */
#define IMU_INT_READ()   HAL_GPIO_ReadPin(IMU_INT_GPIO_Port, IMU_INT_Pin)

/**
 * @brief MPU6050 传感器数据结构体
 */
typedef struct {
    // 原始 16 位 ADC 数据 (补码)
    int16_t acc_x_raw, acc_y_raw, acc_z_raw;
    int16_t gyro_x_raw, gyro_y_raw, gyro_z_raw;

    // 温度原始数据
    int16_t temp_raw;
    float temperature;              // 温度，单位: 癈

    // 转换后的真实物理数据
    float acc_x, acc_y, acc_z;      // 加速度，单位: g
    float gyro_x, gyro_y, gyro_z;   // 角速度，单位: dps
} MPU6050_Data_t;

extern MPU6050_Data_t mpu_data;

/**
 * @brief  检查 MPU6050 是否在线 (读取 WHO_AM_I)
 * @retval 返回 1 表示成功 (ID 正确), 返回 0 表示失败
 */
uint8_t MPU6050_Check(void);

/**
 * @brief  初始化 MPU6050 传感器
 * @retval 返回 1 表示成功, 返回 0 表示失败
 */
uint8_t MPU6050_Init(void);

/**
 * @brief  从 MPU6050 读取加速度和角速度数据
 * @retval None (数据存入全局 mpu_data)
 */
void MPU6050_Read_Data(void);

/* ===================================================================
 * MPU6050 寄存器地址映射表
 * =================================================================== */

/* 1. 基础信息与系统控制 */
#define MPU6050_REG_WHO_AM_I        0x75    // 芯片 ID，默认值 0x68
#define MPU6050_REG_PWR_MGMT_1      0x6B    // 电源管理 1 (复位值 0x40=SLEEP)
#define MPU6050_REG_PWR_MGMT_2      0x6C    // 电源管理 2

/* 2. 采样率与滤波器 */
#define MPU6050_REG_SMPRT_DIV       0x19    // 采样率分频器
#define MPU6050_REG_CONFIG          0x1A    // 低通滤波器配置 (DLPF)

/* 3. 传感器配置 */
#define MPU6050_REG_GYRO_CONFIG     0x1B    // 陀螺仪配置 (含 FS_SEL)
#define MPU6050_REG_ACCEL_CONFIG    0x1C    // 加速度计配置 (含 AFS_SEL)

/* 4. 传感器数据寄存器 (从 ACCEL_XOUT_H 开始连续读取 14 字节)
 *    数据顺序: ACC_X, ACC_Y, ACC_Z, TEMP, GYRO_X, GYRO_Y, GYRO_Z
 */
#define MPU6050_REG_ACCEL_XOUT_H    0x3B
#define MPU6050_REG_ACCEL_XOUT_L    0x3C
#define MPU6050_REG_ACCEL_YOUT_H    0x3D
#define MPU6050_REG_ACCEL_YOUT_L    0x3E
#define MPU6050_REG_ACCEL_ZOUT_H    0x3F
#define MPU6050_REG_ACCEL_ZOUT_L    0x40
#define MPU6050_REG_TEMP_OUT_H      0x41
#define MPU6050_REG_TEMP_OUT_L      0x42
#define MPU6050_REG_GYRO_XOUT_H     0x43
#define MPU6050_REG_GYRO_XOUT_L     0x44
#define MPU6050_REG_GYRO_YOUT_H     0x45
#define MPU6050_REG_GYRO_YOUT_L     0x46
#define MPU6050_REG_GYRO_ZOUT_H     0x47
#define MPU6050_REG_GYRO_ZOUT_L     0x48

/* 5. 中断控制 */
#define MPU6050_REG_INT_PIN_CFG     0x37    // 中断引脚配置
#define MPU6050_REG_INT_ENABLE      0x38    // 中断使能
#define MPU6050_REG_INT_STATUS      0x3A    // 中断状态

/* ===================================================================
 * MPU6050 常用配置值
 * =================================================================== */

#define MPU6050_WHO_AM_I_VAL        0x68    // 默认 Chip ID

/* PWR_MGMT_1: 退出睡眠, 选择内部 8MHz 振荡器 (CLKSEL=0) */
#define MPU6050_PWR_MGMT1_WAKE      0x00

/* SMPRT_DIV: 采样率 = 输出率 / (1 + SMPRT_DIV),  0 = 不分频 */
#define MPU6050_SMPRT_DIV_VAL       0x00

/* CONFIG: DLPF_CFG[2:0] = 1 -> 陀螺 188Hz / 加速度 184Hz */
#define MPU6050_CONFIG_DLPF_VAL     0x01

/* GYRO_CONFIG: FS_SEL[1:0] = 3 -> 卤2000 dps */
#define MPU6050_GYRO_FS_2000        0x18    // 0b00011000

/* ACCEL_CONFIG: AFS_SEL[1:0] = 2 -> 卤8g */
#define MPU6050_ACCEL_FS_8G         0x10    // 0b00010000

/* 灵敏度常量 (来自 MPU-6050 Product Specification Rev 3.4) */
#define MPU6050_GYRO_SENS_2000      16.4f   // LSB/(鲁/s)  at 卤2000 dps
#define MPU6050_ACCEL_SENS_8G       4096.0f // LSB/g       at 卤8g

#endif