// RTC Library
//
// Functions to operate the RTC
// Assumes LSE present
//
// Simon Walker
// Created September 2024, initial build
//
// Version 1.0 - Initial
// Version 1.1 - OCTOBER 11 2024 - Altered init to use a time string 
//  same format as __TIMESTAMP__, to make cold boots easier
// 
///////////////////////////////////////////////////////////////////////

//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>

// should work on other variants
#include "stm32g041xx.h"

#include "rtc.h"

// local helper prototypes
void _RTC_SetTime (_RTC_STime settime);

// local helper methods
// Format: "Fri Oct 11 08:03:00 2024"
_RTC_STime _RTC_TimeStringToSTime (char * tbuff)
{
  _RTC_STime curtime;

  // year
  curtime._RTC_Time_YT = tbuff[22] - '0';
  curtime._RTC_Time_YU = tbuff[23] - '0';

  // day of week 1Mo, 2Tu, 3We, 4Th, 5Fr, 6Sa, 7Su
  switch (tbuff[0])
  {
    case 'M':
      curtime._RTC_Time_WDU = 1;
      break;
    case 'T':
      curtime._RTC_Time_WDU = tbuff[1] == 'u' ? 2 : 4;
      break;
    case 'W':
      curtime._RTC_Time_WDU = 3;
      break;
    case 'F':
      curtime._RTC_Time_WDU = 5;
      break;
    case 'S':
      curtime._RTC_Time_WDU = tbuff[1] == 'a' ? 6 : 7;
      break;
  }    
  
  // month
  curtime._RTC_Time_MT = 0;
  if (strncmp ("Jan", tbuff + 4, 3) == 0)
    curtime._RTC_Time_MU = 1;
  if (strncmp ("Feb", tbuff + 4, 3) == 0)
    curtime._RTC_Time_MU = 2;
  if (strncmp ("Mar", tbuff + 4, 3) == 0)
    curtime._RTC_Time_MU = 3;
  if (strncmp ("Apr", tbuff + 4, 3) == 0)
    curtime._RTC_Time_MU = 4;
  if (strncmp ("May", tbuff + 4, 3) == 0)
    curtime._RTC_Time_MU = 5;
  if (strncmp ("Jun", tbuff + 4, 3) == 0)
    curtime._RTC_Time_MU = 6;
  if (strncmp ("Jul", tbuff + 4, 3) == 0)
    curtime._RTC_Time_MU = 7;
  if (strncmp ("Aug", tbuff + 4, 3) == 0)
    curtime._RTC_Time_MU = 8;
  if (strncmp ("Sep", tbuff + 4, 3) == 0)
    curtime._RTC_Time_MU = 9;
  if (strncmp ("Oct", tbuff + 4, 3) == 0)
  {
    curtime._RTC_Time_MT = 1;
    curtime._RTC_Time_MU = 0;
  }
  if (strncmp ("Nov", tbuff + 4, 3) == 0)
  {
    curtime._RTC_Time_MT = 1;
    curtime._RTC_Time_MU = 1;
  }
  if (strncmp ("Dec", tbuff + 4, 3) == 0)
  {
    curtime._RTC_Time_MT = 1;
    curtime._RTC_Time_MU = 2;
  }

  // Format: Fri Oct 11 08:03:00 2024
  //         012345678901234567890123

  // day
  curtime._RTC_Time_DT = tbuff[8] - '0';
  curtime._RTC_Time_DU = tbuff[9] - '0';

  curtime._RTC_Time_PM = 0;           //24 hour

  // hour
  curtime._RTC_Time_HT = tbuff[11] - '0';
  curtime._RTC_Time_HU = tbuff[12] - '0';

  // minute
  curtime._RTC_Time_MNT = tbuff[14] - '0';
  curtime._RTC_Time_MNU = tbuff[15] - '0';

  // second
  curtime._RTC_Time_ST = tbuff[17] - '0';
  curtime._RTC_Time_SU = tbuff[18] - '0';

  curtime._RTC_Time_SSR = 0; // not currently used

  return curtime;
}
///////////////////////

// cold boot will use string in following format to init the RTC
//  in warm boot, this string will not be used, but *must* be present
// Format: Fri Oct 11 08:03:00 2024
_RTC_INIT_RESULT _RTC_Init (char * TimeStringInit, _RTC_TIME_RESET_OPTION timeresetoption)
{
  // convert the time string into a _RTC_STime instance
  _RTC_STime settime = _RTC_TimeStringToSTime (TimeStringInit);
 
 // 4.1.2 -> RTC domain access
  /*
RTC domain access
After a system reset, the RTC domain (RTC registers and backup registers) is protected
against possible unwanted write accesses. To enable access to the RTC domain, proceed
as follows:
1. Enable the power interface clock by setting the PWREN bits of the APB peripheral
clock enable register 1 (RCC_APBENR1).
2. Set the DBP bit of the Power control register 1 (PWR_CR1) to enable access to the
RTC domain.
3. Select the RTC clock source in the RTC domain control register (RCC_BDCR).
4. Enable the RTC clock by setting the RTCEN bit in the RTC domain control register
(RCC_BDCR).  
  */  

  // it's probably important to note that the RTC clock can't be changed
  //  unless the RTC has gone through a backup domain reset
  //  this will happen if VBAT and main power is lost
  //  this will always happen with a device that does not have a battery backup
  //  BUT, it will not happen on a warm reset, so if there is no battery backup
  //  it would make sense to perform a (possibly redundant) backup domain
  //  reset in the case where there is not battery backup, and that will ensure
  //  that warm resets always reset everything (or, optionally, detemine if
  //  the reset is required by looking at the state of the backup domain)

  // this is not listed in the checklist, could it be important?
  // yes, yes it is very important or we can't talk APB to RTC
  RCC->APBENR1 |= RCC_APBENR1_RTCAPBEN_Msk;

  // steps 1 and 2, steps 3 and 4 below
  RCC->APBENR1 |= RCC_APBENR1_PWREN_Msk;
  PWR->CR1 |= PWR_CR1_DBP_Msk; // access to RTC and backup domain registers

  // check and see if the LSE is already running
  //  if the calendar registers are already set
  //  if so, this is warm reset, and there is no need to do an RTC init
  //  unless the option is set to do so
  if (RCC->BDCR & RCC_BDCR_LSERDY)
  {
    if (RCC->BDCR & RCC_BDCR_RTCEN_Msk)
    {
      // LSE is running, and the RTC is already enabled
      // a warm reset?     

      // set clock if open set // this does not appear to work?
      if (timeresetoption == _RTC_FORCE_TIME_RESET)
        _RTC_SetTime (settime);
      return _RTC_INIT_WARM_RESET;
    }
  }

  // this should be a RTC domain reset situation (cold reset)
  // turn on LSE, wait to become stable  
  RCC->BDCR |= RCC_BDCR_LSEON;
  while (!(RCC->BDCR & RCC_BDCR_LSERDY))
  {;;;}

  // select LSE as the RTC clock source (step 3 from above)
  uint32_t bdcr = RCC->BDCR;
  bdcr &= ~RCC_BDCR_RTCSEL_Msk;
  bdcr |= 0b00000000000000000000000100000000; // 01 for RTCSEL (LSE)
  RCC->BDCR = bdcr;
  
  // step 4 from above -> enable RTC clock
  RCC->BDCR |= RCC_BDCR_RTCEN_Msk;
  while (!(RCC->BDCR & RCC_BDCR_RTCEN_Msk))
  {;;;}

/*
After a system reset, the application can read the INITS flag in the RTC_ICSR register to
check if the calendar has been initialized or not. If this flag equals 0, the calendar has not
been initialized since the year field is set at its Backup domain reset default value (0x00).
To read the calendar after initialization, the software must first check that the RSF flag is set
in the RTC_ICSR register.
*/

  // program the clock, it's a cold boot
  _RTC_SetTime (settime);

  return _RTC_INIT_COLD_RESET;
}

_RTC_STime _RTC_Read (void)
{
  // wait for RSF (requires RTC to be running and initialized, so don't call this function if that's not done)
  // add timeout condition here
  while (!(RTC->ICSR & RTC_ICSR_RSF_Msk))
  {;;;}

  // read ssr, tr and dr registers then clear RSF flag
  uint32_t ssr = RTC->SSR;
  uint32_t tr = RTC->TR;
  uint32_t dr = RTC->DR;

  // pack register reads into structure
  _RTC_STime res;
  
  // TR components
  res._RTC_Time_PM = (tr & RTC_TR_PM_Msk) >> RTC_TR_PM_Pos;
  res._RTC_Time_HT = (tr & RTC_TR_HT_Msk) >> RTC_TR_HT_Pos;
  res._RTC_Time_HU = (tr & RTC_TR_HU_Msk) >> RTC_TR_HU_Pos;
  res._RTC_Time_MNT = (tr & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos;
  res._RTC_Time_MNU = (tr & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos;
  res._RTC_Time_ST = (tr & RTC_TR_ST_Msk) >> RTC_TR_ST_Pos;
  res._RTC_Time_SU = (tr & RTC_TR_SU_Msk) >> RTC_TR_SU_Pos;

  // dr components
  res._RTC_Time_YT = (dr & RTC_DR_YT_Msk) >> RTC_DR_YT_Pos;
  res._RTC_Time_YU = (dr & RTC_DR_YU_Msk) >> RTC_DR_YU_Pos;
  res._RTC_Time_WDU = (dr & RTC_DR_WDU_Msk) >> RTC_DR_WDU_Pos;
  res._RTC_Time_MT = (dr & RTC_DR_MT_Msk) >> RTC_DR_MT_Pos;
  res._RTC_Time_MU = (dr & RTC_DR_MU_Msk) >> RTC_DR_MU_Pos;
  res._RTC_Time_DT = (dr & RTC_DR_DT_Msk) >> RTC_DR_DT_Pos;
  res._RTC_Time_DU = (dr & RTC_DR_DU_Msk) >> RTC_DR_DU_Pos;

  // subseconds component
  res._RTC_Time_SSR = (ssr & RTC_SSR_SS_Msk);

  return res;
}

long _RTC_STimeHash (_RTC_STime t)
{
  return 
    t._RTC_Time_YT * 10 * 365 * 24 * 60 * 60 +
    t._RTC_Time_YU * 365 * 24 * 60 * 60 +
    t._RTC_Time_MT * 10 * 31 * 24 * 60 * 60 +
    t._RTC_Time_MU * 31 * 24 * 60 * 60 +
    t._RTC_Time_DT * 10 * 24 * 60 * 60 +
    t._RTC_Time_DU * 24 * 60 * 60 +
    t._RTC_Time_HT * 10 * 60 * 60 +
    t._RTC_Time_HU * 60 * 60 +
    t._RTC_Time_MNT * 10 * 60 +
    t._RTC_Time_MNU * 60 +
    t._RTC_Time_ST * 10 +
    t._RTC_Time_SU;
}

long _RTC_STimeHashDiff (_RTC_STime A, _RTC_STime B)
{
  return _RTC_STimeHash (A) - _RTC_STimeHash(B);
}

void _RTC_SetTime (_RTC_STime settime)
{
/*
1. Set INIT bit to 1 in the RTC_ICSR register to enter initialization mode. In this mode, the
calendar counter is stopped and its value can be updated.
2. Poll INITF bit of in the RTC_ICSR register. The initialization phase mode is entered
when INITF is set to 1. It takes around 2 RTCCLK clock cycles (due to clock
synchronization).
3. To generate a 1 Hz clock for the calendar counter, program both the prescaler factors
in RTC_PRER register.
4. Load the initial time and date values in the shadow registers (RTC_TR and RTC_DR),
and configure the time format (12 or 24 hours) through the FMT bit in the RTC_CR
register.
5. Exit the initialization mode by clearing the INIT bit. The actual calendar counter value is
then automatically loaded and the counting restarts after 4 RTCCLK clock cycles.
*/

  // unlock write protection on the RTC registers
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  // start init mode
  RTC->ICSR |= RTC_ICSR_INIT_Msk;

  // wait for device to report init ready
  while (!(RTC->ICSR & RTC_ICSR_INITF_Msk))
  {;;;}

  // program 1Hz clock prescaler values (actually the defaults)
  RTC->PRER = 0x007F00FF; // 128 for PREDIV_A, and 256 for PREDIV_B (32768 / 32768 = 1Hz)

  RTC->TR =
    (settime._RTC_Time_SU) |
    (settime._RTC_Time_ST << RTC_TR_ST_Pos) |
    (settime._RTC_Time_MNU << RTC_TR_MNU_Pos) |
    (settime._RTC_Time_MNT << RTC_TR_MNT_Pos) |
    (settime._RTC_Time_HU << RTC_TR_HU_Pos) |
    (settime._RTC_Time_HT << RTC_TR_HT_Pos) |
    (settime._RTC_Time_PM << RTC_TR_PM_Pos);
  
  RTC->DR = 
    (settime._RTC_Time_DU) |
    (settime._RTC_Time_DT << RTC_DR_DT_Pos) |
    (settime._RTC_Time_MU << RTC_DR_MU_Pos) |
    (settime._RTC_Time_MT << RTC_DR_MT_Pos) |
    (settime._RTC_Time_WDU << RTC_DR_WDU_Pos) |
    (settime._RTC_Time_YU << RTC_DR_YU_Pos) |
    (settime._RTC_Time_YT << RTC_DR_YT_Pos);
    
  RTC->CR &= ~RTC_CR_FMT_Msk;                   // default, but ensure 24-hour time format

  // exit init mode
  RTC->ICSR &= ~RTC_ICSR_INIT_Msk; 
}