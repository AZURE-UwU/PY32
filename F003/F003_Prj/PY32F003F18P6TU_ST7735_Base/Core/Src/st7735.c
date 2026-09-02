/******************************************************************************
 * @file    st7735.c
 * @brief   ST7735S LCD 驱动
 * @details This file contains LCD initialization, basic drawing primitives,
 *          and example usage. It demonstrates multiple comment styles used
 *          in embedded projects: file header, Doxygen, block separators,
 *          inline comments, changelog, and usage notes.
 *
 * @author  Bowen
 * @date    2026-08-29
 * @version 1.2.1
 *
 * @hardware
 *   Display:  ST7735S 0.96inch 160x80（横屏）SPI
 *   Pins:
 *     - SPI_SCK  -> PB0 (AF0)
 *     - SPI_MOSI -> PB7 (AF0)
 *     - CS       -> PA3
 *     - DC       -> PA6
 *     - RST      -> PA7
 *     - BLK      -> PA1 (TIM1_CH2 PWM)
 *
 * @build
 *   - Compiler: arm-none-eabi-gcc
 *   - CFLAGS: -O2 -g -Wall -Wextra
 *
 ******************************************************************************/
/******************************************************************************
 * @license
 *   Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
 *   (CC BY-NC-SA 4.0)
 *
 *   本源码仅供学习与个人使用，禁止任何商业用途。
 *   使用者必须：
 *     - 署名 (BY)：在使用或修改本代码时注明原作者 Bowen。
 *     - 非商业 (NC)：不得将本代码或其修改版本用于商业目的。
 *     - 相同方式共享 (SA)：若修改或衍生本代码，必须以相同的 CC BY-NC-SA 协议发布。
 *
 *   本代码按“现状”提供，不附带任何保证。作者不对使用过程中产生的任何损失负责。
 *
 *   完整协议文本请参阅：
 *   https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
 *
 * @note
 *   作者本人保留商业使用权，并可另行授权他人商业使用。
 *   若您已获得作者的商业授权，请遵循商业授权协议的条款，
 *   而非 CC BY-NC-SA 的限制。
 *
 *   商业授权特别说明：
 *     - 商业授权用户可以在商业项目中使用本源码。
 *     - 商业授权用户不得再次分发本源码或其修改版本，
 *       包括但不限于公开发布、转售或再授权。
 *     - 商业授权仅限于获得许可的个人或组织，不得转移。
 ******************************************************************************/

/* ============================================================================
   CHANGELOG
   ----------------------------------------------------------------------------
   2026-01-29  v1.0.0  Bowen  - Initial release
	 2026-02-08  v1.0.1  Bowen  - 修改DMA通道，修复Fill函数中DMA BUGs
	 2026-02-08  v1.1.0  Bowen  - 修改画圆，方形函数，增加输入厚度，修改进度条
	 2026-02-11  v1.1.1  Bowen  - 增加虚线圆函数
	 2026-03-19  v1.2.0  Bowen  - 修改进度条函数，结构体变更，不传入背景参数和之前不再兼容
	 2026-03-19  v1.2.1  Bowen  - 增加进度条全局重绘函数，无需重新init
	 2026-08-19  v2.0.0  Bowen  - 移植 PY32F002B：去 DMA、背光接 TIM1_CH2
	 2026-08-25  v1.0.0  Bowen  - ST7735S 版：替换初始化/方向/偏移/分辨率，
	                              绘制函数与外部接口保持与 ST7789 版一致
	 2026-08-29  v1.1.0  Bowen  - 移植 PY32F003F18P6TU：SCK/MOSI=PA1/PA2，
	                              DC=PA3；CS/RST/BLK 省脚：复位走 SWRESET，
	                              背光用 DISPON/DISPOFF 开关显示代替
	 2026-08-29  v1.2.0  Bowen  - PF 引脚复用：恢复 CS(PF1)/RST(PF4) 硬件复位，
	                              背光改 TIM14_CH1(PF0) PWM 无级调光
	 2026-08-30  v1.2.1  Bowen  - 统一字符显示接口：全部调用改走通用
	                              LCD_ShowString(sample,data,h,w,...)；
	                              LCD_ShowString_16_8/24_12 实现移除
	                              （#if 0 保留对比），头文件接口保留
	 ========================================================================== */

#include "st7735.h"
#include "Font_asc.h"
#include "bsp_tim.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

/* ------------------------------------------------------------------
   轻量数学辅助：PY32F002B 只有 24KB Flash，
   避免链接浮点数学库（sqrtf/atan2f 会引入数 KB 库代码）
   ------------------------------------------------------------------ */

/* 整数开平方（逐位逼近），用于圆角矩形像素计算 */
static uint32_t isqrt_u32(uint32_t n)
{
    uint32_t res = 0;
    uint32_t bit = 1UL << 30;
    while (bit > n)
    {
        bit >>= 2;
    }
    while (bit)
    {
        if (n >= res + bit)
        {
            n -= res + bit;
            res = (res >> 1) + bit;
        }
        else
        {
            res >>= 1;
        }
        bit >>= 2;
    }
    return res;
}

/* 快速 atan2：多项式近似，误差 < 0.01 rad，画虚线圆足够用 */
static float fast_atan2(float y, float x)
{
    const float PI_F      = 3.14159265358979f;
    const float HALF_PI_F = 1.57079632679490f;

    float ax = (x >= 0.0f) ? x : -x;
    float ay = (y >= 0.0f) ? y : -y;
    float M  = (ax > ay) ? ax : ay;
    float m  = (ax > ay) ? ay : ax;

    if (M < 1e-12f)
    {
        return 0.0f;
    }

    float z = m / M;
    float a = 0.785398163397448f
              - z * (0.785398163397448f
              - z * z * (0.2167732100289f
              - 0.05982215922919f * z * z));

    float th;
    if (ax >= ay)
    {
        if (x >= 0.0f)
        {
            th = (y >= 0.0f) ? a : -a;
        }
        else
        {
            th = (y >= 0.0f) ? (PI_F - a) : (a - PI_F);
        }
    }
    else
    {
        if (y >= 0.0f)
        {
            th = (x >= 0.0f) ? (HALF_PI_F - a) : (HALF_PI_F + a);
        }
        else
        {
            th = (x >= 0.0f) ? (a - HALF_PI_F) : (-HALF_PI_F - a);
        }
    }
    return th;
}



/* ------------------------------------------------------------------
   初始化硬件函数
   ------------------------------------------------------------------ */

/* 当前方向：0=竖屏(MV=0)，1=横屏(MV=1)。SetRegion 按此选择行列偏移 */
static uint8_t s_lcd_mv = 1;

/* 四个方向的 MADCTL（与 0.96 寸 ST7735S 模组实测匹配）：
   0=竖屏 0x08，1=横屏 0xA8，2=竖屏180° 0x48，3=横屏180° 0x68 */
static const uint8_t MADCTL_TABLE[4] = {0x08, 0xA8, 0x48, 0x68};

/* 硬件复位 */
void LCD_Reset(void)
{
    RST_L();        /* 拉低复位（ST7735S 要求低电平保持 >= 100ms） */
    HAL_Delay(100);
    RST_H();        /* 释放复位 */
    HAL_Delay(50);
}

/* 初始化 */
void LCD_Init(uint16_t color)
{
    LCD_Reset();
    HAL_Delay(120);
    LCD_BLK(100);

    /* 退出睡眠 */
    LCD_SendIndex(0x11);
    HAL_Delay(120);

    /* 颜色反转：0.96 寸模组必须开启，否则颜色不对 */
    LCD_SendIndex(0x21);

    /* 帧率控制（正常/空闲/部分模式） */
    LCD_SendIndex(0xB1);
    LCD_SendData(0x05);
    LCD_SendData(0x3A);
    LCD_SendData(0x3A);
    LCD_SendIndex(0xB2);
    LCD_SendData(0x05);
    LCD_SendData(0x3A);
    LCD_SendData(0x3A);
    LCD_SendIndex(0xB3);
    LCD_SendData(0x05);
    LCD_SendData(0x3A);
    LCD_SendData(0x3A);
    LCD_SendData(0x05);
    LCD_SendData(0x3A);
    LCD_SendData(0x3A);
    LCD_SendIndex(0xB4);
    LCD_SendData(0x03);

    /* 电源控制 */
    LCD_SendIndex(0xC0);
    LCD_SendData(0x62);
    LCD_SendData(0x02);
    LCD_SendData(0x04);
    LCD_SendIndex(0xC1);
    LCD_SendData(0xC0);
    LCD_SendIndex(0xC2);
    LCD_SendData(0x0D);
    LCD_SendData(0x00);
    LCD_SendIndex(0xC3);
    LCD_SendData(0x8D);
    LCD_SendData(0x6A);
    LCD_SendIndex(0xC4);
    LCD_SendData(0x8D);
    LCD_SendData(0xEE);
    LCD_SendIndex(0xC5);
    LCD_SendData(0x0E);

    /* 伽马校正 */
    LCD_SendIndex(0xE0);
    LCD_SendData(0x10);
    LCD_SendData(0x0E);
    LCD_SendData(0x02);
    LCD_SendData(0x03);
    LCD_SendData(0x0E);
    LCD_SendData(0x07);
    LCD_SendData(0x02);
    LCD_SendData(0x07);
    LCD_SendData(0x0A);
    LCD_SendData(0x12);
    LCD_SendData(0x27);
    LCD_SendData(0x37);
    LCD_SendData(0x00);
    LCD_SendData(0x0D);
    LCD_SendData(0x0E);
    LCD_SendData(0x10);
    LCD_SendIndex(0xE1);
    LCD_SendData(0x10);
    LCD_SendData(0x0E);
    LCD_SendData(0x03);
    LCD_SendData(0x03);
    LCD_SendData(0x0F);
    LCD_SendData(0x06);
    LCD_SendData(0x02);
    LCD_SendData(0x08);
    LCD_SendData(0x0A);
    LCD_SendData(0x13);
    LCD_SendData(0x26);
    LCD_SendData(0x36);
    LCD_SendData(0x00);
    LCD_SendData(0x0D);
    LCD_SendData(0x0E);
    LCD_SendData(0x10);

    /* 16bit 像素格式 */
    LCD_SendIndex(0x3A);
    LCD_SendData(0x05);

    /* 初始方向：默认横屏（USE_HORIZONTAL 默认 1） */
    LCD_SendIndex(0x36);
    if (USE_HORIZONTAL == 0)
        LCD_SendData(MADCTL_TABLE[0]);
    else if (USE_HORIZONTAL == 1)
        LCD_SendData(MADCTL_TABLE[1]);
    else if (USE_HORIZONTAL == 2)
        LCD_SendData(MADCTL_TABLE[2]);
    else
        LCD_SendData(MADCTL_TABLE[3]);
    s_lcd_mv = ((USE_HORIZONTAL == 1) || (USE_HORIZONTAL == 3)) ? 1U : 0U;

    /* 打开显示 */
    LCD_SendIndex(0x29);
    LCD_Fill(0, 0, LCD_W, LCD_H, color);
}


/* ------------------------------------------------------------------
   控制函数
   ------------------------------------------------------------------ */

/*设置显示区域*/
void LCD_SetRegion(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
    uint16_t xo;
    uint16_t yo;

    /* 0.96 寸模组在 132x162 的驱动芯片上只露出 80x160 可视区：
       横屏(MV=1)偏移 x+1/y+26，竖屏(MV=0)偏移 x+26/y+1 */
    if (s_lcd_mv)
    {
        xo = 1U;
        yo = 26U;
    }
    else
    {
        xo = 26U;
        yo = 1U;
    }

    LCD_SendIndex(0x2a); /* 列地址设置 */
    LCD_Send16Bit(xs + xo);
    LCD_Send16Bit(xe + xo);
    LCD_SendIndex(0x2b); /* 行地址设置 */
    LCD_Send16Bit(ys + yo);
    LCD_Send16Bit(ye + yo);
    LCD_SendIndex(0x2c); /* 储存器写 */
}


/* 设置光标位置 */
void LCD_SetCursor(uint16_t x, uint16_t y)
{
    LCD_SetRegion(x, y, x, y);
}


/*旋转屏幕*/
void LCD_SpinScreen(uint8_t locate){
    locate &= 0x03U;               /* 防御：非法方向值取低两位 */
    s_lcd_mv = ((locate == 1U) || (locate == 3U)) ? 1U : 0U;
    LCD_SendIndex(0x36);           /* 屏幕的显示方向、像素读写顺序 */
    LCD_SendData(MADCTL_TABLE[locate]);
}
/* 设置背光 */

////无PWM
//void LCD_BLK(uint8_t io){
//	if(io) BLK_H();
//	else BLK_L();
//}

void LCD_BLK(uint8_t duty){
  /* 背光由 TIM14_CH1(PF0) 输出 PWM，实现无级调光 */
#if LCD_BLK_USE_PWM
  BSP_PWM_SetBLK(duty);
#else
  if (duty) BLK_H();
  else BLK_L();
#endif
}








/* ------------------------------------------------------------------
   基本绘制
   ------------------------------------------------------------------ */

/* 填充矩形区域 */

//非DMA测试使用
//void LCD_Fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
//{
//    LCD_SetRegion(xs, ys, xe, ye);
//    for (uint32_t i = 0; i < (xe - xs + 1) * (ye - ys + 1); i++)
//    {
//        LCD_Send16Bit(color);
//    }
//}

void LCD_Fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
    uint32_t total   = (xe - xs) * (ye - ys) + 1;

    // 设置显示区域
    LCD_SetRegion(xs, ys, xe - 1, ye - 1);
    CS_L();

    /* 切入 16 位 SPI 模式：一次写半个字，效率接近原来的 DMA 方案 */
    LL_SPI_Disable(SPI1);
    LL_SPI_SetDataWidth(SPI1, LL_SPI_DATAWIDTH_16BIT);
    LL_SPI_Enable(SPI1);

    /* PY32F002B 没有 DMA，改为软件连续发送同一个颜色值 */
    for (uint32_t i = 0; i < total; i++)
    {
        LL_SPI_TransmitData16(ST7735_SPI, color);
        while (LL_SPI_IsActiveFlag_TXE(ST7735_SPI) == 0) { }
    }
    /* 等最后一个字节真正移出后再切回 8 位，防止尾巴被截断 */
    while (LL_SPI_IsActiveFlag_BSY(ST7735_SPI) != 0) { }

    // 恢复 8 位 SPI 模式
    LL_SPI_Disable(SPI1);
    LL_SPI_SetDataWidth(SPI1, LL_SPI_DATAWIDTH_8BIT);
    LL_SPI_Enable(SPI1);

    CS_H();
}


/* 清屏 */
void LCD_Clear(uint16_t color)
{
    LCD_Fill(0, 0, LCD_W, LCD_H, color);
}


/* 画点 */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_SetCursor(x, y);
    LCD_Send16Bit(color);
}


/* 空心圆，支持厚度，避免边角漏点 */
void LCD_DrawCircle(uint16_t X, uint16_t Y, uint16_t R, uint16_t fc, uint8_t thickness)
{
    if (thickness < 1) thickness = 1;

    // 遍历整个圆的外接矩形
    for (int dx = -R; dx <= R; dx++) {
        for (int dy = -R; dy <= R; dy++) {
            int dist2 = dx*dx + dy*dy; // 距离平方
            int r_outer2 = R * R;
            int r_inner2 = (R - thickness + 1) * (R - thickness + 1);

            // 在外圆内 && 在内圆外 → 属于圆环区域
            if (dist2 <= r_outer2 && dist2 >= r_inner2) {
                LCD_DrawPoint(X + dx, Y + dy, fc);
            }
        }
    }
}

#define TWO_PI_F 6.28318530717959f  /* 2π，float 精度足够画虚线圆 */
/* 虚线圆，支持厚度和虚线段数 */
void LCD_DrawDashedCircle(uint16_t X, uint16_t Y, uint16_t R, uint16_t fc, uint8_t thickness, uint8_t segments)
{
    if (thickness < 1) thickness = 1;
    if (segments < 1) segments = 1;

    // 每个虚线段对应的角度范围
    float segmentAngle = TWO_PI_F / (float)segments;

    for (int dx = -R; dx <= R; dx++) {
        for (int dy = -R; dy <= R; dy++) {
            int dist2 = dx*dx + dy*dy;
            int r_outer2 = R * R;
            int r_inner2 = (R - thickness + 1) * (R - thickness + 1);

            if (dist2 <= r_outer2 && dist2 >= r_inner2) {
                float angle = fast_atan2((float)dy, (float)dx);
                if (angle < 0) angle += TWO_PI_F;

                // 当前点属于第几段
                int segIndex = (int)(angle / segmentAngle);

                // 偶数段绘制，奇数段空白 → 形成虚线
                if (segIndex % 2 == 0) {
                    LCD_DrawPoint(X + dx, Y + dy, fc);
                }
            }
        }
    }
}

/* 实心圆 */
void LCD_DrawCircle_Fill(uint16_t X, uint16_t Y, uint16_t R, uint16_t fc)
{
    unsigned short a = 0, b = R;
    int c = 3 - 2 * R;

    while (a <= b)
    {
        // 在对称的八个方向上画水平线段，而不是单点
        LCD_DrawLine(X - a, Y - b, X + a, Y - b, fc); // 上
        LCD_DrawLine(X - a, Y + b, X + a, Y + b, fc); // 下
        LCD_DrawLine(X - b, Y - a, X + b, Y - a, fc); // 左上
        LCD_DrawLine(X - b, Y + a, X + b, Y + a, fc); // 左下

        if (c < 0)
        {
            c = c + 4 * a + 6;
        }
        else
        {
            c = c + 4 * (a - b) + 10;
            b--;
        }
        a++;
    }
}


/* 直线 */
void LCD_DrawLine(uint16_t x0, uint16_t y0,uint16_t x1, uint16_t y1,uint16_t Color)
{
	int dx,            // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping	

	LCD_SetCursor(x0,y0);
	dx = x1-x0;//计算x距离
	dy = y1-y0;//计算y距离

	if (dx>=0) x_inc = 1; 
	else{
		x_inc = -1;
		dx    = -dx;  
	} 
	if (dy>=0) y_inc = 1; 
	else{
		y_inc = -1;
		dy    = -dy; 
	} 
	dx2 = dx << 1;
	dy2 = dy << 1;
	if (dx > dy) //x距离大于y距离，那么每个x轴上只有一个点，每个y轴上有若干个点
	{           //且线的点数等于x距离，以x轴递增画点
		error = dy2 - dx; 
		for (index=0; index <= dx; index++){
			LCD_DrawPoint(x0,y0,Color);
			if (error >= 0)
			{
				error-=dx2;
				y0+=y_inc;
			}
			error+=dy2;
			x0+=x_inc;
		}
	}
	else
	{
		error = dx2 - dy; 

		// draw the line
		for (index=0; index <= dy; index++)
		{
			LCD_DrawPoint(x0,y0,Color);
			if (error >= 0){
				error-=dy2;
				x0+=x_inc;
			}
			error+=dx2;
			y0+=y_inc;
		}
	}
}

/* 空心矩形，支持厚度 */
void LCD_DrawRect(uint16_t X, uint16_t Y,
                  uint16_t W, uint16_t H,
                  uint16_t fc, uint8_t thickness)
{
    if (thickness < 1) thickness = 1;

    // 上边框
    for (uint16_t dy = 0; dy < thickness; dy++) {
        for (uint16_t dx = 0; dx < W; dx++) {
            LCD_DrawPoint(X + dx, Y + dy, fc);
        }
    }

    // 下边框
    for (uint16_t dy = 0; dy < thickness; dy++) {
        for (uint16_t dx = 0; dx < W; dx++) {
            LCD_DrawPoint(X + dx, Y + H - 1 - dy, fc);
        }
    }

    // 左边框
    for (uint16_t dx = 0; dx < thickness; dx++) {
        for (uint16_t dy = 0; dy < H; dy++) {
            LCD_DrawPoint(X + dx, Y + dy, fc);
        }
    }

    // 右边框
    for (uint16_t dx = 0; dx < thickness; dx++) {
        for (uint16_t dy = 0; dy < H; dy++) {
            LCD_DrawPoint(X + W - 1 - dx, Y + dy, fc);
        }
    }
}

/* 顶点三角形 */
void LCD_DrawTriangel(uint16_t x, uint16_t y, uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{
    LCD_DrawLine(x, y, xs, ys, color);
    LCD_DrawLine(xs, ys, xe, ye, color);
    LCD_DrawLine(xe, ye, x, y, color);
}

/* 实心矩形 */
void LCD_DrawRect_Fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bc) 
{
LCD_Fill(x,y, x + w, y + h, bc);
}

/* 圆角矩形 */
void LCD_FillRoundRect(uint16_t x, uint16_t y,
                       uint16_t w, uint16_t h,
                       uint16_t r, uint16_t color)
{
    // 如果半径为 0，当成普通矩形处理
    if (r == 0) {
        LCD_Fill(x, y, x + w, y + h, color);
        return;
    }

    // 保证圆角不会超过宽高一半
    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;

    // 1. 填充中间宽条矩形
    LCD_Fill(x + r,      y, x + w - r,   y + h,     color);

    // 2. 填充左右竖条矩形
    LCD_Fill(x,          y + r, x + r,       y + h - r, color);
    LCD_Fill(x + w - r,  y + r, x + w,       y + h - r, color);

    // 3. 填充四个圆角
    for (int16_t dy = 0; dy < r; dy++) {
        int32_t sq = (int32_t)((uint32_t)r * (uint32_t)r
                               - (uint32_t)(r - 1 - dy) * (uint32_t)(r - 1 - dy));
        int16_t dx = (int16_t)isqrt_u32((uint32_t)sq);

        // 上半圆：左、右两个角
        LCD_Fill(x + r - dx,        y + dy, x + r,        y + dy + 1, color);
        LCD_Fill(x + w - r,         y + dy, x + w - r + dx, y + dy + 1, color);

        // 下半圆：左、右两个角
        LCD_Fill(x + r - dx,        y + h - dy - 1, x + r,        y + h - dy, color);
        LCD_Fill(x + w - r,         y + h - dy - 1, x + w - r + dx, y + h - dy, color);
    }
}


/* 空心圆角矩形 */
void LCD_DrawRoundRectStroke(uint16_t x, uint16_t y,
                             uint16_t w, uint16_t h,
                             uint16_t r, uint16_t t,
                             uint16_t color)
{
    // 基本校验：宽高必须至少 2*r，且粗细 t > 0 且 2*t < min(w,h)
    if (w  <  2 * r ||
        h  <  2 * r ||
        t == 0 ||
        t*2 >= w ||
        t*2 >= h) {
        return;
    }

    // 内圆角半径
    uint16_t ri = (r > t ? r - t : 0);

    // 对每一行（dy = 0 到 h-1）做：
    for (uint16_t dy = 0; dy < h; dy++) {
        uint16_t Y = y + dy;

        // —— 计算外圆角切除量 cut_out ——  
        // oy = 到外圆角圆心的垂直距离
        int16_t oy = 0;
        if      (dy <  r)        oy =  r - 1 - dy;           // 顶部圆角区
        else if (dy >= h - r)    oy = dy - (h - r);          // 底部圆角区

        uint16_t cut_out = 0;
        if (oy > 0) {
            // 水平可保留长度 dx = sqrt(r^2 - oy^2)
            uint16_t dx = (uint16_t)isqrt_u32((uint32_t)r * (uint32_t)r - (uint32_t)oy * (uint32_t)oy);
            cut_out = r - dx;
        }
        // 外轮廓水平绘制范围
        uint16_t x0 = x + cut_out;
        uint16_t x1 = x + w - 1 - cut_out;

        // —— 计算内圆角切除量 cut_in ——  
        // 只有在中间区域（排除上下 t 行）才有内圆角影响
        if (ri > 0 && dy >= t && dy < h - t) {
            uint16_t dy_in = dy - t;
            int16_t  oy_in = 0;
            uint16_t h_in = h - 2*t;

            if      (dy_in <  ri)         oy_in =  ri - 1 - dy_in;
            else if (dy_in >= h_in - ri)  oy_in = dy_in - (h_in - ri);

            uint16_t cut_in = 0;
            if (oy_in > 0) {
                uint16_t dx_in = (uint16_t)isqrt_u32((uint32_t)ri * (uint32_t)ri - (uint32_t)oy_in * (uint32_t)oy_in);
                cut_in = ri - dx_in;
            }

            // 内轮廓水平范围
            uint16_t xi0 = x + t + cut_in;
            uint16_t xi1 = x + t + (w - 2*t) - 1 - cut_in;

            // 左条：从外左到内左-1
            for (uint16_t xx = x0; xx < xi0; xx++) {
                LCD_DrawPoint(xx, Y, color);
            }
            // 右条：从内右+1 到外右
            for (uint16_t xx = xi1 + 1; xx <= x1; xx++) {
                LCD_DrawPoint(xx, Y, color);
            }
        }
        else {
            // 不在内圆角影响区，整条都画
            for (uint16_t xx = x0; xx <= x1; xx++) {
                LCD_DrawPoint(xx, Y, color);
            }
        }
    }
}




/* ------------------------------------------------------------------
  集成绘制
   ------------------------------------------------------------------ */

/* 显示图像 @ RGB565 */
void LCD_ShowImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *p)
{
    LCD_SetRegion(x, y, x + width - 1, y + height - 1);
    for (uint32_t i = 0; i < width * height * 2; i++)
    {
        LCD_SendData(p[i]);
    }
}


void LCD_ShowPicture(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t pic[])
{
    uint32_t num  = width * height * 2;   // 总字节数（RGB565 每像素 2 字节）

    // 设置显示区域
    LCD_SetRegion(x, y, x + width - 1, y + height - 1);
    CS_L();

    /* PY32F002B 没有 DMA：逐字节软件发送。
       DC 已在 LCD_SetRegion 里拉高，这里在 CS 有效期内连续发送即可 */
    for (uint32_t i = 0; i < num; i++)
    {
        LL_SPI_TransmitData8(ST7735_SPI, pic[i]);
        while (LL_SPI_IsActiveFlag_TXE(ST7735_SPI) == 0) { }
    }

    while (LL_SPI_IsActiveFlag_BSY(ST7735_SPI) != 0) { }

    CS_H();
}

/* 进度条 */


//初始化进度条
void ProgressBar_Init(
    ProgressBar *pb,
    uint16_t     x,
    uint16_t     y,
    uint16_t     max_len,
    uint16_t     height,
    float        min_val,
    float        max_val,
    uint16_t     fg_color)
{
    pb->x        = x;
    pb->y        = y;
    pb->max_len  = max_len;
    pb->height   = height;
    pb->min_val  = min_val;
    pb->max_val  = max_val;
    pb->fg_color = fg_color;
    pb->prev_len = 0;
}

/** 更新进度条（只传入当前值） */
void ProgressBar_Update(ProgressBar *pb, float cur_val, uint16_t bg_color)
{
    /* 1. 计算比例 */
    float ratio = (cur_val - pb->min_val) / (pb->max_val - pb->min_val);
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    /* 2. 新像素长度 */
    uint16_t new_len = (uint16_t)(ratio * pb->max_len + 0.5f);

    /* 3. 长度未变直接返回 */
    int16_t delta = (int16_t)new_len - (int16_t)pb->prev_len;
    if (delta == 0) return;

    /* 4. 绘制增量或清除多余 */
    if (delta > 0) {
        LCD_DrawRect_Fill(pb->x + pb->prev_len, pb->y, delta, pb->height, pb->fg_color);
    } else {
        LCD_DrawRect_Fill(pb->x + new_len, pb->y, (uint16_t)(-delta), pb->height, bg_color);
    }

    /* 5. 更新状态 */
    pb->prev_len = new_len;
}


/** 全局重绘进度条（适合屏幕旋转后调用） */
void ProgressBar_Redraw(ProgressBar *pb, float cur_val, uint16_t bg_color)
{
    /* 1. 计算比例 */
    float ratio = (cur_val - pb->min_val) / (pb->max_val - pb->min_val);
    if (ratio < 0.0f) ratio = 0.0f;
    if (ratio > 1.0f) ratio = 1.0f;

    /* 2. 当前长度 */
    uint16_t cur_len = (uint16_t)(ratio * pb->max_len + 0.5f);

    /* 3. 先清空整个区域为背景色 */
    LCD_DrawRect_Fill(pb->x, pb->y, pb->max_len, pb->height, bg_color);

    /* 4. 再绘制前景部分 */
    if (cur_len > 0) {
        LCD_DrawRect_Fill(pb->x, pb->y, cur_len, pb->height, pb->fg_color);
    }

    /* 5. 更新状态 */
    pb->prev_len = cur_len;
}

/* ------------------------------------------------------------------
  字符绘制
   ------------------------------------------------------------------ */
/* 查找索引位置 */
int findCharPos(const char* sample, const char* target)
{
    if (target == NULL || *target == '\0') return -1;
    char c = *target;
    for (int i = 0; sample[i] != '\0'; i++) {
        if (sample[i] == c) return i;
    }
    return -1;
}

/*-------------------
		SAMPLE
-------------------*/

/*
 * data: 字模数据，按字符顺序排列，每字符占 bytes_per_char = ((w+7)/8) * h 字节
 * w: 字符宽度（像素），可以不是8的倍数
 * h: 字符高度（像素）
 * fc: 前景色，bc: 背景色）
 */
void LCD_ShowChar(const char *sample,
                  const unsigned char *data,
                  uint8_t h, uint8_t w,
                  uint16_t x, uint16_t y,
                  uint16_t fc, uint16_t bc,
                  char c)
{
    if (c == ' ') return;

    unsigned char uc = (unsigned char)c;
    if (uc < 32 || uc > 126) uc = '?';

    char t[2] = { (char)uc, '\0' };
    int pos = findCharPos(sample, t);
    if (pos < 0) {
        char q[2] = { '?', '\0' };
        pos = findCharPos(sample, q);
        if (pos < 0) return;
    }

    /* 每行占用字节数（向上取整） */
    uint32_t row_bytes = (uint32_t)( (w + 7) / 8 );
    uint32_t bytes_per_char = row_bytes * (uint32_t)h;
    uint32_t base = (uint32_t)pos * bytes_per_char;

    for (uint32_t row = 0; row < h; row++) {
        uint32_t row_base = base + row * row_bytes;
        for (uint32_t col = 0; col < w; col++) {
            uint32_t byte_idx = row_base + (col >> 3); /* col / 8 */
            uint8_t b = data[byte_idx];
            uint8_t bit_mask = (uint8_t)(0x80 >> (col & 0x07)); /* MSB-first */
            bool pixel = (b & bit_mask) != 0;

            if (pixel)
                LCD_DrawPoint((uint16_t)(x + col), (uint16_t)(y + row), fc);
            else
                LCD_DrawPoint((uint16_t)(x + col), (uint16_t)(y + row), bc);
        }
    }
}

void LCD_ShowString(const char *sample,
                  const unsigned char *data,
                  uint8_t h, uint8_t w,
                  uint16_t x, uint16_t y,
                  uint16_t fc, uint16_t bc,
                  char* str)
{
    while (*str) {
        LCD_ShowChar(sample, data, h, w, x, y, fc, bc, *str++);
        x += w;
    }
	
}
/*-------------------
		XL 64*48
-------------------*/
#if 0 /* 未使用字号，裁掉节省 Flash */
void LCD_ShowChar_48_64(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, char c)
{
    /* 如果字符是 ASCII 空格，直接跳过不绘制 */
    if (c == ' ') return;

    /* 处理不可打印字符，映射到 '?' */
    unsigned char uc = (unsigned char)c;
    if (uc < 32 || uc > 126) uc = '?';

    /* 使用以 '\0' 结尾的临时字符串传入 findCharPos，避免传入非终止字符串 */
    char t[2] = { (char)uc, '\0' };
    int pos = findCharPos(h48w64_sample, t);
    if (pos < 0) {
        /* 找不到就尝试用 '?' 替代；若 '?' 也没定义则直接返回（不绘制） */
        char q[2] = { '?', '\0' };
        pos = findCharPos(h48w64_sample, q);
        if (pos < 0) return;
    }

    /* 每字符占 384 字节 (48 行 × 8 字节) */
    uint32_t base = (uint32_t)pos * 384U;

    for (int row = 0; row < 48; row++) {               /* 48 行 */
        uint32_t row_base = base + (uint32_t)row * 8U; /* 每行 8 字节 */
        for (int col = 0; col < 64; col++) {           /* 64 列 */
            uint32_t byte_idx = row_base + (uint32_t)(col >> 3);
            uint8_t b = h48w64[byte_idx];
            uint8_t bit_mask = (uint8_t)(0x80 >> (col & 0x07));
            bool pixel = (b & bit_mask) != 0;

            if (pixel)
                LCD_DrawPoint((uint16_t)(x + col), (uint16_t)(y + row), fc);
            else
                LCD_DrawPoint((uint16_t)(x + col), (uint16_t)(y + row), bc);
        }
    }
}

void LCD_ShowString_48_64(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *str)
{
    while (*str) {
        LCD_ShowChar_48_64(x, y, fc, bc, *str++);
        x += 64;
    }
}
#endif



/*-------------------
		L 32*16
-------------------*/
#if 0 /* 未使用字号，裁掉节省 Flash */
void LCD_ShowChar_32_16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, char c)
{
    // 在 h32w16_sample[] 中查找字符位置
    int pos = findCharPos(h32w16_sample, &c);
    if (pos < 0) {
        // 找不到就用 '?'
        pos = findCharPos(h32w16_sample, "?");
        if (pos < 0) return; // 如果 '?' 也没定义，直接返回
    }

    // 每字符占 64 字节 (32 行 × 2 字节)
    uint32_t base = pos * 64;

    for (uint8_t i = 0; i < 32; i++) {         // 32 行
        uint16_t row_data = (h32w16[base + 2*i] << 8) | h32w16[base + 2*i + 1];
        for (uint8_t j = 0; j < 16; j++) {     // 16 列
            if (row_data & (0x8000 >> j))
                LCD_DrawPoint(x + j, y + i, fc);
            else
                LCD_DrawPoint(x + j, y + i, bc);
        }
    }
}


void LCD_ShowString_32_16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *s)
{
    while (*s) {
        LCD_ShowChar_32_16(x, y, fc, bc, *s++);
        x += 16;  // 字符宽度为 16
    }
}
#endif



/*-------------------
		M 24*12
-------------------*/
void LCD_ShowChar_24_12(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, char c)
{
    // 在 h24w12_sample[] 中查找字符位置
    int pos = findCharPos(h24w12_sample, &c);
    if (pos < 0) {
        // 找不到就用 '?'
        pos = findCharPos(h24w12_sample, "x");
        if (pos < 0) return; // 如果 '?' 也没定义，直接返回
    }

    // 每字符占 48 字节 (24 行 × 2 字节)
    uint16_t base = pos * 48;

    for (int i = 0; i < 24; i++) {        // 24 行
        for (int j = 0; j < 12; j++) {    // 12 列
            bool pixel;
            if (j < 8) {
                // 前 8 列取第 0 字节
                pixel = h24w12[base + 2*i] & (0x80 >> j);
            } else {
                // 后 4 列取第 1 字节
                pixel = h24w12[base + 2*i + 1] & (0x80 >> (j - 8));
            }
            if (pixel)
                LCD_DrawPoint(x + j, y + i, fc);
            else
                LCD_DrawPoint(x + j, y + i, bc);
        }
    }
}


#if 0 /* 2026-08-30 统一走通用 LCD_ShowString(h24w12_sample, h24w12, 24, 12, ...)，旧实现保留作对比 */
void LCD_ShowString_24_12(uint16_t x, uint16_t y,  uint16_t fc, uint16_t bc,  const char *c)
{
    int len = strlen(c);  // 字符串长度
    for (int i = 0; i < len; i++) {
        LCD_ShowChar_24_12(x, y, fc, bc, c[i]);
        x += 12;           // 字符宽度为 12
    }
}
#endif


/*-------------------
		S 16*8
-------------------*/

void LCD_ShowChar_16_8(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, char c)
{
    // 在 h16w8_sample[] 中查找字符位置
    int pos = findCharPos(h16w8_sample, &c);
    if (pos < 0) {
        // 找不到就用 '?'
        pos = findCharPos(h16w8_sample, "?");
        if (pos < 0) return; // 如果 '?' 也没定义，直接返回
    }

    // 每字符占 16 字节
    int k = pos * 16;

    for (int i = 0; i < 16; i++) {       // 16 行
        for (int j = 0; j < 8; j++) {    // 8 列
            if (h16w8[k + i] & (0x80 >> j))
                LCD_DrawPoint(x + j, y + i, fc);
            else
                LCD_DrawPoint(x + j, y + i, bc);
        }
    }
}


#if 0 /* 2026-08-30 统一走通用 LCD_ShowString(h16w8_sample, h16w8, 16, 8, ...)，旧实现保留作对比 */
void LCD_ShowString_16_8(uint16_t x,uint16_t y,uint16_t fc,uint16_t bc,char *c)
{
	int t=strlen(c);
	for(int i=0;i<t;i++){
		LCD_ShowChar_16_8(x,y,fc,bc,c[i]);
		x+=8;     //字符的宽度为8 
	}		
}
#endif

/*-------------------
		XS 12*6
-------------------*/
#if 0 /* 未使用字号，裁掉节省 Flash */
void LCD_ShowChar_12_6(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, char c)
{
    // 在 h12w6_sample[] 中查找字符位置
    int pos = findCharPos(h12w6_sample, &c);
    if (pos < 0) {
        // 找不到就用 '?'
        pos = findCharPos(h12w6_sample, "?");
        if (pos < 0) return; // 如果 '?' 也没定义，直接返回
    }

    // 每字符占 12 字节
    int k = pos * 12;

    for (int i = 0; i < 12; i++) {    // 12 行
        for (int j = 0; j < 6; j++) { // 6 列
            if (h12w6[k + i] & (0x80 >> j))
                LCD_DrawPoint(x + j, y + i, fc);
            else
                LCD_DrawPoint(x + j, y + i, bc);
        }
    }
}


void LCD_ShowString_12_6(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *str)
{
    while (*str) {
        LCD_ShowChar_12_6(x, y, fc, bc, *str++);
        x += 6;  // 宽度为 6
    }

}
#endif


/* ------------------------------------------------------------------
  加速缓冲
   ------------------------------------------------------------------ */


/*
 * 通用槽位差异更新模块
 *
 * 功能：
 *  - 按 slot id 维护每个槽位的上次缓存（固定 BUF_LEN 字节）
 *  - 提供两种接口：
 *      1) 以 C 字符串输入（自动按 strlen 处理并清尾）
 *      2) 以定长缓冲输入（长度为 BUF_LEN）
 *  - 输出为定长 BUF_LEN 缓冲：若某位与历史相同则输出空格 ' '，否则输出新字节并更新历史
 *  - 提供清除单个槽位与全部槽位的函数
 *  - 可选的显示回调（用户实现）用于在发现差异时直接绘制
 *
 * 说明：
 *  - 若 id 可能为 0，请使用 INVALID_SLOT_ID 作为空闲标志（默认 0xFFFF）
 *  - slot_prev_buf 为以 '\0' 结尾的缓冲（长度 BUF_LEN+1），便于使用 strlen
 */




/* 槽位 id 表，初始化为 INVALID_SLOT_ID 表示空闲 */
static uint16_t slot_id[MAX_SLOTS];


/* 每个槽位上次保存的缓冲，保存为以 '\0' 结尾的字符串（长度 BUF_LEN） */
static char slot_prev_buf[MAX_SLOTS][BUF_LEN + 1];


/* 显示回调类型（当某个位置需要更新时调用） */
typedef void (*display_update_cb_t)(uint16_t id, int index, char ch);


/* 实现此回调并在初始化时设置，若为 NULL 则不调用 */
static display_update_cb_t g_display_cb = NULL;


/* 初始化模块 */
void slot_module_init(void)
{
    for (int i = 0; i < MAX_SLOTS; ++i) {
        slot_id[i] = INVALID_SLOT_ID;
        slot_prev_buf[i][0] = '\0';
    }
    g_display_cb = NULL;
}


/* 设置显示回调 */
void slot_set_display_callback(display_update_cb_t cb)
{
    g_display_cb = cb;
}


/* 查找或分配槽位，返回槽位索引或 -1（无可用槽位） */
static int find_or_alloc_slot(uint16_t id)
{
    int free_index = -1;
		//先查找已有槽位
    for (int i = 0; i < MAX_SLOTS; ++i) {   

        if (slot_id[i] == id) return i;
        if (slot_id[i] == INVALID_SLOT_ID && free_index < 0) free_index = i;
    }

    //分配空闲槽位
    if (free_index >= 0) {
        slot_id[free_index] = id;
        slot_prev_buf[free_index][0] = '\0';
        return free_index;
    }

    return -1;
}


/**
 * 按定长缓冲比较并生成差异输出
 * 参数：
 *  id  - 槽位编号
 *  in  - 输入缓冲，长度为 BUF_LEN（调用者保证）
 *  out - 输出缓冲，长度为 BUF_LEN（调用者分配）
 * 返回：
 *  0 成功，-1 无可用槽位
 *
 * 规则：
 *  - 若 in[i] 与历史相同，则 out[i] = ' '
 *  - 否则 out[i] = in[i]，并更新历史缓存
 *  - 历史缓存以 '\0' 结尾，若输入中某位置为 '\0'，也会被保存
 */

/* slot_prev_buf 已声明为: static char slot_prev_buf[MAX_SLOTS][BUF_LEN+1]; */

int slot_diff_from_buf(uint16_t id, const char in[BUF_LEN], char out[])
{
    if (out == NULL || in == NULL) return -1;

    int slot = find_or_alloc_slot(id);
    if (slot < 0) return -1;

    /* 如果 in 与 out 地址相同或 in 指向 slot_prev_buf[slot]，先拷贝到临时缓冲 */
    char tmp[BUF_LEN];
    const char *src = in;
    if ((const void*)in == (const void*)out || (const void*)in == (const void*)slot_prev_buf[slot]) {
        memcpy(tmp, in, BUF_LEN);
        src = tmp;
    }

    /* 为了安全，避免在循环中回调导致缓冲被修改，先记录变化位置（索引和字符） */
    int changes = 0;
    int changed_idx[BUF_LEN];
    char changed_ch[BUF_LEN];

    for (int i = 0; i < BUF_LEN; ++i) {
        char newb = src[i];
        char oldb = slot_prev_buf[slot][i]; /* 若历史短，可能为 '\0' */

        if (newb == oldb) {
            out[i] = ' ';
        } else {
            out[i] = newb;
            slot_prev_buf[slot][i] = newb;
            changed_idx[changes] = i;
            changed_ch[changes] = newb;
            ++changes;
        }
    }

    /* 确保 out 与历史以 '\0' 结尾 */
    out[BUF_LEN] = '\0';
    slot_prev_buf[slot][BUF_LEN] = '\0';

    /* 统一触发回调（如果需要且回调存在）——在主上下文调用，避免在中断中直接绘制 */
    if (g_display_cb && changes > 0) {
        for (int k = 0; k < changes; ++k) {
            g_display_cb(id, changed_idx[k], changed_ch[k]);
        }
    }

    return changes; /* 返回变化数，0 表示无变化 */
}


/**
 * 以 C 字符串输入（自动按 strlen 处理并清尾）
 * 参数：
 *  id  - 槽位编号
 *  s   - 以 '\0' 结尾的字符串（长度 <= BUF_LEN）
 *  out - 输出缓冲，长度为 BUF_LEN（调用者分配）
 * 返回：
 *  0 成功，-1 无可用槽位
 *
 * 处理逻辑：
 *  - 对 0..new_len-1 按字符比较并更新
 *  - 对 new_len..BUF_LEN-1 若历史非 '\0' 则视为被清空（输出空格并把历史置 '\0'）
 */
int slot_diff_from_cstr(uint16_t id, const char *s, char out[BUF_LEN])
{
    int slot = find_or_alloc_slot(id);
    if (slot < 0) return -1;

    /* 规范化输入长度 */
    size_t new_len = 0;
    if (s) new_len = strlen(s);
    if (new_len > BUF_LEN) new_len = BUF_LEN;

    /* 比较并更新前 new_len 字节 */
    for (size_t i = 0; i < new_len; ++i) {
        char newb = s[i];
        char oldb = slot_prev_buf[slot][i];
        if (newb == oldb) {
            out[i] = ' ';
        } else {
            out[i] = newb;
            slot_prev_buf[slot][i] = newb;
            if (g_display_cb) g_display_cb(id, (int)i, newb);
        }
    }

    /* 清理历史尾部（若历史比新输入长） */
    size_t old_len = strlen(slot_prev_buf[slot]);
    if (old_len > new_len) {
        for (size_t i = new_len; i < old_len && i < BUF_LEN; ++i) {
            /* 这里输出空格表示清除显示位置 */
            out[i] = ' ';
            slot_prev_buf[slot][i] = '\0';
            if (g_display_cb) g_display_cb(id, (int)i, ' ');
        }
        /* 对剩余未触及的位置也填空格输出，保持 out 的一致性 */
        for (size_t i = old_len; i < BUF_LEN; ++i) out[i] = ' ';
    } else {
        /* 若 old_len <= new_len，则对 new_len..BUF_LEN-1 填空格输出 */
        for (size_t i = new_len; i < BUF_LEN; ++i) out[i] = ' ';
    }

    /* 保证历史以 '\0' 结尾 */
    slot_prev_buf[slot][BUF_LEN] = '\0';
    return 0;
}
void slot_clear_id(uint16_t id)
{
    for (int i = 0; i < MAX_SLOTS; ++i) {
        if (slot_id[i] == id) {
            slot_id[i] = INVALID_SLOT_ID;
            memset(slot_prev_buf[i], 0, sizeof(slot_prev_buf[i])); /* 清空整行，包括终止符 */
            break;
        }
    }
}

void slot_clear_all(void)
{
    for (int i = 0; i < MAX_SLOTS; ++i) {
        slot_id[i] = INVALID_SLOT_ID;
        memset(slot_prev_buf[i], 0, sizeof(slot_prev_buf[i])); /* 关键：清空整行 */
    }
}









