/**
  ******************************************************************************
  * @file    theme.c
  * @author  Bowen (wbw20)
  * @date    2026-09-07
  * @version V1.1
  * @hardware PY32F003F18P6TU 开发板（TSSOP20），ST7735S 160x80 横屏
  * @brief   主界面主题渲染：每个主题一套 Frame/Values，颜色与布局写死
  *
  * 设计说明：
  *   - 主题只作用于主界面，设置/数值/选项页固定深色底（见 app_ui.c）；
  *   - g_cfg.theme 是主题编号，Theme_RenderFrame/Values 里用 switch 分发；
  *   - 每个主题持有自己的进度条配置（位置/颜色/量程），Frame 里重新 Init；
  *   - 测量数据统一放 global.c（main 采集，theme 格式化/差分/绘制）。
  *
  * CHANGELOG:
  *   V1.0 (2026-09-07) 从 main.c 拆分主题渲染到独立模块。
  *   V1.1 (2026-09-07) 格式化/差分/显示缓冲移入本模块；测量量改用 g_v/g_i/g_p 数组
  *                     及 g_temp/g_wh/g_pwm。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "theme.h"
#include "global.h"
#include "st7735.h"
#include "Font_asc.h"

#include <string.h>

/* 主界面进度条（主题内部持有，不同主题可不同位置/颜色/量程） */
static ProgressBar votageBar;
static ProgressBar currentBar;
static ProgressBar powerBar;

/* 主题内部的显示缓冲：格式化、差分、绘制都在本模块完成 */
static char V1[32];
static char I1[32];
static char P1[32];
static char temp[32];
static char tempPwm[32];
static char tempWh[32];

/* 主题 0：深色（当前默认布局） */
static void Theme0_Frame(void)
{
  LCD_Clear(BLACK);

  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 3, GREEN_UI, BLACK, "U");
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 27, RED_UI, BLACK, "I");
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 51, BLUE_UI, BLACK, "P");
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 73, ORANGE, BLACK, "T");
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 58, 73, YELLOW_UI, BLACK, "F");
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 88, 73, SKYBLUE, BLACK, "wh");

  ProgressBar_Init(&votageBar,  88, 28, 68, 3, 0.0f,  30.0f, GREEN_UI);
  ProgressBar_Init(&currentBar, 88, 48, 68, 3, 0.0f,   5.0f, RED_UI);
  ProgressBar_Init(&powerBar,   88, 68, 68, 3, 0.0f, 150.0f, BLUE_UI);

  ProgressBar_Redraw(&votageBar,  g_v[0], BLACK);
  ProgressBar_Redraw(&currentBar, g_i[0], BLACK);
  ProgressBar_Redraw(&powerBar,   g_p[0], BLACK);
}

static void Theme0_Values(void)
{
  /* 主题决定显示哪些量、怎么格式化、怎么差分 */
  formatFloatToStr(g_v[0], V1, 5, 3);
  formatFloatToStr(g_i[0], I1, 5, 3);
  formatFloatToStr(g_p[0], P1, 5, 3);
  slot_diff_from_buf(1, V1, V1);
  slot_diff_from_buf(2, I1, I1);
  slot_diff_from_buf(3, P1, P1);
  formatFloatToStr(g_temp, temp, 4, 1);
  formatFloatToStr((float)g_pwm, tempPwm, 2, 0);
  formatFloatToStr(g_wh, tempWh, 5, 3);

  ProgressBar_Update(&votageBar,  g_v[0], BLACK);
  ProgressBar_Update(&currentBar, g_i[0], BLACK);
  ProgressBar_Update(&powerBar,   g_p[0], BLACK);

  LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20,  0, GREEN_UI, BLACK, V1);
  LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20, 24, RED_UI,   BLACK, I1);
  LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20, 48, BLUE_UI,  BLACK, P1);

  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 100, 0, ORANGE,    BLACK, temp);
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 100, 24, YELLOW_UI, BLACK, tempPwm);
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 100, 48, SKYBLUE,   BLACK, tempWh);
}

/* 主题 1：浅色（测试主题），前景换成深色保证对比度 */
static void Theme1_Frame(void)
{
  LCD_Clear(WHITE);

  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 3, 0x03E0, WHITE, "U");
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 27, 0xC000, WHITE, "I");
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 51, 0x0010, WHITE, "P");
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 73, 0xA800, WHITE, "T");
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 58, 73, 0x8A00, WHITE, "F");
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 88, 73, 0x041F, WHITE, "wh");

  ProgressBar_Init(&votageBar,  88, 28, 68, 3, 0.0f,  30.0f, 0x03E0);
  ProgressBar_Init(&currentBar, 88, 48, 68, 3, 0.0f,   5.0f, 0xC000);
  ProgressBar_Init(&powerBar,   88, 68, 68, 3, 0.0f, 150.0f, 0x0010);

  ProgressBar_Redraw(&votageBar,  g_v[0], WHITE);
  ProgressBar_Redraw(&currentBar, g_i[0], WHITE);
  ProgressBar_Redraw(&powerBar,   g_p[0], WHITE);
}

static void Theme1_Values(void)
{
  formatFloatToStr(g_v[0], V1, 5, 3);
  formatFloatToStr(g_i[0], I1, 5, 3);
  formatFloatToStr(g_p[0], P1, 5, 3);
  slot_diff_from_buf(1, V1, V1);
  slot_diff_from_buf(2, I1, I1);
  slot_diff_from_buf(3, P1, P1);
  formatFloatToStr(g_temp, temp, 4, 1);
  formatFloatToStr((float)g_pwm, tempPwm, 2, 0);
  formatFloatToStr(g_wh, tempWh, 5, 3);

  ProgressBar_Update(&votageBar,  g_v[0], WHITE);
  ProgressBar_Update(&currentBar, g_i[0], WHITE);
  ProgressBar_Update(&powerBar,   g_p[0], WHITE);

  LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20,  0, 0x03E0, WHITE, V1);
  LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20, 24, 0xC000, WHITE, I1);
  LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20, 48, 0x0010, WHITE, P1);

  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 100, 0, 0xA800, WHITE, temp);
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 100, 24, 0x8A00, WHITE, tempPwm);
  LCD_ShowString(h16w8_sample, h16w8, 16, 8, 100, 48, 0x041F, WHITE, tempWh);
}

/* 集中主题 switch：编号只在这两处分发，各主题绘制在 case 里写死 */
void Theme_RenderFrame(void)
{
  /* 整帧重绘前复位差分缓存与电压/电流/功率显示缓冲 */
  slot_clear_all();
  memset(V1, ' ', BUF_LEN); V1[BUF_LEN] = '\0';
  memset(I1, ' ', BUF_LEN); I1[BUF_LEN] = '\0';
  memset(P1, ' ', BUF_LEN); P1[BUF_LEN] = '\0';

  switch (g_cfg.theme)
  {
    case 0:  Theme0_Frame();  break;
    case 1:  Theme1_Frame();  break;
    default: Theme0_Frame();  break;   /* 防御：越界回主题 0 */
  }
}

void Theme_RenderValues(void)
{
  switch (g_cfg.theme)
  {
    case 0:  Theme0_Values();  break;
    case 1:  Theme1_Values();  break;
    default: Theme0_Values();  break;
  }
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
