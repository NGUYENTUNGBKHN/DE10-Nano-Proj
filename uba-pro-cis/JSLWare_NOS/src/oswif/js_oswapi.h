/*==============================================================================*/
/* Copyright (C) 2011 JSL Technology. All right reserved.						*/
/* Tittle: OS Wrraper API header(共通ヘッダ)									*/
/* Comment:																		*/
/* 	・OSのAPIを使用するときは必ず、本ヘッダに記述されたAPIのみを使用する事。	*/
/*	　(移植の容易性を高くするため)												*/
/*==============================================================================*/

#ifndef __J_OSWIF__
#define __J_OSWIF__

/*==============================================================================*/
/* バージョン情報																*/
/*==============================================================================*/
#define	OSWAPI_VER			100

/*==============================================================================*/
/* 基本型宣言マクロ																*/
/*==============================================================================*/
typedef	unsigned char		UINT8 ;
typedef	unsigned short		UINT16 ;
//typedef unsigned int		UINT32 ;
typedef unsigned long		UINT32 ;
typedef unsigned long long	UINT64 ;
typedef	char				INT8 ;
typedef	short				INT16 ;
typedef int					INT32 ;
typedef long long			INT64 ;
typedef float				FLOAT32 ;
typedef double				FLOAT64 ;

#ifndef NULL
#define NULL				0
#endif
#ifndef TRUE
#define TRUE				1
#endif
#ifndef FALSE
#define FALSE				0
#endif

#ifdef __cplusplus
#define	C_EXTERN 	extern "c"
#else
#define	C_EXTERN	extern
#endif

#include "js_oswapi_env.h"

/*==============================================================================*/
/* API引数定義																	*/
/*==============================================================================*/

/* tsk_priority */
#define		OSW_TSKPRIO_IDLE		0
#define		OSW_TSKPRIO_LOWEST		1
#define		OSW_TSKPRIO_NORMAL		2
#define		OSW_TSKPRIO_HIGHEST		3
#define		OSW_TSKPRIO_CRITICAL	4
#define		OSW_TSKPRIO_REALTIME	5

/* time_out */
#define		OSW_TOUT_INFINITY		0xFFFFFFFF

/*==============================================================================*/
/* API(タスク)																	*/
/*==============================================================================*/
typedef void (*osw_tsk_func)( void *arg );
C_EXTERN INT32 OSW_TSK_create( OSW_TSK_HANDLE *handle, osw_tsk_func func, INT32 tsk_priority, UINT32 stack_size, void *stack, void *arg );
C_EXTERN void OSW_TSK_delete( OSW_TSK_HANDLE *handle );
C_EXTERN void OSW_TSK_sleep( UINT32 tick );
C_EXTERN void OSW_TSK_get_handle( OSW_TSK_HANDLE *handle );
C_EXTERN INT32 OSW_TSK_setpri( OSW_TSK_HANDLE *handle, INT32 tsk_priority );
C_EXTERN void OSW_TSK_exit( void );
C_EXTERN INT32 OSW_TSK_terminated( OSW_TSK_HANDLE *handle );
C_EXTERN void OSW_TSK_enable( void );
C_EXTERN UINT32 OSW_TSK_disable( void );
C_EXTERN void OSW_TSK_restore( UINT32 disable_stat );
C_EXTERN UINT32 OSW_TSK_max_stack( OSW_TSK_HANDLE *handle );

/*==============================================================================*/
/* API(メッセジーキュー)														*/
/*==============================================================================*/
C_EXTERN INT32 OSW_MSG_create( OSW_MSG_HANDLE *handle, UINT32 cnt );
C_EXTERN void OSW_MSG_delete( OSW_MSG_HANDLE *handle );
C_EXTERN INT32 OSW_MSG_post( OSW_MSG_HANDLE *handle, void *msg, UINT32 time_out );
C_EXTERN INT32 OSW_MSG_pend( OSW_MSG_HANDLE *handle, void **msg, UINT32 time_out );

/*==============================================================================*/
/* API(セマフォ)																*/
/*==============================================================================*/
C_EXTERN INT32 OSW_SEM_create( OSW_SEM_HANDLE *handle, UINT32 init_count );
C_EXTERN void OSW_SEM_delete( OSW_SEM_HANDLE *handle );
C_EXTERN void OSW_SEM_reset( OSW_SEM_HANDLE *handle );
C_EXTERN UINT32 OSW_SEM_count( OSW_SEM_HANDLE *handle );
C_EXTERN void OSW_SEM_post( OSW_SEM_HANDLE *handle );
C_EXTERN INT32 OSW_SEM_pend( OSW_SEM_HANDLE *handle, UINT32 time_out );

/*==============================================================================*/
/* API(イベントフラグ)															*/
/*==============================================================================*/
C_EXTERN INT32 OSW_EVT_create( OSW_EVT_HANDLE *handle, UINT16 bit_cnt );
C_EXTERN void OSW_EVT_delete( OSW_EVT_HANDLE *handle );
C_EXTERN void OSW_EVT_set( OSW_EVT_HANDLE *handle, UINT16 bit );
C_EXTERN UINT16 OSW_EVT_wait( OSW_EVT_HANDLE *handle, UINT32 time_out );

/*==============================================================================*/
/* API(タイマー)																*/
/*==============================================================================*/
C_EXTERN UINT32 OSW_TIM_value( void );

/*==============================================================================*/
/* API(割り込み)																*/
/*==============================================================================*/
typedef void (*osw_isr_func)( void );
C_EXTERN INT32 OSW_ISR_create( OSW_ISR_HANDLE *handle, UINT16 interrupt_id, osw_isr_func func );
C_EXTERN void OSW_ISR_delete( OSW_ISR_HANDLE *handle );
C_EXTERN void OSW_ISR_enable( UINT16 interrupt_id );
C_EXTERN void OSW_ISR_disable( UINT16 interrupt_id );
C_EXTERN void OSW_ISR_global_enable( void );
C_EXTERN void OSW_ISR_global_restore( UINT32 isr_stat );
C_EXTERN UINT32 OSW_ISR_global_disable( void );
C_EXTERN INT32 OSW_ISR_global_status( void );

/*==============================================================================*/
/* API(キャッシュ制御)															*/
/*==============================================================================*/
C_EXTERN void OSW_CAC_writeclean( void *addr, UINT32 size, INT32 wait );
C_EXTERN void OSW_CAC_write( void *addr, UINT32 size, INT32 wait );
C_EXTERN void OSW_CAC_clean( void *addr, UINT32 size, INT32 wait );

/*==============================================================================*/
/* API(ヒープ)																	*/
/*==============================================================================*/
C_EXTERN INT32 OSW_MEM_create( OSW_MEM_HANDLE *handle, void *buf, UINT32 size );
C_EXTERN void OSW_MEM_delete( OSW_MEM_HANDLE *handle );
C_EXTERN void * OSW_MEM_alloc( OSW_MEM_HANDLE *handle, UINT32 size, UINT32 align );
C_EXTERN void OSW_MEM_free( OSW_MEM_HANDLE *handle, void *mem, UINT32 size );

/*==============================================================================*/
/* API(システム制御)															*/
/*==============================================================================*/
C_EXTERN void OSW_SYS_init( osw_tsk_func func, UINT32 tsk_priority, UINT32 stack_size, void *arg );
C_EXTERN void OSW_SYS_log_start( void );

/*==============================================================================*/
/* ユーザーAPI																	*/
/*==============================================================================*/
C_EXTERN void OSW_USR_logout( INT8 *c, UINT32 length );


#endif /* __J_OSWIF__ */









