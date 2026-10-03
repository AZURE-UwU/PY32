================================================================================
              PY32F003F18P6TU + ST7735  V0.5 速查
================================================================================
芯片：PY32F003F18P6TU（TSSOP20，64KB Flash / 8KB SRAM，最高 32MHz）
本版：V0.5（2026-09-06）PA0 电源使能 + 关机低功耗 + THEME 选项式
上一版：V0.4（掉电保存）

本版改了什么
  - PA0 改为电源使能：上电/运行输出高，关机拉低（不再作板上 LED）；
  - 新增 BSP_GPIO_LowPowerConfig()：关机时非必要引脚置模拟输入，
    PA0 拉低、背光/风扇拉低，只保留 SW_WKUP(PA12) 作唤醒源；
    power_off() 进 STOP 前停 SysTick、清 pending，唤醒后恢复三键并复位按键状态机；
  - THEME 改为选项式（IT_OPTION/UI_OPTION）：UP/DOWN 在 WHITE/BLACK 间循环，SET 确认；
    本版 g_cfg.theme 仍存背景颜色值（V0.6 改编号）。

菜单项
  FLIP / THEME(选项) / AUTO OFF / BLK / AOFF I / AOFF MIN / VBUS DIV / SAVE & EXIT / DISCARD

按键
  UP   (PA12)  主界面长按关机 ／ 菜单上移 ／ 值编辑 +步长 ／ 选项上一个
  SET  (PA6)   主界面短按进设置 ／ 切换勾选或进入编辑 ／ 确认
  DOWN (PA7)   主界面无功能 ／ 菜单下移 ／ 值编辑 -步长 ／ 选项下一个

引脚（本版起 PA0 定义变化）
  显示屏 SPI1 : SCK=PA1  MOSI=PA2  DC=PA3  CS=PF1  RST=PF4
  背光        : PF0 = TIM14_CH1 PWM，10kHz，无级调光
  INA226 I2C1 : SCL=PB6  SDA=PB7，100kHz，地址 0x40
  ADC         : PA5=IN5（VCC 分压）  PA4=IN4（NTC 分压）
  风扇 PWM    : PB5 = TIM3_CH2，10kHz
  电源使能    : PA0（推挽输出，上电高 / 关机低）
  调试 / 复位 : SWD=PA13/PA14，NRST=PF2

编译
  Keil MDK 打开 MDK-ARM/Project.uvprojx，需装 Puya.PY32F0xx_DFP.1.1.0；
  编译器 ARMCLANG（AC6）；SWD 烧录 PA13/PA14，NRST PF2。

文件版本
  main.c V1.7 / app_ui.c V1.5 / bsp_gpio.c V1.7 / function.c V1.8 /
  bsp_flash.c V1.0 / global.c V1.2 / st7735.c 1.2.1
================================================================================
