/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "py32f003xx_ll_Start_Kit.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_SystemClockConfig(void);
static void APP_FlashOBProgram(void);

/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{
  /*初始化systick*/
  HAL_Init();

  /*初始化按键PA12*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);

  /*等待按键按下，防止每次上电都擦写OPTION*/
  while (BSP_PB_GetState(BUTTON_USER));

  /*时钟初始化,配置系统时钟为HSI*/
  APP_SystemClockConfig();

  /*写OPTION*/
  APP_FlashOBProgram();

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
  /* HSI使能及初始化 */
  LL_RCC_HSI_Enable();
  LL_RCC_HSI_SetCalibFreq(LL_RCC_HSICALIBRATION_24MHz);
  while (LL_RCC_HSI_IsReady() != 1)
  {
  }

  /* 设置AHB分频*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* 配置HSISYS为系统时钟及初始化 */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSISYS);
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSISYS)
  {
  }

  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  /*设置APB1分频及初始化*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  /* 更新系统时钟全局变量SystemCoreClock(也可以通过调用SystemCoreClockUpdate函数更新) */
  LL_SetSystemCoreClock(24000000);
}

/*******************************************************************************
**功能描述 ：写OPTION
**输入参数 ：
**输出参数 ：
*******************************************************************************/
static void APP_FlashOBProgram(void)
{
  FLASH_OBProgramInitTypeDef OBInitCfg;
  if( READ_BIT(FLASH->OPTR, FLASH_OPTR_NRST_MODE) == 0)
  {
  HAL_FLASH_Unlock();/*解锁FLASH*/
  HAL_FLASH_OB_Unlock();/*解锁OPTION*/

  /*配置OPTION选项*/
  OBInitCfg.OptionType = OPTIONBYTE_USER;
  OBInitCfg.USERType = OB_USER_BOR_EN | OB_USER_BOR_LEV | OB_USER_IWDG_SW | OB_USER_WWDG_SW | OB_USER_NRST_MODE | OB_USER_nBOOT1;

  /*BOR不使能/BOR上升3.0,下降2.9/软件模式看门狗/仅复位输入/System memory作为启动区*/
  OBInitCfg.USERConfig = OB_BOR_DISABLE | OB_BOR_LEVEL_2p9_3p0 | OB_IWDG_SW | OB_WWDG_SW | OB_RESET_MODE_RESET | OB_BOOT1_SYSTEM;

  /*BOR不使能/BOR上升3.2,下降3.1/软件模式看门狗/仅复位输入/System memory作为启动区*/
  //OBInitCfg.USERConfig = OB_BOR_DISABLE | OB_BOR_LEVEL_3p1_3p2 | OB_IWDG_SW | OB_WWDG_SW | OB_RESET_MODE_RESET | OB_BOOT1_SYSTEM;/*恢复OPTION*/

  /* 启动option byte编程 */
  HAL_FLASH_OBProgram(&OBInitCfg);

  HAL_FLASH_Lock();/*锁定FLASH*/
  HAL_FLASH_OB_Lock();/*锁定OPTION*/

  /*产生一个复位，option byte装载*/
  HAL_FLASH_OB_Launch();
  }
}

/*******************************************************************************
**功能描述 ：错误执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void Error_Handler(void)
{
  while (1);
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
