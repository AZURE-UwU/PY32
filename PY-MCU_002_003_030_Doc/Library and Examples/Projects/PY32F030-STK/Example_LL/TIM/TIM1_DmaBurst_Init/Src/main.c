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
uint32_t TIM1DataBuff[] = {2,200,3,300,4,400,5,500,6,600,7,700,8,800,9,900};

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void APP_ConfigTIM1Base(void);
static void APP_ConfigPWMChannel(void);
static void APP_ConfigDMABurst(void);
/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{
  /*配置系统时钟*/
  SystemClock_Config();
  
  /*使能TIM1、GPIO、DMA、SYSCFG时钟*/
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
  
  /*配置TIM1 PWM通道*/
  APP_ConfigPWMChannel();
  
  /*配置DMA通道*/
  APP_ConfigDMABurst();
  
  /*配置并开启TIM1 DMA Burst模式*/
  APP_ConfigTIM1Base();
  
  while (1)
  {
  }
}

/*******************************************************************************
**功能描述 ：配置TIM1 PWM模式以及相关GPIO
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_ConfigPWMChannel(void)
{
  LL_GPIO_InitTypeDef TIM1CH1MapInit= {0};
  LL_TIM_OC_InitTypeDef TIM_OC_Initstruct ={0};

  /*配置PA8为TIM1_CH1*/
  TIM1CH1MapInit.Pin        = LL_GPIO_PIN_8;
  TIM1CH1MapInit.Mode       = LL_GPIO_MODE_ALTERNATE;
  TIM1CH1MapInit.Alternate  = LL_GPIO_AF_2; 
  LL_GPIO_Init(GPIOA,&TIM1CH1MapInit);

  /*配置PWM通道*/
  /***********************************************
  ** 通道模式：        PWM1
  ** 通道状态：        开启
  ** 通道有效极性：    高电平
  ** 通道空闲极性：    低电平
  ************************************************/
  TIM_OC_Initstruct.OCMode        = LL_TIM_OCMODE_PWM1;
  TIM_OC_Initstruct.OCState       = LL_TIM_OCSTATE_ENABLE;
  TIM_OC_Initstruct.OCPolarity    = LL_TIM_OCPOLARITY_HIGH;
  TIM_OC_Initstruct.OCIdleState   = LL_TIM_OCIDLESTATE_LOW;

  /*通道1比较值:100*/
  TIM_OC_Initstruct.CompareValue  = 100;
  /*配置通道1*/
  LL_TIM_OC_Init(TIM1,LL_TIM_CHANNEL_CH1,&TIM_OC_Initstruct);
}

/*******************************************************************************
**功能描述 ：配置DMA Burst传输
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_ConfigDMABurst(void)
{
  
  LL_DMA_InitTypeDef DMA_TIM1DMABurst ={0};
  
  /*配置DMA通道1*/
  DMA_TIM1DMABurst.PeriphOrM2MSrcAddress  = (uint32_t)&(TIM1->DMAR);
  DMA_TIM1DMABurst.MemoryOrM2MDstAddress  = (uint32_t)TIM1DataBuff;
  DMA_TIM1DMABurst.Direction              = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  DMA_TIM1DMABurst.Mode                   = LL_DMA_MODE_NORMAL;
  DMA_TIM1DMABurst.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
  DMA_TIM1DMABurst.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
  DMA_TIM1DMABurst.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  DMA_TIM1DMABurst.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
  DMA_TIM1DMABurst.Priority               = LL_DMA_PRIORITY_MEDIUM;
  DMA_TIM1DMABurst.NbData                 = 16;
  /*初始化DMA通道1*/
  LL_DMA_Init(DMA1,LL_DMA_CHANNEL_1,&DMA_TIM1DMABurst);
  
  /*将TIM1更新中断映射到通道1*/
  LL_SYSCFG_SetDMARemap_CH1(LL_SYSCFG_DMA_MAP_TIM1_UP);
  
  /*使能DMA传输完成中断*/
  LL_DMA_EnableIT_TC(DMA1,LL_DMA_CHANNEL_1);
  
  /*使能DMA通道1*/
  LL_DMA_EnableChannel(DMA1,LL_DMA_CHANNEL_1);
  
  /*使能DMA通道1中断请求*/
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  NVIC_SetPriority(DMA1_Channel1_IRQn,0);
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
  ** 时钟预分频：  24
  ** 自动重装载值：1000
  ** 重复计数值：  0
  ************************************************/
  TIM1CountInit.ClockDivision       = LL_TIM_CLOCKDIVISION_DIV1;
  TIM1CountInit.CounterMode         = LL_TIM_COUNTERMODE_UP;
  TIM1CountInit.Prescaler           = 24-1;
  TIM1CountInit.Autoreload          = 1000-1;
  TIM1CountInit.RepetitionCounter   = 1;
  
  /*初始化TIM1*/
  LL_TIM_Init(TIM1,&TIM1CountInit); 
  
  /*开启更新事件DMA请求*/
  LL_TIM_EnableDMAReq_UPDATE(TIM1);
  
  /*开启DMA突发传输*/
  LL_TIM_ConfigDMABurst(TIM1,LL_TIM_DMABURST_BASEADDR_RCR,LL_TIM_DMABURST_LENGTH_2TRANSFERS);
  
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
