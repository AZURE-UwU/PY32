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
uint32_t uwStep = 0;
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void APP_ConfigTIM1Base(void);
static void APP_Config6StepPWMChannel(void);

/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{
  /*配置系统时钟*/
  SystemClock_Config();
  
  /*开启SysTick中断*/
  LL_SYSTICK_EnableIT();
  
  /*使能TIM1、GPIO时钟*/
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA|LL_IOP_GRP1_PERIPH_GPIOB);
  
  /*配置并开启TIM16StepPWM模式*/
  APP_ConfigTIM1Base();
  
  while (1)
  {
  }
}

/*******************************************************************************
**功能描述 ：配置TIM1 6StepPWM相关GPIO
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_Config6StepPWMChannel(void)
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
  ** 比较值：          50
  ************************************************/
  TIM_OC_Initstruct.OCMode        = LL_TIM_OCMODE_PWM1;
  TIM_OC_Initstruct.OCState       = LL_TIM_OCSTATE_ENABLE;
  TIM_OC_Initstruct.OCNState      = LL_TIM_OCSTATE_ENABLE;
  TIM_OC_Initstruct.OCPolarity    = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_Initstruct.OCNPolarity   = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_Initstruct.OCIdleState   = LL_TIM_OCIDLESTATE_LOW;
  TIM_OC_Initstruct.OCNIdleState  = LL_TIM_OCIDLESTATE_LOW;
  TIM_OC_Initstruct.CompareValue  = 50;
  /*配置通道1*/
  LL_TIM_OC_Init(TIM1,LL_TIM_CHANNEL_CH1,&TIM_OC_Initstruct);
  /*配置通道2*/
  LL_TIM_OC_Init(TIM1,LL_TIM_CHANNEL_CH2,&TIM_OC_Initstruct);
  /*配置通道3*/
  LL_TIM_OC_Init(TIM1,LL_TIM_CHANNEL_CH3,&TIM_OC_Initstruct);
  /*关闭所有通道输出*/
  LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH1|LL_TIM_CHANNEL_CH1N|
                                LL_TIM_CHANNEL_CH2|LL_TIM_CHANNEL_CH2N|
                                LL_TIM_CHANNEL_CH3|LL_TIM_CHANNEL_CH3N);
}

/*******************************************************************************
**功能描述 ：TIM1 6StepPWM配置函数
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
  ** 时钟预分频：  240
  ** 自动重装载值：100
  ** 重复计数值：  0
  ************************************************/
  TIM1CountInit.ClockDivision       = LL_TIM_CLOCKDIVISION_DIV1;
  TIM1CountInit.CounterMode         = LL_TIM_COUNTERMODE_UP;
  TIM1CountInit.Prescaler           = 24-1;
  TIM1CountInit.Autoreload          = 100-1;
  TIM1CountInit.RepetitionCounter   = 0;
  
  /*初始化TIM1*/
  LL_TIM_Init(TIM1,&TIM1CountInit);
  
  /*配置TIM16步PWM相关通道*/
  APP_Config6StepPWMChannel();
  
  /*使能COM中断*/
  LL_TIM_EnableIT_COM(TIM1);
  /*设置COM中断优先级*/
  NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn,1);
  /*使能COM中断NVIC IRQ*/
  NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
  
  /*使能预装载功能*/
  LL_TIM_CC_EnablePreload(TIM1);
  
  /*主输出使能*/
  LL_TIM_EnableAllOutputs(TIM1);

  /*使能TIM1计数器*/
  LL_TIM_EnableCounter(TIM1);
}

/*******************************************************************************
**功能描述 ：TIM1 COM中断回调函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_COMCallback(void)
{
  switch(uwStep)
  {
    /*第0步（起始状态）*/
    case 0:
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH1);   /*CH1 : 1*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH1N); /*CH1N: 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH2);  /*CH2 : 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH2N); /*CH2N: 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH3);  /*CH3 : 0*/
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH3N);  /*CH3N: 1*/
      uwStep++;
      break;
    /*第1步*/
    case 1:
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH1);   /*CH1 : 1*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH1N); /*CH1N: 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH2);  /*CH2 : 0*/
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH2N);  /*CH2N: 1*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH3);  /*CH3 : 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH3N); /*CH3N: 0*/
      uwStep++;
      break;
    /*第2步*/
    case 2:
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH1);  /*CH1 : 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH1N); /*CH1N: 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH2);  /*CH2 : 0*/
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH2N);  /*CH2N: 1*/
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH3);   /*CH3 : 1*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH3N); /*CH3N: 0*/
      uwStep++;
      break;
    /*第3步*/
    case 3:
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH1);  /*CH1 : 0*/
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH1N);  /*CH1N: 1*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH2);  /*CH2 : 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH2N); /*CH2N: 0*/
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH3);   /*CH3 : 1*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH3N); /*CH3N: 0*/
      uwStep++;
      break;
    /*第4步*/
    case 4:
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH1);  /*CH1 : 0*/
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH1N);  /*CH1N: 1*/
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH2);   /*CH2 : 1*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH2N); /*CH2N: 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH3);  /*CH3 : 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH3N); /*CH3N: 0*/
      uwStep++;
      break;
    /*第5步*/
    case 5:
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH1);  /*CH1 : 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH1N); /*CH1N: 0*/
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH2);   /*CH2 : 1*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH2N); /*CH2N: 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH3);  /*CH3 : 0*/
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH3N);  /*CH3N: 1*/
      uwStep++;
      break;
    /*第6步*/
    case 6:
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH1);   /*CH1 : 1*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH1N); /*CH1N: 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH2);  /*CH2 : 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH2N); /*CH2N: 0*/
      LL_TIM_CC_DisableChannel(TIM1,LL_TIM_CHANNEL_CH3);  /*CH3 : 0*/
      LL_TIM_CC_EnableChannel(TIM1,LL_TIM_CHANNEL_CH3N);  /*CH3N: 1*/
      uwStep = 1;
      break;
  }
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
