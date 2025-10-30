/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: UART driver															*/
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
#include "js_uart.h"
#include "js_uart_reg.h"


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
#define	UART_DLB_ENABLE			0		/* Loop Back Mode */


/*==============================================================================*/
/* ローカル構造体																*/
/*==============================================================================*/

typedef struct {
	UINT32 base;
	UART_PARAM prm;
	OSW_ISR_HANDLE hIsr;
	UINT32 ier;
} UART_STR;

static UART_STR *pshUart[UART_PORT_CNT];
static OSW_MEM_HANDLE *pshMemUart = 0;


/*==============================================================================*/
/* ベースアドレステーブル														*/
/*==============================================================================*/
const UINT32 uart_base[UART_PORT_CNT] = {
#if (UART_PORT_CNT >= 1)
	UART0_BASE
#endif
#if (UART_PORT_CNT >= 2)
	,UART1_BASE
#endif
#if (UART_PORT_CNT >= 3)
	,UART2_BASE
#endif
#if (UART_PORT_CNT >= 4)
	,UART3_BASE
#endif
#if (UART_PORT_CNT >= 5)
	,UART4_BASE
#endif
#if (UART_PORT_CNT >= 6)
	,UART5_BASE
#endif
#if (UART_PORT_CNT >= 7)
	,UART6_BASE
#endif
#if (UART_PORT_CNT >= 8)
	,UART7_BASE
#endif
};


/*==============================================================================*/
/* 割り込みIDテーブル															*/
/*==============================================================================*/
static const UINT16 uart_interrupt_id[UART_PORT_CNT] = {
#if (UART_PORT_CNT >= 1)
	OSW_INT_UART0_IRQ
#endif
#if (UART_PORT_CNT >= 2)
	,OSW_INT_UART1_IRQ
#endif
#if (UART_PORT_CNT >= 3)
	,OSW_INT_UART2_IRQ
#endif
#if (UART_PORT_CNT >= 4)
	,OSW_INT_UART3_IRQ
#endif
#if (UART_PORT_CNT >= 5)
	,OSW_INT_UART4_IRQ
#endif
#if (UART_PORT_CNT >= 6)
	,OSW_INT_UART5_IRQ
#endif
#if (UART_PORT_CNT >= 7)
	,OSW_INT_UART6_IRQ
#endif
#if (UART_PORT_CNT >= 8)
	,OSW_INT_UART7_IRQ
#endif
};


/*==============================================================================*/
/* 共用割り込みエントリ															*/
/*==============================================================================*/
static void uart_isr( UINT8 num )
{
	UART_STR *pUart;
	UINT32 i,j;
	UINT8 c;
	
	if( pshUart[num] == 0 ) return;
	pUart = pshUart[num];
	
#if 1 /* JCM mod 2021-04-16:only notice interrupt */
	i = IOREG32(pUart->base,UART_IIR) & 0xCF;

	if( (i & 0x04) == 0x04)
	{
		j = IOREG32(pUart->base,UART_RFL);
		if(j > 0)
		{
			if( (IOREG32(pUart->base,UART_LSR) & 0x01) == 0x01 )
			{
				c = IOREG32(pUart->base,UART_RBR_THR_DLL);
				(*pUart->prm.recv_cb_func)( c, UART_STAT_RECV, pUart->prm.cb_arg );
			}
		}
	}
	else if( (i & 0x02) == 0x02)
	{
		(*pUart->prm.recv_cb_func)( c, UART_STAT_TRANSRDY, pUart->prm.cb_arg );
		if( (IOREG32(pUart->base,UART_LSR) & 0x60) == 0x60 ){
			IOREG32(pUart->base,UART_IER_DLH) = pUart->ier & (~0x2);
		}
	}
#else
	for( j = 0 ; j < 100 ; j ++ ){
		i = IOREG32(pUart->base,UART_IIR) & 0x3F;
		c = 0;
		
		if( i & 0x1 ) break;
		if( (i == (0x2<<1)) || (i == (0x6<<1)) ){
			c = IOREG32(pUart->base,UART_RBR_THR_DLL);
			(*pUart->prm.recv_cb_func)( c, UART_STAT_RECV, pUart->prm.cb_arg );
		}
		else if( i == (0x1<<1) ){
			(*pUart->prm.recv_cb_func)( c, UART_STAT_TRANSRDY, pUart->prm.cb_arg );
			if( (IOREG32(pUart->base,UART_LSR) & 0x60) == 0x60 ){
				IOREG32(pUart->base,UART_IER_DLH) = pUart->ier & (~0x2);
			}
		}
	}
#endif
}


#if (UART_PORT_CNT >= 1)
/*==============================================================================*/
/* ポート0割り込みエントリ														*/
/*==============================================================================*/
void uart_isr0( void ) { uart_isr(0); }
#endif
#if (UART_PORT_CNT >= 2)
/*==============================================================================*/
/* ポート1割り込みエントリ														*/
/*==============================================================================*/
void uart_isr1( void ) { uart_isr(1); }
#endif
#if (UART_PORT_CNT >= 3)
/*==============================================================================*/
/* ポート2割り込みエントリ														*/
/*==============================================================================*/
void uart_isr2( void ) { uart_isr(2); }
#endif
#if (UART_PORT_CNT >= 4)
/*==============================================================================*/
/* ポート3割り込みエントリ														*/
/*==============================================================================*/
void uart_isr3( void ) { uart_isr(3); }
#endif
#if (UART_PORT_CNT >= 5)
/*==============================================================================*/
/* ポート4割り込みエントリ														*/
/*==============================================================================*/
void uart_isr4( void ) { uart_isr(4); }
#endif
#if (UART_PORT_CNT >= 6)
/*==============================================================================*/
/* ポート5割り込みエントリ														*/
/*==============================================================================*/
void uart_isr5( void ) { uart_isr(5); }
#endif
#if (UART_PORT_CNT >= 7)
/*==============================================================================*/
/* ポート6割り込みエントリ														*/
/*==============================================================================*/
void uart_isr6( void ) { uart_isr(6); }
#endif
#if (UART_PORT_CNT >= 8)
/*==============================================================================*/
/* ポート7割り込みエントリ														*/
/*==============================================================================*/
void uart_isr7( void ) { uart_isr(7); }
#endif

/*==============================================================================*/
/* 割り込みエントリテーブル														*/
/*==============================================================================*/
static const osw_isr_func uart_isr_entry[UART_PORT_CNT] = {
#if (UART_PORT_CNT >= 1)
	uart_isr0
#endif
#if (UART_PORT_CNT >= 2)
	,uart_isr1
#endif
#if (UART_PORT_CNT >= 3)
	,uart_isr2
#endif
#if (UART_PORT_CNT >= 4)
	,uart_isr3
#endif
#if (UART_PORT_CNT >= 5)
	,uart_isr4
#endif
#if (UART_PORT_CNT >= 6)
	,uart_isr5
#endif
#if (UART_PORT_CNT >= 7)
	,uart_isr6
#endif
#if (UART_PORT_CNT >= 8)
	,uart_isr7
#endif
};


/*==============================================================================*/
/* レジスタ初期化																*/
/*==============================================================================*/
static INT8 uart_reg_init( UART_STR *pUart )
{
	UINT16 n,ier;
	UINT32 i0;
	FLOAT32 f0,d0;
	
	n = pUart->prm.port;
	OSW_ISR_disable( uart_interrupt_id[n] );
	
	if( CFG_UART_BPS[n] == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	
	f0 = (FLOAT32)CFG_UART_SYSCLK[n] / ((FLOAT32)CFG_UART_BPS[n] * 16.0f);
	ier = 0;
	i0 = (UINT32)(f0 + 0.5f);
	d0 = (FLOAT32)i0 - f0;
	if( d0 < 0.0f ) d0 *= -1.0f;
	
	DBG_TRACE2( "uart(%u):bps=%u(%f%%)\n", n, ((CFG_UART_SYSCLK[n]/i0)/16), (((FLOAT32)((CFG_UART_SYSCLK[n]/i0)/16)/(FLOAT32)CFG_UART_BPS[n])*100.0f)-100.0f );
	
	IOREG32(pUart->base,UART_LCR) = 0x80;
	IOREG32(pUart->base,UART_RBR_THR_DLL) = i0 & 0xFF;
	IOREG32(pUart->base,UART_IER_DLH) = (i0 >> 8) & 0xFF;
	IOREG32(pUart->base,UART_FCR) = 0x07;
	IOREG32(pUart->base,UART_LCR) = ((CFG_UART_FMT[n] << 2) & 0x18) | ((CFG_UART_FMT[n] >> 1) & 0x4) | ((CFG_UART_FMT[n] >> 5) & 0x3);
	IOREG32(pUart->base,UART_MCR) = 0;
#if UART_DLB_ENABLE
	IOREG32(pUart->base,UART_MCR) |= (1<<4);
#endif
	
	if( CFG_UART_FMT[n] & UART_FMT_RXEN ){
		ier |= 0x1;
	}
	if( CFG_UART_FMT[n] & UART_FMT_TXEN ){
		ier |= 0x2;
	}
	pUart->ier = ier;
	
	IOREG32(pUart->base,UART_IER_DLH) = pUart->ier;
	
	return( TRUE );
}


/*==============================================================================*/
/* ドライバオープンAPI															*/
/*==============================================================================*/
INT8 Uart_open( UART_HANDLE *handle, UART_PARAM *param )
{
	UART_STR *pUart;
	UINT32 n;
	
	if( (param == 0) || 
		(param->port >= UART_PORT_CNT) ||
		(handle == 0) ||
		(pshUart[param->port]) ){
		DBG_ERR();
		return( FALSE );
	}
	pUart = (UART_STR *)OSW_MEM_alloc( pshMemUart, sizeof(UART_STR), 4 );
	if( pUart == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	memset( (void *)pUart, 0, sizeof(UART_STR) );
	handle->hdl = (void *)pUart;
	
	pUart->prm = *param;
	n = pUart->prm.port;
	pUart->base = uart_base[n];
	
	if( OSW_ISR_create( &pUart->hIsr, uart_interrupt_id[n], uart_isr_entry[n] ) == FALSE ){
		Uart_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	if( uart_reg_init( pUart ) == FALSE ){
		Uart_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	pshUart[n] = pUart;
	OSW_ISR_enable( uart_interrupt_id[n] );
	
	DBG_TRACE1( "Uart_open(%u)\n", n );
	
	return( TRUE );
}


/*==============================================================================*/
/* ドライバクローズAPI															*/
/*==============================================================================*/
void Uart_close( UART_HANDLE *handle )
{
	UART_STR *pUart;
	UINT32 n;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return;
	}
	pUart = (UART_STR *)handle->hdl;
	n = pUart->prm.port;
	
	OSW_ISR_disable( uart_interrupt_id[n] );
	IOREG32(pUart->base,UART_IER_DLH) = 0;
	pshUart[n] = 0;
	OSW_ISR_delete( &pUart->hIsr );
	OSW_MEM_free( pshMemUart, pUart, sizeof(UART_STR) );
	handle->hdl = 0;
	
	DBG_TRACE1( "Uart_close(%u)\n", n );
}


/*==============================================================================*/
/* 送信API																		*/
/*==============================================================================*/
INT8 Uart_send( UART_HANDLE *handle, UINT8 c, INT8 wait )
{
	UART_STR *pUart;
	UINT32 t,n;
	INT32 di;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	pUart = (UART_STR *)handle->hdl;
	di = OSW_ISR_global_status();
	n = pUart->prm.port;
	
	if( (CFG_UART_FMT[n] & UART_FMT_TXEN) == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	
	if( wait ){
		if( di ){
			for( t = 0 ; t < UART_ACCESS_TOUT_CNT ; t ++ ){
				if( IOREG32(pUart->base,UART_USR) & 0x2 ){
					IOREG32(pUart->base,UART_RBR_THR_DLL) = c;
					IOREG32(pUart->base,UART_IER_DLH) = pUart->ier;
					return( TRUE );
				}
			}
		}
		else {
			t = OSW_TIM_value();
			do {
				if( IOREG32(pUart->base,UART_USR) & 0x2 ){
					IOREG32(pUart->base,UART_RBR_THR_DLL) = c;
					IOREG32(pUart->base,UART_IER_DLH) = pUart->ier;
					return( TRUE );
				}
			} while( (OSW_TIM_value() - t) < UART_ACCESS_TOUT );
		}
		DBG_ERR();
	}
	else {
		if( IOREG32(pUart->base,UART_USR) & 0x2 ){
			IOREG32(pUart->base,UART_RBR_THR_DLL) = c;
			IOREG32(pUart->base,UART_IER_DLH) = pUart->ier;
			return( TRUE );
		}
	}
	return( FALSE );
}


/*==============================================================================*/
/* 初期化API																	*/
/*==============================================================================*/
INT8 Uart_init( OSW_MEM_HANDLE *mem_handle )
{
	pshMemUart = mem_handle;
	memset( (void *)pshUart, 0, sizeof(pshUart) );
	DBG_TRACE1( "Uart_init()\n" );
	
	return( TRUE );
}

/*==============================================================================*/
/* 受信API																		*/
/*==============================================================================*/
/* JCM ADD */
INT8 Uart_recv( UART_HANDLE *handle, UINT8 *buf, UINT8 size )
{
	UART_STR *pUart;
	UINT32 lsr;
	UINT8 len;
	UINT8 cnt;
	UINT8 c;

	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	pUart = (UART_STR *)handle->hdl;

	len = IOREG32(pUart->base,UART_RFL);
	if(len == 0)
	{
		return 0;
	}

	cnt = 0;
	lsr = IOREG32(pUart->base,UART_LSR);

	while(lsr && 1)
	{
		if(cnt > size) break;

		c = IOREG32(pUart->base,UART_RBR_THR_DLL);
		buf[cnt++] = c;
		lsr = IOREG32(pUart->base,UART_LSR);
	}
	return cnt;
}





