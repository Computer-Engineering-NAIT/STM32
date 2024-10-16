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
#include "stm32g041xx.h"

void _ADC_Init (void);
uint16_t _ADC_Sample (int chan);