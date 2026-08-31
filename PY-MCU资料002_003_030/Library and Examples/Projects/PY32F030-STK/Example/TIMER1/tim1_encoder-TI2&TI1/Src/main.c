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
TIM_HandleTypeDef    TimHandle;
TIM_Encoder_InitTypeDef sEncoderConfig;
uint32_t uwDirection;
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  //初始化所有外设，flash接口，systick
  HAL_Init();                                                          //systick初始化
  TimHandle.Instance = TIM1;                                           //选择TIM1
  TimHandle.Init.Period            = 12800 - 1;                        //自动重装载值
  TimHandle.Init.Prescaler         = 1000 - 1;                         //预分频为1000-1
  TimHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;           //时钟不分频
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;               //向上计数
  TimHandle.Init.RepetitionCounter = 1 - 1;                            //不重复计数
  TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;   //自动重装载寄存器没有缓冲

  sEncoderConfig.EncoderMode        = TIM_ENCODERMODE_TI12;            //选择编码器模式3
  sEncoderConfig.IC1Polarity        = TIM_ICPOLARITY_RISING;           //上升沿捕获
  sEncoderConfig.IC1Selection       = TIM_ICSELECTION_DIRECTTI;        //CC1通道配置为输入
  sEncoderConfig.IC1Prescaler       = TIM_ICPSC_DIV1;                  //输入不分配
  sEncoderConfig.IC1Filter          = 0;                               //输入无滤波

  sEncoderConfig.IC2Polarity        = TIM_ICPOLARITY_RISING;           //上升沿捕获
  sEncoderConfig.IC2Selection       = TIM_ICSELECTION_DIRECTTI;        //CC2通道配置为输入
  sEncoderConfig.IC2Prescaler       = TIM_ICPSC_DIV1;                  //输入不分配
  sEncoderConfig.IC2Filter          = 0;                               //输入无滤波

  if (HAL_TIM_Encoder_Init(&TimHandle, &sEncoderConfig) != HAL_OK)     //编码器模式初始化
  {
    Error_Handler();
  }
  HAL_TIM_Encoder_Start(&TimHandle, TIM_CHANNEL_ALL);                  //所有通道编码器模式开启

  while (1)
  {
    uwDirection = __HAL_TIM_IS_TIM_COUNTING_DOWN(&TimHandle);        //获取当前是否向下计数
    printf("Direction = 0x%x\r\n ", uwDirection);                    //打印计数方向 1:向下，0:向上
  }
}





/********************************************************************************************************
**函数信息 ：void SystemClock_Config(void)
**功能描述 ：系统时钟配置
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /*配置时钟源HSE/HSI/LSE/LSI*/
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                    //开启HSI
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                                                    //不分频
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_4MHz;                          //配置HSI输出时钟为4MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_8MHz;                          //配置HSI输出时钟为8MHz
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz;                           //配置HSI输出时钟为16MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_22p12MHz;                      //配置HSI输出时钟为22.12MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;                         //配置HSI输出时钟为24MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                                                    //关闭HSE
  RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;                                                //HSE晶振工作频率16M~32M
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                                                    //关闭LSI
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                                                    //关闭LSE
  RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;                                          //LSE默认驱动能力
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                                                //开启PLL
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;                                        //选择PLL源为HSI

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)                                        //初始化RCC振荡器
  {
    Error_Handler();
  }

  //初始化CPU,AHB,APB总线时钟
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; //RCC系统时钟类型
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;                                    //SYSCLK的源选择为PLL
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                          //APH时钟不分频
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                            //APB时钟不分频

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)                      //初始化RCC系统时钟(FLASH_LATENCY_0=24M以下;FLASH_LATENCY_1=48M)
  {
    Error_Handler();
  }
}
/********************************************************************************************************
**函数信息 ：Error_Handler(void)
**功能描述 ：错误执行函数
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



