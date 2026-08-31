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
static void APP_ConfigLPTIM_OneShot(void);
void APP_LPTIMCallback(void);
static void APP_uDelay(uint32_t Delay);
static void APP_LPTIMClockconf(void);

/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{  
  /*配置系统时钟*/
  SystemClock_Config();

  /*使能LPTIM、PWR时钟*/
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_LPTIM1);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  
  /*初始化LED、按键*/  
  BSP_LED_Init(LED3);
  BSP_PB_Init(BUTTON_USER,BUTTON_MODE_GPIO);
  
  /*配置LPTIM时钟源为LSI*/
  APP_LPTIMClockconf();
  
  /*配置并使能LPTIM*/
  APP_ConfigLPTIM_OneShot();
  
  /*点亮LED*/
  BSP_LED_On(LED3);
  
  /*等待按键按下*/
  while(BSP_PB_GetState(BUTTON_USER) != 0)
  {}
  
  /*关闭LED*/
  BSP_LED_Off(LED3);
    
  while (1)
  {
    /*使能低功耗运行模式*/
    LL_PWR_EnableLowPowerRunMode();
    
    /*失能LPTIM*/
    LL_LPTIM_Disable(LPTIM1);
    /*使能LPTIM*/
    LL_LPTIM_Enable(LPTIM1);
    
    /*延时60us*/
    APP_uDelay(60);

    /*开启单次模式*/
    LL_LPTIM_StartCounter(LPTIM1,LL_LPTIM_OPERATING_MODE_ONESHOT);

    /*使能STOP模式*/
    LL_LPM_EnableDeepSleep();
    
    /*进入STOP模式，等待中断唤醒*/
    __WFI();
  }
}

/*******************************************************************************
**功能描述 ：LPTIM 时钟配置
**输入参数 ：
**输出参数 ：
*******************************************************************************/
static void APP_LPTIMClockconf(void)
{
  /*开启LSI*/
  LL_RCC_LSI_Enable();
  
  /*等待LSI就绪*/
  while(LL_RCC_LSI_IsReady() == 0)
  {}
    
  /*配置LSI为LPTIM时钟源*/
  LL_RCC_SetLPTIMClockSource(LL_RCC_LPTIM1_CLKSOURCE_LSI);
}

/*******************************************************************************
**功能描述 ：配置LPTIM单次模式
**输入参数 ：
**输出参数 ：
*******************************************************************************/
static void APP_ConfigLPTIM_OneShot(void)
{
  /*配置LPTIM*/
  /*LPTIM预分频器128分频*/
  LL_LPTIM_SetPrescaler(LPTIM1,LL_LPTIM_PRESCALER_DIV128);
  
  /*LPTIM计数周期结束更新ARR*/
  LL_LPTIM_SetUpdateMode(LPTIM1,LL_LPTIM_UPDATE_MODE_ENDOFPERIOD);
  
  /*使能ARR中断*/
  LL_LPTIM_EnableIT_ARRM(LPTIM1);
  
  /*使能NVIC请求*/
  NVIC_EnableIRQ(LPTIM1_IRQn);
  NVIC_SetPriority(LPTIM1_IRQn,0);
  
  /*使能LPTIM*/
  LL_LPTIM_Enable(LPTIM1);
  
  /*配置重装载值：51*/
  LL_LPTIM_SetAutoReload(LPTIM1,51);
}

/*******************************************************************************
**功能描述 ：LPTIM ARR中断回调函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_LPTIMCallback(void)
{
  /*翻转LED*/
  BSP_LED_Toggle(LED3);
}

/*******************************************************************************
**功能描述 ：微秒延时函数
**输入参数 ：Delay：延时微秒值
**输出参数 ：
*******************************************************************************/
static void APP_uDelay(uint32_t Delay)
{
  uint32_t temp;
  SysTick->LOAD=Delay*(SystemCoreClock/1000000);
  SysTick->VAL=0x00;
  SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;
  do
  {
    temp=SysTick->CTRL;
  }
  while((temp&0x01)&&!(temp&(1<<16)));
  SysTick->CTRL=0x00;
  SysTick->VAL =0x00;
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
