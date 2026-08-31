/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @brief   Main program body
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "py32f003xx_Start_Kit.h"

/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void APP_FlashOBProgram(void);

/********************************************************************************************************
**函数信息 ：void main(void)
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  /*初始化systick*/
  HAL_Init();

  /*初始化按键PA12*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);

  /*等待按键按下，防止每次上电都擦写OPTION*/
  while (BSP_PB_GetState(BUTTON_USER));

  /*写OPTION*/
  APP_FlashOBProgram();

  while (1)
  {
  }
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

