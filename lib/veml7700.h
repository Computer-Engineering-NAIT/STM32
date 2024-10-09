// VEML7700 Ambient Light Sensor I2C Library
//
// Functions to operate the VEML7700 on a connected I2C bus
//
// Simon Walker
// Created October 2024, initial build, ported from ATmega328 version
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

#include "stdint.h"

#define VEML7700_ADDR 0x10

int VEML7700_Read_ID (uint16_t * ID);
int VEML7700_ReadWhite (uint16_t * uiWhite);
int VEML7700_ReadAmbient (uint16_t * uiAmbient);

int VEML7700_Init (void);