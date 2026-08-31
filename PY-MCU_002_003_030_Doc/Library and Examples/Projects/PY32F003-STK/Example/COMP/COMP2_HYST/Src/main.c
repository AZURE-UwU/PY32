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
COMP_HandleTypeDef  hcomp;
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
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
  HAL_Init();                                                          //初始化systick

  hcomp.Instance = COMP2;                                              //选择COM2
  hcomp.Init.InputMinus      = COMP_INPUT_MINUS_VREFINT;               //负输入为VREF(1.2V)
  hcomp.Init.InputPlus       = COMP_INPUT_PLUS_IO3;                    //正输入选择为PA3
  hcomp.Init.OutputPol       = COMP_OUTPUTPOL_NONINVERTED;             //COMP2极性选择为不反向
  hcomp.Init.Mode            = COMP_POWERMODE_HIGHSPEED;               //COMP2功耗模式选择为High speed模式
  hcomp.Init.Hysteresis      = COMP_HYSTERESIS_ENABLE;                 //迟滞功能开启
  hcomp.Init.WindowMode      = COMP_WINDOWMODE_DISABLE;                //COMP2输出选择(window模式)
  hcomp.Init.TriggerMode     = COMP_TRIGGERMODE_NONE;                  //COMP2外部初始化不使能
  if (HAL_COMP_Init(&hcomp) != HAL_OK)                                 //COMP2初始化
  {
    Error_Handler();
  }
  HAL_COMP_Start(&hcomp);                                              //COMP2启动

  while (1)
  {
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
