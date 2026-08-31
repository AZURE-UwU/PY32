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
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);
  BSP_LED_Init(LED3);
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
  //GPIOPA01配置
  GPIO_InitTypeDef COMPINPUT;

  COMPINPUT.Pin = GPIO_PIN_1;
  COMPINPUT.Mode = GPIO_MODE_ANALOG;     //模拟模式
  COMPINPUT.Speed = GPIO_SPEED_FREQ_HIGH;
  COMPINPUT.Pull = GPIO_PULLDOWN;        //下拉

  HAL_GPIO_Init(GPIOA,  &COMPINPUT);     //GPIOA初始化

  COMPINPUT.Pin = GPIO_PIN_6;
  COMPINPUT.Mode = GPIO_MODE_AF_PP;      //复用推挽输出
  COMPINPUT.Speed = GPIO_SPEED_FREQ_HIGH;
  COMPINPUT.Pull = GPIO_PULLDOWN;        //下拉
  COMPINPUT.Alternate = GPIO_AF7_COMP1;  //复用COMP1A输出功能

  HAL_GPIO_Init(GPIOA,  &COMPINPUT);     //GPIOA初始化
}
