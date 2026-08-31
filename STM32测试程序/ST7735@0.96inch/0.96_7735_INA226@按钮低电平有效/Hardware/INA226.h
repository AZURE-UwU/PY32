//#ifndef __INA226_H
//#define __INA226_H

//#include "stm32f10x.h"
//#include "soft_i2c.h" 



//// INA226 I2C Address
//extern volatile uint8_t ADDRESS;

//#define INA226_ADDRESS ADDRESS


//#define INA226_REG_CONFIG       0x00
//#define INA226_REG_SHUNTVOLTAGE 0x01
//#define INA226_REG_BUSVOLTAGE   0x02
//#define INA226_REG_POWER        0x03
//#define INA226_REG_CURRENT      0x04
//#define INA226_REG_CALIBRATION  0x05


//void INA226_Init(void);
//void INA226_Reset(void);
//void INA226_Configuration(void);
//void INA226_WriteRegister(uint8_t reg, uint16_t value);
//uint16_t INA226_ReadRegister(uint8_t reg);
//float INA226_GetBusVoltage(void);
//float INA226_GetShuntVoltage(void);
//float INA226_GetCurrent(void);
//float INA226_GetPower(void);

//#endif  // __INA226_H








#ifndef __INA226_H
#define __INA226_H

#include <stdint.h>

//#define INA226_ADDRESS            0x41    
extern volatile uint8_t ADDRESS;			// 7-bit I²C address (A1=0, A0=1 -> 0x41)
#define INA226_ADDRESS            ADDRESS
#define INA226_REG_CONFIG         0x00
#define INA226_REG_SHUNTVOLTAGE   0x01
#define INA226_REG_BUSVOLTAGE     0x02
#define INA226_REG_POWER          0x03
#define INA226_REG_CURRENT        0x04
#define INA226_REG_CALIBRATION    0x05

// Calibration & shunt resistor
#define INA226_CALIBRATION_VALUE  1024
#define SHUNT_RESISTOR_VALUE      0.01f

void INA226_Init(void);
void INA226_Reset(void);
void INA226_Configuration(void);

void INA226_WriteRegister(uint8_t reg, uint16_t value);
uint16_t INA226_ReadRegister(uint8_t reg);

float INA226_GetBusVoltage(void);
float INA226_GetShuntVoltage(void);
float INA226_GetCurrent(void);
float INA226_GetPower(void);

#endif // __INA226_H
