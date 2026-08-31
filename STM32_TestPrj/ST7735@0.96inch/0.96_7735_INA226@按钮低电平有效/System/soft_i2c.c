//#include "soft_i2c.h"
//#define I2C_DELAY()  Soft_I2C_Delay()

//// SCL control
//#define SOFT_I2C_SCL_HIGH()  GPIO_SetBits(SOFT_I2C_SCL_GPIO_PORT, SOFT_I2C_SCL_PIN)
//#define SOFT_I2C_SCL_LOW()   GPIO_ResetBits(SOFT_I2C_SCL_GPIO_PORT, SOFT_I2C_SCL_PIN)

//// SDA control
//#define SOFT_I2C_SDA_HIGH()  GPIO_SetBits(SOFT_I2C_SDA_GPIO_PORT, SOFT_I2C_SDA_PIN)
//#define SOFT_I2C_SDA_LOW()   GPIO_ResetBits(SOFT_I2C_SDA_GPIO_PORT, SOFT_I2C_SDA_PIN)

//// read SDA statement
//#define SOFT_I2C_READ_SDA()  GPIO_ReadInputDataBit(SOFT_I2C_SDA_GPIO_PORT, SOFT_I2C_SDA_PIN)

//void Soft_I2C_Delay(void) {
//    // Delay time
//    volatile uint16_t i = 30;
//    while (i--);
//}

//void Soft_I2C_Init(void) {
//    GPIO_InitTypeDef GPIO_InitStructure;

//    // Enable GPIO
//    RCC_APB2PeriphClockCmd(SOFT_I2C_SCL_GPIO_CLK | SOFT_I2C_SDA_GPIO_CLK, ENABLE);

//    // Set SCL
//    GPIO_InitStructure.GPIO_Pin = SOFT_I2C_SCL_PIN;
//    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  // ????
//    GPIO_Init(SOFT_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);

//    // Set SDA
//    GPIO_InitStructure.GPIO_Pin = SOFT_I2C_SDA_PIN;
//    GPIO_Init(SOFT_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);

//    // Pull up SCL and SDA
//    SOFT_I2C_SCL_HIGH();
//    SOFT_I2C_SDA_HIGH();
//}

//void Soft_I2C_Start(void) {
//    SOFT_I2C_SDA_HIGH();
//    SOFT_I2C_SCL_HIGH();
//    I2C_DELAY();
//    SOFT_I2C_SDA_LOW();
//    I2C_DELAY();
//    SOFT_I2C_SCL_LOW();
//}

//void Soft_I2C_Stop(void) {
//    SOFT_I2C_SDA_LOW();
//    SOFT_I2C_SCL_HIGH();
//    I2C_DELAY();
//    SOFT_I2C_SDA_HIGH();
//    I2C_DELAY();
//}

//void Soft_I2C_SendByte(uint8_t byte) {
//    for (uint8_t i = 0; i < 8; i++) {
//        SOFT_I2C_SCL_LOW();
//        if (byte & 0x80)
//            SOFT_I2C_SDA_HIGH();
//        else
//            SOFT_I2C_SDA_LOW();
//        byte <<= 1;
//        I2C_DELAY();
//        SOFT_I2C_SCL_HIGH();
//        I2C_DELAY();
//    }
//    SOFT_I2C_SCL_LOW();
//}

//uint8_t Soft_I2C_ReadByte(uint8_t ack) {
//    uint8_t byte = 0;
//    SOFT_I2C_SDA_HIGH();  // SDA
//    for (uint8_t i = 0; i < 8; i++) {
//        SOFT_I2C_SCL_LOW();
//        I2C_DELAY();
//        SOFT_I2C_SCL_HIGH();
//        byte <<= 1;
//        if (SOFT_I2C_READ_SDA())
//            byte |= 0x01;
//        I2C_DELAY();
//    }
//    SOFT_I2C_SCL_LOW();
//    if (ack)
//        Soft_I2C_SendAck();
//    else
//        Soft_I2C_SendNack();
//    return byte;
//}

//uint8_t Soft_I2C_WaitAck(void) {
//    uint8_t ack;

//    SOFT_I2C_SDA_HIGH();  // SDA
//    I2C_DELAY();
//    SOFT_I2C_SCL_HIGH();
//    I2C_DELAY();
//    ack = SOFT_I2C_READ_SDA();
//    SOFT_I2C_SCL_LOW();
//    return (ack == 0) ? 1 : 0;
//}

//void Soft_I2C_SendAck(void) {
//    SOFT_I2C_SDA_LOW();
//    I2C_DELAY();
//    SOFT_I2C_SCL_HIGH();
//    I2C_DELAY();
//    SOFT_I2C_SCL_LOW();
//}

//void Soft_I2C_SendNack(void) {
//    SOFT_I2C_SDA_HIGH();
//    I2C_DELAY();
//    SOFT_I2C_SCL_HIGH();
//    I2C_DELAY();
//    SOFT_I2C_SCL_LOW();
//}



#include "soft_i2c.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

// Helper macros
#define I2C_DELAY()        Soft_I2C_Delay()
#define SCL_HIGH()         GPIO_SetBits(SOFT_I2C_SCL_GPIO_PORT, SOFT_I2C_SCL_PIN)
#define SCL_LOW()          GPIO_ResetBits(SOFT_I2C_SCL_GPIO_PORT, SOFT_I2C_SCL_PIN)
#define SDA_HIGH()         GPIO_SetBits(SOFT_I2C_SDA_GPIO_PORT, SOFT_I2C_SDA_PIN)
#define SDA_LOW()          GPIO_ResetBits(SOFT_I2C_SDA_GPIO_PORT, SOFT_I2C_SDA_PIN)
#define SDA_READ()         GPIO_ReadInputDataBit(SOFT_I2C_SDA_GPIO_PORT, SOFT_I2C_SDA_PIN)

void Soft_I2C_Delay(void) {
    volatile uint16_t i = 30;
    while (i--) { __NOP(); }
}

void Soft_I2C_Init(void) {
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(SOFT_I2C_SCL_GPIO_CLK | SOFT_I2C_SDA_GPIO_CLK, ENABLE);

    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode  = GPIO_Mode_Out_OD;

    gpio.GPIO_Pin   = SOFT_I2C_SCL_PIN;
    GPIO_Init(SOFT_I2C_SCL_GPIO_PORT, &gpio);

    gpio.GPIO_Pin   = SOFT_I2C_SDA_PIN;
    GPIO_Init(SOFT_I2C_SDA_GPIO_PORT, &gpio);

    SCL_HIGH();
    SDA_HIGH();
}

void Soft_I2C_Start(void) {
    SDA_HIGH(); SCL_HIGH(); I2C_DELAY();
    SDA_LOW();  I2C_DELAY();
    SCL_LOW();
}

void Soft_I2C_Stop(void) {
    SDA_LOW();  SCL_HIGH(); I2C_DELAY();
    SDA_HIGH(); I2C_DELAY();
}

void Soft_I2C_SendByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        SCL_LOW();
        (byte & 0x80) ? SDA_HIGH() : SDA_LOW();
        byte <<= 1;
        I2C_DELAY();
        SCL_HIGH();
        I2C_DELAY();
    }
    SCL_LOW();
}

uint8_t Soft_I2C_ReadByte(uint8_t ack_flag) {
    uint8_t byte = 0;
    SDA_HIGH();
    for (uint8_t i = 0; i < 8; i++) {
        byte <<= 1;
        SCL_LOW(); I2C_DELAY();
        SCL_HIGH(); I2C_DELAY();
        if (SDA_READ()) byte |= 0x01;
    }
    SCL_LOW();
    ack_flag ? Soft_I2C_SendAck() : Soft_I2C_SendNack();
    return byte;
}

uint8_t Soft_I2C_WaitAck(void) {
    uint8_t ack;
    SDA_HIGH(); I2C_DELAY();
    SCL_HIGH(); I2C_DELAY();
    ack = SDA_READ();
    SCL_LOW();
    return (ack == 0);
}

void Soft_I2C_SendAck(void) {
    SDA_LOW();  I2C_DELAY();
    SCL_HIGH(); I2C_DELAY();
    SCL_LOW();
}

void Soft_I2C_SendNack(void) {
    SDA_HIGH(); I2C_DELAY();
    SCL_HIGH(); I2C_DELAY();
    SCL_LOW();
}
