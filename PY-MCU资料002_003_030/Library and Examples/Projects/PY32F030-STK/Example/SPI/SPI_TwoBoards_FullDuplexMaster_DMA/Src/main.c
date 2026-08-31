#include "main.h"


/* Private define ------------------------------------------------------------*/
#define DARA_LENGTH       15

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef Spi1Handle;
uint8_t TxBuff[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
uint8_t RxBuff[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
DMA_HandleTypeDef HdmaCh1;
DMA_HandleTypeDef HdmaCh2;
DMA_HandleTypeDef HdmaCh3;
static __IO uint32_t transferCompleteDetected = 0; /* 传输正确，则置1 */
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);
void SystemClock_Config(void);
void TransferComplete(DMA_HandleTypeDef *DmaHandle);
/********************************************************************************************************
**函数信息 ：void main(void)
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  //复位所有外设，初始化flash接口和systick.
  HAL_Init();                                                     //初始化systick
  SystemClock_Config();                                           //时钟配置
  /*初始化SPI配置*/
  Spi1Handle.Instance               = SPI1;                       //SPI1
  Spi1Handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;    //4分频
  Spi1Handle.Init.Direction         = SPI_DIRECTION_2LINES;       //全双工
  Spi1Handle.Init.CLKPolarity       = SPI_POLARITY_LOW;           //时钟极性低
  Spi1Handle.Init.CLKPhase          = SPI_PHASE_1EDGE ;           //数据采样从第一个时钟边沿开始
  Spi1Handle.Init.DataSize          = SPI_DATASIZE_8BIT;          //SPI数据长度为8bit
  Spi1Handle.Init.FirstBit          = SPI_FIRSTBIT_MSB;           //先发送MSB
  Spi1Handle.Init.NSS               = SPI_NSS_HARD_OUTPUT;        //NSS软件模式(硬件模式)
  Spi1Handle.Init.Mode = SPI_MODE_MASTER;                         //配置为主机
  if (HAL_SPI_DeInit(&Spi1Handle) != HAL_OK)                      //SPI反初始化
  {
    Error_Handler();
  }
  /*SPI初始化*/
  if (HAL_SPI_Init(&Spi1Handle) != HAL_OK)
  {
    Error_Handler();
  }
  while (BSP_PB_GetState(BUTTON_KEY) == 1)
  {
    ;
  }
  /*SPI DMA方式传输*/
  if (HAL_SPI_TransmitReceive_DMA(&Spi1Handle, (uint8_t *)TxBuff, (uint8_t *)RxBuff, DARA_LENGTH) != HAL_OK)
  {
    Error_Handler();
  }
  while (1)
  {

  }

}
/********************************************************************************************************
**函数信息 ：void DMA_CallbackConfig(void)
**功能描述 ：DMA回调函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void DMA_CallbackConfig(void)
{
  /*DMA正确传输后，调用的回调函数 */
  HAL_DMA_RegisterCallback(&HdmaCh2, HAL_DMA_XFER_CPLT_CB_ID, TransferComplete);
}
/********************************************************************************************************
**函数信息 ：static void TransferComplete(DMA_HandleTypeDef *DmaHandle)
**功能描述 ：完成传输后，完成传输标志位置1
**输入参数 ：DMA_HandleTypeDef *DmaHandle
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void TransferComplete(DMA_HandleTypeDef *DmaHandle)
{
  transferCompleteDetected = 1;
}
/********************************************************************************************************
**函数信息 ：void SystemClock_Config(void)
**功能描述 ：系统时钟配置
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /*配置时钟源HSE/HSI/LSE/LSI*/
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                      //开启HSI
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_4MHz;                            //配置HSI输出时钟为4MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_8MHz;                              //配置HSI输出时钟为8MHz
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz;                           //配置HSI输出时钟为16MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_22p12MHz;                        //配置HSI输出时钟为22.12MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;                             //配置HSI输出时钟为24MHz
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                                                      //HSI不分频
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                                                     //关闭HSE
  RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;                                                 //HSE工作频率范围16M~32M
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                                                     //关闭LSI
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                                                     //关闭LSE
  RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;                                            //LSE默认驱动能力
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;                                                  //关闭PLL

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)                                          //RCC振荡器初始化
  {
    Error_Handler();
  }

  //初始化CPU,AHB,APB总线时钟
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; //RCC系统时钟类型
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;                                        //SYSCLK的源选择为HSI
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                            //APH时钟不分频
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                             //APB时钟不分频

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)                       //初始化RCC系统时钟
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
