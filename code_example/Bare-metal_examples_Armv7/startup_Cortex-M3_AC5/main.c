/*
** Copyright (c) 2006-2014 Arm Limited (or its affiliates). All rights reserved.
** Use, modification and redistribution of this file is subject to your possession of a
** valid End User License Agreement for the Arm Product of which these examples are part of 
** and your compliance with all applicable terms and conditions of such licence agreement.
*/

/* This file contains the main() program that sets the vector table location, displays a welcome message, 
initializes the MPU, starts the SysTick timer, initializes the Process Stack Pointer, changes Thread mode
to Unprivileged and to use the Process Stack, then runs the main application (sorts) */


#include "scs.h"
#include "timer.h"
#include <stdio.h>


#define VectorTableOffsetRegister 0xE000ED08
extern unsigned int Image$$VECTORS$$Base;
extern unsigned int Image$$PROCESS_STACK$$ZI$$Limit;

extern void compare_sorts(void);


__attribute__((noreturn)) void main(void)
{
    /* Processor starts-up in Privileged Thread Mode using Main Stack */

    /* Named register variables */
    register unsigned int SP_PROCESS __asm("psp");
    register unsigned int CONTROL __asm("control");

    /* Tell the processor the location of the vector table, obtained from the scatter file */
    *(volatile unsigned int *)(VectorTableOffsetRegister) = (unsigned int) &Image$$VECTORS$$Base;

    /* Display a welcome message via semihosting */
    printf("Cortex-M3 bare-metal startup example\n");

    /* Initialize MPU */
    SCS_init();

    /* Initialize SysTick Timer */
    SysTick_init();

    /* Initialize Process Stack Pointer */
    SP_PROCESS = (unsigned int) &Image$$PROCESS_STACK$$ZI$$Limit;

    /* Change Thread mode to Unprivileged and to use the Process Stack */
    CONTROL = CONTROL | 3;

    /* Flush and refill pipeline with unprivileged permissions */
    __isb(0xf);

    /* Run the main application (sorts) */
    compare_sorts();

    while( 1 )
    {
        /* Loop forever */
    }
}
