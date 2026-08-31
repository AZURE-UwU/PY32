#ifndef __GPIO_H
#define __GPIO_H

#include "stm32f10x.h"



void GPIO_Configuration(void);

uint16_t ADC1_Read(void);
void ADC1_Init(void);

void ADC2_Init(void);
uint16_t ADC2_Read(void);



void PWM_Init(void);
void PWM_UpdateParameters(void);


void ButtonScan_ms(void);
#endif
