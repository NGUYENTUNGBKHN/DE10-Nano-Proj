/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      com_app2.c                                                              1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Communication Function Driver アプリケーション 2                                        */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   T.Yamaguchi    2021/09/08  V1.00                                                           */
/*                              Created initial version.                                        */
/*                              Communication Function Driver アプリケーションをベースに作成    */
/*                                                                                              */
/************************************************************************************************/

/**** COMPILE OPTIONS ***************************************************************************/
/* Interrupt転送テストを使用する場合はこの定義を有効にする */
#define INTR_TEST_CMD_CHECK

/**** INCLUDE FILES *****************************************************************************/
#include <string.h>

#include "common.h"

#include "comm_def.h"
#include "grcomm.h"
#include "com_app.h"

#ifdef INTR_TEST_CMD_CHECK
#include "com_app_intr_cmd.h"
#endif /* INTR_TEST_CMD_CHECK */

/**** STRUCTURE PROTOTYPES **********************************************************************/


/**** INTERNAL DATA DEFINES *********************************************************************/
#define BUF_SIZE                (512)                                         /* バッファサイズ */

/* Line Coding */
#define LINE_CODING_BUF_SIZE    (8)                              /* Line Coding バッファサイズ  */
#define LINE_CODING_SIZE        (7)                              /* Line Coding Structureサイズ */
#define APP_BAUDRATE            (9600)                           /* ボーレート初期値            */
#define APP_STOPBITS            (0)                              /* ストップビット初期値        */
#define APP_PARITY              (0)                              /* パリティ初期値              */
#define APP_DATABITS            (8)                              /* データビット初期値          */

#if 0//
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
//2023-12-26 #define GRCOMD_CNF_PID_LSB2          (0x1C)  /* product ID 0x011C    */
#define GRCOMD_CNF_PID_LSB2          (0x1B)  /* product ID 0x011B    */

#endif
const GRUSB_PID _UsbPid2 = {GRCOMD_CNF_PID_LSB2, GRCOMD_CNF_PID_MSB2};

/**** INTERNAL VARIABLE DEFINES *****************************************************************/
/* ポートステータス */
DLOCAL BOOLEAN                  l_bPortStat2;
/* 受信要求サイズ(MaxPacketSize) */
DLOCAL UINT16                   l_usRcvSize2;
/* Control Signal Bitmap */
DLOCAL UINT16                   l_usControlLineState2;
/* Line Coding Structureバッファ */
DLOCAL UINT8                    l_aucLineCoding2[LINE_CODING_BUF_SIZE];
/* 送信データバッファ */
DLOCAL UINT32                   l_aulDtSndBuf2[BUF_SIZE/sizeof(UINT32)];
DLOCAL UINT8*                   l_pucDtSndBuf2 = (UINT8*)l_aulDtSndBuf2;
/* 受信データバッファ */
DLOCAL UINT32                   l_aulDtRcvBuf2[BUF_SIZE/sizeof(UINT32)];
DLOCAL UINT8*                   l_pucDtRcvBuf2 = (UINT8*)l_aulDtRcvBuf2;


/**** EXTERNAL VARIABLE DEFINES *****************************************************************/


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL VOID _com_app_cb_ConnStat2( INT );
LOCAL VOID _com_app_cb_SendEncapsulatedCommad2( UINT32, UINT8* );
LOCAL VOID _com_app_cb_GetEncapsulatedResponse2( UINT16 );
LOCAL VOID _com_app_cb_SetLineCoding2( UINT32, UINT8* );
LOCAL VOID _com_app_cb_GetLineCoding2( UINT16 );
LOCAL VOID _com_app_cb_SetControlLineState2( UINT16 );
LOCAL VOID _com_app_cb_NetworkConnection2( VOID );
LOCAL VOID _com_app_cb_ResponseAvailable2( VOID );
LOCAL VOID _com_app_cb_SerialState2( VOID );
#ifdef GRCOMD_COMP_STATUS_USE
LOCAL VOID _com_app_cb_SendData2( UINT32, UINT8*, VOID*, INT );
LOCAL VOID _com_app_cb_RecvData2( UINT32, UINT8*, VOID*, INT );
#else
LOCAL VOID _com_app_cb_SendData2( UINT32, UINT8*, VOID* );
LOCAL VOID _com_app_cb_RecvData2( UINT32, UINT8*, VOID* );
#endif
LOCAL INT  _com_app_ReqSendData2( UINT32, UINT8* );
LOCAL INT  _com_app_ReqRecvData2( UINT32, UINT8* );


/************************************************************************************************/
/* FUNCTION   : com_app_Init2                                                                   */
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
INT com_app_Init2( VOID )
{
GRUSB_COMD_INITINFO             tInitPrm;
INT                             iStat;

    /* ポートステータスの初期化 */
    l_bPortStat2 = GRUSB_FALSE;
    /* 受信要求サイズの初期化 */
    l_usRcvSize2 = 0;
    /* Control Signal Bitmapの初期化 */
    l_usControlLineState2 = 0;
    /* Line Coding バッファ初期化 */
    memset( l_aucLineCoding2, 0x00, LINE_CODING_BUF_SIZE );
    /* 送信データバッファ初期化 */
    memset( l_pucDtSndBuf2, 0x00, BUF_SIZE );
    /* 受信データバッファ初期化 */
    memset( l_pucDtRcvBuf2, 0x00, BUF_SIZE );

    /* コールバック関数の設定 */
    tInitPrm.pfnConnStat              = _com_app_cb_ConnStat2;
    tInitPrm.pfnSendEncapsulatedCmd   = _com_app_cb_SendEncapsulatedCommad2;
    tInitPrm.pfnGetEncapsulatedRes    = _com_app_cb_GetEncapsulatedResponse2;
    tInitPrm.pfnSetCommFeature        = GRUSB_NULL;
    tInitPrm.pfnGetCommFeature        = GRUSB_NULL;
    tInitPrm.pfnClearCommFeature      = GRUSB_NULL;
    tInitPrm.pfnSetLineCoding         = _com_app_cb_SetLineCoding2;
    tInitPrm.pfnGetLineCoding         = _com_app_cb_GetLineCoding2;
    tInitPrm.pfnSetControlLineState   = _com_app_cb_SetControlLineState2;
    tInitPrm.pfnSendBreak             = GRUSB_NULL;
    tInitPrm.pfnNetworkConnection     = _com_app_cb_NetworkConnection2;
    tInitPrm.pfnResponseAvailable     = _com_app_cb_ResponseAvailable2;
    tInitPrm.pfnSerialState           = _com_app_cb_SerialState2;
    tInitPrm.pfnSendData              = _com_app_cb_SendData2;
    tInitPrm.pfnReciveData            = _com_app_cb_RecvData2;
    tInitPrm.pstUsbPid                = (PGRUSB_PID)&_UsbPid2;

    /* Communication Function Driverの初期化 */
    iStat = GRUSB_COMD_Init2( &tInitPrm );

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
    l_aucLineCoding2[0] = (UINT8)( APP_BAUDRATE & 0x000000FF );           /* ボーレート：LSB */
    l_aucLineCoding2[1] = (UINT8)( ( APP_BAUDRATE & 0x0000FF00 ) >>  8 ); /*                 */
    l_aucLineCoding2[2] = (UINT8)( ( APP_BAUDRATE & 0x00FF0000 ) >> 16 ); /*                 */
    l_aucLineCoding2[3] = (UINT8)( ( APP_BAUDRATE & 0xFF000000 ) >> 24 ); /* ボーレート：MSB */
    l_aucLineCoding2[4] = APP_STOPBITS;                                   /* ストップビット  */
    l_aucLineCoding2[5] = APP_PARITY;                                     /* パリティ        */
    l_aucLineCoding2[6] = APP_DATABITS;                                   /* データビット    */

    /* ユーザ指定のデバイスの製造番号(シリアルナンバー)を設定する場合は  */
    /* ここでGRUSB_COMD_SetSerialNumber2()をコールして下さい。           */
    /* (注)製造番号は、UNICODE(2バイトコード)形式で渡して下さい。        */

    /* Pullup ON */
    GRUSB_DEV_ApReqPullupRegister2( GRUSB_TRUE );

    return COM_APP_OK;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_ConnStat2                                                           */
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
LOCAL VOID _com_app_cb_ConnStat2( INT iConnStat )
{
    if( GRUSB_COMD_CON == iConnStat )
    {
        /* 接続 */

        /* Bulk OUTエンドポイントのMaxPacketSizeを取得        */
        /* ※取得したサイズは受信要求サイズとして使用します。 */
        l_usRcvSize2 = GRUSB_DEV_ApGetMaxPacketSize2( GRCOMD_BULKOUT_EP_NUMBER );

        /* データ受信要求開始 */
        _com_app_ReqRecvData2( (UINT32)l_usRcvSize2, l_pucDtRcvBuf2 );
    }
    else
    {
        /* 切断 */

        if( GRUSB_TRUE == l_bPortStat2 )
        {
            l_bPortStat2 = GRUSB_FALSE;
        }

#ifdef INTR_TEST_CMD_CHECK
        /* Notification情報の初期化 */
        com_app_NotificationInit2();
#endif
    }

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_SendEncapsulatedCommad2                                             */
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
LOCAL VOID _com_app_cb_SendEncapsulatedCommad2( UINT32 ulSize, UINT8* pucData )
{
    /*----------------------------------------------------------------------*/
    /* SEND_ENCAPSULATED_COMMANDリクエストを受信すると本関数がコールされ    */
    /* ます。Abstract Control Modelにおいて必須機能のため、コールバックの   */
    /* 設定を行っていますが、本サンプルではコールされることはありません。   */
    /*----------------------------------------------------------------------*/

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_GetEncapsulatedResponse2                                            */
/*                                                                                              */
/* DESCRIPTION: GET_ENCAPSULATED_RESPONSEリクエスト受信処理                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_GetEncapsulatedResponse2( UINT16 usLength )
{
    /*----------------------------------------------------------------------*/
    /* GET_ENCAPSULATED_RESPONSEリクエストを受信すると本関数がコールされ    */
    /* ます。Abstract Control Modelにおいて必須機能のため、コールバックの   */
    /* 設定を行っていますが、本サンプルではコールされることはありません。   */
    /*----------------------------------------------------------------------*/

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_SetLineCoding2                                                      */
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
LOCAL VOID _com_app_cb_SetLineCoding2( UINT32 ulSize, UINT8* pucData )
{
    /* Line Coding Structureバッファにコピー */
    memcpy( l_aucLineCoding2, pucData, ulSize );

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_GetLineCoding2                                                      */
/*                                                                                              */
/* DESCRIPTION: GET_LINE_CODINGリクエスト受信処理                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_GetLineCoding2( UINT16 usLength )
{
    /* Line Coding Structure送信 */
    GRUSB_COMD_Set_GetLineCoding2( (UINT32)usLength, l_aucLineCoding2 );

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_SetControlLineState2                                                */
/*                                                                                              */
/* DESCRIPTION: SET_CONTROL_LINE_STATEリクエスト受信処理                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usValue                         Control Signal Bitmap                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_SetControlLineState2( UINT16 usValue )
{
UINT16                          usRTS;
UINT16                          usDTR;

    /* Control Signal Bitmapをセット */
    l_usControlLineState2 = usValue;

    /* RTS */
    usRTS = (UINT16)( ( l_usControlLineState2 & 0x0002 ) >> 1 );
    /* DTR */
    usDTR = (UINT16)( l_usControlLineState2 & 0x0001 );

    if( ( 1 == usRTS ) && ( 1 == usDTR ) )
    {
        /* ポート オープン */

        if( GRUSB_FALSE == l_bPortStat2 )
        {
            l_bPortStat2 = GRUSB_TRUE;
        }
    }
    else
    {
        /* ポート クローズ */

        if( GRUSB_TRUE == l_bPortStat2 )
        {
            l_bPortStat2 = GRUSB_FALSE;
        }
    }

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_NetworkConnection2                                                  */
/*                                                                                              */
/* DESCRIPTION: NETWORK_CONNECTION通知完了処理                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_NetworkConnection2( VOID )
{
#ifdef INTR_TEST_CMD_CHECK
UINT8                           ucNotificationNo;

    /* Notification番号の更新 */
    com_app_UpdateNotificationNo2();

    /* Notification番号の取得 */
    ucNotificationNo = com_app_GetNotificationNo2();

    /* COM Notification送信 */
    com_app_SendNotification2( ucNotificationNo );
#endif

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_ResponseAvailable2                                                  */
/*                                                                                              */
/* DESCRIPTION: RESPONSE_AVAILABLE通知完了処理                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_ResponseAvailable2( VOID )
{
#ifdef INTR_TEST_CMD_CHECK
UINT8                           ucNotificationNo;

    /* Notification番号の更新 */
    com_app_UpdateNotificationNo2();

    /* Notification番号の取得 */
    ucNotificationNo = com_app_GetNotificationNo2();

    /* COM Notification送信 */
    com_app_SendNotification2( ucNotificationNo );
#endif

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_SerialState2                                                        */
/*                                                                                              */
/* DESCRIPTION: SERIAL_STATE通知完了処理                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_app_cb_SerialState2( VOID )
{
#ifdef INTR_TEST_CMD_CHECK
UINT8                           ucNotificationNo;

    /* Notification番号の更新 */
    com_app_UpdateNotificationNo2();

    /* Notification番号の取得 */
    ucNotificationNo = com_app_GetNotificationNo2();

    /* COM Notification送信 */
    com_app_SendNotification2( ucNotificationNo );
#endif

    return;
}

#ifdef GRCOMD_COMP_STATUS_USE
/************************************************************************************************/
/* FUNCTION   : _com_app_cb_SendData2                                                           */
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
LOCAL VOID _com_app_cb_SendData2( UINT32 ulSize, UINT8* pucData, VOID* pInfo, INT iStat )
{
#ifdef INTR_TEST_CMD_CHECK
UINT8                           ucNotificationNo;

    if( GRUSB_COMD_COMPLETE == iStat )
    {
        /* Notification番号の取得 */
        ucNotificationNo = com_app_GetNotificationNo2();

        if( 0 != ucNotificationNo )
        {
            /* COM Notification送信 */
            com_app_SendNotification2( ucNotificationNo );
        }
    }
#endif

    if( GRUSB_COMD_COMPLETE == iStat )
    {
        /* 送信データバッファクリア */
        memset( l_pucDtSndBuf2, 0x00, BUF_SIZE );

        /* データ受信要求開始 */
        _com_app_ReqRecvData2( (UINT32)l_usRcvSize2, l_pucDtRcvBuf2 );
    }

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_RecvData2                                                           */
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
LOCAL VOID _com_app_cb_RecvData2( UINT32 ulSize, UINT8* pucData, VOID* pInfo, INT iStat )
{
INT                             iRet;
UINT32                          ulSendSize;

    if( GRUSB_COMD_COMPLETE == iStat )
    {
        if( (UINT32)l_usRcvSize2 < ulSize )
        {
            /* バッファオーバーフロー */
            return;
        }
        else if( 0 == ulSize )
        {
            /* データ受信要求開始 */
            _com_app_ReqRecvData2( (UINT32)l_usRcvSize2, l_pucDtRcvBuf2 );

            return;
        }

        /* 受信データを送信データバッファにコピー */
        memcpy( l_pucDtSndBuf2, pucData, ulSize );

#ifdef INTR_TEST_CMD_CHECK
        /* Notification開始コマンドチェック */
        iRet = com_app_CheckNotificationCmd2( l_pucDtSndBuf2 );

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
        iRet = _com_app_ReqSendData2( ulSendSize, l_pucDtSndBuf2 );

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
/* FUNCTION   : _com_app_cb_SendData2                                                           */
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
LOCAL VOID _com_app_cb_SendData2( UINT32 ulSize, UINT8* pucData, VOID* pInfo )
{
#ifdef INTR_TEST_CMD_CHECK
UINT8                           ucNotificationNo;

    /* Notification番号の取得 */
    ucNotificationNo = com_app_GetNotificationNo2();

    if( 0 != ucNotificationNo )
    {
        /* COM Notification送信 */
        com_app_SendNotification2( ucNotificationNo );
    }
#endif

    /* 送信データバッファクリア */
    memset( l_pucDtSndBuf2, 0x00, BUF_SIZE );

    /* データ受信要求開始 */
    _com_app_ReqRecvData2( (UINT32)l_usRcvSize2, l_pucDtRcvBuf2 );

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_cb_RecvData2                                                           */
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
LOCAL VOID _com_app_cb_RecvData2( UINT32 ulSize, UINT8* pucData, VOID* pInfo )
{
INT                             iStat;
UINT32                          ulSendSize;

    if( (UINT32)l_usRcvSize2 < ulSize )
    {
        /* バッファオーバーフロー */
        return;
    }
    else if( 0 == ulSize )
    {
        /* データ受信要求開始 */
        _com_app_ReqRecvData2( (UINT32)l_usRcvSize2, l_pucDtRcvBuf2 );

        return;
    }

    /* 受信データを送信データバッファにコピー */
    memcpy( l_pucDtSndBuf2, pucData, ulSize );

#ifdef INTR_TEST_CMD_CHECK
    /* Notification開始コマンドチェック */
    iStat = com_app_CheckNotificationCmd2( l_pucDtSndBuf2 );

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

    /* データ送信要求 */
    iStat = _com_app_ReqSendData2( ulSendSize, l_pucDtSndBuf2 );

    if( COM_APP_OK != iStat )
    {
        /* 送信要求エラー */
        return;
    }

    return;
}
#endif

/************************************************************************************************/
/* FUNCTION   : _com_app_ReqSendData2                                                           */
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
LOCAL INT _com_app_ReqSendData2( UINT32 ulSize, UINT8* pucBuf )
{
INT                             iStat;
INT                             iRet = COM_APP_OK;

    /* データ送信要求 */
    iStat = GRUSB_COMD_SendData2( ulSize, pucBuf, GRUSB_NULL );

    if( GRUSB_COMD_SUCCESS != iStat )
    {
        /* 送信要求エラー */
        iRet = COM_APP_ERROR;
    }

    return iRet;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_ReqRecvData2                                                           */
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
LOCAL INT _com_app_ReqRecvData2( UINT32 ulSize, UINT8* pucBuf )
{
INT                             iStat;
INT                             iRet = COM_APP_OK;

    /* 受信データバッファクリア */
    memset( pucBuf, 0x00, BUF_SIZE );

    /* データ受信要求 */
    iStat = GRUSB_COMD_ReciveData2( ulSize, pucBuf, GRUSB_NULL );

    if( GRUSB_COMD_SUCCESS != iStat )
    {
        /* 受信要求エラー */
        iRet = COM_APP_ERROR;
    }

    return iRet;
}
