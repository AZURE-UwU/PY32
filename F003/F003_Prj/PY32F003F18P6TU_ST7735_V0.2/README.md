# PY32F003F18P6TU_ST7735_V0.2

> F003 工程的 **V0.2 版本快照**（2026-09-02）。
> 本版新增 **二级设置菜单**：主界面只留"长按 UP 关机、短按 SET 进设置"，其余用户设置全部收进菜单。
> 详细硬件与引脚见 [顶层 readme.txt](../readme.txt)，最新版见 [V0.7 的 README](../PY32F003F18P6TU_ST7735_V0.7/README.md)。

## 本版改动

新增 [app_ui.c](Core/Src/app_ui.c) / [app_ui.h](Core/Inc/app_ui.h)，构成设置界面的全部逻辑：

1. **三状态 UI 状态机**：`UI_MAIN` / `UI_MENU` / `UI_VALUE`（数值编辑）；
2. **菜单查表描述**（`MenuItem_t`）：类型 + 字段 + 范围/步长，加一项只改表不动状态机；
3. **勾选项**：SET 切换，再按取消；
4. **数值项（方案 A）**：UP = +步长、DOWN = -步长、SET = 确认，到边界钳位不循环；
5. **整页回滚**：所有编辑落在工作副本 `g_cfg_edit` 上，
   - `SAVE & EXIT` 把副本写回 `g_cfg` 并生效；
   - `DISCARD` 直接丢弃副本，天然实现"不保存并返回"。

## 设置菜单（本版实际菜单项，一屏 5 行滚动）

| 项 | 类型 | 范围 / 步长 |
| ---- | ---- | ---- |
| FLIP | 勾选 | 屏幕方向 |
| THEME | 勾选 | 黑 / 白背景（本版仍是勾选切换） |
| AUTO OFF | 勾选 | 自动关机使能 |
| BLK | 数值 | 0 ~ 100 %，步长 5 |
| AOFF I | 数值 | 0 ~ 0.1 A，步长 0.005 |
| AOFF MIN | 数值 | 1 ~ 120 min，步长 1 |
| VBUS DIV | 数值 | 0.5 ~ 3.0，步长 0.01 |
| SAVE & EXIT | 动作 | 保存并退出 |
| DISCARD | 动作 | 放弃并返回 |

## 本版还没有的东西

- **掉电保存**：`Settings_Save()` 只有 RAM 逻辑，注释里留了 TODO，**断电即丢**（V0.4 才写内部 Flash）；
- **长按连跳**：菜单与值编辑都只能一下一下按（V0.3 才有）；
- THEME 还是"勾选换背景"，不是选项页（V0.5 才改）。

## 按键（本版起固定下来）

| 按键 | 主界面 | 菜单 | 数值编辑 |
| ---- | ---- | ---- | ---- |
| UP (SW_WKUP / PA12) | 长按关机进 STOP | 上移一项 | +步长 |
| SET (SW_FUNC / PA6) | 短按进设置界面 | 切换勾选 / 进入编辑 | 确认本项 |
| DOWN (SW_MODE / PA7) | 无功能 | 下移一项 | -步长 |

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
| PA12 / PA6 / PA7 | UP / SET / DOWN | 高电平有效 |
| PA0 | 板上 LED | 1s 翻转（**V0.5 起改为电源使能**） |
| PA13 / PA14 | SWD 调试 | — |
| PF2 | NRST | 保留复位 |

## 目录结构

```text
PY32F003F18P6TU_ST7735_V0.2/
├── Core/Inc, Core/Src    应用代码（本版新增 app_ui.c / app_ui.h）
├── Drivers/              CMSIS + PY32F0xx HAL/LL（自包含）
├── MDK-ARM/              Keil 工程 Project.uvprojx
├── Backup/               移植时保留的旧设备文件备份
└── README.md / readme.txt / 增加了设置界面.txt（版本标记）
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
| **V0.2** | **2026-09-02** | **二级设置菜单（本版）** |
| V0.3 | 2026-09-02 | 菜单与值编辑长按连跳 + 增量重绘 |
| V0.4 | 2026-09-02 | 掉电保存（内部 Flash + MAGIC/版本/CRC16） |
| V0.5 | 2026-09-06 | PA0 改电源使能 + 关机低功耗 + THEME 选项式 |
| V0.6 | 2026-09-06 | 主题编号化 + theme 模块拆分 + 采集/呈现分离 |
| V0.7 | 2026-09-08 | 命名规范对齐（g_/s_），最新版本 |

## License

沿用原工程许可：**CC BY-NC-SA 4.0**（署名 - 非商业性使用 - 相同方式共享）。
