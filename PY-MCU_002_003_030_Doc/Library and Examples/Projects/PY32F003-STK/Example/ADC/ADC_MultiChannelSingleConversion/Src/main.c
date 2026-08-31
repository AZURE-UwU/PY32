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
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;
uint32_t adc_value[3];
/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);
void SystemClock_Config(void);
void ADC_Init(void);
/********************************************************************************************************
**函数信息 ：void main(void)
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  uint8_t i;
  //初始化所有外设，flash接口，systick
  HAL_Init();                                   //初始化systick
  SystemClock_Config();                         //时钟初始化
  ADC_Init();                                   //ADC初始化
  DEBUG_USART_Config();                         //UART初始化
  while (1)
  {
    HAL_ADC_Start(&hadc);                       //ADC开启
    for (i = 0; i < 3; i++)
    {
      HAL_ADC_PollForConversion(&hadc, 10000);  //等待ADC转换
      adc_value[i] = HAL_ADC_GetValue(&hadc);   //获取AD值
      printf("adc[%d]:%d\r\n", i, adc_value[i]);
    }
  }
}

/********************************************************************************************************
**函数信息 ：SystemClock_Config(void)
**功能描述 ：配置系统时钟
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /*配置HSI,HSE,LSE,LSI,PLL所有时钟*/
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                      //使能HSI
  RCC_OscInitStruct.HSIDiv =    RCC_HSI_DIV1;                                    //HSI预分频
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_8MHz;              //设置HSI输出时钟为8MHz,库会设置校准值
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                                     //禁止HSE
  RCC_OscInitStruct.HSEFreq =  RCC_HSE_16_32MHz;                                //设置HSE频率范围,没用可以不设置
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                                     //禁止LSI

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)                          //配置时钟
  {
    Error_Handler();
  }

  /*初始化AHB,APB总线时钟*/
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;                        //配置AHB时钟源
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                            //设置AHB预分频
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                             //设置APB1预分频

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)       //配置总线
  {
    Error_Handler();
  }
}

/********************************************************************************************************
**函数信息 ：ADC_Init(void)
**功能描述 ：ADC初始化
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void ADC_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};
	__HAL_RCC_ADC_FORCE_RESET();
  __HAL_RCC_ADC_RELEASE_RESET();
	__HAL_RCC_ADC_CLK_ENABLE();

  /*ADC校准*/
  hadc.Instance = ADC1;

  if (HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK)                  //AD校准
  {
    Error_Handler();
  }

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;              //ADC_CLOCK_SYNC_PCLK_DIV2;  ADC_CLOCK_SYNC_PCLK_DIV4
  hadc.Init.Resolution = ADC_RESOLUTION_12B;                        //设置ADC采样位数
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;                        //右对齐
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;              //扫描方向设置
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;                      //ADC_EOC_SINGLE_CONV:单次采样 ; ADC_EOC_SEQ_CONV:序列采样
  hadc.Init.LowPowerAutoWait = ENABLE;                              //ENABLE=读取ADC值后,开始下一次转换 ; DISABLE=直接转换
  hadc.Init.ContinuousConvMode = DISABLE;                            //ENABLE=连续模式, DISABLE=单次模式
  hadc.Init.DiscontinuousConvMode = DISABLE;                        //非连续转换模式设置
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;                  //触发模式设置
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;   //外部触发沿设置
  hadc.Init.DMAContinuousRequests = DISABLE;                        //DMA连续模式设置
  hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                     //ADC_OVR_DATA_OVERWRITTEN=过载时覆盖,ADC_OVR_DATA_PRESERVED=保留旧值
//  hadc.Init.SamplingTimeCommon=ADC_SAMPLETIME_13CYCLES_5;           //设置采样周期

  if (HAL_ADC_Init(&hadc) != HAL_OK)                                //初始化ADC
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted.
  */
  sConfig.Channel = ADC_CHANNEL_0;                                  //ADC通道选择
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;                           //ADC_RANK_CHANNEL_NUMBER=排如序列通道, ADC_RANK_NONE=非排如序列通道
//  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;                 //设置采样周期
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)             //初始化ADC通道
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_1;                                  //ADC通道选择
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;                           //ADC_RANK_CHANNEL_NUMBER=排如序列通道, ADC_RANK_NONE=非排如序列通道
//  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;                 //设置采样周期
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)             //初始化ADC通道
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_4;                                  //ADC通道选择
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;                           //ADC_RANK_CHANNEL_NUMBER=排如序列通道, ADC_RANK_NONE=非排如序列通道
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;                 //设置采样周期
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)             //初始化ADC通道
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



