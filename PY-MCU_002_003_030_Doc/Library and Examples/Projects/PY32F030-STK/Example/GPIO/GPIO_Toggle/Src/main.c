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
void LED_Init(void);
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
  HAL_Init();                                  //初始化systick
  LED_Init();                                  //LED初始化
  while (1)
  {
    HAL_Delay(250);                          //延时250ms
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_11); //LED翻转
  }
}
/********************************************************************************************************
**函数信息 ：LED_Init(void)
**功能描述 ：LED初始化(PA11)
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void LED_Init()
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  __HAL_RCC_GPIOA_CLK_ENABLE();                          //GPIOA时钟使能

  //初始化GPIOA11
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;            //推挽输出
  GPIO_InitStruct.Pull = GPIO_PULLUP;                    //使能上拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;          //GPIO速度

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                //GPIO初始化
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



