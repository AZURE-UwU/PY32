/**
  ******************************************************************************
  * @file    main.h
  * @author  Bowen (wbw20)
  * @date    2026-08-29
  * @version V1.2
  * @hardware PY32F003F18P6TU 开发板（TSSOP20）
  * @brief   PY32F003F18P6TU 工程公共头文件
  *
  * 设计说明：
  *   1. 本文件集中定义所有引脚宏与应用级配置，改硬件只改这里；
  *   2. 引脚分配原则：PA13(SWDIO)、PA14(SWCLK)、PF2(NRST)、PF0/PF1(晶振)、
  *      PF4(BOOT0) 保留，其余引脚按 F18P 板扩展排针分配，详见 README 引脚表；
  *   3. 关键参数用 #define，配置与逻辑分离。
  *
 * CHANGELOG:
 *   V1.0 (2026-08-29) 从 PY32F002B_ST7735 工程移植到 PY32F003F18P6TU。
 *   V1.1 (2026-08-29) PF0/PF1/PF4 允许复用后重新规划：恢复 LCD 的
 *                     CS/RST/BLK 引脚（背光恢复 PWM 无级调光）。
 *   V1.2 (2026-08-30) 全部按键改为高电平有效：BTN_ACTIVE_LOW 置 0，
 *                     内部下拉，按下为高电平（上升沿唤醒 STOP）。
  ******************************************************************************
  */

/* 防止头文件重复包含 ------------------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* 头文件包含 --------------------------------------------------------*/
#include "py32f0xx_hal.h"

/* 错误处理函数 ------------------------------------------------------*/
void APP_ErrorHandler(void);

/* ------------------------------------------------------------------*/
/* 引脚定义（PY32F003F18P6TU TSSOP20）                                */
/* ------------------------------------------------------------------*/

/* SPI1（驱动 ST7735S，仅发送 MOSI + SCK）：SCK=PA1，MOSI=PA2 */
#define LCD_SCK_Pin              GPIO_PIN_1
#define LCD_SCK_GPIO_Port        GPIOA
#define LCD_MOSI_Pin             GPIO_PIN_2
#define LCD_MOSI_GPIO_Port       GPIOA

/* ST7735S 控制脚：DC=PA3、CS=PF1、RST=PF4（普通 GPIO） */
#define LCD_CS_Pin               GPIO_PIN_1
#define LCD_CS_GPIO_Port         GPIOF
#define LCD_DC_Pin               GPIO_PIN_3
#define LCD_DC_GPIO_Port         GPIOA
#define LCD_RST_Pin              GPIO_PIN_4
#define LCD_RST_GPIO_Port        GPIOF

/* ST7735S 背光：TIM14_CH1(PF0) PWM 无级调光 */
#define LCD_BLK_Pin              GPIO_PIN_0
#define LCD_BLK_GPIO_Port        GPIOF

/* 风扇 PWM（TIM3_CH2） */
#define PWM_FAN_Pin              GPIO_PIN_5
#define PWM_FAN_GPIO_Port        GPIOB

/* 状态指示灯（板上 PA0 LED） */
#define LED_STATE_Pin            GPIO_PIN_0
#define LED_STATE_GPIO_Port      GPIOA

/* I2C1（INA226）：SCL=PB6、SDA=PB7 */
#define I2C_SCL_Pin              GPIO_PIN_6
#define I2C_SCL_GPIO_Port        GPIOB
#define I2C_SDA_Pin              GPIO_PIN_7
#define I2C_SDA_GPIO_Port        GPIOB

/* ADC：PA5=IN5（VCC 分压）、PA4=IN4（NTC 分压） */
#define ADC_VCC_Pin              GPIO_PIN_5
#define ADC_VCC_GPIO_Port        GPIOA
#define ADC_NTC_Pin              GPIO_PIN_4
#define ADC_NTC_GPIO_Port        GPIOA

/* 三个按键（均为下降/上升沿中断，消抖在主循环 50ms 任务完成）：
   SW_WKUP=PA12（板上 KEY，高电平有效）是低功耗唤醒键 */
#define SW_WKUP_Pin              GPIO_PIN_12
#define SW_WKUP_GPIO_Port        GPIOA
#define SW_WKUP_EXTI_IRQn        EXTI4_15_IRQn

#define SW_FUNC_Pin              GPIO_PIN_6
#define SW_FUNC_GPIO_Port        GPIOA
#define SW_FUNC_EXTI_IRQn        EXTI4_15_IRQn

#define SW_MODE_Pin              GPIO_PIN_7
#define SW_MODE_GPIO_Port        GPIOA
#define SW_MODE_EXTI_IRQn        EXTI4_15_IRQn

/* ------------------------------------------------------------------*/
/* 应用配置宏                                                        */
/* ------------------------------------------------------------------*/
#define APP_TASK_50MS_MS         50U     /* 50ms 周期任务：按键消抖/ADC/温度/风扇 */
#define APP_TASK_1S_MS           1000U   /* 1s 周期任务：LED 闪烁/待机倒计时     */

#define INA226_DEV_ADDR          0x40U   /* INA226 器件 7bit 地址               */

/* 按键电平约定：
   0 = 按下为高电平（引脚内部下拉，本工程当前配置）；
   1 = 按下为低电平（引脚内部上拉，参考板"按钮低电平有效"接法）。
   只改这里，bsp_gpio 会自动切换上拉/下拉与唤醒边沿。 */
#define BTN_ACTIVE_LOW           0

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/

