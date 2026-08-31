/**
  ******************************************************************************
  * @file    Project/STM32F0_Internal_RC_Oscillators_Calibration/Inc/hsi.h
  * @author  MCU Application Team
  * @version V0.2.0
  * @date    05-December-2014
  * @brief   Header for hsi.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) Puya Semiconductor Co.
  * All rights reserved.</center></h2>
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HSI_MEASUREMENT_H
#define __HSI_MEASUREMENT_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void HSI_MeasurementInit(uint32_t HSICLKSource_set);
uint32_t 		HSI_FreqMeasure(void);


uint32_t Hsi_Rough_Trimming(void);
uint32_t Hsi_Fine_Trimming(void);

#endif /* __HSI_MEASUREMENT_H */

/******************* (C) COPYRIGHT 2014 STMicroelectronics *****END OF FILE****/
