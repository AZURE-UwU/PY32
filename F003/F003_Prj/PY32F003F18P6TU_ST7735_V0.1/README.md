# PY32F003F18P6TU_ST7735_V0.1

> F003 工程的 **V0.1 版本快照**（2026-08-31）。
> 本版只做一件事：**把用户可改的配置从散落的裸变量收进一个结构体**，为后面的设置界面和掉电保存打地基。
> 移植背景、完整引脚表与编译方法见 [顶层 readme.txt](../readme.txt)，最新版见 [V0.7 的 README](../PY32F003F18P6TU_ST7735_V0.7/README.md)。

## 本版改动（相对 Base）

新增 `Settings_t`，定义在 [global.h](Core/Inc/global.h)，实例 `g_cfg` 定义在 [global.c](Core/Src/global.c)：

| 字段 | 含义 | 默认值 |
| ---- | ---- | ---- |
| `flip` | 屏幕方向 1/3 | 1 |
| `theme` | 主题（本版存的是背景颜色值） | BLACK |
| `blk` | 背光亮度 0~100 | 100 |
| `auto_off` | 自动关机使能 | 开 |
| `auto_off_i` | 判停电流阈值 | 0.015 A |
| `auto_off_min` | 判停时长 | 15 min |
| `vbus_div` | INA226 母线分压微调系数 | 1.0 |

- 原 `FLIP / BG / blk` 裸变量删除，引用**直接改写成 `g_cfg.flip / .theme / .blk`**，不用宏别名过渡；
- 自动关机的阈值与时长一并参数化进结构体；
- 上电行为、显示内容、按键功能与 Base **完全一致**，本版是纯数据搬家。

## 本版还没有的东西

- **没有设置菜单**（`app_ui.c` 从 V0.2 才加入）；
- **配置只在 RAM 里，掉电不保存**（Flash 掉电存储从 V0.4 才有）；
- 主界面按键仍是 Base 那套三键逻辑（见下表）。

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
| PA12 / PA6 / PA7 | SW_WKUP / SW_FUNC / SW_MODE | 高电平有效 |
| PA0 | 板上 LED | 1s 翻转（**V0.5 起改为电源使能**） |
| PA13 / PA14 | SWD 调试 | — |
| PF2 | NRST | 保留复位 |

## 按键（本版：与 Base 相同）

| 按键 | 短按 | 长按 | 双击 |
| ---- | ---- | ---- | ---- |
| SW_WKUP (PA12) | 翻转屏幕 | 关机进 STOP（再按唤醒） | 切换黑白背景 |
| SW_FUNC (PA6) | LED 反馈 | 循环调节背光 | — |
| SW_MODE (PA7) | 翻页 | 切换模式 | — |

## 目录结构

```text
PY32F003F18P6TU_ST7735_V0.1/
├── Core/Inc, Core/Src    应用代码（本版改的是 global.h / global.c / main.c）
├── Drivers/              CMSIS + PY32F0xx HAL/LL（自包含）
├── MDK-ARM/              Keil 工程 Project.uvprojx
├── Backup/               移植时保留的旧设备文件备份
└── README.md / readme.txt / 修改了结构体.txt（版本标记）
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
| **V0.1** | **2026-08-31** | **用户配置结构体迁移（本版）** |
| V0.2 | 2026-09-02 | 二级设置菜单（勾选 / 数值编辑 / 保存回滚） |
| V0.3 | 2026-09-02 | 菜单与值编辑长按连跳 + 增量重绘 |
| V0.4 | 2026-09-02 | 掉电保存（内部 Flash + MAGIC/版本/CRC16） |
| V0.5 | 2026-09-06 | PA0 改电源使能 + 关机低功耗 + THEME 选项式 |
| V0.6 | 2026-09-06 | 主题编号化 + theme 模块拆分 + 采集/呈现分离 |
| V0.7 | 2026-09-08 | 命名规范对齐（g_/s_），最新版本 |

## License

沿用原工程许可：**CC BY-NC-SA 4.0**（署名 - 非商业性使用 - 相同方式共享）。
