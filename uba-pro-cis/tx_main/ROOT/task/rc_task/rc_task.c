/******************************************************************************/
/*! @addtogroup Main
    @file       rc_task.c
    @brief      control rc task function
    @date       2021/04/19
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2021/04/19 Development Dept at Tokyo
      -# Initial Version
      -# Branch from Display Task
*****************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "sub_functions.h"
#include "hal.h"
#include "hal_uart0.h"
#include "rc_task.h"

#include "if_rc.h"
#include "rc_operation.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

#ifdef _ENABLE_JDL
#include "jdl_conf.h"
#endif /* _ENABLE_JDL */

/************************** PRIVATE DEFINITIONS *************************/
T_MSG_BASIC rc_msg;
u8 is_uart0_active = 0;

// Check
#define IS_UART_EVT_RECV(x)		((x & EVT_UART_RCV) == EVT_UART_RCV)
//#define IS_UART_EVT_EMPTY(x)	((x & EVT_UART_EMP) == EVT_UART_EMP)
#define IS_UART_EVT_ERROR(x)	((x & EVT_UART_ERR) == EVT_UART_ERR)
//#define IS_UART_EVT_DISCON(x)	((x & EVT_UART_DISCON) == EVT_UART_DISCON)

// Local flag Clear
#define CLR_UART_EVT_RECV(x)	((x &= (~EVT_UART_RCV)))
//#define CLR_UART_EVT_EMPTY(x)	((x &= (~EVT_UART_EMP)))
#define CLR_UART_EVT_ERROR(x)	((x &= (~EVT_UART_ERR)))
//#define CLR_UART_EVT_DISCON(x)	((x &= (~EVT_UART_DISCON)))

/************************** Function Prototypes ******************************/
void rc_task(VP_INT exinf);
void rc_download_proc(void);
void rc_recieve_message_proc(void);
void rc_send_message_proc(void);
void rc_send_command_proc(void);
void rc_rewait_ready_proc(void);


u16 swap16(u16 value);
u32 swap32(u32 value);
/************************** External functions *******************************/

/************************** Variable declaration *****************************/
u8 pol_time;
static u8 s_rc_alarm_code;
static u8 s_rc_alarm_retry; //ワーニングが出ているが取りあえずkeep
static u16 s_rc_task_seq_next;
static u16 s_rc_retry_count;
// log
//#define RC_LOG
#if defined(RC_LOG)
u32 s_rc_watch_index;
typedef struct _RC_WATCH
{
	u32 start;
	u32	end;
	u16 seq;
	u16 address;
	u16 size;
	u16 ope;
}RC_WATCH;
RC_WATCH s_rc_watch[255];
#define RC_LOG_INDEX (s_rc_watch_index%255)
#endif
/************************** PRIVATE FUNCTIONS *************************/
void rc_initialize(void);
void _set_head_status(void);
void _set_rc_status(void);
void _rc_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _rc_system_error(u8 fatal_err, u8 code);

/************************** EXTERN FUNCTIONS *************************/


/*******************************
        rc_task
 *******************************/
void rc_task(VP_INT exinf)
{
	rc_initialize();			/* rc task initialize *///ok

	while(1)
	{
		/* IDLE */
		if(ex_rc_task_seq == RC_SEQ_IDLE)
		{
			/* recieve message from main_task, send message to rc */
			rc_send_message_proc(); //ok
		}
		/* DOWNLOAD MODE */
		else if(ex_rc_task_seq == RC_SEQ_DL_IDLE)
		{
			/* recieve message from main_task, send message to rc, recive message from rc */
			rc_download_proc(); //ok
		}
		/* REWAIT READY MODE */
		else if(ex_rc_task_seq == RC_SEQ_REWAIT_READY)
		{
			/* recieve message from main_task, send message to rc, recive message from rc */
			rc_rewait_ready_proc(); //okソースの合わせこみ
		}
		/* BUSY */
		else
		{
			/* recieve message from rc */
			rc_recieve_message_proc(); //だいたいok

			/* recieve download end command, setup again for rc  */
			if(_rc_rx_buff.cmd == CMD_RC_DL_END_RSP)
			{
				rc_initialize();			/* rc task initialize *///ok
			}
		}
	}
}


/*********************************************************************//**
 * @brief send message procedure
 *  rc task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_send_message_proc(void) //ok
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u32 result_gpio3 = 0x000000;

	tslp_tsk(pol_time);
	/* Check reset signal */
	result_gpio3 = Gpio_in( GPIO_26 );
	if (result_gpio3 == 0) // not signal Low = not Ready(RTQが動作できない状態) RTQ Dead
	{
	//RTQ Dead
		if(ex_rc_status.sst1A.bit.busy == 1) /* Status busy */
		{
			if (ex_rc_error_status == 0) /* --> ERROR */
			{
				ex_rc_error_status = 1;
				#if defined(RC_ENCRYPTION)
				ex_init_add_enc_flg = 1;
				#endif

				_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_ALARM, ALARM_CODE_RC_COM, 0, 0);
			}
		}
		else  /* if not busy ,will retry connect */
		{
			/* 【RC_SEQ_REWAIT_READY : 0x5300】 */
			_rc_set_seq(RC_SEQ_REWAIT_READY);
			ex_rc_task_rewait_rdy_seq = 0;
			/* send message to main_task (TMSG_RC_STATUS_INFO) */
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_RC_RESET, 0, 0, 0);
			ex_rc_rewait_rdy_flg = REWAIT_RDY_ON;
			ex_rc_rewait_rdy_exec_command = 0;
			return ;
		}
	}

	/* recieve message from main task */
	ercd = trcv_mbx(ID_RC_MBX, (T_MSG **)&tmsg_pt, 0);

	if(ercd == E_OK)
	{
		memcpy(&rc_msg, tmsg_pt, sizeof(T_MSG_BASIC));

		if((rel_mpf(rc_msg.mpf_id, tmsg_pt)) != E_OK)
		{
			_rc_system_error(1, 4);	/* system error */
		}

		/* send command messaeg */
		rc_send_command_proc();  //だいたいokのはず
	}
	else if(ercd == E_TMOUT)
	{
		/* send polling message */
		rc_send_polling_proc(); //ok
	}
}


static void rc_receive_message_listen_ack(void) //rc_recieve_message_proc(void) の ret == RC_LISTEN_ACK OK ソースは合わせこんだ
{
	/* check response message from rc */
	rc_received_check_proc();

#if 1 //2025-02-02 順番をUBA500に合わせる
	if (ex_rc_status.sst1A.bit.error == 1)		   /* error */
	{
		if(ex_rc_error_status == 0)
		{
			ex_rc_error_status = 1;
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_ALARM, ALARM_CODE_RC_ERROR, 0, 0);
			_rc_set_seq(RC_SEQ_IDLE);
		}
	}
	else if (ex_rc_status.sst1A.bit.warning == 1)		/* Warning */
	{
		if(ex_rc_warning_status == 0)
		{
			ex_rc_warning_status = 1;
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_RETRY_REQUEST, 0, 0, 0);
			/* 【RC_SEQ_IDLE : 0x5100】 */
			_rc_set_seq(RC_SEQ_IDLE);
		}
	}
	else if ((ex_rc_status.sst21B.bit.u1_detect_dbl != 0) ||								/* detect bill double twin */
		(ex_rc_status.sst1A.bit.quad == 1 && ex_rc_status.sst22B.bit.u2_detect_dbl != 0))	/* detect bill double quad */
	{
		if(ex_rc_error_status == 0)
		{
			ex_rc_error_status = 1;
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_ALARM, ALARM_CODE_RC_DOUBLE, 0, 0);
			_rc_set_seq(RC_SEQ_IDLE);
		}
	}
	else if (ex_rc_status.sst1B.bit.collect_sw)            /* collection switch */
	{
	//ここの処理のみUBA500から変えた
		if (ex_rc_collect_sw == 0)
		{
		#if 1 //2024-11-17
			if( (ex_main_task_mode1 == MODE1_DISABLE) ||
				(ex_main_task_mode1 == MODE1_ACTIVE_DISABLE) ||
				(ex_main_task_mode1 == MODE1_TEST_ACTIVE) ||
				(ex_main_task_mode1 == MODE1_ENABLE &&  ex_operating_mode != OPERATING_MODE_NORMAL)	
			)
			{
				_rc_send_msg(ID_MAIN_MBX, TMSG_RC_SW_COLLECT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
				ex_rc_collect_sw = 1;
			}
			else
			{
				rc_sw_clear_command();
			}
		#else
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_SW_COLLECT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			ex_rc_collect_sw = 1;
		#endif
		}
	}
	else
	{
		ex_rc_error_status = 0;
		ex_rc_warning_status = 0;		
	}
#else
	if (ex_rc_status.sst1B.bit.collect_sw)            /* collection switch */
	{
		if (ex_rc_collect_sw == 0)
		{
		#if 1 //2024-11-17
			if( (ex_main_task_mode1 == MODE1_DISABLE) ||
				(ex_main_task_mode1 == MODE1_ACTIVE_DISABLE) ||
				(ex_main_task_mode1 == MODE1_TEST_ACTIVE) ||
				(ex_main_task_mode1 == MODE1_ENABLE &&  ex_operating_mode != OPERATING_MODE_NORMAL)	
			)
			{
				_rc_send_msg(ID_MAIN_MBX, TMSG_RC_SW_COLLECT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
				ex_rc_collect_sw = 1;
			}
			else
			{
				rc_sw_clear_command();
			}
		#else
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_SW_COLLECT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			ex_rc_collect_sw = 1;
		#endif
		}
	}
	else if (ex_rc_status.sst1A.bit.error == 1)		   /* error */
	{
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_ALARM, ALARM_CODE_RC_ERROR, 0, 0);
	}
	else if (ex_rc_status.sst1A.bit.warning == 1)		/* Warning */
	{
	//2025-01-30	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_ALARM, TMSG_SUB_RETRY_REQUEST, 0, 0);
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_RETRY_REQUEST, 0, 0, 0);
		/* 【RC_SEQ_IDLE : 0x5100】 */
		_rc_set_seq(RC_SEQ_IDLE);
	}
	else if ((ex_rc_status.sst21B.bit.u1_detect_dbl != 0) ||								/* detect bill double twin */
		(ex_rc_status.sst1A.bit.quad == 1 && ex_rc_status.sst22B.bit.u2_detect_dbl != 0))	/* detect bill double quad */
	{
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_ALARM, ALARM_CODE_RC_DOUBLE, 0, 0);
	}
	else												/* nothing */
	{

	}
#endif	
}

static void rc_receive_message_listen_timeout(void) //okソースは合わせこんだ
{
#if 1 //2025-02-02

	if(ex_rc_status.sst1A.bit.busy == 1)
	{
		if(ex_rc_error_status == 0)
		{
			ex_rc_error_status = 1;
			ex_init_add_enc_flg = 1;

			/* send message to main_task (TMSG_RC_STATUS_INFO) */
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_ALARM, ALARM_CODE_RC_COM, 0, 0);
		}
	}
	else
	{
	#if 1//2025-06-20
	//UBA500RTQとは異なるがこの処理を使用する。
	//UBA700RTQやRSの時に静電評価で、RTQからのレスポンスが1.5sくらい遅れて受信する時が50回中1回程度発生する
	//静電気の設定はQA評価より厳しい状況にて発生。
	//ハードと相談した結果、1度エラーにする事で合意
	//エラー後にポーリングに対するレスポンス受信で自動復帰
	//RTQへのCPUリセットは現状行わない(UBA500RTQも行っていない)
		if(ex_rc_timeout == 0)
		{
			ex_rc_timeout = 1;
			ex_rc_timeout_error = 1; //2025-07-22
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_ALARM, ALARM_CODE_RC_COM, 0, 0);
		}
		_rc_set_seq(RC_SEQ_IDLE);
		return;
	#else
		/* 【RC_SEQ_REWAIT_READY : 0x5300】 */
		_rc_set_seq(RC_SEQ_REWAIT_READY);
		ex_rc_task_rewait_rdy_seq = 0;
		/* send message to main_task (TMSG_RC_STATUS_INFO) */
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_RC_RESET, 0, 0, 0);
		ex_rc_rewait_rdy_flg = REWAIT_RDY_ON;
		ex_rc_rewait_rdy_exec_command = 0;
	
		return;
	#endif
	}

	/* wait polling time */
	tslp_tsk(pol_time);

	/* re-send command message */
	uart_send_rc(&_rc_tx_buff);
#else 
	ex_rc_rewait_rdy_flg = REWAIT_RDY_ON;
	/* 【RC_SEQ_IDLE : 0x5100】 */
	_rc_set_seq(RC_SEQ_IDLE);
#endif
}

static void rc_receive_message_listen_dl_ready(u8 type) //rc_recieve_message_proc(void) RC_LISTEN_DL_READY and rc_download_proc ok ソースは合わせこんだ
{

	u32 temp;
	if(type == 0)
	{
		//Dwonaload
		/* None */
	//2025-02-02	ex_rc_download_ready = 1;

		/* set download offset */
		memcpy((u8 *)&ex_rc_dl_offset, (u8 *)&_rc_rx_buff.data[0], 4);

		/* exchange endian */
		temp = swap32(ex_rc_dl_offset);
		ex_rc_dl_offset = temp;

		/* offset check */
		if (ex_rc_dl_offset == 0xFFFFFFFF)
		{
			/* send message to main_task (TMSG_RC_DL_DATA_RSP) */
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DL_DATA_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);

			/* 【RC_SEQ_IDLE : 0x5100】 */
			_rc_set_seq(RC_SEQ_IDLE);
		}
		else if ((u8 *)(Rom2str + ex_rc_dl_offset) >= (u8 *)Rom2end)
		{
			/* send message to main_task (TMSG_RC_DL_DATA_RSP) */
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DL_DATA_RSP, TMSG_SUB_ALARM, 0, 0, 0);

			/* 【RC_SEQ_IDLE : 0x5100】 */
			_rc_set_seq(RC_SEQ_IDLE);
		}
		else
		{
			/* 【RC_SEQ_DL_IDLE : 0x5200】 */
			_rc_set_seq(RC_SEQ_DL_IDLE);
		}

	}
	else
	{
		//Normal
		/* send message to main_task (TMSG_RC_STATUS_INFO) */
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_DOWNLOAD_READY, 0, 0, 0); //おそらく使ってない可能性が高いがUBA500に合わせる
		/* 【RC_SEQ_IDLE : 0x5100】 */
		_rc_set_seq(RC_SEQ_IDLE);

	}

}

#if 0	//2025-02-02
static void rc_receive_message_listen_other(void)
{

}


static void rc_receive_message_listen_busy(void)
{

}

#endif
/*********************************************************************//**
 * @brief download procedure
 *  rc task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_download_proc(void)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u8 ret;
	u32 temp;
	u32 result_gpio3 = 0x000000;

	tslp_tsk(pol_time);

	result_gpio3 = Gpio_in( GPIO_26 );

	if(result_gpio3 == 0)
	{
	//RTQ Dead
		return;
	}

	/* recieve message from main task */
	ercd = trcv_mbx(ID_RC_MBX, (T_MSG **)&tmsg_pt, 0);

	if(ercd == E_OK)
	{
		memcpy(&rc_msg, tmsg_pt, sizeof(T_MSG_BASIC));

		if((rel_mpf(rc_msg.mpf_id, tmsg_pt)) != E_OK)
		{
			_rc_system_error(1, 4);	/* system error */
		}

		/* send command messaeg */
		rc_send_command_proc();//だいたいokのはず
	}
	else if(ercd == E_TMOUT)
	{
		/* ダウンロードREADYの場合はコマンド送る */
		if(ex_rc_download_ready == 1)
		{
			/* send command messaeg */
			rc_dl_data_command(); //ok
		}
		else
		{
			/* send polling message */
			rc_send_polling_proc(); //ok
		}
	}

	/* 【RC_SEQ_DL_WAIT_RESPONSE : 0x5201】 */
	_rc_set_seq(RC_SEQ_DL_WAIT_RESPONSE);


	//2025-02-02 ok
	/* recieve message from Rxd3 inerrupt */
	//ret = uart_listen_rc((u8 *)&_rc_rx_buff.start_code, 0);
	ret = uart_listen_rc(&_rc_rx_buff);	

	if(ret == RC_LISTEN_ACK)						/* ACK								*/
	{
		ex_rc_download_ready = 0;

		/* 【RC_SEQ_DL_IDLE : 0x5200】 */
		_rc_set_seq(RC_SEQ_DL_IDLE);
	}
	else if(ret == RC_LISTEN_DL_READY)				/* DOWNLOAD READY					*/
	{
		/* None */
		ex_rc_download_ready = 1;

		/* set download offset */
		memcpy((u8 *)&ex_rc_dl_offset, (u8 *)&_rc_rx_buff.data[0], 4);

		/* exchange endian */
		temp = swap32(ex_rc_dl_offset);
		ex_rc_dl_offset = temp;

		/* offset check */
		if(ex_rc_dl_offset == 0xFFFFFFFF)
		{
			/* send message to main_task (TMSG_RC_DL_DATA_RSP) */
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DL_DATA_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);

			/* 【RC_SEQ_IDLE : 0x5100】 */
			_rc_set_seq(RC_SEQ_IDLE);
		}
		else if((u8 *)(Rom2str + ex_rc_dl_offset) >= (u8 *)Rom2end)
		{
			/* send message to main_task (TMSG_RC_DL_DATA_RSP) */
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DL_DATA_RSP, TMSG_SUB_ALARM, 0, 0, 0);

			/* 【RC_SEQ_IDLE : 0x5100】 */
			_rc_set_seq(RC_SEQ_IDLE);
		}
		else
		{
			/* 【RC_SEQ_DL_IDLE : 0x5200】 */
			_rc_set_seq(RC_SEQ_DL_IDLE);
		}
	}
#if 1	/* '18-09-19 */
	else if(ret == RC_LISTEN_BUSY)						/* BUSY								*/
	{
		/* 【RC_SEQ_DL_IDLE : 0x5200】 */
		_rc_set_seq(RC_SEQ_DL_IDLE);
	}
#endif
	else if(ret == RC_LISTEN_TMOUT)
	{
		/* send message to main_task (TMSG_RC_STATUS_INFO) */
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STATUS_INFO, TMSG_SUB_ALARM, ALARM_CODE_RC_COM, 0, 0);

		/* 【RC_SEQ_IDLE : 0x5100】 */
		_rc_set_seq(RC_SEQ_IDLE);
	}
	else											/* BUSY, INVALID, CHECKSUM ERROR	*/
	{
		/* None */
	}

}

/*********************************************************************//**
 * @brief recieve message procedure
 *  rc task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_recieve_message_proc(void) //だいたいok
{
	static FLGPTN flag = 0;
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
#if 1 //2025-02-02
	u8 ret;

	/* recieve message from Rxd3 inerrupt */
	//ret = uart_listen_rc((u8 *)&_rc_rx_buff.start_code);
	//or
	//この中は100msecのタイムアウト
	ret = uart_listen_rc(&_rc_rx_buff);	//UBA500は引数を渡しているが実際は使っていないUBA700はどうする？ ok ソースの合わせこみ完了

	if(ret == RC_LISTEN_ACK)						/* ACK								*/
	{
		rc_receive_message_listen_ack(); //ok ソースは合わせこんだ
	}
	else if(ret == RC_LISTEN_DL_READY)				/* DOWNLOAD READY					*/
	{
		rc_receive_message_listen_dl_ready(1); //ok ソースは合わせこんだ
	}
	else if(ret == RC_LISTEN_TMOUT)
	{
		rc_receive_message_listen_timeout();  //okソースは合わせこんだ
	}
	else if(ret == RC_LISTEN_INVALID && _rc_rx_buff.cmd == CMD_RC_GET_CONFIGURATION_RSP) //ok ソースは合わせこんだ
	{
	#if 0 //#if 0	//現状ここに入る事はない、コマンドに対応していない、RTQ側が旧ソフトの場合には必要になる
		/* check response message from rc */
		ex_rc_configuration.unit_type	= RS_NOT_CONNECT;
		ex_rc_configuration.rfid_module = NOT_CONNECT_RFID;
		ex_rc_configuration.board_type	= RC_OLD_BOARD;

		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_GET_CONFIGURATION_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_GET_CONFIGURATION_RSP) */

		/* 【RC_SEQ_IDLE : 0x5100】 */
		_rc_set_seq(RC_SEQ_IDLE);
	#endif
	}
	else											/* BUSY, INVALID, CHECKSUM ERROR	*///ok ソースは合わせこんだ
	{
		//rc_receive_message_listen_other();
		//Ackやタイムアウト以外で何か受信した時に、ポール周期でコマンド再送
		/* wait polling time */
		tslp_tsk(pol_time);

		ex_rc_error_status = 0;
		ex_rc_warning_status = 0;

		#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)	//2025-07-04	
		if( _rc_tx_buff.cmd == CMD_RC_RFID_WRITE_REQ )
		{
			ex_rtq_rfid_res_totaltime = ex_rtq_rfid_res_totaltime + pol_time;
			if(ex_rtq_rfid_res_totaltime > 5000)
			{
				_rc_send_msg(ID_MAIN_MBX, TMSG_RC_RFID_WRITE_RSP, TMSG_SUB_ALARM, 0, 0, 0);
				_rc_set_seq(RC_SEQ_IDLE);
				return;
			}
		}
		#endif
		/* re-send command message */
		uart_send_rc(&_rc_tx_buff);

	}
#endif
}


void rc_rewait_ready_proc(void) //okソースの合わせこみ
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u32 result_gpio3;
	
	/* recieve message from main task */
	ercd = trcv_mbx(ID_RC_MBX, (T_MSG **)&tmsg_pt, 100);

	if(ercd == E_OK)
	{
		memcpy(&rc_msg, tmsg_pt, sizeof(T_MSG_BASIC));

		if((rel_mpf(rc_msg.mpf_id, tmsg_pt)) != E_OK)
		{
			_rc_system_error(1, 4);	/* system error */
		}

		switch (rc_msg.tmsg_code)
		{
		case TMSG_RC_REWAIT_READY_REQ:
			ex_rc_ready_timeout = 3000; /* set timeout 3.0sec */
			ex_rc_task_rewait_rdy_seq = RC_RDYSEQ_WAIT_READY;
			break;
		default:
			break;
		}
	}

	switch (ex_rc_task_rewait_rdy_seq)
	{
	case RC_RDYSEQ_IDLE:
		break;
	case RC_RDYSEQ_WAIT_READY:
		/* RC READY信号受信待ち */
		if (ex_rc_ready_timeout > 0)
		{
			result_gpio3 = Gpio_in(GPIO_26);

			if (result_gpio3 != 0)
			{
			//RTQ OK
				ex_rc_task_rewait_rdy_seq = RC_RDYSEQ_CONFIRM_READY;
			}
		}
		else
		{
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_REWAIT_READY_RSP, TMSG_SUB_ALARM, ALARM_CODE_RC_COM, 0, 0);
			/* 【RC_SEQ_IDLE : 0x5100】 */
			_rc_set_seq(RC_SEQ_IDLE);
		}
		break;
	case RC_RDYSEQ_CONFIRM_READY:
		if (result_gpio3 != 0)
		{
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_REWAIT_READY_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			/* 【RC_SEQ_IDLE : 0x5100】 */
			_rc_set_seq(RC_SEQ_IDLE);
		}
		else
		{
			ex_rc_task_rewait_rdy_seq = RC_RDYSEQ_WAIT_READY;
		}
		break;
	}
}

/*********************************************************************//**
 * @brief set head status
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _set_head_status(void)
{
	_rc_tx_buff.sst[0] = 0x00;

	if(ex_rc_configuration.board_type == RC_NEW_BOARD)
	{
		_rc_tx_buff.sst[0] |= 0x01;
	}
	if(is_rc_rs_unit())
	{
		_rc_tx_buff.sst[0] |= 0x02;
	}
	if(ex_rc_configuration.rfid_module == CONNECT_RFID)
	{
		_rc_tx_buff.sst[0] |= 0x04;
	}

	/* NOTE: default 0xFF */
	_rc_tx_buff.sst[1] = 0xFF;	
	
	// /* TEST type */
	// if (ex_main_test_no == TEST_RC_AGING)
	// {
	// 	_rc_tx_buff.sst[2] = DIPSW1_RC_AGING_TEST;
	// }
	// else if (ex_main_test_no == TEST_RC_AGING_FACTORY)
	// {
	// 	_rc_tx_buff.sst[2] = DIPSW1_RC_AGING_FACTORY;
	// }
	// else
	// {
	// 	_rc_tx_buff.sst[2] = 0x00;
	// }	
	_rc_tx_buff.sst[2] = 0x00;
	#if 1 //2024-10-17
	if(SENSOR_ENTRANCE)
	{
		_rc_tx_buff.sst[3] = 0x01;
	}
	else
	{
		_rc_tx_buff.sst[3] = 0x00;
	}
	#else
	_rc_tx_buff.sst[3] = 0x00;
	#endif

	_rc_tx_buff.sst[4] = 0x00;

}


/*********************************************************************//**
 * @brief set rc status
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _set_rc_status(void)
{
    ex_rc_status.sst1A.byte  = _rc_rx_buff.sst[0];
    ex_rc_status.sst1B.byte	 = _rc_rx_buff.sst[1];
    ex_rc_status.sst21A.byte = _rc_rx_buff.sst[2];
    ex_rc_status.sst21B.byte = _rc_rx_buff.sst[3];
    ex_rc_status.sst31A.byte = _rc_rx_buff.sst[4];
    ex_rc_status.sst31B.byte = _rc_rx_buff.sst[5];
    ex_rc_status.sst22A.byte = _rc_rx_buff.sst[6];
    ex_rc_status.sst22B.byte = _rc_rx_buff.sst[7];
    ex_rc_status.sst32A.byte = _rc_rx_buff.sst[8];
    ex_rc_status.sst32B.byte = _rc_rx_buff.sst[9];

	if(ex_rc_configuration.board_type == RC_NEW_BOARD)
	{
		if(_rc_rx_buff.cmd != CMD_RC_DEL_RSP)
		{
		    ex_rc_status.sst4A.byte  = _rc_rx_buff.sst[10];
		    ex_rc_status.sst4B.byte	 = _rc_rx_buff.sst[11];
		}
	}

	/* Battery */
	if(ex_rc_status.sst22A.bit.battery_detect == 1)
	{
		/* Low battery */
		if(ex_rc_status.sst21A.bit.battery_low == 0 && ex_cline_status_tbl.ex_rc_option_battery_low_detect == 0)
		{
			ex_cline_status_tbl.ex_rc_option_battery_low_detect = 1;
		}
	}
	/* None Battery */
	else
	{
		ex_cline_status_tbl.ex_rc_option_battery_low_detect = 0;
	}

#if defined(RC_LOG)		/* '18-07-09 */
	/* set communication log */
	set_rclog();
#endif

	if(ex_rc_status.sst1A.bit.quad == 1)
	{
		if((ex_rc_status.sst21B.bit.u1_detect == 0 && (ex_rc_internal_jam_flag == RC_TWIN_DRUM1 || ex_rc_internal_jam_flag == RC_TWIN_DRUM2))
		|| (ex_rc_status.sst22B.bit.u2_detect == 0 && (ex_rc_internal_jam_flag == RC_QUAD_DRUM1 || ex_rc_internal_jam_flag == RC_QUAD_DRUM2)))
		{
			ex_rc_internal_jam_flag = 0;
		}
	}
	else
	{
		if(ex_rc_status.sst21B.bit.u1_detect == 0 && (ex_rc_internal_jam_flag == RC_TWIN_DRUM1 || ex_rc_internal_jam_flag == RC_TWIN_DRUM2))
		{
			ex_rc_internal_jam_flag = 0;
		}
	}
}




/*********************************************************************//**
 * @brief RC message from MAIN task procedure
 *  RC task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _rc_idle_main_msg_proc(void)
{
	switch (rc_msg.tmsg_code)
	{
	case TMSG_RC_INIT_REQ:			/* INITIAL message */
		_rc_set_seq(RC_SEQ_CPU_RESET);
		break;
	default:					/* other */
		/* system error ? */
		_rc_system_error(0, 4);
		break;
	}
}


/*********************************************************************//**
 * @brief initialize rc task
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_initialize(void) //ok
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	s_rc_alarm_code = 0;
	s_rc_alarm_retry = 0;
	s_rc_task_seq_next = 0;
	s_rc_retry_count = 0;

	ex_rc_rewait_rdy_flg = REWAIT_RDY_OFF;
	ex_rc_rewait_rdy_exec_command = 0;
	ex_init_add_enc_flg = 0;

//#if defined(RC_BOARD_GREEN)
	/* check response message from rc */
	ex_rc_configuration.unit_type	= RS_NOT_CONNECT;
	ex_rc_configuration.rfid_module = NOT_CONNECT_RFID;
	ex_rc_configuration.board_type	= RC_OLD_BOARD;		//起動時の最初は旧基板として通信
	ex_rc_configuration.unit_type_bk = 0;	//RSのエージング用
//#endif
	
	/* recieve message from main task */
	ercd = trcv_mbx(ID_RC_MBX, (T_MSG **)&tmsg_pt, TMO_FEVR);

	if(ercd == E_OK)
	{
		memcpy(&rc_msg, tmsg_pt, sizeof(T_MSG_BASIC));

		if(rc_msg.tmsg_code != TMSG_RC_INIT_REQ)
		{
			_rc_system_error(1, 4);	/* system error */
		}

		if((rel_mpf(rc_msg.mpf_id, tmsg_pt)) != E_OK)
		{
			_rc_system_error(1, 4);	/* system error */
		}
	}
	/* 2025/03/12 When download was completed, initializing uart0 is not necessary */
	/* if re-initialize, it will cause error in the system */
	if (is_uart0_active == 0)
	{
		/* initialize uart for rc [UBA ULTRA UART0]*/
		uart_init_rc();
	}
	/* 【RC_SEQ_IDLE : 0x5100】 */
	ex_rc_task_seq		= RC_SEQ_IDLE;

	/* send message to main_task (TMSG_RC_INIT_RSP)  */
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_INIT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);

	pol_time = 100;			/* set polling timer 100msec */
	ex_rc_error_status = 0;
	ex_rc_warning_status = 0;
	ex_rc_collect_sw = 0;
	ex_pre_feed_after_jam = 0;
}

/*********************************************************************//**
 * @brief MBX message procedure
 *  rc task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _rc_busy_msg_proc(void)
{	
	switch (rc_msg.tmsg_code)
	{
	case TMSG_RC_INIT_REQ:			/* INITIAL message */
		_rc_set_seq(RC_SEQ_CPU_RESET);
		break;
	default:					/* other */
		/* system error ? */
		_rc_system_error(0, 4);
		break;
	}
}


/*********************************************************************//**
 * @brief rc control sub function
 *  set rc sequence
 * @param[in]	sequence no.
 * 				time out
 * @return 		None
 **********************************************************************/
void _rc_set_seq(u16 seq)
{
	ex_rc_task_seq = seq;

#ifdef _ENABLE_JDL
	// jdl_add_trace(ID_RC_TASK, ((ex_rc_task_seq >> 8) & 0xFF), (ex_rc_task_seq & 0xFF), s_rc_alarm_code, s_rc_alarm_retry, 0);
#endif /* _ENABLE_JDL */

}


/*********************************************************************//**
 * @brief exchange endian(16)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u16 swap16(u16 value)
{
	u16 ret;

	ret  = value << 8;
	ret |= value >> 8;

	return(ret);
}


/*********************************************************************//**
 * @brief exchange endian(32)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u32 swap32(u32 value)
{
	u32 ret;

	ret  = value				<< 24;
    ret |= (value&0x0000FF00)	<<  8;
    ret |= (value&0x00FF0000)	>>  8;
    ret |= value				>> 24;

    return(ret);
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
void _rc_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_RC_TASK;
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
			_rc_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_rc_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _rc_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	_rc_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_TEST_RUNNING, 0, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	//if (fatal_err)
	//{
	//	_rc_send_msg(ID_RC_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	//}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_RC_TASK, (u16)code, (u16)rc_msg.tmsg_code, (u16)rc_msg.arg1, fatal_err);
}


/* EOF */
