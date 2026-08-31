#include "stm32f10x.h"
#include "Delay.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_rcc.h"
#include "misc.h"


/* 采样周期（单位：秒），此处设置为 10ms */
   



//变量在main.c中
extern volatile uint32_t SW1;
extern volatile uint32_t SW2;
extern volatile uint32_t SW1_E;
extern volatile uint32_t SW2_E;
extern volatile uint32_t SW1_2_E;
extern volatile uint8_t SW1_LongE;    // SW1 长按事件标志
extern volatile uint8_t SW2_LongE;    // SW2 长按事件标志
extern volatile uint32_t SW1_F;
extern volatile uint32_t SW2_F;
extern volatile uint32_t pressT1;    // PA0 上次按下时间戳
extern volatile uint32_t pressT2;    // PC13 上次按下时间戳
extern volatile uint32_t msTicks;
extern int mode;



extern uint8_t tim_flag;
extern uint32_t msTicks_PWoff;


/* 外部中断配置：对 PA0 和 PC13分别配置中断 */
void EXTI_Configuration(void)
{
//    EXTI_InitTypeDef EXTI_InitStructure;
//    
//    /* -------------------- 配置 PA0 -------------------- */
//    // 将 EXTI Line0 映射到 GPIOA 的 PA0
//    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
//    EXTI_InitStructure.EXTI_Line    = EXTI_Line0;
//    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
//    /* 使用上升沿：PA0 输入由低变高触发 a 的增值 */
////    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
////		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
//    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//    EXTI_Init(&EXTI_InitStructure);
//    
//    /* -------------------- 配置 PC13 -------------------- */
//    // 将 EXTI Line13 映射到 GPIOC 的 PC13
//    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);
//    EXTI_InitStructure.EXTI_Line    = EXTI_Line13;
//    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
//    /* 使用下降沿：PC13 按钮按下时，电平由高变低触发 b 的增值 */
////    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
////		EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
//    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//    EXTI_Init(&EXTI_InitStructure);
}


/* EXTI0 中断服务函数：处理 PA0 中断 */
void EXTI0_IRQHandler(void)
{		
//    if (EXTI_GetITStatus(EXTI_Line0) != RESET)
//    {
//			SW1_E = 1;
//			
//			if (SW2_E == 0)//单键按下
//			{
//				if (SW1 == 1){SW1 = 0;}
//				else {SW1 = 1;}
//			}
//			else
//			{
//				SW1_E = 0;
//				SW2_E = 0;
//				if (mode == 2){mode = 1;SW1_2_E = 1;}
//				else {mode = 2;SW1_2_E = 1;}
//			}

	
	
	
        /* 清除 EXTI Line0 的中断标志位，防止重复进入中断 */
			EXTI_ClearITPendingBit(EXTI_Line0);
//    }
}

/* EXTI15_10 中断服务函数：处理 PC13（EXTI Line13）中断 */
void EXTI15_10_IRQHandler(void)
{
	
//   if (EXTI_GetITStatus(EXTI_Line13) != RESET)   // PC13 触发
//   {
//		  SW2_E = 1;
//		 
//			if (SW1_E == 0)//单键按下
//			{		
//				if (SW2 == 1){SW2 = 3;}			
//				else{SW2 = 1;}
//			}
//			else
//			{	
//				SW2_E = 0;
//				SW1_E = 0;
//				if (mode == 2){mode = 1;SW1_2_E = 1;}
//				else {mode = 2;SW1_2_E = 1;}
//			}
//		}
	
	
	
      /* 清除 EXTI Line13 的中断标志位 */
      EXTI_ClearITPendingBit(EXTI_Line13);
}






/* TIM2 中断服务函数 */
void TIM2_IRQHandler(void)
{
    /* 检查 TIM2 更新中断标志 */
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        /* 清除中断待处理标志 */
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
				tim_flag = 1; 
        

    }
}

// TIM3 更新中断服务函数
void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update)) {
        msTicks_PWoff++;
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}
