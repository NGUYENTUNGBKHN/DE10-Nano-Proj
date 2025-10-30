/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: MMC driver															*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_MMC__
#define __J_MMC__


/*==============================================================================*/
/* バージョン情報																*/
/*==============================================================================*/
#define	MMC_VER				100

/*==============================================================================*/
/* ハンドル定義																	*/
/*==============================================================================*/
typedef struct {
	void *hdl;
} MMC_HANDLE;

/*==============================================================================*/
/* 各種デファイン(変更可能)														*/
/*==============================================================================*/
#define	MMC_ACCESS_TOUT			(4000)		/* タイムアウト */
#define	MMC_ACCESS_TOUT_CNT		(10000000)	/* タイムアウトカウンタ */

/*==============================================================================*/
/* API引数用定義																*/
/*==============================================================================*/

typedef struct {
	UINT8 port;
} MMC_PARAM;

/* card_type */
#define	MMC_INFO_TYPE_UNKNOWN	0x00
#define	MMC_INFO_TYPE_MEM		0x01

typedef struct {
	UINT8 card_type;
	UINT32 sector_cnt;
} MMC_INFO;

/*==============================================================================*/
/* API																			*/
/*==============================================================================*/
/* Mmc_ident->attr */
#define	MMC_ATTR_INS			0x01	/* カード挿入有効 */
#define	MMC_ATTR_WP				0x02	/* ライトプロテクト有効 */

/* Mmc_read(),Mmc_write() return */
#define	MMC_RWSTAT_OK			0		/* 正常終了 */
#define	MMC_RWSTAT_ERR_CARD		1		/* カード判定不可 */
#define	MMC_RWSTAT_ERR_DATA		2		/* データ転送エラー */
#define	MMC_RWSTAT_ERR_WP		3		/* ライトプロテクトエラー */

INT8 Mmc_open( MMC_HANDLE *handle, MMC_PARAM *param );
void Mmc_close( MMC_HANDLE *handle );
INT8 Mmc_ident( MMC_HANDLE *handle, UINT8 attr, MMC_INFO *info );
INT8 Mmc_mem_read( MMC_HANDLE *handle, UINT8 *buf, UINT32 lba, UINT32 blk );
INT8 Mmc_mem_write( MMC_HANDLE *handle, UINT8 *buf, UINT32 lba, UINT32 blk );
INT8 Mmc_init( OSW_MEM_HANDLE *mem_handle );


/*==============================================================================*/
/* 外部で宣言するコンフィグレーション定数										*/
/*==============================================================================*/

extern const UINT32 CFG_MMC_SYSCLK[];		/* ペリフェラルシステムクロック周波数(Hz) */


#endif /* __J_MMC__ */









