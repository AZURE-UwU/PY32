/**
  ******************************************************************************
  * @file    main.h
  * @author  MCU Application Team
  * @Version V1.0.0
  * @Date    
  * @brief   Header for main.c file.
  *          This file contains the common defines of the application.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "py32f0xx_hal.h"
#include "py32f030xx_Start_Kit.h"

/* Private includes ----------------------------------------------------------*/


/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
#define LEDx_PIN                           GPIO_PIN_0
#define LEDx_GPIO_PORT                     GPIOA
//#define LEDx_GPIO_CLK_ENABLE()             __HAL_RCC_GPIOA_CLK_ENABLE()

/**
  * @brief Key push-button
  */

#define EXTIx_LINE                   EXTI_LINE_0
#define EXTIx_IRQn                   EXTI4_15_IRQn
#define EXTIx_IRQHANDLER             EXTI4_15_IRQHandler

/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

