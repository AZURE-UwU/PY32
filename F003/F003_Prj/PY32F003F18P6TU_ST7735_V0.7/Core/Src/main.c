/**
  ******************************************************************************
  * @file    main.c
  * @author  Bowen (wbw20)
  * @date    2026-09-02
  * @version V1.10
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
  *   V1.7 (2026-09-06) 宏名/变量名优化：APP_TASK_*_MS 去冗余后缀，
  *                     RESISTOR -> g_shunt_resistor_ohm。
  *   V1.8 (2026-09-06) 主题改为"编号 + main 集中 switch"：
  *                     每套主题在 ThemeX_Frame/ThemeX_Values 里写死绘制，
  *                     主题只作用于主界面，开机/关机画面固定不动。
  *   V1.9 (2026-09-07) 主题渲染拆分到独立 theme 模块；主界面测量/显示缓冲
  *                     上收 global.c；main 只负责采集调度并调用 Theme_Render*。
  *   V1.10 (2026-09-07) 采集与呈现彻底分离：测量数据改为 g_v/g_i/g_p 多路数组
  *                     及 g_temp/g_wh/g_pwm；格式化/差分/显示全部移入 theme.c。
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
#include "theme.h"
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

/* INA226 采样电阻（测量数据 g_v/g_i/g_p/g_temp/g_wh/g_pwm 已上收 global.c） */
float g_shunt_resistor_ohm = 0.01f;   /* 采样电阻 10mΩ，改这里不动逻辑 */

/* ADC 电压（VCC 分压原始换算值，当前未参与显示/控制） */
volatile float extVin  = 0.0f;

/* 页面状态 */
volatile uint8_t  PAGE = 0;
volatile uint8_t  MODE = 0;

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

  /* 从内部 Flash 恢复用户配置：必须在 LCD 初始化之前（方向依赖 g_cfg） */
  APP_UI_SettingsLoad();

  /* 显示屏启动 */
  LCD_Init(BLACK);

  /* 开机提示 */
  LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20, 16, RED, BLACK, "Powered by");
  LCD_ShowString(h24w12_sample, h24w12, 24, 12, 20, 44, RED, BLACK, "RedStoneS&T");

  /* 启动风扇与背光 PWM */
  BSP_PWM_Start();

  /* INA226：地址 0x40，16 次平均，母线/分流转换时间 4，连续 Shunt+Bus */
  INA226_Init(INA226_DEV_ADDR, 2, 4, 4, 7);

  /* 显示差分刷新槽位 */
  slot_module_init();

  /* 卡尔曼滤波：用 NTC 初始电压换算温度作为初值 */
  KalmanInit(&KalmanTemp, 25.0f, 1.0f);

  /* 电流分段线性校准表 */
  #define NUM_CAL_POINTS 6
  static const CalTable ct0 = {
      .id = 0,
      .num_points = NUM_CAL_POINTS,
      .points = {0.0f, 0.1f, 0.5f, 1.0f, 5.0f, 10.0f},
      .offsets = {0.0003f, 0.003f, 0.012f, 0.02f, 0.2f, 0.1f}
  };

  HAL_Delay(100);
  LCD_Clear(BLACK);

  /* 主循环 */
  while (1)
  {
    /* ---------------- 50ms 周期任务 ---------------- */
    if ((sys - last50) >= APP_TASK_50MS)
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
      g_temp = KalmanUpdate(&KalmanTemp,
                            Voltage_To_Temperature((float)adcN * 3.3f / 4096.0f),
                            0.001f, 4.0f);

      /* 风扇：温度滞回控制 PWM */
      g_pwm = CalculatePWM(g_temp);
      BSP_PWM_SetFan(g_pwm);

      /* 瓦时积分 */
      g_wh = calc_wh(sys, g_p[0]);
    }

    /* ---------------- 1s 周期任务 ---------------- */
    if ((sys - last1s) >= APP_TASK_1S)
    {
      last1s = sys;
      /* 负载断开（电流小于阈值）持续一段时间后自动关机；
         使能开关与阈值/时长都来自用户配置 g_cfg */
      if (g_cfg.auto_off && (g_i[0] < g_cfg.auto_off_i))
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
      LCD_BLK(g_cfg.blk);
      LCD_SpinScreen(g_cfg.flip);

      Theme_RenderFrame();   /* 主题相关整帧（清屏/标签/进度条） */
    }

    /* 读 INA226 并换算（失败时 GetBusVoltage 返回哨兵值）。
       母线电压 × 分压微调系数：g_cfg.vbus_div 默认 1.0 表示直通 */
    g_v[0] = INA226_GetBusVoltage(INA226_DEV_ADDR) * g_cfg.vbus_div;
    if (g_v[0] >= 0.0f)
    {
      float iCal = CalibrateCurrent(&ct0, INA226_GetCurrent(INA226_DEV_ADDR));
      g_i[0] = (iCal < 0.0f) ? -iCal : iCal;   /* 手动取绝对值，省掉库函数 */
      g_p[0] = g_v[0] * g_i[0];
    }

    /* 数值刷新只属于主界面；菜单/数值编辑期间继续采样但不绘制 */
    if (UI_STATE == UI_MAIN)
    {
      Theme_RenderValues();   /* 格式化/差分/绘制全在主题模块 */
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

