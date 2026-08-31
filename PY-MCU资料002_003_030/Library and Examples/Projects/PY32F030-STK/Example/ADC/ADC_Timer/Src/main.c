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
volatile uint16_t   aADCxConvertedData;
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
  //初始化所有外设，flash接口，systick
  HAL_Init();             //初始化systick
  DEBUG_USART_Config();   //初始化uart
  Timer_Init();           //TImer1初始化
  ADCConfig();            //ADC初始化
  while (1)
  {

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
  }                                                    //ADC时钟使能
  AdcHandle.Instance                    = ADC1;                                    //ADC
  AdcHandle.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV1;                //模拟ADC时钟源为PCLK
  AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;                      //转换分辨率12bit
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;                     //数据右对齐
  AdcHandle.Init.ScanConvMode          = ADC_SCAN_DIRECTION_BACKWARD;             //扫描序列方向：向下
  AdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;                     //转换结束标志
  AdcHandle.Init.LowPowerAutoWait      = ENABLE;                                  //等待转换模式开启
  AdcHandle.Init.ContinuousConvMode    = DISABLE;                                 //单次转换模式
  AdcHandle.Init.DiscontinuousConvMode = ENABLE;                                  //使能非连续模式
  AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_TRGO;            //外部触发转换启动事件为TIM1_TRG0
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_RISINGFALLING;  //上升沿和下降沿进行外部驱动
  AdcHandle.Init.DMAContinuousRequests = DISABLE;                                 //DMA单次模式选择
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
  if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)                      //通道0配置
  {
    Error_Handler();
  }
  if (HAL_ADC_Start_IT(&AdcHandle) != HAL_OK)                                     //ADC开启，并且开启中断
  {
    Error_Handler();
  }
}

/********************************************************************************************************
**函数信息 ：void Timer_Init(void)
**功能描述 ：Timer1初始化
**输入参数 ：
**输出参数 ：
**    备注 ：TIM1每100ms产生更新事件
********************************************************************************************************/
void Timer_Init(void)
{

  __HAL_RCC_TIM1_CLK_ENABLE();                                        //TIM1时钟使能
  TimHandle.Instance = TIM1;                                          //TIM1
  TimHandle.Init.Period            = 8000 - 1;                        //TIM1重装载值位8000-1
  TimHandle.Init.Prescaler         = 100 - 1;                         //预分频为100-1
  TimHandle.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;          //时钟不分配
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;              //向上计数
  TimHandle.Init.RepetitionCounter = 0;                               //不重复
  TimHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;  //自动重装载寄存器没有缓冲
  if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)                        //初始化TIM1
  {
    Error_Handler();
  }
  /*配置TIM1为主机模式*/
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;                // 选择更新事件作为触发源
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;        //主/从模式无作用
  HAL_TIMEx_MasterConfigSynchronization(&TimHandle, &sMasterConfig);  //配置TIM1
  if (HAL_TIM_Base_Start(&TimHandle) != HAL_OK)                       //TIM1时钟启动
  {
    Error_Handler();
  }

}
/********************************************************************************************************
**函数信息 ：void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
**功能描述 ：ADC执行函数，获取当前的ADC值
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  aADCxConvertedData = hadc->Instance->DR;
  printf("ADC: %03x\n\r", aADCxConvertedData);

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



