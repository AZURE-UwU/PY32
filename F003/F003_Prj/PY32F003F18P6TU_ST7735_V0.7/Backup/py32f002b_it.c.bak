/**
  ******************************************************************************
  * @file    py32f002b_it.c
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   中断服务函数
  *
  * 设计说明（前后台）：
  *   - SysTick：只维护毫秒时钟 sys 与 HAL tick，不干别的；
  *   - EXTI：只记录按键时间戳，消抖与按键逻辑放 50ms 周期任务；
  *   - 中断里尽量少干活。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 首次创建：G030 的 TIM14/TIM16/DMA 中断改为
  *                     SysTick + EXTI 方案（PY32F002B 无 DMA，HAL 未实现 TIM14）。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "main.h"
#include "py32f002b_it.h"
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
  * @brief  EXTI0_1 中断：SW_MODE（PC1）
  */
void EXTI0_1_IRQHandler(void)
{
  test++;
  HAL_GPIO_EXTI_IRQHandler(SW_MODE_Pin);
}

/**
  * @brief  EXTI2_3 中断：SW_FUNC（PB2）
  */
void EXTI2_3_IRQHandler(void)
{
  test++;
  HAL_GPIO_EXTI_IRQHandler(SW_FUNC_Pin);
}

/**
  * @brief  EXTI4_15 中断：SW_WKUP（PB5），也是 STOP 唤醒键
  */
void EXTI4_15_IRQHandler(void)
{
  test++;
  HAL_GPIO_EXTI_IRQHandler(SW_WKUP_Pin);
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
