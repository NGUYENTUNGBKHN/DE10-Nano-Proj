/******************************************************************************/
/*! @addtogroup Main
    @file       feed_task.h
    @brief      feed task function
    @file       feed_task.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/
// メインタスクからのメッセージにより、搬送動作を制御する。
//搬送ポジションセンサーイベントを監視する。
//搬送モーターイベントを監視する。
//モータータスクに搬送モーターの動作/停止メッセージを送る。"

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "sub_functions.h"
#include "motor_ctrl.h"
#include "sensor.h"
#include "sensor_ad.h"
#include "status_tbl.h"
#if defined(UBA_RTQ)
#include "if_rc.h"
#include "feed_rc_unit.h"
#endif // UBA_RTQ

#include "feed.h" //2024-08-01

#define EXT
#include "com_ram.c"

#include "jdl_conf.h"


/************************** Variable declaration *****************************/
static T_MSG_BASIC feed_msg;

/************************** PRIVATE VARIABLES *************************/
T_MSG_BASIC feed_msg;
u16 s_feed_task_wait_seq;
u16 s_feed_task_pause_seq;
u8 s_feed_alarm_code;
u8 s_feed_alarm_retry;
u8 s_feed_reject_code;
u8 s_feed_reject_option;
u16 s_feed_sensor_backup;
u8 s_feed_aging_no;
u8 s_feed_freerun_dir;
u8 s_feed_search_option;
u8 s_feed_pullback_off;	//2023-11-01
u16 backup_feed_apb_sequence; // use 0x04XX
bool ex_reject_escrow;

/************************** EXTERN FUNCTIONS *************************/
extern const u16 banknote_length_min;		/*	LENGTH MIN */
extern const u16 banknote_length_max;	 	/*	LENGTH MAX */
/************************** 				 *************************/
typedef struct _FEED_SEQ_TABLE
{
	u16 pulse;
	u16 next_seq;
} FEED_SEQ_TABLE;

const FEED_SEQ_TABLE feed_initial_table[] =
{ /*                      Feed Pulse                       */ /* next sequence */
 {                                                         0, 0x0101}	/* [Sequence:0x0100] wait motor start */
,{                                                         0, 0x0102}	/* [Sequence:0x0101] reverse motor wait stability */
,{     (u16)(FEED_SET_SLIP(DIST_ENT_TO_PBI-DIST_ENT_TO_CEN)), 0x0103}	/* [Sequence:0x0102] check reverse motor remain note */
,{                                                         0, 0x0104}	/* [Sequence:0x0103] stop motor */
,{                                                         0, 0x0105}	/* [Sequence:0x0104] forward motor wait stability */
,{     (u16)(FEED_SET_SLIP(DIST_ENT_TO_PBI-DIST_ENT_TO_CEN)), 0x0106}	/* [Sequence:0x0105] check forward motor remain note */
,{                                                         0,      0}	/* [Sequence:0x0106] stop motor */
};

const FEED_SEQ_TABLE feed_cnetering_table[] =
{ /*                      Feed Pulse                       */ /* next sequence */
  {                                                         0, 0x0201}	/* [Sequence:0x0200] wait motor start */
 ,{                                                         0, 0x0202}	/* [Sequence:0x0201] feed centering-sensor ON position */
 ,{                                                         0,      0}	/* [Sequence:0x0202] stop motor */
};

const u16 feed_aging_pulse[] =
{ /*                            Feed Pulse                                */
                                     (u16)(CONV_PULSE((DIST_ENT_TO_CEN+20)))	/* [Sequence:0x0F00] aging centering */
 ,                (u16)(CONV_PULSE((DIST_CEN_TO_ESP+LENGTH_UPPER_LIMIT+10)))	/* [Sequence:0x0F01] aging escrow */
 ,                   (u16)(CONV_PULSE((DIST_ESP_TO_PBI+DIST_PBI_TO_PBO+10)))	/* [Sequence:0x0F02] aging APB */
 ,                   (u16)(CONV_PULSE((DIST_PBO_TO_EXI+DIST_EXI_TO_STP+10)))	/* [Sequence:0x0F03] aging stack */
 ,(u16)(CONV_PULSE((LENGTH_UPPER_LIMIT+DIST_CEN_TO_EXI+DIST_CEN_TO_REP+10)))	/* [Sequence:0x0F04] aging reject */
 ,(u16)(CONV_PULSE((LENGTH_UPPER_LIMIT+DIST_CEN_TO_EXI+DIST_CEN_TO_REP+10)))	/* [Sequence:0x0F05] aging payout */
};

#define FEED_SEARCH_RETRY 5
/* Rev time, Fwd time, */
const u16 search_time[FEED_SEARCH_RETRY][4] =
{
	{ 500, 250},/* 1*/
	{ 600, 300},/* 2*/
	{ 700, 350},/* 3*/
	{ 800, 400},/* 4*/
	{ 900, 450},/* 5*/
};

//u32 s_feed_retention_pulse;
/************************** PRIVATE FUNCTIONS *************************/
void feed_task(VP_INT exinf);
void _initialize_feed_motor_pulse(void);
void _initialize_position_pulse(void);
void _check_position_pulse(void);

void _feed_initialize(void);
void _feed_init_motor_speed(void);
void _feed_idle_proc(void);
void _feed_idle_msg_proc(void);
void _feed_busy_proc(void);
void _feed_busy_msg_proc(void);

/* Feed idle (Stop Motor) sequence */
void _feed_idle_seq_proc(u32 flag);
//void _feed_idle_0000_seq(u32 flag);
void _feed_idle_0001_seq(u32 flag);

/* Feed initialize sequence */
static u16 s_tmp_cis_encoder_pulse;
void _feed_initial_seq_proc(u32 flag);
void _feed_initial_0100_seq(u32 flag);
void _feed_initial_0101_seq(u32 flag);
void _feed_initial_0102_seq(u32 flag);
void _feed_initial_0103_seq(u32 flag);
void _feed_initial_0104_seq(u32 flag);
void _feed_initial_0105_seq(u32 flag);
void _feed_initial_0106_seq(u32 flag);

/* Feed to centering position */
void _feed_centering_seq_proc(u32 flag);
void _feed_centering_0200_seq_start(void);
void _feed_centering_0200_seq(u32 flag);
void _feed_centering_0210_seq(u32 flag);
void _feed_centering_0220_seq(u32 flag);
void _feed_centering_0230_seq(u32 flag);

/* Feed to escrow position */
void _feed_escrow_seq_proc(u32 flag);
void _feed_escrow_0300_seq2_start(void);
void _feed_escrow_0300_seq2(u32 flag);
void _feed_escrow_0303_seq2(u32 flag);
void _feed_escrow_0304_seq2(u32 flag);
void _feed_escrow_0305_seq2(u32 flag);
void _feed_escrow_0306_seq2(u32 flag);
void _feed_escrow_0307_seq2(u32 flag);
void _feed_escrow_0387_seq2(u32 flag);
void _feed_escrow_0308_seq2(u32 flag);

/* Feed to apb position */
void _feed_apb_seq_proc(u32 flag);	// only use SS not use RTQ
void _feed_apb_0400_seq_start(void);//ok//#if defined(SPEED_UP)
void _feed_apb_0400_seq(u32 flag);//ok
void _feed_apb_0410_seq(u32 flag);//ok
void _feed_apb_0420_seq(u32 flag);//ok
void _feed_apb_0430_seq(u32 flag);//ok
void _feed_apb_0440_seq(u32 flag);//ok
void _feed_apb_0450_seq(u32 flag);// ok

// ポーズからの復帰
void _feed_apb_0460_seq(u32 flag);	//ok
void _feed_apb_0470_seq(u32 flag);	//ok
void _feed_apb_0480_seq(u32 flag);	//ok
void _feed_apb_0490_seq(u32 flag);	//ok
void _feed_apb_04A0_seq(u32 flag);	//ok
void _feed_apb_04B0_seq(u32 flag);	//ok
void _feed_apb_04C0_seq(u32 flag);	//ok
void _feed_apb_04E0_seq(u32 flag);	//ok
void _feed_apb_04F0_seq(u32 flag);	//ok
void _feed_set_apb_pause(void);	//ok
void _feed_set_feed_apb_retry(u32 alarm_code, u16 error_sequence);// use 第2引数が必要か不明

/* Force feed to stack position */
void _feed_force_stack_seq_proc(u32 flag);
void _feed_fstack_0600_seq(u32 flag);
void _feed_fstack_0601_seq(u32 flag);
void _feed_fstack_0602_seq(u32 flag);
void _feed_fstack_0603_seq(u32 flag);
void _feed_fstack_0604_seq(u32 flag);
void _feed_fstack_0605_seq(u32 flag);
void _feed_fstack_0606_seq(u32 flag);
void _feed_fstack_0608_seq(u32 flag);
void _feed_fstack_0609_seq(u32 flag);
void _feed_set_fstack_pause(void);
void _feed_fstack_0610_seq(u32 flag);
void _feed_fstack_0611_seq(u32 flag);
void _feed_fstack_0612_seq(u32 flag);
void _feed_fstack_0613_seq(u32 flag);
void _feed_set_fstack_retry(u32 alarm_code);
void _feed_fstack_0620_seq(u32 flag);
void _feed_fstack_0621_seq(u32 flag);
void _feed_fstack_0680_seq(u32 flag);
void _feed_fstack_0681_seq(u32 flag);

/* Reject */
void _feed_set_feed_reject_retry(u32 alarm_code);	// only 0x07XX
void _feed_reject_seq_proc2(u32 flag);
void _feed_reject_0700_seq2(u32 flag);	/* REV*/
void _feed_reject_0702_seq2(u32 flag);	/* REV*/
void _feed_reject_0703_seq2(u32 flag);	/* REV*/
void _feed_reject_0704_seq2(u32 flag);	/* REV*/
void _feed_reject_0706_seq2(u32 flag);	/* REV*/
void _feed_reject_0708_seq2(u32 flag);	/* REV*/
void _feed_reject_070A_seq2(u32 flag);	/* REV*/
void _feed_reject_070C_seq2(u32 flag);	/* REV*/
void _feed_reject_0710_seq2(u32 flag);	/* FWD*/
void _feed_reject_0712_seq2(u32 flag);	/* FWD*/

#if defined(UBA_RTQ)
void _entry_back_seq_proc(u32 flag);
void _entry_back_0b00_seq(u32 flag);
void _entry_back_0b01_seq(u32 flag);
void _entry_back_0b02_seq(u32 flag);
#define	ENTRY_BACK_TIME					100

void _feed_rc_payout_stop_seq_proc(u32 flag);
#endif

/* Serch bill */
void _feed_search_seq_proc(u32 flag);
void _feed_search_0a00_seq(u32 flag);
void _feed_search_0a01_seq(u32 flag);
void _feed_search_0a02_seq(u32 flag);
void _feed_search_0a03_seq(u32 flag);
void _feed_search_0a04_seq(u32 flag);
void _feed_search_0a05_seq(u32 flag);

/* Motor tset */
void _feed_aging_seq_proc(u32 flag);
void _feed_aging_0e00_seq(u32 flag);
void _feed_aging_0e01_seq(u32 flag);
void _feed_aging_0e02_seq(u32 flag);

void _feed_freerun_seq_proc(u32 flag);
void _feed_freerun_0f00_seq(u32 flag);
void _feed_freerun_0f01_seq(u32 flag);
void _feed_freerun_0f02_seq(u32 flag);

//2024-03-18a
void _entry_feed_force_rev_seq_proc(u32 flag);
void _entry_feed_force_rev_0800_seq(u32 flag);
void _entry_feed_force_rev_0802_seq(u32 flag);
void _entry_feed_force_rev_0806_seq(u32 flag);
void _entry_feed_force_rev_0808_seq(u32 flag);

#if !defined(UBA_RTQ)//#if defined(HIGH_SECURITY_MODE)
void _feed_rev_seq_proc(u32 flag);
void _feed_rev_0500_seq_start(void);
void _feed_rev_0500_seq(u32 flag);
void _feed_rev_0502_seq(u32 flag);
void _feed_rev_0504_seq(u32 flag);
void _feed_rev_0508_seq(u32 flag);
void _feed_rev_050A_seq(u32 flag);
#endif

/* Centering sub functions */
void _feed_set_seq(u16 seq, u16 time_out);
void _feed_set_waiting_seq(u32 seq);
void _feed_set_reject(u32 reject_code);
void _feed_set_alarm(u32 alarm_code);
bool _is_feed_all_sensor_off(void);
void _feed_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _feed_system_error(u8 fatal_err, u8 code);


/*********************************************************************//**
 * @ feed task
 * @param[in]	extension information
 * @return 		None
 **********************************************************************/
void feed_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u16 aa;

	_feed_initialize();						/* feed task initialize */

	while (1)
	{
		if (ex_feed_task_seq == FEED_SEQ_IDLE)
		{
		/* idle */
			ercd = trcv_mbx(ID_FEED_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
			if (ercd == E_OK)
			{
				memcpy(&feed_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(feed_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_feed_system_error(1, 3);
				}
				_feed_idle_proc();
				_feed_idle_msg_proc();
			}
			else
			{
				_feed_idle_proc();
			}
		}
		else
		{
		/* busy */
			_feed_busy_proc();
			ercd = prcv_mbx(ID_FEED_MBX, (T_MSG **)&tmsg_pt);
			if (ercd == E_OK)
			{
				memcpy(&feed_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(feed_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_feed_system_error(1, 4);
				}
				_feed_busy_msg_proc();
			}
		}
	}
}


/*********************************************************************//**
 * @brief initialize feed task
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _feed_initialize(void)
{
	tx_thread_vfp_enable();

	ex_feed_task_seq = FEED_SEQ_IDLE;
	s_feed_task_wait_seq = FEED_SEQ_IDLE;

	/* set feed speed */
	_feed_init_motor_speed();
}


/*********************************************************************//**
 * @brief initialize feed motor speed value
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _feed_init_motor_speed(void)
{
	memset(ex_feed_motor_speed, 0, sizeof(ex_feed_motor_speed));
	// Initial value MOTOR_MAX_SPEED, set_speed decrease in _feed_initial
#if defined(UBA_RTQ)
	ex_feed_motor_speed[FEED_SPEED_CENTERING].set_speed = FEED_MOTOR_SPEED_600MM;
	ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed = FEED_MOTOR_SPEED_600MM;
	ex_feed_motor_speed[FEED_SPEED_APB].set_speed = FEED_MOTOR_SPEED_550M;
	ex_feed_motor_speed[FEED_SPEED_STACK].set_speed = FEED_MOTOR_SPEED_550M;
	ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed = FEED_MOTOR_SPEED_550M;
	ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed = FEED_MOTOR_SPEED_550M;
	ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed = FEED_MOTOR_SPEED_FULL;
	ex_feed_motor_speed[FEED_SPEED_DISABLE].set_speed = FEED_MOTOR_SPEED_550M;
#else
	ex_feed_motor_speed[FEED_SPEED_CENTERING].set_speed = FEED_MOTOR_SPEED_600MM;
	ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed = FEED_MOTOR_SPEED_600MM;
	ex_feed_motor_speed[FEED_SPEED_APB].set_speed = FEED_MOTOR_SPEED_FULL;
	ex_feed_motor_speed[FEED_SPEED_STACK].set_speed = FEED_MOTOR_SPEED_FULL;
	ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed = FEED_MOTOR_SPEED_FULL;
	ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed = FEED_MOTOR_SPEED_600MM;
	ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed = FEED_MOTOR_SPEED_FULL;
	ex_feed_motor_speed[FEED_SPEED_DISABLE].set_speed = FEED_MOTOR_SPEED_600MM;
#endif // UAB_RTQ

	clr_flg(ID_FEED_CTRL_FLAG, ~EVT_ALL_BIT);
}


/*********************************************************************//**
 * @brief feed task idle procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _feed_idle_proc(void)
{
	FLGPTN flag = 0;
	ER ercd;

	ercd = pol_flg(ID_FEED_CTRL_FLAG, EVT_ALL_BIT, TWF_ORW, &flag);
	if ((ercd == E_OK) && (flag != 0))
	{
		#if 0 
		// UBA500では処理していない
		#endif
	}
}


/*********************************************************************//**
 * @brief MBX message procedure
 *  feed task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _feed_idle_msg_proc(void)
{
	switch (feed_msg.tmsg_code)
	{
	case TMSG_FEED_INITIAL_REQ:			/* INITIAL message */
		_initialize_position_pulse();
		_initialize_feed_motor_pulse();
		_feed_init_motor_speed();
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		_feed_set_seq(FEED_SEQ_INITIAL, FEED_SEQ_TIMEOUT);
		break;

	case TMSG_FEED_CENTERING_REQ:		/* CENTERING message */
		_initialize_position_pulse();
		_initialize_feed_motor_pulse();
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		_feed_set_seq(FEED_SEQ_CENTERING, FEED_SEQ_TIMEOUT);
		_feed_centering_0200_seq_start();
		break;

	case TMSG_FEED_ESCROW_REQ:			/* ESCROW message */
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		_feed_set_seq(FEED_SEQ_ESCROW, FEED_SEQ_TIMEOUT);
		_feed_escrow_0300_seq2_start();
		break;

	case TMSG_FEED_APB_REQ:				/* APB message */// only SS
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		_feed_set_seq(FEED_SEQ_APB, FEED_SEQ_TIMEOUT);
		_feed_apb_0400_seq_start();
		break;

	#if !defined(UBA_RTQ)//#if defined(HIGH_SECURITY_MODE) //Check for remaining bills on the box transport path after stack
	case TMSG_FEED_REV_CHECK_BILL_REQ:
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		_feed_set_seq(FEED_SEQ_REV_CHECK_BILL, FEED_SEQ_TIMEOUT);
		_feed_rev_0500_seq_start();
		break;
	#endif

	case TMSG_FEED_FORCE_STACK_REQ:
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		_feed_set_seq(FEED_SEQ_FORCE_STACK, FEED_SEQ_TIMEOUT);
		break;

	case TMSG_FEED_REJECT_REQ:			/* REJECT message */
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = feed_msg.arg2; //リトライ
		s_feed_reject_code = 0;
		s_feed_reject_option = feed_msg.arg1;
		s_feed_pullback_off = feed_msg.arg3; //2023-11-01
		_feed_set_seq(FEED_SEQ_REJECT, FEED_SEQ_TIMEOUT);
		_check_position_pulse();
		break;

	case TMSG_FEED_SEARCH_REQ:
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		s_feed_search_option = feed_msg.arg1;	//use
#if defined(UBA_RTQ)
		feed_rc_set_search_dir(feed_msg.arg1);
#endif // UBA_RTQ
		_feed_set_seq(FEED_SEQ_SEARCH, FEED_SEQ_TIMEOUT);
		break;

	#if defined(UBA_RTQ)
	case TMSG_ENTRY_BACK_REQ:	//UBA700では必要ない処理、UBA500は入口ローラが左右に分離する機能なので、返却後に回したかったらしい
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		_feed_set_seq(ENTRY_SEQ_BACK, FEED_SEQ_TIMEOUT);
		_ir_feed_motor_da_control = 0;
		break;
	#endif

	case TMSG_FEED_AGING_REQ:			/* AGING message */
		s_feed_alarm_code = 0;
		s_feed_reject_code = 0;
		s_feed_aging_no = feed_msg.arg1;
		s_feed_pullback_off = 0;
		_feed_set_seq(FEED_SEQ_AGING, FEED_SEQ_TIMEOUT);
		break;
	case TMSG_FEED_FREERUN_REQ:			/* FREERUN message */
		s_feed_alarm_code = 0;
		s_feed_reject_code = 0;
		s_feed_freerun_dir = feed_msg.arg1;
		s_feed_pullback_off = 0;
		_feed_set_seq(FEED_SEQ_FREERUN, FEED_SEQ_TIMEOUT);
		break;

	case TMSG_FEED_FORCE_REV_REQ: //2024-03-18a ID-003 Disable, Forced return of bills detected at entrance
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		_feed_set_seq(FEED_SEQ_FORCE_REV, FEED_SEQ_TIMEOUT);
		break;

#if defined(UBA_RTQ)
	case TMSG_FEED_RC_STACK_REQ:
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0; //もとからリトライがないので、必要ない
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		
		feed_rc_set_unit(feed_msg.arg1);
		feed_rc_set_recover(feed_msg.arg2);
		set_recovery_count(feed_msg.arg1); //2025-04-18
		
		feed_rc_set_sensor_bk(0);
		_feed_set_seq(FEED_SEQ_RC_STACK, FEED_SEQ_RC_STACK_TIMEOUT);

		_feed_rc_stack_1200_seq_start();

		break;
	case TMSG_FEED_RC_PAYOUT_REQ:
		s_feed_alarm_code = 0;
		//s_feed_alarm_retry = 0; //UBA500RTQは初期化していない、ここで初期化すると無限リトライになるから
								//理想は、通常返却と同様に1回目のみ初期化
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;

		if (feed_msg.arg2 != FEED_PAYOUT_OPTION_RETRY)
		{
			ex_rc_internal_jam_flag = feed_msg.arg1;		/* '19-09-19 */
			ex_rc_internal_jam_flag_bk = 0;
		}
		
		feed_rc_set_unit(feed_msg.arg1);				// drum init
		feed_rc_set_payout_option(feed_msg.arg2);		// option
		set_recovery_count(feed_msg.arg1); //2025-04-18
		feed_rc_set_sensor_bk(0);

		if(is_rc_rs_unit())
		{
			/* RS */
			if (ex_main_test_no == TEST_RC_AGING || ex_main_test_no == TEST_RC_AGING_FACTORY)
			{
				_feed_set_seq(FEED_SEQ_RC_PAYOUT, FEED_SEQ_RC_TIMEOUT);
			}
			else
			{
				feed_rc_set_payout_last(feed_msg.arg3);
				_feed_set_seq(FEED_SEQ_RS_PAYOUT, FEED_SEQ_RC_TIMEOUT);
			}
		}
		else
		{
			/* RC */
			_feed_set_seq(FEED_SEQ_RC_PAYOUT, FEED_SEQ_RC_TIMEOUT);
		}

		if (feed_msg.arg2 != FEED_PAYOUT_OPTION_RETRY)
		{
			set_recovery_step(RECOVERY_STEP_PAYOUT_DRUM);
			s_feed_alarm_retry = 0;		//1回目のみクリア、ここ以外で0にすると無限リトライになる
		
			motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, 
												FEED_HUNGING_POSITION_START);
		}
		
		break;

	case	TMSG_FEED_RC_PAYOUT_STOP_REQ:	/* '21-10-11 */
			/* clear code */
			s_feed_alarm_code = 0;
			s_feed_reject_code = 0;
			/* set payout sequence */
			_feed_set_seq(FEED_SEQ_RC_PAYOUT_STOP, FEED_SEQ_RC_TIMEOUT);
			break;

	case TMSG_FEED_RC_COLLECT_REQ:
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;

	
		ex_rc_internal_jam_flag = feed_msg.arg1;			/* '19-09-19 */
		ex_rc_internal_jam_flag_bk = 0;
		feed_rc_set_sensor_bk(0);
		feed_rc_set_unit(feed_msg.arg1);
		set_recovery_count(feed_msg.arg1); //2025-04-18

		/* set recovery step */
		//set_recovery_step(RECOVERY_STEP_COLLECT_DRUM);	//UBA500はここにもあるが1箇所に統一できるかも

		_feed_set_seq(FEED_SEQ_RC_COLLECT, FEED_SEQ_RC_TIMEOUT);
		break;
	/* RETRY case  */
	case TMSG_FEED_RC_FORCE_STACK_REQ: //背面搬送で紙幣が検知できなくなるまで、取り込み方法に回す,イニシャル時と、Payout時に意味合いが異なる可能性あり フラッパ動作させてない場合は、おそらくドラムの方へ行く
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		feed_rc_set_unit(feed_msg.arg1);
		set_recovery_count(feed_msg.arg1); //2025-04-18

		_feed_set_seq(FEED_SEQ_RC_FORCE_STACK, FEED_SEQ_RC_TIMEOUT);
		break;
	case TMSG_FEED_RC_FORCE_PAYOUT_REQ: //payout以外でも使用している。Payoutというより、ドラムから背面搬送まで戻す処理、
		s_feed_alarm_code = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		/* select unit(drum or box) */
		feed_rc_set_unit(feed_msg.arg1);
		/* set collect sequence */

		set_recovery_count(feed_msg.arg1); //2025-04-18

		_feed_set_seq(FEED_SEQ_RC_FORCE_PAYOUT, FEED_SEQ_RC_TIMEOUT);
		break;
	case TMSG_FEED_RS_FORCE_PAYOUT_REQ:
		s_feed_alarm_code = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		/* select unit(drum or box) */
		feed_rc_set_unit(feed_msg.arg1);
		set_recovery_count(feed_msg.arg1); //2025-04-18
		_feed_rs_fpayout_2400_seq_start();
		_feed_set_seq(FEED_SEQ_RS_FORCE_PAYOUT, FEED_SEQ_RC_TIMEOUT);
		break;
	case TMSG_FEED_RC_BILLBACK_REQ:
		/* clear code */
		s_feed_alarm_code = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		/* select unit(drum or box) */
		feed_rc_set_unit(feed_msg.arg1);
		// /* RC保有枚数退避 */

		set_recovery_count(feed_msg.arg1); //2025-04-18

		/* set BillBack */
		_feed_set_seq(FEED_SEQ_RC_BILL_BACK, FEED_SEQ_RC_TIMEOUT);
		break;
	/*   */
#endif // UBA_RTQ
	default:					/* other */
		/* system error ? */
		_feed_system_error(0, 5);
		break;
	}
}


/*********************************************************************//**
 * @brief feed task busy procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _feed_busy_proc(void)
{
	FLGPTN flag = 0;
	ER ercd;

	ercd = twai_flg(ID_FEED_CTRL_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, TASK_WAIT_TIME);
	if (ercd != E_OK)
	{
		flag = 0;
	}

//2023-03-17	ex_validation.pulse_count = (u16)((ex_position_pulse_count.centering.end - ex_position_pulse_count.centering.start) * PITCH);

	switch (ex_feed_task_seq & 0xFF00)
	{
	case FEED_SEQ_IDLE:
		_feed_idle_seq_proc(flag);
		break;
	case FEED_SEQ_INITIAL:
		_feed_initial_seq_proc(flag);
		break;
	case FEED_SEQ_CENTERING:
		ex_validation.pulse_count = (u16)((ex_position_pulse_count.centering.end - ex_position_pulse_count.centering.start) * PITCH); //2023-03-17
		_feed_centering_seq_proc(flag);
		break;
	case FEED_SEQ_ESCROW:
		ex_validation.pulse_count = (u16)((ex_position_pulse_count.centering.end - ex_position_pulse_count.centering.start) * PITCH); //2023-03-17
		_feed_escrow_seq_proc(flag);
		break;
	case FEED_SEQ_APB:
		_feed_apb_seq_proc(flag); // only use SS not use RTQ
		break;
#if !defined(UBA_RTQ)//#if defined(HIGH_SECURITY_MODE)
	case FEED_SEQ_REV_CHECK_BILL:
		_feed_rev_seq_proc(flag);
		break;
#endif
	case FEED_SEQ_FORCE_STACK:
		_feed_force_stack_seq_proc(flag);
		break;
	case FEED_SEQ_REJECT:
		//2024-02-14
		_feed_reject_seq_proc2(flag);
		break;

	case FEED_SEQ_SEARCH:
#if defined(UBA_RTQ)
		_feed_rc_search_bill_seq_proc(flag);
#else
		_feed_search_seq_proc(flag);
#endif // UBA_RTQ	
		break;

#if defined(UBA_RTQ)
	case ENTRY_SEQ_BACK:
		_entry_back_seq_proc(flag);
		break;
#endif

	case FEED_SEQ_AGING:
		_feed_aging_seq_proc(flag);
		break;
	case FEED_SEQ_FREERUN:
		_feed_freerun_seq_proc(flag);
		break;

	case FEED_SEQ_FORCE_REV:	/* 0x0800 */
		_entry_feed_force_rev_seq_proc(flag);
		break;
#if defined(UBA_RTQ)
	case FEED_SEQ_RC_STACK:
		_feed_rc_stack_seq_proc(flag);	//ok
		break;
	case FEED_SEQ_RC_PAYOUT:
		_feed_rc_payout_seq_proc(flag); //ok
		break;
	case FEED_SEQ_RC_PAYOUT_STOP:
		 _feed_rc_payout_stop_seq_proc(flag); //ok
		 break;			
	case FEED_SEQ_RC_FORCE_STACK:
		_feed_rc_force_stack_proc(flag); //ok
		break;
	case FEED_SEQ_RC_COLLECT:
		_feed_rc_collect_seq_proc(flag); //ok
		break;
	case FEED_SEQ_RC_FORCE_PAYOUT:
		_feed_rc_fpayout_seq_proc(flag); //ok
		break;
	case FEED_SEQ_RC_BILL_BACK:
		_feed_rc_bill_back_seq_proc(flag); //ok
		break;
	//#if defined(UBA_RS)
	case FEED_SEQ_RS_PAYOUT:
		_feed_rs_payout_seq_proc(flag);
		break;
	case FEED_SEQ_RS_FORCE_PAYOUT:
		_feed_rs_fpayout_seq_proc(flag);
		break;
	//#endif
#endif // UBA_RTQ
	default:
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 6);
		break;
	}
}


/*********************************************************************//**
 * @brief MBX message procedure
 *  feed task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _feed_busy_msg_proc(void)
{
	switch (feed_msg.tmsg_code)
	{
	case TMSG_FEED_INITIAL_REQ:			/* INITIAL message */
		_feed_init_motor_speed();
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		_feed_set_waiting_seq(FEED_SEQ_INITIAL);
		break;

	case TMSG_FEED_CENTERING_REQ:		/* CENTERING message */
		s_feed_alarm_code = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		_feed_set_waiting_seq(FEED_SEQ_CENTERING);
		break;

	case TMSG_FEED_ESCROW_REQ:			/* ESCROW message */
		s_feed_alarm_code = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		_feed_set_waiting_seq(FEED_SEQ_ESCROW);
		break;

	case TMSG_FEED_APB_REQ:				/* APB message */
		s_feed_alarm_code = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		_feed_set_waiting_seq(FEED_SEQ_APB); // only use SS not use RTQ
		break;

	case TMSG_FEED_FORCE_STACK_REQ:		/* FORCE STACK message */
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		_feed_set_waiting_seq(FEED_SEQ_FORCE_STACK);
		break;
		
	case TMSG_FEED_REJECT_REQ:			/* REJECT message */
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = feed_msg.arg2; //リトライ
		s_feed_reject_code = 0;
		s_feed_reject_option = feed_msg.arg1;

		s_feed_pullback_off = feed_msg.arg3; //2023-11-01
		_feed_set_waiting_seq(FEED_SEQ_REJECT);
		break;

	case TMSG_FEED_SEARCH_REQ:			/* SEARCE message */
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		s_feed_search_option = feed_msg.arg1;  //use
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
#if defined(UBA_RTQ)
		feed_rc_set_search_dir(feed_msg.arg1);
#endif // UBA_RTQ
		_feed_set_waiting_seq(FEED_SEQ_SEARCH);
		break;

	#if defined(UBA_RTQ)
	case TMSG_ENTRY_BACK_REQ:
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		_feed_set_waiting_seq(ENTRY_SEQ_BACK);
		break;
	#endif

	case TMSG_FEED_AGING_REQ:			/* AGING message */
		s_feed_alarm_code = 0;
		s_feed_aging_no = feed_msg.arg1;
		s_feed_reject_code = 0;
		s_feed_pullback_off = 0;
		_feed_set_waiting_seq(FEED_SEQ_AGING);
		break;
	case TMSG_FEED_FREERUN_REQ:			/* FREERUN message */
		s_feed_alarm_code = 0;
		s_feed_reject_code = 0;
		s_feed_pullback_off = 0;
		if (feed_msg.arg1 == MOTOR_STOP)
		{
			_feed_set_waiting_seq(FEED_SEQ_FREERUN);
		}
		break;

	#if !defined(UBA_RTQ)//#if defined(HIGH_SECURITY_MODE)
	case TMSG_FEED_REV_CHECK_BILL_REQ:
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		s_feed_pullback_off = 0;
		_feed_set_waiting_seq(FEED_SEQ_REV_CHECK_BILL);
		break;
	#endif
	case TMSG_FEED_FORCE_REV_REQ: //2024-03-18a ID-003 Disable, Forced return of bills detected at entrance
		if(	1 == feed_msg.arg1 )
		{
			motor_ctrl_feed_stop();
			_feed_set_seq(0x0806, FEED_SEQ_TIMEOUT);
		}
		else
		{
			s_feed_alarm_code = 0;
			s_feed_alarm_retry = 0;
			s_feed_reject_code = 0;
			ex_reject_escrow = 0;
			_feed_set_waiting_seq(FEED_SEQ_FORCE_REV);
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_FEED_RC_STACK_REQ:
		/* clear code */
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;

		/* set stack sequence */
		_feed_set_waiting_seq(FEED_SEQ_RC_STACK);
		break;
	case TMSG_FEED_RC_PAYOUT_REQ:
		/* clear code */
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;
		feed_rc_set_payout_option(feed_msg.arg2); // set option

		if(is_rc_rs_unit())
		{
			feed_rc_set_payout_last(feed_msg.arg3);

			if (ex_main_test_no == TEST_RC_AGING || 
				ex_main_test_no == TEST_RC_AGING_FACTORY)
			{
				_feed_set_waiting_seq(FEED_SEQ_RC_PAYOUT);
			}
			else
			{
				_feed_set_waiting_seq(FEED_SEQ_RS_PAYOUT);
			}
		}
		else
		{
			_feed_set_waiting_seq(FEED_SEQ_RC_PAYOUT);
		}
		break;

	case	TMSG_FEED_RC_PAYOUT_STOP_REQ:	/* '21-10-11 */
			/* clear code */
			s_feed_alarm_code = 0;
			s_feed_alarm_retry = 0;
			s_feed_reject_code = 0;
			/* set payout sequence */
			_feed_set_waiting_seq(FEED_SEQ_RC_PAYOUT_STOP);
			break;

	case TMSG_FEED_RC_COLLECT_REQ:
		/* clear code */
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;

		_feed_set_waiting_seq(FEED_SEQ_RC_COLLECT);
		break;
	/* RETRY CASE */
	case TMSG_FEED_RC_FORCE_STACK_REQ:
		/* clear code */
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;

		_feed_set_waiting_seq(FEED_SEQ_RC_FORCE_STACK);
		break;
	case TMSG_FEED_RC_FORCE_PAYOUT_REQ:
		/* clear code */
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;

		_feed_set_waiting_seq(FEED_SEQ_RC_FORCE_PAYOUT);
		break;
	case TMSG_FEED_RC_BILLBACK_REQ:
		/* clear code */
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;

		_feed_set_waiting_seq(FEED_SEQ_RC_BILL_BACK);
		break;
	case TMSG_FEED_RS_FORCE_PAYOUT_REQ:
		/* clear code */
		s_feed_alarm_code = 0;
		s_feed_alarm_retry = 0;
		s_feed_reject_code = 0;
		ex_reject_escrow = 0;

		_feed_set_waiting_seq(FEED_SEQ_RS_FORCE_PAYOUT);
		break;
#endif // UBA_RTQ
	default:					/* other */
		/* system error ? */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		_feed_system_error(0, 7);
		break;
	}
}


/*********************************************************************//**
 * idle
 **********************************************************************/
/*********************************************************************//**
 * @brief feed control interrupt procedure (idle sequence)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_idle_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x01:
		_feed_idle_0001_seq(flag);
		break;
	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 8);
		break;
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0001
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_idle_0001_seq(u32 flag)
{
	if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if (is_motor_ctrl_feed_stop())
	{
		if (s_feed_task_wait_seq == FEED_SEQ_IDLE)
		{
			_feed_set_seq(s_feed_task_wait_seq, 0);
		}
		else if (s_feed_task_wait_seq == FEED_SEQ_INITIAL)
		{
			_initialize_position_pulse();
			_initialize_feed_motor_pulse();
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
		else if (s_feed_task_wait_seq == FEED_SEQ_FREERUN)
		{
			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_FREERUN_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_feed_set_seq(FEED_SEQ_IDLE, 0);
		}
		else if (s_feed_task_wait_seq == FEED_SEQ_CENTERING)
		{
			_initialize_position_pulse();
			_initialize_feed_motor_pulse();			
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
		else if (s_feed_task_wait_seq == FEED_SEQ_ESCROW)
		{
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
		else if (s_feed_task_wait_seq == FEED_SEQ_REJECT)
		{
			_check_position_pulse();
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
		else if (s_feed_task_wait_seq == FEED_SEQ_FORCE_REV)
		{
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
#if defined(UBA_RTQ)
		else if (s_feed_task_wait_seq == FEED_SEQ_RC_STACK)
		{
			_initialize_position_pulse();
			_initialize_feed_motor_pulse();
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
		else if (s_feed_task_wait_seq == FEED_SEQ_RC_PAYOUT)
		{
			_initialize_position_pulse();
			_initialize_feed_motor_pulse();
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
		else if (s_feed_task_wait_seq == FEED_SEQ_RC_FORCE_STACK)
		{
			_initialize_position_pulse();
			_initialize_feed_motor_pulse();
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
		else if (s_feed_task_wait_seq == FEED_SEQ_RC_COLLECT)
		{
			_initialize_position_pulse();
			_initialize_feed_motor_pulse();
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
		else if (s_feed_task_wait_seq == FEED_SEQ_RC_FORCE_PAYOUT)
		{
			_initialize_position_pulse();
			_initialize_feed_motor_pulse();
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
		else if (s_feed_task_wait_seq == FEED_SEQ_RC_BILL_BACK)
		{
			_initialize_position_pulse();
			_initialize_feed_motor_pulse();
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
		else if (s_feed_task_wait_seq == FEED_SEQ_RS_FORCE_PAYOUT)
		{
			_initialize_position_pulse();
			_initialize_feed_motor_pulse();
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
#endif // UBA_RTQ
		else
		{
			_initialize_position_pulse();
			_initialize_feed_motor_pulse();
			_feed_set_seq(s_feed_task_wait_seq, FEED_SEQ_TIMEOUT);
		}
		s_feed_task_wait_seq = FEED_SEQ_IDLE;
	}
}


/*********************************************************************//**
 * initial movement
 **********************************************************************/
/*********************************************************************//**
 * @brief feed control interrupt procedure (initialize sequence)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_initial_seq_proc(u32 flag) //今はエージングでも使用している
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:									/* seq0100 */
		_feed_initial_0100_seq(flag);
		break;
	case 0x01:									/* seq0101 */
		_feed_initial_0101_seq(flag);
		break;
	case 0x02:									/* seq0102 */
		_feed_initial_0102_seq(flag);
		break;
	case 0x03:									/* seq0103 */
		_feed_initial_0103_seq(flag);
		break;
	case 0x04:									/* seq0104 */
		_feed_initial_0104_seq(flag);
		break;
	case 0x05:									/* seq0105 */
		_feed_initial_0105_seq(flag);
		break;
	case 0x06:									/* seq0106 */
		_feed_initial_0106_seq(flag);
		break;
	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 9);
		break;
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0100
 *  wait motor start
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_initial_0100_seq(u32 flag)
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ((SENSOR_ENTRANCE) || (SENSOR_CENTERING) || (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		/* retry */
		s_feed_alarm_retry++;
		if (s_feed_alarm_retry >= FEED_STACK_RETRY_COUNT)
		{
			_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
		}
		else
		{
			s_feed_alarm_code = ALARM_CODE_FEED_MOTOR_LOCK;
			motor_ctrl_feed_stop();
			_feed_set_seq(FEED_SEQ_INITIAL, FEED_SEQ_TIMEOUT);
		}
	}
#if defined(UBA_RTQ)
	else if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_NEXT_PULSE(feed_initial_table)))
#else
	else if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed, FEED_NEXT_PULSE(feed_initial_table)))
#endif
	{
		//ivizionは読んでいるが意味がない start_ad();	//2024-03-04
	#if 1	//2023-04-03 //2025-09-26
		_feed_set_seq(FEED_NEXT_SEQ(feed_initial_table), 200);	/* 200msec後に計測 */
	#else
		_feed_set_seq(FEED_NEXT_SEQ(feed_initial_table), FEED_SPEED_CHECK_1ST_TIME);
	#endif
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x0101
 *  revers motor wait stability
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_initial_0101_seq(u32 flag)
{

	u8 cnt=0;	/*2023-04-03 */
	u8 ad[16];
	u16 ave=0;

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ((SENSOR_ENTRANCE) || (SENSOR_CENTERING) || (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		do{
			/* 16回 5msec, 40回 12msec */
			ad[cnt] = unit_voltage_check_uba();	/* 電圧計測 *//* 2023-04-03 */
			ave += ad[cnt];
			cnt++;
		}while(cnt<16);

		/* ave16 */
		ave = ave / 16;
		if(ave < 79) //9.6V未満から = 79
		{
			/* Low mode */
			ex_is_uba_mode = 1;
		}
		else
		{
			/* High mode*/
			ex_is_uba_mode = 0;
		}
	#if defined(UBA_RTQ)
		ex_is_uba_mode = 0;
	#endif
		s_tmp_cis_encoder_pulse = _pl_cis_encoder_count();

		_ir_feed_motor_ctrl.speed_check_pulse = 0;	//2025-09-24
		_ir_feed_motor_ctrl.speed_check_time = FEED_SPEED_CHECK_TIME;	/* 100ms	*///2025-09-24
		dly_tsk(50);

		/* 100ms経過 (1回目は400msec経過) */
		motor_ctrl_feed_set_pulse(FEED_NEXT_PULSE(feed_initial_table));
		_feed_set_seq(FEED_NEXT_SEQ(feed_initial_table), FEED_SPEED_CHECK_TIME);
	}
}


/*********************************************************************//**
 * @brief accept control sequence 0x0102
 *  revers motor speed check
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_initial_0102_seq(u32 flag)
{
	u16 m_speed;

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ((SENSOR_ENTRANCE) || (SENSOR_CENTERING) || (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* 100ms経過 */
		m_speed = _ir_feed_motor_ctrl.speed_check_pulse / (FEED_SPEED_CHECK_TIME / 100);

		if (m_speed < (FEED_SPEED_ERR_LOWER_LIMIT))
		{
			_feed_set_alarm(ALARM_CODE_FEED_MOTOR_SPEED_LOW);
		}
		else if (m_speed > (FEED_SPEED_ERR_UPPER_LIMIT))
		{
			_feed_set_alarm(ALARM_CODE_FEED_MOTOR_SPEED_HIGH);
		}		
		else if (s_tmp_cis_encoder_pulse == _pl_cis_encoder_count())
		{
			_feed_set_alarm(ALARM_CODE_CIS_ENCODER);
		}
		else
		{
	#if defined(UBA_RTQ) //2024-10-23 UBA_MUST
			uba_feed_speed_rev = (m_speed * PITCH) * 10; //550付近が理想 RTQへ通知する
	#endif // UBA_RTQ
			s_feed_alarm_retry = 0;
			motor_ctrl_feed_stop();
			//ivizionは読んでいるが意味がない stop_ad(); //2024-03-04
			_feed_set_seq(FEED_NEXT_SEQ(feed_initial_table), FEED_SEQ_TIMEOUT);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0103
 *  stop motor wait
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_initial_0103_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ((SENSOR_ENTRANCE) || (SENSOR_CENTERING) || (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		/* retry */
		s_feed_alarm_retry++;
		if (s_feed_alarm_retry >= FEED_STACK_RETRY_COUNT)
		{
			_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
		}
		else
		{
			s_feed_alarm_code = ALARM_CODE_FEED_MOTOR_LOCK;
			motor_ctrl_feed_stop();
			//ivizionは読んでいるが意味がない stop_ad(); //2024-03-04
			_feed_set_seq(ex_feed_task_seq, FEED_SEQ_TIMEOUT);
		}
	}
#if defined(UBA_RTQ)
	else if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, FEED_NEXT_PULSE(feed_initial_table)))
#else
	else if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed, FEED_NEXT_PULSE(feed_initial_table)))
#endif
	{
		_feed_set_seq(FEED_NEXT_SEQ(feed_initial_table), FEED_SPEED_CHECK_1ST_TIME);
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x0104
 *  forward motor wait stability
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_initial_0104_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ((SENSOR_ENTRANCE) || (SENSOR_CENTERING) || (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		s_tmp_cis_encoder_pulse = _pl_cis_encoder_count();		

		_ir_feed_motor_ctrl.speed_check_pulse = 0;		//2025-09-24
		_ir_feed_motor_ctrl.speed_check_time = FEED_SPEED_CHECK_TIME;	/* 100ms	*/
		dly_tsk(50);

		/* 100ms経過 (1回目は400msec経過) */
		motor_ctrl_feed_set_pulse(FEED_NEXT_PULSE(feed_initial_table));
		_feed_set_seq(FEED_NEXT_SEQ(feed_initial_table), FEED_SPEED_CHECK_TIME);
	}
}


/*********************************************************************//**
 * @brief accept control sequence 0x0105
 *  forward motor speed check
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_initial_0105_seq(u32 flag)
{
	u16 m_speed;

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ((SENSOR_ENTRANCE) || (SENSOR_CENTERING) || (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		/* 100ms経過 */
		m_speed = _ir_feed_motor_ctrl.speed_check_pulse / (FEED_SPEED_CHECK_TIME / 100);
		if (m_speed < (FEED_SPEED_ERR_LOWER_LIMIT))
		{
			_feed_set_alarm(ALARM_CODE_FEED_MOTOR_SPEED_LOW);
		}
		else if (m_speed > (FEED_SPEED_ERR_UPPER_LIMIT))
		{
			_feed_set_alarm(ALARM_CODE_FEED_MOTOR_SPEED_HIGH);
		}
		else if (s_tmp_cis_encoder_pulse == _pl_cis_encoder_count())
		{
			_feed_set_alarm(ALARM_CODE_CIS_ENCODER);
		}
		else
		{
		#if defined(UBA_RTQ) //2024-10-23 UBA_MUST
			uba_feed_speed_fwd = (m_speed * PITCH) * 10; //550付近が理想 RTQへ通知する
		#endif // UBA_RTQ
			s_feed_alarm_retry = 0;
			motor_ctrl_feed_stop();
			//ivizionは読んでいるが意味がない stop_ad(); //2024-03-04
			_feed_set_seq(FEED_NEXT_SEQ(feed_initial_table), FEED_SEQ_TIMEOUT);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0106
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_initial_0106_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ((SENSOR_ENTRANCE) || (SENSOR_CENTERING) || (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		/* retry */
		s_feed_alarm_retry++;
		if (s_feed_alarm_retry >= FEED_STACK_RETRY_COUNT)
		{
			_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
		}
		else
		{
			s_feed_alarm_code = ALARM_CODE_FEED_MOTOR_LOCK;
			motor_ctrl_feed_stop();
			//ivizionは読んでいるが意味がない stop_ad(); //2024-03-04
			_feed_set_seq(ex_feed_task_seq, FEED_SEQ_TIMEOUT);
		}
	}
	else if (is_motor_ctrl_feed_stop())
	{
	/* no error <正常終了> */
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_INITIAL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}


/*********************************************************************//**
 * feed to centering position
 **********************************************************************/
/*********************************************************************//**
 * @brief feed control interrupt procedure (centering sequence)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_centering_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:									/* seq0200 */
		_feed_centering_0200_seq(flag);
		break;

	case 0x10:									/* seq0201 */
		_feed_centering_0210_seq(flag);			// 幅よせセンサON待ち	//itou seq6
		break;
	case 0x20:									/* seq0202 */
		_feed_centering_0220_seq(flag);
		break;
	case 0x30:									/* seq0202 */
		_feed_centering_0230_seq(flag);
		break;
	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 10);
		break;
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0200
 *  wait motor start
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_centering_0200_seq_start(void)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
#if 0 /* 2021-10-19 */
	else if (!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
#endif
	else if (SENSOR_EXIT)
	{
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_AT);
	}
#if 0
	/* 挿入キャンセルは必ず残留検知より後に処理する */
	else if (!(SENSOR_ENTRANCE))
	{
		_feed_set_reject(REJECT_CODE_INSERT_CANCEL);
	}
#endif

#if defined(UBA_RTQ) //2025-04-24
	else if( ex_main_emergency_flag == 1 )
	{
	//Emergency Stopからの取り込み返却対策(そこそこの回数問題発生)
	//Emergency Stopでの取り込みの場合、ハンギング位置がすでにCISセンサに近い
	//識別サンプリング開始前に紙幣がかかり、返却する(データの札なし部分を見つけれないので、札幅エラーなど)
	//問題の理由
	//UBA500と比べハンギング位置が10mm程度 識別センサに近い
	//UBA500は入口と搬送でローラ駆動が別
	//識別センサに紙幣が若干かかる程度では識別、搬送エラーにならない
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_CENTERING_RSP, TMSG_SUB_START, 0, 0, 0);	//モータ起動を通知
		dly_tsk(200);
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_CENTERING_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);		
	}
#endif

	// 入口モータ -> 搬送モータ
//	else if (IERR_CODE_OK == motor_ctrl_entry_feed_fwd(ex_entry_motor_speed[ENTRY_SPEED_CENTERING].set_speed, 0,
//															ex_feed_motor_speed[FEED_SPEED_CENTERING].set_speed, 0, 0))
//	else if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_CENTERING].set_speed, FEED_NEXT_PULSE(feed_cnetering_table)))
	else if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_CENTERING].set_speed, 0))
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_CENTERING_RSP, TMSG_SUB_START, 0, 0, 0);	//モータ起動を通知
		_feed_set_seq(0x0210, FEED_SEQ_TIMEOUT);
	}
}


void _feed_centering_0200_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
#if 0 /* 2021-10-19 */
	else if (!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
#endif
	else if (SENSOR_EXIT)
	{
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_AT);
	}
#if 0
	/* 挿入キャンセルは必ず残留検知より後に処理する */
	else if (!(SENSOR_ENTRANCE))
	{
		_feed_set_reject(REJECT_CODE_INSERT_CANCEL);
	}
#endif
	else if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_CENTERING].set_speed, 0))
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_CENTERING_RSP, TMSG_SUB_START, 0, 0, 0);	//モータ起動を通知
		_feed_set_seq(0x0210, FEED_SEQ_TIMEOUT);
	}
}



/*********************************************************************//**
 * @brief feed control sequence 0x0201
 *  feed to CENTERING sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
// 幅よせセンサ紙幣待ち
 **********************************************************************/
void _feed_centering_0210_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	#if 0 /* 2021-10-19 */
	else if (!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	#endif
	else if (SENSOR_EXIT)
	{
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_AT);
	}
	/* 挿入キャンセルは必ず残留検知より後に処理する */
#if 0 // FOR_POLI ポリマ紙幣の為、ここでのチェックは辞める
	else if (!(SENSOR_ENTRANCE))
	{
		_feed_set_reject(REJECT_CODE_INSERT_CANCEL);
	}
#endif

	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_reject(REJECT_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (SENSOR_CENTERING)
	{
		#if 0 /* 2022-01-12 */
		motor_ctrl_feed_stop();
		_feed_set_seq(0x0230, FEED_SEQ_TIMEOUT);
		#else
		motor_ctrl_feed_set_pulse(STOP_WID);	// 厳しく/ 幅よせセンサがONしてから停止命令までの搬送距離 (理論値23mm) // itou seq6 メカ要望のトリガ
		_feed_set_seq(0x0220, FEED_SEQ_TIMEOUT);
		#endif
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0201
 *  feed to CENTERING sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
// 指定パルス搬送完了待ち
 **********************************************************************/
void _feed_centering_0220_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	#if 0 /* 2021-10-19 */
	else if (!(SENSOR_VALIDATION_OFF))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	#endif
	else if (SENSOR_EXIT)
	{
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_AT);
	}
#if 0 // FOR_POLI ポリマ紙幣の為、ここでのチェックは辞める
	/* 挿入キャンセルは必ず残留検知より後に処理する */
	else if (!(SENSOR_ENTRANCE))
	{
		_feed_set_reject(REJECT_CODE_INSERT_CANCEL);
	}
#endif
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_reject(REJECT_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
//	else if (IS_ENTRY_EVT_OVER_PULSE(flag))
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
	/* 指定パルス搬送完了 */
	// モータ停止

		motor_ctrl_feed_stop();

//		_feed_set_seq(FEED_NEXT_SEQ(feed_cnetering_table), FEED_SEQ_TIMEOUT);
		_feed_set_seq(0x0230, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0230
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
 //モータ停止待ち
 **********************************************************************/
void _feed_centering_0230_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	#if 0 /* 2021-10-19 */
	else if (!(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	#endif
	else if (SENSOR_EXIT)
	{
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_AT);
	}


	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (is_motor_ctrl_feed_stop())
	{
	/* no error <正常終了> */
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_CENTERING_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

/*********************************************************************//**
 * @brief feed control sub function
 *  detect banknote long
 * @param 		None
 * @return 		CLEANING CARD: true
 *				OTHER		: false
 **********************************************************************/
bool _is_banknote_too_long(void)//0x03XX用
{
	if((ex_position_pulse_count.centering.start == 0)
	 || (ex_position_pulse_count.centering.end == 0))
	{
		return false;
	}
	if((ex_position_pulse_count.centering.end - ex_position_pulse_count.centering.start) > CONV_PULSE(LENGTH_UPPER_LIMIT))
	{
		return true;
	}
	return false;
}

/*********************************************************************//**
 * @brief feed control sub function
 *  detect short note
 * @param 		None
 * @return 		CLEANING CARD: true
 *				OTHER		: false
 **********************************************************************/
bool _is_short_note_window(void) //0x03XX用
{
	// ex_feed_pulse_count.fwdを使用すると、ex_position_pulse_count.centering.endとキャップが生じる +10mm
	// 理由
	// ex_feed_pulse_count.fwdはモータ動作時のパスルをカウントしている
	// ex_feed_pulse_count.totalは実際のパスルを監視している
	// ex_position_pulse_count.centering.end は ex_feed_pulse_count.totalを使用している
	// 幅よせ位置へ搬送完了のモータ停止後のイナーシャでex_feed_pulse_count.fwdがex_feed_pulse_count.totalより10mm短くなっている
	// 実際の位置で考えるとex_feed_pulse_count.totalが正しい
	// Escrow位置への開始前の段階で実際の位置より10mm短くなっているので札長監視にstart endはtotalを使用しているので問題ないが
	// fwdは正しい値ではない。
	// ivizionは幅よせ位置で停止しないので、fwdを使用して問題ないがUBA700はtotalを使用しないと10mmの誤差がでる。
	// 札長監視は問題ないが、クリアウインドを過ぎたらという条件が実際の設定より+10mmになって複雑になるのでtotalを使用する

	//feedの各モード開始時にクリアされる
	//ex_feed_pulse_count.total = 0; //短券の監視にも使用
	//ex_feed_pulse_count.fwd = 0;  //FWD動作時ｎエンコーダのカウント
	//ex_feed_pulse_count.rev = 0;
	//ex_validation.pulse_count = 0;
	//ex_position_pulse_count.centering.start   幅よせセンサONでその時の ex_feed_pulse_count.total が代入される
	//ex_position_pulse_count.centering.end     幅よせセンサONの間カウントし続ける

	#if 0 //2024-03-08 幅よせで監視するので、同じ処理を入口でもチェックする必要はない
	// 0.entrance
	if(ex_position_pulse_count.entrance.start)
	{
		if (ex_feed_pulse_count.total - ex_position_pulse_count.entrance.end > CONV_PULSE(CLEAR_WINDOW_SIZE))
		{
			//if (ex_position_pulse_count.entrance.end - ex_position_pulse_count.entrance.start + CONV_PULSE(DIST_ENT_TO_CEN + 10) < CONV_PULSE(LENGTH_LOWER_LIMIT))
			if (ex_position_pulse_count.entrance.end - ex_position_pulse_count.entrance.start  <  CONV_PULSE(LENGTH_LOWER_LIMIT) - CONV_PULSE(30))
			{
				return true;	/* Reject */
			}
		}
	}
	else
	{
		// REJECT_CODE_PAPER_SHORT
		return true;
	}
	#endif

	// 1.centering
	if(ex_position_pulse_count.centering.start)
	{

		if(ex_feed_pulse_count.total - ex_position_pulse_count.centering.end > CONV_PULSE(CLEAR_WINDOW_SIZE))
		/* 今現在までの搬送パルス-幅よせがOFFのなったタイミングのパルス > クリアウインド間はチェックの保留*/
		/* 要約すると*/
		/* 幅よせがOFFしてから何パルス経過したか */
		{
			//if(ex_position_pulse_count.centering.end - ex_position_pulse_count.centering.start + CONV_PULSE(10) < CONV_PULSE(LENGTH_LOWER_LIMIT) - CONV_PULSE(30))
			if(ex_position_pulse_count.centering.end - ex_position_pulse_count.centering.start < CONV_PULSE(LENGTH_LOWER_LIMIT) - CONV_PULSE(30))
			/* 幅よせがONしていた間のパルス数 */
			{			
				motor_ctrl_feed_stop();

				return true;	/* Reject */
			}
		}
	}
	
#if 0 //2024-03-08 Escrow位置が微妙なので、以降はチェックに含めない
	//以降は取り込みキャンセル用が気がするが、逆に必要ないような気がする
	// 2.apb_in
	if(ex_position_pulse_count.apb_in.start)
	{
		if(ex_feed_pulse_count.total - ex_position_pulse_count.apb_in.end > CONV_PULSE(CLEAR_WINDOW_SIZE))
		{
			return true;	/* Reject */
		}
	}
	// 3.apb_out
	if(ex_position_pulse_count.apb_out.start)
	{
		if(ex_feed_pulse_count.total - ex_position_pulse_count.apb_out.end > CONV_PULSE(CLEAR_WINDOW_SIZE))
		{
			return true;	/* Reject */
		}
	}
	// 4.exit
	if(ex_position_pulse_count.exit.start)
	{
		if(ex_feed_pulse_count.total - ex_position_pulse_count.exit.end > CONV_PULSE(CLEAR_WINDOW_SIZE))
		{
			return true;	/* Reject */
		}
	}
#endif
	return false;
}

/*********************************************************************//**
 * feed to escrow position
 **********************************************************************/
/*********************************************************************//**
 * @brief feed control interrupt procedure (escrow sequence)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_escrow_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:									/* seq0300 */
		_feed_escrow_0300_seq2(flag);
		break;
	case 0x03:									/* seq0303 */
		_feed_escrow_0303_seq2(flag);			/* 幅よせONの監視*/
		break;
	case 0x04:									/* seq0304 */
		_feed_escrow_0304_seq2(flag);			/* 入口ONの監視*/
		break;
	case 0x05:									/* seq0305 */
		_feed_escrow_0305_seq2(flag);			//幅よせOFF待ち
		break;
	case 0x06:									/* seq0306 */
		_feed_escrow_0306_seq2(flag);
		break;
	case 0x07:									/* seq0307 */
		_feed_escrow_0307_seq2(flag);
		break;

	case 0x87:									/* seq0387 */
		_feed_escrow_0387_seq2(flag);
		break;

	case 0x08:									/* seq0308 */
		_feed_escrow_0308_seq2(flag);
		break;
	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);
		/* system error ? */
		_feed_system_error(0, 11);
		break;
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0300
 *  wait motor start
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_escrow_0300_seq2_start(void)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	else if (SENSOR_EXIT)
	{
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_SK);
	}
	else if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, (u16)(FEED_SET_SLIP(DIST_CEN_TO_PBI)) ))
	{
		//2022-10-31a
		// UBAは幅よせ位置からの搬送なので少し違う
		// 2018-07-27 yuji
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_ESCROW_RSP, TMSG_SUB_INTERIM, 0, 0, 0);
		_feed_set_seq(0x0303, FEED_SEQ_ENTRANCE_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x0300
 *  wait motor start
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_escrow_0300_seq2(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ((SENSOR_APB_IN) || (SENSOR_APB_OUT))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	else if (SENSOR_EXIT)
	{
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_SK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, (u16)(FEED_SET_SLIP(DIST_CEN_TO_PBI))  ))
	{

		// UBAは幅よせ位置からの搬送なので少し違う
		// 2018-07-27 yuji
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_ESCROW_RSP, TMSG_SUB_INTERIM, 0, 0, 0);
		_feed_set_seq(0x0303, FEED_SEQ_ENTRANCE_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0303
 *  feed to APB-IN sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_escrow_0303_seq2(u32 flag)
{



	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (SENSOR_APB_OUT)
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	else if (SENSOR_EXIT)
	{
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_SK);
	}
	/* 挿入キャンセルは必ず残留検知より後に処理する */
	else if (_is_short_note_window())
	{
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
	else if(_is_banknote_too_long())
	{
		_feed_set_reject(REJECT_CODE_PAPER_LONG);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_reject(REJECT_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
	/* feed slip */
		_feed_set_reject(REJECT_CODE_FEED_SLIP);
	}
	else if (IS_FEED_EVT_CIS_SKEW(flag))
	{
	/* skew */
		_feed_set_reject(REJECT_CODE_SKEW);
	}
	else if (IS_FEED_EVT_CIS_MLT(flag))
	{
	/* multi paper */
		_feed_set_reject(REJECT_CODE_PHOTO_LEVEL);
	}
	else if (IS_FEED_CIS_SHORT_EDGE(flag))
	{
	/* short edge */
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
	else if (SENSOR_APB_IN)
	{
	/*  */
		motor_ctrl_feed_set_pulse((u16)(FEED_SET_SLIP(DIST_PBI_TO_PBO)));		
		_feed_set_seq(0x0304, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0304
 *  feed to APB-OUT sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_escrow_0304_seq2(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (SENSOR_EXIT)
	{
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_SK);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_reject(REJECT_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
	/* feed slip */
		_feed_set_reject(REJECT_CODE_FEED_SLIP);
	}
	/* 挿入キャンセルは必ず残留検知より後に処理する */
	else if (_is_short_note_window())
	{
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
	else if(_is_banknote_too_long())
	{
		_feed_set_reject(REJECT_CODE_PAPER_LONG);
	}
	else if (IS_FEED_EVT_CIS_SKEW(flag))
	{
	/* skew */
		_feed_set_reject(REJECT_CODE_SKEW);
	}
	else if (IS_FEED_EVT_CIS_MLT(flag))
	{
	/* multi paper */
		_feed_set_reject(REJECT_CODE_PHOTO_LEVEL);
	}
	else if (IS_FEED_CIS_SHORT_EDGE(flag))
	{
	/* short edge */
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
#if POINT_UV1_ENABLE || POINT_UV2_ENABLE
	else if(is_uv_led_check())
	{
		_feed_set_reject(REJECT_CODE_UV);
	}
#endif
	else if (!(SENSOR_ENTRANCE))
	{
		ex_feed_task_seq = 0x0306;
		motor_ctrl_feed_set_pulse((u16)(FEED_SET_SLIP(DIST_ENT_TO_CEN)));
		_feed_set_seq(0x0307, FEED_SEQ_TIMEOUT);
	}
	else if (SENSOR_APB_OUT)
	{
	/*  */
		if(ex_feed_pulse_count.total - ex_position_pulse_count.centering.end > CONV_PULSE(CLEAR_WINDOW_SIZE))
		{
			motor_ctrl_feed_set_pulse( (u16)(CONV_PULSE(DIST_CEN_TO_ESP)) ); //ここの設定怪しいかも 修正が逆に影響するかも
			_feed_set_seq(0x0308, FEED_SEQ_TIMEOUT);
		}
		else if(ex_feed_pulse_count.total - ex_position_pulse_count.entrance.end > CONV_PULSE(CLEAR_WINDOW_SIZE))
		{
			motor_ctrl_feed_set_pulse( (u16)(FEED_SET_SLIP(DIST_ENT_TO_CEN)) ); //ここの設定怪しいかも 修正が逆に影響するかも
			_feed_set_seq(0x0307, FEED_SEQ_TIMEOUT);
		}
		else
		{
			motor_ctrl_feed_set_pulse( (u16)(FEED_SET_SLIP(DIST_PBO_TO_EXI)) );
			_feed_set_seq(0x0305, FEED_SEQ_TIMEOUT);
		}

	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0305
 *  feed to EXIT sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_escrow_0305_seq2(u32 flag)
{
	s32 rest;
	/* 紙幣ロストは必ず残留検知より後に処理する */
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (_is_short_note_window())
	{
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_reject(REJECT_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
#if !defined(UBA_RTQ) //2024-09-10
	else if (!is_ld_mode() && (SENSOR_EXIT) && (!SENSOR_PUSHER_HOME))
	{
		_feed_set_reject(REJECT_CODE_STACKER_HOME);
	}
#endif
	else if (IS_FEED_EVT_CIS_SKEW(flag))
	{
	/* skew */
		_feed_set_reject(REJECT_CODE_SKEW);
	}
	else if (IS_FEED_EVT_CIS_MLT(flag))
	{
	/* multi paper */
		_feed_set_reject(REJECT_CODE_PHOTO_LEVEL);
	}
	else if (IS_FEED_CIS_SHORT_EDGE(flag))
	{
	/* short edge */
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
#if POINT_UV1_ENABLE || POINT_UV2_ENABLE
	else if(is_uv_led_check())
	{
		_feed_set_reject(REJECT_CODE_UV);
	}
#endif
	/* Fixed EXIT sensor JAM(LD-Mode) */
	else if ((SENSOR_EXIT) || (IS_FEED_EVT_OVER_PULSE(flag)))
	{
	/*  */
		if (!(SENSOR_CENTERING))
		{
			rest = CONV_PULSE(CLEAR_WINDOW_SIZE) - (ex_feed_pulse_count.total - ex_position_pulse_count.centering.end);
			/* check window in 0387 */
			if( rest > 0)
			{
				motor_ctrl_feed_set_pulse((u16)rest);
				_feed_set_seq(0x0387, FEED_SEQ_TIMEOUT);
			}
			else
			{
				ex_feed_task_seq = 0x0307;
				motor_ctrl_feed_set_pulse( (u16)(CONV_PULSE(DIST_CEN_TO_ESP)) - CONV_PULSE(CLEAR_WINDOW_SIZE) - rest);
				_feed_set_seq(0x0308, FEED_SEQ_TIMEOUT);
			}
		}
		else
		{
			motor_ctrl_feed_set_pulse( (u16)(FEED_SET_SLIP((LENGTH_UPPER_LIMIT-DIST_ENT_TO_EXI))) );
			_feed_set_seq(0x0306, FEED_SEQ_TIMEOUT);
		}
	}
	/* Fixed EXIT sensor JAM(LD-Mode) */
}


/*********************************************************************//**
 * @brief feed control sequence 0x0306
 *  feed to ENTRANCE sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_escrow_0306_seq2(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (_is_short_note_window())
	{
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_reject(REJECT_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
	/* feed slip */
		_feed_set_reject(REJECT_CODE_FEED_SLIP);
	}
	else if (IS_FEED_EVT_CIS_SKEW(flag))
	{
	/* skew */
		_feed_set_reject(REJECT_CODE_SKEW);
	}
	else if (IS_FEED_EVT_CIS_MLT(flag))
	{
	/* multi paper */
		_feed_set_reject(REJECT_CODE_PHOTO_LEVEL);
	}
	else if (IS_FEED_CIS_SHORT_EDGE(flag))
	{
	/* short edge */
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
#if POINT_UV1_ENABLE || POINT_UV2_ENABLE
	else if(is_uv_led_check())
	{
		_feed_set_reject(REJECT_CODE_UV);
	}
#endif
	else if ((((ex_validation.pulse_count + DIST_ENT_TO_CEN) > banknote_length_max) && !(is_all_accept_mode()) )
		  || ((ex_validation.pulse_count + DIST_ENT_TO_CEN) > (LENGTH_UPPER_LIMIT + 10)))
	{
	/* paper too long */
		_feed_set_reject(REJECT_CODE_PAPER_LONG);
	}
	else if(_is_banknote_too_long())
	{
		_feed_set_reject(REJECT_CODE_PAPER_LONG);
	}
#if !defined(UBA_RTQ) //2024-09-10
	else if (!is_ld_mode() && (SENSOR_EXIT) && (!SENSOR_PUSHER_HOME))
	{
		_feed_set_reject(REJECT_CODE_STACKER_HOME);
	}
#endif
	else if (!(SENSOR_ENTRANCE))
	{
	/*  */
		motor_ctrl_feed_set_pulse( (u16)(FEED_SET_SLIP(DIST_ENT_TO_CEN)) );
		_feed_set_seq(0x0307, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0307
 *  feed to CENTERING sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_escrow_0307_seq2(u32 flag)
{
	s32 rest;
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	/* 挿入キャンセルは必ず残留検知より後に処理する */
	else if (_is_short_note_window())
	{
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_reject(REJECT_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
	/* feed slip */
		_feed_set_reject(REJECT_CODE_FEED_SLIP);
	}
	else if (IS_FEED_EVT_CIS_SKEW(flag))
	{
	/* skew */
		_feed_set_reject(REJECT_CODE_SKEW);
	}
	else if (IS_FEED_EVT_CIS_MLT(flag))
	{
	/* multi paper */
		_feed_set_reject(REJECT_CODE_PHOTO_LEVEL);
	}
	else if (IS_FEED_CIS_SHORT_EDGE(flag))
	{
	/* short edge */
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
#if POINT_UV1_ENABLE || POINT_UV2_ENABLE
	else if(is_uv_led_check())
	{
		_feed_set_reject(REJECT_CODE_UV);
	}
#endif
	// POS1,POS2が近く、POS2がONした後のホールドが多発するため,POSの長さチェック使用不可
	else if ((((ex_validation.pulse_count) > banknote_length_max) && !(is_all_accept_mode()) )
		  || ((ex_validation.pulse_count) > (LENGTH_UPPER_LIMIT + 10)))
	{
	/* paper too long */
		_feed_set_reject(REJECT_CODE_PAPER_LONG);
	}

	else if(_is_banknote_too_long())
	{
		_feed_set_reject(REJECT_CODE_PAPER_LONG);
	}
#if !defined(UBA_RTQ) //2024-09-10
	else if (!is_ld_mode() && (SENSOR_EXIT) && (!SENSOR_PUSHER_HOME))
	{
		_feed_set_reject(REJECT_CODE_STACKER_HOME);
	}
#endif
	else if (!(SENSOR_CENTERING))
	{
	/*  */
		rest = CONV_PULSE(CLEAR_WINDOW_SIZE) - (ex_feed_pulse_count.total - ex_position_pulse_count.centering.end);
		/* check window in 0387 */
		if( rest > 0)
		{
			motor_ctrl_feed_set_pulse((u16)rest);
			_feed_set_seq(0x0387, FEED_SEQ_TIMEOUT);
		}
		else
		{
			ex_feed_task_seq = 0x0307;
			motor_ctrl_feed_set_pulse( (u16)(CONV_PULSE(DIST_CEN_TO_ESP)) - CONV_PULSE(CLEAR_WINDOW_SIZE) - rest);
			_feed_set_seq(0x0308, FEED_SEQ_TIMEOUT);
		}
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x0387
 *  feed to CENTERING sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_escrow_0387_seq2(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	/* 挿入キャンセルは必ず残留検知より後に処理する */
	else if (_is_short_note_window())
	{
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_reject(REJECT_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (IS_FEED_EVT_CIS_SKEW(flag))
	{
	/* skew */
		_feed_set_reject(REJECT_CODE_SKEW);
	}
	else if (IS_FEED_EVT_CIS_MLT(flag))
	{
	/* multi paper */
		_feed_set_reject(REJECT_CODE_PHOTO_LEVEL);
	}
	else if(_is_banknote_too_long())
	{
		_feed_set_reject(REJECT_CODE_PAPER_LONG);
	}
#if !defined(UBA_RTQ) //2024-09-10
	else if (!is_ld_mode() && (SENSOR_EXIT) && (!SENSOR_PUSHER_HOME))
	{
		_feed_set_reject(REJECT_CODE_STACKER_HOME);
	}
#endif
	else if (SENSOR_CENTERING)
	{
		/* back to 0307 */
		ex_feed_task_seq = 0x0306;
		motor_ctrl_feed_set_pulse((u16)(FEED_SET_SLIP(DIST_ENT_TO_CEN)));
		_feed_set_seq(0x0307, FEED_SEQ_TIMEOUT);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
		ex_feed_task_seq = 0x0307; //307のようで、308に移動する(308に移動する為に307を設定している)
		motor_ctrl_feed_set_pulse( (u16)(CONV_PULSE(DIST_CEN_TO_ESP)) - CONV_PULSE(CLEAR_WINDOW_SIZE));
		_feed_set_seq(0x0308, FEED_SEQ_TIMEOUT); //
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0308
 *  feed to escrow position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_escrow_0308_seq2(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
#if !defined(UBA_RTQ) //2024-09-10
	else if (!is_ld_mode() && (SENSOR_EXIT) && (!SENSOR_PUSHER_HOME))
	{
		_feed_set_reject(REJECT_CODE_STACKER_HOME);
	}
#endif
	else
	if (SENSOR_CENTERING)
	{
		motor_ctrl_feed_set_pulse((u16)(FEED_SET_SLIP(DIST_ENT_TO_CEN)));
		_feed_set_seq(0x0306, FEED_SEQ_TIMEOUT);
	}

	else if (_is_short_note_window())
	{
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
	else if(_is_banknote_too_long())
	{
		_feed_set_reject(REJECT_CODE_PAPER_LONG);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_reject(REJECT_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (IS_FEED_EVT_CIS_SKEW(flag))
	{
	/* skew */
		_feed_set_reject(REJECT_CODE_SKEW);
	}
	else if (IS_FEED_EVT_CIS_MLT(flag))
	{
	/* multi paper */
		_feed_set_reject(REJECT_CODE_PHOTO_LEVEL);
	}
	else if (IS_FEED_CIS_SHORT_EDGE(flag))
	{
	/* short edge */
		_feed_set_reject(REJECT_CODE_PAPER_SHORT);
	}
#if POINT_UV1_ENABLE || POINT_UV2_ENABLE
	else if(is_uv_led_check())
	{
		_feed_set_reject(REJECT_CODE_UV);
	}
#endif
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_feed_stop();
	/* no error <正常終了> */
		ex_reject_escrow = 1;
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_ESCROW_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

/*********************************************************************//**
 * feed to APB position
 **********************************************************************/
/*********************************************************************//**
 * @brief feed control interrupt procedure (APB sequence)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_apb_seq_proc(u32 flag) //UBA_PROを完全に移植 // only use SS not use RTQ
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:									/* seq0400 */
		_feed_apb_0400_seq(flag);				// escrow位置からの搬送
		break;
	case 0x10:									/* seq0401 */
		_feed_apb_0410_seq(flag);
		break;
	case 0x20:									/* seq0402 */
		_feed_apb_0420_seq(flag);
		break;
	case 0x30:									/* seq0403 */
		_feed_apb_0430_seq(flag);
		break;
	case 0x40:									/* seq0403 */
		_feed_apb_0440_seq(flag);				// 最終搬送
		break;
	case 0x50:									/* seq0410 */
		_feed_apb_0450_seq(flag);
		break;

	// ポーズ処理
	case 0x60:									/* seq0411 */
		_feed_apb_0460_seq(flag);
		break;
	case 0x70:									/* seq0412 */
		_feed_apb_0470_seq(flag);
		break;
	case 0x80:									/* seq0413 */
		_feed_apb_0480_seq(flag);
		break;
	case 0x90:									/* seq0480 */
		_feed_apb_0490_seq(flag);
		break;
	case 0xA0:									/* seq0413 */
		_feed_apb_04A0_seq(flag);
		break;
	case 0xB0:									/* seq0480 */
		_feed_apb_04B0_seq(flag);
		break;

	// リトライ処理のリバース
	case 0xC0:
		_feed_apb_04C0_seq(flag);
		break;
	case 0xE0:
		_feed_apb_04E0_seq(flag);
		break;
	case 0xF0:
		_feed_apb_04F0_seq(flag);
		break;

	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 12);
		break;
	}
}


void _feed_apb_0400_seq_start(void)// OK only SS
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	#if 0 /* 2021-10-19 */
	else if ( !(SENSOR_VALIDATION_OFF))
	{
	// cheat　ここでいきなりcheatにするかMAINタスクでCheatと判断するか考える

		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	#endif
	else if (SENSOR_CENTERING)
	{
		/* pause */
		_feed_set_apb_pause();
	}
//	エスクロ以降はモータ全開
	else if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, WAIT_PB_IN_OFF ))
	{
		_feed_set_seq( 0x0410, FEED_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x0400
 *  wait motor start
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_apb_0400_seq(u32 flag)// OK
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
	// cheat　ここでいきなりcheatにするかMAINタスクでCheatと判断するか考える
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (SENSOR_CENTERING)
	{
		/* pause */
		_feed_set_apb_pause();
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	// モータが起動しないエラーコード検討する
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
//	エスクロ以降はモータ全開
	else if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, WAIT_PB_IN_OFF ))
	{
		_feed_set_seq( 0x0410, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0401
 *  feed to APB-IN sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// PB IN　札抜け待ち
//and
//APB-OUT　or EXIT
//札検知待ち
//and
//指定パルス搬送待ち
 **********************************************************************/
void _feed_apb_0410_seq(u32 flag)// ok
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
	//	_feed_set_alarm(ALARM_CODE_CHEAT);
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (SENSOR_CENTERING)
	{
		/* pause */
		_feed_set_apb_pause();
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */

			_feed_set_feed_apb_retry(REJECT_CODE_FEED_TIMEOUT, ex_feed_task_seq);
		}
		else
		{
		/* feed slip */

			_feed_set_feed_apb_retry(REJECT_CODE_FEED_SLIP, ex_feed_task_seq);
		}
	}
	else if (!(SENSOR_APB_IN) && _ir_feed_motor_ctrl.over_pulse == 1
			&& ( (SENSOR_APB_OUT) || (SENSOR_EXIT) ))
	{
	/* next step */
		set_recovery_step(RECOVERY_STEP_APB_OUT);
		motor_ctrl_feed_set_pulse(WAIT_PB_OUT_OFF);
		_feed_set_seq( 0x0420, FEED_SEQ_TIMEOUT);
	}

}


/*********************************************************************//**
 * @brief feed control sequence 0x0402
 *  feed to APB-OUT sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
//PB OUT　紙抜け待ち
//and
//指定パルス搬送待ち

 **********************************************************************/
void _feed_apb_0420_seq(u32 flag)// ok
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
	//	_feed_set_alarm(ALARM_CODE_CHEAT);
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}

	else if ( (SENSOR_CENTERING) && (SENSOR_EXIT) )
	{
		/* pause */
		_feed_set_apb_pause();
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_feed_apb_retry(REJECT_CODE_FEED_TIMEOUT,	ex_feed_task_seq);
		}
		else
		{
		/* feed slip */

			_feed_set_feed_apb_retry(REJECT_CODE_FEED_SLIP,	ex_feed_task_seq);
		}
	}
	else if (!(SENSOR_APB_OUT) && _ir_feed_motor_ctrl.over_pulse == 1 )
	{
	// next step	// 2018-04-11 どちらが先が検討の余地あり、どちらでも結果は同じ様な気がする

		_feed_set_seq( 0x0430, FEED_SEQ_TIMEOUT );		// 全イベントクリア
		motor_ctrl_feed_set_pulse( WAIT_EXIT_OFF );		// オーバパルスのイベントのみクリア
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0402
 *  feed to APB-OUT sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
//Exit紙幣抜け待ち
//and
//指定パスル搬送待ち
 **********************************************************************/
void _feed_apb_0430_seq(u32 flag)// ok
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
	//	_feed_set_alarm(ALARM_CODE_CHEAT);
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}

	else if(SENSOR_APB_IN)
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if ( (SENSOR_CENTERING) && (SENSOR_EXIT) )
	{
		/* pause */
		_feed_set_apb_pause();
	}
	else if (SENSOR_APB_OUT)
	{
	// 紙幣をPB OUTで見つけたので
//		motor_ctrl_feed_set_pulse(feed_apb_table[(0x0420 & 0x000F)].pulse);
		motor_ctrl_feed_set_pulse(WAIT_PB_OUT_OFF);
		_feed_set_seq(0x0420, FEED_SEQ_TIMEOUT);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_feed_apb_retry(REJECT_CODE_FEED_TIMEOUT,	ex_feed_task_seq);
		}
		else
		{
		/* feed slip */
			_feed_set_feed_apb_retry(REJECT_CODE_FEED_SLIP,	ex_feed_task_seq);
		}
	}
	else if (!(SENSOR_EXIT) && _ir_feed_motor_ctrl.over_pulse == 1)
	{
	// next step
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_APB_RSP, TMSG_SUB_EXIT_OUT, 0, 0, 0);		// testmessage 最終的には削除する可能性が高い

		motor_ctrl_feed_set_pulse(WAIT_LAST_FEED);
		_feed_set_seq( 0x0440, FEED_SEQ_TIMEOUT);

	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0402
 *  feed to APB-OUT sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
//押し込み開始位置までの搬送中
//紙幣はEXITを通過済み
//指定パスル搬送待ち
// この時点で返却は基本的にむり エラーとする
 **********************************************************************/
void _feed_apb_0440_seq(u32 flag)
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}

	else if(SENSOR_APB_OUT || SENSOR_APB_IN)
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if (SENSOR_EXIT)
	{
	// 通常はONにならないはずだが、ONとなった為、再度搬送距離を伸ばす
	// タイマを更新すると、タイムアウトが機能しないので削除
	// UBA10 iPROはモータ動作を停止後、1s間 EXITがONの場合、エラ、ONが解除された場合、正転再開
	/* Stacker JAM */
//		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_SK);
		motor_ctrl_feed_set_pulse(160);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	// EXITを完全に通過後のJAMなので、エラーとする
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
		// モータ停止命令
		motor_ctrl_feed_stop();
		_feed_set_seq( 0x0450, FEED_SEQ_TIMEOUT);
	}
}



/*********************************************************************//**
 * @brief feed control sequence 0x0402
 *  feed to APB-OUT sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
//押し込み開始位置までの搬送完了
//モータ停止待ち
// この時点で返却は基本的にむり
 **********************************************************************/
void _feed_apb_0450_seq(u32 flag)
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if(SENSOR_APB_OUT || SENSOR_APB_IN)
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (SENSOR_EXIT)
	{
		/* Stacker JAM */
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_SK);
	}

	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	// EXITを完全に通過後のJAMなので、エラーとする
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if (is_motor_ctrl_feed_stop())
	{

#if defined(JCME_DATA)
		result_ad_tape();
#endif
	/* no error <正常終了> */
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_APB_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}




/*********************************************************************//**
 * @brief feed control sub function
 *  set pause (APB)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _feed_set_apb_pause(void)// ok
{
//keep_war	s_feed_task_pause_seq = ex_feed_task_seq;
	motor_ctrl_feed_stop();

	_feed_set_seq(0x0460, FEED_SEQ_TIMEOUT);
}


/*********************************************************************//**
 * @brief feed control sequence 0x0410
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
// ポーズ検知後、モータ停止待ち
 **********************************************************************/
 void _feed_apb_0460_seq(u32 flag)//ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (is_motor_ctrl_feed_stop())
	{
	/* no error */
		s_feed_sensor_backup = (ex_position_sensor & (POSI_CENTERING|POSI_APB_IN|POSI_APB_OUT|POSI_EXIT));
	#if 0 /* 2021-10-19 */
		if (!(SENSOR_VALIDATION_OFF))
		{
			s_feed_sensor_backup |= POSI_VALIDATION;
		}
	#endif
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_APB_RSP, TMSG_SUB_PAUSE, 0, 0, 0);
		_feed_set_seq(0x0470, FEED_APB_PAUSE_TIMEOUT);	// いろいろなシーケンスからこのシケンスに飛んでくるので、ここでは次のシーケンス番号は直接設定する
	}
	else
	{
		motor_ctrl_feed_stop();
	}
}


/*********************************************************************//**
 * @brief accept control sequence 0x0411
 *  check sensor
 * @param[in]	feed motor event flag
 * @return 		None
 // // Pausing
 //
 **********************************************************************/
void _feed_apb_0470_seq(u32 flag)// Pausing
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (!(SENSOR_ENTRANCE) && !(SENSOR_CENTERING))
	{
	//ポーズが解除された
		_feed_set_seq(0x0480, FEED_PAUSE_CHECK_TIME);	// このタイムアウト時間は、本当にポーズか解除されているかの待ち時間
	}
}


/*********************************************************************//**
 * @brief accept control sequence 0x0412
 *  check sensor (500msec)
 * @param[in]	feed motor event flag
 * @return 		None
// ポーズ解除解除後のポーズ解除で正しいかの待ち処理
//
 ********************************************************************/
void _feed_apb_0480_seq(u32 flag)// Pausing
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if (SENSOR_CENTERING)
	{
	// ポーズは解除されていなかった
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_APB_RSP, TMSG_SUB_PAUSE, 0, 0, 0);
		_feed_set_seq(0x0470, FEED_APB_PAUSE_TIMEOUT);	//シーケンスを戻すので直接設定
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	// ポーズ解除確定
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_APB_RSP, TMSG_SUB_RESUME, 0, 0, 0);
		_feed_set_seq(0x0490, 0);
		// 再開処理は次のシケンスでセンサ状態を確認して行う。
		// 本来ここで、シーケンスを確定してもいい様な。
	}
}


/*********************************************************************//**
 * @brief accept control sequence 0x0413
 *  wait motor start
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_apb_0490_seq(u32 flag)// Pausing
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if (SENSOR_CENTERING)
	{
	// ポーズ再開
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_APB_RSP, TMSG_SUB_PAUSE, 0, 0, 0);
		_feed_set_seq(0x0470, FEED_APB_PAUSE_TIMEOUT);
	}
	// ポーズ解除搬送再開
	else if (SENSOR_APB_IN)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, WAIT_PB_IN_OFF))
		{
			_feed_set_seq(0x0410, FEED_SEQ_TIMEOUT);
		}
	}
	else if (SENSOR_APB_OUT)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, WAIT_PB_OUT_OFF))
		{
			motor_ctrl_feed_set_pulse(WAIT_PB_OUT_OFF);
			_feed_set_seq(0x0420, FEED_SEQ_TIMEOUT);
			set_recovery_step(RECOVERY_STEP_APB_OUT);
		}
	}
	else
	{
		// 搬送パルス数も設定
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, WAIT_PB_OUT_OFF))	//2019-09-25
		{
			_feed_set_seq( 0x0430, FEED_SEQ_TIMEOUT );		// 全イベントクリア
			motor_ctrl_feed_set_pulse( WAIT_EXIT_OFF );		// オーバパルスのイベントのみクリア
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0402
 *  feed to APB-OUT sensor OFF position + Clear Window Size
 * @param[in]	feed motor event flag
 * @return 		None
 //ポーズ再開後がPB OUTがOFFの状態での最終搬送完了待ち
 **********************************************************************/
void _feed_apb_04A0_seq(u32 flag)// ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}

	else if ((SENSOR_CENTERING))
	{
	/* centering sensor or validation sensor ON */
	// ポーズ再開
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_APB_RSP, TMSG_SUB_PAUSE, 0, 0, 0);
		_feed_set_seq(0x0470, FEED_APB_PAUSE_TIMEOUT);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_reject(REJECT_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
	}
	else if (SENSOR_APB_OUT)
	{
	// 紙幣をPB OUTで見つけたので
//		motor_ctrl_feed_set_pulse(feed_apb_table[(0x0420 & 0x000F)].pulse);
		motor_ctrl_feed_set_pulse(WAIT_PB_OUT_OFF);
		_feed_set_seq(0x0420, FEED_SEQ_TIMEOUT);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
	/*  */
		motor_ctrl_feed_stop();
		_feed_set_seq(0x04B0, FEED_SEQ_TIMEOUT);
		set_recovery_step(RECOVERY_STEP_APB_OUT);
	}
}

/*********************************************************************//**
 * @brief feed control sequence
 *  feed to APB-OUT sensor OFF position
 * @return 		None
//ポーズ再開後がPB OUTがOFFの状態での最終搬送完了後のモータ停止まち
//
 **********************************************************************/
void _feed_apb_04B0_seq(u32 flag)// ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (SENSOR_EXIT)
	{
		/* Stacker JAM */
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_SK);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_reject(REJECT_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_reject(REJECT_CODE_FEED_TIMEOUT);
		}
		else
		{
		/* feed slip */
			_feed_set_reject(REJECT_CODE_FEED_SLIP);
		}
	}

	else if (is_motor_ctrl_feed_stop())
	{
	/* no error <正常終了> */
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_APB_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}


/*********************************************************************//**
 * @brief feed APB control sub function
 *  set stack retry
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _feed_set_feed_apb_retry(u32 reject_code, u16 error_sequence)// 0x04XX
{
	s_feed_alarm_retry++;
	backup_feed_apb_sequence = error_sequence;

	if( s_feed_alarm_retry >= FEED_ACCEPT_RETRY_COUNT)
	{
	/* retry over */
		_feed_set_reject(reject_code);
	}
	else
	{
		s_feed_alarm_code = reject_code;
		motor_ctrl_feed_stop();
		_feed_set_seq(0x04C0, FEED_SEQ_TIMEOUT);
	}
}



/*********************************************************************//**
 * @brief feed control sequence
 *  feed to APB-OUT sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
//リトライ処理搬送モータ停止まち
//
 **********************************************************************/
void _feed_apb_04C0_seq(u32 flag)// OK
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_apb_retry(REJECT_CODE_FEED_TIMEOUT, ex_feed_task_seq);
	}
	else if (is_motor_ctrl_feed_stop())
	{
		if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, FEED_STACK_RETRY_REV_PULSE))// 20mm逆転させる
		{
			_feed_set_seq(0x04E0, FEED_SEQ_TIMEOUT);
		}
	}
}

/*********************************************************************//**
 * @brief feed control sequence
 *  feed to APB-OUT sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
// リトライ処理搬
// 逆転で規定パルス搬送待ち
 **********************************************************************/
void _feed_apb_04E0_seq(u32 flag)// ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (SENSOR_CENTERING)
	{
	/* 入口レバーは閉じたままなので、ぶつかる前に停止させる */
		motor_ctrl_feed_stop();
		_feed_set_seq(0x04F0, FEED_SEQ_TIMEOUT);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_apb_retry(REJECT_CODE_FEED_TIMEOUT, ex_feed_task_seq);
	}

	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
	/* 規定パルス逆転完了 搬送停止 */
		motor_ctrl_feed_stop();
		_feed_set_seq(0x04F0, FEED_SEQ_TIMEOUT);
	}

}


/*********************************************************************//**
 * @brief feed control sequence
 *  feed to APB-OUT sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
//リトライ処理搬送モータ停止まち
//
 **********************************************************************/
void _feed_apb_04F0_seq(u32 flag)//OK
{
	u16 pulse=1;

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}

	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_feed_apb_retry(REJECT_CODE_FEED_TIMEOUT, ex_feed_task_seq);
	}

	else if (is_motor_ctrl_feed_stop())
	{
	// リトライ開始前のシーケンスに戻す

		#if 0 /* 2021-10-19 */
		if (!(SENSOR_VALIDATION_OFF))	//09-26 improtant
		#else
		if(0)
		#endif
		{
		// 戻し動作で識別ｾﾝｻがONしてしまった。 紙幣返却
			_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
		}
		else
		{
			switch (backup_feed_apb_sequence)
			{
				case 0x0410:
					if(WAIT_PB_IN_OFF <= FEED_STACK_RETRY_REV_PULSE)
					{
					//今はこっちのなっているが、返却距離を変えると変わってくるので、保護処理としていれておく
						pulse = 1;
					}
					else
					{
						pulse = WAIT_PB_IN_OFF - FEED_STACK_RETRY_REV_PULSE;
					}

					if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, (pulse)))
					{
						_feed_set_seq(backup_feed_apb_sequence, FEED_SEQ_TIMEOUT);
					}
					break;
				case 0x0420:
					if(WAIT_PB_OUT_OFF <= FEED_STACK_RETRY_REV_PULSE)
					{
						pulse = 1;
					}
					else
					{
						pulse = WAIT_PB_OUT_OFF - FEED_STACK_RETRY_REV_PULSE;
					}

					if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, ( WAIT_PB_OUT_OFF - FEED_STACK_RETRY_REV_PULSE )))
					{
						_feed_set_seq(backup_feed_apb_sequence, FEED_SEQ_TIMEOUT);
					}
				case 0x0430:
					if(WAIT_EXIT_OFF <= FEED_STACK_RETRY_REV_PULSE)
					{
						pulse = 1;
					}
					else
					{
						pulse = WAIT_EXIT_OFF - FEED_STACK_RETRY_REV_PULSE;
					}

					if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, ( WAIT_EXIT_OFF - FEED_STACK_RETRY_REV_PULSE )))
					{
						_feed_set_seq(backup_feed_apb_sequence, FEED_SEQ_TIMEOUT);
					}
				default:
					// あり得ない処理
					if(WAIT_PB_IN_OFF <= FEED_STACK_RETRY_REV_PULSE)
					{
					//今はこっちのなっているが、返却距離を変えると変わってくるので、保護処理としていれておく
						pulse = 1;
					}
					else
					{
						pulse = WAIT_PB_IN_OFF - FEED_STACK_RETRY_REV_PULSE;
					}

					if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, ( WAIT_PB_IN_OFF - FEED_STACK_RETRY_REV_PULSE )))
					{
						_feed_set_seq( 0x0410, FEED_SEQ_TIMEOUT);
					}
					break;
			}
		}
	}
}


/*********************************************************************//**
 * force feed to stack position
 **********************************************************************/
/*********************************************************************//**
 * @brief feed control interrupt procedure (force stack sequence)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_force_stack_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:									/* seq0600 */
		_feed_fstack_0600_seq(flag);
		break;
	case 0x01:									/* seq0601 */
		_feed_fstack_0601_seq(flag);
		break;
	case 0x02:									/* seq0602 */
		_feed_fstack_0602_seq(flag);
		break;
	case 0x03:									/* seq0603 */
		_feed_fstack_0603_seq(flag);
		break;
	case 0x04:									/* seq0604 */
		_feed_fstack_0604_seq(flag);
		break;
	case 0x05:									/* seq0605 */
		_feed_fstack_0605_seq(flag);
		break;
	case 0x06:									/* seq0606 */
		_feed_fstack_0606_seq(flag);
		break;
	case 0x80:									/* seq0680 */
		_feed_fstack_0680_seq(flag);
		break;
	case 0x08:									/* seq0608 */
		_feed_fstack_0608_seq(flag);
		break;
	case 0x09:									/* seq0609 */
		_feed_fstack_0609_seq(flag);
		break;
	/* stack pause */
	case 0x10:									/* seq0610 */
		_feed_fstack_0610_seq(flag);
		break;
	case 0x11:									/* seq0611 */
		_feed_fstack_0611_seq(flag);
		break;
	case 0x12:									/* seq0612 */
		_feed_fstack_0612_seq(flag);
		break;
	case 0x13:									/* seq0613 */
		_feed_fstack_0613_seq(flag);
		break;
	/* stack retry */
	case 0x20:									/* seq0620 */
		_feed_fstack_0620_seq(flag);
		break;
	case 0x21:									/* seq0621 */
		_feed_fstack_0621_seq(flag);
		break;
	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 14);
		break;
	}
}



/*********************************************************************//**
 * @brief feed control sequence 0x0500
 *  wait motor start
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/



/*********************************************************************//**
 * @brief force feed control sequence 0x0600
 *  wait motor start
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0600_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		s_feed_alarm_retry++;
		if (s_feed_alarm_retry >= FEED_STACK_RETRY_COUNT)
		{
			_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
		}
		else
		{
			s_feed_alarm_code = ALARM_CODE_FEED_TIMEOUT_SK;
			motor_ctrl_feed_stop();
			_feed_set_seq(FEED_SEQ_FORCE_STACK, FEED_SEQ_TIMEOUT);
		}
	}
	else if (SENSOR_ENTRANCE)
	{
		if (!(SENSOR_CENTERING))
		{
			_feed_set_fstack_pause();
		}
		else if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, 0))
		{
			_feed_set_seq(0x0601, FEED_SEQ_TIMEOUT);
		}
	}
	else if (SENSOR_CENTERING)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, 0))
		{
			_feed_set_seq(0x0602, FEED_SEQ_TIMEOUT);
		}
	}

	else if (!(SENSOR_VALIDATION_OFF))	//2024-08-02
	{
	//識別ON
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, 0))
		{
			_feed_set_seq(0x0603, FEED_SEQ_TIMEOUT);
		}
	}

	else if (SENSOR_APB_IN)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, 0))
		{
			_feed_set_seq(0x0604, FEED_SEQ_TIMEOUT);
		}
	}
	else if (SENSOR_APB_OUT)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, 0))
		{
			_feed_set_seq(0x0605, FEED_SEQ_TIMEOUT);
		}
	}
	else if (SENSOR_EXIT)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, 2000))
		{
			_feed_set_seq(0x0606, FEED_SEQ_TIMEOUT);
		}
	}
	else
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, (u16)(CONV_PULSE(CLEAR_WINDOW_SIZE))))
		{
			_feed_set_seq(0x0680, FEED_SEQ_TIMEOUT);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0601
 *  ENTRANCE sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0601_seq(u32 flag) //ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_fstack_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_fstack_retry(FEED_TIMEOUT);
		}
		else
		{
			_feed_set_fstack_retry(FEED_SLIP);
		}
	}
	else if (!(SENSOR_ENTRANCE))
	{
	/*  */
		motor_ctrl_feed_set_pulse(0);
		_feed_set_seq(0x0602, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0602
 *  CENTERING sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0602_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_fstack_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_fstack_retry(FEED_TIMEOUT);
		}
		else
		{
			_feed_set_fstack_retry(FEED_SLIP);
		}
	}
	else if (!(SENSOR_CENTERING))
	{
	/*  */
		motor_ctrl_feed_set_pulse(0);
		_feed_set_seq(0x0603, FEED_SEQ_TIMEOUT);
	}
}


void _feed_fstack_0603_seq(u32 flag) //2024-08-02
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_fstack_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_fstack_retry(FEED_TIMEOUT);
		}
		else
		{
			_feed_set_fstack_retry(FEED_SLIP);
		}
	}
	else if (SENSOR_VALIDATION_OFF)	//09-26 improtant
	{
	/*  */
		motor_ctrl_feed_set_pulse(0);
		_feed_set_seq(0x0604, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0604
 *  APB_IN sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0604_seq(u32 flag) //ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (SENSOR_ENTRANCE)
	{
	/* pause */
		_feed_set_fstack_pause();
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_fstack_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_fstack_retry(FEED_TIMEOUT);
		}
		else
		{
			_feed_set_fstack_retry(FEED_SLIP);
		}
	}
	else if (!(SENSOR_APB_IN))
	{
	/*  */
		motor_ctrl_feed_set_pulse(0);
		_feed_set_seq(0x0605, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0605
 *  APB_OUT sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0605_seq(u32 flag) //ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (SENSOR_ENTRANCE)
	{
	/* pause */
		_feed_set_fstack_pause();
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_fstack_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_fstack_retry(FEED_TIMEOUT);
		}
		else
		{
			_feed_set_fstack_retry(FEED_SLIP);
		}
	}
	else if (!(SENSOR_APB_OUT))
	{
	/*  */
		motor_ctrl_feed_set_pulse(0);
		_feed_set_seq(0x0606, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0606
 *  EXIT sensor OFF position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0606_seq(u32 flag) //ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (SENSOR_ENTRANCE)
	{
	/* pause */
		_feed_set_fstack_pause();
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_fstack_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_fstack_retry(FEED_TIMEOUT);
		}
		else
		{
			_feed_set_fstack_retry(FEED_SLIP);
		}
	}
	else if (!(SENSOR_EXIT))
	{
	/*  */
		motor_ctrl_feed_set_pulse(WAIT_LAST_FEED_FORCE);
		_feed_set_seq(0x0680, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0680
 *  EXIT sensor OFF position + Clear Window Size
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0680_seq(u32 flag) //ok
{
	u16 rest_pulse;

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (SENSOR_ENTRANCE)
	{
	/* pause */
		_feed_set_fstack_pause();
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_fstack_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_fstack_retry(FEED_TIMEOUT);
	}
	else if (SENSOR_EXIT)
	{
	/*  */
		motor_ctrl_feed_set_pulse(0);
		_feed_set_seq(0x0606, FEED_SEQ_TIMEOUT);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_feed_set_pulse(WAIT_LAST_FEED_FORCE);
		_feed_set_seq(0x0608, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0608
 *  feed to stack position
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0608_seq(u32 flag)//ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_fstack_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_fstack_retry(FEED_TIMEOUT);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x0609, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0609
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0609_seq(u32 flag)//ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ((SENSOR_CENTERING) || (SENSOR_APB_IN)
	 || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
	/* centering sensor or validation sensor or apb-in sensor
	   or apb-out sensor or exit sensor or stacker-in sensor ON */
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_SK);
	}
	else if ((is_ld_mode()) && (SENSOR_EXIT)) /* Fixed EXIT sensor JAM(LD-Mode) */
	{
		_feed_set_fstack_retry(FEED_OTHER_SENSOR);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_fstack_retry(FEED_TIMEOUT);
	}
	else if (is_motor_ctrl_feed_stop())
	{
	/* no error <正常終了> */
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_FORCE_STACK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}


/*********************************************************************//**
 * @brief feed control sub function
 *  set pause (force stack)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _feed_set_fstack_pause(void)
{
	s_feed_task_pause_seq = ex_feed_task_seq;
	motor_ctrl_feed_stop();
	_feed_set_seq(0x0610, FEED_SEQ_TIMEOUT);
}


/*********************************************************************//**
 * @brief feed control sequence 0x0610
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0610_seq(u32 flag) //ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if (is_motor_ctrl_feed_stop())
	{
	/* no error */
		s_feed_sensor_backup = (ex_position_sensor & (POSI_CENTERING|POSI_APB_IN|POSI_APB_OUT|POSI_EXIT));
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_FORCE_STACK_RSP, TMSG_SUB_PAUSE, 0, 0, 0);
		_feed_set_seq(0x0611, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief accept control sequence 0x0611
 *  check sensor
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0611_seq(u32 flag) //ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (!(SENSOR_ENTRANCE))
	{
		_feed_set_seq(0x0612, FEED_PAUSE_CHECK_TIME);
	}
}


/*********************************************************************//**
 * @brief accept control sequence 0x0612
 *  check sensor (500msec)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0612_seq(u32 flag) //ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (SENSOR_ENTRANCE)
	{
		_feed_set_seq(0x0611, FEED_SEQ_TIMEOUT);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_FORCE_STACK_RSP, TMSG_SUB_RESUME, 0, 0, 0);
		_feed_set_seq(0x0613, 0);
	}
}


/*********************************************************************//**
 * @brief accept control sequence 0x0613
 *  wait motor start
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0613_seq(u32 flag) //ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (SENSOR_ENTRANCE)
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_FORCE_STACK_RSP, TMSG_SUB_PAUSE, 0, 0, 0);
		_feed_set_seq(0x0611, FEED_SEQ_TIMEOUT);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if (SENSOR_APB_IN)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, 0))
		{
			_feed_set_seq(0x0604, FEED_SEQ_TIMEOUT);
		}
	}
	else if (SENSOR_APB_OUT)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, 0))
		{
			_feed_set_seq(0x0605, FEED_SEQ_TIMEOUT);
		}
	}
	else if (SENSOR_EXIT)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, 0))
		{
			_feed_set_seq(0x0606, FEED_SEQ_TIMEOUT);
		}
	}
	else
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, (u16)(CONV_PULSE(CLEAR_WINDOW_SIZE))))
		{
			_feed_set_seq(0x0680, FEED_SEQ_TIMEOUT);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sub function
 *  set stack retry
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _feed_set_fstack_retry(u32 alarm_code) //0x0600 イニシャル時の強制搬送
{
	//2024-08-02
	u16 bill_in;

	bill_in = _main_bill_in();
	if (bill_in == BILL_IN_STACKER)
	{
	    switch (alarm_code)
	    {
	        case FEED_OTHER_SENSOR:
				alarm_code = ALARM_CODE_FEED_OTHER_SENSOR_SK;
		        break;
	        case FEED_SLIP:
				alarm_code = ALARM_CODE_FEED_SLIP_SK;
		        break;
	        case FEED_TIMEOUT:
				alarm_code = ALARM_CODE_FEED_TIMEOUT_SK;
		        break;
	        case FEED_MOTOR_LOCK:
				alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_SK;
		        break;
	        default:
				alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_SK;
		        break;
	    }
	}
	else
	{
	    switch (alarm_code)
	    {
	        case FEED_OTHER_SENSOR:
				alarm_code = ALARM_CODE_FEED_OTHER_SENSOR_AT;
		        break;
	        case FEED_SLIP:
				alarm_code = ALARM_CODE_FEED_SLIP_AT;
		        break;
	        case FEED_TIMEOUT:
				alarm_code = ALARM_CODE_FEED_TIMEOUT_AT;
		        break;
	        case FEED_MOTOR_LOCK:
				alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_AT;
		        break;
	        default:
				alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_AT;
		        break;
	    }
	}

	s_feed_alarm_retry++;
	if (s_feed_alarm_retry >= FEED_STACK_RETRY_COUNT)
	{
	/* retry over */
		_feed_set_alarm(alarm_code);
	}
	else
	{
		s_feed_alarm_code = alarm_code;
		motor_ctrl_feed_stop();
		_feed_set_seq(0x0620, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0620
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0620_seq(u32 flag)//ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_fstack_retry(FEED_TIMEOUT);
	}
	else if (is_motor_ctrl_feed_stop())
	{
		if (SENSOR_ENTRANCE)
		{
			_feed_set_seq(0x0600, FEED_SEQ_TIMEOUT);
		}
		else
		{
			if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, FEED_STACK_RETRY_REV_PULSE))
			{
				_feed_set_seq(0x0621, FEED_SEQ_TIMEOUT);
			}
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0621
 *  feed revers
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_fstack_0621_seq(u32 flag) //ok
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_fstack_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_fstack_retry(FEED_TIMEOUT);
	}
	else if ((IS_FEED_EVT_OVER_PULSE(flag)) || (SENSOR_ENTRANCE))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x0600, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control interrupt procedure (reject sequence)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_reject_seq_proc2(u32 flag) //2024-02-14
{

	switch(ex_feed_task_seq & 0x00FF)
	{
	case 0x00:
		_feed_reject_0700_seq2(flag);
		break;
	case 0x02:
		_feed_reject_0702_seq2(flag);
		break;
	case 0x03:
		_feed_reject_0703_seq2(flag);
		break;
	case 0x04:
		_feed_reject_0704_seq2(flag);
		break;
	case 0x06:
		_feed_reject_0706_seq2(flag);
		break;
	case 0x08:
		_feed_reject_0708_seq2(flag);
		break;
	case 0x0A:
		_feed_reject_070A_seq2(flag);
		break;
	case 0x0C:
		_feed_reject_070C_seq2(flag);
		break;

	case 0x10:
		_feed_reject_0710_seq2(flag);
		break;
	case 0x12:
		_feed_reject_0712_seq2(flag);
		break;

	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);
		/* system error ? */
		_feed_system_error(0, 15);
		break;
	}
}

void _feed_reject_0700_seq2(u32 flag)
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	// リトライ処理用とこのシーケンスでモータが起動できない場合のタイムアウト

		s_feed_alarm_retry++;
		if (s_feed_alarm_retry > FEED_REJECT_RETRY_COUNT)
		{
		/* retry over */
			_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
		}
		else
		{
			s_feed_alarm_code = ALARM_CODE_FEED_TIMEOUT_SK;

			motor_ctrl_feed_stop();

			_feed_set_seq(FEED_SEQ_REJECT, FEED_SEQ_TIMEOUT);
		}
	}
	else
	{
		//連続処理か判断
		if(1)
		{
			if ((s_feed_reject_option & FEED_REJECT_OPTION_AFTER_ESCROW) == FEED_REJECT_OPTION_AFTER_ESCROW)
			{
			// まず、紙幣を見つけて、PB INまで紙幣を返却
				// entry->feed

				if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed, FEED_REJECT_CENTERING_ON_TO_OFF))
				{
					// PB INまで返却したい。
					_feed_set_seq(0x0702, FEED_SEQ_TIMEOUT);
				}
			}
			else
			{
				if ( (!SENSOR_VALIDATION_OFF) || SENSOR_APB_IN || SENSOR_APB_OUT || SENSOR_EXIT )
				{
				// 札長185mmの場合、幅寄せONの状態でEXIT ON
					if(SENSOR_CENTERING)
					{
						// entry->feed

						if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed, FEED_REJECT_CENTERING))
						{
							_feed_set_seq(0x0706, FEED_SEQ_TIMEOUT);
						}
					}
					else
					{
						// entry->feed
	
						if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed, FEED_REJECT_CENTERING_ON_TO_OFF))
						{
							// 幅よせセンサONまで返却したい。
							_feed_set_seq(0x0704, FEED_SEQ_TIMEOUT);
						}
					}
				}
				else if(SENSOR_CENTERING)
				{
				// 幅寄せセンサがONしている。
				// 紙幣位置がおよそ推測できるので、パルスで返却
				// 紙なし位置まで紙幣を搬送したい
		        // マージンをみて185mmは返却
					// entry->feed
	
					if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed, FEED_REJECT_CENTERING))
					{
						_feed_set_seq(0x0706, FEED_SEQ_TIMEOUT);
					}
				}
				else if (SENSOR_ONLY_ENTRANCE_ON)
				{
				//入り口を除く全てのセンサがOFFの場合
					// Rev 30mm
		
					if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed, FEED_HUNGING_POSITION_START))
					{
						_feed_set_seq(0x0708, FEED_SEQ_TIMEOUT);
					}
				}
				else
				{
				// 紙幣が全く見つからない　紙幣長手方向の完全折れに対応する為、長めに返却
		
					if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed, FEED_REJECT_CENTERING_ON_TO_OFF))
					{
						_feed_set_seq(0x0708, FEED_SEQ_TIMEOUT);
					}
				}
			}
		}
		else
		{
		// 連続処理

		}
	}
}


// PB INまで紙幣を返却
void _feed_reject_0702_seq2(u32 flag)//ok same
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_feed_reject_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
#if defined(UBAPRO_LD)
		if (!(SENSOR_EXIT))
		{
			_feed_set_alarm(ALARM_CODE_CHEAT);
		}
		else if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_feed_reject_retry(FEED_TIMEOUT);
		}
		else
		{
		/* feed slip */
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
#else
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_feed_reject_retry(FEED_TIMEOUT);
		}
		else
		{
		/* feed slip */
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
#endif
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
#if defined(UBAPRO_LD)
		if (!(SENSOR_EXIT))
		{
			_feed_set_alarm(ALARM_CODE_CHEAT);
		}
		else
		{
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
#else
		_feed_set_feed_reject_retry(FEED_SLIP);
#endif
	}
	else if (SENSOR_APB_IN)
	{
		motor_ctrl_feed_set_pulse(FEED_REJECT_CENTERING_ON_TO_OFF);
		_feed_set_seq(0x0703, FEED_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)		/* '21-01-21 */
	else if((s_feed_reject_option & FEED_REJECT_OPTION_AFTER_ESCROW) != FEED_REJECT_OPTION_AFTER_ESCROW)
	{
		if((ex_rc_status.sst21A.byte & RC1_POS2_POS3) != 0
		|| (ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0
		|| (((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0 || (ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0) && is_quad_model()))
		{
			_feed_set_alarm(ALARM_CODE_CHEAT);
		}
	}
#endif
}


// 識別センサまで紙幣を返却
void _feed_reject_0703_seq2(u32 flag)// same
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_feed_reject_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_feed_reject_retry(FEED_TIMEOUT);
		}
		else
		{
		/* feed slip */
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
		if ( (SENSOR_VALIDATION_OFF) && !(SENSOR_APB_IN) && (SENSOR_APB_OUT || SENSOR_EXIT) )
		{
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
		else
		{
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
	}
	else if (!(SENSOR_VALIDATION_OFF))
	{
		motor_ctrl_feed_set_pulse(FEED_REJECT_CENTERING_ON_TO_OFF);
		_feed_set_seq(0x0704, FEED_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)		/* '21-01-21 */
	else if((s_feed_reject_option & FEED_REJECT_OPTION_AFTER_ESCROW) != FEED_REJECT_OPTION_AFTER_ESCROW)
	{
		if((ex_rc_status.sst21A.byte & RC1_POS2_POS3) != 0
		|| (ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0
		|| (((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0 || (ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0) && is_quad_model()))
		{
			_feed_set_alarm(ALARM_CODE_CHEAT);
		}
	}
#endif
}

/*********************************************************************//**
 * @brief feed control sequence
 *  feed to EXIT sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
 * 識別と幅よせのON ONを待つ

 **********************************************************************/
void _feed_reject_0704_seq2(u32 flag)//ok
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
//#if 1 //2024-05-24 //QAテストで返却スリップが発生する時がまれにある為、UBA500と同じEXIT ONでのパスル監視は廃止
	#if !defined(UBA_RTQ) //RTQは連れ戻し監視していないので無効にする
	else if( SENSOR_CENTERING && SENSOR_EXIT )
	{
	//最大札長以上なので、BOX内の紙幣まで引き抜かれている可能性あり	
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	#endif
//#endif
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_feed_reject_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_feed_reject_retry(FEED_TIMEOUT);
		}
		else
		{
		/* feed slip */
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
		if ( (SENSOR_VALIDATION_OFF) && !(SENSOR_APB_IN) && (SENSOR_APB_OUT || SENSOR_EXIT) )
		{
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
		else
		{
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
	}
	else if( !(SENSOR_VALIDATION_OFF) && SENSOR_CENTERING && !(SENSOR_APB_IN) )
	{
	// 主の処理
	// 次は識別センサがOFFするのは待つ
		_feed_set_seq(0x0706, FEED_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)		/* '21-01-21 */
	else if((s_feed_reject_option & FEED_REJECT_OPTION_AFTER_ESCROW) != FEED_REJECT_OPTION_AFTER_ESCROW)
	{
		if((ex_rc_status.sst21A.byte & RC1_POS2_POS3) != 0
		|| (ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0
		|| (((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0 || (ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0) && is_quad_model()))
		{
			_feed_set_alarm(ALARM_CODE_CHEAT);
		}
	}
#endif
}



/*********************************************************************//**
 * @brief feed control sequence
 *  feed to EXIT sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None

// 識別センサOFF待ち
 **********************************************************************/
void _feed_reject_0706_seq2(u32 flag)//ok NEW_HANG
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	//#if 1 //2024-05-24 //QAテストで返却スリップが発生する時がまれにある為、UBA500と同じEXIT ONでのパスル監視は廃止
	#if !defined(UBA_RTQ) //RTQは連れ戻し監視していないので無効にする
	else if( SENSOR_CENTERING && SENSOR_EXIT )
	{
	//最大札長以上なので、BOX内の紙幣まで引き抜かれている可能性あり	
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	#endif
	//#endif
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_feed_reject_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_feed_reject_retry(FEED_TIMEOUT);
		}
		else
		{
		/* feed slip */
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
	}
	else if (SENSOR_APB_IN)
	{
		motor_ctrl_feed_set_pulse(FEED_REJECT_CENTERING_ON_TO_OFF);
		_feed_set_seq(0x0704, FEED_SEQ_TIMEOUT);
	}
	else
	{
		if(ex_reject_escrow == 1)
		{
			if ((SENSOR_VALIDATION_OFF))
			{
				motor_ctrl_feed_set_pulse(FEED_HANGING_POSITION_NEW);
				_feed_set_seq(0x0708, FEED_SEQ_TIMEOUT);
			}
		}
		else
		{
		/* Escrow以外からの返却は、ハンギングより返却優先 */
			if (SENSOR_ONLY_ENTRANCE_ON)
			{
			//ポリマ紙幣の時、問題発生するかもしれないが、RTQはUBA500RTQに合わせる(返却より、ハンギング優先)
			#if !defined(UBA_RTQ) //2023-03-29	
				motor_ctrl_feed_set_pulse(FEED_NO_HANGING_POSITION_SHORT_TICKET);
			#else
				motor_ctrl_feed_set_pulse(FEED_HANGING_POSITION_SHORT_TICKET);
			#endif
				_feed_set_seq(0x0708, FEED_SEQ_TIMEOUT);
			}
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0703
 *  feed to VALIDATION sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
// 紙幣ハンギング位置までの紙幣返却中
// 入り口を除く全てのセンサがOFFの場合
 **********************************************************************/
void _feed_reject_0708_seq2(u32 flag)// ok
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	//#if 1 //2024-05-24 //QAテストで返却スリップが発生する時がまれにある為、UBA500と同じEXIT ONでのパスル監視は廃止
	#if !defined(UBA_RTQ) //RTQは連れ戻し監視していないので無効にする
	else if( SENSOR_CENTERING && SENSOR_EXIT )
	{
	//最大札長以上なので、BOX内の紙幣まで引き抜かれている可能性あり	
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	#endif
	//#endif
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_feed_reject_retry(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if(SENSOR_ONLY_ENTRANCE_ON || SENSOR_ALL_OFF)
		{

			motor_ctrl_feed_stop();

			_feed_set_seq(0x070A, FEED_SEQ_TIMEOUT);
		}
		else
		{
			if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
			{
			/* time out */
				_feed_set_feed_reject_retry(FEED_TIMEOUT);
			}
			else
			{
			/* feed slip */
				_feed_set_feed_reject_retry(FEED_SLIP);
			}
		}
	}
	else if ( SENSOR_APB_IN || SENSOR_APB_OUT || SENSOR_EXIT )
	{
	// 意図しないセンサがONした
		motor_ctrl_feed_set_pulse(FEED_REJECT_CENTERING_ON_TO_OFF);
		_feed_set_seq(0x0704, FEED_SEQ_TIMEOUT);
	}
	else if ( !(SENSOR_VALIDATION_OFF) )
	{
	// 意図しないセンサがONした
		_feed_set_seq(0x0706, FEED_SEQ_TIMEOUT);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x070A, FEED_SEQ_TIMEOUT);
	}
#if defined(UBA_RTQ)		/* '21-01-21 */
	else if((s_feed_reject_option & FEED_REJECT_OPTION_AFTER_ESCROW) != FEED_REJECT_OPTION_AFTER_ESCROW)
	{
		if((ex_rc_status.sst21A.byte & RC1_POS2_POS3) != 0
		|| (ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0
		|| (((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0 || (ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0) && is_quad_model()))
		{
			_feed_set_alarm(ALARM_CODE_CHEAT);
		}
	}
#endif
}

/*********************************************************************//**
 * @brief feed control sequence 0x0703
 *  feed to VALIDATION sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
// 返却完了、モータ停止待ち
 **********************************************************************/
void _feed_reject_070A_seq2(u32 flag) //ok
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	//#if 1 //2024-05-24 //QAテストで返却スリップが発生する時がまれにある為、UBA500と同じEXIT ONでのパスル監視は廃止
	#if !defined(UBA_RTQ) //RTQは連れ戻し監視していないので無効にする
	else if( SENSOR_CENTERING && SENSOR_EXIT )
	{
	//最大札長以上なので、BOX内の紙幣まで引き抜かれている可能性あり	
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	#endif
	//#endif
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_feed_reject_retry(FEED_MOTOR_LOCK);
	}
	// モータが停止しないエラ-を追加してもいいかも
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_feed_reject_retry(FEED_TIMEOUT);
		}
		else
		{
		/* feed slip */
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
	}
	else if (is_motor_ctrl_feed_stop() )
	{
//		ex_reject_escrow = 0; これを使用するとポリマ紙幣の2回目返却でハンギングできない
		_feed_set_seq(0x070C, FEED_SEQ_STOP_CONF_TIMEOUT);
	}
#if defined(UBA_RTQ)		/* '21-01-21 */
	else if((s_feed_reject_option & FEED_REJECT_OPTION_AFTER_ESCROW) != FEED_REJECT_OPTION_AFTER_ESCROW)
	{
		if((ex_rc_status.sst21A.byte & RC1_POS2_POS3) != 0
		|| (ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0
		|| (((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0 || (ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0) && is_quad_model()))
		{
			_feed_set_alarm(ALARM_CODE_CHEAT);
		}
	}
#endif
}


void _feed_reject_070C_seq2(u32 flag) //ok
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ( SENSOR_APB_IN || SENSOR_APB_OUT || SENSOR_EXIT )
	{
	// 意図しないセンサがONした
		_feed_set_seq(0x0700, FEED_SEQ_TIMEOUT);
	}
	else if ( !(SENSOR_VALIDATION_OFF) )
	{
	// 意図しないセンサがONした
		_feed_set_seq(0x0700, FEED_SEQ_TIMEOUT);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_REJECT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
#if defined(UBA_RTQ)		/* '21-01-21 */
	else if((s_feed_reject_option & FEED_REJECT_OPTION_AFTER_ESCROW) != FEED_REJECT_OPTION_AFTER_ESCROW)
	{
		if((ex_rc_status.sst21A.byte & RC1_POS2_POS3) != 0
		|| (ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0
		|| (((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0 || (ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0) && is_quad_model()))
		{
			_feed_set_alarm(ALARM_CODE_CHEAT);
		}
	}
#endif
}


/*********************************************************************//**
 * @brief feed control sequence 0x0703
 *  feed to VALIDATION sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
// リトライ処理の開始シーケンス、モータ停止確認後、紙幣位置により正転距離を設定
 **********************************************************************/
void _feed_reject_0710_seq2(u32 flag) //ok
{

	u16 drive_pulse = 0;

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_feed_reject_retry(FEED_MOTOR_LOCK);
	}
	// モータが停止しないエラ-を追加してもいいかも
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_feed_reject_retry(FEED_TIMEOUT);
		}
		else
		{
		/* feed slip */
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
	}
	else if(ex_validation.pulse_count < (LENGTH_LOWER_LIMIT - 5))
	{
	/* only short bill	*/// Feed->Entry
		//if (IERR_CODE_OK == motor_ctrl_entry_feed_fwd(ex_entry_motor_speed[ENTRY_SPEED_ESCROW].set_speed, 0,
		//													ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, FEED_REJECT_RETRY1_FWD_SHORT_PULSE, 1)) // 20mm

		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, FEED_REJECT_RETRY1_FWD_SHORT_PULSE))
		{
			_feed_set_seq(0x0712, FEED_REJECT_RETRY_FWD_TIME);
		}
	}
	else if (SENSOR_CENTERING)
	{
		if( s_feed_alarm_retry == 1 )
		{
		// 1st retry
			drive_pulse = FEED_REJECT_RETRY1_FWD_LONG_PULSE;
		}
		else
		{
			drive_pulse = FEED_REJECT_RETRY2_FWD_LONG_PULSE;
		}
		// Feed->Entry
	//	if (IERR_CODE_OK == motor_ctrl_entry_feed_fwd(ex_entry_motor_speed[ENTRY_SPEED_ESCROW].set_speed, 0,
	//														ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, drive_pulse, 1)) // 70mm or 90mm
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, drive_pulse))
		{
			_feed_set_seq(0x0712, FEED_REJECT_RETRY_FWD_TIME);
		}
	}
	else
	{
		if( s_feed_alarm_retry == 1 )
		{
		// 1st retry
			drive_pulse = FEED_REJECT_RETRY1_FWD_SHORT_PULSE;
		}
		else
		{
			drive_pulse = FEED_REJECT_RETRY2_FWD_SHORT_PULSE;
		}
		// Feed->Entry
		//if (IERR_CODE_OK == motor_ctrl_entry_feed_fwd(ex_entry_motor_speed[ENTRY_SPEED_ESCROW].set_speed, 0,
		//													ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, drive_pulse, 1))// 20mm or 40mm
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, drive_pulse))
		{
			_feed_set_seq(0x0712, FEED_REJECT_RETRY_FWD_TIME);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0703
 *  feed to VALIDATION sensor ON position
 * @param[in]	feed motor event flag
 * @return 		None
// リトライ処理、規定パルス正転中
 **********************************************************************/
void _feed_reject_0712_seq2(u32 flag) //ok
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_feed_reject_retry(FEED_MOTOR_LOCK);
	}
	// モータが停止しないエラ-を追加してもいいかも
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		if (_ir_feed_motor_ctrl.pulse < FEED_MOTOR_SLIP_LIMIT)
		{
		/* time out */
			_feed_set_feed_reject_retry(FEED_TIMEOUT);
		}
		else
		{
		/* feed slip */
			_feed_set_feed_reject_retry(FEED_SLIP);
		}
	}
	else if (!(SENSOR_CENTERING) && (SENSOR_VALIDATION_OFF) &&!(SENSOR_APB_IN) && !(SENSOR_APB_OUT) )
	{
	// 紙幣がBOX内に完全に入る前には、正転動作は停止

		motor_ctrl_feed_stop();

		if (s_feed_reject_option == FEED_REJECT_OPTION_INITIAL)
		{
		/* イニシャル時の搬送リトライ時は幅よせを動かさない */
			_feed_set_seq(0x0700, FEED_SEQ_TIMEOUT);
		}
		else
		{
			if( (SENSOR_ENTRANCE) && (SENSOR_CENTERING) && !(SENSOR_VALIDATION_OFF) && (SENSOR_APB_IN) && (SENSOR_APB_OUT) )
			{
				_feed_set_seq(0x0700, FEED_SEQ_TIMEOUT);
			}
			else
			{			
				_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_REJECT_RSP, TMSG_SUB_FEED_REJECT_RETRY, 0, 0, 0);
				_feed_set_seq(FEED_SEQ_IDLE, 0);
			}
		}
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
	// 規定パルス正転完了

		motor_ctrl_feed_stop();

		if (s_feed_reject_option == FEED_REJECT_OPTION_INITIAL)
		{
		/* イニシャル時の搬送リトライ時は幅よせを動かさない */
			_feed_set_seq(0x0700, FEED_SEQ_TIMEOUT);
		}
		else
		{
//2020-01-14				if( (SENSOR_ENTRANCE) && (SENSOR_CENTERING) && !(SENSOR_VALIDATION_OFF) && (SENSOR_APB_IN) && (SENSOR_APB_OUT) )
			if( (SENSOR_ENTRANCE) && (SENSOR_CENTERING) && (SENSOR_APB_IN) && (SENSOR_APB_OUT) )
			{
				_feed_set_seq(0x0700, FEED_SEQ_TIMEOUT);
			}
			else
			{
			/* 正転処理が完了したので、幅よせを動かす為、一度mainタスクへ*/
				_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_REJECT_RSP, TMSG_SUB_FEED_REJECT_RETRY, 0, 0, 0);
				_feed_set_seq(FEED_SEQ_IDLE, 0);
			}
		}
	}
#if defined(UBA_RTQ)		/* '21-01-21 */
	else if((s_feed_reject_option & FEED_REJECT_OPTION_AFTER_ESCROW) != FEED_REJECT_OPTION_AFTER_ESCROW)
	{
		if((ex_rc_status.sst21A.byte & RC1_POS2_POS3) != 0
		|| (ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0
		|| (((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0 || (ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0) && is_quad_model()))
		{
			_feed_set_alarm(ALARM_CODE_CHEAT);
		}
	}
#endif
}


/*********************************************************************//**
 * Search bill
 **********************************************************************/
/*********************************************************************//**
 * @brief feed control interrupt procedure (search sequence)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_search_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:									/* seq0A00 */
		_feed_search_0a00_seq(flag);
		break;
	case 0x01:									/* seq0A01 */
		_feed_search_0a01_seq(flag);
		break;
	case 0x03:									/* seq0A03 */
		_feed_search_0a03_seq(flag);
		break;
	case 0x04:									/* seq0A04 */
		_feed_search_0a04_seq(flag);
		break;
	case 0x05:									/* seq0A05 */
		_feed_search_0a05_seq(flag);
		break;



	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 18);
		break;
	}
}



void _feed_search_0a00_seq(u32 flag)// use//2023-12-05
{

	u16 time;

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if (is_motor_ctrl_feed_stop())
	{
		if ((SENSOR_CENTERING) || /* !(SENSOR_VALIDATION_OFF)  || */ (SENSOR_APB_IN)
		 || (SENSOR_APB_OUT)
		 && (s_feed_search_option == FEED_SEARCH_OPTION_WINDOW)
		 )
		{
		/* bill in Acceptorにしたい処理なので、Exitは除外する */
			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_feed_set_seq(FEED_SEQ_IDLE, 0);
		}
		else if ((SENSOR_CENTERING) || /* !(SENSOR_VALIDATION_OFF)  || */ (SENSOR_APB_IN)
		 || (SENSOR_APB_OUT) || (SENSOR_EXIT)
		 && (s_feed_search_option == 0)
		 )
		{
		/* found bill */
			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_feed_set_seq(FEED_SEQ_IDLE, 0);
		}
		else
		{
			//if (IERR_CODE_OK == motor_ctrl_feed_rev(LOW_PWM_UBA_SPEED, 0))
			if (IERR_CODE_OK == motor_ctrl_feed_rev(FEED_MOTOR_SPEED_300MM, 0))
			{
				time = search_time[s_feed_alarm_retry][0];
				_feed_set_seq(0x0A01, time);
			}
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0A01
 *  feed revers
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_search_0a01_seq(u32 flag)// use Rev
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ((SENSOR_CENTERING) || /* !(SENSOR_VALIDATION_OFF)  || */ (SENSOR_APB_IN)
		|| (SENSOR_APB_OUT)
		&& (s_feed_search_option == FEED_SEARCH_OPTION_WINDOW)
		)
	{
	/* bill in Acceptorにしたい処理なので、Exitは除外する */
		motor_ctrl_feed_stop();
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
	else if ((SENSOR_CENTERING) || /* !(SENSOR_VALIDATION_OFF) || */ (SENSOR_APB_IN)
	 || (SENSOR_APB_OUT) || (SENSOR_EXIT)
		&& (s_feed_search_option == 0)	 
	  )
	{
		/* found bill */
		motor_ctrl_feed_stop();
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x0A03, FEED_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief feed control sequence 0x0A03
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_search_0a03_seq(u32 flag)
{

	u16 time;

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if (is_motor_ctrl_feed_stop())
	{
		if ((SENSOR_CENTERING) || /* !(SENSOR_VALIDATION_OFF)  || */ (SENSOR_APB_IN)
			|| (SENSOR_APB_OUT)
			&& (s_feed_search_option == FEED_SEARCH_OPTION_WINDOW)
			)
		{
			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_feed_set_seq(FEED_SEQ_IDLE, 0);
		}
		else if ((SENSOR_CENTERING) || /* !(SENSOR_VALIDATION_OFF) || */ (SENSOR_APB_IN)
		 || (SENSOR_APB_OUT) || (SENSOR_EXIT)
			&& (s_feed_search_option == 0)		 
		  )
		{
		/* found bill */
			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_feed_set_seq(FEED_SEQ_IDLE, 0);
		}
		else
		{
		// 見つからないのでFWD
			//if (IERR_CODE_OK == motor_ctrl_feed_fwd(MOTOR_MAX_SPEED, 0))
			if (IERR_CODE_OK == motor_ctrl_feed_fwd(FEED_MOTOR_SPEED_600MM, 0))
			{
				time = search_time[s_feed_alarm_retry][1];
				_feed_set_seq(0x0A04, time);
			}
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0A04
 *  feed forward
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_search_0a04_seq(u32 flag)// モータ正しい転中
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ((SENSOR_CENTERING) || /* !(SENSOR_VALIDATION_OFF)  || */ (SENSOR_APB_IN)
		|| (SENSOR_APB_OUT)
		&& (s_feed_search_option == FEED_SEARCH_OPTION_WINDOW)
		)
	{
		/* found bill */
		motor_ctrl_feed_stop();
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
	else if ((SENSOR_CENTERING) || /* !(SENSOR_VALIDATION_OFF) || */ (SENSOR_APB_IN)
	 || (SENSOR_APB_OUT) || (SENSOR_EXIT)
		&& (s_feed_search_option == 0)	 
	  )
	{
		/* found bill */
		motor_ctrl_feed_stop();
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		motor_ctrl_feed_stop();

		s_feed_alarm_retry++;

		if (s_feed_alarm_retry >  FEED_SEARCH_RETRY - 1 )
		{
		//UBA500と異なるが、見つからないだけなので、ここでエラーと判断しない
		//紙幣を探す処理のエラーは基本的にBOX OPEN
		//モータロックエラーもID-003ではパワーアップ前は出力しないので、
		//mainタスク側で無視する	

			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			_feed_set_seq(FEED_SEQ_IDLE, 0);

		}
		else
		{
			_feed_set_seq(0x0A00, FEED_SEQ_TIMEOUT);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0A05
 *  feed forward
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_search_0a05_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_SK);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_feed_stop();
		s_feed_alarm_retry++;
		_feed_set_seq(0x0A00, FEED_SEQ_TIMEOUT);
	}
}



/*********************************************************************//**
 * @brief entry control interrupt procedure (back sequence)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
#if defined(UBA_RTQ)
//UBA500は以前の入口ローラが分離型立ったため、返却時に入口ローラが開いたままになった状態になる場合があり
//その時に、次の紙幣の幅寄せが上手いいかない時があっとのと事
//一度搬送を動かす事で復活するとの事、

//UBA700では必用ないが、実装してしまったのでこのまま
//ポリマ紙幣の窓の問題で見失う事もあるかもしれないので、この処理をのこした方がいいかもしれない
void _entry_back_seq_proc(u32 flag)
{

	switch(ex_feed_task_seq & 0x00FF)
	{
	case	0x00:									/* seq 0B00 */
			_entry_back_0b00_seq(flag);
			break;
	case	0x01:									/* seq 0B01 */
			_entry_back_0b01_seq(flag);
			break;
	case	0x02:									/* seq 0B02 */
			_entry_back_0b02_seq(flag);
			break;
	default:									/* other */
			_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

			/* system error ? */
			_feed_system_error(0, 19);
			break;
	}
}


/*********************************************************************//**
 * @brief entry control sequence
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
// Start Entry Motor REV // 入り口モータREV
 **********************************************************************/
void _entry_back_0b00_seq(u32 flag)
{
	//if(IERR_CODE_OK == motor_ctrl_entry_rev(ex_entry_motor_speed[ENTRY_SPEED_FREE_RUN].set_speed, 0))
	if (IERR_CODE_OK == motor_ctrl_feed_rev(FEED_MOTOR_SPEED_300MM, 0))
	{
		_feed_set_seq(0x0b01, ENTRY_BACK_TIME);
	}
}


/*********************************************************************//**
 * @brief entry control sequence
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
// wait 100msec // 100msec経過待ち
 **********************************************************************/
void _entry_back_0b01_seq(u32 flag)
{
	if(IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x0b02, FEED_SEQ_TIMEOUT);
	}
}

/*********************************************************************//**
 * @brief entry control sequence
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
// wait entry motor stop // entry motor停止待ち
 **********************************************************************/
void _entry_back_0b02_seq(u32 flag)
{
	if(IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if(is_motor_ctrl_feed_stop())
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_ENTRY_BACK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

/*-------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------*/


#endif


/*********************************************************************//**
 * aging
 **********************************************************************/
/*********************************************************************//**
 * @brief feed control interrupt procedure (aging sequence)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_aging_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:									/* seq0E00 */
		_feed_aging_0e00_seq(flag);
		break;
	case 0x01:									/* seq0E01 FEED_AGING_CENTERING */
	case 0x11:									/* seq0E11 FEED_AGING_ESCROW */
	case 0x21:									/* seq0E21 FEED_AGING_APB */
	case 0x31:									/* seq0E31 FEED_AGING_STACK */
	case 0x41:									/* seq0E41 FEED_AGING_REJECT */
	case 0x51:									/* seq0E51 FEED_AGING_PAYOUT */
		_feed_aging_0e01_seq(flag);
		break;
	case 0x02:									/* seq0E02 */
		_feed_aging_0e02_seq(flag);
		break;
	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 19);
		break;
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0E00
 *  wait motor start
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_aging_0e00_seq(u32 flag)
{
	if ((SENSOR_ENTRANCE) || (SENSOR_CENTERING) || (SENSOR_APB_IN))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	else if ((SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_SK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else
	{
		switch (s_feed_aging_no)
		{
		case FEED_AGING_CENTERING:
			if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_CENTERING].set_speed, feed_aging_pulse[FEED_AGING_CENTERING]))
			{
				_feed_set_seq(0x0E01, FEED_SEQ_TIMEOUT);
			}
			break;
		case FEED_AGING_ESCROW:
			if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_ESCROW].set_speed, feed_aging_pulse[FEED_AGING_ESCROW]))
			{
// EMI用
				start_ad();
				_feed_set_seq(0x0E11, FEED_SEQ_TIMEOUT);
			}
			break;
		case FEED_AGING_APB:
			if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, feed_aging_pulse[FEED_AGING_APB]))
			{
				_feed_set_seq(0x0E21, FEED_SEQ_TIMEOUT);
			}
			break;
		case FEED_AGING_STACK:
			if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_STACK].set_speed, feed_aging_pulse[FEED_AGING_STACK]))
			{
				_feed_set_seq(0x0E31, FEED_SEQ_TIMEOUT);
			}
			break;
		case FEED_AGING_REJECT:
			if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed, feed_aging_pulse[FEED_AGING_REJECT]))
			{
				_feed_set_seq(0x0E41, FEED_SEQ_TIMEOUT);
			}
			break;
		case FEED_AGING_PAYOUT:
			if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_PAYOUT].set_speed, feed_aging_pulse[FEED_AGING_PAYOUT]))
			{
				_feed_set_seq(0x0E51, FEED_SEQ_TIMEOUT);
			}
			break;
		default:
			_feed_set_seq(FEED_SEQ_IDLE, 0);

			/* system error ? */
			_feed_system_error(0, 20);
			break;
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0E00
 *  aging
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_aging_0e01_seq(u32 flag)
{
	if ((SENSOR_ENTRANCE) || (SENSOR_CENTERING) || (SENSOR_APB_IN))
	{
		_feed_set_reject(REJECT_CODE_ACCEPTOR_STAY_PAPER);
	}
	else if ((SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		_feed_set_alarm(ALARM_CODE_FEED_OTHER_SENSOR_SK);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
		motor_ctrl_feed_stop();
// EMI用
#if 1
		if (s_feed_aging_no == FEED_AGING_ESCROW)
		{
		//	change_ad_sampling_mode(AD_MODE_WATCH);
			stop_ad();
		}
#endif
		_feed_set_seq(0x0E02, FEED_SEQ_TIMEOUT);
		if (s_feed_aging_no == FEED_AGING_CENTERING)
		{
			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_AGING_RSP, TMSG_SUB_INTERIM, 0, 0, 0);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0E02
 *  wait motor stop
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_aging_0e02_seq(u32 flag)
{
	if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_feed_set_alarm(ALARM_CODE_FEED_TIMEOUT_AT);
	}
	else if (is_motor_ctrl_feed_stop())
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_AGING_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}


//2024-03-18a
void _entry_feed_force_rev_retry(u32 alarm_code);	// only 0x08XX
void _entry_feed_force_rev_seq_proc(u32 flag) //ID-003 Disable, Forced return of bills detected at entrance
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:
		_entry_feed_force_rev_0800_seq(flag);
		break;
	case 0x02:
		_entry_feed_force_rev_0802_seq(flag);
		break;
	case 0x06:
		_entry_feed_force_rev_0806_seq(flag);
		break;
	case 0x08:
		_entry_feed_force_rev_0808_seq(flag);
		break;
	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 31);
		break;
	}
}


void _entry_feed_force_rev_0800_seq(u32 flag)
{

	if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed, 0))
	{

		_feed_set_seq(0x0802, 2500); /* JCM-E request 2.5s*/
		//UBA500は下記の理由でまたせている、同様にする
		//モータ起動をまたせないと、次のシーケンスですぐにモータ停止をすると、
		//モータ動作的には、停止しているがフラグ的には動作している事になって0x0806でタイムアウトになる。
		dly_tsk(500); 

	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out *//* 適切なエラーがないので、Acceptor JAMと分ける*/
		_feed_set_alarm(FEED_TIMEOUT);
	}
}



/*********************************************************************//**
 * @brief feed control sequence
 *  motor wait stability
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _entry_feed_force_rev_0802_seq(u32 flag) //全てOFF待ち
{
	if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_entry_feed_force_rev_retry(FEED_TIMEOUT);
	}
	//ここでモータ起動から一定時間またないと、モータ論理矛盾で次の0806でモータ停止にずっとならない
	//else if (SENSOR_ALL_OFF)	//全てOFF
	else if(SENSOR_ALL_OFF_WITHOUT_CIS) //OK
	{
		motor_ctrl_feed_stop();
		_feed_set_seq(0x0806, FEED_SEQ_TIMEOUT);
	}
}

void _entry_feed_force_rev_0806_seq(u32 flag) //モータ停止待ち
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_entry_feed_force_rev_retry(FEED_TIMEOUT);
	}
	else if ( SENSOR_APB_IN || SENSOR_APB_OUT || SENSOR_EXIT )
	{
	// 意図しないセンサがONした
		_entry_feed_force_rev_retry(FEED_OTHER_SENSOR);
	}
	else if (is_motor_ctrl_feed_stop() )
	{
		_feed_set_seq(0x0808, FEED_SEQ_STOP_CONF_TIMEOUT);
	}

}

void _entry_feed_force_rev_0808_seq(u32 flag) //センサ確認待ち
{

	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if ( SENSOR_APB_IN || SENSOR_APB_OUT || SENSOR_EXIT )
	{
	// 意図しないセンサがONした
		_entry_feed_force_rev_retry(FEED_OTHER_SENSOR);
	}
	else if(IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_FORCE_REV_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}

}



void _entry_feed_force_rev_retry(u32 alarm_code)	// only 0x08XX
{

	u16 bill_in;
	
	motor_ctrl_feed_stop();

	bill_in = _main_bill_in();
	if (bill_in == BILL_IN_STACKER)
	{
	    switch (alarm_code)
	    {
	        case FEED_OTHER_SENSOR:
				alarm_code = ALARM_CODE_FEED_OTHER_SENSOR_SK;
		        break;
	        case FEED_SLIP:
				alarm_code = ALARM_CODE_FEED_SLIP_SK;
		        break;
	        case FEED_TIMEOUT:
				alarm_code = ALARM_CODE_FEED_TIMEOUT_SK;
		        break;
	        case FEED_MOTOR_LOCK:
				alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_SK;
		        break;
	        default:
				alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_SK;
		        break;
	    }
	}
	else
	{
	    switch (alarm_code)
	    {
	        case FEED_OTHER_SENSOR:
				alarm_code = ALARM_CODE_FEED_OTHER_SENSOR_AT;
		        break;
	        case FEED_SLIP:
				alarm_code = ALARM_CODE_FEED_SLIP_AT;
		        break;
	        case FEED_TIMEOUT:
				alarm_code = ALARM_CODE_FEED_TIMEOUT_AT;
		        break;
	        case FEED_MOTOR_LOCK:
				alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_AT;
		        break;
	        default:
				alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_AT;
		        break;
	    }
	}

	s_feed_alarm_retry++;
	if(s_feed_alarm_retry > FEED_REJECT_RETRY_COUNT)
	{
	/* retry over */
		_feed_set_alarm(alarm_code);
	}
	else
	{
		s_feed_alarm_code = alarm_code;
		_feed_set_seq(0x0800, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * free run
 **********************************************************************/
/*********************************************************************//**
 * @brief feed control interrupt procedure (free run sequence)
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_freerun_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:									/* seq0F00 */
		_feed_freerun_0f00_seq(flag);
		break;
	case 0x01:									/* seq0F01 */
		_feed_freerun_0f01_seq(flag);
		break;
	case 0x02:									/* seq0F02 */
		_feed_freerun_0f02_seq(flag);
		break;
	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

		/* system error ? */
		_feed_system_error(0, 21);
		break;
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0F00
 *  wait motor start
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_freerun_0f00_seq(u32 flag)
{
	if (s_feed_freerun_dir == MOTOR_FWD)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed, 0))
		{
			_feed_set_seq(0x0F01, FEED_SPEED_CHECK_TIME);
		}
	}
	else if (s_feed_freerun_dir == MOTOR_REV)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed, 0))
		{
			_feed_set_seq(0x0F01, FEED_SPEED_CHECK_TIME);
		}
	}
	else
	{
		_feed_set_seq(FEED_SEQ_IDLE, 0);

		/* system error ? */
		_feed_system_error(0, 22);
	}

}


/*********************************************************************//**
 * @brief feed control sequence 0x0F01
 *  motor wait stability
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_freerun_0f01_seq(u32 flag)
{
	if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		_ir_feed_motor_ctrl.speed_check_pulse = 0;		//2025-09-24
		_ir_feed_motor_ctrl.speed_check_time = FEED_FREERUN_CHECK_TIME;	/* 100ms	*/
		dly_tsk(50);

		_feed_set_seq(0x0F02, FEED_FREERUN_CHECK_TIME);
	}
}


/*********************************************************************//**
 * @brief feed control sequence 0x0F02
 *  motor speed check
 * @param[in]	feed motor event flag
 * @return 		None
 **********************************************************************/
void _feed_freerun_0f02_seq(u32 flag)
{
	if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* time out */
		ex_feed_motor_test_speed = _ir_feed_motor_ctrl.speed_check_pulse;

		ex_dline_testmode.test_result = TEST_RESULT_OK;

		if ((ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed == MOTOR_MAX_SPEED)
		 && (ex_feed_motor_test_speed < (FEED_SPEED_ERR_LOWER_LIMIT * 10)))
		{
			_feed_set_alarm(ALARM_CODE_FEED_MOTOR_SPEED_LOW);
		}
		else if ((ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed == MOTOR_MAX_SPEED)
		 && (ex_feed_motor_test_speed > (FEED_SPEED_ERR_UPPER_LIMIT * 10)))
		{
			_feed_set_alarm(ALARM_CODE_FEED_MOTOR_SPEED_HIGH);
		}
		else if (s_feed_freerun_dir == MOTOR_FWD)
		{
			if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed, 0))
			{
			//2023-06-29	_feed_set_seq(0x0F01, FEED_SPEED_CHECK_TIME);
				_ir_feed_motor_ctrl.speed_check_pulse = 0;		//2025-09-24
				_ir_feed_motor_ctrl.speed_check_time = FEED_FREERUN_CHECK_TIME;	/* 100ms	*/
				dly_tsk(50);

				_feed_set_seq(0x0F02, FEED_FREERUN_CHECK_TIME);
			}
			else
			{
				_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

				/* system error ? */
				_feed_system_error(0, 23);
			}
		}
		else if (s_feed_freerun_dir == MOTOR_REV)
		{
			if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed, 0))
			{
			//2023-06-29	_feed_set_seq(0x0F01, FEED_SPEED_CHECK_TIME);
				_ir_feed_motor_ctrl.speed_check_pulse = 0;		//2025-09-24
				_ir_feed_motor_ctrl.speed_check_time = FEED_FREERUN_CHECK_TIME;	/* 100ms	*/
				dly_tsk(50);

				_feed_set_seq(0x0F02, FEED_FREERUN_CHECK_TIME);
			}
			else
			{
				_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

				/* system error ? */
				_feed_system_error(0, 24);
			}
		}
		else
		{
			_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);

			/* system error ? */
			_feed_system_error(0, 25);
		}
	}
}


/*********************************************************************//**
 * @brief feed control sub function
 *  set feed sequence
 * @param[in]	sequence no.
 * 				time out
 * @return 		None
 **********************************************************************/
void _feed_set_seq(u16 seq, u16 time_out)
{
	ex_feed_task_seq = seq;
	_ir_feed_ctrl.time = time_out;
	_ir_feed_ctrl.time_init = 1;

#if 1//2024-01-29a
	clr_flg(ID_FEED_CTRL_FLAG, ~EVT_ALL_BIT);
#else
	#if (DEBUG_POLYMER_DEBUG == 1)
	clr_flg(ID_FEED_CTRL_FLAG, EVT_FEED_SENSOR);
	#endif
#endif

#ifdef _ENABLE_JDL
    jdl_add_trace(ID_FEED_TASK, ((ex_feed_task_seq >> 8) & 0xFF), (ex_feed_task_seq & 0xFF), s_feed_alarm_code, s_feed_alarm_retry, s_feed_reject_code);
#endif /* _ENABLE_JDL */
}


/*********************************************************************//**
 * @brief feed control sub function
 *  set feed waiteing sequence
 * @param[in]	sequence no.
 * @return 		None
 **********************************************************************/
void _feed_set_waiting_seq(u32 seq)
{
	if (ex_feed_task_seq == FEED_SEQ_FORCE_QUIT)
	{
		s_feed_task_wait_seq = seq;
	}
	else
	{
		s_feed_task_wait_seq = seq;

		motor_ctrl_feed_stop();
		_feed_set_seq(FEED_SEQ_FORCE_QUIT, FEED_SEQ_TIMEOUT);
	}
}


/*********************************************************************//**
 * @brief feed control sub function
 *  reject response
 * @param[in]	reject code
 * @return 		None
 **********************************************************************/
void _feed_set_reject(u32 reject_code)
{
	u32 msg;

#ifdef _DEBUG_SIDE_BUFFERLING
	u8 cnt;
	if ((ex_feed_task_seq > 0x0308) && (ex_feed_task_seq < FEED_SEQ_FORCE_STACK))
	{
		for (cnt = 0; cnt < 64; cnt++)
		{
			ex_side_ad_after_buff[cnt] = ex_side_info.debug_buff[cnt];
		}
	}
#endif /* _DEBUG_SIDE_BUFFERLING */

	s_feed_reject_code = reject_code;


	motor_ctrl_feed_stop();

//	change_ad_sampling_mode(AD_MODE_WATCH);

	switch (ex_feed_task_seq & 0xFF00)
	{
	case FEED_SEQ_INITIAL:
		msg = TMSG_FEED_INITIAL_RSP;
		break;
	case FEED_SEQ_CENTERING:
		msg = TMSG_FEED_CENTERING_RSP;
		break;
	case FEED_SEQ_ESCROW:
		msg = TMSG_FEED_ESCROW_RSP;
		break;
	case FEED_SEQ_APB:
		msg = TMSG_FEED_APB_RSP;
		break;
	case FEED_SEQ_FORCE_STACK:
		msg = TMSG_FEED_FORCE_STACK_RSP;
		break;
	case FEED_SEQ_REJECT:
		msg = TMSG_FEED_REJECT_RSP;
		break;

	case FEED_SEQ_FORCE_REV:
		msg = TMSG_FEED_FORCE_REV_RSP;
		break;

	case FEED_SEQ_SEARCH:
		msg = TMSG_FEED_SEARCH_RSP;
		break;
	case FEED_SEQ_AGING:
		msg = TMSG_FEED_AGING_RSP;
		break;
	case FEED_SEQ_FREERUN:
		msg = TMSG_FEED_FREERUN_RSP;
		break;
#if defined(UBA_RTQ)
	case	ENTRY_SEQ_BACK:
			msg = TMSG_ENTRY_BACK_RSP;
			break;
	case FEED_SEQ_RC_PAYOUT:
	case FEED_SEQ_RS_PAYOUT:
		msg = TMSG_FEED_RC_PAYOUT_RSP;
		break;
	case	FEED_SEQ_RC_PAYOUT_STOP:	/* '21-10-11 */
			msg = TMSG_FEED_RC_PAYOUT_STOP_RSP;
			break;
	case FEED_SEQ_RC_STACK:
		msg = TMSG_FEED_RC_STACK_RSP;
		break;
	case FEED_SEQ_RC_COLLECT:
		msg = TMSG_FEED_RC_COLLECT_RSP;
		break;
	case FEED_SEQ_RC_FORCE_PAYOUT:
		msg = TMSG_FEED_RC_FORCE_PAYOUT_RSP;
		break;
	case FEED_SEQ_RS_FORCE_PAYOUT:
		msg = TMSG_FEED_RS_FORCE_PAYOUT_RSP;
		break;
#else
	case FEED_SEQ_REV_CHECK_BILL:
		msg = TMSG_FEED_REV_CHECK_BILL_RSP;
		break;
#endif // UBA_RTQ
	default:
		msg = TMSG_FEED_STATUS_INFO;
		break;
	}
	_feed_send_msg(ID_MAIN_MBX, msg, TMSG_SUB_REJECT, s_feed_reject_code, ex_feed_task_seq, ex_position_sensor);
	s_feed_task_wait_seq = FEED_SEQ_IDLE;
	_feed_set_seq(FEED_SEQ_FORCE_QUIT, FEED_SEQ_TIMEOUT);
}


/*********************************************************************//**
 * @brief feed control sub function
 *  alarm response
 * @param[in]	alarm code
 * @return 		None
 **********************************************************************/
void _feed_set_alarm(u32 alarm_code)
{
	u32 msg;

	s_feed_alarm_code = alarm_code;

	motor_ctrl_feed_stop();

	switch (ex_feed_task_seq & 0xFF00)
	{
	case FEED_SEQ_INITIAL:
		msg = TMSG_FEED_INITIAL_RSP;
		break;
	case FEED_SEQ_CENTERING:
		msg = TMSG_FEED_CENTERING_RSP;
		break;
	case FEED_SEQ_ESCROW:
		msg = TMSG_FEED_ESCROW_RSP;
		break;
	case FEED_SEQ_APB:
		msg = TMSG_FEED_APB_RSP;
		break;
	case FEED_SEQ_FORCE_STACK:
		msg = TMSG_FEED_FORCE_STACK_RSP;
		break;
	case FEED_SEQ_REJECT:
		msg = TMSG_FEED_REJECT_RSP;
		break;

	case FEED_SEQ_FORCE_REV:
		msg = TMSG_FEED_FORCE_REV_RSP;
		break;

	case FEED_SEQ_SEARCH:
		msg = TMSG_FEED_SEARCH_RSP;
		break;
	case FEED_SEQ_AGING:
		msg = TMSG_FEED_AGING_RSP;
		break;
	case FEED_SEQ_FREERUN:
		msg = TMSG_FEED_FREERUN_RSP;
		break;
#if defined(UBA_RTQ)
	case ENTRY_SEQ_BACK:
		msg = TMSG_ENTRY_BACK_RSP;
		break;

	case FEED_SEQ_RC_PAYOUT:
	case FEED_SEQ_RS_PAYOUT:
		msg = TMSG_FEED_RC_PAYOUT_RSP;
		break;
	case	FEED_SEQ_RC_PAYOUT_STOP:	/* '21-10-11 */
			msg = TMSG_FEED_RC_PAYOUT_STOP_RSP;
			break;

	case FEED_SEQ_RC_STACK:
		msg = TMSG_FEED_RC_STACK_RSP;
		break;
	case FEED_SEQ_RC_COLLECT:
		msg = TMSG_FEED_RC_COLLECT_RSP;
		break;
	case FEED_SEQ_RC_FORCE_PAYOUT:
		msg = TMSG_FEED_RC_FORCE_PAYOUT_RSP;
		break;
	case FEED_SEQ_RS_FORCE_PAYOUT:
		msg = TMSG_FEED_RS_FORCE_PAYOUT_RSP;
		break;
#else
	case FEED_SEQ_REV_CHECK_BILL:
		msg = TMSG_FEED_REV_CHECK_BILL_RSP;
		break;
#endif // UBA_RTQ
	default:
		msg = TMSG_FEED_STATUS_INFO;
		break;
	}

	_feed_send_msg(ID_MAIN_MBX, msg, TMSG_SUB_ALARM, s_feed_alarm_code, ex_feed_task_seq, ex_position_sensor);
	s_feed_task_wait_seq = FEED_SEQ_IDLE;
	_feed_set_seq(FEED_SEQ_FORCE_QUIT, FEED_SEQ_TIMEOUT);
}


/*********************************************************************//**
 * @brief feed control sub function
 *  all sensor status
 * @param[in]	None
 * @return 		true  : all sensor off
 * 				false : any sensor on
 **********************************************************************/
bool _is_feed_all_sensor_off(void)
{
	return (!(SENSOR_ENTRANCE)
		 && !(SENSOR_CENTERING)
		 && !(SENSOR_APB_IN)
		 && !(SENSOR_APB_OUT)
		 && !(SENSOR_EXIT)) ? true : false;
}

/******************************************************************************/
/*! @brief Initialize feed motor encoder pulse
    @param       	none
    @par            Refer
    @par            Modify
    - 変更するグローバル変数 ex_validation.pulse_count
    @return        	none
    @exception      none
******************************************************************************/
void _initialize_feed_motor_pulse(void) //0x3XXで使用するので、0x03XXでは呼び出し禁止
{
	ex_feed_pulse_count.total = 0;
	ex_feed_pulse_count.fwd = 0;
	ex_feed_pulse_count.rev = 0;
	ex_validation.pulse_count = 0;
}
/******************************************************************************/
/*! @brief Initialize feed motor encoder pulse
    @param       	none
    @par            Refer
    @par            Modify
    - 変更するグローバル変数 ex_total_pulse_count
    @return        	none
    @exception      none
******************************************************************************/
void _initialize_position_pulse(void)	//0x3XXで使用するので、0x03XXでは呼び出し禁止
{
	ex_position_pulse_count.entrance.start = 0;
	ex_position_pulse_count.entrance.end = 0;
	ex_position_pulse_count.centering.start = 0;
	ex_position_pulse_count.centering.end = 0;
	ex_position_pulse_count.apb_in.start = 0;
	ex_position_pulse_count.apb_in.end = 0;
	ex_position_pulse_count.apb_out.start = 0;
	ex_position_pulse_count.apb_out.end = 0;
	ex_position_pulse_count.exit.start = 0;
	ex_position_pulse_count.exit.end = 0;
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
void _feed_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_FEED_TASK;
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
			_feed_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_feed_system_error(1, 2);
	}
}

/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _feed_system_error(u8 fatal_err, u8 code)
{
#ifdef _DEBUG_SYSTEM_ERROR
	//_feed_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_feed_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_FEED_TASK, (u16)code, (u16)feed_msg.tmsg_code, (u16)feed_msg.arg1, fatal_err);
}


/*********************************************************************//**
 * @brief extra feed
	used to feed bill with big window deepeer into stacker in power up
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _feed_bill_over_window_rev(void)
{

	if (!is_motor_ctrl_feed_stop())
	{
		motor_ctrl_feed_stop();
		while(!is_motor_ctrl_feed_stop()){}
	}

	while (1)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_rev(FEED_MOTOR_SPEED_300MM, 0))
		{
			ex_1msec_timer = 500;
			do
			{
				//識別センサがCIS使えないので if ((SENSOR_CENTERING) || !(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN) || (SENSOR_APB_OUT) )
				if ((SENSOR_CENTERING) ||  (SENSOR_APB_IN) || (SENSOR_APB_OUT) )
				{
					break;
				}
				else if ( SENSOR_EXIT )
				{
					break;
				}
				else if ((SENSOR_ENTRANCE))
				{
					break;
				}

				OSW_TSK_sleep(2);					//最適化の関係でリリース版だと無限ループするのでディレイ追加  2022-01-14 2022-03-08

			}while(ex_1msec_timer);

			motor_ctrl_feed_stop();
			break;
		}
		break;
	}
}


void _feed_bill_over_window_fwd(void)
{
	if (!is_motor_ctrl_feed_stop())
	{
		motor_ctrl_feed_stop();
		while(!is_motor_ctrl_feed_stop()){}
	}
	while (1)
	{
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(FEED_MOTOR_SPEED_300MM, 0))	//2019-09-25 これはパルスの意味をなしてないので0でもいい
		{
			dly_tsk(500);
			motor_ctrl_feed_stop();
			break;
		}
	}
}


#if !defined(UBA_RTQ)//#if defined(HIGH_SECURITY_MODE)

void _feed_rev_seq_proc(u32 flag)
{
	switch (ex_feed_task_seq & 0x00FF)
	{
	case 0x00:
		_feed_rev_0500_seq(flag);
		break;
	case 0x02:
		_feed_rev_0502_seq(flag);
		break;
	case 0x04:
		_feed_rev_0504_seq(flag);
		break;
	case 0x08:
		_feed_rev_0508_seq(flag);
		break;
	case 0x0A:
		_feed_rev_050A_seq(flag);
		break;
	default:									/* other */
		_feed_set_alarm(ALARM_CODE_FEED_FORCED_QUIT);
		/* system error ? */
		_feed_system_error(0, 12);
		break;
	}
}

void _feed_rev_0500_seq_start(void)// OK
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_seq( 0x0508, FEED_SEQ_TIMEOUT);

	}

	else if( SENSOR_EXIT || SENSOR_APB_OUT || SENSOR_APB_IN )
	{
		_feed_set_seq( 0x0508, FEED_SEQ_TIMEOUT);

	}
	else if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed, 0))
	{
		_feed_set_seq( 0x0502, FEED_REV_CHECK_BILL);
	}
}


void _feed_rev_0500_seq(u32 flag)// OK
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		_feed_set_seq( 0x0508, FEED_SEQ_TIMEOUT);
	}

	else if( SENSOR_EXIT || SENSOR_APB_OUT || SENSOR_APB_IN )
	{
		_feed_set_seq( 0x0508, FEED_SEQ_TIMEOUT);

	}
	else if (IERR_CODE_OK == motor_ctrl_feed_rev(ex_feed_motor_speed[FEED_SPEED_REJECT].set_speed, 0))
	{
		_feed_set_seq( 0x0502, FEED_REV_CHECK_BILL);
	}
}



void _feed_rev_0502_seq(u32 flag)// 逆転中
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		// モータ停止命令
		motor_ctrl_feed_stop();
		_feed_set_seq( 0x0508, FEED_SEQ_TIMEOUT);

//		_feed_set_alarm(ALARM_CODE_CHEAT);
	}

	else if(SENSOR_EXIT || SENSOR_APB_OUT || SENSOR_APB_IN)
	{
		// モータ停止命令
		motor_ctrl_feed_stop();
		_feed_set_seq( 0x0508, FEED_SEQ_TIMEOUT);
		/* これでシーケンス終了してもいいかも */
//		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	/* 紙幣は見つからなかった */
		motor_ctrl_feed_stop();
		_feed_set_seq( 0x0504, FEED_SEQ_TIMEOUT);
	}
}

void _feed_rev_0504_seq(u32 flag)// 紙幣がなかった時、成功の停止待ち
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}

	else if(!(SENSOR_VALIDATION_OFF))
	//else if ( ex_cheat_occurred == 1 )
	{
		// モータ停止命令
		motor_ctrl_feed_stop();
		_feed_set_seq( 0x0508, FEED_SEQ_TIMEOUT);

//		_feed_set_alarm(ALARM_CODE_CHEAT);
	}

	else if( SENSOR_EXIT || SENSOR_APB_OUT || SENSOR_APB_IN )
	{
		// モータ停止命令
		motor_ctrl_feed_stop();
		_feed_set_seq( 0x0508, FEED_SEQ_TIMEOUT);

//		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if (IS_FEED_EVT_MOTOR_LOCK(flag))
	{
	/* motor lock */
		_feed_set_alarm(ALARM_CODE_FEED_MOTOR_LOCK);
	}
	else if (is_motor_ctrl_feed_stop())
	{
	/* no error <正常終了> */
		_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_REV_CHECK_BILL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		_feed_set_seq(FEED_SEQ_IDLE, 0);
	}
}

void _feed_rev_0508_seq(u32 flag)// cheatは確定、モータ停止後転 紙幣が見つかった時
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
	// モータが起動しないチートを優先する
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if (is_motor_ctrl_feed_stop())
	{
	//	エスクロ以降はモータ全開
		if (IERR_CODE_OK == motor_ctrl_feed_fwd(ex_feed_motor_speed[FEED_SPEED_APB].set_speed, ( WAIT_PB_IN_OFF + WAIT_LAST_FEED )))
		{
			_feed_set_seq( 0x050A, FEED_SEQ_TIMEOUT);
		}
	}
}


void _feed_rev_050A_seq(u32 flag)
{
	if (!(is_box_set()))
	{
		_feed_set_alarm(ALARM_CODE_BOX);
	}
	else if (IS_FEED_EVT_OVER_PULSE(flag))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
	else if (IS_FEED_EVT_TIMEOUT(flag))
	{
		_feed_set_alarm(ALARM_CODE_CHEAT);
	}
}

#endif	//end HIGH_SECURITY_MODE


/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
// 返却
/*-----------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/

void _check_position_pulse(void)
{
	memcpy(&exbk_prev_position_pulse_count, &ex_position_pulse_count, sizeof(POSITION_AREA));

	_initialize_position_pulse();
	_initialize_feed_motor_pulse();
}



//_check_prev_apb_in_on ////0x07XXの開始直前のみ使用 引き抜き対策ではない
//_check_prev_apb_out_on ////0x07XXの開始直前のみ使用 引き抜き対策ではない
//_check_prev_exit_on ////0x07XXの開始直前のみ使用 引き抜き対策ではない
//_get_rejecting_banknote_last_sensor //引き抜き対策ではない



bool _check_prev_apb_in_on(void) //0x07XXの開始直前のみ使用
{
	if((exbk_prev_position_pulse_count.apb_in.start == 0) && (exbk_prev_position_pulse_count.apb_in.end == 0))
	{
		return false;
	}
	return true;
}

bool _check_prev_apb_out_on(void) //0x07XXの開始直前のみ使用
{
	if((exbk_prev_position_pulse_count.apb_out.start == 0) && (exbk_prev_position_pulse_count.apb_out.end == 0))
	{
		return false;
	}
	return true;
}

bool _check_prev_exit_on(void) //0x07XXの開始直前のみ使用
{
	if((exbk_prev_position_pulse_count.exit.start == 0) && (exbk_prev_position_pulse_count.exit.end == 0))
	{
		return false;
	}
	return true;
}

u16 _get_rejecting_banknote_last_sensor(void) //返却リトライ 正転完了後(イニシャル動作時のみ使用)
{
	u16 last_sensor = 0;
	u16 pulse = 0;

	// return last on sensor in reject seq
	if(ex_position_pulse_count.exit.end > pulse)
	{
		last_sensor = POSI_EXIT;
		pulse = ex_position_pulse_count.exit.end;
	}
	if(ex_position_pulse_count.apb_out.end > pulse)
	{
		last_sensor = POSI_APB_OUT;
		pulse = ex_position_pulse_count.apb_out.end;
	}
	if(ex_position_pulse_count.apb_in.end > pulse)
	{
		last_sensor = POSI_APB_IN;
		pulse = ex_position_pulse_count.apb_in.end;
	}
	if(ex_position_pulse_count.centering.end > pulse)
	{
		last_sensor = POSI_CENTERING;
		pulse = ex_position_pulse_count.centering.end;
	}

	if(pulse == 0)
	{
		// return last on sensor in reject seq
		if(exbk_prev_position_pulse_count.exit.end > pulse)
		{
			last_sensor = POSI_EXIT;
			pulse = exbk_prev_position_pulse_count.exit.end;
		}
		if(exbk_prev_position_pulse_count.apb_out.end > pulse)
		{
			last_sensor = POSI_APB_OUT;
			pulse = exbk_prev_position_pulse_count.apb_out.end;
		}
		if(exbk_prev_position_pulse_count.apb_in.end > pulse)
		{
			last_sensor = POSI_APB_IN;
			pulse = exbk_prev_position_pulse_count.apb_in.end;
		}
		if(exbk_prev_position_pulse_count.centering.end > pulse)
		{
			last_sensor = POSI_CENTERING;
			pulse = exbk_prev_position_pulse_count.centering.end;
		}
	}

	return last_sensor;
}


void _feed_set_feed_reject_retry(u32 alarm_code)	// only 0x07XX
{
	u16 bill_in;
#if 1//#if defined(NEW_HANG)
	ex_reject_escrow = 0;
#endif

	bill_in = _main_bill_in();
	if (bill_in == BILL_IN_STACKER)
	{
	    switch (alarm_code)
	    {
	        case FEED_OTHER_SENSOR:
				alarm_code = ALARM_CODE_FEED_OTHER_SENSOR_SK;
		        break;
	        case FEED_SLIP:
				alarm_code = ALARM_CODE_FEED_SLIP_SK;
		        break;
	        case FEED_TIMEOUT:
				alarm_code = ALARM_CODE_FEED_TIMEOUT_SK;
		        break;
	        case FEED_MOTOR_LOCK:
				alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_SK;
		        break;
	        default:
				alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_SK;
		        break;
	    }
	}
	else
	{
	    switch (alarm_code)
	    {
	        case FEED_OTHER_SENSOR:
				alarm_code = ALARM_CODE_FEED_OTHER_SENSOR_AT;
		        break;
	        case FEED_SLIP:
				alarm_code = ALARM_CODE_FEED_SLIP_AT;
		        break;
	        case FEED_TIMEOUT:
				alarm_code = ALARM_CODE_FEED_TIMEOUT_AT;
		        break;
	        case FEED_MOTOR_LOCK:
				alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_AT;
		        break;
	        default:
				alarm_code = ALARM_CODE_FEED_MOTOR_LOCK_AT;
		        break;
	    }
	}

	s_feed_alarm_retry++;
	if(s_feed_alarm_retry > FEED_REJECT_RETRY_COUNT)
	{
	/* retry over */
		_feed_set_alarm(alarm_code);
	}
	else
	{
		/* 返却動作が上手くいかなかった時にリトライする前に	*/
		/* 幅よせがHomeにあるか確認して、Homeにない場合、	*/
		/* 一度幅よせ動作を行う								*/
		if(SENSOR_CENTERING_HOME)
		{
		/* 			*/
			s_feed_alarm_code = alarm_code;
			motor_ctrl_feed_stop();
			_feed_set_seq(0x0710, FEED_SEQ_TIMEOUT);
		}
		else
		{
		/*  Home out */
			s_feed_alarm_code = alarm_code;
			motor_ctrl_feed_stop();

			//2019-01-23 幅よせOpen処理の為、一度Mainへ通知
			_feed_send_msg(ID_MAIN_MBX, TMSG_FEED_REJECT_RSP, TMSG_SUB_INTERIM, 0, 0, 0);
			_feed_set_seq(FEED_SEQ_IDLE, 0);
		}
	}
}

/* EOF */
