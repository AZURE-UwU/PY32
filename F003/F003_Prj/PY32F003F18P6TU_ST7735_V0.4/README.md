# PY32F003F18P6TU_ST7735_V0.4

> F003 工程的 **V0.4 版本快照**（2026-09-02）。
> 本版给设置菜单补上**掉电保存**：配置写进内部 Flash，断电再上电仍然记得。
> 详细硬件与引脚见 [顶层 readme.txt](../readme.txt)，最新版见 [V0.7 的 README](../PY32F003F18P6TU_ST7735_V0.7/README.md)。

## 本版改动

### 1. 新增底层模块 [bsp_flash.c](Core/Src/bsp_flash.c) / [bsp_flash.h](Core/Inc/bsp_flash.h)

- 配置页取主 Flash **倒数第二页 `0x0800FF00`**（页大小 128 B）。
  刻意避开最后一页 `0x0800FF80`，因为 HAL 的页数边界宏会把末页算成越界；
  该页距代码末端（约 `0x08006Cxx`）有 30 KB 以上裕量；
- 擦除 + 整页编程走官方 HAL，`py32f0xx_hal_flash.c` 已注册进 Keil 工程；
- 擦写期间关中断，保证写入原子性。

### 2. [app_ui.c](Core/Src/app_ui.c) 负责组帧与校验

页内格式（`CFG_VERSION = 1`）：

```text
[0..3] MAGIC "PY32" | [4..5] 版本 | [6..7] CRC16 | [8..] Settings_t | 其余 0xFF
```

- **保存并退出**：先把编辑副本组帧写 Flash，**写成功才提交到 `g_cfg` 并生效**；
  写失败保持旧配置，避免"屏幕变了、断电又回旧值"的不一致；
- **开机**：`main.c` 在 `LCD_Init` 前调用 `APP_UI_SettingsLoad()`，
  校验 MAGIC、版本、CRC16，并对每个字段做范围校验（含 NaN 防御）；
  通过就恢复，不通过（首次上电 / 数据损坏）就用默认值写回 Flash。

## 设置菜单（本版菜单项与 V0.3 相同，SAVE 真正落盘）

| 项 | 类型 | 范围 / 步长 |
| ---- | ---- | ---- |
| FLIP | 勾选 | 屏幕方向 |
| THEME | 勾选 | 黑 / 白背景 |
| AUTO OFF | 勾选 | 自动关机使能 |
| BLK | 数值 | 0 ~ 100 %，步长 5 |
| AOFF I | 数值 | 0 ~ 0.1 A，步长 0.005 |
| AOFF MIN | 数值 | 1 ~ 120 min，步长 1 |
| VBUS DIV | 数值 | 0.5 ~ 3.0，步长 0.01 |
| SAVE & EXIT | 动作 | 写 Flash 并退出 |
| DISCARD | 动作 | 放弃并返回（不落盘） |

## 使用注意

- 保存瞬间屏幕会卡几毫秒（Flash 擦写阻塞），属正常现象；
- 用 Keil **全片擦除**下载固件会清掉配置页，之后首次上电回到默认值，属正常现象；
- 写失败目前是静默保持旧值，界面没有 "SAVE FAIL" 提示。

## 按键

| 按键 | 主界面 | 菜单 | 数值编辑 |
| ---- | ---- | ---- | ---- |
| UP (SW_WKUP / PA12) | 长按关机进 STOP | 上移（长按连跳） | +步长（长按连跳） |
| SET (SW_FUNC / PA6) | 短按进设置界面 | 切换勾选 / 进入编辑 | 确认本项 |
| DOWN (SW_MODE / PA7) | 无功能 | 下移（长按连跳） | -步长（长按连跳） |

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
PY32F003F18P6TU_ST7735_V0.4/
├── Core/Inc, Core/Src    应用代码（本版新增 bsp_flash.c/h）
├── Drivers/              CMSIS + PY32F0xx HAL/LL（含 py32f0xx_hal_flash.c）
├── MDK-ARM/              Keil 工程 Project.uvprojx
├── Backup/               移植时保留的旧设备文件备份
└── README.md / readme.txt / 掉电保存.txt（版本标记）
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
| **V0.4** | **2026-09-02** | **掉电保存（内部 Flash + MAGIC/版本/CRC16）（本版）** |
| V0.5 | 2026-09-06 | PA0 改电源使能 + 关机低功耗 + THEME 选项式 |
| V0.6 | 2026-09-06 | 主题编号化 + theme 模块拆分 + 采集/呈现分离 |
| V0.7 | 2026-09-08 | 命名规范对齐（g_/s_），最新版本 |

## License

沿用原工程许可：**CC BY-NC-SA 4.0**（署名 - 非商业性使用 - 相同方式共享）。
