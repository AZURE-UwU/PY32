/**
  ******************************************************************************
  * @file    py32f002b_hal_msp.c
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   HAL 全局底层初始化（芯片级时钟）
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 首次创建。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "main.h"

/**
  * @brief  全局 MSP 初始化（HAL_Init 内部调用）
  */
void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();  /* EXTI 复用选择等功能需要 */
  __HAL_RCC_PWR_CLK_ENABLE();     /* STOP 低功耗模式需要     */
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
