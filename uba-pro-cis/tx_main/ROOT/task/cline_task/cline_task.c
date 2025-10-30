/******************************************************************************/
/*! @addtogroup Main
    @file       cline_task.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * cline_task.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

/************************** PRIVATE DEFINITIONS *************************/

/************************** Function Prototypes ******************************/
void cline_task(VP_INT exinf);
void _cline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _cline_system_error(u8 fatal_err, u8 code);

/************************** External functions *******************************/
#if defined(_PROTOCOL_ENABLE_ID003)
extern u8 id003_main(void);
extern bool _id003_check_opt_func(void);
extern void interface_DeInit_id003(void);
#elif defined(_PROTOCOL_ENABLE_ID0G8)
extern u8 id0g8_main(void);
//extern void interface_DeInit_id0G8(void);
#else
extern u8 id003_main(void);
extern void interface_DeInit_id003(void);
#endif /* _PROTOCOL_ENABLE_ID003 */
/************************** Variable declaration *****************************/
extern T_MSG_BASIC cline_msg;

#if (LOOPBACK_UBA==1)
void check_loopback_data_uba(u8 data);
u8 uart_dummy_receive_uba[10];
u8 loop_length;
u8 loop_length_check; //最初の3回はチェックしない
#endif

#if defined(_DEBUG_UART_LOOPBACK)
#include "alt_16550_uart.h"
extern ALT_16550_HANDLE_t handleUart1;
extern OSW_ISR_HANDLE Uart1Isr;
static INT8 s_cline_count = 0;
static T_MSG_BASIC ex_cline_msg;
/*==============================================================================*/
/* 共用割り込みエントリ															*/
/*==============================================================================*/
static void uart1_isr( void )
{
	UINT32 i,j;
	UINT8 c;

	alt_16550_int_status_get(&handleUart1, &i);

	if( (i & 0x04) == 0x04)
	{
		alt_16550_fifo_level_get_rx(&handleUart1, &j);
		if(j > 0)
		{
			alt_16550_fifo_read(&handleUart1, (char *)&c, 1);

		#if (LOOPBACK_UBA==1)
			check_loopback_data_uba(c);
		#else
			_intr_uart_id003( c, UART_STAT_RECV, 0 );
		#endif
		}
	}
	else if( (i & 0x02) == 0x02)
	{
		alt_16550_int_disable_tx(&handleUart1);
		_intr_uart_id003( 0, UART_STAT_TRANSRDY, 0 );
	}
}
void _intr_uart_tempty_teminal(void)
{
	return;
}
void _intr_uart_receive_teminal( void )
{
	return;
}
void _intr_uart_terminal(UINT8 c, UINT8 stat, void *arg )
{

	if( stat & UART_STAT_TMO )
	{
	}
	if( stat & UART_STAT_RECV )
	{
		iset_flg(ID_UART01_CB_FLAG, EVT_UART_RCV);
	}
	if( stat & UART_STAT_TRANEMPTY )
	{
		iset_flg(ID_UART01_CB_FLAG, EVT_UART_EMP);
	}
	if( stat & UART_STAT_TRANSRDY )
	{
		// nothing
	}
}

/*********************************************************************//**
 * @brief process of UART callback notice message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _cline_callback_proc(void)
{
	switch (ex_cline_msg.arg1)
	{
	case TMSG_SUB_RECEIVE:
		_intr_uart_receive_teminal();
		break;
	case TMSG_SUB_EMPTY:
		_intr_uart_tempty_teminal();
		break;
	default:
		break;
	}
}
/*********************************************************************//**
 * @brief process of receive message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _cline_rcv_msg_proc(void)
{
	switch(ex_cline_msg.tmsg_code)
	{
	case TMSG_UART01CB_CALLBACK_INFO:			/* set timer */
		_cline_callback_proc();
		break;
	case TMSG_CLINE_INITIAL_RSP:
		/* nothing todo */
		break;
	default:				/* other */
		/* system error ? */
		_cline_system_error(1, 4);
		break;
	}
}

void uart_init_terminal(void)
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
#if 0	/* ループバック割り込み無効 */
		/* create intgerrupt */
		if( OSW_ISR_create( &Uart1Isr, OSW_INT_UART1_IRQ, &uart1_isr ) == FALSE ){
			stat = ALT_E_ERROR;
		}
#endif

	#if (LOOPBACK_UBA==1)
		/* create intgerrupt */
		if( OSW_ISR_create( &Uart1Isr, OSW_INT_UART1_IRQ, &uart1_isr ) == FALSE ){
			stat = ALT_E_ERROR;
		}
	#endif

	}
	if (stat == ALT_E_SUCCESS)
	{
#if 0	/* ループバック割り込み無効 */
		/* enable intgerrupt */
		stat = alt_16550_int_enable_rx(&handleUart1);
#endif
	#if (LOOPBACK_UBA==1)
		/* enable intgerrupt */
		stat = alt_16550_int_enable_rx(&handleUart1);
	#endif
	}
	if (stat == ALT_E_SUCCESS)
	{
		_uart_fifo_clear_id003();
#if 0	/* ループバック割り込み無効 */
		OSW_ISR_enable( OSW_INT_UART1_IRQ );
#endif
	#if (LOOPBACK_UBA==1)
		OSW_ISR_enable( OSW_INT_UART1_IRQ );
	#endif
	}
#endif

	__hal_if_select_rs232c();

}
u8 uart_dummy[16];
void cline_main(void)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u8 count=0;

	for(int cnt = 0; cnt < 16;cnt++)
	{
		uart_dummy[cnt] = 0x30 + cnt;
	}
	uart_init_terminal();

	#if (LOOPBACK_UBA==1)
		#if 1 //2025-04-10
		uart_dummy[0] = 0x55;
		#else
		uart_dummy[0] = 0x00;
		uart_dummy[1] = 0x01;
		uart_dummy[2] = 0x02;
		uart_dummy[3] = 0x03;
		uart_dummy[4] = 0x04;
		#endif

		#if 1 //2025-04-10
		alt_16550_fifo_write(&handleUart1
			, (const char *)&uart_dummy[0]
			, 1);	
		#else
		alt_16550_fifo_write(&handleUart1
					, (const char *)&uart_dummy[0]
					, 5);
		#endif

	#endif

	/* 待ち */
	while(1)
	{
		ercd = trcv_mbx(ID_CLINE_MBX, (T_MSG **)&tmsg_pt, 500);
		if(ercd == E_OK)
		{
			memcpy(&ex_cline_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(ex_cline_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_cline_system_error(1, 1);
			}
			_cline_rcv_msg_proc();
		}
		else
		{
#if (LOOPBACK_UBA!=1)
			alt_16550_fifo_write(&handleUart1
						, (const char *)&uart_dummy[s_cline_count%16]
						, 1);
#endif
	#if (LOOPBACK_UBA==1)
			if(loop_length_check == 2)
			{
			//最初の3回はチェックしない
				if(loop_length != 1)
				{
					//割り込みが入って来ない時のエラー
					ex_loopback_error = 1;
				}
			}
			else
			{
				loop_length_check++;
			}
			loop_length = 0;

		//	if(ex_loopback_error == 0) //2024-04-09 Errro発生で停止
		//	{
			#if 1 //2025-04-10
			alt_16550_fifo_write(&handleUart1
				, (const char *)&uart_dummy[0]
				, 1);	
			#else		
				alt_16550_fifo_write(&handleUart1
					, (const char *)&uart_dummy[0]
					, 5);
			#endif
			//	}
	#endif
		}

		s_cline_count++;
	}
}
#endif

/*******************************
        cline_task
 *******************************/
void cline_task(VP_INT exinf)
{
	//2025-09-03 UBA500と同様に強制設定にする
	#if defined(_DEBUG_UART_LOOPBACK)
	cline_main();
	#endif
	#if defined(_PROTOCOL_ENABLE_ID003)
	id003_main();
	#elif defined(_PROTOCOL_ENABLE_ID0G8)
	id0g8_main();
	#else
	id003_main();
	#endif /* _PROTOCOL_ENABLE_ID003 */
}



/*********************************************************************//**
 * @brief		Disable interface for Download
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void interface_DeInit(void)
{
	switch(ex_cline_status_tbl.protocol_select)
	{
#if defined(_DEBUG_UART_LOOPBACK)
	case PROTOCOL_SELECT_TEST:
		break;
#endif
#if defined(_PROTOCOL_ENABLE_ID003)
	case PROTOCOL_SELECT_ID003:
		interface_DeInit_id003();
		break;
	default:
		break;
#elif defined(_PROTOCOL_ENABLE_ID0G8)
	case PROTOCOL_SELECT_ID0G8:
		//interface_DeInit_id0g8();
		break;
	default:
		break;
#else
	default:
		interface_DeInit_id003();
		break;
#endif
	}
}

/*********************************************************************//**
 * @brief send task message
 * @param[in]	receiver task id
 * 				task message code
 * 				argument 1
 * 				argument 2
 * 				argument 3
 * 				argument 4
 * @return 		None
 **********************************************************************/
void _cline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_CLINE_TASK;
		t_msg->mpf_id = ID_MBX_MPF;
		t_msg->tmsg_code = tmsg_code;
		t_msg->arg1 = arg1;
		t_msg->arg2 = arg2;
		t_msg->arg3 = arg3;
		t_msg->arg4 = arg4;
		ercd = snd_mbx(receiver_id, (T_MSG *)t_msg);
		if (ercd != E_OK)
		{
			/* system error */
			_cline_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_cline_system_error(1, 2);
	}
}

/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _cline_system_error(u8 fatal_err, u8 code)
{
#ifdef _DEBUG_SYSTEM_ERROR
	//_cline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_cline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_CLINE_TASK, (u16)code, (u16)cline_msg.tmsg_code, (u16)cline_msg.arg1, fatal_err);
}
/* EOF */


bool _cline_check_opt_func(void)
{
	bool result;
#if defined(_PROTOCOL_ENABLE_ID003)
	result = _id003_check_opt_func();
#else
#endif
	return result;
}


#if (LOOPBACK_UBA==1)
u8 uart_dummy[16];
void check_loopback_data_uba(u8 data)
{

	if( loop_length < 10 )
	{
	#if 1 //2025-04-10
		uart_dummy_receive_uba[loop_length] = data;
		if(loop_length == 0)	
		{
			if(uart_dummy[0] == uart_dummy_receive_uba[0])
			{
			/* ok*/
			
			}
			else
			{
				if(loop_length_check == 2)
				{
				//最初の3回はチェックしない	
					/* ng */
					ex_loopback_error = 2;
				}
			}		
		}
		else
		{

		}
		loop_length++;
	#else
		uart_dummy_receive_uba[loop_length] = data;
		if(loop_length == 4)	
		{
			/* data check */
			if(
			uart_dummy[0] == uart_dummy_receive_uba[0]
			&&
			uart_dummy[1] == uart_dummy_receive_uba[1]
			&&
			uart_dummy[2] == uart_dummy_receive_uba[2]
			&&
			uart_dummy[3] == uart_dummy_receive_uba[3]
			&&
			uart_dummy[4] == uart_dummy_receive_uba[4]
			)
			{
			/* ok*/
			
			}
			else
			{
				if(loop_length_check == 2)
				{
				//最初の3回はチェックしない	
					/* ng */
					ex_loopback_error = 2;
				}
			}
		}
		else
		{
			loop_length++;
		}
	#endif		
	}
}
#endif

