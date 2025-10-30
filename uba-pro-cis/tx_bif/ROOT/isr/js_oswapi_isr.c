/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: OS Wrraper API(Cyclone V Cortex-A9 OS無し)							*/
/* Comment:																		*/
/* 	・本コード内で、実際に使用しているOSとのI/F変換を行います。					*/
/*==============================================================================*/

/*==============================================================================*/
/* インクルード																	*/
/*==============================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "kernel_inc.h"
#include "kernel_config.h"
#include "txi_knl.h"

#include "js_oswapi_cfg.h"
#include "js_oswapi.h"
#include "js_io.h"
#include "cortex_a9/cpu_api.h"
#include "intc/js_intc_reg.h"
#if JS_E_DEBUG
#include "js_e_debug.h"
#else
#define	_E_DEBUG()
#endif
#include "alt_interrupt.h"

#if OSW_FAIL_STOP
#define	OSW_FAIL_DETECT(x)	if(x){program_error();}
#else
#define OSW_FAIL_DETECT(x)
#endif

/*==============================================================================*/
/* Extern																		*/
/*==============================================================================*/
extern T_DINH _kernel_inhno[SYSTEM_INHNO_MAX+1];


/*==============================================================================*/
/* 割り込み登録																	*/
/*==============================================================================*/
INT32 OSW_ISR_create( OSW_ISR_HANDLE *handle, UINT16 interrupt_id, osw_isr_func func )
{
	OSW_FAIL_DETECT(handle==NULL);
	if( handle == 0 ) return( FALSE );

#if 1
    T_DINH dinh;

	if( _kernel_inhno[interrupt_id].inthdr ) return( FALSE );

	dinh.inhatr = 0;
	dinh.inthdr = (FP)func;
    if (E_OK > def_inh(interrupt_id, &dinh)) {
    	return( FALSE );
    }
	handle->hdl = (void *)1;
	OSW_FAIL_DETECT(handle->hdl==NULL);

	if( handle->hdl == 0 ) return( FALSE );
	handle->interrupt_id = interrupt_id;
#else
	if( osw_isr_entry[interrupt_id] ) return( FALSE );
	osw_isr_entry[interrupt_id] = func;
	handle->hdl = (void *)1;
	OSW_FAIL_DETECT(handle->hdl==NULL);

	if( handle->hdl == 0 ) return( FALSE );
	handle->interrupt_id = interrupt_id;
#endif

	return( TRUE );
}


/*==============================================================================*/
/* 割り込み削除																	*/
/*==============================================================================*/
void OSW_ISR_delete( OSW_ISR_HANDLE *handle )
{
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle->hdl==NULL);
	if( handle && handle->hdl ){
#if 1
		_kernel_inhno[handle->interrupt_id].inthdr = NULL;
#else
		osw_isr_entry[handle->interrupt_id] = 0;
#endif
		handle->hdl = 0;
	}
}


/*==============================================================================*/
/* 指定割り込み有効																*/
/*==============================================================================*/
void OSW_ISR_enable( UINT16 interrupt_id )
{
	UINT32 i = interrupt_id >> 5;
	UINT32 id = get_core_id();
	UINT32 isr = irq_disable();

	IOREG8(INTC_BASE,INTC_ICDIPTR(interrupt_id)) |= (1<<id);
	IOREG32(INTC_BASE,INTC_ICDISER(i)) = (1<<(interrupt_id&0x1F));
	irq_restore( isr );
}


/*==============================================================================*/
/* 指定割り込み無効																*/
/*==============================================================================*/
void OSW_ISR_disable( UINT16 interrupt_id )
{
	UINT32 i = interrupt_id >> 5;
	UINT32 id = get_core_id();
	UINT32 isr = irq_disable();

	IOREG8(INTC_BASE,INTC_ICDIPTR(interrupt_id)) &= (~(1<<id));
	IOREG32(INTC_BASE,INTC_ICDICER(i)) = (1<<(interrupt_id&0x1F));
	irq_restore( isr );
}


/*==============================================================================*/
/* 指定割り込み優先度設定														*/
/*==============================================================================*/
void OSW_ISR_set_priority( UINT16 interrupt_id , UINT8 level)
{
	IOREG8(INTC_BASE,INTC_ICDIPR(interrupt_id)) = level;
}


#if 0
ER ena_int(INTNO intno);		///< 割込み許可
ER dis_int(INTNO intno);		///< 割込み禁止
ER_ID acre_isr(T_CISR *pk_cisr);		///< 割込みサービスルーチン設定(ID自動割付け)
ER cre_isr(ID isrid, T_CISR *pk_cisr);		///< 割込みサービスルーチン設定
ER del_isr(ID isrid);		///< 割込みサービスルーチン削除
ER set_int_typ(INTNO intno, irq_type_t type);		///< 割込みタイプ設定
/**
 * @brife 割込み許可
 * @param intno 割込み番号
 * @return
**/
ER ena_int(INTNO intno)
{
	ER er = E_OK;
	ALT_STATUS_CODE code = ALT_E_SUCCESS;
	ALT_INT_INTERRUPT_t int_num = (ALT_INT_INTERRUPT_t)intno;

	code = alt_int_dist_pending_clear(int_num);		// 割込み許可前にペンディング要求をクリアしておく, 20/06/22
	er = (code == ALT_E_SUCCESS ? E_OK : E_PAR);
	if (er == E_OK) {
		code = alt_int_dist_enable(int_num);
		er = (code == ALT_E_SUCCESS ? E_OK : E_PAR);
	}

	return er;
}

/**
 * @brife 割込み禁止
 * @param intno 割込み番号
 * @return
**/
ER dis_int(INTNO intno)
{
	ER er = E_OK;
	ALT_STATUS_CODE code = ALT_E_SUCCESS;
	ALT_INT_INTERRUPT_t int_num = (ALT_INT_INTERRUPT_t)intno;

	code = alt_int_dist_disable(int_num);
	er = (code == ALT_E_SUCCESS ? E_OK : E_PAR);
	if (er == E_OK) {
		code = alt_int_dist_pending_clear(int_num);
		er = (code == ALT_E_SUCCESS ? E_OK : E_PAR);
	}

	return er;
}

/**
 * @brife 割込みサービスルーチン設定
 * @param isrid 割込みサービスルーチンID
 * @param pk_cisr 割込みサービスルーチン情報パケット
 * @return
**/
ER cre_isr(ID isrid, T_CISR *pk_cisr)
{
	UINT32 int_stat;		// 割込みステータス
	ER_ID er = E_OK;		// E_OK, E_PAR
	ALT_STATUS_CODE code = ALT_E_SUCCESS;
	ATR isratr = pk_cisr->isratr;
	VP_INT exinf = pk_cisr->exinf;
	ALT_INT_INTERRUPT_t int_num = (ALT_INT_INTERRUPT_t)pk_cisr->intno;
	FP isr = pk_cisr->isr;
	UINT imask = pk_cisr->imask;
	T_DINH dinh = {isratr, isr};

	int_stat = OSW_ISR_global_disable();		// 割込み禁止, 19/04/15
	er = (kernel_isr_id[isrid] == NULL ? E_OK : E_ID);
	if (er == E_OK) {
		er = dis_int(int_num);
	}
	if (er == E_OK) {
//		code = alt_int_isr_register(int_num, isr, exinf);
//		er = (code == ALT_E_SUCCESS ? E_OK : E_PAR);
		er = def_inh(int_num, &dinh);
	}
	if (er == E_OK) {
		code = alt_int_dist_priority_set(int_num, imask);
		er = (code == ALT_E_SUCCESS ? E_OK : E_PAR);
	}
	if (er == E_OK) {
		code = alt_int_dist_target_set(int_num, 1);		// 割込み通知CPU0
		er = (code == ALT_E_SUCCESS ? E_OK : E_PAR);
	}
	if (er == E_OK) {
		kernel_isr_id[isrid] = pk_cisr;
	}

	OSW_ISR_global_restore(int_stat);		// 割込み状態復帰, 19/04/15

	return er;
}

/**
 * @brief 割込みサービスルーチン設定(ID自動設定)
 * @param pk_cisr 割込みサービスルーチン情報パケット
 * @return
**/
ER_ID acre_isr(T_CISR *pk_cisr)
{
	ER er = E_OK;
	ID id = ISRID_MAX;
TX_INTERRUPT_SAVE_AREA

    TX_DISABLE  /* Lockout interrupts. */
	while (id > 0) {		// 降順検索に変更, 19/04/17
		id--;
		if (id > 0) {		// ISRIDの0番は未使用とする, 20/05/19
			if (kernel_isr_id[id] == NULL) {		// 未割当て領域有りのとき
				er = cre_isr(id, pk_cisr);		// 割込みサービスルーチン設定
				er = (er == E_OK ? id : er);		// OKのときISRのIDを返す、NGのとき0を返す
				break;		// 検索終了
			}
		}
	}
	if (id == 0) {		// 割当て可能IDが無い
		er = E_NOID;
	}
    TX_RESTORE  /* Restore interrupts. */

	return (ER_ID)er;
}
/**
 * @brife 割込みサービスルーチン削除
 * @param isrid
 * @return
**/
ER del_isr(ID isrid)
{
	kernel_isr_id[isrid] = NULL;

	return E_OK;
}
#endif

#if 1
#include "common.h"
/**
 * @brife 割込みタイプ設定
 * @param intno 割込み番号
 * @param type 割込みタイプ(エッジ割込み/レベル割込み)
 * @return
**/
ER set_int_typ(INTNO intno, irq_type_t type)
{
	ER er = E_OK;
	ALT_STATUS_CODE code = ALT_E_SUCCESS;
	ALT_INT_TRIGGER_t trigger = (type == IRQ_EDGE_RISING ? ALT_INT_TRIGGER_EDGE : ALT_INT_TRIGGER_LEVEL);
	ALT_INT_INTERRUPT_t int_num = (ALT_INT_INTERRUPT_t)intno;

	code = alt_int_dist_trigger_set(int_num, trigger);
	er = (code == ALT_E_SUCCESS ? E_OK : E_PAR);

	return er;
}
#endif


