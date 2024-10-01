// ADXL343 Accelerometer Library
//
// Functions to operate the ADXL343 on a connected I2C bus
//
// Simon Walker
//
// Feb 15 2024 - SLW - Added TAP interrupt support (only had activity interrupt previously)
// Created September 2024, initial build, ported from ATmega328 version
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

#ifndef _ADXL343ADDR
#define _ADXL343ADDR 0x53
#endif

int _ADXL343_Init (void);
int _ADXL343_ReadRegAccData ( short * iX,  short * iY,  short * iZ);
void _ADXL343_ActivityInterrupt (unsigned char Thresh_Act);
void _ADXL343_TapInterrupt (void);
unsigned char _ADXL343_ReadInterruptSource (void);

// private helpers
// int ADXL343_ReadReg8 (unsigned char TargetRegister, unsigned char * TargetValue);
// int ADXL343_WriteReg8 (unsigned char TargetRegister, unsigned char TargetValue);