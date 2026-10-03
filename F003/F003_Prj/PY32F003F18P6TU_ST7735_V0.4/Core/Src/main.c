/**
  ******************************************************************************
  * @file    main.c
  * @author  Bowen (wbw20)
  * @date    2026-09-02
  * @version V1.6
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   ST7735S（0.96 寸 160x80 横屏）控制板演示程序（移植自 PY32F002B）
  *
  * 功能概述：
  *   - SPI1 驱动 ST7735S 显示电压/电流/功率/温度/风扇占空比/瓦时；
  *   - I2C1 读取 INA226（母线电压、电流、功率）；
  *   - ADC 两通道读取 VCC 分压与 NTC 分压，NTC 温度经卡尔曼滤波；
  *   - TIM3_CH2 风扇 PWM（按温度滞回控制），背光用显示开关（无 BLK 引脚）；
  *   - 三个按键：短按/长按/双击状态机；SW_WKUP 长按进入 STOP，再按唤醒。
  *
  * 前后台架构：
  *   - 前台（中断）：SysTick 维护 ms 时钟，EXTI 只记按键时间戳；
  *   - 后台（主循环）：50ms 任务（按键/ADC/温度/风扇）与 1s 任务（LED/待机），
  *     其余时间做显示刷新。
  *
 * CHANGELOG:
 *   V1.0 (2026-08-29) 将 PY32F002B_ST7735 程序移植到 PY32F003F18P6TU：
 *                     时钟 HSI 24MHz、风扇 PWM 改 TIM3_CH2(PB5)、
 *                     按键/ADC/SPI/I2C 按 F18P 板重新配引脚，功能不变。
 *   V1.1 (2026-08-29) PF0/PF1/PF4 允许复用后：恢复 LCD CS(PF1)/RST(PF4)，
 *                     背光改 TIM14_CH1(PF0) PWM 无级调光。
  *   V1.2 (2026-08-30) 字符显示统一走通用 LCD_ShowString：16x8/24x12 专用接口
  *                     改为 LCD_ShowString(h16w8_sample, h16w8, 16, 8, ...) /
  *                     LCD_ShowString(h24w12_sample, h24w12, 24, 12, ...)。
  *   V1.3 (2026-08-31) 用户可调配置迁移到 Settings_t g_cfg（global.c）：
  *                     FLIP->g_cfg.flip、BG->g_cfg.theme、blk->g_cfg.blk；
  *                     自动关机阈值/时长参数化；新增 INA226 母线分压微调系数。
  *   V1.4 (2026-08-31) 新增设置菜单（app_ui）：主界面仅保留长按UP关机/短按SET进设置，
  *                     翻屏/主题/背光/自动关机等全部移入二级菜单，显示按 UI_STATE 分流。
  *   V1.5 (2026-09-02) UP/DOWN 改用 btnLogic_withExtLongFlag：值编辑长按连跳时
  *                     每 50ms 持续返回 BTN_LONG，供 app_ui 按时间分档步进。
  *   V1.6 (2026-09-02) 新增掉电存储：开机从内部 Flash 恢复用户配置（app_ui+bsp_flash），
  *                     首启/数据损坏自动写回默认值。
 ******************************************************************************
 */

/* 头文件包含 --------------------------------------------------------*/
#include "main.h"
#include "py32f0xx_it.h"
#include "global.h"
#include "function.h"
#include "st7735.h"
#include "INA226.h"
#include "Font_asc.h"
#include "app_ui.h"
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

/* 风扇/页面运行状态（用户配置已统一移到 global.c 的 g_cfg） */
volatile uint8_t  pwm  = 0;
volatile uint8_t  PAGE = 0;
volatile uint8_t  MODE = 0;

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
  uint16_t adcV = 0;     /* ADC IN5 原始值     */
  uint16_t adcN = 0;     /* ADC IN4 原始值     */
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
  BSP_TIM3_PWM_Init();
  BSP_TIM14_PWM_Init();

  /* 从内部 Flash 恢复用户配置：必须在 LCD 初始化之前，主题/方向依赖 g_cfg */
  APP_UI_SettingsLoad();

  /* 显示屏启动 */
  LCD_Init(g_cfg.theme);

  /* 开机提示（24x12 字库，统一走通用 LCD_ShowString） */
  LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20, 16, RED, g_cfg.theme, "Powered by");
  LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20, 44, RED, g_cfg.theme, "RedStoneS&T");

  /* 启动风扇与背光 PWM */
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
  ProgressBar_Init(&votageBar,  88, 28, 68, 3, 0.0f,  30.0f, GREEN_UI);
  ProgressBar_Init(&currentBar, 88, 48, 68, 3, 0.0f,   5.0f, RED_UI);
  ProgressBar_Init(&powerBar,   88, 68, 68, 3, 0.0f, 150.0f, BLUE_UI);

  /* 电流分段线性校准表 */
  #define NUM_CAL_POINTS 6
  static const CalTable ct0 = {
      .id = 0,
      .num_points = NUM_CAL_POINTS,
      .points = {0.0f, 0.1f, 0.5f, 1.0f, 5.0f, 10.0f},
      .offsets = {0.0003f, 0.003f, 0.012f, 0.02f, 0.2f, 0.1f}
  };

  HAL_Delay(100);
  LCD_Clear(g_cfg.theme);

  /* 主循环 */
  while (1)
  {
    /* ---------------- 50ms 周期任务 ---------------- */
    if ((sys - last50) >= APP_TASK_50MS_MS)
    {
      last50 = sys;

      /* 按键消抖 + 状态机 */
      Debounce_Process();
      uint8_t bt_up   = btnLogic_withExtLongFlag(0);   /* SW_WKUP = UP：长按持续返回LONG，供连跳 */
      uint8_t bt_set  = btnLogic(1);                   /* SW_FUNC = SET */
      uint8_t bt_down = btnLogic_withExtLongFlag(2);   /* SW_MODE = DOWN：同上 */

      /* 三键语义由 UI 状态机决定：主界面 / 设置菜单 / 数值编辑 */
      APP_UI_HandleKeys(bt_up, bt_set, bt_down);
      lock[0] = lock[1] = lock[2] = 0;   /* 消费事件后统一解锁 */

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
//      BSP_LED_Toggle();

      /* 负载断开（电流小于阈值）持续一段时间后自动关机；
         使能开关与阈值/时长都来自用户配置 g_cfg */
      if (g_cfg.auto_off && (i1 < g_cfg.auto_off_i))
      {
        cnt += 1;
        if (cnt > (int)(g_cfg.auto_off_min * 60))
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
    if (FLAG && (UI_STATE != UI_MAIN))
    {
      FLAG = 0;
      APP_UI_Draw();            /* 设置菜单 / 数值编辑 整屏 */
    }
    else if (FLAG)
    {
      FLAG = 0;
      LCD_Clear(g_cfg.theme);
      LCD_BLK(g_cfg.blk);
      LCD_SpinScreen(g_cfg.flip);

      /* 静态字符只画一次（160x80 横屏紧凑布局，16x8 字库） */
      LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 3, GREEN_UI, g_cfg.theme, "U");
      LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 27, RED_UI, g_cfg.theme, "I");
      LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 51, BLUE_UI, g_cfg.theme, "P");
      LCD_ShowString(h16w8_sample, h16w8, 16, 8, 2, 73, ORANGE, g_cfg.theme, "T");
      LCD_ShowString(h16w8_sample, h16w8, 16, 8, 58, 73, YELLOW_UI, g_cfg.theme, "F");
      LCD_ShowString(h16w8_sample, h16w8, 16, 8, 88, 73, SKYBLUE, g_cfg.theme, "wh");

//      /* 页面/模式指示：让按键 3（短按翻页 / 长按切模式）的动作可见 */
//      {
//        char modeBuf[6];
//        modeBuf[0] = 'M';
//        modeBuf[1] = (char)('0' + MODE);
//        modeBuf[2] = ' ';
//        modeBuf[3] = 'P';
//        modeBuf[4] = (char)('0' + PAGE);
//        modeBuf[5] = '\0';
//        LCD_ShowString(h16w8_sample, h16w8, 16, 8, 112, 2, YELLOW_UI, BG, modeBuf);
//      }

      /* 槽位缓存与数值缓冲复位，保证差异刷新从空状态开始 */
      slot_clear_all();
      memset(V1, ' ', BUF_LEN); V1[BUF_LEN] = '\0';
      memset(I1, ' ', BUF_LEN); I1[BUF_LEN] = '\0';
      memset(P1, ' ', BUF_LEN); P1[BUF_LEN] = '\0';

      ProgressBar_Redraw(&votageBar, v1, g_cfg.theme);
      ProgressBar_Redraw(&currentBar, i1, g_cfg.theme);
      ProgressBar_Redraw(&powerBar, p1, g_cfg.theme);
    }

    /* 读 INA226 并换算（失败时 GetBusVoltage 返回哨兵值）。
       母线电压 × 分压微调系数：g_cfg.vbus_div 默认 1.0 表示直通 */
    v1 = INA226_GetBusVoltage(INA226_DEV_ADDR) * g_cfg.vbus_div;
    if (v1 >= 0.0f)
    {
      float iCal = CalibrateCurrent(&ct0, INA226_GetCurrent(INA226_DEV_ADDR));
      i1 = (iCal < 0.0f) ? -iCal : iCal;   /* 手动取绝对值，省掉库函数 */
      p1 = v1 * i1;
    }

    /* 数值刷新只属于主界面；菜单/数值编辑期间继续读 INA（自动关机 i1 实时）但不画屏 */
    if (UI_STATE == UI_MAIN)
    {
      ProgressBar_Update(&votageBar, v1, g_cfg.theme);
      ProgressBar_Update(&currentBar, i1, g_cfg.theme);
      ProgressBar_Update(&powerBar, p1, g_cfg.theme);

      formatFloatToStr(v1, V1, 5, 3);
      formatFloatToStr(i1, I1, 5, 3);
      formatFloatToStr(p1, P1, 5, 3);

      /* 差分刷新：没变化的位输出空格，减少 SPI 传输量 */
      slot_diff_from_buf(1, V1, V1);
      slot_diff_from_buf(2, I1, I1);
      slot_diff_from_buf(3, P1, P1);

      LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20,  0, GREEN_UI, g_cfg.theme, V1);
      LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20, 24, RED_UI,   g_cfg.theme, I1);
      LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20, 48, BLUE_UI,  g_cfg.theme, P1);

      /* 外部温度 */
      formatFloatToStr(extTemp, temp, 4, 1);
      LCD_ShowString(h16w8_sample, h16w8, 16, 8, 100, 0, ORANGE, g_cfg.theme, temp);
      /* 风扇PWM */
      formatFloatToStr((float)pwm, temp, 2, 0);
      LCD_ShowString(h16w8_sample, h16w8, 16, 8, 100, 24, YELLOW_UI, g_cfg.theme, temp);
      /* 瓦时积分 */
      formatFloatToStr(wh1, temp, 5, 3);
      LCD_ShowString(h16w8_sample, h16w8, 16, 8, 100, 48, SKYBLUE, g_cfg.theme, temp);
    }
  }
}

/**
  * @brief  系统时钟配置：HSI 24MHz，SYSCLK=HSI，AHB/APB 不分频
  */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* PY32F003 无 LSE：只配置 HSE/HSI/LSI */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;   /* 板上有 24MHz 晶振，但本工程用 HSI */
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;   /* PY32F003 的 HSI 时钟源宏 */
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

