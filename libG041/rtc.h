// RTC Library
//
// Functions to operate the RTC
// Assumes LSE present
//
// Simon Walker
// Created September 2024, initial build
//
// Version 1.0
// Version 1.1 -> Changed to use time string instead of structure to set
//  RTC time on cold boot
///////////////////////////////////////////////////////////////////////

typedef enum
{
  _RTC_INIT_WARM_RESET,
  _RTC_INIT_COLD_RESET
} _RTC_INIT_RESULT;

typedef enum
{
  _RTC_NO_FORCE_TIME_RESET,
  _RTC_FORCE_TIME_RESET
} _RTC_TIME_RESET_OPTION;

typedef struct
{
  unsigned int _RTC_Time_YT  : 4; // Year Tens (BCD)
  unsigned int _RTC_Time_YU  : 4; // Year Units (BCD)
  unsigned int _RTC_Time_WDU : 3; // Week Day 001:Monday ... 111:Sunday
  unsigned int _RTC_Time_MT  : 1; // Month Tens (BCD)
  unsigned int _RTC_Time_MU  : 4; // Month Units (BCD)
  unsigned int _RTC_Time_DT  : 2; // Day Tens (BCD)
  unsigned int _RTC_Time_DU  : 4; // Day Units (BCD)
  unsigned int _RTC_Time_PM  : 1; // AM/PM 0 -> AM/24HR
  unsigned int _RTC_Time_HT  : 2; // Hour Tens (BCD)
  unsigned int _RTC_Time_HU  : 4; // Hour Units (BCD)
  unsigned int _RTC_Time_MNT : 3; // Minute Tens (BCD)
  unsigned int _RTC_Time_MNU : 4; // Minute Units (BCD)
  unsigned int _RTC_Time_ST  : 3; // Seconds Tens (BCD)
  unsigned int _RTC_Time_SU  : 4; // Seconds Units (BCD)
  unsigned int _RTC_Time_SSR :16; // sub seconds (RAW) 
} _RTC_STime;

// cold boot will use string in following format to init the RTC
//  in warm boot, this string will not be used, but *must* be present
// Format: Fri Oct 11 08:03:00 2024
_RTC_INIT_RESULT _RTC_Init (char * TimeStringInit, _RTC_TIME_RESET_OPTION timeresetoption);

// get the current time as a STime structure
_RTC_STime _RTC_Read (void);

long _RTC_STimeHash (_RTC_STime t);
long _RTC_STimeHashDiff (_RTC_STime A, _RTC_STime B);

// hidden helper
//void _RTC_SetTime (_RTC_STime settime);