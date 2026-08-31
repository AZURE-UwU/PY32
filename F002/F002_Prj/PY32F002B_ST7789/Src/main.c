/**
  ******************************************************************************
  * @file    main.c
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.1
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   ST7789 控制板演示程序（移植自 STM32G030F6P6 工程）
  *
  * 功能概述：
  *   - SPI1 驱动 ST7789 显示电压/电流/功率/温度/风扇占空比/瓦时；
  *   - I2C1 读取 INA226（母线电压、电流、功率）；
  *   - ADC 两通道读取 VCC 分压与 NTC 分压，NTC 温度经卡尔曼滤波；
  *   - TIM1_CH1 风扇 PWM（按温度滞回控制）、TIM1_CH2 背光 PWM；
  *   - 三个按键：短按/长按/双击状态机；SW_WKUP 长按进入 STOP，再按唤醒。
  *
  * 前后台架构：
  *   - 前台（中断）：SysTick 维护 ms 时钟，EXTI 只记按键时间戳；
  *   - 后台（主循环）：50ms 任务（按键/ADC/温度/风扇）与 1s 任务（LED/待机），
  *     其余时间做显示刷新。PY32F002B 无 DMA、HAL 未实现 TIM14，
  *     因此原工程的 DMA 采样和定时器周期任务改为本方案。
  *
 * CHANGELOG:
 *   V1.0 (2026-08-19) 首次创建：移植自 G030F6P6 HAL Finished 工程。
 *   V1.1 (2026-08-24) 修复按键无业务逻辑（PY32 HAL 仅提供 HAL_GPIO_EXTI_Callback）；
 *                     按键 3 的 PAGE/MODE 状态增加屏幕指示，三个按键均有可见效果。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "main.h"
#include "py32f002b_it.h"
#include "global.h"
#include "function.h"
#include "st7789.h"
#include "INA226.h"
#include "Font_asc.h"
#include "bsp_gpio.h"
#include "bsp_spi.h"
#include "bsp_i2c.h"
#include "bsp_adc.h"
#include "bsp_tim.h"

#include <math.h>

/* 应用全局变量 ------------------------------------------------------*/
char VCC[32];
char NTC[32];
char TEMP[32];

/* INA226 读数 */
float v1  = 0.0f;
float i1  = 0.0f;
float p1  = 0.0f;
float wh1 = 0.0f;

char V1[32];
char I1[32];
char P1[32];
float RESISTOR = 0.01f;      /* 采样电阻 10mΩ，改这里不动逻辑 */

/* ADC/温度 */
volatile float extTemp = 0.0f;
volatile float extVin  = 0.0f;

/* 风扇/背光/页面状态 */
volatile uint8_t  pwm  = 0;
volatile uint8_t  FLAG = 1;      /* 置 1 触发整屏重绘 */
volatile uint8_t  PAGE = 0;
volatile uint8_t  MODE = 0;
volatile uint8_t  FLIP = 1;      /* 屏幕方向 */
volatile uint16_t BG   = BLACK;  /* 背景色 */
volatile uint8_t  blk  = 100;    /* 背光亮度 0~100 */

char temp[32];

KalmanFilter KalmanTemp;

/* 私有函数 ----------------------------------------------------------*/
static void SystemClock_Config(void);

/**
  * @brief  主函数
  * @retval int
  */
int main(void)
{
  uint64_t last50 = 0;   /* 上次 50ms 任务时刻 */
  uint64_t last1s = 0;   /* 上次 1s 任务时刻   */
  uint16_t adcV = 0;     /* ADC IN0 原始值     */
  uint16_t adcN = 0;     /* ADC IN2 原始值     */
  static int cnt = 0;    /* 低电流待机倒计时   */

  /* 复位外设、配置 SysTick 为 1ms */
  HAL_Init();

  /* 系统时钟：HSI 24MHz */
  SystemClock_Config();

  /* 外设初始化顺序：GPIO 先，避免复用引脚电平冲突 */
  BSP_GPIO_Init();
  BSP_SPI1_Init();
  BSP_I2C1_Init();
  BSP_ADC1_Init();
  BSP_TIM1_PWM_Init();

  /* 显示屏启动 */
  LCD_Init(BG);

  /* 开机提示 */
  LCD_ShowString_24_12(100, 65, RED, BG, "Powered by");
  LCD_ShowString_24_12(100, 90, RED, BG, "RedStoneS&T");

  /* 两路 PWM 启动：风扇、背光 */
  BSP_PWM_Start();

  /* INA226：地址 0x40，16 次平均，母线/分流转换时间 4，连续 Shunt+Bus */
  INA226_Init(INA226_DEV_ADDR, 2, 4, 4, 7);

  /* 显示差分刷新槽位 */
  slot_module_init();

  /* 卡尔曼滤波：用 NTC 初始电压换算温度作为初值 */
  KalmanInit(&KalmanTemp, 25.0f, 1.0f);

  /* 三个进度条：电压 / 电流 / 功率 */
  ProgressBar votageBar;
  ProgressBar currentBar;
  ProgressBar powerBar;
  ProgressBar_Init(&votageBar,  10,  55, 200, 5, 0.0f,  30.0f, GREEN_UI);
  ProgressBar_Init(&currentBar, 10, 105, 190, 5, 0.0f,   5.0f, RED_UI);
  ProgressBar_Init(&powerBar,   10, 153, 180, 5, 0.0f, 150.0f, BLUE_UI);

  /* 电流分段线性校准表 */
  #define NUM_CAL_POINTS 6
  static const CalTable ct0 = {
      .id = 0,
      .num_points = NUM_CAL_POINTS,
      .points = {0.0f, 0.1f, 0.5f, 1.0f, 5.0f, 10.0f},
      .offsets = {0.0003f, 0.003f, 0.012f, 0.02f, 0.2f, 0.1f}
  };

  HAL_Delay(100);
  LCD_Clear(BG);

  /* 主循环 */
  while (1)
  {
    /* ---------------- 50ms 周期任务 ---------------- */
    if ((sys - last50) >= APP_TASK_50MS_MS)
    {
      last50 = sys;

      /* 按键消抖 + 状态机 */
      Debounce_Process();
      uint8_t bt1 = btnLogic(0);                 /* SW_WKUP */
      uint8_t bt2 = btnLogic_withExtLongFlag(1); /* SW_FUNC */
      uint8_t bt3 = btnLogic(2);                 /* SW_MODE */

      /* 按键 1（唤醒键）：短按翻转屏幕 / 长按关机 / 双击换背景 */
      if (bt1 == BTN_SHORT)
      {
        FLIP = (FLIP == 3) ? 1 : 3;
        LCD_SpinScreen(FLIP);
        lock[0] = 0;
        FLAG = 1;
      }
      else if (bt1 == BTN_LONG)
      {
        power_off();          /* 进 STOP，SW_WKUP 再按唤醒 */
        lock[0] = 0;
        FLAG = 1;             /* 唤醒后整屏重绘 */
      }
      else if (bt1 == BTN_DOUBLE)
      {
        BG = (BG == BLACK) ? WHITE : BLACK;
        lock[0] = 0;
        FLAG = 1;
      }

      /* 按键 2（功能键）：短按预留（原 G030 为继电器脉冲，PY32 板未引出） */
      if (bt2 == BTN_SHORT)
      {
        BSP_LED_Toggle();     /* 暂用 LED 反馈按键生效 */
        lock[1] = 0;
      }
      else if (bt2 == BTN_LONG)
      {
        /* 长按调节背光：方向到顶/到底自动反转，可持续长按 */
        static int8_t dir = -1;
        blk = (uint8_t)((int16_t)blk + dir * 2);
        if (blk >= 100) { blk = 100; dir = -1; }
        else if (blk <= 0) { blk = 0; dir = 1; }
        LCD_BLK(blk);
        if ((blk == 100) || (blk == 0)) { longFlag[1] = 1; }
        lock[1] = 0;
      }

      /* 按键 3（模式键）：短按翻页 / 长按切换模式 */
      if (bt3 == BTN_SHORT)
      {
        PAGE = (uint8_t)((PAGE + 1U) & 1U);
        lock[2] = 0;
        FLAG = 1;
      }
      else if (bt3 == BTN_LONG)
      {
        MODE = (uint8_t)((MODE + 1U) & 1U);
        lock[2] = 0;
        FLAG = 1;
      }

      /* ADC 两通道读取（无 DMA，扫描轮询） */
      BSP_ADC_ReadAll(&adcV, &adcN);
      extVin = (float)adcV * 3.3f / 4096.0f;
      extTemp = KalmanUpdate(&KalmanTemp,
                             Voltage_To_Temperature((float)adcN * 3.3f / 4096.0f),
                             0.001f, 4.0f);

      /* 风扇：温度滞回控制 PWM */
      pwm = CalculatePWM(extTemp);
      BSP_PWM_SetFan(pwm);

      /* 瓦时积分 */
      wh1 = calc_wh(sys, p1);
    }

    /* ---------------- 1s 周期任务 ---------------- */
    if ((sys - last1s) >= APP_TASK_1S_MS)
    {
      last1s = sys;
      BSP_LED_Toggle();

      /* 电流小于 15mA 视为负载断开，连续 15 分钟自动关机 */
      if (i1 < 0.015f)
      {
        cnt += 1;
        if (cnt > (15 * 60))
        {
          cnt = 0;
          power_off();
          FLAG = 1;
        }
      }
      else
      {
        cnt = 0; /* 恢复负载后清零 */
      }
    }

    /* ---------------- 显示刷新 ---------------- */
    if (FLAG)
    {
      FLAG = 0;
      LCD_Clear(BG);
      LCD_BLK(blk);
      LCD_SpinScreen(FLIP);

      /* 静态字符只画一次 */
      LCD_ShowString(h30w27_sample, h30w27, 30, 27,  10, 20, GREEN_UI, BG, "U");
      LCD_ShowString(h30w27_sample, h30w27, 30, 27, 190, 20, GREEN_UI, BG, "V");
      LCD_ShowString(h26w23_sample, h26w23, 26, 23,  18, 75, RED_UI,   BG, "I");
      LCD_ShowString(h26w23_sample, h26w23, 26, 23, 175, 75, RED_UI,   BG, "A");
      LCD_ShowString(h22w20_sample, h22w20, 22, 20,  18, 125, BLUE_UI, BG, "P");
      LCD_ShowString(h22w20_sample, h22w20, 22, 20, 160, 125, BLUE_UI, BG, "W");
      LCD_ShowString(h18w16_sample, h18w16, 18, 16, 275,  41, YELLOW_UI, BG, "%");
      LCD_DrawDashedCircle(265, 50, 35, GRAY, 5, 20);
      LCD_ShowString_16_8(253, 26, YELLOW_UI, BG, "FAN");
      LCD_ShowString(h18w16_sample, h18w16, 18, 16, 290,  95, ORANGE, BG, "C");
      LCD_ShowString(h18w16_sample, h18w16, 18, 16, 280, 130, SKYBLUE, BG, "wh");

//      /* 页面/模式指示：让按键 3（短按翻页 / 长按切模式）的动作可见 */
//      {
//        char modeBuf[6];
//        modeBuf[0] = 'M';
//        modeBuf[1] = (char)('0' + MODE);
//        modeBuf[2] = ' ';
//        modeBuf[3] = 'P';
//        modeBuf[4] = (char)('0' + PAGE);
//        modeBuf[5] = '\0';
//        LCD_ShowString_16_8(0, 0, YELLOW_UI, BG, modeBuf);
//      }

      /* 槽位缓存与数值缓冲复位，保证差异刷新从空状态开始 */
      slot_clear_all();
      memset(V1, ' ', BUF_LEN); V1[BUF_LEN] = '\0';
      memset(I1, ' ', BUF_LEN); I1[BUF_LEN] = '\0';
      memset(P1, ' ', BUF_LEN); P1[BUF_LEN] = '\0';

      ProgressBar_Redraw(&votageBar, v1, BG);
      ProgressBar_Redraw(&currentBar, i1, BG);
      ProgressBar_Redraw(&powerBar, p1, BG);
    }

    /* 读 INA226 并换算（失败时 GetBusVoltage 返回哨兵值） */
    v1 = INA226_GetBusVoltage(INA226_DEV_ADDR);
    if (v1 >= 0.0f)
    {
      float iCal = CalibrateCurrent(&ct0, INA226_GetCurrent(INA226_DEV_ADDR));
      i1 = (iCal < 0.0f) ? -iCal : iCal;   /* 手动取绝对值，省掉库函数 */
      p1 = v1 * i1;
    }

    ProgressBar_Update(&votageBar, v1, BG);
    ProgressBar_Update(&currentBar, i1, BG);
    ProgressBar_Update(&powerBar, p1, BG);

    formatFloatToStr(v1, V1, 5, 3);
    formatFloatToStr(i1, I1, 5, 3);
    formatFloatToStr(p1, P1, 5, 3);

    /* 差分刷新：没变化的位输出空格，减少 SPI 传输量 */
    slot_diff_from_buf(1, V1, V1);
    slot_diff_from_buf(2, I1, I1);
    slot_diff_from_buf(3, P1, P1);

    LCD_ShowString(h30w27_sample, h30w27, 30, 27, 45,  20, GREEN_UI, BG, V1);
    LCD_ShowString(h26w23_sample, h26w23, 26, 23, 45,  75, RED_UI,   BG, I1);
    LCD_ShowString(h22w20_sample, h22w20, 22, 20, 45, 125, BLUE_UI,  BG, P1);

    formatFloatToStr((float)pwm, temp, 2, 0);
    LCD_ShowString(h18w16_sample, h18w16, 18, 16, 237,  41, YELLOW_UI, BG, temp);
    formatFloatToStr(extTemp, temp, 4, 1);
    LCD_ShowString(h18w16_sample, h18w16, 18, 16, 220,  95, ORANGE,    BG, temp);
    formatFloatToStr(wh1, temp, 5, 3);
    LCD_ShowString(h18w16_sample, h18w16, 18, 16, 195, 130, SKYBLUE,   BG, temp);
  }
}

/**
  * @brief  系统时钟配置：HSI 24MHz，SYSCLK=HSISYS，AHB/APB 不分频
  */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS_DISABLE;
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSISYS;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

/**
  * @brief  错误处理：初始化失败时停在这里，便于调试定位
  */
void APP_ErrorHandler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
