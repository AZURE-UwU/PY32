================================================================================
                        PY32F003F18P6TU 工程说明
================================================================================
芯片：PY32F003F18P6TU（TSSOP20，64KB Flash / 8KB SRAM，最高 32MHz）
版本：V1.1（2026-08-29，由 PY32F002B_ST7735 移植）

外设与引脚：
  SPI1 -> ST7735S 0.96寸 160x80 横屏（SCK=PA1, MOSI=PA2, DC=PA3,
          CS=PF1, RST=PF4）
  I2C1 -> INA226（SCL=PB6, SDA=PB7, 100kHz, 地址 0x40）
  ADC  -> IN5(PA5)=VCC 分压, IN4(PA4)=NTC 分压（扫描轮询）
  TIM3 -> CH2(PB5)=风扇 PWM, 10kHz
  TIM14-> CH1(PF0)=背光 PWM 无级调光, 10kHz
  按键 -> SW_WKUP(PA12 板上KEY), SW_FUNC(PA6), SW_MODE(PA7)，高电平有效
  LED  -> PA0（板上 LED）

按键：
  SW_WKUP 短按转屏 / 长按关机进 STOP（再按开机）/ 双击换背景
  SW_FUNC 短按 LED 反馈 / 长按调节背光
  SW_MODE 短按翻页 / 长按切换模式

编译：Keil 打开 MDK-ARM/Project.uvprojx，安装 Puya.PY32F0xx_DFP.1.1.0
（资料包 pack/MDK/Keil 内），编译后 SWD（PA13/PA14）下载。

详细说明见 README.md。
================================================================================
