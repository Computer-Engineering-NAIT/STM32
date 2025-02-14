// Basic STM32G031 Demo PWM on Timer 3, Channel 1
// Core clock is 64MHz
//
// Reads the A/D value from PA12, and sets duty
// on TIM3::CH1 proportionally, at a fixed 1kHZ frequency.
//
// Requires user LED on PC6 (default for NUCLEO board)
// Uses A/D reading on A4 (PA12) to control duty of TIM3::CH1
// Requires pin A6(PA6) for PWM output (AF1::TIM3::CH1)
//
// Using Simon's Libraries
//////////////////////////////////////

#include "stm32g031xx.h"
#include "cmsis_gcc.h"

#include <stdio.h>
#include "gpio.h"
#include "pll.h"
#include "usart.h"
#include "adc.h"

// definition for number of clocks used for period
// this is based on an unscaled clock at 64MHz
const uint16_t _PWM_PERIOD_COUNT = 64000;

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

    // this code will fall through every 100ms, so the LED toggle and
    // PWM updates will occur 10 times a second

    // toggle the user LED on the NUCLEO board
    // LED blinking, on 100ms, off 100ms (200ms period)
    _GPIO_PinToggle (GPIOC, 6);

    // take an A/D sample on channel 16
    uint16_t samp = _ADC_Sample(16);

    // use the sample value to set the duty (adjusting the pot sets the PWM duty)
    uint16_t duty = ((float)samp/0xFFF) * _PWM_PERIOD_COUNT;         
    TIM3->CCR1 = duty;
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

  // starting up channel 16 (PA10/PA12 (A4)) on NUCLEO-G031K8
  _ADC_Init();

  // setup PA6 as TIM3, Chan1 alternate function
  GPIOA->MODER &= ~GPIO_MODER_MODE6_Msk;
  GPIOA->MODER |= (0x02ul << GPIO_MODER_MODE6_Pos); // alternate function
  
  // AF1 for pin PA6 is TIM3_CH1
  GPIOA->AFR[0] &= ~0b00001111000000000000000000000000;
  GPIOA->AFR[0] |=  0b00000001000000000000000000000000; // AF1 for PA6

  // start TIM3
  RCC->APBENR1 |= RCC_APBENR1_TIM3EN_Msk; // turn on peripheral clock for TIM3

  // RM0444::22.3.9 PWM mode
  TIM3->CCMR1 |= 0b00000000000000000000000001101000; // PWM Mode 1, with preload enabled

  // clock is running at 64MHz, so each tick is around 15.625ns
  // 64MHz / 40000 = 1600 clocks/sec, or a fundamental PWM clock of 1.6kHz
  // a duty of 5000/40000 is 12.5% positive duty with default polarity
  // you would adjust ARR to provide the fundamental frequency for your PWM
  //  if it needs to exceed 64K, then you need to use a prescaler on the timer
  //  if it goes too small then the rate you are looking for will have either little control
  //  over duty, or the rate is beyond what the channel can do at 64MHz.
  
  // if, for example, you wanted 1kHz fundamental, exactly*, you would set ARR to:
  // 1000 clocks/sec = 64MHz / x, x = 64000, this would be nearly the limit for ARR in a 16-bit timer,
  // at a non-prescaled clock rate (the slowest you could go).
  // if you need faster, say 50kHz, that's easy, but now you have lower resolution with the duty
  // 50000 clocks/sec = 64MHz / x, x = 1280
  // this still is pretty good, because 1/1280 means you can still have lots of control over the duty
  // * remember that the clock is based on the HSI16 clock, which is approximate (but 'good enough')
   
  // example with 1kHz fundamental
  TIM3->ARR = _PWM_PERIOD_COUNT; // period
  TIM3->CCR1 = _PWM_PERIOD_COUNT / 2; // duty (50% to start)
  TIM3->CR1 |= TIM_CR1_ARPE_Msk;  // turn on auto-reload preload enable
  TIM3->CCER |= TIM_CCER_CC1E_Msk; // turn on the channel (channel 1 of timer 3)
  TIM3->EGR |= TIM_EGR_UG_Msk; // lock n load values (restart all)
  TIM3->CR1 |= TIM_CR1_CEN_Msk; // enable the counter
}

void TIM2_IRQHandler (void)
{
  // check if read write back is allowed here
  TIM2->SR = ~TIM_SR_UIF_Msk; // yep, these are rc_w0 bits

  // clear the pending interrupt request
  NVIC_ClearPendingIRQ (TIM2_IRQn);

  // count this tick
  ++_GlobalCounter;
}


