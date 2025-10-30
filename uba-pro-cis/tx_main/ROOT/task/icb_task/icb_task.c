/******************************************************************************/
/*! @addtogroup Main
    @file       icb_task.c
    @brief      control icb task function
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

#if defined(UBA_RTQ)
	#include "if_rc.h"
	#include "rc_operation.h"
#endif // UBA_RTQ

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"
#include "user/icb/icb_ram.c"
#include "user/icb/icb.h"

#ifdef _ENABLE_JDL
#include "jdl_conf.h"
#endif /* _ENABLE_JDL */

#if defined(UBA_RTQ)
static u8 icb_rc_rfid_write_buffer[256];
static u8 icb_rc_rfid_read_buffer[256];
#endif

#if defined(UBA_LOG)
#define ICB_LOG_COUNT_UBA 30
u16 icb_log[ICB_LOG_COUNT_UBA];
u8 icb_log_index;
#endif

/************************** PRIVATE DEFINITIONS *************************/
enum _ICB_SEQ
{
	ICB_SEQ_IDLE					= 0x0000,
	ICB_SEQ_INITIAL					= 0x0100,
	ICB_SEQ_INITIAL_RECOVERY		= 0x0111,
	ICB_SEQ_INITIAL_DATA_CHECK		= 0x0121,
	ICB_SEQ_INITIAL_DATA_WRITE		= 0x0131,
	ICB_SEQ_INITIAL_BACKUP			= 0x0140,

	ICB_SEQ_ACCEPT					= 0x0300,
	ICB_SEQ_ACCEPT_FRAM				= 0x0340,	// reserved _icb_fram_backup_seq

	ICB_SEQ_REJECT					= 0x0400,
	ICB_SEQ_REJECT_FRAM				= 0x0440,	// reserved _icb_fram_backup_seq

	ICB_SEQ_REJECT_TICKET			= 0x0500,
	ICB_SEQ_REJECT_TICKET_FRAM		= 0x0540,	// reserved _icb_fram_backup_seq

	ICB_SEQ_ERROR_CODE				= 0x0600,
	ICB_SEQ_ERROR_CODE_FRAM			= 0x0640,	// reserved _icb_fram_backup_seq

	ICB_SEQ_SET_TIME				= 0x0800,
	ICB_SEQ_SET_TIME_FRAM			= 0x0840,	// reserved _icb_fram_backup_seq

#if defined(UBA_RTQ_ICB)
	ICB_SEQ_ACCEPT_RTQ				= 0x0B00,
#endif

	ICB_SEQ_WARNING					= 0x1000,	// WARNING時 通信失敗中
};

#define ICB_SEQ_TIMEOUT				3000	// 3sec
#define ICB_SEQ_CMD_RETRY			30		// 30msec   hal_uart_icbに同じdefineある。値を同じにすること

#define IS_ICB_EVENT_TIMEOUT(x)			((x & EVT_ICB_TIMEOUT)					== EVT_ICB_TIMEOUT)
#define IS_ICB_EVENT_READ_SUCCESS(x)	((x & EVT_ICB_DATA_READ)				== EVT_ICB_DATA_READ)
#define IS_ICB_EVENT_READ_FAIL(x)		((x & EVT_ICB_DATA_READ_FAIL)			== EVT_ICB_DATA_READ_FAIL)
#define IS_ICB_EVENT_WRITE_SUCCESS(x)	((x & EVT_ICB_DATA_WRITE)				== EVT_ICB_DATA_WRITE)
#define IS_ICB_EVENT_WRITE_FAIL(x)		((x & EVT_ICB_DATA_WRITE_FAIL)			== EVT_ICB_DATA_WRITE_FAIL)
#define IS_FRAM_EVENT_WRITE_SUCCESS(x)	((x & EVT_FRAM_DATA_WRITE)				== EVT_FRAM_DATA_WRITE)
#define IS_FRAM_EVENT_WRITE_FAIL(x)		((x & EVT_FRAM_DATA_WRITE_FAIL)			== EVT_FRAM_DATA_WRITE_FAIL)
/************************** Function Prototypes ******************************/
void icb_task(VP_INT exinf);
void _icb_set_seq(u16 seq, u16 time_out);

/************************** External functions *******************************/

/************************** Variable declaration *****************************/
static T_MSG_BASIC icb_msg;
static u8 s_icb_alarm_code;
static u8 s_icb_alarm_retry; //ワーニングが出ているが取りあえずkeep
static u8 s_icb_reject_code;//use
static u8 s_icb_log_denomi_code;
static u8 s_icb_log_reject_code;
static u8 s_icb_log_error_code;
static u16 s_icb_task_seq_next;
static u16 s_icb_retry_count;
static u16 s_icb_req_task_id;

#if defined(UBA_RTQ_ICB)
static u8 s_icb_arg1;
static u8 s_icb_arg2;
static u8 s_icb_arg3;
#endif

// log
#define ICB_LOG
#if defined(ICB_LOG)
u32 s_icb_watch_index;
typedef struct _ICB_WATCH
{
	u32 start;
	u32	end;
	u16 seq;
	u16 address;
	u16 size;
	u16 ope;
}ICB_WATCH;
ICB_WATCH s_icb_watch[255];
#define ICB_LOG_INDEX (s_icb_watch_index%255)
#endif
/************************** PRIVATE FUNCTIONS *************************/
/* ICB initialize sequence */
#if defined(UBA_RTQ_ICB)
	static void _icb_initial_seq_proc_rtq(u32 flag, s32 data);
	static void _icb_initial_0100_seq_rtq(u32 flag, s32 data);
	static void _icb_initial_0101_seq_rtq(u32 flag, s32 data);
	// recovery
	static void _icb_initial_0111_seq_rtq(u32 flag, s32 data);
	// data check
	static void _icb_initial_0121_seq_rtq(u32 flag, s32 data);
	// initialize data
	static void _icb_initial_0131_seq_rtq(u32 flag, s32 data);
	static void _icb_initial_0135_seq_rtq(u32 flag, s32 data); //new
	static void _icb_initial_0136_seq_rtq(u32 flag, s32 data); //new
	// when the initialize, read data 
	static void _icb_initial_0150_seq_rtq(u32 flag, s32 data);
	static void _icb_initial_0151_seq_rtq(u32 flag, s32 data);
	static void _icb_initial_0160_seq_rtq(u32 flag, s32 data);
	static void _icb_initial_0161_seq_rtq(u32 flag, s32 data);
	static void _icb_initial_0162_seq_rtq(u32 flag, s32 data);
	static void _icb_initial_0150_seq(u32 flag, s32 data);
	static void _icb_initial_0151_seq(u32 flag, s32 data);

	static void _icb_initial_0160_seq(u32 flag, s32 data);
	static void _icb_initial_0161_seq(u32 flag, s32 data);
	static void _icb_initial_0162_seq(u32 flag, s32 data);
	static void _icb_accept_rtq_req_proc(void);
	static void _icb_accept_rtq_seq_proc(u32 flag, s32 data);
	static void _icb_fram_backup_rtq_seq(u32 flag, s32 data);
	static void _icb_accept_rtq_0B00_seq(u32 flag, s32 data);
	static void _icb_rc_rfid_read_rsp_proc(void);
	static void _icb_rc_rfid_reset_rsp_proc(void);
	static void _icb_rc_rfid_write_rsp_proc(void);
#else
	static void _icb_initial_seq_proc(u32 flag, s32 data);
	static void _icb_initial_0112_seq(u32 flag, s32 data);
	static void _icb_initial_0113_seq(u32 flag, s32 data);
	static void _icb_fram_backup(u32 next_seq);
	static void _icb_fram_backup_seq(u32 flag, s32 data);
	static void _icb_set_time_req_proc(void);
	static void _icb_accept_req_proc(void);
	static void _icb_reject_req_proc(void);
	static void _icb_reject_ticket_req_proc(void);
	static void _icb_error_code_req_proc(void);
	static void _icb_rfid_status_info_proc(void);
	static void _icb_rfid_read_rsp_proc(void);
	static void _icb_rfid_write_rsp_proc(void);
	static void _icb_rfid_init_rsp_proc(void);
#endif
/*----------------------------------------------------------------------------*/

static void _icb_initial_0100_seq(u32 flag, s32 data);
static void _icb_initial_0101_seq(u32 flag, s32 data);
// recovery
static void _icb_initial_0111_seq(u32 flag, s32 data);


// data check
static void _icb_initial_0121_seq(u32 flag, s32 data);
// initialize data
static void _icb_initial_0131_seq(u32 flag, s32 data);
static void _icb_initial_0132_seq(u32 flag, s32 data);
static void _icb_initial_0133_seq(u32 flag, s32 data);
static void _icb_initial_0134_seq(u32 flag, s32 data);

/* ICB accept sequence */
static void _icb_accept_seq_proc(u32 flag, s32 data);
static void _icb_accept_0300_seq(u32 flag, s32 data);
static void _icb_accept_0301_seq(u32 flag, s32 data);
static void _icb_accept_0302_seq(u32 flag, s32 data);

/* ICB reject sequence */
static void _icb_reject_seq_proc(u32 flag, s32 data);
static void _icb_reject_0400_seq(u32 flag, s32 data);
static void _icb_reject_0401_seq(u32 flag, s32 data);
static void _icb_reject_0402_seq(u32 flag, s32 data);

/* ICB reject ticket sequence */
static void _icb_reject_ticket_seq_proc(u32 flag, s32 data);
static void _icb_reject_ticket_0500_seq(u32 flag, s32 data);
static void _icb_reject_ticket_0501_seq(u32 flag, s32 data);
static void _icb_reject_ticket_0502_seq(u32 flag, s32 data);
/* ICB error sequence */
static void _icb_err_code_seq_proc(u32 flag, s32 data);
static void _icb_errcode_0600_seq(u32 flag, s32 data);
static void _icb_errcode_0601_seq(u32 flag, s32 data);
static void _icb_errcode_0602_seq(u32 flag, s32 data);

/* ICB set time sequence */
static void _icb_set_time_seq_proc(u32 flag, s32 data);
static void _icb_set_time_0800_seq(u32 flag, s32 data);
static void _icb_set_time_0801_seq(u32 flag, s32 data);
static void _icb_set_time_0802_seq(u32 flag, s32 data);

static void _icb_box_data_clear(void);
static void _icb_initialize(void);
static void _icb_idle_msg_proc(void);
static void _icb_busy_msg_proc(void);
static void _icb_idle_proc(void);
static void _icb_busy_proc(void);
static void _icb_init_req_proc(void);

static void _icb_fram_write_rsp_proc(void);
static void _icb_seq_complete(void);
static void _icb_set_alarm(u32 alarm_code);
void _icb_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _icb_system_error(u8 fatal_err, u8 code);



/************************** EXTERN FUNCTIONS *************************/

// ICB data read / write message
// iVIZION function : static void send_ICBrequest_packet(u8 id)
#if !defined(UBA_RTQ) //現状RTQは使用していない、将来使用するかも
void set_rfid_write_buffer(int num) //not use RTQ
{
	u32 size;
	u8	*sptr;

#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) //2023-04-18
	#if 1//#if (_RFID_BACK_DATA28==1)
	if(num >= 1 && num <= 18)
	#else
	if(num >= 1 && num <= 31)
	#endif
#else
	if(num >= 1 && num <= 31)
#endif
	{
		sptr = send_blk_info[num - 1].buffer;
		size = send_blk_info[num - 1].size;
#if defined(UBA_RTQ)
	memcpy(&icb_rc_rfid_write_buffer, sptr, size);
#else
	#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) //2023-04-18
		memcpy(&ex_rfid_2k_write_data_bufer, sptr, size);
	#else
		memcpy(&ex_ICB_rfid_write_data_bufer, sptr, size);
	#endif
#endif // UBA_RTQ
		
	}
}

//現状RTQは使用していない、将来使用するかも
// 指定BLKデータの読み出し及び書き込みデータとの比較処理
// iVIZION function : static int icb_read_and_verify_each_data(int num)
int icb_read_and_verify(int num)
{
	u8	*sptr;
	u32 size;
	int result = TRUE;

#if 1//#if (_RFID_BACK_DATA28==1) (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) //2023-04-18
	if(num >= 1 && num <= 18)
#else
	if(num >= 1 && num <= 31)
#endif
	{
		sptr = send_blk_info[num - 1].buffer;
		size = send_blk_info[num - 1].size;
#if defined(UBA_RTQ)
		if (0 != memcmp(&icb_rc_rfid_read_buffer, sptr, size))
#else
		/*	データ確認	*/
	#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
		if(0 != memcmp(&ex_rfid_2k_read_data_bufer, sptr, size))
	#else
		if(0 != memcmp(&ex_ICB_rfid_read_data_bufer, sptr, size))
	#endif
#endif // UBA_RTQ
		{
			result = FALSE;
		}
	}
	return result;
}

void icb_rfid_read_start(void) //将来RTQでも使用するかも
{
	u32 address;
	u32 size;

	set_rfid_write_buffer(ex_ICBrecovery[ex_ICBsend_num].BLK);
	address = send_blk_info[ex_ICBrecovery[ex_ICBsend_num].BLK - 1].address;
	size = send_blk_info[ex_ICBrecovery[ex_ICBsend_num].BLK - 1].size;

#if defined(ICB_LOG)
	s_icb_watch[ICB_LOG_INDEX].seq = ex_icb_task_seq;
	s_icb_watch[ICB_LOG_INDEX].ope = 2;
	s_icb_watch[ICB_LOG_INDEX].address = address;
	s_icb_watch[ICB_LOG_INDEX].size = size;
	s_icb_watch[ICB_LOG_INDEX].start = OSW_TIM_value();
#endif

#if defined(UBA_RTQ)
	rc_rfid_read(address, size, (u8*)&icb_rc_rfid_read_buffer[0]);
#else
	_icb_send_msg(ID_RFID_MBX, TMSG_RFID_READ_REQ, RFID_ICB, address, size, 0);
#endif
}

void icb_rfid_write_start(void)
{
	u32 address;
	u32 size;

	set_rfid_write_buffer(ex_ICBrecovery[ex_ICBsend_num].BLK);
	address = send_blk_info[ex_ICBrecovery[ex_ICBsend_num].BLK - 1].address;
	size = send_blk_info[ex_ICBrecovery[ex_ICBsend_num].BLK - 1].size;

#if defined(ICB_LOG)
	s_icb_watch[ICB_LOG_INDEX].seq = ex_icb_task_seq;
	s_icb_watch[ICB_LOG_INDEX].ope = 1;
	s_icb_watch[ICB_LOG_INDEX].address = address;
	s_icb_watch[ICB_LOG_INDEX].size = size;
	s_icb_watch[ICB_LOG_INDEX].start = OSW_TIM_value();
#endif

#if defined(UBA_RTQ)
	rc_rfid_write(address, size, (u8*)&icb_rc_rfid_write_buffer[0]);
#else
	_icb_send_msg(ID_RFID_MBX, TMSG_RFID_WRITE_REQ, RFID_ICB, address, size, 0);
#endif
}

#endif //end ! UBA_RTQ

#if defined(UBA_RTQ)
void icb_rc_rfid_read(u16 icbAdrr, u16 lenght, u8 *data)
{
	rc_rfid_read(icbAdrr, lenght, data);
}
#endif

/*******************************
        icb_task
 *******************************/
void icb_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	_icb_initialize();						/* icb task initialize */

	while (1)
	{
		/* 待機中(idle) */
		if((ex_icb_task_seq & 0xFF00) == ICB_SEQ_IDLE)
		{
			ercd = trcv_mbx(ID_ICB_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
			if (ercd == E_OK)
			{
				memcpy(&icb_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(icb_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_icb_system_error(1, 3);
				}
				_icb_idle_proc();	/* 特になにもしない	*/
				_icb_idle_msg_proc();	/* キューの移動 NEW_ICBの場合、場合によっては、_icb_idle_msg_queue_exec();を呼び出してシーケンス番号移動	*/
			}
			else
			{
				_icb_idle_proc();	/* 特になにもしない	*/
			}
		}
		/* 動作中(busy) */
		else
		{
			//ercd = trcv_mbx(ID_ICB_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
			ercd = prcv_mbx(ID_ICB_MBX, (T_MSG **)&tmsg_pt);
			if (ercd == E_OK)
			{
				memcpy(&icb_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(icb_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_icb_system_error(1, 5);
				}
				_icb_busy_msg_proc();
			}
			else
			{
				_icb_busy_proc();
			}
		}
	}
}

/*********************************************************************//**
 * @brief MBX message from MAIN task procedure
 *  ICB task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_idle_main_msg_proc(void)
{
	switch (icb_msg.tmsg_code)
	{
	case TMSG_ICB_INITIAL_REQ:			/* INITIAL message */
		_icb_init_req_proc();
		break;
	#if !defined(UBA_RTQ)
		case TMSG_ICB_ACCEPT_REQ:
		_icb_accept_req_proc();
		break;
	case TMSG_ICB_REJECT_REQ:
		_icb_reject_req_proc();
		break;
	case TMSG_ICB_REJECT_TICKET_REQ:
		_icb_reject_ticket_req_proc();
		break;
	case TMSG_ICB_ERROR_CODE_REQ:
		_icb_error_code_req_proc();
		break;
	case TMSG_ICB_SET_TIME_REQ:
		_icb_set_time_req_proc();
		break;	
	#else
		case TMSG_ICB_ACCEPT_RTQ_REQ:
		_icb_accept_rtq_req_proc();
		break;
	#endif

	default:					/* other */
		/* system error ? */
		_icb_system_error(0, 4);
		break;
	}
}

/*********************************************************************//**
 * @brief MBX message from MAIN task procedure
 *  ICB task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_idle_dline_msg_proc(void)
{
	switch (icb_msg.tmsg_code)
	{
	default:					/* other */
		/* system error ? */
		_icb_system_error(0, 4);
		break;
	}
}
/*********************************************************************//**
 * @brief MBX message from RFID task procedure
 *  ICB task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_idle_rfid_msg_proc(void)
{
	// nothing todo
	switch (icb_msg.tmsg_code)
	{
	case TMSG_RFID_STATUS_INFO:
		break;
	case TMSG_RFID_READ_RSP:
		break;
	case TMSG_RFID_WRITE_RSP:
		break;
	case TMSG_RFID_RESET_RSP:
		break;
	default:					/* other */
		/* system error ? */
		_icb_system_error(0, 4);
		break;
	}
}

/*********************************************************************//**
 * @brief MBX message procedure
 *  ICB task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_idle_msg_proc(void)
{
	if(icb_msg.sender_id == ID_MAIN_TASK)
	{
		/* Request from MAIN Task */
		_icb_idle_main_msg_proc();
	}
	else if((icb_msg.sender_id == ID_DLINE_TASK) || (icb_msg.sender_id == ID_CLINE_TASK))
	{
		/* Request from CLINE/DLINE Task */
		_icb_idle_dline_msg_proc();
	}
	else if(icb_msg.sender_id == ID_RFID_TASK)
	{
		/* Response from RFID Task */
		_icb_idle_rfid_msg_proc();	//何もしていない
	}
	else
	{
		/* system error ? */
		_icb_system_error(0, 8);
	}
}

/*********************************************************************//**
 * @brief MBX message from MAIN task procedure
 *  ICB task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_busy_main_msg_proc(void)
{
	switch (icb_msg.tmsg_code)
	{
	case TMSG_ICB_INITIAL_REQ:			/* INITIAL message */
		s_icb_req_task_id = icb_msg.sender_id;
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);
		break;
	case TMSG_ICB_ACCEPT_REQ:
		s_icb_req_task_id = icb_msg.sender_id;
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);
		break;
	case TMSG_ICB_REJECT_REQ:
		s_icb_req_task_id = icb_msg.sender_id;
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);
		break;
	case TMSG_ICB_REJECT_TICKET_REQ:
		s_icb_req_task_id = icb_msg.sender_id;
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);
		break;
	case TMSG_ICB_ERROR_CODE_REQ:
		s_icb_req_task_id = icb_msg.sender_id;
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);
		break;
	case TMSG_ICB_SET_TIME_REQ:
		s_icb_req_task_id = icb_msg.sender_id;
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);
		break;
	default:					/* other */
		/* system error ? */
		_icb_system_error(0, 4);
		break;
	}
}
/*********************************************************************//**
 * @brief MBX message from RFID task procedure
 *  ICB task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if !defined(UBA_RTQ)
static void _icb_busy_rfid_msg_proc(void)
{
	switch (icb_msg.tmsg_code)
	{
	case TMSG_RFID_STATUS_INFO:
		_icb_rfid_status_info_proc(); //Idleに強制的に戻しているだけ
		break;
	case TMSG_RFID_READ_RSP:
		_icb_rfid_read_rsp_proc();
		break;
	case TMSG_RFID_WRITE_RSP:
		_icb_rfid_write_rsp_proc();
		break;
	case TMSG_RFID_RESET_RSP:
		_icb_rfid_init_rsp_proc();
		break;
	default:					/* other */
		/* system error ? */
		_icb_system_error(0, 4);
		break;
	}
}
#endif
/*********************************************************************//**
 * @brief MBX message from FRAM task procedure
 *  ICB task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_busy_fram_msg_proc(void)
{
	switch (icb_msg.tmsg_code)
	{
	case TMSG_FRAM_WRITE_RSP:	//use rtq
		_icb_fram_write_rsp_proc();
		break;
	default:					/* other */
		/* system error ? */
		_icb_system_error(0, 4);
		break;
	}
}

#if defined(UBA_RTQ)
static void _icb_busy_rc_rfid_msg_proc(void)
{
	switch (icb_msg.tmsg_code)
	{
	case TMSG_RC_RFID_RESET_RSP:
		_icb_rc_rfid_reset_rsp_proc();
		break;
	case TMSG_RC_RFID_READ_RSP:
		_icb_rc_rfid_read_rsp_proc();
		break;
	case TMSG_RC_RFID_WRITE_RSP:
		_icb_rc_rfid_write_rsp_proc();
		break;
	default:
		break;
	}
}
#endif
/*********************************************************************//**
 * @brief icb task initialize procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_box_data_clear(void)
{
	/*	All 0	*/
	fill_memo32(0, (u32 *)&Smrtdat, sizeof(Smrtdat) / 4);
	fill_memo32(0, (u32 *)&Smrtdat2, sizeof(Smrtdat2) / 4);
}
/*********************************************************************//**
 * @brief icb task initialize procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_initialize(void)
{
	s_icb_alarm_code = 0;
	s_icb_alarm_retry = 0;
	s_icb_reject_code = 0;
	s_icb_log_denomi_code = 0;
	s_icb_log_reject_code = 0;
	s_icb_log_error_code = 0;
	s_icb_retry_count = 0;
	s_icb_req_task_id = 0;
	_icb_box_data_clear();
}
/*********************************************************************//**
 * @brief icb task idle procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_idle_proc(void)
{
	// Nothing todo
}
/*********************************************************************//**
 * @brief icb task busy procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_busy_proc(void)
{
	u32 flag = 0;
	u32 data = 0;
	ER ercd;
	u32 rtn;

	//ercd = prcv_dtq(ID_ICB_DTQ, (VP_INT *)&rtn);
	ercd = trcv_dtq(ID_ICB_DTQ, (VP_INT *)&rtn, 1);
	if (ercd == E_OK)
	{
		switch(rtn)
		{
		case ICB_LISTEN_TMOUT:
			flag = EVT_ICB_TIMEOUT;
			break;
		case ICB_READ_SUCCESS:
			flag = EVT_ICB_DATA_READ;
			break;
		case ICB_READ_FAIL:
			flag = EVT_ICB_DATA_READ_FAIL;
			break;
		case ICB_WRITE_SUCCESS:
			flag = EVT_ICB_DATA_WRITE;
			break;
		case ICB_WRITE_FAIL:
			flag = EVT_ICB_DATA_WRITE_FAIL;
			break;
		//case FRAM_READ_SUCCESS:
		//	flag = EVT_FRAM_DATA_READ;
		//	break;
		//case FRAM_READ_FAIL:
		//	flag = EVT_FRAM_DATA_READ_FAIL;
		//	break;
		case FRAM_WRITE_SUCCESS:
			flag = EVT_FRAM_DATA_WRITE;
			break;
		case FRAM_WRITE_FAIL:
			flag = EVT_FRAM_DATA_WRITE_FAIL;
			break;
		default:
			flag = 0;
			break;
		}
	}
	else if (ercd == E_TMOUT)
	{
	}
	else
	{
		/* system error ? */
		_icb_system_error(1, 12);
	}
	// Nothing todo
	switch (ex_icb_task_seq & 0xFF00)
	{
#if !defined(UBA_RTQ)
	case ICB_SEQ_INITIAL:			// 0x0100
		_icb_initial_seq_proc(flag, data);
		break;
	case ICB_SEQ_ACCEPT:			// 0x0300
		_icb_accept_seq_proc(flag, data);
		break;
	case ICB_SEQ_REJECT:			// 0x0400
		_icb_reject_seq_proc(flag, data);
		break;
	case ICB_SEQ_REJECT_TICKET:		// 0x0500
		_icb_reject_ticket_seq_proc(flag, data);
		break;
	case ICB_SEQ_ERROR_CODE:		// 0x0600
		_icb_err_code_seq_proc(flag, data);
		break;
	case ICB_SEQ_SET_TIME:		// 0x0800
		_icb_set_time_seq_proc(flag, data);
		break;
#else
	case ICB_SEQ_INITIAL:			// 0x0100
		_icb_initial_seq_proc_rtq(flag, data);
		break;
	case ICB_SEQ_ACCEPT_RTQ:			// 0x0B00
		_icb_accept_rtq_seq_proc(flag, data);
		break;	
#endif
	default:
		/* system error ? */
		_icb_system_error(0, 6);
		break;
	}
}




/*********************************************************************//**
 * @brief MBX message procedure
 *  icb task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_busy_msg_proc(void)
{
	if(icb_msg.sender_id == ID_MAIN_TASK)
	{
		/* Request from MAIN Task */
		_icb_busy_main_msg_proc();
	}
	else if(icb_msg.sender_id == ID_RFID_TASK)
	{
		/* Response from RFID Task */
	#if !defined(UBA_RTQ)
		_icb_busy_rfid_msg_proc();
	#endif
	}
	else if(icb_msg.sender_id == ID_FRAM_TASK)
	{
		/* Response from FRAM Task */
		_icb_busy_fram_msg_proc();
	}
#if defined(UBA_RTQ)
	else if (icb_msg.sender_id == ID_RC_TASK)
	{
		/* Response from FRAM Task */
		_icb_busy_rc_rfid_msg_proc();
	}
#endif
	else
	{
		/* system error ? */
		_icb_system_error(0, 7);
	}
}
/*********************************************************************//**
 * initial movement
 **********************************************************************/
/*********************************************************************//**
 * @brief process of ICB initialize message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_init_req_proc(void)
{
	s_icb_req_task_id = icb_msg.sender_id;
	_icb_set_seq(ICB_SEQ_INITIAL, ICB_SEQ_TIMEOUT);
}

/*********************************************************************//**
 * @brief icb control interrupt procedure (initialize sequence)
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
#if !defined(UBA_RTQ)
static void _icb_initial_seq_proc(u32 flag, s32 data)// ok
{
	switch (ex_icb_task_seq & 0x00FF)
	{
		// read data
	case 0x00:
		_icb_initial_0100_seq(flag, data);
		break;
	case 0x01:
		_icb_initial_0101_seq(flag, data);
		break;
		// recovery
	case 0x11:
		_icb_initial_0111_seq(flag, data);
		break;
	case 0x12:
		_icb_initial_0112_seq(flag, data);
		break;
	case 0x13:
		_icb_initial_0113_seq(flag, data);
		break;
		// data check
	case 0x21:
		_icb_initial_0121_seq(flag, data);
		break;
		// initialize data
	case 0x31:
		_icb_initial_0131_seq(flag, data);
		break;
	case 0x32:
		_icb_initial_0132_seq(flag, data);
		break;
	case 0x33:
		_icb_initial_0133_seq(flag, data);
		break;
	case 0x34:
		_icb_initial_0134_seq(flag, data);
		break;
		// fram backup
	case 0x40:
		_icb_fram_backup_seq(flag, data);
		break;
	/* RC RFID */
	default:								/* other */
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);

		/* system error ? */
		_icb_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0100
 *  ICB_SUB_INITIAL_BACKUP_STATUS
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0100_seq(u32 flag, s32 data)
{
	s_icb_watch_index = 0;
	s_icb_watch[ICB_LOG_INDEX].seq = ex_icb_task_seq;
	s_icb_watch[ICB_LOG_INDEX].ope = 0;
	s_icb_watch[ICB_LOG_INDEX].address = 0;
	s_icb_watch[ICB_LOG_INDEX].size = 232;
	s_icb_watch[ICB_LOG_INDEX].start = OSW_TIM_value();
	_icb_send_msg(ID_RFID_MBX, TMSG_RFID_RESET_REQ, 0, 0, 0, 0);
	// 0x0101モード（ICBデータリード待ち）移行
	_icb_set_seq( 0x0101, ICB_SEQ_TIMEOUT);
}


/*********************************************************************//**
 * @brief icb control sequence 0x0101
 *  ICB_SUB_INITIAL_UID_READ_DATA & ICB_SUB_INITIAL_READ_DATA
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0101_seq(u32 flag, s32 data)
{
	if(IS_ICB_EVENT_READ_FAIL(flag))
	{
		/* ここに入る事はない */
		/* RFIDの通信エラーは、直接mianタスクへ送信しているようなので、*/
		_icb_set_alarm(icb_msg.arg2);
	}
	else if(IS_ICB_EVENT_READ_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		/*	ICB functionがDisableのときは終了	*/
		if(!is_icb_enable())
		{
		#if 1 
		//2023-11-09
			//ICB無効なのに、ICB通信できた事によるエラー
			_icb_set_alarm(ALARM_CODE_RFID_ICB_SETTING);
		#else
			_icb_seq_complete();
		#endif
		}
		else if(is_box_flag_inhibit()) //use SS and RTQ 使用するか未定だが残す
		{
			/*	SYSTEM INHIBITフラグ(INHIBITフラグセットされたBOX)　*/
			_icb_set_alarm(ALARM_CODE_RFID_ICB_SETTING);
		}
		/* Add Clear request flg	*/
		else if((is_box_flag_no_data()) || (is_box_flag_initial_request()))
		{
			// ICB_SEQ_INITIAL_DATA_WRITEモード（ICBデータ初期化）移行
			_icb_set_seq( ICB_SEQ_INITIAL_DATA_WRITE, ICB_SEQ_TIMEOUT); //0x0131
		}
		else if(is_box_flag_read())
		{
			/*	集計済みBoxのためError	*/
			/*	End process	*/
			_icb_set_alarm(ALARM_CODE_RFID_ICB_NOT_INITIALIZE);
		}
		else
		{
			/*	初期化及び動作中BOXの場合は、Recovery処理へ移行	*/
			_icb_set_seq( ICB_SEQ_INITIAL_RECOVERY, ICB_SEQ_TIMEOUT);
		}
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0111
 *  ICB_SUB_INITIAL_DATA_RECOVERY,
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0111_seq(u32 flag, s32 data)
{
	if((icb_machine_number_is_valid() == FALSE)
		&& (check_ICBflag() == TRUE))
	{
		/* ICB有効設定で、マシン番号が壊れている*/
		/*	ICB Machine Number Error */
		_icb_set_alarm(ALARM_CODE_RFID_ICB_MC_INVALID);
	}
	else if(ALARM_CODE_RFID_ICB_NUMBER_MISMATCH == icb_check_machineNo())
	{
		/*	ICB Machine Number Mismatch */
		_icb_set_alarm(ALARM_CODE_RFID_ICB_NUMBER_MISMATCH);
	}
	/*	check recovery data	*/
	else if(TRUE == icb_check_recovery_data())
	{
		/* Non Recovery data	*/
		/*	go to Next process			*/
		/*	初期化及び動作中BOXの場合は、データチェック
		 * 処理へ移行	*/
		_icb_set_seq( ICB_SEQ_INITIAL_DATA_CHECK, ICB_SEQ_TIMEOUT);
	}
	else
	{
		icb_rfid_write_start();
		/*	set write recovery data	*/
		_icb_set_seq( 0x0112, ICB_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0112
 * ICB_SUB_INITIAL_DATA_RECOVERY_VERIFY
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0112_seq(u32 flag, s32 data)
{
	if(IS_ICB_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_WRITE_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		icb_rfid_read_start();
		/* verify data read	*/
		_icb_set_seq( 0x0113, ICB_SEQ_TIMEOUT);
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}



/*********************************************************************//**
 * @brief icb control sequence 0x0113
 * ICB_SUB_INITIAL_DATA_RECOVERY_VERIFY
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0113_seq(u32 flag, s32 data)
{
	int result;

	if(IS_ICB_EVENT_READ_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_READ_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		result = icb_read_and_verify(ex_ICBrecovery[ex_ICBsend_num].BLK);
		if(result)
		{
			/*	Check Next Data	*/
			renewal_ICBsend_flag();
			if(TRUE == icb_check_recovery_data())
			{
				/* Non Recovery data	*/
				/*	go to Next process			*/
				/*	初期化及び動作中BOXの場合は、データチェック
				 * 処理へ移行	*/
				_icb_set_seq( ICB_SEQ_INITIAL_DATA_CHECK, ICB_SEQ_TIMEOUT);
			}
			else
			{
				icb_rfid_write_start();
				/*	set write recovery data	*/
				_icb_set_seq( 0x0112, ICB_SEQ_TIMEOUT);
			}
		}
		else
		{
			_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
		}
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}

/*********************************************************************//**
 * @brief icb control sequence 0x0121
 *  ICB_SUB_INITIAL_DATA_CHECK (check DATA from FRAM)
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0121_seq(u32 flag, s32 data)
{
	if(is_box_flag_data_exist())
	{
		/*<< 集計データありBoxの場合 >>*/
		/*<< 集計ありBoxの場合イニシャル書き込み中として扱わない >>*/
		ex_Info_ICBrecovery &= ~BIT_INITIAL_DATA_WRITING;
		if(is_icb_checksum_error())
		{
		#if 0//#if defined(UBA_RTQ)
			_icb_set_seq(0x0160, ICB_SEQ_TIMEOUT);
		#else
			/*	チェックサムエラー	*/
			_icb_set_alarm(ALARM_CODE_RFID_ICB_DATA);
		#endif 	
		}
		/*	During use BOX	*/
		else
		{
			ex_rtc_clock = Convert32bitToTimeUnix(Smrtdat.initim);

			/*	End process	*/
			_icb_seq_complete();
		}
	}
	else
	{
		/*<< 初期化済みBoxの場合 >>*/
		/*	Initialization BOX	*/
		/* ｲﾆｼｬﾙﾃﾞｰﾀ書き込み中のHard Reset対策	 */
		if((ex_Info_ICBrecovery & BIT_INITIAL_DATA_WRITING) == BIT_INITIAL_DATA_WRITING)
		{
			// ICB_SEQ_INITIAL_DATA_WRITEモード（ICBデータ初期化）移行
			_icb_set_seq( ICB_SEQ_INITIAL_DATA_WRITE, ICB_SEQ_TIMEOUT); //0x0131
		}
		else if(is_icb_checksum_error())
		{
		#if 0//#if defined(UBA_RTQ)
			_icb_set_seq(0x0160, ICB_SEQ_TIMEOUT);
		#else
			/*	チェックサムエラー	*/
			_icb_set_alarm(ALARM_CODE_RFID_ICB_DATA);
		#endif 	
		}
		/*	During use BOX	*/
		else
		{
			ex_rtc_clock = Convert32bitToTimeUnix(Smrtdat.initim);

			/*	End process	*/
			_icb_seq_complete();
		}
	}
}

/*********************************************************************//**
 * @brief icb control sequence 0x0131
 *  ICB_SUB_INITIAL_SET_DATA
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0131_seq(u32 flag, s32 data)
{
	/* Add Clear request flg	*/
	if((is_box_flag_no_data()) || (is_box_flag_initial_request()))
	{
		/*
		 * iVIZION erases the cash box and reach idle normally
		 * when check sum error and flag is 0(no data) or 4(initial request)
		 */
	/*--------------------------------------------------------*/
		u32 initTime;
		u8 boxNum[20];

		/* save the cashbox number and init time  */
		initTime = Smrtdat.initim;
		memcpy(&boxNum[0], &Smrtdat.boxno[0], 20);

		/*	All 0	*/
		fill_memo32(0, (u32 *)&Smrtdat, sizeof(Smrtdat) / 4);
		fill_memo32(0, (u32 *)&Smrtdat2, sizeof(Smrtdat2) / 4);

		/* restore the cashbox number and init time */
		Smrtdat.initim = initTime;				/*	'14-11-21	*/
		memcpy(&Smrtdat.boxno[0], &boxNum[0], 20);

		/*	clear recovery flag*/
		icb_clear_recovery_flag(ON);
		/*	Set Initial Info to ICB memory	*/
		icb_set_initial_info();				/* with Calculation of checksum	*/
	/*--------------------------------------------------------*/

		/*	Set the initialization request flags of all the data	*/
		ex_icb_state |= BIT_ICB_DATA_ALL_CLEAR;
		ex_icb_state &= ~BIT_ICB_DATA_SUM_ERROR;	/*	ICB	Data error			*/
		/**/
		/*	initialization request of all the data	*/
		icb_set_initial_all_data();

		// Box をFRAMに保存
		_icb_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_SETTING, 0, 0, 0);
		// 0x0134モード（RFAMデータライト待ち）移行
		_icb_set_seq( 0x0132, ICB_SEQ_TIMEOUT);
	}
	else
	{
		/*	clear ICB recovery flag */
		icb_clear_recovery_flag(OFF);
		icb_set_initial_info();
		/*	Set to send buffer and data BLK number of information	*/
		icb_send_buffer_initial_info();

		// Box をFRAMに保存
		_icb_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_SETTING, 0, 0, 0);
		// 0x0134モード（RFAMデータライト待ち）移行
		_icb_set_seq( 0x0132, ICB_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief icb control sequence 0x0132
 *  ICB_SUB_INITIAL_VERIFY
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0132_seq(u32 flag, s32 data) //FRAM書き込み完了待ち
{
	if(IS_FRAM_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_FRAM);
	}
	else if(IS_FRAM_EVENT_WRITE_SUCCESS(flag))
	{
		ex_Info_ICBrecovery |= BIT_INITIAL_DATA_WRITING;	/*	イニシャルデータ書込み動作中フラグ	*/
		_icb_fram_backup(0x0133);	//FRAM書き込み、0xXX40で待つ、RFID書き込み、0x0133で待つ
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// FRAMタスクを待つ (wait FRAM task)
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0133
 *  ICB_SUB_INITIAL_WRITE_DATA
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0133_seq(u32 flag, s32 data) //RFID書き込み待ち(FRAM書き込み->RFID書き込み)
{
	if(IS_ICB_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_WRITE_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		icb_rfid_read_start();
		/*	verify recovery data	*/
		_icb_set_seq( 0x0134, ICB_SEQ_TIMEOUT);
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0134
 *  ICB_SUB_INITIAL_VERIFY
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0134_seq(u32 flag, s32 data) //Read完了待ち
{
	int result;

	if(IS_ICB_EVENT_READ_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_READ_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		result = icb_read_and_verify(ex_ICBrecovery[ex_ICBsend_num].BLK);
		if(result)
		{
			/*	Check Next Data	*/
			renewal_ICBsend_flag();
			/*	check wait data		*/
			if( ex_ICBsend_num == ex_ICBsave_num
			 ||
			#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) //2023-04-18
				#if 1//#if (_RFID_BACK_DATA28==1)			
			  ( ex_ICBsend_num == 0 && ex_ICBsave_num == 17 )
				#else
			  ( ex_ICBsend_num == 0 && ex_ICBsave_num == 30 )
				#endif
			#else
			  ( ex_ICBsend_num == 0 && ex_ICBsave_num == 30 )
			#endif
			 ) //2023-02-15 maxおくる場合、上の関数でex_ICBsend_num = 0にされるので条件追加
			{
				/*	書込みBufferの初期化	*/
				icb_clear_recovery_flag(ON);
				// sync time 
				ex_rtc_clock = Convert32bitToTimeUnix(Smrtdat.initim);

				_icb_fram_backup(ICB_SEQ_IDLE);

				#if defined(UBA_RTQ_ICB)	//初期化済み BOXが接続されたので、Head側の情報を初期化してもいいかも 2025-08-06 大切
				memcpy(&Smrtdat_fram, &Smrtdat, sizeof(Smrtdat_fram));							//Box側の情報をHead側運用変数 Smrtdat_fram にコピー 基本ここのみ
				memset((u8 *)&Smrtdat_fram_bk_power, 0, sizeof(Smrtdat_fram_bk_power));			//初期化済みBOXなのでリカバリキャンセル
				_icb_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_RECOVER_RTQ, 0, 0, 0); //初期化済みBOXの情報にHead側も更新 FRAMバックアップ更新
				#endif
			
			}
			else
			{
				_icb_fram_backup(0x0133);
			}
		}
		else
		{
			_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
		}
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}

static void _icb_fram_backup(u32 next_seq) //ここ関数は各シーケンスの0xXX40へ遷移させているんで注意
{
	s_icb_watch[ICB_LOG_INDEX].seq = ex_icb_task_seq;
	s_icb_watch[ICB_LOG_INDEX].ope = 0x10;
	s_icb_watch[ICB_LOG_INDEX].address = 0;
	s_icb_watch[ICB_LOG_INDEX].size = 0;
	s_icb_watch[ICB_LOG_INDEX].start = OSW_TIM_value();
	s_icb_task_seq_next = next_seq;

	switch (ex_icb_task_seq & 0xFF00)
	{
	case ICB_SEQ_INITIAL:			// 0x0100
	case ICB_SEQ_ACCEPT:			// 0x0300
	case ICB_SEQ_REJECT:			// 0x0400
	case ICB_SEQ_REJECT_TICKET:		// 0x0500
	case ICB_SEQ_ERROR_CODE:		// 0x0600
	case ICB_SEQ_SET_TIME:			// 0x0800
		_icb_set_seq( (ex_icb_task_seq & 0xFF00) | 0x0040, ICB_SEQ_TIMEOUT); //0xXX40へ遷移させているんで注意
		break;
	default:
		/* system error ? */
		_icb_system_error(0, 6);
		break;
	}
	_icb_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_RECOVER, 0, 0, 0);
}
/*********************************************************************//**
 * @brief icb control sequence 0x0140
 *  FRAM BACKUP
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_fram_backup_seq(u32 flag, s32 data) //FRAM書き込み待ち
{
	if(IS_FRAM_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_FRAM);
	}
	else if(IS_FRAM_EVENT_WRITE_SUCCESS(flag))
	{
 		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		if(s_icb_task_seq_next == ICB_SEQ_IDLE)
		{
			_icb_seq_complete();
		}
		else
		{
			icb_rfid_write_start();
			_icb_set_seq( s_icb_task_seq_next, ICB_SEQ_TIMEOUT);
		}
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// FRAMタスクを待つ (wait FRAM task)
	}
}
#endif //end ! UBA_RTQ



/*********************************************************************//**
 * accept banknote movement
 **********************************************************************/
/*********************************************************************//**
 * @brief icb control interrupt procedure (accept banknote sequence)
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
#if !defined(UBA_RTQ_ICB)
static void _icb_accept_seq_proc(u32 flag, s32 data)// ok
{
	switch (ex_icb_task_seq & 0x00FF)
	{
		// write data
	case 0x00:
		_icb_accept_0300_seq(flag, data);
		break;
	case 0x01:
		_icb_accept_0301_seq(flag, data);
		break;
	case 0x02:
		_icb_accept_0302_seq(flag, data);
		break;
		// fram backup
	case 0x40:
		_icb_fram_backup_seq(flag, data);
		break;
	default:								/* other */
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);

		/* system error ? */
		_icb_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0300
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_accept_0300_seq(u32 flag, s32 data)
{
	int num;
	RTC_INFO rtc;

	if((is_box_flag_no_data()) || (is_box_flag_initial_request()))
	{
		_icb_seq_complete();
		return;
	}
	else if(is_box_flag_read())
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_NOT_INITIALIZE);
		return;
	}

	if(s_icb_log_denomi_code != BAR_INDX)
	{
		icb_totalInsert_counter();
		/*	set denomi/ticket counter	*/
		num = icb_update_denomi_counter(s_icb_log_denomi_code);
		(void)set_ICBrecovery_data(num);
		/*	set total counter Only Note	*/
		(void)set_ICBrecovery_data(icb_update_total_counter());
		(void)set_ICBrecovery_data(BLK_INSERT_BILL_NUMBER);
	}
	else
	{
		ex_BAR_length[0] = ex_barcode_charactor_count;
		memcpy(ICBBarcode, ex_barcode, ex_barcode_charactor_count);
		/*	set ticket counter	*/
		num = icb_update_ticket_counter();
		(void)set_ICBrecovery_data(num);
		/*	set ticket number */
		(void)set_ICBrecovery_data(icb_update_ticket_number());

		memset((u8*)&ex_cline_status_tbl.ex_Barcode_recovery_icb[0], 0, sizeof(ex_cline_status_tbl.ex_Barcode_recovery_icb));
	}
	/* update time  */
	rtc = get_date_from_RTC();
	/*	Timer OK then copy to ICB buffer */
	Smrtdat.restim = ConvertTimeUnixTo32bit(rtc);
	(void)set_ICBrecovery_data(BLK_REM_TIME_NUMBER);
	if(Smrtdat.settim == 0)
	{
		Smrtdat.settim = Smrtdat.initim;
		(void)set_ICBrecovery_data(BLK_SET_TIME_NUMBER);
	}
	/*	culc check sum	*/
	Smrtdat.sum = (u8)culc_BLK1_checksum();
//#endif
	(void)set_ICBrecovery_data(BLK_SUM_NUMBER);

	_icb_fram_backup(0x0301);
}


/*********************************************************************//**
 * @brief icb control sequence 0x0301
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_accept_0301_seq(u32 flag, s32 data)
{
	if(IS_ICB_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_WRITE_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		icb_rfid_read_start();
		/*	verify recovery data	*/
		_icb_set_seq( 0x0302, ICB_SEQ_TIMEOUT);
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}

/*********************************************************************//**
 * @brief icb control sequence 0x0302
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_accept_0302_seq(u32 flag, s32 data)
{
	int result;
	if(IS_ICB_EVENT_READ_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_READ_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		result = icb_read_and_verify(ex_ICBrecovery[ex_ICBsend_num].BLK);
		if(result)
		{
			/*	Check Next Data	*/
			renewal_ICBsend_flag();
			/*	check wait data		*/
			if(ex_ICBsend_num == ex_ICBsave_num)
			{
				/*	書込みBufferの初期化	*/
				icb_clear_recovery_flag(ON);
				_icb_fram_backup(ICB_SEQ_IDLE);
			}
			else
			{
				icb_rfid_write_start();
				_icb_set_seq( 0x0301, ICB_SEQ_TIMEOUT);
			}
		}
		else
		{
			_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
		}
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}

/*********************************************************************//**
 * reject banknote movement
 **********************************************************************/
/*********************************************************************//**
 * @brief icb control interrupt procedure (reject banknote sequence)
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_reject_seq_proc(u32 flag, s32 data)// ok
{
	switch (ex_icb_task_seq & 0x00FF)
	{
		// write data
	case 0x00:
		_icb_reject_0400_seq(flag, data);
		break;
	case 0x01:
		_icb_reject_0401_seq(flag, data);
		break;
	case 0x02:
		_icb_reject_0402_seq(flag, data);
		break;
	case 0x40:
		_icb_fram_backup_seq(flag, data);
		break;
	default:								/* other */
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);

		/* system error ? */
		_icb_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0400
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_reject_0400_seq(u32 flag, s32 data)
{
	RTC_INFO rtc;

	if((is_box_flag_no_data()) || (is_box_flag_initial_request()))
	{
		_icb_seq_complete();
		return;
	}
	else if(is_box_flag_read())
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_NOT_INITIALIZE);
		return;
	}
	icb_totalInsert_counter();
	icb_update_reject_counter(s_icb_log_reject_code);
	(void)set_ICBrecovery_data(BLK_REJECT_NUMBER);
	(void)set_ICBrecovery_data(BLK_SUM2_NUMBER);
	(void)set_ICBrecovery_data(BLK_INSERT_BILL_NUMBER);
	/* update time  */
	rtc = get_date_from_RTC();
	/*	Timer OK then copy to ICB buffer */
	Smrtdat.restim = ConvertTimeUnixTo32bit(rtc);
	(void)set_ICBrecovery_data(BLK_REM_TIME_NUMBER);
	if(Smrtdat.settim == 0)
	{
		Smrtdat.settim = Smrtdat.initim;
		(void)set_ICBrecovery_data(BLK_SET_TIME_NUMBER);
	}
	/*	culc check sum	*/
	Smrtdat.sum = (u8)culc_BLK1_checksum();
	//#endif

	(void)set_ICBrecovery_data(BLK_SUM_NUMBER);

	_icb_fram_backup(0x0401);
}


/*********************************************************************//**
 * @brief icb control sequence 0x0401
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_reject_0401_seq(u32 flag, s32 data)
{
	if(IS_ICB_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_WRITE_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		icb_rfid_read_start();
		/*	verify recovery data	*/
		_icb_set_seq( 0x0402, ICB_SEQ_TIMEOUT);
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0402
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_reject_0402_seq(u32 flag, s32 data)
{
	int result;
	if(IS_ICB_EVENT_READ_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_READ_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		result = icb_read_and_verify(ex_ICBrecovery[ex_ICBsend_num].BLK);
		if(result)
		{
			/*	Check Next Data	*/
			renewal_ICBsend_flag();
			/*	check wait data		*/
			if(ex_ICBsend_num == ex_ICBsave_num)
			{
				/*	書込みBufferの初期化	*/
				icb_clear_recovery_flag(ON);
				_icb_fram_backup(ICB_SEQ_IDLE);
			}
			else
			{
				icb_rfid_write_start();
				_icb_set_seq( 0x0401, ICB_SEQ_TIMEOUT);
			}
		}
		else
		{
			_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
		}
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}
/*********************************************************************//**
 * reject ticket movement
 **********************************************************************/
/*********************************************************************//**
 * @brief icb control interrupt procedure (reject ticket sequence)
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_reject_ticket_seq_proc(u32 flag, s32 data)// ok
{
	switch (ex_icb_task_seq & 0x00FF)
	{
		// write data
	case 0x00:
		_icb_reject_ticket_0500_seq(flag, data);
		break;
	case 0x01:
		_icb_reject_ticket_0501_seq(flag, data);
		break;
	case 0x02:
		_icb_reject_ticket_0502_seq(flag, data);
		break;
	case 0x40:
		_icb_fram_backup_seq(flag, data);
		break;
	default:								/* other */
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);

		/* system error ? */
		_icb_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0500
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_reject_ticket_0500_seq(u32 flag, s32 data)
{
	RTC_INFO rtc;

	if((is_box_flag_no_data()) || (is_box_flag_initial_request()))
	{
		_icb_seq_complete();
		return;
	}
	else if(is_box_flag_read())
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_NOT_INITIALIZE);
		return;
	}
	icb_update_ticket_reject_counter(s_icb_log_reject_code);

	#if 1//#if (_RFID_BACK_DATA28==1) (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) //2023-04-18
	(void)set_ICBrecovery_data(BLK_TICKET_REJ_NUMBER1);
	(void)set_ICBrecovery_data(BLK_TICKET_REJ_NUMBER2);
	#else
	(void)set_ICBrecovery_data(BLK_TICKET_REJ_NUMBER);
	#endif

	(void)set_ICBrecovery_data(BLK_SUM2_NUMBER);
	/* update time  */
	rtc = get_date_from_RTC();
	/*	Timer OK then copy to ICB buffer */
	Smrtdat.restim = ConvertTimeUnixTo32bit(rtc);
	(void)set_ICBrecovery_data(BLK_REM_TIME_NUMBER);
	if(Smrtdat.settim == 0)
	{
		Smrtdat.settim = Smrtdat.initim;
		(void)set_ICBrecovery_data(BLK_SET_TIME_NUMBER);
	}
	/*	culc check sum	*/
	Smrtdat.sum = (u8)culc_BLK1_checksum();
	(void)set_ICBrecovery_data(BLK_SUM_NUMBER);
	//#endif

	_icb_fram_backup(0x0501);
}


/*********************************************************************//**
 * @brief icb control sequence 0x0501
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_reject_ticket_0501_seq(u32 flag, s32 data)
{
	if(IS_ICB_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_WRITE_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		icb_rfid_read_start();
		/*	verify recovery data	*/
		_icb_set_seq( 0x0502, ICB_SEQ_TIMEOUT);
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0502
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_reject_ticket_0502_seq(u32 flag, s32 data)
{
	int result;
	if(IS_ICB_EVENT_READ_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_READ_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		result = icb_read_and_verify(ex_ICBrecovery[ex_ICBsend_num].BLK);
		if(result)
		{
			/*	Check Next Data	*/
			renewal_ICBsend_flag();
			/*	check wait data		*/
			if(ex_ICBsend_num == ex_ICBsave_num)
			{
				/*	書込みBufferの初期化	*/
				icb_clear_recovery_flag(ON);
				_icb_fram_backup(ICB_SEQ_IDLE);
			}
			else
			{
				icb_rfid_write_start();
				_icb_set_seq( 0x0501, ICB_SEQ_TIMEOUT);
			}
		}
		else
		{
			_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
		}
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}
/*********************************************************************//**
 * error movement
 **********************************************************************/
/*********************************************************************//**
 * @brief icb control interrupt procedure (reject ticket sequence)
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_err_code_seq_proc(u32 flag, s32 data)// ok
{
	switch (ex_icb_task_seq & 0x00FF)
	{
		// write data
	case 0x00:
		_icb_errcode_0600_seq(flag, data);
		break;
	case 0x01:
		_icb_errcode_0601_seq(flag, data);
		break;
	case 0x02:
		_icb_errcode_0602_seq(flag, data);
		break;
	case 0x40:
		_icb_fram_backup_seq(flag, data);
		break;
	default:								/* other */
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);

		/* system error ? */
		_icb_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0600
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_errcode_0600_seq(u32 flag, s32 data)
{
	RTC_INFO rtc;

	int num;

	if((is_box_flag_no_data()) || (is_box_flag_initial_request()))
	{
		_icb_seq_complete();
		return;
	}
	else if(is_box_flag_read())
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_NOT_INITIALIZE);
		return;
	}

	num = icb_update_error_counter(s_icb_log_error_code);
	(void)set_ICBrecovery_data(num);
	/*	Set Time stamp	*/
	/* update time  */
	rtc = get_date_from_RTC();
	/*	Timer OK then copy to ICB buffer */
	Smrtdat.restim = ConvertTimeUnixTo32bit(rtc);
	(void)set_ICBrecovery_data(BLK_REM_TIME_NUMBER);
	if(Smrtdat.settim == 0)
	{
		Smrtdat.settim = Smrtdat.initim;
		(void)set_ICBrecovery_data(BLK_SET_TIME_NUMBER);
	}
	//#endif
	/*	culc check sum	*/
	Smrtdat.sum = (u8)culc_BLK1_checksum();
	(void)set_ICBrecovery_data(BLK_SUM_NUMBER);

	if(s_icb_log_error_code != 9)
	{
		_icb_fram_backup(0x0601);
	}
	else
	{
		/* ALARM_CODE_BOXはFRAMバックアップをして終了 */
		_icb_fram_backup(ICB_SEQ_IDLE);
		/*	clear 0	*/
		_icb_box_data_clear();
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0602
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_errcode_0601_seq(u32 flag, s32 data)
{
	if(IS_ICB_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_WRITE_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		icb_rfid_read_start();
		/*	verify recovery data	*/
		_icb_set_seq( 0x0602, ICB_SEQ_TIMEOUT);
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0602
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_errcode_0602_seq(u32 flag, s32 data)
{
	int result;
	if(IS_ICB_EVENT_READ_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_READ_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		result = icb_read_and_verify(ex_ICBrecovery[ex_ICBsend_num].BLK);
		if(result)
		{
			/*	Check Next Data	*/
			renewal_ICBsend_flag();
			/*	check wait data		*/
			if(ex_ICBsend_num == ex_ICBsave_num)
			{
				/*	書込みBufferの初期化	*/
				icb_clear_recovery_flag(ON);
				_icb_fram_backup(ICB_SEQ_IDLE);
			}
			else
			{
				icb_rfid_write_start();
				_icb_set_seq( 0x0601, ICB_SEQ_TIMEOUT);
			}
		}
		else
		{
			_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
		}
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}


/*********************************************************************//**
 * reject total in movement
 **********************************************************************/
/*********************************************************************//**
 * @brief icb control interrupt procedure (set time in sequence)
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_set_time_seq_proc(u32 flag, s32 data)// ok
{
	switch (ex_icb_task_seq & 0x00FF)
	{
		// write data
	case 0x00:
		_icb_set_time_0800_seq(flag, data);
		break;
	case 0x01:
		_icb_set_time_0801_seq(flag, data);
		break;
	case 0x02:
		_icb_set_time_0802_seq(flag, data);
		break;
	case 0x40:
		_icb_fram_backup_seq(flag, data);
		break;
	default:								/* other */
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);

		/* system error ? */
		_icb_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0800
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_set_time_0800_seq(u32 flag, s32 data)
{
	RTC_INFO rtc;

	if((is_box_flag_no_data()) || (is_box_flag_initial_request()))
	{
		_icb_seq_complete();
		return;
	}
	else if(is_box_flag_read())
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_NOT_INITIALIZE);
		return;
	}

	/*	Get Timer from RTC	*/
	rtc = get_date_from_RTC();
	/*	Timer OK then copy to ICB buffer */
	Smrtdat.restim = ConvertTimeUnixTo32bit(rtc);
	(void)set_ICBrecovery_data(BLK_REM_TIME_NUMBER);
	if(Smrtdat.settim == 0)
	{
		Smrtdat.settim = Smrtdat.initim;
		(void)set_ICBrecovery_data(BLK_SET_TIME_NUMBER);
	}
	/*	culc check sum	*/
	Smrtdat.sum = (u8)culc_BLK1_checksum();
	(void)set_ICBrecovery_data(BLK_SUM_NUMBER);

	_icb_fram_backup(0x0801);
}


/*********************************************************************//**
 * @brief icb control sequence 0x0801
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_set_time_0801_seq(u32 flag, s32 data)
{
	if(IS_ICB_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_WRITE_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		icb_rfid_read_start();
		/*	verify recovery data	*/
		_icb_set_seq( 0x0802, ICB_SEQ_TIMEOUT);
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0802
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_set_time_0802_seq(u32 flag, s32 data)
{
	int result;
	if(IS_ICB_EVENT_READ_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_READ_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		result = icb_read_and_verify(ex_ICBrecovery[ex_ICBsend_num].BLK);
		if(result)
		{
			/*	Check Next Data	*/
			renewal_ICBsend_flag();
			/*	check wait data		*/
			if(ex_ICBsend_num == ex_ICBsave_num)
			{
				/*	書込みBufferの初期化	*/
				icb_clear_recovery_flag(ON);
				_icb_fram_backup(ICB_SEQ_IDLE);
			}
			else
			{
				icb_rfid_write_start();
				_icb_set_seq( 0x0801, ICB_SEQ_TIMEOUT);
			}
		}
		else
		{
			_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
		}
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}


static void _icb_set_time_req_proc(void)
{
	s_icb_req_task_id = icb_msg.sender_id;
	_icb_set_seq(ICB_SEQ_SET_TIME, ICB_SEQ_TIMEOUT);
}

/*********************************************************************//**
 * @brief process of ICB accept message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_accept_req_proc(void)
{
	s_icb_req_task_id = icb_msg.sender_id;
	s_icb_log_denomi_code = icb_msg.arg1;
	s_icb_log_reject_code = icb_msg.arg2;
	_icb_set_seq(ICB_SEQ_ACCEPT, ICB_SEQ_TIMEOUT);
}

/*********************************************************************//**
 * @brief process of ICB reject message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_reject_req_proc(void)
{
	s_icb_req_task_id = icb_msg.sender_id;
	s_icb_log_denomi_code = icb_msg.arg1;
	s_icb_log_reject_code = icb_msg.arg2;
	_icb_set_seq(ICB_SEQ_REJECT, ICB_SEQ_TIMEOUT);
}

/*********************************************************************//**
 * @brief process of ICB ticket reject message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_reject_ticket_req_proc(void)
{
	s_icb_req_task_id = icb_msg.sender_id;
	s_icb_log_denomi_code = icb_msg.arg1;
	s_icb_log_reject_code = icb_msg.arg2;
	_icb_set_seq(ICB_SEQ_REJECT_TICKET, ICB_SEQ_TIMEOUT);
}

/*********************************************************************//**
 * @brief process of ICB error message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_error_code_req_proc(void)
{
	s_icb_req_task_id = icb_msg.sender_id;
	s_icb_log_error_code = (icb_msg.arg1 - 1);
	_icb_set_seq(ICB_SEQ_ERROR_CODE, ICB_SEQ_TIMEOUT);
}

/*********************************************************************//**
 * @brief process of RFID status info response
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_rfid_status_info_proc(void)
{
	switch (icb_msg.arg1)
	{
	case TMSG_SUB_SUCCESS:
		break;
	case TMSG_SUB_ALARM:
		break;
	default:
		/* system error */
		_icb_system_error(1, 5);
		break;
	}

	_icb_set_seq(ICB_SEQ_IDLE, 0);
}
/*********************************************************************//**
 * @brief process of RFID read response
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_rfid_read_rsp_proc(void)
{
	ER ercd;
	VP_INT send_dtq_data;

	switch (icb_msg.arg1)
	{
	case TMSG_SUB_SUCCESS:
		send_dtq_data = (VP_INT)ICB_READ_SUCCESS;
		ercd = snd_dtq(ID_ICB_DTQ, send_dtq_data);
		if(ercd != E_OK)
		{
			/* system error */
			_icb_system_error(1, 5);
		}
		break;
	case TMSG_SUB_ALARM:
		_icb_set_alarm(icb_msg.arg2);
		break;
	default:
		/* system error */
		_icb_system_error(1, 5);
		break;
	}
}
/*********************************************************************//**
 * @brief process of RFID write response
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_rfid_write_rsp_proc(void)
{
	ER ercd;
	VP_INT send_dtq_data;

	switch (icb_msg.arg1)
	{
	case TMSG_SUB_SUCCESS:
		send_dtq_data = (VP_INT)ICB_WRITE_SUCCESS;
		ercd = snd_dtq(ID_ICB_DTQ, send_dtq_data);
		if(ercd != E_OK)
		{
			/* system error */
			_icb_system_error(1, 5);
		}
		break;
	case TMSG_SUB_ALARM:
		_icb_set_alarm(icb_msg.arg2);
		break;
	default:
		/* system error */
		_icb_system_error(1, 5);
		break;
	}
}
/*********************************************************************//**
 * @brief process of RFID initialize response
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_rfid_init_rsp_proc(void)
{
	ER ercd;
	VP_INT send_dtq_data;

	switch (icb_msg.arg1)
	{
	case TMSG_SUB_SUCCESS:

	#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) //2023-04-18
		memcpy(&Smrtdat, &ex_rfid_2k_read_data_bufer[ICB_DATA1_OFFSET], sizeof(Smrtdat));
		memcpy(&Smrtdat2, &ex_rfid_2k_read_data_bufer[ICB_DATA2_OFFSET], sizeof(Smrtdat2));
	#else
		memcpy(&Smrtdat, &ex_ICB_rfid_read_data_bufer[ICB_DATA1_OFFSET], sizeof(Smrtdat));
		memcpy(&Smrtdat2, &ex_ICB_rfid_read_data_bufer[ICB_DATA2_OFFSET], sizeof(Smrtdat2));
	#endif

		send_dtq_data = (VP_INT)ICB_READ_SUCCESS;
		ercd = snd_dtq(ID_ICB_DTQ, send_dtq_data);
		if(ercd != E_OK)
		{
			/* system error */
			_icb_system_error(1, 5);
		}
		break;
	case TMSG_SUB_ALARM:
		_icb_set_alarm(icb_msg.arg2);
		break;
	default:
		/* system error */
		_icb_system_error(1, 5);
		break;
	}

}

#endif
/*********************************************************************//**
 * @brief process of FRAM write response
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _icb_fram_write_rsp_proc(void) //SS and RTQ use
{
	ER ercd;
	VP_INT send_dtq_data;

	switch (icb_msg.arg1)
	{
	case TMSG_SUB_SUCCESS:
		send_dtq_data = (VP_INT)FRAM_WRITE_SUCCESS;
		ercd = snd_dtq(ID_ICB_DTQ, send_dtq_data);
		if(ercd != E_OK)
		{
			/* system error */
			_icb_system_error(1, 5);
		}
		break;
	case TMSG_SUB_ALARM:
		_icb_set_alarm(icb_msg.arg2);
		break;
	default:
		/* system error */
		_icb_system_error(1, 5);
		break;
	}
}




#if defined(UBA_RTQ)
static void _icb_rc_rfid_reset_rsp_proc()
{
	ER ercd;
	VP_INT send_dtq_data;

	switch (icb_msg.arg1)
	{
	case TMSG_SUB_SUCCESS:
		if (icb_msg.arg2 == 0) /* RESPONSE 0 ->> OK */
		{
			send_dtq_data = (VP_INT)ICB_READ_SUCCESS;
		}
		else/* RESPONSE  ! 0 ->> NG */
		{
			send_dtq_data = (VP_INT)ICB_READ_FAIL;
		}
		ercd = snd_dtq(ID_ICB_DTQ, send_dtq_data);
		if (ercd != E_OK)
		{
			/* system error */
			_icb_system_error(1, 5);
		}
		break;
	case TMSG_SUB_ALARM:
		_icb_set_alarm(icb_msg.arg2);
		break;
	default:
		/* system error */
		_icb_system_error(1, 5);
		break;
	}
}

static void _icb_rc_rfid_read_rsp_proc()
{
	ER ercd;
	VP_INT send_dtq_data;

	switch (icb_msg.arg1)
	{
	case TMSG_SUB_SUCCESS:
		if (icb_msg.arg2 == 0) /* RESPONSE 0 ->> OK */
		{
			send_dtq_data = (VP_INT)ICB_READ_SUCCESS;
		}
		else/* RESPONSE  ! 0 ->> NG */
		{
			send_dtq_data = (VP_INT)ICB_READ_FAIL;
		}

		ercd = snd_dtq(ID_ICB_DTQ, send_dtq_data);
		if(ercd != E_OK)
		{
			/* system error */
			_icb_system_error(1, 5);
		}
		break;
	case TMSG_SUB_ALARM:
		_icb_set_alarm(icb_msg.arg2);
		break;
	default:
		/* system error */
		_icb_system_error(1, 5);
		break;
	}
}

static void _icb_rc_rfid_write_rsp_proc()
{
	ER ercd;
	VP_INT send_dtq_data;

	switch (icb_msg.arg1)
	{
	case TMSG_SUB_SUCCESS:
		if (icb_msg.arg2 == 0) /* RESPONSE 0 ->> OK */
		{
			send_dtq_data = (VP_INT)ICB_WRITE_SUCCESS;
		}
		else/* RESPONSE  ! 0 ->> NG */
		{
			send_dtq_data = (VP_INT)ICB_WRITE_FAIL;
		}

		ercd = snd_dtq(ID_ICB_DTQ, send_dtq_data);
		if(ercd != E_OK)
		{
			/* system error */
			_icb_system_error(1, 5);
		}
		break;
	case TMSG_SUB_ALARM:
		_icb_set_alarm(icb_msg.arg2);
		break;
	default:
		/* system error */
		_icb_system_error(1, 5);
		break;
	}
}

#endif // UBA_RTQ
/*********************************************************************//**
 * @brief icb control sub function
 *  set icb sequence
 * @param[in]	sequence no.
 * 				time out
 * @return 		None
 **********************************************************************/
static void _icb_set_seq(u16 seq, u16 time_out)
{
	ex_icb_task_seq = seq;
	_ir_icb_ctrl_time_out = time_out;

	
#if defined(UBA_LOG)	//2025-08-26 log
	icb_log[icb_log_index] = ex_icb_task_seq;
	icb_log_index++;
	icb_log_index = icb_log_index % ICB_LOG_COUNT_UBA;
#endif

#ifdef _ENABLE_JDL
	jdl_add_trace(ID_ICB_TASK, ((ex_icb_task_seq >> 8) & 0xFF), (ex_icb_task_seq & 0xFF), s_icb_alarm_code, s_icb_alarm_retry, s_icb_reject_code);
#endif /* _ENABLE_JDL */

}


/*********************************************************************//**
 * @brief icb control sub function
 *  request complete response
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
static void _icb_seq_complete(void)
{
	u32 msg;

	s_icb_alarm_code = 0;
	s_icb_alarm_retry = 0;
	s_icb_reject_code = 0;
	s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
	s_icb_watch_index++;

	switch (ex_icb_task_seq & 0xFF00)
	{
	case ICB_SEQ_INITIAL:
		msg = TMSG_ICB_INITIAL_RSP;
		break;
	case ICB_SEQ_ACCEPT:
		msg = TMSG_ICB_ACCEPT_RSP;
		break;
#if defined(UBA_RTQ_ICB)
		case ICB_SEQ_ACCEPT_RTQ:
		msg = TMSG_ICB_ACCEPT_RTQ_RSP;
		break;
#endif
	case ICB_SEQ_REJECT:
		msg = TMSG_ICB_REJECT_RSP;
		break;
	case ICB_SEQ_REJECT_TICKET:
		msg = TMSG_ICB_REJECT_TICKET_RSP;
		break;
	case ICB_SEQ_ERROR_CODE:
		msg = TMSG_ICB_ERROR_CODE_RSP;
		break;
	case ICB_SEQ_SET_TIME:
		msg = TMSG_ICB_SET_TIME_RSP;
		break;
	default:
		break;
	}

	alt_cache_l1_data_purge_all();
	_kernel_synch_cache();

	_icb_send_msg(s_icb_req_task_id, msg, TMSG_SUB_SUCCESS, 0, 0, 0);
	s_icb_req_task_id = 0;
	_icb_set_seq(ICB_SEQ_IDLE, 0);
}


/*********************************************************************//**
 * @brief icb control sub function
 *  alarm response
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
static void _icb_set_alarm(u32 alarm_code)
{
	u32 msg;

	s_icb_alarm_code = alarm_code;

	switch (ex_icb_task_seq & 0xFF00)
	{
	case ICB_SEQ_INITIAL:
		msg = TMSG_ICB_INITIAL_RSP;
		break;
	case ICB_SEQ_ACCEPT:
		msg = TMSG_ICB_ACCEPT_RSP;
		break;

	#if defined(UBA_RTQ_ICB)
	case ICB_SEQ_ACCEPT_RTQ:
		msg = TMSG_ICB_ACCEPT_RTQ_RSP;
		break;
	#endif

	case ICB_SEQ_REJECT:
		msg = TMSG_ICB_REJECT_RSP;
		break;
	case ICB_SEQ_REJECT_TICKET:
		msg = TMSG_ICB_REJECT_TICKET_RSP;
		break;
	case ICB_SEQ_ERROR_CODE:
		msg = TMSG_ICB_ERROR_CODE_RSP;
		break;
	case ICB_SEQ_SET_TIME:
		msg = TMSG_ICB_SET_TIME_RSP;
		break;
	default:
		break;
	}

	_icb_send_msg(s_icb_req_task_id, msg, TMSG_SUB_ALARM, s_icb_alarm_code, ex_icb_task_seq, ex_position_sensor);
	s_icb_req_task_id = 0;
	_icb_set_seq(ICB_SEQ_IDLE, ICB_SEQ_TIMEOUT);
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
void _icb_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_ICB_TASK;
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
			_icb_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_icb_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _icb_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	_icb_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_TEST_RUNNING, 0, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	//if (fatal_err)
	//{
	//	_icb_send_msg(ID_ICB_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	//}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_ICB_TASK, (u16)code, (u16)icb_msg.tmsg_code, (u16)icb_msg.arg1, fatal_err);
}


#if defined(UBA_RTQ_ICB)

static void _icb_accept_rtq_req_proc(void)
{
	s_icb_req_task_id = icb_msg.sender_id;
	s_icb_arg1 = icb_msg.arg1;
	s_icb_arg2 = icb_msg.arg2;
	s_icb_arg3 = icb_msg.arg3;
	_icb_set_seq(ICB_SEQ_ACCEPT_RTQ, ICB_SEQ_TIMEOUT);
}

static void _icb_accept_rtq_seq_proc(u32 flag, s32 data)// ok
{
	//この処理はFRAMにバックアップ用に書き込むだけ、
	//RTQに命令してRFID書き込みは、MainからRCタスクに直接命令する
	switch (ex_icb_task_seq & 0x00FF)
	{
	case 0x00:
		_icb_accept_rtq_0B00_seq(flag, data);
		break;
	case 0x01:
		_icb_fram_backup_rtq_seq(flag, data);
		break;
	default:								/* other */
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);

		/* system error ? */
		_icb_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0300
 *
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_accept_rtq_0B00_seq(u32 flag, s32 data)
{
	u8 aa;

	if((is_box_flag_no_data()) || (is_box_flag_initial_request()))
	{
		_icb_seq_complete();
		return;
	}
	else if(is_box_flag_read())
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_NOT_INITIALIZE);
		return;
	}

	if(s_icb_arg1 == RFID_DENOMI)
	{
		//通常時
		if(s_icb_arg2 != BAR_INDX)
		{
			memset((u8 *)&Smrtdat_fram_bk, 0, sizeof(Smrtdat_fram_bk)); //受け取り完了なのでリカバリ用はクリア、書き込み時にこの情報もクリアで書き込みされる
			//受け取った金種のカウントアップとSumの計算のみ	
			icb_update_denomi_counter_rtq(0,s_icb_arg2);  //この中でもculc_BLK1_checksum_rtqをコールしている
		}
		else
		{
		//Ticketは未対応
			_icb_seq_complete();
			return;
		}		
	}
	else if(s_icb_arg1 == RFID_DENOMI_UNIT)
	{
		//Collect時、Payout失敗からの収納
		//RecycleSettingInfo.DenomiInfo[OperationDenomi.unit - 1].Data1
		aa = RecycleSettingInfo.DenomiInfo[s_icb_arg2 - 1].Data1;
		switch (aa)
		{
		case 0x01:
		aa = 1;
			break;
		case 0x02:
		aa = 2;
			break;
		case 0x04:
		aa = 3;
			break;
		case 0x08:
		aa = 4;
			break;
		case 0x10:
		aa = 5;
			break;
		case 0x20:
		aa = 6;
			break;
		case 0x40:
		aa = 7;
			break;
		case 0x80:
		aa = 8;
			break;
		default:								/* other */
		aa = 0;
			break;
		}
		
		if(aa != 0)
		{
			memset((u8 *)&Smrtdat_fram_bk, 0, sizeof(Smrtdat_fram_bk)); //受け取り完了なのでリカバリ用はクリア、書き込み時にこの情報もクリアで書き込みされる
			icb_update_denomi_counter_rtq(1, aa); //collect時 この中でも culc_BLK1_checksum_rtq をコールしている
		}
	}
	//#if defined(RFID_RECOVER)
	else if(s_icb_arg1 == RFID_BACK)
	{
	//パワーリカバリの為 情報書き込み
		Smrtdat_fram_bk.unit = s_icb_arg2;
		Smrtdat_fram_bk.mode = s_icb_arg3;
	}
	//#endif
	_icb_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_RECOVER_RTQ, 0, 0, 0);
	_icb_set_seq(0x0B01, ICB_SEQ_TIMEOUT);
}

static void _icb_fram_backup_rtq_seq(u32 flag, s32 data)
{
	if(IS_FRAM_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_FRAM);
	}
	else if(IS_FRAM_EVENT_WRITE_SUCCESS(flag))
	{
	//1回でFRAMへ全書き
		_icb_seq_complete();
		ex_rtq_rfid_data = 1;
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// FRAMタスクを待つ (wait FRAM task)
	}
}

#endif


#if defined(UBA_RTQ_ICB)
static void _icb_initial_seq_proc_rtq(u32 flag, s32 data)// ok
{
	switch (ex_icb_task_seq & 0x00FF)
	{
		// read data
	case 0x00:
		_icb_initial_0100_seq_rtq(flag, data);
		break;
	case 0x01:
		_icb_initial_0101_seq_rtq(flag, data);
		break;
		// recovery
	case 0x11:
		_icb_initial_0111_seq_rtq(flag, data);
		break;
		// data check
	case 0x21:
		_icb_initial_0121_seq_rtq(flag, data);
		break;
		// initialize data
	case 0x31:
		_icb_initial_0131_seq_rtq(flag, data);
		break;
	case 0x35:
		_icb_initial_0135_seq_rtq(flag, data);
		break;
	case 0x36:
		_icb_initial_0136_seq_rtq(flag, data);
		break;

	/* RC RFID */
		// READ DATA
	case 0x50:
		_icb_initial_0150_seq_rtq(flag, data);	// Smrtdat をRTQから入手
		break;
	case 0x51:
		_icb_initial_0151_seq_rtq(flag, data);	// Smrtdat2 をRTQから入手
		break;
	#if 0	//sumエラーの時に、Headから強制的にsumエラーにならない様にする処理
	case 0x60:
		_icb_initial_0160_seq(flag, data);
		break;
	case 0x61:
		_icb_initial_0161_seq(flag, data);
		break;
	case 0x62:
		_icb_initial_0162_seq(flag, data);
		break;
	#endif	
	default:								/* other */
		_icb_set_alarm(ALARM_CODE_ICB_FORCED_QUIT);

		/* system error ? */
		_icb_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0100
 *  ICB_SUB_INITIAL_BACKUP_STATUS
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0100_seq_rtq(u32 flag, s32 data)
{
	s_icb_watch_index = 0;
	rc_rfid_reset();

	_icb_set_seq( 0x0150, ICB_SEQ_TIMEOUT);
}


/*********************************************************************//**
 * @brief icb control sequence 0x0101
 *  ICB_SUB_INITIAL_UID_READ_DATA & ICB_SUB_INITIAL_READ_DATA
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0101_seq_rtq(u32 flag, s32 data)
{
	if(IS_ICB_EVENT_READ_FAIL(flag))
	{
		/* ここに入る事はない */
		/* RFIDの通信エラーは、直接mianタスクへ送信しているようなので、*/
		_icb_set_alarm(icb_msg.arg2);
	}
	else if(IS_ICB_EVENT_READ_SUCCESS(flag))
	{
		s_icb_watch[ICB_LOG_INDEX].end = OSW_TIM_value() - s_icb_watch[ICB_LOG_INDEX].start;
		s_icb_watch_index++;
		/*	ICB functionがDisableのときは終了	*/
		if(!is_icb_enable())
		{
		#if 1 
		//2023-11-09
			//ICB無効なのに、ICB通信できた事によるエラー
			_icb_set_alarm(ALARM_CODE_RFID_ICB_SETTING);
		#else
			_icb_seq_complete();
		#endif
		}
		else if(is_box_flag_inhibit()) //use SS and RTQ 使用するか未定だが残す
		{
			/*	SYSTEM INHIBITフラグ(INHIBITフラグセットされたBOX)　*/
			_icb_set_alarm(ALARM_CODE_RFID_ICB_SETTING);
		}
		/* Add Clear request flg	*/
		else if((is_box_flag_no_data()) || (is_box_flag_initial_request()))
		{
		//新規BOX
			_icb_set_seq( ICB_SEQ_INITIAL_DATA_WRITE, ICB_SEQ_TIMEOUT); //0x0131 (BOXが新規なので、Head側の情報を追加してRFIDへ書き込み)
		}
		else if(is_box_flag_read())
		{
		//中途半端なBOX(BoxがRead Write ToolでReadされているが、初期化されていない為、エラー)
			/*	集計済みBoxのためError	*/
			/*	End process	*/
			_icb_set_alarm(ALARM_CODE_RFID_ICB_NOT_INITIALIZE);
		}
		else
		{
		//使用中BOX
			/*	初期化及び動作中BOXの場合は、Recovery処理へ移行	*/
			_icb_set_seq( ICB_SEQ_INITIAL_RECOVERY, ICB_SEQ_TIMEOUT); //0x0111 (マシン番号の一致を確認が主)
		}
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}


/*********************************************************************//**
 * @brief icb control sequence 0x0111
 *  ICB_SUB_INITIAL_DATA_RECOVERY,
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0111_seq_rtq(u32 flag, s32 data) //ここに来るのは、1==1度でもHeadでイニシャル動作完了、2==データが存在する (マシン番号の一致を確認が主)
//ここでマシン番号だけでなく、sumのチェックも行っていいが、SSの場合、データの分割送信なので、リカバリデータが残っていて、sumが不一致の場合がある
//その為、リカバリデータがある場合、0x0112でリカバリデータを書き込んで、最終的に0x0121でsum確認を行っている。
//RTQの場合、ここでsumチェックを行って処理を抜けてもいいが、将来リカバリ処理が入るかもしれないので、sumチェックの為0x0121を送る
{
	if((icb_machine_number_is_valid() == FALSE)
		&& (check_ICBflag() == TRUE))
	{
		/* ICB有効設定で、マシン番号が壊れている*/
		/*	ICB Machine Number Error */
		_icb_set_alarm(ALARM_CODE_RFID_ICB_MC_INVALID);
	}
	else if(ALARM_CODE_RFID_ICB_NUMBER_MISMATCH == icb_check_machineNo())
	{
		/*	ICB Machine Number Mismatch */
		_icb_set_alarm(ALARM_CODE_RFID_ICB_NUMBER_MISMATCH);
	}
	//RTQ
	else
	{
		/* Non Recovery data	*/
		/*	go to Next process			*/
		/*	初期化及び動作中BOXの場合は、データチェック
		 * 処理へ移行	*/
		_icb_set_seq( ICB_SEQ_INITIAL_DATA_CHECK, ICB_SEQ_TIMEOUT); //0x0121 sumチェックへ
	}
}

/*********************************************************************//**
 * @brief icb control sequence 0x0121
 *  ICB_SUB_INITIAL_DATA_CHECK (check DATA from FRAM)
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
static void _icb_initial_0121_seq_rtq(u32 flag, s32 data) //リカバリ後のsumチェックが主 0x0111 0x0113からの流れ ここに来るのは、1==1度でもHeadでイニシャル動作完了、2==データが存在する, リカバリ後のsumの一致確認が主
{
	//SSの場合、イニシャル動作後、1度でも書き込んだことがあるかで処理を分けていて、
	//違いはイニシャル中のリカバリのようだ,RTQでは処理イニシャル中のリカバリは行わないので処理分けない
	//RFIDで保持する情報で時間も使用されない可能性が高いので、ここでex_rtc_clockには保存しない
	/*<< 集計データありBox, 集計データなしでHeadとのイニシャルは1度完了しているBox >>*/
	if(is_icb_checksum_error())
	{
	#if 0//#if defined(UBA_RTQ)
		_icb_set_seq(0x0160, ICB_SEQ_TIMEOUT);
	#else
		/*	チェックサムエラー	*/
		_icb_set_alarm(ALARM_CODE_RFID_ICB_DATA);
	#endif 	
	}
	/*	During use BOX	*/
	else
	{
		/*	End process	*/
		_icb_seq_complete();
	}
}

/*********************************************************************//**
 * @brief icb control sequence 0x0131
 *  ICB_SUB_INITIAL_SET_DATA
 * @param[in]	icb event flag
 * @return 		None
 **********************************************************************/
extern u8 rc_rfid_data[255]; //RTQへ通信する送信バッファにコピーするバッファ
static void _icb_initial_0131_seq_rtq(u32 flag, s32 data) //ここの来るのは初期化するBoxのみ 0,4, SSの場合リカバリ中のResetリカバリの場合もここに遷移させているが、RTQでは処理外したため、RTQでは初期化するBoxのみここに来る
{
	//RTQでは初期化するBoxのみなので最初の条件式を削除した
	/*--------------------------------------------------------*/
	u32 initTime;
	u8 boxNum[20];

	/* save the cashbox number and init time  */
	/* 時間とBox番号のみBox側の情報を保持して、それ以外は初期化 */
	initTime = Smrtdat.initim; 					//初期化前に時間バックアップ
	memcpy(&boxNum[0], &Smrtdat.boxno[0], 20);	//初期化前にBox番号バックアップ

	/*	All 0	*/
	fill_memo32(0, (u32 *)&Smrtdat, sizeof(Smrtdat) / 4);
	fill_memo32(0, (u32 *)&Smrtdat2, sizeof(Smrtdat2) / 4);

	/* restore the cashbox number and init time */
	Smrtdat.initim = initTime;				/*	時間を戻す	*/
	memcpy(&Smrtdat.boxno[0], &boxNum[0], 20);

	/*	Set Initial Info to ICB memory	*/
	icb_set_initial_info();	//same ex_ICB_gameno,software_ver..Smrtdatのsum.Smrtdat2のsum
/*--------------------------------------------------------*/
	//Smrtdat,Smrtdat2の初期化完了、ここからRFID側へ初期化した値を書き込んでいく
	//new
	memset((u8 *)&Smrtdat_fram_bk_power, 0, sizeof(Smrtdat_fram_bk_power));			//初期化済みBOXなのでリカバリキャンセル
	_icb_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_RECOVER_RTQ, 0, 0, 0); //初期化済みBOXの情報にHead側も更新 FRAMバックアップ更新

	memcpy(&Smrtdat_fram, &Smrtdat, sizeof(Smrtdat_fram));		//Box側の情報をHead側運用変数 Smrtdat_fram にコピー 基本ここのみ

	memcpy(rc_rfid_data, &Smrtdat_fram, sizeof(Smrtdat_fram));	//SmrtdatのRFIDへの書き込み
	_icb_send_msg(ID_RC_MBX, TMSG_RC_RFID_WRITE_REQ, RFID_RUN, 0, sizeof(Smrtdat_fram), 0);		//SmrtdatのRFIDへの書き込み	

	_icb_set_seq( 0x0135, ICB_SEQ_TIMEOUT);	//FRAMがないので、0x0132をスキップ
}

static void _icb_initial_0135_seq_rtq(u32 flag, s32 data) //初期化したSmrtdatのRFIDへの書き込み中(RTQとの通信)
{

	if(IS_ICB_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_WRITE_SUCCESS(flag))
	{
	//SmrtdatのRTQへの通知完了
	#if 1 //エリア2も書かないとRead write Toolでの受け取り枚数が表示されない
		memcpy(rc_rfid_data, &Smrtdat2, sizeof(Smrtdat2));		//初期化したSmrtdat2のRFIDへの書き込み
		_icb_send_msg(ID_RC_MBX, TMSG_RC_RFID_WRITE_REQ, RFID_RUN, 172, sizeof(Smrtdat2), 0);	//初期化したSmrtdat2のRFIDへの書き込み
		_icb_set_seq( 0x0136, ICB_SEQ_TIMEOUT);
	#else
		_icb_seq_complete();
	#endif
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}

}


static void _icb_initial_0136_seq_rtq(u32 flag, s32 data)//初期化したSmrtdat2のRFIDへの書き込み中(RTQとの通信)
{

	if(IS_ICB_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_WRITE_SUCCESS(flag))
	{
	//Smrtdat2のRTQへの通知完了
		_icb_seq_complete();
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}

static void _icb_initial_0150_seq_rtq(u32 flag, s32 data) // Smrtdat をRTQから入手
{
	u16 addr_start = 0;
	if(IS_ICB_EVENT_READ_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if (IS_ICB_EVENT_READ_SUCCESS(flag))
	{
		/* READ DATA SmrtDat */
		icb_rc_rfid_read(addr_start, sizeof(Smrtdat)/sizeof(u8), (u8*)&Smrtdat.denomi[0]); //データ保存先 Smrtdat.denomi
		_icb_set_seq( 0x0151, ICB_SEQ_TIMEOUT);
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{

	}
}

static void _icb_initial_0151_seq_rtq(u32 flag, s32 data)	// Smrtdat2 をRTQから入手
{
	u16 addr_start = 0;
	if(IS_ICB_EVENT_READ_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if (IS_ICB_EVENT_READ_SUCCESS(flag))
	{
		addr_start = 0x00 + sizeof(Smrtdat) / sizeof(u8);
		/* READ DATA SmrtDat2 */
		icb_rc_rfid_read(addr_start, sizeof(Smrtdat2) / sizeof(u8), (u8 *)&Smrtdat2.rej[0][0]);//データ保存先 Smrtdat2
		_icb_set_seq(0x0101, ICB_SEQ_TIMEOUT);
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{

	}
}

#if 0	//sumエラーの時に、Headから強制的にsumエラーにならない様にする処理
static void _icb_initial_0160_seq(u32 flag, s32 data)
{
	u8 addr_start = 0;
	if(1)
	{
		/*	initialization request of all the data	*/
		// icb_set_initial_all_data();
		Smrtdat.sum =  (u8)culc_BLK1_checksum();
		update_checksum2();
		(void)set_ICBrecovery_data(BLK_SUM_NUMBER);
		(void)set_ICBrecovery_data(BLK_SUM2_NUMBER);
		icb_rfid_write_start();
		_icb_set_seq( 0x0161, ICB_SEQ_TIMEOUT);
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{

	}
	else
	{
		_icb_set_alarm(ALARM_CODE_RFID_UNIT_MAIN);
	}
	
}

static void _icb_initial_0161_seq(u32 flag, s32 data)
{
	u8 addr_start = 0;
	if(IS_ICB_EVENT_WRITE_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if(IS_ICB_EVENT_WRITE_SUCCESS(flag))
	{
		/* READ DATA SmrtDat */
		icb_rfid_read_start();
		_icb_set_seq( 0x0162, ICB_SEQ_TIMEOUT);
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{
		// RFIDタスクを待つ (wait RFID task)
	}
}

static void _icb_initial_0162_seq(u32 flag, s32 data)
{
	int result;
	if(IS_ICB_EVENT_READ_FAIL(flag))
	{
		_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
	}
	else if (IS_ICB_EVENT_READ_SUCCESS(flag))
	{
		result = icb_read_and_verify(ex_ICBrecovery[ex_ICBsend_num].BLK);
		if(result)
		{
			/*	Check Next Data	*/
			renewal_ICBsend_flag();
			/*	check wait data		*/
			if(ex_ICBsend_num == ex_ICBsave_num)
			{
				/*	書込みBufferの初期化	*/
				//icb_clear_recovery_flag(ON);
				_icb_set_seq( 0x0121, ICB_SEQ_TIMEOUT);
			}
			else
			{
				icb_rfid_write_start();
				_icb_set_seq( 0x0161, ICB_SEQ_TIMEOUT);
			}
		}
		else
		{
			_icb_set_alarm(ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN);
		}
	}
	else if(IS_ICB_EVENT_TIMEOUT(flag))
	{

	}
}
#endif


#endif //end UBA_RTQ_ICB


/* EOF */
