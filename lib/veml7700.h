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

///////////////////////////////////////////////////////////
// VEML7700 Sample Use
//if (VEML7700_Init())
//{
//  _USART2_TxStringXY(1, 15, "VEML7700 Init Failure...");
//}

//////////////////////////////////////////////////////////    
// show ambient light level (debug only, VEML7700)
//{
  
//  uint16_t ambient;
//  if (!VEML7700_ReadAmbient(&ambient))
//  {
//    char buff[80];
//    (void)sprintf (buff, "ALS: %4.4X", ambient);
//    _USART2_TxStringXY(40, 3, buff);
//  }
//}

#include "stdint.h"

#define VEML7700_ADDR 0x10

int VEML7700_Read_ID (uint16_t * ID);
int VEML7700_ReadWhite (uint16_t * uiWhite);
int VEML7700_ReadAmbient (uint16_t * uiAmbient);

int VEML7700_Init (void);