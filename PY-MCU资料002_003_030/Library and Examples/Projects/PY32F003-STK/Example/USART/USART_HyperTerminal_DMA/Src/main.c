#include "main.h"

/* Private define ------------------------------------------------------------*/
#ifdef __GNUC__
  /* With GCC, small printf (option LD Linker->Libraries->Small printf
  set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef UartHandle;
uint8_t aTxBuffer[12] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
uint8_t aRxBuffer[12] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;
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
  while (1)
  {
    /* 通过DMA方式接收数据 */
    if (HAL_UART_Receive_DMA(&UartHandle, (uint8_t *)aRxBuffer, 12) != HAL_OK)
    {
      Error_Handler();
    }
    /* 等待接收数据完成 */
    while(HAL_UART_GetState(&UartHandle) != HAL_UART_STATE_READY)
    {}

    /* 通过DMA方式发送数据 */
    if (HAL_UART_Transmit_DMA(&UartHandle, (uint8_t *)aRxBuffer, 12) != HAL_OK)
    {
      Error_Handler();
    }
    /* 等待发送数据完成 */
    while(HAL_UART_GetState(&UartHandle) != HAL_UART_STATE_READY)
    {}
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
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_DMA_CLK_ENABLE();
  UartHandle.Instance          = USART2;
  UartHandle.Init.BaudRate     = 115200;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;

  if (HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }
  /**USART2 GPIO Configuration
  PA0     ------> USART2_TX
  PA1     ------> USART2_RX
  */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  //====================
  //USART2 DMA相关配置
  //====================
  /* USART2_TX 初始化 */
  hdma_usart2_tx.Instance = DMA1_Channel1;
  hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart2_tx.Init.Mode = DMA_NORMAL;
  hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_LINKDMA(&UartHandle, hdmatx, hdma_usart2_tx);
  /* USART2_RX 初始化 */
  hdma_usart2_rx.Instance = DMA1_Channel2;
  hdma_usart2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_usart2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_usart2_rx.Init.MemInc = DMA_MINC_ENABLE;
  hdma_usart2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_usart2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_usart2_rx.Init.Mode = DMA_NORMAL;
  hdma_usart2_rx.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_usart2_rx) != HAL_OK)
  {
    Error_Handler();
  }
  __HAL_LINKDMA(&UartHandle, hdmarx, hdma_usart2_rx);
  /*UART2_TX DMA_CH1-0x7  ; UART2_RX DMA_CH2-0x8*/
  HAL_SYSCFG_DMA_Req(0x0807);
  /* 使能NVIC */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1, 1);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

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


