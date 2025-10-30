/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: QSPI Flash driver													*/
/* Comment:																		*/
/*==============================================================================*/

/*==============================================================================*/
/* インクルード																	*/
/*==============================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "js_oswapi.h"
#include "js_io.h"
#include "js_qspi_flash.h"
#include "js_qspi_flash_reg.h"
#if JS_E_DEBUG
#include "js_e_debug.h"
#else
#define	_E_DEBUG()
#endif


/*==============================================================================*/
/* デバッグトレース宣言(有効にするとトレース出力あり)							*/
/*==============================================================================*/
#define	DBG_ERR()			//osw_printf("ERR:%s(line %u)\n",__FUNCTION__,__LINE__)
#define DBG_TRACE1(...)		//osw_printf(__VA_ARGS__)
#define DBG_TRACE2(...)		//osw_printf(__VA_ARGS__)
#if DBG_ERR_ALL_ENABLE
#ifdef DBG_ERR
#undef DBG_ERR
#define	DBG_ERR() osw_printf("ERR:%s(line %u)\n",__FUNCTION__,__LINE__)
#endif
#endif


/*==============================================================================*/
/* ローカルデファイン															*/
/*==============================================================================*/
#ifndef QSPI_FLASH_MAX_PORT
#define	QSPI_FLASH_MAX_PORT		1
#endif

#define	QSPI_FLASH_CMD_TOUT		(1000*4)

#define	QCMD_EXECUTE			0x1
#define	QCMD_STAT				0x2
#define	QCMD_DUMMY(n)			(n<<7)
#define	QCMD_WR(n)				(((n-1)<<12)|(1<<15))
#define	QCMD_ADDR(n)			(((n-1)<<16)|(1<<19))
#define	QCMD_RD(n)				(((n-1)<<20)|(1<<23))
#define	QCMD_OP(n)				((UINT32)n<<24)

#define	QSPI_AT_FLAG_STATUS_70h		0x0001
#define	QSPI_AT_ENTER_4BYTE_B7h		0x0002
#define	QSPI_AT_BANK_ADDR_17h		0x0004
#define QSPI_AT_BULK_ERACE_CODE(n)	((n>>24)&0xFF)

/*==============================================================================*/
/* ローカル構造体																*/
/*==============================================================================*/

typedef struct {
	UINT32 base;
	QSPI_FLASH_PARAM prm;
	OSW_SEM_HANDLE hSem;
	UINT32 cmd_mode;
	UINT32 size;
	UINT32 cmdadr;
} QSPI_FLASH_STR;

typedef struct {
	UINT32 pack;
	UINT32 index;
	UINT32 sector;
	UINT32 sector_size;
	const UINT32 *info;
	UINT8 erase_code;
} QSPI_FLASH_BLK_CONTEXT;

static QSPI_FLASH_STR *pshQspi[QSPI_FLASH_MAX_PORT];
static OSW_MEM_HANDLE *pshMemQspi = 0;
static UINT32 qspi_device_cnt;


/*==============================================================================*/
/* ベースアドレステーブル														*/
/*==============================================================================*/
const UINT32 qspi_base[QSPI_PORT_CNT] = {
#if (QSPI_PORT_CNT >= 1)
	QSPI0_BASE
#endif
#if (QSPI_PORT_CNT >= 2)
	,QSPI1_BASE
#endif
#if (QSPI_PORT_CNT >= 3)
	,QSPI2_BASE
#endif
#if (QSPI_PORT_CNT >= 4)
	,QSPI3_BASE
#endif
#if (QSPI_PORT_CNT >= 5)
	,QSPI4_BASE
#endif
#if (QSPI_PORT_CNT >= 6)
	,QSPI5_BASE
#endif
#if (QSPI_PORT_CNT >= 7)
	,QSPI6_BASE
#endif
#if (QSPI_PORT_CNT >= 8)
	,QSPI7_BASE
#endif
};


/*==============================================================================*/
/* コマンドコード																*/
/*==============================================================================*/
static const UINT8 cmd_fast_read[2] = {
	0x0B,0x0C
};
static const UINT8 cmd_page_program[2] = {
	0x02,0x12
};


/*==============================================================================*/
/* Sector Context設定															*/
/*==============================================================================*/
static INT8 qspi_context_cfg( QSPI_FLASH_BLK_CONTEXT *tex, const UINT32 *info, UINT32 addr )
{
	UINT32 *wp,i,j,k;
	
	wp = (UINT32 *)info;
	k = 0;
	j = 0;
	
	while( 1 ){
		if( wp[0] == 0 ){
			DBG_ERR();
			return( FALSE );
		}
		for( i = 0 ; i < wp[1] ; i ++ ){
			if( (addr < (j + wp[0])) ){
				tex->pack = k;
				tex->index = i;
				tex->sector = j;
				tex->sector_size = wp[0];
				tex->info = info;
				tex->erase_code = (UINT8)wp[2];
				return( TRUE );
			}
			j += wp[0];
		}
		k++;
		wp = &wp[3];
	};
}


/*==============================================================================*/
/* Next Sector																	*/
/*==============================================================================*/
static INT8 qspi_context_nextsector( QSPI_FLASH_BLK_CONTEXT *tex )
{
	UINT32 i;
	
	i = tex->pack * 3;
	if( tex->info[i] ){
		tex->index++;
		tex->sector += tex->info[i];
		if( tex->index >= tex->info[i+1] ){
			tex->pack++;
			tex->index = 0;
			i = tex->pack * 3;
			tex->sector_size = tex->info[i];
			if( tex->sector_size == 0 ){
				return( FALSE );
			}
		}
		tex->erase_code = tex->info[i+2];
	}
	return( TRUE );
}


/*==============================================================================*/
/* Comand Issue																	*/
/*==============================================================================*/
static INT8 qspi_cmd( QSPI_FLASH_STR *pQspi, UINT32 cmd )
{
	UINT32 st,t;
	
	IOREG32(pQspi->base,QSPI_FLASHCMD) = cmd;
	
	st = OSW_TIM_value();
	while( 1 ){
		t = OSW_TIM_value() - st;
		if( t >= QSPI_FLASH_CMD_TOUT ){
			DBG_ERR();
			return( FALSE );
		}
		if( (IOREG32(pQspi->base,QSPI_FLASHCMD) & 0x3) == 0 ) break;
		if( t >= 1 ){
			OSW_TSK_sleep( 1 );
		}
	};
	
	while( 1 ){
		t = OSW_TIM_value() - st;
		if( t >= QSPI_FLASH_CMD_TOUT ){
			DBG_ERR();
			return( FALSE );
		}
		if( IOREG32(pQspi->base,QSPI_CFG) & ((UINT32)1<<31) ) break;
		if( t >= 1 ){
			OSW_TSK_sleep( 1 );
		}
	};
	
	return( TRUE );
}


/*==============================================================================*/
/* Busy Status																	*/
/*==============================================================================*/
static INT8 qspi_busy( QSPI_FLASH_STR *pQspi, INT8 *stat )
{
	UINT8 c;
	
	*stat = TRUE;
	if( CFG_QSPI_ATTR[pQspi->prm.port] & QSPI_AT_FLAG_STATUS_70h ){
		/* Read Flag Status */
		if( qspi_cmd( pQspi, (QCMD_EXECUTE|QCMD_RD(1)|QCMD_OP(0x70)) ) == FALSE ){
			DBG_ERR();
			*stat = FALSE;
			return( FALSE );
		}
		c = IOREG32(pQspi->base,QSPI_FLASHCMDRDDATALO);
		if( (*stat == TRUE && ((c & 0x80) == 0)) ){
			return( TRUE );
		}
	}
	else {
		/* Read Status */
		if( qspi_cmd( pQspi, (QCMD_EXECUTE|QCMD_RD(1)|QCMD_OP(0x05)) ) == FALSE ){
			DBG_ERR();
			*stat = FALSE;
			return( FALSE );
		}
		c = IOREG32(pQspi->base,QSPI_FLASHCMDRDDATALO);
		if( (*stat == TRUE && (c & 0x1)) ){
			return( TRUE );
		}
	}
	return( FALSE );
}


/*==============================================================================*/
/* Read 																		*/
/*==============================================================================*/
static INT8 qspi_read( QSPI_FLASH_STR *pQspi, UINT8 *buf, UINT32 addr, UINT32 len )
{
	UINT32 i;
	UINT32 w[2];
	INT8 ret = TRUE;
	
	while( len ){
		i = len;
		if( i > 8 ) i = 8;
		
		IOREG32(pQspi->base,QSPI_FLASHCMDADDR) = addr;
		/* Fast Read */
		if( qspi_cmd( pQspi, (QCMD_EXECUTE|pQspi->cmdadr|QCMD_DUMMY(1)|QCMD_RD(i)|QCMD_OP(cmd_fast_read[pQspi->cmd_mode])) ) == FALSE ){
			DBG_ERR();
			ret = FALSE;
			break;
		}
		
		w[0] = IOREG32(pQspi->base,QSPI_FLASHCMDRDDATALO);
		w[1] = IOREG32(pQspi->base,QSPI_FLASHCMDRDDATAUP);
		memcpy( (void *)buf, (void *)w, i );
		
		len -= i;
		addr += i;
		buf = &buf[i];
	};
	
	return( ret );
}


/*==============================================================================*/
/* Program Command																*/
/*==============================================================================*/
static INT8 qspi_program_sect( QSPI_FLASH_STR *pQspi, QSPI_FLASH_BLK_CONTEXT *tex, 
				UINT8 *buf, UINT32 addr, UINT32 len, UINT32 *itime, UINT32 *byte )
{
	UINT32 time,i,a,m,w[2];
	INT8 ret = TRUE;
	DBG_TRACE2( "qspi:qspi_program_sect(%u,0x%08X,0x%08X)\n", pQspi->prm.port, addr,len );
	
	if( pQspi->prm.status_cb_func ){
		(*pQspi->prm.status_cb_func)( QSPI_FLASH_STAT_WRITEADDR, addr, pQspi->prm.cb_arg );
	}
	
	a = addr;
	m = len;
	
	while( len ){
		i = len;
		if( i > 8 ) i = 8;
		
		/* Write Enable */
		if( qspi_cmd( pQspi, (QCMD_EXECUTE|QCMD_OP(0x06)) ) == FALSE ){
			DBG_ERR();
			ret = FALSE;
			break;
		}
		
		w[0] = w[1] = 0;
		memcpy( (void *)w, (void *)buf, i );
		IOREG32(pQspi->base,QSPI_FLASHCMDADDR) = addr;
		IOREG32(pQspi->base,QSPI_FLASHCMDWRDATALO) = w[0];
		IOREG32(pQspi->base,QSPI_FLASHCMDWRDATAUP) = w[1];
		/* Page Program */
		if( qspi_cmd( pQspi, (QCMD_EXECUTE|pQspi->cmdadr|QCMD_WR(i)|QCMD_OP(cmd_page_program[pQspi->cmd_mode])) ) == FALSE ){
			DBG_ERR();
			ret = FALSE;
			break;
		}
		
		time = OSW_TIM_value();
		do {
			if( (OSW_TIM_value() - time) >= QSPI_FLASH_WRITE_TOUT ){
				DBG_ERR();
				ret = FALSE;
				break;
			}
			if( qspi_busy( pQspi, &ret ) == FALSE ) break;
		} while( 1 );
		if( ret == FALSE ) break;
		
		len -= i;
		addr += i;
		buf = &buf[i];
	};
	
	if( ret == TRUE ){
		*byte = *byte + m;
	}
	
	if( (ret == FALSE) && pQspi->prm.status_cb_func ){
		(*pQspi->prm.status_cb_func)( QSPI_FLASH_STAT_WRITE_ERR, a, pQspi->prm.cb_arg );
	}
	
	return( ret );
}


/*==============================================================================*/
/* Verify																		*/
/*==============================================================================*/
static INT8 qspi_verify( QSPI_FLASH_STR *pQspi, UINT8 *buf, UINT32 addr, UINT32 len )
{
	UINT32 i;
	UINT8 c[32];
	
	while( len ){
		i = len;
		if( i >= sizeof(c) ) i = sizeof(c);
		if( qspi_read( pQspi, c, addr, i ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
		if( memcmp( (void *)c, (void *)buf, i ) != 0 ){
			DBG_ERR();
			return( FALSE );
		}
		len -= i;
		addr += i;
		buf = &buf[i];
	};
	return( TRUE );
}


/*==============================================================================*/
/* Program 																		*/
/*==============================================================================*/
static INT8 qspi_program( QSPI_FLASH_STR *pQspi, UINT8 *buf, UINT32 addr, UINT32 len, UINT32 *byte )
{
	QSPI_FLASH_BLK_CONTEXT tex0;
	UINT32 i,j,page,itime;
	UINT8 n;
	
	n = pQspi->prm.port;
	DBG_TRACE2( "qspi:qspi_program(%u)\n", n );
	
	if( len == 0 ) return( TRUE );
	
	if( qspi_context_cfg( &tex0, CFG_QSPI_FLASH_INFO[n], addr ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	page = CFG_QSPI_FLASH_PAGE_SIZE[n];
	itime = OSW_TIM_value();
	
	while( len ){
		i = len;
		if( i > tex0.sector_size ) i = tex0.sector_size;
		j = tex0.sector + tex0.sector_size;
		j = (j - addr);
		if( i > j ) i = j;
		if( i > page ) i = page;
		j = (addr + page) & (~(page-1));
		j = (j - addr);
		if( i > j ) i = j;
		
		if( qspi_program_sect( pQspi, &tex0, buf, addr, i, &itime, byte ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
		if( qspi_verify( pQspi, buf, addr, i ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
		
		len -= i;
		addr += i;
		buf = &buf[i];
		if( addr >= (tex0.sector + tex0.sector_size) ){
			if( qspi_context_nextsector( &tex0 ) == FALSE ) break;
		}
	};
	
	return( TRUE );
}


/*==============================================================================*/
/* Sector Erase																	*/
/*==============================================================================*/
static INT8 qspi_sector_erase( QSPI_FLASH_STR *pQspi, QSPI_FLASH_BLK_CONTEXT *tex )
{
	UINT32 time,d;
	INT8 ret = TRUE;
	
	/* Write Enable */
	if( qspi_cmd( pQspi, (QCMD_EXECUTE|QCMD_OP(0x06)) ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	IOREG32(pQspi->base,QSPI_FLASHCMDADDR) = tex->sector;
	/* Sector Erase */
	if( qspi_cmd( pQspi, (QCMD_EXECUTE|pQspi->cmdadr|QCMD_OP(tex->erase_code)) ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	time = OSW_TIM_value();
	d = 0;
	do {
		if( (OSW_TIM_value() - time) >= QSPI_FLASH_WRITE_TOUT ){
			DBG_ERR();
			ret = FALSE;
			break;
		}
		if( qspi_busy( pQspi, &ret ) == FALSE ) break;
		OSW_TSK_sleep( 10 );
		
		if( ((d++ & 0x1F) == 0) && pQspi->prm.status_cb_func ){
			(*pQspi->prm.status_cb_func)( QSPI_FLASH_STAT_ERASING, tex->sector, pQspi->prm.cb_arg );
		}
	} while( 1 );
	
	if( (ret == FALSE) && pQspi->prm.status_cb_func ){
		(*pQspi->prm.status_cb_func)( QSPI_FLASH_STAT_ERASE_ERR, tex->sector, pQspi->prm.cb_arg );
	}
	
	DBG_TRACE2( "qspi:sector_erase(%u,0x%08X,%02X)\n", pQspi->prm.port, tex->sector, tex->erase_code );
	
	return( ret );
}


/*==============================================================================*/
/* 範囲指定セクター消去															*/
/*==============================================================================*/
static INT8 qspi_sector_range_erase( QSPI_FLASH_STR *pQspi, UINT32 addr, UINT32 len, INT8 auto_erase )
{
	QSPI_FLASH_BLK_CONTEXT tex0,tex1;
	UINT32 n = pQspi->prm.port;
	
	DBG_TRACE2( "qspi:sector_range_erase(%u)\n", n );
	
	if( len == 0 ) return( TRUE );
	
	if( qspi_context_cfg( &tex0, CFG_QSPI_FLASH_INFO[n], addr ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	if( qspi_context_cfg( &tex1, CFG_QSPI_FLASH_INFO[n], (addr+len-1) ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	if( auto_erase ){
		if( addr > tex0.sector ){
			if( qspi_context_nextsector( &tex0 ) == FALSE ){
				return( TRUE );
			}
			if( tex0.sector > tex1.sector ){
				return( TRUE );
			}
		}
	}
	
	while( 1 ){
		if( qspi_sector_erase( pQspi, &tex0 ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
		if( tex0.sector >= tex1.sector ) break;
		if( qspi_context_nextsector( &tex0 ) == FALSE ) break;
	};
	
	return( TRUE );
}


/*==============================================================================*/
/* Chip Erase																	*/
/*==============================================================================*/
static INT8 qspi_chip_erase( QSPI_FLASH_STR *pQspi )
{
	UINT32 time,d,cmd;
	INT8 ret = TRUE;
	
	/* Write Enable */
	if( qspi_cmd( pQspi, (QCMD_EXECUTE|QCMD_OP(0x06)) ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	DBG_TRACE2( "qspi:chip_erase(%u)\n", pQspi->prm.port );
	
	/* Bulk Erase */
	cmd = QSPI_AT_BULK_ERACE_CODE(CFG_QSPI_ATTR[pQspi->prm.port]);
	if( qspi_cmd( pQspi, (QCMD_EXECUTE|QCMD_OP(cmd)) ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	time = OSW_TIM_value();
	d = 0;
	do {
		if( (OSW_TIM_value() - time) >= QSPI_FLASH_CERASE_TOUT ){
			DBG_ERR();
			ret = FALSE;
			break;
		}
		if( qspi_busy( pQspi, &ret ) == FALSE ) break;
		OSW_TSK_sleep( 10 );
		
		if( ((d++ & 0x1F) == 0) && pQspi->prm.status_cb_func ){
			(*pQspi->prm.status_cb_func)( QSPI_FLASH_STAT_ERASING, 0, pQspi->prm.cb_arg );
		}
	} while( 1 );
	
	if( (ret == FALSE) && pQspi->prm.status_cb_func ){
		(*pQspi->prm.status_cb_func)( QSPI_FLASH_STAT_ERASE_ERR, 0, pQspi->prm.cb_arg );
	}
	
	return( ret );
}


/*==============================================================================*/
/* レジスタ初期化																*/
/*==============================================================================*/
static INT8 qspi_reg_init( QSPI_FLASH_STR *pQspi )
{
	UINT32 n,i,div,cfg;
	
	n = pQspi->prm.port;
	
	if( CFG_QSPI_CLK[n] == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	
	i = CFG_QSPI_SYSCLK[n] / CFG_QSPI_CLK[n];
	if( i > 32 ){
		DBG_ERR();
		return( FALSE );
	}
	
	for( div = 0 ; div < 15 ; div ++ ){
		if( i <= ((div+1)*2) ) break;
	}
	
	DBG_TRACE2( "qspi:ioclk(%u)=%uHz\n", n, (CFG_QSPI_SYSCLK[n]/((div+1)*2)) );
	
	cfg = (div<<19);
	IOREG32(pQspi->base,QSPI_CFG) = cfg;
	IOREG32(pQspi->base,QSPI_DELAY) = (16<<24)|(1<<16)|(1<<8)|1;
	IOREG32(pQspi->base,QSPI_RDDATACAP) = 0x1;
	IOREG32(pQspi->base,QSPI_DEVRD) = 0x3;
	IOREG32(pQspi->base,QSPI_DEVWR) = 0x2;
	IOREG32(pQspi->base,QSPI_CFG) |= 0x1;
	
	if( pQspi->size > 0x1000000 ){
		if( CFG_QSPI_ATTR[n] & QSPI_AT_ENTER_4BYTE_B7h ){
			/* Write Enable */
			if( qspi_cmd( pQspi, (QCMD_EXECUTE|QCMD_OP(0x06)) ) == FALSE ){
				DBG_ERR();
				return( FALSE );
			}
			/* Enter 4 Byte */
			if( qspi_cmd( pQspi, (QCMD_EXECUTE|QCMD_OP(0xB7)) ) == FALSE ){
				DBG_ERR();
				return( FALSE );
			}
		}
		if( CFG_QSPI_ATTR[n] & QSPI_AT_BANK_ADDR_17h ){
			IOREG32(pQspi->base,QSPI_FLASHCMDWRDATALO) = 0x80;
			/* Bank Address Write */
			if( qspi_cmd( pQspi, (QCMD_EXECUTE|QCMD_WR(1)|QCMD_OP(0x17)) ) == FALSE ){
				DBG_ERR();
				return( FALSE );
			}
			pQspi->cmd_mode = 1;
		}
		pQspi->cmdadr = QCMD_ADDR(4);
	}
	else {
		pQspi->cmdadr = QCMD_ADDR(3);
	}
	
	return( TRUE );
}


/*==============================================================================*/
/* 終了設定																		*/
/*==============================================================================*/
static INT8 qspi_reg_end( QSPI_FLASH_STR *pQspi )
{
	UINT32 n = pQspi->prm.port;
	
	if( pQspi->size > 0x1000000 ){
		if( CFG_QSPI_ATTR[n] & QSPI_AT_ENTER_4BYTE_B7h ){
			/* Write Enable */
			if( qspi_cmd( pQspi, (QCMD_EXECUTE|QCMD_OP(0x06)) ) == FALSE ){
				DBG_ERR();
				return( FALSE );
			}
			/* Exit 4 Byte */
			if( qspi_cmd( pQspi, (QCMD_EXECUTE|QCMD_OP(0xE9)) ) == FALSE ){
				DBG_ERR();
				return( FALSE );
			}
		}
		if( CFG_QSPI_ATTR[n] & QSPI_AT_BANK_ADDR_17h ){
			IOREG32(pQspi->base,QSPI_FLASHCMDWRDATALO) = 0x00;
			/* Bank Address Write */
			if( qspi_cmd( pQspi, (QCMD_EXECUTE|QCMD_WR(1)|QCMD_OP(0x17)) ) == FALSE ){
				DBG_ERR();
				return( FALSE );
			}
			pQspi->cmd_mode = 0;
		}
	}
	return( TRUE );
}


/*==============================================================================*/
/* リードAPI																	*/
/*==============================================================================*/
INT8 QSPI_Flash_Read( QSPI_FLASH_HANDLE *handle, QSPI_BUF_INFO *buf )
{
	QSPI_FLASH_STR *pQspi;
	UINT32 i,j;
	INT8 ret = TRUE;
	
	if( (handle == 0) || (handle->hdl == 0) || (buf == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	_E_DEBUG();
	pQspi = (QSPI_FLASH_STR *)handle->hdl;
	
	if( buf->addr >= pQspi->size ){
		/* 領域超え */
		if( buf->byte_count ) *buf->byte_count = 0;
		return( TRUE );
	}
	j = buf->len;
	i = pQspi->size - buf->addr;
	if( j > i ) j = i;
	if( j == 0 ){
		if( buf->byte_count ) *buf->byte_count = 0;
		return( TRUE );
	}
	
	if( OSW_SEM_pend( &pQspi->hSem, QSPI_FLASH_ACCESS_TOUT ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	if( qspi_read( pQspi, buf->buf, buf->addr, j ) == FALSE ){
		ret = FALSE;
		j = 0;
	}
	
	if( buf->byte_count ) *buf->byte_count = j;
	
	OSW_SEM_post( &pQspi->hSem );
	
	return( ret );
}


/*==============================================================================*/
/* ライトAPI																	*/
/*==============================================================================*/
INT8 QSPI_Flash_Write( QSPI_FLASH_HANDLE *handle, QSPI_BUF_INFO *buf, INT8 erase )
{
	QSPI_FLASH_STR *pQspi;
	UINT32 i,j,k = 0;
	INT8 ret = TRUE;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	_E_DEBUG();
	pQspi = (QSPI_FLASH_STR *)handle->hdl;
	
	if( buf->addr >= pQspi->size ){
		/* 領域超え */
		if( buf->byte_count ) *buf->byte_count = 0;
		return( TRUE );
	}
	j = buf->len;
	i = pQspi->size - buf->addr;
	if( j > i ) j = i;
	if( j == 0 ){
		if( buf->byte_count ) *buf->byte_count = 0;
		return( TRUE );
	}
	
	if( OSW_SEM_pend( &pQspi->hSem, QSPI_FLASH_ACCESS_TOUT ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	if( erase == QSPI_FLASH_ERASE_ONLY ){
		/* 消去のみ */
		if( qspi_sector_range_erase( pQspi, buf->addr, j, 0 ) == FALSE ){
			DBG_ERR();
			ret = FALSE;
		}
	}
	else {
		if( erase == QSPI_FLASH_ERASE_AUTO ){
			if( qspi_sector_range_erase( pQspi, buf->addr, j, 1 ) == FALSE ){
				DBG_ERR();
				ret = FALSE;
			}
		}
		if( ret == TRUE ){
			if( qspi_program( pQspi, buf->buf, buf->addr, j, &k ) == FALSE ){
				DBG_ERR();
				ret = FALSE;
			}
		}
	}
	
	if( buf->byte_count ) *buf->byte_count = k;
	
	OSW_SEM_post( &pQspi->hSem );
	DBG_TRACE1( "QSPI_Flash_Write(%u)\n", pQspi->prm.port );
	
	return( ret );
}


/*==============================================================================*/
/* ChipErase API																*/
/*==============================================================================*/
INT8 QSPI_Flash_ChipErase( QSPI_FLASH_HANDLE *handle )
{
	QSPI_FLASH_STR *pQspi;
	INT8 ret;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	_E_DEBUG();
	pQspi = (QSPI_FLASH_STR *)handle->hdl;
	
	if( OSW_SEM_pend( &pQspi->hSem, QSPI_FLASH_ACCESS_TOUT ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	if( (CFG_QSPI_ATTR[pQspi->prm.port] >> 24) == 0 ){
		ret = qspi_sector_range_erase( pQspi, 0, pQspi->size, 0 );
	}
	else {
		ret = qspi_chip_erase( pQspi );
	}
	
	OSW_SEM_post( &pQspi->hSem );
	DBG_TRACE1( "QSPI_Flash_ChipErase(%u)\n", pQspi->prm.port );
	
	return( ret );
}


/*==============================================================================*/
/* ドライバオープンAPI															*/
/*==============================================================================*/
INT8 QSPI_Flash_open( QSPI_FLASH_HANDLE *handle, QSPI_FLASH_PARAM *param, UINT32 *size )
{
	QSPI_FLASH_STR *pQspi;
	UINT32 n,*wp,i;
	
	if( (param == 0) || 
		(handle == 0) ||
		(param->port >= qspi_device_cnt) ||
		(pshQspi[param->port]) ){
		DBG_ERR();
		return( FALSE );
	}
	pQspi = (QSPI_FLASH_STR *)OSW_MEM_alloc( pshMemQspi, sizeof(QSPI_FLASH_STR), 4 );
	if( pQspi == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	memset( (void *)pQspi, 0, sizeof(QSPI_FLASH_STR) );
	handle->hdl = (void *)pQspi;
	
	pQspi->prm = *param;
	n = pQspi->prm.port;
	pQspi->base = qspi_base[n];
	
	if( OSW_SEM_create( &pQspi->hSem, 1 ) == FALSE ){
		DBG_ERR();
		QSPI_Flash_close( handle );
		return( FALSE );
	}
	
	i = 0;
	wp = (UINT32 *)CFG_QSPI_FLASH_INFO[n];
	while( wp[0] ){
		i += (wp[0] * wp[1]);
		wp = &wp[3];
	};
	pQspi->size = i;
	if( size ) *size = i;
	
	if( qspi_reg_init( pQspi ) == FALSE ){
		DBG_ERR();
		QSPI_Flash_close( handle );
		return( FALSE );
	}
	
	pshQspi[n] = pQspi;
	DBG_TRACE1( "QSPI_Flash_open(%u)\n", n );
	
	return( TRUE );
}


/*==============================================================================*/
/* ドライバクローズAPI															*/
/*==============================================================================*/
void QSPI_Flash_close( QSPI_FLASH_HANDLE *handle )
{
	QSPI_FLASH_STR *pQspi;
	UINT32 n;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return;
	}
	
	pQspi = (QSPI_FLASH_STR *)handle->hdl;
	n = pQspi->prm.port;
	qspi_reg_end( pQspi );
	IOREG32(pQspi->base,QSPI_CFG) = 0x0;
	IOREG32(pQspi->base,QSPI_IRQMASK) = 0x0;
	IOREG32(pQspi->base,QSPI_IRQSTAT) = 0xFFFFFFFF;
	pshQspi[n] = 0;
	OSW_SEM_delete( &pQspi->hSem );
	OSW_MEM_free( pshMemQspi, pQspi, sizeof(QSPI_FLASH_STR) );
	handle->hdl = 0;
	
	DBG_TRACE1( "QSPI_Flash_close(%u)\n", n );
}


/*==============================================================================*/
/* 初期化API																	*/
/*==============================================================================*/
INT8 QSPI_Flash_init( OSW_MEM_HANDLE *mem_handle )
{
	UINT32 i;
	
	pshMemQspi = mem_handle;
	memset( (void *)pshQspi, 0, sizeof(pshQspi) );
	
	for( i = 0 ; i < QSPI_FLASH_MAX_PORT ; i ++ ){
		if( CFG_QSPI_FLASH_INFO[i] == NULL ) break;
	}
	
	if( i == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	qspi_device_cnt = i;
	
	DBG_TRACE2( "qspi:device = %u\n", i );
	DBG_TRACE1( "QSPI_Flash_init()\n" );
	
	return( TRUE );
}








