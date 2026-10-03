/**
  ******************************************************************************
  * @file    bsp_tim.h
  * @author  Bowen (wbw20)
  * @date    2026-08-29
  * @version V1.2
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   TIM3 PWM 初始化与占空比接口
  *
  * 设计说明：
  *   - TIM3_CH2(PB5) = 风扇 PWM（需求中的"一路 PWM"）；
  *   - TIM14_CH1(PF0) = 背光 PWM 无级调光。
  *   - PWM 频率：24MHz/(23+1)/(99+1) = 10kHz，占空比 0~99。
  *
 * CHANGELOG:
 *   V1.0 (2026-08-19) 首次创建（TIM1 双通道）；
 *   V1.1 (2026-08-29) 移植 PY32F003F18P6TU：改 TIM3_CH2(PB5)，去掉背光通道。
 *   V1.2 (2026-08-29) PF 引脚复用后恢复背光：TIM14_CH1(PF0)。
  ******************************************************************************
  */

#ifndef __BSP_TIM_H
#define __BSP_TIM_H

#include "main.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim14;

void BSP_TIM3_PWM_Init(void);   /* 风扇 PWM：TIM3_CH2(PB5) */
void BSP_TIM14_PWM_Init(void);  /* 背光 PWM：TIM14_CH1(PF0) */
void BSP_PWM_SetFan(uint8_t duty); /* 风扇占空比 0~99 */
void BSP_PWM_SetBLK(uint8_t duty); /* 背光占空比 0~99 */
void BSP_PWM_Start(void);          /* 启动风扇+背光 PWM */

#endif /* __BSP_TIM_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
