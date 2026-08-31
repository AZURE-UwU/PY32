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

static DMA_HandleTypeDef HdmaCh1;
static DMA_HandleTypeDef HdmaCh2;
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  BSP_PB_Init(BUTTON_USER,BUTTON_MODE_GPIO);
  
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
  __HAL_RCC_TIM1_CLK_ENABLE();                              //TIM1时钟使能
  __HAL_RCC_GPIOA_CLK_ENABLE();                             //GPIOA时钟使能
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;                   //复用功能
  GPIO_InitStruct.Pull = GPIO_PULLUP;                       //上拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  /*GPIOA3初始化 TIM1_CH1*/
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Alternate = GPIO_AF13_TIM1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  /*GPIOA13初始化 TIM1_CH2*/
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Alternate = GPIO_AF13_TIM1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  /*GPIOA0初始化  TIM1_CH3*/
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Alternate = GPIO_AF13_TIM1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  /*GPIOA1初始化 TIM1_CH4*/
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Alternate = GPIO_AF13_TIM1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
