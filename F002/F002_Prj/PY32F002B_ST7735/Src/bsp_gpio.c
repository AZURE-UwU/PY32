/**
  ******************************************************************************
  * @file    bsp_gpio.c
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.1
  * @hardware PY32F002B 开发板（TSSOP20）
  * @brief   GPIO 初始化：LCD 控制脚、LED、三个按键（EXTI）
  *
  * 设计说明：
  *   - LCD_CS/RST 上电默认高、DC 默认低，避免屏幕复位时序异常；
  *   - 按键配置为双沿中断 + 下拉：按下接高、松开回低；
  *   - 中断里只记录时间戳，消抖和按键逻辑放 50ms 周期任务（前后台）。
  *
 * CHANGELOG:
 *   V1.0 (2026-08-19) 首次创建。
 *   V1.1 (2026-08-24) 拆出按键唤醒/恢复配置：关机只留 SW_WKUP 上升沿唤醒，
 *                     解决长按关机后立刻被按键沿唤醒的循环问题。
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
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* 默认电平：CS/RST 高（空闲），DC/LED 低 */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port,  LCD_CS_Pin,  GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LCD_DC_GPIO_Port,  LCD_DC_Pin,  GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LED_STATE_GPIO_Port, LED_STATE_Pin, GPIO_PIN_RESET);

  /* LCD 控制脚：推挽输出，高速（SPI 时序相关） */
  GPIO_InitStruct.Pin   = LCD_CS_Pin | LCD_RST_Pin | LCD_DC_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* 状态指示灯：推挽输出，低速即可 */
  GPIO_InitStruct.Pin   = LED_STATE_Pin;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_STATE_GPIO_Port, &GPIO_InitStruct);

  /* 三个按键：双沿中断 + 下拉（与唤醒后的恢复共用同一份配置） */
  BSP_BtnRestoreConfig();
}

/**
  * @brief  恢复三个按键的双沿中断配置（正常运行态）
  */
void BSP_BtnRestoreConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 双沿 + 上/下拉（BTN_ACTIVE_LOW 决定按下是高还是低电平） */
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
  * @brief  进入 STOP 前的唤醒配置：只允许 SW_WKUP 的"按下"（上升沿）唤醒
  *
  * 设计说明：
  *   - 长按 SW_WKUP 关机时按键仍被按住，若保留双沿中断，
  *     松手产生的下降沿会立刻把芯片唤醒，形成关机-开机循环；
  *   - 改为上升沿后：松手不唤醒，只有下一次按下才唤醒；
  *   - 另两个按键关掉 EXTI（HAL 的非 EXTI 模式不会动 EXTI 寄存器，
  *     需手动清 IMR），防止误唤醒。
  */
void BSP_BtnWakeConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pull  = (BTN_ACTIVE_LOW ? GPIO_PULLUP : GPIO_PULLDOWN);
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  /* 唤醒键：只响应"按下"（高电平有效=上升沿，低电平有效=下降沿） */
  GPIO_InitStruct.Mode = (BTN_ACTIVE_LOW ? GPIO_MODE_IT_FALLING : GPIO_MODE_IT_RISING);
  GPIO_InitStruct.Pin  = SW_WKUP_Pin;
  HAL_GPIO_Init(SW_WKUP_GPIO_Port, &GPIO_InitStruct);
  HAL_NVIC_EnableIRQ(SW_WKUP_EXTI_IRQn);

  /* 功能键：普通输入 + 关 EXTI 中断 */
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pin  = SW_FUNC_Pin;
  HAL_GPIO_Init(SW_FUNC_GPIO_Port, &GPIO_InitStruct);
  CLEAR_BIT(EXTI->IMR, SW_FUNC_Pin);
  HAL_NVIC_DisableIRQ(SW_FUNC_EXTI_IRQn);

  /* 模式键：普通输入 + 关 EXTI 中断 */
  GPIO_InitStruct.Pin = SW_MODE_Pin;
  HAL_GPIO_Init(SW_MODE_GPIO_Port, &GPIO_InitStruct);
  CLEAR_BIT(EXTI->IMR, SW_MODE_Pin);
  HAL_NVIC_DisableIRQ(SW_MODE_EXTI_IRQn);
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

/**
  * @brief  翻转状态指示灯
  */
void BSP_LED_Toggle(void)
{
  HAL_GPIO_TogglePin(LED_STATE_GPIO_Port, LED_STATE_Pin);
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
