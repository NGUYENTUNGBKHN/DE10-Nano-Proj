/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/

#include "ff.h"		// FatFs, 19/07/16
#include "diskio.h"		// FatFs low level ディスクインタフェース, 19/07/16
#include "js_oswapi.h"
#include "js_io.h"
#include "js_gpio.h"
#if defined(JSL_MMC)
#include "js_mmc.h"
#endif
#include "alt_sdmmc.h"
#include "hal_gpio_reg.h"
#include "js_rstmgr_reg.h"
#include "system_timer.h"
//#include <stdbool.h>
#include <string.h>
#include <time.h>

#if 0 //2024-01-31
/*==============================================================================*/
/* SDMMCドライバ設定用コンフィグレーション定数									*/
/*==============================================================================*/
#if defined(JSL_MMC)
const UINT32 CFG_MMC_SYSCLK[] =			/* ペリフェラルシステムクロック周波数(Hz) */
{
	// ESmodelではsdmmc_clk_diveded50MHzで動作せず,ソース100Mhzにして半分の速度(12.5Mhz,HS25.0Mhz)では動作
	// WSmodelではsdmmc_clk_diveded50MHzで動作せず,ソース100Mhzにして半分の速度(12.5Mhz,HS25.0Mhz)では動作
	//200000000		// 100MHzに変更(sdmmc_clkを200MHzに変更、4分周したsdmmc_clk_diveded(50MHz)を指定)
	100000000		// 100MHzに変更(sdmmc_clkを200MHzに変更、4分周したsdmmc_clk_diveded(50MHz)を指定)
	//50000000		// 50MHzに変更(sdmmc_clkを200MHzに変更、4分周したsdmmc_clk_diveded(50MHz)を指定)
};

struct {
	MMC_HANDLE handle;
	MMC_INFO info;
} sdmmc;
#else
extern ALT_SDMMC_CARD_INFO_t card_info;
extern ALT_SDMMC_CARD_MISC_t card_misc_cfg;
#endif

// SDMMCカード制御信号
//#define SDMMC_DETECT GPIO_SD_SET		// 0:カード無し、1:カード有り
#define SDMMC_WP GpioID(0, 9)		// 0:ライトプロテクト、1:ライト可能

// カード検知
bool sdmmc_get_detect(void)
{
	return (!Gpio_in(SDMMC_DETECT) ? true : false);
}

// ライトプロテクト設定
bool sdmmc_get_wp(void)
{
	return (Gpio_in(SDMMC_WP) == 0 ? true : false);
}

/*==============================================================================*/
/* ファイルシステム下位インターフェース											*/
/*==============================================================================*/
bool sdmmc_init(void)
{
#if defined(JSL_MMC)
	RSTMGR_PERMODRST_T permodrst = {0};
	MMC_PARAM param = {0};
	bool ret = TRUE;
	
	memset(&sdmmc, 0, sizeof(sdmmc));
	
	/* SDカード制御ピンを入力に設定 */
	Gpio_mode(SDMMC_DETECT, GPIO_MODE_INPUT);
	
	// SDモジュール, 19/07/16
	permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);
	permodrst.bit.sdmmc = 1;		// リセットビットセット
	IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;
	OSW_TSK_sleep(1);
	permodrst.bit.sdmmc = 0;		// リセットビットクリア
	IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;
	
	ret = Mmc_init(NULL);		// ドライバ初期化
	
	if (ret != FALSE) {		// ドライバオープンOKのとき
		memset((void *)&param, 0, sizeof(param));
		param.port = 0;
		ret = Mmc_open(&sdmmc.handle, &param);		// ドライバオープン
	}
	alt_sdmmc_card_pwr_off();
	return ret;
#else
	ALT_STATUS_CODE status;
	/* SDカード制御ピンを入力に設定 */
	Gpio_mode(SDMMC_DETECT, GPIO_MODE_INPUT);
	// SDモジュール
	status = _hal_sdmmc_init();
    if (status != ALT_E_SUCCESS) {
    	return false;
    }
    return true;
#endif
}


/*==============================================================================*/
/* ファイルシステム下位インターフェース											*/
/*==============================================================================*/
void sdmmc_term(void)
{
#if defined(JSL_MMC)
#else
	alt_sdmmc_uninit();
#endif
}


/*==============================================================================*/
/* ファイルシステム下位インターフェース											*/
/*==============================================================================*/
DSTATUS disk_initialize(BYTE pdrv)
{
#if defined(JSL_MMC)
	UINT8 attr = 0;
	INT8 ret = TRUE;
	
	attr = (sdmmc_get_detect() ? MMC_ATTR_INS : 0);		// カード検知
	attr |= (sdmmc_get_wp() ? MMC_ATTR_WP : 0);		// ライトプロテクト設定
	ret = Mmc_ident(&sdmmc.handle, attr, &sdmmc.info);		// カード認識
	
	if (ret != FALSE) {		// SDカード認識OKのとき
		ret = (sdmmc.info.card_type == MMC_INFO_TYPE_MEM ? TRUE : FALSE);		// SDカードタイプ
	}
	
	if (ret != FALSE) {		// SDカードタイプOKのとき
		switch (pdrv) {
		case 0:
			if (sdmmc_get_detect()) {		// SDカード有り
				if (sdmmc_get_wp()) {		// ライトプロテクト
					return(STA_PROTECT);
				} else {		// 書込み可
					return(RES_OK);
				}
			}
			break;
		default:
			break;
		}
	}
	
	return(STA_NOINIT);
#else
	ALT_STATUS_CODE ret = ALT_E_SUCCESS;
	bool card_det;

	card_det = (sdmmc_get_detect() ? 1 : 0);		// カード検知

	// SDモジュール
	ret = _hal_sdmmc_init();

	if (ret == ALT_E_SUCCESS) {		// SDカードタイプOKのとき
		switch (pdrv) {
		case 0:
			if (sdmmc_get_detect()) {		// SDカード有り
				if (sdmmc_get_wp()) {		// ライトプロテクト
					return(STA_PROTECT);
				} else {		// 書込み可
					return(RES_OK);
				}
			}

		default:
			break;
		}
	}
	return(STA_NOINIT);

#endif
}


/*==============================================================================*/
/* ファイルシステム下位インターフェース											*/
/*==============================================================================*/
DSTATUS disk_status(BYTE pdrv)
{
	switch (pdrv) {
	case 0:
		if (sdmmc_get_detect()) {		// SDカード有り
			if (sdmmc_get_wp()) {		// ライトプロテクト
				return(STA_PROTECT);
			} else {		// 書込み可
				return(RES_OK);
			}
		}
		break;
	default:
		break;
	}
	
	return(STA_NOINIT);
}


/*==============================================================================*/
/* ファイルシステム下位インターフェース											*/
/*==============================================================================*/
DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count)
{
#if defined(JSL_MMC)
	switch (pdrv) {
	case 0:
		if (Mmc_mem_read(&sdmmc.handle, (UINT8 *)buff, (UINT32)sector, (UINT32)count) != MMC_RWSTAT_OK) {
			return(RES_ERROR);
		}
		
		return(RES_OK);
		
	default:
		break;
	}
	
	return(RES_PARERR);
#else
	switch (pdrv) {
	case 0:
		if (alt_sdmmc_read(&card_info, (UINT8 *)buff, (void *)(sector), (UINT32)count * 512) != ALT_E_SUCCESS) {
			return(RES_ERROR);
		}

		return(RES_OK);

	default:
		break;
	}
#endif
}


/*==============================================================================*/
/* ファイルシステム下位インターフェース											*/
/*==============================================================================*/
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count)
{
#if defined(JSL_MMC)
	switch (pdrv) {
	case 0:
		if (sdmmc_get_wp()) {		// ライトプロテクト
			return(RES_WRPRT);
		}
		
		if (Mmc_mem_write(&sdmmc.handle, (UINT8 *)buff, (UINT32)sector, (UINT32)count) != MMC_RWSTAT_OK) {
			return(RES_ERROR);
		}
		
		return(RES_OK);
		
	default:
		break;
	}
	
	return(RES_PARERR);
#else
	switch (pdrv) {
	case 0:
		if (sdmmc_get_wp()) {		// ライトプロテクト
			return(RES_WRPRT);
		}

		if (alt_sdmmc_write(&card_info, (UINT8 *)buff, (void *)(sector/512), (UINT32)count * 512) != ALT_E_SUCCESS) {
			return(RES_ERROR);
		}

		return(RES_OK);

	default:
		break;
	}

	return(RES_PARERR);
#endif
}


/*==============================================================================*/
/* ファイルシステム下位インターフェース											*/
/*==============================================================================*/
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
#if defined(JSL_MMC)
	switch (pdrv) {
	case 0:
		switch (cmd) {
		case CTRL_SYNC:
			break;
			
		case GET_SECTOR_SIZE:
			*((WORD *)buff) = 512;
			break;
			
		case GET_SECTOR_COUNT:
			*((DWORD *)buff) = sdmmc.info.sector_cnt;
			break;
			
		case GET_BLOCK_SIZE:
			*((DWORD *)buff) = 1;
			break;
			
		case CTRL_TRIM:
			break;
			
		default:
			break;
		}
		
		return(RES_OK);
		
	default:
		break;
	}
	
	return(RES_PARERR);
#else
	switch (pdrv) {
	case 0:
		switch (cmd) {
		case CTRL_SYNC:
			break;

		case GET_SECTOR_SIZE:
			*((WORD *)buff) = card_misc_cfg.block_size;
			break;

		case GET_SECTOR_COUNT:
			*((WORD *)buff) = card_info.blk_number_low;
			break;
		case GET_BLOCK_SIZE:
			*((DWORD *)buff) = 1;
			break;

		case CTRL_TRIM:
			break;

		default:
			break;
		}

		return(RES_OK);

	default:
		break;
	}

	return(RES_PARERR);
#endif
}


/*==============================================================================*/
/* ファイルシステム下位インターフェース											*/
/*==============================================================================*/
DWORD get_fattime(void)
{
	union {
		DWORD dword;
		struct {
			DWORD sec:5;		// 秒(0～29、実秒の1/2
			DWORD min:6;		// 分(0～59)
			DWORD hour:5;		// 時(0～23)
			DWORD mday:5;		// 日(1～31)
			DWORD mon:4;		// 月(1～12)
			DWORD year:7;		// 1980年からの経過年
		} bit;
	} fattime = {0};
	struct tm time = {0};
	
	if (system_timer_get_localtime(&time) != NULL) {
		fattime.bit.sec = time.tm_sec / 2;		// 秒を実秒の1/2に変換
		fattime.bit.min = time.tm_min;
		fattime.bit.hour = time.tm_hour;
		fattime.bit.mday = time.tm_mday;
		fattime.bit.mon = time.tm_mon;
		fattime.bit.year = time.tm_year - 1980;		// 1980年からの経過年に変換
	}
	
	return fattime.dword;
}


/*==============================================================================*/
/* ファイルシステム外部インターフェース											*/
/*==============================================================================*/
#if _FS_REENTRANT
int ff_cre_syncobj(BYTE vol, _SYNC_t *sobj)
{
	OSW_SEM_HANDLE *hSem;
	
	hSem = OSW_MEM_alloc(NULL, sizeof(OSW_SEM_HANDLE), 4);
	
	if (hSem == NULL) {
		return(0);
	}
	
	if (OSW_SEM_create(hSem, 1) == FALSE) {
		OSW_MEM_free(NULL, (void *)hSem, sizeof(OSW_SEM_HANDLE));
		return(0);
	}
	
	*sobj = (_SYNC_t)hSem;
	
	return(1);
}


/*==============================================================================*/
/* ファイルシステム外部インターフェース											*/
/*==============================================================================*/
int ff_del_syncobj(_SYNC_t sobj)
{
	OSW_SEM_HANDLE *hSem = (OSW_SEM_HANDLE *)sobj;
	OSW_SEM_delete(hSem);
	OSW_MEM_free(NULL, (void *)hSem, sizeof(OSW_SEM_HANDLE));
	return(1);
}


/*==============================================================================*/
/* ファイルシステム外部インターフェース											*/
/*==============================================================================*/
int ff_req_grant(_SYNC_t sobj)
{
	OSW_SEM_HANDLE *hSem = (OSW_SEM_HANDLE *)sobj;
	
	if (OSW_SEM_pend(hSem, _FS_TIMEOUT) == FALSE) {
		return(0);
	}
	
	return(1);
}


/*==============================================================================*/
/* ファイルシステム外部インターフェース											*/
/*==============================================================================*/
void ff_rel_grant(_SYNC_t sobj)
{
	OSW_SEM_HANDLE *hSem = (OSW_SEM_HANDLE *)sobj;
	OSW_SEM_post(hSem);
}
#endif


#if _USE_LFN == 3
/*==============================================================================*/
/* ファイルシステム外部インターフェース											*/
/*==============================================================================*/
void *ff_memalloc(UINT msize)
{
	return(OSW_MEM_alloc(NULL, msize, 1));
}


/*==============================================================================*/
/* ファイルシステム外部インターフェース											*/
/*==============================================================================*/
void ff_memfree(void *mblock)
{
	OSW_MEM_free(NULL, mblock, sizeof(OSW_SEM_HANDLE));
}
#endif


#endif
