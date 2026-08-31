/**
  ******************************************************************************
  * @file    py32f002b_it.h
  * @author  Bowen (wbw20)
  * @date    2026-08-18
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   中断服务函数头文件
  *
  * 设计说明：
  *   - 只声明本工程实际用到的中断处理函数；
  *   - 中断函数本体在 py32f002b_it.c 中实现，原则是"中断里少干活"。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-18) 首次创建。
  ******************************************************************************
  */

/* 防止头文件重复包含 ------------------------------------------------*/
#ifndef __PY32F002B_IT_H
#define __PY32F002B_IT_H

#ifdef __cplusplus
extern "C" {
#endif

/* 中断处理函数声明 --------------------------------------------------*/
void NMI_Handler(void);
void HardFault_Handler(void);
void SVC_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __PY32F002B_IT_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
