================================================================================
              PY32F003F18P6TU + ST7735  V0.4 速查
================================================================================
芯片：PY32F003F18P6TU（TSSOP20，64KB Flash / 8KB SRAM，最高 32MHz）
本版：V0.4（2026-09-02）掉电保存（内部 Flash）
上一版：V0.3（菜单长按加速 + 增量重绘）

本版改了什么
  - 新增 Core/Src/bsp_flash.c + Core/Inc/bsp_flash.h：
    配置页 = 主 Flash 倒数第二页 0x0800FF00（128B），刻意避开末页；
    擦除 + 整页编程走官方 HAL，擦写期间关中断；
  - app_ui 负责组帧与校验，页内格式 CFG_VERSION = 1：
    [0..3] MAGIC "PY32" | [4..5] 版本 | [6..7] CRC16 | [8..] Settings_t；
  - "SAVE & EXIT" 写 Flash 成功才提交 g_cfg；"DISCARD" 不落盘；
  - 开机在 LCD_Init 前调 APP_UI_SettingsLoad()：校验 MAGIC/版本/CRC16 +
    字段范围（含 NaN 防御），不通过则写回默认值。

使用注意
  - 保存瞬间屏幕卡几毫秒属正常（Flash 擦写阻塞）；
  - Keil 全片擦除下载会清掉配置页，首次上电回默认值，属正常现象；
  - 写失败目前静默保持旧值，没有界面提示。

菜单项
  FLIP / THEME / AUTO OFF / BLK / AOFF I / AOFF MIN / VBUS DIV / SAVE & EXIT / DISCARD

按键
  UP   (PA12)  主界面长按关机 ／ 菜单上移 ／ 值编辑 +步长   （均可长按连跳）
  SET  (PA6)   主界面短按进设置 ／ 切换勾选或进入编辑 ／ 确认
  DOWN (PA7)   主界面无功能 ／ 菜单下移 ／ 值编辑 -步长   （均可长按连跳）

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
  main.c V1.6 / app_ui.c V1.4 / bsp_flash.c V1.0 / global.c V1.2 /
  function.c V1.5 / st7735.c 1.2.1
================================================================================
