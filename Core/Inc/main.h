/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define M3_LIN2_Pin GPIO_PIN_5
#define M3_LIN2_GPIO_Port GPIOE
#define M3_LIN3_Pin GPIO_PIN_6
#define M3_LIN3_GPIO_Port GPIOE
#define VBAT_SENSE_Pin GPIO_PIN_0
#define VBAT_SENSE_GPIO_Port GPIOC
#define IADC1_V_Pin GPIO_PIN_1
#define IADC1_V_GPIO_Port GPIOC
#define IADC1_U_Pin GPIO_PIN_2
#define IADC1_U_GPIO_Port GPIOC
#define IADC4_U_Pin GPIO_PIN_0
#define IADC4_U_GPIO_Port GPIOA
#define IADC4_V_Pin GPIO_PIN_1
#define IADC4_V_GPIO_Port GPIOA
#define IADC3_U_Pin GPIO_PIN_2
#define IADC3_U_GPIO_Port GPIOA
#define IADC3_V_Pin GPIO_PIN_3
#define IADC3_V_GPIO_Port GPIOA
#define IMU_NSS_Pin GPIO_PIN_4
#define IMU_NSS_GPIO_Port GPIOA
#define IMU_SCK_Pin GPIO_PIN_5
#define IMU_SCK_GPIO_Port GPIOA
#define IMU_MISO_Pin GPIO_PIN_6
#define IMU_MISO_GPIO_Port GPIOA
#define IMU_MOSI_Pin GPIO_PIN_7
#define IMU_MOSI_GPIO_Port GPIOA
#define IADC2_U_Pin GPIO_PIN_4
#define IADC2_U_GPIO_Port GPIOC
#define IADC2_V_Pin GPIO_PIN_5
#define IADC2_V_GPIO_Port GPIOC
#define M1_LIN1_Pin GPIO_PIN_0
#define M1_LIN1_GPIO_Port GPIOB
#define M1_LIN2_Pin GPIO_PIN_1
#define M1_LIN2_GPIO_Port GPIOB
#define M1_LIN3_Pin GPIO_PIN_8
#define M1_LIN3_GPIO_Port GPIOE
#define M3_HIN3_Pin GPIO_PIN_10
#define M3_HIN3_GPIO_Port GPIOB
#define M3_LIN1_Pin GPIO_PIN_11
#define M3_LIN1_GPIO_Port GPIOB
#define M2_LIN1_Pin GPIO_PIN_14
#define M2_LIN1_GPIO_Port GPIOB
#define M2_LIN2_Pin GPIO_PIN_15
#define M2_LIN2_GPIO_Port GPIOB
#define USART3_TX_Pin GPIO_PIN_8
#define USART3_TX_GPIO_Port GPIOD
#define USART3_RX_Pin GPIO_PIN_9
#define USART3_RX_GPIO_Port GPIOD
#define LORA_RXEN_Pin GPIO_PIN_10
#define LORA_RXEN_GPIO_Port GPIOD
#define M4_HIN1_Pin GPIO_PIN_12
#define M4_HIN1_GPIO_Port GPIOD
#define M4_HIN2_Pin GPIO_PIN_13
#define M4_HIN2_GPIO_Port GPIOD
#define M4_HIN3_Pin GPIO_PIN_14
#define M4_HIN3_GPIO_Port GPIOD
#define M4_LIN1_Pin GPIO_PIN_15
#define M4_LIN1_GPIO_Port GPIOD
#define M2_HIN1_Pin GPIO_PIN_6
#define M2_HIN1_GPIO_Port GPIOC
#define M2_HIN2_Pin GPIO_PIN_7
#define M2_HIN2_GPIO_Port GPIOC
#define M2_HIN3_Pin GPIO_PIN_8
#define M2_HIN3_GPIO_Port GPIOC
#define M2_LIN3_Pin GPIO_PIN_9
#define M2_LIN3_GPIO_Port GPIOC
#define M1_HIN1_Pin GPIO_PIN_8
#define M1_HIN1_GPIO_Port GPIOA
#define M1_HIN2_Pin GPIO_PIN_9
#define M1_HIN2_GPIO_Port GPIOA
#define M1_HIN3_Pin GPIO_PIN_10
#define M1_HIN3_GPIO_Port GPIOA
#define M3_HIN1_Pin GPIO_PIN_15
#define M3_HIN1_GPIO_Port GPIOA
#define LORA_SCK_Pin GPIO_PIN_10
#define LORA_SCK_GPIO_Port GPIOC
#define LORA_MISO_Pin GPIO_PIN_11
#define LORA_MISO_GPIO_Port GPIOC
#define LORA_MOSI_Pin GPIO_PIN_12
#define LORA_MOSI_GPIO_Port GPIOC
#define LORA_BUSY_Pin GPIO_PIN_0
#define LORA_BUSY_GPIO_Port GPIOD
#define IMU_INT_Pin GPIO_PIN_1
#define IMU_INT_GPIO_Port GPIOD
#define IMU_INT_EXTI_IRQn EXTI1_IRQn
#define LED_RED_Pin GPIO_PIN_2
#define LED_RED_GPIO_Port GPIOD
#define LED_BLUE_Pin GPIO_PIN_3
#define LED_BLUE_GPIO_Port GPIOD
#define LORA_NRST_Pin GPIO_PIN_4
#define LORA_NRST_GPIO_Port GPIOD
#define LORA_TXEN_Pin GPIO_PIN_5
#define LORA_TXEN_GPIO_Port GPIOD
#define LORA_NSS_Pin GPIO_PIN_6
#define LORA_NSS_GPIO_Port GPIOD
#define LORA_DIO0_Pin GPIO_PIN_7
#define LORA_DIO0_GPIO_Port GPIOD
#define LORA_DIO0_EXTI_IRQn EXTI9_5_IRQn
#define M3_HIN2_Pin GPIO_PIN_3
#define M3_HIN2_GPIO_Port GPIOB
#define I2C1_SCL_Pin GPIO_PIN_6
#define I2C1_SCL_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_7
#define I2C1_SDA_GPIO_Port GPIOB
#define M4_LIN2_Pin GPIO_PIN_8
#define M4_LIN2_GPIO_Port GPIOB
#define M4_LIN3_Pin GPIO_PIN_9
#define M4_LIN3_GPIO_Port GPIOB
#define BEEP_Pin GPIO_PIN_0
#define BEEP_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
