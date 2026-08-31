/**
  ******************************************************************************
  * @file    global.h
  * @author  Bowen (wbw20)
  * @date    2026-08-18
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   全局共享变量声明（extern）
  *
  * 设计说明：
  *   - 中断与应用层共享的变量统一在 global.c 定义，本头文件 extern 声明；
  *   - 其它模块只 include 本头文件，禁止自行重复定义全局变量；
  *   - 共享变量一律 volatile，防止优化器缓存旧值。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-18) 首次创建。
  ******************************************************************************
  */

/* 防止头文件重复包含 ------------------------------------------------*/
#ifndef __GLOBAL_H
#define __GLOBAL_H

#include <stdint.h>

extern volatile uint32_t sys;   /* 全局毫秒时钟：SysTick 每 1ms 加 1，所有计时复用 */

#endif /* __GLOBAL_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
