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
  GPIO_InitTypeDef COMPINPUT;

  COMPINPUT.Pin = GPIO_PIN_1;
  COMPINPUT.Mode = GPIO_MODE_ANALOG;
  COMPINPUT.Speed = GPIO_SPEED_FREQ_HIGH;
  COMPINPUT.Pull = GPIO_PULLDOWN;

  HAL_GPIO_Init(GPIOA,  &COMPINPUT);

  COMPINPUT.Pin = GPIO_PIN_6;
  COMPINPUT.Mode = GPIO_MODE_AF_PP;
  COMPINPUT.Speed = GPIO_SPEED_FREQ_HIGH;
  COMPINPUT.Pull = GPIO_PULLDOWN;
  COMPINPUT.Alternate = GPIO_AF7_COMP1;

  HAL_GPIO_Init(GPIOA,  &COMPINPUT);
}
