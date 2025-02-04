// PLL and Clocking library
//
// Functions to control the clocking options on the device
//
// Simon Walker
// Created September 2024, initial build
//
// Version 1.0
//
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

// should function with stm32g031xx.h as well
//#include "stm32g041xx.h"
#include "stm32g031xx.h"

#include "pll.h"

void _PLL_To_64 (void)
{
  // specific to G0 (possibly G041 variant)
  // 5.2.8 :: PLLRCLK max is 64MHz, and requires Range 1 (high core voltage)

  // 4.4.1 :: PWR_CR1 - VOS default is 01 -> range 1 (high performance)

  // 4.1.4 :: Go from Range 2 to Range 1 (1 should be default)
  // 1- program VOS to 01
  // 2- wait for VOSF
  // 3- adjust wait states for the desired frequency in FLASH_ACR
  // 4- increase system frequency

  // in the event we are going from a VOS change (should not happen)
  uint32_t cur = PWR->CR1; // can't write 00 or 11, so direct register mod not possible?
  cur &= ~0b00000000000000000000011000000000; // punch out VOS bits
  cur |=  0b00000000000000000000001000000000; // vos to 01 (range 1)
  PWR->CR1 = cur;
  
  //VOSF in PWR_SR2 needs to go clear
  while (PWR->SR2 & PWR_SR2_VOSF_Msk)
  {;;;}

  // 3.3.4 Flash requirements for increasing frequency
  // at 64 MHz we need two wait states

  // *also*   // turn on instruction cache and CPU prefetch
  cur = FLASH->ACR;
  cur &= ~FLASH_ACR_LATENCY_Msk; // punch out current bits for latency
  cur |= FLASH_ACR_LATENCY_2;
  cur |= FLASH_ACR_DBG_SWEN_Msk;
  cur |= FLASH_ACR_ICEN_Msk;
  cur |= FLASH_ACR_PRFTEN_Msk;
  FLASH->ACR = cur;

  // wait for what we wrote to take effect
  while ((FLASH->ACR & FLASH_ACR_LATENCY_Msk) != FLASH_ACR_LATENCY_2)
  {;;;}

  // clear to go fast... need to set PLL and use PLLR as clock source
  // turn on the HSI16 clock, need it as source of PLL
  // this should be on and ready out of reset (do it, in case this isn't out of reset)
  RCC->CR |= RCC_CR_HSION_Msk;
  while (!(RCC->CR & RCC_CR_HSIRDY))
  {;;;}

  // now, bring SYSCLK via PLLRCLK clock up to 64MHz
  // 5.2.4
  /*
    The PLL configuration (selection of the input clock and multiplication factor) must be done
    before enabling the PLL. Once the PLL is enabled, these parameters cannot be changed.
    To modify the PLL configuration, proceed as follows:
    1. Disable the PLL by setting PLLON to 0 in Clock control register (RCC_CR).
    2. Wait until PLLRDY is cleared. The PLL is now fully stopped.
    3. Change the desired parameter.
    4. Enable the PLL again by setting PLLON to 1.
    5. Enable the desired PLL outputs by configuring PLLPEN, PLLQEN, and PLLREN in PLL
    configuration register (RCC_PLLCFGR).
  */

  // turn off PLL (should be off already)
  RCC->CR &= ~RCC_CR_PLLON_Msk;

  // wait for PLL to fully stop
  while (RCC->CR & RCC_CR_PLLRDY_Msk)
  {;;;}
  
  // set PLL clock parameters
  RCC->PLLCFGR = 0b00100110000000100000100000000010;
  // PLLR    = / 2
  // PLLREN  = 0
  // PLLQ    = / 4
  // PLLQEN  = 0
  // PLLP    = /2
  // PLLPEN  = 0
  // PLLN    = * 8
  // PLLM    = / 1
  // PLLSRC  = HSI16

  // enable PLL R (main PLL) output
  RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;

  // turn on PLL and check status to ensure PLL is locked
  RCC->CR |= RCC_CR_PLLON_Msk;
  while (!(RCC->CR & RCC_CR_PLLRDY))
  {;;;}

  // select PLL as clock source
  RCC->CFGR |= 0x02; // bottom three bits are SW, 010 means use PLLR  
}