/**
  ******************************************************************************
  * @file    bsp_pa1.c
  * @author  Bowen (wbw20)
  * @date    2026-08-18
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   PA1 输出控制模块：初始化 / 置高 / 置低 / 翻转
  *
  * 设计说明：
  *   - 一个外设一个模块：PA1 的全部操作收在本文件，main 只调 API；
  *   - 分层调用：HAL 底层 -> 本模块 API -> main 应用，职责清晰；
  *   - 防御性编程：入口参数做校验，非法电平直接忽略。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-18) 首次创建：PA1 推挽输出，默认低电平。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "bsp_pa1.h"

/**
  * @brief  初始化 PA1 为推挽输出，并输出默认低电平
  * @param  无
  * @retval 无
  */
void BSP_PA1_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 使用 GPIOA 前必须先打开 GPIOA 外设时钟，否则寄存器写不进去 */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  GPIO_InitStruct.Pin   = GPIO_PIN_1;                 /* 目标引脚：PA1            */
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;        /* 推挽输出：可输出高/低电平 */
  GPIO_InitStruct.Pull  = GPIO_NOPULL;                /* 推挽输出不需要上下拉      */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;        /* 1s 低频翻转，低速足够，且更省电 */

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* 上电默认输出低电平，避免 LED/负载在上电瞬间误动作 */
  BSP_PA1_Set(PA1_OFF);
}

/**
  * @brief  设置 PA1 输出电平
  * @param  level: PA1_ON（高）/ PA1_OFF（低）
  * @retval 无
  */
void BSP_PA1_Set(uint8_t level)
{
  /* 防御：只接受合法电平值，非法参数直接忽略，不写入硬件 */
  if ((level != PA1_ON) && (level != PA1_OFF))
  {
    return;
  }

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, level);
}

/**
  * @brief  翻转 PA1 输出电平（高变低 / 低变高）
  * @param  无
  * @retval 无
  */
void BSP_PA1_Toggle(void)
{
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
