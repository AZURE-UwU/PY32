/**
  ******************************************************************************
  * @file    bsp_adc.h
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.0
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   ADC 初始化与两通道读取（PY32F002B 无 DMA，用扫描轮询）
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 首次创建。
  ******************************************************************************
  */

#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "main.h"

extern ADC_HandleTypeDef hadc1;

void BSP_ADC1_Init(void);
void BSP_ADC_ReadAll(uint16_t *vcc, uint16_t *ntc); /* 读两路 ADC：VCC、NTC */

#endif /* __BSP_ADC_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

