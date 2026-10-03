/**
  ******************************************************************************
  * @file    INA226.c
  * @author  Bowen (wbw20)
  * @date    2026-09-06
  * @version V1.1
  * @hardware PY32F003F18P6TU 开发板（TSSOP20），INA226 挂在 I2C1
  * @brief   INA226 驱动：分层调用（寄存器读写 -> 对外 API -> 应用）
  *
  * 设计说明：
  *   - 底层只提供 WriteRegister/ReadRegister；
  *   - 中层 GetBusVoltage/GetCurrent/GetPower 换算物理量；
  *   - 所有 I2C 操作带超时，失败时返回哨兵值（防御）。
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 移植自 G030 工程：hi2c2 改为 hi2c1。
  *   V1.1 (2026-09-06) SHUNT_RESISTOR_VALUE -> g_shunt_resistor_ohm。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "INA226.h"
#include "bsp_i2c.h"
#include "main.h"

/* I2C 传输超时（ms） */
#ifndef INA226_I2C_TIMEOUT_MS
#define INA226_I2C_TIMEOUT_MS  500
#endif

/* Configuration Register 字段位 */
#define INA226_CFG_RST      (1U << 15)
#define INA226_CFG_AVG(x)   (((x) & 0x7U) << 12)
#define INA226_CFG_VBUSCT(x)(((x) & 0x7U) << 9)
#define INA226_CFG_VSHCT(x) (((x) & 0x7U) << 6)
#define INA226_CFG_MODE(x)  (((x) & 0x7U) << 0)

/* ------------------------------------------------------------------*/
/* 底层：寄存器读写                                                  */
/* ------------------------------------------------------------------*/

/**
  * @brief  写 INA226 寄存器（大端，先高后低）
  * @param  dev_addr: 器件 7bit 地址（0x40）
  * @param  reg: 寄存器地址
  * @param  value: 16bit 数值
  */
void INA226_WriteRegister(uint8_t dev_addr, uint8_t reg, uint16_t value)
{
  uint8_t tx[3];
  tx[0] = reg;
  tx[1] = (uint8_t)(value >> 8);
  tx[2] = (uint8_t)(value & 0xFF);

  /* PY32 HAL I2C 使用 8bit 地址（7bit 地址左移一位） */
  uint16_t devAddr8 = (uint16_t)((uint16_t)dev_addr << 1);

  if (HAL_I2C_Master_Transmit(&hi2c1, devAddr8, tx, sizeof(tx), INA226_I2C_TIMEOUT_MS) != HAL_OK)
  {
    /* 传输失败：应用层读到哨兵值后可以自行重试 */
  }
}

/**
  * @brief  读 INA226 寄存器
  * @retval 16bit 寄存器值；失败返回 0xFFFF 哨兵值
  */
uint16_t INA226_ReadRegister(uint8_t dev_addr, uint8_t reg)
{
  uint8_t buf[2] = {0};
  uint16_t devAddr8 = (uint16_t)((uint16_t)dev_addr << 1);

  if (HAL_I2C_Mem_Read(&hi2c1, devAddr8, reg, I2C_MEMADD_SIZE_8BIT,
                       buf, 2, INA226_I2C_TIMEOUT_MS) != HAL_OK)
  {
    return 0xFFFFU;
  }

  return (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
}

/* ------------------------------------------------------------------*/
/* 对外 API                                                          */
/* ------------------------------------------------------------------*/

/**
  * @brief  软复位 INA226
  */
void INA226_Reset(uint8_t dev_addr)
{
  INA226_WriteRegister(dev_addr, INA226_REG_CONFIG, 0x8000U);
}

/**
  * @brief  配置平均次数、转换时间与工作模式
  */
void INA226_Configuration(uint8_t dev_addr, uint8_t avgSamples, uint8_t vbusCT, uint8_t vshCT, uint8_t mode)
{
  uint16_t cfg = 0;
  cfg |= (uint16_t)INA226_CFG_AVG(avgSamples & 0x7U);
  cfg |= (uint16_t)INA226_CFG_VBUSCT(vbusCT & 0x7U);
  cfg |= (uint16_t)INA226_CFG_VSHCT(vshCT & 0x7U);
  cfg |= (uint16_t)INA226_CFG_MODE(mode & 0x7U);
  INA226_WriteRegister(dev_addr, INA226_REG_CONFIG, cfg);
}

/**
  * @brief  初始化：复位 -> 配置 -> 写校准值
  */
void INA226_Init(uint8_t dev_addr, uint8_t avgSamples, uint8_t vbusCT, uint8_t vshCT, uint8_t mode)
{
  INA226_Reset(dev_addr);
  HAL_Delay(5);
  INA226_Configuration(dev_addr, avgSamples, vbusCT, vshCT, mode);
  INA226_WriteRegister(dev_addr, INA226_REG_CALIBRATION, INA226_CALIBRATION_VALUE);
}

/**
  * @brief  单独更新配置（滤波/转换时间/模式）
  */
void INA226_SetConfig(uint8_t dev_addr, uint8_t avgSamples, uint8_t vbusCT, uint8_t vshCT, uint8_t mode)
{
  INA226_Configuration(dev_addr, avgSamples, vbusCT, vshCT, mode);
}

/**
  * @brief  读取母线电压（V），1 LSB = 1.25mV
  */
float INA226_GetBusVoltage(uint8_t dev_addr)
{
  uint16_t raw = INA226_ReadRegister(dev_addr, INA226_REG_BUSVOLTAGE);
  if (raw == 0xFFFFU)
  {
    return -1.0f; /* 哨兵值：总线电压不可能为负，调用方可识别失败 */
  }
  return (float)raw * 1.25f / 1000.0f;
}

/**
  * @brief  读取分流电压（V，有符号），1 LSB = 2.5µV
  */
float INA226_GetShuntVoltage(uint8_t dev_addr)
{
  uint16_t v = INA226_ReadRegister(dev_addr, INA226_REG_SHUNTVOLTAGE);
  if (v == 0xFFFFU)
  {
    return 0.0f;
  }
  int16_t raw = (int16_t)v;
  return (float)raw * 2.5f / 1000.0f;
}

/**
  * @brief  读取电流（A），由校准值 + 采样电阻决定 LSB
  */
float INA226_GetCurrent(uint8_t dev_addr)
{
  uint16_t v = INA226_ReadRegister(dev_addr, INA226_REG_CURRENT);
  if (v == 0xFFFFU)
  {
    return 0.0f;
  }
  int16_t raw = (int16_t)v;
  float current_lsb = 0.00512f / ((float)INA226_CALIBRATION_VALUE * g_shunt_resistor_ohm);
  return (float)raw * current_lsb;
}

/**
  * @brief  读取功率（W），功率 LSB = 25 × 电流 LSB
  */
float INA226_GetPower(uint8_t dev_addr)
{
  uint16_t raw = INA226_ReadRegister(dev_addr, INA226_REG_POWER);
  if (raw == 0xFFFFU)
  {
    return 0.0f;
  }
  float current_lsb = 0.00512f / ((float)INA226_CALIBRATION_VALUE * g_shunt_resistor_ohm);
  float power_lsb   = current_lsb * 25.0f;
  return (float)raw * power_lsb;
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

