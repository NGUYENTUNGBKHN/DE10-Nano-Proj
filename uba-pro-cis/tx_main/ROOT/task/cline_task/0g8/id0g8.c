/******************************************************************************/
/*! @addtogroup Group2
    @file       id0g8.c
    @brief      id0g8 process
    @date       2013/03/22
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan CashMachine Co, Limited. All rights reserved.
*****************************************************************ex_cline_status_tbl.line_task_mode**************
    @par        History
    - 2013/03/22 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
#include <string.h>
#include "kernel.h"
#include "itron.h"
#include "common.h"
#include "custom.h"
#include "id0g8.h"
#include "com_hid_id0g8.h"
#include "com_dfu_id0g8.h"
#include "status_tbl.h"
#include "memorymap.h"
#include "sub_functions.h"
#include "hal_sensor.h"
#include "icb_struct.h"

#include  "IF_ID0G8.def"

#define EXT
#include "../common/global.h"
#include "com_ram.c"

/************************** PRIVATE DEFINITIONS *************************/
static u8 check_last_sequence_id0g8(void);
bool _is_id0g8_bar_inhibit(void);

/*----------------------------------------*/
extern const u8 id0g8_bill_info[18][6];
extern const u16 id0g8_inhi_mask;
extern const u16 dipsw_inhi[];
extern const u8 id0g8_index_to_code[];

static BOOL ex_self_test_0g8;
static u8 s_id0g8_powerup_box_disconnect;

/*----------------------------------------*/

u8  s_id0g8_2nd_note=0;

#define RECOVER_BILL_ESCROW 	1
#define RECOVER_TICKET_ESCROW 	2
#define RECOVER_GLI_ERROR	 	3

static u8 ex_0g8_recovery;		/* パワーリカバリ	*/
static u8 ex_0g8_recovery_com;	/* 通信リカバリ		*/


// Failure data
struct _FAIL_DATA
{
	u8	fail_type;
	u8	diagnostics;
};

#define FAIL_TYPE_FW						0x01
#define FAIL_TYPE_MECH						0x02
//not use uba #define FAIL_TYPE_OPT			0x04
//not use uba #define FAIL_TYPE_COMP		0x08
//not use #define FAIL_TYPE_NVM				0x10
#define FAIL_TYPE_OTHER						0x80

//EXTERN struct _FAIL_DATA ex_fail_data;
struct _FAIL_DATA ex_fail_data;


/* 移動するか検討、パワーリカバリで使用する可能性もある為、このままここに置いておく */

/***** TID Event Status *****/
#define TID_EVT_ST_WAITING_FOR_ACK			2	/* TID Event Status : Waiting For ACK  */

/***** Note/Ticket Status *****/
#define NOTE_TICKET_STATUS_JAM				0x80
#define NOTE_TICKET_STATUS_CHEAT			0x40
#define NOTE_TICKET_STATUS_CLEAR			0x10
#define NOTE_TICKET_STATUS_REMOVED			0x08
#define NOTE_TICKET_STATUS_REJECTED			0x04
#define NOTE_TICKET_STATUS_RETURNED			0x02
#define NOTE_TICKET_STATUS_ACCEPTED			0x01


/***** Stacker Status *****/
//not use #define STACKER_STATUS_FAULT				0x80
#define STACKER_STATUS_JAM					0x04
#define STACKER_STATUS_FULL					0x02
#define STACKER_STATUS_DISCONNECT			0x01


u8 GatData[TOTAL_SIZE_OF_GAT_DATA_REPORTS];

/* TIDイベント構造体 */
struct _TID_EVENT_TBL
{
	/* イベントステータス(0：イベント発生なし、1：イベント発生ありでイベント送信予約待ち、2：イベント送信待ち、3：イベント送信完了でACK受信待ち) */
	u8	TidEvtStatus;		//リカバリ未使用
	u8	CrtEvtNo;			//リカバリ未使用
	u8	note_ticket_status;	//リカバリ未使用
	u8	stacker_status;		//リカバリ未使用
};
struct _TID_EVENT_TBL ex_tid_event_tbl;

typedef struct _ID0G8_DEVICE_INFO
{
	u8 event_occur;					/* リカバリには使用しない */
	bool wait_event_response;		/* ID0G8 wait response *//* リカバリには使用しない */
	u16 res_id[10];					/* 受信関係なので、おそらくリカバリで使わない */
	u8 res_cnt;						/* 受信関係なので、おそらくリカバリで使わない */
	u8 res_occur;					/* リカバリには使用しない */
	u8 res_occur_crc;				
};
struct  _ID0G8_DEVICE_INFO    s_id0g8_info;


#define GAT_DATA_REPORT_LEN			0x3D	/* use */
#define GAT_DATA_REPORT_LEN_LAST	0x3C	/* use */
static u8 s_id0g8_gat_index;	//use
static u8 s_id0g8_note_data_cnt; //use

/************************** PRIVATE VARIABLES *************************/
static u8 s_id0g8_powerup_stat;



/************************** PRIVATE FUNCTIONS *************************/
void _id0g8_initial_msg_proc(void);

/* --------------------------------------------------------------------- */
/* ID-003 Command receiving procedure                                    */
/* --------------------------------------------------------------------- */
static void _id0g8_cmd_proc(void);

//static void _id0g8_dfu_detach_cmd_proc(void);
static void _id0g8_ack_cmd_proc(void);
static void _id0g8_enable_cmd_proc(void);
static void _id0g8_disable_cmd_proc(void);
static void _id0g8_self_test_cmd_proc(void);
static void _id0g8_gat_report_cmd_proc(void);
static void _id0g8_calc_crc_cmd_proc(void);
static void _id0g8_num_of_note_data_cmd_proc(void);
static void _id0g8_read_note_data_cmd_proc(void);
static void _id0g8_extend_timeout_cmd_proc(void);
static void _id0g8_accept_note_ticket_cmd_proc(void);
static void _id0g8_return_note_ticket_cmd_proc(void);

/* --------------------------------------------------------------------- */
/* Task Message receiving procedure                                      */
/* --------------------------------------------------------------------- */



void _id0g8_msg_proc(void);

/* UART1 callback message */
void _id0g8_callback_msg_proc(void);
void _id0g8_callback_receive(void);
void _id0g8_callback_empty(void);

/* Status infomation message */
void _id0g8_status_info_msg_proc(void);
void _id0g8_status_info_mode_powerup(void);
void _id0g8_status_info_mode_escrow(void);
void _id0g8_status_info_mode_vend(void);
void _id0g8_status_info_mode_error(void);
void _id0g8_status_info_mode_def(void);

/* Reset response message */
void _id0g8_reset_rsp_msg_proc(void);
void _id0g8_reset_rsp_mode_power_init(void);
void _id0g8_reset_rsp_mode_init(void);
void _id0g8_reset_rsp_mode_recovery(void);

/* Disable response message */
void _id0g8_disable_rsp_msg_proc(void);

/* Enable response message */
void _id0g8_enable_rsp_msg_proc(void);

/* Accept response message */
void _id0g8_accept_rsp_msg_proc(void);
void _id0g8_accept_rsp_mode_accept(void);
void _id0g8_accept_rsp_mode_escrow(void);
void _id0g8_accept_rsp_mode_rej_wait_accrsp(void);

/* Stack response message */
void _id0g8_stack_rsp_msg_proc(void);
void _id0g8_stack_rsp_mode_power_init(void);
void _id0g8_stack_rsp_mode_init(void);
void _id0g8_stack_rsp_mode_recovery(void);
void _id0g8_stack_rsp_mode_stack(void);
void _id0g8_stack_rsp_mode_wait_accepted_ack(void);

//void _id0g8_stack_rsp_mode_stacked(void);
void _id0g8_stack_rsp_mode_stack_finish(void);

/* Reject response message */
void _id0g8_reject_rsp_msg_proc(void);
void _id0g8_reject_rsp_mode_power_init_rej(void);
void _id0g8_reject_rsp_mode_power_init_pau(void);
void _id0g8_reject_rsp_mode_init_rej(void);
void _id0g8_reject_rsp_mode_init_pau(void);
void _id0g8_reject_rsp_mode_recovery_rej(void);
void _id0g8_reject_rsp_mode_recovery_pau(void);
void _id0g8_reject_rsp_mode_reject(void);
void _id0g8_reject_rsp_mode_accept(void);
void _id0g8_reject_rsp_mode_escrow(void);
void _id0g8_reject_rsp_mode_init(void);
void _id0g8_reject_rsp_mode_stack(void);
void _id0g8_reject_rsp_mode_recovery(void);
void _id0g8_reject_rsp_mode_return(void);
void _id0g8_reject_rsp_mode_enable_disable_rej(void);

/* Times up message */
void _id0g8_times_up_msg_proc(void);
void _id0g8_timeup_dipsw_read_proc(void);

void _id0g8_timeup_escrow_hold1_proc(void);


void _id0g8_timeup_re_send_proc(void);

/* --------------------------------------------------------------------- */
/* Sub functions                                                         */
/* --------------------------------------------------------------------- */


void _id0g8_intr_mode_sub(u16 mode, u8 wait_flag);
u8 check_failure_0g8(u16 alarm_code);


static void _id0g8_send_failure_status(void);


u16 _id0g8_dipsw_disable(void);

void _set_id0g8_reject(u16 mode, u16 code);
void _set_id0g8_alarm(u16 mode, u16 code);

bool _is_id0g8_enable(void);
bool _is_id0g8_denomi_inhibit(u32 escrow_code, u32 direction);
u8  _is_id0g8_escrow_data(u16 denomi_code);

void  _is_id0g8_error_sts(u16 alarm_code, u8 *data);

/************************** EXTERN VARIABLES *************************/
T_MSG_BASIC cline_msg;

/************************** EXTERN FUNCTIONS *************************/
extern void _cline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _cline_system_error(u8 fatal_err, u8 code);

u8 l_aulID0G8_rx_buff[COM_DFU_BUF_SIZE];
static u8 l_aulID0G8_tx_buff[COM_DFU_BUF_SIZE];
u16 l_ausID0G8_tx_size;

#if defined(ID0G8_LOG)
void save_log_0g8(u8 type)
{

	if(type == 0)	/* 2020-11-05 */
	{
	    debug_line_mode[debug_line_count] = ex_cline_status_tbl.line_task_mode;
	    debug_line_type[debug_line_count] = 0;
	    debug_line_data0[debug_line_count] = 0;
	    debug_line_data1[debug_line_count] = 0;
	    debug_line_data2[debug_line_count] = 0;

	    debug_line_mode1[debug_line_count] = ex_main_task_mode1;
	    debug_line_mode2[debug_line_count] = ex_main_task_mode2;

	    debug_line_count++;
	}
	else
	{
	    debug_line_mode[debug_line_count] = ex_cline_status_tbl.line_task_mode;
	    debug_line_type[debug_line_count] = 1;
	    debug_line_data0[debug_line_count] = (u8)cline_msg.tmsg_code;
	    debug_line_data1[debug_line_count] = (u8)cline_msg.arg1;
	    debug_line_data2[debug_line_count] = (u8)cline_msg.arg2;

	    debug_line_mode1[debug_line_count] = ex_main_task_mode1;
	    debug_line_mode2[debug_line_count] = ex_main_task_mode2;

	    debug_line_count++;
	}
	if(debug_line_count >= 100)
	{
	    debug_line_count = 0;
	}


}
#endif

void _id0g8_transfer_event_clear(void)
{
	/* イベント送信の為のフラグ関係を初期化する処理 */
	memset((u8 *)&ex_cline_status_tbl.event_id[0], 0, sizeof(ex_cline_status_tbl.event_id));
	memset((u8 *)&ex_cline_status_tbl.event_id_sub[0], 0, sizeof(ex_cline_status_tbl.event_id_sub));
	ex_cline_status_tbl.event_cnt = 0;
	s_id0g8_info.event_occur = false;

}

void _id0g8_denomi_clear(void)
{
	/* リカバリにも使用しているので、処理完了後はクリア */
	ex_cline_status_tbl.escrow_code = 0;
	memset((u8 *)&ex_cline_status_tbl.ex_Barcode_recovery_0g8[0], 0, sizeof(ex_cline_status_tbl.ex_Barcode_recovery_0g8));
}




/*********************************************************************//**
 * @brief set tid
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _id0g8_set_new_tid_event(u8 tid_event)//ok 新規 or 保留イベント送信
{
	// TIDイベントステータスがイベント無しの場合
	ex_cline_status_tbl.Tid = ex_cline_status_tbl.NextTid;

	if(ex_cline_status_tbl.NextTid != 255)
	{
		ex_cline_status_tbl.NextTid++;
	}
	else
	{
		ex_cline_status_tbl.NextTid = 0;
	}

	ex_tid_event_tbl.CrtEvtNo = tid_event;
	s_id0g8_info.event_occur = true;

}

static void _id0g8_transfer_next_res(void)//ok
{

	u8 cnt;

	for(cnt = 0; cnt < s_id0g8_info.res_cnt; cnt++)
	{
		s_id0g8_info.res_id[cnt] = s_id0g8_info.res_id[cnt+1];
	}
	if(s_id0g8_info.res_cnt > 0)
	{
		s_id0g8_info.res_cnt--;
	}
	
	if(s_id0g8_info.res_cnt > 0)
	{
		s_id0g8_info.res_occur = true;
	}
}

static void _id0g8_set_event( u8 event, u8 type)//ok
{
	u8 event_u8 = 0;

	event_u8 = event;

	switch(event_u8)
	{
		case ID0G8_EVT_NOTE_VALIDATED:
		case ID0G8_EVT_TICKET_VALIDATED:
		case ID0G8_EVT_NOTE_TICKET_STATUS:
		case ID0G8_EVT_STACKER_STATUS:

			if(ex_cline_status_tbl.event_cnt < (10 - 1))
			{
				ex_cline_status_tbl.event_id[ex_cline_status_tbl.event_cnt] = event;
				ex_cline_status_tbl.event_id_sub[ex_cline_status_tbl.event_cnt] = type;

				ex_cline_status_tbl.event_cnt++;

				if(ex_cline_status_tbl.event_cnt == 1)
				{
					// TID番号を付与する
					_id0g8_set_new_tid_event(event); //保持イベントがない時の新規イベント
				}
			}
			break;
		case ID0G8_EVT_CRC_DATA:
			s_id0g8_info.res_occur_crc = true;
			break;
		default:

			if(s_id0g8_info.res_cnt < (10 - 1))
			{
				s_id0g8_info.res_id[s_id0g8_info.res_cnt] = event;		/* 1コマンドのみにする */
				s_id0g8_info.res_cnt++;
				s_id0g8_info.res_occur = true;
			}
			break;
	}

}

static void _id0g8_make_event( void )//ok
{
	extern const unsigned char GsaNumNoteDataEntries;
	extern const unsigned char GsaNoteTable[];
	extern const u8 accept_denomi[];
	
	u16 cnt;
	u8	set_time = 0;
	u8 type = 0;

	if(s_id0g8_info.res_occur_crc == true)
	{
		type = ID0G8_EVT_CRC_DATA;
	}
	else if(s_id0g8_info.res_occur == true)
	{
		type = s_id0g8_info.res_id[0];
	}
	else if(s_id0g8_info.event_occur == true)
	{
		type = ex_cline_status_tbl.event_id[0];
	}
	else
	{
		return;
	}

	switch(type)
	{
	case ID0G8_EVT_POWER_STATUS:
		break;
	case ID0G8_EVT_GAT_DATA:
		l_aulID0G8_tx_buff[0] = ID0G8_EVT_GAT_DATA;
		l_aulID0G8_tx_buff[1] = s_id0g8_gat_index;
		memcpy(&l_aulID0G8_tx_buff[3], &GatData[GAT_DATA_REPORT_LEN * (s_id0g8_gat_index - 1)], GAT_DATA_REPORT_LEN);
		
		if(s_id0g8_gat_index == 1)
		{
			l_aulID0G8_tx_buff[2] = GAT_DATA_REPORT_LEN;
			s_id0g8_gat_index = 2;
		}
		else
		{
			l_aulID0G8_tx_buff[2] = GAT_DATA_REPORT_LEN_LAST;
			s_id0g8_info.res_occur = false;
			_id0g8_transfer_next_res();
		}
		l_ausID0G8_tx_size = 64;
		break;

	case ID0G8_EVT_CRC_DATA:
		l_aulID0G8_tx_buff[0] = ID0G8_EVT_CRC_DATA;
		l_aulID0G8_tx_buff[1] = (u8)((ex_rom_crc32      ) & 0xff);
		l_aulID0G8_tx_buff[2] = (u8)((ex_rom_crc32 >>  8) & 0xff);
		l_aulID0G8_tx_buff[3] = (u8)((ex_rom_crc32 >> 16) & 0xff);
		l_aulID0G8_tx_buff[4] = (u8)((ex_rom_crc32 >> 24) & 0xff);
		l_ausID0G8_tx_size = 5;
		s_id0g8_info.res_occur_crc = false;
		break;

	case ID0G8_EVT_DEVICE_STATE:
		l_aulID0G8_tx_buff[0] = ID0G8_EVT_DEVICE_STATE;
		//if(ex_cline_status_tbl.accept_disable)
		if(ex_cline_status_tbl.accept_disable)
		{
			l_aulID0G8_tx_buff[1] = 0x02;
		}
		else
		{
			l_aulID0G8_tx_buff[1] = 0x01;
		}
		l_ausID0G8_tx_size = 2;

		s_id0g8_info.res_occur = false;
		_id0g8_transfer_next_res();
		break;

	case ID0G8_EVT_NUM_OF_NOTE_DATA:
		l_aulID0G8_tx_buff[0] = ID0G8_EVT_NUM_OF_NOTE_DATA;
		l_aulID0G8_tx_buff[1] = GsaNumNoteDataEntries;
		l_ausID0G8_tx_size = 2;

		s_id0g8_info.res_occur = false;
		_id0g8_transfer_next_res();
		break;

	case ID0G8_EVT_READ_NOTE_DATA_TABLE:
		l_aulID0G8_tx_buff[0] = ID0G8_EVT_READ_NOTE_DATA_TABLE;
		l_aulID0G8_tx_buff[1] = GsaNoteTable[s_id0g8_note_data_cnt * 8 + 0];
		l_aulID0G8_tx_buff[2] = GsaNoteTable[s_id0g8_note_data_cnt * 8 + 1];
		l_aulID0G8_tx_buff[3] = GsaNoteTable[s_id0g8_note_data_cnt * 8 + 2];
		l_aulID0G8_tx_buff[4] = GsaNoteTable[s_id0g8_note_data_cnt * 8 + 3];
		l_aulID0G8_tx_buff[5] = GsaNoteTable[s_id0g8_note_data_cnt * 8 + 4];
		l_aulID0G8_tx_buff[6] = GsaNoteTable[s_id0g8_note_data_cnt * 8 + 5];
		l_aulID0G8_tx_buff[7] = GsaNoteTable[s_id0g8_note_data_cnt * 8 + 6];
		l_aulID0G8_tx_buff[8] = GsaNoteTable[s_id0g8_note_data_cnt * 8 + 7];
		l_ausID0G8_tx_size = 9;
		
		s_id0g8_note_data_cnt++;
		
		if(s_id0g8_note_data_cnt >= GsaNumNoteDataEntries)
		{
			s_id0g8_info.res_occur = false;
			_id0g8_transfer_next_res();
		}
		else
		{

		}
		break;

	case ID0G8_EVT_FAIL_STATUS:
		l_aulID0G8_tx_buff[0] = ID0G8_EVT_FAIL_STATUS;
		l_aulID0G8_tx_buff[1] = ex_fail_data.fail_type;
		l_aulID0G8_tx_buff[2] = ex_fail_data.diagnostics;
		l_ausID0G8_tx_size = 3;

		s_id0g8_info.res_occur = false;
		_id0g8_transfer_next_res();
		break;


	case ID0G8_EVT_NOTE_VALIDATED:		// TIDイベント

		l_aulID0G8_tx_buff[0] = ID0G8_EVT_NOTE_VALIDATED;
		l_aulID0G8_tx_buff[1] = ex_cline_status_tbl.Tid;
		l_aulID0G8_tx_buff[2] = id0g8_index_to_code[ex_cline_status_tbl.escrow_code];
		l_ausID0G8_tx_size = 3;
		
		ex_tid_event_tbl.TidEvtStatus = TID_EVT_ST_WAITING_FOR_ACK;
		s_id0g8_info.wait_event_response = true;
		set_time = 1;

		s_id0g8_info.event_occur = false;

		break;

	case ID0G8_EVT_TICKET_VALIDATED:	// TIDイベント

		memset((u8 *)&l_aulID0G8_tx_buff[0], 0, sizeof(l_aulID0G8_tx_buff));
		
		l_aulID0G8_tx_buff[0] = ID0G8_EVT_TICKET_VALIDATED;
		l_aulID0G8_tx_buff[1] = ex_cline_status_tbl.Tid;
		for( cnt = 0; cnt < 28; cnt++ )
		{
			if( ex_cline_status_tbl.ex_Barcode_recovery_0g8[cnt] == 0x00 )
			{
				break;
			}
			l_aulID0G8_tx_buff[3 + cnt] = ex_cline_status_tbl.ex_Barcode_recovery_0g8[cnt];
		}

		l_aulID0G8_tx_buff[2] = cnt;
		
		// 0g8はバーコードの長さが２４バイトまでしか対応していない為、送れるところまで送る
		l_ausID0G8_tx_size = 27;

		
		ex_tid_event_tbl.TidEvtStatus = TID_EVT_ST_WAITING_FOR_ACK;
		s_id0g8_info.wait_event_response = true;
		set_time = 1;

		s_id0g8_info.event_occur = false;

		break;

	case ID0G8_EVT_NOTE_TICKET_STATUS:	// TIDイベント

		l_aulID0G8_tx_buff[0] = ID0G8_EVT_NOTE_TICKET_STATUS;
		l_aulID0G8_tx_buff[1] = ex_cline_status_tbl.Tid;
		l_aulID0G8_tx_buff[2] = ex_cline_status_tbl.event_id_sub[0];
		l_ausID0G8_tx_size = 3;

		
		ex_tid_event_tbl.TidEvtStatus = TID_EVT_ST_WAITING_FOR_ACK;
		s_id0g8_info.wait_event_response = true;
		set_time = 1;

		s_id0g8_info.event_occur = false;

		break;

	case ID0G8_EVT_STACKER_STATUS:		// TIDイベント
		l_aulID0G8_tx_buff[0] = ID0G8_EVT_STACKER_STATUS;
		l_aulID0G8_tx_buff[1] = ex_cline_status_tbl.Tid;
		l_aulID0G8_tx_buff[2] = ex_cline_status_tbl.event_id_sub[0];
		l_ausID0G8_tx_size = 3;
		
		ex_tid_event_tbl.TidEvtStatus = TID_EVT_ST_WAITING_FOR_ACK;
		s_id0g8_info.wait_event_response = true;
		set_time = 1;

		s_id0g8_info.event_occur = false;

		break;

	default:
		break;
	}

	/* 	TIDイベントを送信する場合	*/
//	if(s_id0g8_info.wait_event_response == true)
	if( set_time == 1 )
	{
	/* イベント再送用のタイマ */
		_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_EVENT_SEND, 100, 0, 0); //100msec
	}

}



/*********************************************************************//**
 * @brief trans disable mode
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_trans_mode_disable(void)//ok 
{
	/* 2枚目の取り込み開始している場合 */
	if( 0 != s_id0g8_2nd_note )
	{
		_set_id0g8_reject(ID0G8_MODE_DISABLE_REJECT, REJECT_CODE_INHIBIT);
	}
}

/*********************************************************************//**
 * @brief trans disable mode
 * @param[in]	None
 * @return 		None  				Enableに遷移する時に常に呼び出される
 **********************************************************************//* この状態の場合を再評価必要 */
void _id0g8_trans_mode_enable(void)//ok
{
	/* 2枚目の取り込み開始している場合 *//* 2枚目の取り込みが完了していて識別OKの場合が2 *//* 2020-12-09*/
    if( 2 == s_id0g8_2nd_note )
    {

		ex_cline_status_tbl.escrow_code = ex_cline_status_tbl.escrow_code_2nd_note;

       // バーコード
        if(ex_cline_status_tbl.escrow_code_2nd_note == BAR_INDX)
        {
			if(_is_id0g8_bar_inhibit())
			{
                _set_id0g8_reject(ID0G8_MODE_REJECT, REJECT_CODE_INHIBIT);
			}
			else
			{
				_id0g8_intr_mode_sub( ID0G8_MODE_ESCROW_WAIT_ACK, 0 );

				// TIDイベント作成予定 Ticket validated
				memo_copy( (u8*)&ex_cline_status_tbl.ex_Barcode_recovery_0g8[0], &ex_barcode[0], 32);
				_id0g8_set_event(ID0G8_EVT_TICKET_VALIDATED, 0);

				set_recovery_step(RECOVERY_STEP_ESCORW_WAIT_ACK);	/* 2021-01-13 */
			}
        }
        // 紙幣
        else
        {
			// inhibitは確認済みだが念のため
			/* 方向の情報はリカバリで保持していないのでとりあえず0*/
            if( _is_id0g8_denomi_inhibit((u32)ex_cline_status_tbl.escrow_code_2nd_note, 0) )
            {
                _set_id0g8_reject(ID0G8_MODE_REJECT, REJECT_CODE_INHIBIT);
            }
            else
            {
                _id0g8_intr_mode_sub( ID0G8_MODE_ESCROW_WAIT_ACK, 0 );
                
                // TIDイベント作成予定 Note validated
                _id0g8_set_event(ID0G8_EVT_NOTE_VALIDATED, 0);

				set_recovery_step(RECOVERY_STEP_ESCORW_WAIT_ACK);	/* 2021-01-13 */

            }
        }
    	s_id0g8_2nd_note = 0;
		ex_cline_status_tbl.escrow_code_2nd_note = 0;

        set_recovery_step( RECOVERY_STEP_ACCEPT );
    }
    else
    {
        _cline_send_msg( ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0 );
	    if( s_id0g8_2nd_note == 0 )
		{
			_cline_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, 0, 0, 0, 0);
		}
    }
}


/*********************************************************************//**
 * @brief trans escrow mode
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_trans_mode_accepted_wait_ack(void)//okS
{
	s_id0g8_2nd_note = 0;
	
	// 返却タイムアウト時間を設定する
	_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_HOLD1, 500, 0, 0);
}


static void _id0g8_set_same_tid_event(u8 tid)//okS
{	
	ex_cline_status_tbl.Tid = tid;
	ex_cline_status_tbl.event_id[0] = ex_tid_event_tbl.CrtEvtNo;
	s_id0g8_info.event_occur = true;	
}


u8 _id0g8_check_remaining_events(void)
{
	if(ex_cline_status_tbl.event_cnt == 0)
	{
		return(0);
	}
	else
	{

		return(1);	/* 0 */
	}
}



static void _id0g8_transfer_next_event(void) //Ack受信処理
{
	u8 cnt;
	
	for(cnt = 0; cnt < ex_cline_status_tbl.event_cnt; cnt++)
	{
		ex_cline_status_tbl.event_id[cnt] = ex_cline_status_tbl.event_id[cnt+1];
		ex_cline_status_tbl.event_id_sub[cnt] = ex_cline_status_tbl.event_id_sub[cnt+1];
	}
	if(ex_cline_status_tbl.event_cnt > 0)
	{
		ex_cline_status_tbl.event_cnt--;
	}
		
	
	// ex_tid_event_tbl.Tidはそのまま
	ex_tid_event_tbl.CrtEvtNo = 0;
	
	// TIDイベントの場合
	switch(ex_cline_status_tbl.event_id[0])
	{
	case ID0G8_EVT_NOTE_VALIDATED:
	case ID0G8_EVT_TICKET_VALIDATED:
	case ID0G8_EVT_NOTE_TICKET_STATUS:
	case ID0G8_EVT_STACKER_STATUS:
		// TID番号を付与する
		_id0g8_set_new_tid_event(ex_cline_status_tbl.event_id[0]);//Ack受信処理完了、保留されている次のイベント
		break;
	default:
		break;
	}
}


void _id0g8_make_signature_gat(void)
{
	extern const unsigned char romid_name[];
	/* Rom ID */
	extern const unsigned char romid[];
	extern const unsigned char romid_UBA[];

	/* CR(Carrige Return) + LF(Line Feed) */
	extern const unsigned char gat_new_line[];
	/* Item Name for Version & Date */
	extern const unsigned char ver_name[];
	/* Version & Date */
	extern const unsigned char ver[];
	/* Item Name for CRC16 */
	extern const unsigned char crc16_name[];
	
	u8 DucCrc16[5];
	u8 cnt;
	
	memset((u8 *)&GatData[0], 0, sizeof(GatData));

	/* Item Name for Rom IDをGAT Dataに追加 */
	strcat((char *)GatData, (char *)romid_name);

	/* Rom IDをGAT Dataに追加 */
	if(is_legacy_mode_enable() == true)
	{
		/* Rom IDをGAT Dataに追加 */
		strcat((char *)GatData, (char *)romid_UBA);
	}
	else
	{
		/* Rom IDをGAT Dataに追加 */
		strcat((char *)GatData, (char *)romid);
	}

	/* CR(Carrige Return) + LF(Line Feed)をGAT Dataに追加 */
	strcat((char *)GatData, (char *)gat_new_line);
	/* Item Name for Version & DateをGAT Dataに追加 */
	strcat((char *)GatData, (char *)ver_name);
	/* Version & DateをGAT Dataに追加 */
	strcat((char *)GatData, (char *)ver);
	/* CR(Carrige Return) + LF(Line Feed)をGAT Dataに追加 */
	strcat((char *)GatData, (char *)gat_new_line);
	/* Item Name for CRC16をGAT Dataに追加 */
	strcat((char *)GatData, (char *)crc16_name);

	/* CRC16演算値を文字列に変換 */
	DucCrc16[0] = ((u8)((ex_rom_crc16 >> 12) & 0xf) >= 10) ? ((u8)((ex_rom_crc16 >> 12) & 0xf) - 10 + 'A') : ((u8)((ex_rom_crc16 >> 12) & 0xf) + '0');
	DucCrc16[1] = ((u8)((ex_rom_crc16 >>  8) & 0xf) >= 10) ? ((u8)((ex_rom_crc16 >>  8) & 0xf) - 10 + 'A') : ((u8)((ex_rom_crc16 >>  8) & 0xf) + '0');
	DucCrc16[2] = ((u8)((ex_rom_crc16 >>  4) & 0xf) >= 10) ? ((u8)((ex_rom_crc16 >>  4) & 0xf) - 10 + 'A') : ((u8)((ex_rom_crc16 >>  4) & 0xf) + '0');
	DucCrc16[3] = ((u8)((ex_rom_crc16      ) & 0xf) >= 10) ? ((u8)((ex_rom_crc16      ) & 0xf) - 10 + 'A') : ((u8)((ex_rom_crc16      ) & 0xf) + '0');
	DucCrc16[4] = 0;
	
	/* CRC16文字列データをGAT Dataに追加 */
	strcat((char *)GatData, (char *)DucCrc16);
	
	/* カウント数がGATデータレポートサイズ-1に達するまで繰り返す */
	for (cnt = 0; cnt < TOTAL_SIZE_OF_GAT_DATA_REPORTS - 1; cnt++) {
		/* NULL文字の場合？ */
		if (GatData[cnt] == 0) {
			/* GAT Dataの残り領域にSPACE文字を設定 */
			memset(&GatData[cnt], 0x20, TOTAL_SIZE_OF_GAT_DATA_REPORTS - cnt);
			/* forループ文を抜ける */
			break;
		}
	}
	s_id0g8_gat_index = 1;

	_id0g8_set_event(ID0G8_EVT_GAT_DATA, 0);

}



static void _id0g8_init_nvm_info(void)
{
	#if 1
	// TIDイベント関連を初期化, 起動時も初期化
	memset((u8 *)&ex_tid_event_tbl, 0, sizeof(ex_tid_event_tbl));
	memset((u8 *)&ex_fail_data, 0, sizeof(ex_fail_data));
	memset((u8 *)&s_id0g8_info, 0, sizeof(s_id0g8_info));
	s_id0g8_2nd_note = 0;

	/* リカバリに使用している */
	memset((u8 *)&ex_cline_status_tbl.event_id_sub[0], 0, sizeof(ex_cline_status_tbl.event_id_sub));
	memset((u8 *)&ex_cline_status_tbl.event_id[0], 0, sizeof(ex_cline_status_tbl.event_id));
	ex_cline_status_tbl.event_cnt = 0;
	ex_cline_status_tbl.Tid = 0;
	ex_cline_status_tbl.NextTid = 0;
	#else
	u16 back_accept_disable=0;

	// TIDイベント関連を初期化, 起動時も初期化
	memset((u8 *)&ex_tid_event_tbl, 0, sizeof(ex_tid_event_tbl));
	memset((u8 *)&ex_fail_data, 0, sizeof(ex_fail_data));
	memset((u8 *)&s_id0g8_info, 0, sizeof(s_id0g8_info));
	s_id0g8_2nd_note = 0;

	back_accept_disable = ex_cline_status_tbl.accept_disable;	/* 初期化しないので退避 */
	memset((u8 *)&ex_cline_status_tbl, 0, sizeof(ex_cline_status_tbl));
	/* 起動時設定に戻す */
	ex_cline_status_tbl.protocol_select = PROTOCOL_SELECT_ID0G8;
	ex_cline_status_tbl.if_select = IF_SELECT_USB;
	ex_cline_status_tbl.dipsw_disable = _id0g8_dipsw_disable();
	ex_cline_status_tbl.accept_disable = back_accept_disable;
	#endif

}


void _id0g8_tid_decrease(void)
{
	if(ex_cline_status_tbl.Tid != 0)
	{
		ex_cline_status_tbl.Tid--;
	}
	else
	{
		ex_cline_status_tbl.Tid = 254;
	}

	if(ex_cline_status_tbl.NextTid != 0)
	{
		ex_cline_status_tbl.NextTid--;
	}
	else
	{
		ex_cline_status_tbl.NextTid = 254;
	}
}


void _id0g8_tid_recovery(void)	/*2020-12-15 */
{
	if( ( ex_cline_status_tbl.event_id[0] == ID0G8_EVT_NOTE_TICKET_STATUS && ex_cline_status_tbl.event_id_sub[0] == NOTE_TICKET_STATUS_ACCEPTED )
	||
		( ex_cline_status_tbl.event_id[0] == ID0G8_EVT_NOTE_VALIDATED || ex_cline_status_tbl.event_id[0] == ID0G8_EVT_TICKET_VALIDATED )
	)
	{
	/* 電源OFF前にAcceptedの為にTIDを更新しているので、TIDを1つ戻していく、送る時に更新する、主はex_tid_event_tbl.NextTid	*/
		_id0g8_tid_decrease();
	}
}

/* 2021-01-13 */
/* リカバリでEscrowを送信するか判断*/

void _id0g8_escrow_recover(void)
{
	u8 cnt;
	u8 data_ng=0;

	if( ex_cline_status_tbl.escrow_code != BAR_INDX )
	{
	/* Escrow not accepted ACK*/
	/* bill */
		ex_0g8_recovery = RECOVER_BILL_ESCROW;
		_id0g8_tid_recovery();
	#if defined(ID0G8_LOG)
		ex_iwasaki_test_powerup[4] = 1;
	#endif
	}
	else
	{
	/* Escrow not accepted ACK*/
	/* ticket */
		/* check appropriate data*/
		for( cnt = 0; cnt < 16; cnt++ )
		{
			if( ex_cline_status_tbl.ex_Barcode_recovery_0g8[cnt] >= 0x30 && ex_cline_status_tbl.ex_Barcode_recovery_0g8[cnt] <= 0x39 )
			{
			/* ok*/
			}
			else
			{
			/* clear */
				data_ng = 1;
				break;
			}
		}
		if(data_ng == 0)
		{
			ex_0g8_recovery = RECOVER_TICKET_ESCROW;
			_id0g8_tid_recovery();
		#if defined(ID0G8_LOG)
			ex_iwasaki_test_powerup[4] = 2;
		#endif
		}
		else
		{
			//data ng
			_id0g8_denomi_clear();
		}
	}
}

static u8 check_last_sequence_id0g8(void)
{
	u8 result = 0;	/* reject */

    switch( ex_recovery_info.step )
    {
		case RECOVERY_STEP_NON: 
        case RECOVERY_STEP_ACCEPT:
	        /* Powerup with bill in acceptor */
			result = 0;	/* reject*/
            break;

        case RECOVERY_STEP_ESCORW:
		case RECOVERY_STEP_APB_IN:		/* 使用していないが今後の為 */
		case RECOVERY_STEP_APB_OUT:
		case RECOVERY_STEP_STACKING:	//押し込み開始直前
		case RECOVERY_STEP_STACKING_BILL_IN_BOX:
		case RECOVERY_STEP_ICB_ACCEPT: //2023-04-28
        case RECOVERY_STEP_VEND:
		/* ivizion2で新規追加 */
		case RECOVERY_STEP_STACK: //搬送完了
		case RECOVERY_STEP_EXIT:

			result = 1;	/* stack */
			break;
		default:
	        /* Powerup with bill in acceptor */
			result = 0;	/* reject*/
            break;
    }

	return(result);

}


/*-------------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------------------------*/
/*******************************
        line_task
 *******************************/
u8 id0g8_main(void) //main1
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	INT iStat;

	ex_usb_hid_disconect_0g8 = 0;
	ex_usb_dfu_disconect_0g8 = 0;

	#if defined(ID0G8_LOG)
	debug_line_count = 0;
	#endif

	OperationUSBConnect();
	_dline_initialize_flash();
	_id0g8_initial_msg_proc();

	while (1)
	{

		if( ex_usb_hid_disconect_0g8 == 1 )
		{
			if(ex_cline_status_tbl.line_task_mode >= ID0G8_MODE_DISABLE_REJECT
			&& ex_cline_status_tbl.line_task_mode < ID0G8_MODE_WAIT_ACCEPTED_ACK ) 
			{

			}
			else
			{
				_soft_reset();
			}
		}
		else
		{

		}

		if(
		(( !l_ausID0G8_tx_size ) && ( s_id0g8_info.event_occur ))

		|| (( !l_ausID0G8_tx_size ) && ( s_id0g8_info.res_occur ))
		|| (( !l_ausID0G8_tx_size ) && ( s_id0g8_info.res_occur_crc ))
		)
		{
			_id0g8_make_event();
		}
		if( l_ausID0G8_tx_size )
		{

		//#ifdef _ENABLE_JDL	ID0G8_JDL
			jdl_comm_tx_pkt(&l_aulID0G8_tx_buff[0], (u8)l_ausID0G8_tx_size );
		/* _ENABLE_JDL */

			com_hid_0g8_send(l_ausID0G8_tx_size, l_aulID0G8_tx_buff);
			l_ausID0G8_tx_size = 0;
		}
		ercd = trcv_mbx(ID_CLINE_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
		if (ercd == E_OK)
		{
		/* Receiving task message */
			memcpy(&cline_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(cline_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_cline_system_error(1, 3);
			}
			_id0g8_msg_proc();
		}

		if(DucDfuProgramModeFlag == PROGRAM_MODE_DFU_INI)
		{
			dly_tsk(10);
			com_hid_0g8_detach();

			iStat = com_dfu_0g8_Init2(COM_DFU_BUF_SIZE, l_aulID0G8_rx_buff, COM_DFU_BUF_SIZE, l_aulID0G8_tx_buff);

			Fid0g8_download_init();

			DucDfuProgramModeFlag = PROGRAM_MODE_APPLICATION;
			/* HID */
			com_hid_0g8_detach(); 		
			dly_tsk(10);
			iStat = com_hid_0g8_Init2(1);

		}
	}
}



void _id0g8_com_recover(u8 step)	/* 通信リカバリ	*/
{

	if(step==0)
	{
	/* 起動時に、通信リカバリが必要か判断する処理	*/
		ex_0g8_recovery_com = 0;

		if( ex_cline_status_tbl.event_cnt )
		{
			if( ex_cline_status_tbl.event_id[0] == ID0G8_EVT_NOTE_TICKET_STATUS )
			{
				if(ex_cline_status_tbl.event_id_sub[0] == NOTE_TICKET_STATUS_RETURNED)
				{
					ex_0g8_recovery_com = NOTE_TICKET_STATUS_RETURNED;
				}
				else if(ex_cline_status_tbl.event_id_sub[0] == NOTE_TICKET_STATUS_REJECTED)
				{
					ex_0g8_recovery_com = NOTE_TICKET_STATUS_REJECTED;
				}
				else if(ex_cline_status_tbl.event_id_sub[0] == NOTE_TICKET_STATUS_REMOVED)
				{
					ex_0g8_recovery_com = NOTE_TICKET_STATUS_REMOVED;
				}
			}
			else if(ex_cline_status_tbl.event_id[0] == ID0G8_EVT_NOTE_VALIDATED)
			{
				ex_0g8_recovery_com = ID0G8_EVT_NOTE_VALIDATED;	
			}
			else if(ex_cline_status_tbl.event_id[0] == ID0G8_EVT_TICKET_VALIDATED)
			{
				ex_0g8_recovery_com = ID0G8_EVT_TICKET_VALIDATED;				
			}
		}
	}
	else if(step==1)
	{
	/* 電源ONからの最初のイニシャル動作完了後の、通信リカバリとして送信データを設定する処理 */
	/* Escrow,Reject,Retrun,Remove　関係 		*/
	/* モードはRemovedにしてAck受信待ちにする	*/

		_id0g8_tid_decrease();	/* 電源OFF前にイベント番号が1進んでいるので戻す */

		if( ex_0g8_recovery_com == ID0G8_EVT_NOTE_VALIDATED )
		{

			_id0g8_set_event(ID0G8_EVT_NOTE_VALIDATED, 0);

			ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_RETURNED;
		}
		else if( ex_0g8_recovery_com == ID0G8_EVT_TICKET_VALIDATED)
		{

			_id0g8_set_event(ID0G8_EVT_TICKET_VALIDATED, 0);

			ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_RETURNED;
		}
		else if( ex_0g8_recovery_com == NOTE_TICKET_STATUS_RETURNED)
		{
			ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_RETURNED;
		}
		else if( ex_0g8_recovery_com == NOTE_TICKET_STATUS_REJECTED)
		{
			ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_REJECTED;
		}

		/* Reject or Retrun */
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status );

		/* Removed */
		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_REMOVED;
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status );

		_id0g8_intr_mode_sub( ID0G8_MODE_WAIT_REMOVED_ACK, 0);

	}
}



void _id0g8_initial_msg_proc(void) //power1
{

	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u8 init_seq = 1;
	u8 exit_recovery = 0;

#if defined(ID0G8_LOG)
	ex_iwasaki_test_powerup[0] = ex_recovery_info.step;

	memset((u8 *)&ex_iwasaki_test[0], 0, sizeof(ex_iwasaki_test));

#endif

	//リカバリに使用してないので初期化  */
	ex_cline_status_tbl.reject_code = 0;
	ex_cline_status_tbl.error_code = 0;
	ex_cline_status_tbl.dipsw_disable = 0;
	ex_cline_status_tbl.accept_disable = 0;
	ex_cline_status_tbl.escrow_code_2nd_note = 0;

	/* 起動時専用設定 */
	ex_cline_status_tbl.dipsw_disable = _id0g8_dipsw_disable();
	ex_cline_status_tbl.accept_disable = 0x0001; 	/* Resetでも維持 */
	
	/* nvm resetでもreset */
	memset((u8 *)&ex_tid_event_tbl, 0, sizeof(ex_tid_event_tbl));
	memset((u8 *)&ex_fail_data, 0, sizeof(ex_fail_data));
	memset((u8 *)&s_id0g8_info, 0, sizeof(s_id0g8_info));
	s_id0g8_2nd_note = 0;

	s_id0g8_powerup_box_disconnect = 0;

	#if defined(ID0G8_LOG)
	if( ex_iwasaki_test[28] == 1 )
	{
		_id0g8_init_nvm_info();
	}
	#endif

	/* --------------------------------------------------------------------- */
	/* Wait TMSG_CLINE_INITIAL_RSP Message                                    */
	/* --------------------------------------------------------------------- */
	while (init_seq != 0)
	{
		ercd = rcv_mbx(ID_CLINE_MBX, (T_MSG **)&tmsg_pt);
		if (ercd == E_OK)
		{
			memcpy(&cline_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(cline_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_cline_system_error(1, 4);
			}
			switch (init_seq)
			{
			case 1:
				if ((cline_msg.tmsg_code == TMSG_CLINE_INITIAL_RSP)
				 && !(cline_msg.arg1 & OPERATING_MODE_TEST))
				{
					if (cline_msg.arg2 == TMSG_SUB_SUCCESS)
					{
					/* Set Powerup status */// 若干異なるがとりあえず、このまま
						if (cline_msg.arg3 == BILL_IN_ACCEPTOR)
						{
							#if 1 //2023-04-28 ICBカウント関係に矛盾がでるので、無効にする
							//Vend関係はラインタスク関係で完結するので、
							//main側がBILL_IN_ACCEPTORで
							//ラインタスク側がPOWERUP_STAT_RECOVER_STACK
							//でも問題ないが、
							//ICBカウント関係は
							//はex_recovery_infoとex_cline_status_tbl.escrow_codeを使用しているので、
							//上記2つのフラグをクリアするタイミングにより今後不具合が発生する可能性ある為
							//ex_recovery_info.stepよりcline_msg.arg3 == BILL_IN_ACCEPTORを優先する
							if(0){
							#else
							/* stepがVend,Stacking(押し込み直前)まで進んでいる場合は、Bill in acceptorは2枚目の紙幣 */
							if( (ex_recovery_info.step == RECOVERY_STEP_VEND)    ||
								(ex_recovery_info.step == RECOVERY_STEP_STACKING_BILL_IN_BOX) ||
								(ex_recovery_info.step == RECOVERY_STEP_STACKING) )
							{
							#endif	
							#if defined(ID0G8_LOG)
								ex_iwasaki_test_powerup[1] = 1;
							#endif
								_id0g8_tid_recovery();			/* Vend上がる*/

								/* Powerup with bill in stacker */
								s_id0g8_powerup_stat = POWERUP_STAT_RECOVER_STACK;
								_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP_BILLIN_SK, 0);
								init_seq = 0; /* 処理を抜ける	*/
							}
							else
							{
							#if defined(ID0G8_LOG)
								ex_iwasaki_test_powerup[1] = 2;
							#endif
								if( ex_recovery_info.step == RECOVERY_STEP_ESCORW_WAIT_ACK )/* 2021-01-13 */
								{
								/* Escrow後, Rejected, Removed	*/
									_id0g8_escrow_recover();	/* check need send escrow */
								}
								else
								{

								}

								/* Powerup with bill in acceptor */
								s_id0g8_powerup_stat = POWERUP_STAT_REJECT;
								_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP_BILLIN_AT, 0);
								init_seq = 0;	/* 処理を抜ける	*/
							}
						}
						else if (cline_msg.arg3 == BILL_IN_STACKER)
						{
						/* Powerup with bill in stacker */
							exit_recovery = check_last_sequence_id0g8();
							// reject
							if( exit_recovery == 0)
							{
							#if defined(ID0G8_LOG)
								ex_iwasaki_test_powerup[1] = 3;
							#endif
#if 1
								/* Search bill (RECOVERY_STEP_NON) */
								init_seq++;
								_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SEARCH_BILL_REQ, SEARCH_TYPE_WINDOW, 0, 0, 0);
								break;
#else
								/* Vend上げない */
								s_id0g8_powerup_stat = POWERUP_STAT_FEED_STACK;
								_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP_BILLIN_SK, 0);
#endif
							}
							// stack
							else
							{
							#if defined(ID0G8_LOG)
								ex_iwasaki_test_powerup[1] = 4;
							#endif
								_id0g8_tid_recovery();	/* Vend上げる*/

								/* Powerup with bill in stacker */
								s_id0g8_powerup_stat = POWERUP_STAT_RECOVER_FEED_STACK;
								_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP_BILLIN_SK, 0);
							}
							init_seq = 0;	/* 処理を抜ける	*/
						}
						else
						{
							// 紙無し
							if( ex_recovery_info.step == RECOVERY_STEP_VEND
							||
							( (ex_recovery_info.step == RECOVERY_STEP_STACKING) && (!SENSOR_PUSHER_HOME ) ) /* 押し込み直前 */
							||
							(ex_recovery_info.step == RECOVERY_STEP_ICB_ACCEPT) || //2023-04-28
							(ex_recovery_info.step == RECOVERY_STEP_STACKING_BILL_IN_BOX)
							 )
							{
							#if defined(ID0G8_LOG)
								ex_iwasaki_test_powerup[1] = 5;
							#endif
								_id0g8_tid_recovery();	/* Vend上げる*/

								/* Powerup with bill in stacker */
								s_id0g8_powerup_stat = POWERUP_STAT_RECOVER_STACK;
								_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP_BILLIN_SK, 0);
								init_seq = 0; /* 処理を抜ける */
							}
							else if( ex_recovery_info.step >= RECOVERY_STEP_ENABLE_NOTE_PATH_CLER )	/* UBAはEscrow位置にある紙幣がなくなった場合もNote Pathにしているので*/
							{
								/* フラグにEscrow状態がないので、Escrow直前のフラグを使用する */
							#if defined(ID0G8_LOG)
								ex_iwasaki_test_powerup[1] = 6;
							#endif
								/* Search bill */
								init_seq++;
								_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SEARCH_BILL_REQ, 0, 0, 0, 0);
							}
							else
							{
							#if defined(ID0G8_LOG)
								ex_iwasaki_test_powerup[1] = 7;
							#endif
								if( ex_recovery_info.step == RECOVERY_STEP_ESCORW_WAIT_ACK )/* 2021-01-13 */
								{
								/* Escrow後, Note path clear	*/
									_id0g8_escrow_recover();	/* check need send escrow */
								}
								else
								{
									_id0g8_com_recover(0); //通信リカバリを入れる
								}

								/* Powerup */
								s_id0g8_powerup_stat = POWERUP_STAT_NORMAL;
								_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP, 0);
								init_seq = 0; /* 処理を抜ける */
							}
						}
					}
					else if (cline_msg.arg2 == TMSG_SUB_ALARM)
					{
					/* Set Error status */
#if 1
						if(cline_msg.arg3 == ALARM_CODE_FRAM )
						{
							s_id0g8_powerup_stat = POWERUP_STAT_NORMAL;
							ex_cline_status_tbl.error_code = cline_msg.arg3;

							_id0g8_intr_mode_sub(ID0G8_MODE_SYSTEM_ERROR,0); //現状はエラーとして存在しない

							#if defined(ID0G8_LOG)
							ex_iwasaki_test_powerup[1] = 9;
							#endif
							init_seq = 3; /* 致命的はエラーなので無限ループさせる */
						}
						else
						{

							/* Powerup */
							s_id0g8_powerup_stat = POWERUP_STAT_NORMAL;
							_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP, 0);
							init_seq = 0; /* 処理を抜ける */
						}
#else
						s_id0g8_powerup_stat = POWERUP_STAT_NORMAL;
						ex_cline_status_tbl.error_code = cline_msg.arg3;

						_id0g8_intr_mode_sub(ID0G8_MODE_SYSTEM_ERROR,0); //現状はエラーとして存在しない	
						init_seq = 0;

						#if defined(ID0G8_LOG)
						ex_iwasaki_test_powerup[1] = 9;
						#endif
						init_seq = 3; /* 致命的はエラーなので無限ループさせる */
#endif
					}
					else
					{
						#if defined(ID0G8_LOG)
						ex_iwasaki_test_powerup[1] = 10;
						#endif

						/* system error ? */
						_cline_system_error(0, 5);
					}
				}
				break;
			case 2:
				if ((cline_msg.tmsg_code == TMSG_CLINE_SEARCH_BILL_RSP)
				 || (cline_msg.tmsg_code == TMSG_CLINE_SEARCH_WINDOW_RSP))
				{
					if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
					{
					/* Set Powerup status */
						if (cline_msg.arg2 == BILL_IN_ACCEPTOR)
						{
						/* Powerup with bill in acceptor */
							s_id0g8_powerup_stat = POWERUP_STAT_RECOVER_SEARCH_REJECT;

							_id0g8_intr_mode_sub(ID0G8_MODE_POWERUP_BILLIN_AT,0);							
							init_seq = 0;
						#if defined(ID0G8_LOG)
							ex_iwasaki_test_powerup[2] = 1;
						#endif

						}
						else if (cline_msg.arg2 == BILL_IN_STACKER)
						{
						/* Powerup with bill in stacker */
							exit_recovery = check_last_sequence_id0g8();
							if( exit_recovery == 0)
							{
							#if defined(ID0G8_LOG)
							ex_iwasaki_test_powerup[2] = 10;
							#endif
								/* Vend上げない */
								s_id0g8_powerup_stat = POWERUP_STAT_FEED_STACK;
								_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP_BILLIN_SK, 0);
							}
							else
							{
							#if defined(ID0G8_LOG)
							ex_iwasaki_test_powerup[2] = 11;
							#endif
								_id0g8_tid_recovery();	/* Vend上げる*/

								/* Powerup with bill in stacker */
								s_id0g8_powerup_stat = POWERUP_STAT_RECOVER_FEED_STACK;
								_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP_BILLIN_SK, 0);
							}
							init_seq = 0;	/* 処理を抜ける	*/
						}
						else
						{
						/* Powerup */
						/* 紙幣がHead,BOXにない*/
							/* Stack要求後かつStacker Homeでない場合は押し込み中 */
							/* なので紙幣見つからない */
							if( ((ex_recovery_info.step == RECOVERY_STEP_STACKING) && (!SENSOR_PUSHER_HOME )  /*(0 == SENSOR_STACKER_HOME )*/)
							||
								(ex_recovery_info.step == RECOVERY_STEP_ICB_ACCEPT) || //2023-04-28
								(ex_recovery_info.step == RECOVERY_STEP_STACKING_BILL_IN_BOX))
							{
							#if defined(ID0G8_LOG)
								ex_iwasaki_test_powerup[2] = 12;
							#endif
								_id0g8_tid_recovery();	/* Vend上げる*/

								s_id0g8_powerup_stat = POWERUP_STAT_RECOVER_SEARCH_STACK;
								_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP_BILLIN_SK, 0);
							}
							else
							{
							#if defined(ID0G8_LOG)
								ex_iwasaki_test_powerup[2] = 13;
							#endif
								/* ivizionはmainからのメッセージ処理が若干違いう */
								/* 紙幣が見つからない場合、succesではなく、エラーメッセージでくるので */
								/* ここのGLIエラーは削除	*/
								s_id0g8_powerup_stat = POWERUP_STAT_NORMAL;
								_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP, 0);
							}
							init_seq = 0;	/* 処理を抜ける	*/
						}
					}
					else if ((cline_msg.arg1 == TMSG_SUB_ALARM)
					 && (cline_msg.arg2 == ALARM_CODE_FEED_LOST_BILL))
					{
						#if defined(ID0G8_LOG)
						ex_iwasaki_test_powerup[2] = 5;
						#endif

						ex_0g8_recovery = RECOVER_GLI_ERROR;	/* for GLI error */

						/* Powerup */
						s_id0g8_powerup_stat = POWERUP_STAT_RECOVER_SEARCH_NON;
						_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP, 0);

						init_seq = 0;

					}
					else if (cline_msg.arg1 == TMSG_SUB_ALARM)
					{
					/* Set Error status */
						s_id0g8_powerup_stat = POWERUP_STAT_NORMAL;

						ex_cline_status_tbl.error_code = cline_msg.arg2;
						_id0g8_intr_mode_sub(ID0G8_MODE_POWERUP_ERROR,0);	

						init_seq = 0;
						#if defined(ID0G8_LOG)
						ex_iwasaki_test_powerup[2] = 6;
						#endif
					}
					else
					{
						#if defined(ID0G8_LOG)
						ex_iwasaki_test_powerup[2] = 7;
						#endif
						/* system error ? */
						_cline_system_error(0, 6);
					}
				}
				break;
			case 3:
			/* 致命的はエラーなので無限ループさせる */
				OSW_TSK_sleep(200);
				break;

			default:
				/* system error ? */
				_cline_system_error(0, 7);
				break;
			}
		}
	}

	#if defined(ID0G8_LOG)
	ex_iwasaki_test_powerup[3] = s_id0g8_powerup_stat;
	#endif


	/* --------------------------------------------------------------------- */
	/* Initialize line status table                                          */
	/* --------------------------------------------------------------------- */
	INT iStat;

    /* 受信データバッファ初期化 */
	memset((u8 *)&l_aulID0G8_rx_buff[0], 0x00, COM_DFU_BUF_SIZE);
    /* 送信データバッファ初期化 */
	memset((u8 *)&l_aulID0G8_tx_buff[0], 0x00, COM_DFU_BUF_SIZE);
    /* 送信データサイズ初期化 */
	l_ausID0G8_tx_size = 0;
	DucDfuProgramModeFlag = 0;

	/* ID-0G8 *//* 移植 start */
	if ((ex_cline_status_tbl.line_task_mode == ID0G8_MODE_POWERUP_ERROR)
	/* || (ex_cline_status_tbl.line_task_mode == ID0G8_MODE_SYSTEM_ERROR) */
	){
		if (ex_recovery_info.step != RECOVERY_STEP_NON)
		{
			set_recovery_step(RECOVERY_STEP_NON);
		}
	}
	else
	{
		ex_cline_status_tbl.error_code = 0;	//use
	}


	if(ex_0g8_recovery_com == 0)
	{
		_id0g8_transfer_event_clear();	/* 通信リカバリをしない、イベントバッファをクリア */
	}


	/* Start DipSW reading interval */
	_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DIPSW_READ, WAIT_TIME_DIPSW_READ, 0, 0);

	/* write status table */
	write_status_table();


	/* --------------------------------------------------------------------- */
	/* COMMNUCATION LOG INIT                                                 */
	/* --------------------------------------------------------------------- */
	//#ifdef _ENABLE_JDL	ID0G8_JDL
	/* ID-0G8のHIDモードはポールに該当するのがないので、ログの省略は必要ない、DUFモードにはステータスリクエストがあるが、HIDモードでは使用できない事と*/
	/* DUFモードはダウンロード用なので、ログ機能は使用しない */

	jdl_comm_init(PROTOCOL_SELECT_ID0G8, 0, 0x00);	/* ポールがないので0x00とする, スキップも必要ないので0とする */
	/* _ENABLE_JDL */


	/* --------------------------------------------------------------------- */
	/* Start id-0g8 communication                                            */
	/* --------------------------------------------------------------------- */
	/* Initialize USB */
	iStat = com_hid_0g8_Init2(0);

	
	act_tsk(ID_USB2_CB_TASK);


}


/*********************************************************************//**
 * @brief Command receiving procedure
 * @param[in]	_id0g8_rx_buff.cmd : command code
 * @return 		None
 **********************************************************************/
void _id0g8_cmd_proc(void) //cmd1
{
	switch (l_aulID0G8_rx_buff[0])
	{
    //Detachコマンド処理は、割り込みで直接フラグを立てる
    //理由は、Host側が先にUSB Reset処理を行う場合があるので、DUFモード用のPID切替が間に合わない可能性があるので、
    //他のコマンドは、通常通り_com_hid_0g8_cb_RecvData2で処理する
	//		_id0g8_dfu_detach_cmd_proc();
	//		write_status_table();
	//		break;
	case	ID0G8_CMD_ACK:
			_id0g8_ack_cmd_proc();
			write_status_table();
			break;
	case	ID0G8_CMD_ENABLE:
			_id0g8_enable_cmd_proc();
			write_status_table();
			break;
	case	ID0G8_CMD_DISABLE:
			_id0g8_disable_cmd_proc();
			write_status_table();
			break;
	case	ID0G8_CMD_SELF_TEST:
			_id0g8_self_test_cmd_proc();
			write_status_table();
			break;
	case	ID0G8_CMD_GAT_REPORT:
			_id0g8_gat_report_cmd_proc();
			write_status_table();
			break;
	case	ID0G8_CMD_CALC_CRC:
			_id0g8_calc_crc_cmd_proc();
			write_status_table();
			break;
	case	ID0G8_CMD_NUM_OF_NOTE_DATA:
			_id0g8_num_of_note_data_cmd_proc();//test
			write_status_table();
			break;
	case	ID0G8_CMD_READ_NOTE_DATA_TABLE:
			_id0g8_read_note_data_cmd_proc();//test
			write_status_table();
			break;
	case	ID0G8_CMD_EXTEND_TIMEOUT:
			_id0g8_extend_timeout_cmd_proc();
			write_status_table();
			break;
	case	ID0G8_CMD_ACCEPT_NOTE_TICKET:
			_id0g8_accept_note_ticket_cmd_proc();
			write_status_table();
			break;
	case	ID0G8_CMD_RETURN_NOTE_TICKET:
			_id0g8_return_note_ticket_cmd_proc();
			write_status_table();
			break;
	}

}


/*********************************************************************//**
 * @brief ACK receiving procedure (after vend valid)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
//static void _id0g8_dfu_detach_cmd_proc(void)	//not use
//{
//	com_hid_0g8_detach();

//	DucDfuProgramModeFlag = PROGRAM_MODE_DFU_INI;
//	dly_tsk(10);
//}

/*********************************************************************//**
 * @brief ACK receiving procedure (after vend valid)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _id0g8_ack_cmd_proc(void)// ok 移植
{

	s_id0g8_info.wait_event_response = false;
	
	if(ex_tid_event_tbl.TidEvtStatus == TID_EVT_ST_WAITING_FOR_ACK)
	{
		// TIDイベント承認 resync == 0
		if( l_aulID0G8_rx_buff[1] == 0x00 )
		{
			// TID番号が一致
			if(l_aulID0G8_rx_buff[2] == ex_cline_status_tbl.Tid)
			{
				ex_tid_event_tbl.TidEvtStatus = 0;
				// イベント更新
				_id0g8_transfer_next_event();
			}
			// TID番号が不一致
			else
			{
				_id0g8_set_same_tid_event(ex_cline_status_tbl.Tid);	// 同じTIDで再送
				return;
			}
		}
		// TIDイベント未承認 resync == 1
		else
		{
			_id0g8_set_same_tid_event(l_aulID0G8_rx_buff[2]);
			
			if(l_aulID0G8_rx_buff[2] != 255)
			{
				ex_cline_status_tbl.NextTid = l_aulID0G8_rx_buff[2] + 1;
			}
			else
			{
				ex_cline_status_tbl.NextTid = 0;
			}
			return;
		}
	}
	else
	{
		return;
	}
	
	if( ex_cline_status_tbl.line_task_mode == ID0G8_MODE_ESCROW_WAIT_ACK )
	{
		/* Escrowに対するAck受信以降をNot path clear条件にする */
		set_recovery_step(RECOVERY_STEP_ENABLE_NOTE_PATH_CLER);

		//2023-04-28 リカバリのICBカウントが常に$1になるので、廃止 _id0g8_denomi_clear();

		_id0g8_intr_mode_sub( ID0G8_MODE_ESCROW, 0);
	}
	else if( ex_cline_status_tbl.line_task_mode == ID0G8_MODE_WAIT_ACCEPTED_ACK )
	{
		set_recovery_step(RECOVERY_STEP_NON);
		
		ex_tid_event_tbl.note_ticket_status = 0x00;

		if(ex_cline_status_tbl.error_code == ALARM_CODE_STACKER_FULL)
		{
			_set_id0g8_alarm(ID0G8_MODE_ERROR, ALARM_CODE_STACKER_FULL);
		}
		else if( _is_id0g8_enable() )
		{
			_id0g8_intr_mode_sub( ID0G8_MODE_ENABLE,  0);

		}
		else
		{
			_id0g8_intr_mode_sub( ID0G8_MODE_DISABLE, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
		}
	}
	else if( ex_cline_status_tbl.line_task_mode == ID0G8_MODE_WAIT_REMOVED_ACK )
	{
		if(!(_id0g8_check_remaining_events()) )
		{
			set_recovery_step(RECOVERY_STEP_NON);
			/* リカバリVend用変数クリア */
			_id0g8_denomi_clear();

			if( _is_id0g8_enable() )
			{
				_id0g8_intr_mode_sub( ID0G8_MODE_ENABLE, 0);

			}
			else
			{
				_id0g8_intr_mode_sub( ID0G8_MODE_DISABLE, 0);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

			}
		}
	}
}

/*********************************************************************//**
 * @brief Enable Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _id0g8_enable_cmd_proc(void)//ok　移植
{
	// POWER UPモードは動作しない
	if ((ex_cline_status_tbl.line_task_mode != ID0G8_MODE_POWERUP)           &&
		(ex_cline_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_BILLIN_AT) &&
		(ex_cline_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_BILLIN_SK) &&
		(ex_cline_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_ERROR))
	{
		ex_cline_status_tbl.accept_disable = 0x0000;
		
		// DISABLEモード
		if (((ex_cline_status_tbl.line_task_mode == ID0G8_MODE_DISABLE) ||
			/* 2019-10-29 */
			 (ex_cline_status_tbl.line_task_mode == ID0G8_MODE_DISABLE_REJECT))
		&&  (_is_id0g8_enable()))
		{
			if(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_DISABLE)
			{
				_id0g8_intr_mode_sub( ID0G8_MODE_ENABLE, 0);

			}
			else
			{
				_set_id0g8_reject(ID0G8_MODE_ENABLE_REJECT, (u16)cline_msg.arg2);
			}
		}
		
		else if( ex_cline_status_tbl.line_task_mode == ID0G8_MODE_ERROR )	/* 2020-11-11 */
		{
		/* 現在のエラー状態を返すがどうするのか?*/
		/* Stacker系*/
		/* Note/Ticket系*/
		/* Failure系*/
			_is_id0g8_error_sts(ex_cline_status_tbl.error_code, 0);
			return;
		}
		_id0g8_set_event(ID0G8_EVT_DEVICE_STATE, 0);
	}
}

/*********************************************************************//**
 * @brief Disable Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _id0g8_disable_cmd_proc(void)//ok　移植
{
	u16 before_mode;

	ex_cline_status_tbl.accept_disable = 0x0001;
	before_mode = ex_cline_status_tbl.line_task_mode;

	// POWER UPモード
	if( (ex_cline_status_tbl.line_task_mode == ID0G8_MODE_POWERUP)           ||
		(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_POWERUP_BILLIN_AT) ||
		(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_POWERUP_BILLIN_SK) ||
		(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_POWERUP_ERROR))
	{
		if(ex_cline_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_BILLIN_SK)
		{
			ex_cline_status_tbl.escrow_code = 0x0000;//2023-04-28 ID-003に合わせた
		}

		_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP_INITIAL, 0);
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
	}
	// ENABLEモード
	else if (((ex_cline_status_tbl.line_task_mode == ID0G8_MODE_ENABLE) ||
			/* 2019-10-29 */
			  (ex_cline_status_tbl.line_task_mode == ID0G8_MODE_ENABLE_REJECT))
		&&  !(_is_id0g8_enable()))
	{
		if(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_ENABLE)
		{
			_id0g8_intr_mode_sub( ID0G8_MODE_DISABLE, 0);
		}
		else
		{
			_id0g8_intr_mode_sub( ID0G8_MODE_DISABLE_REJECT, 0);
		}
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

	}
	// ACCEPTモード	 // /* Escrow通知後はReturned,Escrow通知前はRejected*/
	else if( (ex_cline_status_tbl.line_task_mode == ID0G8_MODE_ACCEPT)
		  && (ex_cline_status_tbl.accept_disable))
	{
		// REJECTした後にdisableになる？
		_set_id0g8_reject(ID0G8_MODE_REJECT_WAIT_ACCEPT_RSP, REJECT_CODE_INHIBIT);
//		_set_id0g8_reject(ID0G8_MODE_RETURN, REJECT_CODE_RETURN);	/* UBAと同様にDisableコマンドによる返却はReturned*/

	}
	/* 元々ESCROW状態でpoll受信前でも受け付けていたからそのままにする。 */
	/* 今の処理ではpoll待ちは継続 */
	// ACCEPTモード
	else if (
			( 
			(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_ESCROW_WAIT_ACK)	/* 2020-11-30 */
			||
			(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_ESCROW)
			)
			&& (ex_cline_status_tbl.accept_disable))
	{
		// REJECTした後にdisableになる？
//		_set_id0g8_reject(ID0G8_MODE_REJECT, REJECT_CODE_INHIBIT);
		_set_id0g8_reject(ID0G8_MODE_RETURN, REJECT_CODE_RETURN);	/* UBAと同様にDisableコマンドによる返却はReturned*/
	}

	_id0g8_set_event(ID0G8_EVT_DEVICE_STATE, 0);
#if 1
	if(s_id0g8_powerup_box_disconnect != 0)
	{
		ex_fail_data.fail_type = 0;
		ex_fail_data.diagnostics = 0;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
		ex_tid_event_tbl.stacker_status = STACKER_STATUS_DISCONNECT;
		_id0g8_set_event(ID0G8_EVT_STACKER_STATUS, ex_tid_event_tbl.stacker_status);
		s_id0g8_powerup_box_disconnect = 0;
	}
#endif
}

/*********************************************************************//**
 * @brief Command receiving procedure
 * @param[in]	_id0g8_rx_buff.cmd : command code
 * @return 		None
 **********************************************************************/
static void _id0g8_self_test_cmd_proc(void) //ok　移植
{
	#if 1
	/* 仕様での受け付け条件は厳密には少し複雑、理由は */
	/* コマンドの受付条件は、Disable状態は受け付け、Enable状態は受付不可となるが */
	/* Stacker Status ErrorとNote Statsu Errorでエラー発生後のDisable,Enable遷移が事なる為 */
	/* 特にNote Statsu Error(Acceptor JAMなど)は、複雑 */

	/* Acceptor JAMなどはNote Statsuエラー系なので、エラー発生時に		*/
	/* エラー発生後も以前の、Enable, Disableを保持するという仕様である為*/
	/* EnableからのAcceptor JAMでは、Enableである為、Self testを受け付けない*/
	/* DisableからのAcceptor JAMでは、Disableである為、Self testを受け付ける*/

	/* Stackr JAMなどはStacker Statsuエラー系なので、エラー発生時に		*/
	/* Disableになるという仕様である為、シンプル						*/
	/* Stacker Status系のエラーは、Self testを受け付けない*/

	/* また、UBAはDisableからのSelf testを1度は受け付けるが、そのままイニシャルが完了しない場合 */
	/* エラーステータスも送信せず、その後のSelf testコマンドもデータステージでNAK処理している */
	/* 動作が複雑すぎるので、UBA500は完全には同じにしない*/


	/* 上記よりコマンド受け付け条件が複雑なので、仕様よりコマンド受け付け条件を広げ、下記のようにする 	*/
	/* Disableステータスと、Note Status系のエラーのみ受け付ける		*/
	if(1)	//デバッグ用に今は無条件

	#if 0
	if( (ex_cline_status_tbl.line_task_mode == ID0G8_MODE_DISABLE)           ||

		(
		(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_ERROR) 
		&&
		(ex_tid_event_tbl.stacker_status == 0x00 ) 
		&&
		(ex_fail_data.fail_type == 0x00 ) 

		)
	)
	#endif

	#else
	// POWER UPモードは動作しない
	if( (ex_line_status_tbl.line_task_mode != ID0G8_MODE_POWERUP)           &&
		(ex_line_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_BILLIN_AT) &&
		(ex_line_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_BILLIN_SK) &&
		(ex_line_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_ERROR))
	#endif
	{
		// NVMの消去
		if((l_aulID0G8_rx_buff[1] & 0x01) != 0)
		{
			_id0g8_init_nvm_info();
		}
		
		set_recovery_step(RECOVERY_STEP_NON);
		
		ex_self_test_0g8 = 1;
		_id0g8_intr_mode_sub( ID0G8_MODE_INITIAL, 0);
		
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
	}
}


/*********************************************************************//**
 * @brief Command receiving procedure
 * @param[in]	_id0g8_rx_buff.cmd : command code
 * @return 		None
 **********************************************************************/
static void _id0g8_gat_report_cmd_proc(void)
{

	#if 1
	/* ゲーム機はいろいろなタイミングでリクエストして来るので、無条件で受け付ける様にする */
	if(1)
	#else

    // POWER UPモードは動作しない
	if( (ex_line_status_tbl.line_task_mode != ID0G8_MODE_POWERUP)           &&
		(ex_line_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_BILLIN_AT) &&
		(ex_line_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_BILLIN_SK) &&
		(ex_line_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_ERROR))

	#endif

	{
    	// crc16を取りにいかなければならない
        ex_seed_crc16 = 0;
        /* crc16を0クリア */
        ex_rom_crc16  = 0;
        _cline_send_msg(ID_DISCRIMINATION_MBX, TMSG_SIGNATURE_REQ, SIGNATURE_CRC16, 0, 0, 0);
    }
}


/*********************************************************************//**
 * @brief Command receiving procedure
 * @param[in]	_id0g8_rx_buff.cmd : command code
 * @return 		None
 **********************************************************************/
static void _id0g8_calc_crc_cmd_proc(void)
{

	/* ゲーム機はいろいろなタイミングでリクエストして来るので、無条件で受け付ける様にする */
	if(1)
	{
    	/* Seed値の設定 */
		(*(((u8 *)&ex_seed_crc32) + 0)) = l_aulID0G8_rx_buff[1];
		(*(((u8 *)&ex_seed_crc32) + 1)) = l_aulID0G8_rx_buff[2];
		(*(((u8 *)&ex_seed_crc32) + 2)) = l_aulID0G8_rx_buff[3];
		(*(((u8 *)&ex_seed_crc32) + 3)) = l_aulID0G8_rx_buff[4];
        /* crc32を0クリア */
        ex_rom_crc32  = 0;        
        _cline_send_msg(ID_DISCRIMINATION_MBX, TMSG_SIGNATURE_REQ, SIGNATURE_CRC32, 0, 0, 0);

	}
}


/*********************************************************************//**
 * @brief Command receiving procedure
 * @param[in]	_id0g8_rx_buff.cmd : command code
 * @return 		None
 **********************************************************************/
static void _id0g8_num_of_note_data_cmd_proc(void)//ok
{
	// POWER UPモードは動作しない
	_id0g8_set_event(ID0G8_EVT_NUM_OF_NOTE_DATA, 0);
}


/*********************************************************************//**
 * @brief Command receiving procedure
 * @param[in]	_id0g8_rx_buff.cmd : command code
 * @return 		None
 **********************************************************************/
static void _id0g8_read_note_data_cmd_proc(void)//ok
{
	s_id0g8_note_data_cnt = 0;
	_id0g8_set_event(ID0G8_EVT_READ_NOTE_DATA_TABLE, 0);
}


/*********************************************************************//**
 * @brief Command receiving procedure
 * @param[in]	_id0g8_rx_buff.cmd : command code
 * @return 		None
 **********************************************************************/
static void _id0g8_extend_timeout_cmd_proc(void)//ok
{
	if (ex_cline_status_tbl.line_task_mode == ID0G8_MODE_ESCROW) //escrowに対するAck受信後
	{
		_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_HOLD1, 500, 0, 0);
	}
}


/*********************************************************************//**
 * @brief Command receiving procedure
 * @param[in]	_id0g8_rx_buff.cmd : command code
 * @return 		None
 **********************************************************************/
static void _id0g8_accept_note_ticket_cmd_proc(void) //ok
{
	if (ex_cline_status_tbl.line_task_mode == ID0G8_MODE_ESCROW) //escrowに対するAck受信後
	{
		_id0g8_intr_mode_sub( ID0G8_MODE_STACK, 0);
		
		set_recovery_step(RECOVERY_STEP_ESCORW);

		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, VEND_POSITION_1, 0, 0, 0);
	}
}


/*********************************************************************//**
 * @brief Command receiving procedure
 * @param[in]	_id0g8_rx_buff.cmd : command code
 * @return 		None
 **********************************************************************/
static void _id0g8_return_note_ticket_cmd_proc(void)//ok
{
	if (ex_cline_status_tbl.line_task_mode == ID0G8_MODE_ESCROW)
	{
		_set_id0g8_reject(ID0G8_MODE_RETURN, REJECT_CODE_RETURN);
	}
}


void _id0g8_signature_rsp_msg_proc(void)//ok 移植 _id0g8_signature_calc_crc32_or_gat_new
{
	if(cline_msg.arg2 == SIGNATURE_CRC16)
	{
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* GAT */
			_id0g8_make_signature_gat();
		}
		else
		{
			/* system error ? */
		//	_cline_system_error(0, 17);
		}
	}
	else if(cline_msg.arg2 == SIGNATURE_CRC32)
	{
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* CRC 32*/
			_id0g8_set_event(ID0G8_EVT_CRC_DATA, 0);
		}
		else
		{
			/* system error ? */
		//	_cline_system_error(0, 17);
		}
	}
}
/*********************************************************************//**
 * @brief TMSG
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_msg_proc(void) //seq1
{

	switch (cline_msg.tmsg_code)
	{
	case TMSG_USB2CB_CALLBACK_INFO:
		_id0g8_callback_msg_proc();
		break;
	case TMSG_CLINE_STATUS_INFO:
		#if defined(ID0G8_LOG)
		save_log_0g8(1);
		#endif

		_id0g8_status_info_msg_proc();//ok 移植
		write_status_table();
		break;
	case TMSG_CLINE_RESET_RSP:
		#if defined(ID0G8_LOG)
		save_log_0g8(1);
		#endif

		_id0g8_reset_rsp_msg_proc();//full 以外 ok 移植
		write_status_table();
		break;
	case TMSG_CLINE_DISABLE_RSP:
		#if defined(ID0G8_LOG)
		save_log_0g8(1);
		#endif

		_id0g8_disable_rsp_msg_proc();//ok 移植
		write_status_table();
		break;
	case TMSG_CLINE_ENABLE_RSP:
		#if defined(ID0G8_LOG)
		save_log_0g8(1);
		#endif

		_id0g8_enable_rsp_msg_proc();//ok 移植
		write_status_table();
		break;
	case TMSG_CLINE_ACCEPT_RSP:
		#if defined(ID0G8_LOG)
		save_log_0g8(1);
		#endif

		_id0g8_accept_rsp_msg_proc();//ok 移植 モードも確認済み
		write_status_table();
		break;
	case TMSG_CLINE_STACK_RSP:
		#if defined(ID0G8_LOG)
		save_log_0g8(1);
		#endif

		_id0g8_stack_rsp_msg_proc();//ok 移植  モードも確認済み
		write_status_table();
		break;
	case TMSG_CLINE_REJECT_RSP:
		#if defined(ID0G8_LOG)
		save_log_0g8(1);
		#endif

		_id0g8_reject_rsp_msg_proc();//ok 移植 モードも確認済み
		write_status_table();
		break;
	case TMSG_TIMER_TIMES_UP:
		_id0g8_times_up_msg_proc(); //ok 移植
		write_status_table();
		break;
	case TMSG_SIGNATURE_RSP:
		#if defined(ID0G8_LOG)
		save_log_0g8(1);
		#endif

		_id0g8_signature_rsp_msg_proc();
		write_status_table();
		break;
	case TMSG_FRAM_READ_RSP:
	case TMSG_FRAM_WRITE_RSP:
		// TODO:
		break;
	default:					/* other */
		break;
	}
}

/*********************************************************************//**
 * @brief UART1 callback message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_callback_msg_proc(void)
{
	switch (cline_msg.arg1)
	{
	case TMSG_SUB_CONNECT:
	// 0 configu後のUSB断
	// 1 configuまで完了
	// 2 configu前のbus reset発生(ラインモニタ上でのReset,通常のPCでも2発は来る)
		if(cline_msg.arg2 == 1)
		{
			/* connect */
			ex_usb_hid_disconect_0g8 = 2;
		}
		else if(cline_msg.arg2 == 0)
		{
			/* connect -> disconnect */
//			if(ex_usb_hid_disconect_0g8 == 2)
//			{
				/* connect -> disconnect */
				ex_usb_hid_disconect_0g8 = 1;
//			}
		}
		break;

	case TMSG_SUB_RECEIVE:
		_id0g8_callback_receive();
		break;
	case TMSG_SUB_EMPTY:
		_id0g8_callback_empty();
		break;
	default:					/* other */
		break;
	}
}
/*********************************************************************//**
 * @brief UART1 callback TXD message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_callback_empty(void)
{
}
/*********************************************************************//**
 * @brief UART1 callback RXD message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_callback_receive(void)
{
	u32 rcvSize;
	//rcvSize = com_hid_0g8_listen();
	rcvSize = com_hid_0g8_listen(l_aulID0G8_rx_buff);
	if (rcvSize != 0)
	{
	/* Receiving command */
		_id0g8_cmd_proc();
	}
}

/*********************************************************************//**
 * @brief Status infomation message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_status_info_msg_proc(void)//ok 移植
{

	//基本的にPower up以外はエラーのみ考慮すればよさそう
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID0G8_MODE_POWERUP:
	case ID0G8_MODE_POWERUP_BILLIN_AT:
	case ID0G8_MODE_POWERUP_BILLIN_SK:
	case ID0G8_MODE_POWERUP_ERROR:
		_id0g8_status_info_mode_powerup();	//割と処理がある ok 移植
		break;
	#if 0
	case ID0G8_MODE_ESCROW:
	case ID0G8_MODE_ESCROW_WAIT_CMD:
	case ID0G8_MODE_HOLD1:
	case ID0G8_MODE_HOLD2:
		_id0g8_status_info_mode_escrow(); //rejectとエラー処理
		break;
	case ID0G8_MODE_VEND:

	case ID0G8_MODE_STACKED:
		_id0g8_status_info_mode_vend(); //エラーのみ処理
		break;
	#endif

	case ID0G8_MODE_ERROR:
		_id0g8_status_info_mode_error(); //ok 移植
		break;

	default:					/* other */
		_id0g8_status_info_mode_def(); //エラーのみ処理 ok 移植
		break;
	}
}



/*********************************************************************//**
 * @brief Status infomation message receiving [Mode : power-up relate]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_status_info_mode_powerup(void) //移植 ok
{
	u8 vend_recovery = 0;

	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
		if (cline_msg.arg2 == BILL_IN_ACCEPTOR)
		{
		/* Powerup with bill in acceptor */
			if (ex_cline_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_BILLIN_AT)
			{
                /* 突然紙幣が現れたのでリジェクトする */
				s_id0g8_powerup_stat = POWERUP_STAT_REJECT;
				ex_cline_status_tbl.error_code = 0;
				_id0g8_intr_mode_sub(ID0G8_MODE_POWERUP_BILLIN_AT, 0);
			}
		}
		else if (cline_msg.arg2 == BILL_IN_STACKER)
		{
            /* 現在既にPowerup with bill in stackerの場合は何もしない */
			if (ex_cline_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_BILLIN_SK)
			{
				/* UBA10,iPROはスタックコマンド受信前は、EXITのみONでもVendは送信しない*/
				vend_recovery = check_last_sequence_id0g8();
				if( vend_recovery == 1)
				{
				/* Vend上げる */
					s_id0g8_powerup_stat = POWERUP_STAT_RECOVER_FEED_STACK;
				}
				else
				{
				/* Vend上げない */
					s_id0g8_powerup_stat = POWERUP_STAT_FEED_STACK;
				}
				ex_cline_status_tbl.error_code = 0;
				_id0g8_intr_mode_sub(ID0G8_MODE_POWERUP_BILLIN_SK, 0);
			}
		}
		else
		{
			if ((ex_recovery_info.step >= RECOVERY_STEP_STACKING)
				&& (ex_recovery_info.step <= RECOVERY_STEP_VEND))
			{
			/* 押し込み位置までの搬送後なので、紙幣が見つからない場合もありえる*/
			/* Powerup with bill in stacker */
				if (ex_cline_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_BILLIN_SK)
				{
					s_id0g8_powerup_stat = POWERUP_STAT_RECOVER_STACK;

					ex_cline_status_tbl.error_code = 0;
					_id0g8_intr_mode_sub(ID0G8_MODE_POWERUP_BILLIN_SK, 0);
				}
			}
			else
			{
			/* Powerup */
				if (ex_cline_status_tbl.line_task_mode != ID0G8_MODE_POWERUP)
				{
					s_id0g8_powerup_stat = POWERUP_STAT_NORMAL;

					ex_cline_status_tbl.error_code = 0;
					_id0g8_intr_mode_sub(ID0G8_MODE_POWERUP, 0);
				}
			}
		}
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		if ((ex_cline_status_tbl.line_task_mode != ID0G8_MODE_POWERUP_ERROR)
			|| (ex_cline_status_tbl.error_code != cline_msg.arg2))
		{
			s_id0g8_powerup_stat = POWERUP_STAT_NORMAL;

			_set_id0g8_alarm(ID0G8_MODE_POWERUP_ERROR, (u16)cline_msg.arg2);
		}
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 9);
	}
}


/*********************************************************************//**
 * @brief Status infomation message receiving [Mode : ESCROW]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_status_info_mode_escrow(void)
{
	if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
		_set_id0g8_reject(ID0G8_MODE_REJECT, (u16)cline_msg.arg2);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		if (ex_cline_status_tbl.error_code != cline_msg.arg2)
		{
			_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
		}
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 11);
	}
}


/*********************************************************************//**
 * @brief Status infomation message receiving [Mode : ESCROW]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_status_info_mode_vend(void)
{
	if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		if (ex_cline_status_tbl.error_code != cline_msg.arg2)
		{
			ex_cline_status_tbl.error_code = cline_msg.arg2;

		}
	}
	else if (cline_msg.arg1 != TMSG_SUB_REJECT)
	{
		/* system error ? */
	//	_cline_system_error(0, 62);
	}
}


/*********************************************************************//**
 * @brief Status infomation message receiving [Mode : error]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_status_info_mode_error(void) //ok 移植
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
		ex_cline_status_tbl.escrow_code = 0x0000;
		ex_cline_status_tbl.error_code = ALARM_CODE_OK;
		ex_cline_status_tbl.reject_code = 0x0000;

		/* Stacker errorはchnge disable, Acceptor errorはそのまま */
        if( ex_tid_event_tbl.stacker_status != 0 )
		{
			ex_cline_status_tbl.accept_disable = 0x0001;
		}

		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);

		/* change status */
		_id0g8_intr_mode_sub(ID0G8_MODE_AUTO_RECOVERY, 0);

	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		if (ex_cline_status_tbl.error_code != cline_msg.arg2)
		{
			_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
		}
	}
	else if (cline_msg.arg1 != TMSG_SUB_REJECT)
	{
		/* system error ? */
	//	_cline_system_error(0, 13);
	}
}


/*********************************************************************//**
 * @brief Status infomation message receiving [Mode : default]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_status_info_mode_def(void)
{
	if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		if (ex_cline_status_tbl.error_code != cline_msg.arg2)
		{
			_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
		}
	}
	else if (cline_msg.arg1 != TMSG_SUB_REJECT)
	{
		/* system error ? */
	//	_cline_system_error(0, 15);
	}
}


/*********************************************************************//**
 * @brief Reset response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reset_rsp_msg_proc(void)
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID0G8_MODE_POWERUP_INITIAL:
	//2022-11-14
	case ID0G8_MODE_POWERUP_INITIAL_REJECT:
	case ID0G8_MODE_POWERUP_INITIAL_PAUSE:
	case ID0G8_MODE_POWERUP_INITIAL_STACK:
		_id0g8_reset_rsp_mode_power_init();	//full 以外 ok 移植
		break;
	case ID0G8_MODE_INITIAL:
	//2022-11-14
	case ID0G8_MODE_INITIAL_PAUSE:

		_id0g8_reset_rsp_mode_init();//ok 移植
		break;
	case ID0G8_MODE_AUTO_RECOVERY:
	//2022-11-14
	case ID0G8_MODE_AUTO_RECOVERY_PAUSE:
		_id0g8_reset_rsp_mode_recovery();//ok 移植
		break;
	default:					/* other */
		break;
	}
}


void _id0g8_power_recover(void) // パワーリカバリ
{

	// UBAはFailerステータス後に送信している
	// TIDイベント作成 Note/Ticket status
	if( 
		(s_id0g8_powerup_stat == POWERUP_STAT_RECOVER_FEED_STACK)
	||	(s_id0g8_powerup_stat == POWERUP_STAT_RECOVER_STACK)
	||	(s_id0g8_powerup_stat == POWERUP_STAT_RECOVER_SEARCH_STACK)
	)
	{
	/* Accepted */
		/* RECOVERY_STEP_VENDはここでvend */
		#if defined(ID0G8_LOG)
		ex_iwasaki_test_powerup[8] = 1;
		#endif
		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_ACCEPTED;	/* 0x01 */
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status );

		_id0g8_intr_mode_sub( ID0G8_MODE_WAIT_ACCEPTED_ACK, 0);

	}
	else if(
		(s_id0g8_powerup_stat == POWERUP_STAT_REJECT)
	||	(s_id0g8_powerup_stat == POWERUP_STAT_RECOVER_SEARCH_REJECT)
	)
	{
	/* Escrow Rejected. Removed */
	/* Rejected. Removed 		*/
		/*	Note / Ticket Status Rejected (0x88,0x0B,0x04)	*/
		/*	Note / Ticket Status Removed (0x88,0x0C,0x08)	*/
		#if defined(ID0G8_LOG)
		ex_iwasaki_test_powerup[8] = 4;
		#endif
		if( ex_0g8_recovery == RECOVER_BILL_ESCROW )
		{
			/* Escrow Bill */
			_id0g8_set_event(ID0G8_EVT_NOTE_VALIDATED, 0);
		}
		else if( ex_0g8_recovery == RECOVER_TICKET_ESCROW )
		{
			/* Escrow  Ticket */
			_id0g8_set_event(ID0G8_EVT_TICKET_VALIDATED, 0);
		}

		/* Rejected, Removed*/
		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_REJECTED;
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status );

		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_REMOVED;
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status );

		_id0g8_intr_mode_sub( ID0G8_MODE_WAIT_REMOVED_ACK, 0);

	}
	else
	{
	/* Escrow, Note patch clear */
	/* Normal */
	// Note/Ticketステータスクリア
		if( ex_0g8_recovery == RECOVER_BILL_ESCROW
		||  ex_0g8_recovery == RECOVER_TICKET_ESCROW
		)
		{
		/* Escrow, Note patch clear */
			if( ex_0g8_recovery == RECOVER_BILL_ESCROW )
			{
				/* Escrow Bill, Note path clear */
				_id0g8_set_event(ID0G8_EVT_NOTE_VALIDATED, 0);
			}
			else if( ex_0g8_recovery == RECOVER_TICKET_ESCROW )
			{
				/* Escrow  Ticket, Note path clear */
				_id0g8_set_event(ID0G8_EVT_TICKET_VALIDATED, 0);
			}

			ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_CLEAR;
			_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);
			_id0g8_intr_mode_sub( ID0G8_MODE_WAIT_REMOVED_ACK, 0);
		}
		else
		{
			// Normal 通常の待機状態へ
			// Note/Ticketステータスクリア
			if( (ex_tid_event_tbl.note_ticket_status & (~NOTE_TICKET_STATUS_ACCEPTED)) != 0x00 )
			{
				/* accepted以外のイベントで発生していたのをクリアして通知 *//* エラーなしを通知 */
				#if defined(ID0G8_LOG)
				ex_iwasaki_test_powerup[8] = 2;
				#endif
				ex_tid_event_tbl.note_ticket_status = 0x00;
				_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);
			}

			if( _is_id0g8_enable() )
			{
			/* Enable */
				_id0g8_intr_mode_sub( ID0G8_MODE_ENABLE, 0);
			}
			else
			{
			/* Disable */
				_id0g8_intr_mode_sub( ID0G8_MODE_DISABLE, 0);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
			}

			/* 待機状態になるので、リカバリ関係をクリア */
			set_recovery_step(RECOVERY_STEP_NON);
			_id0g8_denomi_clear();

		}
	}
}


/*********************************************************************//**
 * @brief Reset response message receiving [Mode : power-up initial relate]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reset_rsp_mode_power_init(void)//_id0g8_reset_rsp_mode_power_init full 以外 ok 移植
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{

		_id0g8_transfer_event_clear();	/* イベント送り直しのため、一度全てクリア */

		// イニシャル動作が正常に終わった為エラーを解除する
		ex_fail_data.fail_type = 0;
		ex_fail_data.diagnostics = 0;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);


		if(ex_0g8_recovery == RECOVER_GLI_ERROR)
		{
		/* このままだと、Disableステータスだがどうするか? */
		#if defined(ID0G8_LOG)
			ex_iwasaki_test_powerup[8] = 5;
		#endif
			ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_CLEAR;
			_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);

			// mainにエラーを送信する事で、このモードのままErrorがかえって来るので、その時のモード遷移する
    		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_FEED_LOST_BILL, 0, 0);

			return;
		}


		// スタッカーステータスをクリアする
		if(ex_tid_event_tbl.stacker_status != 0)
		{
			ex_tid_event_tbl.stacker_status = 0;
			_id0g8_set_event(ID0G8_EVT_STACKER_STATUS, ex_tid_event_tbl.stacker_status);
		}
		if( ex_0g8_recovery_com == 0)
		{
		/* パワーリカバリ */
			#if defined(ID0G8_LOG)
			ex_iwasaki_test_powerup[7] = 1;
			#endif
			_id0g8_power_recover();	/* パワーリカバリ */
		}
		else
		{
		/* 通信リカバリ	*/
			#if defined(ID0G8_LOG)
			ex_iwasaki_test_powerup[7] = 2;
			#endif
			_id0g8_com_recover(1);
		}

		ex_tid_event_tbl.note_ticket_status = 0x00;

	}
	else if (cline_msg.arg1 == TMSG_SUB_REMAIN)
	{
	/* Bill Remain */
		if (cline_msg.arg2 == BILL_IN_STACKER)
		{
		/* Stack */
			//_id0g8_intr_mode_sub(ID0G8_MODE_POWERUP_INITIAL_STACK, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, 0, 0, 0, 0);
		}
		else
		{
		/* Reject */
			_set_id0g8_reject(ID0G8_MODE_POWERUP_INITIAL_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER);

		}
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
	/* Reject */
		_set_id0g8_reject(ID0G8_MODE_POWERUP_INITIAL_REJECT, (u16)cline_msg.arg2);
	}
	else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
	{
	/* Pause */
	}
	else if (cline_msg.arg1 == TMSG_SUB_RESUME)
	{
	/* Ent off */
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		if ((cline_msg.arg2 == ALARM_CODE_STACKER_FULL)

			&& ((s_id0g8_powerup_stat == POWERUP_STAT_RECOVER_FEED_STACK)
			|| (s_id0g8_powerup_stat == POWERUP_STAT_RECOVER_STACK)
			|| (s_id0g8_powerup_stat == POWERUP_STAT_RECOVER_SEARCH_STACK)))
		{
			/* ACCEPTEDのAck受信後でStacker Fullの送信 */
			ex_cline_status_tbl.error_code =  ALARM_CODE_STACKER_FULL;

    		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_ACCEPTED;	/* 0x01 */
			_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status );
			_id0g8_intr_mode_sub( ID0G8_MODE_WAIT_ACCEPTED_ACK, 0);

		}
		else
		{
			if( !(check_failure_0g8((u16)cline_msg.arg2)) )
			{
				/* Failure errorでない場合、Failure 00は送る */
				ex_fail_data.fail_type = 0;
				ex_fail_data.diagnostics = 0;
				_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
			}
			_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
		}
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 17);
	}
}


/*********************************************************************//**
 * @brief Reset response message receiving [Mode : initial relate]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reset_rsp_mode_init(void)//ok 移植
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
		ex_self_test_0g8 = 0;

		if (_is_id0g8_enable())
		{
			_id0g8_intr_mode_sub( ID0G8_MODE_ENABLE, 0);

		}
		else
		{
			_id0g8_intr_mode_sub( ID0G8_MODE_DISABLE, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
		}
		// イニシャル動作が正常に終わった為エラーを解除する
		ex_fail_data.fail_type = 0;
		ex_fail_data.diagnostics = 0;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
		
		// スタッカーステータスをクリアする
		if(ex_tid_event_tbl.stacker_status != 0)
		{
			ex_tid_event_tbl.stacker_status = 0;
			_id0g8_set_event(ID0G8_EVT_STACKER_STATUS, ex_tid_event_tbl.stacker_status);
		}
		// Note/Ticketステータスクリア

		if( (ex_tid_event_tbl.note_ticket_status & (~NOTE_TICKET_STATUS_ACCEPTED)) != 0x00 )
		{
			ex_tid_event_tbl.note_ticket_status = 0x00;
			_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);
		}
		ex_tid_event_tbl.note_ticket_status = 0x00;

	}
	else if (cline_msg.arg1 == TMSG_SUB_REMAIN)
	{
	/* Bill Remain */
		if (cline_msg.arg2 == BILL_IN_STACKER)
		{
		/* Stack */
			_id0g8_intr_mode_sub(ID0G8_MODE_INITIAL_STACK,0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, 0, 0, 0, 0);
		}
		else
		{
		/* Reject */
			_set_id0g8_reject(ID0G8_MODE_INITIAL_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER);

		}
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
	/* Reject */
		_set_id0g8_reject(ID0G8_MODE_INITIAL_REJECT, (u16)cline_msg.arg2);


	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		if(ex_self_test_0g8 == 1)
		{
		//selftestコマンドからのイニシャル完了時にFailure関係のレスポンスが必要 */
			if( !(check_failure_0g8((u16)cline_msg.arg2)) )
			{
				/* Failure errorでない場合、Failure 00は送る */
				ex_fail_data.fail_type = 0;
				ex_fail_data.diagnostics = 0;
				_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
			}
		}
		ex_self_test_0g8 = 0;

		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 19);
	}
}


/*********************************************************************//**
 * @brief Reset response message receiving [Mode : auto recovery relate]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reset_rsp_mode_recovery(void) // ok 移植  _id0g8_reset_rsp_mode_recovery
{

	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
        if( _is_id0g8_enable() )
        {
			_id0g8_intr_mode_sub( ID0G8_MODE_ENABLE, 0);

        }
        else
        {
			_id0g8_intr_mode_sub( ID0G8_MODE_DISABLE, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

        }
		// イニシャル動作が正常に終わった為エラーを解除する
//		if(ex_fail_data.fail_type != 0)
		if(ex_fail_data.fail_type != 0 || ex_fail_data.diagnostics != 0 )
		{
			ex_fail_data.fail_type = 0;
			ex_fail_data.diagnostics = 0;
			_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
		}
		ex_fail_data.fail_type = 0;
		ex_fail_data.diagnostics = 0;

		// スタッカーステータスをクリアする
		if(ex_tid_event_tbl.stacker_status != 0)
		{
			ex_tid_event_tbl.stacker_status = 0;
			_id0g8_set_event(ID0G8_EVT_STACKER_STATUS, ex_tid_event_tbl.stacker_status);
		}

		// Note/Ticketステータスクリア
//		if(ex_tid_event_tbl.note_ticket_status != 0x00)
		if( (ex_tid_event_tbl.note_ticket_status & (~NOTE_TICKET_STATUS_ACCEPTED)) != 0x00 )
		{
			ex_tid_event_tbl.note_ticket_status = 0x00;
			_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);
		}
		ex_tid_event_tbl.note_ticket_status = 0x00;
	}
	else if (cline_msg.arg1 == TMSG_SUB_REMAIN)
	{
	/* Bill Remain */
		if (cline_msg.arg2 == BILL_IN_STACKER)
		{
		/* Stack */
			/* JCM-Eの要望であまり払い出してほしくない為、収納*/
            /* Stack */
			_id0g8_intr_mode_sub(ID0G8_MODE_AUTO_RECOVERY_STACK, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, 0, 0, 0, 0);
		}
		else
		{
		/* Reject */
			_set_id0g8_reject(ID0G8_MODE_AUTO_RECOVERY_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER);
		}
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
	/* Reject */
		_set_id0g8_reject(ID0G8_MODE_AUTO_RECOVERY_REJECT, (u16)cline_msg.arg2);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 21);
	}

}


/*********************************************************************//**
 * @brief Disable response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_disable_rsp_msg_proc(void)//ok 移植
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID0G8_MODE_DISABLE: //_id0g8_disable_rsp_mode_disable, 
		if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id0g8_reject(ID0G8_MODE_DISABLE_REJECT, (u16)cline_msg.arg2);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
		}
		else
		{
			/* system error ? */
		//	_cline_system_error(0, 23);
		}
		break;

	case ID0G8_MODE_ENABLE: //_id0g8_enable_rsp_mode_disable UBA500では存在するのて追加する
		if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id0g8_reject(ID0G8_MODE_ENABLE_REJECT, (u16)cline_msg.arg2);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
		}
		else
		{
			/* system error ? */
		//	_cline_system_error(0, 23);
		}
		break;
	default:					/* other */
		break;
	}
}


/*********************************************************************//**
 * @brief Enable response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_enable_rsp_msg_proc(void)//ok 移植 _id0g8_enable_mode_accept, _id0g8_enable_mode_accepting,_id0g8_enable_mode_reject
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID0G8_MODE_ENABLE:
		if (cline_msg.arg1 == TMSG_SUB_ACCEPT)
		{
			/*取り込み動作開始前に受信して、mainタスクに取り込み許可をする処理 */
			_id0g8_intr_mode_sub(ID0G8_MODE_ACCEPT, 0);
    		set_recovery_step(RECOVERY_STEP_ACCEPT);
			ex_cline_status_tbl.reject_code = 0x0000;			
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ACCEPT_REQ, 0, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD1, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id0g8_reject(ID0G8_MODE_ENABLE_REJECT, (u16)cline_msg.arg2);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
		}
		else
		{
			/* system error ? */
		//	_cline_system_error(0, 25);
		}
		break;

	case ID0G8_MODE_DISABLE:	//uba500はdisableもあるので _id0g8_disable_mode_evt_reject, 
		if (cline_msg.arg1 == TMSG_SUB_ACCEPT)
		{
			_id0g8_intr_mode_sub(ID0G8_MODE_ACCEPT, 0);
    		set_recovery_step(RECOVERY_STEP_ACCEPT);
			ex_cline_status_tbl.reject_code = 0x0000;			
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ACCEPT_REQ, 0, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD1, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id0g8_reject(ID0G8_MODE_DISABLE_REJECT, (u16)cline_msg.arg2);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
		}
		else
		{
			/* system error ? */
		//	_cline_system_error(0, 25);
		}
		break;


	default:					/* other */
		break;
	}
}

#if 0 //
void _id0g8_disable_mode_reject( void )//ok 移植 or _id0g8_disable_reject_mode_evt_accept_success
{

	if (cline_msg.arg1 == TMSG_SUB_SUCCESS) //_id0g8_accept_mode_success
	{
	/* 2枚目の入金NG */
		s_id0g8_2nd_note = 0;
	    _set_id0g8_reject(ID0G8_MODE_DISABLE_REJECT, REJECT_CODE_INHIBIT);
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT) // _id0g8_accept_mode_reject
	{
	/* 2枚目の入金NG */
		s_id0g8_2nd_note = 0;
	    _set_id0g8_reject(ID0G8_MODE_DISABLE_REJECT, REJECT_CODE_INHIBIT);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 27);
	}
}
#endif


/*********************************************************************//**
 * @brief Accept response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_accept_rsp_msg_proc(void)//ok 移植
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID0G8_MODE_ACCEPT:
		_id0g8_accept_rsp_mode_accept();//ok 移植
		break;
	case ID0G8_MODE_ESCROW_WAIT_ACK:
	case ID0G8_MODE_ESCROW:
		_id0g8_accept_rsp_mode_escrow();//ok 移植
		break;
	case ID0G8_MODE_REJECT_WAIT_ACCEPT_RSP:
		_id0g8_accept_rsp_mode_rej_wait_accrsp();//ok 移植
		break;

	/* 下記は、将来的にAcceptedに対するAck受信前に2枚目紙幣を取り込み時に必要になる */
	/* 現状は、使用しないはず	*/
	case ID0G8_MODE_DISABLE:
	case ID0G8_MODE_DISABLE_REJECT:
		//並列動作の保険処理だが、現状はAcceptedのAck受信後しか取り込み許可してないので、
		//ここにはいる可能性はない
		//_id0g8_disable_mode_reject();   //ok 移植 reject
		//break;
	/* 紙幣取り込み開始前にacceptモードになるので、通常はありえない */
	case ID0G8_MODE_ENABLE:
		//_id0g8_enable_mode_accepting
		//_id0g8_accept_evt_success_2nd_in_enable
		//_id0g8_enable_mode_reject

	//現状存在しないはず(AcceptedのAck待ちで取り込んでいない為)
	case ID0G8_MODE_ENABLE_REJECT:
		//_id0g8_enable_mode_reject reject

	case ID0G8_MODE_STACK:
		//_id0g8_accept_evt_success_2nd
		//_id0g8_accept_evt_reject_2nd

	case ID0G8_MODE_WAIT_ACCEPTED_ACK:
		//_id0g8_accept_evt_success_2nd
		//_id0g8_accept_evt_reject_2nd

	default:					/* other */
		break;
	}
}


/*********************************************************************//**
 * @brief Accept response message receiving [Mode : accept]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_accept_rsp_mode_accept(void)//ok 移植 _id0g8_accept_mode_success 
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS) //_id0g8_accept_mode_success
	{
		if( cline_msg.arg2 == BAR_INDX )
		{
			if(_is_id0g8_bar_inhibit())
			{
                _set_id0g8_reject(ID0G8_MODE_REJECT, REJECT_CODE_INHIBIT);
			}
			else
			{
				ex_cline_status_tbl.escrow_code = cline_msg.arg2;
				_id0g8_intr_mode_sub( ID0G8_MODE_ESCROW_WAIT_ACK, 0 );
				
				// TIDイベント作成予定 Ticket validated
				memo_copy( (u8*)&ex_cline_status_tbl.ex_Barcode_recovery_0g8[0], &ex_barcode[0], 32);
				_id0g8_set_event(ID0G8_EVT_TICKET_VALIDATED, 0);
				set_recovery_step(RECOVERY_STEP_ESCORW_WAIT_ACK);
			}
		}
		else
		{
			if( _is_id0g8_denomi_inhibit(cline_msg.arg2, cline_msg.arg3) )
			{
				_set_id0g8_reject(ID0G8_MODE_REJECT, REJECT_CODE_INHIBIT);
			}
			else
			{
				ex_cline_status_tbl.escrow_code = cline_msg.arg2;
				_id0g8_intr_mode_sub( ID0G8_MODE_ESCROW_WAIT_ACK, 0 );

				// TIDイベント作成予定 Note validated
				_id0g8_set_event(ID0G8_EVT_NOTE_VALIDATED, 0);
				set_recovery_step(RECOVERY_STEP_ESCORW_WAIT_ACK);
			}
		}
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT) // _id0g8_accept_mode_reject
	{
		_set_id0g8_reject(ID0G8_MODE_REJECT, (u16)cline_msg.arg2);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 27);
	}
}



void _id0g8_accept_rsp_mode_escrow(void)//ok 移植 _id0g8_accept_mode_reject 
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS) //
	{
		/* 通常はあり得ない */
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT) // _id0g8_accept_mode_reject
	{
		_set_id0g8_reject(ID0G8_MODE_REJECT, (u16)cline_msg.arg2);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 27);
	}
}

/*********************************************************************//**
 * @brief Accept response message receiving [Mode : reject wait accept response]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_accept_rsp_mode_rej_wait_accrsp(void) //ok 移植 _id0g8_accept_evt_reject
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)// _id0g8_accept_evt_reject
	{
		_set_id0g8_reject(ID0G8_MODE_REJECT, REJECT_CODE_INHIBIT);
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)//_id0g8_accept_evt_reject
	{
		//_set_id0g8_reject(ID0G8_MODE_REJECT, (u16)cline_msg.arg2);
		_set_id0g8_reject(ID0G8_MODE_REJECT, REJECT_CODE_INHIBIT);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)// _id0g8_accept_rsp_mode_rej_wait_accrsp
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 29);
	}
}


/*********************************************************************//**
 * @brief Stack response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_stack_rsp_msg_proc(void)//ok 移植
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID0G8_MODE_POWERUP_INITIAL:
	case ID0G8_MODE_POWERUP_INITIAL_REJECT:
	case ID0G8_MODE_POWERUP_INITIAL_STACK:
	case ID0G8_MODE_POWERUP_INITIAL_PAUSE:
		_id0g8_stack_rsp_mode_power_init(); //ok 移植
		break;
	case ID0G8_MODE_INITIAL_STACK:
	case ID0G8_MODE_INITIAL_PAUSE:
		_id0g8_stack_rsp_mode_init();//ok 移植
		break;
	case ID0G8_MODE_AUTO_RECOVERY_STACK:
	case ID0G8_MODE_AUTO_RECOVERY_PAUSE:
		_id0g8_stack_rsp_mode_recovery();//ok 移植
		break;
	case ID0G8_MODE_STACK:
	case ID0G8_MODE_PAUSE:
		_id0g8_stack_rsp_mode_stack();//ok 移植
		break;


	//case ID0G8_MODE_VEND:
	case ID0G8_MODE_WAIT_ACCEPTED_ACK: //uba500使用しているので
		_id0g8_stack_rsp_mode_wait_accepted_ack();	//ok 移植
		break;

	//case ID0G8_MODE_STACKED:
	//	_id0g8_stack_rsp_mode_stacked();
	//	break;
	//case ID0G8_MODE_STACK_FINISH:
	//	_id0g8_stack_rsp_mode_stack_finish();
	//	break;
	default:					/* other */
		break;
	}
}


/*********************************************************************//**
 * @brief Stack response message receiving [Mode : PowerUp Initialize]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_stack_rsp_mode_power_init(void) //ok 移植　_id0g8_power_mode_success,  _id0g8_mode_alarm
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)//_id0g8_power_mode_success
	{
		_id0g8_intr_mode_sub(ID0G8_MODE_POWERUP_INITIAL, 0); //UBA500は遷移させていないが。。		
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
	}
	else if (cline_msg.arg1 == TMSG_SUB_VEND)
	{
		/*  */
	}
	else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
	{
		//UBA500では存在しない
		_id0g8_intr_mode_sub(ID0G8_MODE_POWERUP_INITIAL_PAUSE, 0);
		
	}
	else if (cline_msg.arg1 == TMSG_SUB_RESUME)
	{
		//UBA500では存在しない
		_id0g8_intr_mode_sub(ID0G8_MODE_POWERUP_INITIAL_STACK,0);
		
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
		/*  */
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)// _id0g8_mode_alarm
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 31);
	}
}


/*********************************************************************//**
 * @brief Stack response message receiving [Mode : Initialize]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_stack_rsp_mode_init(void) //ok 移植 _id0g8_stack_mode_success_ini
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)// _id0g8_stack_mode_success_ini
	{
		_id0g8_intr_mode_sub(ID0G8_MODE_INITIAL, 0);
		
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
	}
	else if (cline_msg.arg1 == TMSG_SUB_VEND)
	{
		/*  */
	}
	else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
	{
		//UBA500は存在しないが
		_id0g8_intr_mode_sub(ID0G8_MODE_INITIAL_PAUSE, 0);
		
	}
	else if (cline_msg.arg1 == TMSG_SUB_RESUME)
	{
		//UBA500は存在しないが
		_id0g8_intr_mode_sub(ID0G8_MODE_INITIAL_STACK, 0);
		
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
		/*  */
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 33);
	}
}


/*********************************************************************//**
 * @brief Stack response message receiving [Mode : Auto Recovery]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_stack_rsp_mode_recovery(void)//ok 移植 _id0g8_stack_mode_recovery_success,_id0g8_mode_alarm
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)//_id0g8_stack_mode_recovery_success
	{
		_id0g8_intr_mode_sub(ID0G8_MODE_AUTO_RECOVERY,0);
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
	}
	else if (cline_msg.arg1 == TMSG_SUB_VEND)
	{
		/*  */
	}
	else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
	{
		//UBA500は存在しないが
		_id0g8_intr_mode_sub(ID0G8_MODE_AUTO_RECOVERY_PAUSE,0);
		
	}
	else if (cline_msg.arg1 == TMSG_SUB_RESUME)
	{
		//UBA500は存在しないが
		_id0g8_intr_mode_sub(ID0G8_MODE_AUTO_RECOVERY_STACK, 0);
		
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
		/*  */
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 35);
	}
}


/*********************************************************************//**
 * @brief Stack response message receiving [Mode : stack]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_stack_rsp_mode_stack(void)//ok 移植 _id0g8_stack_mode_vend_success,_id0g8_stack_evt_reject,_id0g8_stack_evt_pause,_id0g8_stack_rsp_mode_stack
{
	//UBA500は収納完了のSuccessでAcceptedイベントを通知しているが、
	//ivizion2のid-003のvendタイミングに合わせる、
	//ivizion2はモードによってvendタイミングが異なるでの
	//id-0g8も同様にする為にはSuccessではなく、vendのタイミングでAcceptedイベントを送信するようにする
	//ちなみに、ivizionは収納完了時にacceptedイベント

	if (cline_msg.arg1 == TMSG_SUB_VEND) //これではない_id0g8_stack_mode_vend
	{									 //これ _id0g8_stack_mode_vend_success
		// TIDイベント作成 Note/Ticket status
		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_ACCEPTED;
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);

		_id0g8_intr_mode_sub( ID0G8_MODE_WAIT_ACCEPTED_ACK, 0);

	}
	else if (cline_msg.arg1 == TMSG_SUB_PAUSE)//_id0g8_stack_evt_pause
	{
		if (ex_cline_status_tbl.line_task_mode == ID0G8_MODE_STACK)
		{
			_id0g8_intr_mode_sub(ID0G8_MODE_PAUSE, 0);
		}
	}
	else if (cline_msg.arg1 == TMSG_SUB_RESUME)//_id0g8_stack_rsp_mode_stack
	{
		if (ex_cline_status_tbl.line_task_mode == ID0G8_MODE_PAUSE)
		{
			_id0g8_intr_mode_sub(ID0G8_MODE_STACK, 0);
		}
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)//_id0g8_stack_evt_reject
	{
		_set_id0g8_reject(ID0G8_MODE_RETURN, REJECT_CODE_RETURN);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 37);
	}
}

void _id0g8_stack_rsp_mode_wait_accepted_ack(void)//ok 移植
{

	if (cline_msg.arg1 == TMSG_SUB_ALARM) //_id0g8_stack_mode_vend_alarm
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}

}


/*********************************************************************//**
 * @brief Stack response message receiving [Mode : vend relate]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0 //2022-08-26
void _id0g8_stack_rsp_mode_stacked(void)
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
		s_id0g8_stacking_info &= ~(ID0G8_STACKING_INFO_BUSY);
		/* [Interrupt Mode 2] don't wait status request in STACKED status */
		if (_is_id0g8_enable())
		{
			_id0g8_intr_mode_sub(ID0G8_MODE_ENABLE, 0);
			
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

		}
		else
		{
			_id0g8_intr_mode_sub(ID0G8_MODE_DISABLE, 0);
			
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
		}
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		s_id0g8_stacking_info &= ~(ID0G8_STACKING_INFO_BUSY);
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 41);
	}
}
#endif

/*********************************************************************//**
 * @brief Stack response message receiving [Mode : stack finish]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0 //2022-08-26
void _id0g8_stack_rsp_mode_stack_finish(void)
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
		s_id0g8_stacking_info &= ~(ID0G8_STACKING_INFO_BUSY);
		if (_is_id0g8_enable())
		{
			_id0g8_intr_mode_sub(ID0G8_MODE_ENABLE, 0);
			
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
		}
		else
		{
			_id0g8_intr_mode_sub(ID0G8_MODE_DISABLE, 0);
			
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

		}
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 43);
	}
}
#endif

/*********************************************************************//**
 * @brief Reject response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reject_rsp_msg_proc(void)
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID0G8_MODE_POWERUP_INITIAL:
	case ID0G8_MODE_POWERUP_INITIAL_REJECT:
		_id0g8_reject_rsp_mode_power_init_rej();//ok 移植した
		break;
	case ID0G8_MODE_POWERUP_INITIAL_PAUSE:
		_id0g8_reject_rsp_mode_power_init_pau();//ok 移植した
		break;

	case ID0G8_MODE_INITIAL: // UBA500が存在するのでとりあえず
		_id0g8_reject_rsp_mode_init();//ok 移植した
		break;

	case ID0G8_MODE_INITIAL_REJECT:
		_id0g8_reject_rsp_mode_init_rej();//ok 移植した
		break;
	case ID0G8_MODE_INITIAL_PAUSE:
		_id0g8_reject_rsp_mode_init_pau();//ok 移植した
		break;

	case ID0G8_MODE_AUTO_RECOVERY: // UBA500が存在するのでとりあえず
		_id0g8_reject_rsp_mode_recovery();//ok 移植した
		break;

	case ID0G8_MODE_AUTO_RECOVERY_REJECT:
		_id0g8_reject_rsp_mode_recovery_rej();	//ok 移植
		break;
	case ID0G8_MODE_AUTO_RECOVERY_PAUSE:
		_id0g8_reject_rsp_mode_recovery_pau();	//ok 移植
		break;
	case ID0G8_MODE_REJECT:
	case ID0G8_MODE_REJECT_WAIT_ACCEPT_RSP:// UBA500が存在するのでとりあえず
		_id0g8_reject_rsp_mode_reject();	//ok 移植
		break;
	case ID0G8_MODE_RETURN:
		_id0g8_reject_rsp_mode_return();//ok 移植
		break;

	case ID0G8_MODE_ENABLE: // UBA500が存在するのでとりあえず
	case ID0G8_MODE_DISABLE: // UBA500が存在するのでとりあえず
	case ID0G8_MODE_DISABLE_REJECT:
	case ID0G8_MODE_ENABLE_REJECT:
		_id0g8_reject_rsp_mode_enable_disable_rej(); //ok 移植
		break;


	case ID0G8_MODE_ACCEPT:
	case ID0G8_MODE_ESCROW_WAIT_ACK:
		//_id0g8_accept_mode_reject_success();
		_id0g8_reject_rsp_mode_accept();// ok　移植
		break;

	case ID0G8_MODE_ESCROW:
		//_id0g8_reject_mode_success();
		_id0g8_reject_rsp_mode_escrow();// ok　移植
		break;

	case ID0G8_MODE_STACK:
		//_id0g8_mode_alarm();
		_id0g8_reject_rsp_mode_stack();// ok　移植
		break;


	default:					/* other */
		break;
	}
}



/*********************************************************************//**
 * @brief Reject response message receiving [Mode : PowerUp Initialize Reject]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reject_rsp_mode_power_init_rej(void) //ok 移植した
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
		_id0g8_intr_mode_sub( ID0G8_MODE_POWERUP_INITIAL, 0);		
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
	}
	else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
	{
		//#if defined(ID0G8_PS)	/* Power upからのポーズ */
	    /* Failure Status non, Unable (0x85,0x80,0x01)(ポーズステータス) */
		ex_fail_data.fail_type |= FAIL_TYPE_OTHER;
		ex_fail_data.diagnostics = 1;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
		//#endif

        _id0g8_intr_mode_sub( ID0G8_MODE_POWERUP_INITIAL_PAUSE, 0);	/* 2020-11-11 */
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
		_set_id0g8_reject(ID0G8_MODE_POWERUP_INITIAL_REJECT, (u16)cline_msg.arg2);

	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 47);
	}
}


/*********************************************************************//**
 * @brief Reject response message receiving [Mode : PowerUp Initialize Pause]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reject_rsp_mode_power_init_pau(void)//ok 移植した
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
        _id0g8_intr_mode_sub( ID0G8_MODE_POWERUP_INITIAL, 0);				
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);

#if defined(PRJ_IVIZION2)
		/* iVIZIONはポーズ解除送っていないのでiVIZIONに合わせる */
#else
		/* ポーズ解除で常に送る必要がある為、				*/
		/* パワーリカバリの返却紙幣の取り除きだけでなく、	*/
		/* パワーアップ時のイニシャル時のポーズ解除も同様	*/
		//if( s_id0g8_powerup_stat == POWERUP_STAT_REJECT )
		//{
        /* Failure Status non, Unable (0x85,0x00,0x01)(返却紙幣取り除かれ後) */
		ex_fail_data.fail_type = 0;
		ex_fail_data.diagnostics = 1;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
#endif
	}
	else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
	{
		/*  */
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
		_set_id0g8_reject(ID0G8_MODE_POWERUP_INITIAL_REJECT, (u16)cline_msg.arg2);

	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);

	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 49);
	}
}

void _id0g8_reject_rsp_mode_init(void) //ok 移植 _id0g8_reject_rsp_mode_init
{

    if( cline_msg.arg1 == TMSG_SUB_SUCCESS )
    {
        _id0g8_intr_mode_sub( ID0G8_MODE_INITIAL, 0);
        _cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
    }
    else if( cline_msg.arg1 == TMSG_SUB_INTERIM )
    {
        _id0g8_intr_mode_sub( ID0G8_MODE_INITIAL_PAUSE, 0);
    }
    else if( cline_msg.arg1 == TMSG_SUB_REJECT )
    {
        _set_id0g8_reject(ID0G8_MODE_INITIAL_REJECT, (u16)cline_msg.arg2);

    }
    else if( cline_msg.arg1 == TMSG_SUB_ALARM )
    {
        _set_id0g8_alarm( ID0G8_MODE_ERROR, (u16)cline_msg.arg2 );
    }
    else
    {
        /* system error ? */
    //    _cline_system_error(0, 51);
    }

}



/*********************************************************************//**
 * @brief Reject response message receiving [Mode : Initialize Reject]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reject_rsp_mode_init_rej(void) //ok 移植した
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
		_id0g8_intr_mode_sub( ID0G8_MODE_INITIAL, 0);	
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
	}
	else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
	{
		_id0g8_intr_mode_sub( ID0G8_MODE_INITIAL_PAUSE, 0);

		/* 通常の紙幣検知後のポーズへの遷移*/
	    /* Failure Status non, Unable (0x85,0x80,0x01)(ポーズステータス) */
		ex_fail_data.fail_type |= FAIL_TYPE_OTHER;
		ex_fail_data.diagnostics = 1;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);

	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
		_set_id0g8_reject(ID0G8_MODE_INITIAL_REJECT, (u16)cline_msg.arg2);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 51);
	}
}


/*********************************************************************//**
 * @brief Reject response message receiving [Mode : Initialize Pause]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reject_rsp_mode_init_pau(void) //ok 移植した
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
		_id0g8_intr_mode_sub( ID0G8_MODE_INITIAL, 0);
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);

#if defined(PRJ_IVIZION2)
		/* iVIZIONはポーズ解除送っていないのでiVIZIONに合わせる */
#else
		/* 通常イニシャルでのポーズ解除で常に送る必要がある為、				*/
        /* Failure Status non, Unable (0x85,0x00,0x01)(返却紙幣取り除かれ後ポーズ解除) */
		ex_fail_data.fail_type = 0;
		ex_fail_data.diagnostics = 1;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
#endif
	}
	else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
	{
		/*  */
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
		_set_id0g8_reject(ID0G8_MODE_INITIAL_REJECT, (u16)cline_msg.arg2);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 53);
	}
}


//UBA500が存在するのでとりあえず
void _id0g8_reject_rsp_mode_recovery(void) //ok 移植_id0g8_reject_rsp_mode_recovery
{
    if( cline_msg.arg1 == TMSG_SUB_SUCCESS )
    {
        _id0g8_intr_mode_sub( ID0G8_MODE_AUTO_RECOVERY, 0);
        _cline_send_msg( ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0 );
    }
    else if( cline_msg.arg1 == TMSG_SUB_INTERIM )
    {
        _id0g8_intr_mode_sub( ID0G8_MODE_AUTO_RECOVERY_PAUSE, 0);
    }
    else if( cline_msg.arg1 == TMSG_SUB_REJECT )
    {
        _set_id0g8_reject( ID0G8_MODE_AUTO_RECOVERY_REJECT, (u16)cline_msg.arg2 );

    }
    else if( cline_msg.arg1 == TMSG_SUB_ALARM )
    {
        _set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
    }
    else
    {
        /* system error ? */
    //    _cline_system_error(0, 55);
    }



}



/*********************************************************************//**
 * @brief Reject response message receiving [Mode : Auto Recovery Reject]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reject_rsp_mode_recovery_rej(void)	//ok 移植
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
		_id0g8_intr_mode_sub(ID0G8_MODE_AUTO_RECOVERY, 0);
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
	}
	else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
	{
		//#if defined(ID0G8_PS)
		/* エラーからの自動復帰中のポーズへの遷移*/
	    /* Failure Status non, Unable (0x85,0x80,0x01)(ポーズステータス) */
		ex_fail_data.fail_type |= FAIL_TYPE_OTHER;
		ex_fail_data.diagnostics = 1;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
		//#endif

		/* change status */
		_id0g8_intr_mode_sub(ID0G8_MODE_AUTO_RECOVERY_PAUSE, 0);
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
		_set_id0g8_reject(ID0G8_MODE_AUTO_RECOVERY_REJECT, (u16)cline_msg.arg2);

	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 55);
	}
}


/*********************************************************************//**
 * @brief Reject response message receiving [Mode : Auto Recovery Pause]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reject_rsp_mode_recovery_pau(void)	//ok 移植
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
	{
		_id0g8_intr_mode_sub(ID0G8_MODE_AUTO_RECOVERY, 0);
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);

#if defined(PRJ_IVIZION2)
		/* iVIZIONはポーズ解除送っていないのでiVIZIONに合わせる */
#else
	//#if defined(ID0G8_PS)
		/* エラーからの自動復帰中のポーズ状態で、ポーズ解除で常に送る必要がある為、				*/
		/* Failure Status non, Unable (0x85,0x00,0x01)(紙幣取り除かれ後ポーズ解除) */
		ex_fail_data.fail_type = 0;
		ex_fail_data.diagnostics = 1;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
	//#endif
#endif
	}
	else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
	{
		/*  */
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
		_set_id0g8_reject(ID0G8_MODE_AUTO_RECOVERY_REJECT, (u16)cline_msg.arg2);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 57);
	}
}


/*********************************************************************//**
 * @brief Reject response message receiving [Mode : reject]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reject_rsp_mode_reject(void) //ok 移植   _id0g8_reject_mode_success, _id0g8_reject_mode_reject, _id0g8_reject_mode_remain_rej
{
	/* Normal (Not Status WAIT) */
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS) //_id0g8_reject_mode_success
	{
		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_REMOVED;
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);

		if( !(_id0g8_check_remaining_events()) )
		{
			/* Normal (Not Status WAIT) */
			if( _is_id0g8_enable() )
			{
				_id0g8_intr_mode_sub( ID0G8_MODE_ENABLE, 0);

			}
			else
			{
				_id0g8_intr_mode_sub( ID0G8_MODE_DISABLE,  0);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
			}
		}
		else
		{
			_id0g8_intr_mode_sub( ID0G8_MODE_WAIT_REMOVED_ACK, 0 );	/* MainのEnableもDisableも送っていないので、mainはRejectモードのラインタスクレスポンス待ち*/
		}

	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT) //_id0g8_reject_mode_reject
	{
	    _set_id0g8_reject(ID0G8_MODE_AUTO_RECOVERY_REJECT, (u16)cline_msg.arg2);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else if (cline_msg.arg1 == TMSG_SUB_REMAIN) //_id0g8_reject_mode_remain_rej
	{
	// TIDイベント作成 Note/Ticket status
		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_REJECTED;
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 59);
	}
}


void _id0g8_reject_rsp_mode_return(void)	//ok 移植　_id0g8_reject_mode_remain_ret,_id0g8_reject_mode_success
{
	if(cline_msg.arg1 == TMSG_SUB_SUCCESS) //_id0g8_reject_mode_success
	{
		// TIDイベント作成 Note/Ticket status
		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_REMOVED;
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);

		if( !(_id0g8_check_remaining_events()) )
		{
			/* Normal (Not Status WAIT) */
			if( _is_id0g8_enable() )
			{
				_id0g8_intr_mode_sub( ID0G8_MODE_ENABLE, 0);

			}
			else
			{
				_id0g8_intr_mode_sub( ID0G8_MODE_DISABLE, 0);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

			}
		}
		else
		{
			_id0g8_intr_mode_sub( ID0G8_MODE_WAIT_REMOVED_ACK, 0);	/* MainのEnableもDisableも送っていないので、mainはRejectモードのラインタスクレスポンス待ち*/
		}
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT) //_id0g8_reject_mode_reject
	{
		_set_id0g8_reject(ex_cline_status_tbl.line_task_mode, (u16)cline_msg.arg2); //そのまま

	}
	else if (cline_msg.arg1 == TMSG_SUB_REMAIN)//_id0g8_reject_mode_remain_ret
	{
	// TIDイベント作成 Note/Ticket status
		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_RETURNED;
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 60);
	}
}


/*********************************************************************//**
 * @brief Reject response message receiving [Mode : reject]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_reject_rsp_mode_enable_disable_rej(void) //ok 移植 _id0g8_reject_rsp_mode_disable_rej  , _id0g8_reject_mode_reject
{
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS) // _id0g8_reject_mode_success
	{
		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_REMOVED;
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);

		if( !(_id0g8_check_remaining_events()) )
		{
			if (_is_id0g8_enable())
			{
				_id0g8_intr_mode_sub(ID0G8_MODE_ENABLE, 0);				

			}
			else
			{
				_id0g8_intr_mode_sub(ID0G8_MODE_DISABLE, 0);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
			}
		}
		else
		{
			_id0g8_intr_mode_sub( ID0G8_MODE_WAIT_REMOVED_ACK, 0);	/* MainのEnableもDisableも送っていないので、mainはRejectモードのラインタスクレスポンス待ち*/
		}
	}
	else if (cline_msg.arg1 == TMSG_SUB_REJECT)
	{
    	_set_id0g8_reject(ID0G8_MODE_AUTO_RECOVERY_REJECT, (u16)cline_msg.arg2);
	}
	else if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
	else
	{
		/* system error ? */
	//	_cline_system_error(0, 59);
	}
}


void _id0g8_reject_rsp_mode_accept(void)// ok　移植
{
	//uba500もsuccessのみ
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS) // _id0g8_accept_mode_reject_success
	{
		if( _is_id0g8_enable() )
		{
			_id0g8_intr_mode_sub( ID0G8_MODE_ENABLE, 0);

		}
		else
		{
			_id0g8_intr_mode_sub( ID0G8_MODE_DISABLE, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

		}
	}
}



void _id0g8_reject_rsp_mode_escrow(void)// ok　移植
{
	//uba500もsuccessのみ
	if (cline_msg.arg1 == TMSG_SUB_SUCCESS) // _id0g8_reject_mode_success
	{
		// TIDイベント作成 Note/Ticket status
		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_REMOVED;
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);

		if( !(_id0g8_check_remaining_events()) )
		{
			/* Normal (Not Status WAIT) */
			if( _is_id0g8_enable() )
			{
				_id0g8_intr_mode_sub( ID0G8_MODE_ENABLE, 0);

			}
			else
			{
				_id0g8_intr_mode_sub( ID0G8_MODE_DISABLE, 0);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
			}
		}
		else
		{
			_id0g8_intr_mode_sub( ID0G8_MODE_WAIT_REMOVED_ACK, 0);	/* MainのEnableもDisableも送っていないので、mainはRejectモードのラインタスクレスポンス待ち*/
		}
	}
}


void _id0g8_reject_rsp_mode_stack(void)// ok　移植
{
	//uba500もerrorのみ
	if (cline_msg.arg1 == TMSG_SUB_ALARM)
	{
		_set_id0g8_alarm(ID0G8_MODE_ERROR, (u16)cline_msg.arg2);
	}
}


/*********************************************************************//**
 * @brief Times up message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_times_up_msg_proc(void)//ok 移植
{
	switch (cline_msg.arg1)
	{
	case TIMER_ID_DIPSW_READ:
		_id0g8_timeup_dipsw_read_proc(); //ok 移植
		break;

	case TIMER_ID_ESCROW_HOLD1:
		_id0g8_timeup_escrow_hold1_proc(); //ok 移植
		break;

	case TIMER_ID_EVENT_SEND:
		_id0g8_timeup_re_send_proc();// ok 移植
		break;

	default:					/* other */
		/* system error ? */
	//	_cline_system_error(0, 61);
		break;
	}
}

/*********************************************************************//**
 * @brief Read dipsw message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_timeup_dipsw_read_proc(void)// ok 移植_id0g8_timeup_dipsw_read_mode_enable, _id0g8_timeup_dipsw_read_mode_disable,  _id0g8_timeup_dipsw_read_mode
{

	ex_cline_status_tbl.dipsw_disable = _id0g8_dipsw_disable();

	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID0G8_MODE_DISABLE:

		if (_is_id0g8_enable())
		{
			_id0g8_intr_mode_sub(ID0G8_MODE_ENABLE, 0);			

		}
		break;

	case ID0G8_MODE_ENABLE:

		if (!(_is_id0g8_enable()))
		{
			_id0g8_intr_mode_sub(ID0G8_MODE_DISABLE, 0);			
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
		}
		break;

	default:					/* other */
		break;
	}
	
	_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DIPSW_READ, WAIT_TIME_DIPSW_READ, 0, 0);

}


/*********************************************************************//**
 * @brief Hold1 times up message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id0g8_timeup_escrow_hold1_proc(void)//_id0g8_timeup_escrow_mode_hold
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID0G8_MODE_ESCROW_WAIT_ACK:
	case ID0G8_MODE_ESCROW:
		_set_id0g8_reject(ID0G8_MODE_RETURN, REJECT_CODE_RETURN);	/* UBAと同様にタイムアウト返却はReturned	*/
		break;
	default:					/* other */
		break;
	}
}


void _id0g8_timeup_re_send_proc(void)//ok 移植
{
	if(s_id0g8_info.wait_event_response == true)
	{
		// TIDイベントの再送(TIDは同じ)
		_id0g8_set_same_tid_event(ex_cline_status_tbl.Tid);
	}
}


void _id0g8_intr_mode_sub(u16 mode, u8 wait_flag)//ok
{
	ex_cline_status_tbl.line_task_mode = mode;

	#if defined(ID0G8_LOG)
	save_log_0g8(0);

	#endif

	switch(mode)
	{
	case ID0G8_MODE_DISABLE:
		_id0g8_trans_mode_disable();
		break;
	case ID0G8_MODE_ENABLE:
		_id0g8_trans_mode_enable();
		break;
	case ID0G8_MODE_ESCROW_WAIT_ACK:
	case ID0G8_MODE_ESCROW:		/* Ack受信済み */
		_id0g8_trans_mode_accepted_wait_ack();
		break;
	case ID0G8_MODE_POWERUP_ERROR:
	case ID0G8_MODE_ERROR:
		_is_id0g8_error_sts(ex_cline_status_tbl.error_code, 0);
		break;
	}

}

u16 _id0g8_dipsw_disable(void)//ok
{
	u16 dipsw_disable;

	dipsw_disable = (ex_dipsw1 & 0x7F);

	return dipsw_disable;
}

void _set_id0g8_reject(u16 mode, u16 code)//ok
{
	if (ex_recovery_info.step != RECOVERY_STEP_NON)
	{
		set_recovery_step(RECOVERY_STEP_NON);
	}

	ex_cline_status_tbl.reject_code = code;
	_id0g8_intr_mode_sub( mode, 0);

	s_id0g8_2nd_note = 0;
	ex_cline_status_tbl.escrow_code_2nd_note = 0;

	_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
}


void _set_id0g8_alarm(u16 mode, u16 code)//ok
{
	if (ex_recovery_info.step != RECOVERY_STEP_NON)
	{
		set_recovery_step(RECOVERY_STEP_NON);
	}

	ex_cline_status_tbl.error_code = code;
	_id0g8_intr_mode_sub( mode, 0);

}

/*********************************************************************//**
 * @brief is accept enable
 * @param[in]	None
 * @return true  : accept enable
 * @return false : accept disable
 **********************************************************************/
bool _is_id0g8_enable(void)
{
	extern const u8 id0g8_bill_info[18][6];
	extern const u16 id0g8_inhi_mask;
	extern const u16 dipsw_inhi[];
	extern const u8 id0g8_index_to_code[];
	u16 inhi = 0;
	u16 denomi_idx;
	u16 binfo_idx;
	bool rtn = true;
/***/
    for( denomi_idx = 0; denomi_idx < (DENOMI_SIZE / 2); denomi_idx++ )
    {
		inhi|= dipsw_inhi[denomi_idx];
    }
	if(( inhi != 0 ) && ( inhi == (ex_cline_status_tbl.dipsw_disable & inhi) ) ) 
	{
		rtn = false;
	}

    if( ex_cline_status_tbl.accept_disable )
    {
        rtn = false;
    }

	return rtn;
}

/*********************************************************************//**
 * @brief is denomi inhibit
 * @param[in]	None
 * @return true  : inhibit
 * @return false : accept
 **********************************************************************/
bool _is_id0g8_denomi_inhibit(u32 denomi_code, u32 direction)
{
#if (MULTI_COUNTRY == 0)
	extern const u8 id0g8_bill_info[18][6];
#else  /* MULTI_COUNTRY != 0 */
	extern const u8 id0g8_bill_info[MULTI_COUNTRY][18][6];
#endif /* MULTI_COUNTRY == 0 */
	extern const u16 dipsw_inhi[];
	bool rtn = false;

	if ((ex_cline_status_tbl.dipsw_disable & dipsw_inhi[denomi_code]) != 0)
	{
		rtn = true;
	}
	if(accept_denomi[denomi_code] == 0)
	{
		rtn = true;
	}

	return rtn;
}



void _is_id0g8_error_sts(u16 alarm_code, u8 *data) // ok _id0g8_set_error
{

	ex_0g8_recovery = 0;
	_id0g8_denomi_clear();

	u8 rtn;
	switch (alarm_code)
	{
	/*
	case ALARM_CODE_BOOT_AREA:
	case ALARM_CODE_BOOTIF_AREA:
	case ALARM_CODE_TASK_AREA:
	case ALARM_CODE_COUNTRY_AREA:
	case ALARM_CODE_BASE_DATA_AREA:
	case ALARM_CODE_IN_RAM:
	case ALARM_CODE_EX_RAM:
	case ALARM_CODE_ROM_WRITE:
	case ALARM_CODE_EEPROM:
	case ALARM_CODE_DOWNLOAD:
	case ALARM_CODE_MAG:
	case ALARM_CODE_I2C:
	case ALARM_CODE_PL_OFF:
	case ALARM_CODE_CISA_OFF:
	case ALARM_CODE_CISB_OFF:
	*/
	case ALARM_CODE_STACKER_FULL:
		ex_tid_event_tbl.stacker_status = STACKER_STATUS_FULL;
		_id0g8_set_event(ID0G8_EVT_STACKER_STATUS, ex_tid_event_tbl.stacker_status);
		break;

	case ALARM_CODE_FEED_OTHER_SENSOR_SK:
	case ALARM_CODE_FEED_SLIP_SK:
	case ALARM_CODE_FEED_TIMEOUT_SK:
	case ALARM_CODE_FEED_LOST_BILL:
	case ALARM_CODE_FEED_MOTOR_LOCK_SK:

	case ALARM_CODE_STACKER_MOTOR_LOCK:

	case ALARM_CODE_STACKER_GEAR:
	case ALARM_CODE_STACKER_TIMEOUT:
	case ALARM_CODE_STACKER_HOME:
		ex_tid_event_tbl.stacker_status = STACKER_STATUS_JAM;
		_id0g8_set_event(ID0G8_EVT_STACKER_STATUS, ex_tid_event_tbl.stacker_status);
		break;

	case ALARM_CODE_FEED_MOTOR_SPEED_LOW:
	case ALARM_CODE_FEED_MOTOR_SPEED_HIGH:
		// mechanical error
		ex_fail_data.fail_type |= FAIL_TYPE_MECH;
		ex_fail_data.diagnostics = 0;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
		break;
		
	case ALARM_CODE_FEED_MOTOR_LOCK:
	case ALARM_CODE_FEED_MOTOR_LOCK_REJ:
		// mechanical error
		ex_fail_data.fail_type |= FAIL_TYPE_MECH;
		ex_fail_data.diagnostics = 0;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
		break;

	case ALARM_CODE_FEED_OTHER_SENSOR_AT:
	case ALARM_CODE_FEED_SLIP_AT:
	case ALARM_CODE_FEED_TIMEOUT_AT:
	case ALARM_CODE_FEED_MOTOR_LOCK_AT:

	case ALARM_CODE_FEED_OTHER_SENSOR_ENT:
	case ALARM_CODE_FEED_SLIP_ENT:
	case ALARM_CODE_FEED_TIMEOUT_ENT:
	case ALARM_CODE_FEED_MOTOR_LOCK_ENT:

	case ALARM_CODE_SIDE_LOW_LEVEL:
	case ALARM_CODE_SIDE_HIGH_LEVEL:
	case ALARM_CODE_UV_LOW_LEVEL:
		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_JAM;
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status);
		break;

	case ALARM_CODE_APB_HOME:
	case ALARM_CODE_APB_TIMEOUT:
	case ALARM_CODE_APB_HOME_STOP:
		// mechanical error
		ex_fail_data.fail_type |= FAIL_TYPE_MECH;
		ex_fail_data.diagnostics = 0;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
		break;

	case ALARM_CODE_BOX:
	case ALARM_CODE_BOX_INIT:
		ex_tid_event_tbl.stacker_status = STACKER_STATUS_DISCONNECT;

		/* iVIZIONではPOWERUP中にStacker Disconnect通知しない,イニシャル動作前に通知する */
		if(ex_cline_status_tbl.line_task_mode == ID0G8_MODE_POWERUP_ERROR)
		{
			s_id0g8_powerup_box_disconnect = 0xFF;
		}
		else
		{
			_id0g8_set_event(ID0G8_EVT_STACKER_STATUS, ex_tid_event_tbl.stacker_status);
		}
		break;

	case ALARM_CODE_CHEAT:
		ex_tid_event_tbl.note_ticket_status = NOTE_TICKET_STATUS_CHEAT;
		_id0g8_set_event(ID0G8_EVT_NOTE_TICKET_STATUS, ex_tid_event_tbl.note_ticket_status );
		break;

	case ALARM_CODE_FRAM:
	case ALARM_CODE_CENTERING_TIMEOUT:
	case ALARM_CODE_CENTERING_HOME:
	case ALARM_CODE_CENTERING_HOME_REMOVAL:
	case ALARM_CODE_CENTERING_HOME_STOP:
	case ALARM_CODE_RFID_UNIT_MAIN:					/* ICB有効でRFIDユニットがない場合 */
	case ALARM_CODE_RFID_ICB_SETTING: 			/* ICB無効でICBユニットがある場合 */
	case ALARM_CODE_RFID_ICB_SETTING_VALUE:	 	/* ICB有効無効フラグ、マシンナンバー未設定 */
	case ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN: 		/* ICB読み込み、書込み失敗 */
	case ALARM_CODE_RFID_ICB_DATA: 				/* ICBチェックサムデータ異常 */
	case ALARM_CODE_RFID_ICB_NUMBER_MISMATCH:	/* ICB別のゲーム機にセットされていたBOXです */
	case ALARM_CODE_RFID_ICB_NOT_INITIALIZE: 	/* ICB集計データセーブ済みのBOXです */
	case ALARM_CODE_RFID_ICB_MC_INVALID:	 	/* ICBマシンナンバー無効 */
		// mechanical error
		ex_fail_data.fail_type |= FAIL_TYPE_MECH;
		ex_fail_data.diagnostics = 0;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
		break;

//	case ALARM_CODE_STACKER_KEY:
//		rtn = ID0G8_STS_EXT_BOXKEY_OPEN;
//		break;
	/*
	case ALARM_CODE_STACKER_LOSS:
	case ALARM_CODE_DISPENSE_OTHERSENS_AT:
	case ALARM_CODE_DISPENSE_TIMEOUT_AT:
	case ALARM_CODE_DISPENSE_SLIP_AT:
	*/
	/*
	case ALARM_CODE_FEED_RUNAWAY:
	case ALARM_CODE_STACKER_RUNAWAY:
	case ALARM_CODE_APB_RUNAWAY:
	case ALARM_CODE_CENTERING_RUNAWAY:
	*/
	/*
	case ALARM_CODE_FEED_FORCED_QUIT:
	case ALARM_CODE_STACKER_FORCED_QUIT:
	case ALARM_CODE_APB_FORCED_QUIT:
	case ALARM_CODE_CENTERING_FORCED_QUIT:
	*/
	/*
	case ALARM_CODE_SETUP_WRONG:
		rtn = ID0G8_STS_FAILURE;
		*data = ID0G8_FAILURE_SETUP_WRONG;
		break;
	*/
	/*

	case ALARM_CODE_DISPENSE_NO_NOTE:
	case ALARM_CODE_DISPENSE_NOTE_REMOVE:
	*/
	default:					/* other */
		ex_fail_data.fail_type |= FAIL_TYPE_MECH;
		ex_fail_data.diagnostics = 0;
		_id0g8_set_event(ID0G8_EVT_FAIL_STATUS, ex_fail_data.fail_type);
		break;
	}

}


//void interface_get_bar_info(u8 *type, u8 *length, u8 *length2, u8 *inhibit)
void interface_get_bar_info(u8 *type, u8 *length, u8 *length2, u8 *length3,u8 *length4,u8 *inhibit)//ok
{
#if defined(PRJ_IVIZION2)
	//ID-0G8 enable 16char～24char, disable 20, 26, 28 char
    *type = 1 /* BARTYPE*/;						/* Barcode type */
    *length = 18;
    *length2 = 16;
    *length3 = 22;
    *length4 = 24;
    *inhibit = 0x03;	/* Bar Enable/Disable 1:札のみ 2:ﾊﾞｰｸｰﾎﾟﾝ 3:両方*/
#else
	//ID-0G8 enable 18char～24char, disable 16,26,28char
    *type = 1 /* BARTYPE*/;						/* Barcode type */
    *length = 18;
    *length2 = 20;
    *length3 = 22;
    *length4 = 24;
    *inhibit = 0x03;	/* Bar Enable/Disable 1:札のみ 2:ﾊﾞｰｸｰﾎﾟﾝ 3:両方*/
#endif
}

u8 check_failure_0g8(u16 alarm_code)
{
	/* return 1 if failure */
	switch (alarm_code)
	{

	case ALARM_CODE_STACKER_FULL:
		return(0);

	case ALARM_CODE_FEED_OTHER_SENSOR_SK:
	case ALARM_CODE_FEED_SLIP_SK:
	case ALARM_CODE_FEED_TIMEOUT_SK:
	case ALARM_CODE_FEED_LOST_BILL:
	case ALARM_CODE_FEED_MOTOR_LOCK_SK:

	case ALARM_CODE_STACKER_MOTOR_LOCK:

	case ALARM_CODE_STACKER_GEAR:
	case ALARM_CODE_STACKER_TIMEOUT:
	case ALARM_CODE_STACKER_HOME:
		return(0);

	case ALARM_CODE_FEED_MOTOR_SPEED_LOW:
	case ALARM_CODE_FEED_MOTOR_SPEED_HIGH:
		// mechanical error
		return(1);
		
	case ALARM_CODE_FEED_MOTOR_LOCK:
	case ALARM_CODE_FEED_MOTOR_LOCK_REJ:
		// mechanical error
		return(1);

	case ALARM_CODE_FEED_OTHER_SENSOR_AT:
	case ALARM_CODE_FEED_SLIP_AT:
	case ALARM_CODE_FEED_TIMEOUT_AT:
	case ALARM_CODE_FEED_MOTOR_LOCK_AT:

	case ALARM_CODE_SIDE_LOW_LEVEL:
	case ALARM_CODE_SIDE_HIGH_LEVEL:
	case ALARM_CODE_UV_LOW_LEVEL:
		return(0);

	case ALARM_CODE_APB_HOME:
	case ALARM_CODE_APB_TIMEOUT:
	case ALARM_CODE_APB_HOME_STOP:
		// mechanical error
		return(1);

	case ALARM_CODE_BOX:
	case ALARM_CODE_BOX_INIT:
		return(0);

	case ALARM_CODE_CHEAT:
		return(0);

	case ALARM_CODE_CENTERING_TIMEOUT:
	case ALARM_CODE_CENTERING_HOME:
	case ALARM_CODE_CENTERING_HOME_REMOVAL:
	case ALARM_CODE_CENTERING_HOME_STOP:
	case ALARM_CODE_RFID_UNIT_MAIN:					/* ICB有効でRFIDユニットがない場合 */
	case ALARM_CODE_RFID_ICB_SETTING: 			/* ICB無効でICBユニットがある場合 */
	case ALARM_CODE_RFID_ICB_SETTING_VALUE:	 	/* ICB有効無効フラグ、マシンナンバー未設定 */
	case ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN: 		/* ICB読み込み、書込み失敗 */
	case ALARM_CODE_RFID_ICB_DATA: 				/* ICBチェックサムデータ異常 */
	case ALARM_CODE_RFID_ICB_NUMBER_MISMATCH:	/* ICB別のゲーム機にセットされていたBOXです */
	case ALARM_CODE_RFID_ICB_NOT_INITIALIZE: 	/* ICB集計データセーブ済みのBOXです */
	case ALARM_CODE_RFID_ICB_MC_INVALID:	 	/* ICBマシンナンバー無効 */
		// mechanical error
		return(1);

	default:					/* other */
		return(1);

	}

}


bool _is_id0g8_bar_inhibit(void)
{
	extern const u16 dipsw_bar_inhi;
	bool rtn = false;

	/* Check DipSW Inhibit */
	if ((ex_cline_status_tbl.dipsw_disable & dipsw_bar_inhi) != 0)
	{
		rtn = true;
	}

	return rtn;
}

