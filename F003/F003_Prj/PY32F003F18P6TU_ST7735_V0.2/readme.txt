================================================================================
              PY32F003F18P6TU + ST7735  V0.2 速查
================================================================================
芯片：PY32F003F18P6TU（TSSOP20，64KB Flash / 8KB SRAM，最高 32MHz）
本版：V0.2（2026-09-02）新增二级设置菜单
上一版：V0.1（用户配置结构体迁移）

本版改了什么
  - 新增 Core/Src/app_ui.c + Core/Inc/app_ui.h；
  - 三状态 UI 状态机：UI_MAIN / UI_MENU / UI_VALUE；
  - 菜单查表描述（MenuItem_t），勾选项 SET 切换，数值项 UP/DOWN 加减、SET 确认；
  - 编辑落在工作副本 g_cfg_edit：SAVE & EXIT 才写回 g_cfg，DISCARD 丢弃（整页回滚）；
  - 主界面简化为"长按 UP 关机、短按 SET 进设置、DOWN 无功能"。

菜单项（本版）
  FLIP / THEME / AUTO OFF / BLK / AOFF I / AOFF MIN / VBUS DIV / SAVE & EXIT / DISCARD

本版还没有
  - 掉电保存（Settings_Save 只有 RAM 逻辑，留 TODO；V0.4 才写 Flash）；
  - 长按连跳（V0.3 才有）；THEME 还是勾选式（V0.5 改选项式）。

按键
  UP   (PA12)  主界面长按关机 ／ 菜单上移 ／ 值编辑 +步长
  SET  (PA6)   主界面短按进设置 ／ 切换勾选或进入编辑 ／ 确认
  DOWN (PA7)   主界面无功能 ／ 菜单下移 ／ 值编辑 -步长

引脚（全系列通用）
  显示屏 SPI1 : SCK=PA1  MOSI=PA2  DC=PA3  CS=PF1  RST=PF4
  背光        : PF0 = TIM14_CH1 PWM，10kHz，无级调光
  INA226 I2C1 : SCL=PB6  SDA=PB7，100kHz，地址 0x40
  ADC         : PA5=IN5（VCC 分压）  PA4=IN4（NTC 分压）
  风扇 PWM    : PB5 = TIM3_CH2，10kHz
  板上 LED    : PA0（本版仍是 LED；V0.5 起改为电源使能）
  调试 / 复位 : SWD=PA13/PA14，NRST=PF2

编译
  Keil MDK 打开 MDK-ARM/Project.uvprojx，需装 Puya.PY32F0xx_DFP.1.1.0；
  编译器 ARMCLANG（AC6）；SWD 烧录 PA13/PA14，NRST PF2。

文件版本
  main.c V1.4 / app_ui.c V1.0 / global.c V1.2 / function.c V1.5 / st7735.c 1.2.1
================================================================================
