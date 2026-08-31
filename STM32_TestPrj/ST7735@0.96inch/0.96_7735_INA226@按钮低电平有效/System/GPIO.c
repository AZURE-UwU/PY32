#include "stm32f10x.h"  
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "Delay.h"

/*配置说明：

PA0		：	SW1
PC13	：	SW2

PA1		：	ADC1
PA8		：	PWM

*/

/* 长按判定阈值： 2000ms） */
#define LONG_PRESS_COUNT   1000


//变量在main.c中
extern volatile uint32_t SW1;
extern volatile uint32_t SW2;
extern volatile uint32_t SW1_F;
extern volatile uint32_t SW2_F;
extern volatile uint32_t SW1_E;
extern volatile uint32_t SW2_E;
extern volatile uint32_t SW1_2_E;
extern volatile uint8_t  SW1_LongE;
extern volatile uint8_t  SW2_LongE;
extern volatile uint32_t msTicks;
extern volatile uint32_t pressT1;
extern volatile uint32_t pressT2;
extern volatile uint32_t desiredFreq;    // 目标 PWM 频率（Hz）
extern volatile uint8_t dutyCyclePercent;  // 目标占空比（%）
extern int mode;



/* GPIO 配置：设置 PA0 为浮空输入，PC13 为上拉输入 */
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /* 配置 PA0 为浮空输入，用于 a 的自增触发（上升沿） */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // 对输入来说，速度参数无实际影响
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 配置 PC13 为浮空输入（假设未按下时为高电平，按下产生下降沿） */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
    // PB6 输入
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // PB7 推挽输出
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
}

///////////////////////////////////ADC1 @ PA1		ADC2 @ PA2///////////////////////////////////
/**
  * @brief  配置 ADC1 以及 PA1 为模拟输入
  * @param  无
  * @retval 无
  */
void ADC1_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能 GPIOA 和 ADC1 的时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

    /* 配置 PA1 为模拟输入（ADC1 channel 1） */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* ADC1 配置 */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;               // 单通道采样
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;          // 连续转换（若需要单次转换，可改为 DISABLE）
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    /* 配置 ADC1 的规则通道为 ADC_Channel_1 (对应 PA1)，采样时间可根据实际应用调整 */
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);

    /* 使能 ADC1 */
    ADC_Cmd(ADC1, ENABLE);

    /* ADC 校准步骤 */
    ADC_ResetCalibration(ADC1);                           // 重置校准寄存器
    while (ADC_GetResetCalibrationStatus(ADC1));          // 等待重置完成

    ADC_StartCalibration(ADC1);                           // 启动校准
    while (ADC_GetCalibrationStatus(ADC1));               // 等待校准完成

    /* 启动 ADC 软件转换 */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

/**
  * @brief  读取 ADC1 转换结果
  * @param  无
  * @retval 12位 ADC 数值（0~4095）
  */
uint16_t ADC1_Read(void)
{
    /* 等待转换完成 */
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    /* 返回 ADC 转换值 */
    return ADC_GetConversionValue(ADC1);
}



/**
  * @brief  配置 ADC2 以及 PA2 为模拟输入
  * @param  无
  * @retval 无
  */
void ADC2_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能 GPIOA 和 ADC2 的时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC2, ENABLE);

    /* 配置 PA2 为模拟输入（ADC2 channel 2） */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* ADC2 配置 */
    ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode       = DISABLE;                   // 单通道采样
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                    // 连续转换（若需要单次转换，可改为 DISABLE）
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel       = 1;
    ADC_Init(ADC2, &ADC_InitStructure);

    /* 配置 ADC2 的规则通道为 ADC_Channel_2 (对应 PA2)，采样时间可根据实际应用调整 */
    ADC_RegularChannelConfig(ADC2, ADC_Channel_2, 1, ADC_SampleTime_55Cycles5);

    /* 使能 ADC2 */
    ADC_Cmd(ADC2, ENABLE);

    /* ADC 校准步骤 */
    ADC_ResetCalibration(ADC2);                           // 重置校准寄存器
    while (ADC_GetResetCalibrationStatus(ADC2));          // 等待重置完成

    ADC_StartCalibration(ADC2);                           // 启动校准
    while (ADC_GetCalibrationStatus(ADC2));               // 等待校准完成

    /* 启动 ADC 软件转换 */
    ADC_SoftwareStartConvCmd(ADC2, ENABLE);
}

/**
  * @brief  读取 ADC2 转换结果
  * @param  无
  * @retval 12位 ADC 数值（0~4095）
  */
uint16_t ADC2_Read(void)
{
    /* 等待转换完成 */
    while (ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET);
    /* 返回 ADC 转换值 */
    return ADC_GetConversionValue(ADC2);
}

//////////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////PWM @ 发波//////////////////////////////////////
// 定义定时器时钟频率：
// 预分频设置为 71，则 TIM1 的计数频率为 72MHz/72 = 1MHz
#define TIMER_CLOCK    1000000UL



/**
  * @brief  初始化 TIM1 和 PA8 用于 PWM 输出
  * @note   使用 TIM1_CH1 输出 PWM 信号，PA8 配置为复用推挽输出
  */
void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    // 使能 GPIOA 和 TIM1 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);

    // 将 PA8 配置为复用推挽输出，供 TIM1_CH1 使用
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 定时器基础配置：
       设时钟 72MHz，预分频设置为 71 则计数频率为 1MHz， */
	  // 根据目标频率（单位 kHz）计算自动重装载值（ARR）
    // PWM 频率 = TIMER_CLOCK / (ARR + 1)，这里先把 desiredFreq 转为 Hz
		// 使用一个中间变量计算频率（单位 Hz）
    TIM_TimeBaseStructure.TIM_Period = (TIMER_CLOCK / (desiredFreq * 1000)) - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 71;         // 分频系数：(72MHz/ (71+1)) = 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    /* 配置 TIM1 通道1 为 PWM 输出
       PWM1 模式：输出为高电平直到计数值达到 CCR1 值后变低 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = ((TIMER_CLOCK / (desiredFreq * 1000)) * (100-dutyCyclePercent)) / 100;//设置频率
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);

    // 使能 CCR1 的寄存器预装载功能
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

    // 使能 TIM1 主输出（对于高级定时器必须配置）
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    // 使能 TIM1
    TIM_Cmd(TIM1, ENABLE);
}


/**
  * @brief  更新 PWM 参数函数，当 desiredFreq 或 dutyCyclePercent 发生改变时调用
  */
void PWM_UpdateParameters(void)
{
    // 根据新的 desiredFreq（单位 kHz）重新计算 ARR
    
    // 禁止 ARR 预装载并更新 ARR
    TIM_ARRPreloadConfig(TIM1, DISABLE);
    TIM_SetAutoreload(TIM1, (TIMER_CLOCK / (desiredFreq * 1000)) - 1);
    
    // 更新CCR（占空比）值
    uint16_t compareVal = ((TIMER_CLOCK / (desiredFreq * 1000)) * (100-dutyCyclePercent)) / 100;
    TIM_SetCompare1(TIM1, compareVal);
    
    // 重新使能 ARR 预装载
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    // 产生更新事件，使新参数立刻生效
    TIM_GenerateEvent(TIM1, TIM_EventSource_Update);
}
//////////////////////////////////////////////////////////////////////////////////////////













/////////////////////////////按钮检测/////////////////////////
void ButtonScan_ms(void)
{
    uint8_t key1 = !(GPIOA->IDR & GPIO_Pin_0) ? 1 : 0;   // PA0 低电平有效
    uint8_t key2 = !(GPIOC->IDR & GPIO_Pin_13) ? 1 : 0;  // PC13 低电平有效
	

			/*—— SW1 上升沿（按下）检测 ——*/
			if (key1 && !SW1_F)
			{
					SW1_F   = 1;               // 标记 SW1 正在按下
					pressT1 = msTicks;      // 记录按下时刻（单位：1ms）	
			}
			
			
			
					/*—— SW2 上升沿（按下）检测 ——*/
			if (key2 && !SW2_F)
			{
					SW2_F   = 1;               // 标记 SW2 正在按下
					pressT2 = msTicks;      // 记录按下时刻
			}
		
		
    /*—— SW1 下降沿（松开）检测 ——*/
    if (!key1 && SW1_F)
    {
        /* 同时按下检测：若 SW2 仍在按下，则视为同时短按 */
        if (SW2_F && SW1_F)
        {
						SW1_F   = 0;
						SW2_F   = 0;			
					
            SW1_E   = 0;
            SW2_E   = 0;
            SW1_2_E = 1;
						if (mode == 2){mode = 1;}else {mode = 2;}
						while (key1 == 1 || key2 == 1)
						{
						key1 = (GPIOA->IDR & GPIO_Pin_0) ? 1 : 0;   // PA0 高电平有效
						key2 = (GPIOC->IDR & GPIO_Pin_13) ? 1 : 0;  // PC13 高电平有效
						}
					

        }
        else
        {
            uint32_t dt = msTicks - pressT1;
            if (dt >= LONG_PRESS_COUNT)
            {
                /* SW1 长按 */
                SW1_LongE = 1;
								SW1_F   = 0;
								SW2_F   = 0;	
            }
            else
            {
                /* SW1 单独短按：0 ↔ 1 切换 */
                
								SW1_F   = 0;
								SW2_F   = 0;
							
								SW1_E   = 1;
								SW2_E   = 0;
								SW1_2_E = 0;
								SW1 = (SW1 == 1) ? 0 : 1;
            }
        }
    }


    /*—— SW2 下降沿（松开）检测 ——*/
    if (!key2 && SW2_F)
    {
        /* 同时按下检测：若 SW1 仍在按下，则视为同时短按 */
        if (SW2_F && SW1_F) 
        {
						SW1_F   = 0;
						SW2_F   = 0;
					
            SW1_E   = 0;
            SW2_E   = 0;
            SW1_2_E = 1;
					
						if (mode == 2){mode = 1;}else {mode = 2;}
						
						while (key1 == 1 || key2 == 1)
						{
						key1 = (GPIOA->IDR & GPIO_Pin_0) ? 1 : 0;   // PA0 高电平有效
						key2 = (GPIOC->IDR & GPIO_Pin_13) ? 1 : 0;  // PC13 高电平有效
						}
        }
        else
        {
            uint32_t dt = msTicks - pressT2;
            if (dt >= LONG_PRESS_COUNT)
            {
                /* SW2 长按 */
                SW2_LongE = 1;
								SW1_F   = 0;
								SW2_F   = 0;	
            }
            else
            {
                /* SW2 单独短按：1 ↔ 3 切换 */
							
								SW1_F   = 0;
								SW2_F   = 0;	
							
								SW1_E   = 0;
								SW2_E   = 1;
								SW1_2_E = 0;
								SW2 = (SW2 == 1) ? 3 : 1;
            }
        }
    }
}
/////////////////////////////////////////////////////////////////////////////////////
