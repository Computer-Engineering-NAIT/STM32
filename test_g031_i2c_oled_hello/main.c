// Basic STM32G031 Demo SSD1306 Basic Output
// Core clock is 64MHz
//
// This requires the I2C1 config on pins PB6/PB7 (d0 and d1)
// I2C Bus pull-ups are required.
//
// Using Simon's Libraries
//////////////////////////////////////

#include "stm32g031xx.h"
#include "cmsis_gcc.h"

#include <stdio.h>
#include "gpio.h"
#include "pll.h"
#include "usart.h"
#include "i2c.h"
#include "SSD1306.h"

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
    int iLine = 5;
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

  // begin main program loop
  int iFastCounter = 0;
  int iSlowCounter = 0;
  do
  { 
    // count at long count intervals
    if (!(iFastCounter++ % 1000000))
    {
      // every 1M iterations:
      char buff[80];
      (void)sprintf(buff, "Count is %d", iSlowCounter++);
      _SSD1306_StringXY(0, 1, buff);
      
      // perform display inversion every 10 counts
      if (!(iSlowCounter % 10))
        _SSD1306_SetInverse(iSlowCounter / 10 % 2);
      
      // cause changes to be rendered on the device
      _SSD1306_Render();
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
  _USART2_TxStringXY(1, 3, "starting I2C bus...");
  _I2C1_Init ();
  _USART2_TxStringXY(1, 4, "I2C bus started...");

  // one-time OLED init
  _SSD1306_DispInit(_SSD1306_OR_UP);
  _SSD1306_DisplayOn();
}

