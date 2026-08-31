/**
  ******************************************************************************
  * @file    INA226.h
  * @author  Bowen (wbw20)
  * @date    2026-08-19
  * @version V1.0
  * @hardware PY32F002B 开发板（TSSOP20），INA226 挂在 I2C1
  * @brief   INA226 电压/电流/功率监测芯片驱动头文件
  *
  * CHANGELOG:
  *   V1.0 (2026-08-19) 移植自 G030 工程，I2C2 改为 I2C1。
  ******************************************************************************
  */

#ifndef __INA226_H
#define __INA226_H

#include <stdint.h>

/* 寄存器地址 */
#define INA226_REG_CONFIG         0x00
#define INA226_REG_SHUNTVOLTAGE   0x01
#define INA226_REG_BUSVOLTAGE     0x02
#define INA226_REG_POWER          0x03
#define INA226_REG_CURRENT        0x04
#define INA226_REG_CALIBRATION    0x05

/* 校准值：与采样电阻配合决定电流 LSB */
#define INA226_CALIBRATION_VALUE  1024
#define SHUNT_RESISTOR_VALUE      RESISTOR

/* 采样电阻（欧姆），在 main.c 定义，可改配置不动逻辑 */
extern float RESISTOR;

void    INA226_Reset(uint8_t dev_addr);
void    INA226_Configuration(uint8_t dev_addr, uint8_t avgSamples, uint8_t vbusCT, uint8_t vshCT, uint8_t mode);
void    INA226_Init(uint8_t dev_addr, uint8_t avgSamples, uint8_t vbusCT, uint8_t vshCT, uint8_t mode);
void    INA226_SetConfig(uint8_t dev_addr, uint8_t avgSamples, uint8_t vbusCT, uint8_t vshCT, uint8_t mode);
float   INA226_GetBusVoltage(uint8_t dev_addr);
float   INA226_GetShuntVoltage(uint8_t dev_addr);
float   INA226_GetCurrent(uint8_t dev_addr);
float   INA226_GetPower(uint8_t dev_addr);

#endif /* __INA226_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
