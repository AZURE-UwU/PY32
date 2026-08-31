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
#define STATE_OVER_THRESHOLD    0x00000001
#define STATE_WITHIN_THRESHOLD  0x00000002
#define STATE_UNDER_THRESHOLD   0x00000003
/* Private variables ---------------------------------------------------------*/
COMP_HandleTypeDef  hcomp1,hcomp2;
__IO uint32_t State = 0;
/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);
void COMP_Init(void);
void InputVoltageLevel_Check(void);
/********************************************************************************************************
**函数信息 ：int main(void)    
**功能描述 ：main函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main()
{
  HAL_Init();                                                           //初始化systick
  COMP_Init();
  while(1)
  {
    InputVoltageLevel_Check();                                            //输入电压检测
    if(State==STATE_OVER_THRESHOLD)
    {
      BSP_LED_Toggle(LED_GREEN);
      HAL_Delay(200);
    }
    else if(State==STATE_WITHIN_THRESHOLD)
    {
      BSP_LED_On(LED_GREEN);
    }
    else
    {
      BSP_LED_Off(LED_GREEN);
    }
  }
}
/********************************************************************************************************
**函数信息 ：void COMP_Init(void)    
**功能描述 ：比较器初始化
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void COMP_Init(void)
{
  hcomp1.Instance = COMP1;                                              //选择COM1
  hcomp1.Init.InputMinus      = COMP_INPUT_MINUS_VREFINT;               //负输入为VREF(1.2V)
  hcomp1.Init.InputPlus       = COMP_INPUT_PLUS_IO3;                    //正输入选择为PA1
  hcomp1.Init.OutputPol       = COMP_OUTPUTPOL_NONINVERTED;             //COMP1极性选择为不反向
  hcomp1.Init.Mode            = COMP_POWERMODE_HIGHSPEED;               //COMP1功耗模式选择为High speed模式
  hcomp1.Init.Hysteresis      = COMP_HYSTERESIS_DISABLE;                //迟滞功能关闭
  hcomp1.Init.WindowMode      = COMP_WINDOWMODE_COMP1_INPUT_PLUS_COMMON;//COMP1输出选择(window模式)
  hcomp1.Init.TriggerMode     = COMP_TRIGGERMODE_IT_RISING_FALLING;     //COMP1上升/下降沿中断
  if(HAL_COMP_Init(&hcomp1) != HAL_OK)                                  //COMP1初始化
  {
    Error_Handler();
  }
  HAL_COMP_Start(&hcomp1);                                              //COMP1启动  
  
  hcomp2.Instance = COMP2;                                              //选择COM2
  hcomp2.Init.InputMinus      = COMP_INPUT_MINUS_1_4VREFINT;            //负输入为VREF(0.3V)
  hcomp2.Init.InputPlus       = COMP_INPUT_PLUS_IO3;                    //正输入选择为PA3
  hcomp2.Init.OutputPol       = COMP_OUTPUTPOL_NONINVERTED;             //COMP2极性选择为不反向
  hcomp2.Init.Mode            = COMP_POWERMODE_HIGHSPEED;               //COMP2功耗模式选择为High speed模式
  hcomp2.Init.Hysteresis      = COMP_HYSTERESIS_DISABLE;                //迟滞功能关闭
  hcomp2.Init.WindowMode      = COMP_WINDOWMODE_COMP1_INPUT_PLUS_COMMON;//COMP2输出选择(window模式)
  hcomp2.Init.TriggerMode     = COMP_TRIGGERMODE_IT_RISING_FALLING;     //COMP2上升/下降沿中断
  if(HAL_COMP_Init(&hcomp2) != HAL_OK)                                  //COMP2初始化
  {
    Error_Handler();
  }
  HAL_COMP_Start(&hcomp2);                                              //COMP2启动  
}
/********************************************************************************************************
**函数信息 ：void InputVoltageLevel_Check(void)    
**功能描述 ：输入电压检测
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void InputVoltageLevel_Check(void)
{
  /* 检查COMP1和COMP2输出 */
  if (((HAL_COMP_GetOutputLevel(&hcomp1)) == COMP_OUTPUT_LEVEL_HIGH) 
   && ((HAL_COMP_GetOutputLevel(&hcomp2)) == COMP_OUTPUT_LEVEL_HIGH))
  {
    State = STATE_OVER_THRESHOLD;
  }
  else if (((HAL_COMP_GetOutputLevel(&hcomp1)) == COMP_OUTPUT_LEVEL_LOW)
       && ((HAL_COMP_GetOutputLevel(&hcomp2)) == COMP_OUTPUT_LEVEL_HIGH))
  {
    State = STATE_WITHIN_THRESHOLD;   //
  }
  else if (((HAL_COMP_GetOutputLevel(&hcomp1)) == COMP_OUTPUT_LEVEL_LOW)
       && ((HAL_COMP_GetOutputLevel(&hcomp2)) == COMP_OUTPUT_LEVEL_LOW))
  {
    State = STATE_UNDER_THRESHOLD;
  }
}
/********************************************************************************************************
**函数信息 ：HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp)
**功能描述 ：COMP执行函数
**输入参数 ：COMP_HandleTypeDef *hcomp 
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp)
{ 
  InputVoltageLevel_Check();
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
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


/* Private function -------------------------------------------------------*/



