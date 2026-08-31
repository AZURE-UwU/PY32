/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @Version V1.0.0
  * @Date    2020-10-19
  * @brief   main function
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef RTCinit;
RTC_TimeTypeDef RTCtime;
uint8_t gsecond, gminute, ghour;
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);
void APP_RTC_Init(void);
void SystemClock_Config(void);
void APP_RTC_SetAlarm_IT(uint8_t Sec, uint8_t Min, uint8_t Hour);
/********************************************************************************************************
**函数信息 ：int main(void)
**功能描述 ：main函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  HAL_Init();                                                            //初始化systick
  SystemClock_Config();                                                  //时钟使能LSE
  DEBUG_USART_Config();
  APP_RTC_Init();                                                        //RTC初始化
  BSP_LED_On(LED_GREEN);                                                 //LED开启
  /*等待按键按下*/
  while (BSP_PB_GetState(BUTTON_USER) != 0)
  {
  }
  BSP_LED_Off(LED_GREEN);                                                //关闭LED
  HAL_SuspendTick();
  while (1)
  {
    HAL_RTC_WaitForSynchro(&RTCinit);
    HAL_RTC_GetTime(&RTCinit, &RTCtime, RTC_FORMAT_BIN);                 //获取RTC当前时间,格式为BIN
    ghour = RTCtime.Hours;
    gminute = RTCtime.Minutes;
    gsecond = RTCtime.Seconds;
    APP_RTC_SetAlarm_IT(gsecond, gminute, ghour);                           //RTC闹钟中断
    //进入STOP模式
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  }
}

/********************************************************************************************************
**函数信息 ：void APP_RTC_Init(void)
**功能描述 ：RTC初始化函数，RTC设置为22年1月1日星期六，0:0:0
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void APP_RTC_Init(void)
{
  RTC_TimeTypeDef Timeinit;
  RCC_PeriphCLKInitTypeDef RTCLCKconfig;
  //====================
  //备份域设置和RTC时钟设置
  //====================
  HAL_PWR_EnableBkUpAccess();                           //使能备份域(RTC设置在备份域中)访问
  /*RTC时钟使能*/
  __HAL_RCC_RTCAPB_CLK_ENABLE();                        //RTC模块APB时钟使能
  __HAL_RCC_RTC_ENABLE();                               //RTC时钟使能
  RTCLCKconfig.PeriphClockSelection = RCC_PERIPHCLK_RTC;//RCC扩展外设时钟为RTC
  RTCLCKconfig.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;//RTC源选择为LSE
  HAL_RCCEx_PeriphCLKConfig(&RTCLCKconfig);             //RCC扩展外设时钟初始化
  //====================
  //RTC初始化
  //====================
  RTCinit.Instance = RTC;                               //选择RTC
  RTCinit.Init.AsynchPrediv = RTC_AUTO_1_SECOND;        //RTC的1S时基自动计算
  /*2022-1-1-00:00:00*/
  RTCinit.DateToUpdate.Year = 22;                     //22年
  RTCinit.DateToUpdate.Month = RTC_MONTH_JANUARY;       //1月
  RTCinit.DateToUpdate.Date = 1;                        //1日
  RTCinit.DateToUpdate.WeekDay = RTC_WEEKDAY_SATURDAY;  //星期六
  Timeinit.Hours = 0x00;                                //0时
  Timeinit.Minutes = 0x00;                              //0分
  Timeinit.Seconds = 0x00;                              //0秒

  HAL_RTC_DeInit(&RTCinit);                             //RTC反初始化
  HAL_RTC_Init(&RTCinit);                               //RTC初始化
  HAL_RTC_SetTime(&RTCinit, &Timeinit, RTC_FORMAT_BIN); //设置RTC当前时间，格式为BIN
}
/********************************************************************************************************
**函数信息 ：void APP_RTC_SetAlarm_IT(void)
**功能描述 ：设置RTC闹钟中断
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void APP_RTC_SetAlarm_IT(uint8_t Sec, uint8_t Min, uint8_t Hour)
{
  RTC_AlarmTypeDef Alarminit;
  /*00:00:5*/
  RTCinit.Instance = RTC;
  Alarminit.AlarmTime.Hours = Hour;                           //时
  Alarminit.AlarmTime.Minutes = Min;                          //分
  Alarminit.AlarmTime.Seconds = Sec + 1;                          //秒
  HAL_RTC_SetAlarm_IT(&RTCinit, &Alarminit, RTC_FORMAT_BIN);  //设置RTC时钟和中断使能
}
/********************************************************************************************************
**函数信息 ：void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
**功能描述 ：RTC中断执行函数，退出低功耗时，LED灯亮
**输入参数 ：RTC_HandleTypeDef *hrtc
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  BSP_LED_Toggle(LED_GREEN);
  printf("%02d:%02d:%02d\r\n", ghour, gminute, gsecond);
}
/********************************************************************************************************
**函数信息 ：void SystemClock_Config(void)
**功能描述 ：RCC时钟配置
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  /*配置时钟源HSE/HSI/LSE/LSI*/
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                      //开启HSI
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                                                      //不分频
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_4MHz;                            //配置HSI输出时钟为4MHz
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_8MHz;                              //配置HSI输出时钟为8MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz;                           //配置HSI输出时钟为16MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_22p12MHz;                        //配置HSI输出时钟为22.12MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;                           //配置HSI输出时钟为24MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                                                      //关闭HSE
  RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;                                                 //HSE工作频率范围16M~32M
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;                                                      //开启LSI
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                                                      //关闭LSE
  RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;                                            //LSE默认驱动能力
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;                                                  //关闭PLL

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)                                          //初始化RCC振荡器
  {
    Error_Handler();
  }


}
/********************************************************************************************************
**函数信息 ：void Error_Handler(void)
**功能描述 ：系统时钟配置
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void Error_Handler(void)
{
  while (1)
  {
  }
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



