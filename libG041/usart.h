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

void _USART2_Init (long bus_rate, long baud_rate);
void _USART2_TxByte (unsigned char data);
int _USART2_RxByte (unsigned char * pData);
void _USART2_GotoXY (int iCol, int iRow);
void _USART2_TxString (char * pStr);
void _USART2_TxStringXY (int iCol, int iRow, char * straddr);
void _USART2_ClearScreen (void);
