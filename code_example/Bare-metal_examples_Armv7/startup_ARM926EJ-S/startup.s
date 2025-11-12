;----------------------------------------------------------------
; Copyright (c) 2002-2018 Arm Limited (or its affiliates). All rights reserved.
; Use, modification and redistribution of this file is subject to your possession of a
; valid End User License Agreement for the Arm Product of which these examples are part of 
; and your compliance with all applicable terms and conditions of such licence agreement.
;
; ARM926EJ-S Embedded example - Startup Code
;----------------------------------------------------------------


; Standard definitions of mode bits and interrupt (I & F) flags in PSRs

Mode_USR        EQU     0x10
Mode_FIQ        EQU     0x11
Mode_IRQ        EQU     0x12
Mode_SVC        EQU     0x13
Mode_ABT        EQU     0x17
Mode_UND        EQU     0x1B
Mode_SYS        EQU     0x1F

I_Bit           EQU     0x80               ; When I bit is set, IRQ is disabled
F_Bit           EQU     0x40               ; When F bit is set, FIQ is disabled

Section         EQU     2_0010             ; 2_ denotes a binary number
TTBit           EQU     2_10000
Domain          EQU     2_111100000
FullAccess      EQU     2_110000000000
CacheBuff       EQU     2_1100


        PRESERVE8
        AREA   VECTORS, CODE, READONLY

        ENTRY

;----------------------------------------------------------------
; Entry point for the Reset handler
;----------------------------------------------------------------

        EXPORT StartHere

StartHere

;----------------------------------------------------------------
; Exception Vector Table
;----------------------------------------------------------------
; Note: LDR PC instructions are used here, though branch (B) instructions
; could also be used, unless the exception handlers are >32MB away.

Vectors
        LDR PC, Reset_Addr
        LDR PC, Undefined_Addr
        LDR PC, SVC_Addr
        LDR PC, Prefetch_Addr
        LDR PC, Abort_Addr
        B .                                ; Reserved vector
        LDR PC, IRQ_Addr
        LDR PC, FIQ_Addr


Reset_Addr      DCD     Reset_Handler
Undefined_Addr  DCD     Undefined_Handler
SVC_Addr        DCD     SVC_Handler
Prefetch_Addr   DCD     Prefetch_Handler
Abort_Addr      DCD     Abort_Handler
IRQ_Addr        DCD     IRQ_Handler
FIQ_Addr        DCD     FIQ_Handler


;----------------------------------------------------------------
; Exception Handlers
;----------------------------------------------------------------

; The following dummy handlers do not do anything useful in this example.
; They are set up here for completeness.

Undefined_Handler
        B   Undefined_Handler
SVC_Handler
        B   SVC_Handler
Prefetch_Handler
        B   Prefetch_Handler
Abort_Handler
        B   Abort_Handler
IRQ_Handler
        B   IRQ_Handler
FIQ_Handler
        B   FIQ_Handler


        
;----------------------------------------------------------------
; Reset Handler
;----------------------------------------------------------------
Reset_Handler   FUNCTION {}

;----------------------------------------------------------------
; Disable cache and MMU in case it was left enabled from an earlier run
; This does not need to be done from a cold reset
;----------------------------------------------------------------

        MRC     p15, 0, r0, c1, c0, 0      ; Read Control Register
        BIC     r0, r0, #(0x1<<12)         ; Ensure I Cache disabled
        BIC     r0, r0, #(0x1<<3)          ; Ensure Write buffer disabled
        BIC     r0, r0, #(0x1<<2)          ; Ensure D Cache disabled
        BIC     r0, r0, #0x1               ; Clear M bit 0 to disable MMU
        MCR     p15, 0, r0, c1, c0, 0      ; Write Control Register

;----------------------------------------------------------------
; Invalidate caches
;----------------------------------------------------------------
        MOV     r0, #0
        MCR     p15, 0, r0, c7, c7, 0      ; Invalidate ICache and DCache

;----------------------------------------------------------------
; Drain write buffer
;----------------------------------------------------------------
        MCR     p15, 0, r0, c7, c10, 4     ; Drain write buffer

;----------------------------------------------------------------
; TLB maintenance, Invalidate Data and Instruction TLBs
;----------------------------------------------------------------
        MCR     p15, 0, r0, c8, c7, 0      ; ARM926EJ-S TLB invalidation

;----------------------------------------------------------------
; Initialize Supervisor Mode Stack
; Note stack must be 8 byte aligned.
;----------------------------------------------------------------

        IMPORT  ||Image$$ARM_LIB_STACK$$ZI$$Limit|| ; Linker symbol from scatter file
        LDR     SP, =||Image$$ARM_LIB_STACK$$ZI$$Limit||

;----------------------------------------------------------------
; MMU Configuration
; Set translation table base
;----------------------------------------------------------------
        IMPORT ||Image$$VECTORS$$Base||    ; From scatter file
        IMPORT  ||Image$$TTB$$ZI$$Base||

        LDR     r0,=||Image$$TTB$$ZI$$Base|| ; Set start of Translation Table base (16k Boundary)
        MCR     p15, 0, r0, c2, c0, 0

;----------------------------------------------------------------
; PAGE TABLE generation

; Generate the page tables
; Build a flat translation table for the whole address space.
; Create 4096 1MB sections from 0x000xxxxx to 0xFFFxxxxx

; 31                 20 19  18  17  16 15  14   12 11 10    9   8     5   4   3 2   1 0
; |section base address|          SBZ              |  AP | SBZ | Domain | 1 | C B | 1 0|
;
; Bits[31:20]   - Top 12 bits of VA is pointer into table
; SBZ[19:12]=0  - Should be zero
; AP[11:10]=11  - Configure for full read/write access in all modes
; Domain[8:5]=1111   - Set all pages to use domain 15
; SBZ[9]=0      - Should be zero
; 1[4]=1        - Must be 1
; CB[3:2]=00    - Uncached and unbuffered
;                 (except for the code segment descriptor, see below)
; Bits[1:0]=10  - Marks a section description

;----------------------------------------------------------------

        LDR     r1,=0xfff                  ; Loop counter
        MOV     r2, #TTBit:OR:Section      ; Build descriptor pattern in register
        ORR     r2, r2, #Domain:OR:FullAccess

        ; Use loop counter to create 4096 individual table entries.
        ; This writes from address 'Image$$TTB$$ZI$$Base' +
        ; Offset 0x3FFC down to offset 0x0 in word steps (4 bytes)
init_ttb_1
        ORR     r3, r2, r1, LSL#20         ; R3 now contains full level1 descriptor to write
        STR     r3, [r0, r1, LSL#2]        ; Str table entry at TTB base + loopcount*4
        SUBS    r1, r1, #1                 ; Decrement loop counter
        BPL     init_ttb_1

;
; In this example, the 1MB section based at '||Image$$VECTORS$$Base||' is setup specially as cacheable .
; We could perform ROM/RAM remapping using the MMU at this point if required.
;
        LDR     r1,=||Image$$VECTORS$$Base|| ; Base physical address of code segment
        LSR     r1,#20                     ; Shift right to align to 1MB boundaries
        ORR     r3, r2, r1, LSL#20         ; Setup the initial level1 descriptor again
        ORR     r3,r3,#CacheBuff           ; Set cachable and bufferable attributes for section 0 (3:2)
        STR     r3, [r0, r1, LSL#2]        ; str table entry

;----------------------------------------------------------------
; Setup domain control register - Enable all domains to client mode
;----------------------------------------------------------------

        MRC     p15, 0, r0, c3, c0, 0      ; Read Domain Access Control Register
        LDR     r0, =0x55555555            ; Initialize every domain entry to b01 (client)
        MCR     p15, 0, r0, c3, c0, 0      ; Write Domain Access Control Register

;----------------------------------------------------------------
; Enable MMU and Branch to __main
; Leaving the caches disabled until after scatter loading.
;----------------------------------------------------------------

        IMPORT  __main                     ; Before MMU enabled import label to __main
        LDR     r12,=__main                ; Save this in register for possible long jump

        MRC     p15, 0, r0, c1, c0, 0      ; Read Control Register
        BIC     r0, r0, #(0x1<<12)         ; Ensure I Cache disabled
        BIC     r0, r0, #(0x1<<3)          ; Ensure Write buffer disabled
        BIC     r0, r0, #(0x1<<2)          ; Ensure D Cache disabled
        BIC     r0, r0, #(0x1<<1)          ; Clear A bit 1 to disable strict alignment fault checking
        BIC     r0, r0, #(0x1<<13)         ; Select Normal exception vectors
        ORR     r0, r0, #0x1               ; Set M bit 0 to enable MMU before scatter loading
        MCR     p15, 0, r0, c1, c0, 0      ; Write Control Register

; Now the MMU is enabled, virtual to physical address translations will occur. This will affect the next
; instruction fetch.
;
; The two instructions currently in the pipeline will have been fetched before the MMU was enabled.
; The branch to __main is safe because the Virtual Address (VA) is the same as the Physical Address (PA)
; (flat mapping) of this code that enables the MMU and performs the branch

        BX      r12                        ; Branch to __main

        ENDFUNC


        EXPORT core_init

core_init FUNCTION

;----------------------------------------------------------------
; Enable caches and Write buffer
; Caches are controlled by the Control Register:
;----------------------------------------------------------------
        MRC     p15, 0, r0, c1, c0, 0      ; Read Control Register
        ORR     r0, r0, #(0x1<<12)         ; Enable I Cache
        ORR     r0, r0, #(0x1<<3)          ; Enable Write buffer
        ORR     r0, r0, #(0x1<<2)          ; Enable D Cache
        MCR     p15, 0, r0, c1, c0, 0      ; Write Control Register

        BX    lr

        ENDFUNC

        END
