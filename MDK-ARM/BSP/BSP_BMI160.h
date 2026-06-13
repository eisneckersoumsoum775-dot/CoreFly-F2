#ifndef __BSP_BMI160_H__
#define __BSP_BMI160_H__

#include "main.h"
#include "spi.h"

#include "App_FreeRTOS.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


/**
 * @brief  低电平拉低 CS 引脚 (SPI 通信)
 */
#define BMI_CS_Down()  HAL_GPIO_WritePin(IMU_NSS_GPIO_Port, IMU_NSS_Pin, GPIO_PIN_RESET);
/**
 * @brief  高电平拉高 CS 引脚 (SPI 通信)
 */
#define BMI_CS_Up()    HAL_GPIO_WritePin(IMU_NSS_GPIO_Port, IMU_NSS_Pin, GPIO_PIN_SET);



/**
 * @brief  BMI160 传感器数据结构体
 */
typedef struct {
    // 原始 16 位 ADC 数据 (补码)
    int16_t acc_x_raw, acc_y_raw, acc_z_raw;
    int16_t gyro_x_raw, gyro_y_raw, gyro_z_raw;

    // 转换后的真实物理数据
    float acc_x, acc_y, acc_z;       // 加速度，单位: g
    float gyro_x, gyro_y, gyro_z;    // 角速度，单位: dps
} BMI160_Data_t;

extern BMI160_Data_t bmi_data;




/**
 * @brief  检查 BMI160 是否在线 (读取 Chip ID)
 * @retval 返回 1 表示成功 (ID 正确), 返回 0 表示失败
 */
uint8_t BMI160_Check(void);


/**
 * @brief  初始化 BMI160 传感器
 * @retval 返回 1 表示成功, 返回 0 表示失败
 */
uint8_t BMI160_Init(void);


/**
 * @brief  从 BMI160 读取加速度和角速度数据
 * @retval None (数据存入全局 bmi_data)
 */
void BMI160_Read_Data(void);


/* ===================================================================
 * BMI160 寄存器地址映射表
 * =================================================================== */

/* 1. 基础信息与系统控制 */
#define BMI160_REG_CHIP_ID          0x00    // 芯片 ID，默认值 0xD1
#define BMI160_REG_ERR_REG          0x02    // 错误寄存器
#define BMI160_REG_PMU_STATUS       0x03    // PMU 状态

/* 2. 传感器数据寄存器 (突发读取从 GYR_X_LSB 开始，连续读 12 字节)
 *    BMI160 数据顺序: GYR_X → GYR_Y → GYR_Z → ACC_X → ACC_Y → ACC_Z
 */
#define BMI160_REG_GYR_X_LSB        0x0C    // 角速度 X 轴低 8 位
#define BMI160_REG_GYR_X_MSB        0x0D    // 角速度 X 轴高 8 位
#define BMI160_REG_GYR_Y_LSB        0x0E    // 角速度 Y 轴低 8 位
#define BMI160_REG_GYR_Y_MSB        0x0F    // 角速度 Y 轴高 8 位
#define BMI160_REG_GYR_Z_LSB        0x10    // 角速度 Z 轴低 8 位
#define BMI160_REG_GYR_Z_MSB        0x11    // 角速度 Z 轴高 8 位

#define BMI160_REG_ACC_X_LSB        0x12    // 加速度 X 轴低 8 位
#define BMI160_REG_ACC_X_MSB        0x13    // 加速度 X 轴高 8 位
#define BMI160_REG_ACC_Y_LSB        0x14    // 加速度 Y 轴低 8 位
#define BMI160_REG_ACC_Y_MSB        0x15    // 加速度 Y 轴高 8 位
#define BMI160_REG_ACC_Z_LSB        0x16    // 加速度 Z 轴低 8 位
#define BMI160_REG_ACC_Z_MSB        0x17    // 加速度 Z 轴高 8 位

#define BMI160_REG_SENSORTIME_0     0x18    // 传感器内部时间戳 (低字节)

/* 3. 中断状态 */
#define BMI160_REG_INT_STATUS_0     0x1C    // 中断状态 0
#define BMI160_REG_INT_STATUS_1     0x1D    // 中断状态 1
#define BMI160_REG_INT_STATUS_2     0x1E    // 中断状态 2
#define BMI160_REG_INT_STATUS_3     0x1F    // 中断状态 3

/* 4. 传感器配置 */
#define BMI160_REG_ACC_CONF         0x40    // 加速度计配置 (ODR/BWP)
#define BMI160_REG_ACC_RANGE        0x41    // 加速度计量程
#define BMI160_REG_GYR_CONF         0x42    // 陀螺仪配置 (ODR/BWP)
#define BMI160_REG_GYR_RANGE        0x43    // 陀螺仪量程

/* 5. 中断控制 */
#define BMI160_REG_INT1_IO_CTRL     0x53    // INT1 引脚配置
#define BMI160_REG_INT2_IO_CTRL     0x54    // INT2 引脚配置
#define BMI160_REG_INT_LATCH        0x55    // 中断锁存设置
#define BMI160_REG_INT_MAP_0        0x56    // 中断路由 0
#define BMI160_REG_INT_MAP_1        0x57    // 中断路由 1
#define BMI160_REG_INT_MAP_2        0x58    // 中断路由 2

/* 6. 命令寄存器 */
#define BMI160_REG_CMD              0x7E    // 命令寄存器

/* ===================================================================
 * BMI160 常用指令与配置值
 * =================================================================== */

#define BMI160_CHIP_ID_VAL          0xD1    // 默认 Chip ID

/* CMD 寄存器指令 */
#define BMI160_CMD_SOFT_RESET       0xB6    // 软复位
#define BMI160_CMD_ACC_NORMAL       0x11    // 加速度计进入 Normal 模式
#define BMI160_CMD_GYR_NORMAL       0x15    // 陀螺仪进入 Normal 模式

/* ACC_CONF: ODR=1600Hz, BWP=Normal, us=0 */
#define BMI160_ACC_CONF_VAL         0xC4
/* ACC_RANGE: ±8g */
#define BMI160_ACC_RANGE_VAL        0x08

/* GYR_CONF: ODR=1600Hz, BWP=Normal */
#define BMI160_GYR_CONF_VAL         0xC8
/* GYR_RANGE: ±2000dps */
#define BMI160_GYR_RANGE_VAL        0x00


#endif
