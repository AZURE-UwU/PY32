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
#include "py32f003xx_Start_Kit.h"

/* Private includes ----------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

#define TIMx                           TIM3
#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM3_CLK_ENABLE()
#define DMAx_CLK_ENABLE                  __HAL_RCC_DMA_CLK_ENABLE

//#define TIMx_IRQn                      TIM1_BRK_UP_TRG_COM_IRQn


#define TIMx_CHANNEL_GPIO_PORT()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define TIMx_GPIO_PORT_CHANNEL1        GPIOB
#define TIMx_GPIO_PORT_CHANNEL2        GPIOA
#define TIMx_GPIO_PORT_CHANNEL3        GPIOA
#define TIMx_GPIO_PORT_CHANNEL4        GPIOA
#define TIMx_GPIO_PIN_CHANNEL1         GPIO_PIN_4
#define TIMx_GPIO_PIN_CHANNEL2         GPIO_PIN_9
#define TIMx_GPIO_PIN_CHANNEL3         GPIO_PIN_10
#define TIMx_GPIO_PIN_CHANNEL4         GPIO_PIN_11

/* Definition for TIMx's DMA */
#define TIMx_CC1_DMA_INST                DMA1_Channel1

/* Definition for ADCx's NVIC */
#define TIMx_DMA_IRQn                    DMA1_Channel1_IRQn
#define TIMx_DMA_IRQHandler              DMA1_Channel1_IRQHandler
extern uint32_t CC1_Capture;

/* Exported functions prototypes ---------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

