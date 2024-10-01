// Utility Functions Library
//
// Functions that have no specific home
//
// Simon Walker
// Created September 2024, initial build, ported from ATmega328 version
//
// Version 1.0
// 
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
//#include <stdlib.h>

#include "util.h"

// requires 16 bytes of space, no null appended
void _Util_ShowBin (unsigned int val, char * buff)
{
  unsigned int iMask = 0x8000;
  for (iMask = 0x8000; iMask; iMask >>= 1)
  {
    *buff = (iMask & val) ? '1' : '0';   
    ++buff;
  }
}

// build a formatted time string from an SClockVals structure
// buff needs to have enough room to hold DDDDD HH:MM:SS.H0 (at least 17 characters)
void _Util_BuildTimeString(char *buff, _Util_SClockVals val)
{
  (void)sprintf(buff, "%0.5d %2.2d:%2.2d:%2.2d.%1.1d", val.days, val.hours, val.minutes, val.seconds, val.thow / 100);
}

long _Util_TimeDiff (_Util_SClockVals A, _Util_SClockVals B)
{
  return _Util_TimeToTicks (A) - _Util_TimeToTicks (B);
}

unsigned long _Util_TimeToTicks (_Util_SClockVals t)
{
  long total = 0;
  total += t.thow;
  total += t.seconds * 1000;
  total += t.minutes * 60 * 1000;
  total += t.hours * 60 * 60 * 1000;
  total += t.days * 24 * 60 * 60 * 1000;
  return total;
}

_Util_SClockVals _Util_ConvertTime(long msTicks)
{
  _Util_SClockVals temp = {0, 0, 0, 0, 0};
  temp.thow = (int)(msTicks % 1000);
  temp.seconds = (char)((msTicks / 1000l) % 60);
  temp.minutes = (char)((msTicks / (1000l * 60)) % 60);
  temp.hours = (char)((msTicks / (1000l * 60 * 60)) % 24);
  temp.days = (unsigned int)((msTicks / (1000l * 60 * 60 * 24)));
  return temp;
}

char _Util_CountBits(unsigned char val)
{
  char bits = 0;
  unsigned char mask = 0x80;
  while (mask)
  {
    if (mask & val)
      ++bits;
    mask >>= 1;
  }

  return bits;
}

unsigned char _Util_ToLower(unsigned char val)
{
  if (val >= 'A' && val <= 'Z')
    return val + 32;
  return val;
}

