/*==============================================================================*/
/* Copyright (C) 2012 JSL Technology. All right reserved.						*/
/* Tittle: SPI driver															*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_SPI__
#define __J_SPI__


/*==============================================================================*/
/* バージョン情報																*/
/*==============================================================================*/
#define	SPI_VER				100

/*==============================================================================*/
/* ハンドル定義																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} SPI_HANDLE;

/*==============================================================================*/
/* 各種デファイン(変更可能)														*/
/*==============================================================================*/
#define	SPIM_ACCESS_TOUT		(1000*4)	/* タイムアウト */
#define	SPIM_ACCESS_TOUT_CNT	(10000000)	/* タイムアウトカウンタ */

/*==============================================================================*/
/* API引数用定義																*/
/*==============================================================================*/
/* mode */
#define SPI_MODE_MASTER			0x00	/* Master Mode */
#define SPI_MODE_SLAVE			0x01	/* Slave Mode */

/* opt(Master Only) */
#define	SPIM_OPT_PACK_END		0x01	/* 出力後、CSをDisableにする */
#define SPIM_OPT_COND			0x02	/* recvデータがcondの値になるまで待ってから */
										/* バッファ入力する */
#define	SPIM_8BIT_GRAN			0x00	/* 8bit転送 */
#define	SPIM_16BIT_GRAN			0x10	/* 16bit転送 */
#define	SPIM_32BIT_GRAN			0x20	/* 32bit転送 */

typedef struct {
	UINT8 port;
	UINT8 mode;
} SPI_PARAM;

/* Master Only */
typedef struct {
	UINT8 opt;
	UINT8 *send;			/* NULL指定の時、0データ出力 */
	UINT8 *recv;			/* NULL指定の時、受信データはバッファに出力しない */
	UINT16 len;				/* ワード長 */
	UINT8 cond;				/* 待ち条件 */
} SPIM_SEND;

/*==============================================================================*/
/* API																			*/
/*==============================================================================*/
INT8 Spi_open( SPI_HANDLE *handle, SPI_PARAM *param );
void Spi_close( SPI_HANDLE *handle );
INT8 Spi_send( SPI_HANDLE *handle, UINT8 ch, SPIM_SEND *send );
INT8 Spi_init( OSW_MEM_HANDLE *mem_handle );


/*==============================================================================*/
/* 外部で宣言するコンフィグレーション定数										*/
/*==============================================================================*/

/* CFG_SPIM_FMT */
#define	SPIM_FMT_CLK_H_ACTIVE		0x01
#define	SPIM_FMT_CLK_L_ACTIVE		0x00
#define	SPIM_FMT_STB_H_ACTIVE		0x02
#define	SPIM_FMT_STB_L_ACTIVE		0x00

extern const UINT32 CFG_SPIM_SYSCLK[];		/* ペリフェラルシステムクロック周波数(Hz) */
extern const UINT16 *CFG_SPIM_STBGPIOID[];	/* STB端子用GPIO_ID */
extern const UINT8 CFG_SPIM_FMT[];			/* SPI出力フォーマット */
extern const UINT32 CFG_SPIM_CLK[];			/* SPI目標ビットCLK周波数 */


#endif /* __J_SPI__ */









