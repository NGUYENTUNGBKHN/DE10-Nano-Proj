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
 * MODEL NAME : RBA-40 CIS
 * @file js_uart_id003.c
 * @brief ADP ID-003シリアル通信ペリフェラル実装ファイル。
 * @date 2021/08/05
 * @author JCM. Tokyo R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
*******************************************************************************///
/***************************** Include Files *********************************/
#include "kernel.h"
#include "itron.h"
#include <string.h>
#include "custom.h"
#include "common.h"
#include "sysmgr_setup.h"
#include "js_io.h"
#include "js_oswapi.h"
#include "uart/js_uart_reg.h"
#include "uart/js_uart.h"
#include "sub_functions.h"
#include "id003.h"
#include "id003_crc.h"
#include "js_uart_id003.h"
#include "alt_interrupt_common.h"
#include "alt_16550_uart.h"
#include "hal_gpio.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

ALT_16550_HANDLE_t handleUart1;
OSW_ISR_HANDLE Uart1Isr;

void _intr_uart_id003(UINT8 c, UINT8 stat, void *arg );
static void Uart_send_id003(void);
/*==============================================================================*/
/* 共用割り込みエントリ															*/
/*==============================================================================*/
static void uart1_isr( void )
{
	UINT32 i,j;
	UINT8 c;

	alt_16550_int_status_get(&handleUart1, (ALT_16550_INT_STATUS_t *)&i);

	if( (i & 0x04) == 0x04)
	{
		alt_16550_fifo_level_get_rx(&handleUart1, (uint32_t *)&j);
		if(j > 0)
		{
			alt_16550_fifo_read(&handleUart1, (char *)&c, 1);
			_intr_uart_id003( c, UART_STAT_RECV, 0 );
		}
	}
	else if( (i & 0x02) == 0x02)
	{
		alt_16550_int_disable_tx(&handleUart1);
		_intr_uart_id003( 0, UART_STAT_TRANSRDY, 0 );
	}
}
/************************** PRIVATE DEFINITIONS *************************/
#define BYTE_TO_BYTE (UW)50		/* Polling monitor time ( BYTE TO  BYTE) */

#define WAIT	(1)
#define NOWAIT	(0)

#define UART_RING_BUFSIZE 256
typedef enum {RESET = 0, SET = !RESET} FlagStatus, IntStatus, SetState;
typedef struct {
	UINT32 base;
	UART_PARAM prm;
	OSW_ISR_HANDLE hIsr;
} UART_STR;
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
static UART_INFO  *s_uart_info_id003;
static UART_INFO  _ir_uart1_info;
//static u32        s_id003_comm_err;
static u16        s_id003_rx_seq;
static u32        s_id003_rx_len;
static u8         s_id003_rx_crc1;
static u8         s_id003_rx_crc2;
static u8         s_id003_tx_crc1;
static u8         s_id003_tx_crc2;

/************************** PRIVATE VARIABLES *************************/
/************************** PRIVATE FUNCTIONS *************************/
void _intr_uart_receive_id003( UINT8 data, UINT8 stat );
void _intr_uart_tempty_id003(void);
void _intr_uart_tready_id003(void);
void _uart_fifo_clear_id003(void);
/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/



/********************************************************************//**
 * @brief 		UART receive function (ring buffer used)
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void _intr_uart_id003(UINT8 c, UINT8 stat, void *arg )
{
	if( stat & UART_STAT_TMO )
	{
		// nothing
	}
	if( stat & UART_STAT_RECV )
	{
		_intr_uart_receive_id003(c, stat);
	}
	if( stat & UART_STAT_TRANEMPTY )
	{
		_intr_uart_tempty_id003();
	}
	if( stat & UART_STAT_TRANSRDY )
	{
		_intr_uart_tready_id003();
	}

}
void _intr_uart_receive_id003( UINT8 data, UINT8 stat )
{
	s_uart_info_id003 = &_ir_uart1_info;
	// FORMAT:
	//      9600 bps
	//      1 start bit
	//      8 data bits
	//      1 parity bit
	//      1 stop bit
	// seq0:Received SYNC
	// seq1:Received LENGTH
	// seq2:Received COMMAND & Data
	// seq3:Received CRC
	// seq4:Received CRC check
	if ((s_id003_rx_seq != 0) && (_ir_line_cmd_monitor_time == 0))
	{
		s_id003_rx_len = 0;
		s_id003_rx_seq = 0;
		iset_flg(ID_UART01_CB_FLAG, EVT_UART_ERR);
	}
	_ir_line_cmd_monitor_time = 100;
	switch (s_id003_rx_seq)
	{
	case 0: /* SYNC */
		if (data == ID003_SYNC)
		{
			s_id003_rx_crc1 = 0;
			s_id003_rx_crc2 = 0;
			s_id003_rx_len = 0;

			s_uart_info_id003->ring_buff.rx[s_id003_rx_len++] = data;

			crccal(data, &s_id003_rx_crc1, &s_id003_rx_crc2);	/* CRC演算 */
			s_id003_rx_seq = 1;
		}
		break;
	case 1: /* LENGTH */
		if (data > 4)
		{
			s_uart_info_id003->ring_buff.rx[s_id003_rx_len++] = data;

			crccal(data, &s_id003_rx_crc1, &s_id003_rx_crc2);	/* CRC演算 */
			s_id003_rx_seq = 2;
		}
		else
		{
			s_id003_rx_len = 0;
			s_id003_rx_seq = 0;
			//s_id003_comm_err = ID003_LISTEN_LENGTH_ERROR;
			iset_flg(ID_UART01_CB_FLAG, EVT_UART_ERR);
			/* 通信エラー Length error */
		}
		break;
	case 2: /* COMMAND & DATA */
		s_uart_info_id003->ring_buff.rx[s_id003_rx_len++] = data;

		crccal(data, &s_id003_rx_crc1, &s_id003_rx_crc2);	/* CRC演算 */
		//if (s_id003_rx_len >= (s_uart_info_id003->ring_buff.rx[1] - 2))
		if (s_id003_rx_len + 2 >= (s_uart_info_id003->ring_buff.rx[1])) //Fix loop around of unsigned variable
		{
			s_id003_rx_seq = 3;
		}
		break;
	case 3: /* CRC (L) */
		s_uart_info_id003->ring_buff.rx[s_id003_rx_len++] = data;
		s_id003_rx_seq = 4;
		break;
	case 4: /* CRC (H) */
		s_uart_info_id003->ring_buff.rx[s_id003_rx_len] = data;
		if ((s_uart_info_id003->ring_buff.rx[(s_id003_rx_len - 1)] == s_id003_rx_crc1)
		 && (s_uart_info_id003->ring_buff.rx[(s_id003_rx_len)] == s_id003_rx_crc2))
		{

			s_uart_info_id003->ring_buff.rx_tail = s_id003_rx_len;
			iset_flg(ID_UART01_CB_FLAG, EVT_UART_RCV);
		}
		else
		{
			s_id003_rx_len = 0;
			//s_id003_comm_err = ID003_LISTEN_CRC_ERROR;
			iset_flg(ID_UART01_CB_FLAG, EVT_UART_ERR);
			/* 通信エラー CRC error */
		}
		s_id003_rx_seq = 0;
		break;
	default:
		s_id003_rx_len = 0;
		s_id003_rx_seq = 0;
		break;
	}
}

/********************************************************************//**
 * @brief 		UART transmit empty interrupt
 * @param[in]	None
 * @return 		None
 *********************************************************************/
void _intr_uart_tempty_id003(void)
{
	_uart_fifo_clear_id003();
	iset_flg(ID_UART01_CB_FLAG, EVT_UART_EMP);
}

u8 uart_tx_buff_empty_id003(void)
{
	s_uart_info_id003 = &_ir_uart1_info;

	if (__BUF_IS_EMPTY(s_uart_info_id003->ring_buff.tx_head, s_uart_info_id003->ring_buff.tx_tail))
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
void _intr_uart_tready_id003(void)
{
#if 1
#ifdef JSL_UART
	while (uart_tx_buff_empty_id003() == 0)
	{
		if(TRUE == (res = Uart_send( &hUart1, s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_tail], NOWAIT)))
		{
			__BUF_INCR(s_uart_info_id003->ring_buff.tx_tail);
		}
		else
		{
			break;
		}
	}
#else
	// TODO:
	Uart_send_id003();
#endif
#else
	while (uart_tx_buff_empty_id003() == 0) {
		Uart_send( &hUart1, s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_tail], NOWAIT);
		__BUF_INCR(s_uart_info_id003->ring_buff.tx_tail);
	}
#endif
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
static void Uart_send_id003(void)
{
#if 1
#ifdef JSL_UART
	OSW_ISR_disable( OSW_INT_UART1_IRQ );
	while (uart_tx_buff_empty_id003() == 0)
	{
		if(TRUE == (res = Uart_send( &hUart1, s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_tail], NOWAIT)))
		{
			__BUF_INCR(s_uart_info_id003->ring_buff.tx_tail);
		}
		else
		{
			break;
		}
	}
	OSW_ISR_enable( OSW_INT_UART1_IRQ );
#else
	// TODO:
	uint32_t fifo_size;
	ALT_STATUS_CODE stat = ALT_E_SUCCESS;
	stat = alt_16550_fifo_size_get_tx(&handleUart1
			, &fifo_size);
	if (ALT_E_SUCCESS != stat)
	{
		return;
	}

	while (uart_tx_buff_empty_id003() == 0)
	{
		if(ALT_E_SUCCESS == alt_16550_fifo_write(&handleUart1
						, (const char *)&s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_tail]
						, 1))
		{
			__BUF_INCR(s_uart_info_id003->ring_buff.tx_tail);
		}
		else
		{
			break;
		}
	}
#if 0
	uint32_t write_size;
	write_size = s_uart_info_id003->ring_buff.tx_head - s_uart_info_id003->ring_buff.tx_tail;
	if(write_size > fifo_size)
	{
		write_size = fifo_size;
	}
	if(write_size == 0)
	{
		return;
	}
	OSW_ISR_disable( OSW_INT_UART1_IRQ );

	for(int i = 0; i < write_size;i++)
	{
		__BUF_INCR(s_uart_info_id003->ring_buff.tx_tail);
	}
#endif
#endif
#else
	OSW_ISR_disable( OSW_INT_USB0_IRQ );
	while (uart_tx_buff_empty_id003() == 0) {
		Uart_send( &hUart1, s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_tail], NOWAIT);
		__BUF_INCR(s_uart_info_id003->ring_buff.tx_tail);
	}
	OSW_ISR_enable( OSW_INT_USB0_IRQ );
#endif
}
//extern u8 uart_send_id003(u8 *txbuf, u8 txlen)
u8 uart_send_id003(ID003_CMD_BUFFER *txbuf)
{
	s32 rtn = 0;
	u32 bytes = txbuf->length;
	u32 cnt;
#if 1
	OSW_ISR_disable( OSW_INT_UART1_IRQ );

	if ((bytes > 0) && (bytes < 254))
	{
		s_id003_tx_crc1 = 0;
		s_id003_tx_crc2 = 0;

		// SYNC
		s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_head] = 0xFC;
		crccal(s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_head], &s_id003_tx_crc1, &s_id003_tx_crc2);	/* CRC演算 */
		__BUF_INCR(s_uart_info_id003->ring_buff.tx_head);

		// LENGTH
		s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_head] = (txbuf->length);
		crccal(s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_head], &s_id003_tx_crc1, &s_id003_tx_crc2);	/* CRC演算 */
		__BUF_INCR(s_uart_info_id003->ring_buff.tx_head);

		// CMD
		s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_head] = (txbuf->cmd);
		crccal(s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_head], &s_id003_tx_crc1, &s_id003_tx_crc2);	/* CRC演算 */
		__BUF_INCR(s_uart_info_id003->ring_buff.tx_head);

		if(bytes > 5)
		{
			for (cnt = 0; cnt < bytes-5; cnt++)
			{
				s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_head] = txbuf->data[cnt];
				crccal(s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_head], &s_id003_tx_crc1, &s_id003_tx_crc2);	/* CRC演算 */
				__BUF_INCR(s_uart_info_id003->ring_buff.tx_head);
			}
		}

		s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_head] = s_id003_tx_crc1;
		__BUF_INCR(s_uart_info_id003->ring_buff.tx_head);

		s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_head] = s_id003_tx_crc2;
		__BUF_INCR(s_uart_info_id003->ring_buff.tx_head);

		Uart_send_id003();
	}

	OSW_ISR_enable( OSW_INT_UART1_IRQ );
#else

	if (txlen > 0)
	{
		Uart_send( &hUart1, s_uart_info_id003->ring_buff.tx[s_uart_info_id003->ring_buff.tx_tail], NOWAIT);
	}
#endif
	return rtn;
}


//u8 uart_listen_id003(u8 *rxbuf, u8 buflen, u8 *size)
u8 uart_listen_id003(ID003_CMD_BUFFER *rxbuf)
{
	s32 rtn;
	s_uart_info_id003 = &_ir_uart1_info;
	rtn = ID003_LISTEN_COMMAND;

	memcpy((u8 *)(rxbuf), (u8 *)&s_uart_info_id003->ring_buff.rx[0], (int)s_uart_info_id003->ring_buff.rx_tail+1);
	rxbuf->crc1 = s_uart_info_id003->ring_buff.rx[s_uart_info_id003->ring_buff.rx_tail-1];
	rxbuf->crc2 = s_uart_info_id003->ring_buff.rx[s_uart_info_id003->ring_buff.rx_tail];
	_uart_fifo_clear_id003();

	return (s32)rtn;
}


void uart_init_id003(void)
{
#ifdef JSL_UART
	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * EVEN parity
	 */
	UART_STR *pUart;
	s_uart_info_id003 = &_ir_uart1_info;
	UART_PARAM prm;
	u32 dummy;

	memset(s_uart_info_id003, 0, sizeof(UART_INFO));

	OSW_ISR_set_priority(OSW_INT_UART1_IRQ, IPL_USER_HIGHEST);
	uart1_reset();
	/* UART1 */
	if( Uart_init( NULL ) == FALSE )
	{
		/* system error */
		program_error();
	}
	/* UARTコンソール */
	memset( (void *)&prm, 0, sizeof(prm) );
	prm.port = 1;
	prm.recv_cb_func = _intr_uart_id003;
	if( Uart_open( &hUart1, &prm ) == FALSE )
	{
		/* system error */
		program_error();
	}
	pUart = (UART_STR *)hUart1.hdl;
	dummy = IOREG32(pUart->base,UART_MSR);
#else
	// TODO:
	ALT_STATUS_CODE stat = ALT_E_SUCCESS;
	stat = alt_16550_init(ALT_16550_DEVICE_SOCFPGA_UART1
			, (void *)0
			, (alt_freq_t)0
			, &handleUart1);
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_reset(&handleUart1);
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* config settings */
		stat = alt_16550_line_config_set(&handleUart1
				, ALT_16550_DATABITS_8
				, ALT_16550_PARITY_EVEN
				, ALT_16550_STOPBITS_1);
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* baudrate settings */
		stat = alt_16550_baudrate_set(&handleUart1
				, ALT_16550_BAUDRATE_9600);
	}
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_enable(&handleUart1);
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* disable mode bit */
		stat = alt_16550_line_break_disable(&handleUart1);
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* initialize fifo */
		stat = alt_16550_fifo_enable(&handleUart1);
	}
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_fifo_trigger_set_rx(&handleUart1
				, ALT_16550_FIFO_TRIGGER_RX_ANY );
	}
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_fifo_trigger_set_tx(&handleUart1
				, ALT_16550_FIFO_TRIGGER_TX_EMPTY );
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* create intgerrupt */
		if( OSW_ISR_create( &Uart1Isr, OSW_INT_UART1_IRQ, &uart1_isr ) == FALSE ){
			stat = ALT_E_ERROR;
		}
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* enable intgerrupt */
		stat = alt_16550_int_enable_rx(&handleUart1);
	}
	if (stat == ALT_E_SUCCESS)
	{
		_uart_fifo_clear_id003();
		OSW_ISR_enable( OSW_INT_UART1_IRQ );
	}
#endif
#if defined(PRJ_IVIZION2)
#if (DEBUG_UART_CCTALK==1)
	__hal_if_select_cctalk();
#else
	if(!__hal_if_set_read())
	{
		// i/F Switch OFF
		__hal_if_select_rs232c();
	}
	else
	{
		// i/F Switch ON
		__hal_if_select_pc();
	}
#endif
#else
	__hal_if_select_rs232c();
#endif
}


void interface_DeInit_id003(void)
{
#ifdef JSL_UART
	Uart_close(&hUart1);
#else
	alt_16550_fifo_disable(&handleUart1);
	alt_16550_int_disable_all(&handleUart1);
	alt_16550_reset(&handleUart1);
	alt_16550_uninit(&handleUart1);
	OSW_ISR_delete(&Uart1Isr);
#endif
}

void uart_txd_stall_id003(void)
{
	// keep txd high(rs232c txd low)
	uart1_halt(1);
}
void uart_txd_active_id003(void)
{
	// clear txd high
	uart1_halt(0);
#if 0
	interface_DeInit_id003();
	uart_init_id003();
#endif
}

void _uart_fifo_clear_id003(void)
{
	OSW_ISR_disable( OSW_INT_UART1_IRQ );
#ifdef JSL_UART
#else
	// TODO:
	ALT_STATUS_CODE stat = ALT_E_SUCCESS;
	stat = alt_16550_fifo_clear_all(&handleUart1);
    if (stat != ALT_E_SUCCESS)
    {
    	program_error();
    }
#endif
	__BUF_RESET(s_uart_info_id003->ring_buff.tx_tail);
	__BUF_RESET(s_uart_info_id003->ring_buff.tx_head);
	__BUF_RESET(s_uart_info_id003->ring_buff.rx_tail);
	__BUF_RESET(s_uart_info_id003->ring_buff.rx_head);
	/* clear pending interrupts */
	alt_int_dist_pending_clear(ALT_INT_INTERRUPT_UART1);
	alt_int_dist_priority_set(ALT_INT_INTERRUPT_UART1, IPL_USER_NORMAL);
	OSW_ISR_enable( OSW_INT_UART1_IRQ );
}



u8 UART_change_baudrate(uint32_t baudrate)
{
	ALT_STATUS_CODE stat = ALT_E_SUCCESS;
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_int_disable_rx(&handleUart1);
	}
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_fifo_disable(&handleUart1);
	}
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_disable(&handleUart1);
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* baudrate settings */
		stat = alt_16550_baudrate_set(&handleUart1
				, baudrate);
	}
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_enable(&handleUart1);
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* initialize fifo */
		stat = alt_16550_fifo_enable(&handleUart1);
	}
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_fifo_trigger_set_rx(&handleUart1
				, ALT_16550_FIFO_TRIGGER_RX_ANY );
	}
	if (stat == ALT_E_SUCCESS)
	{
		stat = alt_16550_fifo_trigger_set_tx(&handleUart1
				, ALT_16550_FIFO_TRIGGER_TX_EMPTY );
	}
	if (stat == ALT_E_SUCCESS)
	{
		/* enable intgerrupt */
		_uart_fifo_clear_id003();
		OSW_ISR_enable( OSW_INT_UART1_IRQ );
		alt_16550_int_enable_rx(&handleUart1);
	}
	return stat;
}

/********************************************************************//**
 * @brief 		UART FIFO empty transmit function
 * @param[in]	None
 * @return 		TX FIFO empty or not
 * @retval 		0:Empty
 * @retval 		1:Not Empty
 *********************************************************************/
u8 UART_is_txd_active(void)
{
	u32 status;

#ifdef JSL_UART
	UART_STR *pUart;
	pUart = (UART_STR *)hUart1.hdl;
	status = IOREG32(pUart->base,UART_LSR);
#else
	// TODO:
	ALT_STATUS_CODE stat = ALT_E_SUCCESS;
	stat = alt_16550_line_status_get(&handleUart1
			 ,&status);
    if (stat != ALT_E_SUCCESS)
    {
    	program_error();
    }
#endif

	/* if TEMT or Tx FIFO is empty */
 	if((status & (1<<6)) == (1<<6))
	{
		return 0;
	}
	return 1;
}

/* EOF */
