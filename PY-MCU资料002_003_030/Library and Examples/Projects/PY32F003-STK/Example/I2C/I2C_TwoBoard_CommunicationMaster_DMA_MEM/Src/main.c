#include "main.h"
/* Private define ------------------------------------------------------------*/
#define DARA_LENGTH      15                 //数据长度
#define I2C_ADDRESS      0xA0               //本机地址0xA0
#define I2C_SPEEDCLOCK   100000             //通讯速度100K
#define I2C_DUTYCYCLE    I2C_DUTYCYCLE_16_9 //占空比

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef I2cHandle;
uint8_t aTxBuffer[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
uint8_t aRxBuffer[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

/* Private function prototypes -----------------------------------------------*/
/* Private user code ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
  HAL_Init();

//  SystemClock_Config();
  I2cHandle.Instance             = I2C;                                                                   //I2C
  I2cHandle.Init.ClockSpeed      = I2C_SPEEDCLOCK;                                                        //I2C通讯速度
  I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE;                                                         //I2C占空比
  I2cHandle.Init.OwnAddress1     = I2C_ADDRESS;                                                           //I2C地址
  I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;                                               //禁止广播呼叫
  I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;                                                 //允许时钟延长

  if (HAL_I2C_Init(&I2cHandle) != HAL_OK)                                                                 //I2C初始化
  {
    Error_Handler();
  }
  //等待用户按键按下，主机程序开始运行
  while (BSP_PB_GetState(BUTTON_KEY) == 1)
  {
  }

  /*I2C主机DMA方式发送*/
  while (HAL_I2C_Mem_Write_DMA(&I2cHandle, I2C_ADDRESS, 0x00, I2C_MEMADD_SIZE_16BIT, (uint8_t *)aTxBuffer, DARA_LENGTH) != HAL_OK)
  {
    Error_Handler();
  }
  /*判断当前I2C状态*/
  while (HAL_I2C_GetState(&I2cHandle) != HAL_I2C_STATE_READY);
  HAL_Delay(100);//100
  while (HAL_I2C_Mem_Read_DMA(&I2cHandle, I2C_ADDRESS, 0x00, I2C_MEMADD_SIZE_16BIT, (uint8_t *)aRxBuffer, DARA_LENGTH) != HAL_OK)
  {
    Error_Handler();
  }
  while (1)
  {
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

  /*配置时钟源HSE/HSI/LSE/LSI*/
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                    //开启HSI
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                                                    //不分频
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_4MHz;                          //配置HSI输出时钟为4MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_8MHz;                          //配置HSI输出时钟为8MHz
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz;                           //配置HSI输出时钟为16MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_22p12MHz;                      //配置HSI输出时钟为22.12MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;                         //配置HSI输出时钟为24MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;                                                    //关闭HSE
  RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;                                                //HSE晶振工作频率16M~32M
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;                                                    //关闭LSI


  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)                                        //初始化RCC振荡器
  {
    Error_Handler();
  }

  //初始化CPU,AHB,APB总线时钟
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; //RCC系统时钟类型
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;                                    //SYSCLK的源选择为PLL
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;                                          //APH时钟不分频
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;                                            //APB时钟不分频

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)                      //初始化RCC系统时钟(FLASH_LATENCY_0=24M以下;FLASH_LATENCY_1=48M)
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
