# PY32F002B_MainBoard

基于 **PY32F002B（TSSOP20）** 的控制板工程，从 `G030F6P6 HAL Finished`
（STM32G030F6P6）工程移植，保留原工程的模块结构与交互逻辑，并按
PY32F002B 的外设能力重新分配引脚。

**当前版本：V1.2（2026-08-24）**

## 功能概览

- ST7789 320x172 SPI 屏：显示电压 / 电流 / 功率 / 温度 / 风扇占空比 / 瓦时，
  支持进度条增量绘制与数值差分刷新；
- INA226（I2C）：母线电压、电流、功率测量，电流做分段线性校准；
- 两路 ADC：VCC 分压、NTC 分压，NTC 温度经卡尔曼滤波；
- 风扇 PWM（TIM1_CH1）按温度滞回控制，背光 PWM（TIM1_CH2）无级调光；
- 三个按键：短按 / 长按 / 双击状态机；长按 SW_WKUP 关机进 STOP，再按唤醒；
- 负载断开（电流 < 15mA）持续 15 分钟自动关机。

## 外设清单

| 外设 | 用途 | 说明 |
| ---- | ---- | ---- |
| SPI1 | ST7789 显示屏 | 仅发送，MOSI + SCK，软件 NSS |
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

按键电路约定：按键一端接对应引脚、另一端接高电平，内部下拉，
因此“按下 = 高电平”（与原 G030 工程一致）。

## 按键操作

| 按键 | 短按 | 长按 | 双击 |
| ---- | ---- | ---- | ---- |
| SW_WKUP (PB5) | 翻转屏幕 | 关机进 STOP（再按开机） | 切换黑白背景 |
| SW_FUNC (PB2) | LED 反馈（预留继电器接口） | 循环调节背光 | - |
| SW_MODE (PC1) | 翻页（左上角 P0/P1） | 切换模式（左上角 M0/M1） | - |

关机/唤醒行为：

1. 长按 SW_WKUP → 屏幕显示 `POWEROFF` → 黑屏进入 STOP；
2. STOP 期间只有“再次按下 SW_WKUP”能唤醒，松手不会唤醒；
3. 唤醒后外设配置保留，屏幕整屏重绘，按键状态机复位，
   唤醒这一次按键按正常的短按/长按/双击逻辑跟踪。

## 目录结构

```text
PY32F002B_MainBoard/
├── README.md          本说明
├── readme.txt         速查说明（Keil Doc 组引用）
├── .gitignore         忽略 Keil 编译产物
├── Inc/               头文件
│   ├── main.h         引脚与配置宏
│   ├── py32f002b_*.h  中断 / HAL 配置
│   ├── global.h       全局共享变量 extern 声明
│   ├── function.h     通用工具（滤波/校准/按键/低功耗）
│   ├── bsp_*.h        GPIO/SPI/I2C/ADC/TIM 模块
│   ├── st7789.h       LCD 驱动
│   ├── INA226.h       电流/电压监测驱动
│   └── Font_asc.h     字模表声明
├── Src/               源文件
│   ├── main.c         主流程 + 50ms/1s 任务 + 显示刷新
│   ├── function.c     滤波、校准、按键状态机、power_off
│   ├── global.c       全局变量定义
│   ├── py32f002b_it.c SysTick / EXTI 中断
│   ├── bsp_*.c        各外设初始化
│   ├── st7789.c       LCD 驱动（无 DMA 软件刷新）
│   ├── INA226.c       INA226 驱动
│   ├── Font_asc.c     字模数据
│   └── system_py32f002b.c  启动/时钟（含 HSI 出厂校准）
└── MDK-ARM/           Keil 工程（Project.uvprojx）
```

## 代码架构

- **前后台**：SysTick / EXTI 中断只记录时间戳；主循环执行 50ms 任务
  （消抖、ADC、温度、风扇、瓦时）和 1s 任务（LED、自动关机倒计时），
  其余时间刷新屏幕。
- **模块化**：`bsp_gpio / bsp_spi / bsp_i2c / bsp_adc / bsp_tim`
  一个外设一个模块；`st7789 / INA226 / function / global` 为应用层模块。
- **差分刷新**：数值显示用槽位缓存对比，没变化的位输出空格，减少 SPI 传输。
- **算法**：NTC 温度走卡尔曼滤波，电流用分段线性校准表，风扇用温度滞回状态机。

## 与 G030 原工程的差异

1. **去 DMA**：PY32F002B 没有 DMA。整屏填充和图片发送改为软件连续 SPI 发送；
   ADC 三通道 DMA 循环采样改为“两通道扫描 + 轮询”。
2. **定时器**：原 TIM3/TIM17 两路 PWM 合并到 TIM1_CH1/CH2；
   原 TIM14/TIM16 周期中断改为 SysTick 时钟 + 主循环调度
   （PY32 HAL 未实现 TIM14）。
3. **去 RTC 备份**：PY32F002B 没有 RTC 备份寄存器，
   FLIP/背景色/背光亮度改为开机默认值，掉电不保存（可后续用 FLASH 模拟）。
4. **低功耗**：原 STANDBY 改为 STOP。长按 SW_WKUP 进 STOP，只有再次按下
   SW_WKUP 才唤醒；唤醒后外设配置保留，主循环直接恢复。
5. **Ctrl_OUT 继电器脉冲**：原按钮 2 短按动作，PY32 开发板未引出该引脚，
   暂用 LED 反馈替代，预留接口见 `main.c` 按键 2 分支。

## Flash 占用（重要）

PY32F002B 只有 **24KB Flash / 3KB RAM**，而原 G030 工程 ROM 约 30KB，直接移植放不下。
为适配本芯片做了以下裁剪，编译结果为：

```text
Program Size: Code=17152 RO-data=6396 RW-data=120 ZI-data=2064
Flash 占用 ≈ 23.1KB（余量约 0.9KB），RAM 占用 ≈ 2.1KB
```

- 裁剪未使用的字号：48x64、32x16、12x6（保留界面实际使用的 6 种字号）；
- 去除浮点数学库依赖：`sqrtf/atan2f/logf` 分别改为整数开方、多项式近似、
  查表插值（NTC 温度转换），`snprintf` 改为手写整数转字符串；
- 只链接实际使用的 HAL 驱动（SPI 全部走 LL，去掉 hal_spi/hal_exti/hal_rcc_ex）；
- 编译优化 O2 + MicroLIB。

如需恢复某个被裁字号，把 `Font_asc.c` 与 `st7789.c` 中对应 `#if 0` 块和
`Font_asc.h` 的 extern 恢复即可，但需同步评估 Flash 空间。

## 使用方法

1. Keil 打开 `MDK-ARM/Project.uvprojx`；
2. 安装 Puya PY32F0xx DFP 1.2.15（资料包 `FW_SW` 内有）；
3. 编译下载，屏上显示电压/电流/功率/温度/风扇/瓦时。

工程通过相对路径 `..\..\..\Drivers` 引用
`PY32F002B_Firmware_V1.2.1` 的驱动库，请保持本工程与 SDK 的目录层级关系。

## 调试注意

- 本板实测“上电后需按 RST 才能启动”与下载调试器（DAP）有关，
  脱离调试器单独上电时请留意复位行为；
- NRST 建议 10k 上拉 + ≤10nF 对地电容，使其与 3.3V 同步上升；
- 修改 BOR、选项字节等系统配置前，务必保留 NRST/SWD 位，避免锁死芯片。

## 版本记录

| 版本 | 日期 | 说明 |
| ---- | ---- | ---- |
| V1.0 | 2026-08-19 | 首次创建：从 G030F6P6 HAL Finished 移植到 PY32F002B |
| V1.1 | 2026-08-24 | 修复按键链路：改用 PY32 HAL 的 HAL_GPIO_EXTI_Callback，三个按键恢复短按/长按/双击业务 |
| V1.2 | 2026-08-24 | 修复长按关机后立即唤醒循环：STOP 期间仅 SW_WKUP 按下可唤醒，松手不再唤醒 |

## License

本项目移植自 Bowen 的 `G030F6P6 HAL Finished` 工程，沿用其许可约定：
**CC BY-NC-SA 4.0**（署名 - 非商业性使用 - 相同方式共享），
详见各源文件头部的说明；商业使用请联系原作者。
