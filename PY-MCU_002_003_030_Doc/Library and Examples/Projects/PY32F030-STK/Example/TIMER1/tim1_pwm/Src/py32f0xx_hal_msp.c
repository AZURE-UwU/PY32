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
  /*GPIOA8初始化*/
  GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL1;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
  HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL1, &GPIO_InitStruct);
  /*GPIOA9初始化*/
  GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL2;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
  HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL2, &GPIO_InitStruct);
  /*GPIOA10初始化*/
  GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL3;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
  HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL3, &GPIO_InitStruct);
  /*GPIOA11初始化*/
  GPIO_InitStruct.Pin = TIMx_GPIO_PIN_CHANNEL4;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
  HAL_GPIO_Init(TIMx_GPIO_PORT_CHANNEL4, &GPIO_InitStruct);
}
