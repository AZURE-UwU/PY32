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
}
/********************************************************************************************************
**函数信息 ：void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
**功能描述 ：初始化TIM相关MSP
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  GPIO_InitTypeDef   GPIO_InitStruct;
  __HAL_RCC_GPIOA_CLK_ENABLE();                  //使能GPIOA时钟
  __HAL_RCC_TIM3_CLK_ENABLE();                   //使能TIM3时钟
  __HAL_RCC_TIM1_CLK_ENABLE();                   //使能TIM1时钟
  /*GPIOA3初始化TIM1_CH1*/
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF13_TIM1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);        //设置中断优先级
  HAL_NVIC_EnableIRQ(TIM3_IRQn);                //使能TIM1中断
}
