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
ADC_HandleTypeDef             AdcHandle;
ADC_ChannelConfTypeDef        sConfig;
uint32_t   gADCxConvertedData = 1;
TIM_HandleTypeDef    TimHandle;
TIM_OC_InitTypeDef       OCConfig;
TIM_MasterConfigTypeDef sMasterConfig;

/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);
void Timer_Init(void);
void ADCConfig(void);

/********************************************************************************************************
**函数信息 ：void main(void)
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  HAL_Init();                                            //初始化systick
  DEBUG_USART_Config();                                  //初始化uart
  ADCConfig();                                           //ADC初始化
  while (1)
  {
    if (__HAL_DMA_GET_FLAG(DMA1->ISR, DMA_ISR_TCIF1))    //DMA通道1传输完成
    {
      __HAL_DMA_CLEAR_FLAG(DMA1->ISR, DMA_IFCR_CTCIF1); //清DMA通道1传输完成标志
      printf("ADC: %03x \r\n", gADCxConvertedData);
    }
  }
}


/********************************************************************************************************
**函数信息 ：void ADCConfig(void)
**功能描述 ：ADC初始化
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void ADCConfig(void)
{
	__HAL_RCC_ADC_FORCE_RESET();
  __HAL_RCC_ADC_RELEASE_RESET();
	__HAL_RCC_ADC_CLK_ENABLE();

  /*ADC校准*/
  AdcHandle.Instance = ADC1;

  if (HAL_ADCEx_Calibration_Start(&AdcHandle) != HAL_OK)                  //AD校准
  {
    Error_Handler();
  }                                                  //ADC时钟使能
  AdcHandle.Instance                   = ADC1;                                    //ADC
  AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV1;                //模拟ADC时钟源为PCLK
  AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;                      //转换分辨率12bit
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;                     //数据右对齐
  AdcHandle.Init.ScanConvMode          = ADC_SCAN_DIRECTION_BACKWARD;             //扫描序列方向：向下
  AdcHandle.Init.LowPowerAutoWait      = ENABLE;                                  //等待转换模式开启
  AdcHandle.Init.ContinuousConvMode    = ENABLE;                                  //连续转换模式
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;                                 //使能连续模式
  AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;                      //ADC无外部事件
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;           //硬件驱动检测不使能
  AdcHandle.Init.DMAContinuousRequests = ENABLE;                                  //DMA循环模式选择
  AdcHandle.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;                //当过载发生时，ADC_DR保留久值
  AdcHandle.Init.SamplingTimeCommon    = ADC_SAMPLETIME_239CYCLES_5;              //通道采样时间为239.5ADC时钟周期
  /*ADC初始化*/
  if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
  {
    Error_Handler();
  }
  //====================
  //输入通道0选择配置
  //====================
  sConfig.Rank         = ADC_RANK_CHANNEL_NUMBER;
  sConfig.Channel      = ADC_CHANNEL_0;
  if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)                     //通道0配置
  {
    Error_Handler();
  }
  if (HAL_ADC_Start_DMA(&AdcHandle, &gADCxConvertedData, 1) != HAL_OK)            //ADC开启
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



