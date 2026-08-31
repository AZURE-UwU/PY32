================================================================================
                        PY32F002B_ST7735 工程说明
================================================================================
芯片：PY32F002B（TSSOP20）
屏幕：ST7735S 0.96 寸，横屏 160x80（驱动由 ST7789 替换，接口不变）
版本：V1.0（2026-08-25）

外设：
  SPI1 -> ST7735S（SCK=PB0, MOSI=PB7, CS=PA3, DC=PA6, RST=PA7, BLK=PA1 PWM）
  I2C1 -> INA226（SCL=PB3, SDA=PB4, 100kHz, 地址 0x40）
  ADC  -> IN0(PB1)=VCC 分压, IN2(PA4)=NTC 分压（扫描轮询，无 DMA）
  TIM1 -> CH1(PA0)=风扇 PWM, CH2(PA1)=背光 PWM, 10kHz
  按键 -> SW_WKUP(PB5), SW_FUNC(PB2), SW_MODE(PC1)，EXTI 双沿

按键：
  SW_WKUP 短按转屏 / 长按关机进 STOP（再按开机）/ 双击换背景
  SW_FUNC 短按 LED 反馈 / 长按调节背光
  SW_MODE 短按翻页 / 长按切换模式

按键低电平有效时：把 Inc/main.h 的 BTN_ACTIVE_LOW 改为 1。

使用方法：Keil 打开 MDK-ARM/Project.uvprojx，安装 Puya PY32F0xx DFP 1.2.15，
保持与 PY32F002B_Firmware_V1.2.1 的目录层级后编译下载。

详细说明见 README.md。
================================================================================
