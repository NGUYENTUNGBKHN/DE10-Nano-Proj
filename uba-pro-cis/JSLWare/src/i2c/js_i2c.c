/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: I2C driver															*/
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
#include "js_i2c.h"
#include "js_i2c_reg.h"
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
#if (_FAST_MODE_ENABLE==1) /* jcm mod fast mode対応(マスターのみ) */
/* state */
#define	I2C_STATE_DISABLE			0
#define	I2C_STATE_MASTER_SS			1
#define	I2C_STATE_SLAVE				2
#define	I2C_STATE_SLAVE_RECEIVE		3
#define	I2C_STATE_SLAVE_TRANS		4
#define	I2C_STATE_MASTER_FS			5

/* CON */
#define	MST_CON_SS					0x0063
#define	MST_CON_FS					0x0065
#define	SLV_CON						0x0000
#else
/* state */
#define	I2C_STATE_DISABLE			0
#define	I2C_STATE_MASTER			1
#define	I2C_STATE_SLAVE				2
#define	I2C_STATE_SLAVE_RECEIVE		3
#define	I2C_STATE_SLAVE_TRANS		4

/* CON */
#define	MST_CON						0x0063
#define	SLV_CON						0x0002
#endif

/* MASK */
#define	MASK_MASTER_TRANS			0x250
#define	MASK_MASTER_RECEIVE			0x244
#define	MASK_SLAVE					0x664


/*==============================================================================*/
/* ローカル構造体																*/
/*==============================================================================*/

typedef struct {
	UINT32 base;
	I2C_PARAM prm;
	OSW_ISR_HANDLE hIsr;
	OSW_SEM_HANDLE hMst;
	volatile INT8 state;
	UINT16 r_idx;
	UINT16 s_idx;
	UINT16 s_len;
} I2C_STR;

static I2C_STR *pshI2c[I2C_PORT_CNT];
static OSW_MEM_HANDLE *pshMemI2c = 0;
static volatile UINT32 i2c_dummy_dw;


/*==============================================================================*/
/* ベースアドレステーブル														*/
/*==============================================================================*/
const UINT32 i2c_base[I2C_PORT_CNT] = {
#if (I2C_PORT_CNT >= 1)
	I2C0_BASE
#endif
#if (I2C_PORT_CNT >= 2)
	,I2C1_BASE
#endif
#if (I2C_PORT_CNT >= 3)
	,I2C2_BASE
#endif
#if (I2C_PORT_CNT >= 4)
	,I2C3_BASE
#endif
#if (I2C_PORT_CNT >= 5)
	,I2C4_BASE
#endif
#if (I2C_PORT_CNT >= 6)
	,I2C5_BASE
#endif
#if (I2C_PORT_CNT >= 7)
	,I2C6_BASE
#endif
#if (I2C_PORT_CNT >= 8)
	,I2C7_BASE
#endif
};


/*==============================================================================*/
/* 割り込みIDテーブル															*/
/*==============================================================================*/
static const UINT16 i2c_interrupt_id[I2C_PORT_CNT] = {
#if (I2C_PORT_CNT >= 1)
	OSW_INT_I2C0_IRQ
#endif
#if (I2C_PORT_CNT >= 2)
	,OSW_INT_I2C1_IRQ
#endif
#if (I2C_PORT_CNT >= 3)
	,OSW_INT_I2C2_IRQ
#endif
#if (I2C_PORT_CNT >= 4)
	,OSW_INT_I2C3_IRQ
#endif
#if (I2C_PORT_CNT >= 5)
	,OSW_INT_I2C4_IRQ
#endif
#if (I2C_PORT_CNT >= 6)
	,OSW_INT_I2C5_IRQ
#endif
#if (I2C_PORT_CNT >= 7)
	,OSW_INT_I2C6_IRQ
#endif
#if (I2C_PORT_CNT >= 8)
	,OSW_INT_I2C7_IRQ
#endif
};


/*==============================================================================*/
/* 共用割り込みエントリ															*/
/*==============================================================================*/
static void i2c_isr( UINT8 num )
{
	I2C_STR *pI2c;
	UINT32 i,j,k;
	UINT8 c;
	
	if( pshI2c[num] == 0 ) return;
	pI2c = pshI2c[num];

#if (_FAST_MODE_ENABLE==1) /* jcm mod fast mode対応(マスターのみ) */
	if(( pI2c->state == I2C_STATE_MASTER_SS )
	 ||( pI2c->state == I2C_STATE_MASTER_FS )){
		OSW_SEM_post( &pI2c->hMst );
		OSW_ISR_disable( i2c_interrupt_id[num] );
		return;
	}
#else
	if( pI2c->state < I2C_STATE_SLAVE ){
		if( pI2c->state == I2C_STATE_MASTER ){
			OSW_SEM_post( &pI2c->hMst );
			OSW_ISR_disable( i2c_interrupt_id[num] );
		}
		return;
	}
#endif
	
	for( i = 0 ; i < 20 ; i ++ ){
		j = IOREG32(pI2c->base,I2C_INTR_STAT);
		if( j & (1<<6) ){
			/* ABRT */
			i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_TX_ABRT);
		}
		
		if( j & (1<<10) ){
			if( pI2c->state == I2C_STATE_SLAVE ){
				/* r_start_det */
				pI2c->r_idx = 0;
				pI2c->s_idx = 0;
				pI2c->s_len = 0;
				pI2c->state = I2C_STATE_SLAVE_RECEIVE;
			}
			i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_START_DET);
		}
		if( j & (1<<2) ){
			/* r_rx_full */
			for( k = 0 ; k < pI2c->prm.buf_len ; k ++ ){
				if( (IOREG32(pI2c->base,I2C_STATUS) & (1<<3)) == 0 ) break;
				c = IOREG32(pI2c->base,I2C_DATA_CMD) & 0xFF;
				if( (pI2c->r_idx < pI2c->prm.buf_len) && (pI2c->s_len == 0) ){
					pI2c->prm.buf[pI2c->r_idx++] = c;
				}
			}
		}
		if( j & (1<<5) ){
			/* r_rd_req */
			if( pI2c->state == I2C_STATE_SLAVE_RECEIVE ){
				if( (IOREG32(pI2c->base,I2C_STATUS) & (1<<3)) == 0 ){
					for( k = 0 ; k < pI2c->prm.buf_len ; k ++ ){
						if( (IOREG32(pI2c->base,I2C_STATUS) & (1<<3)) == 0 ) break;
						c = IOREG32(pI2c->base,I2C_DATA_CMD) & 0xFF;
						if( (pI2c->r_idx < pI2c->prm.buf_len) && (pI2c->s_len == 0) ){
							pI2c->prm.buf[pI2c->r_idx++] = c;
						}
					}
				}
				pI2c->s_len = (*pI2c->prm.recv_cb_func)( pI2c->prm.buf, pI2c->r_idx, pI2c->prm.buf_len );
				if( pI2c->s_len > pI2c->prm.buf_len ) pI2c->s_len = pI2c->prm.buf_len;
				pI2c->r_idx = 0;
				pI2c->state = I2C_STATE_SLAVE_TRANS;
			}
			if( (pI2c->s_len == 0) || (pI2c->s_idx >= pI2c->s_len) ){
				c = 0;
			}
			else {
				c = pI2c->prm.buf[pI2c->s_idx++];
			}
			IOREG32(pI2c->base,I2C_DATA_CMD) = c;
			i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_RD_REQ);
		}
		if( j & (1<<9) ){
			/* r_stop_det */
			if( pI2c->r_idx ){
				(*pI2c->prm.recv_cb_func)( pI2c->prm.buf, pI2c->r_idx, 0 );
				pI2c->r_idx = 0;
			}
			pI2c->state = I2C_STATE_SLAVE;
			i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_STOP_DET);
		}
		if( j == 0 ){
			break;
		}
	}
}


#if (I2C_PORT_CNT >= 1)
/*==============================================================================*/
/* ポート0割り込みエントリ														*/
/*==============================================================================*/
void i2c_isr0( void ) { i2c_isr(0); }
#endif
#if (I2C_PORT_CNT >= 2)
/*==============================================================================*/
/* ポート1割り込みエントリ														*/
/*==============================================================================*/
void i2c_isr1( void ) { i2c_isr(1); }
#endif
#if (I2C_PORT_CNT >= 3)
/*==============================================================================*/
/* ポート2割り込みエントリ														*/
/*==============================================================================*/
void i2c_isr2( void ) { i2c_isr(2); }
#endif
#if (I2C_PORT_CNT >= 4)
/*==============================================================================*/
/* ポート3割り込みエントリ														*/
/*==============================================================================*/
void i2c_isr3( void ) { i2c_isr(3); }
#endif
#if (I2C_PORT_CNT >= 5)
/*==============================================================================*/
/* ポート4割り込みエントリ														*/
/*==============================================================================*/
void i2c_isr4( void ) { i2c_isr(4); }
#endif
#if (I2C_PORT_CNT >= 6)
/*==============================================================================*/
/* ポート5割り込みエントリ														*/
/*==============================================================================*/
void i2c_isr5( void ) { i2c_isr(5); }
#endif
#if (I2C_PORT_CNT >= 7)
/*==============================================================================*/
/* ポート6割り込みエントリ														*/
/*==============================================================================*/
void i2c_isr6( void ) { i2c_isr(6); }
#endif
#if (I2C_PORT_CNT >= 8)
/*==============================================================================*/
/* ポート7割り込みエントリ														*/
/*==============================================================================*/
void i2c_isr7( void ) { i2c_isr(7); }
#endif

/*==============================================================================*/
/* 割り込みエントリテーブル														*/
/*==============================================================================*/
static const osw_isr_func i2c_isr_entry[I2C_PORT_CNT] = {
#if (I2C_PORT_CNT >= 1)
	i2c_isr0
#endif
#if (I2C_PORT_CNT >= 2)
	,i2c_isr1
#endif
#if (I2C_PORT_CNT >= 3)
	,i2c_isr2
#endif
#if (I2C_PORT_CNT >= 4)
	,i2c_isr3
#endif
#if (I2C_PORT_CNT >= 5)
	,i2c_isr4
#endif
#if (I2C_PORT_CNT >= 6)
	,i2c_isr5
#endif
#if (I2C_PORT_CNT >= 7)
	,i2c_isr6
#endif
#if (I2C_PORT_CNT >= 8)
	,i2c_isr7
#endif
};


/*==============================================================================*/
/* I2Cモード変更																*/
/*==============================================================================*/
static void i2c_state_change( I2C_STR *pI2c, INT8 state )
{
	UINT32 i,n;
	
	if( pI2c->state == state ) return;
	
	n = pI2c->prm.port;
	OSW_ISR_disable( i2c_interrupt_id[n] );
	
	IOREG32(pI2c->base,I2C_ENABLE) = 0x0;
	for( i = 0 ; i < 1000 ; i ++ ){
		if( (IOREG32(pI2c->base,I2C_ENABLE) & 0x1) == 0 ) break;
	}
	if( i >= 1000 ){
		DBG_ERR();
	}
	
	if( state == I2C_STATE_DISABLE ){
		/* Disable */
		pI2c->state = I2C_STATE_DISABLE;
		IOREG32(pI2c->base,I2C_CON) = 0;
		IOREG32(pI2c->base,I2C_INTR_MASK) = 0;
	}
#if (_FAST_MODE_ENABLE==1) /* jcm mod fast mode対応(マスターのみ) */
	else if( state == I2C_STATE_MASTER_SS ){
		/* Master Standard speed*/
		pI2c->state = I2C_STATE_MASTER_SS;
		IOREG32(pI2c->base,I2C_CON) = MST_CON_SS;
		IOREG32(pI2c->base,I2C_INTR_MASK) = 0;
	}
	else if( state == I2C_STATE_MASTER_FS ){
		/* Master Full speed */
		pI2c->state = I2C_STATE_MASTER_FS;
		IOREG32(pI2c->base,I2C_CON) = MST_CON_FS;
		IOREG32(pI2c->base,I2C_INTR_MASK) = 0;
	}
#else
	else if( state == I2C_STATE_MASTER ){
		/* Master */
		pI2c->state = I2C_STATE_MASTER;
		IOREG32(pI2c->base,I2C_CON) = MST_CON;
		IOREG32(pI2c->base,I2C_INTR_MASK) = 0;
	}
#endif
	else if( state == I2C_STATE_SLAVE ){
		/* Slave */
		pI2c->r_idx = 0;
		pI2c->s_idx = 0;
		pI2c->s_len = 0;
		pI2c->state = I2C_STATE_SLAVE;
		IOREG32(pI2c->base,I2C_CON) = SLV_CON;
		IOREG32(pI2c->base,I2C_INTR_MASK) = MASK_SLAVE;
		OSW_ISR_enable( i2c_interrupt_id[n] );
	}
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_INTR_STAT);
	
	IOREG32(pI2c->base,I2C_ENABLE) = 0x1;
	for( i = 0 ; i < 1000 ; i ++ ){
		if( IOREG32(pI2c->base,I2C_ENABLE) & 0x1 ) break;
	}
	if( i >= 1000 ){
		DBG_ERR();
	}
}


/*==============================================================================*/
/* レジスタ初期化																*/
/*==============================================================================*/
static INT8 i2c_reg_init( I2C_STR *pI2c )
{
	UINT32 n,i,hcnt,lcnt;
	
	n = pI2c->prm.port;
	OSW_ISR_disable( i2c_interrupt_id[n] );
	
	if( (CFG_I2C_SYSCLK[n] == 0) || (CFG_I2C_CLK[n] == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	
	i = (CFG_I2C_SYSCLK[n] / CFG_I2C_CLK[n]);
	hcnt = (i >> 1);
	lcnt = (i >> 1);
	if( i & 0x1 ){
		lcnt++;
	}
	if( ((hcnt+lcnt) * CFG_I2C_CLK[n]) != CFG_I2C_SYSCLK[n] ){
		lcnt++;
	}
	
	if( ((hcnt < 13) || (lcnt < 9)) ){
		DBG_ERR();
		return( FALSE );
	}
	DBG_TRACE2( "i2c:ioclk(%u)=%uHz\n", n, (UINT32)(CFG_I2C_SYSCLK[n] / (hcnt+lcnt)) );
	
	hcnt -= 7;
	lcnt -= 1;
	
	if( ((hcnt > 0xFFFF) || (lcnt > 0xFFFF)) ){
		DBG_ERR();
		return( FALSE );
	}
	
	IOREG32(pI2c->base,I2C_ENABLE) = 0x0;
	for( i = 0 ; i < 1000 ; i ++ ){
		if( (IOREG32(pI2c->base,I2C_ENABLE) & 0x1) == 0 ) break;
	}
	if( i >= 1000 ){
		DBG_ERR();
		return( FALSE );
	}
#if (_FAST_MODE_ENABLE==1) /* jcm mod fast mode対応(マスターのみ) */
	if(CFG_I2C_CLK[n] > 100000)
	{
		pI2c->prm.opt |= I2C_OPT_MASTER_FS;
		IOREG32(pI2c->base,I2C_FS_SPKLEN) = 4;
	}
	IOREG32(pI2c->base,I2C_SS_SCL_HCNT) = hcnt;
	IOREG32(pI2c->base,I2C_SS_SCL_LCNT) = lcnt;
	IOREG32(pI2c->base,I2C_FS_SCL_HCNT) = hcnt;
	IOREG32(pI2c->base,I2C_FS_SCL_LCNT) = lcnt;
	/* 400ns = 40 * 10ns(CFG_I2C_SYSCLK[n]の1 clk時間).  */
	/*  IC:Data hold time の要求(min:100ns) */
	/*  I2C Controller:Data hold time の要求(min:300ns) */
	IOREG32(pI2c->base,I2C_SDA_HOLD) = 40;
#else
	IOREG32(pI2c->base,I2C_SS_SCL_HCNT) = hcnt;
	IOREG32(pI2c->base,I2C_SS_SCL_LCNT) = lcnt;
#endif
	IOREG32(pI2c->base,I2C_SAR) = pI2c->prm.slave_address >> 1;
	
	if( pI2c->prm.opt & I2C_OPT_SLAVE ){
		i2c_state_change( pI2c, I2C_STATE_SLAVE );
	}
	else {
		i2c_state_change( pI2c, I2C_STATE_DISABLE );
	}
	DBG_TRACE2( "i2c_reg_init(%u)\n", n );
	
	return( TRUE );
}


/*==============================================================================*/
/* I2Cバス割込みクリア															*/
/*==============================================================================*/
static void i2c_isr_clr( I2C_STR *pI2c )
{
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_RX_UNDER);
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_RX_OVER);
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_TX_OVER);
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_RD_REQ);
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_TX_ABRT);
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_RX_DONE);
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_ACTIVITY);
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_STOP_DET);
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_START_DET);
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_GEN_CALL);
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_INTR_STAT);
	OSW_SEM_reset( &pI2c->hMst );
}


/*==============================================================================*/
/* I2C ISRポーリング待ち														*/
/*==============================================================================*/
static INT8 i2c_isr_wait( I2C_STR *pI2c, UINT32 *stat )
{
	UINT32 t;
	volatile UINT32 j;
	
	for( t = 0 ; t < I2C_ACCESS_TOUT_CNT ; t ++ ){
		j = IOREG32(pI2c->base,I2C_INTR_STAT);
		if( j ) break;
	}
	if( t >= I2C_ACCESS_TOUT_CNT ){
		DBG_ERR();
		return( FALSE );
	}
	*stat = (UINT32)j;
	
	return( TRUE );
}



/*==============================================================================*/
/* I2Cバス送信																	*/
/*==============================================================================*/
static INT8 i2c_bus_trans( I2C_STR *pI2c, I2C_SEND_PACK *send, INT32 di )
{
	UINT32 i,j,n,con = 0;
	const UINT8 *buf;
	
	n = pI2c->prm.port;
	i = send->write_len;
	buf = send->write_dat;
	
	IOREG32(pI2c->base,I2C_INTR_MASK) = MASK_MASTER_TRANS;
	i2c_isr_clr( pI2c );
	
	if( i == 1 ){
		if( send->read_len ){
			/* Restert */
		}
		else {
			/* Stop Condition */
			con |= (1<<9);
		}
	}
	IOREG32(pI2c->base,I2C_DATA_CMD) = (*buf++) | con;
	i--;
	
	while( i ){
		if( IOREG32(pI2c->base,I2C_STATUS) & (1<<1) ){
			/* Not Full */
			if( i == 1 ){
				if( send->read_len ){
					/* Restert */
				}
				else {
					/* Stop Condition */
					con |= (1<<9);
				}
			}
			IOREG32(pI2c->base,I2C_DATA_CMD) = (*buf++) | con;
			i--;
		}
		else {
			if( di ){
				if( i2c_isr_wait( pI2c, &j ) == FALSE ){
					DBG_ERR();
					return( FALSE );
				}
			}
			else {
				OSW_ISR_enable( i2c_interrupt_id[n] );
				if( OSW_SEM_pend( &pI2c->hMst, I2C_ACCESS_TOUT ) == FALSE ){
					DBG_ERR();
					return( FALSE );
				}
				j = IOREG32(pI2c->base,I2C_INTR_STAT);
			}
			if( j & (1<<6) ){
				/* ABRT */
				DBG_TRACE2( "ABRT(%u) %08X\n", n, IOREG32(pI2c->base,I2C_TX_ABRT_SOURCE) );
				DBG_ERR();
				i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_TX_ABRT);
				return( FALSE );
			}
		}
	};
	
	return( TRUE );
}


/*==============================================================================*/
/* I2Cバス受信																	*/
/*==============================================================================*/
static INT8 i2c_bus_receive( I2C_STR *pI2c, I2C_SEND_PACK *send, INT32 di )
{
	UINT32 n,i,j,con = 0x100;
	UINT8 *buf;
	
	n = pI2c->prm.port;
	i = send->read_len;
	buf = send->read_dat;
	
	IOREG32(pI2c->base,I2C_INTR_MASK) = MASK_MASTER_RECEIVE;
	i2c_isr_clr( pI2c );
	
	if( i == 1 ){
		/* Stop Condition */
		con |= (1<<9);
	}
	IOREG32(pI2c->base,I2C_DATA_CMD) = con;
	
	while( i ){
		if( IOREG32(pI2c->base,I2C_STATUS) & (1<<3) ){
			/* Not Empty */
			*buf++ = IOREG32(pI2c->base,I2C_DATA_CMD) & 0xFF;
			i--;
			if( i ){
				if( i == 1 ){
					/* Stop Condition */
					con |= (1<<9);
				}
				IOREG32(pI2c->base,I2C_DATA_CMD) = con;
			}
		}
		else {
			if( di ){
				if( i2c_isr_wait( pI2c, &j ) == FALSE ){
					DBG_ERR();
					return( FALSE );
				}
			}
			else {
				OSW_ISR_enable( i2c_interrupt_id[n] );
				if( OSW_SEM_pend( &pI2c->hMst, I2C_ACCESS_TOUT ) == FALSE ){
					DBG_ERR();
					return( FALSE );
				}
				j = IOREG32(pI2c->base,I2C_INTR_STAT);
			}
			if( j & (1<<6) ){
				/* ABRT */
				DBG_TRACE2( "ABRT(%u) %08X\n", n, IOREG32(pI2c->base,I2C_TX_ABRT_SOURCE) );
				DBG_ERR();
				i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_TX_ABRT);
				return( FALSE );
			}
		}
	};
	
	return( TRUE );
}


/*==============================================================================*/
/* Master転送																	*/
/*==============================================================================*/
static INT8 i2c_master_send( I2C_STR *pI2c, I2C_SEND_PACK *send, INT32 di )
{
	UINT32 n,j,t = 0;
	
	n = pI2c->prm.port;
#if (_FAST_MODE_ENABLE==1) /* jcm mod fast mode対応(マスターのみ) */
	if( pI2c->prm.opt & I2C_OPT_MASTER_FS )
	{
		i2c_state_change( pI2c, I2C_STATE_MASTER_FS );
	}
	else
	{
		i2c_state_change( pI2c, I2C_STATE_MASTER_SS );
	}
#else
	i2c_state_change( pI2c, I2C_STATE_MASTER );
#endif
	i2c_isr_clr( pI2c );
	
	IOREG32(pI2c->base,I2C_TAR) = send->address >> 1;
	
	if( send->write_len ){
		/* Write */
		if( i2c_bus_trans( pI2c, send, di ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
	}
	
	if( send->read_len ){
		if( i2c_bus_receive( pI2c, send, di ) == FALSE ){
			DBG_ERR();
			return( FALSE );
		}
	}
	
	if( di == 0 ){
		t = OSW_TIM_value();
	}
	while( 1 ){
		if( di ){
			if( i2c_isr_wait( pI2c, &j ) == FALSE ){
				DBG_ERR();
				return( FALSE );
			}
		}
		else {
			if( (OSW_TIM_value()-t) >= I2C_ACCESS_TOUT ){
				return( FALSE );
			}
			OSW_ISR_enable( i2c_interrupt_id[n] );
			if( OSW_SEM_pend( &pI2c->hMst, I2C_ACCESS_TOUT ) == FALSE ){
				DBG_ERR();
				return( FALSE );
			}
			j = IOREG32(pI2c->base,I2C_INTR_STAT);
		}
		if( j & (1<<6) ){
			/* ABRT */
			DBG_TRACE2( "ABRT(%u) %08X\n", n, IOREG32(pI2c->base,I2C_TX_ABRT_SOURCE) );
			DBG_ERR();
			i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_TX_ABRT);
			return( FALSE );
		}
		if( j & (1<<9) ){
			/* STOP */
			i2c_dummy_dw = IOREG32(pI2c->base,I2C_CLR_STOP_DET);
			break;
		}
	};
	
	return( TRUE );
}


/*==============================================================================*/
/* ドライバオープンAPI															*/
/*==============================================================================*/
INT8 I2c_open( I2C_HANDLE *handle, I2C_PARAM *param )
{
	I2C_STR *pI2c;
	UINT32 n;
	
	if( (param == 0) || 
		(param->port >= I2C_PORT_CNT) ||
		(handle == 0) ||
		(pshI2c[param->port]) ){
		DBG_ERR();
		return( FALSE );
	}
	pI2c = (I2C_STR *)OSW_MEM_alloc( pshMemI2c, sizeof(I2C_STR), 4 );
	if( pI2c == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	memset( (void *)pI2c, 0, sizeof(I2C_STR) );
	handle->hdl = (void *)pI2c;
	
	pI2c->prm = *param;
	n = pI2c->prm.port;
	pI2c->base = i2c_base[n];
	
	if( OSW_ISR_create( &pI2c->hIsr, i2c_interrupt_id[n], i2c_isr_entry[n] ) == FALSE ){
		I2c_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	if( OSW_SEM_create( &pI2c->hMst, 0 ) == FALSE ){
		DBG_ERR();
		I2c_close( handle );
		return( FALSE );
	}
	
	if( i2c_reg_init( pI2c ) == FALSE ){
		I2c_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	pshI2c[n] = pI2c;
	
	DBG_TRACE1( "I2c_open(%u)\n", n );
	
	return( TRUE );
}


/*==============================================================================*/
/* ドライバクローズAPI															*/
/*==============================================================================*/
void I2c_close( I2C_HANDLE *handle )
{
	I2C_STR *pI2c;
	UINT32 n;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return;
	}
	pI2c = (I2C_STR *)handle->hdl;
	n = pI2c->prm.port;
	
	OSW_ISR_disable( i2c_interrupt_id[n] );
	IOREG32(pI2c->base,I2C_CON) = 0;
	IOREG32(pI2c->base,I2C_INTR_MASK) = 0;
	i2c_dummy_dw = IOREG32(pI2c->base,I2C_INTR_STAT);
	
	pshI2c[n] = 0;
	OSW_SEM_delete( &pI2c->hMst );
	OSW_ISR_delete( &pI2c->hIsr );
	OSW_MEM_free( pshMemI2c, pI2c, sizeof(I2C_STR) );
	handle->hdl = 0;
	
	DBG_TRACE1( "I2c_close(%u)\n", n );
}


/*==============================================================================*/
/* 送信API																		*/
/*==============================================================================*/
INT8 I2c_send( I2C_HANDLE *handle, I2C_SEND_PACK *send )
{
	I2C_STR *pI2c;
	UINT32 time;
	INT32 di;
	INT8 stat = TRUE;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	_E_DEBUG();
	pI2c = (I2C_STR *)handle->hdl;
	di = OSW_ISR_global_status();
	
	if( send->write_len || send->read_len ){
		if( pI2c->state > I2C_STATE_SLAVE ){
			/* Slave転送中 */
			if( di ){
				DBG_ERR();
				return( FALSE );
			}
			time = OSW_TIM_value();
			while( 1 ){
				if( pI2c->state <= I2C_STATE_SLAVE ){
					break;
				}
				if( (OSW_TIM_value() - time) >= I2C_ACCESS_TOUT ){
					DBG_ERR();
					break;
				}
			};
		}
		
		stat = i2c_master_send( pI2c, send, di );
		if( pI2c->prm.opt & I2C_OPT_SLAVE ){
			i2c_state_change( pI2c, I2C_STATE_SLAVE );
		}
		else {
			i2c_state_change( pI2c, I2C_STATE_DISABLE );
		}
	}
	DBG_TRACE1( "I2c_send(%u)\n", pI2c->prm.port );
	
	return( stat );
}


/*==============================================================================*/
/* 初期化API																	*/
/*==============================================================================*/
INT8 I2c_init( OSW_MEM_HANDLE *mem_handle )
{
	pshMemI2c = mem_handle;
	memset( (void *)pshI2c, 0, sizeof(pshI2c) );
	DBG_TRACE1( "I2c_init()\n" );
	
	return( TRUE );
}






