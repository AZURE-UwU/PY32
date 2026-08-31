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

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);
static void APP_ExtiConfig(void);
static void APP_PvdConfig(void);

/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{
  /*时钟初始化,配置系统时钟为HSI*/
  APP_SystemClockConfig();

  /*配置PB5输出*/
  BSP_LED_Init(LED_GREEN);

  /*配置EXTI*/
  APP_ExtiConfig();
  
  /*配置PVD*/
  APP_PvdConfig();
  
  /*使能PVD*/
  LL_PWR_EnablePVD();
  
  while (1)
  {
  }
}

/*******************************************************************************
**功能描述 ：系统时钟配置函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
static void APP_SystemClockConfig(void)
{
  /* HSI使能及初始化 */
  LL_RCC_HSI_Enable();
  LL_RCC_HSI_SetCalibFreq(LL_RCC_HSICALIBRATION_24MHz);
  while (LL_RCC_HSI_IsReady() != 1)
  {
  }


  /* 设置AHB分频*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* 配置HSISYS为系统时钟及初始化 */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSISYS);
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSISYS)
  {
  }

  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  /*设置APB1分频及初始化*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_Init1msTick(24000000);
  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(24000000);
}

/*******************************************************************************
**功能描述 ：配置EXTI
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_ExtiConfig(void)
{
  /*GPIOB时钟使能*/
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);

  LL_GPIO_InitTypeDef GPIO_InitStruct;
  /*选择PB07引脚*/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  /*选择模拟模式*/
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  /*选择上拉*/
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  /*GPIOB初始化*/
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  LL_EXTI_InitTypeDef EXTI_InitStruct;
  /*选择EXTI16*/
  EXTI_InitStruct.Line = LL_EXTI_LINE_16;
  /*使能*/
  EXTI_InitStruct.LineCommand = ENABLE;
  /*选择中断模式*/
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
  /*选择上升/下降沿触发*/
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_RISING_FALLING;
  /*外部中断初始化*/
  LL_EXTI_Init(&EXTI_InitStruct);
}

/*******************************************************************************
**功能描述 ：配置PVD
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_PvdConfig(void)
{
  /*使能PWR时钟*/
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  
  /*PB07作为检测源，此参数设置无效*/
  LL_PWR_SetPVDLevel(LL_PWR_PVDLEVEL_0);
  
  /*滤波功能禁止*/
  LL_PWR_DisablePVDFilter();
  LL_PWR_SetPVDFilter(LL_PWR_PVD_FILTER_1CLOCK);
  
  /*PVD检测为PB07*/
  LL_PWR_SetPVDSource(LL_PWR_PVD_SOURCE_PB7);

  /*设置中断优先级*/
  NVIC_SetPriority(PVD_IRQn, 1);
  /*使能中断*/
  NVIC_EnableIRQ(PVD_IRQn);
}

/*******************************************************************************
**功能描述 ：错误执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void Error_Handler(void)
{

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
