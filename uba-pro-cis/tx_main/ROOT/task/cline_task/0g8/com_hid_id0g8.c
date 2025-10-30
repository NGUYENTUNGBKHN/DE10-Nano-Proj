/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      com_hid_0g82.c                                                          1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Communication Function Driver アプリケーション 2                                         */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   JCM-HQ     2022/06/21  V1.00                                                               */
/*                          Created initial version                                             */
/*                                                                                              */
/************************************************************************************************/

/**** COMPILE OPTIONS ***************************************************************************/
/* Interrupt転送テストを使用する場合はこの定義を有効にする */
#define INTR_TEST_CMD_CHECK

/**** INCLUDE FILES *****************************************************************************/
#include <string.h>
#include "kernel_inc.h"
#include "common.h"
#include "grp_cyclonev_tg.h"
#include "grp_cyclonev_cfg.h"

#include "com_hid_def.h"
#include "com_hid.h"
#include "com_hid_id0g8.h"

#include  "IF_ID0G8.def"

#define EXT
#include "com_ram.c"

#ifdef INTR_TEST_CMD_CHECK
#include "../../../usb/com_app_intr_cmd.h"
#endif /* INTR_TEST_CMD_CHECK */

/**** STRUCTURE PROTOTYPES **********************************************************************/



/**** INTERNAL DATA DEFINES *********************************************************************/
//#define BUF_SIZE                (512)                                         /* バッファサイズ */

/* ------------------------------------------------------------------------------------ */
/* HID_RXBUFF_RINGを定義すると、受信バッファをリングバッファとしてデータを格納する          */
/*                  定義しなければ、受信バッファをキューとしてデータを格納する              */
/*                                                                                      */
/* Ring  : オーバフロー時は読込みポイントを書込みポイントへずらす (前回までのデータは捨てる) */
/*                                                                                      */
/* Queue : オーバフロー時はバッファ先頭から書込む (前回までのデータは捨てる)                */
/*         シンプルなのでこちらを使用する                                                 */
/* ------------------------------------------------------------------------------------ */
//#define HID_RXBUFF_RING

/* Line Coding */
#define LINE_CODING_BUF_SIZE    (8)                              /* Line Coding バッファサイズ  */
#define LINE_CODING_SIZE        (7)                              /* Line Coding Structureサイズ */
#define APP_BAUDRATE            (9600)                           /* ボーレート初期値            */
#define APP_STOPBITS            (0)                              /* ストップビット初期値        */
#define APP_PARITY              (0)                              /* パリティ初期値              */
#define APP_DATABITS            (8)                              /* データビット初期値          */


/**** INTERNAL VARIABLE DEFINES *****************************************************************/
/* ポートステータス */
DLOCAL BOOLEAN                  l_bPortStatHid0G8;//これは使用してない、正常に動かない
/* Control Signal Bitmap */
DLOCAL UINT16                   l_usControlLineStateHid0G8;
/* Line Coding Structureバッファ */
DLOCAL UINT8                    l_aucLineCodingHid0G8[LINE_CODING_BUF_SIZE];
/* 送信データバッファ */
//DLOCAL UINT32                   l_aulDtSndBufHid0G8[COM_HID_BUF_SIZE/sizeof(UINT32)];
//DLOCAL UINT8*                   l_pucDtSndBufHid0G8 = (UINT8*)l_aulDtSndBufHid0G8;
/* 受信データバッファ */
DLOCAL UINT8                    l_aucDtRcvBufHid0G8[COM_HID_BUF_SIZE];
DLOCAL UINT16                   l_usRcvBufWriteSize;
#if defined(HID_RXBUFF_RING)
DLOCAL UINT16                   l_usRcvBufReadP;
DLOCAL UINT16                   l_usRcvBufWriteP;;
#endif
//DLOCAL UINT32                   l_aulDtRcvBufHid0G8[COM_HID_BUF_SIZE/sizeof(UINT32)];
//DLOCAL UINT8*                   l_pucDtRcvBufHid0G8 = (UINT8*)l_aulDtRcvBufHid0G8;
/* 受信データサイズ */
DLOCAL UINT16                   l_usRevDataSizeHid0G8;
DLOCAL UINT16                   l_usSerialNomber[SERIAL_NUMBER_SIZE];
DLOCAL UINT16                   l_usInterface[GRCOMD_HID_IF_CHARS];


/**** EXTERNAL VARIABLE DEFINES *****************************************************************/
/* iInterface(HID I/Fに関する文字列ディスクリプタ)のヘッド部分 */
extern const UINT8 Head_iInterface_GSA[];
/* Firmware Issue Field of iInterface Descriptor */
extern const UINT8 DucGdsFirmwareIssue[];
/* Build Version Field of iInterface Descriptor */
extern const UINT8 DucGdsBuildVersion[];
/* Manufacturing Date Field of iInterface Descriptor */
extern const UINT8 DucGdsManufacuturingDate[];

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL VOID _com_hid_0g8_cb_ConnStat2( INT );
LOCAL VOID _com_hid_0g8_cb_SendEncapsulatedCommad2( UINT32, UINT8* );
LOCAL VOID _com_hid_0g8_cb_GetEncapsulatedResponse2( UINT16 );
LOCAL VOID _com_hid_0g8_cb_SetLineCoding2( UINT32, UINT8* );
LOCAL VOID _com_hid_0g8_cb_GetLineCoding2( UINT16 );
LOCAL VOID _com_hid_0g8_cb_SetControlLineState2( UINT16 );
LOCAL VOID _com_hid_0g8_cb_NetworkConnection2( VOID );
LOCAL VOID _com_hid_0g8_cb_ResponseAvailable2( VOID );
LOCAL VOID _com_hid_0g8_cb_SerialState2( VOID );
#if 0//#ifdef GRCOMD_COMP_STATUS_USE //not use
LOCAL VOID _com_hid_0g8_cb_SendData2( UINT32, UINT8*, VOID*, INT );
LOCAL VOID _com_hid_0g8_cb_RecvData2( UINT32, UINT8*, VOID*, INT );
#else
LOCAL VOID _com_hid_0g8_cb_SendData2( UINT32, UINT8*, VOID* );
LOCAL VOID _com_hid_0g8_cb_RecvData2( UINT32, UINT8*, VOID* );
#endif
LOCAL INT  _com_hid_0g8_ReqSendData2( UINT32, UINT8* );
LOCAL INT  _com_hid_0g8_ReqRecvData2( UINT32, UINT8* );
LOCAL VOID _com_hid_0g8_cb_DfuDetach2(  UINT16 usLength  );


static INT ex_usb_status_0g8_low;

/************************************************************************************************/
/* FUNCTION   : com_app_Init2                                                                   */
/*                                                                                              */
/* DESCRIPTION: Communication Function Driver アプリケーションの初期化処理                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          データバッファサイズ                             */
/*              pucData                         データバッファポインタ                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : COM_HID_OK                      Success                                         */
/*              COM_HID_ERROR                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
// INT com_hid_0g8_Init2( VOID )
INT com_hid_0g8_Init2( u8 flag )
{
GRUSB_COMD_HID_INITINFO         tInitPrm;
INT                             iStat;
INT                             iCnt;
INT                             iNum;

#if !defined(ID0G8_BOOTIF)
    if(flag == 0)
    {
        /* H/W init */
        if ( GRP_TARGET_OK != grp_target_HwInit2( GRP_CYCLONEV_MODE_DEVICE ) )
        {
            /* H/W Init Error */
            return COM_HID_ERROR;
        }
    }
#endif

    /* ポートステータスの初期化 */
    l_bPortStatHid0G8 = GRUSB_FALSE;
    /* 受信データサイズ初期化 */
    l_usRevDataSizeHid0G8 = 0;
    /* Control Signal Bitmapの初期化 */
    l_usControlLineStateHid0G8 = 0;
    /* Line Coding バッファ初期化 */
    memset( l_aucLineCodingHid0G8, 0x00, LINE_CODING_BUF_SIZE );
    /* 送信データバッファ初期化 */
    //memset( l_pucDtSndBufHid0G8, 0x00, COM_HID_BUF_SIZE );
    /* 受信データバッファ初期化 */
    //memset( l_pucDtRcvBufHid0G8, 0x00, COM_HID_BUF_SIZE );
    memset( l_aucDtRcvBufHid0G8, 0x00, COM_HID_BUF_SIZE);
    l_usRcvBufWriteSize = 0;
#if defined(HID_RXBUFF_RING)
    l_usRcvBufReadP = 0;
    l_usRcvBufWriteP = 0;
#endif

    /* コールバック関数の設定 */
    tInitPrm.pfnConnStat              = _com_hid_0g8_cb_ConnStat2;
    tInitPrm.pfnSendEncapsulatedCmd   = _com_hid_0g8_cb_SendEncapsulatedCommad2;
    tInitPrm.pfnGetEncapsulatedRes    = _com_hid_0g8_cb_GetEncapsulatedResponse2;
    tInitPrm.pfnSetCommFeature        = GRUSB_NULL;
    tInitPrm.pfnGetCommFeature        = GRUSB_NULL;
    tInitPrm.pfnClearCommFeature      = GRUSB_NULL;
    tInitPrm.pfnSetLineCoding         = _com_hid_0g8_cb_SetLineCoding2;
    tInitPrm.pfnGetLineCoding         = _com_hid_0g8_cb_GetLineCoding2;
    tInitPrm.pfnSetControlLineState   = _com_hid_0g8_cb_SetControlLineState2;
    tInitPrm.pfnSendBreak             = GRUSB_NULL;
    tInitPrm.pfnNetworkConnection     = _com_hid_0g8_cb_NetworkConnection2;
    tInitPrm.pfnResponseAvailable     = _com_hid_0g8_cb_ResponseAvailable2;
    tInitPrm.pfnSerialState           = _com_hid_0g8_cb_SerialState2;
    tInitPrm.pfnSendData              = _com_hid_0g8_cb_SendData2;
    tInitPrm.pfnReciveData            = _com_hid_0g8_cb_RecvData2;  /* HID use all cmd, include Detach cmd, DFU use only Download data, */
 
    //Detachコマンド処理は、割り込みで直接フラグを立てる
    //理由は、Host側が先にUSB Reset処理を行う場合があるので、DUFモード用のPID切替が間に合わない可能性があるので、
    //他のコマンドは、通常通り_com_hid_0g8_cb_RecvData2で処理する
    //
    tInitPrm.pfnHidDetach             = _com_hid_0g8_cb_DfuDetach2; /* DFU use Detach cmd */

    /* Communication Function Driverの初期化 */
    iStat = GRUSB_COMD_HID_Init2( &tInitPrm );

    if( GRUSB_COMD_HID_SUCCESS != iStat )
    {
        /* error */
        return COM_HID_ERROR;
    }

#ifdef INTR_TEST_CMD_CHECK
    /* Notification情報の初期化 */
    com_app_NotificationInit();
#endif

    /*--- Line Codingの初期値を設定 ---*/
    l_aucLineCodingHid0G8[0] = (UINT8)( APP_BAUDRATE & 0x000000FF );           /* ボーレート：LSB */
    l_aucLineCodingHid0G8[1] = (UINT8)( ( APP_BAUDRATE & 0x0000FF00 ) >>  8 ); /*                 */
    l_aucLineCodingHid0G8[2] = (UINT8)( ( APP_BAUDRATE & 0x00FF0000 ) >> 16 ); /*                 */
    l_aucLineCodingHid0G8[3] = (UINT8)( ( APP_BAUDRATE & 0xFF000000 ) >> 24 ); /* ボーレート：MSB */
    l_aucLineCodingHid0G8[4] = APP_STOPBITS;                                   /* ストップビット  */
    l_aucLineCodingHid0G8[5] = APP_PARITY;                                     /* パリティ        */
    l_aucLineCodingHid0G8[6] = APP_DATABITS;                                   /* データビット    */

    /* ユーザ指定のデバイスの製造番号(シリアルナンバー)を設定する場合は  */
    /* ここでGRUSB_COMD_SetSerialNumber2()をコールして下さい。           */
    /* (注)製造番号は、UNICODE(2バイトコード)形式で渡して下さい。        */
#if 0  /* TODO: DEBUG用 削除する */
    strcpy(ex_adjustment_data.maintenance_info.serial_no, "180100000103");
#endif /* TODO: DEBUG用 削除する */
    for (iCnt = 0; iCnt < SERIAL_NUMBER_SIZE; iCnt++)
    {
        l_usSerialNomber[iCnt] = (0x00FF & ex_adjustment_data.maintenance_info.serial_no[iCnt]);
    }
    GRUSB_COMD_HID_SetSerialNumber2(l_usSerialNomber, 12);
    
    /* iInterfaceを設定する */
    iNum = 0;
	for (iCnt = 0; Head_iInterface_GSA[iCnt] != 0; iCnt++)
    {
        l_usInterface[iNum++] = (0x00FF & Head_iInterface_GSA[iCnt]);
    }
    for (iCnt = 0; DucGdsFirmwareIssue[iCnt] != 0; iCnt++)
    {
        l_usInterface[iNum++] = (0x00FF & DucGdsFirmwareIssue[iCnt]);
    }
    for (iCnt = 0; DucGdsBuildVersion[iCnt] != 0; iCnt++)
    {
        l_usInterface[iNum++] = (0x00FF & DucGdsBuildVersion[iCnt]);
    }
    for (iCnt = 0; DucGdsManufacuturingDate[iCnt] != 0; iCnt++)
    {
        l_usInterface[iNum++] = (0x00FF & DucGdsManufacuturingDate[iCnt]);
    }
    if (iNum <= GRCOMD_HID_IF_CHARS)
    {
        GRUSB_COMD_HID_SetInterface2(l_usInterface, iNum);
    }

    /* Pullup ON */
    GRUSB_DEV_ApReqPullupRegister2( GRUSB_TRUE );

    /* Enable of D+ pull up is done in com_app_Init2. */
    /* Start interrupt */
    if ( GRP_TARGET_OK != grp_target_StartIntr2( GRP_CYCLONEV_MODE_DEVICE ) )
    {
        /* Start Interrupt Error */
        return COM_HID_ERROR;
    }

    return COM_HID_OK;
}

VOID com_hid_0g8_detach( VOID )
{
    /* Stip interrupt */
    grp_target_StopIntr2( GRP_CYCLONEV_MODE_DEVICE );
    
    /* Pullup OFF */
    GRUSB_DEV_ApReqPullupRegister2( GRUSB_FALSE );
}

/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_cb_ConnStat2                                                           */
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
LOCAL VOID _com_hid_0g8_cb_ConnStat2( INT iConnStat )
{
    if( GRUSB_COMD_HID_CON == iConnStat ) // 1 configuまで完了
    {
        /* 接続 */
    	ex_usb_status_0g8_low = iConnStat;
        iset_flg(ID_USB2_CB_FLAG, EVT_USB_CON);
#if 0
        /* Bulk OUTエンドポイントのMaxPacketSizeを取得        */
        /* ※取得したサイズは受信要求サイズとして使用します。 */
        l_usRcvSizeHid0G8 = GRUSB_DEV_ApGetMaxPacketSize2( GRCOMD_BULKOUT_EP_NUMBER );

        /* データ受信要求開始 */
        _com_hid_0g8_ReqRecvData2( (UINT32)l_usRcvSizeHid0G8, l_pucDtRcvBufHid0G8 );
#endif
    }
    else if( 2 == iConnStat )   //NEW_BUS_RESET configu前のbus reset発生(ラインモニタ上でのReset,通常のPCでも2発は来る)
    {
        //Detachでは発生しない、USB接続で発生するが、Bus Resetなので接続時に数発発生する
        SignalUsbBusResetCmd();

    	ex_usb_status_0g8_low = iConnStat;
        iset_flg(ID_USB2_CB_FLAG, EVT_USB_CON);

    }
    else // 0 configu後のUSB断
    {
        /* 切断 */
    	ex_usb_status_0g8_low = iConnStat;
        iset_flg(ID_USB2_CB_FLAG, EVT_USB_CON);

        if( GRUSB_TRUE == l_bPortStatHid0G8 )
        {
            l_bPortStatHid0G8 = GRUSB_FALSE;    //これは使用してない、正常に動かない
 
        }

 

#ifdef INTR_TEST_CMD_CHECK
        /* Notification情報の初期化 */
        com_app_NotificationInit2();
#endif
    }


    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_cb_SendEncapsulatedCommad2                                         */
/*                                                                                              */
/* DESCRIPTION: SEND_ENCAPSULATED_COMMANDリクエスト受信処理                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          データサイズ                                     */
/*              pucData                         データバッファポインタ                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_hid_0g8_cb_SendEncapsulatedCommad2( UINT32 ulSize, UINT8* pucData )
{
    /*----------------------------------------------------------------------*/
    /* SEND_ENCAPSULATED_COMMANDリクエストを受信すると本関数がコールされ    */
    /* ます。Abstract Control Modelにおいて必須機能のため、コールバックの   */
    /* 設定を行っていますが、本サンプルではコールされることはありません。   */
    /*----------------------------------------------------------------------*/

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_cb_GetEncapsulatedResponse2                                        */
/*                                                                                              */
/* DESCRIPTION: GET_ENCAPSULATED_RESPONSEリクエスト受信処理                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_hid_0g8_cb_GetEncapsulatedResponse2( UINT16 usLength )
{
    /*----------------------------------------------------------------------*/
    /* GET_ENCAPSULATED_RESPONSEリクエストを受信すると本関数がコールされ    */
    /* ます。Abstract Control Modelにおいて必須機能のため、コールバックの   */
    /* 設定を行っていますが、本サンプルではコールされることはありません。   */
    /*----------------------------------------------------------------------*/

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_cb_SetLineCoding2                                                  */
/*                                                                                              */
/* DESCRIPTION: SET_LINE_CODINGリクエスト受信処理                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          Line Coding Structureサイズ                     */
/*              pucData                         Line Coding Structureバッファポインタ            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_hid_0g8_cb_SetLineCoding2( UINT32 ulSize, UINT8* pucData )
{
    /* Line Coding Structureバッファにコピー */
    memcpy( l_aucLineCodingHid0G8, pucData, ulSize );

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_cb_GetLineCoding2                                                  */
/*                                                                                              */
/* DESCRIPTION: GET_LINE_CODINGリクエスト受信処理                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_hid_0g8_cb_GetLineCoding2( UINT16 usLength )
{
    /* Line Coding Structure送信 */
    GRUSB_COMD_HID_Set_GetLineCoding2( (UINT32)usLength, l_aucLineCodingHid0G8 );

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_cb_SetControlLineState2                                            */
/*                                                                                              */
/* DESCRIPTION: SET_CONTROL_LINE_STATEリクエスト受信処理                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usValue                         Control Signal Bitmap                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_hid_0g8_cb_SetControlLineState2( UINT16 usValue )
{
UINT16                          usRTS;
UINT16                          usDTR;

    /* Control Signal Bitmapをセット */
    l_usControlLineStateHid0G8 = usValue;

    /* RTS */
    usRTS = (UINT16)( ( l_usControlLineStateHid0G8 & 0x0002 ) >> 1 );
    /* DTR */
    usDTR = (UINT16)( l_usControlLineStateHid0G8 & 0x0001 );

    if( ( 1 == usRTS ) && ( 1 == usDTR ) )
    {
        /* ポート オープン */

        if( GRUSB_FALSE == l_bPortStatHid0G8 )
        {
            l_bPortStatHid0G8 = GRUSB_TRUE;
        }
    }
    else
    {
        /* ポート クローズ */

        if( GRUSB_TRUE == l_bPortStatHid0G8 )
        {
            l_bPortStatHid0G8 = GRUSB_FALSE;
        }
    }

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_cb_NetworkConnection2                                              */
/*                                                                                              */
/* DESCRIPTION: NETWORK_CONNECTION通知完了処理                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_hid_0g8_cb_NetworkConnection2( VOID )
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
/* FUNCTION   : _com_hid_0g8_cb_ResponseAvailable2                                              */
/*                                                                                              */
/* DESCRIPTION: RESPONSE_AVAILABLE通知完了処理                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_hid_0g8_cb_ResponseAvailable2( VOID )
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
/* FUNCTION   : _com_hid_0g8_cb_SerialState2                                                    */
/*                                                                                              */
/* DESCRIPTION: SERIAL_STATE通知完了処理                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_hid_0g8_cb_SerialState2( VOID )
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

#if 0//#ifdef GRCOMD_COMP_STATUS_USE //not use
/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_cb_SendData2                                                       */
/*                                                                                              */
/* DESCRIPTION: データ送信完了通知処理                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          送信データサイズ                                 */
/*              pucData                         送信データバッファポインタ                       */
/*              pInfo                           情報ポインタ                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_hid_0g8_cb_SendData2( UINT32 ulSize, UINT8* pucData, VOID* pInfo, INT iStat )
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
#if 0
    if( GRUSB_COMD_COMPLETE == iStat )
    {
        /* 送信データバッファクリア */
        memset( l_pucDtSndBufHid0G8, 0x00, COM_HID_BUF_SIZE );

        /* データ受信要求開始 */
        _com_hid_0g8_ReqRecvData2( (UINT32)l_usRcvSizeHid0G8, l_pucDtRcvBufHid0G8 );
    }
#endif

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_cb_RecvData2                                                       */
/*                                                                                              */
/* DESCRIPTION: データ受信完了通知処理                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          受信データサイズ                                 */
/*              pucData                         受信データバッファポインタ                       */
/*              pInfo                           情報ポインタ                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_hid_0g8_cb_RecvData2( UINT32 ulSize, UINT8* pucData, VOID* pInfo, INT iStat )
{
#if !defined(HID_RXBUFF_RING)
/* ------------------------------------------------------------------------------------ */
/* Queue : オーバフロー時はバッファ先頭から書込む (前回までのデータは捨てる)                */
/*         シンプルなのでこちらを使用する                                                 */
/* ------------------------------------------------------------------------------------ */
INT                             iRet;

    if( GRUSB_COMD_COMPLETE == iStat )
    {
        if (ulSize != 0)
        {
            /* バッファのオーバーフロー確認 */
            if ((l_usRcvBufWriteSize + ulSize) > COM_HID_BUF_SIZE)
            {
                /* 前回までのデータは捨てる */
                l_usRcvBufWriteSize = 0;
            }
            /* 追加書込み */
            memcpy(&l_aucDtRcvBufHid0G8[l_usRcvBufWriteSize], pucData, ulSize);
            l_usRcvBufWriteSize += ulSize;

            iset_flg(ID_USB2_CB_FLAG, EVT_USB_RCV);
        }
    }

    return;
#else
/* ------------------------------------------------------------------------------------ */
/* Ring  : オーバフロー時は読込みポイントを書込みポイントへずらす (前回までのデータは捨てる) */
/* ------------------------------------------------------------------------------------ */
INT                             iStat;
UINT16                          usNextWriteP;
UINT16                          usWsize;
UINT16                          usWsize2;


    if( GRUSB_COMD_COMPLETE == iStat )
    {
        if (ulSize != 0)
        {
            /* バッファの終端確認 (書込みポイント+書込みサイズがバッファ最大数を超えるか?) */
            if ((l_usRcvBufWriteP + ulSize) > COM_HID_BUF_SIZE)
            {
            /* バッファ周回 */
                /* バッファ終端まで書込み */
                usWsize = (COM_HID_BUF_SIZE - l_usRcvBufWriteP);
                memcpy(&l_aucDtRcvBufHid0G8[l_usRcvBufWriteP], pucData, usWsize);
                /* バッファ先頭から書込み */
                usWsize2 = (ulSize - usWsize);
                memcpy(&l_aucDtRcvBufHid0G8[0], (pucData + usWsize), usWsize2);

                /* 書込みポイントの更新 */
                usNextWriteP = ((l_usRcvBufWriteP + ulSize) - COM_HID_BUF_SIZE);
                if (usNextWriteP >= l_usRcvBufReadP)
                {
                    /* 読込みポイントを上書きした場合は、前回までのデータは捨てる */
                    l_usRcvBufReadP = l_usRcvBufWriteP;
                    l_usRcvBufWriteSize = 0;
                }
                l_usRcvBufWriteP = usNextWriteP;

                /* 書込みサイズ更新 */
                l_usRcvBufWriteSize += ulSize;
            }
            else
            {
            /* バッファ周回なし */
                /* 書込み */
                memcpy(&l_aucDtRcvBufHid0G8[l_usRcvBufWriteP], pucData, ulSize);
                
                /* 書込みポイントの更新 */
                if ((l_usRcvBufWriteP + ulSize) >= COM_HID_BUF_SIZE)
                {
                    usNextWriteP = 0;
                }
                else
                {
                    usNextWriteP = (l_usRcvBufWriteP + ulSize);
                }
                if ((l_usRcvBufWriteP < l_usRcvBufReadP) && (usNextWriteP > l_usRcvBufReadP))
                {
                    /* 読込みポイントを上書きした場合は前回までのデータは捨てる */
                    l_usRcvBufReadP = l_usRcvBufWriteP;
                    l_usRcvBufWriteSize = 0;
                }
                l_usRcvBufWriteP = usNextWriteP;

                /* 書込みサイズ更新 */
                l_usRcvBufWriteSize += ulSize;
            }
            iset_flg(ID_USB2_CB_FLAG, EVT_USB_RCV);
        }
    }

    return;
#endif
}
#else
/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_cb_SendData2                                                       */
/*                                                                                              */
/* DESCRIPTION: データ送信完了通知処理                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          送信データサイズ                                 */
/*              pucData                         送信データバッファポインタ                       */
/*              pInfo                           情報ポインタ                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_hid_0g8_cb_SendData2( UINT32 ulSize, UINT8* pucData, VOID* pInfo )
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
#if 0
    /* 送信データバッファクリア */
    memset( l_pucDtSndBufHid0G8, 0x00, COM_HID_BUF_SIZE );

    /* データ受信要求開始 */
    _com_hid_0g8_ReqRecvData2( (UINT32)l_usRcvSizeHid0G8, l_pucDtRcvBufHid0G8 );
#endif

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_cb_RecvData2                                                       */
/*                                                                                              */
/* DESCRIPTION: データ受信完了通知処理                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          受信データサイズ                                */
/*              pucData                         受信データバッファポインタ                       */
/*              pInfo                           情報ポインタ                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_hid_0g8_cb_RecvData2( UINT32 ulSize, UINT8* pucData, VOID* pInfo )
{
#if !defined(HID_RXBUFF_RING)
/* ------------------------------------------------------------------------------------ */
/* Queue : オーバフロー時はバッファ先頭から書込む (前回までのデータは捨てる)                */
/*         シンプルなのでこちらを使用する                                                 */
/* ------------------------------------------------------------------------------------ */
INT                             iStat;


    if (ulSize != 0)
    {
        /* バッファのオーバーフロー確認 */
        if ((l_usRcvBufWriteSize + ulSize) > COM_HID_BUF_SIZE)
        {
            /* 前回までのデータは捨てる */
            l_usRcvBufWriteSize = 0;
        }
        /* 追加書込み */
        memcpy(&l_aucDtRcvBufHid0G8[l_usRcvBufWriteSize], pucData, ulSize);
        l_usRcvBufWriteSize += ulSize;

        iset_flg(ID_USB2_CB_FLAG, EVT_USB_RCV);
    }

    return;
#else
/* ------------------------------------------------------------------------------------ */
/* Ring  : オーバフロー時は読込みポイントを書込みポイントへずらす (前回までのデータは捨てる) */
/* ------------------------------------------------------------------------------------ */
INT                             iStat;
UINT16                          usNextWriteP;
UINT16                          usWsize;
UINT16                          usWsize2;


    if (ulSize != 0)
    {
        /* バッファの終端確認 (書込みポイント+書込みサイズがバッファ最大数を超えるか?) */
        if ((l_usRcvBufWriteP + ulSize) > COM_HID_BUF_SIZE)
        {
        /* バッファ周回 */
            /* バッファ終端まで書込み */
            usWsize = (COM_HID_BUF_SIZE - l_usRcvBufWriteP);
            memcpy(&l_aucDtRcvBufHid0G8[l_usRcvBufWriteP], pucData, usWsize);
            /* バッファ先頭から書込み */
            usWsize2 = (ulSize - usWsize);
            memcpy(&l_aucDtRcvBufHid0G8[0], (pucData + usWsize), usWsize2);

            /* 書込みポイントの更新 */
            usNextWriteP = ((l_usRcvBufWriteP + ulSize) - COM_HID_BUF_SIZE);
            if (usNextWriteP >= l_usRcvBufReadP)
            {
                /* 読込みポイントを上書きした場合は、前回までのデータは捨てる */
                l_usRcvBufReadP = l_usRcvBufWriteP;
                l_usRcvBufWriteSize = 0;
            }
            l_usRcvBufWriteP = usNextWriteP;

            /* 書込みサイズ更新 */
            l_usRcvBufWriteSize += ulSize;
        }
        else
        {
        /* バッファ周回なし */
            /* 書込み */
            memcpy(&l_aucDtRcvBufHid0G8[l_usRcvBufWriteP], pucData, ulSize);
            
            /* 書込みポイントの更新 */
            if ((l_usRcvBufWriteP + ulSize) >= COM_HID_BUF_SIZE)
            {
                usNextWriteP = 0;
            }
            else
            {
                usNextWriteP = (l_usRcvBufWriteP + ulSize);
            }
            if ((l_usRcvBufWriteP < l_usRcvBufReadP) && (usNextWriteP > l_usRcvBufReadP))
            {
                /* 読込みポイントを上書きした場合は前回までのデータは捨てる */
                l_usRcvBufReadP = l_usRcvBufWriteP;
                l_usRcvBufWriteSize = 0;
            }
            l_usRcvBufWriteP = usNextWriteP;

            /* 書込みサイズ更新 */
            l_usRcvBufWriteSize += ulSize;
        }
        iset_flg(ID_USB2_CB_FLAG, EVT_USB_RCV);
    }

    return;
#endif
}
#endif

/************************************************************************************************/
/* FUNCTION   : com_hid_0g8_listen                                                              */
/*                                                                                              */
/* DESCRIPTION: データ受信確認                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : ulSize                          受信データ数                                     */
/*                                                                                              */
/************************************************************************************************/
UINT32 com_hid_0g8_listen( UINT8 *pucBuf )
{
#if !defined(HID_RXBUFF_RING)
/* ------------------------------------------------------------------------------------ */
/* Queue : オーバフロー時はバッファ先頭から書込む (前回までのデータは捨てる)                */
/*         シンプルなのでこちらを使用する                                                 */
/* ------------------------------------------------------------------------------------ */
UINT32 ulSize = 0;


    if (l_usRcvBufWriteSize != 0)
    {

        /* 割り込み禁止 (必要に応じて有効に) */
        //OSW_ISR_disable(160);
        
        /* 読込み */
        ulSize = l_usRcvBufWriteSize;
        memcpy(pucBuf, l_aucDtRcvBufHid0G8, ulSize);
        l_usRcvBufWriteSize = 0;


        //#ifdef _ENABLE_JDL	ID0G8_JDL
        jdl_comm_rx_pkt(pucBuf, (u8)ulSize);
        //jdl_comm_rx_pkt_id0g8();
        /* _ENABLE_JDL */

        // For DEBUG
        //memset(l_aucDtRcvBufHid0G8, 0x00, COM_HID_BUF_SIZE);

        /* 割り込み許可 (必要に応じて有効に) */
        //OSW_ISR_enable(160);
    }

    return ulSize;

#else
/* ------------------------------------------------------------------------------------ */
/* Ring  : オーバフロー時は読込みポイントを書込みポイントへずらす (前回までのデータは捨てる) */
/* ------------------------------------------------------------------------------------ */

UINT32 ulSize = 0;
UINT32 ulRead;
UINT32 ulRead2;
UINT16 usReadSize;


    if (l_usRcvBufWriteSize != 0)
    {

        /* 割り込み禁止 (必要に応じて有効に) */
        //OSW_ISR_disable(160);

        /* バッファの終端確認 (読込みポイントが書込みポイント以上か?) */
        if (l_usRcvBufReadP >= l_usRcvBufWriteP)
        {
        /* バッファ周回 */
            /* バッファ終端まで読込み */
            ulRead = (COM_HID_BUF_SIZE - l_usRcvBufReadP);
            memcpy(pucBuf, &l_aucDtRcvBufHid0G8[l_usRcvBufReadP], ulRead);
            /* バッファ先頭から読込み */
            ulRead2 = l_usRcvBufWriteP;
            memcpy((pucBuf + ulRead), &l_aucDtRcvBufHid0G8[0], ulRead2);
            ulSize = (ulRead + ulRead2);
        }
        else
        {
        /* 周回なし */
            /* 読込み */
            ulSize = (l_usRcvBufWriteP - l_usRcvBufReadP);
            memcpy(pucBuf, &l_aucDtRcvBufHid0G8[l_usRcvBufReadP], ulSize);
        }

        /* 読込みポイント更新 */
        l_usRcvBufReadP = l_usRcvBufWriteP;
        
        /* 書込みサイズ更新 */
        l_usRcvBufWriteSize = 0;

        // For DEBUG
        //memset(l_aucDtRcvBufHid0G8, 0x00, COM_HID_BUF_SIZE);
        
        /* 割り込み許可 (必要に応じて有効に) */
        //OSW_ISR_enable(160);
    }

    return ulSize;
#endif
}

/************************************************************************************************/
/* FUNCTION   : com_hid_0g8_send                                                                */
/*                                                                                              */
/* DESCRIPTION: データ送信要求                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          送信データサイズ                                 */
/*              pucBuf                          送信データバッファポインタ                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : COM_HID_OK                      Success                                         */
/*              COM_HID_ERROR                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
UINT32 com_hid_0g8_send( UINT32 ulSize, UINT8* pucBuf )
{
INT                             iStat;
INT                             iRet = COM_HID_OK;

    iStat = GRUSB_COMD_HID_Notification2(ulSize, pucBuf);
    if( GRUSB_COMD_HID_SUCCESS != iStat )
    {
        /* 送信要求エラー */
        iRet = COM_HID_ERROR;
    }
    return iRet;
}

/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_ReqSendData2                                                       */
/*                                                                                              */
/* DESCRIPTION: データ送信要求                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          送信データサイズ                                 */
/*              pucBuf                          送信データバッファポインタ                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : COM_HID_OK                      Success                                         */
/*              COM_HID_ERROR                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL INT _com_hid_0g8_ReqSendData2( UINT32 ulSize, UINT8* pucBuf )
{
INT                             iStat;
INT                             iRet = COM_HID_OK;

    /* データ送信要求 */
    iStat = GRUSB_COMD_HID_SendData2( ulSize, pucBuf, GRUSB_NULL );

    if( GRUSB_COMD_HID_SUCCESS != iStat )
    {
        /* 送信要求エラー */
        iRet = COM_HID_ERROR;
    }

    return iRet;
}

/************************************************************************************************/
/* FUNCTION   : _com_hid_0g8_ReqRecvData2                                                       */
/*                                                                                              */
/* DESCRIPTION: データ受信要求                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          受信データサイズ                                 */
/*              pucBuf                          受信データバッファポインタ                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : COM_HID_OK                      Success                                         */
/*              COM_HID_ERROR                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL INT _com_hid_0g8_ReqRecvData2( UINT32 ulSize, UINT8* pucBuf )
{
INT                             iStat;
INT                             iRet = COM_HID_OK;

    /* 受信データバッファクリア */
    memset( pucBuf, 0x00, COM_HID_BUF_SIZE );

    /* データ受信要求 */
    iStat = GRUSB_COMD_HID_ReciveData2( ulSize, pucBuf, GRUSB_NULL );

    if( GRUSB_COMD_HID_SUCCESS != iStat )
    {
        /* 受信要求エラー */
        iRet = COM_HID_ERROR;
    }

    return iRet;
}


LOCAL VOID _com_hid_0g8_cb_DfuDetach2(  UINT16 usLength  ) // 割り込みから直接呼び出し
{

    DucDfuProgramModeFlag = PROGRAM_MODE_DFU_INI;

    DusDetachTimeout = usLength;
    DucUsbCmdNo = CMD_NUM_DFU_DETACH;


    //com_hid_0g8_detach(); //ここで通信を止めると、StatusステージのAck前に通信断となる

    return;
}

#if defined(_PROTOCOL_ENABLE_ID0G8)
u32 is_usb2_status_0g8(void)
{
	u32 bStat;

    bStat = ex_usb_status_0g8_low;
	return bStat;

}
#endif


//#ifdef _ENABLE_JDL	ID0G8_JDL
#if 0
void jdl_comm_rx_pkt_id0g8(void)
{
	u8 count=0;
	u8 length=0;
	u8 length_p=0;
	u8 data_start=0;

	length_p = ex_id0g8_length_get_point;
	length = ex_id0g8_length[length_p];

	/* 現在受信したコマンド数分保存	*/
	for(count=0; count<ex_pusb_pkt.current_cmd_count; count++)
	{

		jdl_comm_rx_pkt(&ex_pusb_read_buffer_0g8[data_start], (u8)length);

		if( ID0G8_COMMAND_LENGTH_BUFFER_SIZE - 1 >  length_p )
		{
			length_p++;
		}
		else
		{
		/* キューが1回りしている時 */
			length_p = 0;
		}
		data_start = length;
		length = ex_id0g8_length[length_p];
	}
}
#endif

/* _ENABLE_JDL */

