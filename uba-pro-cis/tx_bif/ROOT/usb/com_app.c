/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      com_app.c                                                               1.02            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Communication Function Driver アプリケーション                                          */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   M.Suzuki       2013/04/02  V1.00                                                           */
/*                              Created initial version                                         */
/*   M.Suzuki       2018/11/26  V1.01                                                           */
/*                              受信要求サイズを以下の関数で取得するように変更                  */
/*                              - GRUSB_DEV_ApGetMaxPacketSize                                  */
/*   M.Suzuki       2020/04/10  V1.02                                                           */
/*                              Interrupt転送のテストコマンド処理を別ファイルに移行             */
/*                                                                                              */
/************************************************************************************************/

/**** COMPILE OPTIONS ***************************************************************************/
/* Interrupt転送テストを使用する場合はこの定義を有効にする */
#define INTR_TEST_CMD_CHECK

/**** INCLUDE FILES *****************************************************************************/
#include <string.h>

#include "common.h"

#include "grp_vos.h"

#include "comm_def.h"
#include "grcomm.h"
#include "com_app.h"

#ifdef INTR_TEST_CMD_CHECK
#include "com_app_intr_cmd.h"
#endif /* INTR_TEST_CMD_CHECK */


/**** STRUCTURE PROTOTYPES **********************************************************************/


/**** INTERNAL DATA DEFINES *********************************************************************/
#if 0 // for test 20211104
#define BUF_SIZE                (512)                                         /* バッファサイズ */
#else
#define BUF_SIZE                (64*1024)                                     /* バッファサイズ */
#define BUF_TOTAL_SIZE        (10 * 1024 * 1024)                         /* 合計データサイズ */
#define BUF_SEND_NUM        (BUF_TOTAL_SIZE / BUF_SIZE)             /* バッファ送信回数 */

//#define TEST_WRITE              (0)                                             /* 0: データ受信、1: データ送信 */
#define TEST_WRITE              (1)                                             /* 0: データ受信、1: データ送信 */
//#define JCM_DEBUG				(1)
#endif

/* Line Coding */
#define LINE_CODING_BUF_SIZE    (8)                              /* Line Coding バッファサイズ  */
#define LINE_CODING_SIZE        (7)                              /* Line Coding Structureサイズ */
#define APP_BAUDRATE            (9600)                           /* ボーレート初期値            */
#define APP_STOPBITS            (0)                              /* ストップビット初期値        */
#define APP_PARITY              (0)                              /* パリティ初期値              */
#define APP_DATABITS            (8)                              /* データビット初期値          */



#if (defined(PRJ_IVIZION2) && (BV_UNIT_TYPE <= WS_MODEL))
/* Product ID(USB:0) */
#define GRCOMD_CNF_PID_MSB           (0x01)  /* idProduct            */
#define GRCOMD_CNF_PID_LSB           (0x17)  /* product ID 0x0117    */
/* Product ID(USB:1) */
#define GRCOMD_CNF_PID_MSB2          (0x01)  /* idProduct            */
#define GRCOMD_CNF_PID_LSB2          (0x18)  /* product ID 0x0118    */
#elif defined(PRJ_IVIZION2)
/* Product ID(USB:0) */
#define GRCOMD_CNF_PID_MSB           (0x01)  /* idProduct            */
#define GRCOMD_CNF_PID_LSB           (0x19)  /* product ID 0x0119    */
/* Product ID(USB:1) */
#define GRCOMD_CNF_PID_MSB2          (0x01)  /* idProduct            */
#define GRCOMD_CNF_PID_LSB2          (0x1A)  /* product ID 0x011A    */
#else
/* Product ID(USB:0) */
#define GRCOMD_CNF_PID_MSB           (0x01)  /* idProduct            */
#define GRCOMD_CNF_PID_LSB           (0x1B)  /* product ID 0x011B    */
/* Product ID(USB:1) */
#define GRCOMD_CNF_PID_MSB2          (0x01)  /* idProduct            */
#define GRCOMD_CNF_PID_LSB2          (0x1C)  /* product ID 0x011C    */
#endif
const GRUSB_PID _UsbPid = {GRCOMD_CNF_PID_LSB, GRCOMD_CNF_PID_MSB};

/**** INTERNAL VARIABLE DEFINES *****************************************************************/
/* ポートステータス */
DLOCAL BOOLEAN                  l_bPortStat;
/* 受信要求サイズ(MaxPacketSize) */
DLOCAL UINT32                   l_usRcvSize;
/* Control Signal Bitmap */
DLOCAL UINT16                   l_usControlLineState;
/* Line Coding Structureバッファ */
DLOCAL UINT8                    l_aucLineCoding[LINE_CODING_BUF_SIZE];
/* 送信データバッファ */
DLOCAL UINT32                   l_aulDtSndBuf[BUF_SIZE/sizeof(UINT32)];
DLOCAL UINT8*                   l_pucDtSndBuf = (UINT8*)l_aulDtSndBuf;
/* 受信データバッファ */
DLOCAL UINT32                   l_aulDtRcvBuf[BUF_SIZE/sizeof(UINT32)];
DLOCAL UINT8*                   l_pucDtRcvBuf = (UINT8*)l_aulDtRcvBuf;

DLOCAL UINT8					 l_pucTestDtSndBuf[BUF_TOTAL_SIZE]	__attribute__ ((at(0x03000000)));

DLOCAL UINT32					l_iSendCnt;


/**** EXTERNAL VARIABLE DEFINES *****************************************************************/


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL VOID _com_app_cb_ConnStat( INT );
LOCAL VOID _com_app_cb_SendEncapsulatedCommad( UINT32, UINT8* );
LOCAL VOID _com_app_cb_GetEncapsulatedResponse( UINT16 );
LOCAL VOID _com_app_cb_SetLineCoding( UINT32, UINT8* );
LOCAL VOID _com_app_cb_GetLineCoding( UINT16 );
LOCAL VOID _com_app_cb_SetControlLineState( UINT16 );
LOCAL VOID _com_app_cb_NetworkConnection( VOID );
LOCAL VOID _com_app_cb_ResponseAvailable( VOID );
LOCAL VOID _com_app_cb_SerialState( VOID );
#ifdef GRCOMD_COMP_STATUS_USE
LOCAL VOID _com_app_cb_SendData( UINT32, UINT8*, VOID*, INT );
LOCAL VOID _com_app_cb_RecvData( UINT32, UINT8*, VOID*, INT );
#else
LOCAL VOID _com_app_cb_SendData( UINT32, UINT8*, VOID* );
LOCAL VOID _com_app_cb_RecvData( UINT32, UINT8*, VOID* );
#endif
LOCAL INT  _com_app_ReqSendData( UINT32, UINT8* );
LOCAL INT  _com_app_ReqRecvData( UINT32, UINT8* );


/************************************************************************************************/
/* FUNCTION   : com_app_Init                                                                    */
/*                                                                                              */
/* DESCRIPTION: Communication Function Driver アプリケーションの初期化処理                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : COM_APP_OK                      Success                                         */
/*              COM_APP_ERROR                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
INT com_app_Init( VOID )
{
GRUSB_COMD_INITINFO             tInitPrm;
INT                             iStat;

    /* ポートステータスの初期化 */
    l_bPortStat = GRUSB_FALSE;
    /* 受信要求サイズの初期化 */
    l_usRcvSize = 0;
    /* Control Signal Bitmapの初期化 */
    l_usControlLineState = 0;
    /* Line Coding バッファ初期化 */
    memset( l_aucLineCoding, 0x00, LINE_CODING_BUF_SIZE );
    /* 送信データバッファ初期化 */
    memset( l_pucDtSndBuf, 0x00, BUF_SIZE );
    /* 受信データバッファ初期化 */
    memset( l_pucDtRcvBuf, 0x00, BUF_SIZE );

    /* コールバック関数の設定 */
    tInitPrm.pfnConnStat              = _com_app_cb_ConnStat;
    tInitPrm.pfnSendEncapsulatedCmd   = _com_app_cb_SendEncapsulatedCommad;
    tInitPrm.pfnGetEncapsulatedRes    = _com_app_cb_GetEncapsulatedResponse;
    tInitPrm.pfnSetCommFeature        = GRUSB_NULL;
    tInitPrm.pfnGetCommFeature        = GRUSB_NULL;
    tInitPrm.pfnClearCommFeature      = GRUSB_NULL;
    tInitPrm.pfnSetLineCoding         = _com_app_cb_SetLineCoding;
    tInitPrm.pfnGetLineCoding         = _com_app_cb_GetLineCoding;
    tInitPrm.pfnSetControlLineState   = _com_app_cb_SetControlLineState;
    tInitPrm.pfnSendBreak             = GRUSB_NULL;
    tInitPrm.pfnNetworkConnection     = _com_app_cb_NetworkConnection;
    tInitPrm.pfnResponseAvailable     = _com_app_cb_ResponseAvailable;
    tInitPrm.pfnSerialState           = _com_app_cb_SerialState;
    tInitPrm.pfnSendData              = _com_app_cb_SendData;
    tInitPrm.pfnReciveData            = _com_app_cb_RecvData;
    tInitPrm.pstUsbPid                = (PGRUSB_PID)&_UsbPid;

    /* Communication Function Driverの初期化 */
    iStat = GRUSB_COMD_Init( &tInitPrm );

    if( GRUSB_COMD_SUCCESS != iStat )
    {
        /* error */
        return COM_APP_ERROR;
    }

#ifdef INTR_TEST_CMD_CHECK
    /* Notification情報の初期化 */
    com_app_NotificationInit();
#endif

    /*--- Line Codingの初期値を設定 ---*/
    l_aucLineCoding[0] = (UINT8)( APP_BAUDRATE & 0x000000FF );           /* ボーレート：LSB */
    l_aucLineCoding[1] = (UINT8)( ( APP_BAUDRATE & 0x0000FF00 ) >>  8 ); /*                 */
    l_aucLineCoding[2] = (UINT8)( ( APP_BAUDRATE & 0x00FF0000 ) >> 16 ); /*                 */
    l_aucLineCoding[3] = (UINT8)( ( APP_BAUDRATE & 0xFF000000 ) >> 24 ); /* ボーレート：MSB */
    l_aucLineCoding[4] = APP_STOPBITS;                                   /* ストップビット  */
    l_aucLineCoding[5] = APP_PARITY;                                     /* パリティ        */
    l_aucLineCoding[6] = APP_DATABITS;                                   /* データビット    */

    /* ユーザ指定のデバイスの製造番号(シリアルナンバー)を設定する場合は  */
    /* ここでGRUSB_COMD_SetSerialNumber()をコールして下さい。            */
    /* (注)製造番号は、UNICODE(2バイトコード)形式で渡して下さい。        */

    /* Pullup ON */
    GRUSB_DEV_ApReqPullupRegister( GRUSB_TRUE );

    return COM_APP_OK;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_ConnStat                                                            */
/*                                                                                              */
/* DESCRIPTION: 接続・切断通知                                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iConnStat                       接続ステータス                                  */
/*                                                  GRUSB_COMD_DISC         切断                */
/*                                                  GRUSB_COMD_CON          接続                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_ConnStat( INT iConnStat )
{
    int i, j, k;
#if 0 // for test 20211104
    int cnt = 0;
#endif
    int iRet;

    if( GRUSB_COMD_CON == iConnStat )
    {
        /* 接続 */

#if 0 // for test 20211104
        /* Bulk OUTエンドポイントのMaxPacketSizeを取得        */
        /* ※取得したサイズは受信要求サイズとして使用します。 */
        l_usRcvSize = GRUSB_DEV_ApGetMaxPacketSize( GRCOMD_BULKOUT_EP_NUMBER );
#else
        l_usRcvSize = BUF_SIZE;
#endif

#if TEST_WRITE // for test 20211104
        for (i = 0; i < BUF_SEND_NUM; i++)
        {
#if 1
        	/* 先頭1バイトに連番をセット、先頭以外は全て0x00をセット */
            memset(&l_pucTestDtSndBuf[i * BUF_SIZE], 0x00, BUF_SIZE);
            for (j = 0, k = 0; j <= BUF_SIZE; j += 512, k++)
            {
                l_pucTestDtSndBuf[(i * BUF_SIZE) + j] = (UINT8)k;
            }
#else
            /* 全データに0xffをセット */
            memset(&l_pucTestDtSndBuf[i * BUF_SIZE], 0xff, BUF_SIZE);
#endif
        }
        l_iSendCnt = 0;

#if JCM_DEBUG
        iRet = _com_app_ReqSendData( BUF_TOTAL_SIZE, l_pucTestDtSndBuf );
#else
        iRet = _com_app_ReqSendData( BUF_SIZE, l_pucTestDtSndBuf );
#endif
#else
        /* データ受信要求開始 */
        _com_app_ReqRecvData( (UINT32)l_usRcvSize, l_pucDtRcvBuf );
#endif
    }
    else
    {
        /* 切断 */

        if( GRUSB_TRUE == l_bPortStat )
        {
            l_bPortStat = GRUSB_FALSE;
        }

#ifdef INTR_TEST_CMD_CHECK
        /* Notification情報の初期化 */
        com_app_NotificationInit();
#endif
    }

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_SendEncapsulatedCommad                                              */
/*                                                                                              */
/* DESCRIPTION: SEND_ENCAPSULATED_COMMANDリクエスト受信処理                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          データサイズ                                    */
/*              pucData                         データバッファポインタ                          */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_SendEncapsulatedCommad( UINT32 ulSize, UINT8* pucData )
{
    /*----------------------------------------------------------------------*/
    /* SEND_ENCAPSULATED_COMMANDリクエストを受信すると本関数がコールされ    */
    /* ます。Abstract Control Modelにおいて必須機能のため、コールバックの   */
    /* 設定を行っていますが、本サンプルではコールされることはありません。   */
    /*----------------------------------------------------------------------*/

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_GetEncapsulatedResponse                                             */
/*                                                                                              */
/* DESCRIPTION: GET_ENCAPSULATED_RESPONSEリクエスト受信処理                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_GetEncapsulatedResponse( UINT16 usLength )
{
    /*----------------------------------------------------------------------*/
    /* GET_ENCAPSULATED_RESPONSEリクエストを受信すると本関数がコールされ    */
    /* ます。Abstract Control Modelにおいて必須機能のため、コールバックの   */
    /* 設定を行っていますが、本サンプルではコールされることはありません。   */
    /*----------------------------------------------------------------------*/

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_SetLineCoding                                                       */
/*                                                                                              */
/* DESCRIPTION: SET_LINE_CODINGリクエスト受信処理                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          Line Coding Structureサイズ                     */
/*              pucData                         Line Coding Structureバッファポインタ           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_SetLineCoding( UINT32 ulSize, UINT8* pucData )
{
    /* Line Coding Structureバッファにコピー */
    memcpy( l_aucLineCoding, pucData, ulSize );

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_GetLineCoding                                                       */
/*                                                                                              */
/* DESCRIPTION: GET_LINE_CODINGリクエスト受信処理                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_GetLineCoding( UINT16 usLength )
{
    /* Line Coding Structure送信 */
    GRUSB_COMD_Set_GetLineCoding( (UINT32)usLength, l_aucLineCoding );

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_SetControlLineState                                                 */
/*                                                                                              */
/* DESCRIPTION: SET_CONTROL_LINE_STATEリクエスト受信処理                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usValue                         Control Signal Bitmap                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_SetControlLineState( UINT16 usValue )
{
UINT16                          usRTS;
UINT16                          usDTR;

    /* Control Signal Bitmapをセット */
    l_usControlLineState = usValue;

    /* RTS */
    usRTS = (UINT16)( ( l_usControlLineState & 0x0002 ) >> 1 );
    /* DTR */
    usDTR = (UINT16)( l_usControlLineState & 0x0001 );

    if( ( 1 == usRTS ) && ( 1 == usDTR ) )
    {
        /* ポート オープン */

        if( GRUSB_FALSE == l_bPortStat )
        {
            l_bPortStat = GRUSB_TRUE;
        }
    }
    else
    {
        /* ポート クローズ */

        if( GRUSB_TRUE == l_bPortStat )
        {
            l_bPortStat = GRUSB_FALSE;
        }
    }

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_NetworkConnection                                                   */
/*                                                                                              */
/* DESCRIPTION: NETWORK_CONNECTION通知完了処理                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_NetworkConnection( VOID )
{
#ifdef INTR_TEST_CMD_CHECK
UINT8                           ucNotificationNo;

    /* Notification番号の更新 */
    com_app_UpdateNotificationNo();

    /* Notification番号の取得 */
    ucNotificationNo = com_app_GetNotificationNo();

    /* COM Notification送信 */
    com_app_SendNotification( ucNotificationNo );
#endif

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_ResponseAvailable                                                   */
/*                                                                                              */
/* DESCRIPTION: RESPONSE_AVAILABLE通知完了処理                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_ResponseAvailable( VOID )
{
#ifdef INTR_TEST_CMD_CHECK
UINT8                           ucNotificationNo;

    /* Notification番号の更新 */
    com_app_UpdateNotificationNo();

    /* Notification番号の取得 */
    ucNotificationNo = com_app_GetNotificationNo();

    /* COM Notification送信 */
    com_app_SendNotification( ucNotificationNo );
#endif

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_SerialState                                                         */
/*                                                                                              */
/* DESCRIPTION: SERIAL_STATE通知完了処理                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_SerialState( VOID )
{
#ifdef INTR_TEST_CMD_CHECK
UINT8                           ucNotificationNo;

    /* Notification番号の更新 */
    com_app_UpdateNotificationNo();

    /* Notification番号の取得 */
    ucNotificationNo = com_app_GetNotificationNo();

    /* COM Notification送信 */
    com_app_SendNotification( ucNotificationNo );
#endif

    return;
}

#ifdef GRCOMD_COMP_STATUS_USE
/************************************************************************************************/
/* FUNCTION   : _com_app_cb_SendData                                                            */
/*                                                                                              */
/* DESCRIPTION: データ送信完了通知処理                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          送信データサイズ                                */
/*              pucData                         送信データバッファポインタ                      */
/*              pInfo                           情報ポインタ                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_SendData( UINT32 ulSize, UINT8* pucData, VOID* pInfo, INT iStat )
{
#ifdef INTR_TEST_CMD_CHECK
UINT8                           ucNotificationNo;

    if( GRUSB_COMD_COMPLETE == iStat )
    {
        /* Notification番号の取得 */
        ucNotificationNo = com_app_GetNotificationNo();

        if( 0 != ucNotificationNo )
        {
            /* COM Notification送信 */
            com_app_SendNotification( ucNotificationNo );
        }
    }
#endif

    if( GRUSB_COMD_COMPLETE == iStat )
    {
        /* 送信データバッファクリア */
        memset( l_pucDtSndBuf, 0x00, BUF_SIZE );

        /* データ受信要求開始 */
        _com_app_ReqRecvData( (UINT32)l_usRcvSize, l_pucDtRcvBuf );
    }

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_RecvData                                                            */
/*                                                                                              */
/* DESCRIPTION: データ受信完了通知処理                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          受信データサイズ                                */
/*              pucData                         受信データバッファポインタ                      */
/*              pInfo                           情報ポインタ                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_RecvData( UINT32 ulSize, UINT8* pucData, VOID* pInfo, INT iStat )
{
INT                             iRet;
UINT32                          ulSendSize;

    if( GRUSB_COMD_COMPLETE == iStat )
    {
        if( (UINT32)l_usRcvSize < ulSize )
        {
            /* バッファオーバーフロー */
            return;
        }
        else if( 0 == ulSize )
        {
            /* データ受信要求開始 */
            _com_app_ReqRecvData( (UINT32)l_usRcvSize, l_pucDtRcvBuf );

            return;
        }

        /* 受信データを送信データバッファにコピー */
        memcpy( l_pucDtSndBuf, pucData, ulSize );

#ifdef INTR_TEST_CMD_CHECK
        /* Notification開始コマンドチェック */
        iRet = com_app_CheckNotificationCmd( l_pucDtSndBuf );

        if( GRUSB_TRUE == iRet )
        {
            /* Responseを付与したサイズに変更 */
            ulSendSize = ( ( ulSize - 2 ) + CMD_RESP_LEN );
        }
        else
        {
            /* Notification開始コマンドではなく、通常のデータ受信なので */
            /* 受信データサイズを送信データサイズとする                 */
            ulSendSize = ulSize;
        }
#else
        /* 受信データサイズを送信データサイズとする */
        ulSendSize = ulSize;
#endif

        /* データ送信要求 */
        iRet = _com_app_ReqSendData( ulSendSize, l_pucDtSndBuf );

        if( COM_APP_OK != iRet )
        {
            /* 送信要求エラー */
            return;
        }
    }

    return;
}
#else
/************************************************************************************************/
/* FUNCTION   : _com_app_cb_SendData                                                            */
/*                                                                                              */
/* DESCRIPTION: データ送信完了通知処理                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          送信データサイズ                                */
/*              pucData                         送信データバッファポインタ                      */
/*              pInfo                           情報ポインタ                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_SendData( UINT32 ulSize, UINT8* pucData, VOID* pInfo )
{
#ifdef INTR_TEST_CMD_CHECK
UINT8                          ucNotificationNo;

    l_iSendCnt++;

    /* Notification番号の取得 */
    ucNotificationNo = com_app_GetNotificationNo();

    if( 0 != ucNotificationNo )
    {
        /* COM Notification送信 */
        com_app_SendNotification( ucNotificationNo );
    }
#endif

#if 0 // for test 20211104
    /* データ受信要求開始 */
    _com_app_ReqRecvData( (UINT32)l_usRcvSize, l_pucDtRcvBuf );
#endif

    if (l_iSendCnt < BUF_SEND_NUM)
    {
        _com_app_ReqSendData( BUF_SIZE, l_pucTestDtSndBuf + (l_iSendCnt * BUF_SIZE));
    }

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_RecvData                                                            */
/*                                                                                              */
/* DESCRIPTION: データ受信完了通知処理                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          受信データサイズ                                */
/*              pucData                         受信データバッファポインタ                      */
/*              pInfo                           情報ポインタ                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_RecvData( UINT32 ulSize, UINT8* pucData, VOID* pInfo )
{
INT                             iStat;
UINT32                          ulSendSize;
#if 0 // for test 20211104
INT								iCnt = 0;
INT								i = 0;
#endif

    if( (UINT32)l_usRcvSize < ulSize )
    {
        /* バッファオーバーフロー */
        return;
    }
    else if( 0 == ulSize )
    {
        /* データ受信要求開始 */
        _com_app_ReqRecvData( (UINT32)l_usRcvSize, l_pucDtRcvBuf );

        return;
    }

#if 0 // for test 20211104
    /* 受信データを送信データバッファにコピー */
    memcpy( l_pucDtSndBuf, pucData, ulSize );
#endif

#ifdef INTR_TEST_CMD_CHECK
    /* Notification開始コマンドチェック */
    iStat = com_app_CheckNotificationCmd( l_pucDtSndBuf );

    if( GRUSB_TRUE == iStat )
    {
        /* Responseを付与したサイズに変更 */
        ulSendSize = ( ( ulSize - 2 ) + CMD_RESP_LEN );
    }
    else
    {
        /* Notification開始コマンドではなく、通常のデータ受信なので */
        /* 受信データサイズを送信データサイズとする                 */
        ulSendSize = ulSize;
    }
#else
    /* 受信データサイズを送信データサイズとする */
    ulSendSize = ulSize;
#endif

#if 0 // for test 20211104
    /* データ送信要求 */
    iStat = _com_app_ReqSendData( ulSendSize, l_pucDtSndBuf );
#else
    /* データ受信要求開始 */
    _com_app_ReqRecvData( (UINT32)l_usRcvSize, l_pucDtRcvBuf );
#endif

    if( COM_APP_OK != iStat )
    {
        /* 送信要求エラー */
        return;
    }

    return;
}
#endif

/************************************************************************************************/
/* FUNCTION   : _com_app_ReqSendData                                                            */
/*                                                                                              */
/* DESCRIPTION: データ送信要求                                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          送信データサイズ                                */
/*              pucBuf                          送信データバッファポインタ                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : COM_APP_OK                      Success                                         */
/*              COM_APP_ERROR                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL INT _com_app_ReqSendData( UINT32 ulSize, UINT8* pucBuf )
{
INT                             iStat;
INT                             iRet = COM_APP_OK;

    /* データ送信要求 */
    iStat = GRUSB_COMD_SendData( ulSize, pucBuf, GRUSB_NULL );

    if( GRUSB_COMD_SUCCESS != iStat )
    {
        /* 送信要求エラー */
        iRet = COM_APP_ERROR;
    }

    return iRet;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_ReqRecvData                                                            */
/*                                                                                              */
/* DESCRIPTION: データ受信要求                                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          受信データサイズ                                */
/*              pucBuf                          受信データバッファポインタ                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : COM_APP_OK                      Success                                         */
/*              COM_APP_ERROR                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL INT _com_app_ReqRecvData( UINT32 ulSize, UINT8* pucBuf )
{
INT                             iStat;
INT                             iRet = COM_APP_OK;

#if 0 // for test 20211104
    /* 受信データバッファクリア */
    memset( pucBuf, 0x00, BUF_SIZE );
#endif

    /* データ受信要求 */
    iStat = GRUSB_COMD_ReciveData( ulSize, pucBuf, GRUSB_NULL );

    if( GRUSB_COMD_SUCCESS != iStat )
    {
        /* 受信要求エラー */
        iRet = COM_APP_ERROR;
    }

    return iRet;
}
