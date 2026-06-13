#ifndef __BSP_SPL06_H__
#define __BSP_SPL06_H__

#include "main.h"
#include "i2c.h"

#include "App_FreeRTOS.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "stdint.h"


/**
 * @brief  SPL06-001 传感器数据结构体
 */
typedef struct {
    // 原始 24 位 ADC 数据 (补码)
    int32_t pressure_raw;   // 原始气压 ADC
    int32_t temp_raw;       // 原始温度 ADC

    // 转换后的真实物理数据
    float pressure_pa;      // 气压，单位: Pa
    float temperature_c;    // 温度，单位: ℃
} SPL06_Data_t;

extern SPL06_Data_t spl06_data;


/**
 * @brief  检查 SPL06-001 是否在线 (读取 Chip ID)
 * @retval 返回 1 表示成功, 返回 0 表示失败
 */
uint8_t SPL06_Check(void);

/**
 * @brief  初始化 SPL06-001 传感器
 * @retval 返回 1 表示成功, 返回 0 表示失败
 */
uint8_t SPL06_Init(void);

/**
 * @brief  从 SPL06-001 读取气压和温度数据
 * @retval None (数据存入全局 spl06_data)
 */
void SPL06_Read_Data(void);


/* ===================================================================
 * SPL06-001 寄存器地址映射表
 * =================================================================== */

/* 1. 传感器数据寄存器 (24 位，高位在前) */
#define SPL06_REG_PSR_B2        0x00    // 气压数据 MSB
#define SPL06_REG_PSR_B1        0x01    // 气压数据中间字节
#define SPL06_REG_PSR_B0        0x02    // 气压数据 LSB
#define SPL06_REG_TMP_B2        0x03    // 温度数据 MSB
#define SPL06_REG_TMP_B1        0x04    // 温度数据中间字节
#define SPL06_REG_TMP_B0        0x05    // 温度数据 LSB

/* 2. 传感器配置寄存器 */
#define SPL06_REG_P_CFG         0x06    // 气压配置 (过采样率、精度)
#define SPL06_REG_T_CFG         0x07    // 温度配置 (过采样率、精度)
#define SPL06_REG_MEAS_CFG      0x08    // 测量配置与状态

/* 3. 状态与中断寄存器 */
#define SPL06_REG_CFG_REG       0x09    // 传感器状态 (T/Rdy, P/Rdy)
#define SPL06_REG_INT_STS       0x0A    // 中断状态
#define SPL06_REG_FIFO_STS      0x0B    // FIFO 状态

/* 4. 系统控制寄存器 */
#define SPL06_REG_RESET         0x0C    // 软复位寄存器
#define SPL06_REG_ID            0x0D    // 芯片 ID 寄存器

/* 5. 校准系数 (共 18 字节, 0x10 ~ 0x21) */
#define SPL06_REG_COEF_SRC      0x10    // 校准系数起始地址
#define SPL06_COEF_SIZE         18      // 校准系数总字节数

/* ===================================================================
 * SPL06-001 常用配置值与指令
 * =================================================================== */

#define SPL06_CHIP_ID_VAL       0x10    // 默认 Chip ID

/* I2C 从机地址 (7-bit)
 *   SDO 接 GND → 0x76
 *   SDO 接 VDD → 0x77
 */
#define SPL06_I2C_ADDR          0x76

/* 软复位命令 */
#define SPL06_CMD_SOFT_RESET    0x89    // 写入 0x0C 触发软复位

/* Pressure Configuration (P_CFG, 0x06):
 *   PM_RATE[6:4]: 测量频率 (0=1Hz, 1=2Hz, 2=4Hz, 3=8Hz, 4=16Hz, 5=32Hz, 6=64Hz, 7=128Hz)
 *   PM_PRC[3:0] : 过采样率 (0=1x, 1=2x, 2=4x, 3=8x, 4=16x, 5=32x, 6=64x, 7=128x)
 */
#define SPL06_P_CFG_VAL         0x56    // PM_RATE=5(32Hz), PM_PRC=6(64x 过采样)

/* Temperature Configuration (T_CFG, 0x07):
 *   TMP_EXT[7]   : 外部温度传感器选择 (0=内部)
 *   TMP_RATE[6:4]: 测量频率
 *   TMP_PRC[3:0] : 过采样率
 */
#define SPL06_T_CFG_VAL         0x56    // TMP_RATE=5(32Hz), TMP_PRC=6(64x 过采样)

/* Measurement Configuration (MEAS_CFG, 0x08):
 *   COEF_RDY[7] : 校准系数就绪标志 (只读)
 *   SENSOR_RDY[6]: 传感器初始化完成 (只读)
 *   TMP_RDY[5]  : 温度数据就绪 (只读)
 *   P_RDY[4]    : 气压数据就绪 (只读)
 *   MEAS_CTRL[2:0]: 测量模式控制
 *     000 = Idle / Stop
 *     001 = Temperature only
 *     010 = Pressure only
 *     011 = Temperature + Pressure (Background)
 *     100-111 = Reserved
 */
#define SPL06_MEAS_CFG_VAL      0x07    // 连续模式: 温度 + 气压

/* 状态寄存器位掩码 */
#define SPL06_CFG_TMP_RDY       0x20    // 温度数据就绪
#define SPL06_CFG_P_RDY         0x10    // 气压数据就绪
#define SPL06_CFG_SENSOR_RDY    0x40    // 传感器初始化完成
#define SPL06_CFG_COEF_RDY      0x80    // 校准系数就绪

/* 过采样率对应的比例因子 (用于原始数据归一化) */
#define SPL06_OSR_1X            524288
#define SPL06_OSR_2X            1048576
#define SPL06_OSR_4X            2097152
#define SPL06_OSR_8X            4194304
#define SPL06_OSR_16X           8388608
#define SPL06_OSR_32X           16777216
#define SPL06_OSR_64X           33554432
#define SPL06_OSR_128X          67108864

#endif
