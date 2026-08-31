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

#define DEF_IOMUX2

static DMA_HandleTypeDef HdmaCh1;
static DMA_HandleTypeDef HdmaCh2;
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
**函数信息 ：void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
**功能描述 ：初始化I2C相关MSP
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_SYSCFG_CLK_ENABLE();                              //SYSCFG时钟使能
  __HAL_RCC_DMA_CLK_ENABLE();                                 //DMA时钟使能
  __HAL_RCC_I2C_CLK_ENABLE();                                 //I2C时钟使能
  __HAL_RCC_GPIOA_CLK_ENABLE();                               //GPIOA时钟使能

  /**I2C GPIO Configuration
  PA9     ------> I2C1_SCL
  PA10     ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;                     //开漏
  GPIO_InitStruct.Pull = GPIO_PULLUP;                         //上拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF6_I2C;                   //复用为I2C
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                     //GPIO初始化
  /*复位I2C*/
  __HAL_RCC_I2C_FORCE_RESET();
  __HAL_RCC_I2C_RELEASE_RESET();

  /* I2C1 interrupt Init */
  HAL_NVIC_SetPriority(I2C1_IRQn, 0, 0);                     //中断优先级设置
  HAL_NVIC_EnableIRQ(I2C1_IRQn);                              //使能I2C中断

  //DMA
  //DMA配置
  HAL_SYSCFG_DMA_Req(9);                                      //DMA1_MAP选择为IIC_TX
  HAL_SYSCFG_DMA_Req(0xA00);                                  //DMA2_MAP选择为IIC_RX

  /*##-3- Configure the DMA ##################################################*/
  /* Configure the DMA handler for Transmission process */
  HdmaCh1.Instance                 = DMA1_Channel1;           //选择DMA通道1
  HdmaCh1.Init.Direction           = DMA_MEMORY_TO_PERIPH;    //方向为从存储器到外设
  HdmaCh1.Init.PeriphInc           = DMA_PINC_DISABLE;        //禁止外设地址增量
  HdmaCh1.Init.MemInc              = DMA_MINC_ENABLE;         //使能存储器地址增量
  HdmaCh1.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;     //外设数据宽度为8位
  HdmaCh1.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;     //存储器数据宽度位8位
  HdmaCh1.Init.Mode                = DMA_NORMAL;              //禁止循环模式
  HdmaCh1.Init.Priority            = DMA_PRIORITY_VERY_HIGH;  //通道优先级为很高

  HAL_DMA_Init(&HdmaCh1);                                     //初始化DMA通道1
  __HAL_LINKDMA(hi2c, hdmatx, HdmaCh1);                       //DMA1关联IIC_TX

  /* Configure the DMA handler for Transmission process */
  HdmaCh2.Instance                 = DMA1_Channel2;           //选择DMA通道1
  HdmaCh2.Init.Direction           = DMA_PERIPH_TO_MEMORY;    //方向为从外设到存储
  HdmaCh2.Init.PeriphInc           = DMA_PINC_DISABLE;        //禁止外设地址增量
  HdmaCh2.Init.MemInc              = DMA_MINC_ENABLE;         //使能存储器地址增量
  HdmaCh2.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;     //外设数据宽度为8位
  HdmaCh2.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;     //存储器数据宽度位8位
  HdmaCh2.Init.Mode                = DMA_NORMAL;              //禁止循环模式
  HdmaCh2.Init.Priority            = DMA_PRIORITY_HIGH;       //通道优先级为高

  HAL_DMA_Init(&HdmaCh2);                                     //初始化DMA通道1
  __HAL_LINKDMA(hi2c, hdmarx, HdmaCh2);                       //DMA1关联IIC_RX

  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 1);             //中断优先级设置
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);                     //使能DMA通道1中断

  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1, 1);           //中断优先级设置
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);                   ////使能DMA通道2_3中断
}
