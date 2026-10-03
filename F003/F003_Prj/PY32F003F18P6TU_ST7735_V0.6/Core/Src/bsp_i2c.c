/**
  ******************************************************************************
  * @file    bsp_i2c.c
  * @author  Bowen (wbw20)
  * @date    2026-08-29
  * @version V1.0
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   I2C1 初始化（100kHz，INA226 挂在上面）
  *
 * CHANGELOG:
 *   V1.0 (2026-08-19) 首次创建。
 *   V1.1 (2026-08-29) 移植 PY32F003F18P6TU：SCL=PB6、SDA=PB7。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "bsp_i2c.h"

I2C_HandleTypeDef hi2c1;

/**
  * @brief  初始化 I2C1：SCL=PB6，SDA=PB7，100kHz
  */
void BSP_I2C1_Init(void)
{
  hi2c1.Instance             = I2C1;
  hi2c1.Init.ClockSpeed      = 100000;                  /* 100kHz 标准模式 */
  hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;         /* 标准模式 2:1 占空比 */
  hi2c1.Init.OwnAddress1     = 0;                       /* 主机模式无需本机地址 */
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;   /* 允许从机时钟拉伸 */

  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    APP_ErrorHandler();
  }
}

/**
  * @brief  I2C1 底层初始化（HAL_I2C_Init 内部调用）
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (hi2c->Instance == I2C1)
  {
    __HAL_RCC_I2C_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* I2C 必须开漏输出，外部上拉电阻决定总线电平 */
    GPIO_InitStruct.Pin       = I2C_SCL_Pin | I2C_SDA_Pin;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;            /* 配合板载上拉，增强抗干扰 */
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_I2C;
    HAL_GPIO_Init(I2C_SCL_GPIO_Port, &GPIO_InitStruct);

    /* 复位外设，保证初始化状态干净 */
    __HAL_RCC_I2C_FORCE_RESET();
    __HAL_RCC_I2C_RELEASE_RESET();
  }
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
