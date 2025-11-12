/* Bare-metal fireworks example for Pandaboard */

/* Timer and interrupts */

/* Copyright (c) 2011 Arm Limited (or its affiliates). All rights reserved. */
/* Use, modification and redistribution of this file is subject to your possession of a     */
/* valid End User License Agreement for the Arm Product of which these examples are part of */
/* and your compliance with all applicable terms and conditions of such licence agreement.  */

// "MPU INTC" Interrupt Controller
// Distributor
#define INT_DISTRIBUTOR 0x48241000
#define ICDDCR   (volatile unsigned int *)(INT_DISTRIBUTOR + 0x00)
#define ICDISER0 (volatile unsigned int *)(INT_DISTRIBUTOR + 0x100)
#define ICDICPR0 (volatile unsigned int *)(INT_DISTRIBUTOR + 0x280)

// Processor interfaces
#define PROC_INTERFACE 0x48240100
#define PROC_ICCICR  (volatile unsigned int *)(PROC_INTERFACE + 0x00)
#define PROC_ICCIAR  (volatile unsigned int *)(PROC_INTERFACE + 0x0c)
#define PROC_ICCEOIR (volatile unsigned int *)(PROC_INTERFACE + 0x10)

// Private timer
#define PRIVATE_TIMER 0x48240600
#define PT_LOAD    (volatile unsigned int *)(PRIVATE_TIMER + 0x00)
#define PT_COUNTER (volatile unsigned int *)(PRIVATE_TIMER + 0x04)
#define PT_CONTROL (volatile unsigned int *)(PRIVATE_TIMER + 0x08)
#define PT_STATUS  (volatile unsigned int *)(PRIVATE_TIMER + 0x0C)
#define PRIVATE_TMR 29

// LEDs
#define GPIO1_BASE 0x4A310000
#define GPIO1_OE (volatile unsigned int *)(GPIO1_BASE + 0x134)

//definitions for GPIO_0
#define LED_OFF (volatile unsigned int *)(GPIO1_BASE + 0x190)
#define LED_ON  (volatile unsigned int *)(GPIO1_BASE + 0x194)
#define LED_0 1<<7
#define LED_1 1<<8

#define LED_0_OFF *LED_OFF = LED_0
#define LED_0_ON  *LED_ON  = LED_0
#define LED_1_OFF *LED_OFF = LED_1
#define LED_1_ON  *LED_ON  = LED_1


// Change LED state on each tick, modulo 4
void nudge_leds(void)
{
    static int nudge = 0;

    if      (nudge == 0) { LED_0_OFF; LED_1_OFF; }
    else if (nudge == 1) { LED_0_ON;  LED_1_OFF; }
    else if (nudge == 2) { LED_0_OFF; LED_1_ON;  }
    else if (nudge == 3) { LED_0_ON;  LED_1_ON;  }

    nudge++;
    if (nudge >= 4) nudge = 0;
}


// 2nd level handling in the Timer
void timer_interrupt(void)
{
	// Perform tick actions
	nudge_leds();

}


// 1st level handling in the Interrupt Controller
void C_interrupt_handler(unsigned int intID)
{
    // Identify interrupt source - was it internal timer ?
    if (intID == PRIVATE_TMR)
    {
        // Yes, so now process the timer interrupt
        timer_interrupt();
        // Reset interrupt output and allow subsequent interrupts
        *PROC_ICCEOIR = PRIVATE_TMR;
    }

    // Data Sync Barrier to ensure previous writes over Interconnect bus complete
    __dsb(15);
}


// Initialise Private Timer and Interrupt Controller
void init_timer(void)
{
	//LED0 and LED1 as output
	*GPIO1_OE = *GPIO1_OE & ~(LED_0) & ~(LED_1);

    // Initialise Core private timer
    *PT_LOAD = 0x0FFFFFFF;      // Reload value after overflow
    *PT_COUNTER = 0x0FFFFFFF;   // Initial value to start incrementing from
    *PT_CONTROL = 0x00000007;   // Enable timer, interrupts and reload

    //Enable GIC
    *ICDDCR = 0x1;
    *PROC_ICCEOIR = 0x3FF;
    //Enable GIC Processor Interface. Enable signaling of interrupts.
    *PROC_ICCICR = 0x1;
    //Clear all pending interrupts
    *ICDICPR0 = 0;
    //Install private timer interrupt (Num 29)
    *ICDISER0 = 0x20000000;

    // Data Sync Barrier to ensure previous writes over Interconnect bus complete
    __dsb(15);
}
