// USART Library
//
// Functions perform basic async subset of USART operations
//
// Simon Walker
// Created September 2024, initial build, ported from STM32L476 version
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

// should work on other variants
//#include "stm32g041xx.h"
#include "stm32g031xx.h"

#include "usart.h"

void _USART2_Init (long bus_rate, long baud_rate)
{  
  // turn on GPIO clock for Port A, necessary to set alternate function
  RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

  // apb peripheral clock enable for USART2 [5.4.15]
  RCC->APBENR1 |= RCC_APBENR1_USART2EN;

  // configure pins of usart as alternate function
  GPIOA->MODER &= ~GPIO_MODER_MODE2_Msk;
  GPIOA->MODER |= (0x02ul << GPIO_MODER_MODE2_Pos);
  GPIOA->MODER &= ~GPIO_MODER_MODE3_Msk;  
  GPIOA->MODER |= (0x02ul << GPIO_MODER_MODE3_Pos);
  
  // AF1 on G041
  GPIOA->AFR[0] &= ~0b00000000000000001111111100000000;
  GPIOA->AFR[0] |=  0b00000000000000000001000100000000; // AF1 for PA2 and PA3 means PA2::USART2_TX, PA3::USART2_RX

  // turn on the USART, with transmitter and receiver enabled
  USART2->CR1 |= USART_CR1_TE_Msk | USART_CR1_RE_Msk;

  USART2->BRR = ((bus_rate * 10) / baud_rate) / 10;

  // last! turn on the usart (most flags can't be set if it's turned on)
  USART2->CR1 |= USART_CR1_UE_Msk;
}

void _USART2_TxByte (unsigned char data)
{
    // transmit a character over USART2
    while (!(USART2->ISR & USART_ISR_TXE_TXFNF))
    {
    }
    
    USART2->TDR = data;   
}

int _USART2_RxByte (unsigned char * pData)
{
  if (USART2->ISR & USART_ISR_RXNE_RXFNE)
  {
    *pData = USART2->RDR;
    return 1;
  }

  return 0;
}

void _USART2_TxString (char * pStr)
{
  while (*pStr)
  {
    _USART2_TxByte (*pStr);
    pStr++;
  }
}

// use escape sequences to place the cursor at the specified position
// this is the \x1B[Y;XH form with formatted argument replacement (sprintf)
void _USART2_GotoXY (int iCol, int iRow)
{
  char buff [20] = {0};
  (void)sprintf (buff, "\x1b[%d;%dH", iRow, iCol);
  _USART2_TxString (buff);
}

// use sci0_GotoXY and sci0_txStr to place the string
void _USART2_TxStringXY (int iCol, int iRow, char * straddr)
{
  // got the X/Y
  _USART2_GotoXY (iCol, iRow);

  // send out string at the new cursor position
  _USART2_TxString (straddr);
}

// use an escape sequence to clear the terminal
void _USART2_ClearScreen (void)
{
  _USART2_TxString ("\x1b[2J");
}