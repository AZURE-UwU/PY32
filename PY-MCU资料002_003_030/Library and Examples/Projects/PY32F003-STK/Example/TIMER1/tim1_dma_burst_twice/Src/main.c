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
#include "py32f0xx_hal_flash.h"
/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef    TimHandle;
TIM_OC_InitTypeDef sConfig;
uint32_t Arr_DMA[6] = {50 - 1, 3200 - 1, 1 - 1, 5 - 1, 320 - 1, 1 - 1};
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);
void SystemClock_Config(void);
void GpioPort_Init(void);
/********************************************************************************************************
**函数信息 ：int main(void)
**功能描述 ：main函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  //初始化所有外设，flash接口，systick
  HAL_Init();                                                          //systick初始化
  GpioPort_Init();                                                     //GPIOA0初始化
  SystemClock_Config();                                                //时钟配置为PLL 32M
  TimHandle.Instance = TIM1;                                           //选择TIM1
  TimHandle.Init.Period            = 6400 - 1;                         //自动重装载值
  TimHandle.Init.Prescaler         = 1000 - 1;                         //预分频为1000-1
  TimHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;           //时钟不分频
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;               //向上计数
  TimHandle.Init.RepetitionCounter = 1 - 1;                            //不重复计数
  TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;    //自动重装载寄存器缓冲使能
  if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)                         //TIM1初始化
  {
    Error_Handler();
  }
  if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)                     //TIM1使能启动，并使能中断
  {
    Error_Handler();
  }
  HAL_TIM_DMABurst_MultiWriteStart(&TimHandle, TIM_DMABASE_PSC, TIM_DMA_UPDATE,
                                   (uint32_t *)Arr_DMA, TIM_DMABURSTLENGTH_3TRANSFERS, 6);
  while (1)
  {

  }
}
/********************************************************************************************************
**函数信息 ：void GpioPort_Init(void)
**功能描述 ：初始化GPIOA0
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void GpioPort_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  __HAL_RCC_GPIOA_CLK_ENABLE();                   //使能GPIOA时钟
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;     //推挽输出
  GPIO_InitStruct.Pull = GPIO_NOPULL;             //无上拉和下拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);         //初始化GPIO
}
/********************************************************************************************************
**函数信息 ：void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
**功能描述 ：TIM执行函数，TIM1更新中断后翻转LED
**输入参数 ：TIM_HandleTypeDef *htim
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                    //开启HSI
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                                                    //不分频
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_4MHz;                          //配置HSI输出时钟为4MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_8MHz;                          //配置HSI输出时钟为8MHz
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz;                           //配置HSI输出时钟为16MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_22p12MHz;                      //配置HSI输出时钟为22.12MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;                         //配置HSI输出时钟为24MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                                                   //关闭HSE
  RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;                                               //HSE晶振工作频率16M~32M
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                                                   //关闭LSI


  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)                                        //初始化RCC振荡器
  {
    Error_Handler();
  }

  //初始化CPU,AHB,APB总线时钟
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; //RCC系统时钟类型
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;                                      //SYSCLK的源选择为HSI
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                          //APH时钟不分频
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                           //APB时钟不分频

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)                     //初始化RCC系统时钟(FLASH_LATENCY_0=24M以下;FLASH_LATENCY_1=48M)
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



