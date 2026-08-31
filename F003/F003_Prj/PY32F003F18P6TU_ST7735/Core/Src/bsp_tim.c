/**
  ******************************************************************************
  * @file    bsp_tim.c
  * @author  Bowen (wbw20)
  * @date    2026-08-29
  * @version V1.2
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   TIM3_CH2(PB5) 风扇 PWM + TIM14_CH1(PF0) 背光 PWM
  *
  * 设计说明：
  *   - PWM 频率 10kHz（24MHz/24/100），占空比 0~99 无级调节；
  *   - TIM3 为通用定时器，无需 TIM1 的 MOE 处理，配置更简单；
  *   - 背光用 TIM14_CH1(PF0) 输出 PWM，LCD_BLK 无级调光。
  *
  * CHANGELOG:
 *   V1.0 (2026-08-19) 首次创建（TIM1 双通道）；
 *   V1.1 (2026-08-29) 移植 PY32F003F18P6TU：TIM3_CH2(PB5) 单路风扇 PWM。
 *   V1.2 (2026-08-29) PF 引脚复用后恢复背光：TIM14_CH1(PF0)。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "bsp_tim.h"

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim14;

/**
  * @brief  初始化 TIM3_CH2(PB5) 风扇 PWM：10kHz
  */
void BSP_TIM3_PWM_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_TIM3_CLK_ENABLE();

  /* 10kHz：24MHz/(23+1)/(99+1) */
  htim3.Instance               = TIM3;
  htim3.Init.Prescaler         = 23;
  htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim3.Init.Period            = 99;
  htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.RepetitionCounter = 0;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* CH2：初始占空比 0，避免上电瞬间误动作 */
  sConfigOC.OCMode     = TIM_OCMODE_PWM1;
  sConfigOC.Pulse      = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* PB5 -> TIM3_CH2（AF1），推挽高速 */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct.Pin       = PWM_FAN_Pin;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
  HAL_GPIO_Init(PWM_FAN_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief  启动风扇 + 背光 PWM 输出
 */
void BSP_PWM_Start(void)
{
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
}

/**
  * @brief  设置风扇占空比
  * @param  duty: 0~99，越界自动钳位（防御）
  */
void BSP_PWM_SetFan(uint8_t duty)
{
  if (duty > 99U)
  {
    duty = 99U;
  }
  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, duty);
}

/**
  * @brief  初始化 TIM14_CH1(PF0) 背光 PWM：10kHz
  */
void BSP_TIM14_PWM_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_TIM14_CLK_ENABLE();

  /* 10kHz：24MHz/(23+1)/(99+1) */
  htim14.Instance               = TIM14;
  htim14.Init.Prescaler         = 23;
  htim14.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim14.Init.Period            = 99;
  htim14.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.RepetitionCounter = 0;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim14) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* CH1：初始占空比 0，避免上电瞬间满亮度 */
  sConfigOC.OCMode     = TIM_OCMODE_PWM1;
  sConfigOC.Pulse      = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim14, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* PF0 -> TIM14_CH1（AF2），推挽高速 */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  GPIO_InitStruct.Pin       = LCD_BLK_Pin;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM14;
  HAL_GPIO_Init(LCD_BLK_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief  设置背光占空比
  * @param  duty: 0~99，越界自动钳位（防御）
  */
void BSP_PWM_SetBLK(uint8_t duty)
{
  if (duty > 99U)
  {
    duty = 99U;
  }
  __HAL_TIM_SET_COMPARE(&htim14, TIM_CHANNEL_1, duty);
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
