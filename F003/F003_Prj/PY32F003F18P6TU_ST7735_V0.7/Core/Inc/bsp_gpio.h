/**
  ******************************************************************************
  * @file    bsp_gpio.h
  * @author  Bowen (wbw20)
  * @date    2026-09-06
  * @version V1.2
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   GPIO 初始化与按键/LED 底层接口
  *
 * CHANGELOG:
 *   V1.0 (2026-08-19) 首次创建。
 *   V1.1 (2026-08-24) 增加 BSP_BtnWakeConfig / BSP_BtnRestoreConfig：
 *                     关机时只保留 SW_WKUP 上升沿唤醒，唤醒后恢复三键双沿。
 *   V1.2 (2026-09-06) PA0 改为电源使能（上电高/关机低）；新增 BSP_GPIO_LowPowerConfig：
 *                     关机时非必要引脚置模拟输入，PA0 拉低，仅保留唤醒键。
  ******************************************************************************
  */

#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "main.h"

void BSP_GPIO_Init(void);          /* LCD 控制脚 / 电源使能 / 按键（EXTI）初始化 */
uint8_t BSP_BtnRead(uint8_t index);/* 读按键稳定电平：0=松开，1=按下         */
void BSP_BtnRestoreConfig(void);   /* 恢复三个按键双沿中断配置               */
void BSP_GPIO_LowPowerConfig(void);/* 关机：非必要引脚模拟输入，PA0拉低，仅留唤醒键 */

#endif /* __BSP_GPIO_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

