/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: ARM Cortex-A9 API													*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_CORTEXA9API__
#define __J_CORTEXA9API__


/*==============================================================================*/
/* ƒo[ƒWƒ‡ƒ“î•ñ																*/
/*==============================================================================*/
#define	CORTEXA9API_VER				100

/* mmu_tbl_set()->cb */
#define MMU_CB_NONE					0x00000000
#define MMU_CB_BUFF					0x00000004
#define MMU_CB_WTHROUGH				0x00000008
#define MMU_CB_WBACK				0x0000000C
#define MMU_CB_SHARED				0x00010000

/* mmu_config()->tbl_addr_l1 */
#define	MMU_CFG_INNER_WB_ALLOC		0x40
#define	MMU_CFG_INNER_WT			0x01
#define	MMU_CFG_INNER_WB_NO_ALLOC	0x51
#define	MMU_CFG_INNER_SHARED		0x20

#define	MMU_CFG_INNER_CACHE			0x01
#define	MMU_CFG_SHARED				0x02
#define	MMU_CFG_RGN_NO_CACHE		0x00
#define	MMU_CFG_RGN_WB_WALLOC		0x08
#define	MMU_CFG_RGN_WT_NO_ALLOC		0x10
#define	MMU_CFG_RGN_WB_NO_ALLOC		0x18

/* MMU */
void mmu_tbl_set( UINT32 *tbl, UINT32 start, UINT32 size, UINT32 cb );
void mmu_config( UINT32 tbl_addr_l1 );
void mmu_enable( void );
void mmu_disable( void );

/* Interrupt */
void irq_enable( void );
void irq_restore( UINT32 stat );
UINT32 irq_disable( void );
UINT32 irq_status( void );
void fiq_enable( void );
void fiq_restore( UINT32 stat );
UINT32 fiq_disable( void );
UINT32 fiq_status( void );

/* Cache */
void icache_enable( void );
void icache_disable( void );
void icache_inv( void );
void dcache_enable( void );
void dcache_disable( void );
void dcache_wb( UINT32 addr, UINT32 size, UINT32 wait );
void dcache_wbinv( UINT32 addr, UINT32 size, UINT32 wait );
void dcache_inv( UINT32 addr, UINT32 size, UINT32 wait );

/* L2 Cache */
void cache_l2_init( void );
void cache_l2_enable( void );
void cache_l2_disable( void );
void cache_l2_wb( UINT32 addr, UINT32 size, UINT32 wait );
void cache_l2_wbinv( UINT32 addr, UINT32 size, UINT32 wait );

/* Other */
void wait_isr( void );
UINT32 get_core_id( void );


#endif /* __J_CORTEXA9API__ */





