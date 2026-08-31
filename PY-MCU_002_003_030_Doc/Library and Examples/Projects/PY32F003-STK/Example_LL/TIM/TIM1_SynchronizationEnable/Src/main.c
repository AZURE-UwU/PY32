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
static void APP_ConfigTIM1Trigger_Slave(void);
static void APP_ConfigTIM3Trigger_Master(void);
/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{
  /* 配置系统时钟 */
  SystemClock_Config();
  
  /*初始化LED*/
  BSP_LED_Init(LED_GREEN);
  BSP_LED_On(LED_GREEN);
  
  /*使能TIM1、TIM3时钟*/
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

  /*配置TIM1为同步触发从模式*/
  APP_ConfigTIM1Trigger_Slave();

  /*配置TIM3为同步触发主模式*/
  APP_ConfigTIM3Trigger_Master();
  
  while (1)
  {

  }
}

/*******************************************************************************
**功能描述 ：TIM1 同步触发模式配置函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_ConfigTIM1Trigger_Slave(void)
{
  /*CK_INT 1分频*/
  LL_TIM_SetClockDivision(TIM1,LL_TIM_CLOCKDIVISION_DIV1);
  /*向上计数模式*/
  LL_TIM_SetCounterMode(TIM1,LL_TIM_COUNTERMODE_UP);
  /*重装载值1000*/
  LL_TIM_SetAutoReload(TIM1,1000-1);
  /*CK_CNT 预分频值：8000*/
  LL_TIM_SetPrescaler(TIM1,8000-1);
  /*从模式触发*/
  LL_TIM_SetSlaveMode(TIM1,LL_TIM_SLAVEMODE_TRIGGER);
  /*触发输入选择ITR2（TIM3）*/
  LL_TIM_SetTriggerInput(TIM1,LL_TIM_TS_ITR2);
  
  /*开启UPDATE中断*/
  LL_TIM_EnableIT_UPDATE(TIM1);
  /*设置UPDATE中断NVIC优先级*/
  NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 1);
  /*使能UPDATE中断NVIC IRQ*/
  NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
  
}

/*******************************************************************************
**功能描述 ：TIM1 更新中断回调函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_TIM1UPDATECallback(void)
{
  BSP_LED_Toggle(LED_GREEN);
}

/*******************************************************************************
**功能描述 ：TIM3 同步触发模式配置函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_ConfigTIM3Trigger_Master(void)
{
  /*CK_INT 1分频*/
  LL_TIM_SetClockDivision(TIM3,LL_TIM_CLOCKDIVISION_DIV1);
  /*向上计数模式*/
  LL_TIM_SetCounterMode(TIM3,LL_TIM_COUNTERMODE_UP);
  /*重装载值1000*/
  LL_TIM_SetAutoReload(TIM3,1000-1);
  /*CK_CNT 预分频值：8000*/
  LL_TIM_SetPrescaler(TIM3,8000-1);
  /*设置触发输出为更新事件*/
  LL_TIM_SetTriggerOutput(TIM3,LL_TIM_TRGO_UPDATE);
  
  /*使能TIM3计数器*/
  LL_TIM_EnableCounter(TIM3);
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
