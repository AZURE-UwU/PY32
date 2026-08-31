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
extern DMA_HandleTypeDef HdmaCh1;
extern DMA_HandleTypeDef HdmaCh2;
extern DMA_HandleTypeDef HdmaCh3;

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
  __HAL_RCC_GPIOB_CLK_ENABLE();                   //GPIOB时钟使能
  __HAL_RCC_GPIOA_CLK_ENABLE();                   //GPIOA时钟使能
  __HAL_RCC_SYSCFG_CLK_ENABLE();                  //使能SYSCFG时钟
  __HAL_RCC_SPI1_CLK_ENABLE();                    //SPI1时钟使能
  __HAL_RCC_DMA_CLK_ENABLE();                     //DMA时钟使能
  HAL_SYSCFG_DMA_Req(1);                          //SPI1_TX DMA_CH1
  HAL_SYSCFG_DMA_Req(0x200);                      //SPI1_RX DMA_CH2
  /*
      PA2-SCK (AF10)
      PA0-MISO(AF10)
      PA1-MOSI(AF10)
      PA4-NSS (AF0)
  */
  /*SCK*/
  GPIO_InitStruct.Pin       = GPIO_PIN_2;
  if (hspi->Init.CLKPolarity == SPI_POLARITY_LOW)
  {
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  }
  else
  {
    GPIO_InitStruct.Pull = GPIO_PULLUP;
  }
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  //SPI NSS*/
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


  /*GPIO配置为SPI：MISO/MOSI*/
  GPIO_InitStruct.Pin       = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
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
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}

