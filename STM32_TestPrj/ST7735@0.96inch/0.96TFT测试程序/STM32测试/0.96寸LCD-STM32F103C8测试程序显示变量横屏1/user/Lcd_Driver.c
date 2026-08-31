


#include "stm32f10x.h"
#include "Lcd_Driver.h"
#include "LCD_Config.h"
#include "delay.h"

//液晶IO初始化配置
void LCD_GPIO_Init(void)
{

	GPIO_InitTypeDef  GPIO_InitStructure;
	      
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB ,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_10| GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
      
	
		RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA ,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
//向SPI总线传输一个8位数据
void  SPI_WriteData(u8 Data)
{
	unsigned char i=0;
	for(i=8;i>0;i--)
	{
		if(Data&0x80)	
	  LCD_SDA_SET; //输出数据
      else LCD_SDA_CLR;
	   
      LCD_SCL_CLR;       
      LCD_SCL_SET;
      Data<<=1; 
	}
}

//向液晶屏写一个8位指令
void Lcd_WriteIndex(u8 Index)
{
   //SPI 写命令时序开始
   LCD_RS_CLR;
	 SPI_WriteData(Index);
}

//向液晶屏写一个8位数据
void Lcd_WriteData(u8 Data)
{
   LCD_RS_SET;
   SPI_WriteData(Data); 
}
//向液晶屏写一个16位数据
void LCD_WriteData_16Bit(u16 Data)
{

   LCD_RS_SET;
	 SPI_WriteData(Data>>8); 	//写入高8位数据
	 SPI_WriteData(Data); 			//写入低8位数据

}

void Lcd_WriteReg(u8 Index,u8 Data)
{
	Lcd_WriteIndex(Index);
  Lcd_WriteData(Data);
}

void Lcd_Reset(void)
{
	LCD_RST_CLR;
	delay_ms(100);
	LCD_RST_SET;
	delay_ms(50);
}

//LCD Init For 1.44Inch LCD Panel with ST7735R.
void Lcd_Init(void)
{	
	LCD_GPIO_Init();
	Lcd_Reset(); //Reset before LCD Init.

	//LCD Init For 1.44Inch LCD Panel with ST7735R.
	Lcd_WriteIndex(0x11);//Sleep exit 
	delay_ms (120);
	Lcd_WriteIndex(0x21); 
Lcd_WriteIndex(0x21); 

Lcd_WriteIndex(0xB1); 
Lcd_WriteData(0x05);
Lcd_WriteData(0x3A);
Lcd_WriteData(0x3A);

Lcd_WriteIndex(0xB2);
Lcd_WriteData(0x05);
Lcd_WriteData(0x3A);
Lcd_WriteData(0x3A);

Lcd_WriteIndex(0xB3); 
Lcd_WriteData(0x05);  
Lcd_WriteData(0x3A);
Lcd_WriteData(0x3A);
Lcd_WriteData(0x05);
Lcd_WriteData(0x3A);
Lcd_WriteData(0x3A);

Lcd_WriteIndex(0xB4);
Lcd_WriteData(0x03);

Lcd_WriteIndex(0xC0);
Lcd_WriteData(0x62);
Lcd_WriteData(0x02);
Lcd_WriteData(0x04);

Lcd_WriteIndex(0xC1);
Lcd_WriteData(0xC0);

Lcd_WriteIndex(0xC2);
Lcd_WriteData(0x0D);
Lcd_WriteData(0x00);

Lcd_WriteIndex(0xC3);
Lcd_WriteData(0x8D);
Lcd_WriteData(0x6A);   

Lcd_WriteIndex(0xC4);
Lcd_WriteData(0x8D); 
Lcd_WriteData(0xEE); 

Lcd_WriteIndex(0xC5);  /*VCOM*/
Lcd_WriteData(0x0E);    

Lcd_WriteIndex(0xE0);
Lcd_WriteData(0x10);
Lcd_WriteData(0x0E);
Lcd_WriteData(0x02);
Lcd_WriteData(0x03);
Lcd_WriteData(0x0E);
Lcd_WriteData(0x07);
Lcd_WriteData(0x02);
Lcd_WriteData(0x07);
Lcd_WriteData(0x0A);
Lcd_WriteData(0x12);
Lcd_WriteData(0x27);
Lcd_WriteData(0x37);
Lcd_WriteData(0x00);
Lcd_WriteData(0x0D);
Lcd_WriteData(0x0E);
Lcd_WriteData(0x10);

Lcd_WriteIndex(0xE1);
Lcd_WriteData(0x10);
Lcd_WriteData(0x0E);
Lcd_WriteData(0x03);
Lcd_WriteData(0x03);
Lcd_WriteData(0x0F);
Lcd_WriteData(0x06);
Lcd_WriteData(0x02);
Lcd_WriteData(0x08);
Lcd_WriteData(0x0A);
Lcd_WriteData(0x13);
Lcd_WriteData(0x26);
Lcd_WriteData(0x36);
Lcd_WriteData(0x00);
Lcd_WriteData(0x0D);
Lcd_WriteData(0x0E);
Lcd_WriteData(0x10);

Lcd_WriteIndex(0x3A); 
Lcd_WriteData(0x05);

Lcd_WriteIndex(0x36);
Lcd_WriteData(0xA8);//

Lcd_WriteIndex(0x29); 

	 
}


/*************************************************
函数名：LCD_Set_Region
功能：设置lcd显示区域，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
*************************************************/
void Lcd_SetRegion(u16 x_start,u16 y_start,u16 x_end,u16 y_end)
{		
	Lcd_WriteIndex(0x2a);
	Lcd_WriteData(0x00);
	Lcd_WriteData(x_start+1);
	Lcd_WriteData(0x00);
	Lcd_WriteData(x_end+1);

	Lcd_WriteIndex(0x2b);
	Lcd_WriteData(0x00);
	Lcd_WriteData(y_start+0x1A);
	Lcd_WriteData(0x00);
	Lcd_WriteData(y_end+0x1A);	
	Lcd_WriteIndex(0x2c);

}

/*************************************************
函数名：LCD_Set_XY
功能：设置lcd显示起始点
入口参数：xy坐标
返回值：无
*************************************************/
void Lcd_SetXY(u16 x,u16 y)
{
  	Lcd_SetRegion(x,y,x,y);
}

	
/*************************************************
函数名：LCD_DrawPoint
功能：画一个点
入口参数：无
返回值：无
*************************************************/
void Gui_DrawPoint(u16 x,u16 y,u16 Data)
{
	Lcd_SetRegion(x,y,x+1,y+1);
	LCD_WriteData_16Bit(Data);

}    

/*****************************************
 函数功能：读TFT某一点的颜色                          
 出口参数：color  点颜色值                                 
******************************************/
unsigned int Lcd_ReadPoint(u16 x,u16 y)
{
  unsigned int Data;
  Lcd_SetXY(x,y);

  //Lcd_ReadData();//丢掉无用字节
  //Data=Lcd_ReadData();
  Lcd_WriteData(Data);
  return Data;
}
/*************************************************
函数名：Lcd_Clear
功能：全屏清屏函数
入口参数：填充颜色COLOR
返回值：无
*************************************************/
void Lcd_Clear(u16 Color)               
{	
   unsigned int i,m;
   Lcd_SetRegion(0,0,X_MAX_PIXEL-1,Y_MAX_PIXEL-1);
   Lcd_WriteIndex(0x2C);
   for(i=0;i<X_MAX_PIXEL;i++)
    for(m=0;m<Y_MAX_PIXEL;m++)
    {	
	  	LCD_WriteData_16Bit(Color);
    }   
}

