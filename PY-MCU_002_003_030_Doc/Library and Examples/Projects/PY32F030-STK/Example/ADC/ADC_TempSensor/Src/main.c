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
#define Vcc_Power     3.3l                                            //VCC电源电压,根据实际情况修改
#define TScal1        (float)((HAL_ADC_TSCAL1) * 3.3 / Vcc_Power)     //85摄氏度校准值对应电压
#define TScal2        (float)((HAL_ADC_TSCAL2) * 3.3 / Vcc_Power)     //30摄氏度校准值对应电压
#define TStem1        30l                                             //30摄氏度
#define TStem2        85l                                             //85摄氏度
#define Temp_k        ((float)(TStem2-TStem1)/(float)(TScal2-TScal1)) //温度计算
/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef             AdcHandle;
ADC_ChannelConfTypeDef        sConfig;
volatile int16_t   aADCxConvertedData;
int16_t   aTEMPERATURE;
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);
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
  //初始化所有外设，flash接口，systick
  HAL_Init();                                                             //初始化systick
  DEBUG_USART_Config();                                                   //USART初始化
  ADCConfig();                                                            //ADC初始化
  while (1)
  {
    HAL_Delay(500);                                                     //
    if (HAL_ADC_Start_IT(&AdcHandle) != HAL_OK)                         //启动ADC并使能ADC中断
    {
      Error_Handler();
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
  }
	
  AdcHandle.Instance                    = ADC1;                                    //ADC
  AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV1;                //模拟ADC时钟源为PCLK
  AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;                      //转换分辨率12bit
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;                     //数据右对齐
  AdcHandle.Init.ScanConvMode          = ADC_SCAN_DIRECTION_FORWARD;              //扫描序列方向：向上(从通道0到通道11)
  AdcHandle.Init.EOCSelection          = ADC_EOC_SEQ_CONV;                        //转换结束标志
  AdcHandle.Init.LowPowerAutoWait      = ENABLE;                                  //等待转换模式开启
  AdcHandle.Init.ContinuousConvMode    = DISABLE;                                 //单次转换模式
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;                                 //不使能非连续模式
  AdcHandle.Init.ExternalTrigConv      = ADC_SOFTWARE_START;                      //ADC无外部事件
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;           //硬件驱动检测不使能
  AdcHandle.Init.DMAContinuousRequests = ENABLE;                                  //DMA单次模式选择
  AdcHandle.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;                //当过载发生时，ADC_DR保留久值
  AdcHandle.Init.SamplingTimeCommon    = ADC_SAMPLETIME_41CYCLES_5;              //通道采样时间为239.5ADC时钟周期
  if (HAL_ADC_Init(&AdcHandle) != HAL_OK)                                         //ADC初始化
  {
    Error_Handler();
  }

  sConfig.Rank         = ADC_RANK_CHANNEL_NUMBER;                                 //设置是否排行, 想设置单通道采样,需配置ADC_RANK_NONE
  sConfig.Channel      = ADC_CHANNEL_TEMPSENSOR;                                  //设置采样通道
  if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)                      //配置ADC通道
  {
    Error_Handler();
  }

}
/********************************************************************************************************
**函数信息 ：HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
**功能描述 ：ADC转换完成执行函数,打印当前测得得温度值
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  aADCxConvertedData = hadc->Instance->DR;
  aTEMPERATURE =(int16_t)(Temp_k * aADCxConvertedData - Temp_k * TScal1 + TStem1);
  printf(" TEMPERAUTE=%d \r\n", aTEMPERATURE);

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



