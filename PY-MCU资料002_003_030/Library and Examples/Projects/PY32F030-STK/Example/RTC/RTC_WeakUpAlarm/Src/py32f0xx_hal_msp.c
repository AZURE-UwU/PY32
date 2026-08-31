/**
  ******************************************************************************
  * @file    py32f0xx_hal_msp.c
  * @author  MCU Application Team
  * @Version V1.0.0
  * @Date    2020-10-19
  * @brief   This file provides code for the MSP Initialization
  *          and de-Initialization codes.
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "main.h"


/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);
  BSP_LED_Init(LED_GREEN);
}

/********************************************************************************************************
**函数信息 ：void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
**功能描述 ：初始化RTC可用时钟
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;


  /*##-2- Configue LSE/LSI as RTC clock soucre ###############################*/
#ifdef RTC_CLOCK_SOURCE_LSE
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
//    Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
//    Error_Handler();
  }
#elif defined (RTC_CLOCK_SOURCE_LSI)
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
//   Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
//   Error_Handler();
  }
#else
#error Please select the RTC Clock source inside the main.h file
#endif /*RTC_CLOCK_SOURCE_LSE*/

  /*##-2- Enable RTC peripheral Clocks #######################################*/
  __HAL_RCC_RTCAPB_CLK_ENABLE();
  /* Enable RTC Clock */
  __HAL_RCC_RTC_ENABLE();

  /*使能RTC中断*/
  HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
  NVIC_EnableIRQ(RTC_IRQn);
  /*使能RTC闹钟中断*/
//  __HAL_RTC_OVERFLOW_DISABLE_IT(hrtc, RTC_IT_OW);
  __HAL_RTC_ALARM_ENABLE_IT(hrtc, RTC_IT_ALRA);
//  __HAL_RTC_SECOND_DISABLE_IT(hrtc, RTC_IT_SEC);
}

/********************************************************************************************************
**函数信息 ：void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
**功能描述 ：关闭RTC时钟
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
  /*RTC时钟失能*/
  __HAL_RCC_RTC_DISABLE();
}




