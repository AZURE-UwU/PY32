#include "stm32f10x.h"





/* NVIC 配置：分别为 EXTI0（PA0）和 EXTI15_10（包含 PC13）配置中断 */
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* 设置 NVIC 优先级分组 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    /* -------------------- 配置 EXTI0_IRQn（用于 PA0） -------------------- */
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority   = 1;  // 高抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority          = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                  = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    /* -------------------- 配置 EXTI15_10_IRQn（用于 PC13） -------------------- */
    NVIC_InitStructure.NVIC_IRQChannel                  	 = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority   = 1;  // 可以设置为稍低的优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority          = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                  = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	    /* -------------------- 配置 TIM2_IRQn（用于 库仑计） -------------------- */
	  NVIC_InitStructure.NVIC_IRQChannel										 = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority	 = 0;  // 配置抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority					 = 1;  // 配置响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd									 = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}




/* TIM2 配置函数：设置 TIM2 以 10ms 为周期产生中断 */ //@ 库仑计计时器
void TIM2_Config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    
    
    /* 使能 TIM2 时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    /* 配置 TIM2 时间基准：
       假设系统时钟为 72MHz：
       - 预分频器设为 7200-1：72MHz/7200 = 10kHz，即计数频率为 10kHz
       - 自动重装载寄存器 (ARR) 设置为 100-1：100/10kHz = 10ms */
    TIM_TimeBaseStructure.TIM_Period = 100 - 1;         
    TIM_TimeBaseStructure.TIM_Prescaler = 7200 - 1;       
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    /* 使能 TIM2 更新中断 */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    /* 配置 NVIC：设置 TIM2 中断优先级及使能 */

    
    /* 启动 TIM2 */
    TIM_Cmd(TIM2, ENABLE);
}

// 设置 TIM3 生成 1 kHz（1 ms）中断 @ 关机计时器
void TIM3_Config(void) {
    TIM_TimeBaseInitTypeDef  tb;
    NVIC_InitTypeDef         nvic;

    // 1）使能 TIM3 时钟（APB1 总线）
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // 2）定时器参数：72 MHz / (71+1) = 1 MHz, ARR = 999 → 1 kHz
    tb.TIM_Period        = 999;
    tb.TIM_Prescaler     = 71;
    tb.TIM_ClockDivision = TIM_CKD_DIV1;
    tb.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &tb);

    // 3）使能更新中断
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    // 4）配置中断优先级并使能
    nvic.NVIC_IRQChannel                   = TIM3_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);

    // 5）启动 TIM3
    TIM_Cmd(TIM3, ENABLE);
}
