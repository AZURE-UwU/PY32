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
WWDG_HandleTypeDef   WwdgHandle;
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
  /* 外设模块初始化 */
  HAL_Init();                                        //初始化systick

  /* WWDG模块初始化 */
  WwdgHandle.Instance = WWDG;                        //选择WWDG
  WwdgHandle.Init.Prescaler = WWDG_PRESCALER_8;       //选择8分频
  WwdgHandle.Init.Window    = 127;                   //7位窗口值为0x40~0x7f
  WwdgHandle.Init.Counter   = 127;                   //计数器值(7位)
  WwdgHandle.Init.EWIMode   = WWDG_EWI_ENABLE;       //使能提前唤醒中断
  if (HAL_WWDG_Init(&WwdgHandle) != HAL_OK)           //WWDG初始化
  {
    Error_Handler();
  }

  if (HAL_WWDG_Refresh(&WwdgHandle) != HAL_OK)       //喂狗
  {
    Error_Handler();
  }

  while (1)
  {
  }
}
/********************************************************************************************************
**函数信息 ：HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
**功能描述 ：提前唤醒中断执行函数，中断中喂狗，并翻转LED
**输入参数 ：WWDG_HandleTypeDef *hwwdg
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
{

  if (HAL_WWDG_Refresh(hwwdg) != HAL_OK)     //喂狗
  {
    Error_Handler();
  }

  BSP_LED_Toggle(LED_GREEN);                 //翻转LED灯
}
/********************************************************************************************************
**函数信息 ：Error_Handler(void)
**功能描述 ：错误执行函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
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



