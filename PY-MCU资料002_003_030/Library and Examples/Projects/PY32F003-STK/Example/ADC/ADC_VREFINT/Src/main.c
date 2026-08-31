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
ADC_HandleTypeDef hadc;
uint8_t adcValueflag;
uint32_t adc_value[6];
float  T_VCC;
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);
void ADC_Config(void);
/********************************************************************************************************
**函数信息 ：void main(void)
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
//    uint8_t i;
  //初始化所有外设，flash接口，systick

  HAL_Init();                                 //初始化systick
  ADC_Config();                               //初始化ADC
  DEBUG_USART_Config();                       //UART初始化
  printf("test start..\r\n");
  while (1)
  {
    HAL_ADC_Start(&hadc);                     //启动ADC
    HAL_ADC_PollForConversion(&hadc, 1000000); //等待ADC转换完成
    adc_value[0] = HAL_ADC_GetValue(&hadc);   //获取ADC值
    T_VCC = (4095 * 1.2) / adc_value[0];      //计算VCC电压
    printf("vcc:%f V\r\n", T_VCC);
  }
}
/********************************************************************************************************
**函数信息 ：void ADCConfig(void)
**功能描述 ：ADC初始化
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void ADC_Config(void)
{

  ADC_ChannelConfTypeDef sConfig = {0};
  /*ADC校准*/
	__HAL_RCC_ADC_FORCE_RESET();
  __HAL_RCC_ADC_RELEASE_RESET();
	__HAL_RCC_ADC_CLK_ENABLE();

  /*ADC校准*/
  hadc.Instance = ADC1;

  if (HAL_ADCEx_Calibration_Start(&hadc) != HAL_OK)                  //AD校准
  {
    Error_Handler();
  }    
  //=============
  //ADC配置
  //=============
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;              //设置ADC时钟
  hadc.Init.Resolution = ADC_RESOLUTION_12B;                        //设置ADC采样位数
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;                        //设置ADC转换结构右对齐
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;              //设置ADC转换方向, 向上或者向下
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;                     //ADC_EOC_SINGLE_CONV:单次采样 ; ADC_EOC_SEQ_CONV:序列采样
  hadc.Init.LowPowerAutoWait = ENABLE;                              //ENABLE=读取ADC值后,开始下一次转换 ; DISABLE=直接转换
  hadc.Init.ContinuousConvMode = DISABLE;                            //ENABLE=连续模式, DISABLE=单次模式
  hadc.Init.DiscontinuousConvMode = DISABLE;                        //非连续转换
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;                  //软件使能
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;   //外部触发方式
  hadc.Init.DMAContinuousRequests = DISABLE;                        //禁止DMA
  hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                     //过载模式,0=保留旧值,1=新值覆盖旧值
  hadc.Init.SamplingTimeCommon = ADC_SAMPLETIME_41CYCLES_5;         //设置采样周期41.5个ADC时钟
  if (HAL_ADC_Init(&hadc) != HAL_OK)                                //ADC初始化
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_VREFINT;                            //设置采样通道
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;                           //设置是否排行, 想设置单通道采样,需配置ADC_RANK_NONE
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)             //配置ADC通道
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



