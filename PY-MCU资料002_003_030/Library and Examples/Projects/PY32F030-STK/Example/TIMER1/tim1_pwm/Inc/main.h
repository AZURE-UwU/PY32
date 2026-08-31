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




#define TIMx_CHANNEL_GPIO_PORT()       __HAL_RCC_GPIOA_CLK_ENABLE()
#define TIMx_GPIO_PORT_CHANNEL1        GPIOA
#define TIMx_GPIO_PORT_CHANNEL2        GPIOA
#define TIMx_GPIO_PORT_CHANNEL3        GPIOA
#define TIMx_GPIO_PORT_CHANNEL4        GPIOA
#define TIMx_GPIO_PIN_CHANNEL1         GPIO_PIN_8
#define TIMx_GPIO_PIN_CHANNEL2         GPIO_PIN_9
#define TIMx_GPIO_PIN_CHANNEL3         GPIO_PIN_10
#define TIMx_GPIO_PIN_CHANNEL4         GPIO_PIN_11

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

