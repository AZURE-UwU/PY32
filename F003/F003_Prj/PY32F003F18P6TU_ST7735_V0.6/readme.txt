================================================================================
              PY32F003F18P6TU + ST7735  V0.6 速查
================================================================================
芯片：PY32F003F18P6TU（TSSOP20，64KB Flash / 8KB SRAM，最高 32MHz）
本版：V0.6（2026-09-06）主题编号化 + theme 模块拆分 + 采集/呈现分离
上一版：V0.5（PA0 电源使能 + 关机低功耗 + THEME 选项式）

本版改了什么
  - 新增 Core/Src/theme.c + Core/Inc/theme.h：主题渲染与进度条迁入，
    对外只暴露 Theme_RenderFrame() / Theme_RenderValues()；
  - g_cfg.theme 由颜色值改为主题编号（0/1），设置里显示 BLACK/WHITE；
    主题只影响主界面，设置/数值/选项页固定深色底，开机画面与 POWEROFF 固定黑底；
  - 采集与呈现分离：测量量集中到 global.c 并预留多路
    （g_v[] / g_i[] / g_p[] / g_temp / g_wh / g_pwm），main.c 只采集调度，
    格式化 + 差分 + 绘制全部在 theme.c；
  - 删除只写不读的 test 变量（含 EXTI 中断里的 test++）；
  - Settings_t 布局变化，CFG_VERSION 由 1 升到 2（旧数据自动回默认）。

菜单项
  FLIP / THEME(选项) / AUTO OFF / BLK / AOFF I / AOFF MIN / VBUS DIV / SAVE & EXIT / DISCARD

按键
  UP   (PA12)  主界面长按关机 ／ 菜单上移 ／ 值编辑 +步长 ／ 选项上一个
  SET  (PA6)   主界面短按进设置 ／ 切换勾选或进入编辑 ／ 确认
  DOWN (PA7)   主界面无功能 ／ 菜单下移 ／ 值编辑 -步长 ／ 选项下一个

引脚
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
  main.c V1.10 / app_ui.c V1.6 / theme.c V1.1 / global.c V1.5 /
  bsp_gpio.c V1.7 / function.c V1.9 / bsp_flash.c V1.0 / st7735.c 1.2.1
================================================================================
