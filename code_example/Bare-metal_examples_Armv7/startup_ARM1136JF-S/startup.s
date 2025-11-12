;----------------------------------------------------------------
; Copyright (c) 2005-2018 Arm Limited (or its affiliates). All rights reserved.
; Use, modification and redistribution of this file is subject to your possession of a
; valid End User License Agreement for the Arm Product of which these examples are part of 
; and your compliance with all applicable terms and conditions of such licence agreement.
;
; ARM1136JF-S Embedded example - Startup Code
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


;----------------------------------------------------------------
; Exception Vector Table
;----------------------------------------------------------------

; Note: LDR PC instructions are used here, though branch (B) instructions
; could also be used, unless the exception handlers are >32MB away.

        PRESERVE8
        AREA   HAND, CODE, READONLY
Hand
        LDR PC, Undefined_Addr
        LDR PC, SVC_Addr
        LDR PC, Prefetch_Addr
        LDR PC, Abort_Addr
        B .                                ; Reserved vector
        LDR PC, IRQ_Addr
        LDR PC, FIQ_Addr

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

; Reset vector is deliberately separated from other vectors to match
; i.MX35 handler remapping - see scatter-file scatter.scat

        AREA   RESET, CODE, READONLY
        ENTRY

;----------------------------------------------------------------
; Entry point for the Reset handler
;----------------------------------------------------------------
       EXPORT StartHere
StartHere
       LDR PC, Reset_Addr


Reset_Addr      DCD     Reset_Handler

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
        BIC     r0, r0, #(0x1<<2)          ; Ensure D Cache disabled
        BIC     r0, r0, #0x1               ; Clear M bit 0 to disable MMU
        MCR     p15, 0, r0, c1, c0, 0      ; Write Control Register

;----------------------------------------------------------------
; Invalidate instruction and data cache, also flushes BTAC
;----------------------------------------------------------------

        MOV     r0, #0                     ; SBZ
        MCR     p15, 0, r0, c7, c7, 0      ; Invalidate both caches
                                           ; Also flushes BTAC

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
        IMPORT  ||Image$$VECTORS$$Base||    ; From scatter file
        IMPORT  ||Image$$TTB$$ZI$$Base||

        ; Specify v6 format pagetables with no subpages
        ; set bit 23 [XP] in CP15 control register.

        MRC     p15, 0, r0, c1, c0, 0
        MOV     r1, #0x800000
        ORR     r0, r0, r1
        MCR     p15, 0, r0, c1, c0, 0

        ; Two translation tables are supported, TTBR0 and TTBR1
        ; Configure translation table base (TTB) control register cp15,c2
        ; to a value of all zeros, indicates we are using TTB register 0.

        MOV     r0, #0x0
        MCR     p15, 0, r0, c2, c0, 2

        ; Write the address of our page table base to TTB register 0.

        LDR     r0,=||Image$$TTB$$ZI$$Base|| ; Set start of Translation Table base (16k Boundary)
        MCR     p15, 0, r0, c2, c0, 0

;----------------------------------------------------------------
; PAGE TABLE generation

; Generate the page tables. ARMv6 first-level descriptor formats with subpages disabled
; Build a flat translation table for the whole address space.
; ie: Create 4096 1MB sections from 0x000xxxxx to 0xFFFxxxxx


; 31                 20 19  18  17  16 15  14   12 11 10  9  8     5   4    3 2   1 0
; |section base address| 0  0  |nG| S |APX|  TEX  |  AP | P | Domain | XN | C B | 1 0|
;
; Bits[31:20]   - Top 12 bits of VA is pointer into table
; nG[17]=0      - Translation is marked as global.
; S[16]=0       - Translation is for Non-Shared memory.
; APX[15]=0     - Disable access permissions extension (APX)
; AP[11:10]=11  - Configure for full read/write access in all modes
; TEX[14:12]=000
; CB[3:2]= 00   - Set attributes to Strongly-ordered memory.
;                 (except for the code segment descriptor, see below)
; IMPP[9]=0     - Ignored
; Domain[5:8]=1111   - Set all pages to use domain 15
; XN[4]=0       - Execute never disabled
; Bits[1:0]=10  - Indicate entry is a 1MB section
;----------------------------------------------------------------

        LDR     r1,=0xfff                  ; Loop counter
        LDR     r2,=2_00000000000000000000110111100010

        ; r0 contains the address of the translation table base
        ; r1 is loop counter
        ; r2 is level1 descriptor (bits 19:0)

        ; Use loop counter to create 4096 individual table entries.
        ; This writes from address 'Image$$TTB$$ZI$$Base' +
        ; offset 0x3FFC down to offset 0x0 in word steps (4 bytes)

init_ttb_1

        ORR     r3, r2, r1, LSL#20         ; r3 now contains full level1 descriptor to write
        STR     r3, [r0, r1, LSL#2]        ; str table entry at TTB base + loopcount*4
        SUBS    r1, r1, #1                 ; decrement loop counter
        BPL     init_ttb_1

        ; In this example, the 1MB section based at '||Image$$VECTORS$$Base||' is setup specially as cacheable (write back mode).
        ; TEX[14:12]=000 and CB[3:2]= 11, Outer and inner write back, no Write-allocate normal memory.

        LDR     r1,=||Image$$VECTORS$$Base|| ; Base physical address of code segment
        LSR     r1,#20                     ; Shift right to align to 1MB boundaries
        ORR     r3, r2, r1, LSL#20         ; Setup the initial level1 descriptor again
        ORR     r3,r3,#2_0000000001100     ; Set CB bits
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
        BIC     r0, r0, #(0x1<<2)          ; Ensure D Cache disabled
        BIC     r0, r0, #(0x1<<1)          ; Clear A bit  1 to disable strict alignment fault checking
        ORR     r0, r0, #0x1               ; Set M bit 0 to enable MMU before scatter loading
        MCR     p15, 0, r0, c1, c0, 0      ; Write Control Register

; Now the MMU is enabled, virtual to physical address translations will occur. This will affect the next
; instruction fetch.
;
; The two instructions currently in the pipeline will have been fetched before the MMU was enabled.
; The branch to __main is safe because the Virtual Address (VA) is the same as the Physical Address (PA)
; (flat mapping) of this code that enables the MMU and performs the branch

        BX      r12                        ; branch to __main

        ENDFUNC


        EXPORT core_init

core_init FUNCTION

;----------------------------------------------------------------
; Enable caches
; Caches are controlled by the Control Register:
;----------------------------------------------------------------
        MRC     p15, 0, r0, c1, c0, 0      ; Read Control Register
        ORR     r0, r0, #(0x1<<12)         ; Enable I Cache
        ORR     r0, r0, #(0x1<<2)          ; Enable D Cache
        MCR     p15, 0, r0, c1, c0, 0      ; Write Control Register

;----------------------------------------------------------------
; Enable Program Flow Prediction
;
; Branch prediction is controlled by the Auxiliary Control Register:
; Set Bit 0 to enable return stack (Set on reset)
; Set Bit 1 to enable dynamic branch prediction (Set on reset)
; Set Bit 2 to enable static branch prediction (Set on reset)
;----------------------------------------------------------------

        MRC     p15, 0, r0, c1, c0, 1      ; Read Auxiliary Control Register
        MOV     r1, #0x7
        ORR     r0 ,r0, r1
        MCR     p15, 0, r0, c1, c0, 1      ; Write Auxiliary Control Register

        MRC     p15 ,0, r0, c1, c0, 0      ; Read Control Register
        ORR     r0, r0, #(0x1 << 11)       ; Enable Program flow prediction
        MCR     p15, 0, r0, c1, c0, 0      ; Write Control Register

        BX     lr

        ENDFUNC

        END
