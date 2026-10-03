# PY32F003F18P6TU

把 **PY32F002B_ST7735**（ST7735S 0.96 寸 160x80 横屏控制板程序）移植到
**PY32F003F18P6TU**（TSSOP20，64KB Flash / 8KB SRAM / 最高 32MHz）的完整工程，
屏幕驱动接口与业务功能保持不变，按 F18P 核心板原理图重新做了引脚匹配。

**当前版本：V1.1（2026-08-29）**

## 与 PY32F002B_ST7735 的差异

| 项目 | PY32F002B_ST7735 | 本工程（PY32F003F18P6TU） |
| ---- | ---- | ---- |
| 主控 | PY32F002B，24KB/3KB | PY32F003F18P6TU，64KB/8KB，最高 32MHz |
| 时钟 | HSI 24MHz | HSI 24MHz（FLASH_LATENCY_0） |
| LCD SPI | SCK=PB0，MOSI=PB7 | SCK=PA1，MOSI=PA2（SPI1 AF0） |
| LCD DC | PA6 | PA3 |
| LCD CS/RST/BLK | 均有独立引脚 | CS=PF1、RST=PF4（GPIO），BLK=PF0（TIM14_CH1 PWM 无级调光） |
| 风扇 PWM | TIM1_CH1（PA0） | TIM3_CH2（PB5） |
| INA226 I2C | PB3/PB4 | PB6/PB7 |
| ADC | PB1(IN0)、PA4(IN2) | PA5(IN5)、PA4(IN4) |
| 按键 | PB5/PB2/PC1，按下高有效 | PA12（板上 KEY）/PA6/PA7，按下高有效 |
| LED | PA5 | PA0（板上 LED） |
| 设备驱动 | py32f002b_* HAL/LL | py32f0xx_* HAL/LL |

其余全部保持：ST7735 驱动接口 `LCD_* / ProgressBar_* / slot_*`、
INA226 读取、卡尔曼/分段校准、三键短按/长按/双击状态机、
长按关机进 STOP + SW_WKUP 按键唤醒、1s LED 闪烁与自动关机逻辑。

## 引脚分配（TSSOP20，按 F18P 板原理图）

| 引脚 | 功能 | 配置 |
| ---- | ---- | ---- |
| PA1  | LCD_SCK  | SPI1_SCK（AF0） |
| PA2  | LCD_MOSI | SPI1_MOSI（AF0） |
| PA3  | LCD_DC   | GPIO 输出，默认低 |
| PF1  | LCD_CS   | GPIO 输出，默认高 |
| PF4  | LCD_RST  | GPIO 输出，默认高 |
| PF0  | LCD_BLK  | TIM14_CH1（AF2）PWM 无级调光 |
| PA0  | LED      | 板上 LED，1s 翻转 |
| PA12 | SW_WKUP  | 板上 KEY，高电平有效，STOP 唤醒键 |
| PA6  | SW_FUNC  | EXTI6，高电平有效 |
| PA7  | SW_MODE  | EXTI7，高电平有效 |
| PA5  | ADC_VCC  | ADC IN5（VCC 分压） |
| PA4  | ADC_NTC  | ADC IN4（NTC 分压） |
| PB5  | PWM_FAN  | TIM3_CH2（AF1），10kHz |
| PB6  | I2C_SCL  | I2C1_SCL（AF6，开漏） |
| PB7  | I2C_SDA  | I2C1_SDA（AF6，开漏） |
| PA13 | SWDIO    | 调试 |
| PA14 | SWCLK    | 调试 |
| PF0/PF1 | LCD_BLK / LCD_CS | 复用（见上）；板上原为 24MHz 晶振，使用前需移除/断开晶振 |
| PF2 | NRST | 复位 |
| PF4 | LCD_RST | 复用（默认 BOOT0 功能未使用） |

屏幕接线：VCC、GND、SCK→PA1、SDA/MOSI→PA2、DC→PA3、
CS→PF1、RST→PF4、BLK→PF0（背光 PWM 无级调光，长按 SW_FUNC 调节）。

## 按键说明

本板按键为**高电平有效**（内部下拉，按下接 VCC）：

| 按键 | 短按 | 长按 | 双击 |
| ---- | ---- | ---- | ---- |
| SW_WKUP (PA12) | 翻转屏幕 | 关机进 STOP（再按开机） | 切换黑白背景 |
| SW_FUNC (PA6) | LED 反馈 | 循环调节背光（本板为显示开关） | - |
| SW_MODE (PA7) | 翻页 | 切换模式 | - |

本工程已改为高电平有效；若需改回低电平有效，把 `Core/Inc/main.h` 的 `BTN_ACTIVE_LOW` 改回 `1`。

## 目录结构

```text
PY32F003F18P6TU/
├── Core/Inc, Core/Src   应用代码（移植自 PY32F002B_ST7735）
├── Drivers/CMSIS         CMSIS 头文件 + py32f003x8 启动/系统文件
├── Drivers/PY32F0xx_HAL_Driver  003/030 系列 HAL/LL 驱动（自包含）
├── MDK-ARM/              Keil 工程（Project.uvprojx）
├── Backup/               旧设备文件与参考 README 备份
├── README.md / readme.txt
└── .gitignore
```

工程自包含全部驱动，不依赖 SDK 目录，直接打开 MDK 工程即可编译。

## 编译与烧录

1. Keil MDK（ARMCC V5）打开 `MDK-ARM/Project.uvprojx`；
2. 需要安装 Puya 器件包 `Puya.PY32F0xx_DFP.1.1.0`
   （资料包 `PY-MCU资料002_003_030/pack/MDK/Keil` 内有 .pack，双击安装；
   若已装过旧版可覆盖升级）；
3. 编译下载（SWD：PA13/PA14，NRST：PF2）。

## 版本记录

| 版本 | 日期 | 说明 |
| ---- | ---- | ---- |
| V1.0 | 2026-08-29 | 从 PY32F002B_ST7735 移植到 PY32F003F18P6TU，完成引脚匹配与自包含工程 |
| V1.1 | 2026-08-29 | PF0/PF1/PF4 允许复用：恢复 LCD CS/RST，背光改 TIM14_CH1(PF0) PWM 无级调光 |

## License

沿用原工程许可：**CC BY-NC-SA 4.0**（署名 - 非商业性使用 - 相同方式共享）。
