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
  DEBUG_USART_Config();
}
/********************************************************************************************************
**函数信息 ：void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim)
**功能描述 ：初始化TIM相关MSP
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim)
{
  GPIO_InitTypeDef   GPIO_InitStruct;
  __HAL_RCC_TIM1_CLK_ENABLE();                  //TIM3时钟使能
  __HAL_RCC_GPIOA_CLK_ENABLE();                 //GPIOA时钟使能

  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;       //模拟输出
  GPIO_InitStruct.Pull = GPIO_PULLUP;           //上拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;    //复用为TIM1

  GPIO_InitStruct.Pin = GPIO_PIN_8;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);       //PA8初始化

//    GPIO_InitStruct.Pin = GPIO_PIN_9;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);       //A9初始化
}
