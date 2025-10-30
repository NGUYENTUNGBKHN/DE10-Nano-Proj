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
		_kernel_inhno[handle->interrupt_id].inthdr = NULL;
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


