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
  GPIO_InitTypeDef gpioinitstruct;
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

  gpioinitstruct.Pin = GPIO_PIN_15;
  gpioinitstruct.Pull = GPIO_NOPULL;
  gpioinitstruct.Speed = GPIO_SPEED_FREQ_HIGH;

  /* Configure Button pin as input */
  gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;

  HAL_GPIO_Init(GPIOA, &gpioinitstruct);
}






