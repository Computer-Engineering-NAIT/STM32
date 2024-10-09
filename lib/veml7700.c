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

#include "i2c.h"
#include "veml7700.h"

// by default, the device is in shutdown, so make it alive
//  and bring it up with conservative sampling rates
int VEML7700_Init (void)
{
  // gain of 1
  // integration time 100ms
  // persistence protect 1
  // interrupt off
  // power on

  uint16_t uiSettings = 0b0000000000000000;

  unsigned char buff [3];
  buff[0] = 0x00;
  buff[1] = (unsigned char)uiSettings;
  buff[2] = (unsigned char)(uiSettings >> 8);

  if (_I2C1_StartWrite(VEML7700_ADDR, 3, buff, _I2C_AutoEnd_Hardware))
    return -1;

  // read device ID, should be configured for default address and
  //  fixed device ID of 0x81 (0xC481 expected)
  uint16_t id;
  if (VEML7700_Read_ID(&id))
    return -2;  

  // return error result if this is not as expected
  if (id != 0xC481)
    return -3;
  
  return 0;
}

int VEML7700_Read_ID (uint16_t * ID)
{
  unsigned char RegID = 0x07;

  unsigned char IDRead [2];

  if (_I2C1_StartWrite(VEML7700_ADDR, 1, &RegID, _I2C_AutoEnd_Software))
      return -1;

  if (_I2C1_StartRead (VEML7700_ADDR, 2, IDRead, _I2C_AutoEnd_Hardware))
    return -2;

  *ID = IDRead[1] * 256 + IDRead[0];	
  
  return 0;
}

int VEML7700_ReadWhite (uint16_t * uiWhite)
{
  unsigned char RegWhite = 0x05;

  unsigned char WhiteRead [2];

  if (_I2C1_StartWrite(VEML7700_ADDR, 1, &RegWhite, _I2C_AutoEnd_Software))
      return -1;

  if (_I2C1_StartRead (VEML7700_ADDR, 2, WhiteRead, _I2C_AutoEnd_Hardware))
    return -2;

  *uiWhite = WhiteRead[1] * 256 + WhiteRead[0];	
  
  return 0;
}

int VEML7700_ReadAmbient (uint16_t * uiAmbient)
{
  unsigned char RegALS = 0x04; // ALS value

  unsigned char ALSRead [2];

  if (_I2C1_StartWrite(VEML7700_ADDR, 1, &RegALS, _I2C_AutoEnd_Software))
      return -1;

  if (_I2C1_StartRead (VEML7700_ADDR, 2, ALSRead, _I2C_AutoEnd_Hardware))
    return -2;

  *uiAmbient = ALSRead[1] * 256 + ALSRead[0];	
  
  return 0;
}

