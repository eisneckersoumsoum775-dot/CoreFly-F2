#include "Alg_SPL06_Data.h"

/** 国际气压高度公式:
 *  Altitude = 44330 * [1 - (P / P0)^(1/5.255)]
 */
void Alg_SPL06_Init(SPL06_Altitude_t *att, float p0)
{
    att->temperature = 0.0f;
    att->pressure    = 0.0f;
    att->altitude    = 0.0f;
    att->p0          = p0;
}

void Alg_SPL06_Update(SPL06_Altitude_t *att, float pressure_pa, float temp_c)
{
    att->temperature = temp_c;
    att->pressure    = pressure_pa;

    if (pressure_pa > 0.0f && att->p0 > 0.0f)
    {
        att->altitude = 44330.0f * (1.0f - powf(pressure_pa / att->p0, 1.0f / 5.255f));
    }
    else
    {
        att->altitude = 0.0f;
    }
}