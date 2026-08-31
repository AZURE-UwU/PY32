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
/* Private variables ---------------------------------------------------------*/
COMP_HandleTypeDef  hcomp1;
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
  HAL_Init();                                                           //初始化systick

  hcomp1.Instance = COMP1;                                              //选择COM1
  hcomp1.Init.InputMinus      = COMP_INPUT_MINUS_VREFINT;               //负输入为VREF(1.2V)
  hcomp1.Init.InputPlus       = COMP_INPUT_PLUS_IO3;                    //正输入选择为PA1
  hcomp1.Init.OutputPol       = COMP_OUTPUTPOL_NONINVERTED;             //COMP1极性选择为不反向
  hcomp1.Init.Mode            = COMP_POWERMODE_HIGHSPEED;               //COMP1功耗模式选择为High speed模式
  hcomp1.Init.Hysteresis      = COMP_HYSTERESIS_DISABLE;                //迟滞功能关闭
  hcomp1.Init.WindowMode      = COMP_WINDOWMODE_DISABLE;                //WIODOW关闭
  hcomp1.Init.TriggerMode     = COMP_TRIGGERMODE_IT_RISING_FALLING;     //COMP1上升/下降沿触发
  if (HAL_COMP_Init(&hcomp1) != HAL_OK)                                 //COMP1初始化
  {
    Error_Handler();
  }
  HAL_COMP_Start(&hcomp1);                                              //COMP1启动

  while (1)
  {
  }
}
/********************************************************************************************************
**函数信息 ：HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp)
**功能描述 ：COMP执行函数，每进一次中断，翻转PA11
**输入参数 ：COMP_HandleTypeDef *hcomp
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp)
{
  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
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
