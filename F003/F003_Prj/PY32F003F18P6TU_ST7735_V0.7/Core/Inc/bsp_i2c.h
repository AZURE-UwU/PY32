/**
  ******************************************************************************
  * @file    bsp_i2c.h
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.0
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   I2C1 初始化（INA226 挂在上面）
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 首次创建。
  ******************************************************************************
  */

#ifndef __BSP_I2C_H
#define __BSP_I2C_H

#include "main.h"

extern I2C_HandleTypeDef hi2c1;

void BSP_I2C1_Init(void);

#endif /* __BSP_I2C_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

