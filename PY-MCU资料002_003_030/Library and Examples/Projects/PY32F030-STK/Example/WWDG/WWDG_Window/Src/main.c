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

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static WWDG_HandleTypeDef   WwdgHandle;
static uint32_t TimeoutCalculation(uint32_t timevalue);

/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/********************************************************************************************************
**函数信息 ：int main(void)
**功能描述 ：main函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  uint32_t delay;

  /* 外设模块初始化 */
  HAL_Init();                                                                             //初始化systick

  /* WWDG模块初始化 */
  WwdgHandle.Instance = WWDG;                                                             //选择WWDG
  WwdgHandle.Init.Prescaler = WWDG_PRESCALER_8;                                           //选择8分频
  WwdgHandle.Init.Window    = 0x50;                                                       //7位窗口值为0x40~0x7f
  WwdgHandle.Init.Counter   = 0x7F;                                                       //计数器值(7位)
  WwdgHandle.Init.EWIMode   = WWDG_EWI_DISABLE;                                           //关闭提前唤醒中断
  if (HAL_WWDG_Init(&WwdgHandle) != HAL_OK)                                               //WWDG初始化
  {
    Error_Handler();
  }
  /* 计算进入窗口内的延时 */
  delay = TimeoutCalculation((WwdgHandle.Init.Counter - WwdgHandle.Init.Window) + 1) + 1;
  /* 无限循环 */
  while (1)
  {
    /* 翻转LED灯 */
    BSP_LED_Toggle(LED_GREEN);

    /* 插入上面计算好的延时 */
    HAL_Delay(delay);

    /* 更新看门狗计数器 */
    if (HAL_WWDG_Refresh(&WwdgHandle) != HAL_OK)
    {
      Error_Handler();
    }
  }
}
/********************************************************************************************************
**函数信息 ：static uint32_t TimeoutCalculation(uint32_t timevalue)
**功能描述 ：计算进入窗口内的延时
**输入参数 ：
**输出参数 ：
**    备注 ：WWDG超时公式tWWDG=tPCLK*4096*分频*(T[5:0]+1)
********************************************************************************************************/
static uint32_t TimeoutCalculation(uint32_t timevalue)
{
  uint32_t timeoutvalue = 0;
  uint32_t pclk1 = 0;
  uint32_t wdgtb = 0;
  /* 获取PCLK的值 */
  pclk1 = HAL_RCC_GetPCLK1Freq();
  /* 获取分频值 */
  wdgtb = (1 << ((WwdgHandle.Init.Prescaler) >> 7)); /* 2^WDGTB[1:0] */
  /* 计算超时时间 */
  timeoutvalue = ((4096 * wdgtb * timevalue) / (pclk1 / 1000));
  return timeoutvalue;
}
/********************************************************************************************************
**函数信息 ：Error_Handler(void)
**功能描述 ：错误执行函数，进入后LED关闭
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void Error_Handler(void)
{
  BSP_LED_Off(LED_GREEN);//关闭LED

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



