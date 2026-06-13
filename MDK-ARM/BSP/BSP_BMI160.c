#include "BSP_BMI160.h"

BMI160_Data_t bmi_data;

extern SemaphoreHandle_t SPI2_Mutex;



/*
 * @brief  向 BMI160 写入寄存器
 * @param reg: 寄存器地址
 * @param data: 要写入的数据
 * @retval None
*/
static void BMI160_Write_reg(uint8_t reg, uint8_t data)
{
    uint8_t tx_reg = reg & 0x7F;

    BMI_CS_Down();
    HAL_SPI_Transmit(&hspi1, &tx_reg, 1, 0xFFFF);
    HAL_SPI_Transmit(&hspi1, &data, 1, 0xFFFF);
    BMI_CS_Up();
}


/*
 * @brief  从 BMI160 读取寄存器
 * @param reg: 寄存器地址
 * @param buf: 读取到的数据
 * @param len: 要读取的字节数
 * @retval None
*/
static void BMI160_Read_Len(uint8_t reg, uint8_t *buf, uint16_t len)
{
    uint8_t tx_reg = reg | 0x80;
    uint8_t dummy_byte;

    BMI_CS_Down();
    HAL_SPI_Transmit(&hspi1, &tx_reg, 1, 0xFFFF);
    HAL_SPI_Receive(&hspi1, &dummy_byte, 1, 0xFFFF);
    HAL_SPI_Receive(&hspi1, buf, len, 0xFFFF);
    BMI_CS_Up();
}




/**
 * @brief  检查 BMI160 是否在线 (读取 Chip ID)
 * @retval 返回 1 表示成功 (ID 正确), 返回 0 表示失败
 */
uint8_t BMI160_Check(void)
{
    uint8_t chip_id = 0;

    BMI160_Read_Len(BMI160_REG_CHIP_ID, &chip_id, 1);

    if (chip_id == BMI160_CHIP_ID_VAL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/**
 * @brief  初始化 BMI160 传感器
 * @retval 返回 1 表示成功, 返回 0 表示失败
 */
uint8_t BMI160_Init(void)
{
    // 1. 检查 BMI160 是否在线
    if (BMI160_Check() == 0)
    {
        return 0;
    }

    // 2. 软复位
    BMI160_Write_reg(BMI160_REG_CMD, BMI160_CMD_SOFT_RESET);
    vTaskDelay(pdMS_TO_TICKS(30));

    // 3. 配置加速度计: 1600Hz ODR, Normal 带宽
    BMI160_Write_reg(BMI160_REG_ACC_CONF, BMI160_ACC_CONF_VAL);
    vTaskDelay(pdMS_TO_TICKS(1));

    // 4. 配置加速度计量程: ±8g
    BMI160_Write_reg(BMI160_REG_ACC_RANGE, BMI160_ACC_RANGE_VAL);
    vTaskDelay(pdMS_TO_TICKS(1));

    // 5. 配置陀螺仪: 1600Hz ODR, Normal 带宽
    BMI160_Write_reg(BMI160_REG_GYR_CONF, BMI160_GYR_CONF_VAL);
    vTaskDelay(pdMS_TO_TICKS(1));

    // 6. 配置陀螺仪量程: ±2000dps
    BMI160_Write_reg(BMI160_REG_GYR_RANGE, BMI160_GYR_RANGE_VAL);
    vTaskDelay(pdMS_TO_TICKS(1));

    // 7. 加速度计进入 Normal 模式
    BMI160_Write_reg(BMI160_REG_CMD, BMI160_CMD_ACC_NORMAL);
    vTaskDelay(pdMS_TO_TICKS(10));

    // 8. 陀螺仪进入 Normal 模式
    BMI160_Write_reg(BMI160_REG_CMD, BMI160_CMD_GYR_NORMAL);
    vTaskDelay(pdMS_TO_TICKS(100));

    return 1;
}



void BMI160_Read_Data(void)
{
    uint8_t buf[12];

    // 1. 获取 SPI 互斥锁
    if (xSemaphoreTake(SPI2_Mutex, pdMS_TO_TICKS(5)) == pdTRUE)
    {
        // 2. 从 GYR_X_LSB (0x0C) 开始连续读取 12 字节
        //    BMI160 数据顺序: GYR_X, GYR_Y, GYR_Z, ACC_X, ACC_Y, ACC_Z
        BMI160_Read_Len(BMI160_REG_GYR_X_LSB, buf, 12);
        xSemaphoreGive(SPI2_Mutex);
    }
    else
    {
        return;
    }

    // 3. 拼接 16 位原始数据 (低 8 位在前，高 8 位在后)
    //    buf[0-1] = GYR_X, buf[2-3] = GYR_Y, buf[4-5] = GYR_Z
    //    buf[6-7] = ACC_X, buf[8-9] = ACC_Y, buf[10-11] = ACC_Z
    bmi_data.gyro_x_raw = (int16_t)((buf[1]  << 8) | buf[0]);
    bmi_data.gyro_y_raw = (int16_t)((buf[3]  << 8) | buf[2]);
    bmi_data.gyro_z_raw = (int16_t)((buf[5]  << 8) | buf[4]);

    bmi_data.acc_x_raw  = (int16_t)((buf[7]  << 8) | buf[6]);
    bmi_data.acc_y_raw  = (int16_t)((buf[9]  << 8) | buf[8]);
    bmi_data.acc_z_raw  = (int16_t)((buf[11] << 8) | buf[10]);

    // 4. 换算为物理单位 (Accel = ±8g, Gyro = ±2000dps)
    float acc_scale  = 8.0f / 32768.0f;
    float gyro_scale = 2000.0f / 32768.0f;

    bmi_data.acc_x = bmi_data.acc_x_raw * acc_scale;
    bmi_data.acc_y = bmi_data.acc_y_raw * acc_scale;
    bmi_data.acc_z = bmi_data.acc_z_raw * acc_scale;

    bmi_data.gyro_x = bmi_data.gyro_x_raw * gyro_scale;
    bmi_data.gyro_y = bmi_data.gyro_y_raw * gyro_scale;
    bmi_data.gyro_z = bmi_data.gyro_z_raw * gyro_scale;
}
