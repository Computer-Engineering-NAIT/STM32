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

// should work on other variants
//#include "stm32g041xx.h"
#include "stm32g031xx.h"

// based on the following for the GPIO port structure
//typedef struct
//{
//  __IO uint32_t MODER;       /*!< GPIO port mode register,               Address offset: 0x00      */
//  __IO uint32_t OTYPER;      /*!< GPIO port output type register,        Address offset: 0x04      */
//  __IO uint32_t OSPEEDR;     /*!< GPIO port output speed register,       Address offset: 0x08      */
//  __IO uint32_t PUPDR;       /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C      */
//  __IO uint32_t IDR;         /*!< GPIO port input data register,         Address offset: 0x10      */
//  __IO uint32_t ODR;         /*!< GPIO port output data register,        Address offset: 0x14      */
//  __IO uint32_t BSRR;        /*!< GPIO port bit set/reset  register,     Address offset: 0x18      */
//  __IO uint32_t LCKR;        /*!< GPIO port configuration lock register, Address offset: 0x1C      */
//  __IO uint32_t AFR[2];      /*!< GPIO alternate function registers,     Address offset: 0x20-0x24 */
//  __IO uint32_t BRR;         /*!< GPIO Bit Reset register,               Address offset: 0x28      */
//  __IO uint32_t ASCR;        /*!< GPIO analog switch control register,   Address offset: 0x2C      */
//} GPIO_TypeDef;


// Pin Modes
typedef enum
{
  _GPIO_PinMode_Input = 0x00,              // digital input
  _GPIO_PinMode_Output = 0x01,             // digital output
  _GPIO_PinMode_AlternateFunction = 0x02,  // alternate function (peripheral use)
  _GPIO_PinMode_Analog = 0x03 // default   // analog input (default, unmapped)
} _GPIO_PinMode;

// fast, no error checking versions of set, clear, and toggle for outputs
#define _GPIO_PinToggle(p, num) ((p)->ODR ^= (1u << (num)))
#define _GPIO_PinSet(p, num) ((p)->BSRR |= (1u << (num)))
#define _GPIO_PinClear(p, num) ((p)->BSRR |= (0x10000u << (num)))

// set the mode of an individual pin, includes mode only Input, Output, AlternateFunction, Analog
void _GPIO_SetPinMode (GPIO_TypeDef * pPort, int PinNumber, _GPIO_PinMode mode);

// get the state of the specified pin (0 == clear, !0 == set)
// from IDR (IDR is read only, assumes input pin)
int _GPIO_GetPinIState (GPIO_TypeDef * pPort, int PinNumber);

// get the state of the specified pin (0 == clear, !0 == set)
// from ODR (IDR is R/W, assumes buffered output state)
int _GPIO_GetPinOState (GPIO_TypeDef * pPort, int PinNumber);
