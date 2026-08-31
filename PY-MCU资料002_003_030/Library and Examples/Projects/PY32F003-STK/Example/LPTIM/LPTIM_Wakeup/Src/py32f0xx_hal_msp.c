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
**函数信息 ：LPTIM MSP初始化
**功能描述 ：配置所需的硬件资源
**输入参数 ：LPTIM handle pointer
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_MspInit(void)
{
  BSP_LED_Init(LED3);
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);

  HAL_NVIC_SetPriority(LPTIM1_IRQn, 0x01,  0);                     //设置LPTIM中断优先级
  HAL_NVIC_EnableIRQ(LPTIM1_IRQn);                                 //使能LPTIM全局中断
}
