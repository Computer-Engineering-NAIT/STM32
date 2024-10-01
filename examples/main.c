// Die Roller Project
//
// Project to use RNG to pick random die rolls
//  Die count and side counts to be determined by user
//  Rolls started with ADXL343 accelerometer (activity, tap, or shake)
//
// Simon Walker
// Created September 2024, initial build
//
// Version 1.0
// 
// TODO: RNG appears to bail on random number generation after a while
//       lib needs to be checked.
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
//#include <stdlib.h>
#include "gpio.h"
#include "pll.h"
#include "usart.h"
#include "i2c.h"
#include "rng.h"
#include "LM75A.h"
#include "ADXL343.h"
#include "SSD1306.h"
#include "util.h"
#include "rtc.h"

#include "stm32g041xx.h"
#include "cmsis_gcc.h"

// POC function, generates random stats, not used in final version
// void ShowRandomStats (void);

volatile uint32_t _GlobalCounter = 0;
volatile static int resetcounter = 0;

void GPIO_Basic_Init(void);

// PIN ASSIGNMENTS USED IN THIS PROJECT
///////////////////////////////////////
// A0 -> RED LED
// A1 -> GREED LED
// A4 -> Left
// A5 -> Centre
// A6 -> Right
// A7 -> Accelerometer Interrupt
///////////////////////////////////////

int GPIO_LeftPushed (void)
{
  return GPIOA->IDR & GPIO_IDR_ID4_Msk;     
}

int GPIO_LeftTransitioned (void)
{
  // will be invalid on first use
  //  this function needs to be called one time
  //  at startup to load the correct state  
  static uint32_t oldState; 
  
  // single point read
  uint32_t curstate = GPIOA->IDR & GPIO_IDR_ID4_Msk;

  if (curstate && (curstate != oldState))
  {
    oldState = curstate;
    return 1;
  }

  oldState = curstate;
  return 0;
}

#define _DIE_MAX_COUNT 8
#define _DIE_MAX_SIDES 100

// these only exist in the rolled state
typedef struct
{
  int iSides;
  int icount;
  char roll [_DIE_MAX_COUNT];
} SDieRoll;

void RollDice (int iCount, int iSides, SDieRoll * pResult)
{
 for (int i = 0; i < iCount; ++i)
    pResult->roll[i] = _RNG_GetRndRange(1, iSides + 1);
}

int main(void)
{
  // used to make time updates more efficient
  _RTC_STime oldtime = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  // number of dice  : 1 - 8
  int DieCount = 5;

  // number of sides : 2 - 100
  int DieSides = 6;

  // device should be running on HSI / 1 so 16MHz
  // not using external 32kHz XTAL yet

  // configure port pins for LEDs and switches
  GPIO_Basic_Init();
  
  // kick it up to 64MHz
  _PLL_To_64();

  // bring up the serial port, 38400 BAUD currently on VCOM through programmer
  _USART2_Init(64E6, 38400);
  _USART2_ClearScreen();

  _USART2_TxStringXY(1, 1, "USART started...");

  _USART2_TxStringXY(1, 2, "starting RTC...");
  // turn on RTC with 'current' time <<< would be better if we had a battery backup...
  _RTC_STime curtime;
  curtime._RTC_Time_YT = 2;
  curtime._RTC_Time_YU = 4;
  curtime._RTC_Time_WDU = 5; //fri
  curtime._RTC_Time_MT = 0;
  curtime._RTC_Time_MU = 9;
  curtime._RTC_Time_DT = 2;
  curtime._RTC_Time_DU = 7;
  curtime._RTC_Time_PM = 0; //24 hour
  curtime._RTC_Time_HT = 1;
  curtime._RTC_Time_HU = 7;
  curtime._RTC_Time_MNT = 5;
  curtime._RTC_Time_MNU = 5;
  curtime._RTC_Time_ST = 0;
  curtime._RTC_Time_SU = 0;
  curtime._RTC_Time_SSR = 0; // not currently used
  _RTC_INIT_RESULT ires = _RTC_Init(curtime);
  // LSE now up, clock running
  _USART2_TxStringXY(1, 3, "RTC started...");

  // bring up the random number generator
  _USART2_TxStringXY(1, 4, "starting RNG...");  
  _RNG_Init64();
  _USART2_TxStringXY(1, 5, "RNG started...");

  // transition initializations
  (void)GPIO_LeftTransitioned();

  // note reset type
  if (ires == _RTC_INIT_WARM_RESET)
    _USART2_TxStringXY(5, 6, "RTC Init: Warm Reset");
  else
    _USART2_TxStringXY(5, 6, "RTC Init: Cold Reset");

  // I2C Stuff
  _USART2_TxStringXY(1, 7, "starting I2C bus...");
  _I2C1_Init ();
  _USART2_TxStringXY(1, 8, "I2C bus started...");
  
  // bus scan
  //{
  //  USART2_TxStringXY(40, 1, "Starting I2C Bus Scan...");
  //  unsigned char addrs [128] = {0};
  //  I2C1_BusScan(addrs);
  //  USART2_GotoXY(1, 2);
  //  for (int i = 0; i < 128; ++i)
  //  {
  //    if (addrs[i])
  //    {
  //      char buff[80] = {0};
  //      (void)sprintf (buff, "\r\nI2C 7-bit Addr: %2.2X", i);
  //      USART2_TxString(buff);
  //    }
  //  }
  //  USART2_TxStringXY(40, 1, "I2C Bus Scan complete...");
  //}

  // show the die stats with the random generator
  //ShowRandomStats();

  // setup a basic timer
  _USART2_TxStringXY(1, 9, "starting timer interrupt...");  
  RCC->APBENR1 |= RCC_APBENR1_TIM2EN_Msk; // turn on timer 2
  TIM2->CR1 = TIM_CR1_URS_Msk;            // overflow/underflow only
  TIM2->ARR = 6400000;                    // 100ms (current prescale is 1)
  TIM2->DIER |= TIM_DIER_UIE_Msk;
  TIM2->CR1 |= TIM_CR1_CEN_Msk;

  NVIC_EnableIRQ (TIM2_IRQn);
  __enable_irq();
  _USART2_TxStringXY(1, 10, "timer interrupt started...");

  // one-time accelerometer start
  _USART2_TxStringXY(1, 11, "starting ADXL343...");
  _ADXL343_Init();
  _USART2_TxStringXY(1, 10, "ADXL343 started...");

  // turn on activity interrupt on accelerometer
  _ADXL343_ActivityInterrupt(10);

  // one-time OLED init
  _SSD1306_DispInit(_SSD1306_OR_UP);
  _SSD1306_DisplayOn();

  while (_GlobalCounter < 15)
  {;;;}

  _USART2_ClearScreen();

  // begin main program loop
  do
  { 
    // wait for an interrupt:
    // sources :: 100ms tim2 interrupt
    //         :: accelerometer tap
    __WFI();

    // show interrupt pin state of accelerometer
    if (_GPIO_GetPinIState(GPIOA, 7))
    {
      // reading the interrupt source will also clear the interrupt flags
      unsigned char iSource = _ADXL343_ReadInterruptSource();

      // toggle red led to show activity on accelerometer
      _GPIO_PinToggle (GPIOA, 0);
    
      //if (iSource & 0b01000000)
      //  USART2_TxStringXY(1, 9, "Single_Tap");
      //if (iSource & 0b00100000)
      //  USART2_TxStringXY(1, 9, "Double_Tap");
      //if (iSource & 0b00010000)
      //  USART2_TxStringXY(1, 9, "Activity");
      //if (iSource & 0b00001000)
      //  USART2_TxStringXY(1, 9, "Inactivity");
    }
    
    // at 100ms intervals...
    {
      // toggle green LED for indication that something is going on...
      
      // toggle green LED state
      _GPIO_PinToggle (GPIOA, 1);

      //show temp (debug I2C lib, using LM75A,118)
      {
        char buff[80];
        (void)sprintf (buff, "Temp: %0.2fC            ", _LM75A_GetTempF()); 
        _USART2_TxStringXY(1, 3, buff);
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
          
          if (gx < -0.5f)
            _USART2_TxStringXY(1, 5, "-X");
          else if (gx > 0.5f)
            _USART2_TxStringXY(1, 5, "+X");
          else 
            _USART2_TxStringXY(1, 5, " X");

          if (gy < -0.5f)
            _USART2_TxStringXY(1, 6, "-Y");
          else if (gy > 0.5f)
            _USART2_TxStringXY(1, 6, "+Y");
          else 
            _USART2_TxStringXY(1, 6, " Y");

          if (gz < -0.5f)
            _USART2_TxStringXY(1, 7, "-Z");
          else if (gz > 0.5f)
            _USART2_TxStringXY(1, 7, "+Z");
          else 
            _USART2_TxStringXY(1, 7, " Z");

          //char buff[80];

          //(void)sprintf (buff, "X:%6.2f\x1b[0K", gx);
          //_USART2_TxStringXY(1, 5, buff);
          //(void)sprintf (buff, "Y:%6.2f\x1b[0K", gy);
          //_USART2_TxStringXY(1, 6, buff);
          //(void)sprintf (buff, "Z:%6.2f\x1b[0K", gz);
          //_USART2_TxStringXY(1, 7, buff);
        }
      }

      // show RTC time on terminal
      {
        _RTC_STime curtime = _RTC_Read();
        if (_RTC_STimeHashDiff(curtime, oldtime))
        {
          // format time as:
          // YYYY MM DD HH:MM:SS
          char buff [30];
          (void)sprintf (buff, " 20%1.1X%1.1X %1.1X%1.1X %1.1X%1.1X %1.1X%1.1X:%1.1X%1.1X:%1.1X%1.1X",
          curtime._RTC_Time_YT,
          curtime._RTC_Time_YU,
          curtime._RTC_Time_MT,
          curtime._RTC_Time_MU,
          curtime._RTC_Time_DT,
          curtime._RTC_Time_DU,
          curtime._RTC_Time_HT,
          curtime._RTC_Time_HU,
          curtime._RTC_Time_MNT,
          curtime._RTC_Time_MNU,
          curtime._RTC_Time_ST,
          curtime._RTC_Time_SU);

          _USART2_TxStringXY(1, 11, buff);
          _SSD1306_StringXY(0, 1, buff);

          //DEBUG: show a roll every second
          {
            SDieRoll dieroll;
            RollDice(DieCount, DieSides, &dieroll);
            char buff[80];
            (void)sprintf (buff, "#: %d   S: %d", DieCount, DieSides);
            _SSD1306_StringXY(1, 2, buff);
            for (int i = 0; i < DieCount; ++i)
            {
              (void)sprintf (buff, "%d", dieroll.roll[i]);
              _SSD1306_StringXY(i * 4, 3, buff);
            }
          }

          _SSD1306_Render();

          oldtime = curtime;
        } 
        
        // alternate the OLED inversion the oddness of the minutes
        // TODO: make this buffered so there is no redundant traffic
        _SSD1306_SetInverse(curtime._RTC_Time_MNU % 2);
      }
    } 
  }
  while (1);
}

void GPIO_Basic_Init(void)
{
  // RED LED on PA0
  // GREEN LED on PA1
  // A4 -> Left
  // A5 -> Centre
  // A6 -> Right
  // A7 -> Accelerometer Interrupt Pin

  // turn on GPIO clock for Port A - can't do anything without this
  RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

  // configure Red and Green LED as OUTPUT  
  // clear out mode bits, as the default for the port is input (analog) mode (11)
  _GPIO_SetPinMode(GPIOA, 0, _GPIO_PinMode_Output);
  _GPIO_SetPinMode(GPIOA, 1, _GPIO_PinMode_Output);

  // setup the switches as a digital INPUTs
  _GPIO_SetPinMode(GPIOA, 4, _GPIO_PinMode_Input);
  _GPIO_SetPinMode(GPIOA, 5, _GPIO_PinMode_Input);
  _GPIO_SetPinMode(GPIOA, 6, _GPIO_PinMode_Input);

  // setup the accelerometer input pin
  _GPIO_SetPinMode(GPIOA, 7, _GPIO_PinMode_Input);
}

void ShowRandomStats (void)
{
  // random number stuff
  int const numruns = 50000;
  
  {
    char buff [80] = {0};
    (void)sprintf (buff, "RND Numbers, two die stats (2 - 12) [%d] rolls", numruns);
    _USART2_TxStringXY(1, 12, buff);  
  }

  int stats [11] = {0};
  for (int i = 0; i < numruns; ++i)
  {
    int rnd1 = _RNG_GetRndRange(1, 7);
    int rnd2 = _RNG_GetRndRange(1, 7);
    stats[rnd1+rnd2-2]++;

    if (i % 1000 == 0)
    {
      char buff [50] = {0};
      (void)sprintf (buff, "Processing roll [%7.0d]   ", i);
      _USART2_TxStringXY(1, 13, buff);
    }
  }
  for (int i = 0; i < 11; ++i)
  {
    char buff[50] = {0};
    (void)sprintf (buff, "Outcome %2.0d : %5.0d [%5.2f%%]", i+2, stats[i], 100 * stats[i] / (float)numruns);
    _USART2_TxStringXY (1, 13 + i, buff);
  }
}

void TIM2_IRQHandler (void)
{
  // check if read write back is allowed here
  TIM2->SR &= ~TIM_SR_UIF_Msk;

  // clear the pending interrupt request
  NVIC_ClearPendingIRQ (TIM2_IRQn);

  // count this tick
  ++_GlobalCounter;
}


