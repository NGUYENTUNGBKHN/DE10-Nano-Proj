/*
** Copyright (c) 2006-2014 Arm Limited (or its affiliates). All rights reserved.
** Use, modification and redistribution of this file is subject to your possession of a
** valid End User License Agreement for the Arm Product of which these examples are part of 
** and your compliance with all applicable terms and conditions of such licence agreement.
*/

/* This file contains the main() program that sets the vector table location, displays a welcome message,
starts the SysTick timer, initializes the Process Stack Pointer, changes Thread mode
to use the Process Stack, then runs the main application (sorts) */


#include "scs.h"
#include "timer.h"
#include <stdio.h>
#include "stack.h"


extern unsigned int Image$$PROCESS_STACK$$ZI$$Limit;

extern void compare_sorts(void);


__attribute__((noreturn)) void main(void)
{
    /* Processor starts-up in Privileged Thread Mode using Main Stack */

    /* Named register variables */
    register unsigned int SP_PROCESS __asm("psp");
    register unsigned int CONTROL __asm("control");

    /* Display a welcome message via semihosting */
    printf("Cortex-M0 bare-metal startup example\n");

    /* Initialize SysTick Timer */
    SysTick_init();

    /* Initialize Process Stack Pointer using linker-generated symbol from scatter-file */
    SP_PROCESS = (unsigned int) &Image$$PROCESS_STACK$$ZI$$Limit;

    /* Change Thread mode to use the Process Stack */
    CONTROL = CONTROL | 2;

    /* Flush and refill pipeline before proceeding */
    __isb(0xf);

    /* Run the main application (sorts) */
    compare_sorts();

    while( 1 )
    {
        /* Loop forever */
    }
}


/* Initialize stack and heap symbols for microlib */
__asm void dummy_function(void)
{
    EXPORT __initial_sp
    EXPORT __heap_base
    EXPORT __heap_limit

__initial_sp EQU STACK_BASE
__heap_base  EQU HEAP_BASE
__heap_limit EQU (HEAP_BASE + HEAP_SIZE)
}
