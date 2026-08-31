/**
  ******************************************************************************
  * @file    bsp_tim.c
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   TIM1 双通道 PWM：CH1 风扇、CH2 背光
  *
  * 设计说明：
  *   - PWM 频率 10kHz（24MHz/24/100），风扇与背光同频；
  *   - 占空比 0~99（ARR=99），支持无级调节；
  *   - HAL_TIM_PWM_Start 会自动置 MOE，TIM1 高级定时器输出无需额外配置。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 首次创建：合并原 TIM3/TIM17 两路 PWM 到 TIM1。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "bsp_tim.h"

TIM_HandleTypeDef htim1;

/**
  * @brief  初始化 TIM1 PWM：CH1(PA0)=风扇，CH2(PA1)=背光
  */
void BSP_TIM1_PWM_Init(void)
{
  TIM_OC_InitTypeDef sConfigOC = {0};
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 打开 TIM1 外设时钟，否则寄存器写不进 */
  __HAL_RCC_TIM1_CLK_ENABLE();

  /* 10kHz：24MHz/(23+1)/(99+1) */
  htim1.Instance               = TIM1;
  htim1.Init.Prescaler         = 23;
  htim1.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim1.Init.Period            = 99;
  htim1.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* 两路 PWM 通道：初始占空比 0，避免上电瞬间误动作 */
  sConfigOC.OCMode     = TIM_OCMODE_PWM1;
  sConfigOC.Pulse      = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    APP_ErrorHandler();
  }

  /* PWM 引脚：PA0(CH1)、PA1(CH2)，AF2，推挽高速 */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin       = PWM_FAN_Pin | LCD_BLK_Pin;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
  HAL_GPIO_Init(PWM_FAN_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief  启动两路 PWM 输出
  */
void BSP_PWM_Start(void)
{
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
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
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
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
  __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, duty);
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
