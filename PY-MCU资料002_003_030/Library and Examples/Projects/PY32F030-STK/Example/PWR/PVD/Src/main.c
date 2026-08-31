/**
  ******************************************************************************
  * @file    main.c
  * @author  MCU Application Team
  * @Version V1.0.0
  * @Date    2021-9-22
  * @brief   main function
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
void PVD_Init(void);
void LED_Init(void);

/********************************************************************************************************
**函数信息 ：int main(void)
**功能描述 ：main函数
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
int main(void)
{
  HAL_Init();                 //初始化systick
  LED_Init();                 //初始化LED(PA11)
  PVD_Init();                 //初始化PVD
  HAL_PWR_EnablePVD();        //使能PVD
  while (1)
  {
  }
}
/********************************************************************************************************
**函数信息 ：PVD_Init(void)
**功能描述 ：PVD初始化
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void PVD_Init(void)
{
  /*PWR时钟和GPIOB时钟使能*/
  GPIO_InitTypeDef  GPIO_InitStruct;
  PWR_PVDTypeDef pwr_pvdtypedef;
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*PB7初始化*/
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


  pwr_pvdtypedef.Mode = PWR_PVD_MODE_IT_RISING_FALLING;     //PVD配置为上升/下降沿中断方式
  pwr_pvdtypedef.PVDFilter = PWR_PVD_FILTER_NONE;           //滤波功能禁止
  pwr_pvdtypedef.PVDLevel = PWR_PVDLEVEL_0;                 //PB07作为检测源，此参数设置无效
  pwr_pvdtypedef.PVDSource = PWR_PVD_SOURCE_PB07;           //PVD检测为PB07
  HAL_PWR_ConfigPVD(&pwr_pvdtypedef);                       //PVD初始化
  HAL_NVIC_EnableIRQ(PVD_IRQn);
}
/********************************************************************************************************
**函数信息 ：LED_Init(void)
**功能描述 ：LED初始化(PA11)
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void LED_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  __HAL_RCC_GPIOA_CLK_ENABLE();                          //GPIOA时钟使能

  //初始化GPIOA11
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;            //推挽输出
  GPIO_InitStruct.Pull = GPIO_PULLUP;                    //使能上拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;          //GPIO速度

  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);                //GPIO初始化
}
/********************************************************************************************************
**函数信息 ：HAL_PWR_PVD_Callback(void)
**功能描述 ：PVD执行函数，当PB7上的电压低于VREF(1.2V)时，LED(PA11)亮，反之LED(PA11)灭
**输入参数 ：
**输出参数 ：
**    备注 ：
********************************************************************************************************/
void HAL_PWR_PVD_Callback(void)
{
  if (__HAL_PWR_GET_FLAG(PWR_SR_PVDO))
  {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
  }
  else
  {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
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



