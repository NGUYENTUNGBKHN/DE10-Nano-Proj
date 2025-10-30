;==============================================================================
; Copyright (C) 2014 JSL Technology. All right reserved.
; Tittle: ARM Cortex-A9 API(DS5)
; Comment:
;==============================================================================

;==============================================================================
; Global Symbols
;==============================================================================
	EXPORT mmu_config
	EXPORT mmu_enable
	EXPORT mmu_disable
	EXPORT irq_enable
	EXPORT irq_restore
	EXPORT irq_disable
	EXPORT irq_status
	EXPORT fiq_enable
	EXPORT fiq_restore
	EXPORT fiq_disable
	EXPORT fiq_status
	EXPORT icache_enable
	EXPORT icache_disable
	EXPORT icache_inv
	EXPORT dcache_enable
	EXPORT dcache_disable
	EXPORT dcache_wb
	EXPORT dcache_wbinv
	EXPORT dcache_inv
	EXPORT wait_isr
	EXPORT get_core_id
	
;==============================================================================
; Define
;==============================================================================
IRQ_BIT  		EQU   	0x80
FIQ_BIT  		EQU   	0x40
BPREDICT_BIT	EQU		0x800
ICACHE_BIT		EQU		0x1000
DCACHE_BIT		EQU		0x4
CACHE_IDX		EQU		32
L2_WAYBIT		EQU		29
L2_SETBIT		EQU		6

;==============================================================================
; Code 
;==============================================================================
	AREA    |.text|,CODE,READONLY
	CODE32
CLIENT_DOMAIN
	DCD	0x55555555
;==============================================================================
mmu_config
	MRC		p15, #0, r1, c2, c0, #2
	BIC		r1, r1, #0x07
	MCR		p15, #0, r1, c2, c0, #2
	MOV		r1, #0							; r1 = 0
	MCR		p15, #0, r1, c8, c7, #0			; TLB Invalid
	LDR		r1, CLIENT_DOMAIN				; r1 = CLIENT_DOMAIN
	MCR		p15, #0, r1, c3, c0, #0			; Domain access permission
	MOV		r1, #0							; r1 = 0
	MCR		p15, #0, r1, c13, c0, #1		; Process ID
	ISB
	MCR		p15, #0, r1, c7, c5, #6			; invalidate entire branch predictor array
	DSB
	MCR		p15, #0, r0, c2, c0, #0			; Write TTBR0
	MCR		p15, #0, r0, c2, c0, #1			; Write TTBR1
	BX		lr								; return
;==============================================================================
mmu_enable
	MRC		p15, #0, r0, c1, c0, #0			; Read control register
	ORR		r0, r0, #BPREDICT_BIT			; r0 |= BPREDICT_BIT, (Branch prediction enable)
	ORR		r0, r0, #1						; r0 |= 1, (MMU enable)
    MCR		p15, #0, r0, c1, c0, #0			; Write control register
	BX		lr								; return
;==============================================================================
mmu_disable
	MOV		r0, #0							; r0 = 0
	MCR		p15, #0, r0, c8, c7, #0			; TLB Invalid
	MRC		p15, #0, r0, c1, c0, #0			; Read control register
	BIC		r0, r0, #1						; r0 &= (~0x1), (MMU disable)
    MCR		p15, #0, r0, c1, c0, #0			; Write control register
	BX		lr								; return
;==============================================================================
irq_enable
	MRS		r0, cpsr						; r0 = cpsr
	BIC		r0, r0, #IRQ_BIT				; r0 &= (~IRQ_BIT)
	MSR		cpsr_c, r0						; cpsr = r0
	BX		lr								; return
;==============================================================================
irq_restore
	AND		r0, r0, #IRQ_BIT				; r0 &= IRQ_BIT
	MRS		r1, cpsr						; r1 = cpsr
	BIC		r1, r1, #IRQ_BIT				; r1 &= (~IRQ_BIT)
	ORR		r0, r0, r1						; r0 |= r1
	MSR		cpsr_c, r0						; cpsr = r0
	BX		lr								; return
;==============================================================================
irq_disable
	MRS		r0, cpsr						; r0 = cpsr
	ORR		r1, r0, #IRQ_BIT				; r1 = r0 | IRQ_BIT
	MSR		cpsr_c, r1						; cpsr = r1
	AND		r0, r0, #IRQ_BIT				; r0 &= IRQ_BIT
	BX		lr								; return
;==============================================================================
irq_status
	MRS		r0, cpsr						; r0 = cpsr
	AND		r0, r0, #IRQ_BIT				; r0 &= IRQ_BIT
	BX		lr								; return
;==============================================================================
fiq_enable
	MRS		r0, cpsr						; r0 = cpsr
	BIC		r0, r0, #FIQ_BIT				; r0 &= (~FIQ_BIT)
	MSR		cpsr_c, r0						; cpsr = r0
	BX		lr								; return
;==============================================================================
fiq_restore
	AND		r0, r0, #FIQ_BIT				; r0 &= FIQ_BIT
	MRS		r1, cpsr						; r1 = cpsr
	BIC		r1, r1, #FIQ_BIT				; r1 &= (~FIQ_BIT)
	ORR		r0, r0, r1						; r0 |= r1
	MSR		cpsr_c, r0						; cpsr = r0
	BX		lr								; return
;==============================================================================
fiq_disable
	MRS		r0, cpsr						; r0 = cpsr
	ORR		r1, r0, #FIQ_BIT				; r1 = r0 | FIQ_BIT
	MSR		cpsr_c, r1						; cpsr = r1
	AND		r0, r0, #FIQ_BIT				; r0 &= FIQ_BIT
	BX		lr								; return
;==============================================================================
fiq_status
	MRS		r0, cpsr						; r0 = cpsr
	AND		r0, r0, #FIQ_BIT				; r0 &= FIQ_BIT
	BX		lr								; return
;==============================================================================
icache_enable
	MRC		p15, #0, r0, c1, c0, #0			; Read control register
	ORR		r0, r0, #ICACHE_BIT				; r0 |= ICACHE_BIT, (icache enable)
	MCR		p15, #0, r0, c1, c0, #0			; Write control register
	BX		lr								; return
;==============================================================================
icache_disable
	MRC		p15, #0, r0, c1, c0, #0			; Read control register
	BIC		r0, r0, #ICACHE_BIT				; r0 &= (~ICACHE_BIT), (icache disable)
	MCR		p15, #0, r0, c1, c0, #0			; Write control register
	BX		lr								; return
;==============================================================================
icache_inv
	MOV		r0, #0							; r0 = 0
	MCR		p15, #0, r0, c7, c5, #0			; Invalidate icache
	BX		lr								; return
;==============================================================================
dcache_enable
	MRC		p15, #0, r0, c1, c0, #0			; Read control register
	ORR		r0, r0, #DCACHE_BIT				; r0 |= DCACHE_BIT, (icache enable)
	MCR		p15, #0, r0, c1, c0, #0			; Write control register
	BX		lr								; return
;==============================================================================
dcache_disable
	MRC		p15, #0, r0, c1, c0, #0			; Read control register
	BIC		r0, r0, #DCACHE_BIT				; r0 &= (~DCACHE_BIT), (icache disable)
	MCR		p15, #0, r0, c1, c0, #0			; Write control register
	BX		lr								; return
;==============================================================================
dcache_wb
	ADD		r1, r1, r0						; r1 += r0
	BIC		r0, r0, #(CACHE_IDX-1)			; r0 &= (~(CACHE_IDX-1))
_dcache_wb_loop
	CMP		r0, r1
	BCS		_dcache_wb_end					; if( r0 >= r1 ) goto _dcache_wb_end
	MCR		p15, #0, r0, c7, c10, #1		; clean dcache single entry (MVA)
	ADD		r0, r0, #CACHE_IDX				; r0 += CACHE_IDX
	B		_dcache_wb_loop					; goto _dcache_wb_loop
_dcache_wb_end
	TST		r2, #1
	MCRNE	p15, #0, r0, c7, c10, #4
	BX		lr								; return
;==============================================================================
dcache_wbinv
	ADD		r1, r1, r0						; r1 += r0
	BIC		r0, r0, #(CACHE_IDX-1)			; r0 &= (~(CACHE_IDX-1))
_dcache_wbinv_loop
	CMP		r0, r1
	BCS		_dcache_wbinv_end				; if( r0 >= r1 ) goto _dcache_wbinv_end
	MCR		p15, #0, r0, c7, c14, #1		; clean & invalidate dcache single entry (MVA)
	ADD		r0, r0, #CACHE_IDX				; r0 += CACHE_IDX
	B		_dcache_wbinv_loop				; goto _dcache_wbinv_loop
_dcache_wbinv_end
	TST		r2, #1
	MCRNE	p15, #0, r0, c7, c10, #4
	BX		lr								; return
;==============================================================================
dcache_inv
	ADD		r1, r1, r0						; r1 += r0
	BIC		r0, r0, #(CACHE_IDX-1)			; r0 &= (~(CACHE_IDX-1))
_dcache_inv_loop
	CMP		r0, r1
	BCS		_dcache_inv_end					; if( r0 >= r1 ) goto _dcache_inv_end
	MCR		p15, #0, r0, c7, c6, #1			; invalidate dcache single entry (MVA)
	ADD		r0, r0, #CACHE_IDX				; r0 += CACHE_IDX
	B		_dcache_inv_loop
_dcache_inv_end
	TST		r2, #1
	MCRNE	p15, #0, r0, c7, c10, #4
	BX		lr								; return
;==============================================================================
wait_isr
	WFI
	BX		lr								; return
;==============================================================================
get_core_id
	MRC		p15, #0, r0, c0, c0, #5			; read MPIDR
	AND		r0, r0, #0x0F
	BX		lr								; return
;==============================================================================
	END



