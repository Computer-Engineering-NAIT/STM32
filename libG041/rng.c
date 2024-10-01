// RNG library
//
// Functions to control the clocking RNG module and generate 
//  random numbers
//
// Simon Walker
// Created September 2024, initial build
////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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
  //RNG->CR |= RNG_CR_CED_Msk;

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
  //RNG->CR |= RNG_CR_CED_Msk;

  // turn on RNG
  RNG->CR |= RNG_CR_RNGEN;  
}

int _RNG_GetRND (unsigned int * pData)
{
  // any errors?
  if (RNG->SR & RNG_SR_SEIS || RNG->SR & RNG_SR_CEIS)
  {
    // error!
    return -1;
  }

  // is a random number available?
  if (RNG->SR & RNG_SR_DRDY)
  {
    *pData = RNG->DR;
    return 0;
  }

  // a number isn't ready
  return -2;
}

double _RNG_GetRNDD (void)
{
  unsigned int iVal;

  int iResult;
  do
  {
    iResult = _RNG_GetRND (&iVal);

    // error from RNG
    if (iResult == -1)
      return NAN;

    if (!iResult)
      break;
  }
  while (iResult == -2); // not ready, just wait longer
  
  // got one, not in error conditions, so return it as a double
  return ((double)iVal / ((int64_t)UINT32_MAX + 1));
}

// inclusive/exclusive range function
int _RNG_GetRndRange (int iLow, int iHigh)
{
  double dVal = _RNG_GetRNDD();  
  return (int)((iHigh - iLow) * dVal + iLow);
}