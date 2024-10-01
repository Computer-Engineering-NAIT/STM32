// I2C library
//
// Basic I2C functions, intended to be used in device specific libraries
//
// Simon Walker
// Created September 2024, initial build
// 
// Version 1.0
//
////////////////////////////////////////////////////////////////////////

typedef enum
{
  _I2C_AutoEnd_Software = 0,
  _I2C_AutoEnd_Hardware = 1
} _I2C_AutoEndOption;

typedef enum
{
  _I2C_BusState_Idle = 0,
  _I2C_BusState_Busy = 1
} _I2C_BusState;

int _I2C1_Init (void);
int _I2C1_IsBusy (void);
int _I2C1_StartWrite (unsigned char address, unsigned char bytecount, unsigned char * pData, _I2C_AutoEndOption endoption);
int _I2C1_StartRead (unsigned char address, unsigned char bytecount, unsigned char * pData, _I2C_AutoEndOption endoption);
void _I2C1_BusScan (unsigned char * pAddressTable);