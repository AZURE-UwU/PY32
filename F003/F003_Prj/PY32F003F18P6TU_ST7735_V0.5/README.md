# PY32F003F18P6TU_ST7735_V0.5

> F003 工程的 **V0.5 版本快照**（2026-09-06）。
> 本版做三件事：**PA0 改成电源使能、关机后引脚置模拟输入省电、THEME 改成选项页**。
> 详细硬件与引脚见 [顶层 readme.txt](../readme.txt)，最新版见 [V0.7 的 README](../PY32F003F18P6TU_ST7735_V0.7/README.md)。

## 本版改动

### 1. PA0 改为电源使能（原板上 LED 脚复用）

- 上电 / 运行：**PA0 输出高**（使能整板电源）；
- 关机：**PA0 拉低**（真正断电），不再有 1s LED 翻转的用法。
对应 [bsp_gpio.c](Core/Src/bsp_gpio.c) 的 `BSP_GPIO_Init()` 与 [main.h](Core/Inc/main.h) 里的 PA0 定义。

### 2. 关机低功耗：非必要引脚置模拟输入

新增 `BSP_GPIO_LowPowerConfig()`（[bsp_gpio.h](Core/Inc/bsp_gpio.h)）：

- 把非必要引脚全部设为**模拟输入**（既省电，又避免悬空脚漏电）；
- PA0 拉低、PF0 背光拉低、PB5 风扇拉低；
- **只保留 SW_WKUP（PA12）作为唤醒源**，避免松手时的边沿把芯片立刻唤醒；
- [function.c](Core/Src/function.c) 的 `power_off()` 相应改写：进 STOP 前停 SysTick、清 pending，唤醒后恢复三键双沿中断并复位按键状态机。

### 3. THEME 改成选项式

- 菜单项类型新增 `IT_OPTION` / UI 状态新增 `UI_OPTION`；
- 进入 THEME 后 **UP/DOWN 在 WHITE / BLACK 间循环选择，SET 确认**，交互与数值编辑界面一致；
- 本版 `g_cfg.theme` 存的仍是**背景颜色值**（V0.6 才改成主题编号）。

## 设置菜单（本版）

| 项 | 类型 | 范围 / 步长 |
| ---- | ---- | ---- |
| FLIP | 勾选 | 屏幕方向 |
| THEME | **选项** | BLACK / WHITE |
| AUTO OFF | 勾选 | 自动关机使能 |
| BLK | 数值 | 0 ~ 100 %，步长 5 |
| AOFF I | 数值 | 0 ~ 0.1 A，步长 0.005 |
| AOFF MIN | 数值 | 1 ~ 120 min，步长 1 |
| VBUS DIV | 数值 | 0.5 ~ 3.0，步长 0.01 |
| SAVE & EXIT | 动作 | 写 Flash 并退出 |
| DISCARD | 动作 | 放弃并返回 |

## 按键

| 按键 | 主界面 | 菜单 | 数值 / 选项编辑 |
| ---- | ---- | ---- | ---- |
| UP (SW_WKUP / PA12) | 长按关机进 STOP | 上移（长按连跳） | +步长 / 上一选项 |
| SET (SW_FUNC / PA6) | 短按进设置界面 | 切换勾选 / 进入编辑 | 确认本项 |
| DOWN (SW_MODE / PA7) | 无功能 | 下移（长按连跳） | -步长 / 下一选项 |

## 硬件与引脚

| 引脚 | 功能 | 配置 |
| ---- | ---- | ---- |
| PA1 / PA2 | LCD SCK / MOSI | SPI1（AF0） |
| PA3 | LCD DC | GPIO 输出 |
| PF1 / PF4 | LCD CS / RST | GPIO 输出，默认高 |
| PF0 | LCD 背光 | TIM14_CH1（AF2）PWM 无级调光，10kHz |
| PB6 / PB7 | INA226 SCL / SDA | I2C1（AF6），100kHz，地址 0x40 |
| PA5 / PA4 | ADC VCC / NTC | ADC IN5 / IN4 |
| PB5 | 风扇 PWM | TIM3_CH2（AF1），10kHz |
| PA12 / PA6 / PA7 | UP / SET / DOWN | 高电平有效，PA12 兼 STOP 唤醒 |
| **PA0** | **电源使能** | **推挽输出，上电高、关机低** |
| PA13 / PA14 | SWD 调试 | — |
| PF2 | NRST | 保留复位 |

## 目录结构

```text
PY32F003F18P6TU_ST7735_V0.5/
├── Core/Inc, Core/Src    应用代码（本版改 bsp_gpio / function / app_ui / main）
├── Drivers/              CMSIS + PY32F0xx HAL/LL（自包含）
├── MDK-ARM/              Keil 工程 Project.uvprojx
├── Backup/               移植时保留的旧设备文件备份
└── README.md / readme.txt / MDK-ARM/省电优化，高电平优化.txt（版本标记）
```

## 编译与烧录

1. Keil MDK 打开 `MDK-ARM/Project.uvprojx`；
2. 需要 Puya 器件包 `Puya.PY32F0xx_DFP.1.1.0`；
3. 编译器 ARMCLANG（AC6）；
4. SWD 下载：PA13/PA14，NRST：PF2。

## 各版本快照

| 目录 | 日期 | 说明 |
| ---- | ---- | ---- |
| Base | 2026-08-29 | 移植基线：F002→F003，引脚重排，功能不变 |
| V0.1 | 2026-08-31 | 用户配置结构体迁移 |
| V0.2 | 2026-09-02 | 二级设置菜单（勾选 / 数值编辑 / 保存回滚） |
| V0.3 | 2026-09-02 | 菜单与值编辑长按连跳 + 增量重绘 |
| V0.4 | 2026-09-02 | 掉电保存（内部 Flash + MAGIC/版本/CRC16） |
| **V0.5** | **2026-09-06** | **PA0 电源使能 + 关机低功耗 + THEME 选项式（本版）** |
| V0.6 | 2026-09-06 | 主题编号化 + theme 模块拆分 + 采集/呈现分离 |
| V0.7 | 2026-09-08 | 命名规范对齐（g_/s_），最新版本 |

## License

沿用原工程许可：**CC BY-NC-SA 4.0**（署名 - 非商业性使用 - 相同方式共享）。
