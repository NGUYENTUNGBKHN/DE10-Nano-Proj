/* Bare-metal fireworks example for Beagle Board */

/* Basic i2c driver to drive TPS65950 */

/* Copyright (c) 2010 Arm Limited (or its affiliates). All rights reserved. */
/* Use, modification and redistribution of this file is subject to your possession of a     */
/* valid End User License Agreement for the Arm Product of which these examples are part of */
/* and your compliance with all applicable terms and conditions of such licence agreement.  */

#include <stdint.h>
#include "i2c.h"

/* Timeout number of cycles */
#define TIMEOUT_CYCLES 50


void i2c_write(uint16_t sa, uint16_t addr, uint16_t value)
{
    uint16_t i = 0;
    uint16_t stat_value = 0x00000000;
    uint16_t state = 0;
    uint16_t n_bytes = 2;   /* hard coded number of bytes to be written */
    uint16_t timeout = 0;

    uint16_t data[2];

    data[0] = addr;
    data[1] = value;


    /* See algorithm in documentation: HS i2c Master transmitter mode, polling, F/S */
    while (*I2C_STAT & I2C_STAT_BB)
    {
        if (timeout++ > TIMEOUT_CYCLES)
        {
           timeout = 0;
           *I2C_STAT = 0;
           break;  // Wait for bus free
        }
    }

    while (i < n_bytes)
    {
       /* Read the i2c status to check the flags */
       stat_value = *I2C_STAT;

       if (timeout++ > TIMEOUT_CYCLES)
           break;

       switch (state)
       {
          case INIT:
              *I2C_SA        = sa;
              *I2C_CNT       = n_bytes;
              *I2C_CON       = 0x8603;  /* hard coded: no stop mode i2c transmission */
              state = TF_NACK;
              break;

          case TF_NACK:
              //No ACK returned
              if (stat_value & I2C_STAT_NACK)
              {
                 //Clear NACK BIT
                 //Clear the indicator
                 *I2C_STAT = *I2C_STAT | I2C_STAT_NACK;
                 //Reprogram registers
                 state = INIT;
              }
              else
                 state = TF_AL;
              break;

          case TF_AL:
             if (stat_value & I2C_STAT_AL)
             {
                 //Arbitration lost
                 //Clear the indicator
                 *I2C_STAT = *I2C_STAT | I2C_STAT_AL;
                 //Reprogram registers
                 state = INIT;
              }
              else
                 state = TF_ARDY;
              break;

          case TF_ARDY:
             if (stat_value & I2C_STAT_ARDY)
             {
                //Can't update the registers
                //Clear the indicator
                *I2C_STAT = *I2C_STAT | I2C_STAT_ARDY;
                //Try again
                state = TF_NACK;
              }
              else
                state = TF_XDR;
              break;

          case TF_XDR:
             if (stat_value & I2C_STAT_XDR)
             {
                //There is data in the bus
                //Clear the indicator, wait data be sent
                *I2C_STAT = *I2C_STAT | I2C_STAT_XDR;
                //Try again
                state = TF_NACK;
             }
             else
                 state = TF_XRDY;
             break;

          case TF_XRDY:
             if (stat_value & I2C_STAT_XRDY)
             {
                //Data is being requested
                //Send data to the bus
                *I2C_DATA = data[i];
                i = i + 1;
                //Clear the indicator
                *I2C_STAT = *I2C_STAT | I2C_STAT_XRDY;
                timeout = 0;
             }
             state = TF_NACK;
             break;
       }
    }
}


void i2c_init(void)
{
    /* Enable functional and interface clocks for i2c */
    *CM_FCLKEN1_CORE = *CM_FCLKEN1_CORE | EN_I2C1;
    *CM_ICLKEN1_CORE = *CM_ICLKEN1_CORE | EN_I2C1;

    /* setup i2c speed */
    /* I2C1_CLK = I2C1_FCLK (96 MHz) / 24 = 4 MHz */
    /* SCLt = I2C1_CLK / SCLL(40) = 100 kHz */
    /* SCHt = I2C1_CLK / SCLH(40) = 100 kHz */
    /* kbps = SCHLt + SCHl = 200 kbps */
    *I2C_PSC = 0x17; /* Prescaler sampling clock = 24 */
    *I2C_SCLL= 0x21; /* F/S SCL low time value = 33 (+7) */
    *I2C_SCLH= 0x23; /* F/S SCL high time value = 35 (+5) */
}

