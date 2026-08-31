/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @Version V1.0.0
  * @Date    2021-9-22
  * @brief   main function
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
EXTI_HandleTypeDef exti_handle;
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Configure_EXTI(void);

/********************************************************************************************************
**函数信息 ：void main(void)
**功能描述 ：main函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  HAL_Init();                                   //初始化systick
  Configure_EXTI();                             //配置外部中断
  DEBUG_USART_Config();                         //UART配置
  __HAL_RCC_PWR_CLK_ENABLE();                   //PWR时钟使能
  printf("STOP MODE!\n\n");
  HAL_SuspendTick();                            //systick中断关闭
  /*进入STOP模式*/
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  printf("WAKEUP OK!\n\n");
  while (1)
  {

  }
}

/********************************************************************************************************
**函数信息 ：void Configure_EXTI(void)
**功能描述 ：配置外部中断引脚
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void Configure_EXTI(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  __HAL_RCC_GPIOA_CLK_ENABLE();                  //使能GPIOA时钟
  GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;  //GPIO模式为下降沿中断
  GPIO_InitStruct.Pull  = GPIO_PULLUP;           //上拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  //速度为高速
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);             //使能EXTI中断
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);     //配置中断优先级
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



