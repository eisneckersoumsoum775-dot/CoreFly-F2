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
#include "Alg_FOC_Math.h"
#include "App_Motor_Ctrl.h"



/**
 * @brief 启动FreeRTOS任务
 * 
 */
void App_FreeRTOS_start(void);


#endif
