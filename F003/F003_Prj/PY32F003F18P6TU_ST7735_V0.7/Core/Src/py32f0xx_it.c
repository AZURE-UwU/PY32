/**
  ******************************************************************************
  * @file    py32f0xx_it.c
  * @author  Bowen (wbw20)
  * @date    2026-08-29
  * @version V1.0
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   中断服务函数
  *
  * 设计说明（前后台）：
  *   - SysTick：只维护毫秒时钟 sys 与 HAL tick；
  *   - EXTI4_15：三个按键都在该中断（PA6/PA7/PA12），逐个分发，
  *     回调里只记录时间戳，消抖与按键逻辑放 50ms 周期任务。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-29) 从 PY32F002B_ST7735 移植，按键改为 PA6/PA7/PA12
  *                     共用 EXTI4_15 一条中断线。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "main.h"
#include "py32f0xx_it.h"
#include "global.h"

/**
  * @brief  不可屏蔽中断
  */
void NMI_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief  硬件错误中断：停在这里便于调试定位
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief  系统服务调用
  */
void SVC_Handler(void)
{
}

/**
  * @brief  可挂起系统服务
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  SysTick 中断：每 1ms 一次，统一时间基准
  */
void SysTick_Handler(void)
{
  sys++;
  HAL_IncTick();
}

/**
  * @brief  EXTI4_15 中断：三个按键（PA6/PA7/PA12）共用
  */
void EXTI4_15_IRQHandler(void)
{
  /* 三个按键挂在同一中断向量上，逐引脚分发；未触发的引脚会被 HAL 跳过 */
  HAL_GPIO_EXTI_IRQHandler(SW_WKUP_Pin);
  HAL_GPIO_EXTI_IRQHandler(SW_FUNC_Pin);
  HAL_GPIO_EXTI_IRQHandler(SW_MODE_Pin);
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

