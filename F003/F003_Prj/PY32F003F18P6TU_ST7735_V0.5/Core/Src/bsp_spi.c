/**
  ******************************************************************************
  * @file    bsp_spi.c
  * @author  Bowen (wbw20)
  * @date    2026-08-29
  * @version V1.0
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   SPI1 初始化（LL 库，专供 ST7735S 显示屏使用）
  *
  * 设计说明：
  *   - 主机模式、8bit、CPOL=0/CPHA=1edge、软件 NSS（CS 脚手动控制）；
  *   - 波特率 = PCLK/2 = 12MHz，ST7735S 手册允许，刷屏速度与稳定性均衡；
  *   - ST7735S 只需要单向发送，无需 MISO。
  *
 * CHANGELOG:
 *   V1.0 (2026-08-19) 首次创建：移植自 G030 工程 LL_SPI 配置；
 *   V1.1 (2026-08-29) 移植 PY32F003F18P6TU：SCK=PA1、MOSI=PA2（AF0）。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "bsp_spi.h"
#include "py32f0xx_ll_spi.h"
#include "py32f0xx_ll_gpio.h"

/**
  * @brief  初始化 SPI1（PA1=SCK，PA2=MOSI）
  */
void BSP_SPI1_Init(void)
{
  LL_SPI_InitTypeDef SPI_InitStruct = {0};
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 外设与端口时钟 */
  __HAL_RCC_SPI1_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /* PA1 -> SPI1_SCK（AF0） */
  GPIO_InitStruct.Pin        = LL_GPIO_PIN_1;
  GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate  = LL_GPIO_AF_0;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* PA2 -> SPI1_MOSI（AF0） */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* SPI1：主机 / 8bit / 模式0 / 软件NSS / 12MHz */
  SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
  SPI_InitStruct.Mode              = LL_SPI_MODE_MASTER;
  SPI_InitStruct.DataWidth         = LL_SPI_DATAWIDTH_8BIT;
  SPI_InitStruct.ClockPolarity     = LL_SPI_POLARITY_LOW;
  SPI_InitStruct.ClockPhase        = LL_SPI_PHASE_1EDGE;
  SPI_InitStruct.NSS               = LL_SPI_NSS_SOFT;
  SPI_InitStruct.BaudRate          = LL_SPI_BAUDRATEPRESCALER_DIV2;
  SPI_InitStruct.BitOrder          = LL_SPI_MSB_FIRST;
  LL_SPI_Init(SPI1, &SPI_InitStruct);

  /* 打开 SPI，主循环/驱动直接使用 */
  LL_SPI_Enable(SPI1);
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
