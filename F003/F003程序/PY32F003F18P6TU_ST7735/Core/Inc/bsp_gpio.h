/**
  ******************************************************************************
  * @file    bsp_gpio.h
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.1
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   GPIO 初始化与按键/LED 底层接口
  *
 * CHANGELOG:
 *   V1.0 (2026-08-19) 首次创建。
 *   V1.1 (2026-08-24) 增加 BSP_BtnWakeConfig / BSP_BtnRestoreConfig：
 *                     关机时只保留 SW_WKUP 上升沿唤醒，唤醒后恢复三键双沿。
  ******************************************************************************
  */

#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "main.h"

void BSP_GPIO_Init(void);          /* LCD 控制脚 / LED / 按键（EXTI）初始化 */
uint8_t BSP_BtnRead(uint8_t index);/* 读按键稳定电平：0=松开，1=按下         */
void BSP_LED_Toggle(void);         /* 翻转状态指示灯                        */
void BSP_BtnWakeConfig(void);      /* 仅 SW_WKUP 按下(上升沿)可唤醒 STOP    */
void BSP_BtnRestoreConfig(void);   /* 恢复三个按键双沿中断配置               */

#endif /* __BSP_GPIO_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

