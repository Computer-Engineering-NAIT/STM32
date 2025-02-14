// Basic STM32G031 Demo I2C1 bus scan
//
// This requires the I2C1 config on pins PB6/PB7 (d0 and d1)
// I2C Bus pull-ups are required.
//
// Using Simon's Libraries
//////////////////////////////////////

#include "stm32g031xx.h"
#include "cmsis_gcc.h"

#include <stdio.h>
//#include <string.h>
//#include <stdlib.h>
#include "gpio.h"
#include "pll.h"
#include "usart.h"
#include "i2c.h"
//#include "ADXL343.h"
//#include "SSD1306.h"
//#include "util.h"

// global counter from interrupt timer
//volatile uint32_t _GlobalCounter = 0;

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
        
        // and/or print to debug console
        printf("%s\r\n", buff);
      }
    }
  }

  // begin main program loop
  do
  { 
    // wait for an interrupt:
    // sources :: 100ms tim2 interrupt
    //         :: accelerometer tap
    //__WFI();
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

  //// setup a basic timer
  //_USART2_TxStringXY(1, 8, "starting timer interrupt...");  
  //RCC->APBENR1 |= RCC_APBENR1_TIM2EN_Msk; // turn on timer 2
  //TIM2->CR1 = TIM_CR1_URS_Msk;            // overflow/underflow only
  //TIM2->ARR = 6400000;                    // 100ms (current prescale is 1)
  //TIM2->DIER |= TIM_DIER_UIE_Msk;
  //TIM2->CR1 |= TIM_CR1_CEN_Msk;

  //NVIC_EnableIRQ (TIM2_IRQn);
  //__enable_irq();
  //_USART2_TxStringXY(1, 9, "timer interrupt started...");

  // one-time accelerometer start
  //_USART2_TxStringXY(1, 10, "starting ADXL343...");
  //_ADXL343_Init();
  //_USART2_TxStringXY(1, 11, "ADXL343 started...");

  // turn on activity interrupt on accelerometer
  //(void)_ADXL343_ReadInterruptSource();
  //_ADXL343_ActivityInterrupt(10);

  // one-time OLED init
  //_SSD1306_DispInit(_SSD1306_OR_Down);
  //_SSD1306_DisplayOn();

  // small delay before we begin (to see log)
  //while (_GlobalCounter < 10)
  //{;;;}

  // clear and enter main loop
  //_USART2_ClearScreen();
}

//void TIM2_IRQHandler (void)
//{
//  // check if read write back is allowed here
//  TIM2->SR &= ~TIM_SR_UIF_Msk;

//  // clear the pending interrupt request
//  NVIC_ClearPendingIRQ (TIM2_IRQn);

//  // count this tick
//  ++_GlobalCounter;
//}


//void ShowI2CBusScan (void)
//{
//  _USART2_TxStringXY(40, 1, "Starting I2C Bus Scan...");
//  unsigned char addrs [128] = {0};
//  _I2C1_BusScan(addrs);
//  _USART2_GotoXY(1, 2);
//  for (int i = 0; i < 128; ++i)
//  {
//    if (addrs[i])
//    {
//      char buff[80] = {0};
//      (void)sprintf (buff, "\r\nI2C 7-bit Addr: %2.2X", i);
//      _USART2_TxString(buff);
//    }
//  }
//  _USART2_TxStringXY(40, 1, "I2C Bus Scan complete...");
//}
