// header for BME280, 7-bit address 0x76
// Simon Walker, NAIT
// Edited by: Joseph Limin, NAIT
// Revision History:
//  [see implementation file]
//
//Software by 2020 Bosch Sensortec GmbH. used
//Copyright (c) 2020 Bosch Sensortec GmbH. All rights reserved.

///////////////////////////////////////////////////////////
// BME280 Sample Use
//if (BME280_Init())
//{
//  _USART2_TxStringXY(1, 18, "BME280 Init Failure...");
//}
//else
//{
//  char buff [80];
//  uint32_t temp = BME280_compensate_T_int32(BME280_raw_T());    
//  (void)sprintf (buff, "BME280 Temp : %0.2fC     ", temp / 100.0f);
//  _USART2_TxStringXY(1, 19, buff);

//  uint32_t hum = BME280_compensate_H_int64(BME280_raw_H());
//  (void)sprintf (buff, "BME280 Humidity : %0.2f%%     ", hum / 1024.0f);
//  _USART2_TxStringXY(1, 20, buff);

//  uint32_t press = BME280_compensate_P_int32(BME280_raw_P());
//  (void)sprintf (buff, "BME280 Pressure : %0.2fhPa     ", press / 100.0f + 78); // compensation (78) for 670m altitude
//  _USART2_TxStringXY(1, 21, buff);
//}
///////////////////////////////////////////////////////////

#include "stdint.h"

#ifndef BME280ADDR
#define BME280ADDR 0x76
#endif

// registers
#define BME280_ID 0xD0
#define BME280_RESET 0xE0
#define BME280_CTRL_HUM 0xF2
#define BME280_STATUS 0xF3
#define BME280_CTRL_MEAS 0xF4
#define BME280_CONFIG 0xF5
#define BME280_PRESS 0xF7
#define BME280_TEMP 0xFA
#define BME280_HUM 0xFD

typedef enum BME280_Oversampling
{
  BME280_OS_1x  = 0b001,
  BME280_OS_2x  = 0b010,
  BME280_OS_4x  = 0b011,
  BME280_OS_8x  = 0b100,
  BME280_OS_16x = 0b101,
} BME280_Oversampling;

typedef enum BME280_SensorMode
{
  BME280_ModeSleep =  0b00,
  BME280_ModeForced = 0b01,
  BME280_ModeNormal = 0b11
} BME280_SensorMode;

// private helpers
int BME280_ReadReg8 (unsigned char TargetRegister, unsigned char * TargetValue);
int BME280_WriteReg8 (unsigned char TargetRegister, unsigned char TargetValue);

int BME280_Init (void);
int BME280_SW_RESET (void);
int BME280_SetOversampling (BME280_Oversampling hum, BME280_Oversampling pres, BME280_Oversampling temp, BME280_SensorMode mode);
int BME280_GetData (unsigned char * pData); // requires 8 bytes
int BME280_FetchCompensationValues (void); // fetch compensation values into the compensation table (struct)

//Raw values:
uint32_t BME280_raw_T (void);
uint32_t BME280_raw_P (void);
uint32_t BME280_raw_H (void);

//Compensation:
int32_t BME280_compensate_T_int32(int32_t adc_T);
uint32_t BME280_compensate_P_int32(int32_t adc_P);
uint32_t BME280_compensate_H_int64(int32_t adc_H);


