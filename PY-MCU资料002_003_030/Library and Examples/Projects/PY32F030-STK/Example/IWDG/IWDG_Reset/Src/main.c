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
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);

/********************************************************************************************************
**函数信息 ：int main(void)
**功能描述 ：main函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  IWDG_HandleTypeDef   IwdgHandle;

  HAL_Init();

  /*##-3- Configure & Start the IWDG peripheral #########################################*/
  IwdgHandle.Instance = IWDG;                     //选择IWDG
  IwdgHandle.Init.Prescaler = IWDG_PRESCALER_32;  //配置32分频
  IwdgHandle.Init.Reload = (1000);                //IWDG计数器重装载值为1000，1s

  if (HAL_IWDG_Init(&IwdgHandle) != HAL_OK)        //初始化IWDG
  {

    Error_Handler();
  }

  while (1)
  {
    HAL_Delay(900);                           //每900ms喂一次狗，可以正常运行
//        HAL_Delay(1100);                            //每1.1s喂一次狗，无法正常运行
    /* 翻转LED灯 */
    BSP_LED_Toggle(LED_GREEN);                  //翻转LED
    /*喂狗*/
    if (HAL_IWDG_Refresh(&IwdgHandle) != HAL_OK)
    {
      Error_Handler();
    }
  }
}
/********************************************************************************************************
**函数信息 ：Error_Handler(void)
**功能描述 ：错误执行函数，进入后，关闭LED
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void Error_Handler(void)
{

  BSP_LED_Off(LED_GREEN);           //关闭LED

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



