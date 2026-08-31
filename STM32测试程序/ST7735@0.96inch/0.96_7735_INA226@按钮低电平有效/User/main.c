#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "soft_i2c.h"
#include "GPIO.h"
#include "RCC.h"
#include "EXTI.h"
#include "NVIC.h"
#include "stm32_clock.h"
#include "image1.h"
#include "ina226.h"
#include "St7735tft.h" //ST7735S驱动，与TFT操作
#include "Font_asc.h" //因为在主函数测试中文输出，随时能看到汉字
#include <math.h>
#include "stm32f10x_adc.h"



//颜色数组0-11
int colr[20]={0xf800,0x07e0,0x001f,0x1c9f,0x8811,0xd8a7,0xfa20,0xffff,0xFFE0,0x07ff,0xf81f,0xdb92};

//字库
const char font_sample[]=
	"电压流占空比显示模式库仑计温度操你妈爸功率输入";

////温度采样ADC参数定义////
#define ADC_DOWN    1.0f      		// 当 ADC 电压达到此值（V）时开启 PWM（占空比 1%）
#define ADC_TOP     3.0f      		// 当 ADC 电压达到此值（V）时，PWM 占空比达到 99%
#define ADC_VREF    3.3f      		// ADC 引用电压（V）
#define ADC_MAX     4095.0f       // 12位 ADC 的最大值
//参考VCC电压
#define VCC         3.3f
//分压电路参数
#define R_PULLDOWN  10000.0f  // 下拉电阻 10 kΩ
#define B_VALUE     3950.0f   // 热敏电阻 B 参数
#define R0					10000.0f // 热敏电阻在 25℃ 时的阻值
#define T0_KELVIN		298.15f  // 25℃ 对应的开尔文温度 (273.15 + 25)
float TEMP;
//////////////////////////


////输入电压ADC参数定义////
uint16_t adc_V_value;
float adc_V_voltage;	
#define Multiplier	10	//倍数
//////////////////////////

////风扇控制部分////
volatile uint32_t desiredFreq = 0;     	// 初始化 PWM 频率（kHz）
volatile uint8_t dutyCyclePercent = 0;  // 目标占空比（%）
uint16_t adc_value;											// ADC数值
float adc_voltage;											// 模拟电压
//////////////////

////库仑计部分////
#define SAMPLE_TIME_SEC  									 0.01f//库仑计采样时基
volatile float totalCoulomb1					 =	 0;// 库仑计
volatile float totalCoulomb2					 =	 0;	// 库仑计
/////////////////

////按钮检测部分////
volatile uint32_t SW1				 =	 0;
volatile uint32_t SW2				 =	 3;			//屏幕初始方向
volatile uint32_t SW1_E			 =	 0;
volatile uint32_t SW2_E			 =	 0;
volatile uint32_t SW1_2_E		 =	 0;
volatile uint8_t SW1_LongE 	= 0;    // SW1 长按事件标志
volatile uint8_t SW2_LongE 	= 0;    // SW2 长按事件标志
volatile uint32_t SW1_F 		= 0;
volatile uint32_t SW2_F 		= 0;
volatile uint32_t msTicks	 	= 0;     // SysTick 毫秒计数器
volatile uint32_t pressT1  	= 0;    // PA0 上次按下时间戳
volatile uint32_t pressT2  	= 0;    // PC13 上次按下时间戳
int mode									 	= 1;
///////////////////


////INA226需求变量////
float busVoltage						 =	 0;
float shuntVoltage					 =	 0;
float current								 =	 0;
float power									 =	 0;

volatile uint8_t ADDRESS 		 =	 0x40; // 防止编译器优化
/////////////////////

////关机计数器////
volatile uint32_t msTicks_PWoff = 0;
/////////////////



//////////////////////////////////////////////////检测数据正负并返回任意字符//////////////////////////////////////////////////
char* check_sign(float x) {
    // 通过 IEEE‐754 浮点数最高位判断正负（1 表示负数或 -0.0）
    union {
        float    f;
        unsigned u;
    } un = { x };

    static char buf[2];
    buf[0] = (un.u & 0x80000000U) ? 'N' : 'P';
    buf[1] = '\0';
    return buf;
}
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////将电压值转换为温度（单位：℃）//////////////////////////////////////////////////
float Votage_To_Temperature(float voltage)
{
{
    // 1. 电压不能是 0
    if (voltage <= 0.0f) return -273.15f;  

    // 2. 计算热敏电阻阻值
    float r_ntc = R_PULLDOWN * (VCC - voltage) / voltage;
    
    // 3. B 参数方程
    float inv_T = 1.0f / T0_KELVIN + logf(r_ntc / R0) / B_VALUE;
    float temp_k = 1.0f / inv_T;
    return temp_k - 273.15f;
}
}

//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////变量格式化函数//////////////////////////////////////////////////

/// 根据浮点数的大小格式化输出，并在末尾添加指定的单位字符。
/// 当浮点数大于等于10时，保留3位小数；小于10时保留4位小数。
/// 参数：
///   a      : 待处理的浮点数
///   unit   : 要附加的单位，例如 "V", "A", "Ω"
///   str    : 用于存放格式化结果的字符数组（建议长度不小于32）
void formatFloatToStrWithUnit(float a, const char unit[2], char str[32])
{
    if (a >= 10.0f) {
        // 大于等于10时保留3位小数，并添加单位
        sprintf(str, "%.3f%s", a, unit);
    } else {
        // 小于10时保留4位小数，并添加单位
        sprintf(str, "%.4f%s", a, unit);
    }
}
////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////INA226库仑计计数//////////////////////////////////////////////////
volatile uint8_t tim_flag = 0;
void processINA226Data(void)
{
    if (tim_flag)
    {
        tim_flag = 0;  // 清除标志
        ADDRESS = 0x40;
        float current1 = INA226_GetCurrent();
//			  float current1 = 800;

				ADDRESS = 0x41;
        float current2 = INA226_GetCurrent();
//				float current2 = 800;

        totalCoulomb1 += fabs(current1) * SAMPLE_TIME_SEC / 3600;
        totalCoulomb2 += fabs(current2) * SAMPLE_TIME_SEC / 3600;
    }
}
/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////关机计数器配置//////////////////////////////////////////////////
// 获取当前毫秒计数
uint32_t GetMsTicks(void) {
    return msTicks_PWoff;
}
// 非阻塞状态机：PB6 高 1 秒后拉低 PB7
void Check_PB6_Timeout(void) {
    static uint8_t  waiting    = 0;
    static uint32_t start_time = 0;
    uint8_t state = !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6);

    if (!waiting) {
        if (state) {
            waiting    = 1;
            start_time = GetMsTicks();
        }
    }
    else {
        if (!state) {
            waiting = 0;  // 电平中断，重置
        }
        else if ((GetMsTicks() - start_time) >= 1000) {
            GPIO_ResetBits(GPIOB, GPIO_Pin_7);
            waiting = 0;  // 完成后重置，避免重复
        }
    }
}
////////////////////////////////////////////////////////////////////////////////




int main(void)
{
	//系统及外设初始化
	SystemInit();
	SystemClock_Config();
	Soft_I2C_Init();
	TFT_Init();
	TFT_TurnOff(1);  
	PWM_Init();
	ADC1_Init();
	ADC2_Init();		
	TIM2_Config();
	TIM3_Config();
	
	// 第一路 @0x40
	ADDRESS = 0x40;
  INA226_Init();

  // 第二路 @0x41
	ADDRESS = 0x41;
  INA226_Init();

  /* 配置 RCC、GPIO、EXTI 和 NVIC */
  RCC_Configuration();
  GPIO_Configuration();
  EXTI_Configuration();
  NVIC_Configuration();
	
	//配置系统计时器
	/* SystemCoreClock 是系统时钟频率，例如 72MHz */
	SysTick_Config(SystemCoreClock / 1000);
	
	/////////////////开机锁定/////////////////
	GPIO_SetBits(GPIOB, GPIO_Pin_7);//立刻拉高PB7
	/////////////////////////////////////////

	TFT_SpinScreen(SW2);
	
	
	//显示字符@开机首次显示
	TFT_ShowChinese(75,30,colr[1],BLACK,"V");
	TFT_ShowChinese(75,55,colr[2],BLACK,"I");
	TFT_ShowChinese(75,80,colr[3],BLACK,"P");
	
	char pwm[32];
	char temp[32];
	char votage[32];
	char C1[32];
	char C2[32];
	char I[32];
	char V[32];
	char P[32];


	while (1)
	{
		///////////////////////////////////////////////////////////////////////////输入电压分压读取///////////////////////////////////////////////////////////////////////////
		adc_V_value = ADC2_Read();  
    adc_V_voltage = ((float)adc_V_value / ADC_MAX) * ADC_VREF;    /* 计算电压值，假设参考电压为 3.3V  */
		//////////////////////////////////////////
		
		///////////////////////////////////////////////////////////////////////////关机计时器///////////////////////////////////////////////////////////////////////////
		Check_PB6_Timeout();
		//////////////////////////////////////////
		
		///////////////////////////////////////////////////////////////////////////库仑计计数///////////////////////////////////////////////////////////////////////////
		processINA226Data(); 
		/////////////////////
		
		///////////////////////////////////////////////////////////////////////////库仑计重置///////////////////////////////////////////////////////////////////////////
		if (SW1_LongE == 1){totalCoulomb1 = 0;SW1_LongE = 0;}
		if (SW2_LongE == 1){totalCoulomb2 = 0;SW2_LongE = 0;}
		//////////////////////////////////////
				
		///////////////////////////////////////////////////////////////////////////风扇PWM频率设置///////////////////////////////////////////////////////////////////////////	
		adc_value = ADC1_Read();  
    adc_voltage = ((float)adc_value / ADC_MAX) * ADC_VREF;    /* 计算电压值，假设参考电压为 3.3V  */
    if(adc_voltage < ADC_DOWN)// 根据采集电压计算 PWM 占空比（百分比）
    {
            // 低于 DOWN 阈值时，关闭 PWM（或输出 0% 占空比）
      dutyCyclePercent = 0;
    }
    else if(adc_voltage >= ADC_TOP)
    {
            // 电压达到或超过 TOP 时，输出 99% 占空比
      dutyCyclePercent = 99;
    }
    else
    {
            // 在 ADC_DOWN 与 ADC_TOP 之间，线性映射至 [1%, 99%]
            // 公式： duty = 1 + (voltage - ADC_DOWN) * 98/(ADC_TOP - ADC_DOWN)
      dutyCyclePercent = (uint8_t)(1 + ((adc_voltage - ADC_DOWN) * 98.0f / (ADC_TOP - ADC_DOWN)));
    }
        
			PWM_UpdateParameters();// 更新 PWM 占空比（内部函数根据当前 ARR 计算对应 CCR1 值）
    /////////////////////////////////////////////////////////////////////////////////

		///////////////////////////////////////////////////////////////////////////旋转屏幕///////////////////////////////////////////////////////////////////////////
		if (SW2_E == 1)//判断是否按下SW2 @旋转屏幕	
		{
			TFT_Clear(BLACK);
			TFT_SpinScreen(SW2);
		}
		/////////////////////////////////////////////////
		
		
		
		
		
		
		
		
		
		
		
		//////////////////////////////////////页面1///////////////////////////////////////////////////
		
		
		if (SW1 == 0)//页面1
		{		
			if (mode == 1)//模式1///
			{
				
				if (SW1_2_E == 1 || SW1_E == 1 || SW2_E == 1)
				{
					//首次进入界面清屏
					TFT_Clear(BLACK);SW1_2_E = 0;SW1_E = 0;SW2_E = 0;
					//显示字符
					TFT_ShowChinese(75,30,colr[1],BLACK,"V");
					TFT_ShowChinese(75,55,colr[2],BLACK,"I");
					TFT_ShowChinese(75,80,colr[3],BLACK,"P");
				}
				

				
				//地址==0x40
				ADDRESS = 0x40;
				busVoltage = INA226_GetBusVoltage();
				shuntVoltage = INA226_GetShuntVoltage();
				current = INA226_GetCurrent();
				power = busVoltage * current;
				
				//处理变量
				formatFloatToStrWithUnit(busVoltage,"V",V);
				formatFloatToStrWithUnit(fabs(current),"A",I);
				formatFloatToStrWithUnit(fabs(power),"W",P);
				//显示变量
				TFT_ShowString(5,30,WHITE,BLACK,V);
				TFT_ShowString(5,55,WHITE,BLACK,I);
				TFT_ShowString(65,55,colr[1],BLACK,check_sign(current));
				TFT_ShowString(5,80,WHITE,BLACK,P);
				//显示进度条
				TFT_filledBox(5,50,busVoltage*2,3,colr[1]);
				TFT_filledBox(busVoltage*2+5,50,60-busVoltage*2,3,BLACK);		
				TFT_filledBox(5,75,current*2,3,colr[2]);
				TFT_filledBox(current*2+5,75,60-current*2,3,BLACK);		
				TFT_filledBox(5,105,power*2,3,colr[3]);
				TFT_filledBox(power*2+5,105,60-power*2,3,BLACK);		
				
				//地址==0x41
				ADDRESS = 0x41;
				busVoltage = INA226_GetBusVoltage();
				shuntVoltage = INA226_GetShuntVoltage();
				current = INA226_GetCurrent();
				power = INA226_GetPower();
				//处理变量
				formatFloatToStrWithUnit(busVoltage,"V",V);
				formatFloatToStrWithUnit(fabs(current),"A",I);
				formatFloatToStrWithUnit(fabs(power),"W",P);
				//显示变量
				TFT_ShowString(95,30,WHITE,BLACK,V);
				TFT_ShowString(95,55,WHITE,BLACK,I);
				TFT_ShowString(85,55,colr[1],BLACK,check_sign(current));
				TFT_ShowString(95,80,WHITE,BLACK,P);
				//显示进度条
				TFT_filledBox(95,50,busVoltage*2,3,colr[1]);
				TFT_filledBox(busVoltage*2+95,50,60-busVoltage*2,3,BLACK);		
				TFT_filledBox(95,75,current*2,3,colr[2]);
				TFT_filledBox(current*2+95,75,60-current*2,3,BLACK);		
				TFT_filledBox(95,105,power*2,3,colr[3]);
				TFT_filledBox(power*2+95,105,60-power*2,3,BLACK);
				
			}
			if (mode == 2)//模式2//
			{
				
				if (SW1_2_E == 1 || SW1_E == 1 || SW2_E == 1)
				{
					//首次清屏
					TFT_Clear(BLACK);SW1_2_E = 0;SW1_E = 0;SW2_E = 0;
					//显示字符
					TFT_ShowChinese(5,30,colr[1],BLACK,"V1");
					TFT_ShowChinese(5,50,colr[1],BLACK,"I1");
					TFT_ShowChinese(5,70,colr[1],BLACK,"C1");
					TFT_ShowChinese(5,90,colr[1],BLACK,"P1");
					TFT_ShowChinese(90,30,colr[1],BLACK,"PWM");
					TFT_ShowChinese(90,50,colr[1],BLACK,"TEMP");
				}
				


				//地址==0x40
				ADDRESS = 0x40;
				busVoltage = INA226_GetBusVoltage();
				shuntVoltage = INA226_GetShuntVoltage();
				current = INA226_GetCurrent();
				power = INA226_GetPower();
				//处理数字
				formatFloatToStrWithUnit(busVoltage,"V",V);
				formatFloatToStrWithUnit(fabs(current),"A",I);
				formatFloatToStrWithUnit(fabs(power),"W",P);
				sprintf(C1, "%.3fAh", totalCoulomb1);
				sprintf(pwm, "%d%%", dutyCyclePercent);
				TEMP = Votage_To_Temperature(adc_voltage);
				sprintf(temp, "%.1fC", TEMP);
				sprintf(votage, "%.2fV", adc_V_voltage * Multiplier);
				//显示数字
				TFT_ShowString(25,30,WHITE,BLACK,V);
				TFT_ShowString(25,50,WHITE,BLACK,I);
				TFT_ShowString(25,70,WHITE,BLACK,C1);
				TFT_ShowString(25,90,WHITE,BLACK,P);
				TFT_ShowString(120,30,WHITE,BLACK,pwm);
				TFT_ShowString(90,70,WHITE,BLACK,temp);
				TFT_ShowString(90,90,WHITE,BLACK,votage);
	
			}
		}
		
		
		/////////////////////////////////////界面2//////////////////////////////////////////
		
		
		if (SW1 == 1)//页面2
		{		
			if (mode == 1)//模式1//
			{
				if (SW1_2_E == 1 || SW1_E == 1 || SW2_E == 1)
				{
					//首次进入界面清屏
					TFT_Clear(BLACK);SW1_2_E = 0;SW1_E = 0;SW2_E = 0;
					//显示字符
					TFT_ShowChinese(5,30,colr[1],BLACK,"PWM");
					TFT_ShowChinese(70,30,colr[1],BLACK,"TEMP");
					TFT_ShowChinese(70,70,colr[1],BLACK,"库仑计2");
					TFT_ShowChinese(5,70,colr[1],BLACK,"库仑计1");
					TFT_ShowChinese(5,50,colr[1],BLACK,"输入电压");
				}

				//处理变量
				sprintf(pwm, "%d%%", dutyCyclePercent);
				TEMP = Votage_To_Temperature(adc_voltage);
				sprintf(temp, "%.1fC", TEMP);
				sprintf(votage, "%.2fV", adc_V_voltage * Multiplier);
				sprintf(C1, "%.3fAh", totalCoulomb1);
				sprintf(C2, "%.3fAh", totalCoulomb2);
				//显示变量
				TFT_ShowString(40,30,WHITE,BLACK,pwm);
				TFT_ShowString(110,30,WHITE,BLACK,temp);
				TFT_ShowString(70,50,WHITE,BLACK,votage);
				TFT_ShowString(5,90,WHITE,BLACK,C1);
				TFT_ShowString(70,90,WHITE,BLACK,C2);
			}
			if (mode == 2)//模式2//
			{
				if (SW1_2_E == 1 || SW1_E == 1 || SW2_E == 1)
				{
					//首次清屏
					TFT_Clear(BLACK);SW1_2_E = 0;SW1_E = 0;SW2_E = 0;
					//显示字符
					TFT_ShowChinese(5,30,colr[1],BLACK,"V2");
					TFT_ShowChinese(5,50,colr[1],BLACK,"I2");
					TFT_ShowChinese(5,70,colr[1],BLACK,"C2");
					TFT_ShowChinese(5,90,colr[1],BLACK,"P2");
					TFT_ShowChinese(90,30,colr[1],BLACK,"PWM");
					TFT_ShowChinese(90,50,colr[1],BLACK,"TEMP");
				}
				//地址==0x41
				ADDRESS = 0x41;
				busVoltage = INA226_GetBusVoltage();
				shuntVoltage = INA226_GetShuntVoltage();
				current = INA226_GetCurrent();
				power = INA226_GetPower();
				//处理数字
				formatFloatToStrWithUnit(busVoltage,"V",V);
				formatFloatToStrWithUnit(fabs(current),"A",I);
				formatFloatToStrWithUnit(fabs(power),"W",P);
				sprintf(C2, "%.3fAh", totalCoulomb2);
				sprintf(pwm, "%d%%", dutyCyclePercent);
				TEMP = Votage_To_Temperature(adc_voltage);
				sprintf(temp, "%.1fC", TEMP);
				sprintf(votage, "%.2fV", adc_V_voltage * Multiplier);
				//显示数字
				TFT_ShowString(25,30,WHITE,BLACK,V);
				TFT_ShowString(25,50,WHITE,BLACK,I);
				TFT_ShowString(25,70,WHITE,BLACK,C2);
				TFT_ShowString(25,90,WHITE,BLACK,P);
				TFT_ShowString(120,30,WHITE,BLACK,pwm);
				TFT_ShowString(90,70,WHITE,BLACK,temp);
				TFT_ShowString(90,90,WHITE,BLACK,votage);
			}
		}
	}
	
    return 0;
}



