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

#if OSW_FAIL_STOP
#define	OSW_FAIL_DETECT(x)	if(x){while(1);}
#else
#define OSW_FAIL_DETECT(x)
#endif

/*==============================================================================*/
/* Extern																		*/
/*==============================================================================*/
extern osw_isr_func osw_isr_entry[];


/*==============================================================================*/
/* ローカル変数																	*/
/*==============================================================================*/
static volatile INT32 sys_log_start;
#if (OSW_MEM_DISABLE == 0)
typedef struct {
	UINT32 mem;
	UINT32 size;
	INT32 next_idx;
} OSW_MEM_LIST_DATA;

typedef struct {
	UINT8 *buf;
	INT32 cnt;
	UINT8 *buf_bot;
	OSW_MEM_LIST_DATA *dat;
} OSW_MEM_LIST;

OSW_MEM_HANDLE osw_default_heap;
static OSW_MEM_LIST osw_heap_list;
static UINT8 osw_heap_mem[OSW_DEF_HEAP];
#endif
volatile UINT32 osw_sys_timer;


/*==============================================================================*/
/* タスク生成																	*/
/*==============================================================================*/
INT32 OSW_TSK_create( OSW_TSK_HANDLE *handle, osw_tsk_func func, INT32 tsk_priority, UINT32 stack_size, void *stack, void *arg )
{
	return( FALSE );
}


/*==============================================================================*/
/* タスク削除																	*/
/*==============================================================================*/
void OSW_TSK_delete( OSW_TSK_HANDLE *handle )
{
}


/*==============================================================================*/
/* 指定時間遅延																	*/
/*==============================================================================*/
void OSW_TSK_sleep( UINT32 tick )
{
	UINT32 i = osw_sys_timer;
	_E_DEBUG();
	while( 1 ){
		if( (osw_sys_timer - i) >= tick ) break;
	};
}


/*==============================================================================*/
/* カレントタスクハンドル取得													*/
/*==============================================================================*/
void OSW_TSK_get_handle( OSW_TSK_HANDLE *handle )
{
	OSW_FAIL_DETECT(handle==NULL);
	if( handle ){
		handle->hdl = (void *)1;
	}
}


/*==============================================================================*/
/* プライオリティ変更															*/
/*==============================================================================*/
INT32 OSW_TSK_setpri( OSW_TSK_HANDLE *handle, INT32 tsk_priority )
{
	if( tsk_priority > OSW_TSKPRIO_REALTIME ) return( FALSE );
	return( TRUE );
}


/*==============================================================================*/
/* カレントタスク終了															*/
/*==============================================================================*/
void OSW_TSK_exit( void )
{
}


/*==============================================================================*/
/* 指定タスクが終了しているか確認												*/
/*==============================================================================*/
INT32 OSW_TSK_terminated( OSW_TSK_HANDLE *handle )
{
	return( TRUE );
}


/*==============================================================================*/
/* ディスパッチ許可																*/
/*==============================================================================*/
void OSW_TSK_enable( void )
{
}


/*==============================================================================*/
/* ディスパッチ禁止																*/
/*==============================================================================*/
UINT32 OSW_TSK_disable( void )
{
	return( 0 );
}


/*==============================================================================*/
/* ディスパッチ禁止/許可状態の復元												*/
/*==============================================================================*/
void OSW_TSK_restore( UINT32 disable_stat )
{
}


/*==============================================================================*/
/* スタック使用最大値の取得														*/
/*==============================================================================*/
UINT32 OSW_TSK_max_stack( OSW_TSK_HANDLE *handle )
{
	return( 0 );
}


#if (OSW_MSG_DISABLE == 0)
/*==============================================================================*/
/* メッセージキュー生成															*/
/*==============================================================================*/
INT32 OSW_MSG_create( OSW_MSG_HANDLE *handle, UINT32 cnt )
{
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle==NULL);
	
	if( handle == 0 ) return( FALSE );
	
	if( cnt == 0 ){
		/* Default */
		cnt = 1;
	}
	handle->q_total = cnt;
	
	handle->post_hdl = (void *)OSW_MEM_alloc( NULL, 4, 4 );
	OSW_FAIL_DETECT(handle->post_hdl==NULL);
	if( handle->post_hdl == 0 ) return( FALSE );
	*((UINT32 *)handle->post_hdl) = handle->q_total;
	
	handle->pend_hdl = (void *)OSW_MEM_alloc( NULL, 4, 4 );
	OSW_FAIL_DETECT(handle->pend_hdl==NULL);
	if( handle->pend_hdl == 0 ) return( FALSE );
	*((UINT32 *)handle->pend_hdl) = 0;
	
	handle->q_mem = (UINT32 *)OSW_MEM_alloc( NULL, (handle->q_total*sizeof(UINT32)), 4 );
	OSW_FAIL_DETECT(handle->q_mem==NULL);
	if( handle->q_mem == 0 ) return( FALSE );
	
	handle->post_idx = 0;
	handle->pend_idx = 0;
	
	return( TRUE );
}


/*==============================================================================*/
/* メッセージキュー削除															*/
/*==============================================================================*/
void OSW_MSG_delete( OSW_MSG_HANDLE *handle )
{
	OSW_FAIL_DETECT(handle==NULL);
	if( handle ){
		if( handle->q_mem ){
			OSW_MEM_free( NULL, handle->q_mem, (handle->q_total*sizeof(UINT32)) );
			handle->q_mem = 0;
		}
		if( handle->pend_hdl ){
			OSW_MEM_free( NULL, handle->pend_hdl, 4 );
			handle->pend_hdl = 0;
		}
		if( handle->post_hdl ){
			OSW_MEM_free( NULL, handle->post_hdl, 4 );
			handle->post_hdl = 0;
		}
	}
}


/*==============================================================================*/
/* メッセージキュー送信															*/
/*==============================================================================*/
INT32 OSW_MSG_post( OSW_MSG_HANDLE *handle, void *msg, UINT32 time_out )
{
	UINT32 isr_stat,i;
	
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle->post_hdl==NULL);
	OSW_FAIL_DETECT(handle->pend_hdl==NULL);
	OSW_FAIL_DETECT(handle->q_mem==NULL);
	
	if( (handle==NULL) || (handle->post_hdl==NULL) || (handle->pend_hdl==NULL) || (handle->q_mem==NULL) ) return( FALSE );
	
	i = osw_sys_timer;
	while( 1 ){
		if( *((volatile UINT32 *)handle->post_hdl) ){
			isr_stat = OSW_ISR_global_disable();
			if( *((volatile UINT32 *)handle->post_hdl) ){
				(*((volatile UINT32 *)handle->post_hdl))--;
			}
			OSW_ISR_global_restore( isr_stat );
			break;
		}
		if( time_out != OSW_TOUT_INFINITY ){
			if( (osw_sys_timer - i) >= time_out ) return( FALSE );
		}
		wait_isr();
	};
	
	handle->q_mem[handle->post_idx++] = (UINT32)msg;
	if( handle->post_idx >= handle->q_total ) handle->post_idx = 0;
	
	isr_stat = OSW_ISR_global_disable();
	(*((volatile UINT32 *)handle->pend_hdl))++;
	OSW_ISR_global_restore( isr_stat );
	
	return( TRUE );
}


/*==============================================================================*/
/* メッセージキュー受信待ち														*/
/*==============================================================================*/
INT32 OSW_MSG_pend( OSW_MSG_HANDLE *handle, void **msg, UINT32 time_out )
{
	UINT32 isr_stat,i;
	
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle->post_hdl==NULL);
	OSW_FAIL_DETECT(handle->pend_hdl==NULL);
	OSW_FAIL_DETECT(handle->q_mem==NULL);
	
	if( (handle==NULL) || (handle->post_hdl==NULL) || (handle->pend_hdl==NULL) || (handle->q_mem==NULL) ) return( FALSE );
	
	_E_DEBUG();
	i = osw_sys_timer;
	while( 1 ){
		if( *((volatile UINT32 *)handle->pend_hdl) ){
			isr_stat = OSW_ISR_global_disable();
			if( *((volatile UINT32 *)handle->pend_hdl) ){
				(*((volatile UINT32 *)handle->pend_hdl))--;
			}
			OSW_ISR_global_restore( isr_stat );
			break;
		}
		if( time_out != OSW_TOUT_INFINITY ){
			if( (osw_sys_timer - i) >= time_out ) return( FALSE );
		}
		wait_isr();
	};
	
	*msg = (void *)handle->q_mem[handle->pend_idx++];
	if( handle->pend_idx >= handle->q_total ) handle->pend_idx = 0;
	
	isr_stat = OSW_ISR_global_disable();
	(*((volatile UINT32 *)handle->post_hdl))++;
	OSW_ISR_global_restore( isr_stat );
	
	return( TRUE );
}
#endif


#if (OSW_SEM_DISABLE == 0)
/*==============================================================================*/
/* セマフォ生成																	*/
/*==============================================================================*/
INT32 OSW_SEM_create( OSW_SEM_HANDLE *handle, UINT32 init_count )
{
	OSW_FAIL_DETECT(handle==NULL);
	if( handle == 0 ) return( FALSE );
	
	handle->hdl = (void *)OSW_MEM_alloc( NULL, 4, 4 );
	OSW_FAIL_DETECT(handle->hdl==NULL);
	if( handle->hdl == 0 ) return( FALSE );
	*((UINT32 *)handle->hdl) = init_count;
	
	return( TRUE );
}


/*==============================================================================*/
/* セマフォ削除																	*/
/*==============================================================================*/
void OSW_SEM_delete( OSW_SEM_HANDLE *handle )
{
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle->hdl==NULL);
	if( handle && handle->hdl ){
		OSW_MEM_free( NULL, handle->hdl, 4 );
		handle->hdl = 0;
	}
}


/*==============================================================================*/
/* セマフォカウントリセット														*/
/*==============================================================================*/
void OSW_SEM_reset( OSW_SEM_HANDLE *handle )
{
	UINT32 isr_stat;
	
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle->hdl==NULL);
	if( handle && handle->hdl ){
		isr_stat = OSW_ISR_global_disable();
		*((volatile UINT32 *)handle->hdl) = 0;
		OSW_ISR_global_restore( isr_stat );
	}
}


/*==============================================================================*/
/* セマフォカウント取得															*/
/*==============================================================================*/
UINT32 OSW_SEM_count( OSW_SEM_HANDLE *handle )
{
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle->hdl==NULL);
	if( handle && handle->hdl ){
		return( *((UINT32 *)handle->hdl) );
	}
	return( 0 );
}


/*==============================================================================*/
/* セマフォポスト																*/
/*==============================================================================*/
void OSW_SEM_post( OSW_SEM_HANDLE *handle )
{
	UINT32 isr_stat;
	
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle->hdl==NULL);
	if( handle && handle->hdl ){
		isr_stat = OSW_ISR_global_disable();
		(*((volatile UINT32 *)handle->hdl))++;
		OSW_ISR_global_restore( isr_stat );
	}
}


/*==============================================================================*/
/* セマフォ待ち																	*/
/*==============================================================================*/
INT32 OSW_SEM_pend( OSW_SEM_HANDLE *handle, UINT32 time_out )
{
	UINT32 isr_stat,i;
	
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle->hdl==NULL);
	
	_E_DEBUG();
	i = osw_sys_timer;
	while( 1 ){
		if( *((volatile UINT32 *)handle->hdl) ){
			isr_stat = OSW_ISR_global_disable();
			if( *((volatile UINT32 *)handle->hdl) ){
				(*((volatile UINT32 *)handle->hdl))--;
			}
			OSW_ISR_global_restore( isr_stat );
			break;
		}
		if( time_out != OSW_TOUT_INFINITY ){
			if( (osw_sys_timer - i) >= time_out ) return( FALSE );
		}
		wait_isr();
	};
	return( TRUE );
}
#endif


#if (OSW_EVT_DISABLE == 0)
/*==============================================================================*/
/* イベントフラグ生成															*/
/*==============================================================================*/
INT32 OSW_EVT_create( OSW_EVT_HANDLE *handle, UINT16 bit_cnt )
{
	OSW_FAIL_DETECT(handle==NULL);
	
	if( (handle == 0) || (bit_cnt > 16) || (bit_cnt == 0) ) return( FALSE );
	
	handle->hdl = (void *)OSW_MEM_alloc( NULL, 2, 2 );
	OSW_FAIL_DETECT(handle->hdl==NULL);
	
	if( handle->hdl == 0 ) return( FALSE );
	*((volatile UINT16 *)handle->hdl) = 0;
	
	return( TRUE );
}


/*==============================================================================*/
/* イベントフラグ削除															*/
/*==============================================================================*/
void OSW_EVT_delete( OSW_EVT_HANDLE *handle )
{
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle->hdl==NULL);
	if( handle && handle->hdl ){
		OSW_MEM_free( NULL, handle->hdl, 2 );
		handle->hdl = 0;
	}
}


/*==============================================================================*/
/* イベントフラグセット															*/
/*==============================================================================*/
void OSW_EVT_set( OSW_EVT_HANDLE *handle, UINT16 bit )
{
	UINT32 isr_stat;
	
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle->hdl==NULL);
	if( handle && handle->hdl ){
		isr_stat = OSW_ISR_global_disable();
		*((volatile UINT16 *)handle->hdl) |= (1<<bit);
		OSW_ISR_global_restore( isr_stat );
	}
}


/*==============================================================================*/
/* イベントフラグ待ち															*/
/*==============================================================================*/
UINT16 OSW_EVT_wait( OSW_EVT_HANDLE *handle, UINT32 time_out )
{
	UINT32 isr_stat,i;
	UINT16 flg;
	
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle->hdl==NULL);
	
	_E_DEBUG();
	i = osw_sys_timer;
	while( 1 ){
		flg = *((volatile UINT16 *)handle->hdl);
		if( flg ){
			isr_stat = OSW_ISR_global_disable();
			*((volatile UINT16 *)handle->hdl) = 0;
			OSW_ISR_global_restore( isr_stat );
			break;
		}
		if( time_out != OSW_TOUT_INFINITY ){
			if( (osw_sys_timer - i) >= time_out ) return( 0 );
		}
		wait_isr();
	};
	return( flg );
}
#endif


/*==============================================================================*/
/* システムタイマー取得(ms)														*/
/*==============================================================================*/
UINT32 OSW_TIM_value( void )
{
	return( osw_sys_timer );
}


#if (OSW_ISR_DISABLE == 0)
/*==============================================================================*/
/* 割り込み登録																	*/
/*==============================================================================*/
INT32 OSW_ISR_create( OSW_ISR_HANDLE *handle, UINT16 interrupt_id, osw_isr_func func )
{
	OSW_FAIL_DETECT(handle==NULL);
	if( handle == 0 ) return( FALSE );
	
	if( osw_isr_entry[interrupt_id] ) return( FALSE );
	osw_isr_entry[interrupt_id] = func;
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
		osw_isr_entry[handle->interrupt_id] = 0;
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


#endif


/*==============================================================================*/
/* グローバル割り込み有効														*/
/*==============================================================================*/
void OSW_ISR_global_enable( void )
{
	irq_enable();
}


/*==============================================================================*/
/* グローバル割り込みステータス復元												*/
/*==============================================================================*/
void OSW_ISR_global_restore( UINT32 isr_stat )
{
	irq_restore( isr_stat );
}


/*==============================================================================*/
/* グローバル割り込み禁止														*/
/*==============================================================================*/
UINT32 OSW_ISR_global_disable( void )
{
	return( irq_disable() );
}


/*==============================================================================*/
/* グローバル割り込み禁止状態取得												*/
/*==============================================================================*/
INT32 OSW_ISR_global_status( void )
{
	if( irq_status() ){
		return( TRUE );
	}
	return( FALSE );
}


/*==============================================================================*/
/* データーキャッシュWrite Back + Invalidate									*/
/*==============================================================================*/
void OSW_CAC_writeclean( void *addr, UINT32 size, INT32 wait )
{
	dcache_wbinv( (UINT32)addr, size, (UINT32)wait );
	cache_l2_wbinv( (UINT32)addr, size, (UINT32)wait );
}


/*==============================================================================*/
/* データーキャッシュWrite Back													*/
/*==============================================================================*/
void OSW_CAC_write( void *addr, UINT32 size, INT32 wait )
{
	dcache_wb( (UINT32)addr, size, (UINT32)wait );
	cache_l2_wb( (UINT32)addr, size, (UINT32)wait );
}


/*==============================================================================*/
/* データーキャッシュクリーン(Invalidate)										*/
/*==============================================================================*/
void OSW_CAC_clean( void *addr, UINT32 size, INT32 wait )
{
	dcache_inv( (UINT32)addr, size, (UINT32)wait );
	cache_l2_wbinv( (UINT32)addr, size, (UINT32)wait );
}


#if (OSW_MEM_DISABLE == 0)
/*==============================================================================*/
/* メモリーDB初期化																*/
/*==============================================================================*/
static void osw_mem_init( OSW_MEM_HANDLE *handle, UINT8 *buf, UINT32 size )
{
	OSW_MEM_LIST *list = (OSW_MEM_LIST *)handle->hdl;
	UINT32 i;
	
	list->buf = list->buf_bot = buf;
	list->cnt = 1;
	i = (UINT32)&buf[size-sizeof(OSW_MEM_LIST_DATA)];
	list->dat = (OSW_MEM_LIST_DATA *)(i & (~0x3));
	list->dat[0].mem = (UINT32)buf;
	list->dat[0].size = 0;
	list->dat[0].next_idx = 0;
	
	i = (UINT32)&buf[size] - (UINT32)&list->dat[0];
	handle->free -= i;
}


/*==============================================================================*/
/* メモリーDBから１件確保														*/
/*==============================================================================*/
static INT32 osw_mem_alloc_list( OSW_MEM_HANDLE *handle, INT32 *idx )
{
	OSW_MEM_LIST *list = (OSW_MEM_LIST *)handle->hdl;
	INT32 i;
	
	for( i = -1 ; i > (-list->cnt) ; i -- ){
		if( list->dat[i].mem == 0 ){
			*idx = i;
			return( TRUE );
		}
	}
	
	if( (UINT32)&list->dat[i] < (UINT32)list->buf_bot ){
		return( FALSE );
	}
	if( list->cnt >= 10000000 ){
		return( FALSE );
	}
	list->dat[i].mem = 0;
	list->dat[i].size = 0;
	list->dat[i].next_idx = 0;
	list->cnt++;
	handle->free -= sizeof(OSW_MEM_LIST_DATA);
	*idx = i;
	
	return( TRUE );
}


/*==============================================================================*/
/* アライメント値補正															*/
/*==============================================================================*/
static UINT32 osw_align_chk( UINT32 align )
{
	INT32 i;
	
	for( i = 15 ; i > 0 ; i -- ){
		if( align >= (UINT32)(1<<i) ) break;
	}
	return( (UINT32)(1<<i) );
}


/*==============================================================================*/
/* メモリー確保																	*/
/*==============================================================================*/
static void * osw_mem_alloc( OSW_MEM_HANDLE *handle, UINT32 size, UINT32 align )
{
	OSW_MEM_LIST *list = (OSW_MEM_LIST *)handle->hdl;
	UINT32 j;
	INT32 i,n,f,next;
	UINT8 *p,*limit;
	
	if( osw_mem_alloc_list( handle, &f ) == FALSE ) return( NULL );
	align = osw_align_chk( align );
	n = list->cnt-1;
	limit = (UINT8 *)&list->dat[-n];
	
	n = 0;
	for( i = 0 ; i < list->cnt ; i ++ ){
		if( list->dat[n].next_idx == 0 ){
			/* Last */
			p = (UINT8 *)((UINT32)list->dat[n].mem + list->dat[n].size);
			p = (UINT8 *)(((UINT32)p + (align-1)) & (~(align-1)));
			if( &p[size] > limit ) break;
			list->dat[f].mem = (UINT32)p;
			list->dat[f].size = size;
			list->dat[f].next_idx = 0;
			list->dat[n].next_idx = (-f);
			list->buf_bot = p;
			handle->free -= size;
			return( (void *)p );
		}
		else {
			next = (-list->dat[n].next_idx);
			j = (UINT32)list->dat[n].mem + list->dat[n].size;
			j = (j + (align-1)) & (~(align-1));
			if( j < (UINT32)list->dat[next].mem ){
				if( ((UINT32)list->dat[next].mem - j) >= size ){
					/* 隙間あり */
					list->dat[f].mem = (UINT32)j;
					list->dat[f].size = size;
					list->dat[f].next_idx = (-next);
					list->dat[n].next_idx = (-f);
					handle->free -= size;
					return( (void *)j );
				}
			}
		}
		n = next;
	}
	return( NULL );
}


/*==============================================================================*/
/* メモリー解放																	*/
/*==============================================================================*/
static void osw_mem_free( OSW_MEM_HANDLE *handle, UINT8 *buf )
{
	OSW_MEM_LIST *list = (OSW_MEM_LIST *)handle->hdl;
	UINT32 j;
	INT32 i,n,b;
	
	n = b = 0;
	for( i = 0 ; i < list->cnt ; i ++ ){
		if( (list->dat[n].mem == (UINT32)buf) && list->dat[n].size ){
			j = list->dat[n].size;
			list->dat[b].next_idx = list->dat[n].next_idx;
			list->dat[n].mem = 0;
			list->dat[n].size = 0;
			list->dat[n].next_idx = 0;
			if( list->dat[b].next_idx == 0 ){
				/* Last */
				list->buf_bot = (UINT8 *)list->dat[b].mem;
			}
			handle->free += j;
			if( n == (-(list->cnt-1)) ){
				while( n < 0 ){
					if( list->dat[n].mem ) break;
					list->cnt--;
					n++;
					handle->free += sizeof(OSW_MEM_LIST_DATA);
				};
			}
			return;
		}
		b = n;
		if( list->dat[n].next_idx == 0 ) break;
		n = (-list->dat[n].next_idx);
	}
}


/*==============================================================================*/
/* ヒープ生成																	*/
/*==============================================================================*/
INT32 OSW_MEM_create( OSW_MEM_HANDLE *handle, void *buf, UINT32 size )
{
	UINT32 i;
	OSW_FAIL_DETECT(handle==NULL);
	if( handle == 0 ) return( FALSE );
	
	handle->hdl = (void *)OSW_MEM_alloc( NULL, sizeof(OSW_MEM_LIST), 4 );
	if( handle->hdl == NULL ) return( FALSE );
	memset( (void *)handle->hdl, 0, sizeof(OSW_MEM_LIST) );
	
	handle->total = size;
	handle->free = size;
	handle->max_use = 0;
	
	i = OSW_ISR_global_disable();
	osw_mem_init( handle, buf, size );
	OSW_ISR_global_restore( i );
	
	return( TRUE );
}


/*==============================================================================*/
/* ヒープ削除																	*/
/*==============================================================================*/
void OSW_MEM_delete( OSW_MEM_HANDLE *handle )
{
	OSW_FAIL_DETECT(handle==NULL);
	OSW_FAIL_DETECT(handle->hdl==NULL);
	
	if( handle && handle->hdl ){
		OSW_MEM_free( NULL, (void *)handle->hdl, sizeof(OSW_MEM_LIST) );
		handle->hdl = 0;
	}
}


/*==============================================================================*/
/* ヒープよりメモリ取得															*/
/*==============================================================================*/
void * OSW_MEM_alloc( OSW_MEM_HANDLE *handle, UINT32 size, UINT32 align )
{
	void *mem;
	UINT32 i,j;
	
	if( size == 0 ) return( NULL );
	if( handle ){
		OSW_FAIL_DETECT(handle->hdl==NULL);
	}
	else {
		handle = &osw_default_heap;
	}
	
	i = OSW_ISR_global_disable();
	mem = osw_mem_alloc( handle, size, align );
	OSW_FAIL_DETECT(mem==NULL);
	
	j = handle->total - handle->free;
	if( j > handle->max_use ) handle->max_use = j;
	
	OSW_ISR_global_restore( i );
	
	return( mem );
}


/*==============================================================================*/
/* ヒープへメモリ開放															*/
/*==============================================================================*/
void OSW_MEM_free( OSW_MEM_HANDLE *handle, void *mem, UINT32 size )
{
	UINT32 i;
	
	OSW_FAIL_DETECT(mem==NULL);
	if( handle ){
		OSW_FAIL_DETECT(handle->hdl==NULL);
	}
	else {
		handle = &osw_default_heap;
	}
	i = OSW_ISR_global_disable();
	osw_mem_free( handle, mem );
	OSW_ISR_global_restore( i );
}
#endif


/*==============================================================================*/
/* OS起動開始																	*/
/*==============================================================================*/
void OSW_SYS_init( osw_tsk_func func, UINT32 tsk_priority, UINT32 stack_size, void *arg )
{
	UINT32 n;
	
	sys_log_start = 0;
#if (OSW_MEM_DISABLE == 0)
	memset( (void *)&osw_default_heap, 0, sizeof(osw_default_heap) );
	memset( (void *)&osw_heap_list, 0, sizeof(osw_heap_list) );
	osw_default_heap.total = osw_default_heap.free = OSW_DEF_HEAP;
	osw_default_heap.hdl = (void *)&osw_heap_list;
	osw_mem_init( &osw_default_heap, osw_heap_mem, OSW_DEF_HEAP );
#endif
	
	if( get_core_id() == 0 ){
		/* INTC Reset */
		for( n = 0 ; n < 8 ; n ++ ){
			IOREG32(INTC_BASE,INTC_ICDICER(n)) = 0xFFFFFFFF;
			IOREG32(INTC_BASE,INTC_ICDICPR(n)) = 0xFFFFFFFF;
		}
		for( n = 0 ; n < 256 ; n ++ ){
			IOREG8(INTC_BASE,INTC_ICDIPR(n)) = 0;
		}
		for( n = OSW_PRIVINT_NUM ; n < 256 ; n ++ ){
			IOREG8(INTC_BASE,INTC_ICDIPTR(n)) = 0;
		}
		for( n = 2 ; n < 16 ; n ++ ){
			IOREG32(INTC_BASE,INTC_ICDICFR(n)) = 0x55555555;
		}
		IOREG32(INTC_BASE,INTC_ICDDCR) = 0x1;
	}
	
	IOREG32(INTC_BASE,INTC_ICCPMR) = 0xF0;	/* 16 supported levels */
	IOREG32(INTC_BASE,INTC_ICCBPR) = 2;
	
	while( 1 ){
		n = IOREG32(INTC_BASE,INTC_ICCIAR) = 0x3FF;
		if( n == 0x3FF ) break;
		IOREG32(INTC_BASE,INTC_ICCEOIR) = n;
	};
	IOREG32(INTC_BASE,INTC_ICCICR) = 0x1;
	
	osw_sys_timer = 0;
	OSW_ISR_global_enable();
	if( func ){
		func( arg );
	}
}


/*==============================================================================*/
/* ログ出力許可																	*/
/*==============================================================================*/
void OSW_SYS_log_start( void )
{
	sys_log_start = 1;
}


/*==============================================================================*/
/* 標準出力の置き換え関数 printf()												*/
/*==============================================================================*/
int osw_printf( const INT8 *format, ... )
{
	UINT32 isr_stat;
	static INT8 c[100];
	va_list va;
	
	if( sys_log_start == 0 ) return( 0 );
	
	isr_stat = OSW_ISR_global_disable();
	
	va_start( va, format );
	vsnprintf( c, sizeof(c), format, va );
	va_end( va );
	
	OSW_USR_logout( c, strlen(c) );
	
	OSW_ISR_global_restore( isr_stat );
	
	return( 1 );
}


