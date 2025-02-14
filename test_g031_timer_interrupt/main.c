// Basic STM32G031 Demo TIM2 timer interrupt
// Core clock is 64MHz
//
// Using Simon's Libraries
//////////////////////////////////////

#include "stm32g031xx.h"
#include "cmsis_gcc.h"

#include <stdio.h>
#include "gpio.h"
#include "pll.h"
#include "usart.h"

// global counter from interrupt timer
volatile uint32_t _GlobalCounter = 0;

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

  // begin main program loop
  do
  { 
    // wait for an interrupt:
    // sources :: 100ms tim2 interrupt
    __WFI();

    // print interrupt count to the debug console if debugging
    //printf ("Interrupt count: %d\r\n", _GlobalCounter);

    // toggle the user LED on the NUCLEO board
    // LED blinking, on 100ms, off 100ms (200ms period)
    _GPIO_PinToggle (GPIOC, 6);

  }
  while (1);
}

void _OneTimeInits(void)
{
  // turn on GPIO clock for Port A - can't do anything without this
  RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

  // PC6 is user LED, so turn on peripheral clock for port c
  RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
  _GPIO_SetPinMode (GPIOC, 6, _GPIO_PinMode_Output); 

  // bring up the serial port, 38400 BAUD currently on VCOM through programmer
  _USART2_Init(64E6, 38400);
  _USART2_ClearScreen();

  _USART2_TxStringXY(1, 1, "USART started...");

  // setup a basic timer
  _USART2_TxStringXY(1, 8, "starting timer interrupt...");  
  RCC->APBENR1 |= RCC_APBENR1_TIM2EN_Msk; // turn on timer 2
  TIM2->CR1 = TIM_CR1_URS_Msk;            // overflow/underflow only
  TIM2->ARR = 6400000;                    // 100ms (current prescale is 1)
  TIM2->DIER |= TIM_DIER_UIE_Msk;
  TIM2->CR1 |= TIM_CR1_CEN_Msk;

  NVIC_EnableIRQ (TIM2_IRQn);
  __enable_irq();
  _USART2_TxStringXY(1, 9, "timer interrupt started...");
}

void TIM2_IRQHandler (void)
{
  // check if read write back is allowed here
  //TIM2->SR &= ~TIM_SR_UIF_Msk;
  TIM2->SR = ~TIM_SR_UIF_Msk; // yep, these are rc_w0 bits

  // clear the pending interrupt request
  NVIC_ClearPendingIRQ (TIM2_IRQn);

  // count this tick
  ++_GlobalCounter;
}


