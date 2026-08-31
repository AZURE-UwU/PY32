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

#define TIMx                           TIM1
#define TIMx_CLK_ENABLE()              __HAL_RCC_TIM1_CLK_ENABLE()
#define TIMx_IRQn                      TIM1_BRK_UP_TRG_COM_IRQn


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

