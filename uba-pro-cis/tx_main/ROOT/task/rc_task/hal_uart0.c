/****************************************************************************/
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2021                          */
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
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の                                                         */
/* 企業機密情報含んでいます。                                                                                                                           */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を                                                     */
/* 公開も複製も行いません。                                                                                                                                */
/*                                                                          */
/****************************************************************************/
/******************************************************************************/
/**
 * MODEL NAME :
 * @file hal_uart0.c
 * @brief RC T/Qシリアル通信ペリフェラル実装ファイル。
 * @date 2023/10/17
 * @author JCM. Tokyo R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
*******************************************************************************///
/***************************** Include Files *********************************/
#include "kernel.h"
#include "itron.h"
#include <string.h>
#include "custom.h"
#include "common.h"
#include "sysmgr_setup.h"
#include "cyc.h"
#include "js_io.h"
#include "js_oswapi.h"
#include "sensor_ad.h"
#include "sub_functions.h"
#include "status_tbl.h"
#include "alt_16550_uart.h"
#include "rc_task.h"
#include "hal_uart0.h"

#define EXT
#include "com_ram.c"
#include "com_ram_ncache.c"

#if 1
#define ALT_16550_DEVICE_SOCFPGA_UART_RC 	ALT_16550_DEVICE_SOCFPGA_UART0
#define OSW_INT_UART_RC_IRQ 		OSW_INT_UART0_IRQ
#else
#define ALT_16550_DEVICE_SOCFPGA_UART_RC ALT_16550_DEVICE_SOCFPGA_UART1
#define OSW_INT_UART_RC_IRQ OSW_INT_UART1_IRQ
#endif

ALT_16550_HANDLE_t handleUartRC;
OSW_ISR_HANDLE UartRCIsr;
extern const u16 GC2tbl[0x100];

void _intr_uart_rc(UINT8 c, UINT8 stat, void *arg );
static void Uart_send_rc(void);
/*==============================================================================*/
/* 共用割り込みエントリ															*/
/*==============================================================================*/
static void uart_rc_isr( void )
{
	UINT32 i,j;
	UINT8 c;

	alt_16550_int_status_get(&handleUartRC, (ALT_16550_INT_STATUS_t *)&i);

	if( (i & 0x04) == 0x04)
	{
		alt_16550_fifo_level_get_rx(&handleUartRC, (uint32_t *)&j);
		if(j > 0)
		{
			alt_16550_fifo_read(&handleUartRC, (char *)&c, 1);
			_intr_uart_rc( c, UART_STAT_RECV, 0 );
		}
	}
	else if( (i & 0x02) == 0x02)
	{
		alt_16550_int_disable_tx(&handleUartRC);
		_intr_uart_rc( 0, UART_STAT_TRANSRDY, 0 );
	}
}
/************************** PRIVATE DEFINITIONS *************************/
#define BYTE_TO_BYTE (UW)50		/* Polling monitor time ( BYTE TO  BYTE) */

#define WAIT	(1)
#define NOWAIT	(0)

#define UART_RING_BUFSIZE 256
typedef enum {RESET = 0, SET = !RESET} FlagStatus, IntStatus, SetState;
//typedef struct {
//	UINT32 base;
//	UART_PARAM prm;
//	OSW_ISR_HANDLE hIsr;
//} UART_STR;

/** @brief UART Ring buffer structure */
typedef struct
{
    u32 tx_head;                /*!< UART Tx ring buffer head index */
    u32 tx_tail;                /*!< UART Tx ring buffer tail index */
    u32 rx_head;                /*!< UART Rx ring buffer head index */
    u32 rx_tail;                /*!< UART Rx ring buffer tail index */
    u8  tx[UART_RING_BUFSIZE];  /*!< UART Tx data ring buffer */
    u8  rx[UART_RING_BUFSIZE];  /*!< UART Rx data ring buffer */
} UART_RING_BUFFER_T;
typedef struct _UART_INFO
{
	UART_RING_BUFFER_T ring_buff;
} UART_INFO;
/************************** PRIVATE MACROS *************************/
/* Buf mask */
#define __BUF_MASK (UART_RING_BUFSIZE-1)
/* Check buf is full or not */
#define __BUF_IS_FULL(head, tail) ((tail&__BUF_MASK)==((head+1)&__BUF_MASK))
/* Check buf will be full in next receiving or not */
#define __BUF_WILL_FULL(head, tail) ((tail&__BUF_MASK)==((head+2)&__BUF_MASK))
/* Check buf is empty */
#define __BUF_IS_EMPTY(head, tail) ((head&__BUF_MASK)==(tail&__BUF_MASK))
/* Check is last data */
#define __BUF_IS_LAST(head, tail) ((head&__BUF_MASK)==((tail+1)&__BUF_MASK))
/* Reset buf */
#define __BUF_RESET(bufidx)	(bufidx=0)
#define __BUF_INCR(bufidx)	(bufidx=(bufidx+1)&__BUF_MASK)
/************************** PRIVATE VARIABLES *************************/
static UART_INFO  *s_uart_info_rc;
static UART_INFO  _ir_uart_rc_info;
//static u32        s_rc_comm_err;
static u16        s_rc_rx_seq;
static u32        s_rc_rx_len;
static u8         s_rc_rx_sum;
static u8         s_rc_tx_sum;

/************************** PRIVATE VARIABLES *************************/
/************************** PRIVATE FUNCTIONS *************************/
void _intr_uart_receive_rc( UINT8 data, UINT8 stat );
void _intr_uart_tempty_rc(void);
void _intr_uart_tready_rc(void);
void _uart_fifo_clear_rc(void);
/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/



/********************************************************************//**
 * @brief 		UART receive function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void _intr_uart_rc(UINT8 c, UINT8 stat, void *arg )
{
	if( stat & UART_STAT_TMO )
	{
		// nothing
	}
	if( stat & UART_STAT_RECV )
	{
		_intr_uart_receive_rc(c, stat);
	}
	#if 0	//ID-003も引数的にこれは存在していない	
	if( stat & UART_STAT_TRANEMPTY )
	{
		_intr_uart_tempty_rc();
	}
	#endif
	if( stat & UART_STAT_TRANSRDY )
	{
		_intr_uart_tready_rc();
	}

}
void _intr_uart_receive_rc( UINT8 data, UINT8 stat )
{
	// FORMAT:
	//      19200 bps
	//      1 start bit
	//      8 data bits
	//      1 even parity bit
	//      1 stop bit
	// seq0:Received SYNC
	// seq1:Received LENGTH
	// seq2:Received COMMAND & SST & Data
	// seq3:Received SUM check

	u8 tmp_rsp;
	VP_INT send_dtq_data;

	switch (s_rc_rx_seq)
	{
	case 0: /* SYNC */
		if (data == RC_SYNC)
		{
			s_rc_rx_sum = 0;
			s_rc_rx_len = 0;

			s_uart_info_rc->ring_buff.rx[s_rc_rx_len++] = data;

			//s_rc_rx_sum += data; /* SUM加算 */
			s_rc_rx_seq = 1;
		}
		break;
	case 1: /* LENGTH */
		//2025-02-02
		s_uart_info_rc->ring_buff.rx[s_rc_rx_len++] = data;
		s_rc_rx_seq = 2;
		break;
	case 2: /* COMMAND & SST & DATA */
		s_uart_info_rc->ring_buff.rx[s_rc_rx_len++] = data;

			s_rc_rx_sum += data; /* SUM加算 */
		if (s_rc_rx_len - 2 >= (s_uart_info_rc->ring_buff.rx[1])) //Fix loop around of unsigned variable
		{
			s_rc_rx_seq = 3;
		}
		break;
	case 3: /* SUM */
		s_uart_info_rc->ring_buff.rx[s_rc_rx_len] = data;

		if (s_uart_info_rc->ring_buff.rx[(s_rc_rx_len)] != s_rc_rx_sum)
		{
			/* 通信エラー CRC error */
			s_rc_rx_len = 0;
			send_dtq_data = (VP_INT)RC_LISTEN_ERROR;
		}
		else
		{
			//2025-02-02
			if(ex_rc_configuration.board_type == RC_NEW_BOARD)
			{
				if(s_uart_info_rc->ring_buff.rx[2] != 0x10)
				{
					tmp_rsp = s_uart_info_rc->ring_buff.rx[15];
				}
				else
				{
					tmp_rsp = s_uart_info_rc->ring_buff.rx[13];
				}
			}
			else
			{
				tmp_rsp = s_uart_info_rc->ring_buff.rx[13];
			}

			switch(tmp_rsp)
			{
			case	0:
					send_dtq_data = (VP_INT)RC_LISTEN_ACK;					/* ACK				*/
					break;
			case	1:
					send_dtq_data = (VP_INT)RC_LISTEN_BUSY;					/* BUSY				*/
					break;
			case	2:
					send_dtq_data = (VP_INT)RC_LISTEN_INVALID;				/* INVALID			*/
					break;
			case	3:
					send_dtq_data = (VP_INT)RC_LISTEN_CHECKSUM_ERROR;		/* CHECKSUM ERROR	*/
					break;
			case	4:
					send_dtq_data = (VP_INT)RC_LISTEN_DL_READY;				/* DOWNLOAD READY	*/
					break;
			default:
					send_dtq_data = (VP_INT)RC_LISTEN_ERROR;				/* ERROR			*/
					break;
			}
		}
		/* set data to dtq */
		ipsnd_dtq(ID_UART_RC_RX_DTQ, (VP_INT)send_dtq_data);
		s_rc_rx_len = 0;
		s_rc_rx_seq = 0;
		break;
	default:
		s_rc_rx_len = 0;
		s_rc_rx_seq = 0;
		break;
	}
}

/********************************************************************//**
 * @brief 		UART transmit empty interrupt
 * @param[in]	None
 * @return 		None
 *********************************************************************/
#if 0	//ID-003も引数的に存在していない
void _intr_uart_tempty_rc(void)
{

}
#endif

u8 uart_tx_buff_empty_rc(void)
{
	if (__BUF_IS_EMPTY(s_uart_info_rc->ring_buff.tx_head, s_uart_info_rc->ring_buff.tx_tail))
	{
		return 1;
	}
	return 0;
}

/********************************************************************//**
 * @brief 		UART transmit FIFO ready interrupt
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void _intr_uart_tready_rc(void)
{
	Uart_send_rc();
}

/************************************************************************************************/
/* FUNCTION   : OSW_USR_logout                                                                  */
/*                                                                                              */
/* DESCRIPTION: ログ出力処理                                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pcBuf       送信バッファ                                                        */
/*              ulLength    送信データ長                                                        */
/* OUTPUT     : none                                                                            */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
static void Uart_send_rc(void)
{
#if 1 
	//Tunさんが作成した処理、RTQのダウンロード1回のパケット0xD2に対応 動いている
	//既存との違いは、alt_16550_fifo_level_get_txでFIFOの状況を確認してFIFOが空なら入れている。
	//空でない場合 ループは継続している

	ALT_STATUS_CODE stat = ALT_E_SUCCESS;
	u32 level = 0;

	while (uart_tx_buff_empty_rc() == 0)
	{
		/* level : Pointer to an output parameter that contains the level
		or number of characters in the transmitter FIFO 
		For the Altera 16550 Compatible UART, it may not be possible to read the
		FIFO level and this function may always report 0. */
		if (alt_16550_fifo_level_get_tx(&handleUartRC, &level) != ALT_E_SUCCESS)
		{
			/* system error */
			break;
		}
		/* level = 0 is can to transmit */
		/* Level check */
	//	if (!level)
		if(level <= 100) //2025-05-27 バッファは128だがこのサイズにしておく
		{
			if(ALT_E_SUCCESS == alt_16550_fifo_write(&handleUartRC
						, (const char *)&s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_tail]
						, 1))
			{
				__BUF_INCR(s_uart_info_rc->ring_buff.tx_tail);
			}
			else
			{
				break;
			}
		}
		/* level != 0 is FIFO not enough space to transmit */
		else
		{
			alt_16550_int_enable_tx(&handleUartRC); //2025-05-28
			break; //2025-05-28
		}
	}

#else	
	#if 0 //処理1 start
	//もともと使用していた処理、CS生産で使用した処理で元々のID-003に近い形 RTQのダウンロード1回のパケット0x5A、0xD2には対応していない
	uint32_t fifo_size;
	ALT_STATUS_CODE stat = ALT_E_SUCCESS;
	u32 count = 0;
	
	stat = alt_16550_fifo_size_get_tx(&handleUartRC
			, &fifo_size);
	if (ALT_E_SUCCESS != stat)
	{
		return;
	}

	while (uart_tx_buff_empty_rc() == 0)
	{
		if(ALT_E_SUCCESS == alt_16550_fifo_write(&handleUartRC
						, (const char *)&s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_tail]
						, 1))
		{
			count++;
			/* This source is temporary to deal with problem is length of send data so long */
			/* Fifo size is 128 byte. if Count > 64, not stabilize */
			if (count == 0x80 || count == 0xA0)
			{
				tslp_tsk(10); // wait
			}
			__BUF_INCR(s_uart_info_rc->ring_buff.tx_tail);
		}
		else
		{
			break;
		}
	}
	#endif	//処理1 end
#endif		
}

u8 uart_send_rc_encode(RC_TX_ENCRYPTION_BUFFER *txbuff_enc)
{
	s32 rtn = 0;
	u32 bytes = txbuff_enc->length;
	u32 cnt;

	OSW_ISR_disable( OSW_INT_UART_RC_IRQ );

	if ((bytes > 0) && (bytes < 252))
	{
		s_rc_tx_sum = 0;

		// SYNC
		s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = txbuff_enc->start_code;
		//s_rc_tx_sum += s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head]; /* SUM加算しない */
		__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);

		// LENGTH
		s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = (txbuff_enc->length);
		//s_rc_tx_sum += s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head]; /* SUM加算しない */
		__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);

		// DEL CMD
		s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = (txbuff_enc->del_cmd);
		s_rc_tx_sum += s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head]; /* SUM加算 */
		__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);

		// CMD
		s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = (txbuff_enc->cmd);
		s_rc_tx_sum += s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head]; /* SUM加算 */
		__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);


		for (cnt = 0; cnt < 5; cnt++)
		{
			s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = txbuff_enc->sst[cnt];
			s_rc_tx_sum += s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head]; /* SUM加算 */
			__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);
		}

		if(bytes > 7)
		{
			for (cnt = 0; cnt < bytes-7; cnt++)
			{
				s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = txbuff_enc->data[cnt];
				s_rc_tx_sum += s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head]; /* SUM加算 */
				__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);
			}
		}

		s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = s_rc_tx_sum;
		__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);

		Uart_send_rc();
	}

	OSW_ISR_enable( OSW_INT_UART_RC_IRQ );
	return rtn;
}

u8 uart_send_rc(RC_TX_BUFFER *txbuf)
{
	s32 rtn = 0;
	u32 bytes = txbuf->length;
	u32 cnt;

	OSW_ISR_disable( OSW_INT_UART_RC_IRQ );

	if ((bytes > 0) && (bytes < 252))
	{
		s_rc_tx_sum = 0;

		// SYNC
		s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = txbuf->start_code;
		//s_rc_tx_sum += s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head]; /* SUM加算しない */
		__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);

		// LENGTH
		s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = (txbuf->length);
		//s_rc_tx_sum += s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head]; /* SUM加算しない */
		__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);

		// CMD
		s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = (txbuf->cmd);
		s_rc_tx_sum += s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head]; /* SUM加算 */
		__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);


		for (cnt = 0; cnt < 5; cnt++)
		{
			s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = txbuf->sst[cnt];
			s_rc_tx_sum += s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head]; /* SUM加算 */
			__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);
		}

		if(bytes > 6)
		{
			for (cnt = 0; cnt < bytes-6; cnt++)
			{
				s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = txbuf->data[cnt];
				s_rc_tx_sum += s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head]; /* SUM加算 */
				__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);
			}
		}

		s_uart_info_rc->ring_buff.tx[s_uart_info_rc->ring_buff.tx_head] = s_rc_tx_sum;
		__BUF_INCR(s_uart_info_rc->ring_buff.tx_head);

		Uart_send_rc();
	}

	OSW_ISR_enable( OSW_INT_UART_RC_IRQ );
	return rtn;
}

static u8 decode_cmd(u8 *decoded)
{
	u8	res;
	u8	RND;
	u8	NUM;
	
	// 払出しコマンド
	if(ex_last_encryption_cmd == CMD_RC_PAYOUT_REQ)
	{
		/* 受信データから RNDを決める */
		RND = (u8 )(*decoded ^ ex_last_encryption_cmd);

		/* テーブルから正解値 NUM2を決める */
		NUM = (u8 )(GC2tbl[ex_encryption_number]);

		if(RND == NUM)
		{
			*decoded = CMD_RC_PAYOUT_RSP;
			res = TRUE;
		}
		else
		{
			res = FALSE;
		}
	}
	else
	{
		res = FALSE;
	}
	return res;
}

u8 uart_listen_rc(RC_RX_BUFFER *rxbuf) //UBA500の_intr_uart_receive_rc(void)の処理が一部必要 ok ソースの合わせこみ完了
{
#if 1 //2025-02-02
	s32 rtn = 0;
	ER ercd;

	/* check dtq with timeout 500msec */
//2025-05-28	ercd = trcv_dtq(ID_UART_RC_RX_DTQ, (VP_INT *)&rtn, 100);
	ercd = trcv_dtq(ID_UART_RC_RX_DTQ, (VP_INT *)&rtn, 400);


	if(ercd == E_OK)
	{
		if(rtn == RC_LISTEN_ACK || rtn == RC_LISTEN_DL_READY)			/* ACK	*//* DL READY	*/
		{
			rxbuf->start_code = s_uart_info_rc->ring_buff.rx[0];												/* header */
			rxbuf->length 	  = s_uart_info_rc->ring_buff.rx[1];												/* length */

			// 通常コマンド
			if(s_uart_info_rc->ring_buff.rx[2] != CMD_RC_DEL_RSP)
			{
				rxbuf->del_cmd 	= 0;																			/* del cmd */
				rxbuf->cmd 		= (s8)s_uart_info_rc->ring_buff.rx[2];											/* cmd */

				if (ex_rc_configuration.board_type == RC_NEW_BOARD)
				{
					memcpy((u8*)&rxbuf->sst[0], (u8*)&s_uart_info_rc->ring_buff.rx[3], 12);						/* sst */
					rxbuf->res = (s8)s_uart_info_rc->ring_buff.rx[15];											/* res */
					memcpy((u8*)&rxbuf->data[0], (u8*)&s_uart_info_rc->ring_buff.rx[16], (rxbuf->length - 14));	/* data */
			//2025-02-02		rtn = s_uart_info_rc->ring_buff.rx[15];
				}
				else
				{
					memcpy((u8 *)&rxbuf->sst[0], (u8 *)&s_uart_info_rc->ring_buff.rx[3], 10);						/* sst		*/
					rxbuf->res = (s8)s_uart_info_rc->ring_buff.rx[13];												/* res		*/
					memcpy((u8 *)&rxbuf->data[0], (u8 *)&s_uart_info_rc->ring_buff.rx[14], (rxbuf->length - 12)); 	/* data		*/
			//2025-02-02		rtn = s_uart_info_rc->ring_buff.rx[13];
				}
				rxbuf->sum = (s8)s_uart_info_rc->ring_buff.rx[(rxbuf->length + 2)];									/* checksum	*/
				ex_rc_timeout = 0;
			}
			// 暗号コマンド
			else
			{
				/****************/
				/**** 複合化 ****/
				/****************/
				// 受信lengthが暗号キーコマンド、暗号番号コマンド以外の場合
				if ((s_uart_info_rc->ring_buff.rx[1] != TMSG_RC_ENC_KEY_LENGTH) &&
					(s_uart_info_rc->ring_buff.rx[1] != TMSG_RC_ENC_NUM_LENGTH))
				{
					// コマンド複合化
					if (decode_cmd(&s_uart_info_rc->ring_buff.rx[3]) == TRUE)
					{
						rxbuf->del_cmd = (s8)s_uart_info_rc->ring_buff.rx[2]; 											/* del cmd	*/
						rxbuf->cmd = (s8)s_uart_info_rc->ring_buff.rx[3];	  				 							/* cmd		*/

						memcpy((u8 *)&rxbuf->sst[0], (u8 *)&s_uart_info_rc->ring_buff.rx[4], 10);						/* sst		*/
						rxbuf->res = (s8)s_uart_info_rc->ring_buff.rx[14];												/* res		*/
						memcpy((u8 *)&rxbuf->data[0], (u8 *)&s_uart_info_rc->ring_buff.rx[15], (rxbuf->length - 13)); 	/* data		*/
						rxbuf->sum = (s8)s_uart_info_rc->ring_buff.rx[(rxbuf->length + 2)];								/* checksum	*/

						/* 正常の場合はCBC Contextを更新 */
						memcpy(&(ex_cbc_context[0]), &(ex_cbc_context_work[0]), 8);
						++ex_encryption_number;

						if (ex_encryption_number > 0xFF || ex_encryption_number == 0x00)
						{
							ex_encryption_number = 1;
						}
					//2025-02-02	rtn = (s8)s_uart_info_rc->ring_buff.rx[14];	
						ex_rc_timeout = 0;
					}
					else
					{
						rtn = RC_LISTEN_ERROR;
					}
				}
				else
				{
					rxbuf->del_cmd = (s8)s_uart_info_rc->ring_buff.rx[2]; 											/* del cmd	*/
					rxbuf->cmd = (s8)s_uart_info_rc->ring_buff.rx[3];	   											/* cmd		*/

					memcpy((u8 *)&rxbuf->sst[0], (u8 *)&s_uart_info_rc->ring_buff.rx[4], 10);						/* sst		*/
					rxbuf->res = (s8)s_uart_info_rc->ring_buff.rx[14];												/* res		*/
					memcpy((u8 *)&rxbuf->data[0], (u8 *)&s_uart_info_rc->ring_buff.rx[15], (rxbuf->length - 13)); 	/* data		*/
					rxbuf->sum = (s8)s_uart_info_rc->ring_buff.rx[(rxbuf->length + 2)];								/* checksum	*/
				//2025-02-02	rtn = (s8)s_uart_info_rc->ring_buff.rx[14];		
					ex_rc_timeout = 0;
				}
			}
		}
		else if(rtn == RC_LISTEN_BUSY)				/* BUSY				*/
		{
			/* None */
		}
		else if(rtn == RC_LISTEN_INVALID)			/* INVALID			*/
		{
			if((s8)s_uart_info_rc->ring_buff.rx[2] == CMD_RC_GET_CONFIGURATION_RSP)
			{
			#if 0	//現状ここに入る事はない、コマンドに対応していない、RTQ側が旧ソフトの場合には必要になる	
				_rc_rx_buff.start_code 	= (s8)s_uart_info_rc->ring_buff.rx[0];											/* header	*/
				_rc_rx_buff.length 		= (s8)s_uart_info_rc->ring_buff.rx[1];											/* length	*/
				_rc_rx_buff.del_cmd	= 0;																				/* del cmd	*/
				_rc_rx_buff.cmd		= (s8)s_uart_info_rc->ring_buff.rx[2];												/* cmd		*/
				memcpy((u8 *)&_rc_rx_buff.sst[0], (u8 *)&s_uart_info_rc->ring_buff.rx[3], 10);							/* sst		*/
				_rc_rx_buff.res			= (s8)s_uart_info_rc->ring_buff.rx[13];											/* res		*/
				memcpy((u8 *)&_rc_rx_buff.data[0], (u8 *)&s_uart_info_rc->ring_buff.rx[14], (_rc_rx_buff.length - 12));	/* data		*/
				_rc_rx_buff.sum			= (s8)s_uart_info_rc->ring_buff.rx[(_rc_rx_buff.length + 2)];					/* checksum	*/
			#endif
			}
		}
		else if(rtn == RC_LISTEN_CHECKSUM_ERROR)	/* CHECK SUM ERROR	*/
		{
			/* None */
		}
	}
	else if (ercd == E_TMOUT)
	{
		rtn = RC_LISTEN_TMOUT;
	}
	else
	{
		/* system error */
		_rc_system_error(1, 1);
	}

	return (s32)rtn;

#else
	s32 rtn;
	rtn = RC_LISTEN_COMMAND;

	rxbuf->start_code = s_uart_info_rc->ring_buff.rx[0];												/* header */
	rxbuf->length 	  = s_uart_info_rc->ring_buff.rx[1];												/* length */

	if (s_uart_info_rc->ring_buff.rx[2] != CMD_RC_DEL_RSP) /* CMD_RC_DEL_REQ */
	{
		rxbuf->del_cmd 	= 0;																			/* del cmd */
		rxbuf->cmd 		= (s8)s_uart_info_rc->ring_buff.rx[2];											/* cmd */

		if (ex_rc_configuration.board_type == RC_NEW_BOARD)
		{
			memcpy((u8*)&rxbuf->sst[0], (u8*)&s_uart_info_rc->ring_buff.rx[3], 12);						/* sst */
			rxbuf->res = (s8)s_uart_info_rc->ring_buff.rx[15];											/* res */
			memcpy((u8*)&rxbuf->data[0], (u8*)&s_uart_info_rc->ring_buff.rx[16], (rxbuf->length - 14));	/* data */
			rtn = s_uart_info_rc->ring_buff.rx[15];
		}
		else
		{
			memcpy((u8 *)&rxbuf->sst[0], (u8 *)&s_uart_info_rc->ring_buff.rx[3], 10);						/* sst		*/
			rxbuf->res = (s8)s_uart_info_rc->ring_buff.rx[13];												/* res		*/
			memcpy((u8 *)&rxbuf->data[0], (u8 *)&s_uart_info_rc->ring_buff.rx[14], (rxbuf->length - 12)); 	/* data		*/
			rtn = s_uart_info_rc->ring_buff.rx[13];
		}
		rxbuf->sum = (s8)s_uart_info_rc->ring_buff.rx[(rxbuf->length + 2)];									/* checksum	*/
	}
	// 暗号コマンド
	else
	{
		/****************/
		/**** 複合化 ****/
		/****************/
		// 受信lengthが暗号キーコマンド、暗号番号コマンド以外の場合
		if ((s_uart_info_rc->ring_buff.rx[1] != TMSG_RC_ENC_KEY_LENGTH) &&
			(s_uart_info_rc->ring_buff.rx[1] != TMSG_RC_ENC_NUM_LENGTH))
		{
			// コマンド複合化
			if (decode_cmd(&s_uart_info_rc->ring_buff.rx[3]) == TRUE)
			{
				rxbuf->del_cmd = (s8)s_uart_info_rc->ring_buff.rx[2]; 											/* del cmd	*/
				rxbuf->cmd = (s8)s_uart_info_rc->ring_buff.rx[3];	  				 							/* cmd		*/

				memcpy((u8 *)&rxbuf->sst[0], (u8 *)&s_uart_info_rc->ring_buff.rx[4], 10);						/* sst		*/
				rxbuf->res = (s8)s_uart_info_rc->ring_buff.rx[14];												/* res		*/
				memcpy((u8 *)&rxbuf->data[0], (u8 *)&s_uart_info_rc->ring_buff.rx[15], (rxbuf->length - 13)); 	/* data		*/
				rxbuf->sum = (s8)s_uart_info_rc->ring_buff.rx[(rxbuf->length + 2)];								/* checksum	*/

				/* 正常の場合はCBC Contextを更新 */
				memcpy(&(ex_cbc_context[0]), &(ex_cbc_context_work[0]), 8);
				++ex_encryption_number;

				if (ex_encryption_number > 0xFF || ex_encryption_number == 0x00)
				{
					ex_encryption_number = 1;
				}
				rtn = (s8)s_uart_info_rc->ring_buff.rx[14];	
			}
			else
			{
				rtn = RC_LISTEN_DECODE_ERROR;
			}
		}
		else
		{
			rxbuf->del_cmd = (s8)s_uart_info_rc->ring_buff.rx[2]; 											/* del cmd	*/
			rxbuf->cmd = (s8)s_uart_info_rc->ring_buff.rx[3];	   											/* cmd		*/

			memcpy((u8 *)&rxbuf->sst[0], (u8 *)&s_uart_info_rc->ring_buff.rx[4], 10);						/* sst		*/
			rxbuf->res = (s8)s_uart_info_rc->ring_buff.rx[14];												/* res		*/
			memcpy((u8 *)&rxbuf->data[0], (u8 *)&s_uart_info_rc->ring_buff.rx[15], (rxbuf->length - 13)); 	/* data		*/
			rxbuf->sum = (s8)s_uart_info_rc->ring_buff.rx[(rxbuf->length + 2)];								/* checksum	*/
			rtn = (s8)s_uart_info_rc->ring_buff.rx[14];		
		}
	}
	
	if(!uart_rc_is_txd_active())
	{
		_uart_fifo_clear_rc();
	}

	return (s32)rtn;
#endif	
}

#if 0 //2024-10-17 こっちは使用していないようなので、uart_listen_rc を使用している様だ
u8 uart_listen_dl_rc(RC_RX_BUFFER *rxbuf)
{
	s32 rtn;
	rtn = RC_LISTEN_COMMAND;

	
#if defined(RC_BOARD_GREEN)
	if (ex_rc_configuration.board_type == RC_NEW_BOARD)
	{	
		if (s_uart_info_rc->ring_buff.rx[2] == 0x10) /* CMD_RC_DEL_REQ */
		{
			rxbuf->start_code = s_uart_info_rc->ring_buff.rx[0];
			rxbuf->length 	  = s_uart_info_rc->ring_buff.rx[1];	
			rxbuf->cmd 		  = s_uart_info_rc->ring_buff.rx[2];
			memcpy((u8 *)&rxbuf->sst[0], (u8 *)&s_uart_info_rc->ring_buff.rx[3], 10);
			rxbuf->res			= (s8)s_uart_info_rc->ring_buff.rx[13];											/* res		*/
			memcpy((u8 *)&rxbuf->data[0], (u8 *)&s_uart_info_rc->ring_buff.rx[16], (rxbuf->length - 14));	/* data		*/
			rtn = s_uart_info_rc->ring_buff.rx[13];
		}
		else
		{
			rtn = s_uart_info_rc->ring_buff.rx[15];
			memcpy((u8 *)(rxbuf), (u8 *)&s_uart_info_rc->ring_buff.rx[0], (int)s_uart_info_rc->ring_buff.rx_tail+1);
			rxbuf->sum = s_uart_info_rc->ring_buff.rx[s_uart_info_rc->ring_buff.rx_tail];
		}
	}
	else
	{
		rxbuf->start_code = s_uart_info_rc->ring_buff.rx[0];
		rxbuf->length = s_uart_info_rc->ring_buff.rx[1];
		rxbuf->cmd = s_uart_info_rc->ring_buff.rx[2];
		memcpy((u8 *)&rxbuf->sst[0], (u8 *)&s_uart_info_rc->ring_buff.rx[3], 10);
		rxbuf->res = (s8)s_uart_info_rc->ring_buff.rx[13];											  /* res		*/
		memcpy((u8 *)&rxbuf->data[0], (u8 *)&s_uart_info_rc->ring_buff.rx[14], (rxbuf->length - 12)); /* data		*/
		rtn = s_uart_info_rc->ring_buff.rx[13];
	}
#else
	memcpy((u8 *)(rxbuf), (u8 *)&s_uart_info_rc->ring_buff.rx[0], (int)s_uart_info_rc->ring_buff.rx_tail+1);
	rxbuf->sum = s_uart_info_rc->ring_buff.rx[s_uart_info_rc->ring_buff.rx_tail];
	rtn = s_uart_info_rc->ring_buff.rx[13];
#endif // RC_BOARD_GREEN
	if(!uart_rc_is_txd_active())
	{
		_uart_fifo_clear_rc();
	}

	return (s32)rtn;
}
#endif


void uart_init_rc(void)
{
	// FORMAT:
	//      19200 bps
	//      38400 bps
	//      1 start bit
	//      8 data bits
	//      1 even parity bit
	//      1 stop bit
	ALT_STATUS_CODE stat = ALT_E_SUCCESS;
	s_uart_info_rc = &_ir_uart_rc_info;
	stat = alt_16550_init(ALT_16550_DEVICE_SOCFPGA_UART_RC
			, (void *)0
			, (alt_freq_t)0
			, &handleUartRC);
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_reset(&handleUartRC);
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* config settings */
		stat = alt_16550_line_config_set(&handleUartRC
				, ALT_16550_DATABITS_8
				, ALT_16550_PARITY_EVEN
				, ALT_16550_STOPBITS_1);
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* baudrate settings */
		stat = alt_16550_baudrate_set(&handleUartRC
				, ALT_16550_BAUDRATE_38400);
	}
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_enable(&handleUartRC);
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* disable mode bit */
		stat = alt_16550_line_break_disable(&handleUartRC);
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* initialize fifo */
		stat = alt_16550_fifo_enable(&handleUartRC);
	}
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_fifo_trigger_set_rx(&handleUartRC
				, ALT_16550_FIFO_TRIGGER_RX_ANY );
	}
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_fifo_trigger_set_tx(&handleUartRC
				, ALT_16550_FIFO_TRIGGER_TX_EMPTY );
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* create intgerrupt */
		if( OSW_ISR_create( &UartRCIsr, OSW_INT_UART_RC_IRQ, &uart_rc_isr ) == FALSE ){
			stat = ALT_E_ERROR;
		}
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* enable intgerrupt */
		stat = alt_16550_int_enable_rx(&handleUartRC);
	}
	if (stat == ALT_E_SUCCESS)
	{
		_uart_fifo_clear_rc();
		OSW_ISR_enable( OSW_INT_UART_RC_IRQ );
	}
	is_uart0_active = 1;
}


void interface_DeInit_rc(void)
{
	alt_16550_fifo_disable(&handleUartRC);
	alt_16550_int_disable_all(&handleUartRC);
	alt_16550_reset(&handleUartRC);
	alt_16550_uninit(&handleUartRC);
	OSW_ISR_delete(&UartRCIsr);
}

void _uart_fifo_clear_rc(void)
{
	OSW_ISR_disable( OSW_INT_UART_RC_IRQ );
	ALT_STATUS_CODE stat = ALT_E_SUCCESS;
	stat = alt_16550_fifo_clear_all(&handleUartRC);
    if (stat != ALT_E_SUCCESS)
    {
    	program_error();
    }
	__BUF_RESET(s_uart_info_rc->ring_buff.tx_tail);
	__BUF_RESET(s_uart_info_rc->ring_buff.tx_head);
	__BUF_RESET(s_uart_info_rc->ring_buff.rx_tail);
	__BUF_RESET(s_uart_info_rc->ring_buff.rx_head);
	OSW_ISR_enable( OSW_INT_UART_RC_IRQ );
}


/********************************************************************//**
 * @brief 		UART FIFO empty transmit function
 * @param[in]	None
 * @return 		TX FIFO empty or not
 * @retval 		0:Empty
 * @retval 		1:Not Empty
 *********************************************************************/
u8 uart_rc_is_txd_active(void)
{
	u32 status;

	ALT_STATUS_CODE stat = ALT_E_SUCCESS;
	stat = alt_16550_line_status_get(&handleUartRC
			 ,&status);
    if (stat != ALT_E_SUCCESS)
    {
    	program_error();
    }

	/* if TEMT or Tx FIFO is empty */
	if((status & (1<<6)) == (1<<6))
	{
		return 0;
	}
	return 1;
}

/* EOF */
