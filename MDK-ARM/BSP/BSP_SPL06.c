#include "BSP_SPL06.h"

SPL06_Data_t spl06_data;
extern SemaphoreHandle_t IIC_Mutex;

static int16_t spl06_c0, spl06_c1;
static int32_t spl06_c00, spl06_c10;
static int16_t spl06_c01, spl06_c11, spl06_c20, spl06_c21, spl06_c30;

/* ---- I2C 读写 ---- */
static HAL_StatusTypeDef SPL06_Write_Reg(uint8_t reg, uint8_t data)
{
    HAL_StatusTypeDef ret = HAL_ERROR;
    if (xSemaphoreTake(IIC_Mutex, 10) == pdTRUE) {
        ret = HAL_I2C_Mem_Write(&hi2c1, SPL06_I2C_ADDR << 1, reg,
                                I2C_MEMADD_SIZE_8BIT, &data, 1, 10);
        xSemaphoreGive(IIC_Mutex);
    }
    return ret;
}

static HAL_StatusTypeDef SPL06_Read_Len(uint8_t reg, uint8_t *buf, uint16_t len)
{
    HAL_StatusTypeDef ret = HAL_ERROR;
    if (xSemaphoreTake(IIC_Mutex, 10) == pdTRUE) {
        ret = HAL_I2C_Mem_Read(&hi2c1, SPL06_I2C_ADDR << 1, reg,
                               I2C_MEMADD_SIZE_8BIT, buf, len, 10);
        xSemaphoreGive(IIC_Mutex);
    }
    return ret;
}

/* ---- 系数解析 (18字节 -> 9个系数) ---- */
static void SPL06_Parse_Coef(const uint8_t src[18])
{
    spl06_c0  = (int16_t)(((uint16_t)src[0] << 4) | (src[1] >> 4));
    if (spl06_c0  & 0x0800) spl06_c0  -= 0x1000;

    spl06_c1  = (int16_t)(((uint16_t)(src[1] & 0x0F) << 8) | src[2]);
    if (spl06_c1  & 0x0800) spl06_c1  -= 0x1000;

    spl06_c00 = (int32_t)(((uint32_t)src[3] << 12) | ((uint32_t)src[4] << 4) | ((uint32_t)src[5] >> 4));
    if (spl06_c00 & 0x00080000) spl06_c00 -= 0x00100000;

    spl06_c10 = (int32_t)(((uint32_t)(src[5] & 0x0F) << 16) | ((uint32_t)src[6] << 8) | src[7]);
    if (spl06_c10 & 0x00080000) spl06_c10 -= 0x00100000;

    spl06_c01 = (int16_t)(((uint16_t)src[8]  << 8) | src[9]);
    spl06_c11 = (int16_t)(((uint16_t)src[10] << 8) | src[11]);
    spl06_c20 = (int16_t)(((uint16_t)src[12] << 8) | src[13]);
    spl06_c21 = (int16_t)(((uint16_t)src[14] << 8) | src[15]);
    spl06_c30 = (int16_t)(((uint16_t)src[16] << 8) | src[17]);
}

/* ---- 检查 ---- */
uint8_t SPL06_Check(void)
{
    uint8_t id = 0;
    if (SPL06_Read_Len(SPL06_REG_ID, &id, 1) != HAL_OK) return 0;
    return (id == SPL06_CHIP_ID) ? 1 : 0;
}

/* ---- 初始化 ---- */
uint8_t SPL06_Init(void)
{
    uint8_t buf[SPL06_COEF_SIZE], cfg;

    SPL06_Write_Reg(SPL06_REG_RESET, SPL06_SOFT_RESET);
    vTaskDelay(pdMS_TO_TICKS(40));

    if (!SPL06_Check()) return 0;

    for (int i = 0; i < 50; i++) {
        if (SPL06_Read_Len(SPL06_REG_MEAS_CFG, &cfg, 1) == HAL_OK && (cfg & SPL06_COEF_RDY))
            break;
        vTaskDelay(2);
    }

    if (SPL06_Read_Len(SPL06_REG_COEF_SRC, buf, SPL06_COEF_SIZE) != HAL_OK) return 0;
    SPL06_Parse_Coef(buf);

    SPL06_Write_Reg(SPL06_REG_TMP_CFG, SPL06_TMP_CFG_VAL);
    SPL06_Write_Reg(SPL06_REG_PRS_CFG, SPL06_PRS_CFG_VAL);
    SPL06_Write_Reg(SPL06_REG_CFG_REG, SPL06_CFG_P_SHIFT | SPL06_CFG_T_SHIFT);
    SPL06_Write_Reg(SPL06_REG_MEAS_CFG, SPL06_MEAS_CTRL_CONT);
    vTaskDelay(pdMS_TO_TICKS(100));

    return 1;
}

/* ---- 读数据 ---- */
void SPL06_Read_Data(void)
{
    uint8_t buf[6];
    int32_t praw, traw;
    float Psc, Tsc, Psc2, Tsc2;

    if (SPL06_Read_Len(SPL06_REG_PSR_B2, buf, 6) != HAL_OK) return;

    praw = ((int32_t)buf[0] << 16) | ((int32_t)buf[1] << 8) | buf[2];
    traw = ((int32_t)buf[3] << 16) | ((int32_t)buf[4] << 8) | buf[5];
    if (praw & 0x800000) praw -= 0x1000000;
    if (traw & 0x800000) traw -= 0x1000000;

    spl06_data.pressure_raw = praw;
    spl06_data.temp_raw     = traw;

    Psc = (float)praw / SPL06_KP_KT;
    Tsc = (float)traw / SPL06_KP_KT;

    spl06_data.temperature_c = (float)spl06_c0 * 0.5f + (float)spl06_c1 * Tsc;

    Psc2 = Psc * Psc;
    Tsc2 = Tsc * Tsc;
    /* P = c00 + c10*Psc + c20*Psc^2 + c30*Psc^3
              + c01*Tsc + c11*Tsc*Psc + c21*Tsc*Psc^2 */
    spl06_data.pressure_pa  = (float)spl06_c00;
    spl06_data.pressure_pa += (float)spl06_c10 * Psc;
    spl06_data.pressure_pa += (float)spl06_c01 * Tsc;
    spl06_data.pressure_pa += (float)spl06_c20 * Psc2;
    spl06_data.pressure_pa += (float)spl06_c11 * Psc * Tsc;
    spl06_data.pressure_pa += (float)spl06_c21 * Psc2 * Tsc;
    spl06_data.pressure_pa += (float)spl06_c30 * Psc2 * Psc;
}