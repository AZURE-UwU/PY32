
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//
//  文 件 名   : main.c
//  版 本 号   : v2.0
//  生成日期   : 2018-1111
//  最近修改   : 
//  功能描述   : 0.96寸ISP LCD 4接口演示例程(STM32系列)
/******************************************************************************
//本程序适用与STM32F103C8
//              GND   电源地
//              VCC   接5V或3.3v电源
//              SCL   接PA5（SCL）
//              SDA   接PA7（SDA）
//              RES   接PB0
//              DC    接PB1
//              CS    接PA4//可以直接接地
//              BLK    接P10 BLK可以悬空不接；在需要关闭背光的时候将BLK置低电平
*******************************************************************************/
// 修改历史   :
// 日    期   : 
// 修改内容   : 
//******************************************************************************/
#include "stm32f10x.h"
#include "delay.h"
#include "QDTFT_demo.h"
#include "Lcd_Driver.h"
#include "GUI.h"


int main(void)
{
u8 t;
			t=0;
  SystemInit();	//System init.
  delay_init(72);//Delay init.
				Lcd_Init();

	Lcd_Clear(WHITE);
	//	Gui_DrawFont_num(16,70, "全程技术支持");
	//Gui_DrawFont_char(20,100,'a');
 // while(t<255)
 // {  

	//	t++;
	//	LCD_ShowNum(50,100,t,3);
//		if(t==255) t=0;
 // }
  while(1)
  {  
	QDTFT_Test_Demo();	//See the test details in QDTFT_Demo.c		
  }
}





/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
