/*==============================================================================*/
/* Copyright (C) 2011 JSL Technology. All right reserved.						*/
/* Tittle: UART driver															*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_UART__
#define __J_UART__


/*==============================================================================*/
/* バージョン情報																*/
/*==============================================================================*/
#define	UART_VER				100

/*==============================================================================*/
/* ハンドル定義																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} UART_HANDLE;

/*==============================================================================*/
/* 各種デファイン(変更可能)														*/
/*==============================================================================*/
#define	UART_ACCESS_TOUT		(50)		/* タイムアウト */
#define	UART_ACCESS_TOUT_CNT	(1000000)	/* タイムアウトカウンタ */

/*==============================================================================*/
/* API引数用定義																*/
/*==============================================================================*/

/* recv_cb_func()->stat */
#define	UART_STAT_RECV			0x1
#define	UART_STAT_TRANSRDY		0x2

typedef struct {
	UINT8 port;
	void (*recv_cb_func)( UINT8 c, UINT8 stat, void *arg );
	void *cb_arg;
} UART_PARAM;

/*==============================================================================*/
/* API																			*/
/*==============================================================================*/
INT8 Uart_open( UART_HANDLE *handle, UART_PARAM *param );
void Uart_close( UART_HANDLE *handle );
INT8 Uart_send( UART_HANDLE *handle, UINT8 c, INT8 wait );
INT8 Uart_init( OSW_MEM_HANDLE *mem_handle );


/*==============================================================================*/
/* 外部で宣言するコンフィグレーション定数										*/
/*==============================================================================*/

/* CFG_UART_FMT */
#define	UART_FMT_RXEN				0x0200
#define	UART_FMT_TXEN				0x0100
#define	UART_FMT_5BIT				0x0000
#define	UART_FMT_6BIT				0x0020
#define	UART_FMT_7BIT				0x0040
#define	UART_FMT_8BIT				0x0060
#define	UART_FMT_STOPBIT1			0x0000
#define	UART_FMT_STOPBIT2			0x0008
#define	UART_FMT_PARITY_ODD			0x0002
#define	UART_FMT_PARITY_EVEN		0x0006

extern const UINT32 CFG_UART_SYSCLK[];		/* ペリフェラルシステムクロック周波数(Hz) */
extern const UINT32 CFG_UART_BPS[];			/* UART目標bps */
extern const UINT16 CFG_UART_FMT[];			/* UARTフォーマット */


#endif /* __J_UART__ */









