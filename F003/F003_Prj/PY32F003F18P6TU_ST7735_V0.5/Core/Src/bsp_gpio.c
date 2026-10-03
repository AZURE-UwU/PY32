/**
  ******************************************************************************
  * @file    bsp_gpio.c
  * @author  Bowen (wbw20)
  * @date    2026-09-06
  * @version V1.7
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   GPIO 初始化：LCD 控制脚、LED、三个按键（EXTI）
  *
  * 设计说明：
  *   - LCD CS/RST 默认高、DC 默认低；
  *   - 按键双沿中断，上/下拉与"按下有效电平"由 BTN_ACTIVE_LOW 决定；
  *     本板按键高电平有效（内部下拉）；
  *   - 中断里只记录时间戳，消抖和按键逻辑放 50ms 周期任务（前后台）。
  *
 * CHANGELOG:
 *   V1.0 (2026-08-19) 首次创建。
 *   V1.1 (2026-08-24) 拆出按键唤醒/恢复配置：关机只留 SW_WKUP 上升沿唤醒，
 *                     解决长按关机后立刻被按键沿唤醒的循环问题。
 *   V1.2 (2026-08-29) 移植 PY32F003F18P6TU：LED=PA0、按键=PA12/PA6/PA7，
 *                     LCD 只保留 DC=PA3（CS/RST/BLK 省脚）。
 *   V1.3 (2026-08-29) PF 引脚复用后恢复 LCD_CS(PF1)/LCD_RST(PF4)。
 *   V1.4 (2026-08-30) 修复 STOP 无法按键唤醒：三个按键共用 EXTI4_15 这一根
 *                     NVIC 中断，原实现关闭 SW_FUNC/SW_MODE 时误把共享的
 *                     EXTI4_15 一起关掉，导致 SW_WKUP 也无法唤醒。
  *   V1.5 (2026-08-30) 全部按键改为高电平有效（BTN_ACTIVE_LOW=0）：
  *                     内部下拉，按下为高，STOP 唤醒走上升沿。
  *   V1.6 (2026-09-06) PA0 改为电源使能（上电高/关机低）；
  *                     新增 BSP_GPIO_LowPowerConfig：关机把非必要引脚置模拟输入，
  *                     PA0 拉低，仅保留唤醒键，最大化降低 STOP 功耗。
  *   V1.7 (2026-09-06) 修复关机后背光仍亮：BLK(PF0)/风扇(PB5) 关机时保持输出低，
  *                     不再悬空；LCD RST 拉低复位、CS 拉高取消片选，进一步降功耗。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "bsp_gpio.h"

/**
  * @brief  初始化 LCD 控制脚、LED、三个按键
  * @param  无
  * @retval 无
  */
void BSP_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 先开端口时钟，再写引脚 */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  /* 默认电平：CS/RST 高（空闲），DC 低，PA0 电源使能置高（上电即开机） */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,  GPIO_PIN_RESET);
  HAL_GPIO_WritePin(PWR_EN_GPIO_Port, PWR_EN_Pin, GPIO_PIN_SET);

  /* LCD 控制脚：推挽输出，高速（SPI 时序相关） */
  GPIO_InitStruct.Pin   = LCD_CS_Pin | LCD_RST_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin   = LCD_DC_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* 电源使能（PA0）：推挽输出，上电高 */
  GPIO_InitStruct.Pin   = PWR_EN_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PWR_EN_GPIO_Port, &GPIO_InitStruct);

  /* 三个按键：双沿中断 + 上/下拉（与唤醒后的恢复共用同一份配置） */
  BSP_BtnRestoreConfig();
}

/**
  * @brief  恢复三个按键的双沿中断配置（正常运行态）
  */
void BSP_BtnRestoreConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 双沿 + 上/下拉（BTN_ACTIVE_LOW 决定按下是高还是低电平）：
     本板按键高电平有效 → 内部下拉 */
  GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull  = (BTN_ACTIVE_LOW ? GPIO_PULLUP : GPIO_PULLDOWN);
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  GPIO_InitStruct.Pin = SW_WKUP_Pin;
  HAL_GPIO_Init(SW_WKUP_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = SW_FUNC_Pin;
  HAL_GPIO_Init(SW_FUNC_GPIO_Port, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = SW_MODE_Pin;
  HAL_GPIO_Init(SW_MODE_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(SW_WKUP_EXTI_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(SW_WKUP_EXTI_IRQn);
  HAL_NVIC_SetPriority(SW_FUNC_EXTI_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(SW_FUNC_EXTI_IRQn);
  HAL_NVIC_SetPriority(SW_MODE_EXTI_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(SW_MODE_EXTI_IRQn);
}

/**
  * @brief  进入 STOP 前的低功耗配置：非必要引脚置模拟输入，必要负载引脚保持确定电平
  *
  * 设计说明：
  *   - 全部引脚先置模拟输入，消除输入浮空/上下拉漏电，最大化降低 STOP 功耗；
  *   - 负载/使能类引脚必须保持确定电平，否则悬空会误开启：
  *       PA0 电源使能拉低、BLK(PF0) 背光拉低、FAN(PB5) 风扇拉低；
  *   - LCD 控制脚：RST(PF4) 拉低复位（更低功耗、状态确定）、CS(PF1) 拉高取消片选；
  *   - 只保留 SW_WKUP 的"按下"边沿唤醒：长按关机时松手的反相沿不会误唤醒；
  *   - 另两键已是模拟输入，仅清其 EXTI 位，不关共享的 EXTI4_15 NVIC。
  */
void BSP_GPIO_LowPowerConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 1. 三端口全部引脚置模拟输入（最低功耗） */
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  for (uint8_t i = 0; i < 16; i++)
  {
    GPIO_InitStruct.Pin = (uint16_t)(1U << i);
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  }

  /* 2. 必要保持电平的引脚：输出低/高，避免悬空导致背光/风扇/负载误开启 */
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  /* 电源使能：低（关机） */
  GPIO_InitStruct.Pin   = PWR_EN_Pin;
  HAL_GPIO_Init(PWR_EN_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(PWR_EN_GPIO_Port, PWR_EN_Pin, GPIO_PIN_RESET);

  /* 风扇 PWM：低（风扇停） */
  GPIO_InitStruct.Pin = PWM_FAN_Pin;
  HAL_GPIO_Init(PWM_FAN_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(PWM_FAN_GPIO_Port, PWM_FAN_Pin, GPIO_PIN_RESET);

  /* 背光 + RST 拉低、CS 拉高：LCD 复位且不选中 */
  GPIO_InitStruct.Pin = (LCD_BLK_Pin | LCD_RST_Pin | LCD_CS_Pin);
  HAL_GPIO_Init(LCD_BLK_GPIO_Port, &GPIO_InitStruct);
  HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_SET);

  /* 3. 唤醒键：只响应"按下"边沿（高有效=上升沿，低有效=下降沿） */
  GPIO_InitStruct.Pull  = (BTN_ACTIVE_LOW ? GPIO_PULLUP : GPIO_PULLDOWN);
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Mode  = (BTN_ACTIVE_LOW ? GPIO_MODE_IT_FALLING : GPIO_MODE_IT_RISING);
  GPIO_InitStruct.Pin   = SW_WKUP_Pin;
  HAL_GPIO_Init(SW_WKUP_GPIO_Port, &GPIO_InitStruct);
  HAL_NVIC_EnableIRQ(SW_WKUP_EXTI_IRQn);

  /* 4. 另外两键已是模拟输入，清其 EXTI 位防误唤醒 */
  CLEAR_BIT(EXTI->IMR,  SW_FUNC_Pin);
  CLEAR_BIT(EXTI->RTSR, SW_FUNC_Pin);
  CLEAR_BIT(EXTI->FTSR, SW_FUNC_Pin);
  CLEAR_BIT(EXTI->IMR,  SW_MODE_Pin);
  CLEAR_BIT(EXTI->RTSR, SW_MODE_Pin);
  CLEAR_BIT(EXTI->FTSR, SW_MODE_Pin);
}

/**
  * @brief  读取按键稳定电平
  * @param  index: 0=SW_WKUP，1=SW_FUNC，2=SW_MODE
  * @retval 0=松开，1=按下；越界返回 0（防御）
  */
uint8_t BSP_BtnRead(uint8_t index)
{
  GPIO_PinState pressed = (BTN_ACTIVE_LOW ? GPIO_PIN_RESET : GPIO_PIN_SET);
  switch (index)
  {
    case 0: return (HAL_GPIO_ReadPin(SW_WKUP_GPIO_Port, SW_WKUP_Pin) == pressed) ? 1U : 0U;
    case 1: return (HAL_GPIO_ReadPin(SW_FUNC_GPIO_Port, SW_FUNC_Pin) == pressed) ? 1U : 0U;
    case 2: return (HAL_GPIO_ReadPin(SW_MODE_GPIO_Port, SW_MODE_Pin) == pressed) ? 1U : 0U;
    default: return 0U;
  }
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
