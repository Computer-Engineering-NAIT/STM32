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

typedef struct
{
  int thow;          // 0-999
  char seconds;      // 0-59
  char minutes;      // 0-59
  char hours;        // 0-23
  unsigned int days; // 0+
} _Util_SClockVals;

// requires 16 bytes of space, no null appended
void _Util_ShowBin (unsigned int val, char * buff);

char _Util_CountBits(unsigned char val);

unsigned char _Util_ToLower(unsigned char val);

_Util_SClockVals _Util_ConvertTime(long msTicks);

// build a formatted time string from an SClockVals structure
// buff needs to have enough room to hold DDDDD HH:MM:SS.H0 (at least 17 characters)
void _Util_BuildTimeString(char *buff, _Util_SClockVals val);

long _Util_TimeDiff (_Util_SClockVals A, _Util_SClockVals B);

unsigned long _Util_TimeToTicks (_Util_SClockVals t);
