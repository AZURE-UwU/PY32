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
COMP_HandleTypeDef COMPINIT;
/* Private function prototypes -----------------------------------------------*/
void APP_RCC_INIT(void);
void APP_COMP_INIT(void);
void APP_COMP_IT(void);
void LED_RUN(void);
/* Private user code ---------------------------------------------------------*/

/********************************************************************************************************
**函数信息 ：void main(void)
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  //外设、systick初始化
  HAL_Init();
  /*关闭systick中断*/
  HAL_SuspendTick();
  /*时钟设置初始化*/
  APP_RCC_INIT();
  /*COMP初始化*/
  APP_COMP_INIT();
  /*中断使能*/
  APP_COMP_IT();
  /*COMP启动*/
  HAL_COMP_Start(&COMPINIT);

  BSP_LED_On(LED_GREEN);
  //等待按键按下
  while (BSP_PB_GetState(BUTTON_USER) != 0)
  {
  }
  BSP_LED_Off(LED_GREEN);
  //进入STOP模式
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  /*恢复systick中断*/
  HAL_ResumeTick();
  HAL_Delay(1000);
  while (1)
  {
    LED_RUN();
  }
}
/********************************************************************************************************
**函数信息 ：void APP_RCC_INIT(void)
**功能描述 ：RCC初始化
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void APP_RCC_INIT(void)
{
  RCC_OscInitTypeDef RCCCONF;
  RCC_PeriphCLKInitTypeDef COMPRCC;

  RCCCONF.OscillatorType = RCC_OSCILLATORTYPE_LSI;        //RCC使用内部LSI
  RCCCONF.LSIState = RCC_LSI_ON;                          //开启LSI

  COMPRCC.PeriphClockSelection = RCC_PERIPHCLK_COMP1;     //RCC扩展外设时钟为RTC
  COMPRCC.Comp1ClockSelection = RCC_COMP1CLKSOURCE_LSC;   //外设独立时钟源选择LSC

  HAL_RCC_OscConfig(&RCCCONF);                            //时钟初始化
  HAL_RCCEx_PeriphCLKConfig(&COMPRCC);                    //RCC扩展外设时钟初始化
}
/********************************************************************************************************
**函数信息 ：void APP_COMP_INIT(void)
**功能描述 ：COMP1初始化
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void APP_COMP_INIT(void)
{
  __HAL_RCC_COMP1_CLK_ENABLE();                       //使能COMP1时钟
  COMP_InitTypeDef COMPCONF = {0};
  COMPINIT.Instance = COMP1;                          //COMP1
  COMPCONF.Mode = COMP_POWERMODE_HIGHSPEED;           //COMP1功耗选择为High speed
  COMPCONF.InputPlus = COMP_INPUT_PLUS_IO3;           //正极引脚为PA1
  COMPCONF.InputMinus = COMP_INPUT_MINUS_VREFINT;     //负极选择为VREFINT
  COMPCONF.TriggerMode = COMP_TRIGGERMODE_IT_FALLING; //触发方式为下降沿中断触发
  COMPCONF.Hysteresis = COMP_HYSTERESIS_ENABLE;       //迟滞功能开启
  COMPINIT.Init = COMPCONF;

  HAL_COMP_Init(&COMPINIT);
}
/********************************************************************************************************
**函数信息 ：void APP_COMP_IT(void)
**功能描述 ：COMP中断使能
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void APP_COMP_IT(void)
{
  /*COMP中断使能*/
  HAL_NVIC_EnableIRQ(ADC_COMP_IRQn);
  HAL_NVIC_SetPriority(ADC_COMP_IRQn, 0x01, 0);
}
/********************************************************************************************************
**函数信息 ：void LED_RUN(void)
**功能描述 ：LED每隔200ms翻转一次
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void LED_RUN(void)
{
  BSP_LED_Toggle(LED_GREEN);
  HAL_Delay(200);
  BSP_LED_Toggle(LED_GREEN);
  HAL_Delay(200);
}
/********************************************************************************************************
**函数信息 ：void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp)
**功能描述 ：LED点亮
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp)
{
  BSP_LED_On(LED_GREEN);
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
