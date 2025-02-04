// GPIO Library
//
// Functions perform basic GPIO operations and configurations
//
// Simon Walker
// Created September 2024, initial build, ported from STM32L476 version
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

//#include <stdio.h>
//#include <stdlib.h>

// should work on other variants
//#include "stm32g041xx.h"
#include "gpio.h"

// quick macros included in header
//#define _GPIO_PinToggle(p, num) ((p)->ODR ^= (1u << (num)))
//#define _GPIO_PinSet(p, num) ((p)->BSRR |= (1u << (num)))
//#define _GPIO_PinClear(p, num) ((p)->BSRR |= (0x10000u << (num)))

// relevant registers made available from the derivative file:
//#define GPIO_MODER_MODE5_Pos           (10U)
//#define GPIO_MODER_MODE5_Msk           (0x3UL << GPIO_MODER_MODE5_Pos)         /*!< 0x00000C00 */
//#define GPIO_MODER_MODE5               GPIO_MODER_MODE5_Msk
//#define GPIO_MODER_MODE5_0             (0x1UL << GPIO_MODER_MODE5_Pos)         /*!< 0x00000400 */
//#define GPIO_MODER_MODE5_1             (0x2UL << GPIO_MODER_MODE5_Pos)         /*!< 0x00000800 */

//#define RCC_AHB2ENR_GPIOAEN_Pos        (0U)
//#define RCC_AHB2ENR_GPIOAEN_Msk        (0x1UL << RCC_AHB2ENR_GPIOAEN_Pos) /*!< 0x00000001 */
//#define RCC_AHB2ENR_GPIOAEN            RCC_AHB2ENR_GPIOAEN_Msk

void _GPIO_SetPinMode (GPIO_TypeDef * pPort, int PinNumber, _GPIO_PinMode mode)
{
  // assumes that peripheral clock is on for this port

  if (PinNumber < 0 || PinNumber > 15)
    return;

  // clear current mode bits
  pPort->MODER &= ~(0b11ul << (PinNumber * 2));
  
  // set desired mode bits
  pPort->MODER |= ((unsigned long)mode << (PinNumber * 2));  
}

int _GPIO_GetPinIState (GPIO_TypeDef * pPort, int PinNumber)
{
  // assumes that peripheral clock is on for this port

  if (PinNumber < 0 || PinNumber > 15)
    return 0;

  return (pPort->IDR & (1u << PinNumber));
}

int _GPIO_GetPinOState (GPIO_TypeDef * pPort, int PinNumber)
{
  // assumes that peripheral clock is on for this port

  if (PinNumber < 0 || PinNumber > 15)
    return 0;

  return (pPort->ODR & (1u << PinNumber));
}

