// ------------------------------------------------------------
// Cortex-A MPCore - Main
//
// Copyright (c) 2011-2014 Arm Limited (or its affiliates). All rights reserved.
// Use, modification and redistribution of this file is subject to your possession of a
// valid End User License Agreement for the Arm Product of which these examples are part of 
// and your compliance with all applicable terms and conditions of such licence agreement.
// ------------------------------------------------------------


#include <stdio.h>
#include <stdint.h>
#include "v7.h"
#include "MP_GIC.h"
#include "MP_Mutexes.h"
#include "hw_init.h"
#include "alt_cache.h"

// compile-time control for the number of CPUs in the cluster
#define nCPUs 4

// per-thread space for each CPU for use by the Arm C libraries - will be zero-initialized on start-up
char user_perthread_libspace[ nCPUs ][96];

// ------------------------------------------------------------
#define CPU1_START_ADDR *(volatile unsigned int *)0xFFFFFE00
#define CPU1_LOAD_ADDR 0x00100000U
extern void *Reset_Handler;

#define MPUMODRST        0xFFD05010U
#define MPUMODRST_CPU1   (1<<1)

void enable_mmu(void)
{
    unsigned int sctlr;

    __asm {
        MRC p15, 0, sctlr, c1, c0, 0
    }

    sctlr |= 0x1;

    __asm {
        MCR p15, 0, sctlr, c1, c0, 0
        ISB
    }
}

#define SECTION_DESCRIPTOR      (0x2)       // Section entry
#define SECTION_BUFFERABLE      (0 << 2)    // Unbuffered
#define SECTION_CACHEABLE       (0 << 3)    // Uncached
#define SECTION_EXECUTE_NEVER   (0 << 4)    // Executable
#define SECTION_DOMAIN0         (0 << 5)    // Domain 0
#define SECTION_AP_RW           (0x3 << 10) // Full access: RW for privileged & user
#define SECTION_FLAGS (SECTION_DESCRIPTOR | SECTION_BUFFERABLE | SECTION_CACHEABLE | SECTION_EXECUTE_NEVER | SECTION_DOMAIN0 | SECTION_AP_RW)
#define L1_TABLE_ENTRIES 4096
unsigned int L1_page_table[L1_TABLE_ENTRIES] __attribute__((aligned(16384)));
void init_page_table(void)
{
    int i;
    unsigned int phys_addr;

    // 1. Clear toàn bộ bảng
    for (i = 0; i < L1_TABLE_ENTRIES; i++) {
        L1_page_table[i] = 0;
    }

    // 2. Map low memory 0x00000000 -> 0x00000000 (ví dụ RAM 64MB)
    for (i = 0; i < 64; i++) { // 64 MB / 1MB per section
        L1_page_table[i] = (i << 20) | SECTION_FLAGS;
    }

    // 3. Map high vector 0xFFFF0000 -> physical 0x00000000
    L1_page_table[0xFFFF0000 >> 20] = (0x00000000 & 0xFFF00000) | SECTION_FLAGS;
}

void start_cpu1()
{
	static volatile uint32_t *const mpumodrst = (uint32_t *) MPUMODRST;  //2024-11-21 1core
	if ((uintptr_t) &Reset_Handler != 0) {
			/* jump to interrupt vector base address at reset */
			*((uint32_t *) 0) = 0xE51FF004U;  /* LDR pc, [pc,#-4] */
			*((uint32_t *) 4) = (uintptr_t) &Reset_Handler;

			/* flush data cache */
			alt_cache_system_clean((void *) 0, ALT_CACHE_LINE_SIZE);
		}

	*mpumodrst &= ~MPUMODRST_CPU1;	 //2024-11-21 1core
	__asm {
	        DSB
	        ISB
	    }

}

extern void main_app(void);


// this main() is only executed by CPU 0
__attribute__((noreturn)) void main(void)
{

//	hw_init();
//	init_page_table();
	start_cpu1();
    sendSGI(0x0, 0x0F, 0x01); // Wake the secondary CPUs by sending SGI (ID 0)
//    enable_mmu();
    main_app();

    for(;;) {} //loop forever
}


// Called by the C library to get the address of the per-thread libspace
// Return a separate space for each CPU
void * __user_perthread_libspace(void)
{
  return user_perthread_libspace[ getCPUID() ];
}


// Called by the C library to initialize the mutex to an unlocked state.
// Return a nonzero value to indicate to the C library that it is being used in a multithreaded environment.
int _mutex_initialize(mutex_t *m)
{
  initMutex(m);
  return 1;
}

// Called by the C library to obtain a lock on the mutex.
void _mutex_acquire(mutex_t *m)
{
  lockMutex(m);
}

// Called by the C library to release the lock on the mutex previously acquired
void _mutex_release(mutex_t *m)
{
  unlockMutex(m);
}


// ------------------------------------------------------------
// End of main.c
// ------------------------------------------------------------
