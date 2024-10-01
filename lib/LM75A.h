// LM75AD,118 Temperature Sensor I2C Library
//
// Functions to operate the LM75AD,118 on a connected I2C bus
//
// Simon Walker
// Created September 2024, initial build, ported from ATmega328 version
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

// default address for this device (device may support other addresses)
#define _LM75A_ADDRESS 0x48

// read 16-bit value from device
// data for NXP version is left-aligned, 11 bits
// upper 8 bits are signed whole degrees C
// lower 3 bits are fractions of a degree (x 0.125)
int _LM75A_ReadTemp (unsigned int * uiTemp);

// use read temp to convert to an actual temperature
//  returns -300 if I2C error
float _LM75A_GetTempF (void);


