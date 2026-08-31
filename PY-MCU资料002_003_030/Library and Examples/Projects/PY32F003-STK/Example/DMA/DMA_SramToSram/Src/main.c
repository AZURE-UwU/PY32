/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
DMA_HandleTypeDef     DmaHandle;
uint32_t aSRC_Const_Buffer[BUFFER_SIZE];           /* 数据传输源buffer */
uint32_t aDST_Buffer[BUFFER_SIZE];                 /* 数据传输目标buffer */

static __IO uint32_t transferErrorDetected = 0 ;    /* 检测到错误传输，则置为1 */
static __IO uint32_t transferCompleteDetected = 0;   /* 传输正确，则置1 */
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void DMA_Config(void);
static void TransferComplete(DMA_HandleTypeDef *DmaHandle);
static void TransferError(DMA_HandleTypeDef *DmaHandle);

/********************************************************************************************************
**函数信息 ：void main(void)
**功能描述 ：main函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  // 复位所有外设，初始化flash和systick
  HAL_Init();

  // 给DMA 源buffer初始化数据
  for (uint8_t i = 0; i < BUFFER_SIZE; i++)
  {
    aSRC_Const_Buffer[i] = i;
  }

  // DMA模块配置
  DMA_Config();

  // LED灯初始状态为常亮
  BSP_LED_On(LED_GREEN);

  /*使能DMA，并使能DMA中断*/
  if (HAL_DMA_Start_IT(&DmaHandle, (uint32_t)&aSRC_Const_Buffer, (uint32_t)&aDST_Buffer, BUFFER_SIZE) != HAL_OK)
  {
    /* Transfer Error */
    Error_Handler();
  }

  while (1)
  {
    if (transferErrorDetected == 1)           //如果产生传输错误，LED灯熄灭
    {
      BSP_LED_Off(LED_GREEN);
      transferErrorDetected = 0;            //清除错误标志位
    }
    else if (transferCompleteDetected == 1)   //如果传输完成，则LED灯闪烁
    {
      HAL_Delay(100);
      BSP_LED_Toggle(LED_GREEN);
    }
  }
}

/********************************************************************************************************
**函数信息 ：static void DMA_Config(void)
**功能描述 ：DMA配置
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
static void DMA_Config(void)
{
  /*使能DMA时钟*/
  __HAL_RCC_DMA_CLK_ENABLE();

  //
  DmaHandle.Init.Direction = DMA_MEMORY_TO_MEMORY;          /* M2M 模式                */
  DmaHandle.Init.PeriphInc = DMA_PINC_ENABLE;               /* 外设地址增量模式使能 */
  DmaHandle.Init.MemInc = DMA_MINC_ENABLE;                  /* 存储器地址增量模式使能     */
  DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD; /* 外设数据宽度为32位 */
  DmaHandle.Init.MemDataAlignment = DMA_PDATAALIGN_WORD;    /* 存储器数据宽度为32位  */
  DmaHandle.Init.Mode = DMA_NORMAL;                         /* DMA循环模式关闭                  */
  DmaHandle.Init.Priority = DMA_PRIORITY_HIGH;              /* 通道优先级为高            */

  /*选择DMA通道1 #*/
  DmaHandle.Instance = DMA1_Channel1;

  /*DMA初始化*/
  if (HAL_DMA_Init(&DmaHandle) != HAL_OK)
  {
    Error_Handler();
  }

  /*选择错误传输和正确传输后，调用的回调函数 */
  HAL_DMA_RegisterCallback(&DmaHandle, HAL_DMA_XFER_CPLT_CB_ID, TransferComplete);
  HAL_DMA_RegisterCallback(&DmaHandle, HAL_DMA_XFER_ERROR_CB_ID, TransferError);
  /* DMA通道1中断使能 */
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

}
/********************************************************************************************************
**函数信息 ：static void TransferComplete(DMA_HandleTypeDef *DmaHandle)
**功能描述 ：完成传输后，完成传输标志位置1
**输入参数 ：DMA_HandleTypeDef *DmaHandle
**输出参数 ：
**    备注 ：
********************************************************************************************************/
static void TransferComplete(DMA_HandleTypeDef *DmaHandle)
{
  transferCompleteDetected = 1;
}

/********************************************************************************************************
**函数信息 ：static void TransferError(DMA_HandleTypeDef *DmaHandle)
**功能描述 ：错误传输后，传输错误标志位置1
**输入参数 ：DMA_HandleTypeDef *DmaHandle
**输出参数 ：
**    备注 ：
********************************************************************************************************/
static void TransferError(DMA_HandleTypeDef *DmaHandle)
{
  transferErrorDetected = 1;
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
