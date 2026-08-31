#ifndef _SPI_H
#define _SPI_H   

/*STM32F103C8T6—ST7735S 接线
	GND   电源地
	VCC   接5V或3.3v电源
	SCL   接PA5
	SDA   接PA7
	RES   接PB0s
	DC    接PB1
	CS    接PA4 
	BL		接PB10    不控制背光，则只接3.3或不接就可以，嫌麻烦可以不接BLK
	                BLK若接B10，初始化时必须再加上 TFT_TurnOff(1);
*/

//宏定义引脚，调整引脚只需对这里修改
#define RCC_BDR RCC_APB2Periph_GPIOB
#define BDR_PORT GPIOB  
#define BLK_PIN GPIO_Pin_10 //B10
#define DC_PIN GPIO_Pin_1  //B1
#define RST_PIN GPIO_Pin_0 //B0

#define RCC_CDS RCC_APB2Periph_GPIOA
#define CDS_PORT GPIOA 
#define SDA_PIN GPIO_Pin_7  //A7
#define CS_PIN GPIO_Pin_4 //A4 
#define SCL_PIN GPIO_Pin_5 //A5

//引脚电平写入，SPI高速协议性能要求较高，故直接调用寄存器
#define CS_H  ((CDS_PORT)->ODR |= (CS_PIN))
#define SCL_H  ((CDS_PORT)->ODR |= (SCL_PIN))
#define SDA_H  ((CDS_PORT)->ODR |= (SDA_PIN))
#define RST_H ((BDR_PORT)->ODR |= (RST_PIN))
#define DC_H ((BDR_PORT)->ODR |= (DC_PIN))
#define BLK_H ((BDR_PORT)->ODR |= (BLK_PIN))

#define CS_L  ((CDS_PORT)->ODR &= ~(CS_PIN))
#define SCL_L  ((CDS_PORT)->ODR &= ~(SCL_PIN))
#define SDA_L  ((CDS_PORT)->ODR &= ~(SDA_PIN))
#define RST_L  ((BDR_PORT)->ODR &= ~(RST_PIN))
#define DC_L  ((BDR_PORT)->ODR &= ~(DC_PIN))
#define BLK_L ((BDR_PORT)->ODR &= ~(BLK_PIN))

/*
在模拟通讯SPI协议时，对性能要求极高，需要精细控制硬件行为
使用如下的宏定义，可能会影响最终结果，bug未知
#define CS_H   GPIO_SetBits(CDS_PORT,CS_PIN)
*/

//颜色
#define RED  		0xf800
#define GREEN		0x07e0
#define BLUE 		0x001f
#define BLUE2 	0x1c9f
#define PINK    0xd8a7
#define ORANGE  0xfa20
#define WHITE		0xffff
#define BLACK		0x0000
#define YELLOW  0xFFE0
#define CYAN    0x07ff
#define PURPLE  0xf81f
#define PURPLE2 0xdb92
#define PURPLE3 0x8811
#define GRAY0   0xEF7D
#define GRAY1   0x8410
#define GRAY2   0x4208

//---------------------封装函数操作--------------------------

//SPI 通信
void Spi_Init(void);							//引脚配置
void Spi_SendData(uint8_t data);  //没有CS操作：根据时钟向总线写一个8位数据
void TFT_SendData(uint8_t Data);   //加上CS DC操作 向液晶屏写一个8位数据
void TFT_Send16Bit(uint16_t Data); //向液晶屏写一个16位数据
void TFT_SendIndex(uint8_t reg);	//向液晶屏写一个8位指令
void TFT_SendReg(uint8_t adress,uint8_t data);//封装前两者(Index,Data)，REG意为寄存器


//TFT初始化
void TFT_Init(void);  //初始化TFT,各种模式设定
void TFT_Reset(void);  //硬件重启，RST拉低再拉高
void TFT_TurnOff(uint8_t io);        //控制屏幕开关，0关-1开，需要BLK引脚接B10,如不接BLK 不要使用
void TFt_SpinScreen(uint8_t locate); //旋转方向0-3
void TFT_SetCursor(uint16_t x,uint16_t y);//起始坐标
void TFT_Clear(uint16_t color);           //纯色清屏
void TFT_SetRegion(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end); //选中区域
void TFT_FullScreen(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color); //区间性质的填充屏幕

//基本绘制
void TFT_DrawPoint(uint16_t x,uint16_t y,uint16_t Data); //点
void TFT_DrawCircle(uint16_t X,uint16_t Y,uint16_t R,uint16_t fc);//圆
void TFT_DrawLine(uint16_t x0, uint16_t y0,uint16_t x1, uint16_t y1,uint16_t Color); //线

//组合绘制
void TFT_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h,uint16_t bc);//矩形：左上角坐标+矩形长宽+颜色
void TFT_box2(uint16_t x,uint16_t y,uint16_t w,uint16_t h, u8 mode);//矩形方案：0-mode
void ButtonDown(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);//按钮特效1 
void ButtonUp(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);  //按钮特效2 按钮框左上角和右下角坐标

//文字图片，字符，变量，中文
void TFT_ShowImage(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const unsigned char *p);//图片左上角坐标，图像长宽，图片数组
void TFT_ShowChar(uint8_t x,uint8_t y,uint16_t fc,uint16_t bc,char c); //显示一个字符
void TFT_ShowString(uint8_t x,uint8_t y,uint16_t fc,uint16_t bc,char *c); //显示字符串，自动换行
void TFT_ShowNumber(uint8_t x,uint8_t y,uint16_t fc,uint16_t bc,long long num);  //显示变量，支持负数

//中文，支持混合字符显示
int map(char *c); //模拟一个map容器 用法：map(“汉字”)=中文字模字体数组的，开始位置下标
void TFT_ShowChinese(uint8_t x,uint8_t y,uint16_t fc,uint16_t bc,char *c); 

#endif
