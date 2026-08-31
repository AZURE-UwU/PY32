/**
  ******************************************************************************
  * @file    py32f0xx_it.c
  * @author  MCU Application Team
  * @Version V1.0.0
  * @Date    2020-10-19
  * @brief   Interrupt Service Routines.
  ******************************************************************************

  */

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "py32f0xx_it.h"
/* Private includes ----------------------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
extern I2C_HandleTypeDef I2cHandle;
extern uint8_t I2C_receive_flag;//已操作时，置1
extern uint8_t I2C_transmit_flag;
extern uint8_t I2C_state;
/* Private function prototypes -----------------------------------------------*/


/* Private user code ---------------------------------------------------------*/


/* External variables --------------------------------------------------------*/


/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  while (1)
  {

  }
}


/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
}


/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{

{
}}

/******************************************************************************/
/* PY32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_py32f003xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles WWDG Interrupt .
  */
void WWDG_IRQHandler(void)
{
}

void PVD_IRQHandler(void)
{
}

void RTC_IRQHandler(void)
{
}

void FLASH_IRQHandler(void)
{
}

void RCC_IRQHandler(void)
{
}

void EXTI0_1_IRQHandler(void)
{
}

void EXTI2_3_IRQHandler(void)
{
}

void EXTI4_15_IRQHandler(void)
{
}

void DMA1_Channel1_IRQHandler(void)
{

  if (READ_BIT(DMA1->ISR, DMA_ISR_TCIF1) == DMA_ISR_TCIF1)
  {
    I2C_receive_flag = 1;
    I2C_state = 0;
  }

  HAL_DMA_IRQHandler(I2cHandle.hdmatx);

}

void DMA1_Channel2_3_IRQHandler(void)
{
  if (READ_BIT(DMA1->ISR, DMA_ISR_TCIF2) == DMA_ISR_TCIF2)
  {

    I2C_transmit_flag = 1;
    I2C_state = 1;
  }
  HAL_DMA_IRQHandler(I2cHandle.hdmarx);
}

void ADC_COMP_IRQHandler(void)
{
}

void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
}

void TIM1_CC_IRQHandler(void)
{
}

void TIM3_IRQHandler(void)
{
}

void LPTIM1_IRQHandler(void)
{
}

void TIM14_IRQHandler(void)
{
}

void TIM16_IRQHandler(void)
{
}

void TIM17_IRQHandler(void)
{
}

void I2C1_IRQHandler(void)
{
  HAL_I2C_EV_IRQHandler(&I2cHandle);
  HAL_I2C_ER_IRQHandler(&I2cHandle);
}

void SPI1_IRQHandler(void)
{
}

void SPI2_IRQHandler(void)
{
}

void USART1_IRQHandler(void)
{
}

void USART2_IRQHandler(void)
{
}




