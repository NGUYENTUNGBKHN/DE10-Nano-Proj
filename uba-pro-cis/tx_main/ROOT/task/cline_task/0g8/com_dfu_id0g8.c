/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      com_dfu_id0g8.c                                                          1.00            */
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

#include "com_dfu_def.h"
#include "com_dfu.h"
#include "com_dfu_id0g8.h"

#include "IF_ID0G8.def" // 


#define EXT
#include "com_ram.c"

#ifdef INTR_TEST_CMD_CHECK
#include "../../../usb/com_app_intr_cmd.h"
#endif /* INTR_TEST_CMD_CHECK */

/**** STRUCTURE PROTOTYPES **********************************************************************/
extern u8 ex_down_start_0g8;


/**** INTERNAL DATA DEFINES *********************************************************************/
//#define BUF_SIZE                (512)                                         /* バッファサイズ */

/* Line Coding */
#define LINE_CODING_BUF_SIZE    (8)                              /* Line Coding バッファサイズ      */
#define LINE_CODING_SIZE        (7)                              /* Line Coding Structureサイズ    */
#define APP_BAUDRATE            (9600)                           /* ボーレート初期値                */
#define APP_STOPBITS            (0)                              /* ストップビット初期値            */
#define APP_PARITY              (0)                              /* パリティ初期値                  */
#define APP_DATABITS            (8)                              /* データビット初期値              */

#define STAT_TX_BUF_SIZE        (8)                              /* State/Status送信バッファサイズ  */


/**** INTERNAL VARIABLE DEFINES *****************************************************************/
/* ポートステータス */
DLOCAL BOOLEAN                  l_bPortStatDfu0G8;
/* 受信要求サイズ(MaxPacketSize) */
//DLOCAL UINT16                   l_usRcvSizeDfu0G8;
/* Control Signal Bitmap */
DLOCAL UINT16                   l_usControlLineStateDfu0G8;
/* Line Coding Structureバッファ */
DLOCAL UINT8                    l_aucLineCodingDfu0G8[LINE_CODING_BUF_SIZE];
/* 送信データバッファ */
//DLOCAL UINT32                   l_aulDtSndBufDfu0G8[COM_DFU_BUF_SIZE/sizeof(UINT32)];
//DLOCAL UINT8*                   l_pucDtSndBufDfu0G8 = (UINT8*)l_aulDtSndBufDfu0G8;
/* 受信データバッファ */
//DLOCAL UINT32                   l_aulDtRcvBufDfu0G8[COM_DFU_BUF_SIZE/sizeof(UINT32)];
//DLOCAL UINT8*                   l_pucDtRcvBufDfu0G8 = (UINT8*)l_aulDtRcvBufDfu0G8;
/* 受信データバッファポインタ */
DLOCAL UINT8*                   l_pucDtRcvBufDfu0G8;
/* 受信バッファサイズ */
DLOCAL UINT16                   l_usRcvBufSizeDfu0G8;

#if defined(ID0G8_BOOTIF)
/* 受信データサイズ */
//DLOCAL UINT16                   l_usRevDataSizeDfu0G8; // 未使用になる可能性が高い 代わりにDuwDnBlockSize
#endif

/* 送信データバッファポインタ */
#if 1 // 
UINT8*                   l_pucDtSndBufDfu0G8;
#else
DLOCAL UINT8*                   l_pucDtSndBufDfu0G8;
#endif
/* 送信バッファサイズ */
DLOCAL UINT16                   l_usSndBufSizeDfu0G8;
/* 送信データサイズ */
#if 1 // 
UINT16                   l_usSndDataSizeDfu0G8;
#else
DLOCAL UINT16                   l_usSndDataSizeDfu0G8;
#endif

DLOCAL UINT16                   l_usSerialNomber[SERIAL_NUMBER_SIZE];
DLOCAL UINT16                   l_usInterface[GRCOMD_DFU_IF_CHARS];


/**** EXTERNAL VARIABLE DEFINES *****************************************************************/
/* iInterface(DFU I/Fに関する文字列ディスクリプタ)のヘッド部分 */
#if defined(ID0G8_BOOTIF)  /* TODO: DEBUG用 削除する */
const UB Head_iInterface_GSA[] = "1.1.2,iVIZION-100-SS,";
const u8 DucGdsBuildVersion[] = "V0.00-00,";
const u8 DucGdsManufacuturingDate[] = "2021-08-31";
const u8 DucGdsFirmwareIssue[] = "i(USA)100-SS ID0G8-01,";
#else  /* TODO: DEBUG用 削除する */
extern const UINT8 Head_iInterface_GSA[];
/* Firmware Issue Field of iInterface Descriptor */
extern const UINT8 DucGdsFirmwareIssue[];
/* Build Version Field of iInterface Descriptor */
extern const UINT8 DucGdsBuildVersion[];
/* Manufacturing Date Field of iInterface Descriptor */
extern const UINT8 DucGdsManufacuturingDate[];
#endif /* TODO: DEBUG用 削除する */


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL VOID _com_dfu_0g8_cb_ConnStat2( INT );
LOCAL VOID _com_dfu_0g8_cb_SendEncapsulatedCommad2( UINT32, UINT8* );
LOCAL VOID _com_dfu_0g8_cb_GetEncapsulatedResponse2( UINT16 );
LOCAL VOID _com_dfu_0g8_cb_SetLineCoding2( UINT32, UINT8* );
LOCAL VOID _com_dfu_0g8_cb_GetLineCoding2( UINT16 );
LOCAL VOID _com_dfu_0g8_cb_SetControlLineState2( UINT16 );
LOCAL VOID _com_dfu_0g8_cb_NetworkConnection2( VOID );
LOCAL VOID _com_dfu_0g8_cb_ResponseAvailable2( VOID );
LOCAL VOID _com_dfu_0g8_cb_SerialState2( VOID );
#if 0//#ifdef GRCOMD_COMP_STATUS_USE //not use
LOCAL VOID _com_dfu_0g8_cb_SendData2( UINT32, UINT8*, VOID*, INT );
LOCAL VOID _com_dfu_0g8_cb_RecvData2( UINT32, UINT8*, VOID*, INT );
#else
LOCAL VOID _com_dfu_0g8_cb_SendData2( UINT32, UINT8*, VOID* );
LOCAL VOID _com_dfu_0g8_cb_RecvData2( UINT32, UINT8*, VOID* );
#endif
LOCAL INT  _com_dfu_0g8_ReqSendData2( UINT32, UINT8* );
LOCAL INT  _com_dfu_0g8_ReqRecvData2( UINT32, UINT8* );
LOCAL VOID _com_dfu_0g8_cb_Upload2( UINT16, UINT16 );
LOCAL VOID _com_dfu_0g8_cb_GetStatus2( UINT16 );
LOCAL VOID _com_dfu_0g8_cb_GetState2( UINT16 );
LOCAL VOID _com_dfu_0g8_cb_DfuDetach2( UINT16 );//  
//  LOCAL VOID _com_dfu_0g8_cb_DfuDetach2( VOID );
LOCAL VOID _com_dfu_0g8_cb_ClrStatus2( VOID );
LOCAL VOID _com_dfu_0g8_cb_Abort2( VOID );
LOCAL VOID _com_dfu_0g8_cb_Out_0data( VOID );   /*     */

/************************************************************************************************/
/* FUNCTION   : com_app_Init2                                                                   */
/*                                                                                              */
/* DESCRIPTION: Communication Function Driver アプリケーションの初期化処理                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          データバッファサイズ                             */
/*              pucData                         データバッファポインタ                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : COM_DFU_OK                      Success                                         */
/*              COM_DFU_ERROR                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
INT com_dfu_0g8_Init2( UINT32 ulRcvSize, UINT8 *pucRcvBuf, UINT32 ulSndSize, UINT8 *pucSndBuf )
{
GRUSB_COMD_DFU_INITINFO         tInitPrm;
INT                             iStat;
INT                             iCnt;
INT                             iNum;

#if defined(ID0G8_BOOTIF)
    /* H/W init */
    if ( GRP_TARGET_OK != grp_target_HwInit2( GRP_CYCLONEV_MODE_DEVICE ) )
    {
        /* H/W Init Error */
        return COM_DFU_ERROR;
    }
#endif

    /* ポートステータスの初期化 */
    l_bPortStatDfu0G8 = GRUSB_FALSE;
    /* 受信バッファサイズ設定 */
    l_usRcvBufSizeDfu0G8 = ulRcvSize;
    /* 受信バッファ設定 */
    l_pucDtRcvBufDfu0G8 = pucRcvBuf;
#if defined(ID0G8_BOOTIF)//2022-09-21
    /* 受信データサイズ初期化 */
    l_usRevDataSizeDfu0G8 = 0;
#endif    
    /* 送信バッファサイズ設定 */
    l_usSndBufSizeDfu0G8 = ulSndSize;
    /* 送信バッファ設定 */
    l_pucDtSndBufDfu0G8 = pucSndBuf;
    /* 送信データサイズ初期化 */
    l_usSndDataSizeDfu0G8 = 0;
    /* Control Signal Bitmapの初期化 */
    l_usControlLineStateDfu0G8 = 0;
    /* Line Coding バッファ初期化 */
    memset( l_aucLineCodingDfu0G8, 0x00, LINE_CODING_BUF_SIZE );
    /* 送信データバッファ初期化 */
    //memset( l_pucDtSndBufDfu0G8, 0x00, COM_DFU_BUF_SIZE );
    /* 受信データバッファ初期化 */
    //memset( l_pucDtRcvBufDfu0G8, 0x00, COM_DFU_BUF_SIZE );

    /* コールバック関数の設定 */
    tInitPrm.pfnConnStat              = _com_dfu_0g8_cb_ConnStat2;
    tInitPrm.pfnSendEncapsulatedCmd   = _com_dfu_0g8_cb_SendEncapsulatedCommad2;
    tInitPrm.pfnGetEncapsulatedRes    = _com_dfu_0g8_cb_GetEncapsulatedResponse2;
    tInitPrm.pfnSetCommFeature        = GRUSB_NULL;
    tInitPrm.pfnGetCommFeature        = GRUSB_NULL;
    tInitPrm.pfnClearCommFeature      = GRUSB_NULL;
    tInitPrm.pfnSetLineCoding         = _com_dfu_0g8_cb_SetLineCoding2;
    tInitPrm.pfnGetLineCoding         = _com_dfu_0g8_cb_GetLineCoding2;
    tInitPrm.pfnSetControlLineState   = _com_dfu_0g8_cb_SetControlLineState2;
    tInitPrm.pfnSendBreak             = GRUSB_NULL;
    tInitPrm.pfnNetworkConnection     = _com_dfu_0g8_cb_NetworkConnection2;
    tInitPrm.pfnResponseAvailable     = _com_dfu_0g8_cb_ResponseAvailable2;
    tInitPrm.pfnSerialState           = _com_dfu_0g8_cb_SerialState2;
    tInitPrm.pfnSendData              = _com_dfu_0g8_cb_SendData2;
    tInitPrm.pfnReciveData            = _com_dfu_0g8_cb_RecvData2;  /* DFU use only Download data, HID mode use all cmd, include Detach cmd */
    tInitPrm.pfnUploadRes             = _com_dfu_0g8_cb_Upload2;
    tInitPrm.pfnGetStatusRes          = _com_dfu_0g8_cb_GetStatus2;
    tInitPrm.pfnGetStateRes           = _com_dfu_0g8_cb_GetState2;
    tInitPrm.pfnDfuDetach             = _com_dfu_0g8_cb_DfuDetach2; /* DFU use Detach cmd */
    tInitPrm.pfnClrStatus             = _com_dfu_0g8_cb_ClrStatus2;
    tInitPrm.pfnAbort                 = _com_dfu_0g8_cb_Abort2;
    tInitPrm.pucReciveBuff            = l_pucDtRcvBufDfu0G8;

    tInitPrm.pfnDfuOut_0data          = _com_dfu_0g8_cb_Out_0data;     /*     */

    /* Communication Function Driverの初期化 */
	iStat = GRUSB_COMD_DFU_Init2( &tInitPrm );

    if( GRUSB_COMD_DFU_SUCCESS != iStat )
    {
        /* error */
        return COM_DFU_ERROR;
    }

#ifdef INTR_TEST_CMD_CHECK
    /* Notification情報の初期化 */
    com_app_NotificationInit();
#endif

    /*--- Line Codingの初期値を設定 ---*/
    l_aucLineCodingDfu0G8[0] = (UINT8)( APP_BAUDRATE & 0x000000FF );           /* ボーレート：LSB */
    l_aucLineCodingDfu0G8[1] = (UINT8)( ( APP_BAUDRATE & 0x0000FF00 ) >>  8 ); /*                 */
    l_aucLineCodingDfu0G8[2] = (UINT8)( ( APP_BAUDRATE & 0x00FF0000 ) >> 16 ); /*                 */
    l_aucLineCodingDfu0G8[3] = (UINT8)( ( APP_BAUDRATE & 0xFF000000 ) >> 24 ); /* ボーレート：MSB */
    l_aucLineCodingDfu0G8[4] = APP_STOPBITS;                                   /* ストップビット  */
    l_aucLineCodingDfu0G8[5] = APP_PARITY;                                     /* パリティ        */
    l_aucLineCodingDfu0G8[6] = APP_DATABITS;                                   /* データビット    */

    /* ユーザ指定のデバイスの製造番号(シリアルナンバー)を設定する場合は  */
    /* ここでGRUSB_COMD_SetSerialNumber2()をコールして下さい。           */
    /* (注)製造番号は、UNICODE(2バイトコード)形式で渡して下さい。        */
#if defined(ID0G8_BOOTIF)
	    /* Boot IFは0固定 */
	    UINT8 tempSrial[SERIAL_NUMBER_SIZE];
	    strcpy(tempSrial, "000000000000");
	    for (iCnt = 0; iCnt < SERIAL_NUMBER_SIZE; iCnt++)
	    {
	        l_usSerialNomber[iCnt] = (0x00FF & tempSrial[iCnt]);
	    }
#else

	#if 0  /* TODO: DEBUG用 削除する */
	    UINT8 tempSrial[SERIAL_NUMBER_SIZE];
	    strcpy(tempSrial, "180100000103");
	    for (iCnt = 0; iCnt < SERIAL_NUMBER_SIZE; iCnt++)
	    {
	        l_usSerialNomber[iCnt] = (0x00FF & tempSrial[iCnt]);
	    }
	#else /* TODO: DEBUG用 削除する */
	    for (iCnt = 0; iCnt < SERIAL_NUMBER_SIZE; iCnt++)
	    {
	        l_usSerialNomber[iCnt] = (0x00FF & ex_adjustment_data.maintenance_info.serial_no[iCnt]);
	    }
	#endif /* TODO: DEBUG用 削除する */

#endif

    GRUSB_COMD_DFU_SetSerialNumber2(l_usSerialNomber, 12);
    
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
    if (iNum <= GRCOMD_DFU_IF_CHARS)
    {
        GRUSB_COMD_DFU_SetInterface2(l_usInterface, iNum);
    }


    /* Pullup ON */
    GRUSB_DEV_ApReqPullupRegister2( GRUSB_TRUE );

    /* Enable of D+ pull up is done in com_app_Init2. */
    /* Start interrupt */
    if ( GRP_TARGET_OK != grp_target_StartIntr2( GRP_CYCLONEV_MODE_DEVICE ) )
    {
        /* Start Interrupt Error */
        return COM_DFU_ERROR;
    }

    return COM_DFU_OK;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_ConnStat2                                                       */
/*                                                                                              */
/* DESCRIPTION: 接続・切断通知                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iConnStat                       接続ステータス                                   */
/*                                                  GRUSB_COMD_DISC         切断                */
/*                                                  GRUSB_COMD_CON          接続                */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_dfu_0g8_cb_ConnStat2( INT iConnStat )
{
    if( GRUSB_COMD_DFU_CON == iConnStat )
    {
        /* 接続 */
        //2022-09-21 iset_flg(ID_USB2_CB_FLAG, EVT_USB_CON);
#if 0
        /* Bulk OUTエンドポイントのMaxPacketSizeを取得        */
        /* ※取得したサイズは受信要求サイズとして使用します。 */
        l_usRcvSizeDfu0G8 = GRUSB_DEV_ApGetMaxPacketSize2( GRCOMD_BULKOUT_EP_NUMBER );

        /* データ受信要求開始 */
        _com_dfu_0g8_ReqRecvData2( (UINT32)l_usRcvSizeDfu0G8, l_pucDtRcvBufDfu0G8 );
#endif /* yamazaki DEBUG */

        //Set configu
        //SignalUsbBusResetCmd();//    
        SignalSET_CONFIGURATIONCmd();//  
    }
    else if( 2 == iConnStat )   //NEW_BUS_RESET
    {
        SignalUsbBusResetCmd(); 
    }
    else
    {
        /* 切断 */
        //2022-09-21 iset_flg(ID_USB2_CB_FLAG, EVT_USB_CON);

        if( GRUSB_TRUE == l_bPortStatDfu0G8 )
        {
            l_bPortStatDfu0G8 = GRUSB_FALSE;
        }

#ifdef INTR_TEST_CMD_CHECK
        /* Notification情報の初期化 */
        com_app_NotificationInit2();
#endif
        ex_usb_dfu_disconect_0g8 = 1;//2022-09-21

    }

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_SendEncapsulatedCommad2                                         */
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
LOCAL VOID _com_dfu_0g8_cb_SendEncapsulatedCommad2( UINT32 ulSize, UINT8* pucData )
{
    /*----------------------------------------------------------------------*/
    /* SEND_ENCAPSULATED_COMMANDリクエストを受信すると本関数がコールされ    */
    /* ます。Abstract Control Modelにおいて必須機能のため、コールバックの   */
    /* 設定を行っていますが、本サンプルではコールされることはありません。   */
    /*----------------------------------------------------------------------*/

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_GetEncapsulatedResponse2                                        */
/*                                                                                              */
/* DESCRIPTION: GET_ENCAPSULATED_RESPONSEリクエスト受信処理                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_dfu_0g8_cb_GetEncapsulatedResponse2( UINT16 usLength )
{
    /*----------------------------------------------------------------------*/
    /* GET_ENCAPSULATED_RESPONSEリクエストを受信すると本関数がコールされ    */
    /* ます。Abstract Control Modelにおいて必須機能のため、コールバックの   */
    /* 設定を行っていますが、本サンプルではコールされることはありません。   */
    /*----------------------------------------------------------------------*/

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_SetLineCoding2                                                  */
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
LOCAL VOID _com_dfu_0g8_cb_SetLineCoding2( UINT32 ulSize, UINT8* pucData )
{
    /* Line Coding Structureバッファにコピー */
    memcpy( l_aucLineCodingDfu0G8, pucData, ulSize );

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_GetLineCoding2                                                  */
/*                                                                                              */
/* DESCRIPTION: GET_LINE_CODINGリクエスト受信処理                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_dfu_0g8_cb_GetLineCoding2( UINT16 usLength )
{
    /* Line Coding Structure送信 */
    GRUSB_COMD_DFU_Set_GetLineCoding2( (UINT32)usLength, l_aucLineCodingDfu0G8 );

    return;
}


/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_SetControlLineState2                                            */
/*                                                                                              */
/* DESCRIPTION: SET_CONTROL_LINE_STATEリクエスト受信処理                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usValue                         Control Signal Bitmap                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_dfu_0g8_cb_SetControlLineState2( UINT16 usValue )
{
UINT16                          usRTS;
UINT16                          usDTR;

    /* Control Signal Bitmapをセット */
    l_usControlLineStateDfu0G8 = usValue;

    /* RTS */
    usRTS = (UINT16)( ( l_usControlLineStateDfu0G8 & 0x0002 ) >> 1 );
    /* DTR */
    usDTR = (UINT16)( l_usControlLineStateDfu0G8 & 0x0001 );

    if( ( 1 == usRTS ) && ( 1 == usDTR ) )
    {
        /* ポート オープン */

        if( GRUSB_FALSE == l_bPortStatDfu0G8 )
        {
            l_bPortStatDfu0G8 = GRUSB_TRUE;
        }
    }
    else
    {
        /* ポート クローズ */

        if( GRUSB_TRUE == l_bPortStatDfu0G8 )
        {
            l_bPortStatDfu0G8 = GRUSB_FALSE;
        }
    }

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_NetworkConnection2                                              */
/*                                                                                              */
/* DESCRIPTION: NETWORK_CONNECTION通知完了処理                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_dfu_0g8_cb_NetworkConnection2( VOID )
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
/* FUNCTION   : _com_dfu_0g8_cb_ResponseAvailable2                                              */
/*                                                                                              */
/* DESCRIPTION: RESPONSE_AVAILABLE通知完了処理                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_dfu_0g8_cb_ResponseAvailable2( VOID )
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
/* FUNCTION   : _com_dfu_0g8_cb_SerialState2                                                    */
/*                                                                                              */
/* DESCRIPTION: SERIAL_STATE通知完了処理                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_dfu_0g8_cb_SerialState2( VOID )
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
/* FUNCTION   : _com_dfu_0g8_cb_SendData2                                                       */
/*                                                                                              */
/* DESCRIPTION: データ送信完了通知処理                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          送信データサイズ                                */
/*              pucData                         送信データバッファポインタ                       */
/*              pInfo                           情報ポインタ                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_dfu_0g8_cb_SendData2( UINT32 ulSize, UINT8* pucData, VOID* pInfo, INT iStat )
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
        memset( l_pucDtSndBufDfu0G8, 0x00, COM_DFU_BUF_SIZE );

        /* データ受信要求開始 */
        _com_dfu_0g8_ReqRecvData2( (UINT32)l_usRcvSizeDfu0G8, l_pucDtRcvBufDfu0G8 );
    }
#endif

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_RecvData2                                                       */
/*                                                                                              */
/* DESCRIPTION: データ受信完了通知処理                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          受信データサイズ                                 */
/*              pucData                         受信データバッファポインタ                       */
/*              pInfo                           情報ポインタ                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_dfu_0g8_cb_RecvData2( UINT32 ulSize, UINT8* pucData, VOID* pInfo, INT iStat )
{
INT                             iRet;
UINT32                          ulSendSize;

    if( GRUSB_COMD_COMPLETE == iStat )
    {
#if 1
        if (ulSize != 0)
        {
            // TODO: データ確認、Lineタスクへの通知を追加する
            if ((*pucData == 0x03) && (*(pucData + 1) == 0x00))
            {
                iset_flg(ID_USB2_CB_FLAG, EVT_USB_RCV);
                l_usRevDataSizeDfu0G8 = ulSize;
            }
            else if (*pucData == GRUSB_DFU_DFU_DETACH)
            {
                /* TODO: DFU切り替え */
                iset_flg(ID_USB2_CB_FLAG, EVT_USB_RCV);
            }
        }
#else
        if( (UINT32)l_usRcvSizeDfu0G8 < ulSize )
        {
            /* バッファオーバーフロー */
            return;
        }
        else if( 0 == ulSize )
        {
            /* データ受信要求開始 */
            _com_dfu_0g8_ReqRecvData2( (UINT32)l_usRcvSizeDfu0G8, l_pucDtRcvBufDfu0G8 );

            return;
        }

        /* 受信データを送信データバッファにコピー */
        memcpy( l_pucDtSndBufDfu0G8, pucData, ulSize );

#ifdef INTR_TEST_CMD_CHECK
        /* Notification開始コマンドチェック */
        iRet = com_app_CheckNotificationCmd2( l_pucDtSndBufDfu0G8 );

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
        iRet = _com_dfu_0g8_ReqSendData2( ulSendSize, l_pucDtSndBufDfu0G8 );

        if( COM_DFU_OK != iRet )
        {
            /* 送信要求エラー */
            return;
        }
#endif
    }

    return;
}
#else // not GRCOMD_COMP_STATUS_USE
/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_SendData2                                                       */
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
LOCAL VOID _com_dfu_0g8_cb_SendData2( UINT32 ulSize, UINT8* pucData, VOID* pInfo )
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
    memset( l_pucDtSndBufDfu0G8, 0x00, COM_DFU_BUF_SIZE );

    /* データ受信要求開始 */
    _com_dfu_0g8_ReqRecvData2( (UINT32)l_usRcvSizeDfu0G8, l_pucDtRcvBufDfu0G8 );
#endif

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_RecvData2                                                       */
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
LOCAL VOID _com_dfu_0g8_cb_RecvData2( UINT32 ulSize, UINT8* pucData, VOID* pInfo )
{
INT                             iStat;
UINT32                          ulSendSize;

    if (ulSize != 0)
    {
        #if 0 //  この割り込み発生は、受信バッファ l_usRevDataSizeDfu0G8にデータ格納済みである
              //わざわざ、usb2_cb_task経由でclineタスクに通知する必要性はない、とりあえずex_dowload_data_recived_0g8でclineタスクで処理する

        // TODO: データ確認、Lineタスクへの通知を追加する
        iset_flg(ID_USB2_CB_FLAG, EVT_USB_RCV);
        l_usRevDataSizeDfu0G8 = ulSize;

        #else
        /* データ受信直後のここで、StatusをIdleからBusyに変えないと */
        /* すぐにStatausリクエスト受信となり、Idleステータスをレスポンスしてしまう */
        /* また、ダウンロードデータ受信フラグをラインタスクで使用する前に  */
        /* Statausリクエストを受信して、ダウンロード受信データフラグを上書きしてしまう  */
        /* 使用するフラグを分けた方がいい                                          */
        /* 2022-07-29a */
        ex_dowload_data_recived_0g8 = CMD_NUM_DFU_DNLOAD_END;
        DuwDnBlockSize = ulSize;

		if(ex_down_start_0g8 == 0)
        {
            ex_down_start_0g8 = 1;
        }
       
        //   DucUsbCmdNoをダウンロードデータ受信通知と、Status受信で共有するとUSB通信が早い場合、上書きされるので分ける DucUsbCmdNo = CMD_NUM_DFU_DNLOAD_END;
        #endif

    }
    return;
}
#endif // end not GRCOMD_COMP_STATUS_USE

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_Upload2                                                         */
/*                                                                                              */
/* DESCRIPTION: GRUSB_DFU_UPLOADリクエスト受信処理                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
extern void data_send_upload(u16 wValue, u16 req_length);
LOCAL VOID _com_dfu_0g8_cb_Upload2( UINT16 usValue, UINT16 usLength ) //usValue== setup stageのwValue, usLength== set up stageのwLength
{

    data_send_upload(usValue, usLength);
    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_GetStatus2                                                      */
/*                                                                                              */
/* DESCRIPTION: GRUSB_DFU_GETSTATUSリクエスト受信処理                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
extern void data_send_status(void); // 
LOCAL VOID _com_dfu_0g8_cb_GetStatus2( UINT16 usLength )
{

    data_send_status();	// 
    GRUSB_COMD_DFU_Set_GetEncapsulatedResponse2(l_usSndDataSizeDfu0G8, l_pucDtSndBufDfu0G8);

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_GetState2                                                       */
/*                                                                                              */
/* DESCRIPTION: GRUSB_DFU_GETSTATEリクエスト受信処理                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
extern void data_send_state(void); // 
LOCAL VOID _com_dfu_0g8_cb_GetState2( UINT16 usLength )
{

    data_send_state();	// 
    GRUSB_COMD_DFU_Set_GetEncapsulatedResponse2(l_usSndDataSizeDfu0G8, l_pucDtSndBufDfu0G8);

    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_DfuDetach2                                                      */
/*                                                                                              */
/* DESCRIPTION: GRUSB_DFU_DETACHリクエスト受信処理                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
//LOCAL VOID _com_dfu_0g8_cb_DfuDetach2( VOID )
//割り込みから直接しないと、処理が間に合わない時がある
LOCAL VOID _com_dfu_0g8_cb_DfuDetach2(  UINT16 usLength  ) //USB割り込みから直接 
{
    DusDetachTimeout = usLength;
    DucUsbCmdNo = CMD_NUM_DFU_DETACH;
    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_ClrStatus2                                                      */
/*                                                                                              */
/* DESCRIPTION: GRUSB_DFU_CLRSTATUSリクエスト受信処理                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_dfu_0g8_cb_ClrStatus2( VOID )
{
	DucUsbCmdNo = CMD_NUM_DFU_CLRSTATUS;// 
    return;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_cb_Abort2                                                          */
/*                                                                                              */
/* DESCRIPTION: GRUSB_DFU_ABORTリクエスト受信処理                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usLength                        要求データサイズ                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _com_dfu_0g8_cb_Abort2( VOID )
{
	DucUsbCmdNo = CMD_NUM_DFU_ABORT;// 
    return;
}

/************************************************************************************************/
/* FUNCTION   : com_dfu_0g8_listen                                                              */
/*                                                                                              */
/* DESCRIPTION: データ受信確認                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : ulSize                          受信データ数                                     */
/*                                                                                              */
/************************************************************************************************/
#if defined(ID0G8_BOOTIF)//2022-09-21
UINT32 com_dfu_0g8_listen( VOID )
{
    UINT32 ulSize;

    if (l_usRevDataSizeDfu0G8 != 0)
    {
        ulSize = l_usRevDataSizeDfu0G8;
        l_usRevDataSizeDfu0G8 = 0;
    }

    return ulSize;
}
#endif

/************************************************************************************************/
/* FUNCTION   : com_dfu_0g8_send                                                                */
/*                                                                                              */
/* DESCRIPTION: データ送信要求                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          送信データサイズ                                 */
/*              pucBuf                          送信データバッファポインタ                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : COM_DFU_OK                      Success                                         */
/*              COM_DFU_ERROR                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
#if 0 //2022-09-21
UINT32 com_dfu_0g8_send( UINT32 ulSize, UINT8* pucBuf )
{
INT                             iStat;
INT                             iRet = COM_DFU_OK;

    iStat = GRUSB_COMD_DFU_Set_GetEncapsulatedResponse2(ulSize, pucBuf);
    //iStat = GRUSB_COMD_DFU_Notification2(ulSize, pucBuf);
    if( GRUSB_COMD_DFU_SUCCESS != iStat )
    {
        /* 送信要求エラー */
        iRet = COM_DFU_ERROR;
    }
    return iRet;
}
#endif

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_ReqSendData2                                                       */
/*                                                                                              */
/* DESCRIPTION: データ送信要求                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          送信データサイズ                                 */
/*              pucBuf                          送信データバッファポインタ                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : COM_DFU_OK                      Success                                         */
/*              COM_DFU_ERROR                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL INT _com_dfu_0g8_ReqSendData2( UINT32 ulSize, UINT8* pucBuf )
{
INT                             iStat;
INT                             iRet = COM_DFU_OK;

    /* データ送信要求 */
    iStat = GRUSB_COMD_DFU_SendData2( ulSize, pucBuf, GRUSB_NULL );

    if( GRUSB_COMD_DFU_SUCCESS != iStat )
    {
        /* 送信要求エラー */
        iRet = COM_DFU_ERROR;
    }

    return iRet;
}

/************************************************************************************************/
/* FUNCTION   : _com_dfu_0g8_ReqRecvData2                                                       */
/*                                                                                              */
/* DESCRIPTION: データ受信要求                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulSize                          受信データサイズ                                 */
/*              pucBuf                          受信データバッファポインタ                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : COM_DFU_OK                      Success                                         */
/*              COM_DFU_ERROR                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL INT _com_dfu_0g8_ReqRecvData2( UINT32 ulSize, UINT8* pucBuf )
{
INT                             iStat;
INT                             iRet = COM_DFU_OK;

    /* 受信データバッファクリア */
    memset( pucBuf, 0x00, COM_DFU_BUF_SIZE );

    /* データ受信要求 */
    iStat = GRUSB_COMD_DFU_ReciveData2( ulSize, pucBuf, GRUSB_NULL );

    if( GRUSB_COMD_DFU_SUCCESS != iStat )
    {
        /* 受信要求エラー */
        iRet = COM_DFU_ERROR;
    }

    return iRet;
}


LOCAL VOID _com_dfu_0g8_cb_Out_0data( VOID )   /*   */
{
    //データありの場合 _com_dfu_0g8_cb_RecvData2 が呼ばれている

    // TODO: データ確認、Lineタスクへの通知を追加する
    #if 0 //この割り込み発生は、受信バッファ l_usRevDataSizeDfu0G8にデータ格納済みである
              //わざわざ、usb2_cb_task経由でclineタスクに通知する必要性はない、とりあえずex_dowload_data_recived_0g8でclineタスクで処理する

    iset_flg(ID_USB2_CB_FLAG, EVT_USB_RCV);
    l_usRevDataSizeDfu0G8 = 0;
    #else
    /*   */
    ex_dowload_data_recived_0g8 = CMD_NUM_DFU_DNLOAD_END;
    DuwDnBlockSize = 0;
    #endif

    return;
}

