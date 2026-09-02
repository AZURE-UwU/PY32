# PY32F002B_ST7735

在 **PY32F002B_MainBoard** 基础上新建的工程：屏幕驱动由 ST7789 更换为
**ST7735S 0.96 寸（横屏 160x80）**，LCD 的外部函数接口（`LCD_*` /
`ProgressBar_*` / `slot_*`）与业务功能保持不变，界面布局按小屏重新排版。

**当前版本：V1.0（2026-08-25）**

## 功能概览

- ST7735S 0.96 寸 160x80 横屏：显示电压 / 电流 / 功率 / 温度 / 风扇占空比 / 瓦时，
  三路进度条与数值差分刷新；
- INA226（I2C）：母线电压、电流、功率测量，电流分段线性校准；
- 两路 ADC：VCC 分压、NTC 分压，NTC 温度经卡尔曼滤波；
- 风扇 PWM（TIM1_CH1）按温度滞回控制，背光 PWM（TIM1_CH2）无级调光；
- 三个按键：短按 / 长按 / 双击状态机；长按 SW_WKUP 关机进 STOP，再按唤醒；
- 负载断开（电流 < 15mA）持续 15 分钟自动关机。

## 与 MainBoard 工程的差异

1. 显示驱动 `st7789.c/h` 替换为 `st7735.c/h`，原 ST7789 驱动备份在
   `Backup/st7789.c.bak`、`Backup/st7789.h.bak`；
2. 分辨率 320x172 → 160x80（横屏），行列偏移按模组参数（横屏 x+1/y+26）；
3. 初始化序列、MADCTL 方向表、复位时序换成 ST7735S 参数（含 INVON 颜色反转）；
4. 界面布局按 160x80 重排：左侧 U/I/P 大数字 + 右侧三路进度条 +
   底部 T/F/wh 一行 + 右上角 M/P 指示；
5. 新增按键极性开关 `BTN_ACTIVE_LOW`（默认 0，行为与原工程一致）。

其余功能（INA226、ADC、PWM、按键逻辑、STOP 关机/唤醒）与 MainBoard 完全相同。

## 外设清单

| 外设 | 用途 | 说明 |
| ---- | ---- | ---- |
| SPI1 | ST7735S 显示屏 | 仅发送，MOSI + SCK，软件 NSS |
| I2C1 | INA226 电压/电流/功率监测 | 100kHz，器件地址 0x40 |
| ADC  | 两路模拟量 | IN0=VCC 分压、IN2=NTC 分压（无 DMA，扫描轮询） |
| TIM1 | 两路 PWM | CH1=风扇（10kHz），CH2=背光调光 |
| EXTI | 三个按键 | 短按/长按/双击状态机，SW_WKUP 兼任 STOP 唤醒 |
| SysTick | 统一毫秒时钟 `sys` | 主循环 50ms/1s 周期任务调度 |

## 引脚分配（TSSOP20）

| 引脚 | 功能 | 配置 |
| ---- | ---- | ---- |
| PB0  | LCD_SCK  | SPI1_SCK（AF0） |
| PB7  | LCD_MOSI | SPI1_MOSI（AF0） |
| PA3  | LCD_CS   | GPIO 输出，默认高 |
| PA6  | LCD_DC   | GPIO 输出，默认低 |
| PA7  | LCD_RST  | GPIO 输出，默认高 |
| PA1  | LCD_BLK  | TIM1_CH2 PWM 调光 |
| PA0  | PWM_FAN  | TIM1_CH1 PWM |
| PB3  | I2C_SCL  | I2C1_SCL（AF6，开漏 + 上拉） |
| PB4  | I2C_SDA  | I2C1_SDA（AF6，开漏 + 上拉） |
| PB1  | ADC_VCC  | ADC IN0，模拟输入 |
| PA4  | ADC_NTC  | ADC IN2，模拟输入 |
| PB5  | SW_WKUP  | EXTI5，双沿中断，STOP 唤醒键 |
| PB2  | SW_FUNC  | EXTI2，双沿中断 |
| PC1  | SW_MODE  | EXTI1，双沿中断 |
| PA5  | LED_STATE| GPIO 输出，1s 翻转 |
| PA2  | SWCLK    | 保留 SWD 调试 |
| PB6  | SWDIO    | 保留 SWD 调试 |
| PC0  | NRST     | 保留复位 |

## 按键

默认电平约定与原工程一致：按键一端接高电平、引脚内部下拉，按下 = 高电平。
若你的板子是“按键低电平有效”（一端接地），把 `Inc/main.h` 里的
`BTN_ACTIVE_LOW` 改为 `1`，bsp_gpio 会自动切换上拉/下拉和唤醒边沿。

| 按键 | 短按 | 长按 | 双击 |
| ---- | ---- | ---- | ---- |
| SW_WKUP (PB5) | 翻转屏幕 | 关机进 STOP（再按开机） | 切换黑白背景 |
| SW_FUNC (PB2) | LED 反馈（预留继电器接口） | 循环调节背光 | - |
| SW_MODE (PC1) | 翻页（右上角 P0/P1） | 切换模式（右上角 M0/M1） | - |

关机/唤醒：长按 SW_WKUP → 显示 POWEROFF → 黑屏进 STOP；
STOP 期间仅“再次按下 SW_WKUP”能唤醒，松手不唤醒。

## 目录结构

```text
PY32F002B_ST7735/
├── README.md          本说明
├── readme.txt         速查说明（Keil Doc 组引用）
├── .gitignore         忽略 Keil 编译产物
├── Backup/            ST7789 原驱动备份（.bak）
├── Inc/               头文件（st7735.h 替换 st7789.h）
├── Src/               源文件（st7735.c 替换 st7789.c）
└── MDK-ARM/           Keil 工程（Project.uvprojx）
```

## 屏幕方向与偏移说明

- 模组可视区 80x160，横屏使用时逻辑分辨率 **160x80**；
- 驱动内部按当前方向自动加偏移：横屏 x+1/y+26，竖屏 x+26/y+1；
- MADCTL 方向表：0=0x08、1=0xA8、2=0x48、3=0x68（默认横屏 0xA8，
  与测试程序的实测参数一致）；若实际模组方向/颜色不符，只需调整
  `st7735.c` 顶部的 `MADCTL_TABLE`。

## Flash 占用

PY32F002B 只有 24KB Flash，工程沿用 MainBoard 的裁剪方案（精简字号、
无浮点数学库、仅链接必需 HAL 驱动），编译结果见 Keil 输出，预留量不大，
新增功能时注意空间。

## 使用方法

1. Keil 打开 `MDK-ARM/Project.uvprojx`；
2. 安装 Puya PY32F0xx DFP 1.2.15；
3. 编译下载。

工程通过相对路径 `..\..\..\Drivers` 引用 `PY32F002B_Firmware_V1.2.1`
的驱动库，请保持目录层级。

## 版本记录

| 版本 | 日期 | 说明 |
| ---- | ---- | ---- |
| V1.0 | 2026-08-25 | 从 PY32F002B_MainBoard 新建，ST7789 驱动替换为 ST7735S（160x80 横屏） |

## License

沿用原工程许可：**CC BY-NC-SA 4.0**（署名 - 非商业性使用 - 相同方式共享），
详见各源文件头部说明。
