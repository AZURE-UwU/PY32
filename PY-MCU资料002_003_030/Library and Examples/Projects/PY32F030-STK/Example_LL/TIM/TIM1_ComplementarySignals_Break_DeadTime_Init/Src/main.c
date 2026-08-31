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
static void APP_ConfigTIM1Base(void);
static void APP_ConfigPWMChannel(void);
static void APP_ConfigBDTR(void);
/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{
  /*配置系统时钟*/
  SystemClock_Config();
  
  /*使能TIM1、GPIO时钟*/
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA|LL_IOP_GRP1_PERIPH_GPIOB);
  
  /*配置TIM1 PWM通道*/
  APP_ConfigPWMChannel();
  
  /*配置刹车和死区时间*/
  APP_ConfigBDTR();
  
  /*配置并开启TIM1 PWM模式*/
  APP_ConfigTIM1Base();
  
  while (1)
  {
  }
}

/*******************************************************************************
**功能描述 ：配置死区、刹车
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_ConfigBDTR(void)
{
  LL_TIM_BDTR_InitTypeDef TIM1BDTRInit = {0};
  LL_GPIO_InitTypeDef     TIM1BreakMapInit = {0};
  
  /*死区和刹车配置*/
  /***********************************************
  **自动输出：     使能自动输出
  **刹车有效极性： 高电平有效
  **刹车状态：     开启
  **死区时间：     200ns
  ***********************************************/
  TIM1BDTRInit.AutomaticOutput  = LL_TIM_AUTOMATICOUTPUT_ENABLE;
  TIM1BDTRInit.BreakPolarity    = LL_TIM_BREAK_POLARITY_HIGH;
  TIM1BDTRInit.BreakState       = LL_TIM_BREAK_ENABLE;
  TIM1BDTRInit.DeadTime         = __LL_TIM_CALC_DEADTIME(24000000,LL_TIM_CLOCKDIVISION_DIV1,200);
  
  /*配置PA6为刹车引脚*/
  TIM1BreakMapInit.Pin        = LL_GPIO_PIN_6;
  TIM1BreakMapInit.Mode       = LL_GPIO_MODE_ALTERNATE;
  TIM1BreakMapInit.Alternate  = LL_GPIO_AF_2; 
  LL_GPIO_Init(GPIOA,&TIM1BreakMapInit);
  
  /*初始化死区和刹车配置*/
  LL_TIM_BDTR_Init(TIM1,&TIM1BDTRInit);
}

/*******************************************************************************
**功能描述 ：配置TIM1 PWM相关GPIO
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_ConfigPWMChannel(void)
{
  LL_GPIO_InitTypeDef TIM1CH1MapInit= {0};
  LL_TIM_OC_InitTypeDef TIM_OC_Initstruct ={0};

  /*配置PA7/PA8/PA9/PA10为TIM1_CH1N/TIM1_CH1/TIM1_CH2/TIM1_CH3*/
  TIM1CH1MapInit.Pin        = LL_GPIO_PIN_7|LL_GPIO_PIN_8|LL_GPIO_PIN_9|LL_GPIO_PIN_10;
  TIM1CH1MapInit.Mode       = LL_GPIO_MODE_ALTERNATE;
  TIM1CH1MapInit.Alternate  = LL_GPIO_AF_2; 
  LL_GPIO_Init(GPIOA,&TIM1CH1MapInit);
  
  /*配置PB0/PB1为TIM1_CH2N/TIM1_CH3N*/
  TIM1CH1MapInit.Pin        = LL_GPIO_PIN_0|LL_GPIO_PIN_1;
  TIM1CH1MapInit.Mode       = LL_GPIO_MODE_ALTERNATE;
  TIM1CH1MapInit.Alternate  = LL_GPIO_AF_2; 
  LL_GPIO_Init(GPIOB,&TIM1CH1MapInit);
  
  /*配置PWM通道*/
  /***********************************************
  ** 通道模式：        PWM1
  ** 通道状态：        开启
  ** 互补通道状态：    开启
  ** 通道有效极性：    高电平
  ** 互补通道有效极性：高电平
  ** 通道空闲极性：    低电平
  ** 互补通道空闲极性：低电平
  ************************************************/
  TIM_OC_Initstruct.OCMode        = LL_TIM_OCMODE_PWM1;
  TIM_OC_Initstruct.OCState       = LL_TIM_OCSTATE_ENABLE;
  TIM_OC_Initstruct.OCNState      = LL_TIM_OCSTATE_ENABLE;
  TIM_OC_Initstruct.OCPolarity    = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_Initstruct.OCNPolarity   = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_Initstruct.OCIdleState   = LL_TIM_OCIDLESTATE_LOW;
  TIM_OC_Initstruct.OCNIdleState  = LL_TIM_OCIDLESTATE_LOW;
  /*通道1比较值:250*/
  TIM_OC_Initstruct.CompareValue  = 250;
  /*配置通道1*/
  LL_TIM_OC_Init(TIM1,LL_TIM_CHANNEL_CH1,&TIM_OC_Initstruct);
  /*通道2比较值:500*/
  TIM_OC_Initstruct.CompareValue  = 500;
  /*配置通道2*/
  LL_TIM_OC_Init(TIM1,LL_TIM_CHANNEL_CH2,&TIM_OC_Initstruct);
  /*通道3比较值:750*/
  TIM_OC_Initstruct.CompareValue  = 750;
  /*配置通道3*/
  LL_TIM_OC_Init(TIM1,LL_TIM_CHANNEL_CH3,&TIM_OC_Initstruct);
}

/*******************************************************************************
**功能描述 ：配置TIM base
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_ConfigTIM1Base(void)
{
  /*配置TIM1*/
  LL_TIM_InitTypeDef TIM1CountInit = {0};
 
  /***********************************************
  ** 输入时钟：    24000000
  ** 计数模式：    向上计数模式
  ** 时钟预分频：  2400
  ** 自动重装载值：1000
  ** 重复计数值：  0
  ************************************************/
  TIM1CountInit.ClockDivision       = LL_TIM_CLOCKDIVISION_DIV1;
  TIM1CountInit.CounterMode         = LL_TIM_COUNTERMODE_UP;
  TIM1CountInit.Prescaler           = 2400-1;
  TIM1CountInit.Autoreload          = 1000-1;
  TIM1CountInit.RepetitionCounter   = 0;
  
  /*初始化TIM1*/
  LL_TIM_Init(TIM1,&TIM1CountInit);
  
  /*主输出使能*/
  LL_TIM_EnableAllOutputs(TIM1);

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
  LL_RCC_HSI_SetCalibFreq(LL_RCC_HSICALIBRATION_24MHz);
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
  LL_Init1msTick(24000000);
  
  /* 更新系统时钟全局变量SystemCoreClock(也可以通过调用SystemCoreClockUpdate函数更新) */
  LL_SetSystemCoreClock(24000000);
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
