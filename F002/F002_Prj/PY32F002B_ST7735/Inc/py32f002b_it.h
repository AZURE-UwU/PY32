/**
  ******************************************************************************
  * @file    py32f002b_it.h
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   中断服务函数声明
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 首次创建。
  ******************************************************************************
  */

#ifndef __PY32F002B_IT_H
#define __PY32F002B_IT_H

#ifdef __cplusplus
extern "C" {
#endif

void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void EXTI0_1_IRQHandler(void);
void EXTI2_3_IRQHandler(void);
void EXTI4_15_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif /* __PY32F002B_IT_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
