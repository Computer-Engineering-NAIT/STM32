// Basic STM32G031 Demo ADXL343 with activity/tap interrupts (GPIO interrupts)
// Core clock is 64MHz
//
// This requires the I2C1 config on pins PB6/PB7 (d0 and d1)
// This demo uses pin D2 (PA15) for activity interrupt 1 from the ADXL343
// This demo uses the user LED (PC6) to indicate activity
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

//      ADXL343        STM32
//      -----          -----
//      |   |          |   |
//      |   |AI1---PA15|   |
//      |   |          |   |
//      -----          -----

// this flag is set by an interrupt ISR when the ADXL343 reports an interrupt on the AI1 line
// the AI1 line is connected to the MCU on pin PA15, which is configured to interrupt on
// a rising edge. Each time this activity is detected, the interrupt will increase this flag
// that way, the flag has two purposes: to show the event, and to show how many time it has
// happened since the main code has gotten around to service the flag. This is not of much
// use here, but can in other circumstances (like budget pulse accumulation).
volatile int _AccActivityFlag = 0;

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

  // I2C bus scan to see what devices are reporting in, remove/comment out if not required
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
    // check for activity on the accelerometer (this comes from a flag in the interrupts ISR)
    if (_AccActivityFlag)
    {
      // clear activity flag that ISR set
      _AccActivityFlag = 0;

      // show action on the user LED
      _GPIO_PinSet (GPIOC, 6);

      // reading the interrupt source will also clear the interrupt flags internal to the ADXL343
      // this has nothing to do with the interrupt on PA15, or the STM32 in any way
      unsigned char iSource = _ADXL343_ReadInterruptSource();
    }
    else
    {
      // turn off the user LED this pass if nothing going on
      _GPIO_PinClear (GPIOC, 6);
    }    
    
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
  // turn on GPIO clock for Port A and C
  RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
  RCC->IOPENR |= RCC_IOPENR_GPIOCEN;

  // configure interrupt activity 1 from ADXL343 to pin PA15 as input (with rising edge interrupt)
  _GPIO_SetPinMode (GPIOA, 15, _GPIO_PinMode_Input); 

  // configure user LED for output
  _GPIO_PinClear (GPIOC, 6);
  _GPIO_SetPinMode (GPIOC, 6, _GPIO_PinMode_Output); 

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

  // turn on activity interrupt
  _ADXL343_ActivityInterrupt(10);
  (void)_ADXL343_ReadInterruptSource();

  // configure PA15 to be an interrupt
  // EXTICR already defaults to PA15 as line EXTI15 input in configurable mode RM0444::13.3.3
  //EXTI->EXTICR[0]

  //_GPIO_SetPinMode (GPIOA, 0, _GPIO_PinMode_Input); 
  //_GPIO_SetPinMode (GPIOA, 1, _GPIO_PinMode_Input); 
  
  // set CPU wakeup on EXTI15 line - RM0444::13.5.12
  EXTI->IMR1 |= EXTI_IMR1_IM15_Msk;
  //EXTI->IMR1 |= EXTI_IMR1_IM0_Msk;
  //EXTI->IMR1 |= EXTI_IMR1_IM1_Msk;
  
  // configure for rising edge on line EXTI15 - RM0444::13.5.1
  EXTI->RTSR1 |= EXTI_RTSR1_RT15_Msk;
  //EXTI->RTSR1 |= EXTI_RTSR1_RT0_Msk;
  //EXTI->RTSR1 |= EXTI_RTSR1_RT1_Msk;
  
  // event unmask (not necessary here, the interrupt is enough)
  //EXTI->EMR1 |= EXTI_EMR1_EM15_Msk;
  
  // enable the IRQ for this type of interrupt
  NVIC_EnableIRQ (EXTI4_15_IRQn);
  //NVIC_EnableIRQ (EXTI0_1_IRQn);

  // enable interrupts in general
  __enable_irq();
}

void EXTI0_1_IRQHandler (void)
{
  // need to differentiate flags before clearing (two sources for this channel)
  //EXTI->RPR1 = EXTI_RPR1_RPIF0_Msk;
  //EXTI->RPR1 = EXTI_RPR1_RPIF1_Msk;

  // clear the pending interrupt request
  //NVIC_ClearPendingIRQ (EXTI0_1_IRQn);
}

// ISR for interrupt source EXTI15 (configured as a PA15 rising edge trigger interrupt)
void EXTI4_15_IRQHandler (void)
{
  // clear the interrupt bit for a rising edge trigger RM0444::13.5.4
  EXTI->RPR1 = EXTI_RPR1_RPIF15_Msk;

  // increase the global flag count (sets flag)
  ++_AccActivityFlag;

  // clear the pending interrupt request
  NVIC_ClearPendingIRQ (EXTI4_15_IRQn);
}

