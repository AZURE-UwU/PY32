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

/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
}

/********************************************************************************************************
**函数信息 ：void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
**功能描述 ：初始化ADC相关MSP
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
  /* ADC1 Periph clock enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_ADC_CLK_ENABLE();
}



