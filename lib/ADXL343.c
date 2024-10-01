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

#include "i2c.h"
#include "ADXL343.h"

// private helpers
int _ADXL343_ReadReg8 (unsigned char TargetRegister, unsigned char * TargetValue)
{
  if (_I2C1_StartWrite(_ADXL343ADDR, 1, &TargetRegister, _I2C_AutoEnd_Software))
      return -1;

  if (_I2C1_StartRead (_ADXL343ADDR, 1, TargetValue, _I2C_AutoEnd_Hardware))
    return -2;
  
  return 0;
}

int _ADXL343_WriteReg8 (unsigned char TargetRegister, unsigned char TargetValue)
{
  unsigned char buff [2];
  buff[0] = TargetRegister;
  buff[1] = TargetValue;

  if (_I2C1_StartWrite(_ADXL343ADDR, 2, buff, _I2C_AutoEnd_Hardware))
      return -1;

  return 0;
}


// end private helpers

int _ADXL343_ReadRegAccData ( short * iX,  short * iY,  short * iZ)
{
  unsigned char buff [1] = { 0x32 };

  if (_I2C1_StartWrite(_ADXL343ADDR, 1, buff, _I2C_AutoEnd_Software))
      return -1;

  unsigned char accdata [6] = { 0 };
  if (_I2C1_StartRead (_ADXL343ADDR, 6, accdata, _I2C_AutoEnd_Hardware))
    return -2;

  *iX = accdata[0] + accdata[1] * 0x100;
  *iY = accdata[2] + accdata[3] * 0x100;
  *iZ = accdata[4] + accdata[5] * 0x100;
    
   return 0;
}

// public functions
int _ADXL343_Init (void)
{
  unsigned char devid = 0;
  
  // in the case of restart, put the device to sleep before changes
  // Register 2D: Power Control
  // D7        D6        D5        D4        D3        D2        D1        D0
  //  0         0       LINK    AutoSleep  Measure    Sleep      | Wakeup   |
  if (_ADXL343_WriteReg8(0x2D, 0b00000100)) // go to sleep
    return -1;
    
  // in the case of restart, turn off all interrupts
  if (_ADXL343_WriteReg8(0x2E, 0b00000000)) 
    return -2;
  
  // why am I not doing this first?
  // read the device ID, if it's not what we expect, abort!
  if (_ADXL343_ReadReg8 (0, &devid))
    return -3;
  
  if (devid != 0b11100101)  // fixed dev id
    return -4;
    
  // data format (all defaults (2G))
  _ADXL343_WriteReg8(0x31, 0b00000000);
  
  // FIFO mode - bypass  
  _ADXL343_WriteReg8(0x38, 0b00000000);
  
  // go for measurements (MEASURE = 1)
  _ADXL343_WriteReg8 (0x2D, 0b00001000);

  // made it here, no error
  return 0;  
}

// testing, enable activity interrupt on interrupt 1 (parameterize later)
void _ADXL343_ActivityInterrupt (unsigned char Thresh_Act)
{
  // assume device is initialized

  if (Thresh_Act < 1)
    Thresh_Act = 1;
  //ADXL343_WriteReg8(0x24, 20); // 20 * 62.5mg
  //ADXL343_WriteReg8(0x24, 2); // 2 * 62.5mg // very, very sensitive
  _ADXL343_WriteReg8(0x24, Thresh_Act); // let the user pick the threshold
  _ADXL343_WriteReg8(0x27, 0b11110000); // act ac, xyz, no inact
  _ADXL343_WriteReg8(0x2F, 0b00000000); // all interrupts to int1
  _ADXL343_WriteReg8(0x2E, 0b00010000); // enable activity interrupt 		
}

void _ADXL343_TapInterrupt (void)
{
  // assume device is initialized
  _ADXL343_WriteReg8(0x1D, 20); // 20 * 62.5mg = 1.25g (instantaneous) 
  _ADXL343_WriteReg8(0x21, 100); // 100 * 625us = ~63ms accelerometer event (shorten to reject doubles)
  _ADXL343_WriteReg8(0x2A, 0b00001111); // tap ac, xyz, suppress doubles
  _ADXL343_WriteReg8(0x2F, 0b00000000); // all interrupts to int1
  _ADXL343_WriteReg8(0x2E, 0b01000000); // enable single tap interrupt
}

unsigned char _ADXL343_ReadInterruptSource (void)
{
  unsigned char target = 0;
  _ADXL343_ReadReg8(0x2B, &target); // discard act/tap status
  _ADXL343_ReadReg8(0x30, &target); // return interrupt source 
  return target;
}

