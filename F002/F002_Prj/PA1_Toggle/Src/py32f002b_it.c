/**
  ******************************************************************************
  * @file    py32f002b_it.c
  * @author  Bowen (wbw20)
  * @date    2026-08-18
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   中断服务函数实现
  *
  * 设计说明：
  *   - 前后台架构：中断（前台）只做最少的活，记录时钟/事件；
  *   - PA1 翻转属于"周期任务"，放在主循环（后台）判断执行，
  *     不在中断里做，避免拖长中断响应时间。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-18) 首次创建：SysTick 维护全局毫秒时钟 sys。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "main.h"
#include "py32f002b_it.h"
#include "global.h"

/**
  * @brief  不可屏蔽中断（NMI）处理
  */
void NMI_Handler(void)
{
}

/**
  * @brief  硬件错误中断处理：程序跑飞时停在这里，便于调试定位
  */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}

/**
  * @brief  系统服务调用（SVC）处理
  */
void SVC_Handler(void)
{
}

/**
  * @brief  可挂起系统服务（PendSV）处理
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  系统滴答定时器中断：每 1ms 触发一次
  *
  * 中断里只做两件事：
  *   1. sys++：维护全局毫秒时钟（应用统一时间基准）；
  *   2. HAL_IncTick()：喂 HAL 库自己的 tick，保证 HAL_Delay 可用。
  * 翻转判断等业务逻辑一律放到主循环，符合"中断里少干活"原则。
  */
void SysTick_Handler(void)
{
  sys++;              /* 全局毫秒时钟：所有应用计时都基于它 */
  HAL_IncTick();      /* HAL 库 tick：HAL_Delay 等函数依赖它 */
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
