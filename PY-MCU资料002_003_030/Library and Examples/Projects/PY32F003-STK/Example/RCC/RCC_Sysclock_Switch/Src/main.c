/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void SetSysClock(uint32_t SYSCLKSource);
void Error_Handler(void);

/********************************************************************************************************
**函数信息 ：int main(void)
**功能描述 ：main函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  //初始化函数
  HAL_Init();                                                             //初始化systick
  HAL_SuspendTick();
  //配置系统时钟为默认HSI 8MHz，之后切换为LSI时钟
  SystemClock_Config();

  //配置PA01引脚为MCO功能，输出系统时钟
  HAL_RCC_MCOConfig(RCC_MCO2, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_1);              //配置PA01引脚为MCO功能，输出系统时钟

  while (BSP_PB_GetState(BUTTON_KEY) == 1)
  {
    ;
  }

  //切换系统时钟为HSE外部晶振时钟
  SetSysClock(RCC_SYSCLKSOURCE_HSE);                                      //配置系统时钟为HSE

  while (1)
  {
    ;
  }
}
/********************************************************************************************************
**函数信息 ：void SystemClock_Config(void)
**功能描述 ：系统时钟配置
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  //配置时钟源HSE/HSI/LSE/LSI
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                    //开启HSI
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                                                    //不分频
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_4MHz;                          //配置HSI输出时钟为4MHz
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_8MHz;                            //配置HSI输出时钟为8MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz;                         //配置HSI输出时钟为16MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_22p12MHz;                      //配置HSI输出时钟为22.12MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;                         //配置HSI输出时钟为24MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;                                                    //开启HSE
  RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;                                               //HSE工作频率范围16M~32M
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;                                                    //开启LSI

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)                                        //初始化RCC振荡器
  {
    Error_Handler();
  }

  //初始化CPU,AHB,APB总线时钟
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; //RCC系统时钟类型
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_LSI;                                      //SYSCLK的源选择为LSI
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                          //APH时钟不分频
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                            //APB时钟不分频

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)                      //初始化RCC系统时钟(FLASH_LATENCY_0=24M以下;FLASH_LATENCY_1=48M)
  {
    Error_Handler();
  }
}
/********************************************************************************************************
**函数信息 ：void SetSysClock(uint32_t SYSCLKSource)
**功能描述 ：设置系统时钟
**输入参数 ：uint32_t SYSCLKSource(可选RCC_SYSCLKSOURCE_LSI/RCC_SYSCLKSOURCE_LSE
                                    /RCC_SYSCLKSOURCE_HSE/RCC_SYSCLKSOURCE_HSI/RCC_SYSCLKSOURCE_PLLCLK)
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void SetSysClock(uint32_t SYSCLKSource)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  //RCC系统时钟类型
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = SYSCLKSource;                          //系统时钟源选择
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                      //APH时钟不分频
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;                        //APB时钟2分频

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)//初始化RCC系统时钟(FLASH_LATENCY_0=24M以下;FLASH_LATENCY_1=48M)
  {
    Error_Handler();
  }
}
/********************************************************************************************************
**函数信息 ：Error_Handler(void)
**功能描述 ：错误执行函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void Error_Handler(void)
{
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */


/* Private function -------------------------------------------------------*/
