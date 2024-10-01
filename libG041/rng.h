// RNG library
//
// Functions to control the clocking RNG module and generate 
//  random numbers
//
// Simon Walker
// Created September 2024, initial build
////////////////////////////////////////////////////////////

// initialized the RNG peripheral
void _RNG_Init16 (void); // if running in default 16MHz HSI clock mode
void _RNG_Init64 (void); // if running in 64MHz PLLR clock mode

// get a random number into target, zero return if OK
int _RNG_GetRND (unsigned int * pData);

// get a random double 0 to <1
double _RNG_GetRNDD (void);

// get a value in the inclusive lower, exclusive upper range
int _RNG_GetRndRange (int iLow, int iHigh);