/**
  ******************************************************************************
  * @file    py32f0xx_hal_msp.c
  * @author  MCU Application Team
  * @Version V1.0.0
  * @Date    2020-10-19
  * @brief   This file provides code for the MSP Initialization
  *          and de-Initialization codes.
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "main.h"


/********************************************************************************************************
**函数信息 ：void HAL_MspInit(void)
**功能描述 ：初始化全局MSP
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_MspInit(void)
{
  BSP_LED_Init(LED_GREEN);
  /* 清除所有复位标志位 */
  __HAL_RCC_CLEAR_RESET_FLAGS();
}
/********************************************************************************************************
**函数信息 ：void HAL_WWDG_MspInit(void)
**功能描述 ：初始化WWDG相关MSP
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_WWDG_MspInit(WWDG_HandleTypeDef *hwwdg)
{
  /* WWDG Peripheral clock enable */
  __HAL_RCC_WWDG_CLK_ENABLE();
  NVIC_SetPriority(WWDG_IRQn, 0);
  NVIC_EnableIRQ(WWDG_IRQn);
}



