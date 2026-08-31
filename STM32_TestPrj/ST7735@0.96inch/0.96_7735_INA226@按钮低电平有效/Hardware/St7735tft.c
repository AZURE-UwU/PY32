#include "stm32f10x.h"
#include "Delay.h" 
#include "string.h"  //strlen
#include "math.h"     //pow

#include "St7735tft.h"

extern const unsigned char asc[];           //汉字字模数组
extern const unsigned char chinese_font[];  //ASCII字模数组
extern const char font_sample[];            //汉字数组


/*
功能：ST7735引脚初始化
参数：-
解释：全设为推免输出
*/
void Spi_Init(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_BDR, ENABLE);//问题1：不同端口，分别开多次
	GPIO_InitStructure.GPIO_Pin = BLK_PIN|DC_PIN|RST_PIN;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推免
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(BDR_PORT, &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_CDS, ENABLE);
	GPIO_InitStructure.GPIO_Pin =  SCL_PIN|SDA_PIN| CS_PIN;//问题2：涉及到的引脚要全部打开
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       //BLK，DC,DC 皆为推免
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(CDS_PORT, &GPIO_InitStructure);

}


/*
功能：向SPI总线发送一个8位数据
参数：8位数据
解释：为了方便后续发送指令 或 数据 代码的编写，本函数不涉及CS与DC的选中，只随CLK跳变发送数据
*/
void Spi_SendData(uint8_t data){
	for(int i=0;i<8;i++){
		
		SCL_L;         //时钟沿0-1，因为ST7735的 CPOL = 0，CPHA = 0
		//Delay_us(1); //控制传输速率
		
		if(data&0x80)  //注意，若用16位数据&0x80，只会取16位的后8位
			 SDA_H;
		else
			 SDA_L;
		
		SCL_H; 
		//Delay_us(1);  //控制传输速率
		data<<=1;
	}
}


/*
功能：向ST7735发送一个8位指令
参数：8位数据
解释：发送指令需要将DC拉低
*/
void TFT_SendIndex(uint8_t reg){
	CS_L;
	DC_L;//DC为低，表传递的是命令
	Spi_SendData(reg);
	CS_H;
}

/*
功能：向ST7735发送一个8位数据
参数：8位数据
解释：发送是数据时，需要将DC拉高
*/
void TFT_SendData(u8 Data)
{
   CS_L;
   DC_H;//DC为高，表传递的是数据
   Spi_SendData(Data);
   CS_H;
}



/*
功能：向ST7735发送一个16位数据
参数：16位数据
解释：分两次，先发高8位，再发低8位
*/
void TFT_Send16Bit(uint16_t Data){
		CS_L;
	  DC_H;//DC高位 表示数据
	  Spi_SendData(Data>>8); //前八位，因为与0x80作&运算，只能操作16位的后8位
	  Spi_SendData(Data);   //后八位
		CS_H;
}

/*
功能：向ST7735发送：指令 + 数据
参数：8位指令，8位数据
解释：调用封装好的函数，包含两次对DC的设置
*/
void TFT_SendReg(uint8_t adress,uint8_t data){
	TFT_SendIndex(adress);
	TFT_SendData(data);
}



/* 
功能：初始化驱动地址
参数：-
解释：全地址注释，128*160 TFT ST7735S驱动，对初始模式设定
      【加中括号为常用寄存器】需要注意
*/
void TFT_Init(void){
	
	Spi_Init();  //初始化SPI引脚
	TFT_Reset(); //芯片复位，RST引脚变化

	TFT_SendIndex(0x11); //【唤醒显示屏】从睡眠模式恢复到正常工作模式 
	Delay_ms (120);      //必要延时

  //----------------------基本配置---------------------------------
	TFT_SendIndex(0x36);        //【数据访问方式】，RGB/BGR,行列读写方向与水平垂直刷新等
	TFT_SendData(0x00); 	      //注：172行进行了详细原理注释，0x00非最终设置
	
	TFT_SendIndex(0x3A);        //【RGB图像数据格式】，后3位设定          
  TFT_SendData(0x05);         //3: 12bit;  5: 16 bit； 6: 18bit ； 7: 未使用
	
	TFT_SendIndex(0xB1);        //帧率控制（正常模式/全彩） 后跟3个数据
	TFT_SendData(0x05);
	TFT_SendData(0x3C);
	TFT_SendData(0x3C); 

	TFT_SendIndex(0xB2);        //帧率控制（空闲模式/ 8色） 后跟3个数据
	TFT_SendData(0x05);
	TFT_SendData(0x3C);
	TFT_SendData(0x3C); 

	TFT_SendIndex(0xB3);       //帧率控制（部分模式/全彩）后跟6个数据
	TFT_SendData(0x05);        //1-3个参数 点反转
	TFT_SendData(0x3C);        
	TFT_SendData(0x3C); 
	TFT_SendData(0x05);        //4-6个参数 列反转
	TFT_SendData(0x3C); 
	TFT_SendData(0x3C); 
	
	TFT_SendIndex(0xB4);      //显示反转控制，D0-D3位有效，分别对应不同模式
	TFT_SendData(0x03);      
	
	//------------------电源控制寄存器1-5------------------
	TFT_SendIndex(0xC0);    //特定颜色模式下的电压参数,调整显示屏的亮度、对比度等显示效果
	TFT_SendData(0x2E);       
	TFT_SendData(0x06); 
	TFT_SendData(0x04); 
	
	TFT_SendIndex(0xC1);   //C1-C4功能同0xC1,更精细的电压调整，以达到更好的视觉效果。
	TFT_SendData(0xC0);
  TFT_SendData(0xC2);	

	TFT_SendIndex(0xC2);   //略
	TFT_SendData(0x0D); 
	TFT_SendData(0x0D); 

	TFT_SendIndex(0xC3);   //略
	TFT_SendData(0x8D); 
	TFT_SendData(0xEE);
	
	TFT_SendIndex(0xC4);   //略
	TFT_SendData(0x8D); 
	TFT_SendData(0xEE); 
	
	TFT_SendIndex(0xC5);  //设置显示屏的VCOM 电压，即显示屏公共电极的电压
	TFT_SendData(0x00);   //影响整体显示效果，调整亮度均匀，减少色彩失真
	
	//---------------------数据显示方式（重要）-----------------
	TFT_SendIndex(0x36); //【数据显示方式格式详解】与屏幕方向息息相关，以0xC0 为例，
	TFT_SendData(0xC0);   //   MY行顺序    MX列顺序    MV行列转换   ML垂直刷新  RGB/BGR   MH水平刷新   -   - 
                        //      1          1            0           0          0           0       0   0
											  //    上至下      左至右        否         关闭         RGB        关闭
											  //设置方向函数，只需要调整  MY，MX, MV 的值即可
											 
	
	//-----------------------伽马序列------------------------
	                      //出厂已调好，一般无需额外调整
	TFT_SendIndex(0xe0);  //伽马极性校正设置，后跟16个8位数据
	TFT_SendData(0x1B);   //涉及高,中，低三个等级调整，有效地址：D0-D5位
	TFT_SendData(0x21);	 
	TFT_SendData(0x10);   //可使屏幕亮度更符合人眼的感知特性，减少亮度失真导致的视觉疲劳
	TFT_SendData(0x15);   //优化色彩准确性,准确地显示出各种颜色
	TFT_SendData(0x2B);   //增强暗部细节,改善在低灰阶显示效果
	TFT_SendData(0x25);	
	TFT_SendData(0x1F); 
	TFT_SendData(0x23); 
	TFT_SendData(0x22); 
	TFT_SendData(0x22); 
	TFT_SendData(0x2B); 
	TFT_SendData(0x37);
	TFT_SendData(0x00); 	
	TFT_SendData(0x15); 
	TFT_SendData(0x02); 
	TFT_SendData(0x3F); 	

	TFT_SendIndex(0xE1);    //同E0，略
	TFT_SendData(0x1A); 
	TFT_SendData(0x20); 
	TFT_SendData(0x0F); 
	TFT_SendData(0x15); 
	TFT_SendData(0x2A); 
	TFT_SendData(0x25); 
	TFT_SendData(0x1E);
	TFT_SendData(0x23); 
	TFT_SendData(0x23); 
	TFT_SendData(0x22); 
	TFT_SendData(0x2B); 
	TFT_SendData(0x37); 
	TFT_SendData(0x00); 
	TFT_SendData(0x15); 
	TFT_SendData(0x02); 
	TFT_SendData(0x3F);  
	
	//---------------------自定补充操作------------------
	TFT_SendIndex(0x2C);  // 【0x2c作用1】初始化设置时，配置显示参数
	TFT_SendIndex(0x21); //【打开颜色反转】正常使用无需打开
	TFT_SendIndex(0x29);  //【打开屏幕】，0x28为关闭屏幕
	TFT_Clear(BLACK);    //初始清屏
}


/* 
功能：设定屏幕旋转方向
参数：0-3
解释：locate：0 1 2 3 旋转 locate*90°,以右侧为x轴正方向
     【173行】见0x36详细注释，如何设置方向的根本原因
*/
void TFT_SpinScreen(uint8_t locate){
	TFT_SendIndex(0x36);      //屏幕的显示方向、像素读写顺序
	if(locate==0) TFT_SendData(0xC0); 	  //纵向，左上角（0，0） 
	if(locate==1) TFT_SendData(0xA0);     //右转90°  横向
	if(locate==2) TFT_SendData(0x00);     //右转180°  纵向
	if(locate==3) TFT_SendData(0x60);     //右转270° 横向
	
	/*
	若：通过设置MV，可交令xy轴互换（即寄存器Mv地址设1）
	    这样即可得到全部的8个坐标系。
	*/
}




/*
功能：打开，关闭背光，效果位等同与熄屏
参数：控制背光BLK，0关-1开
解释：控制寄存器BLK引脚高低，需要BLK引脚接B10,如不接BLK或接3.3，则无法使用
*/
void TFT_TurnOff(uint8_t io){
	if(io) BLK_H;
	else BLK_L;
}


/*
功能：选中一个矩形区域[x,y]-[x1,y1] （两点重合时，只会选中一个点的区域）
入口参数：[x,y]-[x1,y1] ,定位范围【0,0】-【127,159】 超过(127,159)等同(127,159)
解释：调用该函数后，之后再写入 数据，将会覆盖写入，并且会自动换行。
*/
void TFT_SetRegion(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end)
{		                     
	TFT_SendIndex(0x2a);  //设置列地址范围，命令（0x2a）+数据（4字节数据）
	TFT_SendData(0x00);    //x范围:（0，127），含边界0 127
	TFT_SendData(x_start);
	TFT_SendData(0x00);
	TFT_SendData(x_end);
                    
	TFT_SendIndex(0x2b);//设置行地址范围 命令（0x2b）+数据（4字节数据）
	TFT_SendData(0x00); //y范围:（0，159），含边界0 127
	TFT_SendData(y_start);
	TFT_SendData(0x00);
	TFT_SendData(y_end);
	
	TFT_SendIndex(0x2c); //【0x2c作用2】确认将像素数据写入显存，每次设置完0x2a 2x2b后，需调用0x2c
}                      //不确认将无法写入数据


/*
功能：全屏填色函数
入口参数：填充颜色COLOR, 
解释：选中全屏区域，然后发送128*160个16位颜色数据，点亮全屏像素点，即（0，0）-（127，159）
      TFT_SetRegion选中好区域后，寄存器设置会自动换行，就像把海绵球倒入容器一样
*/
void TFT_Clear(uint16_t color){
	unsigned int i,m;
   TFT_SetRegion(0,0,160,160); //定位全屏
   TFT_SendIndex(0x2C);       //确认操作，不确认无法写入
   for(i=0;i<128;i++)
    for(m=0;m<160;m++){	
	  	TFT_Send16Bit(color);
    }  
}

/*
功能：选中区域并用颜色填充
入口参数：区域坐标(x1,y1)-(x2,y2),颜色COLOR  范围：（0，0）-（127，159）
解释：选中一个矩形区域(也可以是一个点)，发送和该区域 像素点数量 相等的16位颜色数据，充满它。
*/
void TFT_FullScreen(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color){
	TFT_SetRegion(x1,y1,x2,y2);//设置区域(0,0)-(127,159)
	
	//写入对应数量的点
	int t=(x2-x1+1)*(y2-y1+1);
	while(t--)
	  	TFT_Send16Bit(color);
	
	TFT_SetRegion(0,0,127,159);//恢复定位全屏
}

/*
功能：设置TFT显示起始点（本质就是选中一个点的区域）
入口参数：xy坐标（0，0）-（127，159）
解释：本质就是将显示区域，缩小在一个点，起到一个定位坐标的作用.后续若不重新定区域，则只能在这一个坐标内画点。
      后续简化 字符，中文输出函数时，用来搭配使用。
*/
void TFT_SetCursor(uint16_t x,uint16_t y){
	TFT_SetRegion(x,y,x,y);
}

/*
功能：画一个点
入口参数：点的坐标（x,y），颜色
解释：因为就一个点，所以用TFT_SetRegion，或TFT_SetCursor都可以，
      前者只要起点对，哪怕终点定位后区域超过了一个点，效果也是一样。
      --其实TFF屏，本质也还是点阵屏，数码管，没什么区别,只是LED换成像素点而已。
      --理解 如何点亮一个点，更容易明白 后续的 线，图形，图片，字符，中文汉字。
*/
void TFT_DrawPoint(uint16_t x,uint16_t y,uint16_t Data){
	//TFT_SetRegion(x,y,x+1,y+1);
	TFT_SetCursor(x,y);
	TFT_Send16Bit(Data);//只写入一个数据，也就是一个点
}


/*
功能：将TFT进行复位，恢复到初始状态
参数：-
解释：【低电平】 触发 ST7735S 芯片的复位操作；
      【高电平】 芯片处于正常工作状态，可对芯片进行各种操作
      如发送命令和数据，控制内容、颜色、亮度等参数。
      小心使用，当心无限重启
*/
void TFT_Reset(void)
{
	RST_L;    //芯片复位
	Delay_ms(100);
	RST_H;   //启动
	Delay_ms(50);
}

//------------图形绘制--------------
/*
功能：画圆
参数：圆心（x,y）半径R, 颜色
解释：略,用处不大
*/
void TFT_DrawCircle(uint16_t X,uint16_t Y,uint16_t R,u16 fc){
	{//Bresenham算法 
    unsigned short  a=0,b=R; 
    int c=3-2*R; 
    while (a<b){ 
        TFT_DrawPoint(X+a,Y+b,fc);     //        7 
        TFT_DrawPoint(X-a,Y+b,fc);     //        6 
        TFT_DrawPoint(X+a,Y-b,fc);     //        2 
        TFT_DrawPoint(X-a,Y-b,fc);     //        3 
        TFT_DrawPoint(X+b,Y+a,fc);     //        8 
        TFT_DrawPoint(X-b,Y+a,fc);     //        5 
        TFT_DrawPoint(X+b,Y-a,fc);     //        1 
        TFT_DrawPoint(X-b,Y-a,fc);     //        4 

        if(c<0) c=c+4*a+6; 
        else{ 
            c=c+4*(a-b)+10; 
            b-=1; 
        } 
       a+=1; 
    }if (a==b){ 
        TFT_DrawPoint(X+a,Y+b,fc); 
        TFT_DrawPoint(X+a,Y+b,fc); 
        TFT_DrawPoint(X+a,Y-b,fc); 
        TFT_DrawPoint(X-a,Y-b,fc); 
        TFT_DrawPoint(X+b,Y+a,fc); 
        TFT_DrawPoint(X-b,Y+a,fc); 
        TFT_DrawPoint(X+b,Y-a,fc); 
        TFT_DrawPoint(X-b,Y-a,fc); 
    } 
  } 
}


/*
功能：画线
参数：两点（x,y）（x2,y2）, 颜色 
解释：略,用处不大。 点范围 x （0，127）  y（0, 159）
*/
void TFT_DrawLine(uint16_t x0, uint16_t y0,uint16_t x1, uint16_t y1,uint16_t Color){
	int dx,            // difference in x's
    dy,             // difference in y's
    dx2,            // dx,dy * 2
    dy2, 
    x_inc,          // amount in pixel space to move during drawing
    y_inc,          // amount in pixel space to move during drawing
    error,          // the discriminant i.e. error i.e. decision variable
    index;          // used for looping	

	TFT_SetCursor(x0,y0);
	dx = x1-x0;//计算x距离
	dy = y1-y0;//计算y距离

	if (dx>=0) x_inc = 1; 
	else{
		x_inc = -1;
		dx    = -dx;  
	} 
	
	if (dy>=0) y_inc = 1; 
	else{
		y_inc = -1;
		dy    = -dy; 
	} 
	dx2 = dx << 1;
	dy2 = dy << 1;
	if (dx > dy) //x距离大于y距离，那么每个x轴上只有一个点，每个y轴上有若干个点
	{           //且线的点数等于x距离，以x轴递增画点
		           // initialize error term
		error = dy2 - dx; 
		// draw the line
		for (index=0; index <= dx; index++){ //要画的点数不会超过x距离{
			//画点
			TFT_DrawPoint(x0,y0,Color);
			
			// test if error has overflowed
			if (error >= 0) //是否需要增加y坐标值
			{
				error-=dx2;

				// move to next line
				y0+=y_inc;//增加y坐标值
			} // end if error overflowed

			// adjust the error term
			error+=dy2;

			// move to the next pixel
			x0+=x_inc;//x坐标值每次画点后都递增1
		} // end for
	} // end if |slope| <= 1
	else//y轴大于x轴，则每个y轴上只有一个点，x轴若干个点
	{//以y轴为递增画点
		// initialize error term
		error = dx2 - dy; 

		// draw the line
		for (index=0; index <= dy; index++)
		{
			// set the pixel
			TFT_DrawPoint(x0,y0,Color);

			// test if error overflowed
			if (error >= 0){
				error-=dy2;

				// move to next line
				x0+=x_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dx2;

			// move to the next pixel
			y0+=y_inc;
		} // end for
	} // end else |slope| > 1
}

/*
功能：画矩形
参数：矩形左上角坐标（x,y）+矩形长宽 w h +颜色
解释：点范围（0，0）-(127,159)
*/
void TFT_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h,uint16_t bc){
	TFT_DrawLine(x,y,x+w,y,bc);
	TFT_DrawLine(x+w,y,x+w,y+h,bc);
	TFT_DrawLine(x,y+h,x+w,y+h,bc);
	TFT_DrawLine(x,y,x,y+h,bc);
  //TFT_DrawLine(x+1,y+1,x+1+w-2,y+1+h-2,bc); //对角线
}


void TFT_filledBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bc) 
{
    for (uint16_t i = 0; i < h; i++) {
        TFT_DrawLine(x, y + i, x + w, y + i, bc);
    }
}
/*
功能：画矩形(有预设方案)
参数：左上角坐标（x,y）+矩形长宽 w h + 预设方案
解释：预设颜色 123，后续自定义
*/
void TFT_box2(uint16_t x,uint16_t y,uint16_t w,uint16_t h, u8 mode){
	if (mode==1)	{  //白灰
		TFT_DrawLine(x,y,x+w,y,0xEF7D);
		TFT_DrawLine(x+w,y,x+w,y+h,0x2965);
		TFT_DrawLine(x,y+h,x+w,y+h,0x2965);
		TFT_DrawLine(x,y,x,y+h,0xEF7D);
		}
	if (mode==2)	{  //红黄
		TFT_DrawLine(x,y,x+w,y,RED);
		TFT_DrawLine(x+w,y,x+w,y+h,YELLOW);
		TFT_DrawLine(x,y+h,x+w,y+h,YELLOW);
		TFT_DrawLine(x,y,x,y+h,RED);
	}
	if (mode==0)	{  //绿灰
		TFT_DrawLine(x,y,x+w,y,GREEN);
		TFT_DrawLine(x+w,y,x+w,y+h,PINK);
		TFT_DrawLine(x,y+h,x+w,y+h,PINK);
		TFT_DrawLine(x,y,x,y+h,GREEN);
	}
	if (mode==3)	{   //白灰2
		TFT_DrawLine(x,y,x+w,y,WHITE);
		TFT_DrawLine(x+w,y,x+w,y+h,GRAY0);
		TFT_DrawLine(x,y+h,x+w,y+h,GRAY0);
		TFT_DrawLine(x,y,x,y+h,WHITE);
	}
}

/*
功能：按钮状态1
参数：按钮框左上角和右下角坐标
解释：不同颜色搭配使用，凑出动态效果
*/
void ButtonDown(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
	TFT_DrawLine(x1,  y1,  x2, y1, GRAY2);  //H
	TFT_DrawLine(x1,  y1,  x2, y1, GRAY1);  //H
	TFT_DrawLine(x1,  y1,  x1, y2, GRAY2);  //V
	TFT_DrawLine(x1,  y1,  x1, y2, GRAY1);  //V
	TFT_DrawLine(x1,  y2,  x2, y2, WHITE);  //H
	TFT_DrawLine(x2,  y1,  x2, y2, WHITE);  //V
}

/*
功能：按钮状态2
参数：按钮框左上角和右下角坐标
解释：不同颜色搭配使用，凑出动态效果
*/
void ButtonUp(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2){
	TFT_DrawLine(x1,  y1,  x2,y1, WHITE); //H
	TFT_DrawLine(x1,  y1,  x1,y2, WHITE); //V
	TFT_DrawLine(x1  ,y2  ,x2,y2  , GRAY1);  //H
	TFT_DrawLine(x1,  y2,  x2,y2, GRAY2);  //H
	TFT_DrawLine(x2  ,y1  ,x2  ,y2, GRAY1);  //V
  TFT_DrawLine(x2  ,y1  ,x2,y2, GRAY2); //V
}

//--------------字，图，字符，中文-----------------
/*
功能：显示图片
参数：图片左上角坐标，图长，图宽      （注意长宽不是坐标，128*160的图就填128 160，而非127，159 ）
      这里仅利用长宽计算图像像素点的个数，而非定位。哪怕少1，图像也会显示出错。

解释：定位区域后，然后将若干像素点输出即可，会自动换行。
      一个像素点占16位，需要2个unsigned chara数组的位置，输出像素点时，对连续2个元素进行位运算合成
      【注意】图像取模时，可勾选 低位或高为在前，但不论怎样，生成的数组元素都是8位大小，都需要再手动合成16位
       调整成16位、按SPI要求的、高位在前的、数据再进行传输
*/
void TFT_ShowImage(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const unsigned char *p){
	TFT_SetRegion(x,y,x+length-1,y+width-1);//定位区域
	unsigned char picH,picL;  
	for(int i=0;i<length*width;i++){  //length*width表示像素点的个数，数组下标表示存下这些颜色的点需要多少8位元素
    picL=p[2*i];
		picH=p[2*i+1]; //2个元素记录一个颜色点
		TFT_Send16Bit(picH<<8|picL);      //图像取模时若选则：低位在前（默认一般为此）
		//TFT_Send16Bit(picL<<8|picH);    //图像取模时若选则：高位在前
	}
	TFT_SetRegion(0,0,127,159);//恢复窗口
}


/*
功能：仅输出字符
参数：坐标，字体颜色，背景颜色，字符内容
解释：字模数组，一个8位数据元素，用于控制8个像素点的亮灭，仅为01，与图片时的颜色数据无关，不要理解混了
      这里其实很类似于点阵屏，只不过把LEd 替换成 像素点就可以，同样由二进制01控制打开关闭
      字符像素大小：8*16 宽8高， 一个8位数组元素，点亮一行，共16行，也就是16个数组元素记录一个字符
*/
void TFT_ShowChar(uint8_t x,uint8_t y,uint16_t fc,uint16_t bc,char c){
		int k=(c-32)*16;   //定位，找到该字符在数组中的起始下标。    映射关系推导见Font_asc.h注释
		for (int i = 0; i < 16; i++)         //行16  字模高度
       for (int j = 0; j < 8; j++) {      //列8   字模宽度
           if (asc[k+i]&(0x80>>j))
              TFT_DrawPoint(x+j,y+i,fc); //点亮不同位置的点，亮与不亮的点，构成字符外观
           else
               TFT_DrawPoint(x+j,y+i,bc);   
        }
}

/*
功能：输出字符串
参数：坐标，字体颜色，背景颜色，字符串内容
解释：遍历字符串，一位一位输出字符即可
*/
void TFT_ShowString(uint8_t x,uint8_t y,uint16_t fc,uint16_t bc,char *c){
	int t=strlen(c);
	//OLED_ShowNum(3,1,t,4);  //调试，显示字符串长度
	for(int i=0;i<t;i++){
//		if(x>=128){x=0;y+=16;}        //x轴越界自动换行，字符高度16，不用TFT_SetRegion，就手搓换行规则
		TFT_ShowChar(x,y,fc,bc,c[i]);
		x+=8;     //字符的宽度为8 
	}		
}

/*
功能：输出数字 或 整数变量
参数：坐标，数字颜色，背景颜色，数字值（可输出负数）
范围：正不超过12位（千亿），负数不超过11位（百亿）
解释：把数字用模运算得出每一位，再转为字符串，然后使用字符串输出即可
*/
void TFT_ShowNumber(uint8_t x,uint8_t y,uint16_t fc,uint16_t bc,long long  num){
	uint8_t k=0;  
	char s[20];       //把整数化为字符串存储输出
	long long t=num;  //中间值
	while(t){
		t/=10;
		k++;           //num的十进制位数
	}
	if(num<0){
		s[0]='-';
	  s[k+1]= '\0';
    num*=-1;     //后续有字符参与运算，必须让num以正值运算
	}else{
		s[k]= '\0';  //定好结尾，防止后续使用strlen函数时边界判断出错
		k-=1;         //从个位开始时，存入末位元素
	}
	while(num){
		s[k--]= '0'+ num%10;  //低位在字符串末尾 （！！！num必须为正值，有字符运算禁止出现负数，此乃大忌）
		num/=10;
	}
	TFT_ShowString(x,y,fc,bc,s);
}

/*
功能：输出中文,（支持中文与字符串混合）
参数：坐标，文字颜色，背景颜色，中文字符串
解释：本项目输出汉字需要注意两点：
          1 【更新字模数组：chinese_font[]】 2【按字模的顺序，在font_sample[]中，输入所有的汉字】
      前者存储了，点亮一个16*16像素大小汉字，16*16个像素点的 亮灭信息；
      后则用于计算，需要显示的汉字，在字模数组下标中的位置，即最下面双层for暴力模拟的map容器
      
      *因为支持混合输出，故遇到字符，重写了TFT_ShowChar操作
       遇到汉字： 1 单个汉字存入数组  2 找到汉字在字模数组中的初始下标，详见封装的map方法
                 3 输出汉字，注意汉字像素宽度是16*16，每2个连续的8位数组元素点一行i像素点

特殊：项目编译选项：
	    魔术棒，c/c++选项卡，Mic control,加入：--no-multibyte-chars ，确定
	    这个选项告诉编译器不要处理多字节字符，从而避免中文字符串引起的问题

*/
void TFT_ShowChinese(uint8_t x,uint8_t y,uint16_t fc,uint16_t bc,char *c){
  int t=strlen(c);
	for(int n=0;n<t;n++)  //遍历字符串，中/字符，判断后分别输出
	{
	  //遇到字符	
		if(c[n]>31&&c[n]<127){
			if(x+8>=128){ //自动换行
				x=0;y+=16;
			}
		  TFT_ShowChar(x,y,fc,bc,c[n]);
		  x+=8; 
			continue; //能少写一个else{}
		}
		//遇到汉字
		char tem[4];
		tem[0]=c[n];tem[1]=c[n+1];tem[2]=c[n+2];tem[3]='\0';     //3位都相同确认一个汉字，2位不行
		int k=map(tem); //获取汉字对应的初始数组下标

		/* debug 
		//@@@(2) 录入map中文字符ascii
		TFT_ShowString(0,96,RED,BLACK,"Ouput: map");
		TFT_ShowNumber(0,112,WHITE,BLACK,c[n]);
		TFT_ShowNumber(30,112,WHITE,BLACK,c[n+1]);
		TFT_ShowNumber(60,112,WHITE,BLACK,c[n+2]);
		//@@ 返回结果，是字库第几个汉字，从0开始
		TFT_ShowString(0,48,RED,BLACK,"k=");
		TFT_ShowNumber(20,48,YELLOW,BLACK,k);
		*/ 
		
		//字库没该汉字时
		if(k == -1){
			if(x+8>=128){ x=0;y+=16;}
			TFT_ShowChar(x,y,YELLOW,RED,'?');  //可自定义一个符号替代
			x+=8;
			n+=2;    //顺便跳过后半个汉字字符,一个汉字，3个位置 见721行注解
			continue;
		}
			
			//输出该汉字
		  if(x+16>=128){   //自动换行
			  x=0;y+=16;
			}
			for(int i = 0; i < 16; i++){
					for (int j = 0; j < 8; j++){            //左半边
						if (chinese_font[k*32+2*i]&(0x80>>j)) //因为中文2个字节，左侧恰好全为偶数，右侧全为奇数   
							TFT_DrawPoint(x+j,y+i,fc);        //数组中，每连续的两个元素，点亮一行。
						else                                //故而奇数元素总是负责左半行，偶数负责右半行
							TFT_DrawPoint(x+j,y+i,bc);   
					}
					for (int j = 0; j < 8; j++){              //右半边，右侧为奇数
						if (chinese_font[k*32+2*i+1]&(0x80>>j)) //k*32得到在字模数组中的初始下标
							TFT_DrawPoint(x+j+8,y+i,fc);
						else
							TFT_DrawPoint(x+j+8,y+i,bc);   
					}
		  }
			x+=16;  //更新位置
		  n+=2;   //我这里一个汉字占3个位置，所以加2
			
			/*汉字字符长度问题
			用sizeof在keil5中测试，一个汉字的字符串，长度为3，所以这里n+=2
			可能是keil5软件,或win系统输入法编码方的问题，用dev测长度倒是正常的。
			正常应当是一个汉字占2个位置，可能用的utf-8编码占3位
		  */
   }
}


/*
功能：查找汉字在“汉字样例数组” 中是第几个汉字。
参数：一个中文汉字
解释：等价于在母串中查找字串位置，返回第一次出现的下标，除以3得到该汉字是第几个汉字
      然后在汉字字模数组中，再用这个位置乘32，就能得到起始坐标。
返回：返回汉字在字体中的位置信息，用于计算字模数组下标
*/
int map(char *c){
	int l1=strlen(font_sample);
	
	/*debug
	//@@ 显示L1 母串长度
	TFT_ShowString(0,32,RED,BLACK,"L1=");
	TFT_ShowNumber(20,32,YELLOW,BLACK,l1);
	*/
	
	for(int i=0;i<l1;i+=3){  //遍历母串
		if((font_sample[i]==c[0])&&(font_sample[i+1]==c[1])&&(font_sample[i+2]==c[2])) //三位定一个汉字
			   return i/3;
  }
  return -1; //找不到时返回-1,所以返回类型不能用u8
}

