# PA1_Toggle — PA1 高低电平切换（周期 1s）

## 功能

PA1 输出周期为 1s 的方波：**高电平 500ms，低电平 500ms**，循环切换。

## 硬件与引脚

| 引脚 | 方向 | 说明 |
| ---- | ---- | ---- |
| PA1  | 输出 | 推挽输出，默认低电平；可接 LED（串限流电阻）或示波器观察波形 |

- 主控：PY32F002B（TSSOP20 开发板）
- 时钟：HSI 24MHz（HSISYS 作为系统时钟）
- 定时：SysTick 1ms 中断维护全局毫秒时钟 `sys`

## 目录结构

```
PA1_Toggle/
├── MDK-ARM/       Keil 工程（Project.uvprojx）
├── Inc/           头文件：main.h / global.h / bsp_pa1.h / 中断头 / HAL 配置
├── Src/           源文件：main.c / global.c / bsp_pa1.c / 中断 / MSP / 系统时钟
└── README.md      本说明文档
```

## 代码架构

- **前后台架构**：SysTick 中断只累加毫秒时钟 `sys`；主循环负责周期翻转。
- **统一时间基准**：所有计时复用 `sys`（1ms 分辨率），无符号减法自动防溢出。
- **模块化**：PA1 操作集中在 `bsp_pa1.c`（初始化/置高/置低/翻转），main 只调 API。
- **全局变量集中**：`sys` 在 `global.c` 定义，`global.h` 用 `extern` 导出。
- **防御性编程**：电平设置入口校验非法参数；错误处理函数兜底。

## 使用方法

1. 用 Keil 打开 `MDK-ARM/Project.uvprojx`；
2. 确认已安装 Puya PY32F0xx DFP 1.2.15（资料包 `FW_SW` 中有安装包）；
3. 编译、下载，用示波器/万用表观察 PA1：高 500ms → 低 500ms 循环。

## 修改周期

在 `Inc/main.h` 中修改 `TOGGLE_HALF_PERIOD_MS`：

```c
#define TOGGLE_HALF_PERIOD_MS   500U   /* 高 500ms + 低 500ms = 周期 1s */
```

例如改为 `1000U` 即周期 2s，改为 `250U` 即周期 500ms。

## 版本记录

| 版本 | 日期       | 说明                         |
| ---- | ---------- | ---------------------------- |
| V1.0 | 2026-08-18 | 首次创建，实现 PA1 周期 1s 翻转 |
