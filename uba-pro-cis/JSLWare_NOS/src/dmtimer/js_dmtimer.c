/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: Timer driver															*/
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
#include "js_dmtimer.h"
#include "js_dmtimer_reg.h"
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
	UINT32 base;
	TIM_INIT_PARAM prm;
	TIM_PARAM tprm;
	OSW_ISR_HANDLE hIsr;
} TIM_STR;

static TIM_STR *pshTim[DTIM_PORT_CNT];
static OSW_MEM_HANDLE *pshMemTim = 0;
static volatile UINT32 tim_dummy_dw;


/*==============================================================================*/
/* ベースアドレステーブル														*/
/*==============================================================================*/
const UINT32 tim_base[DTIM_PORT_CNT] = {
#if (DTIM_PORT_CNT >= 1)
	DTIM0_BASE
#endif
#if (DTIM_PORT_CNT >= 2)
	,DTIM1_BASE
#endif
#if (DTIM_PORT_CNT >= 3)
	,DTIM2_BASE
#endif
#if (DTIM_PORT_CNT >= 4)
	,DTIM3_BASE
#endif
#if (DTIM_PORT_CNT >= 5)
	,DTIM4_BASE
#endif
#if (DTIM_PORT_CNT >= 6)
	,DTIM5_BASE
#endif
#if (DTIM_PORT_CNT >= 7)
	,DTIM6_BASE
#endif
#if (DTIM_PORT_CNT >= 8)
	,DTIM7_BASE
#endif
#if (DTIM_PORT_CNT >= 9)
	,DTIM8_BASE
#endif
#if (DTIM_PORT_CNT >= 10)
	,DTIM9_BASE
#endif
#if (DTIM_PORT_CNT >= 11)
	,DTIM10_BASE
#endif
#if (DTIM_PORT_CNT >= 12)
	,DTIM11_BASE
#endif
#if (DTIM_PORT_CNT >= 13)
	,DTIM12_BASE
#endif
#if (DTIM_PORT_CNT >= 14)
	,DTIM13_BASE
#endif
#if (DTIM_PORT_CNT >= 15)
	,DTIM14_BASE
#endif
#if (DTIM_PORT_CNT >= 16)
	,DTIM15_BASE
#endif
};


/*==============================================================================*/
/* 割り込みIDテーブル															*/
/*==============================================================================*/
static const UINT16 tim_interrupt_id[DTIM_PORT_CNT] = {
#if (DTIM_PORT_CNT >= 1)
	OSW_INT_TIM0_IRQ
#endif
#if (DTIM_PORT_CNT >= 2)
	,OSW_INT_TIM1_IRQ
#endif
#if (DTIM_PORT_CNT >= 3)
	,OSW_INT_TIM2_IRQ
#endif
#if (DTIM_PORT_CNT >= 4)
	,OSW_INT_TIM3_IRQ
#endif
#if (DTIM_PORT_CNT >= 5)
	,OSW_INT_TIM4_IRQ
#endif
#if (DTIM_PORT_CNT >= 6)
	,OSW_INT_TIM5_IRQ
#endif
#if (DTIM_PORT_CNT >= 7)
	,OSW_INT_TIM6_IRQ
#endif
#if (DTIM_PORT_CNT >= 8)
	,OSW_INT_TIM7_IRQ
#endif
#if (DTIM_PORT_CNT >= 9)
	,OSW_INT_TIM8_IRQ
#endif
#if (DTIM_PORT_CNT >= 10)
	,OSW_INT_TIM9_IRQ
#endif
#if (DTIM_PORT_CNT >= 11)
	,OSW_INT_TIM10_IRQ
#endif
#if (DTIM_PORT_CNT >= 12)
	,OSW_INT_TIM11_IRQ
#endif
#if (DTIM_PORT_CNT >= 13)
	,OSW_INT_TIM12_IRQ
#endif
#if (DTIM_PORT_CNT >= 14)
	,OSW_INT_TIM13_IRQ
#endif
#if (DTIM_PORT_CNT >= 15)
	,OSW_INT_TIM14_IRQ
#endif
#if (DTIM_PORT_CNT >= 16)
	,OSW_INT_TIM15_IRQ
#endif
};


/*==============================================================================*/
/* 共用割り込みエントリ															*/
/*==============================================================================*/
static void tim_isr( UINT8 num )
{
	TIM_STR *pTim;
	
	if( pshTim[num] == 0 ) return;
	pTim = pshTim[num];
	
	tim_dummy_dw = IOREG32(pTim->base,DMTIM_TIMERSEOI);
	if( (pTim->tprm.opt & TIM_OPT_CONTINUE) == 0 ){
		IOREG32(pTim->base,DMTIM_TIMER1CONTROLREG) = 0;
	}
	
	if( pTim->prm.tim_cb_func ){
		(*pTim->prm.tim_cb_func)( pTim->prm.cb_arg );
	}
}


#if (DTIM_PORT_CNT >= 1)
/*==============================================================================*/
/* ポート0割り込みエントリ														*/
/*==============================================================================*/
void tim_isr0( void ) { tim_isr(0); }
#endif
#if (DTIM_PORT_CNT >= 2)
/*==============================================================================*/
/* ポート1割り込みエントリ														*/
/*==============================================================================*/
void tim_isr1( void ) { tim_isr(1); }
#endif
#if (DTIM_PORT_CNT >= 3)
/*==============================================================================*/
/* ポート2割り込みエントリ														*/
/*==============================================================================*/
void tim_isr2( void ) { tim_isr(2); }
#endif
#if (DTIM_PORT_CNT >= 4)
/*==============================================================================*/
/* ポート3割り込みエントリ														*/
/*==============================================================================*/
void tim_isr3( void ) { tim_isr(3); }
#endif
#if (DTIM_PORT_CNT >= 5)
/*==============================================================================*/
/* ポート4割り込みエントリ														*/
/*==============================================================================*/
void tim_isr4( void ) { tim_isr(4); }
#endif
#if (DTIM_PORT_CNT >= 6)
/*==============================================================================*/
/* ポート5割り込みエントリ														*/
/*==============================================================================*/
void tim_isr5( void ) { tim_isr(5); }
#endif
#if (DTIM_PORT_CNT >= 7)
/*==============================================================================*/
/* ポート6割り込みエントリ														*/
/*==============================================================================*/
void tim_isr6( void ) { tim_isr(6); }
#endif
#if (DTIM_PORT_CNT >= 8)
/*==============================================================================*/
/* ポート7割り込みエントリ														*/
/*==============================================================================*/
void tim_isr7( void ) { tim_isr(7); }
#endif
#if (DTIM_PORT_CNT >= 9)
/*==============================================================================*/
/* ポート8割り込みエントリ														*/
/*==============================================================================*/
void tim_isr8( void ) { tim_isr(8); }
#endif
#if (DTIM_PORT_CNT >= 10)
/*==============================================================================*/
/* ポート9割り込みエントリ														*/
/*==============================================================================*/
void tim_isr9( void ) { tim_isr(9); }
#endif
#if (DTIM_PORT_CNT >= 11)
/*==============================================================================*/
/* ポート10割り込みエントリ														*/
/*==============================================================================*/
void tim_isr10( void ) { tim_isr(10); }
#endif
#if (DTIM_PORT_CNT >= 12)
/*==============================================================================*/
/* ポート11割り込みエントリ														*/
/*==============================================================================*/
void tim_isr11( void ) { tim_isr(11); }
#endif
#if (DTIM_PORT_CNT >= 13)
/*==============================================================================*/
/* ポート12割り込みエントリ														*/
/*==============================================================================*/
void tim_isr12( void ) { tim_isr(12); }
#endif
#if (DTIM_PORT_CNT >= 14)
/*==============================================================================*/
/* ポート13割り込みエントリ														*/
/*==============================================================================*/
void tim_isr13( void ) { tim_isr(13); }
#endif
#if (DTIM_PORT_CNT >= 15)
/*==============================================================================*/
/* ポート14割り込みエントリ														*/
/*==============================================================================*/
void tim_isr14( void ) { tim_isr(14); }
#endif
#if (DTIM_PORT_CNT >= 16)
/*==============================================================================*/
/* ポート15割り込みエントリ														*/
/*==============================================================================*/
void tim_isr15( void ) { tim_isr(15); }
#endif

/*==============================================================================*/
/* 割り込みエントリテーブル														*/
/*==============================================================================*/
static const osw_isr_func tim_isr_entry[DTIM_PORT_CNT] = {
#if (DTIM_PORT_CNT >= 1)
	tim_isr0
#endif
#if (DTIM_PORT_CNT >= 2)
	,tim_isr1
#endif
#if (DTIM_PORT_CNT >= 3)
	,tim_isr2
#endif
#if (DTIM_PORT_CNT >= 4)
	,tim_isr3
#endif
#if (DTIM_PORT_CNT >= 5)
	,tim_isr4
#endif
#if (DTIM_PORT_CNT >= 6)
	,tim_isr5
#endif
#if (DTIM_PORT_CNT >= 7)
	,tim_isr6
#endif
#if (DTIM_PORT_CNT >= 8)
	,tim_isr7
#endif
#if (DTIM_PORT_CNT >= 9)
	,tim_isr8
#endif
#if (DTIM_PORT_CNT >= 10)
	,tim_isr9
#endif
#if (DTIM_PORT_CNT >= 11)
	,tim_isr10
#endif
#if (DTIM_PORT_CNT >= 12)
	,tim_isr11
#endif
#if (DTIM_PORT_CNT >= 13)
	,tim_isr12
#endif
#if (DTIM_PORT_CNT >= 14)
	,tim_isr13
#endif
#if (DTIM_PORT_CNT >= 15)
	,tim_isr14
#endif
#if (DTIM_PORT_CNT >= 16)
	,tim_isr15
#endif
};


/*==============================================================================*/
/* レジスタ初期化																*/
/*==============================================================================*/
static INT8 tim_reg_init( TIM_STR *pTim )
{
	DBG_TRACE1( "tim:tim_reg_init(%u)\n", pTim->prm.port );
	IOREG32(pTim->base,DMTIM_TIMER1CONTROLREG) = 0;
	
	return( TRUE );
}


/*==============================================================================*/
/* Timer開始																	*/
/*==============================================================================*/
static INT8 tim_start( TIM_STR *pTim, TIM_PARAM *prm )
{
	UINT32 i;
	UINT32 n;
	FLOAT64 f;
	
	DBG_TRACE1( "tim:tim_start(%u)\n", pTim->prm.port );
	
	if( prm->freq <= 0 ){
		DBG_ERR();
		return( FALSE );
	}
	
	n = pTim->prm.port;
	
	f = (FLOAT64)CFG_TIM_SYSCLK[n] / prm->freq;
	if( (f > 0xFFFFFFFF) || (f < 1.0f) ){
		DBG_ERR();
		return( FALSE );
	}
	i = (UINT32)f;
	DBG_TRACE2( "tim:freq(%u) %uHz\n", pTim->prm.port, (CFG_TIM_SYSCLK[n]/i) );
	
	OSW_ISR_disable( tim_interrupt_id[n] );
	IOREG32(pTim->base,DMTIM_TIMER1CONTROLREG) = 0;
	IOREG32(pTim->base,DMTIM_TIMER1LOADCOUNT) = (UINT32)i;
	IOREG32(pTim->base,DMTIM_TIMER1CONTROLREG) = 0x3;
	OSW_ISR_enable( tim_interrupt_id[n] );
	
	return( TRUE );
}


/*==============================================================================*/
/* Timer停止																	*/
/*==============================================================================*/
static void tim_stop( TIM_STR *pTim )
{
	DBG_TRACE1( "tim:tim_stop(%u)\n", pTim->prm.port );
	
	OSW_ISR_disable( tim_interrupt_id[pTim->prm.port] );
	IOREG32(pTim->base,DMTIM_TIMER1CONTROLREG) = 0;
}


/*==============================================================================*/
/* ドライバオープンAPI															*/
/*==============================================================================*/
INT8 Tim_open( TIM_HANDLE *handle, TIM_INIT_PARAM *param )
{
	TIM_STR *pTim;
	UINT32 n;
	
	if( (param == 0) || 
		(param->port >= DTIM_PORT_CNT) ||
		(handle == 0) ||
		(pshTim[param->port]) ){
		DBG_ERR();
		return( FALSE );
	}
	pTim = (TIM_STR *)OSW_MEM_alloc( pshMemTim, sizeof(TIM_STR), 4 );
	if( pTim == 0 ){
		DBG_ERR();
		return( FALSE );
	}
	memset( (void *)pTim, 0, sizeof(TIM_STR) );
	handle->hdl = (void *)pTim;
	
	pTim->prm = *param;
	n = pTim->prm.port;
	pTim->base = tim_base[n];
	
	if( OSW_ISR_create( &pTim->hIsr, tim_interrupt_id[n], tim_isr_entry[n] ) == FALSE ){
		Tim_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	if( tim_reg_init( pTim ) == FALSE ){
		Tim_close( handle );
		DBG_ERR();
		return( FALSE );
	}
	
	pshTim[n] = pTim;
	
	DBG_TRACE1( "Tim_open(%u)\n", n );
	
	return( TRUE );
}


/*==============================================================================*/
/* ドライバクローズAPI															*/
/*==============================================================================*/
void Tim_close( TIM_HANDLE *handle )
{
	TIM_STR *pTim;
	UINT32 n;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return;
	}
	pTim = (TIM_STR *)handle->hdl;
	n = pTim->prm.port;
	tim_stop( pTim );
	pshTim[n] = 0;
	OSW_ISR_delete( &pTim->hIsr );
	OSW_MEM_free( pshMemTim, pTim, sizeof(TIM_STR) );
	handle->hdl = 0;
	
	DBG_TRACE1( "Tim_close(%u)\n", n );
}


/*==============================================================================*/
/* 開始API																		*/
/*==============================================================================*/
INT8 Tim_start( TIM_HANDLE *handle, TIM_PARAM *param )
{
	TIM_STR *pTim;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return( FALSE );
	}
	_E_DEBUG();
	pTim = (TIM_STR *)handle->hdl;
	pTim->tprm = *param;
	
	if( tim_start( pTim, param ) == FALSE ){
		DBG_ERR();
		return( FALSE );
	}
	
	DBG_TRACE1( "Tim_start(%u)\n", pTim->prm.port );
	
	return( TRUE );
}


/*==============================================================================*/
/* 停止API																		*/
/*==============================================================================*/
void Tim_stop( TIM_HANDLE *handle )
{
	TIM_STR *pTim;
	
	if( (handle == 0) || (handle->hdl == 0) ){
		DBG_ERR();
		return;
	}
	_E_DEBUG();
	pTim = (TIM_STR *)handle->hdl;
	tim_stop( pTim );
	
	DBG_TRACE1( "Tim_stop(%u)\n", pTim->prm.port );
}


/*==============================================================================*/
/* 初期化API																	*/
/*==============================================================================*/
INT8 Tim_init( OSW_MEM_HANDLE *mem_handle )
{
	pshMemTim = mem_handle;
	memset( (void *)pshTim, 0, sizeof(pshTim) );
	DBG_TRACE1( "Tim_init()\n" );
	
	return( TRUE );
}






