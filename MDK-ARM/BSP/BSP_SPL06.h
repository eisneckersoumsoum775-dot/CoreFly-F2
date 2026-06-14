#ifndef __BSP_SPL06_H__
#define __BSP_SPL06_H__

#include "main.h"
#include "i2c.h"
#include "App_FreeRTOS.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* I2C 地址 (7-bit, SDO=0) */
#define SPL06_I2C_ADDR          0x76

/* 寄存器 */
#define SPL06_REG_PSR_B2        0x00
#define SPL06_REG_PSR_B1        0x01
#define SPL06_REG_PSR_B0        0x02
#define SPL06_REG_TMP_B2        0x03
#define SPL06_REG_TMP_B1        0x04
#define SPL06_REG_TMP_B0        0x05
#define SPL06_REG_PRS_CFG       0x06
#define SPL06_REG_TMP_CFG       0x07
#define SPL06_REG_MEAS_CFG      0x08
#define SPL06_REG_CFG_REG       0x09
#define SPL06_REG_RESET         0x0C
#define SPL06_REG_ID            0x0D
#define SPL06_REG_COEF_SRC      0x10
#define SPL06_COEF_SIZE         18

#define SPL06_CHIP_ID           0x10
#define SPL06_SOFT_RESET        0x09

/* 64x过采样, 16Hz */
#define SPL06_PRS_CFG_VAL       0x46
#define SPL06_TMP_CFG_VAL       0xC6
#define SPL06_MEAS_CTRL_CONT    0x07
#define SPL06_CFG_P_SHIFT       0x04
#define SPL06_CFG_T_SHIFT       0x08

#define SPL06_COEF_RDY          0x80
#define SPL06_KP_KT             1040384.0f

typedef struct {
    int32_t pressure_raw;
    int32_t temp_raw;
    float   pressure_pa;
    float   temperature_c;
} SPL06_Data_t;

extern SPL06_Data_t spl06_data;

uint8_t SPL06_Check(void);
uint8_t SPL06_Init(void);
void    SPL06_Read_Data(void);

#endif