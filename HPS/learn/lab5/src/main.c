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

// compile-time control for the number of CPUs in the cluster
#define nCPUs 4

// per-thread space for each CPU for use by the Arm C libraries - will be zero-initialized on start-up
char user_perthread_libspace[ nCPUs ][96];
//extern uint32_t page_table[];   // bảng trang hiện tại bạn đã dùng
//#define SECTION_DESCRIPTOR   (0x2)
//#define AP_FULL_ACCESS       (3 << 10)
//#define DOMAIN0              (0 << 5)
//#define TEXCB_DEVICE         (0 << 12) | (0 << 3) | (2 << 2)  // Strongly-Ordered
//#define SECTION_FLAGS        (AP_FULL_ACCESS | DOMAIN0 | TEXCB_DEVICE | SECTION_DESCRIPTOR)
//
//static inline void mmu_flush_tlb(void)
//{
//    asm volatile("dsb");
//    asm volatile("mcr p15, 0, r0, c8, c7, 0"); // Invalidate entire unified TLB
//    asm volatile("dsb");
//    asm volatile("isb");
//}
//
//void mmu_map_sysmgr_region(void)
//{
//    uint32_t section = 0xFFD; // 0xFFD00000 >> 20 = 0xFFD
//    uint32_t phys = 0xFFD00000;
//
//    page_table[section] = (phys & 0xFFF00000) | SECTION_FLAGS;
//
//    // Đồng bộ và cập nhật TLB
//    mmu_flush_tlb();
//}

// ------------------------------------------------------------
#define CPU1STARTADDR_PTR   ((volatile uint32_t *)0xFFD080C4U)
#define CPU1START_PTR       ((volatile uint32_t *)0xFFD080C0U)
#define CPU1_LOAD_ADDR      0x00100000U

void start_cpu1()
{
	*CPU1STARTADDR_PTR = CPU1_LOAD_ADDR;
	*CPU1START_PTR = 1U;
}

extern void main_app(void);


// this main() is only executed by CPU 0
__attribute__((noreturn)) void main(void)
{

//	hw_init();
	start_cpu1();
    sendSGI(0x0, 0x0F, 0x01); // Wake the secondary CPUs by sending SGI (ID 0)

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
