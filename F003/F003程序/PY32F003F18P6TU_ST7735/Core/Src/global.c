/**
  ******************************************************************************
  * @file    global.c
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.0
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   全局共享变量定义
  *
  * 设计说明：
  *   - 共享变量集中定义，global.h 用 extern 导出，禁止其它文件重复定义；
  *   - 全部 volatile：中断与主循环并发访问；
  *   - sys 为统一毫秒时钟（SysTick 每 1ms 加 1）。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 首次创建：移植自 G030，MAX_KEYS=3。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "global.h"

/* 全局变量定义 ------------------------------------------------------*/
volatile uint64_t sys = 0;                     /* 统一毫秒时钟 */

volatile uint8_t  lock[MAX_KEYS]      = {0};   /* 按键事件已消费锁 */
volatile uint64_t btn_last_irq[MAX_KEYS] = {0}; /* 按键中断时间戳 */
volatile uint8_t  longFlag[MAX_KEYS]  = {0};   /* 长按已触发标志 */

volatile uint8_t  test = 0;                    /* 调试计数 */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

