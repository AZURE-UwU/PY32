#include "main.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef UartHandle;
uint8_t aTxBuffer[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
uint8_t aRxBuffer[12] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void USART_Config(void);
void Error_Handler(void);

/********************************************************************************************************
**函数信息 ：void main(void)
**功能描述 ：main函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  HAL_Init();            //systick初始化
  USART_Config();        //USART初始化
  /*通过中断方式发送数据*/
  if (HAL_UART_Transmit_IT(&UartHandle, (uint8_t *)aTxBuffer, 12) != HAL_OK)
  {
    Error_Handler();
  }

  while (1)
  {

  }
}
/********************************************************************************************************
**函数信息 ：void USART_Config(void)
**功能描述 ：USART初始化
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void USART_Config(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  //====================
  //USART2初始化
  //====================
  __HAL_RCC_USART2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  UartHandle.Instance          = USART2;
  UartHandle.Init.BaudRate     = 115200;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  HAL_UART_Init(&UartHandle);


  /**USART2 GPIO Configuration
    PA9     ------> USART1_TX
    PA10    ------> USART1_RX
    */
  GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* 使能NVIC */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}


/********************************************************************************************************
**函数信息 ：void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
**功能描述 ：USART错误回调执行函数，输出错误代码
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  printf("Uart Error, ErrorCode = %d\r\n", huart->ErrorCode);
}
/********************************************************************************************************
**函数信息 ：void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
**功能描述 ：USART发送回调执行函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  if (HAL_UART_Receive_IT(UartHandle, (uint8_t *)aRxBuffer, 12) != HAL_OK)
  {
    Error_Handler();
  }
}
/********************************************************************************************************
**函数信息 ：void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
**功能描述 ：USART接收回调执行函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  /*通过中断方式接收数据*/
  if (HAL_UART_Transmit_IT(UartHandle, (uint8_t *)aRxBuffer, 12) != HAL_OK)
  {
    Error_Handler();
  }

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

