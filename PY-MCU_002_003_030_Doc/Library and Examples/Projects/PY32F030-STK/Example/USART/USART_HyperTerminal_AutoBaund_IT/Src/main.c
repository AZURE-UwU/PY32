#include "main.h"

/* Private define ------------------------------------------------------------*/
#define AUTO_BAUD_MODE0    //自动波特率检测模式选择，从start位开始测量；若屏蔽选择下降沿到下降沿测量
#define RXBUFFERSIZE  1
/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef UartHandle;
uint8_t aTxBuffer[] = "Auto BaudRate Test";
uint8_t aRxBuffer[RXBUFFERSIZE];
uint8_t submitsum = 0;
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
  /*通过中断方式接收数据*/
  if (HAL_UART_Receive_IT(&UartHandle, (uint8_t *)aRxBuffer, 1) != HAL_OK)
  {
    Error_Handler();
  }
  while (1)
  {
    if ((__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_ABRE) == RESET) && \
      (__HAL_UART_GET_FLAG(&UartHandle, UART_FLAG_ABRF) == SET))
    {
      // 清除自动波特率检测完成标志位
      __HAL_UART_SEND_AUTOBAUD_REQ(&UartHandle);
      // 发送数据
      HAL_UART_Transmit_IT(&UartHandle, (uint8_t *)aTxBuffer, 18);
    }
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

  UartHandle.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_AUTOBAUDRATE_INIT;
  UartHandle.AdvancedInit.AutoBaudRateEnable = UART_ADVFEATURE_AUTOBAUDRATE_ENABLE; //自动波特率使能
#ifdef AUTO_BAUD_MODE0
  UartHandle.AdvancedInit.AutoBaudRateMode = UART_ADVFEATURE_AUTOBAUDRATE_ONSTARTBIT; //自动波特率检测模式从start位开始测量波特率，上位机发送0x7f即可
#else
  UartHandle.AdvancedInit.AutoBaudRateMode = UART_ADVFEATURE_AUTOBAUDRATE_ONFALLINGEDGE; //自动波特率检测模式下降沿到下降沿测量，上位机发送0x55即可
#endif
  if (HAL_UART_DeInit(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UART_Init(&UartHandle) != HAL_OK)
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
