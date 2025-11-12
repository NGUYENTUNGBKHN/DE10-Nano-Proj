/* Bare-metal fireworks example for Beagle Board */

/* Basic i2c driver to drive TPS65950 */

/* Copyright (c) 2010 Arm Limited (or its affiliates). All rights reserved. */
/* Use, modification and redistribution of this file is subject to your possession of a     */
/* valid End User License Agreement for the Arm Product of which these examples are part of */
/* and your compliance with all applicable terms and conditions of such licence agreement.  */


#ifndef I2C_H_
#define I2C_H_

/* CORE registers  */
#define CM_CORE_BASE 0x48004A00
#define CM_FCLKEN1_CORE         (volatile unsigned int *)(CM_CORE_BASE + 0x00)
#define CM_ICLKEN1_CORE         (volatile unsigned int *)(CM_CORE_BASE + 0x10)

/* I2C Mask */
#define EN_I2C1   (1<<15)

/* I2C registers */
#define I2C1_BASE 0x48070000
#define I2C_STAT                (volatile uint16_t *)(I2C1_BASE + 0x08)
#define I2C_CNT                 (volatile uint16_t *)(I2C1_BASE + 0x18)
#define I2C_DATA                (volatile uint16_t *)(I2C1_BASE + 0x1c)
#define I2C_CON                 (volatile uint16_t *)(I2C1_BASE + 0x24)
#define I2C_SA                  (volatile uint16_t *)(I2C1_BASE + 0x2c)
#define I2C_PSC                 (volatile uint16_t *)(I2C1_BASE + 0x30)
#define I2C_SCLL                (volatile uint16_t *)(I2C1_BASE + 0x34)
#define I2C_SCLH                (volatile uint16_t *)(I2C1_BASE + 0x38)


/* I2C_STAT MASK bits */
#define I2C_STAT_XDR (1<<14)
#define I2C_STAT_RDR (1<<13)
#define I2C_STAT_BB  (1<<12)
#define I2C_STAT_XRDY (1<<4)
#define I2C_STAT_RRDY (1<<3)
#define I2C_STAT_ARDY (1<<2)
#define I2C_STAT_NACK (1<<1)
#define I2C_STAT_AL   (1<<0)


/* States to control i2c master transmitter mode PM, F/S*/
#define INIT       0
#define TF_NACK    1
#define TF_AL      2
#define TF_ARDY    3
#define TF_XDR     4
#define TF_XRDY    5


#endif /* I2C_H_ */
