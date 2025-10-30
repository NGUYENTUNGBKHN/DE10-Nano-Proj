/******************************************************************************/
/*! @addtogroup Main
    @file       hal_operation_usb.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * hal_operation_usb.c
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
#include "memorymap.h"
#include "grp_cyclonev_macro.h"
#include "grp_cyclonev_reg.h"
#include "grp_cyclonev_bit_val.h"

// GRUSB


#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "usb_ram.c"

#include "usb_cdc_buffer.h"
/************************** Constant Definitions *****************************/
/*==============================================================================*/
/* ローカル構造体																*/
/*==============================================================================*/
static RingBuffer s_operation_usb_cdc_rx_buffer;
static INT s_usb_connect_status;
/************************** Function Prototypes *****************************/

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

/* 定数値定義 */
/* バッファサイズはFullSpeedとHiSpeedの最大パケットサイズの公倍数でご設定下さい */  /* 1.30 */
#define RX_BUFFER_SIZE         (32768)               /* バッファサイズ               */  /* 1.30 */
#define TX_BUFFER_SIZE         (512)               /* バッファサイズ               */  /* 1.30 */
#define LINE_CODING_SIZE    (7)                 /* Line Coding Structureサイズ  */

/* USB connection state */
#define GRUSB_COMD_DISC                 (0) /* Disconnection State（ADDRESS State   */
                                            /* or Disconnection State)              */
#define GRUSB_COMD_CON                  (1) /* Connection State（CONFIGURED State） */

UINT8   g_acOperationUsbSndDt[TX_BUFFER_SIZE];             /* 送信データ用バッファ             */
UINT8   g_acOperationUsbRcvDt[RX_BUFFER_SIZE];             /* 受信データ用バッファ             */
UINT8   g_acOperationUsbLineCoding[LINE_CODING_SIZE];   /* Line Coding Structureバッファ    */
UINT16  g_sOperationUsbControlLineState;                /* Control Signal Bitmap            */
UINT16  g_usOperationUsbRcvSize;                        /* 受信要求サイズ(MaxPacketSize)    */

/* 関数プロトタイプ */
VOID _OperationUsbInit( VOID );                                            /* 初期処理                                        */
VOID _OperationUsbConnStat( INT iConnStat );                               /* 接続・切断通知処理                              */
VOID _OperationUsbSendEncapsulatedCommad( UINT32 ulSize, UINT8* pucData ); /* SEND_ENCAPSULATED_COMMANDリクエスト受信通知処理 */
VOID _OperationUsbGetEncapsulatedResponse( UINT16 usLength );              /* GET_ENCAPSULATED_RESPONSEリクエスト受信通知処理 */
VOID _OperationUsbSetLineCoding( UINT32 ulSize, UINT8* pucdata );          /* SET_LINE_CODINGリクエスト受信通知処理           */
VOID _OperationUsbGetLineCoding( UINT16 usLength );                        /* GET_LINE_CODINGリクエスト受信通知処理           */
VOID _OperationUsbSetControlLineState( UINT16 usValue );                   /* SET_CONTROL_LINE_STATEリクエスト受信通知処理    */
VOID _OperationUsbResponseAvailable( VOID );                               /* RESPONSE_AVAILABLE通知完了処理                  */
VOID _OperationUsbSendData( UINT32 ulSize, UINT8* pucData, VOID* pInfo );  /* データ送信完了通知処理                          */
VOID _OperationUsbRecvData( UINT32 ulSize, UINT8* pucData, VOID* pInfo );  /* データ受信完了通知処理                          */
VOID _OperationUsbReqRecvData( VOID );                                     /* データ受信要求                                  */

void reset_usb2(void) {
#if defined(SWITCH_USB0_USB1)
	Gpio_out( __HAL_USB0_RESET, RESET_ASSERT );
	OSW_TSK_sleep(2);
	Gpio_out( __HAL_USB0_RESET, RESET_DEASSERT );
#else
	Gpio_out( __HAL_USB2_RESET, RESET_ASSERT );
	OSW_TSK_sleep(2);
	Gpio_out( __HAL_USB2_RESET, RESET_DEASSERT );
#endif
	OSW_TSK_sleep(2);
}
/****************************************************************************/
/* FUNCTION   : _OperationUsbInit                                                  */
/*                                                                          */
/* DESCRIPTION: サンプルプログラムの初期処理を行います                      */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _OperationUsbInit( VOID )
{
    /*----------------------------------------------------------------------*/
    /* 本関数では以下の処理を行っています。                                 */
    /*      ・内部変数の初期化                                              */
    /*      ・バッファの初期化                                              */
    /*      ・コールバック関数の設定                                        */
    /*      ・Communication Function Driverの初期化                         */
    /*      ・Line Coding Structureの初期値を設定                           */
    /*      ・Pullup ON                                                     */
    /*                                                                      */
    /* コールバック関数の設定では、Abstract Control Modelにおいて必須機能の */
    /* ものと、本サンプルで使用するもののみ設定しています。                 */
    /*----------------------------------------------------------------------*/
	// TODO:implement
}

/****************************************************************************/
/* FUNCTION   : _OperationUsbConnStat                                              */
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
VOID _OperationUsbConnStat( INT iConnStat )
{
    /*----------------------------------------------------------------------*/
    /* 接続または切断を検出すると本関数がコールされます。                   */
    /* 接続・切断を検出するとその旨をシリアルにより通知し、ターミナルソフト */
    /* に出力します。                                                       */
    /* また本サンプルでは接続を検知したところで、受信要求を実行しています。 */
    /*----------------------------------------------------------------------*/
	// TODO:implement
	s_usb_connect_status = iConnStat;

	iset_flg(ID_USB2_CB_FLAG, EVT_USB_CON);
}

/****************************************************************************/
/* FUNCTION   : _OperationUsbSendEncapsulatedCommad                                */
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
VOID _OperationUsbSendEncapsulatedCommad( UINT32 ulSize, UINT8* pucData )
{
    /*----------------------------------------------------------------------*/
    /* SEND_ENCAPSULATED_COMMANDリクエストを受信すると本関数がコールされ    */
    /* ます。Abstract Control Modelにおいて必須機能のため、コールバックの   */
    /* 設定を行っていますが、本サンプルではコールされることはありません。   */
    /*----------------------------------------------------------------------*/

    return;
}

/****************************************************************************/
/* FUNCTION   : _OperationUsbGetEncapsulatedResponse                               */
/*                                                                          */
/* DESCRIPTION: GET_ENCAPSULATED_RESPONSEリクエスト受信時の処理関数です     */
/*--------------------------------------------------------------------------*/
/* INPUT      : usLength            要求データサイズ                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _OperationUsbGetEncapsulatedResponse( UINT16 usLength )
{
    /*----------------------------------------------------------------------*/
    /* GET_ENCAPSULATED_RESPONSEリクエストを受信すると本関数がコールされ    */
    /* ます。Abstract Control Modelにおいて必須機能のため、コールバックの   */
    /* 設定を行っていますが、本サンプルではコールされることはありません。   */
    /*----------------------------------------------------------------------*/

    return;
}

/****************************************************************************/
/* FUNCTION   : _OperationUsbSetLineCoding                                         */
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
VOID _OperationUsbSetLineCoding( UINT32 ulSize, UINT8* pucData )
{
    /*----------------------------------------------------------------------*/
    /* SET_LINE_CODINGリクエストを受信すると本関数がコールされます。        */
    /* pucDataにホスト側からの設定値が格納されています。                    */
    /* 今回のシステムではこの値を使用することはありませんので、以下のように */
    /* グローバル変数に情報を保存しておき、GET_LINE_CODINGリクエストを受信  */
    /* した際に、レスポンスデータとして利用します。                         */
    /* データの詳細については_OperationUsbInit関数内で初期値を設定している所を     */
    /* 参照してください。                                                   */
    /*----------------------------------------------------------------------*/
    //GRTEST_OUTPUT("SET_LINE_CODINGリクエスト受信\r\n");
    memcpy( g_acOperationUsbLineCoding, pucData, ulSize );
}

/****************************************************************************/
/* FUNCTION   : _OperationUsbGetLineCoding                                         */
/*                                                                          */
/* DESCRIPTION: GET_LINE_CODINGリクエスト受信時の処理関数です               */
/*--------------------------------------------------------------------------*/
/* INPUT      : usLength            要求データサイズ                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _OperationUsbGetLineCoding( UINT16 usLength )
{
    /*----------------------------------------------------------------------*/
    /* GET_LINE_CODINGリクエストを受信すると本関数がコールされます。        */
    /* このリクエストを受信した場合、GRUSB_COMD_Set_GetLineCoding関数を     */
    /* 使ってデバイス側の設定を通知する必要があります。                     */
    /* 基本的には以下のサンプルのように実装すれば問題ないと思います。       */
    /* またGRUSB_COMD_Set_GetLineCoding関数がエラー終了になるのはケーブルの */
    /* 切断以外はありえませんので、エラー処理が必要ならば追加してください。 */
    /* データの詳細については_OperationUsbInit関数内で初期値を設定している所を     */
    /* 参照してください。                                                   */
    /*----------------------------------------------------------------------*/
	// TODO:implement
}

/****************************************************************************/
/* FUNCTION   : _OperationUsbSetControlLineState                                   */
/*                                                                          */
/* DESCRIPTION: SET_CONTROL_LINE_STATEリクエスト受信時の処理関数です        */
/*--------------------------------------------------------------------------*/
/* INPUT      : usValue             Control Signal Bitmap                   */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _OperationUsbSetControlLineState( UINT16 usValue )
{
    /*----------------------------------------------------------------------*/
    /* SET_CONTROL_LINE_STATEリクエストを受信すると本関数がコールされます。 */
    /* usValueにホスト側の情報が格納されています。                          */
    /* このデータはbit0およびbit1のみを使用しています。                     */
    /*      bit1    RS-232C signal RTS                                      */
    /*      bit0    RS-232C signal DTR                                      */
    /* LinuxやWindowsをホストとした場合、シリアルのオープン・クローズに     */
    /* 合わせbit1を変更するようです。                                       */
    /* また全二重モードで通信を行う場合、bit1は無視されるためおそらくフロー */
    /* 制御には利用していないと思います。そのため本サンプルは受信した情報を */
    /* グローバル変数に保存のみしています。                                 */
    /*----------------------------------------------------------------------*/
    //GRTEST_OUTPUT("SET_CONTROL_LINE_STATEリクエスト受信\r\n");

    g_sOperationUsbControlLineState = usValue;
}

/****************************************************************************/
/* FUNCTION   : _OperationUsbResponseAvailable                                     */
/*                                                                          */
/* DESCRIPTION: RESPONSE_AVAILABLE通知完了時の処理関数です                  */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _OperationUsbResponseAvailable( VOID )
{
    /*----------------------------------------------------------------------*/
    /* RESPONSE_AVAILABLEリクエストを受信すると本関数がコールされます。     */
    /* Abstract Control Modelにおいて必須機能のため、コールバックの設定を   */
    /* 行っていますが、本サンプルではコールされることはありません。         */
    /*----------------------------------------------------------------------*/

    return;
}

/****************************************************************************/
/* FUNCTION   : _OperationUsbSendData                                              */
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
VOID _OperationUsbSendData( UINT32 ulSize, UINT8* pucData, VOID* pInfo )
{
    /*----------------------------------------------------------------------*/
    /* データ送信完了時に本関数がコールされます。                           */
    /* 本サンプルでは、送信完了はループバック送信完了を意味しますので、     */  /* 1.31 */
    /* 新たなデータの受信要求を実行します。                                 */  /* 1.31 */
    /*----------------------------------------------------------------------*/
    //GRTEST_OUTPUT("送信完了通知受信\r\n");
	iset_flg(ID_USB2_CB_FLAG, EVT_USB_EMP);

    return;
}

/****************************************************************************/
/* FUNCTION   : _OperationUsbRecvData                                              */
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
VOID _OperationUsbRecvData( UINT32 ulSize, UINT8* pucData, VOID* pInfo )
{
    /*----------------------------------------------------------------------*/
    /* データ受信完了時に本関数がコールされます。                           */
    /* 本サンプルではデータを折り返し送信する作りとしているため、この関数内 */
    /* では、受信したデータを送信用バッファへコピーし、データの送信要求を   */
    /* 実行します。                                                         */  /* 1.31 */
    /* 送信完了通知は_OperationUsbSendData関数に通知されます。                     */
    /*----------------------------------------------------------------------*/
	// TODO:implement
    /* 受信データを送信データバッファへコピー */
	_usb_cdc_buffer_write(&s_operation_usb_cdc_rx_buffer, pucData, ulSize);
	/* データ受信要求 */
	_OperationUsbReqRecvData();
}

/****************************************************************************/
/* FUNCTION   : _OperationUsbReqRecvData                                           */
/*                                                                          */
/* DESCRIPTION: CDCドライバに対し、データの受信要求を実行します             */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
VOID _OperationUsbReqRecvData( VOID )
{
	iset_flg(ID_USB2_CB_FLAG, EVT_USB_RCV);
}

/****************************************************************************/
/* FUNCTION   : _OperationUsbGetAvailableChar                                           */
/*                                                                          */
/* DESCRIPTION: CDCリングバッファのデータ数を返します。            */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
s32 OperationUsbGetAvailableChar( VOID )
{
	s32 size;

	size = _usb_cdc_buffer_available_bytes(&s_operation_usb_cdc_rx_buffer);

	return size;
}
/****************************************************************************/
/* FUNCTION   : _OperationUsbReqRecvData                                           */
/*                                                                          */
/* DESCRIPTION: CDCリングバッファのデータを取得します             */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/****************************************************************************/
s32 OperationUsbGetRecvData( u8* dst, s32 size_to_read )
{
	s32 received_count;

	received_count = _usb_cdc_buffer_read(
			&s_operation_usb_cdc_rx_buffer,
			dst,
			size_to_read);

	return received_count;
}


void OperationUsbReqSendData(void)
{
	// TODO:implement
}
/*******************************
        initialize test ware USB1
 *******************************/
void OperationUSBConnect(void)
{
	// TODO:implement
	reset_usb2();

	/* Initialize the USB controller */
	_OperationUsbInit();

	// initialize variables
	ex_operation_usb_read_size = 0;
	ex_operation_usb_write_size = 0;
}
/*******************************
        terminate test ware USB1
 *******************************/
#if 0
void OperationUSBDisconnect(void)
{
	// TODO:implement
	OSW_ISR_disable( OSW_INT_USB1_IRQ );
}
#endif

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
#if !defined(_PROTOCOL_ENABLE_ID0G8)
s32 is_usb2_connected(void)
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
#endif
/* EOF */
