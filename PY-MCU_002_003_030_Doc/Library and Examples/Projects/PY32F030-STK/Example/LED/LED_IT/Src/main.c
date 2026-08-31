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
const uint8_t dispArr[] = {LED_DISP_0, LED_DISP_1, LED_DISP_2, LED_DISP_3, LED_DISP_4, \
                           LED_DISP_5, LED_DISP_6, LED_DISP_7, LED_DISP_8, LED_DISP_9
                          };
uint8_t dispNum = 0;
LED_HandleTypeDef hled;
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/

/********************************************************************************************************
**函数信息 ：int main(void)
**功能描述 ：主函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  HAL_Init();
  //LED初始化
  hled.Instance = LED;
  hled.Init.ComNum = 4 - 1;               /* 4个COM口均打开 */
  hled.Init.ComDrive = LED_COMDRIVE_LOW;  /* LED 非COM输出 */
  hled.Init.Prescaler = 10 - 1;           /* Fpclk/(PR+1) */
  hled.Init.LightTime = 0xF0;             /* 每个LED被点亮的时间  */
  hled.Init.DeadTime = 0x10;              /* 两个LED切换的间歇时间,间歇时间不能为零 */
  HAL_LED_Init(&hled);

  while (1)
  {
    dispNum++;
    if (dispNum == 10)
    {
      dispNum = 0;
    }
    HAL_Delay(1000);
  }
}
/********************************************************************************************************
**函数信息 ：void HAL_LED_LightComplateCallback(LED_HandleTypeDef *hled)
**功能描述 ：LED中断回调执行函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_LED_LightCpltCallback(LED_HandleTypeDef *hled)
{
  static uint32_t oldValue = 0xFF;

  if (oldValue != dispNum)
  {
    oldValue = dispNum;
    HAL_LED_SetComDisplay(hled, LED_COM0, dispArr[(dispNum) % 10]);
    HAL_LED_SetComDisplay(hled, LED_COM1, dispArr[(dispNum + 1) % 10]);
    HAL_LED_SetComDisplay(hled, LED_COM2, dispArr[(dispNum + 2) % 10]);
    HAL_LED_SetComDisplay(hled, LED_COM3, dispArr[(dispNum + 3) % 10]);
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



