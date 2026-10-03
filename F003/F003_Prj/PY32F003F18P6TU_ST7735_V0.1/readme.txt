================================================================================
              PY32F003F18P6TU + ST7735  V0.1 速查
================================================================================
芯片：PY32F003F18P6TU（TSSOP20，64KB Flash / 8KB SRAM，最高 32MHz）
本版：V0.1（2026-08-31）用户配置结构体迁移
基线：PY32F003F18P6TU_ST7735_Base（2026-08-29 由 PY32F002B_ST7735 移植）

本版改了什么
  - 新增 Settings_t，定义在 Core/Inc/global.h，实例 g_cfg 在 Core/Src/global.c；
    字段：flip / theme / blk / auto_off / auto_off_i / auto_off_min / vbus_div；
  - 原 FLIP / BG / blk 裸变量删除，引用直接改成 g_cfg.flip / .theme / .blk；
  - 上电行为与 Base 完全一致（纯数据搬家，不改逻辑）。

本版还没有
  - 设置菜单（V0.2 才有）；掉电保存（V0.4 才有）。

引脚（全系列通用）
  显示屏 SPI1 : SCK=PA1  MOSI=PA2  DC=PA3  CS=PF1  RST=PF4
  背光        : PF0 = TIM14_CH1 PWM，10kHz，无级调光
  INA226 I2C1 : SCL=PB6  SDA=PB7，100kHz，地址 0x40
  ADC         : PA5=IN5（VCC 分压）  PA4=IN4（NTC 分压）
  风扇 PWM    : PB5 = TIM3_CH2，10kHz
  按键        : SW_WKUP=PA12 / SW_FUNC=PA6 / SW_MODE=PA7（高电平有效）
  板上 LED    : PA0（本版仍是 LED；V0.5 起改为电源使能）
  调试 / 复位 : SWD=PA13/PA14，NRST=PF2

按键（本版：与 Base 相同）
  SW_WKUP  短按转屏 / 长按关机进 STOP（再按唤醒）/ 双击换背景
  SW_FUNC  短按 LED 反馈 / 长按循环调背光
  SW_MODE  短按翻页 / 长按切换模式

编译
  Keil MDK 打开 MDK-ARM/Project.uvprojx，需装 Puya.PY32F0xx_DFP.1.1.0；
  编译器 ARMCLANG（AC6）；SWD 烧录 PA13/PA14，NRST PF2。

文件版本
  main.c V1.3 / global.c V1.1 / function.c V1.4 / bsp_gpio.c V1.5 / st7735.c 1.2.1
================================================================================
