/******************************************************************************/
/*! @addtogroup Main
    @file       hal_usb.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * hal_usb.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */
// Grape System社USBドライバラッパーファイル。

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "common.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "typedefine.h"
#include "memorymap.h"
#include "grp_cyclonev_macro.h"
#include "grp_cyclonev_reg.h"
#include "grp_cyclonev_bit_val.h"

#define EXT
#include "com_ram.c"
#include "usb_ram.c"
#include "jsl_ram.c"

#include "usb_cdc_buffer.h"


/************************** External functions *******************************/

/************************** Constant Definitions *****************************/
/**** INTERNAL DATA DEFINES *********************************************************************/
#define BUF_SIZE                (64*1024)                                 /* バッファサイズ */

/* Line Coding */
#define LINE_CODING_BUF_SIZE    (8)                              /* Line Coding バッファサイズ  */
#define LINE_CODING_SIZE        (7)                              /* Line Coding Structureサイズ */
#define APP_BAUDRATE            (9600)                           /* ボーレート初期値            */
#define APP_STOPBITS            (0)                              /* ストップビット初期値        */
#define APP_PARITY              (0)                              /* パリティ初期値              */
#define APP_DATABITS            (8)                              /* データビット初期値          */


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
//DLOCAL UINT32                   l_aulDtSndBuf[BUF_SIZE/sizeof(UINT32)];
//DLOCAL UINT8*                   l_pucDtSndBuf = (UINT8*)l_aulDtSndBuf;
/* 受信データバッファ */
DLOCAL UINT32                   l_aulDtRcvBuf[BUF_SIZE/sizeof(UINT32)];
DLOCAL UINT8*                   l_pucDtRcvBuf = (UINT8*)l_aulDtRcvBuf;

//DLOCAL UINT32					l_iSendCnt;
/*==============================================================================*/
/* ローカル構造体																*/
/*==============================================================================*/
static RingBuffer s_usb_cdc_rx_buffer;
static INT s_usb_connect_status;
/************************** Function Prototypes *****************************/

//#if defined(SWITCH_USB0_USB1)
//#define _HAL_USB_INT_IRQ OSW_INT_USB1_IRQ
//#else
#define _HAL_USB_INT_IRQ OSW_INT_USB0_IRQ
//#endif
/*-- UBC⇔PC ﾒｯｾｰｼﾞ定義 --*/
/* ｺﾏﾝﾄﾞ */
#define IDX_CMD				0		/* byte0: COMMAND CODE						*/
#define IDX_LEN				1		/* byte1: (DLDataｺﾏﾝﾄﾞ以外)ﾒｯｾｰｼﾞ全長		*/
#define IDX_LEN_H			1		/* byte1: (DLDataｺﾏﾝﾄﾞ)ﾒｯｾｰｼﾞ全長 上位byte	*/
#define IDX_LEN_L			2		/* byte2: (DLDataｺﾏﾝﾄﾞ)ﾒｯｾｰｼﾞ全長 下位byte	*/
#define IDX_DATA			2		/* byte2: (DLDataｺﾏﾝﾄﾞ以外)DATA開始位置 	*/
#define IDX_DLDATA			3		/* byte3: (DLDataｺﾏﾝﾄﾞ)DATA開始位置 		*/
/* ﾚｽﾎﾟﾝｽ */
#define IDX_RES				0		/* byte0: RESPONSE CODE						*/

/*-- ﾀﾞｳﾝﾛｰﾄﾞﾌｧｲﾙﾌｫｰﾏｯﾄ --*/
#define ADROFS_START		0x06	/*  6~ 9byte: ﾀﾞｳﾝﾛｰﾄﾞ先先頭絶対ｱﾄﾞﾚｽ		*/
#define ADROFS_END			0x0A	/* 10~13byte: ﾀﾞｳﾝﾛｰﾄﾞ先終了絶対ｱﾄﾞﾚｽ		*/
#define WADROFS_VER			0x0E	/* 14~15byte: ﾊﾞｰｼﾞｮﾝ情報へのｱﾄﾞﾚｽへのｱﾄﾞﾚｽ */

extern const GRUSB_PID _UsbPid;/* 関数プロトタイプ */
/* デバッグ出力定義 */
/* 必要に応じ printf に相当する文字列出力処理をご定義下さい                     */  /* 1.30 */
//#define GRTEST_OUTPUT(str)  /* printf(str)  */                                      /* 1.30 */
#define GRTEST_OUTPUT(str)  osw_printf(str)                                         /* 1.30 */

/* 関数プロトタイプ */
VOID _UsbInit( VOID );                                            /* 初期処理                                        */
VOID _UsbConnStat( INT iConnStat );                               /* 接続・切断通知処理                              */
VOID _UsbSendEncapsulatedCommad( UINT32 ulSize, UINT8* pucData ); /* SEND_ENCAPSULATED_COMMANDリクエスト受信通知処理 */
VOID _UsbGetEncapsulatedResponse( UINT16 usLength );              /* GET_ENCAPSULATED_RESPONSEリクエスト受信通知処理 */
VOID _UsbSetLineCoding( UINT32 ulSize, UINT8* pucdata );          /* SET_LINE_CODINGリクエスト受信通知処理           */
VOID _UsbGetLineCoding( UINT16 usLength );                        /* GET_LINE_CODINGリクエスト受信通知処理           */
VOID _UsbSetControlLineState( UINT16 usValue );                   /* SET_CONTROL_LINE_STATEリクエスト受信通知処理    */
VOID _UsbResponseAvailable( VOID );                               /* RESPONSE_AVAILABLE通知完了処理                  */
VOID _UsbSendData( UINT32 ulSize, UINT8* pucData, VOID* pInfo );  /* データ送信完了通知処理                          */
VOID _UsbRecvData( UINT32 ulSize, UINT8* pucData, VOID* pInfo );  /* データ受信完了通知処理                          */
VOID _UsbReqRecvData( VOID );                                     /* データ受信要求                                  */

// DLINE
void reset_usb0(void) {
//#if defined(SWITCH_USB0_USB1)
//	Gpio_out( __HAL_USB2_RESET, RESET_ASSERT );
//	OSW_TSK_sleep(2);
//	Gpio_out( __HAL_USB2_RESET, RESET_DEASSERT );
//#else
	Gpio_out( __HAL_USB0_RESET, RESET_ASSERT );
	OSW_TSK_sleep(2);
	Gpio_out( __HAL_USB0_RESET, RESET_DEASSERT );
//#endif
	OSW_TSK_sleep(20);
}
/****************************************************************************/
/* FUNCTION   : _UsbInit                                                  */
/*                                                                          */
/* DESCRIPTION: サンプルプログラムの初期処理を行います                      */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _UsbInit( VOID )
{
    INT iStat;
	GRUSB_COMD_INITINFO      tInitPrm;          /* CDCドライバ初期設定  */

	/* H/W init */
	if ( GRP_TARGET_OK != grp_target_HwInit( GRP_CYCLONEV_MODE_DEVICE ) ) {
		/* H/W Init Error */
		return;
	}
#if (_DEBUG_USB_ULPI_HS_COMPESATION_REG_ACCESS==1)
	ulpi_setting();
	memset(&ulpi_table, 0, sizeof(ulpi_table));
	ulpi_load(0);
#endif

	/* CDC初期設定 */
	tInitPrm.pfnConnStat              = _UsbConnStat;
	tInitPrm.pfnSendEncapsulatedCmd   = _UsbSendEncapsulatedCommad;
	tInitPrm.pfnGetEncapsulatedRes    = _UsbGetEncapsulatedResponse;
	tInitPrm.pfnSetCommFeature        = GRUSB_NULL;
	tInitPrm.pfnGetCommFeature        = GRUSB_NULL;
	tInitPrm.pfnClearCommFeature      = GRUSB_NULL;
	tInitPrm.pfnSetLineCoding         = _UsbSetLineCoding;
	tInitPrm.pfnGetLineCoding         = _UsbGetLineCoding;
	tInitPrm.pfnSetControlLineState   = _UsbSetControlLineState;
	tInitPrm.pfnSendBreak             = GRUSB_NULL;
	tInitPrm.pfnNetworkConnection     = GRUSB_NULL;
	tInitPrm.pfnResponseAvailable     = _UsbResponseAvailable;
	tInitPrm.pfnSerialState           = GRUSB_NULL;
	tInitPrm.pfnSendData              = _UsbSendData;
	tInitPrm.pfnReciveData            = _UsbRecvData;
    tInitPrm.pstUsbPid                = (PGRUSB_PID)&_UsbPid;

	if (GRUSB_COMD_SUCCESS != GRUSB_COMD_Init( &tInitPrm )) {
		return;
	}
	/* Initialize the RX circular buffer */
    iStat = _usb_cdc_buffer_init(&s_usb_cdc_rx_buffer, USB_CDC_MEM_SIZE);
	if (iStat != GRUSB_COMD_SUCCESS) {
        /* error */
        //GRTEST_OUTPUT("初期化エラー\n\r");
        GRTEST_OUTPUT("_UsbInit error\r\n");

        return;
	}


	/* Pullup ON */
	GRUSB_DEV_ApReqPullupRegister( GRUSB_TRUE );

	/* Start interrupt */
	if ( GRP_TARGET_OK != grp_target_StartIntr( GRP_CYCLONEV_MODE_DEVICE ) ) {
		/* Start Interrupt Error */
		return;
	}
}

/****************************************************************************/
/* FUNCTION   : _UsbConnStat                                              */
/*                                                                          */
/* DESCRIPTION: デバイスの接続・切断が通知された時の処理関数です            */
/*--------------------------------------------------------------------------*/
/* INPUT      : iConnStat           GRUSB_COMD_DISC         切断            */
/*                                  GRUSB_COMD_CON          接続            */
/*                                  GRUSB_COMD_RECON        再接続          */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _UsbConnStat( INT iConnStat )
{
    /*----------------------------------------------------------------------*/
    /* 接続または切断を検出すると本関数がコールされます。                   */
    /* 接続・切断を検出するとその旨をシリアルにより通知し、ターミナルソフト */
    /* に出力します。                                                       */
    /* また本サンプルでは接続を検知したところで、受信要求を実行しています。 */
    /*----------------------------------------------------------------------*/

#if (_DEBUG_USB_ULPI_HS_COMPESATION_REG_ACCESS==1)
	//ulpi_load();
#endif

    if( iConnStat == GRUSB_COMD_CON )
    {
        /* 接続 */
        GRTEST_OUTPUT("connect\r\n");


        l_usRcvSize = BUF_SIZE;

        _UsbReqRecvData();                        /* データ受信要求を開始 */

    }
    else
    {
        /* 切断 */
        GRTEST_OUTPUT("disconnect\r\n");
    }
	s_usb_connect_status = iConnStat;
	
	iset_flg(ID_USB0_CB_FLAG, EVT_USB_CON);
}

/****************************************************************************/
/* FUNCTION   : _UsbSendEncapsulatedCommad                                */
/*                                                                          */
/* DESCRIPTION: SEND_ENCAPSULATED_COMMANDリクエスト受信時の処理関数です     */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              データサイズ                            */
/*              pucData             データバッファポインタ                  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _UsbSendEncapsulatedCommad( UINT32 ulSize, UINT8* pucData )
{
    /*----------------------------------------------------------------------*/
    /* SEND_ENCAPSULATED_COMMANDリクエストを受信すると本関数がコールされ    */
    /* ます。Abstract Control Modelにおいて必須機能のため、コールバックの   */
    /* 設定を行っていますが、本サンプルではコールされることはありません。   */
    /*----------------------------------------------------------------------*/

    return;
}

/****************************************************************************/
/* FUNCTION   : _UsbGetEncapsulatedResponse                               */
/*                                                                          */
/* DESCRIPTION: GET_ENCAPSULATED_RESPONSEリクエスト受信時の処理関数です     */
/*--------------------------------------------------------------------------*/
/* INPUT      : usLength            要求データサイズ                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _UsbGetEncapsulatedResponse( UINT16 usLength )
{
    /*----------------------------------------------------------------------*/
    /* GET_ENCAPSULATED_RESPONSEリクエストを受信すると本関数がコールされ    */
    /* ます。Abstract Control Modelにおいて必須機能のため、コールバックの   */
    /* 設定を行っていますが、本サンプルではコールされることはありません。   */
    /*----------------------------------------------------------------------*/

    return;
}

/****************************************************************************/
/* FUNCTION   : _UsbSetLineCoding                                         */
/*                                                                          */
/* DESCRIPTION: SET_LINE_CODINGリクエスト受信時の処理関数です               */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              Line Coding Structureサイズ             */
/*              pucData             Line Coding Structureバッファポインタ   */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _UsbSetLineCoding( UINT32 ulSize, UINT8* pucData )
{
    /*----------------------------------------------------------------------*/
    /* SET_LINE_CODINGリクエストを受信すると本関数がコールされます。        */
    /* pucDataにホスト側からの設定値が格納されています。                    */
    /* 今回のシステムではこの値を使用することはありませんので、以下のように */
    /* グローバル変数に情報を保存しておき、GET_LINE_CODINGリクエストを受信  */
    /* した際に、レスポンスデータとして利用します。                         */
    /* データの詳細については_UsbInit関数内で初期値を設定している所を     */
    /* 参照してください。                                                   */
    /*----------------------------------------------------------------------*/
    //GRTEST_OUTPUT("SET_LINE_CODINGリクエスト受信\r\n");
    GRTEST_OUTPUT("SET_LINE_CODING\r\n");

    memcpy( l_aucLineCoding, pucData, ulSize );
}

/****************************************************************************/
/* FUNCTION   : _UsbGetLineCoding                                         */
/*                                                                          */
/* DESCRIPTION: GET_LINE_CODINGリクエスト受信時の処理関数です               */
/*--------------------------------------------------------------------------*/
/* INPUT      : usLength            要求データサイズ                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _UsbGetLineCoding( UINT16 usLength )
{
    /*----------------------------------------------------------------------*/
    /* GET_LINE_CODINGリクエストを受信すると本関数がコールされます。        */
    /* このリクエストを受信した場合、GRUSB_COMD_Set_GetLineCoding関数を     */
    /* 使ってデバイス側の設定を通知する必要があります。                     */
    /* 基本的には以下のサンプルのように実装すれば問題ないと思います。       */
    /* またGRUSB_COMD_Set_GetLineCoding関数がエラー終了になるのはケーブルの */
    /* 切断以外はありえませんので、エラー処理が必要ならば追加してください。 */
    /* データの詳細については_UsbInit関数内で初期値を設定している所を     */
    /* 参照してください。                                                   */
    /*----------------------------------------------------------------------*/
    /* TODO:Implement */
    INT iStat;

    //GRTEST_OUTPUT("GET_LINE_CODINGリクエスト受信\r\n");
    GRTEST_OUTPUT("GET_LINE_CODINGリクエスト\r\n");

    iStat = GRUSB_COMD_Set_GetLineCoding( (UINT32)usLength, l_aucLineCoding );

    if( iStat != GRUSB_COMD_SUCCESS )
    {
        GRTEST_OUTPUT("Error\r\n");
    }
}

/****************************************************************************/
/* FUNCTION   : _UsbSetControlLineState                                   */
/*                                                                          */
/* DESCRIPTION: SET_CONTROL_LINE_STATEリクエスト受信時の処理関数です        */
/*--------------------------------------------------------------------------*/
/* INPUT      : usValue             Control Signal Bitmap                   */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _UsbSetControlLineState( UINT16 usValue )
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

/****************************************************************************/
/* FUNCTION   : _UsbResponseAvailable                                     */
/*                                                                          */
/* DESCRIPTION: RESPONSE_AVAILABLE通知完了時の処理関数です                  */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _UsbResponseAvailable( VOID )
{
    /*----------------------------------------------------------------------*/
    /* RESPONSE_AVAILABLEリクエストを受信すると本関数がコールされます。     */
    /* Abstract Control Modelにおいて必須機能のため、コールバックの設定を   */
    /* 行っていますが、本サンプルではコールされることはありません。         */
    /*----------------------------------------------------------------------*/

    return;
}

/****************************************************************************/
/* FUNCTION   : _UsbSendData                                              */
/*                                                                          */
/* DESCRIPTION: データ送信完了通知時の処理関数です                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              送信データサイズ                        */
/*              pucData             送信データバッファポインタ              */
/*              pInfo               情報ポインタ                            */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _UsbSendData( UINT32 ulSize, UINT8* pucData, VOID* pInfo )
{
    /*----------------------------------------------------------------------*/
    /* データ送信完了時に本関数がコールされます。                           */
    /* 本サンプルでは、送信完了はループバック送信完了を意味しますので、     */  /* 1.31 */
    /* 新たなデータの受信要求を実行します。                                 */  /* 1.31 */
    /*----------------------------------------------------------------------*/
    //GRTEST_OUTPUT("送信完了通知受信\r\n");
    GRTEST_OUTPUT("_UsbSendData\r\n");
	ex_usb_write_busy = 0;
	iset_flg(ID_USB0_CB_FLAG, EVT_USB_EMP);

    return;
}

/****************************************************************************/
/* FUNCTION   : _UsbRecvData                                              */
/*                                                                          */
/* DESCRIPTION: データ受信完了通知時の処理関数です                          */
/*--------------------------------------------------------------------------*/
/* INPUT      : ulSize              受信データサイズ                        */
/*              pucData             受信データバッファポインタ              */
/*              pInfo               情報ポインタ                            */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _UsbRecvData( UINT32 ulSize, UINT8* pucData, VOID* pInfo )
{
    /*----------------------------------------------------------------------*/
    /* データ受信完了時に本関数がコールされます。                           */
    /* 本サンプルではデータを折り返し送信する作りとしているため、この関数内 */
    /* では、受信したデータを送信用バッファへコピーし、データの送信要求を   */
    /* 実行します。                                                         */  /* 1.31 */
    /* 送信完了通知は_UsbSendData関数に通知されます。                     */
    /*----------------------------------------------------------------------*/


    //GRTEST_OUTPUT("受信完了通知受信\r\n");
    //GRTEST_OUTPUT("_UsbRecvData start\r\n");

    /* 受信データを送信データバッファへコピー */
	_usb_cdc_buffer_write(&s_usb_cdc_rx_buffer, pucData, ulSize);
	iset_flg(ID_USB0_CB_FLAG, EVT_USB_RCV);
	/* データ受信要求 */
	_UsbReqRecvData();
}

/****************************************************************************/
/* FUNCTION   : _UsbReqRecvData                                           */
/*                                                                          */
/* DESCRIPTION: CDCドライバに対し、データの受信要求を実行します             */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _UsbReqRecvData( VOID )
{
    INT iStat;

    /* データ受信要求 */
    iStat = GRUSB_COMD_ReciveData( (UINT32)l_usRcvSize, l_pucDtRcvBuf, GRUSB_NULL );

    if( iStat != GRUSB_COMD_SUCCESS )
    {
        /* 受信要求エラー */
        //GRTEST_OUTPUT("受信要求エラー\r\n");
        GRTEST_OUTPUT("_BootUsbReqRecvData() error\r\n");
    }
}

/****************************************************************************/
/* FUNCTION   : _UsbGetAvailableChar                                           */
/*                                                                          */
/* DESCRIPTION: CDCリングバッファのデータ数を返します。            */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
s32 UsbGetAvailableChar( VOID )
{
	s32 size;

	size = _usb_cdc_buffer_available_bytes(&s_usb_cdc_rx_buffer);

	return size;
}
/****************************************************************************/
/* FUNCTION   : _UsbReqRecvData                                           */
/*                                                                          */
/* DESCRIPTION: CDCリングバッファのデータを取得します             */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
s32 UsbGetRecvData( u8* dst, s32 size_to_read )
{
	s32 received_count;

	received_count = _usb_cdc_buffer_read(
			&s_usb_cdc_rx_buffer,
			dst,
			size_to_read);

	return received_count;
}


void UsbReqSendData(void)
{
    INT iStat;
	/* disable usb0 interrupt. Avoid active usb0 interrupt
	send data 2 time  */
    OSW_ISR_disable( OSW_INT_USB0_IRQ ); //2025-05-14a
	iStat = GRUSB_COMD_SendData(ex_usb_write_size,
			&ex_usb_write_buffer[0],
			0);
	/* re-enable */
	OSW_ISR_enable( OSW_INT_USB0_IRQ ); //2025-05-14a
	ex_usb_write_size = 0;
	ex_usb_write_busy = 1;
    if( iStat != GRUSB_COMD_SUCCESS )
    {
        /* 送信要求エラー */
        //GRTEST_OUTPUT("送信要求エラー\r\n");
        GRTEST_OUTPUT("_boot_musb_send_data() error\r\n");
        return;
    }
}
void UsbReqSendSamplingData(void)
{
    INT iStat;

	iStat = GRUSB_COMD_SendData(ex_usb_write_size,
			ex_usb_pbuf,
			0);
	ex_usb_write_size = 0;
	ex_usb_write_busy = 1;
	ex_usb_buf_change = 0;
    if( iStat != GRUSB_COMD_SUCCESS )
    {
        /* 送信要求エラー */
        //GRTEST_OUTPUT("送信要求エラー\r\n");
        GRTEST_OUTPUT("_boot_musb_send_data() error\r\n");
        return;
    }
}
/****************************************************************************/
/* FUNCTION   : is_usb_connected                                           	*/
/*                                                                          */
/* DESCRIPTION: 接続状態/切断状態を返します											*/
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
s32 is_usb0_connected(void)
{
	BOOLEAN bStat;

	if( s_usb_connect_status == GRUSB_COMD_CON )
	{
		bStat = GRUSB_TRUE;
	}
	else
	{
		bStat = GRUSB_FALSE;
	}
	return bStat;
}

/****************************************************************************/
/* FUNCTION   : is_usb_pull_out                                           	*/
/*                                                                          */
/* DESCRIPTION: ホストケーブルが接続状態かをかえします。実際にはIDピンの状態を返します						*/
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
s32 is_host_usb_pull_out(void)
{
	BOOLEAN                 bStat;
	UINT32                  ulGOtgCtl;

	/* Read control and status register */
	ulGOtgCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GOTGCTL );

	if( CYCLONEVG_B01_CONIDSTS == ( CYCLONEVG_B01_CONIDSTS & ulGOtgCtl ) )
	{
		/* ID PIN HIGH:デバイスモード */
		/* B-Device mode (Device) */
		bStat = GRUSB_TRUE;
	}
	else
	{
		/* ID PIN LOW:ホストモード */
		/* A-Device mode (Host) */
		bStat = GRUSB_FALSE;
	}
	return bStat;
}

/*******************************
        initialize test ware USB0
 *******************************/
//void USBConnect(void)
//{
//	// TODO:implement
//	reset_usb0();
//
//	/* Initialize the USB controller */
//	_UsbInit();
//
//	// initialize variables
//	ex_usb_read_size = 0;
//	ex_usb_write_size = 0;
//}
void FrontUSBConnect(void)
{
	// FUSB_ONOFF
	//LOW：CDCクラスのデバイスとして動作予定
	Gpio_out( __HAL_USB0_RESET, RESET_ASSERT );
	Gpio_out( __HAL_FUSB_SELECT, USB_SELECT_FRONT );
	OSW_TSK_sleep(2);
	Gpio_out( __HAL_USB0_RESET, RESET_DEASSERT );
	OSW_TSK_sleep(2);
}
void RearUSBConnect(void)
{
	// FUSB_ONOFF
	//HIGH：FUZION（CDCクラス）として動作予定
	Gpio_out( __HAL_USB0_RESET, RESET_ASSERT );
	Gpio_out( __HAL_FUSB_SELECT, USB_SELECT_REAR );
	OSW_TSK_sleep(2);
	Gpio_out( __HAL_USB0_RESET, RESET_DEASSERT );
	OSW_TSK_sleep(2);
}
/*******************************
        terminate test ware USB0
 *******************************/
void USBDisconnect(void)
{
	// TODO:implement
	OSW_ISR_disable( _HAL_USB_INT_IRQ );
}
/*******************************
        terminate test ware USB0
 *******************************/
void USBDebugEchoback(void)
{
	s32 numAvailByte;
	u32 read_byte = 0;
	u32 r_size = 0;
	numAvailByte = UsbGetAvailableChar();

	if(numAvailByte > 0)
	{
		if (numAvailByte > USB_BUFFER_SIZE)
		{
			read_byte = USB_BUFFER_SIZE;
		}
		else
		{
			read_byte = numAvailByte;
		}
		OSW_ISR_disable( _HAL_USB_INT_IRQ );
		/* Block receiving the buffer. */
		r_size = UsbGetRecvData(
				&ex_usb_read_buffer[0],
				read_byte);
		OSW_ISR_enable( _HAL_USB_INT_IRQ );
	}
	memcpy(ex_usb_write_buffer, ex_usb_read_buffer, r_size);
	ex_usb_write_size = r_size;
	UsbReqSendData();
}
/* EOF */
