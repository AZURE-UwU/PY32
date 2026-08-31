/**
  ******************************************************************************
  * @file    global.h
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.0
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   全局共享变量 extern 声明
  *
  * 设计说明：
  *   - 中断与应用层共享的变量统一在 global.c 定义，这里只做 extern 声明；
  *   - 共享变量一律 volatile；
  *   - sys 是统一毫秒时钟（SysTick 每 1ms 累加），按钮/滤波/积分都复用它。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 首次创建：移植自 G030 工程，MAX_KEYS 由 2 扩到 3。
  ******************************************************************************
  */

#ifndef __GLOBAL_H
#define __GLOBAL_H

#include <stdint.h>
#include "function.h"

extern volatile uint64_t sys;                    /* 全局毫秒时钟（1ms 分辨率） */
extern volatile uint8_t  lock[MAX_KEYS];         /* 按键事件已消费锁           */
extern volatile uint64_t btn_last_irq[MAX_KEYS]; /* 按键中断时间戳             */
extern volatile uint8_t  longFlag[MAX_KEYS];     /* 长按已触发标志             */
extern volatile uint8_t  test;                   /* 调试计数                   */

#endif /* __GLOBAL_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

