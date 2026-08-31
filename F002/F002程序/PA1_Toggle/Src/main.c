/**
  ******************************************************************************
  * @file    main.c
  * @author  Bowen (wbw20)
  * @date    2026-08-18
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   PA1 高低电平切换：高 500ms / 低 500ms，周期 1s
  *
  * 程序流程：
  *   1. HAL_Init()：复位外设、配置 SysTick 为 1ms 中断；
  *   2. APP_SystemClockConfig()：HSI 24MHz 作为系统时钟；
  *   3. BSP_PA1_Init()：PA1 配置为推挽输出，默认低；
  *   4. 主循环：每 500ms 翻转一次 PA1，形成 1s 周期方波。
  *
  * 架构说明（前后台）：
  *   - 前台（中断）：SysTick 只维护毫秒时钟 sys，不干别的；
  *   - 后台（主循环）：非实时刷新类任务，此处是周期翻转判断。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-18) 首次创建：实现 PA1 每秒翻转，周期 1s。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "main.h"
#include "global.h"
#include "bsp_pa1.h"

/* 私有函数声明 ------------------------------------------------------*/
static void APP_SystemClockConfig(void);

/**
  * @brief  主函数
  * @retval int
  */
int main(void)
{
  uint32_t last = 0U;   /* 上一次翻转的时间戳（ms），初值 0 = 上电时刻 */

  /* 复位所有外设、初始化 Flash 接口和 SysTick（1ms） */
  HAL_Init();

  /* 配置系统时钟：HSI 24MHz，AHB/APB 不分频 */
  APP_SystemClockConfig();

  /* 初始化 PA1：推挽输出，默认低电平 */
  BSP_PA1_Init();

  /* 主循环：只做周期翻转，实时性由 SysTick 时钟保证 */
  while (1)
  {
    /*
      用无符号减法 (sys - last) 判断经过的时间：
      即使 sys 溢出回绕也能正确计算，不需要额外防溢出代码。
      达到 500ms 就翻转一次：高 500ms + 低 500ms = 周期 1s。
    */
    if ((sys - last) >= TOGGLE_HALF_PERIOD_MS)
    {
      last = sys;                 /* 记录本次翻转时刻 */
      BSP_PA1_Toggle();           /* PA1 电平翻转      */
    }
  }
}

/**
  * @brief  系统时钟配置：HSI 24MHz，SYSCLK = HSISYS，AHB/APB1 不分频
  * @param  无
  * @retval 无
  */
static void APP_SystemClockConfig(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* 振荡器配置 */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                            /* 打开内部高速时钟 HSI */
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                            /* HSI 不分频            */
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;   /* HSI 校准到 24MHz      */
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS_DISABLE;                /* 本工程不用外部晶振    */
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                           /* 本工程不用 LSI        */
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                           /* 本工程不用 LSE        */

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* 时钟源配置 */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSISYS;   /* 选择 HSISYS 作为系统时钟 */
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;          /* AHB 不分频               */
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;           /* APB1 不分频              */

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

/**
  * @brief  错误处理函数：初始化失败时停在这里，便于调试
  * @param  无
  * @retval 无
  */
void APP_ErrorHandler(void)
{
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  断言失败处理：开启 USE_FULL_ASSERT 后，参数错误会进入这里
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file;
  (void)line;
  /* 可在此加串口打印或点亮错误灯，方便定位 */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
