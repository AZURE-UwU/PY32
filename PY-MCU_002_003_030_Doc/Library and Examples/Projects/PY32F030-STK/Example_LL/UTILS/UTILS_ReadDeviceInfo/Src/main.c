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
/* Buffer used for displaying different UTILS info */
uint8_t aShowDeviceID[30]    = {0};
uint8_t aShowRevisionID[30]  = {0};
uint8_t aShowCoordinate[40]  = {0};
uint8_t aShowWaferNumber[30] = {0};
uint8_t aShowLotNumber[30]   = {0};
/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);
static void APP_GetMcuInfo(void);

/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{
  /* 配置系统时钟 */
  APP_SystemClockConfig();

  /* 获取MCU信息 */
  APP_GetMcuInfo();

  while (1)
  {
  }
}


/*******************************************************************************
**功能描述 ：系统时钟配置函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
static void APP_SystemClockConfig(void)
{
  /* 使能并配置HSI */
  LL_RCC_HSI_Enable();
  LL_RCC_HSI_SetCalibFreq(LL_RCC_HSICALIBRATION_24MHz);
  while (LL_RCC_HSI_IsReady() != 1)
  {
  }

  /* 设置 AHB 分频*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* 配置HSISYS作为系统时钟源 */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSISYS);
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSISYS)
  {
  }

  /* 设置 APB1 分频*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

  LL_Init1msTick(24000000);

  /* 更新系统时钟全局变量SystemCoreClock(也可以通过调用SystemCoreClockUpdate函数更新) */
  LL_SetSystemCoreClock(24000000);
}

/*******************************************************************************
**功能描述 ：获取MCU信息
**输入参数 ：
**输出参数 ：
*******************************************************************************/
static void APP_GetMcuInfo(void)
{
  register uint32_t size_string = 0, read_info = 0, read_info2 = 0;

  /* Display Device ID in string format */
  sprintf((char *)aShowDeviceID, "Device ID = 0x%03X", LL_DBGMCU_GetDeviceID());

  /* Display Revision ID in string format */
  sprintf((char *)aShowRevisionID, "Revision ID = 0x%04X", LL_DBGMCU_GetRevisionID());

  /* Display  X and Y coordinates on the wafer expressed in BCD format */
  sprintf((char *)aShowCoordinate, "X and Y coordinates = 0x%08X", LL_GetUID_Word2());

  /* Display Waver number and lot number in string format */
  read_info = LL_GetUID_Word1();
  read_info2 = LL_GetUID_Word0();
  sprintf((char *)aShowWaferNumber, "Wafer NB = 0x%08X", (uint8_t)read_info);
  size_string = sprintf((char *)aShowLotNumber, "Lot NB = 0x%08X", read_info2);
  sprintf((char *)aShowLotNumber + size_string, "%08X", (read_info >> 8));
}

/*******************************************************************************
**功能描述 ：错误执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void Error_Handler(void)
{
  while (1)
  {
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
