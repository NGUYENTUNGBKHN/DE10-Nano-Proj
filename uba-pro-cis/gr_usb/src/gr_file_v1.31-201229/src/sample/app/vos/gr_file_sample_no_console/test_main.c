/************************************************************************************************/
/*                                                                                              */
/*                           Copyright(C) 2010-2016 Grape Systems, Inc.                         */
/*                                       All Rights Reserved                                    */
/*                                                                                              */
/* This software is furnished under a license and may be used and copied only in accordance     */
/* with the terms of such license and with the inclusion of the above copyright notice.         */
/* No title to and ownership of the software is transferred. Grape Systems Inc. makes no        */
/* representation or warranties with respect to the performance of this computer program, and   */
/* specifically disclaims any responsibility for any damages, special or consequential,         */
/* connected with the use of this program.                                                      */
/*                                                                                              */
/************************************************************************************************/
/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      test_main.c                                                             0.04            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      USBマスストレージ統合キット用のテスト環境です。                                         */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2010/01/28  V0.01                                                           */
/*                              Created initial version                                         */
/*   K.Takagi       2012/01/25  V0.02                                                           */
/*                              Changes for the sample of GR-FILE.                              */
/*   K.Kaneko       2015/03/04  V0.03                                                           */
/*                              Changes calling of VOS API.                                     */
/*   K.Kaneko       2016/03/17  V0.04                                                           */
/*                              Added comment about the buffer address when using direct-io.    */
/*                              Fixed type to be using in VOS1.xx.                              */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "test_main.h"
#include "gr_vos.h"
#include "grp_std_tools.h"                                                      /* for memset   */
#include "gr_bus.h"
#include "grusb.h"

#include "fat.h"
#include "grp_mem_vl_pool.h"
#include "grp_fat_format.h"
#include "grp_stdio.h"
#include "fsif.h"

#define GRDBG_PRINT                     //

/**** INTERNAL DATA DEFINES *********************************************************************/
/* GR-FILEの初期化に関する定義 -----------------------------------------------------------------*/
/* プールサイズ */
#define FILE_POOL_SIZE                  (1024 * 100)
UINT32  g_aulGrFileBuf[FILE_POOL_SIZE/4];

/* 挿抜処理用タスク */
GRVOS_tTask         *g_ptConDconTask;
#define _CONDCON_TASK_NAME              ((UINT8 *)"tConDcon")
#define _CONDCON_TASK_STACK             (1024)
#define _CONDCON_TASK_PRI               GRVOS_PRI_LOWEST
#define _CONDCON_TASK_STAT              GRVOS_READY

GRVOS_tTask         *g_ptAppTask;
#define _APP_TASK_NAME                  ((UINT8 *)"tApp")
#define _APP_TASK_STACK                 (1024*4)
#define _APP_TASK_PRI                   GRVOS_PRI_LOWEST
#define _APP_TASK_STAT                  GRVOS_READY

/* FSIFからの通知用キュー */
extern GRVOS_tQueue             *g_ptXferQue;

#define _TEST_DATA_BUF_SIZE             (4*1024)
UINT32                          g_aulTestWrBuf[_TEST_DATA_BUF_SIZE/4];
UINT32                          g_aulTestRdBuf[_TEST_DATA_BUF_SIZE/4];
UINT8 *                         g_pucTestWrBuf = (UINT8 *)g_aulTestWrBuf;
UINT8 *                         g_pucTestRdBuf = (UINT8 *)g_aulTestRdBuf;


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL int           _usb_test_GrFileInit(void);
LOCAL int           _usb_test_AppInit(void);
LOCAL INT8*         _usb_test_GetMemoryArea(void);
LOCAL void          _usb_test_ConDconTask( UINT32 ulArg);
LOCAL void          _usb_test_AppTask( UINT32 ulArg);

/************************************************************************************************/
/* FUNCTION   : _usb_test_GetMemoryArea                                                         */
/*                                                                                              */
/* DESCRIPTION: GR-FILE向けプール用メモリアドレスの変換                                         */
/*              ！！！ 使用する環境に併せてポーティングしてください ！！！                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : vpMemAddress                    メモリアドレス                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : 変換後のアドレス                Success                                         */
/*              USB_TEST_NULL                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL INT8* _usb_test_GetMemoryArea(void)
{
    /* GR-FILEで使用するメモリプールを非キャッシュ領域より割り当てます */
    return (INT8*)g_aulGrFileBuf;
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_Test_Stack_Init                                                           */
/*                                                                                              */
/* DESCRIPTION: Initialize of thie test application.                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : USB_TEST_OK                     Success                                         */
/*              USB_TEST_NG                     Error                                           */
/*                                                                                              */
/************************************************************************************************/
int GRUSB_Test_Stack_Init(void)
{
     STATUS  iStatus;

    /*--- Initialize several modules ---*/

    /* initialize VOS */
    GRVOS_Init();
    GRVOS_OsInitialized();

    /* Initialize GR-BUS */
    iStatus = GRBUS_Init();
    if(GRBUS_OK != iStatus)
        return GRUSB_ERROR;

    /* Initialize GR-USB (Class) */
    iStatus = GRUSB_Class_Initialize();
    if(GRUSB_OK != iStatus)
        return GRUSB_ERROR;

    /* Initialize FSIF Driver */
    iStatus = FSCD_init();
    if(FSCD_E_OK != iStatus)
        return GRUSB_ERROR;

    /* Initialize GR-USB */
    iStatus = GRUSB_Initialize();
    if(GRUSB_OK != iStatus)
        return GRUSB_ERROR;

    /* GR-FILE */
    iStatus = _usb_test_GrFileInit();
    if (USB_TEST_OK != iStatus) {
        /* error */
        return GRUSB_ERROR;
    }

    /* Initialize test application */
    iStatus = _usb_test_AppInit();
    if(USB_TEST_OK != iStatus)
        return GRUSB_ERROR;

    /* start USB */
    iStatus = GRUSB_Enable();
    if(GRUSB_OK != iStatus)
        return GRUSB_ERROR;

    return GRUSB_OK;

}

/************************************************************************************************/
/* FUNCTION   : _usb_test_GrFileInit                                                            */
/*                                                                                              */
/* DESCRIPTION: Initialize of GR-FILE.                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : USB_TEST_OK                     Success                                         */
/*              USB_TEST_NG                     Error                                           */
/*                                                                                              */
/************************************************************************************************/
int _usb_test_GrFileInit(void)
{
    INT8                            *pcPoolArea;

    /*--- GR-FILE向けメモリ管理の初期化 ---*/
    /* メモリエリアの取得 */
    pcPoolArea = _usb_test_GetMemoryArea();
    if (pcPoolArea == 0) {
        /* error */
        return USB_TEST_NG;
    }
    /* メモリ管理の初期化 */
    grp_mem_vl_init( pcPoolArea, FILE_POOL_SIZE);

    /*--- GR-FILEの初期化 ---*/
    /* configurationパラメータ(必要ならば) */
        /* 本アプリケーションではデフォルト値を利用する */
    /* GR-FILEの初期化 */
    if (grp_fs_init() != 0) {
        /* error */
        return USB_TEST_NG;
    }

    /*--- フック関数の初期化処理 ---*/
    fat_interrupt_lookup = 0;

    /*--- 標準入出力関数登録(必要ならば) ---*/
        /* 本アプリケーションでは不要なので未設定 */
        /* grp_stdio_io_stdout = GRP_USB_NULL; */
        /* grp_stdio_io_stdin  = GRP_USB_NULL; */

    /*--- I/Oエラー関数登録(必要ならば) ---*/
        /* 本アプリケーションでは不要なので未設定 */

    return USB_TEST_OK;
}

/************************************************************************************************/
/* FUNCTION   : _usb_test_ConDconTask                                                           */
/*                                                                                              */
/* DESCRIPTION: MSメディアの挿抜を監視するタスク                                                */
/*              本タスクでは接続および切断の通知のみを行います。                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulArg                           reserved                                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _usb_test_ConDconTask( UINT32 ulArg)
{
    int                             iPtn;

    /* ワーニング対策 */
    ulArg = 0;

    while (1) {
        if (GRVOS_ReceiveQueue(   g_ptXferQue,
                                  (void *)&iPtn,
                                  GRVOS_INFINITE) != GRVOS_POS_RESULT) {
            continue;
        }

        if (iPtn & ATDT_QUE_DETACHE) {
            GRDBG_PRINT("<-- DISCONNECT USB0 \r\n");
        }
        else {
            GRDBG_PRINT("--> CONNECT USB0 \r\n");
        }
    }
}


/************************************************************************************************/
/* FUNCTION   : _usb_test_mount                                                                 */
/*                                                                                              */
/* DESCRIPTION: マウント処理を実行します                                                        */
/*----------------------------------------------------------------------------------------------*/
int _usb_test_mount(void)
{
    UINT8                           aucDevName[] = "usb0";
    int                             iRet = 0;

    iRet = grp_fs_mount(aucDevName, "/", "fat", GRP_FS_SYNC_FL_CLOSE);
    if (iRet == 0) {
        GRDBG_PRINT("     O mount ok\r\n");
    }
    else if (iRet == GRP_FS_ERR_NEED_CHECK) {
        GRDBG_PRINT("     X mount ng - need force to mount\r\n");
    }
    else {
        GRDBG_PRINT("     X mount ng\r\n");
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_force_mount                                                           */
/*                                                                                              */
/* DESCRIPTION: マウント処理を実行します                                                        */
/*----------------------------------------------------------------------------------------------*/
int _usb_test_force_mount(void)
{
    UINT8                           aucDevName[] = "usb0";
    int                             iRet = 0;

    iRet = grp_fs_mount(aucDevName, "/", "fat", (GRP_FS_SYNC_FL_CLOSE | GRP_FS_FORCE_MOUNT));
    if (iRet == 0) {
        GRDBG_PRINT("     O force mount ok\r\n");
    }
    else {
        GRDBG_PRINT("     X force mount ng\r\n");
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_unmount                                                               */
/*                                                                                              */
/* DESCRIPTION: アンマウント処理を実行します                                                    */
/*----------------------------------------------------------------------------------------------*/
int _usb_test_unmount(void)
{
    UINT8                           aucDevName[] = "usb0";
    int                             iRet = 0;

    iRet = grp_fs_unmount(aucDevName, 0);
    if (iRet == 0) {
        GRDBG_PRINT("     O unmount ok\r\n");
    }
    else if (iRet == GRP_FS_ERR_BUSY) {
        GRDBG_PRINT("     X unmount ng - need force to unmount\r\n");
    }
    else {
        GRDBG_PRINT("     X unmount ng\r\n");
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_force_unmount                                                         */
/*                                                                                              */
/* DESCRIPTION: アンマウント処理を実行します                                                    */
/*----------------------------------------------------------------------------------------------*/
int _usb_test_force_unmount(void)
{
    UINT8                           aucDevName[] = "usb0";
    int                             iRet = 0;

    iRet = grp_fs_unmount(aucDevName, GRP_FS_FORCE_UMOUNT);
    if (iRet == 0) {
        GRDBG_PRINT("     O force unmount ok\r\n");
    }
    else {
        GRDBG_PRINT("     X force unmount ng\r\n");
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_ls                                                                    */
/*                                                                                              */
/* DESCRIPTION: リストの取得処理を実行しシリアルにファイル名を出力します                        */
/*----------------------------------------------------------------------------------------------*/
#define _USB_TEST_FILE_NAME_MAX         (16)
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_ls(void)
{
    int                             iHdr    = 0;
    int                             iRet    = 0;
    grp_fs_dir_ent_t                tDirent;
    UINT8                           aucName[_USB_TEST_FILE_NAME_MAX*2+1];

    /* NULLストッパーの付加（） */
    aucName[_USB_TEST_FILE_NAME_MAX*2] = 0;

    /* ルートディレクトリの情報しか取らないため */
    iHdr = -1;

    GRDBG_PRINT("--- File List---\r\n");
    tDirent.pucName     = aucName;                          /* set file aucName buffer      */
    tDirent.sNameSize   = _USB_TEST_FILE_NAME_MAX*2;        /* set aucName buffer size      */
    tDirent.uiStart     = 0;                                /* start offset is 0            */
    tDirent.uiEnd       = 0;                                /* end offset is 0              */
    while ((iRet = grp_fs_get_dirent(iHdr, &tDirent)) > 0) {
        GRDBG_PRINT("%s\r\n", tDirent.pucName);
        tDirent.sNameSize = _USB_TEST_FILE_NAME_MAX*2;      /* set aucName buffer size      */
        tDirent.uiStart   = tDirent.uiEnd;                  /* set next                     */
    }
    GRDBG_PRINT("\r\n");
    return 0;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_format                                                                */
/*                                                                                              */
/* DESCRIPTION: フォーマットを行ないます                                                        */
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_format(void)
{
    grp_fat_format_param_t          param;
    grp_fs_media_info_t             media;
    int                             iRet;
    UINT8                           aucDevName[] = "usb0";

    memset(&param, 0, sizeof(param));
    memset(&media, 0, sizeof(media));
    iRet = grp_fat_format(aucDevName, &param, &media);
    if (iRet >= 0) {    /* ok */
        GRDBG_PRINT("     O format ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X format ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_open_file                                                             */
/*                                                                                              */
/* DESCRIPTION: ファイルをオープンします                                                        */
/*----------------------------------------------------------------------------------------------*/
int                             g_iHandle1    = -1;
UINT8                           g_aucTestFileName[] = "TEST1.TXT";
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_open_file(void)
{
    int                             iMode       = 0;
    int                             iProt       = 0;
    int                             iRet;
    UINT8                           aucFilneName[_USB_TEST_FILE_NAME_MAX*2+1];

    grp_std_strcpy(aucFilneName, g_aucTestFileName);
    iMode = GRP_FS_O_RDWR | GRP_FS_O_CREAT | GRP_FS_O_DIRECT_IO;
    iProt = GRP_FS_PROT_RWXA;                                           /* すべて許可   */
    iRet = grp_fs_open(aucFilneName, iMode, iProt);
    if (iRet >= 0) {    /* ok */
        g_iHandle1 = iRet;
        iRet = 0;
        GRDBG_PRINT("     O file open ok\r\n");
    }
    else {              /* ng */
        g_iHandle1 = -1;
        GRDBG_PRINT("     X file open ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_close_file                                                            */
/*                                                                                              */
/* DESCRIPTION: ファイルをクローズします                                                        */
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_close_file(void)
{
    int                             iRet;

    if (g_iHandle1 == -1) {
        /* error */
        GRDBG_PRINT("     X file close ng <not open file>\r\n");
    }

    iRet = grp_fs_close(g_iHandle1);
    if (iRet == 0) {    /* ok */
        g_iHandle1 = -1;
        GRDBG_PRINT("     O file close ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file close ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_write_file                                                            */
/*                                                                                              */
/* DESCRIPTION: ファイルにデータを書き込みます                                                  */
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_write_file(void)
{
    int                             iRet;
    UINT8 *                         pucBuf = (UINT8 *)g_pucTestWrBuf;
    UINT32                          ulDataSz = _TEST_DATA_BUF_SIZE;
    int                             i;
    int                             j;

    /* テストデータでバッファを埋める */
    for (i=0, j=0; i<ulDataSz; i++) {
        *(pucBuf + i) = i;
        if (i%512 == 0) {
            *(pucBuf + i) = j++;
        }
    }

    if (g_iHandle1 == -1) {
        /* error */
        GRDBG_PRINT("     X file write ng <not open file>\r\n");
    }

    /* ファイルオープンで GRP_FS_O_DIRECT_IO を指定している場合は */
    /* 非キャッシュ領域のバッファを引数に渡します */
    iRet = grp_fs_write(g_iHandle1, pucBuf, ulDataSz);
    if (iRet >= 0) {    /* ok */
        iRet = 0;
        GRDBG_PRINT("     O file write ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file write ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_read_file                                                             */
/*                                                                                              */
/* DESCRIPTION: ファイルからデータを読み込みます                                                */
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_read_file(void)
{
    int                             iRet;
    UINT8 *                         pucBuf = (UINT8 *)g_pucTestRdBuf;
    UINT32                          ulDataSz = _TEST_DATA_BUF_SIZE;

    if (g_iHandle1 == -1) {
        /* error */
        GRDBG_PRINT("     X file read ng <not open file>\r\n");
    }

    /* バッファの初期化 */
    grp_std_memset(pucBuf, 0, ulDataSz);

    /* ファイルオープンで GRP_FS_O_DIRECT_IO を指定している場合は */
    /* 非キャッシュ領域のバッファを引数に渡します */
    iRet = grp_fs_read(g_iHandle1, pucBuf, ulDataSz);
    if (iRet >= 0) {    /* ok */
        iRet = 0;
        GRDBG_PRINT("     O file read ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file read ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_del_file                                                              */
/*                                                                                              */
/* DESCRIPTION: ファイルを削除します                                                            */
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_del_file(void)
{
    int                             iRet;

    if (g_iHandle1 != -1) {
        /* error */
        GRDBG_PRINT("     X file delete ng <opened file>\r\n");
    }

    iRet = grp_fs_unlink(g_aucTestFileName);
    if (iRet == 0) {    /* ok */
        GRDBG_PRINT("     O file delete ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file delete ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_check_file                                                            */
/*                                                                                              */
/* DESCRIPTION: 読み込んだファイルを書き込んだデータと比較します                                */
/*----------------------------------------------------------------------------------------------*/
LOCAL int _usb_test_check_file(void)
{
    int                             iRet;

    iRet = grp_std_memcmp(g_pucTestWrBuf, g_pucTestRdBuf, _TEST_DATA_BUF_SIZE);
    if (iRet == 0) {    /* ok */
        GRDBG_PRINT("     O file check ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file check ng\r\n");
    }

    /* バッファの初期化 */
    grp_std_memset(g_pucTestRdBuf, 0, _TEST_DATA_BUF_SIZE);

    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_AppTask                                                               */
/*                                                                                              */
/* DESCRIPTION: テストアプリケーション用のタスク                                                */
/*              本タスクではグローバル変数(g_ulTestNum)を変更することで色々なテストを実施します */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulArg                           reserved                                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
UINT32                          g_ulTestNum = 0;
/*----------------------------------------------------------------------------------------------*/
LOCAL void _usb_test_AppTask( UINT32 ulArg)
{
    /* ワーニング対策 */
    ulArg = 0;

    while (1) {
        switch (g_ulTestNum) {
        case 0:     /* アイドル */
            GRP_VOS_DelayTask(100);                                                 break;
        case 1:     /* マウント */
            _usb_test_mount();                              g_ulTestNum = 0;        break;
        case 2:     /* アンマウント */
            _usb_test_unmount();                            g_ulTestNum = 0;        break;
        case 3:     /* ファイルリスト */
            _usb_test_ls();                                 g_ulTestNum = 0;        break;
        case 5:     /* 強制マウント */
            _usb_test_force_mount();                        g_ulTestNum = 0;        break;
        case 6:     /* 強制アンマウント */
            _usb_test_force_unmount();                      g_ulTestNum = 0;        break;
        case 7:     /* フォーマット */
            _usb_test_format();                             g_ulTestNum = 0;        break;
        case 10:    /* ファイルオープン */
            _usb_test_open_file();                          g_ulTestNum = 0;        break;
        case 11:    /* ファイルクローズ */
            _usb_test_close_file();                         g_ulTestNum = 0;        break;
        case 12:    /* ファイルライト g_pucTestWrBuf に設定しファイルへ書き込み */
            _usb_test_write_file();                         g_ulTestNum = 0;        break;
        case 13:    /* ファイルリード g_pucTestRdBuf にファイルより読みこみ */
            _usb_test_read_file();                          g_ulTestNum = 0;        break;
        case 14:    /* ファイル削除 */
            _usb_test_del_file();                           g_ulTestNum = 0;        break;
        case 15:    /* ファイル比較 g_pucTestWrBuf と g_pucTestRdBuf の比較 */
            _usb_test_check_file();                         g_ulTestNum = 0;        break;

        case 20:    /* ファイルのオープン～ライト～クローズ */
            if (_usb_test_open_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_write_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_close_file() != 0) {              g_ulTestNum = 0;        break;}
            g_ulTestNum = 0;
            break;
        case 21:    /* ファイルのオープン～リード～クローズ */
            if (_usb_test_open_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_read_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_close_file() != 0) {              g_ulTestNum = 0;        break;}
            g_ulTestNum = 0;
            break;

        case 30:     /* マウント-リスト-アンマウントの繰り返し */
            _usb_test_mount();
            _usb_test_ls();
            _usb_test_unmount();
            GRP_VOS_DelayTask(500);
            break;
        case 31:     /* マウント-リスト-アンマウントの繰り返し */
            _usb_test_mount();
            _usb_test_unmount();
            GRP_VOS_DelayTask(500);
            break;
        case 32:    /* ファイル比較連続テスト */
            if (_usb_test_open_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_write_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_close_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_open_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_read_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_close_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_check_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_del_file()   != 0) {              g_ulTestNum = 0;        break;}
            GRP_VOS_DelayTask(500);
            break;

        default:    /* 未実装テスト番号 */
            g_ulTestNum = 0;                                                        break;
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : _usb_test_AppInit                                                               */
/*                                                                                              */
/* DESCRIPTION: Initialize of thie test application.                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : USB_TEST_OK                     Success                                         */
/*              USB_TEST_NG                     Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL int _usb_test_AppInit(void)
{
    /* ファイルオープンで GRP_FS_O_DIRECT_IO を指定している場合は */
    /* 非キャッシュ領域のアドレスを設定します */
    g_pucTestWrBuf = (UINT8 *)g_aulTestWrBuf;
    g_pucTestRdBuf = (UINT8 *)g_aulTestRdBuf;

    /*--- メディア挿抜処理タスクの作成 ---*/
    if (GRP_VOS_CreateTask( &g_ptConDconTask,
                            _CONDCON_TASK_NAME,
                            _usb_test_ConDconTask,
                            _CONDCON_TASK_STACK,
                            _CONDCON_TASK_PRI,
                            _CONDCON_TASK_STAT, 0) != GRP_VOS_POS_RESULT) {
        /* error */
        return USB_TEST_NG;
    }

    /*--- テストアプリケーション用タスクの作成 ---*/
    if (GRP_VOS_CreateTask( &g_ptAppTask,
                            _APP_TASK_NAME,
                            _usb_test_AppTask,
                            _APP_TASK_STACK,
                            _APP_TASK_PRI,
                            _APP_TASK_STAT, 0) != GRP_VOS_POS_RESULT) {
        /* error */
        return USB_TEST_NG;
    }

    return USB_TEST_OK;
}

/*----------------------------------------------------------------------------------------------*/
/*  以下の関数はGR-FILEのライブラリをいじりたくないため、ダミーの関数として用意しておく。       */
/*  cons_getcharは通常proc_event内のアプリでしか使われていないので、このデモでは使用しないこと  */
/*  にする。                                                                                    */
/*----------------------------------------------------------------------------------------------*/

/************************************************************************************************/
/* FUNCTION   : cons_putchar                                                                    */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : mode                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
int cons_putchar(int iChar)
{
    return 0;
}

/************************************************************************************************/
/* FUNCTION   : cons_getchar                                                                    */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : mode                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
int cons_getchar(int iMode)
{
    return 0;
}
