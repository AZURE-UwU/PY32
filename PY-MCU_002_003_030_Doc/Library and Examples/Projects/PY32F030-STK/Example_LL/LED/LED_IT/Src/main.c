/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @Version V1.0.0
  * @Date    2022-8-19
  * @brief   main function
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "py32f030xx_ll_Start_Kit.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const uint8_t dispArr[] = {LL_LED_DISP_0, LL_LED_DISP_1, LL_LED_DISP_2, LL_LED_DISP_3, LL_LED_DISP_4, \
                           LL_LED_DISP_5, LL_LED_DISP_6, LL_LED_DISP_7, LL_LED_DISP_8, LL_LED_DISP_9
                          };
uint8_t dispNum = 0;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void APP_ConfigLed(void);

/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{
  /* 配置系统时钟 */
  SystemClock_Config();
  
  /* LED数码管配置 */
  APP_ConfigLed();

  while (1)
  {
    /* 每经过1s切换显示值对应的索引 */
    dispNum++;
    if (dispNum == 10)
    {
      dispNum = 0;
    }
    LL_mDelay(1000);
  }
}

/*******************************************************************************
**功能描述 ：系统时钟配置函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void SystemClock_Config(void)
{
  /* 使能HSI */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  /* 设置 AHB 分频*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* 配置HSISYS作为系统时钟源 */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSISYS);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSISYS)
  {
  }

  /* 设置 APB1 分频*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_Init1msTick(8000000);

  /* 更新系统时钟全局变量SystemCoreClock(也可以通过调用SystemCoreClockUpdate函数更新) */
  LL_SetSystemCoreClock(8000000);
}

/*******************************************************************************
**功能描述 ：错误执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void Error_Handler(void)
{
  while(1)
  {
  }
}

/*******************************************************************************
**功能描述 ：配置LED数码管
**输入参数 ：
**输出参数 ：
*******************************************************************************/
static void APP_ConfigLed(void)
{
  /* 使能时钟 */
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LED);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
  
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* 配置 PB3, PB4, PB5 为 com1 com2 com3 */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_3 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* 配置 PA15 为 com0 */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* 配置 PA0, PA1, PA2, PA3, PA4, PA5, PA6 为 SEG B C D E F G DP */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 | LL_GPIO_PIN_3 |\
                        LL_GPIO_PIN_4 | LL_GPIO_PIN_5 | LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_3;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* 配置 PB8 为 SEG A */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_3;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  /* 使能 LED 中断 */
  NVIC_SetPriority(LED_IRQn, 0);
  NVIC_EnableIRQ(LED_IRQn);
  LL_LED_EnableIT(LED);

  /* 设置 LED COM 驱动能力 */
  LL_LED_SetComDrive(LED, LL_LED_COMDRIVE_LOW);

  /* 设置 COM 打开数 */
  LL_LED_SetComNum(LED, LL_LED_COMSELECT_4COM);

  /* 设置 LED 预分频值 */
  LL_LED_SetPrescaler(LED, 9);

  /* 设置 LED 点亮和切换时间 */
  LL_LED_SetLightAndDeadTime(LED, 0xF0, 0x10);
  
  /* 使能 LED */
  LL_LED_Enable(LED);
}

/*******************************************************************************
**功能描述 ：点亮LED数码管完成的中断回调函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_LedLightCpltCallback(void)
{
  static uint32_t oldValue = 0xFF;

  if (oldValue != dispNum)
  {
    oldValue = dispNum;
    LL_LED_SetDisplayValue(LED, LL_LED_COM0, dispArr[(dispNum) % 10]);
    LL_LED_SetDisplayValue(LED, LL_LED_COM1, dispArr[(dispNum + 1) % 10]);
    LL_LED_SetDisplayValue(LED, LL_LED_COM2, dispArr[(dispNum + 2) % 10]);
    LL_LED_SetDisplayValue(LED, LL_LED_COM3, dispArr[(dispNum + 3) % 10]);
  }
}

#ifdef  USE_FULL_ASSERT
/*******************************************************************************
**功能描述 ：输出产生断言错误的源文件名及行号
**输入参数 ：file：源文件名指针
**输入参数 ：line：发生断言错误的行号
**输出参数 ：
*******************************************************************************/
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT Puya *****END OF FILE****/
