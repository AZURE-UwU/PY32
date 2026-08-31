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
#include "py32f003xx_ll_Start_Kit.h"

/* Private define ------------------------------------------------------------*/
#define I2C_ADDRESS        0xA0     //本机\从机地址
#define I2C_SPEEDCLOCK     100000   //通讯速度100K
#define I2C_STATE_READY    0        //就绪状态
#define I2C_STATE_BUSY_TX  1        //发送状态
#define I2C_STATE_BUSY_RX  2        //接收状态

/* Private variables ---------------------------------------------------------*/
uint8_t aTxBuffer[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
uint8_t aRxBuffer[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

uint8_t         *pBuffPtr   = NULL;
__IO uint16_t   XferCount   = 0;
__IO uint32_t   State       = I2C_STATE_READY;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void APP_ConfigI2cSlave(void);
static void APP_SlaveTransmit_DMA(uint8_t *pData, uint16_t Size);
static void APP_SlaveReceive_DMA(uint8_t *pData, uint16_t Size);

/*******************************************************************************
**功能描述 ：执行函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
int main(void)
{
  /* 配置系统时钟 */
  SystemClock_Config();
  
  /* 配置I2C */
  APP_ConfigI2cSlave();
  
  /* 从机接收数据 */
  APP_SlaveReceive_DMA((uint8_t *)aRxBuffer, sizeof(aRxBuffer));
  
  /* 等待从机接收数据完成 */
  while (State != I2C_STATE_READY);
  
  /* 从机发送数据 */
  APP_SlaveTransmit_DMA((uint8_t *)aTxBuffer, sizeof(aTxBuffer));
  
  /* 等待从机发送数据完成 */
  while (State != I2C_STATE_READY);
  
  while (1)
  {
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
**功能描述 ：I2C配置函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
static void APP_ConfigI2cSlave(void)
{
  /* 使能 GPIOA 的外设时钟 */
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);

  /* 启用 I2C1 的外设时钟 */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
  
  /* 使能DMA时钟 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  
  /* 使能SYSCFG时钟 */
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);

  /* 将 SCL 引脚配置为：可选功能、高速、开漏、上拉 */
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_12;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* 将 SDA 引脚配置为：可选功能、高速、开漏、上拉 */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_12;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* 复位I2C */
  LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C1);
  LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C1);
  
  /* 使能I2C的NVIC中断 */
  NVIC_SetPriority(I2C1_IRQn, 0);
  NVIC_EnableIRQ(I2C1_IRQn);
  
   /* 配置DMA请求映像 */
  LL_SYSCFG_SetDMARemap_CH1(LL_SYSCFG_DMA_MAP_I2C_TX);
  LL_SYSCFG_SetDMARemap_CH2(LL_SYSCFG_DMA_MAP_I2C_RX);
  
  /* DMA通道1初始化 */
  LL_DMA_InitTypeDef DMA_InitStruct;
  DMA_InitStruct.PeriphOrM2MSrcAddress  = 0x00000000U;
  DMA_InitStruct.MemoryOrM2MDstAddress  = 0x00000000U;
  DMA_InitStruct.Direction              = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  DMA_InitStruct.Mode                   = LL_DMA_MODE_NORMAL;
  DMA_InitStruct.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStruct.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
  DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
  DMA_InitStruct.NbData                 = 0x00000000U;
  DMA_InitStruct.Priority               = LL_DMA_PRIORITY_VERYHIGH;
  LL_DMA_Init(DMA1, LL_DMA_CHANNEL_1, &DMA_InitStruct);
  
  /* DMA通道2初始化 */
  DMA_InitStruct.PeriphOrM2MSrcAddress  = 0x00000000U;
  DMA_InitStruct.MemoryOrM2MDstAddress  = 0x00000000U;
  DMA_InitStruct.Direction              = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  DMA_InitStruct.Mode                   = LL_DMA_MODE_NORMAL;
  DMA_InitStruct.PeriphOrM2MSrcIncMode  = LL_DMA_PERIPH_NOINCREMENT;
  DMA_InitStruct.MemoryOrM2MDstIncMode  = LL_DMA_MEMORY_INCREMENT;
  DMA_InitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
  DMA_InitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
  DMA_InitStruct.NbData                 = 0x00000000U;
  DMA_InitStruct.Priority               = LL_DMA_PRIORITY_HIGH;
  LL_DMA_Init(DMA1, LL_DMA_CHANNEL_2, &DMA_InitStruct);
  
  /* 使能DMA的NVIC中断 */
  NVIC_SetPriority(DMA1_Channel1_IRQn, 1);
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  
  NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0);
  NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
  
  /* I2C初始化 */
  LL_I2C_InitTypeDef I2C_InitStruct;
  I2C_InitStruct.ClockSpeed      = I2C_SPEEDCLOCK;
  I2C_InitStruct.DutyCycle       = LL_I2C_DUTYCYCLE_16_9;
  I2C_InitStruct.OwnAddress1     = I2C_ADDRESS;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_NACK;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  
  /* 启用时钟拉伸 */
  /* 复位值是启用时钟延长 */
  //LL_I2C_EnableClockStretching(I2C1);
  
  /* 启用广播呼叫 */
  /* 复位值为禁用广播呼叫 */
  //LL_I2C_EnableGeneralCall(I2C1);
}

/*******************************************************************************
**功能描述 ：I2C发送函数
**输入参数 ：pData：要发送数据指针；
**           Size：要发送数据大小
**输出参数 ：
*******************************************************************************/
static void APP_SlaveTransmit_DMA(uint8_t *pData, uint16_t Size)
{
  /* 清pos */
  LL_I2C_DisableBitPOS(I2C1);
  
  /* 将要发送数据、数据大小、状态赋给全局变量 */
  pBuffPtr    = pData;
  XferCount   = Size;
  State       = I2C_STATE_BUSY_TX;
  
  /* 使能DMA传输中断 */
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
  
  LL_DMA_ClearFlag_GI1(DMA1);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, Size);
  uint32_t dataRegAddr = LL_I2C_DMA_GetRegAddr(I2C1);
  LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_1, (uint32_t)pData, dataRegAddr, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  
  LL_DMA_DisableIT_HT(DMA1, LL_DMA_CHANNEL_1);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
  LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_1);
  
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
  
  /* 使能应答 */
  LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
  
  /* 等待ADDR标志位置位 */
  while(LL_I2C_IsActiveFlag_ADDR(I2C1) != 1);
  LL_I2C_ClearFlag_ADDR(I2C1);
  
  /* 使能I2C的EVT和ERR中断 */
  LL_I2C_EnableIT_EVT(I2C1);
  LL_I2C_EnableIT_ERR(I2C1);
  
  /* 使能DMA请求 */
  LL_I2C_EnableDMAReq_TX(I2C1);
}

/*******************************************************************************
**功能描述 ：I2C接收函数
**输入参数 ：Size：要接收数据大小
**输出参数 ：pData：要接收数据指针；
*******************************************************************************/
static void APP_SlaveReceive_DMA(uint8_t *pData, uint16_t Size)
{
  /* 清pos */
  LL_I2C_DisableBitPOS(I2C1);
  
  /* 将要发送数据、数据大小赋给全局变量 */
  pBuffPtr    = pData;
  XferCount   = Size;
  State       = I2C_STATE_BUSY_RX;
  
  /* 使能DMA传输中断 */
  LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
  
  LL_DMA_ClearFlag_GI2(DMA1);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, Size);
  uint32_t dataRegAddr = LL_I2C_DMA_GetRegAddr(I2C1);
  LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_2, dataRegAddr, (uint32_t)pData, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  
  LL_DMA_DisableIT_HT(DMA1, LL_DMA_CHANNEL_2);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
  LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);
  
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
  
  /* 使能应答 */
  LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
  
  /* 使能I2C的EVT和ERR中断 */
  LL_I2C_EnableIT_EVT(I2C1);
  LL_I2C_EnableIT_ERR(I2C1);
  
  /* 使能DMA请求 */
  LL_I2C_EnableDMAReq_RX(I2C1);
}

/*******************************************************************************
**功能描述 ：I2C中断回调函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_SlaveIRQCallback(void)
{
  /* ADDR标志位置位(用于从机接收) */
  if ((LL_I2C_IsActiveFlag_ADDR(I2C1) == 1) && (LL_I2C_IsEnabledIT_EVT(I2C1) == 1))
  {
    LL_I2C_ClearFlag_ADDR(I2C1);
  }
  /* STOPF 标志位置位(用于从机接收) */
  else if (LL_I2C_IsActiveFlag_STOP(I2C1) == 1)
  {
    LL_I2C_DisableIT_EVT(I2C1);
    LL_I2C_DisableIT_ERR(I2C1);
    
    LL_I2C_ClearFlag_STOP(I2C1);
    
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
    
    /* 数据未全部接收，接收数据失败 */
    if (XferCount != 0U)
    {
      if ((LL_I2C_IsActiveFlag_BTF(I2C1) == 1))
      {
        *pBuffPtr = LL_I2C_ReceiveData8(I2C1);
        pBuffPtr++;
        XferCount--;
      }
      
      if ((LL_I2C_IsActiveFlag_RXNE(I2C1) == 1))
      {
        *pBuffPtr = LL_I2C_ReceiveData8(I2C1);
        pBuffPtr++;
        XferCount--;
      }
      if (XferCount != 0U)
      {
        // 数据接收失败处理
      }
    }
    
    if (State == I2C_STATE_BUSY_RX)
    {
       State = I2C_STATE_READY;
    }
  }
  /* 从机发送 */
  else if (State == I2C_STATE_BUSY_TX)
  {
  }
  /* 从机接收 */
  else
  {
  }
}
/*******************************************************************************
**功能描述 ：I2C主机接收完最后一字节后，向从机发送NACK，从机NACK中断回调函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_SlaveIRQCallback_NACK(void)
{
  if ((LL_I2C_IsActiveFlag_AF(I2C1) == 1) && (LL_I2C_IsEnabledIT_ERR(I2C1) == 1))
  {
    if ((XferCount == 0) && (State == I2C_STATE_BUSY_TX))
    {
      LL_I2C_DisableIT_EVT(I2C1);
      LL_I2C_DisableIT_ERR(I2C1);
      
      LL_I2C_ClearFlag_AF(I2C1);
      
      LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
      
      State = I2C_STATE_READY;
    }
  }
}

/*******************************************************************************
**功能描述 ：DMA中断回调函数
**输入参数 ：
**输出参数 ：
*******************************************************************************/
void APP_DmaIRQCallback(void)
{
  /* 发送完成处理 */
  if (State == I2C_STATE_BUSY_TX)
  {
    if ((LL_DMA_IsActiveFlag_TC1(DMA1) == 1) && (LL_DMA_IsEnabledIT_TC(DMA1, LL_DMA_CHANNEL_1) == 1))
    {
      /* 关传输完成中断 */
      LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_1);
      LL_DMA_DisableIT_TE(DMA1, LL_DMA_CHANNEL_1);
      
      /* 清传输完成标志位 */
      LL_DMA_ClearFlag_TC1(DMA1);
      
      /* 屏蔽I2C的EVT和ERR中断 */
      LL_I2C_DisableIT_EVT(I2C1);
      LL_I2C_DisableIT_ERR(I2C1);
      
      /* 屏蔽I2C的DMA请求 */
      LL_I2C_DisableDMAReq_TX(I2C1);
      
      XferCount = 0U;
      
      /* 使能I2C的EVT和ERR中断 */
      LL_I2C_EnableIT_EVT(I2C1);
      LL_I2C_EnableIT_ERR(I2C1);
    }
  }
  /* 接收完成处理 */
  else
  {
    if ((LL_DMA_IsActiveFlag_TC2(DMA1) == 1) && (LL_DMA_IsEnabledIT_TC(DMA1, LL_DMA_CHANNEL_2) == 1))
    {
      /* 关传输完成中断 */
      LL_DMA_DisableIT_TC(DMA1, LL_DMA_CHANNEL_2);
      LL_DMA_DisableIT_TE(DMA1, LL_DMA_CHANNEL_2);
      
      /* 清传输完成标志位 */
      LL_DMA_ClearFlag_TC2(DMA1);
      
      /* 屏蔽I2C的EVT和ERR中断 */
      LL_I2C_DisableIT_EVT(I2C1);
      LL_I2C_DisableIT_ERR(I2C1);
      
      /* 屏蔽I2C的DMA请求 */
      LL_I2C_DisableDMAReq_RX(I2C1);
      
      XferCount = 0U;
      
      /* 使能I2C的EVT和ERR中断 */
      LL_I2C_EnableIT_EVT(I2C1);
      LL_I2C_EnableIT_ERR(I2C1);
    }
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
