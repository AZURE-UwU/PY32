/* stm32_clock.c */
#include "stm32f10x_rcc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"

void SystemClock_Config(void)
{
    /* 1. 禁用 LSE（32.768 kHz 外部晶振） */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    RCC_LSEConfig(RCC_LSE_OFF);
    while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET) { /* 等待 LSE 失能 */ }
    PWR_BackupAccessCmd(DISABLE);

    /* 2. 启动 HSE（8 MHz 外部晶振） */
    RCC_HSEConfig(RCC_HSE_ON);
    if (RCC_WaitForHSEStartUp() != SUCCESS) {
        while (1);  /* HSE 启动失败，停在这里 */
    }

    /* 3. PLL = HSE × 9 = 72 MHz */
    RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
    RCC_PLLCmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) { /* 等待 PLL 就绪 */ }

    /* 4. 切换系统时钟到 PLL */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    while (RCC_GetSYSCLKSource() != 0x08) { /* 0x08 = PLL */ }

    /* 5. 配置总线时钟分频（也可是默认值） */
    RCC_HCLKConfig(RCC_SYSCLK_Div1);    /* AHB = 72 MHz */
    RCC_PCLK2Config(RCC_HCLK_Div1);     /* APB2 = 72 MHz */
    RCC_PCLK1Config(RCC_HCLK_Div2);     /* APB1 = 36 MHz */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);   /* ADC = 12 MHz */

    /* 6. （可选）如果需要 RTC，用 LSI 替代 LSE；不需要可整段注释掉 */
    RCC_LSICmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) { }
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    RCC_RTCCLKCmd(ENABLE);
}



