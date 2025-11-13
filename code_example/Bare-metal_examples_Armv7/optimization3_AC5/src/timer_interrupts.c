/* Bare-metal fireworks example for Beagle Board */

/* Timer and interrupts */

/* Copyright (c) 2010-2011 Arm Limited (or its affiliates). All rights reserved. */
/* Use, modification and redistribution of this file is subject to your possession of a     */
/* valid End User License Agreement for the Arm Product of which these examples are part of */
/* and your compliance with all applicable terms and conditions of such licence agreement.  */

// "MPU INTC" Interrupt Controller
#define INTCPS_BASE 0x48200000
#define INTCPS_SIR_IRQ    (volatile unsigned int *)(INTCPS_BASE + 0x40)
#define INTCPS_CONTROL    (volatile unsigned int *)(INTCPS_BASE + 0x48)
#define INTCPS_MIR_CLEAR0 (volatile unsigned int *)(INTCPS_BASE + 0x88)
#define INTCPS_MIR_CLEAR1 (volatile unsigned int *)(INTCPS_BASE + 0x88 + 0x20)
#define GPT1_IRQ 37

// General Purpose Timer 1
#define GPTIMER1_BASE 0x48318000
#define GPTIMER1_TISR (volatile unsigned int *)(GPTIMER1_BASE + 0x18)
#define GPTIMER1_TIER (volatile unsigned int *)(GPTIMER1_BASE + 0x1C)
#define GPTIMER1_TCLR (volatile unsigned int *)(GPTIMER1_BASE + 0x24)
#define GPTIMER1_TCRR (volatile unsigned int *)(GPTIMER1_BASE + 0x28)
#define GPTIMER1_TLDR (volatile unsigned int *)(GPTIMER1_BASE + 0x2C)

#define CM_CLKSEL_WKUP (volatile unsigned int *)0x48004c40

// Timer events
#define OVERFLOW   2
#define ALL_EVENTS 7

// LEDs
#define GPIO5_BASE 0x49056000
#define LED_OFF (volatile unsigned int *)(GPIO5_BASE + 0x90)
#define LED_ON  (volatile unsigned int *)(GPIO5_BASE + 0x94)
#define LED_0 0x00200000  // GPIO5 bit 21 = "GPIO_149" (output)
#define LED_1 0x00400000  // GPIO5 bit 22 = "GPIO_150" (output)

#define LED_0_OFF *LED_OFF = LED_0
#define LED_0_ON  *LED_ON  = LED_0
#define LED_1_OFF *LED_OFF = LED_1
#define LED_1_ON  *LED_ON  = LED_1

unsigned int tick_count = 0;

// Change LED state on each call, modulo 4
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
    // Which timer event occurred - was it overflow (time-out) ?
    if (*GPTIMER1_TISR & OVERFLOW)
    {
        // Yes, so now perform tick actions

        tick_count++; // increment milliSecond counter

        if ((tick_count & 1023) == 1023 )
            nudge_leds(); // nudge the LEDs approx every Second
    }

    // Clear all events
    *GPTIMER1_TISR = ALL_EVENTS;
}


// 1st level handling in the Interrupt Controller
void C_interrupt_handler(void)
{
    // Identify interrupt source - was it Timer 1 ?
    if (*INTCPS_SIR_IRQ == GPT1_IRQ)
    {
        // Yes, so now process the timer interrupt
        timer_interrupt();
    }
    else
    {
        // No, an unrecognised interrupt occurred
        // Endless loop trap for debugging
        while (1) {}
    }

    // Reset interrupt output and allow subsequent interrupts
    *INTCPS_CONTROL = 1;

    // Data Sync Barrier to ensure previous writes over Interconnect bus complete
    __dsb(15);
}


// Initialize Timer 1 and Interrupt Controller
void init_timer(void)
{
    // Stop timer
    *GPTIMER1_TCLR = 0;

    // Selects GPTIMER1 source clock as always-on 32K_FCLK (32,768 Hz)
    *CM_CLKSEL_WKUP = 0;

    // Configure timer interval ~ 1mS
    *GPTIMER1_TCRR = 0xFFFFFFFF - 32768/1000; // Initial value to start incrementing from
    *GPTIMER1_TLDR = 0xFFFFFFFF - 32768/1000; // Reload value after overflow

    // Clear any pending events in timer
    *GPTIMER1_TISR = ALL_EVENTS;

    // Enable overflow event in timer
    *GPTIMER1_TIER = OVERFLOW;

    // Clear mask for GPT1_IRQ in Interrupt Controller
    *INTCPS_MIR_CLEAR1 = 0x20;

    // Could also configure priorities and/or steering to IRQ/FIQ in Interrupt Controller.
    // By default, interrupts are mapped to IRQ and priority is 0x0 (the highest), which is OK here.

    // Set auto-reload mode and start timer
    *GPTIMER1_TCLR = 3;

    // Data Sync Barrier to ensure previous writes over Interconnect bus complete
    __dsb(15);
}
