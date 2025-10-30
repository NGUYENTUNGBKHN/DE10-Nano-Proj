/*==============================================================================*/
/* Copyright (C) 2018 JCM Co., Ltd. All right reserved.		      				*/
/* Tittle: SPI F-RAM driver														*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_SPI_FRAM__
#define __J_SPI_FRAM__

//#include "../spi_flash/js_spi_flash.h"

/*==============================================================================*/
/* バージョン情報																*/
/*==============================================================================*/
#define	SPI_FRAM_VER			103

/*==============================================================================*/
/* ハンドル定義																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} SPI_FRAM_HANDLE;


/*==============================================================================*/
/* 各種デファイン(変更可能)														*/
/*==============================================================================*/
#define	SPI_FRAM_ACCESS_TOUT	(1000*10)	/* Readアクセスタイムアウト */
#define	SPI_FRAM_WRITE_TOUT	    (1000*10)	/* Writeタイムアウト */
#define	SPI_FRAM_CERASE_TOUT	(60000*10)	/* チップイレースタイムアウト */
#define	SPI_FRAM_INTERVAL		(10)		/* 書き込み中の連続CPU占有時間 */

/*==============================================================================*/
/* API引数用定義																*/
/*==============================================================================*/

/* status_cb_func()->stat */
#define	SPI_FRAM_STAT_ERASING	0x01
#define	SPI_FRAM_STAT_WRITEADDR	0x02
#define	SPI_FRAM_STAT_ERASE_ERR	0xE1
#define	SPI_FRAM_STAT_WRITE_ERR	0xE2

typedef struct {
	UINT8 port;
	void (*status_cb_func)( UINT8 stat, UINT32 addr, void *arg );
	void *cb_arg;
} SPI_FRAM_PARAM;

/*==============================================================================*/
/* API																			*/
/*==============================================================================*/

typedef struct {
	UINT8 *buf;
	UINT32 addr;
	UINT32 len;
	UINT32 *byte_count;
} SPI_BUF_INFO;

/* SPI_FRAM_Write()->erase */
#define	SPI_FRAM_ERASE_NONE		0
#define	SPI_FRAM_ERASE_AUTO		1
#define	SPI_FRAM_ERASE_ONLY		2

INT8 SPI_FRAM_open( SPI_FRAM_HANDLE *handle, SPI_FRAM_PARAM *param, UINT32 *size );
void SPI_FRAM_close( SPI_FRAM_HANDLE *handle );
INT8 SPI_FRAM_Read( SPI_FRAM_HANDLE *handle, SPI_BUF_INFO *buf );
INT8 SPI_FRAM_Write( SPI_FRAM_HANDLE *handle, SPI_BUF_INFO *buf );
INT8 SPI_FRAM_init( OSW_MEM_HANDLE *mem_handle );


/*==============================================================================*/
/* 外部で宣言するコンフィグレーション定数										*/
/*==============================================================================*/

/* spi_send_func()->smode */
#define	SPI_FRAM_SMODE_READ		    0x1
#define	SPI_FRAM_SMODE_WRITE		0x2
#define	SPI_FRAM_SMODE_PACKEND		0x4

typedef INT8 (*spi_send_func)( UINT8 n, UINT8 *buf, UINT16 len, UINT8 smode );

extern const spi_send_func CFG_SPI_FRAM_SEND_FUNC[];	/* SPI FRAMアクセス関数 */

extern const UINT32 *CFG_SPI_FRAM_INFO[];				/* SPI F-RAMセクター情報 */


#endif /* __J_SPI_FRAM__ */






