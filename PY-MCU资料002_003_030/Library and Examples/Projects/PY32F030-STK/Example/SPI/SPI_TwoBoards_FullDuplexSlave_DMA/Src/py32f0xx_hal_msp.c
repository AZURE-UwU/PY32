/**
  ******************************************************************************
  * @file    py32f0xx_hal_msp.c
  * @author  MCU Application Team
  * @Version V1.0.0
  * @Date    2020-10-19
  * @brief   This file provides code for the MSP Initialization
  *          and de-Initialization codes.
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "main.h"
uint8_t DmaOnFlag, DmaTxRxFlag;
static DMA_HandleTypeDef HdmaCh1;
static DMA_HandleTypeDef HdmaCh2;
static DMA_HandleTypeDef HdmaCh3;

/********************************************************************************************************
**函数信息 ：void HAL_MspInit(void)
**功能描述 ：初始化相关MSP
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_MspInit(void)
{
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);
}

/********************************************************************************************************
**函数信息 ：void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
**功能描述 ：初始化SPI相关MSP
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  //SPI1 初始化
  if (hspi->Instance == SPI1)
  {
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    __HAL_RCC_GPIOB_CLK_ENABLE();                   //GPIOB时钟使能
    __HAL_RCC_GPIOA_CLK_ENABLE();                   //GPIOA时钟使能
    __HAL_RCC_SYSCFG_CLK_ENABLE();                  //使能SYSCFG时钟
    __HAL_RCC_SPI1_CLK_ENABLE();                    //SPI1时钟使能
    __HAL_RCC_DMA_CLK_ENABLE();                     //DMA时钟使能
    HAL_SYSCFG_DMA_Req(1);                          //SPI1_TX DMA_CH1
    HAL_SYSCFG_DMA_Req(0x200);                      //SPI1_RX DMA_CH2
    /*
      PB3-SCK  (AF0)
      PB4-MISO(AF0)
      PB5-MOSI(AF0)
      PA15-NSS(AF0)
    */
    //SPI CS*/
    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    if (hspi->Init.CLKPolarity == SPI_POLARITY_LOW)
    {
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    }
    else
    {
      GPIO_InitStruct.Pull = GPIO_PULLUP;
    }
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
    /*GPIO配置为SPI：SCK/MISO/MOSI*/
    GPIO_InitStruct.Pin       = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    /*中断配置*/
    HAL_NVIC_SetPriority(SPI1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);
    /*DMA_CH1配置*/
    HdmaCh1.Instance                 = DMA1_Channel1;
    HdmaCh1.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    HdmaCh1.Init.PeriphInc           = DMA_PINC_DISABLE;
    HdmaCh1.Init.MemInc              = DMA_MINC_ENABLE;
    if (hspi->Init.DataSize <= SPI_DATASIZE_8BIT)
    {
      HdmaCh1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      HdmaCh1.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    }
    else
    {
      HdmaCh1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      HdmaCh1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    }
    HdmaCh1.Init.Mode                = DMA_NORMAL;
    HdmaCh1.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    /*DMA初始化*/
    HAL_DMA_Init(&HdmaCh1);
    /*DMA句柄关联到SPI句柄*/
    __HAL_LINKDMA(hspi, hdmatx, HdmaCh1);

    /*DMA_CH2配置*/
    HdmaCh2.Instance                 = DMA1_Channel2;
    HdmaCh2.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    HdmaCh2.Init.PeriphInc           = DMA_PINC_DISABLE;
    HdmaCh2.Init.MemInc              = DMA_MINC_ENABLE;
    if (hspi->Init.DataSize <= SPI_DATASIZE_8BIT)
    {
      HdmaCh2.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      HdmaCh2.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    }
    else
    {
      HdmaCh2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      HdmaCh2.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    }
    HdmaCh2.Init.Mode                = DMA_NORMAL;
    HdmaCh2.Init.Priority            = DMA_PRIORITY_LOW;
    /*DMA初始化*/
    HAL_DMA_Init(&HdmaCh2);
    /*DMA句柄关联到SPI句柄*/
    __HAL_LINKDMA(hspi, hdmarx, HdmaCh2);
    /*DMA中断设置*/
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
  }
  else if (hspi->Instance == SPI2)
  {
    __HAL_RCC_GPIOA_CLK_ENABLE();                   //GPIOA时钟使能
    __HAL_RCC_GPIOB_CLK_ENABLE();                   //GPIOB时钟使能
    __HAL_RCC_SYSCFG_CLK_ENABLE();                  //使能SYSCFG时钟
    __HAL_RCC_SPI2_CLK_ENABLE();                    //SPI2时钟使能
    HAL_SYSCFG_DMA_Req(3);                          //SPI2_TX DMA_CH1
    HAL_SYSCFG_DMA_Req(0x00);                       //SPI2_RX DMA_CH2
    /*GPIO配置为SPI：SCK/MISO/MOSI*/
    /*
      PA0-SCK  (AF0)
      PA3-MISO(AF0)
      PB7-MOSI(AF1)
      PB8-NSS  (AF1)
    */
    /*GPIO配置为SPI：SCK/MISO*/
    GPIO_InitStruct.Pin       = GPIO_PIN_0 | GPIO_PIN_3;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    if (hspi->Init.CLKPolarity == SPI_POLARITY_LOW)
    {
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    }
    else
    {
      GPIO_InitStruct.Pull = GPIO_PULLUP;
    }
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    /*GPIO配置为SPI：MOSI/NSS*/
    GPIO_InitStruct.Pin       = GPIO_PIN_7 | GPIO_PIN_8;
    GPIO_InitStruct.Alternate = GPIO_AF1_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(SPI2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPI2_IRQn);
    /*DMA_CH1配置*/
    HdmaCh1.Instance                 = DMA1_Channel1;
    HdmaCh1.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    HdmaCh1.Init.PeriphInc           = DMA_PINC_DISABLE;

    HdmaCh1.Init.MemInc              = DMA_MINC_ENABLE;
    if (hspi->Init.DataSize <= SPI_DATASIZE_8BIT)
    {
      HdmaCh1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      HdmaCh1.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    }
    else
    {
      HdmaCh1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      HdmaCh1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    }
    HdmaCh1.Init.Mode                = DMA_NORMAL;
    HdmaCh1.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    /*DMA初始化*/
    HAL_DMA_Init(&HdmaCh1);
    /*DMA句柄关联到SPI句柄*/
    __HAL_LINKDMA(hspi, hdmatx, HdmaCh1);

    /*DMA_CH2配置*/
    HdmaCh2.Instance                 = DMA1_Channel2;
    HdmaCh2.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    HdmaCh2.Init.PeriphInc           = DMA_PINC_DISABLE;
    HdmaCh2.Init.MemInc              = DMA_MINC_ENABLE;
    if (hspi->Init.DataSize <= SPI_DATASIZE_8BIT)
    {
      HdmaCh2.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      HdmaCh2.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    }
    else
    {
      HdmaCh2.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      HdmaCh2.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    }
    HdmaCh2.Init.Mode                = DMA_NORMAL;
    HdmaCh2.Init.Priority            = DMA_PRIORITY_HIGH;
    /*DMA初始化*/
    HAL_DMA_Init(&HdmaCh2);
    /*DMA句柄关联到SPI句柄*/
    __HAL_LINKDMA(hspi, hdmarx, HdmaCh2);
    /*DMA中断设置*/
    HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
  }
}


void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI1)
  {
    /*##-1- Reset peripherals ##################################################*/
    __HAL_RCC_SPI1_FORCE_RESET();
    __HAL_RCC_SPI1_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* Deconfigure SPI SCK */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);

    HAL_NVIC_DisableIRQ(SPI1_IRQn);

    HAL_DMA_DeInit(&HdmaCh1);
    HAL_NVIC_DisableIRQ(DMA1_Channel1_IRQn);
  }
}

