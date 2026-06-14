#ifndef __APP_FREERTOS_H
#define __APP_FREERTOS_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "stdio.h"
#include "string.h"
#include <math.h>
#include <stdio.h>

#include "main.h"
#include "BSP_USART.h"
#include "BSP_MPU6050.h"
#include "Alg_FOC_Math.h"
#include "Alg_MPU6050_Data.h"
#include "Alg_SPL06_Data.h"
#include "BSP_SPL06.h"
#include "App_Motor_Ctrl.h"



/**
 * @brief 鍚姩FreeRTOS浠诲姟
 * 
 */
void App_FreeRTOS_start(void);


#endif
