#ifndef __ALG_SPL06_DATA_H__
#define __ALG_SPL06_DATA_H__

#include <stdint.h>
#include <math.h>

/**
 * @brief SPL06 气压/高度数据结构体
 */
typedef struct {
    float temperature;      /* 温度 (C) */
    float pressure;         /* 气压 (Pa) */
    float altitude;         /* 海拔高度 (m) */

    /* 海平面参考气压 (Pa), 默认 101325 */
    float p0;
} SPL06_Altitude_t;

/**
 * @brief  初始化
 * @param  att: 数据结构体指针
 * @param  p0:  海平面参考气压 (Pa), 常用 101325
 */
void Alg_SPL06_Init(SPL06_Altitude_t *att, float p0);

/**
 * @brief  气压 -> 实时海拔 + 温度
 * @param  att:        数据结构体指针
 * @param  pressure_pa: 气压 (Pa)
 * @param  temp_c:      温度 (C)
 */
void Alg_SPL06_Update(SPL06_Altitude_t *att, float pressure_pa, float temp_c);

#endif