/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: SPI driver															*/
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
#include "js_spi.h"
#include "js_spi_reg.h"
#include "gpio/js_gpio.h"
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


/*==============================================================================*/
/* ローカル構造体																*/
/*==============================================================================*/

typedef struct {
	UINT16 slen;
	UINT16 rlen;
	UINT16 cn;
	UINT16 tx_pend;
	UINT16 wait;
	UINT8 *tbl;
	UINT32 tx_rem;
	UINT32 rx_rem;
	SPIM_SEND send;
} SPI_MST_ISR;

typedef struct {
	UINT32 base;
	SPI_PARAM prm;
	OSW_ISR_HANDLE hIsr;
	OSW_SEM_HANDLE hMst;
	SPI_MST_ISR mst;
} SPI_STR;

static SPI_STR *pshSpi[SPI_PORT_CNT];
static OSW_MEM_HANDLE *pshMemSpi = 0;
static volatile UINT32 spi_dummy_dw;


/*==============================================================================*/
/* ベースアドレステーブル														*/
/*==============================================================================*/
const UINT32 spi_base[SPI_PORT_CNT] = {
#if (SPI_PORT_CNT >= 1)
	SPI0_BASE
#endif
#if (SPI_PORT_CNT >= 2)
	,SPI1_BASE
#endif
#if (SPI_PORT_CNT >= 3)
	,SPI2_BASE
#endif
#if (SPI_PORT_CNT >= 4)
	,SPI3_BASE
#endif
#if (SPI_PORT_CNT >= 5)
	,SPI4_BASE
#endif
#if (SPI_PORT_CNT >= 6)
	,SPI5_BASE
#endif
#if (SPI_PORT_CNT >= 7)
	,SPI6_BASE
#endif
#if (SPI_PORT_CNT >= 8)
	,SPI7_BASE
#endif
};


/*==============================================================================*/
/* 割り込みIDテーブル															*/
/*==============================================================================*/
static const UINT16 spi_interrupt_id[SPI_PORT_CNT] = {
#if (SPI_PORT_CNT >= 1)
	OSW_INT_SPI0_IRQ
#endif
#if (SPI_PORT_CNT >= 2)
	,OSW_INT_SPI1_IRQ
#endif
#if (SPI_PORT_CNT >= 3)
	,OSW_INT_SPI2_IRQ
#endif
#if (SPI_PORT_CNT >= 4)
	,OSW_INT_SPI3_IRQ
#endif
#if (SPI_PORT_CNT >= 5)
	,OSW_INT_SPI4_IRQ
#endif
#if (SPI_PORT_CNT >= 6)
	,OSW_INT_SPI5_IRQ
#endif
#if (SPI_PORT_CNT >= 7)
	,OSW_INT_SPI6_IRQ
#endif
#if (SPI_PORT_CNT >= 8)
	,OSW_INT_SPI7_IRQ
#endif
};


/*==============================================================================*/
/* 共用割り込みエントリ															*/
/*==============================================================================*/
static void spi_isr( UINT8 num )
{
	SPI_STR *pSpi;
	SPI_MST_ISR *mst;
	UINT32 stat,w,i;
	UINT8 d;
	
	if( pshSpi[num] == 0 ) return;
	pSpi = pshSpi[num];
	mst = &pSpi->mst;
	
	for( i = 0 ; i < 8 ; i ++ ){
		stat = IOREG32(pSpi->base,SPI_ISR);
		if( (mst->rlen < mst->send.len) || mst->wait ){
			if( stat & 0x10 ){
				/* Receive full */
				d = (UINT8)IOREG32(pSpi->base,SPI_DR);
				if( mst->wait ){
					if( mst->cn ){
						mst->cn--;
					}
					if( d == mst->send.cond ) mst->wait = 0;
				}
				else {
					if( mst->send.recv ){
						mst->send.recv[(mst->rlen&(~0x3))+mst->tbl[(mst->rlen&0x3)]] = d;
					}
					mst->rlen++;
				}
			}
		}
		if( (mst->slen < mst->send.len) || mst->wait ){
			if( (stat & 0x1) || mst->tx_pend ){
				/* Transmit empty */
				w = 0;
				mst->tx_pend = 0;
				if( mst->wait ){
					if( mst->cn == 0 ){
						IOREG32(pSpi->base,SPI_DR) = w;
						mst->cn++;
					}
					else {
						mst->tx_pend = 1;
					}
				}
				else {
					if( mst->send.send ){
						w = mst->send.send[(mst->slen&(~0x3))+mst->tbl[(mst->slen&0x3)]];
					}
					mst->slen++;
					IOREG32(pSpi->base,SPI_DR) = w;
				}
			}
		}
		if( (mst->slen >= mst->send.len) && (mst->rlen >= mst->send.len) && (mst->wait == 0) ){
			DBG_TRACE2( "transfer end\n" );
			OSW_ISR_disable( spi_interrupt_id[num] );
			OSW_SEM_post( &pSpi->hMst );
			return;
		}
	}
}


#if (SPI_PORT_CNT >= 1)
/*==============================================================================*/
/* ポート0割り込みエントリ														*/
/*==============================================================================*/
void spi_isr0( void ) { spi_isr(0); }
#endif
#if (SPI_PORT_CNT >= 2)
/*==============================================================================*/
/* ポート1割り込みエントリ														*/
/*==============================================================================*/
void spi_isr1( void ) { spi_isr(1); }
#endif
#if (SPI_PORT_CNT >= 3)
/*==============================================================================*/
/* ポート2割り込みエントリ														*/
/*==============================================================================*/
void spi_isr2( void ) { spi_isr(2); }
#endif
#if (SPI_PORT_CNT >= 4)
/*==============================================================================*/
/* ポート3割り込みエントリ														*/
/*==============================================================================*/
void spi_isr3( void ) { spi_isr(3); }
#endif
#if (SPI_PORT_CNT >= 5)
/*==============================================================================*/
/* ポート4割り込みエントリ														*/
/*==============================================================================*/
void spi_isr4( void ) { spi_isr(4); }
#endif
#if (SPI_PORT_CNT >= 6)
/*==============================================================================*/
/* ポート4割り込みエントリ														*/
/*==============================================================================*/
void spi_isr5( void ) { spi_isr(5); }
#endif
#if (SPI_PORT_CNT >= 7)
/*==============================================================================*/
/* ポート4割り込みエントリ														*/
/*==============================================================================*/
void spi_isr6( void ) { spi_isr(6); }
#endif
#if (SPI_PORT_CNT >= 8)
/*==============================================================================*/
/* ポート4割り込みエントリ														*/
/*==============================================================================*/
void spi_isr7( void ) { spi_isr(7); }
#endif

/*==============================================================================*/
/* 割り込みエントリテーブル														*/
/*==============================================================================*/
static const osw_isr_func spi_isr_entry[SPI_PORT_CNT] = {
#if (SPI_PORT_CNT >= 1)
	spi_isr0
#endif
#if (SPI_PORT_CNT >= 2)
	,spi_isr1
#endif
#if (SPI_PORT_CNT >= 3)
	,spi_isr2
#endif
#if (SPI_PORT_CNT >= 4)
	,spi_isr3
#endif
#if (SPI_PORT_CNT >= 5)
	,spi_isr4
#endif
#if (SPI_PORT_CNT >= 6)
	,spi_isr5
#endif
#if (SPI_PORT_CNT >= 7)
	,spi_isr6
#endif
#if (SPI_PORT_CNT >= 8)
	,spi_isr7
#endif
};


/*==============================================================================*/
/* ローカル定数																	*/
/*==============================================================================*/
static const UINT8 spi_cnv8_tbl[4] = {
	0,1,2,3
};

static const UINT8 spi_cnv16_tbl[4] = {
	1,0,3,2
};

static const UINT8 spi_cnv32_tbl[4] = {
	3,2,1,0
};

static const UINT8 *spi_cnv_tbl[4] = {
	spi_cnv8_tbl,
	spi_cnv16_tbl,
	spi_cnv32_tbl,
	spi_cnv8_tbl
};


/*==============================================================================*/
/* ISRポーリング待ち															*/
/*==============================================================================*/
static INT8 spi_isr_wait( SPI_STR *pSpi, UINT32 *stat )
{
	UINT32 t;
	volatile UINT32 j;
	
	for( t = 0 ; t < SPIM_ACCESS_TOUT_CNT ; t ++ ){
		j = IOREG32(pSpi->base,SPI_RISR) & IOREG32(pSpi->base,SPI_IMR);
		if( j ) break;
	}
	if( t >= SPIM_ACCESS_TOUT_CNT ){
		DBG_ERR();
		return( FALSE );
	}
	*stat = (UINT32)j;
	
	return( TRUE );
}


/*==============================================================================*/
/* STBアサート																	*/
/*==============================================================================*/
static void spi_stb_assert( SPI_STR *pSpi, UINT8 ch )
{
	UINT32 n = pSpi->prm.port;
	const UINT16 *p;
	
	p = CFG_SPIM_STBGPIOID[n];
	if( CFG_SPIM_FMT[n] & SPIM_FMT_STB_H_ACTIVE ){
		Gpio_out( p[ch], 1 );
	}
	else {
		Gpio_out( p[ch], 0 );
	}
}


/*==============================================================================*/
/* STBネゲート																	*/
/*==============================================================================*/
static void spi_stb_negate( SPI_STR *pSpi, UINT8 ch )
{
	UINT32 n = pSpi->prm.port;
	const UINT16 *p;
	
	p = CFG_SPIM_STBGPIOID[n];
	if( CFG_SPIM_FMT[n] & SPIM_FMT_STB_H_ACTIVE ){
		Gpio_out( p[ch], 0 );
	}
	else {
		Gpio_out( p[ch], 1 );
	}
}


/*==============================================================================*/
/* レジスタ初期化																*/
/*==============================================================================*/
static INT8 spi_reg_init( SPI_STR *pSpi )
{
	UINT32 sckdv,n;
	
	n = pSpi->prm.port;
	OSW_ISR_disable( spi_interrupt_id[n] );
	
	if( CFG_SPIM_CLK[n] == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	
	sckdv = CFG_SPIM_SYSCLK[n] / CFG_SPIM_CLK[n];
	if( (sckdv * CFG_SPIM_CLK[n]) != CFG_SPIM_SYSCLK[n] ){
		sckdv++;
	}
	if( sckdv == 0 ) sckdv = 2;
	if( sckdv & 0x1 ) sckdv++;
	if( sckdv > 0xFFFF ){
		DBG_ERR();
		return( FALSE );
	}
	
	DBG_TRACE2( "spi:ioclk(%u)=%uHz\n", n, (UINT32)(CFG_SPIM_SYSCLK[n] / sckdv) );
	
	IOREG32(pSpi->base,SPI_SPIENR) = 0x0;
	
	IOREG32(pSpi->base,SPI_CTRLR0) = 0x17;
	IOREG32(pSpi->base,SPI_CTRLR1) = 0x0;
	IOREG32(pSpi->base,SPI_BAUDR) = sckdv;
	IOREG32(pSpi->base,SPI_TXFTLR) = 0x0;
	IOREG32(pSpi->base,SPI_RXFTLR) = 0x0;
	IOREG32(pSpi->base,SPI_IMR) = 0x11;
	IOREG32(pSpi->base,SPI_SER) = 0x1;
	
	IOREG32(pSpi->base,SPI_SPIENR) = 0x1;
	
	spi_dummy_dw = IOREG32(pSpi->base,SPI_ISR);
	
	return( TRUE );
}


/*==============================================================================*/
/* ドライバオープンAPI															*/
/*==============================================================================*/
INT8 Spi_open( SPI_HANDLE *handle, SPI_PARAM *param )
{
	SPI_STR *pSpi;
	UINT32 n;
	
	if( (param == 0) || 
		(param->port >= SPI_PORT_CNT) ||
		(handle == 0) ||
		(pshSpi[param->port]) ){
		DBG_ERR();
		return( FALSE );
	}
	
	if( param->mode != SPI_MODE_MASTER ){
		DBG_ERR();
		return( FALSE );
	}
	
	pSpi = (SPI_STR *)OSW_MEM_alloc( pshMemSpi, sizeof(SPI_STR), 4 );
	if( pSpi == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	memset( (void *)pSpi, 0, sizeof(SPI_STR) );
	handle->hdl = (void *)pSpi;
	
	pSpi->prm = *param;
	n = pSpi->prm.port;
	pSpi->base = spi_base[n];
	
	if( OSW_ISR_create( &pSpi->hIsr, spi_interrupt_id[n], spi_isr_entry[n] ) == FALSE ){
		DBG_ERR();
		Spi_close( handle );
		return( FALSE );
	}
	
	if( OSW_SEM_create( &pSpi->hMst, 0 ) == FALSE ){
		DBG_ERR();
		Spi_close( handle );
		return( FALSE );
	}
	
	if( spi_reg_init( pSpi ) == FALSE ){
		DBG_ERR();
		Spi_close( handle );
		return( FALSE );
	}
	
	pshSpi[n] = pSpi;
	
	DBG_TRACE1( "Spi_open(%u)\n", n );
	
	return( TRUE );
}


/*==============================================================================*/
/* ドライバクローズAPI															*/
/*==============================================================================*/
void Spi_close( SPI_HANDLE *handle )
{
	SPI_STR *pSpi;
	UINT32 n;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return;
	}
	pSpi = (SPI_STR *)handle->hdl;
	n = pSpi->prm.port;
	
	OSW_ISR_disable( spi_interrupt_id[n] );
	IOREG32(pSpi->base,SPI_SPIENR) = 0x0;
	pshSpi[n] = 0;
	OSW_SEM_delete( &pSpi->hMst );
	OSW_ISR_delete( &pSpi->hIsr );
	OSW_MEM_free( pshMemSpi, pSpi, sizeof(SPI_STR) );
	handle->hdl = 0;
	
	DBG_TRACE1( "Spi_close(%u)\n", n );
}


/*==============================================================================*/
/* 送受信API																	*/
/*==============================================================================*/
INT8 Spi_send( SPI_HANDLE *handle, UINT8 ch, SPIM_SEND *send )
{
	SPI_STR *pSpi;
	UINT32 n,stat;
	INT32 di;
	INT8 ret = TRUE;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	_E_DEBUG();
	pSpi = (SPI_STR *)handle->hdl;
	di = OSW_ISR_global_status();
	n = pSpi->prm.port;
	if( pSpi->prm.mode != SPI_MODE_MASTER ){
		DBG_ERR();
		return( FALSE );
	}
	
	memset( (void *)&pSpi->mst, 0, sizeof(SPI_MST_ISR) );
	spi_dummy_dw = IOREG32(pSpi->base,SPI_ISR);
	OSW_SEM_reset( &pSpi->hMst );
	
	if( send->len ){
		spi_stb_assert( pSpi, ch );
		pSpi->mst.wait = send->opt & SPIM_OPT_COND;
		pSpi->mst.tbl = (UINT8 *)spi_cnv_tbl[(send->opt>>4)&0x3];
		pSpi->mst.send = *send;
		if( di ){
			while( 1 ){
				if( spi_isr_wait( pSpi, &stat ) == TRUE ){
					spi_isr( (UINT8)n );
					if( OSW_SEM_pend( &pSpi->hMst, 0 ) == TRUE ) break;
				}
				else {
					DBG_ERR();
					ret = FALSE;
					break;
				}
			};
		}
		else {
			OSW_ISR_enable( spi_interrupt_id[n] );
			if( OSW_SEM_pend( &pSpi->hMst, SPIM_ACCESS_TOUT ) == FALSE ){
				DBG_ERR();
				ret = FALSE;
			}
			OSW_ISR_disable( spi_interrupt_id[n] );
		}
	}
	if( send->opt & SPIM_OPT_PACK_END ){
		spi_stb_negate( pSpi, ch );
	}
	
	return( ret );
}


/*==============================================================================*/
/* 初期化API																	*/
/*==============================================================================*/
INT8 Spi_init( OSW_MEM_HANDLE *mem_handle )
{
	pshMemSpi = mem_handle;
	memset( (void *)pshSpi, 0, sizeof(pshSpi) );
	
	DBG_TRACE1( "Spi_init()\n" );
	
	return( TRUE );
}






