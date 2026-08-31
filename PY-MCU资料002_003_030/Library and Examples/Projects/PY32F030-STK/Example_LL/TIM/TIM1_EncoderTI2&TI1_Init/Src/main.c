/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @Version V1.0.0
  * @Date    2022-8-19
  * @brief   main function
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "py32f030xx_ll_Start_Kit.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void APP_ConfigTIM1Encoder(void);
/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{
  /*使能TIM1、GPIOA时钟*/
  LL_APB1_GRP2_EnableClock(RCC_APBENR2_TIM1EN);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);

  /*配置系统时钟*/
  SystemClock_Config();
  
  /*初始化LED、USART*/
  BSP_LED_Init(LED3);
  BSP_USART_Config();
  
  /*配置并开启TIM1编码器模式*/
  APP_ConfigTIM1Encoder();
  
  while (1)
  {
  }
}

/*******************************************************************************
**功能描述 ：配置TIM编码器模式
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_ConfigTIM1Encoder(void)
{
  LL_TIM_InitTypeDef TIM1CountInit = {0};
  LL_TIM_ENCODER_InitTypeDef TIM1EncoderInit = {0};
  LL_GPIO_InitTypeDef TIM1ChannelInit = {0};
  
  /*配置TIM1*/
  /***********************************************
  ** 时钟预分频：  1 
  ** 自动重装载值：1000 
  ** 重复计数值：  0
  ************************************************/
  TIM1CountInit.Prescaler           = 1-1;
  TIM1CountInit.Autoreload          = 1000-1;
  TIM1CountInit.RepetitionCounter   = 0;
  
  /*初始化TIM1*/
  LL_TIM_Init(TIM1,&TIM1CountInit);
  
  /*开启CH1和CH2捕获中断*/
  LL_TIM_EnableIT_CC1(TIM1);
  LL_TIM_EnableIT_CC2(TIM1);
  NVIC_EnableIRQ(TIM1_CC_IRQn);
  
  /*配置Encoder*/
  LL_TIM_SetEncoderMode(TIM1,LL_TIM_ENCODERMODE_X4_TI12);
  /***********************************************
  ** 编码器模式：         模式3 
  ** 输入捕获1有效边沿：  双边沿
  ** 输入捕获1有效输入：  TI1
  ** 输入捕获1滤波值：    无滤波
  ** 输入捕获1预分频值：  1
  ** 输入捕获1有效边沿：  双边沿
  ** 输入捕获1有效输入：  TI2
  ** 输入捕获1滤波值：    无滤波 
  ** 输入捕获1预分频值：  1
  ************************************************/
  TIM1EncoderInit.EncoderMode     = LL_TIM_ENCODERMODE_X4_TI12;
  TIM1EncoderInit.IC1Polarity     = LL_TIM_IC_POLARITY_BOTHEDGE;
  TIM1EncoderInit.IC1ActiveInput  = LL_TIM_ACTIVEINPUT_DIRECTTI;
  TIM1EncoderInit.IC1Filter       = LL_TIM_IC_FILTER_FDIV1;
  TIM1EncoderInit.IC1Prescaler    = LL_TIM_ICPSC_DIV1;
  TIM1EncoderInit.IC2Polarity     = LL_TIM_IC_POLARITY_BOTHEDGE;
  TIM1EncoderInit.IC2ActiveInput  = LL_TIM_ACTIVEINPUT_DIRECTTI;
  TIM1EncoderInit.IC2Filter       = LL_TIM_IC_FILTER_FDIV1;
  TIM1EncoderInit.IC2Prescaler    = LL_TIM_ICPSC_DIV1;
  
  /*初始化Encoder配置*/
  LL_TIM_ENCODER_Init(TIM1,&TIM1EncoderInit);
  
  /*TI1和TI2映射到PA8和PA9*/
  TIM1ChannelInit.Pin       = LL_GPIO_PIN_8 | LL_GPIO_PIN_9;
  TIM1ChannelInit.Mode      = LL_GPIO_MODE_ALTERNATE;
  TIM1ChannelInit.Alternate = LL_GPIO_AF_2;
  LL_GPIO_Init(GPIOA,&TIM1ChannelInit);
  
  /*使能TIM1计数器*/
  LL_TIM_EnableCounter(TIM1);
  
}

/*******************************************************************************
**功能描述 ：系统时钟配置函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void SystemClock_Config(void)
{
  /* 使能HSI */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  /* 设置 AHB 分频*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* 配置HSISYS作为系统时钟源 */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSISYS);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSISYS)
  {
  }

  /* 设置 APB1 分频*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_Init1msTick(8000000);
  
  /* 更新系统时钟全局变量SystemCoreClock(也可以通过调用SystemCoreClockUpdate函数更新) */
  LL_SetSystemCoreClock(8000000);
}

/*******************************************************************************
**功能描述 ：捕获回调函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_CCCallback(void)
{
  printf("Count:%d\r\n",LL_TIM_GetCounter(TIM1));
}

/*******************************************************************************
**功能描述 ：错误执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void Error_Handler(void)
{
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/*******************************************************************************
**功能描述 ：输出产生断言错误的源文件名及行号
**输入参数 ：file：源文件名指针
**输入参数 ：line：发生断言错误的行号
**输出参数 ：
*******************************************************************************/
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
