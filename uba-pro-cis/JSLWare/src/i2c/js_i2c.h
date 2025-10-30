/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: I2C driver															*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_I2C__
#define __J_I2C__


/*==============================================================================*/
/* バージョン情報																*/
/*==============================================================================*/
#define	I2C_VER				100

/*==============================================================================*/
/* ハンドル定義																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} I2C_HANDLE;

/*==============================================================================*/
/* 各種デファイン(変更可能)														*/
/*==============================================================================*/
#if 1
#define	I2C_ACCESS_TOUT		(5)		/* タイムアウト */
#define	I2C_ACCESS_TOUT_CNT	(10000)	/* タイムアウトカウンタ */
#else
#define	I2C_ACCESS_TOUT		(1000)		/* タイムアウト */
#define	I2C_ACCESS_TOUT_CNT	(10000000)	/* タイムアウトカウンタ */
#endif

/*==============================================================================*/
/* API引数用定義																*/
/*==============================================================================*/
#define _FAST_MODE_ENABLE 1
/* opt */
#define	I2C_OPT_MASTER		0x00
#define	I2C_OPT_SLAVE		0x01
#if (_FAST_MODE_ENABLE==1) /* jcm mod fast mode対応(マスターのみ) */
#define	I2C_OPT_MASTER_FS	0x02
#endif

typedef struct {
	UINT8 port;
	UINT8 opt;
	/* 以下I2C_OPT_SLAVE Only */
	UINT8 slave_address;	/* スレーブアドレス */
	UINT8 *buf;				/* 送受信バッファ */
	UINT16 buf_len;			/* 送受信バッファサイズ */
	UINT16 (*recv_cb_func)( UINT8 *buf, UINT16 recv_len, UINT16 max );	/* 受信コールバック */
	/* ↑レスポンスバイト数を返す */
} I2C_PARAM;

typedef struct {
	UINT8 address;			/* 送信先アドレス */
	const UINT8 *write_dat;
	UINT16 write_len;		/* Write数 */
	UINT8 *read_dat;
	UINT16 read_len;		/* Read数 */
} I2C_SEND_PACK;

/* write_len + read_lenの両方を指定した場合は、	*/
/* Write → Restert → Read の様に実行する		*/

/*==============================================================================*/
/* API																			*/
/*==============================================================================*/
INT8 I2c_open( I2C_HANDLE *handle, I2C_PARAM *param );
void I2c_close( I2C_HANDLE *handle );
INT8 I2c_send( I2C_HANDLE *handle, I2C_SEND_PACK *send );
INT8 I2c_init( OSW_MEM_HANDLE *mem_handle );


/*==============================================================================*/
/* 外部で宣言するコンフィグレーション定数										*/
/*==============================================================================*/

extern const UINT32 CFG_I2C_SYSCLK[];		/* ペリフェラルシステムクロック周波数(Hz) */
extern const UINT32 CFG_I2C_CLK[];			/* I2C目標CLK周波数 */


#endif /* __J_I2C__ */









