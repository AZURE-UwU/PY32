//#ifndef __SOFT_I2C_H
//#define __SOFT_I2C_H

//#include "stm32f10x.h"

//// Choose SCL and SDA

//#define SOFT_I2C_SCL_GPIO_PORT    GPIOB
//#define SOFT_I2C_SCL_GPIO_CLK     RCC_APB2Periph_GPIOB
//#define SOFT_I2C_SCL_PIN          GPIO_Pin_13

//#define SOFT_I2C_SDA_GPIO_PORT    GPIOB
//#define SOFT_I2C_SDA_GPIO_CLK     RCC_APB2Periph_GPIOB
//#define SOFT_I2C_SDA_PIN          GPIO_Pin_14


//void Soft_I2C_Delay(void);
//void Soft_I2C_Init(void);
//void Soft_I2C_Start(void);
//void Soft_I2C_Stop(void);
//void Soft_I2C_SendByte(uint8_t byte);
//uint8_t Soft_I2C_ReadByte(uint8_t ack);
//uint8_t Soft_I2C_WaitAck(void);
//void Soft_I2C_SendAck(void);
//void Soft_I2C_SendNack(void);

//#endif




#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H

#include <stdint.h>

// Wire these to your GPIO definitions
#define SOFT_I2C_SCL_GPIO_PORT   GPIOB
#define SOFT_I2C_SCL_PIN         GPIO_Pin_13

#define SOFT_I2C_SDA_GPIO_PORT   GPIOB
#define SOFT_I2C_SDA_PIN         GPIO_Pin_14

#define SOFT_I2C_SCL_GPIO_CLK    RCC_APB2Periph_GPIOB
#define SOFT_I2C_SDA_GPIO_CLK    RCC_APB2Periph_GPIOB

void Soft_I2C_Delay(void);
void Soft_I2C_Init(void);

void Soft_I2C_Start(void);
void Soft_I2C_Stop(void);

void Soft_I2C_SendByte(uint8_t byte);
uint8_t Soft_I2C_ReadByte(uint8_t ack_flag);

uint8_t Soft_I2C_WaitAck(void);
void Soft_I2C_SendAck(void);
void Soft_I2C_SendNack(void);

#endif // __SOFT_I2C_H
