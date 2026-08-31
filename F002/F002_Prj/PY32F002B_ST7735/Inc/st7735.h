/**
  ******************************************************************************
  * @file    st7735.h
  * @author  Bowen (wbw20)
  * @date    2026-08-25
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20），ST7735S 0.96 寸 160x80 横屏 SPI 屏
  * @brief   ST7735S 显示驱动头文件（接口与 ST7789 驱动保持一致）
  *
  * 引脚映射（PY32F002B）：
  *   SPI_SCK  -> PB0 (AF0)
  *   SPI_MOSI -> PB7 (AF0)
  *   CS       -> PA3（GPIO）
  *   DC       -> PA6（GPIO）
  *   RST      -> PA7（GPIO）
  *   BLK      -> PA1（TIM1_CH2 PWM 调光）
  *
 * 设计说明：
 *   - PY32F002B 没有 DMA，整屏填充改为软件 16bit SPI 连续发送；
  *   - 背光由 TIM1_CH2 输出 PWM，LCD_BLK() 直接设置比较值；
  *   - 0.96 寸模组可视区 80x160，横屏 160x80，行列有偏移（横屏 x+1/y+26），
  *     由驱动内部按当前方向自动处理；
  *   - 外部函数接口（LCD_* / ProgressBar_* / slot_*）与 ST7789 驱动完全一致。
 *
 * CHANGELOG:
  *   V1.0 (2026-08-25) 基于 PY32F002B_MainBoard 的 ST7789 驱动改写为 ST7735S：
  *                     初始化序列/方向/偏移/分辨率替换，绘制与接口保持不变。
  ******************************************************************************
  */

#ifndef __ST7735_H__
#define __ST7735_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "py32f002b_ll_spi.h"
#include "py32f002b_ll_gpio.h"
#include <stdint.h>
#include <stdbool.h>

/* 控制引脚（普通 GPIO，速度要求高） */
#define DC_PIN                LL_GPIO_PIN_6
#define DC_H()                LL_GPIO_SetOutputPin(GPIOA, DC_PIN)
#define DC_L()                LL_GPIO_ResetOutputPin(GPIOA, DC_PIN)

#define CS_PIN                LL_GPIO_PIN_3
#define CS_H()                LL_GPIO_SetOutputPin(GPIOA, CS_PIN)
#define CS_L()                LL_GPIO_ResetOutputPin(GPIOA, CS_PIN)

#define RST_PIN               LL_GPIO_PIN_7
#define RST_H()               LL_GPIO_SetOutputPin(GPIOA, RST_PIN)
#define RST_L()               LL_GPIO_ResetOutputPin(GPIOA, RST_PIN)

/* 背光：PY32 上由 TIM1_CH2(PA1) PWM 控制，
   若硬件上背光是普通 GPIO，请把 LCD_BLK_USE_PWM 改为 0 并改 BLK 引脚宏 */
#define LCD_BLK_USE_PWM       1
#define BLK_PIN               LL_GPIO_PIN_1
#define BLK_H()               LL_GPIO_SetOutputPin(GPIOA, BLK_PIN)
#define BLK_L()               LL_GPIO_ResetOutputPin(GPIOA, BLK_PIN)

/* ST7735 所在的 SPI */
#ifndef ST7735_SPI
#define ST7735_SPI            SPI1
#endif

/* 屏幕方向：0/2 竖屏(80x160)，1/3 横屏(160x80)，默认横屏 */
#ifndef USE_HORIZONTAL
#define USE_HORIZONTAL        1
#endif

#if (USE_HORIZONTAL == 0) || (USE_HORIZONTAL == 2)
#define LCD_W                 80
#define LCD_H                 160
#else
#define LCD_W                 160
#define LCD_H                 80
#endif

/* 常用颜色（RGB565） */
#define WHITE                 0xFFFF
#define BLACK                 0x0000
#define RED                   0xF800
#define YELLOW                0xffe0
#define GRAY                  0x9cf3
#define ORANGE                0xfd00
#define GRAY_UI               0x2965
#define RED_UI                0xF227
#define YELLOW_UI             0xfea9
#define BLUE_UI               0x7d1f
#define BLUE_UI_2             0x055E
#define GREEN_UI              0x5ee7
#define PINK                  0xfedb
#define SKYBLUE               0x07fe

/* 差分刷新槽位配置 */
#define MAX_SLOTS             16
#define BUF_LEN               (32 - 1)
#define INVALID_SLOT_ID       0xFFFFu

void LCD_Reset(void);
void LCD_Init(uint16_t color);
void LCD_SpinScreen(uint8_t locate);
void LCD_BLK(uint8_t duty);
void LCD_Fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color);
void LCD_Clear(uint16_t color);
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawCircle(uint16_t X, uint16_t Y, uint16_t R, uint16_t fc, uint8_t thickness);
void LCD_DrawCircle_Fill(uint16_t X, uint16_t Y, uint16_t R, uint16_t fc);
void LCD_DrawDashedCircle(uint16_t X, uint16_t Y, uint16_t R, uint16_t fc, uint8_t thickness, uint8_t segments);
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Color);
void LCD_DrawRect(uint16_t X, uint16_t Y, uint16_t W, uint16_t H, uint16_t fc, uint8_t thickness);
void LCD_DrawRect_Fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bc);
void LCD_DrawTriangel(uint16_t x, uint16_t y, uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color);
void LCD_FillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t color);
void LCD_DrawRoundRectStroke(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t r, uint16_t t, uint16_t color);
void LCD_ShowImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *p);
void LCD_ShowPicture(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t pic[]);

/* 进度条（增量绘制） */
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t max_len;
    uint16_t height;
    float    min_val;
    float    max_val;
    uint16_t fg_color;
    uint16_t prev_len;
} ProgressBar;
void ProgressBar_Init(ProgressBar *pb, uint16_t x, uint16_t y, uint16_t max_len, uint16_t height, float min_val, float max_val, uint16_t fg_color);
void ProgressBar_Update(ProgressBar *pb, float cur_val, uint16_t bg_color);
void ProgressBar_Redraw(ProgressBar *pb, float cur_val, uint16_t bg_color);

/* 字符绘制 */
void LCD_ShowString(const char *sample, const unsigned char *data, uint8_t w, uint8_t h,
                    uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, char *str);
void LCD_ShowString_24_12(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *c);
void LCD_ShowString_16_8(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, char *c);

/* 差分刷新槽位 */
void slot_module_init(void);
int  slot_diff_from_buf(uint16_t id, const char in[BUF_LEN], char out[BUF_LEN]);
int  slot_diff_from_cstr(uint16_t id, const char *s, char out[BUF_LEN]);
void slot_clear_id(uint16_t id);
void slot_clear_all(void);

/* SPI 底层发送（供驱动内部与上层复用） */
__STATIC_INLINE void LCD_SendIndex(uint8_t cmd)
{
    DC_L();
    CS_L();
    LL_SPI_TransmitData8(ST7735_SPI, cmd);
    while ((LL_SPI_IsActiveFlag_TXE(ST7735_SPI) == 0) || (LL_SPI_IsActiveFlag_BSY(ST7735_SPI) != 0)) { }
    CS_H();
    DC_H();
}

__STATIC_INLINE void LCD_SendData(uint8_t data)
{
    DC_H();
    CS_L();
    LL_SPI_TransmitData8(ST7735_SPI, data);
    while ((LL_SPI_IsActiveFlag_TXE(ST7735_SPI) == 0) || (LL_SPI_IsActiveFlag_BSY(ST7735_SPI) != 0)) { }
    CS_H();
}

__STATIC_INLINE void LCD_Send16Bit(uint16_t data)
{
    DC_H();
    CS_L();
    LL_SPI_TransmitData8(ST7735_SPI, (uint8_t)(data >> 8));
    while (LL_SPI_IsActiveFlag_TXE(ST7735_SPI) == 0) { }
    LL_SPI_TransmitData8(ST7735_SPI, (uint8_t)(data & 0xFF));
    while ((LL_SPI_IsActiveFlag_TXE(ST7735_SPI) == 0) || (LL_SPI_IsActiveFlag_BSY(ST7735_SPI) != 0)) { }
    CS_H();
}

#ifdef __cplusplus
}
#endif

#endif /* __ST7735_H__ */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

