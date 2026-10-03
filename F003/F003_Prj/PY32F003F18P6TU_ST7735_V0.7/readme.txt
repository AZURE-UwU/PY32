================================================================================
              PY32F003F18P6TU + ST7735  V0.7 速查（当前最新版）
================================================================================
芯片：PY32F003F18P6TU（TSSOP20，64KB Flash / 8KB SRAM，最高 32MHz）
本版：V0.7（2026-09-08，注释修正在 2026-10-03）命名规范对齐
上一版：V0.6（主题编号化 + theme 模块拆分 + 采集/呈现分离）
状态：已上板实测通过；Keil ARMCLANG V6 编译 0 Error / 0 Warning

本版改了什么
  - g_ 前缀严格只给跨模块共享变量，模块私有 static 改 s_：
    g_menu->s_menu、g_theme_opts->s_theme_opts、g_cfg_edit->s_cfg_edit、
    g_display_cb->s_display_cb；
  - app_ui.c 升到 V1.8：修正设计说明中"掉电存储本版暂缺"的过时表述
    （Flash 掉电存储已在 V0.4 实现）。

代码分层
  bsp_*      底层外设（GPIO/SPI/I2C/ADC/TIM/Flash）
  st7735/INA226  设备驱动（含差分槽位、校准）
  global.c   数据仓库：g_cfg 配置 + g_v/g_i/g_p/g_temp/g_wh/g_pwm（预留多路）
  function.c 工具：卡尔曼、分段校准、NTC 换算、风扇滞回、按键状态机、关机
  app_ui.c   设置菜单：菜单表 + 四状态状态机 + 编辑 + Flash 组帧校验
  theme.c    主界面呈现：每主题一组 Frame/Values（格式化+差分+绘制）
  main.c     调度：50ms / 1s / 每圈采集，不含绘制细节

菜单项（CFG_VERSION = 2，掉电保存在主 Flash 0x0800FF00）
  FLIP / THEME(BLACK/WHITE) / AUTO OFF / BLK(0-100%) / AOFF I(0-0.1A) /
  AOFF MIN(1-120min) / VBUS DIV(0.5-3.0) / SAVE & EXIT / DISCARD

按键
  UP   (PA12)  主界面长按关机 ／ 菜单上移 ／ 值编辑 +步长 ／ 选项上一个
  SET  (PA6)   主界面短按进设置 ／ 切换勾选或进入编辑 ／ 确认
  DOWN (PA7)   主界面无功能 ／ 菜单下移 ／ 值编辑 -步长 ／ 选项下一个
  （菜单与值编辑均可长按连跳：前 2 秒 200ms/步，2 秒后 50ms/步）

引脚
  显示屏 SPI1 : SCK=PA1  MOSI=PA2  DC=PA3  CS=PF1  RST=PF4
  背光        : PF0 = TIM14_CH1 PWM，10kHz，无级调光
  INA226 I2C1 : SCL=PB6  SDA=PB7，100kHz，地址 0x40
  ADC         : PA5=IN5（VCC 分压）  PA4=IN4（NTC 分压）
  风扇 PWM    : PB5 = TIM3_CH2，10kHz
  电源使能    : PA0（推挽输出，上电高 / 关机低）
  调试 / 复位 : SWD=PA13/PA14，NRST=PF2
  硬件前提    : PF0/PF1 原接 24MHz 晶振，复用前需移除晶振与 22pF 电容

编译
  Keil MDK 打开 MDK-ARM/Project.uvprojx，需装 Puya.PY32F0xx_DFP.1.1.0；
  编译器 ARMCLANG（AC6）；SWD 烧录 PA13/PA14，NRST PF2。
  注意：全片擦除下载会清掉配置页，首次上电回默认值（正常现象）。

文件版本
  main.c V1.10 / app_ui.c V1.8 / theme.c V1.1 / global.c V1.5 /
  bsp_gpio.c V1.7 / function.c V1.9 / bsp_flash.c V1.0 / st7735.c 1.2.2

未实现（见 260829需求.pdf）
  三路显示(BAT/C1/C2/C3)、采样电阻菜单项、电池串数/单体电压、
  显示自动轮播与掉电记忆、低电量报警、WH 重置、开启温度/开始转速
================================================================================
