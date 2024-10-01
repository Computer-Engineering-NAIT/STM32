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

#include "i2c.h"
#include "LM75A.h"

int _LM75A_ReadTemp (unsigned int * uiTemp)
{
  {
    // want to read temperature (register zero)
    unsigned char data [1] = { 0x00 };
    
    // start transaction with device address
    if (_I2C1_StartWrite(_LM75A_ADDRESS, 1, data, _I2C_AutoEnd_Software))
      return -1;
  }

  {
    // buffer to RX temp
    unsigned char data [2];
    if (_I2C1_StartRead (_LM75A_ADDRESS, 2, data, _I2C_AutoEnd_Hardware))
    {
      return -2;
    }
      // form up the raw data into a 16-bit raw value for return
      *uiTemp = ((unsigned int)data[0] << 8) + data[1];
  }

  return 0;
}

float _LM75A_GetTempF (void)
{
  unsigned int uiTempRaw;

  if (_LM75A_ReadTemp(&uiTempRaw))
    return -300;
  
  // convert to a float (hard way, from unsigned)
  float fdec = 0;
  if (uiTempRaw & 0x8000)
    fdec = -0.125f * (((~uiTempRaw) >> 5) + 1);
  else
    fdec = 0.125f * (uiTempRaw >> 5);
  return fdec;
}


