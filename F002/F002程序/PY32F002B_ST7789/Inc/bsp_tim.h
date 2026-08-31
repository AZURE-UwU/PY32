/**
  ******************************************************************************
  * @file    bsp_tim.h
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   TIM1 PWM 初始化与占空比接口
  *
  * 设计说明：
  *   - TIM1_CH1 = 风扇 PWM（需求中的"一路 PWM"）；
  *   - TIM1_CH2 = LCD 背光调光（原 G030 工程背光也是 PWM，同定时器复用）。
  *   - PWM 频率：24MHz/(23+1)/(99+1) = 10kHz，占空比 0~99。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 首次创建。
  ******************************************************************************
  */

#ifndef __BSP_TIM_H
#define __BSP_TIM_H

#include "main.h"

extern TIM_HandleTypeDef htim1;

void BSP_TIM1_PWM_Init(void);
void BSP_PWM_SetFan(uint8_t duty); /* 风扇占空比 0~99 */
void BSP_PWM_SetBLK(uint8_t duty); /* 背光占空比 0~99 */
void BSP_PWM_Start(void);          /* 启动两路 PWM 输出 */

#endif /* __BSP_TIM_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
