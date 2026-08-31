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

static DMA_HandleTypeDef HdmaCh1;
extern uint32_t   aADCxConvertedData;
/********************************************************************************************************
**函数信息 ：void HAL_MspInit(void)
**功能描述 ：初始化MSP
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_MspInit(void)
{
  BSP_LED_Init(LED_GREEN);
}

/********************************************************************************************************
**函数信息 ：void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
**功能描述 ：初始化ADC相关MSP
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
  GPIO_InitTypeDef          GPIO_InitStruct;
  __HAL_RCC_SYSCFG_CLK_ENABLE();                              //SYSCFG时钟使能
  __HAL_RCC_DMA_CLK_ENABLE();                                 //DMA时钟使能
  __HAL_RCC_GPIOA_CLK_ENABLE();                               //使能GPIOA时钟
  __HAL_RCC_ADC_CLK_ENABLE();                                 //使能ADC时钟

  //----------------
  //ADC通道配置PA0
  //----------------
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  HAL_SYSCFG_DMA_Req(0);                                      //DMA1_MAP选择为ADC
  //------------
  //DMA配置
  //------------
  HdmaCh1.Instance                 = DMA1_Channel1;           //选择DMA通道1
  HdmaCh1.Init.Direction           = DMA_PERIPH_TO_MEMORY;    //方向为从外设到存储器
  HdmaCh1.Init.PeriphInc           = DMA_PINC_DISABLE;        //禁止外设地址增量
  HdmaCh1.Init.MemInc              = DMA_MINC_DISABLE;         //使能存储器地址增量
  HdmaCh1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD; //外设数据宽度为8位
  HdmaCh1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD; //存储器数据宽度位8位
  HdmaCh1.Init.Mode                = DMA_CIRCULAR;            //循环模式
  HdmaCh1.Init.Priority            = DMA_PRIORITY_VERY_HIGH;  //通道优先级为很高

  HAL_DMA_DeInit(&HdmaCh1);                                   //DMA反初始化
  HAL_DMA_Init(&HdmaCh1);                                     //初始化DMA通道1
  __HAL_LINKDMA(hadc, DMA_Handle, HdmaCh1);                   //连接DMA句柄
}



