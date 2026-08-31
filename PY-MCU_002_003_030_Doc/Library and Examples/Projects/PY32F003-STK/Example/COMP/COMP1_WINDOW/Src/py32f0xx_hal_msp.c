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
**功能描述 ：初始化MSP
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_MspInit(void)
{
  BSP_LED_Init(LED_GREEN);
}
/********************************************************************************************************
**函数信息 ：void HAL_COMP_MspInit(COMP_HandleTypeDef *hcomp)
**功能描述 ：初始化COMP相关MSP
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_COMP_MspInit(COMP_HandleTypeDef *hcomp)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();                 //使能GPIOA时钟
  __HAL_RCC_COMP1_CLK_ENABLE();                 //使能COMP1时钟
  __HAL_RCC_COMP2_CLK_ENABLE();                 //使能COMP2时钟
  //GPIOPA1配置为模拟输入
  GPIO_InitTypeDef COMPINPUT;
  COMPINPUT.Pin = GPIO_PIN_1;
  COMPINPUT.Mode = GPIO_MODE_INPUT;            //模拟模式
  COMPINPUT.Speed = GPIO_SPEED_FREQ_HIGH;
  COMPINPUT.Pull = GPIO_PULLDOWN;                //下拉
  HAL_GPIO_Init(GPIOA,  &COMPINPUT);            //GPIO初始化

  //GPIOPA6配置为输出
  COMPINPUT.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  COMPINPUT.Mode = GPIO_MODE_AF_PP;             //输出
  COMPINPUT.Speed = GPIO_SPEED_FREQ_HIGH;
  COMPINPUT.Pull = GPIO_PULLDOWN;               //下拉
  COMPINPUT.Alternate = GPIO_AF7_COMP1;         //复用为COM1_OUT
  HAL_GPIO_Init(GPIOA,  &COMPINPUT);            //初始化GPIO
  HAL_NVIC_EnableIRQ(ADC_COMP_IRQn);            //使能COMP中断
  HAL_NVIC_SetPriority(ADC_COMP_IRQn, 1, 1);
}
