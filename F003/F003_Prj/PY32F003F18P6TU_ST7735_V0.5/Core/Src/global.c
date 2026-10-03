/**
  ******************************************************************************
  * @file    global.c
  * @author  Bowen (wbw20)
  * @date    2026-08-31
  * @version V1.2
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
  *   V1.1 (2026-08-31) 新增 g_cfg 用户配置：收纳屏幕方向/主题/背光/自动关机/
  *                     INA226 母线分压微调，默认值等于迁移前 main.c 散变量的初值。
  *   V1.2 (2026-08-31) FLAG 上收至此：main.c 与设置菜单模块共享整屏重绘标志。
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
volatile uint8_t  FLAG = 1;                    /* 整屏重绘标志（上电置 1，触发首次绘制） */

/* 用户可调配置（主循环专用，无中断共享，故不加 volatile）。
   默认值与原 main.c 里 FLIP=1 / BG=BLACK / blk=100、
   无负载 15mA×15min 自动关机完全一致，上电行为不变。 */
Settings_t g_cfg = {
    .flip         = 1,
    .theme        = 0x0000,      /* BLACK */
    .blk          = 100,
    .auto_off     = 1,
    .auto_off_i   = 0.015f,
    .auto_off_min = 15,
    .vbus_div     = 1.0f         /* 默认直通，实际分压比需按硬件改 */
};

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

