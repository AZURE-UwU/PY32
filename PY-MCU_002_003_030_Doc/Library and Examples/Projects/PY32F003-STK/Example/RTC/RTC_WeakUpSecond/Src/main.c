/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
RTC_HandleTypeDef RtcHandle;
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void RTC_TimeShow(void);



/********************************************************************************************************
**函数信息 ：int main(void)
**功能描述 ：main函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  /* 复位所有外设，初始化flash接口和systick.*/
  HAL_Init();

  SystemClock_Config();                             //系统时钟配置

  DEBUG_USART_Config();                             //UART初始化

  RtcHandle.Instance = RTC;                         //选择RTC
  RtcHandle.Init.AsynchPrediv = RTC_AUTO_1_SECOND;  //RTC一秒时基自动计算
  if (HAL_RTC_Init(&RtcHandle) != HAL_OK)           //RTC初始化
  {
  }

//    RTC_AlarmConfig();                                 //RTC时钟配置
  HAL_SuspendTick();                                 //关闭systick中断，否则会中断唤醒
  //进入STOP模式
  printf("enter stop\r\n");
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  printf("exti stop\r\n");
  while (1)
  {
  }
}
/********************************************************************************************************
**函数信息 ：void SystemClock_Config(void)
**功能描述 ：系统时钟配置
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};

  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
  oscinitstruct.HSIState        = RCC_HSI_ON;                                                //开启HSI
//    oscinitstruct.HSICalibrationValue = RCC_HSICALIBRATION_4MHz;                             //配置HSI输出时钟为4MHz
  oscinitstruct.HSICalibrationValue = RCC_HSICALIBRATION_8MHz;                              //配置HSI输出时钟为8MHz
//    oscinitstruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz;                            //配置HSI输出时钟为16MHz
//    oscinitstruct.HSICalibrationValue = RCC_HSICALIBRATION_22p12MHz;                        //配置HSI输出时钟为22.12MHz
//    oscinitstruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;                            //配置HSI输出时钟为24MHz
  oscinitstruct.HSEState        = RCC_HSE_OFF;                                              //关闭HSE

  oscinitstruct.LSIState        = RCC_LSI_ON;                                                //关闭LSE


  if (HAL_RCC_OscConfig(&oscinitstruct) != HAL_OK)                                           //RCC振荡器初始化
  {
    while (1);
  }

  //初始化CPU,AHB,APB总线时钟
  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1); //RCC系统时钟类型
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;                                           //SYSCLK的源选择为HSI
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                               //APH时钟不分频
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV1;                                                //APB时钟不分频
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_1) != HAL_OK)                          //初始化RCC系统时钟
  {
    while (1);
  }
}
/********************************************************************************************************
**函数信息 void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
**功能描述 ：RTC事件执行函数，通过串口输出字符串
**输入参数 ：RTC_HandleTypeDef *hrtc
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_RTCEx_RTCEventErrorCallback(RTC_HandleTypeDef *hrtc)
{
  printf("RTC_IT_OW\r\n");
}

/********************************************************************************************************
**函数信息 void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
**功能描述 ：RTC事件执行函数，通过串口答应当前时间
**输入参数 ：RTC_HandleTypeDef *hrtc
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_RTCEx_RTCEventCallback(RTC_HandleTypeDef *hrtc)
{
  printf("RTC_IT_SEC\r\n");
  RTC_TimeShow();
}


/********************************************************************************************************
**函数信息 ：void RTC_TimeShow(void)
**功能描述 ：显示RTC当前时间
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
static void RTC_TimeShow(void)
{
  RTC_DateTypeDef sdatestructureget;
  RTC_TimeTypeDef stimestructureget;

  /*设置RTC当前时间*/
  HAL_RTC_GetTime(&RtcHandle, &stimestructureget, RTC_FORMAT_BIN);
  /* 设置RTC当前日期 */
  HAL_RTC_GetDate(&RtcHandle, &sdatestructureget, RTC_FORMAT_BIN);
  /* 显示时间格式为 : hh:mm:ss */
  printf("%02d:%02d:%02d\r\n", stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */


/* Private function -------------------------------------------------------*/


