/**
  ******************************************************************************
  * @file    py32f0xx_it.c
  * @author  MCU Application Team
  * @Version V1.0.0
  * @Date    2020-10-19
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "py32f0xx_it.h"
/* Private includes ----------------------------------------------------------*/


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/

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
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

    /* USER CODE END NonMaskableInt_IRQn 0 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
    /* USER CODE BEGIN HardFault_IRQn 0 */

    /* USER CODE END HardFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_HardFault_IRQn 0 */
        /* USER CODE END W1_HardFault_IRQn 0 */
    }
}


/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
    /* USER CODE BEGIN SVCall_IRQn 0 */

    /* USER CODE END SVCall_IRQn 0 */
}


/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
    /* USER CODE BEGIN PendSV_IRQn 0 */

    /* USER CODE END PendSV_IRQn 0 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/* PY32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_xm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles WWDG Interrupt .
  */
void WWDG_IRQHandler(void)
{
    /* USER CODE BEGIN WWDG_IRQn 0 */

    /* USER CODE END WWDG_IRQn 0 */
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
}
void DMA1_Channel2_3_IRQHandler(void)
{
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
void LPUART_IRQHandler(void)
{
}





