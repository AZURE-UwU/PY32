/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @Version V1.0.0
  * @Date    2020-10-19
  * @brief   main function
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Private define ------------------------------------------------------------*/
#define APP_GPIO_TOGGLE()  {GPIOA->BSRR = u32High;\
                            GPIOA->BSRR = u32Low; \
                            GPIOA->BSRR = u32High;\
                            GPIOA->BSRR = u32Low; \
                            GPIOA->BSRR = u32High;\
                            GPIOA->BSRR = u32Low; \
                            GPIOA->BSRR = u32High;\
                            GPIOA->BSRR = u32Low; \
                            GPIOA->BSRR = u32High;\
                            GPIOA->BSRR = u32Low;}
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_GPIO_Init(void);
static void APP_SystemClock_Config(void);

/********************************************************************************************************
**函数信息 ：int main(void)
**功能描述 ：main函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  uint32_t u32High = 0x00000800;
  uint32_t u32Low  = 0x08000000;

  /* 初始化systick */
  HAL_Init();

  /* PA11 初始化 */
  APP_GPIO_Init();

  /* 配置系统时钟 */
  APP_SystemClock_Config();

  while (1)
  {
    /* LED 翻转输出约12MHz */
    APP_GPIO_TOGGLE();
    APP_GPIO_TOGGLE();
    APP_GPIO_TOGGLE();
    APP_GPIO_TOGGLE();
    APP_GPIO_TOGGLE();
    APP_GPIO_TOGGLE();
    APP_GPIO_TOGGLE();
    APP_GPIO_TOGGLE();
    APP_GPIO_TOGGLE();
    APP_GPIO_TOGGLE();
  }
}

/********************************************************************************************************
**函数信息 ：APP_GPIO_Init(void)
**功能描述 ：GPIO初始化(PA11)
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
static void APP_GPIO_Init()
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  __HAL_RCC_GPIOA_CLK_ENABLE();                          //GPIOA时钟使能

  //初始化GPIOA11
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;            //推挽输出
  GPIO_InitStruct.Pull = GPIO_PULLUP;                    //使能上拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;     //GPIO速度

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                //GPIO初始化
}

/********************************************************************************************************
**函数信息 ：static void APP_SystemClock_Config(void)
**功能描述 ：系统时钟配置
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
static void APP_SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  //配置时钟源HSE/HSI/LSE/LSI
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                    //开启HSI
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                                                    //不分频
//  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_4MHz;                            //配置HSI输出时钟为4MHz
//  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_8MHz;                            //配置HSI输出时钟为8MHz
//  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz;                           //配置HSI输出时钟为16MHz
//  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_22p12MHz;                        //配置HSI输出时钟为22.12MHz
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;                           //配置HSI输出时钟为24MHz

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)                                        //初始化RCC振荡器
  {
    Error_Handler();
  }

  //初始化CPU,AHB,APB总线时钟
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;  //RCC系统时钟类型
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;                                          //SYSCLK的源选择为HSI
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                              //APH时钟不分频
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                               //APB时钟不分频

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)                         //初始化RCC系统时钟(FLASH_LATENCY_0=24M以下;FLASH_LATENCY_1=48M)
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



