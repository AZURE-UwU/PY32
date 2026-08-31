/**
  ******************************************************************************
  * @file    bsp_adc.c
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   ADC 初始化与两通道读取
  *
  * 设计说明：
  *   - PY32F002B 没有 DMA，G030 工程里的 DMA 循环采样改为"扫描 + 轮询"：
  *     每次读取启动一次序列转换，等 EOC 依次取出 IN0、IN2；
  *   - 采样时间取 239.5 周期：NTC/VCC 分压电阻源阻抗高，需要长采样时间；
  *   - 转换一次读一次，50ms 任务里调用，实时性足够。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 首次创建：去 DMA，IN8/IN11 改为 IN0/IN2。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "bsp_adc.h"

ADC_HandleTypeDef hadc1;

/**
  * @brief  初始化 ADC1：IN0(PB1)=VCC，IN2(PA4)=NTC，12bit 扫描序列
  */
void BSP_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance                   = ADC1;
  hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;  /* 24MHz/4=6MHz */
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode          = ADC_SCAN_ENABLE;           /* 顺序扫描已选通道 */
  hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;       /* 每个通道完成置 EOC */
  hadc1.Init.LowPowerAutoWait      = DISABLE;
  hadc1.Init.ContinuousConvMode    = DISABLE;                   /* 单次序列，读完再启动 */
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.SamplingTimeCommon    = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* 通道 0：VCC 分压（PB1） */
  sConfig.Channel      = ADC_CHANNEL_0;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* 通道 2：NTC 分压（PA4） */
  sConfig.Channel = ADC_CHANNEL_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

/**
  * @brief  读取两路 ADC（扫描轮询，无 DMA）
  * @param  vcc: 输出 IN0 原始值（0~4095）
  * @param  ntc: 输出 IN2 原始值（0~4095）
  * @retval 无；读取失败时返回 0xFFFF 哨兵值
  */
void BSP_ADC_ReadAll(uint16_t *vcc, uint16_t *ntc)
{
  /* 防御：入口指针校验 */
  if ((vcc == NULL) || (ntc == NULL))
  {
    return;
  }

  *vcc = 0xFFFFU;
  *ntc = 0xFFFFU;

  /* 启动一次扫描序列：先 IN0 后 IN2（方向 forward） */
  if (HAL_ADC_Start(&hadc1) != HAL_OK)
  {
    return;
  }

  if (HAL_ADC_PollForConversion(&hadc1, 10U) != HAL_OK)
  {
    HAL_ADC_Stop(&hadc1);
    return;
  }
  *vcc = (uint16_t)HAL_ADC_GetValue(&hadc1);

  if (HAL_ADC_PollForConversion(&hadc1, 10U) != HAL_OK)
  {
    HAL_ADC_Stop(&hadc1);
    return;
  }
  *ntc = (uint16_t)HAL_ADC_GetValue(&hadc1);

  HAL_ADC_Stop(&hadc1);
}

/**
  * @brief  ADC 底层初始化（HAL_ADC_Init 内部调用）
  */
void HAL_ADC_MspInit(ADC_HandleTypeDef *adcHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (adcHandle->Instance == ADC1)
  {
    __HAL_RCC_ADC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* ADC 引脚必须配置为模拟输入，且不上拉下拉 */
    GPIO_InitStruct.Pin  = ADC_NTC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ADC_NTC_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ADC_VCC_Pin;
    HAL_GPIO_Init(ADC_VCC_GPIO_Port, &GPIO_InitStruct);
  }
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
