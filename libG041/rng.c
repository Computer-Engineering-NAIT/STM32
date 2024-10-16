// RNG library
//
// Functions to control the clocking RNG module and generate 
//  random numbers
//
// Simon Walker
// Created September 2024, initial build
//
// R1.0 - Initial
// R1.1 - Added error checking and correction code
//      - Added but removed interrupt code - appears to stay
//      -  in ISR like the interrupt is not being cleared
//      -  ISR not super practical in this case, so check later...
//////////////////////////////////////////////////////////////////

#include <stdio.h>
//#include <stdlib.h>
//#include <math.h>

// not at all available on non RNG/AES devices (so G031 series is excluded)
#include "stm32g041xx.h"

#include "rng.h"

// default dedicated clock for RNG is HSI/8, so 2MHz
// RNG needs to be faster than bus / 32, so should be OK
void _RNG_Init16 (void)
{
  // reset the RNG? <<< you don't seem to want to do this...
  //RCC->AHBRSTR |= RCC_AHBRSTR_RNGRST;

  // power up the RNG
  RCC->AHBENR |= RCC_AHBENR_RNGEN;

  // unique to G041:
  RCC->CCIPR &= ~0b00111100000000000000000000000000;
  RCC->CCIPR |=  0b00000100000000000000000000000000; // late stage, div by 1 (hsi already /8)

  // clock error detection turned on
  RNG->CR &= ~RNG_CR_CED_Msk;

  // turn on RNG interrupts
  //RNG->CR |= RNG_CR_IE_Msk;

  // turn on RNG
  RNG->CR |= RNG_CR_RNGEN;  
}

// if the system clock is running at 64MHz, the default config for the RNG is no good
// need to switch to SysClk as source, with div by 8 to drop the RNG clock to 8MHz
void _RNG_Init64 (void)
{
  // reset the RNG? <<< you don't seem to want to do this...
  //RCC->AHBRSTR |= RCC_AHBRSTR_RNGRST;

  // power up the RNG
  RCC->AHBENR |= RCC_AHBENR_RNGEN;

  // unique to G041:
  RCC->CCIPR &= ~0b00111100000000000000000000000000;
  RCC->CCIPR |=  0b00111000000000000000000000000000; // late stage, div by 8 (and use SYSCLK)

  // clock error detection turned on
  RNG->CR &= ~RNG_CR_CED_Msk;

  // turn on RNG interrupts
  //RNG->CR |= RNG_CR_IE_Msk;

  // turn on RNG
  RNG->CR |= RNG_CR_RNGEN;  
}

int _RNG_GetRND (unsigned int * pData)
{
  if (RNG->SR & RNG_SR_SEIS_Msk)
  {
    // seed (generation) error
    // write SEIS as zero to clear the flag (19.3.7)
    RNG->SR = ~RNG_SR_SEIS_Msk;

    // discard all pending data elements in the FIFO and conditioner
    for (int i = 0; i < 12; ++i)
    {
      uint32_t iDiscard = RNG->DR;
    }
    
    printf ("RNG:: seed error detected");

    // validate SEIS is still cleared (no talk of SECS?)
    return -3;    
  }

  if (RNG->SR & RNG_SR_CEIS_Msk)
  {
    // clocking issue, should not happen, and should recover
    // clear the clocking issue flag, but make a note of it in debug configs
    
    // clear CEIS
    RNG->SR = ~RNG_SR_CEIS_Msk;

    printf ("RNG:: clock error detected");

    return -2;
  }

  // is a random number available?
  if (RNG->SR & RNG_SR_DRDY)
  {
    *pData = RNG->DR;
    return 0;
  }

  // a number isn't ready
  return -1;
}

double _RNG_GetRNDD (void)
{
  unsigned int iVal;

  int iResult;
  do
  {
    iResult = _RNG_GetRND (&iVal);
  }
  while (iResult); // not ready or recovering from error, just wait longer
  
  // got one, not in error conditions, so return it as a double
  return ((double)iVal / ((int64_t)UINT32_MAX + 1));
}

// inclusive/exclusive range function
int _RNG_GetRndRange (int iLow, int iHigh)
{
  double dVal = _RNG_GetRNDD();  
  return (int)((iHigh - iLow) * dVal + iLow);
}

//void ShowRandomStats (void)
//{
//  // random number stuff
//  int const numruns = 50000;
  
//  {
//    char buff [80] = {0};
//    (void)sprintf (buff, "RND Numbers, two die stats (2 - 12) [%d] rolls", numruns);
//    _USART2_TxStringXY(1, 12, buff);  
//  }

//  int stats [11] = {0};
//  for (int i = 0; i < numruns; ++i)
//  {
//    int rnd1 = _RNG_GetRndRange(1, 7);
//    int rnd2 = _RNG_GetRndRange(1, 7);
//    stats[rnd1+rnd2-2]++;

//    if (i % 1000 == 0)
//    {
//      char buff [50] = {0};
//      (void)sprintf (buff, "Processing roll [%7.0d]   ", i);
//      _USART2_TxStringXY(1, 13, buff);
//    }
//  }
//  for (int i = 0; i < 11; ++i)
//  {
//    char buff[50] = {0};
//    (void)sprintf (buff, "Outcome %2.0d : %5.0d [%5.2f%%]", i+2, stats[i], 100 * stats[i] / (float)numruns);
//    _USART2_TxStringXY (1, 13 + i, buff);
//  }
//}