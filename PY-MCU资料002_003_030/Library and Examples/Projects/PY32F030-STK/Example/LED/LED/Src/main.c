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

LED_HandleTypeDef hled;

/* Private variables ---------------------------------------------------------*/
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
  hled.Init.ComNum = 4 - 1;                                 /* 开启4个COM口 */
  hled.Init.ComDrive  = LED_COMDRIVE_HIGH;                  /* LED COM输出 */
  hled.Init.Prescaler = 10 - 1;                             /* Fpclk/(PR+1) */
  hled.Init.LightTime = 240;                                /* 每个LED被点亮的时间 240*fLED(fLED为LED模块计数时钟频率) */
  hled.Init.DeadTime = 10;                                  /* 两个LED切换的间歇时间,间歇时间不能为零 */
  HAL_LED_Init(&hled);

  while (1)
  {
    HAL_LED_SetComDisplay(&hled, LED_COM0, LED_DISP_8);       /*显示数字8*/
    HAL_Delay(200);

    HAL_LED_SetComDisplay(&hled, LED_COM1, LED_DISP_8);       /*显示数字8*/
    HAL_Delay(200);

    HAL_LED_SetComDisplay(&hled, LED_COM2, LED_DISP_8);       /*显示数字8*/
    HAL_Delay(200);

    HAL_LED_SetComDisplay(&hled, LED_COM3, LED_DISP_8);       /*显示数字8*/
    HAL_Delay(200);

    HAL_LED_SetComDisplay(&hled, LED_COM_ALL, LED_DISP_NONE); /*关闭显示*/
    HAL_Delay(200);
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



