//#include "ina226.h"



//// Reset
//#define INA226_CALIBRATION_VALUE  1024
//#define SHUNT_RESISTOR_VALUE      0.01f  // Current SET

//// INA226 Init
//void INA226_Init(void) {
//    Soft_I2C_Init();  // Init I2C
//    INA226_Reset();
//    INA226_Configuration();
//    INA226_WriteRegister(INA226_REG_CALIBRATION, INA226_CALIBRATION_VALUE);
//}

//// INA226 Reset
//void INA226_Reset(void) {
//    INA226_WriteRegister(INA226_REG_CONFIG, 0x8000);
//}

//// INA226 Config
//void INA226_Configuration(void) {
//    uint16_t config = 0;

//    // Shunt and Bus
//    config |= (0x4 << 12);  // Averaging mode: 1 sample
//    config |= (0x4 << 9);   // VBUSCT: 1.1ms
//    config |= (0x4 << 6);   // VSHCT: 1.1ms
//    config |= (0x7 << 0);   // Mode: Shunt and Bus, Continuous

//    INA226_WriteRegister(INA226_REG_CONFIG, config);
//}

//// Write memory
//void INA226_WriteRegister(uint8_t reg, uint16_t value) {
//    Soft_I2C_Start();
//    Soft_I2C_SendByte(INA226_ADDRESS & 0xFE);  // 发送设备地址（写操作：最低位设置为0）
//    if (Soft_I2C_WaitAck()) {
//        Soft_I2C_SendByte(reg);
//        Soft_I2C_WaitAck();
//        Soft_I2C_SendByte((value >> 8) & 0xFF);  // 发送数据的高8位
//        Soft_I2C_WaitAck();
//        Soft_I2C_SendByte(value & 0xFF);         // 发送数据的低8位
//        Soft_I2C_WaitAck();
//    }
//    Soft_I2C_Stop();
//}

//// Read Memory
//		uint16_t INA226_ReadRegister(uint8_t reg) {
//    uint16_t value = 0;

//    // Send Memory Adress
//    Soft_I2C_Start();
//		Soft_I2C_SendByte(INA226_ADDRESS); 
//		// 写寄存器时
//		Soft_I2C_SendByte((INA226_ADDRESS << 1) | 0x00);
////    Soft_I2C_SendByte(INA226_ADDRESS & 0xFE);  
//    if (Soft_I2C_WaitAck()) {
//        Soft_I2C_SendByte(reg);
//        Soft_I2C_WaitAck();
//    }
//    Soft_I2C_Stop();

//    // Read DATA
//    Soft_I2C_Start();
//		// 读寄存器时
//		Soft_I2C_SendByte((INA226_ADDRESS << 1) | 0x01);
////    Soft_I2C_SendByte(INA226_ADDRESS | 0x01);  
//    if (Soft_I2C_WaitAck()) {
//        uint8_t highByte = Soft_I2C_ReadByte(1);  // 读取高8位数据，并发送ACK以继续接收
//        uint8_t lowByte = Soft_I2C_ReadByte(0);   // 读取低8位数据，并发送NACK以结束接收
//        value = (highByte << 8) | lowByte;
//    }
//    Soft_I2C_Stop();

//    return value;
//}

//// Read Bus Votage(V)
//float INA226_GetBusVoltage(void) {
//    uint16_t regValue = INA226_ReadRegister(INA226_REG_BUSVOLTAGE);
//		
//    return regValue * 1.25f / 1000.0f;  // 1 LSB = 1.25mV
//}

//// Read Voltage(mV)
//float INA226_GetShuntVoltage(void) {
//    int16_t regValue = (int16_t)INA226_ReadRegister(INA226_REG_SHUNTVOLTAGE);
//    return regValue * 2.5f / 1000.0f;  // 1 LSB = 2.5mV
//}

//// Read Current(A)
//float INA226_GetCurrent(void) {
//    int16_t regValue = (int16_t)INA226_ReadRegister(INA226_REG_CURRENT);
//    float current_LSB = 0.00512f / (INA226_CALIBRATION_VALUE * SHUNT_RESISTOR_VALUE);
//    return regValue * current_LSB;
//}

//// Read Power(W)
//float INA226_GetPower(void) {
//    uint16_t regValue = INA226_ReadRegister(INA226_REG_POWER);
//    float current_LSB = 0.00512f / (INA226_CALIBRATION_VALUE * SHUNT_RESISTOR_VALUE);
//    float power_LSB = current_LSB * 25;
//    return regValue * power_LSB;
//}






#include "ina226.h"
#include "soft_i2c.h"


void INA226_Init(void) {
    Soft_I2C_Init();
    INA226_Reset();
    INA226_Configuration();
    INA226_WriteRegister(INA226_REG_CALIBRATION, INA226_CALIBRATION_VALUE);
}

// Issue a soft reset
void INA226_Reset(void) {
    INA226_WriteRegister(INA226_REG_CONFIG, 0x8000);
}

// Configure averaging, conversion times, mode
void INA226_Configuration(void) {
    uint16_t config = 0;
    config |= (0x4 << 12);  // AVG = 1 sample
    config |= (0x4 << 9);   // VBUSCT = 1.1 ms
    config |= (0x4 << 6);   // VSHCT  = 1.1 ms
    config |= (0x7 << 0);   // MODE   = Shunt+Bus continuous
    INA226_WriteRegister(INA226_REG_CONFIG, config);
}

// Write a 16-bit register
void INA226_WriteRegister(uint8_t reg, uint16_t value) {
    Soft_I2C_Start();
    Soft_I2C_SendByte((INA226_ADDRESS << 1) | 0x00);  // R/W = 0
    Soft_I2C_WaitAck();

    Soft_I2C_SendByte(reg);
    Soft_I2C_WaitAck();

    Soft_I2C_SendByte((uint8_t)(value >> 8));
    Soft_I2C_WaitAck();

    Soft_I2C_SendByte((uint8_t)(value & 0xFF));
    Soft_I2C_WaitAck();

    Soft_I2C_Stop();
}

// Read a 16-bit register
uint16_t INA226_ReadRegister(uint8_t reg) {
    uint16_t result = 0;
    // 1) Write register pointer
    Soft_I2C_Start();
    Soft_I2C_SendByte((INA226_ADDRESS << 1) | 0x00);
    Soft_I2C_WaitAck();

    Soft_I2C_SendByte(reg);
    Soft_I2C_WaitAck();
    Soft_I2C_Stop();

    // 2) Read two bytes back
    Soft_I2C_Start();
    Soft_I2C_SendByte((INA226_ADDRESS << 1) | 0x01);  // R/W = 1
    Soft_I2C_WaitAck();

    uint8_t msb = Soft_I2C_ReadByte(1);  // Ack after MSB
    uint8_t lsb = Soft_I2C_ReadByte(0);  // Nack after LSB
    Soft_I2C_Stop();

    result = ((uint16_t)msb << 8) | lsb;
    return result;
}

// Convert raw to volts
float INA226_GetBusVoltage(void) {
    uint16_t raw = INA226_ReadRegister(INA226_REG_BUSVOLTAGE);
//    return raw * 1.25f / 1000.0f;  // 1 LSB = 1.25 mV
		return 0.1f;
}

float INA226_GetShuntVoltage(void) {
    int16_t raw = (int16_t)INA226_ReadRegister(INA226_REG_SHUNTVOLTAGE);
//    return raw * 2.5f / 1000.0f;  // 1 LSB = 2.5 µV
		return 0.1f;
}

// Compute current using calibration LSB
float INA226_GetCurrent(void) {
    int16_t raw = (int16_t)INA226_ReadRegister(INA226_REG_CURRENT);
    float current_lsb = 0.00512f / (INA226_CALIBRATION_VALUE * SHUNT_RESISTOR_VALUE);
//    return raw * current_lsb;
		return 300.0f;
}

// Compute power using built-in power LSB = 25 × current LSB
float INA226_GetPower(void) {
    uint16_t raw = INA226_ReadRegister(INA226_REG_POWER);
    float current_lsb = 0.00512f / (INA226_CALIBRATION_VALUE * SHUNT_RESISTOR_VALUE);
    float power_lsb   = current_lsb * 25.0f;
//    return raw * power_lsb;
		return 30.0f;
}


