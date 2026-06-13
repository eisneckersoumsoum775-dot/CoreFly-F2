/**
  ******************************************************************************
  * @file    BSP_USART.h
  * @brief   串口通信与 VOFA+ 上位机测试接口头文件
  ******************************************************************************
  */

#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "usart.h"
#include <stdio.h>
#include <string.h>

/* 外部互斥量声明，保护 printf / UART 发送的线程安全 */
#include "FreeRTOS.h"
#include "semphr.h"
extern SemaphoreHandle_t USART_Mutex;

/* 供 stm32f4xx_it.c 调用的 UART3 中断处理函数 */
void BSP_USART3_IRQHandler(void);

#endif /* __BSP_USART_H */
