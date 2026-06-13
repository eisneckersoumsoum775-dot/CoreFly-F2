#include "BSP_SPL06.h"

SPL06_Data_t spl06_data;

extern SemaphoreHandle_t IIC_Mutex;

/* SPL06-001 校准系数 (从寄存器 0x10-0x21 读取并解析) */
static int16_t spl06_c0, spl06_c1;
static int32_t spl06_c00, spl06_c10;
static int16_t spl06_c01, spl06_c11, spl06_c20, spl06_c21, spl06_c30;


/*
 * @brief  通过 I2C 向 SPL06-001 写入单个寄存器
 * @param  reg: 寄存器地址
 * @param  data: 要写入的数据
 * @retval None
 */
static void SPL06_Write_Reg(uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = { reg, data };

    if (xSemaphoreTake(IIC_Mutex, pdMS_TO_TICKS(5)) == pdTRUE)
    {
        HAL_I2C_Master_Transmit(&hi2c1, SPL06_I2C_ADDR << 1, buf, 2, 0xFF);
        xSemaphoreGive(IIC_Mutex);
    }
}


/*
 * @brief  通过 I2C 从 SPL06-001 读取单个寄存器
 * @param  reg: 寄存器地址
 * @param  buf: 读取到的数据指针
 * @retval None
 */
static void SPL06_Read_Reg(uint8_t reg, uint8_t *buf)
{
    if (xSemaphoreTake(IIC_Mutex, pdMS_TO_TICKS(5)) == pdTRUE)
    {
        HAL_I2C_Master_Transmit(&hi2c1, SPL06_I2C_ADDR << 1, &reg, 1, 0xFF);
        HAL_I2C_Master_Receive(&hi2c1, SPL06_I2C_ADDR << 1, buf, 1, 0xFF);
        xSemaphoreGive(IIC_Mutex);
    }
}


/*
 * @brief  通过 I2C 从 SPL06-001 连续读取多个寄存器
 * @param  reg: 起始寄存器地址
 * @param  buf: 读取到的数据缓冲区
 * @param  len: 要读取的字节数
 * @retval None
 */
static void SPL06_Read_Len(uint8_t reg, uint8_t *buf, uint16_t len)
{
    if (xSemaphoreTake(IIC_Mutex, pdMS_TO_TICKS(5)) == pdTRUE)
    {
        HAL_I2C_Master_Transmit(&hi2c1, SPL06_I2C_ADDR << 1, &reg, 1, 0xFF);
        HAL_I2C_Master_Receive(&hi2c1, SPL06_I2C_ADDR << 1, buf, len, 0xFF);
        xSemaphoreGive(IIC_Mutex);
    }
}


/*
 * @brief  解析 SPL06-001 的 9 个校准系数 (18 字节)
 * @param  src: 从 0x10 读取的 18 字节原始数据
 * @retval None
 */
static void SPL06_Parse_Coef(const uint8_t src[18])
{
    // c0: 12-bit 有符号
    spl06_c0 = (int16_t)(((uint16_t)(src[0] & 0x0F) << 8) | src[1]);
    if (spl06_c0 & 0x0800) spl06_c0 -= 0x1000;

    // c1: 12-bit 有符号
    spl06_c1 = (int16_t)(((uint16_t)(src[0] & 0xF0) << 4) | src[2]);
    if (spl06_c1 & 0x0800) spl06_c1 -= 0x1000;

    // c00: 20-bit 有符号
    spl06_c00 = (int32_t)(((uint32_t)src[3] << 12) | ((uint32_t)src[4] << 4) | ((uint32_t)src[5] >> 4));
    if (spl06_c00 & 0x00080000) spl06_c00 -= 0x00100000;

    // c10: 20-bit 有符号
    spl06_c10 = (int32_t)(((uint32_t)(src[5] & 0x0F) << 16) | ((uint32_t)src[6] << 8) | src[7]);
    if (spl06_c10 & 0x00080000) spl06_c10 -= 0x00100000;

    // c01: 16-bit 有符号
    spl06_c01 = (int16_t)(((uint16_t)src[8] << 8) | src[9]);

    // c11: 16-bit 有符号
    spl06_c11 = (int16_t)(((uint16_t)src[10] << 8) | src[11]);

    // c20: 16-bit 有符号
    spl06_c20 = (int16_t)(((uint16_t)src[12] << 8) | src[13]);

    // c21: 16-bit 有符号
    spl06_c21 = (int16_t)(((uint16_t)src[14] << 8) | src[15]);

    // c30: 16-bit 有符号
    spl06_c30 = (int16_t)(((uint16_t)src[16] << 8) | src[17]);
}


/**
 * @brief  检查 SPL06-001 是否在线 (读取 Chip ID)
 * @retval 返回 1 表示成功, 返回 0 表示失败
 */
uint8_t SPL06_Check(void)
{
    uint8_t chip_id = 0;

    SPL06_Read_Reg(SPL06_REG_ID, &chip_id);

    if (chip_id == SPL06_CHIP_ID_VAL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/**
 * @brief  初始化 SPL06-001 传感器
 * @retval 返回 1 表示成功, 返回 0 表示失败
 */
uint8_t SPL06_Init(void)
{
    uint8_t calib[SPL06_COEF_SIZE];

    // 1. 检查 SPL06-001 是否在线
    if (SPL06_Check() == 0)
    {
        return 0;
    }

    // 2. 软复位
    SPL06_Write_Reg(SPL06_REG_RESET, SPL06_CMD_SOFT_RESET);
    vTaskDelay(pdMS_TO_TICKS(30));

    // 3. 读取并解析校准系数
    SPL06_Read_Len(SPL06_REG_COEF_SRC, calib, SPL06_COEF_SIZE);
    SPL06_Parse_Coef(calib);

    // 4. 配置温度过采样率: 64x, 32Hz
    SPL06_Write_Reg(SPL06_REG_T_CFG, SPL06_T_CFG_VAL);
    vTaskDelay(pdMS_TO_TICKS(1));

    // 5. 配置气压过采样率: 64x, 32Hz
    SPL06_Write_Reg(SPL06_REG_P_CFG, SPL06_P_CFG_VAL);
    vTaskDelay(pdMS_TO_TICKS(1));

    // 6. 启动连续测量模式 (温度 + 气压)
    SPL06_Write_Reg(SPL06_REG_MEAS_CFG, SPL06_MEAS_CFG_VAL);
    vTaskDelay(pdMS_TO_TICKS(50));

    return 1;
}


/**
 * @brief  从 SPL06-001 读取气压和温度数据
 * @retval None (数据存入全局 spl06_data)
 */
void SPL06_Read_Data(void)
{
    uint8_t buf[6];
    int32_t traw, praw;

    // 1. 读取 6 字节原始数据: TMP(3B) + PSR(3B)
    SPL06_Read_Len(SPL06_REG_PSR_B2, buf, 6);

    // 2. 拼接 24 位原始数据 (高位在前, 补码格式)
    praw = ((int32_t)buf[0] << 16) | ((int32_t)buf[1] << 8) | (int32_t)buf[2];
    traw = ((int32_t)buf[3] << 16) | ((int32_t)buf[4] << 8) | (int32_t)buf[5];

    // 24 位有符号数扩展
    if (praw & 0x800000) praw -= 0x1000000;
    if (traw & 0x800000) traw -= 0x1000000;

    spl06_data.pressure_raw = praw;
    spl06_data.temp_raw     = traw;

    // 3. 归一化 (过采样率 64x → 比例因子 = 524288 × 64 = 33554432)
    float Tsc = (float)traw / 33554432.0f;
    float Psc = (float)praw / 33554432.0f;

    // 4. 温度补偿计算
    //    T(℃) = c0 × 0.5 + c1 × Tsc
    spl06_data.temperature_c = (float)spl06_c0 * 0.5f + (float)spl06_c1 * Tsc;

    // 5. 气压补偿计算
    //    P(Pa) = c00 + c10·Psc + c01·Tsc + c20·Psc² + c11·Psc·Tsc
    //          + c21·Tsc² + c30·Psc³
    spl06_data.pressure_pa  = (float)spl06_c00;
    spl06_data.pressure_pa += (float)spl06_c10 * Psc;
    spl06_data.pressure_pa += (float)spl06_c01 * Tsc;
    spl06_data.pressure_pa += (float)spl06_c20 * Psc * Psc;
    spl06_data.pressure_pa += (float)spl06_c11 * Psc * Tsc;
    spl06_data.pressure_pa += (float)spl06_c21 * Tsc * Tsc;
    spl06_data.pressure_pa += (float)spl06_c30 * Psc * Psc * Psc;
}
