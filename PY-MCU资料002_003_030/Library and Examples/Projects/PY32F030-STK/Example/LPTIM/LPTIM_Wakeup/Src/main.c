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
LPTIM_HandleTypeDef       LPTIMConf = {0};
/* Clocks structure declaration */
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);
static void APP_RCCOscConfig(void);
static void APP_LPTIMInit(void);
static void APP_LPTIMStart(void);
static void APP_delay_us(uint32_t nus);

/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{
  /*外设、systick初始化*/
  HAL_Init();
  
  /*时钟设置*/
  APP_RCCOscConfig();
  
  /*LPTIM初始化*/
  APP_LPTIMInit();

  /*点亮LED*/
  BSP_LED_On(LED_GREEN);
  
  /*等待按键按下*/
  while (BSP_PB_GetState(BUTTON_USER) != 0)
  {
  }

  /*关闭LED*/
  BSP_LED_Off(LED_GREEN);

  while (1)
  {
    /*Disable LPTIM*/
    __HAL_LPTIM_DISABLE(&LPTIMConf);
   
    /*使能LPTIM并开启中断*/
    APP_LPTIMStart();
    
    /*延时60us*/
    APP_delay_us(60);
    
    /*进入STOP模式，使用中断唤醒*/
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  }
}

/*******************************************************************************
**功能描述 ：时钟配置
**输入参数 ：
**输出参数 ：
*******************************************************************************/
static void APP_RCCOscConfig(void)
{
  RCC_OscInitTypeDef OSCINIT;
  RCC_PeriphCLKInitTypeDef LPTIM_RCC;

  /*LSI时钟配置*/
  /***********************************************
  ** 选择配置时钟：   LSI
  ** LSI状态：        开启
  ************************************************/
  OSCINIT.OscillatorType = RCC_OSCILLATORTYPE_LSI;
  OSCINIT.LSIState = RCC_LSI_ON;
  /*时钟初始化*/
  if (HAL_RCC_OscConfig(&OSCINIT) != HAL_OK)
  {
    Error_Handler();
  }
  
  /*LSI时钟配置*/
  /***********************************************
  ** 选择配置外设时钟：   LPTIM
  ** LPTIM时钟源：        LSI
  ************************************************/
  LPTIM_RCC.PeriphClockSelection = RCC_PERIPHCLK_LPTIM;
  LPTIM_RCC.LptimClockSelection = RCC_LPTIMCLKSOURCE_LSI;
  /*外设时钟初始化*/
  if (HAL_RCCEx_PeriphCLKConfig(&LPTIM_RCC) != HAL_OK)
  {
    Error_Handler();
  }
  
  /*使能LPTIM时钟*/
  __HAL_RCC_LPTIM_CLK_ENABLE();
}

/*******************************************************************************
**功能描述 ：初始化LPTIM
**输入参数 ：
**输出参数 ：
*******************************************************************************/
static void APP_LPTIMInit(void)
{
  /*LPTIM配置*/
  /***********************************************
  ** 选择外设实例：  LPTIM
  ** LPTIM预分频值： 128分频
  ** LPTIM更新模式： 立即更新
  ************************************************/
  LPTIMConf.Instance = LPTIM;
  LPTIMConf.Init.Prescaler = LPTIM_PRESCALER_DIV128;
  LPTIMConf.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  /*初始化LPTIM*/
  if (HAL_LPTIM_Init(&LPTIMConf) != HAL_OK)
  {
    Error_Handler();
  }
}

/*******************************************************************************
**功能描述 ：使能LPTIM和中断
**输入参数 ：
**输出参数 ：
*******************************************************************************/
static void APP_LPTIMStart(void)
{
  /*使能LPTIM并开启中断*/
  if (HAL_LPTIM_SetOnce_Start_IT(&LPTIMConf, 51) != HAL_OK)
  {
    Error_Handler();
  }
}

/*******************************************************************************
**功能描述 ：LPTIM重装载中断回调函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
  BSP_LED_Toggle(LED_GREEN);
}

/*******************************************************************************
**功能描述 ：微秒延时函数
**输入参数 ：nus：延时微秒值
**输出参数 ：
**备注：此函数会关闭SysTick中断，如需要使用请重新初始化SysTick
*******************************************************************************/
static void APP_delay_us(uint32_t nus)
 {
  HAL_SuspendTick();
  uint32_t temp;
  SysTick->LOAD=nus*(SystemCoreClock/1000000);
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
**功能描述 ：错误执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
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



