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
#include "py32f003xx_ll_Start_Kit.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void APP_ConfigTIM1Slave(void);
static void APP_ConfigTIM3Master(void);

/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{
  /*使能TIM1,TIM3时钟*/
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  /*配置系统时钟*/
  SystemClock_Config();
  
  /*初始化LED，USART*/
  BSP_LED_Init(LED3);
  BSP_USART_Config();
  
  /*开启TIM1*/
  APP_ConfigTIM1Slave();
  
  /*开启TIM3*/
  APP_ConfigTIM3Master();
  
  while (1)
  {
    printf("TIM1Count:%d\r\n",LL_TIM_GetCounter(TIM1));
    LL_mDelay(1000);
  }
}

/*******************************************************************************
**功能描述 ：配置TIM3 Master
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_ConfigTIM3Master(void)
{
  /*预分频值：8000*/
  LL_TIM_SetPrescaler(TIM3,8000-1);
  
  /*计数模式：向上计数*/
  LL_TIM_SetCounterMode(TIM3,LL_TIM_COUNTERMODE_UP);
  
  /*重装载值：1000-1*/
  LL_TIM_SetAutoReload(TIM3,1000-1);
  
  /*时钟不分频*/
  LL_TIM_SetClockDivision(TIM3,LL_TIM_CLOCKDIVISION_DIV1);
  
  /*触发输出为更新事件*/
  LL_TIM_SetTriggerOutput(TIM3,LL_TIM_TRGO_UPDATE);
  
  /*使能TIM3计数器*/
  LL_TIM_EnableCounter(TIM3);
}

/*******************************************************************************
**功能描述 ：配置TIM1 Slave
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_ConfigTIM1Slave(void)
{
  /*计数模式：向上计数*/
  LL_TIM_SetCounterMode(TIM1,LL_TIM_COUNTERMODE_UP);
  
  /*自动重装载值：10*/
  LL_TIM_SetAutoReload(TIM1,10);
  
  /*触发输入为TIM3*/
  LL_TIM_SetTriggerInput(TIM1,LL_TIM_TS_ITR2);
  
  /*设置为外部时钟模式1*/
  LL_TIM_SetClockSource(TIM1,LL_TIM_CLOCKSOURCE_EXT_MODE1);
  
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
