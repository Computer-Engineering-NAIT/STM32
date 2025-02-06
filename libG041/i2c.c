// I2C library
//
// Basic I2C functions, intended to be used in device specific libraries
//
// Simon Walker
// Created September 2024, initial build
// 
// Version 1.0 - Initial Build
// Version 1.1 - Fix for Read function - corrected order of Start/Autoend
// Version 1.2 - Added 'unstick' code in init to free the bus from an
//               interrupted transaction (device could still be producing
//               data, this clocks out pulses to free the transaction)
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>

// should operate on other similar variants
//#include "stm32g041xx.h"
#include "stm32g031xx.h"

#include "i2c.h"

int _I2C1_Init (void)
{
  // run the clock for port B, or the pins won't work
  RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
  
  // dev
  RCC->APBENR1 |= RCC_APBENR1_I2C1EN; // turn on peripheral clock for I2C1 (5.4.15)
  
  // turn off, could be second run of init sequence
  I2C1->CR1 &= ~I2C_CR1_PE_Msk;

  // execute unstick clocking for devices that were interrupted while clocking out data
  // this should include clocking out enough pulses to transmit a byte with 
  // the data line idle? to form a nack condition so the device stops transmitting

  // TODO: if both lines are idle, this could probably be skipped...

  // make SCL float high
  GPIOB->BSRR |= (1u << 6);

  // GPIO up the pins to be non-alternate function (GPIO)
  // PB6 should be output
  // PB7 should be input (should idle high)
  GPIOB->MODER &= ~GPIO_MODER_MODE6_Msk;
  GPIOB->MODER |= (0x01ul << GPIO_MODER_MODE6_Pos); //01 - gpo
  GPIOB->MODER &= ~GPIO_MODER_MODE7_Msk;            //00 - input  

  // now clock out 12 slow pulses to end the clocking operation
  for (int i = 0; i < 12; ++i)
  {
    // go low
    GPIOB->BSRR |= (0x10000u << 6);

    for (int k = 0; k < 1000; ++k) // want 10us pulses, measured @64MHz, this is ~ 280us
    {;;;}

    // go high
    GPIOB->BSRR |= (1u << 6);

    for (int k = 0; k < 1000; ++k)
    {;;;}
  }

  // now, back to our usual I2C init...

  // configure pins of i2c1 as alternate function (PB6 -> SCL), (PB7 SDA)
  GPIOB->MODER &= ~GPIO_MODER_MODE6_Msk;
  GPIOB->MODER |= (0x02ul << GPIO_MODER_MODE6_Pos); //10 AF
  GPIOB->MODER &= ~GPIO_MODER_MODE7_Msk;  
  GPIOB->MODER |= (0x02ul << GPIO_MODER_MODE7_Pos); //10 AF
  
  GPIOB->AFR[0] &= ~0b11111111000000000000000000000000;
  GPIOB->AFR[0] |=  0b01100110000000000000000000000000; // AF6 for PB6 and PB7
  
  GPIOB->OTYPER |= 0b0000000011000000; // open drain outputs for PB6 / PB7

  // this does, and I don't know why, makes a difference
  //GPIOB->OSPEEDR |= 0b00000000000000001111000000000000; // PB6/PB7 very high-speed outputs

  GPIOB->PUPDR &=  ~0b00000000000000001111000000000000; // no pull up or down PB6, PB7
  
  // select HSI16 clock as the source for I2C1 (PCLK appears to be default)
  uint32_t ccipr = RCC->CCIPR;
  ccipr &= ~0b00000000000000000011000000000000;
  ccipr |=  0b00000000000000000010000000000000; // HIS16 as I2C1 source
  RCC->CCIPR = ccipr;

  // configure analog filter (or digital filter)
  I2C1->CR1 &= ~I2C_CR1_ANFOFF_Msk; // analog noise filter enabled

  // configure timing characteristics
  I2C1->TIMINGR = 0x00301C79; // 100kHz from Cube, but clock waveform looks really weird
  
  // configure stretch settings
  I2C1->CR1 &= ~I2C_CR1_NOSTRETCH_Msk; // clock stretching enabled

  // turn on module
  I2C1->CR1 |= I2C_CR1_PE;

  return 0;
}

int _I2C1_IsBusy (void)
{
  return (I2C1->ISR & I2C_ISR_BUSY);
}

int _I2C1_StartRead (unsigned char address, unsigned char bytecount, unsigned char * pData, _I2C_AutoEndOption endoption)
{
  // can't wait for bus to be busy in the case of a repeated start condition

  // 7-bit address of target is written to SADD 7:1 (0, 8, 9 ignored)
  I2C1->CR2 &= ~I2C_CR2_SADD_Msk;
  I2C1->CR2 |=  (unsigned int)address << 1; // target address, shifted up one position

  // setup bytes to transfer, excludes addressing byte
  I2C1->CR2 &= ~I2C_CR2_NBYTES_Msk;
  I2C1->CR2 |= bytecount << I2C_CR2_NBYTES_Pos;

  // set direction of transfer
  I2C1->CR2 |= I2C_CR2_RD_WRN; // 1 == want to read

  // issue the start condition
  I2C1->CR2 |= I2C_CR2_START; // start the communication...

  // autoend moved to after start condition issued,
  //  as in a repeated start, this would terminate the transaction
  //  if hardware end was desired in the 2nd half of the transaction
  // so, assume for a restart, the transaction was started as software end
  //  the the TC flag will be set, and the clock is being stretched
  // if we flip to hardware, we want the NBYTES value to be non-zero

  // set the AUTOEND option
  I2C1->CR2 &= ~I2C_CR2_AUTOEND_Msk;
  // this kills an ongoing transaction at this point***
  I2C1->CR2 |= (((unsigned int)endoption) << I2C_CR2_AUTOEND_Pos);

  // no need to wait for address transmission, unless there is a problem  
  //  as the next event if all goes well is a byte from the target device

  int readcounter = 0;

  while (readcounter < bytecount)
  {
    // wait for byte
    while (!(I2C1->ISR & I2C_ISR_RXNE))
    {
    }
  
    // read it
    pData[readcounter++] = I2C1->RXDR;
  }

  if (endoption == _I2C_AutoEnd_Software)
  {
    // don't sent stop condition, but wait for TC flag
    while (!(I2C1->ISR & I2C_ISR_TC))
    {
    }  

    // we're done here, leave transaction active and return
    return 0;
  }

  return 0;
}

int _I2C1_StartWrite (unsigned char address, unsigned char bytecount, unsigned char * pData, _I2C_AutoEndOption endoption)
{  
  // upon entry, if required, wait for bus to be busy or not
  // this assumes no repreated start in the write case
  while (_I2C1_IsBusy())
  {;;;}

  // 7-bit address of target is written to SADD 7:1 (0, 8, 9 ignored)
  I2C1->CR2 &= ~I2C_CR2_SADD_Msk;
  I2C1->CR2 |=  (unsigned int)address << 1; // target address, shifted up one position

  // setup bytes to transfer, excludes addressing byte
  I2C1->CR2 &= ~I2C_CR2_NBYTES_Msk;
  I2C1->CR2 |= bytecount << I2C_CR2_NBYTES_Pos;

  // set direction of transfer
  I2C1->CR2 &= ~I2C_CR2_RD_WRN_Msk;

  // set the AUTOEND option
  I2C1->CR2 &= ~I2C_CR2_AUTOEND_Msk;
  I2C1->CR2 |= (((unsigned int)endoption) << I2C_CR2_AUTOEND_Pos);

  // issue the start condition
  I2C1->CR2 |= I2C_CR2_START; // start the communication...
      
  // wait for address to clear
  while (!(I2C1->ISR & I2C_ISR_TXIS))
  {
      if (I2C1->ISR & I2C_ISR_NACKF)
      {
        // welp, no ack here, so device did not respond, need to clear
        //  nack flag
        return -2;
      }
  }

  // all good so far, so write out the data, with the end style selected
  for (int iByteCount = bytecount; iByteCount > 1; iByteCount--)
  {
    I2C1->TXDR = pData[bytecount - iByteCount];

    // wait for byte to clear
    while (!(I2C1->ISR & I2C_ISR_TXIS))
    { ;;; }
  }

  // final (or only) byte is controlled by TC flag if software end
  I2C1->TXDR = pData [bytecount - 1];

  if (endoption == _I2C_AutoEnd_Software)
  {
    // don't send stop condition, but wait for TC flag
    // this is more for followup transactions, as a restart can't
    //  use busy to tell if a transaction is ongoing
    // we could do TC polling in the read function, if we expect
    //  that we are under a restart condition
    // it's slower here, cause we are waiting for the transmission
    //  to complete, but it makes the read easier if we enter
    //  always knowning the transaction is ready to continue
    // maybe a future update
    
    while (!(I2C1->ISR & I2C_ISR_TC))
    { ;;; }  

    // we're done here, leave transaction active and return
    return 0;
  }

  // so, we're under hardware end (STOP will be automatically issued)  

  return 0;
}

// requires array of 128 bytes to hold address results
void _I2C1_BusScan (unsigned char * pAddressTable)
{
  // attempt a bus scan, then port to function when operational
  // should be able to announce address, if no ACK module should auto stop
  //  if ack, dump address and issue stop, then try next
  I2C1->CR1 = 0;
  for (int i = 0; i < 10; ++i) // at 1000, lose 0x10, otherwise lose 0x53 
  {
  }
  I2C1->CR1 |= I2C_CR1_PE;

  for (unsigned char addr = 0x08; addr < 0x7E; addr++)
  {
    // setup address
    I2C1->CR2 = 0;
    I2C1->CR2 |=  addr << 1; // target address, shifted up one position (7-bit mode)    

    // we are sending zero bytes, so byte flagging for sent bytes won't be active
    //  we are just sending the address of the device and looking for no ACK
    //  this also takes autoend off the table, as we are not sending data        

    // issue start condition
    I2C1->CR2 |= I2C_CR2_START; // start the communication...

    // can't depend on TXIS, as there is no transmission of data here
    //  there will either be a NACK/STOPF or a TC

    // wait for transmission to complete (device present case)
    //  or STOPF/NACK (no device case)
    while (! ((I2C1->ISR & I2C_ISR_NACKF_Msk) || (I2C1->ISR & I2C_ISR_STOPF_Msk) || (I2C1->ISR & I2C_ISR_TC_Msk)))    
    {;;;}
    
    if ((I2C1->ISR & I2C_ISR_NACKF) || (I2C1->ISR & I2C_ISR_STOPF_Msk))
    {
      // wait a bit for stop flag to maybe be expressed (can take a bit)
      for (int i = 0; i < 100000; ++i)
      {
        // bail early if NACK and STOP present
        if ( (I2C1->ISR & (I2C_ISR_NACKF_Msk | I2C_ISR_STOPF_Msk)) == (I2C_ISR_NACKF_Msk | I2C_ISR_STOPF_Msk))
          break;
      }      

      // could have NACK (expresses immediately on 9th bit)
      if (I2C1->ISR & I2C_ISR_NACKF)
      {
        I2C1->ICR = I2C_ICR_NACKCF_Msk; // 32.7.8 -> clear NACK flag

        // require delay here, or things don't work...
        for (int i = 0; i < 1000; ++i)
        {;;;}
      }

      // could also have stop (hardware detected), takes longer to present
      if (I2C1->ISR & I2C_ISR_STOPF)
      {
        I2C1->ICR = I2C_ICR_STOPCF_Msk; // clear STOPF flag

        // may require delay here, or things don't work...
        for (int i = 0; i < 1000; ++i)
        {;;;}        
      }
    }
    else // should be TC
    {
      // no NACK, so we good, record address        
      pAddressTable[addr] = addr;
      
      // issue stop to release the bus
      I2C1->CR2 |= I2C_CR2_STOP;
    }    
  }
}