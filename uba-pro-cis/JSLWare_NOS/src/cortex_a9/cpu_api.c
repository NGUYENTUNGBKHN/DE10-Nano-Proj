/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: ARM Cortex-A9 API													*/
/* Comment:																		*/
/*==============================================================================*/

/*==============================================================================*/
/* インクルード																	*/
/*==============================================================================*/
#include "js_oswapi.h"
#include "js_io.h"
#include "l2c310/js_l2c310_reg.h"


#define	L2C_LINE_SIZE					32


/*==============================================================================*/
/* MMUテーブル生成																*/
/*==============================================================================*/
void mmu_tbl_set( UINT32 *tbl, UINT32 start, UINT32 size, UINT32 cb )
{
	UINT32 i;
	
	start >>= 20;
	size >>= 20;
	cb |= 0xC02;
	
	for( i = start ; i < (start+size) ; i ++ ){
		tbl[i] = (i << 20)|cb;
	}
}


/*==============================================================================*/
/* L2 Cache init																*/
/*==============================================================================*/
void cache_l2_init( void )
{
	UINT32 i;
	
	IOREG32(L2C_BASE,L2C_REG1_CONTROL) = 0x0;
	
	i = IOREG32(L2C_BASE,L2C_REG1_AUX) & (~0xF0000);
	IOREG32(L2C_BASE,L2C_REG1_AUX) = i | 0x70061000;
	/* Early BRESP enable */
	/* Instruction prefetch enable */
	/* Data prefetch enable */
	/* Way-size = 32K */
	/* Associativity = 16-way */
	/* Exclusive cache configuration = Enabled */
	
	IOREG32(L2C_BASE,L2C_REG1_TAG_RAM_CONTROL) = (1<<8)|(3<<4)|(2<<0);
	IOREG32(L2C_BASE,L2C_REG1_DATA_RAM_CONTROL) = (1<<8)|(3<<4)|(2<<0);
	
	IOREG32(L2C_BASE,L2C_REG7_INV_WAY) = 0xFFFF;
	while( IOREG32(L2C_BASE,L2C_REG7_INV_WAY) );
	
	IOREG32(L2C_BASE,L2C_REG2_INT_CLEAR) = 0xFFFFFFFF;
}


/*==============================================================================*/
/* L2 Cache enable																*/
/*==============================================================================*/
void cache_l2_enable( void )
{
	IOREG32(L2C_BASE,L2C_REG1_CONTROL) = 0x1;
}


/*==============================================================================*/
/* L2 Cache disable																*/
/*==============================================================================*/
void cache_l2_disable( void )
{
	IOREG32(L2C_BASE,L2C_REG1_CONTROL) = 0x0;
}


/*==============================================================================*/
/* L2 Cache Write back															*/
/*==============================================================================*/
void cache_l2_wb( UINT32 addr, UINT32 size, UINT32 wait )
{
	UINT32 s,e,i;
	
	s = addr & (~(L2C_LINE_SIZE-1));
	e = (addr + size) & (~(L2C_LINE_SIZE-1));
	for( i = s ; i <= e ; i = i + L2C_LINE_SIZE ){
		IOREG32(L2C_BASE,L2C_REG7_CLEAN_PA) = i;
	};
	if( wait ){
		__asm( " dmb " );
		while( IOREG32(L2C_BASE,L2C_REG7_CACHE_SYNC) & 0x1 );
	}
}


/*==============================================================================*/
/* L2 Cache Write back & invalidate												*/
/*==============================================================================*/
void cache_l2_wbinv( UINT32 addr, UINT32 size, UINT32 wait )
{
	UINT32 s,e,i;
	
	s = addr & (~(L2C_LINE_SIZE-1));
	e = (addr + size) & (~(L2C_LINE_SIZE-1));
	for( i = s ; i <= e ; i = i + L2C_LINE_SIZE ){
		IOREG32(L2C_BASE,L2C_REG7_CLEAN_INV_PA) = i;
	};
	if( wait ){
		__asm( " dmb " );
		while( IOREG32(L2C_BASE,L2C_REG7_CACHE_SYNC) & 0x1 );
	}
}



