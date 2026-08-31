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

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

#define DMA_INSTANCE               DMA_Channel2
#define DMA_INSTANCE_IRQ           DMA1_Channel2_3_IRQn
#define DMA_INSTANCE_IRQHANDLER    DMA1_Channel1_IRQHandler

#define BUFFER_SIZE              32
/* Private defines -----------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

