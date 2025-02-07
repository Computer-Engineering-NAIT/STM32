// ADC Library
//
// Functions perform very basic A/D reading (in development)
//
// Simon Walker
// Created October 2024, initial build, demonstration version only
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

// should work on other variants
//#include "stm32g041xx.h"
#include "stm32g031xx.h"

#include "adc.h"

// this init only turns on channel 16 (A10/A12 on the STM32G041F8P6 chip)

// expect the following code to provoke an actual reading:
// show A/D value from channel 16
//{
//  // start a conversion
//  ADC1->CR |= ADC_CR_ADSTART_Msk;
//  while (!(ADC1->ISR & ADC_ISR_EOC_Msk))
//  { ;;; }
//  ADC1->DR <<< contains sample
//}

// pin configuration (input should be default)
// using pin A10/A12 as channel 16 A/D
//_GPIO_SetPinMode (GPIOA, 10, _GPIO_PinMode_Analog); // ADC_IN16 G041

// only testing, brings up channel 16, manual sampling
void _ADC_Init (void)
{
  //////////////////////////////////////////////////////////////////////////////////////////
  // A/D testing -> A10/A12 as A/D on the G041
  
  // 5.2.11
  //The ADC clock is derived from the system clock, or from the PLLPCLK output. It can reach
  //122 MHz and can be divided by the following prescalers values:
  //1,2,4,6,8,10,12,16,32,64,128 or 256 by configuring the ADC1_CCR register. It is
  //asynchronous to the AHB clock. Alternatively, the ADC clock can be derived from the AHB
  //clock of the ADC bus interface, divided by a programmable factor (1, 2 or 4). This
  //programmable factor is configured using the CKMODE bitfields in the ADC1_CCR.
  //If the programmed factor is 1, the AHB prescaler must be set to 1.

  // turn on the A/D clock
  RCC->APBENR2 |= RCC_APBENR2_ADCEN_Msk;

  // turn off the A/D (if this is reentrant)
  ADC1->CR &= ~ADC_CR_ADEN_Msk;

  // 15.3.2 -> ADC Voltage Regulator
  ADC1->CR |= ADC_CR_ADVREGEN_Msk; // requires setup delay, lol, 20uS
  
  { for (int i = 0; i < 10000; ) { i++; }} // delay, hopefully defeat opt

  // begin A/D calibration
  ADC1->CR |= ADC_CR_ADCAL_Msk;

  // wait for calibration to complete
  while (ADC1->CR & ADC_CR_ADCAL_Msk)
  { ;;; }

  //The devices embed a voltage reference buffer which can be used as voltage reference for
  //ADC, DAC and also as voltage reference for external components through the VREF+ pin.
  // NOTE: When the VREF+ pin is double-bonded with VDDA pin in a package, the voltage reference
  //buffer is not available and must be kept disabled (refer to datasheet for packages pinout
  //description).
  // so... not available in the packages with reduced pins
  // I'd really want the interal vref when monitoring an external
  //  battery voltage, so in the case of a VBAT pin, but that does
  //  not exist on the low pin count versions either, so, self solving I guess
  //VREFBUF->CSR = 0b011; // use internal reference at 2.048V
  //while (!(VREFBUF->CSR & VREFBUF_CSR_VRR_Msk))
  //{ ;;; } // wait for A/D reference buffer to be ready

  // turn on A/D 15.3.4
  ADC1->ISR = ADC_ISR_ADRDY_Msk; // clear pending flag
  ADC1->CR |= ADC_CR_ADEN_Msk;
  while (!(ADC1->ISR & ADC_ISR_ADRDY_Msk))
  { ;;; }

  // enable channel 16
  ADC1->CHSELR = 0x00010000;
}


uint16_t _ADC_Sample (int chan)
{
  if (chan == 16)
  {
    // start a conversion
    ADC1->CR |= ADC_CR_ADSTART_Msk;
    while (!(ADC1->ISR & ADC_ISR_EOC_Msk))
    { ;;; }

    // read channel data (configured only for this channel, currently)
    return ADC1->DR;
  }

  // not a supported channel... yet...
  return 0;
}
