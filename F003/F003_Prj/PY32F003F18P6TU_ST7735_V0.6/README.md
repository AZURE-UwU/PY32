# PY32F003F18P6TU_ST7735_V0.6

> F003 工程的 **V0.6 版本快照**（2026-09-06）。
> 本版做的是**结构重构**：主题从"颜色值"改成"编号"，并拆出独立的 `theme` 模块，把**采集**和**呈现**彻底分开。
> 详细硬件与引脚见 [顶层 readme.txt](../readme.txt)，最新版见 [V0.7 的 README](../PY32F003F18P6TU_ST7735_V0.7/README.md)。

## 本版改动

### 1. 主题改成"编号"，且只影响主界面

- `g_cfg.theme` 由颜色值改成**主题编号**（0 / 1），设置里显示 `BLACK` / `WHITE` 标签；
- 主界面绘制拆成每个主题一组函数，由集中 `switch` 分派：

```c
void Theme_RenderFrame(void);    /* 整帧：背景 + 标签 + 进度条 + 装饰 */
void Theme_RenderValues(void);   /* 每圈：数值刷新 */
```

- **设置菜单、数值编辑、选项页一律固定深色底**，与主题解耦；
- **开机画面与关机提示固定黑底红字**，不随主题变化。

### 2. 新增独立主题模块 [theme.c](Core/Src/theme.c) / [theme.h](Core/Inc/theme.h)

- 六个主题函数（`Theme0/1_Frame/Values`）与三个进度条对象都迁入 `theme.c`，进度条作为模块内 `static`；
- 对外只暴露 `Theme_RenderFrame()` / `Theme_RenderValues()` 两个入口；
- 主题之间布局不同时，各自在自己的函数里写死绘制，互不干扰。

### 3. 采集与呈现分离

- 测量量集中到 [global.c](Core/Src/global.c)，并**预留多路扩展**：

```c
float   g_v[MEAS_CH_MAX];   /* 各通道电压 */
float   g_i[MEAS_CH_MAX];   /* 各通道电流 */
float   g_p[MEAS_CH_MAX];   /* 各通道功率 */
float   g_temp;             /* 外部温度（NTC） */
float   g_wh;               /* 瓦时积分 */
uint8_t g_pwm;              /* 风扇占空比 */
```

- `main.c` 只做**采集 + 调度**；格式化、差分刷新、字符串缓冲全部搬进 `theme.c`；
- 主题想显示别的量（换通道、只显示电压+温度）只改对应 `ThemeX_Values`，采集层不动；
- 顺带删掉只写不读的 `test` 变量（含 EXTI 中断里的 `test++`）。

### 4. 掉电存储版本号升级

`Settings_t` 布局变了（`theme` 由 `uint16_t` 颜色值变 `uint8_t` 编号），
`CFG_VERSION` 由 1 升到 **2**，旧 Flash 数据版本不符会自动回默认值。

## 设置菜单（本版）

| 项 | 类型 | 范围 / 步长 |
| ---- | ---- | ---- |
| FLIP | 勾选 | 屏幕方向 |
| THEME | 选项 | BLACK / WHITE |
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
| PA0 | 电源使能 | 推挽输出，上电高、关机低 |
| PA13 / PA14 | SWD 调试 | — |
| PF2 | NRST | 保留复位 |

## 目录结构

```text
PY32F003F18P6TU_ST7735_V0.6/
├── Core/Inc, Core/Src    应用代码（本版新增 theme.c/h）
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
| V0.5 | 2026-09-06 | PA0 改电源使能 + 关机低功耗 + THEME 选项式 |
| **V0.6** | **2026-09-06** | **主题编号化 + theme 模块拆分 + 采集/呈现分离（本版）** |
| V0.7 | 2026-09-08 | 命名规范对齐（g_/s_），最新版本 |

## License

沿用原工程许可：**CC BY-NC-SA 4.0**（署名 - 非商业性使用 - 相同方式共享）。
