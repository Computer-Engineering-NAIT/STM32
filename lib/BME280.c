// implementation for BME280
// Simon Walker, NAIT
// Edited by: Joseph Limin, NAIT
// Revision:
// 	October 12, 2023: Fixed compensation table values
//			  and added temperature, pressure,
//                        and humidity functionality
//      October 02, 2024: Ported to STM32 version
//                        Added defaults to init, and pop
//                        compensation table, cleaned up code a little
//                        added busy function
//
//Software by 2020 Bosch Sensortec GmbH. used
//Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.

#include "i2c.h"
#include "BME280.h"
#include "stdint.h"

// compensation data
struct BME280_CompensationTable
{
  unsigned short dig_T1;
  short dig_T2;
  short dig_T3;

  unsigned short dig_P1;
  short dig_P2;
  short dig_P3;
  short dig_P4;
  short dig_P5;
  short dig_P6;
  short dig_P7;
  short dig_P8;
  short dig_P9;

  unsigned char dig_H1;
  short dig_H2;
  unsigned char dig_H3;
  short dig_H4;
  short dig_H5;
  char dig_H6;
} BME280_CompTable;

// used in compensation, needs to be populated from BME280_compensate_T_int32
int32_t BME280_t_fine;

// private helpers
int BME280_ReadReg8 (unsigned char TargetRegister, unsigned char * TargetValue)
{
  if (_I2C1_StartWrite(BME280ADDR, 1, &TargetRegister, _I2C_AutoEnd_Software))
      return -1;

  if (_I2C1_StartRead (BME280ADDR, 1, TargetValue, _I2C_AutoEnd_Hardware))
    return -2;
  
  return 0;
}

int BME280_WriteReg8 (unsigned char TargetRegister, unsigned char TargetValue)
{
  unsigned char buff [2];
  buff[0] = TargetRegister;
  buff[1] = TargetValue;

  if (_I2C1_StartWrite(BME280ADDR, 2, buff, _I2C_AutoEnd_Hardware))
      return -1;
  
  return 0;
}

int BME280_Init (void)
{
  if (BME280_SW_RESET())
    return -1;

  if (BME280_WriteReg8(BME280_CONFIG, 0b10000100)) // 500ms standby, 2 filter coefficient
    return -2;

  return 0;
}

int BME280_SW_RESET (void)
{
  if (BME280_WriteReg8(BME280_RESET, 0xB6))
    return -1;

  // now wait for NVM copy/reset to complete
  unsigned long itts = 0;
  unsigned char target = 0;
  do
  {
    if (BME280_ReadReg8(BME280_STATUS, &target))
      return -2;

    ++itts;
  }
  while (target & 1);

  // populate comp values
  if (BME280_FetchCompensationValues())
    return -3;

  // populate temperature compensation value
  (void)BME280_compensate_T_int32(BME280_raw_T());  
  
  // rational defaults
  BME280_SetOversampling(BME280_OS_8x, BME280_OS_8x, BME280_OS_8x, BME280_ModeNormal);

  return 0;
}

int BME280_SetOversampling (BME280_Oversampling hum, BME280_Oversampling pres, BME280_Oversampling temp, BME280_SensorMode mode)
{
  if (BME280_WriteReg8(BME280_CTRL_HUM, 0b00000111 & (unsigned char)hum))
    return -1;

  unsigned char measurement_reg = (unsigned char)mode & 0b00000011;
  measurement_reg |= (((unsigned char)temp & 0b00000111) << 5u);
  measurement_reg |= (((unsigned char)pres & 0b00000111) << 2u);

  if (BME280_WriteReg8(BME280_CTRL_MEAS, measurement_reg))
    return -2;

  return 0;
}

int BME280_FetchCompensationValues (void)
{
  unsigned char scratchL = 0;
  unsigned char scratchH = 0;

  // dig_T1 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0x88, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0x89, &scratchH))
    return -1;

  BME280_CompTable.dig_T1 = (unsigned short)(scratchL + ((unsigned int)scratchH << 8u));
  // dig_T1 //////////////////////////////////////////////////////////////////

  // dig_T2 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0x8A, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0x8B, &scratchH))
    return -1;

  BME280_CompTable.dig_T2 = (short)(scratchL + ((unsigned int)scratchH << 8u));
  // dig_T2 //////////////////////////////////////////////////////////////////

  // dig_T3 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0x8C, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0x8D, &scratchH))
    return -1;

  BME280_CompTable.dig_T3 = (short)(scratchL + ((unsigned int)scratchH << 8u));
  // dig_T3 //////////////////////////////////////////////////////////////////

  // dig_P1 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0x8E, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0x8F, &scratchH))
    return -1;

  BME280_CompTable.dig_P1 = (uint16_t)scratchH << 8 | scratchL;
  // dig_P1 //////////////////////////////////////////////////////////////////

  // dig_P2 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0x90, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0x91, &scratchH))
    return -1;

  BME280_CompTable.dig_P2 = (int16_t)scratchH << 8 | scratchL;
  // dig_P2 //////////////////////////////////////////////////////////////////

  // dig_P3 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0x92, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0x93, &scratchH))
    return -1;

  BME280_CompTable.dig_P3 = (int16_t)scratchH << 8 | scratchL;
  // dig_P3 //////////////////////////////////////////////////////////////////

  // dig_P4 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0x94, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0x95, &scratchH))
    return -1;

  BME280_CompTable.dig_P4 = (int16_t)scratchH << 8 | scratchL;
  // dig_P4 //////////////////////////////////////////////////////////////////

  // dig_P5 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0x96, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0x97, &scratchH))
    return -1;

  BME280_CompTable.dig_P5 = (int16_t)scratchH << 8 | scratchL;
  // dig_P5 //////////////////////////////////////////////////////////////////

  // dig_P6 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0x98, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0x99, &scratchH))
    return -1;

  BME280_CompTable.dig_P6 = (int16_t)scratchH << 8 | scratchL;
  // dig_P6 //////////////////////////////////////////////////////////////////

  // dig_P7 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0x9A, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0x9B, &scratchH))
    return -1;

  BME280_CompTable.dig_P7 = (int16_t)scratchH << 8 | scratchL;
  // dig_P7 //////////////////////////////////////////////////////////////////

  // dig_P8 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0x9C, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0x9D, &scratchH))
    return -1;

  BME280_CompTable.dig_P8 = (int16_t)scratchH << 8 | scratchL;
  // dig_P8 //////////////////////////////////////////////////////////////////

  // dig_P9 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0x9E, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0x9F, &scratchH))
    return -1;

  BME280_CompTable.dig_P9 = (int16_t)scratchH << 8 | scratchL;
  // dig_P9 //////////////////////////////////////////////////////////////////

  // dig_H1 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0xA1, &scratchL))
    return -1;

  BME280_CompTable.dig_H1 = (unsigned char)(scratchL);
  // dig_H1 //////////////////////////////////////////////////////////////////

  // dig_H2 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0xE1, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0xE2, &scratchH))
    return -1;

  BME280_CompTable.dig_H2 = (short)(scratchL + scratchH * 0x100u);
  // dig_H2 //////////////////////////////////////////////////////////////////

  // dig_H3 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0xE3, &scratchL))
    return -1;

  BME280_CompTable.dig_H3 = (unsigned char)(scratchL);
  // dig_H3 //////////////////////////////////////////////////////////////////

  // dig_H4 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0xE4, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0xE5, &scratchH))
    return -1;

  BME280_CompTable.dig_H4 = (short)(((unsigned int)(scratchL) << 4u) | (scratchH & 0x0F));
  // dig_H4 //////////////////////////////////////////////////////////////////

  // dig_H5 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0xE5, &scratchL))
    return -1;

  if (BME280_ReadReg8 (0xE6, &scratchH))
    return -1;

  BME280_CompTable.dig_H5 = (short)(((scratchL & 0xF0) >> 4u) | ((unsigned int)(scratchH) << 4u));
  // dig_H5 //////////////////////////////////////////////////////////////////

  // dig_H6 //////////////////////////////////////////////////////////////////
  if (BME280_ReadReg8 (0xE7, &scratchL))
    return -1;

  BME280_CompTable.dig_H6 = (char)(scratchL);
  // dig_H6 //////////////////////////////////////////////////////////////////

  return 0;
}

int BME280_Busy (void)
{
  unsigned char status = 0;
  
  if (BME280_ReadReg8(BME280_STATUS, &status))
    return -1;

  // either measuring or im_update ongoing
  return status & 0b00001001;
}

//returns the 20-bit uncompensated temeperature value as an unsigned 32-bit type
uint32_t BME280_raw_T (void)
{
  uint32_t adc_T;
  uint8_t msb, lsb, xlsb;

  while (BME280_Busy())
  {;;;}

  (void)BME280_ReadReg8(BME280_TEMP + 0, &msb);
  (void)BME280_ReadReg8(BME280_TEMP + 1, &lsb);
  (void)BME280_ReadReg8(BME280_TEMP + 2, &xlsb);

  adc_T = (uint32_t)msb << 12 | (uint32_t)lsb << 4 | (uint32_t)xlsb >> 4;

  return adc_T;
}

//returns the 20-bit uncompensated pressure value as an unsigned 32-bit type
uint32_t BME280_raw_P (void)
{
  uint32_t adc_P;
  uint8_t msb, lsb, xlsb;

  while (BME280_Busy())
  {;;;}

  (void)BME280_ReadReg8(BME280_PRESS + 0, &msb);
  (void)BME280_ReadReg8(BME280_PRESS + 1, &lsb);
  (void)BME280_ReadReg8(BME280_PRESS + 2, &xlsb);

  adc_P = (uint32_t)msb << 12 | (uint32_t)lsb << 4 | (uint32_t)xlsb >> 4;

  return adc_P;
}

//returns the 16-bit uncompensated humidity value as an unsigned 32-bit type
uint32_t BME280_raw_H (void)
{
  uint32_t adc_H;
  uint8_t msb, lsb;

  while (BME280_Busy())
  {;;;}

  (void)BME280_ReadReg8(BME280_HUM + 0, &msb);
  (void)BME280_ReadReg8(BME280_HUM + 1, &lsb);

  adc_H = (uint32_t)msb << 8 | (uint32_t)lsb;

  return adc_H;
}

//returns temperature in DegC, resolution is 0.01 DegC. Output value of "5123" equals 51.23 DegC.
int32_t BME280_compensate_T_int32 (int32_t adc_T)
{
  int32_t var1;
  int32_t var2;
  int32_t temperature;
  int32_t temperature_min = -4000;
  int32_t temperature_max = 8500;  

  var1 = (int32_t)((adc_T / 8) - ((int32_t)BME280_CompTable.dig_T1 * 2));
  var1 = (var1 * ((int32_t)BME280_CompTable.dig_T2)) / 2048;
  var2 = (int32_t)((adc_T / 16) - ((int32_t)BME280_CompTable.dig_T1));
  var2 = (((var2 * var2) / 4096) * ((int32_t)BME280_CompTable.dig_T3)) / 16384;
  BME280_t_fine = var1 + var2;
  temperature = (BME280_t_fine * 5 + 128) / 256;

  if (temperature < temperature_min)
    temperature = temperature_min;
  else if (temperature > temperature_max)
    temperature = temperature_max;

  return temperature;		
}

//returns pressure as Pa
//output value of "96386" equals 96386Pa = 963hPa
//Note: remember to compensate for the altitude of the device
uint32_t BME280_compensate_P_int32(int32_t adc_P)
{
  int32_t var1, var2;
  uint32_t p;

  var1 = (((int32_t)BME280_t_fine) >> 1) - (int32_t)64000;
  var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)BME280_CompTable.dig_P6);
  var2 = var2 + ((var1 * ((int32_t)BME280_CompTable.dig_P5)) << 1);
  var2 = (var2 >> 2) + (((int32_t)BME280_CompTable.dig_P4) << 16);
  var1 = (((BME280_CompTable.dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)BME280_CompTable.dig_P2) * var1) >> 1)) >> 18;
  var1 = ((((32768 + var1)) * ((int32_t)BME280_CompTable.dig_P1)) >> 15);
  if (var1 == 0)
    return 0;

  p = (((uint32_t)(((int32_t)1048576) - adc_P) - (var2 >> 12))) * 3125;

  if (p < 0x80000000)
    p = (p << 1) / ((uint32_t)var1);	
  else
    p = (p / (uint32_t)var1) * 2;

  var1 = (((int32_t)BME280_CompTable.dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
  var2 = (((int32_t)(p >> 2)) * ((int32_t)BME280_CompTable.dig_P8)) >> 13;

  p = (uint32_t)((int32_t)p + ((var1 + var2 + BME280_CompTable.dig_P7) >> 4));
  return p;
}

//returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits)
//output value of "47445" represents 47445/1024 = 46.333 %RH
uint32_t BME280_compensate_H_int64(int32_t adc_H)
{
  int32_t var1;
  int32_t var2;
  int32_t var3;
  int32_t var4;
  int32_t var5;
  uint32_t humidity;
  uint32_t humidity_max = 102400;

  var1 = BME280_t_fine - ((int32_t)76800);
  var2 = (int32_t)(adc_H * 16384);
  var3 = (int32_t)(((int32_t)BME280_CompTable.dig_H4) * 1048576);
  var4 = ((int32_t)BME280_CompTable.dig_H5) * var1;
  var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
  var2 = (var1 * ((int32_t)BME280_CompTable.dig_H6)) / 1024;
  var3 = (var1 * ((int32_t)BME280_CompTable.dig_H3)) / 2048;
  var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
  var2 = ((var4 * ((int32_t)BME280_CompTable.dig_H2)) + 8192) / 16384;
  var3 = var5 * var2;
  var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
  var5 = var3 - ((var4 * ((int32_t)BME280_CompTable.dig_H1)) / 16);
  var5 = (var5 < 0 ? 0 : var5);
  var5 = (var5 > 419430400 ? 419430400 : var5);
  humidity = (uint32_t)(var5 / 4096);

  if (humidity > humidity_max)
    humidity = humidity_max;

  return humidity;	
}

