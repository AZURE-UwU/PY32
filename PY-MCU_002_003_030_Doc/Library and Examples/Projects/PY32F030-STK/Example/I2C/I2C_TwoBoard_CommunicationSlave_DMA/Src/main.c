#include "main.h"
/* Private define ------------------------------------------------------------*/
#define DARA_LENGTH       15                //数据长度
#define I2C_ADDRESS        0xA0             //本机地址0xA0
#define I2C_SPEEDCLOCK   100000             //通讯速度100K
#define I2C_DUTYCYCLE    I2C_DUTYCYCLE_2    //占空比

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef I2cHandle;
uint8_t aTxBuffer[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
uint8_t aRxBuffer[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
uint8_t I2C_receive_flag = 1; //从机接收标志位，已操作置1
uint8_t I2C_transmit_flag = 1; //从机发送标志位，已操作置1
/*I2C状态标志位，与接收和发送标志位共同操作，
  I2C_state=0并且I2C_receive_flag=1，则I2C从机DMA方式接收，
I2C_state=0并且I2C_transmit_flag=1，则I2C从机DMA方式发送*/
uint8_t I2C_state = 0;

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

  SystemClock_Config();
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

  while (1)
  {
    /*I2C从机DMA方式接收*/
    if (I2C_receive_flag == 1 && I2C_state == 0 && HAL_I2C_GetState(&I2cHandle) == HAL_I2C_STATE_READY)
    {
      I2C_receive_flag = 0;
      while (HAL_I2C_Slave_Receive_DMA(&I2cHandle, (uint8_t *)aRxBuffer, DARA_LENGTH) != HAL_OK)
      {
        Error_Handler();
      }
    }
    /*I2C从机DMA方式发送*/
    if (I2C_transmit_flag == 1 && I2C_state == 1 && HAL_I2C_GetState(&I2cHandle) == HAL_I2C_STATE_READY)
    {
      I2C_transmit_flag = 0;
      while (HAL_I2C_Slave_Transmit_DMA(&I2cHandle, (uint8_t *)aTxBuffer, DARA_LENGTH) != HAL_OK)
      {
        Error_Handler();
      }
    }


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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;                                                    //开启HSI
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;                                                    //不分频
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_4MHz;                          //配置HSI输出时钟为4MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_8MHz;                          //配置HSI输出时钟为8MHz
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_16MHz;                           //配置HSI输出时钟为16MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_22p12MHz;                      //配置HSI输出时钟为22.12MHz
  //RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_24MHz;                         //配置HSI输出时钟为24MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_OFF;                                                    //关闭HSE
  RCC_OscInitStruct.HSEFreq = RCC_HSE_16_32MHz;                                                //HSE晶振工作频率16M~32M
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;                                                    //关闭LSI
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;                                                    //关闭LSE
  RCC_OscInitStruct.LSEDriver = RCC_LSEDRIVE_MEDIUM;                                          //LSE默认驱动能力
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                                                //开启PLL
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;                                        //选择PLL源为HSE

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)                                        //初始化RCC振荡器
  {
    Error_Handler();
  }

  //初始化CPU,AHB,APB总线时钟
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1; //RCC系统时钟类型
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;                                    //SYSCLK的源选择为PLL
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
