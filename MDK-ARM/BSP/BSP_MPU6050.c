#include "BSP_MPU6050.h"

MPU6050_Data_t mpu_data;

/*
 * @brief  向 MPU6050 写入寄存器
 * @param reg: 寄存器地址
 * @param data: 要写入的数据
 * @retval HAL_OK 成功, 其他值表示 I2C 通信失败
 */
static HAL_StatusTypeDef MPU6050_Write_Reg(uint8_t reg, uint8_t data)
{
    return HAL_I2C_Mem_Write(&hi2c1, MPU6050_I2C_ADDR, reg,
                             I2C_MEMADD_SIZE_8BIT, &data, 1,
                             MPU6050_I2C_TIMEOUT);
}

/*
 * @brief  从 MPU6050 读取多个寄存器
 * @param reg: 起始寄存器地址
 * @param buf: 读取到的数据缓冲区
 * @param len: 要读取的字节数
 * @retval HAL_OK 成功, 其他值表示 I2C 通信失败
 */
static HAL_StatusTypeDef MPU6050_Read_Len(uint8_t reg, uint8_t *buf, uint16_t len)
{
    return HAL_I2C_Mem_Read(&hi2c1, MPU6050_I2C_ADDR, reg,
                            I2C_MEMADD_SIZE_8BIT, buf, len,
                            MPU6050_I2C_TIMEOUT);
}

/**
 * @brief  检查 MPU6050 是否在线 (读取 WHO_AM_I)
 * @retval 返回 1 表示成功 (ID 正确), 返回 0 表示失败
 */
uint8_t MPU6050_Check(void)
{
    uint8_t who_am_i = 0;

    if (MPU6050_Read_Len(MPU6050_REG_WHO_AM_I, &who_am_i, 1) != HAL_OK)
    {
        return 0;
    }

    if (who_am_i == MPU6050_WHO_AM_I_VAL)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief  初始化 MPU6050 传感器
 * @retval 返回 1 表示成功, 返回 0 表示失败
 */
uint8_t MPU6050_Init(void)
{
    uint8_t who_am_i = 0;

    // 1. 唤醒设备，选择内部 8MHz 振荡器 (PWR_MGMT_1 复位值 0x40=SLEEP)
    MPU6050_Write_Reg(MPU6050_REG_PWR_MGMT_1, MPU6050_PWR_MGMT1_WAKE);
    vTaskDelay(pdMS_TO_TICKS(30));

    // 2. 再次确保退出睡眠：写 0x00
    MPU6050_Write_Reg(MPU6050_REG_PWR_MGMT_1, 0x00);
    vTaskDelay(pdMS_TO_TICKS(10));

    // 3. 检查 WHO_AM_I (唤醒后读取更可靠)
    if (MPU6050_Read_Len(MPU6050_REG_WHO_AM_I, &who_am_i, 1) != HAL_OK)
    {
        return 0;
    }

    if (who_am_i != MPU6050_WHO_AM_I_VAL)
    {
        return 0;
    }

    // 4. 配置采样率分频: SMPRT_DIV = 0 (不分频, 陀螺1kHz/加速度1kHz)
    MPU6050_Write_Reg(MPU6050_REG_SMPRT_DIV, MPU6050_SMPRT_DIV_VAL);
    vTaskDelay(pdMS_TO_TICKS(1));

    // 5. 配置 DLPF: 陀螺 188Hz, 加速度 184Hz (DLPF_CFG=1)
    MPU6050_Write_Reg(MPU6050_REG_CONFIG, MPU6050_CONFIG_DLPF_VAL);
    vTaskDelay(pdMS_TO_TICKS(1));

    // 6. 配置陀螺仪量程: 卤2000 dps (FS_SEL=3)
    MPU6050_Write_Reg(MPU6050_REG_GYRO_CONFIG, MPU6050_GYRO_FS_2000);
    vTaskDelay(pdMS_TO_TICKS(1));

    // 7. 配置加速度计量程: 卤8g (AFS_SEL=2, 无自检)
    MPU6050_Write_Reg(MPU6050_REG_ACCEL_CONFIG, MPU6050_ACCEL_FS_8G);
    vTaskDelay(pdMS_TO_TICKS(1));

    // 8. 禁用所有中断 (数据通过轮询读取, INT 引脚用于外部中断)
    MPU6050_Write_Reg(MPU6050_REG_INT_ENABLE, 0x00);
    vTaskDelay(pdMS_TO_TICKS(1));

    return 1;
}

/**
 * @brief  从 MPU6050 读取加速度和角速度数据
 * @retval None (数据存入全局 mpu_data)
 */
void MPU6050_Read_Data(void)
{
    uint8_t buf[14];

    // 1. 从 ACCEL_XOUT_H (0x3B) 开始连续读取 14 字节
    if (MPU6050_Read_Len(MPU6050_REG_ACCEL_XOUT_H, buf, 14) != HAL_OK)
    {
        return;
    }

    // 2. 拼接 16 位原始数据 (高 8 位在前，低 8 位在后)
    mpu_data.acc_x_raw  = (int16_t)((buf[0] << 8) | buf[1]);
    mpu_data.acc_y_raw  = (int16_t)((buf[2] << 8) | buf[3]);
    mpu_data.acc_z_raw  = (int16_t)((buf[4] << 8) | buf[5]);

    mpu_data.temp_raw   = (int16_t)((buf[6] << 8) | buf[7]);

    mpu_data.gyro_x_raw = (int16_t)((buf[8]  << 8) | buf[9]);
    mpu_data.gyro_y_raw = (int16_t)((buf[10] << 8) | buf[11]);
    mpu_data.gyro_z_raw = (int16_t)((buf[12] << 8) | buf[13]);

    // 3. 换算为物理单位
    mpu_data.acc_x = mpu_data.acc_x_raw / MPU6050_ACCEL_SENS_8G;
    mpu_data.acc_y = mpu_data.acc_y_raw / MPU6050_ACCEL_SENS_8G;
    mpu_data.acc_z = mpu_data.acc_z_raw / MPU6050_ACCEL_SENS_8G;

    mpu_data.gyro_x = mpu_data.gyro_x_raw / MPU6050_GYRO_SENS_2000;
    mpu_data.gyro_y = mpu_data.gyro_y_raw / MPU6050_GYRO_SENS_2000;
    mpu_data.gyro_z = mpu_data.gyro_z_raw / MPU6050_GYRO_SENS_2000;

    // 4. 温度换算: T = TEMP_OUT / 340 + 36.53 (癈)
    mpu_data.temperature = (mpu_data.temp_raw / 340.0f) + 36.53f;
}