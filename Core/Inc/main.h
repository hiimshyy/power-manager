/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "daly_bms.h"
#include "sk60x.h"
#include "ina219.h"
#include "modbus_rtu.h"
#include "charge_control.h"
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
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define LED_MB_Pin GPIO_PIN_14
#define LED_MB_GPIO_Port GPIOC
#define LED_FAULT_Pin GPIO_PIN_15
#define LED_FAULT_GPIO_Port GPIOC
#define LED_BMS_Pin GPIO_PIN_0
#define LED_BMS_GPIO_Port GPIOA
#define LED_SK_Pin GPIO_PIN_1
#define LED_SK_GPIO_Port GPIOA
#define RL_CHG_Pin GPIO_PIN_6
#define RL_CHG_GPIO_Port GPIOA
#define RL_12V_Pin GPIO_PIN_7
#define RL_12V_GPIO_Port GPIOA
#define RL_5V_Pin GPIO_PIN_0
#define RL_5V_GPIO_Port GPIOB
#define RL_3V3_Pin GPIO_PIN_1
#define RL_3V3_GPIO_Port GPIOB
#define INP_4_Pin GPIO_PIN_8
#define INP_4_GPIO_Port GPIOA
#define INP_3_Pin GPIO_PIN_15
#define INP_3_GPIO_Port GPIOA
#define INP_2_Pin GPIO_PIN_3
#define INP_2_GPIO_Port GPIOB
#define INP_1_Pin GPIO_PIN_4
#define INP_1_GPIO_Port GPIOB
#define FAULT_OUT_Pin GPIO_PIN_5
#define FAULT_OUT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
