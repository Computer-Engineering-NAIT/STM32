// Basic STM32G031 Demo ADXL343 Basic Operations
// Core clock is 64MHz
//
// This requires the I2C1 config on pins PB6/PB7 (d0 and d1)
// This demo uses the serial port for information output
// I2C Bus pull-ups are required.
//
// Using Simon's Libraries
//////////////////////////////////////

#include "stm32g031xx.h"
#include "cmsis_gcc.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include "gpio.h"
#include "pll.h"
#include "usart.h"
#include "i2c.h"
#include "ADXL343.h"

// perform one-time initializations before the main loop
void _OneTimeInits(void);

int main(void)
{
  // device should be running on HSI / 1 so 16MHz
  // not using external 32kHz XTAL yet
  // kick it up to 64MHz
  _PLL_To_64();

  // perform one-time startups for the program, but at full speed
  _OneTimeInits();  

  // I2C bus scan to see what devices are reporting in
  {
    unsigned char pData[128] = {0};
    _I2C1_BusScan(pData);
    int iLine = 7;
    for (int i = 0; i < 128; ++i)
    {
      if (pData[i])
      {
        char buff[80] = {0};
        (void)sprintf (buff, "Device found at 7-bit address: %2.2X ", i);
        _USART2_TxStringXY(1, iLine++, buff);
        // or print to debug console
        printf("%s\r\n", buff);
      }
    }
  }

  // maximum readings
  float mx, my, mz;

  // begin main program loop
  do
  { 
    // show accelerometer data
    {
      short accx, accy, accz;

      // accelerometer stuff
      if (!_ADXL343_ReadRegAccData(&accx, &accy, &accz))
      {
        float gx = accx * 0.0039; // sensitivity is 256 LSB/g (3.9mg/LSB)
        float gy = accy * 0.0039;
        float gz = accz * 0.0039;

        // update maximums
        if (fabs(gx) > fabs(mx))
          mx = gx;
        if (fabs(gy) > fabs(my))
          my = gy;
        if (fabs(gz) > fabs(mz))
          mz = gz;

        // show values and orientations        
        char sx = gx > 0.3 ? '+' : '=';
        sx = gx < -0.3 ? '-' : sx;
        char sy = gy > 0.3 ? '+' : '=';
        sy = gy < -0.3 ? '-' : sy;
        char sz = gz > 0.3 ? '+' : '=';
        sz = gz < -0.3 ? '-' : sz;
        
        char buff[80];

        (void)sprintf (buff, "X: %7.3f Y: %7.3f Z: %7.3f", mx, my, mz);
        _USART2_TxStringXY(1, 17, buff);

        (void)sprintf (buff, "X: %7.3f Y: %7.3f Z: %7.3f", gx, gy, gz);
        _USART2_TxStringXY(1, 18, buff);

        (void)sprintf (buff, "X: %c", sx);
        _USART2_TxStringXY(1, 20, buff);
        (void)sprintf (buff, "Y: %c", sy);
        _USART2_TxStringXY(1, 21, buff);
        (void)sprintf (buff, "Z: %c", sz);
        _USART2_TxStringXY(1, 22, buff);        
      }
    }
  }
  while (1);
}

void _OneTimeInits(void)
{
  // turn on GPIO clock for Port A - can't do anything without this
  RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

  // bring up the serial port, 38400 BAUD currently on VCOM through programmer
  _USART2_Init(64E6, 38400);
  _USART2_ClearScreen();

  _USART2_TxStringXY(1, 1, "USART started...");

  // I2C Stuff
  _USART2_TxStringXY(1, 2, "starting I2C bus...");
  _I2C1_Init ();
  _USART2_TxStringXY(1, 3, "I2C bus started...");

  // one-time accelerometer start
  _USART2_TxStringXY(1, 4, "starting ADXL343...");
  _ADXL343_Init();
  _USART2_TxStringXY(1, 5, "ADXL343 started...");  
}

