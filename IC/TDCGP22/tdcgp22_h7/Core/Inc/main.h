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
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "getL.h"
#include "tdc.h"


#include "arm_math.h"
#include "arm_const_structs.h"
#include <stdio.h>
#include <string.h>
#include "math.h"
#include "stdlib.h"	
#include <stdbool.h>
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
#define TDC_SSN_Pin GPIO_PIN_2
#define TDC_SSN_GPIO_Port GPIOE
#define TDC_SCK_Pin GPIO_PIN_3
#define TDC_SCK_GPIO_Port GPIOE
#define TDC_SI_Pin GPIO_PIN_4
#define TDC_SI_GPIO_Port GPIOE
#define TDC_SO_Pin GPIO_PIN_5
#define TDC_SO_GPIO_Port GPIOE
#define TDC_RTN_Pin GPIO_PIN_6
#define TDC_RTN_GPIO_Port GPIOE
#define TDC_INT_Pin GPIO_PIN_13
#define TDC_INT_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
