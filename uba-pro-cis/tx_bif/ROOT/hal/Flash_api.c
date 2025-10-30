/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      Flash_api.c                                                             1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      紙幣識別機アプリケーション用QSPI Flashメモリインターフェース                            */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Kanamaru     2018/03/06  V1.00                                                           */
/*                              Created initial version                                         */
/*                                                                                              */
/************************************************************************************************/
/**** INCLUDE FILES *****************************************************************************/

//#include <stdio.h>
//#include <string.h>
//#include <stdarg.h>

#include "systemdef.h"

#include "js_oswapi.h"
#include "js_io.h"
#include "js_qspi_flash.h"
#include "Flash_api.h"

/**** INTERNAL DATA DEFINES *********************************************************************/


/**** STRUCTURES ********************************************************************************/


/**** VARIABLE DEFINES **************************************************************************/


typedef enum {
	QSPI_FLASH_FAST_READ = 0x0B,
	QSPI_FLASH_DUAL_IO_FAST_READ = 0xBB,
	QSPI_FLASH_QUAD_IO_FAST_READ = 0xEB,
	QSPI_FLASH_PAGE_PROGRAM = 0x02,
	QSPI_FLASH_EXTENDED_DUAL_INPUT_FAST_PROGRAM = 0xD2,
	QSPI_FLASH_EXTENDED_QUAD_INPUT_FAST_PROGRAM = 0x38,

	QSPI_FLASH_4BYTE_FAST_READ = 0x0C,
	QSPI_FLASH_4BYTE_DUAL_IO_FAST_READ = 0xBC,
	QSPI_FLASH_4BYTE_QUAD_IO_FAST_READ = 0xEC,
	QSPI_FLASH_4BYTE_PAGE_PROGRAM = 0x12,
	QSPI_FLASH_4BYTE_EXTENDED_QUAD_INPUT_FAST_PROGRAM = 0x3E,

	QSPI_FLASH_WRITE_ENABLE = 0x06,
	QSPI_FLASH_WRITE_DISABLE = 0x04,

	QSPI_FLASH_ENTER_4BYTE_ADDRESS = 0xB7,
	QSPI_FLASH_EXIT_4BYTE_ADDRESS = 0xE9,

	QSPI_FLASH_ERASE_4KSEC = 0x20,
	QSPI_FLASH_ERASE_32KSEC = 0x52,
	QSPI_FLASH_ERASE_SEC = 0xD8,
	QSPI_FLASH_ERASE_BULK = 0xC7,

	QSPI_FLASH_STATUS_READ = 0x05,
	QSPI_FLASH_FLAG_STATUS_READ = 0x70,
	QSPI_FLASH_NONVOLATILE_CONFIGURATION_READ = 0xb5,
	QSPI_FLASH_VOLATILE_CONFIGURATION_READ = 0x85,
	QSPI_FLASH_ENHANCED_VOLATILE_CONFIGURATION_READ = 0x65,
	QSPI_FLASH_EXTENDED_ADDRESS_READ = 0xc8,
	QSPI_FLASH_STATUS_WRITE = 0x01,
	QSPI_FLASH_NONVOLATILE_CONFIGURATION_WRITE = 0xB1,
	QSPI_FLASH_VOLATILE_CONFIGURATION_WRITE = 0x81,
	QSPI_FLASH_ENHANCED_VOLATILE_CONFIGURATION_WRITE = 0x61,
	QSPI_FLASH_EXTENDED_ADDRESS_WRITE = 0xC5,
} QSPI_FLASH_CMD;
/*==============================================================================*/
/* QSPIドライバ設定用コンフィグレーション定数                                   */
/*==============================================================================*/
const UINT32 CFG_QSPI_SYSCLK[] =        /* ペリフェラルシステムクロック周波数(Hz) */
{
    400000000
};

const UINT32 CFG_QSPI_CLK[] =           /* QSPI目標CLK周波数 */
{
	20000000
};

const UINT32 CFG_QSPI_ATTR[] =          /* QSPI Flash動作モード */
{
    QSPI_ATTR_TYPE_MICRON
};
const UINT32 CFG_QSPI_FLASH_PAGE_SIZE[] =
{
    256
};
static const UINT32 qspi_flash0[] =
{
    /* Sector Size  /   Count               / Erase Code */
    FLASH_SECTOR_SIZE,  FLASH_SECTOR_NUMBER,  FLASH_ERASE_CODE,
    0               /* <-End */
};
const UINT32 *CFG_QSPI_FLASH_INFO[] =   /* QSPI Flashセクター情報 */
{
    qspi_flash0,    /* Flash0 */
    0               /* <-End */
};


/* FLASHドライバハンドラ */
static QSPI_FLASH_HANDLE   l_hFlash;

/**** FUNCTION PROTOTYPES ***********************************************************************/
int     Flash_Drv_Initialize( void );
void    Flash_Drv_Deinitialize( void );
int     Flash_Drv_Erase( unsigned long ulRomAddr, unsigned long ulSize );
int     Flash_Drv_Write( unsigned long ulRomAddr, unsigned char *pucRamAdd, unsigned long ulSize );
int     Flash_Drv_EraseAndWrite( unsigned long ulRomAddr, unsigned char *pucRamAdd, unsigned long ulSize );
int     Flash_Drv_Read( unsigned long ulRomAddr, unsigned char *pucRamAdd, unsigned long ulSize );

/**** INTERNAL FUNCTIONS ************************************************************************/
/************************************************************************************************/
/* FUNCTION   : _Flash_Drv_WriteAccess                                                          */
/*                                                                                              */
/* DESCRIPTION: QSPI Flashメモリ書き込み方向アクセス                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulRomAddr               書き込むFlashメモリのアドレス                           */
/*              pucRamAddr              書き込みデータを格納する領域のアドレス                  */
/*              ulSize                  書き込みデータのサイズ                                  */
/*              cMode                   書き込みモード                                          */
/* OUTPUT     : none                                                                            */
/* RESULTS    : FLASH_API_SUCCESS       正常終了                                                */
/*              FLASH_API_FAIL          処理失敗                                                */
/*                                                                                              */
/************************************************************************************************/
int _Flash_Drv_WriteAccess(unsigned long ulRomAddr,
                           unsigned char *pucRamAdd,
                           unsigned long ulSize,
                           char cMode)
{
    QSPI_BUF_INFO   tBufInfo;       /* Flash設定パラメータ      */
    UINT32          ulActualSize;   /* 実際に消去したサイズ*/
    int             iRet = FLASH_API_SUCCESS;
    INT8            cStat;

    /* パラメータチェック */
    if (FLASH_MAX_SIZE < (ulRomAddr + ulSize))
    {
        return FLASH_API_FAIL;
    }

    /* 書き込み処理には時間がかかるため、セクタ単位で分割して要求を行う */
    tBufInfo.buf    = pucRamAdd;
    tBufInfo.addr   = ulRomAddr;
    if (FLASH_SECTOR_SIZE < ulSize)
    {
        tBufInfo.len = FLASH_SECTOR_SIZE;
    }
    else
    {
        tBufInfo.len = ulSize;
    }
    tBufInfo.byte_count = &ulActualSize;

    while(ulSize > 0)
    {
        /* ブロック書き込み */
        cStat = QSPI_Flash_Write(&l_hFlash, &tBufInfo, cMode);
        if (FALSE == cStat)
        {
            /* プログラム領域消去失敗ならばエラー */
            iRet = FLASH_API_FAIL;
            break;
        }
        /* 残りサイズの更新 */
        if ( ulSize < ulActualSize )
        {
            ulSize          = 0;
            tBufInfo.len    = 0;
        }
        else
        {
            ulSize          -= ulActualSize;
            tBufInfo.addr   += ulActualSize;
            tBufInfo.buf    += ulActualSize;
            if (FLASH_SECTOR_SIZE < ulSize)
            {
                tBufInfo.len = FLASH_SECTOR_SIZE;
            }
            else
            {
                tBufInfo.len = ulSize;
            }
        }
    }

    return(iRet);
}

/**** FUNCTIONS *********************************************************************************/
/************************************************************************************************/
/* FUNCTION   : Flash_Drv_Initialize                                                            */
/*                                                                                              */
/* DESCRIPTION: QSPI Flashメモリドライバ初期化処理                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/* RESULTS    : FLASH_API_SUCCESS       正常終了                                                */
/*              FLASH_API_INIT_ERROR    初期化エラー                                            */
/*              FLASH_API_OPEN_ERROR    オープンエラー                                          */
/*                                                                                              */
/************************************************************************************************/
int Flash_Drv_Initialize( void )
{
    QSPI_FLASH_PARAM    tFlashPrm;  /* Flash設定パラメータ      */
    UINT32              ulSize;     /* サイズ値                 */
    INT8                cStat;      /* Flashドライバステータス  */

    /* QSPI Flashドライバを起動する    */
    cStat = QSPI_Flash_init( NULL );
    if( FALSE == cStat )
    {
        return(FLASH_API_INIT_ERROR);
    }
    memset( (void *)&tFlashPrm, 0, sizeof(tFlashPrm) );
    cStat = QSPI_Flash_open( &l_hFlash, &tFlashPrm, &ulSize );
    if( FALSE == cStat )
    {
        return(FLASH_API_OPEN_ERROR);
    }

    return(FLASH_API_SUCCESS);
}

/************************************************************************************************/
/* FUNCTION   : Flash_Drv_Deinitialize                                                          */
/*                                                                                              */
/* DESCRIPTION: QSPI Flashメモリドライバ終了処理                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void Flash_Drv_Deinitialize( void )
{
    /* QSPI Flashドライバを終了する    */
    QSPI_Flash_close( &l_hFlash );

    return;
}

/************************************************************************************************/
/* FUNCTION   : Flash_Drv_Erase                                                                 */
/*                                                                                              */
/* DESCRIPTION: QSPI Flashメモリブロック消去処理                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulRomAddr               消去するFlashメモリのアドレス                           */
/*              ulSize                  消去範囲のサイズ                                        */
/* OUTPUT     : none                                                                            */
/* RESULTS    : FLASH_API_SUCCESS       正常終了                                                */
/*              FLASH_API_ERASE_ERROR   消去エラー                                              */
/*                                                                                              */
/************************************************************************************************/
int Flash_Drv_Erase(unsigned long ulRomAddr, unsigned long ulSize)
{
    QSPI_BUF_INFO   tBufInfo;       /* Flash設定パラメータ      */
    int             iRet = FLASH_API_SUCCESS;
    INT8            cStat;

    /* パラメータチェック */
    if (FLASH_MAX_SIZE < (ulRomAddr + ulSize))
    {
        return FLASH_API_ERASE_ERROR;
    }

    /* 消去処理には時間がかかるため、セクタ単位で分割して要求を行う */
    tBufInfo.buf    = 0;
    tBufInfo.addr   = ulRomAddr;
    if (FLASH_SECTOR_SIZE < ulSize)
    {
        tBufInfo.len = FLASH_SECTOR_SIZE;
    }
    else
    {
        tBufInfo.len = ulSize;
    }
    tBufInfo.byte_count = 0;

    while(ulSize > 0)
    {
        /* ブロック書き込み */
        cStat = QSPI_Flash_Write(&l_hFlash, &tBufInfo, QSPI_FLASH_ERASE_ONLY);
        if (FALSE == cStat)
        {
            /* プログラム領域消去失敗ならばエラー */
            iRet = FLASH_API_ERASE_ERROR;
            break;
        }
        /* 残りサイズの更新 */
        if ( ulSize < FLASH_SECTOR_SIZE )
        {
            ulSize          = 0;
            tBufInfo.len    = 0;
        }
        else
        {
            ulSize          -= FLASH_SECTOR_SIZE;
            tBufInfo.addr   += FLASH_SECTOR_SIZE;
            tBufInfo.buf    += FLASH_SECTOR_SIZE;
            if (FLASH_SECTOR_SIZE < ulSize)
            {
                tBufInfo.len = FLASH_SECTOR_SIZE;
            }
            else
            {
                tBufInfo.len = ulSize;
            }
        }
    }

    return(iRet);
}


/************************************************************************************************/
/* FUNCTION   : Flash_Drv_Write                                                                 */
/*                                                                                              */
/* DESCRIPTION: QSPI Flashメモリブロック書き込み処理                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulRomAddr               書き込むFlashメモリのアドレス                           */
/*              pucRamAddr              書き込みデータを格納する領域のアドレス                  */
/*              ulSize                  書き込みデータのサイズ                                  */
/* OUTPUT     : none                                                                            */
/* RESULTS    : FLASH_API_SUCCESS       正常終了                                                */
/*              FLASH_API_WRITE_ERROR   書き込みエラー                                          */
/*                                                                                              */
/************************************************************************************************/
int Flash_Drv_Write(unsigned long ulRomAddr, unsigned char *pucRamAdd, unsigned long ulSize)
{
    int     iRet;

    /* 書き込みモードで書き込み方向アクセス処理を呼び出す */
    iRet = _Flash_Drv_WriteAccess(ulRomAddr, pucRamAdd, ulSize, QSPI_FLASH_ERASE_NONE);
    if (FLASH_API_SUCCESS != iRet)
    {
        iRet = FLASH_API_WRITE_ERROR;
    }

    return(iRet);
}

/************************************************************************************************/
/* FUNCTION   : Flash_Drv_EraseAndWrite                                                         */
/*                                                                                              */
/* DESCRIPTION: QSPI Flashメモリブロック消去＆書き込み処理                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulRomAddr               書き込むFlashメモリのアドレス                           */
/*              pucRamAddr              書き込みデータを格納する領域のアドレス                  */
/*              ulSize                  書き込みデータのサイズ                                  */
/* OUTPUT     : none                                                                            */
/* RESULTS    : FLASH_API_SUCCESS       正常終了                                                */
/*              FLASH_API_WRITE_ERROR   書き込みエラー                                          */
/*                                                                                              */
/************************************************************************************************/
int Flash_Drv_EraseAndWrite(unsigned long ulRomAddr, unsigned char *pucRamAdd, unsigned long ulSize)
{
    int     iRet;

    /* オートイレースモードで書き込み方向アクセス処理を呼び出す */
    iRet = _Flash_Drv_WriteAccess(ulRomAddr, pucRamAdd, ulSize, QSPI_FLASH_ERASE_AUTO);
    if (FLASH_API_SUCCESS != iRet)
    {
        iRet = FLASH_API_WRITE_ERROR;
    }

    return(iRet);
}

/************************************************************************************************/
/* FUNCTION   : Flash_Drv_Read                                                                  */
/*                                                                                              */
/* DESCRIPTION: QSPI Flashメモリ読み出し処理                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulRomAddr               読み出すFlashメモリのアドレス                           */
/*              pucRamAddr              読み出しデータを格納する領域のアドレス                  */
/*              ulSize                  読み出すデータのサイズ                                  */
/* OUTPUT     : pucRamAddr              実際に読み出したデータ                                  */
/* RESULTS    : FLASH_API_SUCCESS       正常終了                                                */
/*              FLASH_API_READ_ERROR    読み出しエラー                                          */
/*                                                                                              */
/************************************************************************************************/
int Flash_Drv_Read(unsigned long ulRomAddr, unsigned char *pucRamAdd, unsigned long ulSize)
{
    QSPI_BUF_INFO   tBufInfo;
    UINT32          ulActualRead;
    int             iRet;
    INT8            cStat;

    /* パラメータチェック */
    if (FLASH_MAX_SIZE < (ulRomAddr + ulSize))
    {
        return FLASH_API_READ_ERROR;
    }

    /* 読み出し設定 */
    tBufInfo.buf        = pucRamAdd;
    tBufInfo.addr       = ulRomAddr;
    tBufInfo.byte_count = &ulActualRead;
    tBufInfo.len        = ulSize;
    /* 読み出し実行 */
    cStat = QSPI_Flash_Read(&l_hFlash, &tBufInfo);
    if( FALSE == cStat )
    {
        iRet = FLASH_API_READ_ERROR;
    }
    else
    {
        iRet = FLASH_API_SUCCESS;
    }
    
    return(iRet);
}


