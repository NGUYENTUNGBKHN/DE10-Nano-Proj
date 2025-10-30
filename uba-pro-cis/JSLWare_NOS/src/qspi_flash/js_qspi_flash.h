/*==============================================================================*/
/* Copyright (C) 2012 JSL Technology. All right reserved.						*/
/* Tittle: QSPI Flash driver														*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_QSPI_FLASH__
#define __J_QSPI_FLASH__


/*==============================================================================*/
/* バージョン情報																*/
/*==============================================================================*/
#define	QSPI_FLASH_VER			102

/*==============================================================================*/
/* ハンドル定義																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} QSPI_FLASH_HANDLE;


/*==============================================================================*/
/* 各種デファイン(変更可能)														*/
/*==============================================================================*/
#define	QSPI_FLASH_ACCESS_TOUT	(1000*10)	/* Readアクセスタイムアウト */
#define	QSPI_FLASH_WRITE_TOUT	(1000*10)	/* Writeタイムアウト */
#define	QSPI_FLASH_CERASE_TOUT	(60000*10)	/* チップイレースタイムアウト */
#define	QSPI_FLASH_INTERVAL		(10)		/* 書き込み中の連続CPU占有時間 */

/*==============================================================================*/
/* API引数用定義																*/
/*==============================================================================*/

/* status_cb_func()->stat */
#define	QSPI_FLASH_STAT_ERASING		0x01
#define	QSPI_FLASH_STAT_WRITEADDR	0x02
#define	QSPI_FLASH_STAT_ERASE_ERR	0xE1
#define	QSPI_FLASH_STAT_WRITE_ERR	0xE2

typedef struct {
	UINT8 port;
	void (*status_cb_func)( UINT8 stat, UINT32 addr, void *arg );
	void *cb_arg;
} QSPI_FLASH_PARAM;

/*==============================================================================*/
/* API																			*/
/*==============================================================================*/

typedef struct {
	UINT8 *buf;
	UINT32 addr;
	UINT32 len;
	UINT32 *byte_count;
} QSPI_BUF_INFO;

/* QSPI_Flash_Write()->erase */
#define	QSPI_FLASH_ERASE_NONE		0
#define	QSPI_FLASH_ERASE_AUTO		1
#define	QSPI_FLASH_ERASE_ONLY		2

INT8 QSPI_Flash_open( QSPI_FLASH_HANDLE *handle, QSPI_FLASH_PARAM *param, UINT32 *size );
void QSPI_Flash_close( QSPI_FLASH_HANDLE *handle );
INT8 QSPI_Flash_Read( QSPI_FLASH_HANDLE *handle, QSPI_BUF_INFO *buf );
INT8 QSPI_Flash_Write( QSPI_FLASH_HANDLE *handle, QSPI_BUF_INFO *buf, INT8 erase );
INT8 QSPI_Flash_ChipErase( QSPI_FLASH_HANDLE *handle );
INT8 QSPI_Flash_init( OSW_MEM_HANDLE *mem_handle );


/*==============================================================================*/
/* 外部で宣言するコンフィグレーション定数										*/
/*==============================================================================*/

/* CFG_QSPI_ATTR */
#define	QSPI_ATTR_TYPE_MICRON			0x0003
#define	QSPI_ATTR_TYPE_SPANCION			0x0004
#define QSPI_ATTR_BULK_ERACE_CODE(n)	((UINT32)n<<24)

extern const UINT32 CFG_QSPI_SYSCLK[];					/* ペリフェラルシステムクロック周波数(Hz) */
extern const UINT32 CFG_QSPI_CLK[];						/* QSPI目標ビットCLK周波数 */
extern const UINT32 CFG_QSPI_ATTR[];					/* QSPI Flash動作モード */
extern const UINT32 CFG_QSPI_FLASH_PAGE_SIZE[];			/* QSPI Flashページサイズ */
extern const UINT32 *CFG_QSPI_FLASH_INFO[];				/* QSPI Flashセクター情報 */


#endif /* __J_QSPI_FLASH__ */






