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
ADC_AnalogWDGConfTypeDef      ADCAnalogWDGConfig;
uint32_t adc_value[2];
/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);
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
  ADC_Init();                                   //ADC初始化
  DEBUG_USART_Config();                         //UART初始化
  while (1)
  {
    HAL_ADC_Start(&hadc);                       //ADC开启
    for (i = 0; i < 2; i++)
    {
      HAL_ADC_PollForConversion(&hadc, 10000);  //等待ADC转换
      adc_value[i] = HAL_ADC_GetValue(&hadc);   //获取AD值
      printf("adc[%d]:%d\r\n", i, adc_value[i]);
    }
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
  /* 先复位ADC值 */
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
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV32;              //设置ADC时钟
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
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)             //初始化ADC通道
  {
    Error_Handler();
  }

  //ADC 模拟看门狗初始化
  ADCAnalogWDGConfig.WatchdogMode = ADC_ANALOGWATCHDOG_ALL_REG;         //所有通道=ADC_ANALOGWATCHDOG_ALL_REG ;   单通道=ADC_ANALOGWATCHDOG_SINGLE_REG;
  ADCAnalogWDGConfig.HighThreshold = 0X1FF;                              //设置上阈值
  ADCAnalogWDGConfig.LowThreshold = 0;                                  //设置下阈值
  ADCAnalogWDGConfig.ITMode = ENABLE;                                    //ENABLE=使能中断, DISABLE=禁止中断

  if (HAL_ADC_AnalogWDGConfig(&hadc, &ADCAnalogWDGConfig) != HAL_OK)     //ADC模拟看门狗配置
  {
    Error_Handler();
  }
  HAL_NVIC_SetPriority(ADC_COMP_IRQn, 0, 0);                            //设置ADC中断优先级
  HAL_NVIC_EnableIRQ(ADC_COMP_IRQn);                                    //设置ADC内核中断
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



