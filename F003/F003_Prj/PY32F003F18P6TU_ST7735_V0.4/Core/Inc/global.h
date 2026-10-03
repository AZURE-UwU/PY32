/**
  ******************************************************************************
  * @file    global.h
  * @author  Bowen (wbw20)
  * @date    2026-08-31
  * @version V1.2
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
  *   V1.1 (2026-08-31) 新增 Settings_t 用户配置结构体：屏幕方向/主题/背光/
  *                     自动关机参数/INA226 母线分压微调统一收纳。
  *   V1.2 (2026-08-31) FLAG 由 main.c 上收为全局共享：设置菜单模块需要触发整屏重绘。
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
extern volatile uint8_t  FLAG;                   /* 整屏重绘标志               */

/* ---------- 用户可调配置（设置界面修改，掉电保存） ---------- */
/* 主循环专用、无中断共享，因此不加 volatile；
   g_cfg 为"当前生效值"，设置界面后续会再引入 g_cfg_edit 作为编辑副本 */
typedef struct {
    uint8_t   flip;          /* 屏幕方向 1/3                */
    uint16_t  theme;         /* 主题背景色（RGB565）        */
    uint8_t   blk;           /* 背光亮度 0~100             */
    uint8_t   auto_off;      /* 自动关机使能 1=开 0=关     */
    float     auto_off_i;    /* 判停电流阈值（A）          */
    uint16_t  auto_off_min;  /* 判停时长（分钟）           */
    float     vbus_div;      /* INA226 母线分压系数（微调） */
} Settings_t;

extern Settings_t g_cfg;                            /* 当前生效的用户配置 */

#endif /* __GLOBAL_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

