/******************************************************************************/
/*! @addtogroup Group2
    @file       id003.c
    @brief      id003 process
    @date       2013/03/22
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2013/03/22 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
#include <string.h>
#include "kernel.h"
#include "itron.h"
#include "common.h"
#include "custom.h"
#include "id003.h"
#include "id003_crc.h"
#include "id003_encryption.h"
#include "id003_authentication.h"
#include "js_uart_id003.h"
#include "status_tbl.h"
#include "memorymap.h"
#include "sub_functions.h"
#include "hal_sensor.h"
#include "icb_struct.h"
#include "user/icb/icb.h"
#include "tx_alt_low_level.h"

#if defined(UBA_RTQ)
#include "if_rc.h"
#endif // UBA_RTQ

#define EXT
#include "../common/global.h"
#include "com_ram.c"

#ifdef _ENABLE_JDL
#include "jdl_conf.h"
#endif /* _ENABLE_JDL */
/************************** PRIVATE DEFINITIONS *************************/
#if defined(UBA_RTQ)
void power_recover_003_rtq(u32 arg3, u8 from);
static void _is_set_unit_id(void);
#else
void power_recover_003(u32 arg3, u8 from);
#endif

/************************** PRIVATE VARIABLES *************************/
static u8 s_id003_powerup_stat;
static u8 s_id003_powerup_stsreq_flag;
static u8 s_id003_communication_stsreq_flag;
static u8 s_id003_stacking_info;
static u8 s_id003_illegal_credit;

#if 1//#if defined(UBA_ID003_ENC)
	//#if defined(ID003_SPECK64)
	u8 _id003_sts_poll;
	u8 ex_illigal_payout_command;
	//#endif
	void _id003_encryption_key_cmd_proc(void);
	void _id003_encryption_set_number_cmd_proc(void);
#endif // UBA_ID003_ENC

static u8 s_id003_enq_resend;
/*static*/ u8 s_id003_status_wait_flag;
/*static*/ u16 s_id003_status_wait_next_mode;
static u16 s_id003_status_wait_escrow;
static u16 s_id003_status_wait_reject;
static u16 s_id003_status_wait_error;
#if !defined(UBA_RTQ)	//2023-12-04
	static u8 ex_poll_count_gli;	/* 10回ポールに対してPower upステータス後にStacker JAM */
	static u8 ex_error_003_gli;
#endif

extern bool _id003_check_opt_func(void);
static u8 s_id003_sensor_enq_status;
#if defined(UBA_RTQ)
static u8 s_id003_paying_info;
static u8 s_id003_collecting_info;
static u8 s_id003_powerup_stacker_to_box;
static u8 recycle_denomi_mask1; /* ex_line_status_tbl.recycle_denomi_mask1 *///UBA500はバックアップ領域にアサインしているが、起動時にクリアしている
static u8 recycle_denomi_mask2; /* ex_line_status_tbl.recycle_denomi_mask2 *///UBA500はバックアップ領域にアサインしているが、起動時にクリアしている

extern const u8 rcTbl_dt1[8][2];
extern const u8 rcTbl_dt2[8][2];

//u8 id003_escrow_payout;
void _id003_get_sensor_status_cmd_proc(void);
void _id003_rc_dldstatus_cmd_proc(void);
#endif // UBA_RTQ 2024/08/01

/************************** PRIVATE FUNCTIONS *************************/
void _id003_initial_msg_proc(void);

/* --------------------------------------------------------------------- */
/* ID-003 Command receiving procedure                                    */
/* --------------------------------------------------------------------- */
void _id003_cmd_proc(void);

/* Status request */
void _id003_sts_request_cmd_proc(void);
void _id003_stsreq_pwr(void);
void _id003_stsreq_pwr_at(void);
void _id003_stsreq_pwr_sk(void);
void _id003_stsreq_pwr_err(void);
void _id003_stsreq_pwr_init(void);
void _id003_stsreq_pwr_init_stack(void);
void _id003_stsreq_pwr_init_icb_recover(void);
void _id003_stsreq_pwr_init_vend(void);
void _id003_stsreq_pwr_init_vend_ack(void);
void _id003_stsreq_pwr_init_reject(void);
void _id003_stsreq_pwr_init_paus(void);
#if defined(UBA_RTQ)
void _id003_stsreq_pwr_init_payvalid(void);
void _id003_stsreq_pwr_init_payvalid_ack(void);
#endif // UBA_RTQ
void _id003_stsreq_init(void);
void _id003_stsreq_init_stack(void);
void _id003_stsreq_init_reject(void);
void _id003_stsreq_init_paus(void);
void _id003_stsreq_auto_recover(void);
void _id003_stsreq_auto_recover_stack(void);
void _id003_stsreq_auto_recover_reject(void);
void _id003_stsreq_auto_recover_paus(void);
void _id003_stsreq_disable(void);
void _id003_stsreq_enable_wait_poll(void);
void _id003_stsreq_enable(void);
void _id003_stsreq_enable_reject(void);
void _id003_stsreq_accept(void);

void _id003_stsreq_accept_wait_poll_for_escrow(void);
void _id003_stsreq_accept_wait_poll(void);
void _id003_stsreq_escrow(void);
void _id003_stsreq_escrow_wait_cmd(void);
void _id003_stsreq_hold1(void);
void _id003_stsreq_hold2(void);
void _id003_stsreq_stack(void);
void _id003_stsreq_pause(void);
void _id003_stsreq_wait_pause(void);
void _id003_stsreq_vend(void);
void _id003_stsreq_vend_ack(void);
void _id003_stsreq_vend_full(void);
void _id003_stsreq_stacked(void);
void _id003_stsreq_stacked_full(void);
//void _id003_stsreq_stack_finish(void);

void _id003_stsreq_reject_wait_accept_rsp(void);
void _id003_stsreq_reject(void);
void _id003_stsreq_reject_wait_poll(void);
void _id003_stsreq_reject_wait_note_removed(void);
void _id003_stsreq_return(void);
void _id003_stsreq_log_access(void);
void _id003_stsreq_error(void);
void _id003_sig_busy(void);
void _id003_sig_end(void);
void _id003_program_signature_proc(void);
void _id003_sha1_hash_cmd_proc(void);
void _id003_stsreq_sys_error(void);

/* Operation command */
void _id003_reset_cmd_proc(void);
void _id003_stack1_cmd_proc(void);
void _id003_stack2_cmd_proc(void);
#if defined(UBA_RTQ)
void _id003_stack3_cmd_proc(void);
#endif // UBA_RTQ
void _id003_return_cmd_proc(void);
void _id003_hold_cmd_proc(void);
void _id003_wait_cmd_proc(void);
//void _id003_bookmark_cmd_proc(void);
//void _id003_bookmark_cancel_cmd_proc(void);
void _id003_ack_cmd_proc(void);
void _id003_download_cmd_proc(void);

/* Setting command */
void _id003_enable_cmd_proc(void);
void _id003_security_cmd_proc(void);
void _id003_communication_cmd_proc(void);
void _id003_inhibit_cmd_proc(void);
void _id003_direction_cmd_proc(void);
void _id003_optional_func_cmd_proc(void);
void _id003_icb_mc_number_cmd_proc(void);
void _id003_encryption_cmd_proc(void);
void _id003_barcode_func_cmd_proc(void);
void _id003_barcode_inhibit_cmd_proc(void);

/* Setting status request */
void _id003_get_enable_cmd_proc(void);
void _id003_get_security_cmd_proc(void);
void _id003_get_comm_cmd_proc(void);
void _id003_get_inhibit_cmd_proc(void);
void _id003_get_direction_cmd_proc(void);
void _id003_get_option_cmd_proc(void);
void _id003_get_icb_mc_number_cmd_proc(void);
void _id003_get_barcode_func_cmd_proc(void);
void _id003_get_barcode_inhibit_cmd_proc(void);
void _id003_version_cmd_proc(void);
void _id003_boot_version_cmd_proc(void);
void _id003_currency_assing_cmd_proc(void);
void _id003_secret_number_cmd_proc(void);
void _id003_serial_number_cmd_proc(void);

// 追加コマンド
void _id003_icb_function_cmd_proc(void);
#if (MULTI_COUNTRY != 0)
void _id003_get_country_type_proc(void);
#endif
void _id003_get_machine_number_cmd_proc(void);
void _id003_get_box_number_cmd_proc(void);
void _id003_revision_number_cmd_proc(void);

void _id003_get_2d_barocode_cmd_proc(void);

#if defined(UBA_RTQ)
void _id003_rc_cmd_proc(void);
void _id003_stsreq_payout(void);
void _id003_stsreq_paystay_wait_poll(void);
void _id003_stsreq_payout_collected_wait_poll(void);
void _id003_stsreq_payvalid(void);
void _id003_stsreq_paystay(void);
void _id003_stsreq_collect(void);
void _id003_stsreq_collect_wait_poll(void);
void _id003_stsreq_return_to_box(void);
void _id003_stsreq_return_error(void);
void _id003_stsreq_payvalid_error(void);
void _id003_stsreq_payvalid_ack(void);
void _id003_stsreq_diagnostic(void);
void _id003_stsreq_collected_wait_poll(void);
// UNIT INFO EXTRA
void _id003_unit_info_cmd_proc(void);
void _id003_stsreq_payout_return_note(void);
/**/
static void set_recycle_total_count(u8 mode, u8 box);
#endif // UBA_RTQ

/* <<<<< Extension Command >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */
/* Response */
void _id003_send_host_ack(void);
void _id003_send_host_invalid(void);
void _id003_send_host_echoback(void);
void _id003_send_host_commu_err(void);
void _id003_send_enc_msg(u8 status, u8 escrow_code);

/* --------------------------------------------------------------------- */
/* Task Message receiving procedure                                      */
/* --------------------------------------------------------------------- */
void _id003_msg_proc(void);

/* UART1 callback message */
void _id003_callback_msg_proc(void);
void _id003_callback_receive(void);
void _id003_callback_empty(void);
void _id003_callback_alarm(void);

/* Status infomation message */
void _id003_status_info_msg_proc(void);
void _id003_status_info_mode_powerup(void);
void _id003_status_info_mode_escrow(void);
void _id003_status_info_mode_vend(void);
void _id003_status_info_mode_error(void);
void _id003_status_info_mode_def(void);

/* Reset response message */
void _id003_reset_rsp_msg_proc(void);
void _id003_reset_rsp_mode_power_init(void);
void _id003_reset_rsp_mode_init(void);
void _id003_reset_rsp_mode_recovery(void);

/* Disable response message */
void _id003_disable_rsp_msg_proc(void);

/* Enable response message */
void _id003_enable_rsp_msg_proc(void);

/* Accept response message */
#if defined(UBA_RTQ)
void _id003_accept_rsp_mode_return_to_box(void);
#endif //UBA_RTQ
void _id003_accept_rsp_msg_proc(void);
void _id003_accept_rsp_mode_accept(void);
//void _id003_accept_rsp_mode_accept_bookmark(void);
void _id003_accept_rsp_mode_rej_wait_accrsp(void);

/* Stack response message */
void _id003_stack_rsp_msg_proc(void);
void _id003_stack_rsp_mode_power_init(void);
void _id003_stack_rsp_mode_init(void);
void _id003_stack_rsp_mode_recovery(void);
void _id003_stack_rsp_mode_stack(void);
//void _id003_stack_rsp_mode_stack_bookmark(void);
void _id003_stack_rsp_mode_vend(void);
void _id003_stack_rsp_mode_stacked(void);


/* Reject response message */
void _id003_reject_rsp_msg_proc(void);
void _id003_reject_rsp_mode_power_init_rej(void);

void _id003_reject_rsp_mode_init_rej(void);

void _id003_reject_rsp_mode_recovery_rej(void);

void _id003_reject_rsp_mode_reject(void);
void _id003_reject_rsp_mode_wait_note_removed(void);
void _id003_reject_rsp_mode_disable_rej(void);

/* ICB Accept response message (recovery) */
void _id003_icb_accept_rsp_mode_power_init_icb_recover(void);

/* Times up message */
void _id003_times_up_msg_proc(void);
void _id003_timeup_dipsw_read_proc(void);
void _id003_timeup_escrow_stsreq_proc(void);
void _id003_timeup_escrow_hold1_proc(void);
void _id003_timeup_escrow_hold2_proc(void);
void _id003_timeup_enq_send_proc(void);
void _id003_timeup_sts_wait1_proc(void);
void _id003_timeup_sts_wait2_proc(void);

/* RC */
#if defined(UBA_RTQ)
void _id003_collect_rsp_msg_proc(void);
void _id003_payout_rsp_msg_proc(void);
void _id003_return_err_mode(void);
static u8 _id003_rc_enable_check(u8 unit);
static bool _is_id003_check_rc_removed_pdw(u8 unit);
static bool _is_id003_check_rc_low_battery(void);


static void clearConcurrentPayout(void);
static void check_payout_zero_note(void);
static u8 check_recieved_data(void);
static u8	check_setting_box_and_count(void);
static u8	check_status_recycler(void);
static u8	check_payout_count_recycler(void);
static u8	check_setting_denomi_recycler(void);

extern void id003_enc_cancel_update_context(void);
//#if defined(UBA_ID003_ENC)
void _id003_enc_mode_req_proc(void);
void _id003_enc_mode_cmd_proc(void);
void _id003_set_encrypted_keynumber_cmd_proc(void);

//#endif //	UBA_ID003_ENC
#endif // UBA_RTQ

static void create_secret_number(void);	//UBA_ID003_ENC
void _secret_number_003(void);	//UBA_ID003_ENC

/* --------------------------------------------------------------------- */
/* Sub functions                                                         */
/* --------------------------------------------------------------------- */
void _id003_status_wait_clear(void);
void _id003_status_wait_release(void);
void _id003_intr_mode_sub(u16 mode, u8 wait_flag);
void _id003_illegal_credit(void);

void _id003_send_host_enq(void);
//void _id003_status_tbl_reset(void);
u16 _id003_dipsw_disable(void);

void _set_id003_reject(u16 mode, u16 code);
void _set_id003_alarm(u16 mode, u16 code);

bool _is_id003_enable(void);
bool _is_id003_bar_inhibit(void);
bool _is_id003_denomi_inhibit(u32 escrow_code, u32 direction);
u8  _is_id003_escrow_data(u16 denomi_code);
u8  _is_id003_reject_data(u16 reject_code);
u8  _is_id003_error_sts(u16 alarm_code, u8 *data);
void _id003_version_cmd_proc_uba(void);//2023-06-28

void _id003_accept_evt_success_2nd(void);
void _id003_set_escrow_or_reject_2nd(u8 type);
static u8 s_id003_enq_last_status;
static u8 _id003_tx_buff_back_uba[20];
void save_send_status_003_uba(void);
void _id003_intr_mode_sub_uba(u16 mode, u8 wait_flag);

/************************** EXTERN VARIABLES *************************/
T_MSG_BASIC cline_msg;

/************************** EXTERN FUNCTIONS *************************/
extern void _cline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _cline_system_error(u8 fatal_err, u8 code);

ID003_CMD_BUFFER _id003_rx_buff;
ID003_CMD_BUFFER _id003_tx_buff;


#if 1 //2025-05-16
void log_id003(u8 type)
{
	ex_free_uba_data1[ex_free_uba_no] = type;
	ex_free_uba_data2[ex_free_uba_no] = ex_cline_status_tbl.line_task_mode;
	ex_free_uba_data3[ex_free_uba_no] = cline_msg.arg1;
	ex_free_uba_no++;
	ex_free_uba_no = ex_free_uba_no%EX_FREE_UBA_DATA;
}
#endif

#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER)
void check_recover_rfid_rtq(u8 type)
{
	//リカバリしてよいと判断されたので、FRAMにバックアップしている情報を Smrtdat_fram_bk_power にコピー
	//イニシャルの最後で Smrtdat_fram_bk_power を使用してHead側にFRAMに書き込みを行う
	//RTQ側へは、待機時に行う
	memcpy((u8 *)&Smrtdat_fram_bk_power, (u8 *)&Smrtdat_fram_bk, sizeof(Smrtdat_fram_bk));
	ex_free_uba[7] = type;
}
#endif

/*******************************
        line_task
 *******************************/
u8 id003_main(void)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	_id003_initial_msg_proc();

	while (1)
	{
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
			_id003_msg_proc();
		}
	}
}

/**
 * @brief When RC download -
 * ID003 Command receiving procedure
 */
#if defined(UBA_RTQ)
static u8 id003_main_rc_download()
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u8 rtn = 0;

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
		if (cline_msg.tmsg_code == TMSG_UART01CB_CALLBACK_INFO)
		{
			if (cline_msg.arg1 = TMSG_SUB_RECEIVE)
			{
				rtn = uart_listen_id003(&_id003_rx_buff);
				if (rtn == ID003_LISTEN_COMMAND)
				{
					_id003_rc_dldstatus_cmd_proc();
				}
			}
		}
	}
}
#endif

void _id003_initial_msg_proc(void)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u8 init_seq = 1;
#if defined(UBA_RTQ)
	u16 rc_bill_remain;
	u8 unit;
	u8 sts;
	ex_rc_download_stat = 0;	/* RC download Ready */
	ex_rc_download_flag = 0;	/* RC download mode OFF */
	u8 run_download=0;
#endif // UBA_RTQ

#if !defined(UBA_RTQ) //2023-12-04
	ex_poll_count_gli = 0;
	ex_error_003_gli = 0;
#endif

	ex_free_uba[4] = ex_recovery_info.step;	//2025-05-16
	ex_free_uba_no = 0;	//2025-05-16

	memset(&ex_free_uba_data1[0], 0, sizeof(ex_free_uba_data1[0])*EX_FREE_UBA_DATA);
	memset(&ex_free_uba_data2[0], 0, sizeof(ex_free_uba_data2[0])*EX_FREE_UBA_DATA);

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
		#if defined(UBA_RTQ)		/* '24-09-13 */
		//	if((cline_msg.tmsg_code == TMSG_LINE_UART_OPEM_REQ) && !(cline_msg.arg1 & OPERATING_MODE_TEST))
			if((cline_msg.tmsg_code == TMSG_CLINE_UART_OPEM_REQ)&& !(cline_msg.arg1 & OPERATING_MODE_TEST))
			{
				/* Initialize UART for only RC download *///2025-02-19
				uart_init_id003();
				/* wakeup ID_UART01_CB_TASK */
				act_tsk(ID_UART01_CB_TASK);
				do
				{
					id003_main_rc_download();
				}while(ex_rc_download_flag != 0);	//2025-02-19
				/* Close peripheral Uart */
			//タスク削除とタスク終了はリスクもありそうなので使用しない
			//代わりに通信割り込み禁止にする
			//	interface_DeInit_id003();
				/* Terminate task Uart (ID_UART01_CB_TASK) */
			//	ter_tsk(ID_UART01_CB_TASK);
				/* Terminate task Cline */
			//	ext_tsk();

				run_download = 1;
				OSW_ISR_disable( OSW_INT_UART1_IRQ );	
			}
			else if((cline_msg.tmsg_code == TMSG_CLINE_INITIAL_RSP) && !(cline_msg.arg1 & OPERATING_MODE_TEST))
		#else
			//SS
				if ((cline_msg.tmsg_code == TMSG_CLINE_INITIAL_RSP)
				 && !(cline_msg.arg1 & OPERATING_MODE_TEST))
		#endif
				 {
					if (cline_msg.arg2 == TMSG_SUB_SUCCESS)
					{
					/* Set Powerup status */
						if (cline_msg.arg3 == BILL_IN_ACCEPTOR)
						{
							#if defined(UBA_RTQ)
							if ((ex_recovery_info.step == RECOVERY_STEP_PAYOUT_POS1) 	||
								(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_ESCROW) 	||
								(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)  	||
								(ex_recovery_info.step == RECOVERY_STEP_EMRGENCY_TRANSPORT) ||
								//#if defined(UBA_RS)
								(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW) ||
								(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_POS7) ||
								//#endif
								(ex_recovery_info.step == RECOVERY_STEP_SWITCHBACK_TRANSPORT))
							{
								s_id003_powerup_stat = POWERUP_STAT_RECOVER_PAYOUT;

								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
								init_seq = 0;	/* 処理を抜ける	*/
	ex_free_uba[5] = 1;	//2025-05-16 ok
							}
							else		// other cases
							{
								//2024-03-28
								/* Powerup with bill in acceptor */
								s_id003_powerup_stat = POWERUP_STAT_REJECT;

								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
								init_seq = 0;	/* 処理を抜ける	*/
	ex_free_uba[5] = 2;	//2025-05-16 ok
							}
							#else
							//2024-03-28
							/* Powerup with bill in acceptor */
							s_id003_powerup_stat = POWERUP_STAT_REJECT;

							ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
							init_seq = 0;	/* 処理を抜ける	*/
							#endif // uBA_RTQ
						}
						else if (cline_msg.arg3 == BILL_IN_STACKER)
						{
						#if defined(UBA_RTQ)
							//2025-08-01
							//UBA500 関数呼んでいるが、このケースでは戻り値、引数共になにもしていない　コメントにする
							//rc_bill_remain = get_rc_recovery_status(&unit, &sts, 0);//ここでは rc_bill_remain, &unit, &sts 未使用

							switch (ex_recovery_info.step)
							{
							case RECOVERY_STEP_ESCORW:
							case RECOVERY_STEP_STACKING:
							case RECOVERY_STEP_APB_IN:
							case RECOVERY_STEP_APB_OUT:
							case RECOVERY_STEP_VEND:
							case RECOVERY_STEP_STACK_TRANSPORT:
							case RECOVERY_STEP_STACK_DRUM:
								/* Vend Valid送信する。 */
								s_id003_powerup_stat = POWERUP_STAT_RECOVER_FEED_STACK;
								break;
							case RECOVERY_STEP_NON:
								/* Vend Valid送信しない、かつCountもしない */
								s_id003_powerup_stat = POWERUP_STAT_RECOVER_NO_COUNT;
								break;
							case RECOVERY_STEP_EMRGENCY_TRANSPORT:
							case RECOVERY_STEP_SWITCHBACK_TRANSPORT:
							case RECOVERY_STEP_PAYOUT_DRUM:
							case RECOVERY_STEP_PAYOUT_TRANSPORT:
							case RECOVERY_STEP_PAYOUT_POS1:
							case RECOVERY_STEP_PAYOUT_ESCROW:
							case RECOVERY_STEP_COLLECT_DRUM:
							case RECOVERY_STEP_COLLECT_TRANSPORT:
							case RECOVERY_STEP_COLLECT_STACKING:
							//#if defined(UBA_RS)
							case RECOVERY_STEP_PAYOUT_RS_POS7:
							case RECOVERY_STEP_PAYOUT_RS_ESCROW:
							//#endif 
								/* Vend Valid送信しない。 */
								s_id003_powerup_stat = POWERUP_STAT_FEED_STACK;
								break;
							case RECOVERY_STEP_PAYOUT_VALID:
								/* Pay Valid送信する */
								/* (出金した紙幣とは別の紙幣として扱う) */
								s_id003_powerup_stat = POWERUP_STAT_RECOVER_PAYOUT;
								break;
							default:
								break;
							}
							s_id003_powerup_stacker_to_box = RC_CASH_BOX;

							ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
							init_seq = 0;	/* 処理を抜ける	*/
	ex_free_uba[5] = 3;	//2025-05-16 ok
							#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER)	//2025-07-23
							if(ex_recovery_info.step != RECOVERY_STEP_NON)
							{
								check_recover_rfid_rtq(1);
							}
							#endif
						#else
						/* ポジションセンサでのStacker判断はUBA500と同じだが*/
						/* Escrow位置が異なる為、	*/
						/* UBA500ではPB IN,OUTの両方で紙幣検知できているが*/
						/* UBA700ではPB OUTのみで紙幣検知となる*/		
						/* Powerup with bill in stacker */

							//UBA500はcheck_last_sequence_id003();を使用しているが、内容はほぼ同じ
							if ((ex_recovery_info.step >= RECOVERY_STEP_ESCORW)
							 && (ex_recovery_info.step <= RECOVERY_STEP_VEND))
							{
							/* Stackコマンド受信後～*/
								s_id003_powerup_stat = POWERUP_STAT_RECOVER_FEED_STACK;
								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
								init_seq = 0;
							}
							else
							{
							/* Vend送る事はない*/
							/* ex_recovery_info.stepが RECOVERY_STEP_ESCORW～RECOVERY_STEP_VENDでない為、Vendする事はない*/
							/* UBA500はこのまま、紙幣を探さずに、Stackerとしているが*/
							/* UBA500,700のEscrow位置の違いにより、UBA700の場合、クリアウインドによってはPB OUTがOFFでEXITのみONとなる場合がある*/
							/* 結果、カナダなどのポリマ紙幣の場合Escrow送信前なのに、Stackerとなり、Resetで収納してしまう */
							/* ここで紙幣を探して停止させる条件はExitではなく、PB OUT検知にして、StackerではなくAcceptorにするべき */

							/* 紙幣位置的にはStackerとしたいが、ログ的には、Acceptorとしたいので、*/
							/* PB OUTまで紙幣を戻してAcceptorにしたい*/

							/* Search bill (RECOVERY_STEP_NON) */
								init_seq++;
								#if defined(UBA_RTQ)
								_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RC_SEARCH_BILL_REQ, RC_TWIN_DRUM1, FEED_SEARCH_OUT, 0, 0);
								#else
								_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SEARCH_BILL_REQ, SEARCH_TYPE_WINDOW, 0, 0, 0); //電源ON時に札探す前に、stackerで検知している時 PB OUTまで紙幣を戻してAcceptorにしたい
								#endif 
								break;
							}
						#endif // UBA_RTQ
						}
						#if defined(UBA_RTQ)
						else if ((cline_msg.arg3 == BILL_IN_ENTRANCE) &&
								 ((ex_recovery_info.step == RECOVERY_STEP_PAYOUT_ESCROW)
								 || (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW)
								 || (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)
								))
						{
							/* Powerup with bill in acceptor */
                            s_id003_powerup_stat = POWERUP_STAT_RECOVER_PAYOUT;

							ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
							init_seq = 0;	/* 処理を抜ける	*/
	ex_free_uba[5] = 4;	//2025-05-16 ok
						}
						#endif // UBA_RTQ
						/* 紙幣無し*/
						else
						{
						#if defined(UBA_RTQ)
							switch (ex_recovery_info.step)
							{
							case RECOVERY_STEP_VEND:
								s_id003_powerup_stacker_to_box = ex_recovery_info.unit;
								/* Powerup with bill in stacker */
								s_id003_powerup_stat = POWERUP_STAT_RECOVER_STACK;
								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;

								_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
								init_seq = 0;
	ex_free_uba[5] = 5;	//2025-05-16 ok
								#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER)	//2025-07-23
								check_recover_rfid_rtq(2);
								#endif

								break;
							case RECOVERY_STEP_PAYOUT_VALID:
							case RECOVERY_STEP_PAYOUT_ESCROW:
							//#if defined(UBA_RS)
							case RECOVERY_STEP_PAYOUT_RS_ESCROW:
							//#endif 
								/* Powerup with bill in acceptor */
								s_id003_powerup_stat = POWERUP_STAT_RECOVER_PAYOUT;
								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;

								_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
								init_seq = 0;
	ex_free_uba[5] = 6;	//2025-05-16 ok
								break;
							case RECOVERY_STEP_ESCORW:
							case RECOVERY_STEP_APB_IN:
							case RECOVERY_STEP_APB_OUT:
							case RECOVERY_STEP_EMRGENCY_TRANSPORT:
							case RECOVERY_STEP_SWITCHBACK_TRANSPORT:
							case RECOVERY_STEP_PAYOUT_TRANSPORT:
							case RECOVERY_STEP_PAYOUT_POS1:
							//#if defined(UBA_RS)
							case RECOVERY_STEP_PAYOUT_RS_POS7:
							//#endif 
							case RECOVERY_STEP_STACKING:
							case RECOVERY_STEP_STACK_TRANSPORT:
								/* Search bill */
								init_seq++;
								_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RC_SEARCH_BILL_REQ, ex_recovery_info.unit, FEED_SEARCH_OUT, 0, 0);
	ex_free_uba[5] = 7;	//2025-05-16 ok
								break;
							case RECOVERY_STEP_COLLECT_STACKING:
							case RECOVERY_STEP_COLLECT_TRANSPORT:
								/* Search bill */
								init_seq++;
								_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RC_SEARCH_BILL_REQ, RC_CASH_BOX, FEED_SEARCH_OUT, 0, 0);
	ex_free_uba[5] = 8;	//2025-05-16 ok
								break;
							case RECOVERY_STEP_STACK_DRUM:
							case RECOVERY_STEP_PAYOUT_DRUM:
							case RECOVERY_STEP_COLLECT_DRUM:
								/* Search bill */
								init_seq++;
								_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RC_SEARCH_BILL_REQ, ex_recovery_info.unit, FEED_SEARCH_IN, 0, 0);
	ex_free_uba[5] = 9;	//2025-05-16 ok
								break;
							default:
								/* Powerup */
								s_id003_powerup_stat = POWERUP_STAT_NORMAL;
								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP;

								//_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
								init_seq = 0;
	ex_free_uba[5] = 10;	//2025-05-16
								break;
							}
						#else
						/* 紙幣ない */

                            if( ex_recovery_info.step == RECOVERY_STEP_VEND
							||
							(ex_recovery_info.step == RECOVERY_STEP_ICB_ACCEPT) /* 2024-04-02 */
							||
                        #if defined(UBAPRO_LD)
                            (ex_recovery_info.step == RECOVERY_STEP_STACKING)
                        #else
							( (ex_recovery_info.step == RECOVERY_STEP_STACKING) && ( 0 == SENSOR_STACKER_HOME ) )
                        #endif
							|| (ex_recovery_info.step == RECOVERY_STEP_STACKING_BILL_IN_BOX)
							 )
                            {
                                /* Powerup with bill in stacker */
                                s_id003_powerup_stat = POWERUP_STAT_RECOVER_STACK;
								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
                                init_seq = 0; /* 処理を抜ける */
                            }
                            else if( ex_recovery_info.step >= RECOVERY_STEP_ESCORW )
                            {
                            /* Stackerにしたいので、紙幣を探す*/
							/* 紙幣をExitまで戻す*/
                                /* Search bill */
                                init_seq++;
								#if defined(UBA_RTQ)
								_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RC_SEARCH_BILL_REQ, RC_CASH_BOX, FEED_SEARCH_OUT, 0, 0);
                                #else
								_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SEARCH_BILL_REQ, 0, 0, 0, 0); //電源ON時札がない時、stackコマンド受信以降 紙幣をExitまで戻す
								#endif 
							}
                            else
                            {
								/* ここで紙幣を探すと、次の処理でエラーにするか含めて判断する必要がでる*/
								/* UBA500もEscrow位置から紙幣が無くなった場合も特に処理していないので、同様にする*/	                        
                                /* Powerup */
								s_id003_powerup_stat = POWERUP_STAT_NORMAL;
								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP;
								init_seq = 0; /* 処理を抜ける */
                            }
						#endif // UBA_RTQ
						}
					}
					else if (cline_msg.arg2 == TMSG_SUB_ALARM)
					{
						if(cline_msg.arg3 == ALARM_CODE_FRAM )
						{
							/* Set Error status */
							s_id003_powerup_stat = 0;
							ex_cline_status_tbl.line_task_mode = ID003_MODE_SYSTEM_ERROR;
							ex_cline_status_tbl.error_code = cline_msg.arg3;
							init_seq = 0;
						}
						else
						{
							/* 起動時にすでにBOX OPEN*/
							/* Set Error status */
							s_id003_powerup_stat = 0;
							ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_ERROR;
							ex_cline_status_tbl.error_code = cline_msg.arg3;
							init_seq = 0;
						}
					}
					else
					{
						/* system error ? */
						_cline_system_error(0, 5);
					}
				}
				break;
			case 2:
			#if defined(UBA_RTQ)
				if (cline_msg.tmsg_code == TMSG_CLINE_RC_SEARCH_BILL_RSP)
			#else
				if (cline_msg.tmsg_code == TMSG_CLINE_SEARCH_BILL_RSP)
			#endif // UBA_RTQ				
				{
#if defined(UBA_RTQ)
					if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
					{
						if (cline_msg.arg2 == BILL_IN_ACCEPTOR)
						{
							if( (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_POS1)         ||
								(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_ESCROW)       ||
								(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)        ||
								(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW)    ||
								(ex_recovery_info.step == RECOVERY_STEP_EMRGENCY_TRANSPORT)  ||
								(ex_recovery_info.step == RECOVERY_STEP_SWITCHBACK_TRANSPORT) )
							{
								s_id003_powerup_stat = POWERUP_STAT_RECOVER_PAYOUT;
								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
								init_seq = 0;	/* 処理を抜ける	*/
	ex_free_uba[6] = 1;	//2025-05-16 ok
							}
							else
							{
								s_id003_powerup_stat = POWERUP_STAT_REJECT; 
								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
								init_seq = 0;	/* 処理を抜ける	*/
	ex_free_uba[6] = 2;	//2025-05-16 ok
							}
						}
						else if (cline_msg.arg2 == BILL_IN_STACKER)
						{
							//2025-08-01
							//UBA500 関数呼んでいるが、このケースでは戻り値、引数共になにもしていない　コメントにする
							//rc_bill_remain = get_rc_recovery_status(&unit, &sts, 0); //ここでは rc_bill_remain, &unit, &sts 未使用
							
							switch( ex_recovery_info.step )
							{
								case RECOVERY_STEP_ESCORW:
								case RECOVERY_STEP_STACKING:
								case RECOVERY_STEP_APB_IN:
								case RECOVERY_STEP_APB_OUT:
								case RECOVERY_STEP_VEND:
								case RECOVERY_STEP_STACK_TRANSPORT:
								case RECOVERY_STEP_STACK_DRUM:
									/* Vend Valid送信する。 */
									s_id003_powerup_stat = POWERUP_STAT_RECOVER_FEED_STACK;
	ex_free_uba[6] = 3;	//2025-05-16 ok
									break;
								case RECOVERY_STEP_NON:
									/* Vend Valid送信しない、かつCountもしない */
									s_id003_powerup_stat = POWERUP_STAT_RECOVER_NO_COUNT;
	ex_free_uba[6] = 4;	//2025-05-16 ok
									break;
								case RECOVERY_STEP_EMRGENCY_TRANSPORT:
								case RECOVERY_STEP_SWITCHBACK_TRANSPORT:
								case RECOVERY_STEP_PAYOUT_DRUM:
								case RECOVERY_STEP_PAYOUT_TRANSPORT:
								case RECOVERY_STEP_PAYOUT_POS1:
								case RECOVERY_STEP_PAYOUT_ESCROW:
								case RECOVERY_STEP_PAYOUT_RS_POS7:
								case RECOVERY_STEP_COLLECT_DRUM:
								case RECOVERY_STEP_COLLECT_TRANSPORT:
								case RECOVERY_STEP_COLLECT_STACKING:
									/* Vend Valid送信しない。 */
									s_id003_powerup_stat = POWERUP_STAT_FEED_STACK;
	ex_free_uba[6] = 5;	//2025-05-16 ok
									break;
								case RECOVERY_STEP_PAYOUT_VALID:
									/* Pay Valid送信する */
									/* (出金した紙幣とは別の紙幣として扱う) */
									s_id003_powerup_stat = POWERUP_STAT_RECOVER_PAYOUT;
	ex_free_uba[6] = 6;	//2025-05-16 ok
									break;
							}
							s_id003_powerup_stacker_to_box = RC_CASH_BOX;
							ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
							#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER)	//2025-07-23
							if(ex_recovery_info.step != RECOVERY_STEP_NON)
							{
								check_recover_rfid_rtq(3);
							}
							#endif
						}
						else
						{
							//ここのみcheatの可能性がある
							//UBA500でも第3引数を設定しているが、実際使用されていないのでUBA700では0にする
							rc_bill_remain = get_rc_recovery_status(&unit, &sts, 0);  //ここでは rc_bill_remain 使用 &unit,未使用　stsはcheatを出す為に使用している。unitは使用していない
							if( ex_recovery_info.step == RECOVERY_STEP_STACKING )
							{
							//回収庫の押しこみ中 ここの来る
								{								
									s_id003_powerup_stat = POWERUP_STAT_RECOVER_FEED_STACK;	//UBA700ではこれに置き換え
									// check 収納開始しているので、こっちの方がいいのでは s_id003_powerup_stat = POWERUP_STAT_RECOVER_STACK;
									ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
								}
	ex_free_uba[6] = 7;	//2025-05-16 ok
							#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER)	//2025-07-23
								check_recover_rfid_rtq(4);
							#endif

							}
							/* Stack要求後かつStacker Homeでない場合は押し込み中 */
							/* なので紙幣見つからない */
							else if( ex_recovery_info.step == RECOVERY_STEP_COLLECT_STACKING )
							{
								/* Vend Valid送信しない。 */
								s_id003_powerup_stat = POWERUP_STAT_FEED_STACK;
								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
	ex_free_uba[6] = 8;	//2025-05-16 ok
							#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER)	//2025-07-23
								check_recover_rfid_rtq(5);
							#endif

							}
							else if( rc_bill_remain == BILL_IN_RC )
							{
								switch( ex_recovery_info.step )
								{
									case RECOVERY_STEP_STACK_DRUM:
										/* Vend Valid送信する。 */
										s_id003_powerup_stat = POWERUP_STAT_RECOVER_FEED_STACK;
										s_id003_powerup_stacker_to_box = ex_recovery_info.unit;
	ex_free_uba[6] = 9;	//2025-05-16 ok
										break;
									default:
										/* Powerup with bill in stacker */
										s_id003_powerup_stat = POWERUP_STAT_FEED_STACK;
	ex_free_uba[6] = 10;	//2025-05-16 ok
										break;
								}
								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
								init_seq = 0;	/* 処理を抜ける	*/
							#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER)	//2025-07-23
								check_recover_rfid_rtq(6);
							#endif
							}
							else if( sts == BILL_CHEAT )
							{
								s_id003_powerup_stat = POWERUP_STAT_RECOVER_CHEAT;
								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP;
	ex_free_uba[6] = 11;	//2025-05-16 ok
							}
							else
							{
								/* Powerup */
							//2025-03-31 UBA500では設定しているがこの設定意味ない	s_id003_powerup_stat = POWERUP_STAT_RECOVER_SEARCH_NON;
							//この設定を使用するとUBA700の場合、明確にGLIエラーにしているので、変更
								s_id003_powerup_stat = 0;							
								ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP;
	ex_free_uba[6] = 12;	//2025-05-16 ok  UBA500RTQはPOWERUP_STAT_RECOVER_SEARCH_NONでGLIエラー用にしているが、
							//本来RTQは通常のPower upでいいのでは？
							//RTQでの POWERUP_STAT_RECOVER_SEARCH_NON の使い方を確認する
							}
						}
						init_seq = 0;	/* 処理を抜ける	*/
					}
					else if (cline_msg.arg1 == TMSG_SUB_ALARM)
					{
						//紙幣探し中のBOX Open
						/* Set Error status */
						s_id003_powerup_stat = 0;
						ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_ERROR;
						ex_cline_status_tbl.error_code = cline_msg.arg2;
						init_seq = 0;
	ex_free_uba[6] = 13;	//2025-05-16 ok
					}
					else
					{
						/* system error ? */
						_cline_system_error(0, 6);
					}
#else
					if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
					{
						power_recover_003(cline_msg.arg2, 2);
						init_seq = 0;
					}
					else if (cline_msg.arg1 == TMSG_SUB_ALARM)
					{
					//紙幣探し中のBOX Open
					/* Set Error status */
						s_id003_powerup_stat = 0;
						ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_ERROR;
						ex_cline_status_tbl.error_code = cline_msg.arg2;
						init_seq = 0;
					}
					else
					{
						/* system error ? */
						_cline_system_error(0, 6);
					}
#endif // UBA_RTQ
				}
				break;
			default:
				/* system error ? */
				_cline_system_error(0, 7);
				break;
			}
		}
	}

	/* --------------------------------------------------------------------- */
    /* COMMNUCATION LOG INIT                                                 */
    /* --------------------------------------------------------------------- */
	//2025-10-24 これより前に配置するとJDLの初期設定前なので、s_jdl_rtn が不定の値の為
	//下記のID設定などが行われない。
#ifdef _ENABLE_JDL
    jdl_comm_init(PROTOCOL_SELECT_ID003, 1, ID003_CMD_STS_REQUEST);
#endif /* _ENABLE_JDL */

	/* --------------------------------------------------------------------- */
	/* Initialize line status table                                          */
	/* --------------------------------------------------------------------- */

#if defined(UBA_RTQ)
	recycle_denomi_mask1 = 0;
	recycle_denomi_mask2 = 0;
	for(u8 count = 0; count < 8; count++)
	{
		recycle_denomi_mask1 |= rcTbl_dt1[count][0];
		recycle_denomi_mask2 |= rcTbl_dt2[count][0];
	}
#endif // UBA_RTQ
	/* Set bill disable mask */
	ex_cline_status_tbl.bill_disable = 0;
	//ex_cline_status_tbl.escrow_code = 0x0000;
	ex_cline_status_tbl.reject_code = 0x0000;
	if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_SYSTEM_ERROR)
	){
		if (ex_recovery_info.step != RECOVERY_STEP_NON)
		{
			set_recovery_step(RECOVERY_STEP_NON);
		}
	}
	else
	{
		ex_cline_status_tbl.error_code = 0x0000;
	}
	ex_cline_status_tbl.security_level = 0x0000;
	if ((ex_cline_status_tbl.comm_mode != 0x0000)
	 && (ex_cline_status_tbl.comm_mode != 0x0001)
	 && (ex_cline_status_tbl.comm_mode != 0x0002))
	{
		ex_cline_status_tbl.comm_mode = 0x0000;
	}
	ex_cline_status_tbl.accept_disable = 0x0001;
	ex_cline_status_tbl.direction_disable = 0x0000;
	ex_cline_status_tbl.option = 0x0000;
	ex_cline_status_tbl.log_access_mode = 0x00;
	ex_cline_status_tbl.log_access_status = 0x00;
	ex_cline_status_tbl.store_task_mode = 0x0000;
	//_id003_status_tbl_reset();
	/* Start DipSW reading interval */
	ex_cline_status_tbl.dipsw_disable = _id003_dipsw_disable();
    ex_cline_status_tbl.barcode_type = 0;
    ex_cline_status_tbl.barcode_length = 0;
    ex_cline_status_tbl.barcode_inhibit = 0xFE;
	_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DIPSW_READ, WAIT_TIME_DIPSW_READ, 0, 0);

	/* write status table */
	write_status_table();


	/* --------------------------------------------------------------------- */
	/* Initialize id-003 encryption                                          */
	/* --------------------------------------------------------------------- */

	/* initialize encryption */
	id003_enc_init();

	/* --------------------------------------------------------------------- */
	/* Start id-003 communication                                            */
	/* --------------------------------------------------------------------- */
	/* Status request wait flag (power up) */
	s_id003_powerup_stsreq_flag = 1;
	s_id003_communication_stsreq_flag = 0;

	s_id003_stacking_info = 0;
	s_id003_illegal_credit = 0;

	/* Initialize Wait Variables */
	s_id003_status_wait_flag = 0;
	s_id003_status_wait_next_mode = 0;
	s_id003_status_wait_escrow = 0;
	s_id003_status_wait_reject = 0;
	s_id003_status_wait_error = 0;
	//s_id003_next_cline_mode = 0;
	s_id003_sensor_enq_status = 0;	//2024-05-13
#if defined (UBA_RTQ)
	ex_main_emergency_flag = 0;
	ex_rc_data_lock = 0;	
#endif // uBA_RTQ

//#if defined(UBA_ID003_ENC)
	update_random_seed(*(unsigned int *)&ex_adjustment_data.maintenance_info.serial_no[8]); //2025-03-28
//#endif

#if defined(UBA_RTQ)	//#if defined(ID003_SPECK64)
	ex_illigal_payout_command = 0;
#endif 	//

#if defined(UBA_RTQ) //2025-06-04
	/* Initialize UART */
	if(run_download == 1)
	{
	//RTQダウンロードを行った場合、すでに通信設定は完了しているので、
	//割り込みを有効にする
		_uart_fifo_clear_id003();
	}
	else
	{
		uart_init_id003();
	/* wakeup ID_UART01_CB_TASK */
		act_tsk(ID_UART01_CB_TASK);
	}
#else
	uart_init_id003();
	/* wakeup ID_UART01_CB_TASK */
	act_tsk(ID_UART01_CB_TASK);
#endif

	if ((ex_cline_status_tbl.comm_mode == 0x0001)
	 || (ex_cline_status_tbl.comm_mode == 0x0002))
	{
	/* [Interrupt Mode 1/2] Send ENQ */
		if (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR)
		{
			s_id003_enq_resend = 2;
		}
		else
		{
			s_id003_enq_resend = 1;
		}
		_id003_send_host_enq();
		_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ENQ_SEND, WAIT_TIME_RESEND_ENQ, 0, 0);
	}
	else
	{
	 /* [Polling Mode] */
		s_id003_enq_resend = 0;
	}

#if defined(_PROTOCOL_ENABLE_SUBLINE)
	act_tsk(ID_SUBLINE_TASK);
#endif /* _PROTOCOL_ENABLE_SUBLINE */
}


/*********************************************************************//**
 * @brief Command receiving procedure
 * @param[in]	_id003_rx_buff.cmd : command code
 * @return 		None
 **********************************************************************/
void _id003_cmd_proc(void)
{
#if defined(UBA_RTQ) //#if defined(ID003_SPECK64) #if defined(UBA_ID003_ENC)
	if(id003_enc_mode == PAYOUT16MODE_SPECK)
	{
		if((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP
		||	ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_AT
		||	ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_SK
		||  ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR)
		&& _id003_sts_poll == 1)
		{
			_id003_sts_poll = 0;
			id003_enc_update_context();
		}
	}
#endif // UBA_ID003_ENC

#ifdef _ENABLE_JDL
    jdl_comm_rx_pkt(&_id003_rx_buff.cmd, _id003_rx_buff.length-4);
#endif /* _ENABLE_JDL */
	if ((s_id003_illegal_credit == 0)
	 || ((_id003_rx_buff.cmd == ID003_CMD_RESET) && (_id003_rx_buff.length == 0x01 + 0x04)))
	{
		if (ex_cline_status_tbl.line_task_mode == ID003_MODE_LOG_ACCESS)
		{
			switch (_id003_rx_buff.cmd)
			{
			case ID003_CMD_STS_REQUEST:						/* Status Request (Poll) */
				_id003_sts_request_cmd_proc();
				break;
			case ID003_CMD_RESET:							/* Reset Command */
				_id003_reset_cmd_proc();
				break;
			default:
				_id003_send_host_invalid();
				break;
			}
		}
		else
		{
			switch (_id003_rx_buff.cmd)
			{
			case ID003_CMD_STS_REQUEST:						/* Status Request (Poll) */
				_id003_sts_request_cmd_proc();
				break;
			case ID003_CMD_RESET:							/* Reset Command */
				_id003_reset_cmd_proc();//ok
				break;
			case ID003_CMD_STACK1:							/* Stack-1 Command */
				_id003_stack1_cmd_proc();//ok
				break;
			case ID003_CMD_STACK2:							/* Stack-2 Command */
				_id003_stack2_cmd_proc();//ok
				break;
		#if defined(UBA_RTQ)
			case ID003_CMD_STACK3:
				_id003_stack3_cmd_proc();//ok
				break;
		#endif		
			case ID003_CMD_RETURN:							/* Reterun Command */
				_id003_return_cmd_proc();//ok
				break;
			case ID003_CMD_HOLD:							/* Hold Command */
				_id003_hold_cmd_proc();//ok
				break;
			case ID003_CMD_WAIT:							/* Wait Command */
				_id003_wait_cmd_proc();//ok
				break;
			//case ID003_CMD_BOOKMARK:						/* Book Mark Command */
			//	_id003_bookmark_cmd_proc();
			//	break;
			//case ID003_CMD_BOOKMARK_CANCEL:					/* Book Mark Cancel Command */
			//	_id003_bookmark_cancel_cmd_proc();
			//	break;
			case ID003_CMD_ACK:								/* ACK */
				_id003_ack_cmd_proc();
				break;
			case ID003_CMD_DOWNLOAD_REQUEST:				/* Download Request */
			case ID003_CMD_DIFFERENTIAL_DOWNLOAD:			/* 差分Download Request */
				_id003_download_cmd_proc();
				break;
			case ID003_CMD_ENABLE:							/* Enable/Disable Command */
				_id003_enable_cmd_proc();//ok
				break;
			case ID003_CMD_SECURITY:						/* Security Command */
				_id003_security_cmd_proc();//ok
				break;
			case ID003_CMD_COMMUNICATION:					/* Communication Command */
				_id003_communication_cmd_proc();//ok
				break;
			case ID003_CMD_INHIBIT:							/* Inhibit Command */
				_id003_inhibit_cmd_proc();//ok
				break;
			case ID003_CMD_DIRECTION:						/* Direction Command */
				_id003_direction_cmd_proc();//ok
				break;
			case ID003_CMD_OPTIONAL_FUNC:					/* Optional Funcktion Command */
				_id003_optional_func_cmd_proc();//ok
				break;
			case ID003_CMD_SET_ICB_MC:						/*	IT BOX用 M/C No from HOST */
				_id003_icb_mc_number_cmd_proc();//ok
				break;
			case ID003_CMD_DLE:								/* Encryption */
				_id003_encryption_cmd_proc(); //ok
				break;
			case ID003_CMD_BARCODE_FUNC:					/* Barcode Funcktion Command */
				_id003_barcode_func_cmd_proc();//ok
				break;
			case ID003_CMD_BARCODE_INHIBIT:					/* Barcode Inhibit Command */
				_id003_barcode_inhibit_cmd_proc();//ok
				break;
			case ID003_CMD_GET_ENABLE:						/* Get Enable Command */
				_id003_get_enable_cmd_proc();//ok
				break;
			case ID003_CMD_GET_SECURITY:					/* Get security Command */
				_id003_get_security_cmd_proc();//ok
				break;
			case ID003_CMD_GET_COMMUNICATION:				/* Get Communication Command */
				_id003_get_comm_cmd_proc();//ok
				break;
			case ID003_CMD_GET_INHIBIT:						/* Get Inhibit Command */
				_id003_get_inhibit_cmd_proc();//ok
				break;
			case ID003_CMD_GET_DIRECTION:					/* Get Direction Command */
				_id003_get_direction_cmd_proc();//ok
				break;
			case ID003_CMD_GET_OPTIONAL_FUNC:				/* Get Optional Funcktion Command */
				_id003_get_option_cmd_proc();//ok
				break;
			case ID003_CMD_GET_BARCODE_FUNC:				/* Get Barcode Funcktion Command */
				_id003_get_barcode_func_cmd_proc();//ok
				break;
			case ID003_CMD_GET_BARCODE_INHIBIT:				/* Get Barcode Inhibit Command */
				_id003_get_barcode_inhibit_cmd_proc();//ok
				break;
			case ID003_CMD_GET_VERSION:						/* Version Request */
				_id003_version_cmd_proc();//ok
				break;
			case ID003_CMD_GET_BOOT_VERSION:				/* Boot Version Request */
				_id003_boot_version_cmd_proc();//ok
				break;
			case ID003_CMD_GET_CURRENCY_ASSING:				/* Currency Assign Request */
				_id003_currency_assing_cmd_proc();//ok
				break;
			case ID003_CMD_GET_SECRET_NUMBER:				/* Secret Number Request */
				_id003_secret_number_cmd_proc();//ok
				break;
			case ID003_CMD_GET_SERIAL_NUMBER:
				_id003_serial_number_cmd_proc();//ok
				break;
			#if defined(UBA_RTQ)
				case ID003_CMD_GET_SENSOR_STATUS:
				_id003_get_sensor_status_cmd_proc(); //ok
				break;
			#endif	
			case ID003_CMD_PROGRAM_SIGNATURE:
				_id003_program_signature_proc();//ok
				break;
			case ID003_CMD_GET_SHA1_HASH:
				_id003_sha1_hash_cmd_proc();//ok
				break;
			case ID003_CMD_ICB_FUNCTION:
				_id003_icb_function_cmd_proc();//ok
				break;
#if (MULTI_COUNTRY != 0)
			case ID003_CMD_GET_COUNTRY_TYPE:
				_id003_get_country_type_proc();
				break;
#endif
			case ID003_CMD_GET_MACHINE_NUMBER:
				_id003_get_machine_number_cmd_proc();//ok
				break;
			case ID003_CMD_GET_BOX_NUMBER:
				_id003_get_box_number_cmd_proc();//ok
				break;
			case ID003_CMD_GET_REVISION_NUMBER:
				_id003_revision_number_cmd_proc();//ok
				break;
			#if 0
			case ID003_CMD_AUTHENTICATION:
				_id003_authentication_cmd_proc();
				break;
			#endif
		#ifdef TWO_D_EXTERNED_BARCODE_TICKET
			// Commands not currently used
			//case ID003_CMD_GET_2D_BARCODE_SETTING:
			//	_id003_get_2d_barcode_setting_cmd_proc();
			//	break;
			case ID003_CMD_RXD_2D_BARCODE:
				_id003_get_2d_barocode_cmd_proc();
				break;
		#endif
		#if defined(UBA_RTQ)	
			case ID003_CMD_EXT_RC:
				{
					if (_id003_rx_buff.data[0] == ID003_CMD_UNIT_RC)					
					{
						_id003_rc_cmd_proc();
					}
					else
					{
						_id003_send_host_invalid();
					}
				}
				break;
			case ID003_CMD_GET_UNIT_INFO:
				_id003_unit_info_cmd_proc(); //ok
				break;
			/* externsion cmd */
			case ID003_CMD_EXTENSION:
				
				break;
			#if defined(UBA_RTQ)	//拡張暗号化はRTQのみ #if defined(UBA_ID003_ENC)
			case ID003_CMD_SET_ENC_MODE:
				_id003_enc_mode_cmd_proc();	//ok SPECKとの共用
				break;
			case ID003_CMD_GET_ENC_MODE:
				_id003_enc_mode_req_proc();	//ok SPECKとの共用
				break;
			#endif // UBA_ID003_ENC
		#endif // UBA_RTQ
			case ID003_CMD_LOGDATA_INITIALIZE:
			case ID003_CMD_LOGDATA_OFFSET:
			case ID003_CMD_LOGDATA_REQUEST:
			case ID003_CMD_LOGDATA_END:
			case ID003_CMD_TOTAL_DENOMINATION:
			default:
				_id003_send_host_invalid();
				break;
			}
		}
	}

#ifdef _ENABLE_JDL
	if(_id003_tx_buff.length)
	{
	    jdl_comm_tx_pkt(&_id003_tx_buff.cmd, _id003_tx_buff.length-4);
	}
#endif /* _ENABLE_JDL */
}


/*********************************************************************//**
 * @brief Status request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_sts_request_cmd_proc(void)
{

	if (s_id003_communication_stsreq_flag != 0)
	{
		_id003_send_host_commu_err();
		/* uart error */
		s_id003_communication_stsreq_flag = 0;
	}
	else
	{

	//#if (ID003_UBA==1)
		/* Clear status request wait flag (power up) */
		if (s_id003_powerup_stsreq_flag != 0)
		{
			s_id003_powerup_stsreq_flag = 0;
		}
		s_id003_enq_resend = 0;	//UBAは1も2も同等として処理するので、ディクリメントではなくて0にする
	//#endif

		switch (ex_cline_status_tbl.line_task_mode)
		{
		case ID003_MODE_POWERUP:
			_id003_stsreq_pwr();
			break;
		case ID003_MODE_POWERUP_BILLIN_AT:
			_id003_stsreq_pwr_at();
			break;
		case ID003_MODE_POWERUP_BILLIN_SK:
			_id003_stsreq_pwr_sk();
			break;
		case ID003_MODE_POWERUP_ERROR:
			_id003_stsreq_pwr_err();
			break;
		case ID003_MODE_POWERUP_INITIAL:
			_id003_stsreq_pwr_init();
			break;
		case ID003_MODE_POWERUP_INITIAL_STACK:
			_id003_stsreq_pwr_init_stack();
			break;
		case ID003_MODE_POWERUP_INITIAL_VEND:
			_id003_stsreq_pwr_init_vend();
			break;
		case ID003_MODE_POWERUP_INITIAL_VEND_ACK:
			_id003_stsreq_pwr_init_vend_ack();
			break;
		case ID003_MODE_POWERUP_INITIAL_REJECT:
			_id003_stsreq_pwr_init_reject();
			break;
		case ID003_MODE_POWERUP_INITIAL_PAUSE:		/* 10 */
			_id003_stsreq_pwr_init_paus();
			break;
#if defined(UBA_RTQ)
		case ID003_MODE_POWERUP_INITIAL_PAYVALID:
			_id003_stsreq_pwr_init_payvalid(); //ok
			break;
		case ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK:
			_id003_stsreq_pwr_init_payvalid_ack(); //ok
			break;
#endif // UBA_RTQ
		case ID003_MODE_INITIAL:
			_id003_stsreq_init();
			break;
		case ID003_MODE_INITIAL_STACK:
			_id003_stsreq_init_stack();
			break;
		case ID003_MODE_INITIAL_REJECT:
			_id003_stsreq_init_reject();
			break;
		case ID003_MODE_INITIAL_PAUSE:
			_id003_stsreq_init_paus();
			break;
		case ID003_MODE_AUTO_RECOVERY:
			_id003_stsreq_auto_recover();
			break;
		case ID003_MODE_AUTO_RECOVERY_STACK:
			_id003_stsreq_auto_recover_stack();
			break;
		case ID003_MODE_AUTO_RECOVERY_REJECT:
			_id003_stsreq_auto_recover_reject();
			break;
		case ID003_MODE_AUTO_RECOVERY_PAUSE:
			_id003_stsreq_auto_recover_paus();
			break;
		case ID003_MODE_DISABLE:
		//case ID003_MODE_DISABLE_BOOKMARK:
			_id003_stsreq_disable();
			break;
		case ID003_MODE_DISABLE_REJECT:				/* 20 */
		//case ID003_MODE_DISABLE_REJECT_BOOKMARK:
			_id003_stsreq_disable();
			break;
		case ID003_MODE_ENABLE_WAIT_POLL:
			_id003_stsreq_enable_wait_poll();
			break;
		case ID003_MODE_ENABLE:
		//case ID003_MODE_ENABLE_BOOKMARK:
			_id003_stsreq_enable();
			break;
		case ID003_MODE_ENABLE_REJECT:
		//case ID003_MODE_ENABLE_REJECT_BOOKMARK:
			_id003_stsreq_enable_reject();
			break;
		case ID003_MODE_ACCEPT:
			_id003_stsreq_accept();
			break;

		case ID003_MODE_ACCEPT_WAIT_POLL_FOR_REJECT:
			_id003_stsreq_accept_wait_poll();
			break;

		case ID003_MODE_ACCEPT_WAIT_POLL_FOR_ESCROW: //2024-03-05
			_id003_stsreq_accept_wait_poll_for_escrow();
			break;

		case ID003_MODE_ESCROW:
			_id003_stsreq_escrow();
			break;
		case ID003_MODE_ESCROW_WAIT_CMD:
			_id003_stsreq_escrow_wait_cmd();
			break;
		case ID003_MODE_HOLD1:
			_id003_stsreq_hold1();
			break;
		case ID003_MODE_HOLD2:
			_id003_stsreq_hold2();
			break;
		case ID003_MODE_STACK:
			_id003_stsreq_stack();
			break;
		case ID003_MODE_PAUSE:
		//case ID003_MODE_PAUSE_BOOKMARK:
			_id003_stsreq_pause();
			break;
		case ID003_MODE_WAIT_PAUSE:
			_id003_stsreq_wait_pause();
			break;
		case ID003_MODE_VEND:
			_id003_stsreq_vend();
			break;
		case ID003_MODE_WAIT_VEND_ACK:
		case ID003_MODE_WAIT_VEND_ACK_FULL:
			_id003_stsreq_vend_ack();
			break;
		case ID003_MODE_VEND_FULL:
			_id003_stsreq_vend_full();
			break;
		case ID003_MODE_STACKED:
		//case ID003_MODE_STACKED_BOOKMARK:
			_id003_stsreq_stacked();
			break;
		case ID003_MODE_STACKED_FULL:
			_id003_stsreq_stacked_full();
			break;
		#if 0
		case ID003_MODE_STACK_FINISH:
			_id003_stsreq_stack_finish();
			break;
		#endif
		case ID003_MODE_REJECT_WAIT_ACCEPT_RSP:
			_id003_stsreq_reject_wait_accept_rsp();
			break;
		case ID003_MODE_REJECT:
			_id003_stsreq_reject();
			break;
		case ID003_MODE_REJECT_WAIT_POLL:
			_id003_stsreq_reject_wait_poll();
			break;
		case ID003_MODE_REJECT_WAIT_NOTE_REMOVED:
			_id003_stsreq_reject_wait_note_removed();
			break;
		case ID003_MODE_RETURN:
			_id003_stsreq_return();
			break;
		case ID003_MODE_LOG_ACCESS:
			_id003_stsreq_log_access();
			break;
		case ID003_MODE_ERROR:
			_id003_stsreq_error();
			break;
		case ID003_MODE_SHA1_HASH_BUSY:
		case ID003_MODE_SIGNATURE_BUSY:
		case ID003_MODE_POWERUP_SHA1_HASH_BUSY:
		case ID003_MODE_POWERUP_SIGNATURE_BUSY:
			_id003_sig_busy();
			break;
		case ID003_MODE_SHA1_HASH_END:
		case ID003_MODE_SIGNATURE_END:
		case ID003_MODE_POWERUP_SHA1_HASH_END:
		case ID003_MODE_POWERUP_SIGNATURE_END:
			_id003_sig_end();
			break;
		case ID003_MODE_SYSTEM_ERROR:
			_id003_stsreq_sys_error();
			break;
	//#if (ID003_UBA==1)
		case ID003_MODE_DISABLE_REJECT_2ND:
			_id003_stsreq_disable();
			break;
#if defined(UBA_RTQ)
		case ID003_MODE_PAYOUT:
			_id003_stsreq_payout();//ok
			break;
		case ID003_MODE_PAYSTAY_WAIT_POLL:
			_id003_stsreq_paystay_wait_poll();//ok
			break;
		case ID003_MODE_PAYOUT_COLLECTED_WAIT_POLL:
			_id003_stsreq_payout_collected_wait_poll(); //ok
			break;
		case ID003_MODE_AFTER_PAYVALID_ACK_PAYOUT://払い出し継続
			_id003_stsreq_payout();//ok
			break;
		case ID003_MODE_AFTER_PAYVALID_ACK_ENABLE: //すべての払い出し完了後の状態,mode_payoutをまだ抜けていないのでのEnableとは分けている
			_id003_stsreq_enable(); //ok UBA500ともともと異なっている
			break;
		case ID003_MODE_AFTER_PAYVALID_ACK_DISABLE: //すべての払い出し完了後の状態,mode_payoutをまだ抜けていないのでのDisableとは分けている
			_id003_stsreq_disable();//ok UBA500ともともと異なっている,UBA500はDisableの時はEnableの可能性をケアしていない
			break;
		case ID003_MODE_PAYSTAY:
			_id003_stsreq_paystay(); //ok
			break;
		case ID003_MODE_PAYVALID:
			_id003_stsreq_payvalid(); //ok
			break;
		case ID003_MODE_PAYVALID_ERROR:
			_id003_stsreq_payvalid_error(); //ok
			break;
		case ID003_MODE_WAIT_PAYVALID_ACK:
		case ID003_MODE_WAIT_PAYVALID_ACK_ERROR:
			_id003_stsreq_payvalid_ack(); //ok
			break;
	
		case ID003_MODE_COLLECT:
			_id003_stsreq_collect(); //ok
			break;
		case ID003_MODE_COLLECTED_WAIT_POLL:
			_id003_stsreq_collected_wait_poll(); //ok
			break;
		case ID003_MODE_RETURN_TO_BOX:
			_id003_stsreq_return_to_box(); //ok
			break;
		//2025-02-06
		case ID003_MODE_RETURN_ERROR:
			_id003_stsreq_return_error();//ok
			break;
		/* Diagnostic */
		case ID003_MODE_DIAGNOSTIC:
			_id003_stsreq_diagnostic(); //ok
			break;
		case ID003_MODE_PAYOUT_RETURN_NOTE:	//2025-03-03
			_id003_stsreq_payout_return_note(); //ok
			break;

#endif // UBA_RTQ

		default:
			/* system error ? */
			_cline_system_error(0, 8);
			break;
		}

	#if 0 //2023-09-04 上の処理で時間によるENQリクエストを設定する場合があるので、ここでクリアすると送れない
		/* Clear status request wait flag (power up) */
		if (s_id003_powerup_stsreq_flag != 0)
		{
			s_id003_powerup_stsreq_flag = 0;
		}
		/* [Interrupt Mode 1/2] Clear enq resend flag */
		if (s_id003_enq_resend != 0)
		{
			s_id003_enq_resend--;
		}
	#endif

		//#if (ID003_UBA==1)
		save_send_status_003_uba();
		//#endif
	}
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_POWERUP)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_pwr(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_POWERUP;

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_POWERUP_BILLIN_AT)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_pwr_at(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_POWERUP_BILLIN_AT;

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_POWERUP_BILLIN_SK)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_pwr_sk(void)
{
#if defined(UBA_RTQ)
	_id003_tx_buff.length = 2 + 4;
	_id003_tx_buff.cmd = ID003_STS_POWERUP_BILLIN_SK;
	_id003_tx_buff.data[0] = s_id003_powerup_stacker_to_box;
#else
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_POWERUP_BILLIN_SK;
#endif //UBA_RTQ
	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_POWERUP_ERROR)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_pwr_err(void)
{
	u8 error_data;

	if (((ex_cline_status_tbl.comm_mode == 0x0001)
	  || (ex_cline_status_tbl.comm_mode == 0x0002))
	 && (s_id003_powerup_stsreq_flag == 1))
	{
	/* [Interrupt Mode 1/2] inform powerup */
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_POWERUP;
	}
#if !defined(UBA_RTQ) //2023-12-04
	else if(s_id003_powerup_stat == POWERUP_STAT_RECOVER_SEARCH_NON)
	{
		/* GLI */
		_id003_tx_buff.length = 1 + 4;

		if( ex_poll_count_gli < 10 )
		{
			_id003_tx_buff.cmd = ID003_STS_POWERUP;
			ex_poll_count_gli++;
		}
		else
		{
			_id003_tx_buff.cmd = ID003_STS_JAM_IN_SK;
			/* Stacker JAMを送信したのでクリア */
			ex_error_003_gli = 0;
		}

	}
#endif	
	else
	{
		_id003_tx_buff.cmd = _is_id003_error_sts(ex_cline_status_tbl.error_code, &error_data);

		if (_id003_tx_buff.cmd == ID003_STS_FAILURE)
		{
			_id003_tx_buff.length = 2 + 4;
			_id003_tx_buff.data[0] = error_data;
		}
		else
		{
			_id003_tx_buff.length = 1 + 4;
		}
	}

	uart_send_id003(&_id003_tx_buff);
}

/*********************************************************************//**
 * @brief response status (ID003_MODE_SYSTEM_ERROR)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_sys_error(void)
{
	_id003_tx_buff.cmd = ID003_STS_FAILURE;
	_id003_tx_buff.length = 2 + 4;
	_id003_tx_buff.data[0] = ID003_FAILURE_EXT_ROM;
	uart_send_id003(&_id003_tx_buff);
}

/*********************************************************************//**
 * @brief response status (ID003_MODE_POWERUP_INITIAL)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_pwr_init(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_INITIALIZE;

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_POWERUP_INITIAL_STACK)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_pwr_init_stack(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_INITIALIZE;

	uart_send_id003(&_id003_tx_buff);
}

/*********************************************************************//**
 * @brief response status (ID003_MODE_POWERUP_INITIAL_VEND)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_pwr_init_vend(void)
{
	ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_VEND_ACK;
	write_status_table();
	/*
	if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
	 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
	*/
	/* iPROでBARの時にもEncryption VENDを出しているので合わせる */
	if ((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
	{
	#if 1 //defined(ID003_SPECK64)#if defined(UBA_ID003_ENC)
		if(id003_enc_mode != PAYOUT16MODE_SPECK)
	#endif
		{
			/* set encryption number */
			id003_enc_set_number(0x00);
		}
		_id003_send_enc_msg(ID003_STS_VEND_VALID, ID003_STS_POWERUP_BILLIN_SK);
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_VEND_VALID;

		uart_send_id003(&_id003_tx_buff);
	}
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_POWERUP_INITIAL_VEND_ACK)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_pwr_init_vend_ack(void)
{
	/* iPROでBARの時にもEncryption VENDを出しているので合わせる */
	if ((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
	{
	#if 1 //defined(ID003_SPECK64)
		if(id003_enc_mode != PAYOUT16MODE_SPECK)
	#endif
		{
			/* set encryption number */
			id003_enc_set_number(0x00);
		}
		_id003_send_enc_msg(ID003_STS_VEND_VALID, ID003_STS_POWERUP_BILLIN_SK);
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_VEND_VALID;

		uart_send_id003(&_id003_tx_buff);
	}
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_POWERUP_INITIAL_REJECT)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_pwr_init_reject(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_INITIALIZE;

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_POWERUP_INITIAL_PAUSE)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_pwr_init_paus(void)
{
	_id003_tx_buff.length = 1 + 4;

#if 1 //2023-11-18
	_id003_tx_buff.cmd = ID003_STS_PAUSE;
#else
	/* iVIZION not send pause in initialize status */
	_id003_tx_buff.cmd = ID003_STS_INITIALIZE;
#endif

	uart_send_id003(&_id003_tx_buff);
}

#if defined(UBA_RTQ)
/*********************************************************************//**
 * @brief response status (ID003_MODE_POWERUP_INITIAL_PAYVALID)
 * @param[in] None
 * @return    None
 **********************************************************************/
void _id003_stsreq_pwr_init_payvalid() //ok
{
	/* Change state --> ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK */
	ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK;
	_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);

	if( (ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION )
	{
	#if 1 //defined(ID003_SPECK64)
		if(id003_enc_mode != PAYOUT16MODE_SPECK)
	#endif
		{
			/* set encryption number */
			id003_enc_set_number(0x00);
		}
		_id003_send_enc_msg(ID003_STS_PAY_VALID, ID003_STS_POWERUP_BILLIN_AT);
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_PAY_VALID;

		uart_send_id003(&_id003_tx_buff);
	}
}

/*********************************************************************//**
 * @brief response status (ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK)
 * @param[in] None
 * @return    None
 **********************************************************************/
void _id003_stsreq_pwr_init_payvalid_ack( void )//ok
{

	if( (ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION )
	{
	#if 1 //defined(ID003_SPECK64)
		if(id003_enc_mode != PAYOUT16MODE_SPECK)
	#endif
		{
			/* set encryption number */
			id003_enc_set_number(0x00);
		}
		_id003_send_enc_msg(ID003_STS_PAY_VALID, ID003_STS_POWERUP_BILLIN_AT);
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_PAY_VALID;

		uart_send_id003(&_id003_tx_buff);
	}
}
#endif  // UBA_RTQ


/*********************************************************************//**
 * @brief response status (ID003_MODE_INITIAL)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_init(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_INITIALIZE;

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_INITIAL_STACK)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_init_stack(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_INITIALIZE;
	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_INITIAL_REJECT)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_init_reject(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_INITIALIZE;

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_INITIAL_PAUSE)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_init_paus(void)
{
	_id003_tx_buff.length = 1 + 4;
#if 1 //2023-11-18
	_id003_tx_buff.cmd = ID003_STS_PAUSE;
#else
	/* iVIZION not send pause in initialize status */
	_id003_tx_buff.cmd = ID003_STS_INITIALIZE;
#endif

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_AUTO_RECOVERY)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_auto_recover(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_INITIALIZE;

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_AUTO_RECOVERY_STACK)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_auto_recover_stack(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_INITIALIZE;

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_AUTO_RECOVERY_REJECT)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_auto_recover_reject(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_INITIALIZE;

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_AUTO_RECOVERY_PAUSE)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_auto_recover_paus(void)
{
	_id003_tx_buff.length = 1 + 4;
#if 1 //2023-11-18
	_id003_tx_buff.cmd = ID003_STS_PAUSE;
#else
	/* iVIZION not send pause in initialize status */
	_id003_tx_buff.cmd = ID003_STS_INITIALIZE;
#endif

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_DISABLE)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_disable(void) //ok
{
#if defined(UBA_RTQ)
	if(ex_rs_payout_remain_flag == RS_NOTE_REMAIN_CONFIRM && 
		((ex_cline_status_tbl.option & ID003_OPTION_REMAIN_NOTE) == ID003_OPTION_REMAIN_NOTE))
	{
	//only RS
		ex_rs_payout_remain_flag = RS_NOTE_REMAIN_NONE;
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd    = ID003_STS_NOTE_REMAIN;

		uart_send_id003(&_id003_tx_buff);
	}
#else
	//SS
	if(0)
	{	
	}
#endif
	else if ((ex_cline_status_tbl.option & ID003_OPTION_ENTRANCE_SENSOR) == ID003_OPTION_ENTRANCE_SENSOR)
	{
		_id003_tx_buff.length = 2 + 4;
		_id003_tx_buff.cmd = ID003_STS_DISABLE;
		// data bit[0]: 0:entrance sensor off
		//              1:entrance sensor on
		#if defined(UBA_RTQ)
		if (is_rc_rs_unit())
		{
			// data1 bit[0]: 0:entrance sensor off
			//               1:entrance sensor on
			// data1 bit[1]: 0:remain sensor off
			//			     1:remain sensor on
			_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
			_id003_tx_buff.data[0] |= RS_REMAIN_ON ? 0x02 : 0x00;
		}
		else
		{
			// data bit[0]: 0:entrance sensor off
			//              1:entrance sensor on
			_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
		}
		#else
		_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
		#endif
		s_id003_sensor_enq_status = _id003_tx_buff.data[0]; //2024-05-13

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_DISABLE;

		uart_send_id003(&_id003_tx_buff);
	}
}

#if defined(UBA_RTQ)
/*********************************************************************//**
 * @brief response status (ID003_MODE_PAYOUT)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_payout() //ok
{
	if ((s_id003_paying_info & ID003_PAYING_INFO_WAIT_STS) == ID003_PAYING_INFO_WAIT_STS)
	{
		s_id003_paying_info &= ~(ID003_PAYING_INFO_WAIT_STS);
	}

	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_PAYING;
	uart_send_id003(&_id003_tx_buff);
}

void _id003_stsreq_paystay_wait_poll() //ok
{
	if ((s_id003_paying_info & ID003_PAYING_INFO_WAIT_STS) == ID003_PAYING_INFO_WAIT_STS)
	{
		s_id003_paying_info &= ~(ID003_PAYING_INFO_WAIT_STS);
	}

	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_PAY_STAY;
	uart_send_id003(&_id003_tx_buff);

	/* change to paystay */
	ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYSTAY;
	_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
	_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_PAYSTAY_REQ, 0, 0, 0, 0);

}


void _id003_stsreq_payout_collected_wait_poll(void) //ok
{
	if ((s_id003_paying_info & ID003_PAYING_INFO_WAIT_STS) == ID003_PAYING_INFO_WAIT_STS)
	{
		s_id003_paying_info &= ~(ID003_PAYING_INFO_WAIT_STS);
	}

	_id003_tx_buff.length = 2 + 4;
	_id003_tx_buff.cmd = ID003_STS_COLLECTED;
	_id003_tx_buff.data[0] = OperationDenomi.unit_retry;

	uart_send_id003(&_id003_tx_buff);

    set_recovery_step(RECOVERY_STEP_NON);

	if(ex_cline_status_tbl.error_code == ALARM_CODE_STACKER_FULL )
	{
		_set_id003_alarm(ID003_MODE_ERROR, ex_cline_status_tbl.error_code);
		_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
	}
	else
	{
		_cline_send_msg(ID_MAIN_MBX, TMSG_LINE_PAYOUT_RETURNED_REQ, 0, 0, 0, 0);//名前がよくないが、Payout紙幣の払い出し失敗で収納成功
	}
}

void _id003_stsreq_payvalid() //ok
{
	if((s_id003_paying_info & ID003_PAYING_INFO_WAIT_STS) == ID003_PAYING_INFO_WAIT_STS)
    {
        s_id003_paying_info &= ~(ID003_PAYING_INFO_WAIT_STS);
    }

	/* change to paystay */
	ex_cline_status_tbl.line_task_mode = ID003_MODE_WAIT_PAYVALID_ACK;
	_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);

	if( (ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION )
	{
		_id003_send_enc_msg(ID003_STS_PAY_VALID, ex_cline_status_tbl.id003_escrow_payout);
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_PAY_VALID;
		uart_send_id003(&_id003_tx_buff);
	}
}

void _id003_stsreq_payvalid_ack(void) //ok 2025-04-08
{
    if( (ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION )
    {
		_id003_send_enc_msg(ID003_STS_PAY_VALID, ex_cline_status_tbl.id003_escrow_payout);
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_PAY_VALID;
		uart_send_id003(&_id003_tx_buff);
	}
}

void _id003_stsreq_payvalid_error() //ok
{
	if((s_id003_paying_info & ID003_PAYING_INFO_WAIT_STS) == ID003_PAYING_INFO_WAIT_STS)
    {
        s_id003_paying_info &= ~(ID003_PAYING_INFO_WAIT_STS);
    }

	if( (ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION )
	{
		_id003_send_enc_msg(ID003_STS_PAY_VALID, ex_cline_status_tbl.id003_escrow_payout);
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_PAY_VALID;
		uart_send_id003(&_id003_tx_buff);
	}

	/* change to paystay */
	ex_cline_status_tbl.line_task_mode = ID003_MODE_WAIT_PAYVALID_ACK_ERROR;
	_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
}

void _id003_stsreq_paystay() //ok
{
	if((s_id003_paying_info & ID003_PAYING_INFO_WAIT_STS) == ID003_PAYING_INFO_WAIT_STS)
    {
        s_id003_paying_info &= ~(ID003_PAYING_INFO_WAIT_STS);
    }
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_PAY_STAY;
	uart_send_id003(&_id003_tx_buff);
}

void _id003_stsreq_collect() //ok
{
	if ((s_id003_collecting_info & ID003_COLLECTING_INFO_WAIT_STS) == ID003_COLLECTING_INFO_WAIT_STS)
	{
		s_id003_collecting_info &= ~(ID003_COLLECTING_INFO_WAIT_STS);
	}

	_id003_tx_buff.length = 2 + 4;
	_id003_tx_buff.cmd = ID003_STS_COLLECTING;
	_id003_tx_buff.data[0] = OperationDenomi.unit;
	uart_send_id003(&_id003_tx_buff);
}

void _id003_stsreq_collected_wait_poll()//2025-03-17 ok
{
	if ((s_id003_collecting_info & ID003_COLLECTING_INFO_WAIT_STS) == ID003_COLLECTING_INFO_WAIT_STS)
	{
		s_id003_collecting_info &= ~(ID003_COLLECTING_INFO_WAIT_STS);
	}

	_id003_tx_buff.length = 2 + 4;
	_id003_tx_buff.cmd = ID003_STS_COLLECTED;

	if ( 0 == ex_main_emergency_flag )
	{
		_id003_tx_buff.data[0] = OperationDenomi.unit;
		/* set total count */
		set_recycle_total_count(RC_MODE_COLLECT, OperationDenomi.unit);
	}
	else
	{
		/* reset value of emergency flag */
		ex_main_emergency_flag = 0;
		_id003_tx_buff.data[0] = OperationDenomi.unit_emergency;
		/* set total count */
		set_recycle_total_count(RC_MODE_COLLECT, OperationDenomi.unit_emergency);
	}

	uart_send_id003(&_id003_tx_buff);
	set_recovery_step(RECOVERY_STEP_NON);

	if(ex_cline_status_tbl.error_code == ALARM_CODE_STACKER_FULL )
	{
		_set_id003_alarm(ID003_MODE_ERROR, ex_cline_status_tbl.error_code);
		_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
	}
	else
	{
		if(ex_rc_collect_sw != 0)
		{
			/* collect sw */
			/* '23-06-01 */
			if(!(ex_rc_status.sst31A.bit.u1_d1_empty) && _id003_rc_enable_check(1) == TRUE)	/* RC-Twin drum1 is not empty */
			{
				OperationDenomi.unit = 1;
				/* 回収元保存 */
				set_recovery_unit( OperationDenomi.unit, OperationDenomi.unit );

				ex_cline_status_tbl.line_task_mode = ID003_MODE_COLLECT;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);

				s_id003_collecting_info = (ID003_COLLECTING_INFO_BUSY|ID003_COLLECTING_INFO_WAIT_STS);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_COLLECT_REQ, 0, 0, 0, 0);

			}
			/* '23-06-01 */
			else if(!(ex_rc_status.sst31A.bit.u1_d2_empty) && _id003_rc_enable_check(2) == TRUE)/* RC-Twin drum2 is not empty */
			{
				OperationDenomi.unit = 2;
				/* 回収元保存 */
				set_recovery_unit( OperationDenomi.unit, OperationDenomi.unit );

				ex_cline_status_tbl.line_task_mode = ID003_MODE_COLLECT;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);

				s_id003_collecting_info = (ID003_COLLECTING_INFO_BUSY|ID003_COLLECTING_INFO_WAIT_STS);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_COLLECT_REQ, 0, 0, 0, 0);
			}
			/* '23-06-01 */
			else if(!(ex_rc_status.sst32A.bit.u2_d1_empty) && ex_rc_status.sst1A.bit.quad && _id003_rc_enable_check(3) == TRUE)	/* RC-Quad drum1 is not empty */
			{
				OperationDenomi.unit = 3;
				/* 回収元保存 */
				set_recovery_unit( OperationDenomi.unit, OperationDenomi.unit );

				ex_cline_status_tbl.line_task_mode = ID003_MODE_COLLECT;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);

				s_id003_collecting_info = (ID003_COLLECTING_INFO_BUSY|ID003_COLLECTING_INFO_WAIT_STS);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_COLLECT_REQ, 0, 0, 0, 0);
			}
			/* '23-06-01 */
			else if(!(ex_rc_status.sst32A.bit.u2_d2_empty) && ex_rc_status.sst1A.bit.quad && _id003_rc_enable_check(4) == TRUE)	/* RC-Quad drum2 is not empty */
			{
				OperationDenomi.unit = 4;
				/* 回収元保存 */
				set_recovery_unit( OperationDenomi.unit, OperationDenomi.unit );

				ex_cline_status_tbl.line_task_mode = ID003_MODE_COLLECT;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);

				s_id003_collecting_info = (ID003_COLLECTING_INFO_BUSY|ID003_COLLECTING_INFO_WAIT_STS);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_COLLECT_REQ, 0, 0, 0, 0);
			}
			else
			{
				ex_rc_collect_sw = 0;
				ex_pre_feed_after_jam = 0;

			#if defined(A_PRO)	/* '23-06-22 */
				_id003_poll_timeout = 0;
				_id003_poll_monitor = 0;
				_id003_poll_error = 0;
			#endif

				if (_is_id003_enable())
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
				//	_line_send_msg(ID_DISPLAY_MBX, TMSG_DISP_BEZEL_OFF, 0, 0, 0, 0);
				}
			}
		}
		else if(OperationDenomi.count == 1)
		{
			/* one note collect */
			ex_rc_collect_sw = 0;
			ex_pre_feed_after_jam = 0;

			if (_is_id003_enable())
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
			//	_line_send_msg(ID_DISPLAY_MBX, TMSG_DISP_BEZEL_OFF, 0, 0, 0, 0);
			}
		}
		else
		{
			/* all note collect */
			if((OperationDenomi.unit == RC_TWIN_DRUM1 && !(ex_rc_status.sst31A.bit.u1_d1_empty))	/* RC-Twin drum1 is not empty */
			|| (OperationDenomi.unit == RC_TWIN_DRUM2 && !(ex_rc_status.sst31A.bit.u1_d2_empty))	/* RC-Twin drum2 is not empty */
			|| (OperationDenomi.unit == RC_QUAD_DRUM1 && !(ex_rc_status.sst32A.bit.u2_d1_empty) && ex_rc_status.sst1A.bit.quad)		/* RC-Quad drum1 is not empty */
			|| (OperationDenomi.unit == RC_QUAD_DRUM2 && !(ex_rc_status.sst32A.bit.u2_d2_empty) && ex_rc_status.sst1A.bit.quad))	/* RC-Quad drum2 is not empty */
			{
				//	fid003_change_mode( ID003_MODE_COLLECT );
					ex_cline_status_tbl.line_task_mode = ID003_MODE_COLLECT;
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);

					s_id003_collecting_info = (ID003_COLLECTING_INFO_BUSY|ID003_COLLECTING_INFO_WAIT_STS);
					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_COLLECT_REQ, 0, 0, 0, 0);
			}
			else
			{
			#if defined(A_PRO)	/* '23-06-22 */
				_id003_poll_timeout = 0;
				_id003_poll_monitor = 0;
				_id003_poll_error = 0;
			#endif
				if (_is_id003_enable())
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
				//	_line_send_msg(ID_DISPLAY_MBX, TMSG_DISP_BEZEL_OFF, 0, 0, 0, 0);
				}
			}
		}
	}
}

void _id003_stsreq_return_to_box() //ok
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_RETURN_TO_BOX;
	uart_send_id003(&_id003_tx_buff);
}

void _id003_stsreq_return_error()	//ok //2025-02-06
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_RETURN_ERROR;
	uart_send_id003(&_id003_tx_buff);
}


void _id003_stsreq_diagnostic() //ok
{
	_id003_tx_buff.length  = 3 + 4;
	_id003_tx_buff.cmd     = ID003_STS_EXTENSION;
	_id003_tx_buff.data[0] = ID003_CMD_EX_DIAG_STATUS;

//	if(ex_rc_status.sst1A.bit.busy == 1)
	if((ex_multi_job.busy & TASK_ST_FEED) != 0 || ex_rc_status.sst1A.bit.busy == 1)
	{
		ex_diag_status = ID003_STS_EXT_DIAG_BUSY;
	}
	else
	{
		if(ex_diag_status == ID003_STS_EXT_DIAG_BUSY)
		{
			if(ex_diag_emg == 0)
			{
				// _cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DIAG_END_REQ, 0, 0, 0, 0);
			}
			ex_diag_emg = 0;
		}
		ex_diag_status = ID003_STS_EXT_DIAG_READY;
	}

	_id003_tx_buff.data[1] = ex_diag_status;

	uart_send_id003(&_id003_tx_buff);
}

void _id003_stsreq_payout_return_note(void) //ok
{
	if ((s_id003_paying_info & ID003_PAYING_INFO_WAIT_STS) == ID003_PAYING_INFO_WAIT_STS)
	{
		s_id003_paying_info &= ~(ID003_PAYING_INFO_WAIT_STS);
	}

	_id003_tx_buff.length = 2 + 4;
	_id003_tx_buff.cmd = ID003_STS_RETURN_PAY_OUT;

	if(ex_rc_status.sst21B.bit.u1_detect_dbl || ex_rc_status.sst22B.bit.u2_detect_dbl)
	{
		_id003_tx_buff.data[0] = 0x01;	/* 長さ異常 */
	}
	else
	{
		_id003_tx_buff.data[0] = 0x03;	/* 搬送異常 */
	}
	uart_send_id003(&_id003_tx_buff);
}



#endif // UBA_RTQ


/*********************************************************************//**
 * @brief response status (ID003_MODE_ENABLE_WAIT_POLL)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_enable_wait_poll(void)
{
	if (_is_id003_enable())
	{

	//#if (ID003_UBA==1)
		//インタラプトモード2以外は、基本Enabe時にこのシーケンスが最初でここでポール待ちでないEnableへ遷移する
		if( s_id003_2nd_note != 0 ) /* EnableステータスでPoll受信 次からAccepting */
		{
			_id003_tx_buff.length = 1 + 4;
			_id003_tx_buff.cmd = ID003_STS_ENABLE;
			uart_send_id003(&_id003_tx_buff);

			if(ex_cline_status_tbl.comm_mode != 2)
			{
				if( s_id003_2nd_note == 10)
				{
					_set_id003_alarm(ID003_MODE_ERROR, (u16)s_id003_2nd_note_code);
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ACCEPT; // Enableを送信したので2枚目のAcceptへ遷移
		//			set_recovery_step( RECOVERY_STEP_ACCEPT );		

					write_status_table();

					_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 0, 0, 0);
					_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD1, 0, 0, 0);
					_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD2, 0, 0, 0);

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
				}
			}
			else
			{
			/* インタラプト2の場合、Enableでのポール待ちのこのモードに遷移しないので、このでのEscrow処理は必要ない*/
			}
			return;
		}
	//#endif

		if ((ex_cline_status_tbl.option & ID003_OPTION_ENTRANCE_SENSOR) == ID003_OPTION_ENTRANCE_SENSOR)
		{
			_id003_tx_buff.length = 2 + 4;
			_id003_tx_buff.cmd = ID003_STS_ENABLE;
			// data bit[0]: 0:entrance sensor off
			//              1:entrance sensor on
			#if defined(UBA_RTQ)
			if (is_rc_rs_unit())
			{
				// data1 bit[0]: 0:entrance sensor off
				//               1:entrance sensor on
				// data1 bit[1]: 0:remain sensor off
				//			     1:remain sensor on
				_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
				_id003_tx_buff.data[0] |= RS_REMAIN_ON ? 0x02 : 0x00;
			}
			else
			{
				// data bit[0]: 0:entrance sensor off
				//              1:entrance sensor on
				_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
			}
			#else
			_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
			#endif //
			s_id003_sensor_enq_status = _id003_tx_buff.data[0]; //2024-05-13

			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

			uart_send_id003(&_id003_tx_buff);
		}
		else
		{
			_id003_tx_buff.length = 1 + 4;
			_id003_tx_buff.cmd = ID003_STS_ENABLE;

			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

			uart_send_id003(&_id003_tx_buff);
		}
	}
	else
	{
	//Disable
		if ((ex_cline_status_tbl.option & ID003_OPTION_ENTRANCE_SENSOR) == ID003_OPTION_ENTRANCE_SENSOR)
		{
			_id003_tx_buff.length = 2 + 4;
			_id003_tx_buff.cmd = ID003_STS_DISABLE;
			// data bit[0]: 0:entrance sensor off
			//              1:entrance sensor on
			#if defined(UBA_RTQ)
			if (is_rc_rs_unit())
			{
				// data1 bit[0]: 0:entrance sensor off
				//               1:entrance sensor on
				// data1 bit[1]: 0:remain sensor off
				//			     1:remain sensor on
				_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
				_id003_tx_buff.data[0] |= RS_REMAIN_ON ? 0x02 : 0x00;
			}
			else
			{
				// data bit[0]: 0:entrance sensor off
				//              1:entrance sensor on
				_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
			}
			#else
			_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
			#endif //
			s_id003_sensor_enq_status = _id003_tx_buff.data[0]; //2024-05-13

			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

			uart_send_id003(&_id003_tx_buff);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else
		{
			_id003_tx_buff.length = 1 + 4;
			_id003_tx_buff.cmd = ID003_STS_DISABLE;

			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

			uart_send_id003(&_id003_tx_buff);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
	}
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_ENABLE)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_enable(void)
{


	//#if (ID003_UBA==1)
	//インタラプトモード2はここのEnableが最初だが、それ以外は_id003_stsreq_enable_wait_poll(void)経由でここに来ている
	//ここのシーケンスはEnable送信待ちのシーケンスではない
	if( s_id003_2nd_note != 0 ) /* EnableステータスでPoll受信 次からAccepting */
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_ENABLE;
		uart_send_id003(&_id003_tx_buff);

		if(ex_cline_status_tbl.comm_mode != 2)
		{
		//  ここAcceptingにする可能性は低い
		//  その前のEnableでのポール待ちの、_id003_stsreq_enable_wait_poll(); でAcceptingにしている
		//  その為、ここでs_id003_2nd_note の変化によるEscrowなどの送信処理は必要ない
		//  代わりにAcceptingでのポール受信処理で行っている
		}
		else
		{
			/* インタラプトモード2なので、AcceptingはSkip*/
			if(s_id003_2nd_note == 2)/* 識別完了、識別OKなので次からEscrow */
			{
				/* 識別完了、識別OKなので次からEscrow */
				_id003_set_escrow_or_reject_2nd(1);
			}
			else if(s_id003_2nd_note == 5)/* 識別完了、識別NGなので次からReReject */
			{
				/* 識別完了、識別NGなので次からReReject */
				_id003_set_escrow_or_reject_2nd(2);
			}
			else if(s_id003_2nd_note == 10)
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)s_id003_2nd_note_code);
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
			}
			else
			{
				/* 設定はするが、mode2なので、ENQとAcceptingは送信されない*/
				/* ホストからのたまたまのPoll用*/
				ex_cline_status_tbl.line_task_mode = ID003_MODE_ACCEPT; //
				write_status_table();

				_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 0, 0, 0);
				_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD1, 0, 0, 0);
				_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD2, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
			}
		}
		return;
	}
	//#endif

	if (_is_id003_enable())
	{	//Enable
	#if defined(UBA_RTQ)
		if(ex_rs_payout_remain_flag == RS_NOTE_REMAIN_CONFIRM && 
			((ex_cline_status_tbl.option & ID003_OPTION_REMAIN_NOTE) == ID003_OPTION_REMAIN_NOTE))
		{
		//only RS	
			ex_rs_payout_remain_flag = RS_NOTE_REMAIN_NONE;
			_id003_tx_buff.length = 1 + 4;
			_id003_tx_buff.cmd    = ID003_STS_NOTE_REMAIN;

			uart_send_id003(&_id003_tx_buff);
		}
	#else
		//SS
		if(0)
		{}	
	#endif
		else
		{
			if ((ex_cline_status_tbl.option & ID003_OPTION_ENTRANCE_SENSOR) == ID003_OPTION_ENTRANCE_SENSOR)
			{
				_id003_tx_buff.length = 2 + 4;
				_id003_tx_buff.cmd = ID003_STS_ENABLE;
				// data bit[0]: 0:entrance sensor off
				//              1:entrance sensor on
				#if defined(UBA_RTQ)
				if (is_rc_rs_unit())
				{
					// data1 bit[0]: 0:entrance sensor off
					//               1:entrance sensor on
					// data1 bit[1]: 0:remain sensor off
					//			     1:remain sensor on
					_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
					_id003_tx_buff.data[0] |= RS_REMAIN_ON ? 0x02 : 0x00;
				}
				else
				{
					// data bit[0]: 0:entrance sensor off
					//              1:entrance sensor on
					_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
				}
				#else
				_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
				#endif //
				s_id003_sensor_enq_status = _id003_tx_buff.data[0]; //2024-05-13

				uart_send_id003(&_id003_tx_buff);
			}
			else
			{
				_id003_tx_buff.length = 1 + 4;
				_id003_tx_buff.cmd = ID003_STS_ENABLE;
				uart_send_id003(&_id003_tx_buff);
			}
		}
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);	//enableを送っている

	}	//Enable End
	else
	{
		//Disable
	#if defined(UBA_RTQ)
		if(ex_rs_payout_remain_flag == RS_NOTE_REMAIN_CONFIRM && 
			((ex_cline_status_tbl.option & ID003_OPTION_REMAIN_NOTE) == ID003_OPTION_REMAIN_NOTE))
		{
		//only RS	
			ex_rs_payout_remain_flag = RS_NOTE_REMAIN_NONE;
			_id003_tx_buff.length = 1 + 4;
			_id003_tx_buff.cmd    = ID003_STS_NOTE_REMAIN;

			uart_send_id003(&_id003_tx_buff);
		}
	#else
		//SS
		if(0)
		{}	
	#endif
		else
		{
			if ((ex_cline_status_tbl.option & ID003_OPTION_ENTRANCE_SENSOR) == ID003_OPTION_ENTRANCE_SENSOR)
			{
				_id003_tx_buff.length = 2 + 4;
				_id003_tx_buff.cmd = ID003_STS_DISABLE;
				// data bit[0]: 0:entrance sensor off
				//              1:entrance sensor on
				#if defined(UBA_RTQ)
				if (is_rc_rs_unit())
				{
					// data1 bit[0]: 0:entrance sensor off
					//               1:entrance sensor on
					// data1 bit[1]: 0:remain sensor off
					//			     1:remain sensor on
					_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
					_id003_tx_buff.data[0] |= RS_REMAIN_ON ? 0x02 : 0x00;
				}
				else
				{
					// data bit[0]: 0:entrance sensor off
					//              1:entrance sensor on
					_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
				}
				#else
				_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
				#endif
				s_id003_sensor_enq_status = _id003_tx_buff.data[0]; //2024-05-13

				uart_send_id003(&_id003_tx_buff);
			}
			else
			{
				_id003_tx_buff.length = 1 + 4;
				_id003_tx_buff.cmd = ID003_STS_DISABLE;

				uart_send_id003(&_id003_tx_buff);
			}
		}
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);		//Disableを送っている
	//Disable End
	}


}


/*********************************************************************//**
 * @brief response status (ID003_MODE_ENABLE_REJECT)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_enable_reject(void)
{
	if ((ex_cline_status_tbl.option & ID003_OPTION_ENTRANCE_SENSOR) == ID003_OPTION_ENTRANCE_SENSOR)
	{
		_id003_tx_buff.length = 2 + 4;
		_id003_tx_buff.cmd = ID003_STS_ENABLE;
		// data bit[0]: 0:entrance sensor off
		//              1:entrance sensor on
		#if defined(UBA_RTQ)
		if (is_rc_rs_unit())
		{
			// data1 bit[0]: 0:entrance sensor off
			//               1:entrance sensor on
			// data1 bit[1]: 0:remain sensor off
			//			     1:remain sensor on
			_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
			_id003_tx_buff.data[0] |= RS_REMAIN_ON ? 0x02 : 0x00;
		}
		else
		{
			// data bit[0]: 0:entrance sensor off
			//              1:entrance sensor on
			_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
		}
		#else
		_id003_tx_buff.data[0] = SENSOR_ENTRANCE ? 0x01 : 0x00;
		#endif //
		s_id003_sensor_enq_status = _id003_tx_buff.data[0]; //2024-05-13

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_ENABLE;
		uart_send_id003(&_id003_tx_buff);
	}
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_ACCEPT)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_accept(void)
{

	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_ACCEPTING;

	if(s_id003_2nd_note == 2)/* 識別完了、識別OKなので次からEscrow */
	{
		/* 識別完了、識別OKなので次からEscrow */
		_id003_set_escrow_or_reject_2nd(1);
	}
	else if(s_id003_2nd_note == 5)/* 識別完了、識別NGなので次からReReject */
	{
		/* 識別完了、識別NGなので次からReReject */
		_id003_set_escrow_or_reject_2nd(2);
	}	
	else if(s_id003_2nd_note == 10)/* Error */
	{
		_set_id003_alarm(ID003_MODE_ERROR, (u16)s_id003_2nd_note_code);
		_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
	}	
	else
	{

	}

	write_status_table();

	uart_send_id003(&_id003_tx_buff);
}

void _id003_stsreq_accept_wait_poll_for_escrow(void) //2024-03-05
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_ACCEPTING;
	uart_send_id003(&_id003_tx_buff);

	ex_cline_status_tbl.line_task_mode = ID003_MODE_ESCROW;
	write_status_table();
	_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 300, 0, 0);

	/* change status */
	_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);

}



/*********************************************************************//**
 * @brief response status (ID003_MODE_ACCEPT_WAIT_POLL_FOR_REJECT)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_accept_wait_poll(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_ACCEPTING;

	_set_id003_reject(ID003_MODE_REJECT, (u16)ex_cline_status_tbl.reject_code);

	_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

	uart_send_id003(&_id003_tx_buff);

	/* change status */
	_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_ESCROW)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_escrow(void)
{
	u16 cnt;
	u8 escrow_code;

	ex_cline_status_tbl.line_task_mode = ID003_MODE_ESCROW_WAIT_CMD;
	write_status_table();
	_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 0, 0, 0);
	_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_HOLD2, 1000, 0, 0);

	if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
	 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
	{
		escrow_code = _is_id003_escrow_data(ex_cline_status_tbl.escrow_code);
		_id003_send_enc_msg(ID003_STS_ESCROW, escrow_code);
	}
	else if (ex_cline_status_tbl.escrow_code == BAR_INDX)
	{
		_id003_tx_buff.cmd = ID003_STS_ESCROW;
		_id003_tx_buff.data[0] = ID003_BARCODE_ESCROW_CODE;
#ifdef TWO_D_EXTERNED_BARCODE_TICKET
		if (   (ex_cline_status_tbl.barcode_type == 0x04)
			|| ((ex_cline_status_tbl.barcode_length == 0x01)
			 && (ex_cline_status_tbl.barcode_length == 0xff)
			 && (ex_bar_length[0] > 240)) )
		{
			_id003_tx_buff.data[1] = 2;	// 2 blocks
			_id003_tx_buff.length = 3 + 4;
		}
		else
#endif
		{
			for (cnt = 0; cnt < ex_bar_length[0]; cnt++)
			{
				_id003_tx_buff.data[(cnt + 1)] = ex_barcode[cnt];
			}
			_id003_tx_buff.length = (cnt + 2) + 4;
		}

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
	#if defined(SKEW_TEST) //2025-07-26
		_id003_tx_buff.length = 2 + 4 + 2;
		_id003_tx_buff.cmd = ID003_STS_ESCROW;
		_id003_tx_buff.data[0] = _is_id003_escrow_data(ex_cline_status_tbl.escrow_code);

		_id003_tx_buff.data[1] = (u8)(ex_rc_skew_pulse >> 8);
		_id003_tx_buff.data[2] = (u8)(ex_rc_skew_pulse);
		uart_send_id003(&_id003_tx_buff);
	#else	

		_id003_tx_buff.length = 2 + 4;
		_id003_tx_buff.cmd = ID003_STS_ESCROW;
		_id003_tx_buff.data[0] = _is_id003_escrow_data(ex_cline_status_tbl.escrow_code);

		uart_send_id003(&_id003_tx_buff);
	#endif
	}
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_ESCROW_WAIT_CMD)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_escrow_wait_cmd(void)
{
	u16 cnt;
	u8 escrow_code;

	if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
	 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
	{
		escrow_code = _is_id003_escrow_data(ex_cline_status_tbl.escrow_code);
		_id003_send_enc_msg(ID003_STS_ESCROW, escrow_code);
	}
	else if (ex_cline_status_tbl.escrow_code == BAR_INDX)
	{
		_id003_tx_buff.cmd = ID003_STS_ESCROW;
		_id003_tx_buff.data[0] = ID003_BARCODE_ESCROW_CODE;
#ifdef TWO_D_EXTERNED_BARCODE_TICKET
		if (   (ex_cline_status_tbl.barcode_type == 0x04)
			|| ((ex_cline_status_tbl.barcode_length == 0x01)
			 && (ex_cline_status_tbl.barcode_length == 0xff)
			 && (ex_bar_length[0] > 240)) )
		{
			_id003_tx_buff.data[1] = 2;	// 2 blocks
			_id003_tx_buff.length = 3 + 4;
		}
		else
#endif
		{
			for (cnt = 0; cnt < ex_bar_length[0]; cnt++)
			{
				_id003_tx_buff.data[(cnt + 1)] = ex_barcode[cnt];
			}
			_id003_tx_buff.length = (cnt + 2) + 4;
		}

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
	#if defined(SKEW_TEST) //2025-07-26
		_id003_tx_buff.length = 2 + 4 + 2;
		_id003_tx_buff.cmd = ID003_STS_ESCROW;
		_id003_tx_buff.data[0] = _is_id003_escrow_data(ex_cline_status_tbl.escrow_code);

		_id003_tx_buff.data[1] = (u8)(ex_rc_skew_pulse >> 8);
		_id003_tx_buff.data[2] = (u8)(ex_rc_skew_pulse);

		uart_send_id003(&_id003_tx_buff);

	#else	
		_id003_tx_buff.length = 2 + 4;
		_id003_tx_buff.cmd = ID003_STS_ESCROW;
		_id003_tx_buff.data[0] = _is_id003_escrow_data(ex_cline_status_tbl.escrow_code);

		uart_send_id003(&_id003_tx_buff);
	#endif
	}
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_HOLD1)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_hold1(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_HOLDING;

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_HOLD2)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_hold2(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_HOLDING;

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_STACK)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_stack(void)
{
	if ((s_id003_stacking_info & ID003_STACKING_INFO_WAIT_STS) == ID003_STACKING_INFO_WAIT_STS)
	{
		s_id003_stacking_info &= ~(ID003_STACKING_INFO_WAIT_STS);
	}
#if defined(UBA_RTQ)
	_id003_tx_buff.length = 2 + 4;
	_id003_tx_buff.cmd = ID003_STS_STACKING;
	_id003_tx_buff.data[0] = OperationDenomi.unit;
#else
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_STACKING;
#endif 
	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_PAUSE)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_pause(void)
{
	if ((s_id003_stacking_info & ID003_STACKING_INFO_WAIT_STS) == ID003_STACKING_INFO_WAIT_STS)
	{
		s_id003_stacking_info &= ~(ID003_STACKING_INFO_WAIT_STS);
	}

	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_PAUSE;

	uart_send_id003(&_id003_tx_buff);
}

/*********************************************************************//**
 * @brief response status (ID003_MODE_WAIT_PAUSE)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_wait_pause(void)
{

	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_STACKING;

	uart_send_id003(&_id003_tx_buff);

	ex_cline_status_tbl.line_task_mode = ID003_MODE_PAUSE;
	write_status_table();

	/* change status */
	_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);

}

/*********************************************************************//**
 * @brief response status (ID003_MODE_VEND)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_vend(void)
{
	u8 escrow_code;

	if ((s_id003_stacking_info & ID003_STACKING_INFO_WAIT_STS) == ID003_STACKING_INFO_WAIT_STS)
	{
		s_id003_stacking_info &= ~(ID003_STACKING_INFO_WAIT_STS);
	}

	ex_cline_status_tbl.line_task_mode = ID003_MODE_WAIT_VEND_ACK;
	write_status_table();

	if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
	 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
	{
		escrow_code = _is_id003_escrow_data(ex_cline_status_tbl.escrow_code);
		_id003_send_enc_msg(ID003_STS_VEND_VALID, escrow_code);
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_VEND_VALID;

		uart_send_id003(&_id003_tx_buff);
	}
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_WAIT_VEND_ACK)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_vend_ack(void)
{
	u8 escrow_code;

	if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
	 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
	{
		escrow_code = _is_id003_escrow_data(ex_cline_status_tbl.escrow_code);
		_id003_send_enc_msg(ID003_STS_VEND_VALID, escrow_code);
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_VEND_VALID;

		uart_send_id003(&_id003_tx_buff);
	}
}

/*********************************************************************//**
 * @brief response status (ID003_MODE_VEND_FULL)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_vend_full(void)
{
	u8 escrow_code;

	if ((s_id003_stacking_info & ID003_STACKING_INFO_WAIT_STS) == ID003_STACKING_INFO_WAIT_STS)
	{
		s_id003_stacking_info &= ~(ID003_STACKING_INFO_WAIT_STS);
	}

	ex_cline_status_tbl.line_task_mode = ID003_MODE_WAIT_VEND_ACK_FULL;
	write_status_table();

	if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
	 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
	{
		escrow_code = _is_id003_escrow_data(ex_cline_status_tbl.escrow_code);
		_id003_send_enc_msg(ID003_STS_VEND_VALID, escrow_code);
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_VEND_VALID;

		uart_send_id003(&_id003_tx_buff);
	}
}





/*********************************************************************//**
 * @brief response status (ID003_MODE_STACKED)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_stacked(void)
{
#if defined(UBA_RTQ)
	_id003_tx_buff.length = 2 + 4;
	_id003_tx_buff.cmd = ID003_STS_STACKED;
	_id003_tx_buff.data[0] = OperationDenomi.unit;
#else
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_STACKED;
#endif 
	/* UBA700はStackedステータス時には、すでにエラーにするか決定しているので上記のID003_MODE_STACK_FINISHは必要ない*/
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (ex_cline_status_tbl.error_code != ALARM_CODE_OK)
		{
			_set_id003_alarm(ID003_MODE_ERROR, ex_cline_status_tbl.error_code);
		}
		else if (_is_id003_enable())
		{
			if (ex_cline_status_tbl.comm_mode == 0x0002)
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
				write_status_table();
			}
		}
		else
		{
			//(ID003_UBA==1) //紙幣取り込んでいる場合返却が必要
			//内部的にEnableへ遷移する前にInhibitコマンドでのDisableはここで処理する
			//Enableステータスへ遷移以降のInhibitコマンドは受信処理で直接対応
			if(s_id003_2nd_note == 2)/* 識別完了、識別OKなので次からEscrow */
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT_2ND, REJECT_CODE_RETURN); //Stacked->Disableステータス
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, REJECT_CODE_RETURN, 0, 0, 0);
				//rejectで書いてる write_status_table();
			}
			else if(s_id003_2nd_note == 5)/* 識別完了、識別NGなので次からReReject */
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT_2ND, REJECT_CODE_RETURN); //Stacked->Disableステータス
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, REJECT_CODE_RETURN, 0, 0, 0);
				//rejectで書いてる write_status_table();
			}
			else if(s_id003_2nd_note == 10)
			{
			// 取り込み中のエラー	
				_set_id003_alarm(ID003_MODE_ERROR, (u16)s_id003_2nd_note_code);
				//alarmで書いてる write_status_table();
			}
			else if(s_id003_2nd_note == 1)
			{
			/* この場合は、Disable Rejectが理想*/
				_set_id003_reject(ID003_MODE_DISABLE_REJECT_2ND, REJECT_CODE_RETURN); //Stacked->Disableステータス
			}	
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
			}
		}

		uart_send_id003(&_id003_tx_buff);

		/* change status */
		_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		
	}
	else
	{
	/* WAIT */
		s_id003_status_wait_next_mode = ID003_MODE_DISABLE;

		uart_send_id003(&_id003_tx_buff);
	}

}





/*********************************************************************//**
 * @brief response status (ID003_MODE_STACKED_FULL)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_stacked_full(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_STACKED;

	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (ex_cline_status_tbl.error_code != ALARM_CODE_OK)
		{
			_set_id003_alarm(ID003_MODE_ERROR, ex_cline_status_tbl.error_code);
		}

		uart_send_id003(&_id003_tx_buff);

		/* change status */
		_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		/* ステータスリクエスト後にs_id003_enq_resendがディクリメントされるため2を設定 */
		if (s_id003_enq_resend == 1)
		{
			s_id003_enq_resend = 2;
		}
	}
	else
	{
	/* WAIT */
		s_id003_status_wait_next_mode = ID003_MODE_ERROR;
		s_id003_status_wait_error = ex_cline_status_tbl.error_code;

		uart_send_id003(&_id003_tx_buff);
	}
}



/*********************************************************************//**
 * @brief response status (ID003_MODE_STACK_FINISH)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
void _id003_stsreq_stack_finish(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_STACKED;
	uart_send_id003(&_id003_tx_buff);
}
#endif


/*********************************************************************//**
 * @brief response status (ID003_MODE_REJECT_WAIT_ACCEPT_RSP)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_reject_wait_accept_rsp(void)
{

#if 1 //2023-08-24
	_id003_tx_buff.length = 2 + 4;
	_id003_tx_buff.cmd = ID003_STS_REJECTING;
	_id003_tx_buff.data[0] = _is_id003_reject_data(ex_cline_status_tbl.reject_code);
	uart_send_id003(&_id003_tx_buff);
#else
	//NG#if !defined(PRJ_IVIZION)//送信済みにした方がいいのでは
	//ex_cline_status_tbl.line_task_mode = ID003_MODE_REJECT_WAIT_NOTE_REMOVED;
	//write_status_table();
	//#endif
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_ACCEPTING;

	uart_send_id003(&_id003_tx_buff);
#endif

}


/*********************************************************************//**
 * @brief response status (ID003_MODE_REJECT)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_reject(void)
{
	_id003_tx_buff.length = 2 + 4;
	_id003_tx_buff.cmd = ID003_STS_REJECTING;
	_id003_tx_buff.data[0] = _is_id003_reject_data(ex_cline_status_tbl.reject_code);

	ex_cline_status_tbl.line_task_mode = ID003_MODE_REJECT_WAIT_NOTE_REMOVED;
	write_status_table();

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_REJECT_WAIT_POLL)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_reject_wait_poll(void)
{
	_id003_tx_buff.length = 2 + 4;
	_id003_tx_buff.cmd = ID003_STS_REJECTING;
	_id003_tx_buff.data[0] = _is_id003_reject_data(ex_cline_status_tbl.reject_code);

	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (_is_id003_enable())
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
			write_status_table();
		}
		else
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
		}
		uart_send_id003(&_id003_tx_buff);

		/* change status */
		_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);

		/* ステータスリクエスト後にs_id003_enq_resendがディクリメントされるため2を設定 */
		if (s_id003_enq_resend == 1)
		{
			s_id003_enq_resend = 2;
		}
	}
	else
	{
		s_id003_status_wait_next_mode = ID003_MODE_DISABLE;

		uart_send_id003(&_id003_tx_buff);
	}
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_REJECT_WAIT_NOTE_REMOVED)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_reject_wait_note_removed(void)
{
	_id003_tx_buff.length = 2 + 4;
	_id003_tx_buff.cmd = ID003_STS_REJECTING;
	_id003_tx_buff.data[0] = _is_id003_reject_data(ex_cline_status_tbl.reject_code);

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_RETURN)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_return(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_STS_RETURNING;

	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief response status (ID003_MODE_LOG_ACCESS)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_log_access(void)
{
	_id003_tx_buff.length = 3 + 4;
	_id003_tx_buff.cmd = ID003_STS_EXTENSION;
	_id003_tx_buff.data[0] = ex_cline_status_tbl.log_access_mode;
	if ((ex_cline_status_tbl.log_access_status == ID003_STS_EXT_LOG_ACCESS_END)
	 || (ex_cline_status_tbl.log_access_status == ID003_STS_EXT_LOG_ACCESS_BUSY))
	{
		_id003_tx_buff.data[1] = ex_cline_status_tbl.log_access_status;
	}
	else
	{
		_id003_tx_buff.data[1] = ID003_STS_EXT_LOG_ACCESS_READY;
	}

	uart_send_id003(&_id003_tx_buff);
}




/*********************************************************************//**
 * @brief response status (ID003_MODE_ERROR)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stsreq_error(void)
{
	u8 error_data;

	_id003_tx_buff.cmd = _is_id003_error_sts(ex_cline_status_tbl.error_code, &error_data);

	if (_id003_tx_buff.cmd == ID003_STS_FAILURE)
	{
		_id003_tx_buff.length = 2 + 4;
		_id003_tx_buff.data[0] = error_data;
	}
	else
	{
		_id003_tx_buff.length = 1 + 4;
	}

#if !defined(UBA_RTQ) //2023-12-04
	if( ex_error_003_gli == 1 )
	{
	/* イニシャルでのエラーもあるので無条件でクリア */
		ex_error_003_gli = 0;
	}
#endif
	uart_send_id003(&_id003_tx_buff);
}
/*********************************************************************//**
 * @brief response status (ID003_MODE_SIGNATURE_BUSY)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_sig_busy(void)
{
	if((ex_cline_status_tbl.line_task_mode == ID003_MODE_SHA1_HASH_BUSY)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_SHA1_HASH_BUSY))
	{
		_id003_tx_buff.length = 3 + 4;
		_id003_tx_buff.cmd = ID003_STS_SHA1_HASH_BUSY_OR_END;
		_id003_tx_buff.data[0] = 0x01;
		_id003_tx_buff.data[1] = 0xFF;
	}
	if((ex_cline_status_tbl.line_task_mode == ID003_MODE_SIGNATURE_BUSY)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_SIGNATURE_BUSY))
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_SIGNATURE_BUSY;
	}

	uart_send_id003(&_id003_tx_buff);
}
/*********************************************************************//**
 * @brief response status (ID003_MODE_SIGNATURE_BUSY)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_sig_end(void)
{
	if(ex_cline_status_tbl.line_task_mode == ID003_MODE_SHA1_HASH_END)
	{
		int i = 0;
		u8 sha1_swapper[20];

		//sha1_swapper[i++] = (ex_rom_sha1[3] << 24) | (ex_rom_sha1[2] << 16) | (ex_rom_sha1[1] << 8) | ex_rom_sha1[0];
		sha1_swapper[i++] = ex_rom_sha1[3];
		sha1_swapper[i++] = ex_rom_sha1[2];
		sha1_swapper[i++] = ex_rom_sha1[1];
		sha1_swapper[i++] = ex_rom_sha1[0];
		sha1_swapper[i++] = ex_rom_sha1[7];
		sha1_swapper[i++] = ex_rom_sha1[6];
		sha1_swapper[i++] = ex_rom_sha1[5];
		sha1_swapper[i++] = ex_rom_sha1[4];
		sha1_swapper[i++] = ex_rom_sha1[11];
		sha1_swapper[i++] = ex_rom_sha1[10];
		sha1_swapper[i++] = ex_rom_sha1[9];
		sha1_swapper[i++] = ex_rom_sha1[8];
		sha1_swapper[i++] = ex_rom_sha1[15];
		sha1_swapper[i++] = ex_rom_sha1[14];
		sha1_swapper[i++] = ex_rom_sha1[13];
		sha1_swapper[i++] = ex_rom_sha1[12];
		sha1_swapper[i++] = ex_rom_sha1[19];
		sha1_swapper[i++] = ex_rom_sha1[18];
		sha1_swapper[i++] = ex_rom_sha1[17];
		sha1_swapper[i] = ex_rom_sha1[16];

		_id003_tx_buff.cmd = ID003_STS_SHA1_HASH_BUSY_OR_END;
		_id003_tx_buff.data[0] = 0x01;
		_id003_tx_buff.data[1] = 0x00;

		for(i = 0;i < 20; i++)
		{
			_id003_tx_buff.data[i+2] = sha1_swapper[i];//s_bif_sha1_context.digest[i];
		}
		_id003_tx_buff.length = 23 + 4;
		if(	ex_cline_status_tbl.error_code == 0)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
		}
		else
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ERROR;
		}
	}
	else if(ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_SHA1_HASH_END)
	{
		int i = 0;
		u8 sha1_swapper[20];

		//sha1_swapper[i++] = (ex_rom_sha1[3] << 24) | (ex_rom_sha1[2] << 16) | (ex_rom_sha1[1] << 8) | ex_rom_sha1[0];
		sha1_swapper[i++] = ex_rom_sha1[3];
		sha1_swapper[i++] = ex_rom_sha1[2];
		sha1_swapper[i++] = ex_rom_sha1[1];
		sha1_swapper[i++] = ex_rom_sha1[0];
		sha1_swapper[i++] = ex_rom_sha1[7];
		sha1_swapper[i++] = ex_rom_sha1[6];
		sha1_swapper[i++] = ex_rom_sha1[5];
		sha1_swapper[i++] = ex_rom_sha1[4];
		sha1_swapper[i++] = ex_rom_sha1[11];
		sha1_swapper[i++] = ex_rom_sha1[10];
		sha1_swapper[i++] = ex_rom_sha1[9];
		sha1_swapper[i++] = ex_rom_sha1[8];
		sha1_swapper[i++] = ex_rom_sha1[15];
		sha1_swapper[i++] = ex_rom_sha1[14];
		sha1_swapper[i++] = ex_rom_sha1[13];
		sha1_swapper[i++] = ex_rom_sha1[12];
		sha1_swapper[i++] = ex_rom_sha1[19];
		sha1_swapper[i++] = ex_rom_sha1[18];
		sha1_swapper[i++] = ex_rom_sha1[17];
		sha1_swapper[i] = ex_rom_sha1[16];

		_id003_tx_buff.cmd = ID003_STS_SHA1_HASH_BUSY_OR_END;
		_id003_tx_buff.data[0] = 0x01;
		_id003_tx_buff.data[1] = 0x00;

		for(i = 0;i < 20; i++)
		{
			_id003_tx_buff.data[i+2] = sha1_swapper[i];//s_bif_sha1_context.digest[i];
		}
		_id003_tx_buff.length = 23 + 4;

		if(s_id003_powerup_stat == POWERUP_STAT_RECOVER_SEARCH_NON)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_ERROR;
		}
		else if(s_id003_powerup_stat == POWERUP_STAT_REJECT)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
		}
		else if((s_id003_powerup_stat == POWERUP_STAT_FEED_STACK)
		 || (s_id003_powerup_stat == POWERUP_STAT_RECOVER_FEED_STACK)
		 || (s_id003_powerup_stat == POWERUP_STAT_RECOVER_STACK)
		 )
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
		}
		else
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP;
		}
	}
	else if(ex_cline_status_tbl.line_task_mode == ID003_MODE_SIGNATURE_END)
	{
		_id003_tx_buff.cmd = ID003_STS_SIGNATURE_END;
		_id003_tx_buff.data[0] = (u8)(ex_rom_crc16 >> 8);
		_id003_tx_buff.data[1] = (u8)(ex_rom_crc16);
		_id003_tx_buff.length = 3 + 4;
		if(	ex_cline_status_tbl.error_code == 0)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
		}
		else
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ERROR;
		}
	}
	else if(ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_SIGNATURE_END)
	{
		_id003_tx_buff.cmd = ID003_STS_SIGNATURE_END;
		_id003_tx_buff.data[0] = (u8)(ex_rom_crc16 >> 8);
		_id003_tx_buff.data[1] = (u8)(ex_rom_crc16);
		_id003_tx_buff.length = 3 + 4;

		if(s_id003_powerup_stat == POWERUP_STAT_RECOVER_SEARCH_NON)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_ERROR;
		}
		else if(s_id003_powerup_stat == POWERUP_STAT_REJECT)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
		}
		else if((s_id003_powerup_stat == POWERUP_STAT_FEED_STACK)
		 || (s_id003_powerup_stat == POWERUP_STAT_RECOVER_FEED_STACK)
		 || (s_id003_powerup_stat == POWERUP_STAT_RECOVER_STACK)
		 )
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
		}
		else
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP;
		}
	}

	uart_send_id003(&_id003_tx_buff);
}
/*********************************************************************//**
 * @brief Reset Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_reset_cmd_proc(void)
{

	u8 reset_mode;

	reset_mode = 0;

	#if defined(UBA_RTQ)//#if defined(UBA_RS)
	ex_rs_payout_remain_flag == RS_NOTE_REMAIN_NONE;
	#endif

#if defined(UBA_RTQ)		/* '22-02-15 */
	if(((ex_cline_status_tbl.option & ID003_OPTION_CHANGE_PAYVALID_TIMING) == ID003_OPTION_CHANGE_PAYVALID_TIMING)
	//#if defined(UBA_RS)
	|| (is_rc_rs_unit())
	//#endif
	|| ((ex_cline_status_tbl.option & ID003_OPTION_SPRAY_MODE) == ID003_OPTION_SPRAY_MODE))		/* '22-02-10 */
	{
		if(ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_PAYVALID_ACK
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_PAYVALID_ACK_ERROR)
		{
			reset_mode = RESET_TYPE_WAIT_PAYVALID_ACK;
		}
		else if(ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYSTAY
		||		ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYSTAY_WAIT_POLL)
		{
			reset_mode = RESET_TYPE_WAIT_PAYSTAY_POLL;
		}
		else
		{
			reset_mode = RESET_TYPE_NORMAL;
		}
	}
	else
	{
		reset_mode = RESET_TYPE_NORMAL;
	}
#endif

	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
	#if 1//#if defined(UBA_ID003_ENC)
		_secret_number_003();
	#endif
		/* 2023-12-04 最初にクリア */
		ex_cline_status_tbl.error_code = ALARM_CODE_OK;
		ex_cline_status_tbl.reject_code = 0x0000;
		ex_cline_status_tbl.bill_disable = 0;
		ex_cline_status_tbl.security_level = 0x0000;
		ex_cline_status_tbl.accept_disable = 0x0001;
		ex_cline_status_tbl.direction_disable = 0x0000;
	#if defined(UBA_RTQ)//defined(EXTEND_ENCRYPTION_PAY)
		ex_cline_status_tbl.option = ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION;
	#else
		ex_cline_status_tbl.option = 0x0000;
	#endif
		ex_cline_status_tbl.barcode_type = 0;
        ex_cline_status_tbl.barcode_length = 0;
		s_id003_2nd_note = 0;
	#if defined(UBA_RTQ)
		//#if defined(ID003_SPECK64)
		ex_illigal_payout_command = 0;
		//#endif
		OperationDenomi.mode = 0;
		RecycleSettingInfo.key = 0;
		ex_rc_internal_jam_flag_bk = 0;
		ex_rc_data_lock = 0;		

		/* 2024-09-25 - Concurrent Dispensing from Multiple-payout command (M_Payout) */
		clearConcurrentPayout();

	#endif

		if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP)
		 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_AT)
		 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_SK)
		 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR))
		{
			#if defined(UBA_RTQ) /* 2025/03/07 */
			/* パワーリカバリー場合、CHEATにする。 */
			if( s_id003_powerup_stat == POWERUP_STAT_RECOVER_CHEAT)
			{
				_id003_send_host_ack();
				set_recovery_step(RECOVERY_STEP_NON);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
				return;
			}
			#endif // UBA_RTQ

		#if !defined(UBA_RTQ) //2023-12-04	//2023-12-04
			if ((s_id003_powerup_stat != POWERUP_STAT_RECOVER_FEED_STACK)
			 && (s_id003_powerup_stat != POWERUP_STAT_RECOVER_STACK)
			)
			{
				ex_cline_status_tbl.escrow_code = 0x0000;
			}

			if( ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR && ex_error_003_gli == 1 )
			{
				if( s_id003_illegal_credit == 1 )
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL;
					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, reset_mode, 0, 0, 0); //2023-12-04
				}
				else
				{
					/* Set Error status */
					_set_id003_alarm(ID003_MODE_ERROR, ALARM_CODE_FEED_TIMEOUT_SK);	/* Stacker JAM */
					/* スタッカーJAM送信前なので、無視する*/

				}
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL;
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, reset_mode, 0, 0, 0); //2023-12-04
			}
		#else
			if ((s_id003_powerup_stat != POWERUP_STAT_RECOVER_FEED_STACK)
			 && (s_id003_powerup_stat != POWERUP_STAT_RECOVER_STACK)
			)
			{
				ex_cline_status_tbl.escrow_code = 0x0000;
			}
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL;
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, reset_mode, 0, 0, 0);
		#endif

		}
		else if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
		 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL))
		{
			// イニシャル動作中なのでACKを返してmainにイニシャル動作中なので要求をしない
		}
		else if (ex_cline_status_tbl.line_task_mode == ID003_MODE_SYSTEM_ERROR)
		{
			// テストモードによる要調整なのでACKを返してmain要求をしない
		}
		else
		{
		#if !defined(UBA_RTQ) //2025-04-08
			if (ex_recovery_info.step != RECOVERY_STEP_NON)
			{
				set_recovery_step(RECOVERY_STEP_NON);
			}
			ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL;
			ex_cline_status_tbl.escrow_code = 0x0000;

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, reset_mode, 0, 0, 0);
		#else //RTQ
			if( (ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_INITIAL_PAYVALID)     &&
			(ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK) &&
			(ex_cline_status_tbl.line_task_mode != ID003_MODE_PAYOUT)                       &&
			(ex_cline_status_tbl.line_task_mode != ID003_MODE_PAYSTAY)                      &&
			(ex_cline_status_tbl.line_task_mode != ID003_MODE_PAYVALID)                     &&
			(ex_cline_status_tbl.line_task_mode != ID003_MODE_PAYSTAY_WAIT_POLL)            &&
			(ex_cline_status_tbl.line_task_mode != ID003_MODE_WAIT_PAYVALID_ACK)            &&
			(ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_INITIAL)              &&
			(ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_INITIAL_STACK)        )
			{
				set_recovery_step(RECOVERY_STEP_NON);
			}
			/* Power UPからのイニシャル中はそのままのモードにする */
			/* (リカバリ中にリカバリフラグクリアしないようにするため) */
			if(ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_INITIAL)
			{
				ex_cline_status_tbl.escrow_code = 0x0000;
				//ex_cline_status_tbl.vend_type_code = 0;	/* 2019-11-21 */
				ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL;
			}
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, reset_mode, 0, 0, 0);
		#endif
		}

        // reset command not initialize barcode_inhibit
        //ex_cline_status_tbl.barcode_inhibit = 0xFE;
		write_status_table();

		_id003_status_wait_clear();

		if (s_id003_illegal_credit != 0)
		{
			s_id003_illegal_credit = 0;
			uart_txd_active_id003();
		}

		_id003_send_host_ack();

		/* change status */
		_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);

	}
	else
	{
		_id003_send_host_invalid();
	}
}

#if defined(UBA_RTQ)
static void set_recycle_total_count(u8 mode, u8 box)
{
	switch (mode)
	{
	case RC_MODE_STACK:
		if (box != 0)
		{
			rcLogdatIF.rcLogIF[box - 1].AcceptCount += 1;
		}
		break;
	case RC_MODE_PAYOUT:
		if (box != 0)
		{
			rcLogdatIF.rcLogIF[box - 1].PayoutCount += 1;
		}
		break;
	case RC_MODE_COLLECT:
		if (box != 0)
		{
			rcLogdatIF.rcLogIF[box - 1].CollectCount += 1;
		}
		break;
	case TOTAL_CLEAR:
		break;
	default:
		break;
	}
	/* Write fram */
	_cline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_RTQ, FRAM_RC_LOG_IF, 0, 0);
}

/*********************************************************************//**
 * @brief response payout (PAYOUT)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if defined(UBA_RTQ)
 static void _id003_rc_payout_cmd_proc(void)  //ok 2025-01-17
{
	u8 unit_no;
	u8 note_cnt;

  /* 2024-09-25 : M_Payout */
	u8 cnt;         
	u8 dataIndx;
	bool m_payout_flag;
	u8 m_total = 0;
  /* --- */

	ex_rs_payout_remain_flag = RS_NOTE_REMAIN_CHECK;
	
	if(_id003_rx_buff.length == (0x05 + ID003_ADD_04) 		/* Normal or Encryption 8-byte */
	|| _id003_rx_buff.length == (0x0D + ID003_ADD_04) 		/* Encryption 16-byte */
    || _id003_rx_buff.length == (0x07 + ID003_ADD_04)       /* M_Payout : 2 Units */
    || _id003_rx_buff.length == (0x09 + ID003_ADD_04)       /* M_Payout : 3 Units */
    || _id003_rx_buff.length == (0x0B + ID003_ADD_04) )     /* M_Payout : 4 Units */
	{
		if(ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
		{
			/* 2024-09-25 - Concurrent Dispensing from Multiple-payout command (M_Payout) */
			if(_id003_rx_buff.length == (0x07 + ID003_ADD_04) || _id003_rx_buff.length == (0x09 + ID003_ADD_04) || _id003_rx_buff.length == (0x0B + ID003_ADD_04) )
			{
				m_payout_flag = true;
			}
			else
			{
				m_payout_flag = false;
			}
		#if defined(A_PRO)	/* '22-02-07 */
			if(!(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN) || (SENSOR_APB_OUT))
		#else
			if((SENSOR_ENTRANCE) || (SENSOR_CENTERING) || !(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN) || (SENSOR_APB_OUT))
		#endif
			{
				_id003_send_host_invalid();

				id003_enc_cancel_update_context();
			}
			else
			{
				//mode1が増えたのでUBA500とは変える
				//if(ex_main_task_mode1 == MODE1_DISABLE)
				if( (ex_main_task_mode1 == MODE1_DISABLE)
					|| (ex_main_task_mode1 == MODE1_ACTIVE_DISABLE)
					|| (ex_main_task_mode1 == MODE1_ADJUST) && (ex_main_task_mode2 == ADJUST_MODE2_DISABLE_TEMP_ADJ)
				)
				{
					note_cnt = _id003_rx_buff.data[2];
					unit_no  = _id003_rx_buff.data[3];

					/* RC-Quad */
					if(ex_rc_status.sst1A.bit.quad)
					{
						/* check payout count */
						if((note_cnt == 0 || note_cnt > 30 || (is_rc_rs_unit() && note_cnt > 10)) && (!m_payout_flag)) /* M_Payout - Check later in function "check_payout_count_recycler()" */
						{
							_id003_send_host_invalid();
							/* 暗号化 コンテキスト更新のキャンセル */
							id003_enc_cancel_update_context();
						}
						/* check rc unit no. */
						else if(_id003_rx_buff.data[3] == 0 || unit_no > 4)
						{
							_id003_send_host_invalid();
							/* 暗号化 コンテキスト更新のキャンセル */
							id003_enc_cancel_update_context();
						}
						/* check not assign recycle denomination */
						else if(RecycleSettingInfo.DenomiInfo[unit_no - 1].Data1 == 0 && RecycleSettingInfo.DenomiInfo[unit_no - 1].Data2 == 0)
						{
							_id003_send_host_invalid();
							/* 暗号化 コンテキスト更新のキャンセル */
							id003_enc_cancel_update_context();
						}
						else if(_id003_rc_enable_check(unit_no) != TRUE)
						{
							_id003_send_host_invalid();
							/* 暗号化 コンテキスト更新のキャンセル */
							id003_enc_cancel_update_context();
						}
						else if(((ex_rc_status.sst31A.bit.u1_d1_empty && unit_no == 1)		/* RC-Twin drum1 is not empty */
							||	 (ex_rc_status.sst31A.bit.u1_d2_empty && unit_no == 2)		/* RC-Twin drum2 is not empty */
							||	 (ex_rc_status.sst32A.bit.u2_d1_empty && unit_no == 3)		/* RC-Quad drum1 is not empty */
							||	 (ex_rc_status.sst32A.bit.u2_d2_empty && unit_no == 4))		/* RC-Quad drum2 is not empty */
							&&  (!m_payout_flag))                                         	/* M_Payout - Check later in function "check_status_recycler()" */
						{
							_id003_send_host_invalid();
							/* 暗号化 コンテキスト更新のキャンセル */
							id003_enc_cancel_update_context();
						}
						else
						{
							OperationDenomi.count	= note_cnt;
							OperationDenomi.unit	= unit_no;

							/* 2024-09-25 - Concurrent Dispensing from Multiple-payout command (M_Payout) */
							if(_id003_rx_buff.length == (0x07 + ID003_ADD_04) || _id003_rx_buff.length == (0x09 + ID003_ADD_04) || _id003_rx_buff.length == (0x0B + ID003_ADD_04))
							{
								memset(OperationDenomiCount, 0, sizeof(OperationDenomiCount));
								memset(OperationDenomiBoxNumber, 0, sizeof(OperationDenomiBoxNumber));

								ConcurrentPayoutFlag = 1;   /* Enable concurrent payout flag */
								CurrentBoxNumberIndx = 0;

								dataIndx = 0;
								for(cnt = 0; cnt < (_id003_rx_buff.length - 3 - ID003_ADD_04) / 2 ; cnt++)
								{
									OperationDenomiCount[cnt]     = _id003_rx_buff.data[dataIndx+2];
									OperationDenomiBoxNumber[cnt] = _id003_rx_buff.data[dataIndx+3];
									m_total += OperationDenomiCount[cnt];
									dataIndx += 2;
								}

								if(is_rc_rs_unit())
								{
									if(m_total > 10)
									{
										clearConcurrentPayout();

										_id003_send_host_invalid();
										/* Encryption Cancel context update */
										id003_enc_cancel_update_context();
										return;
									}
								}

								/* Check abnormal */
								if(check_recieved_data() != TRUE)
								{
									clearConcurrentPayout();

									_id003_send_host_invalid();
									/* Encryption Cancel context update */
									id003_enc_cancel_update_context();
									return;
								}

								check_payout_zero_note();

								/* Update payout count and unit */
								OperationDenomi.count     = OperationDenomiCount[0];
								OperationDenomi.unit      = OperationDenomiBoxNumber[0];
							}
							else
							{
								clearConcurrentPayout();
							}
							/* --- End M_Payout --- */

							/* 出金元保存 */
							set_recovery_unit( OperationDenomi.unit, OperationDenomi.unit );
						//	fid003_change_mode( ID003_MODE_PAYOUT );
							ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYOUT;
							_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);

							OperationDenomi.remain	= OperationDenomi.count;

							s_id003_paying_info = (ID003_PAYING_INFO_BUSY|ID003_PAYING_INFO_WAIT_STS);
							_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_PAYOUT_REQ, 0, 0, 0, 0);
							if(!(is_rc_rs_unit()))
							{
								_cline_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_BLINK, 0, 1, 1, 1);
							}

							_id003_status_wait_clear();
							_id003_send_host_ack();
		                    /* 暗号化 コンテキスト更新 */
							id003_enc_update_context();

						}
					}
					/* RC-Twin */
					else
					{
						/* check payout count */
						if((note_cnt == 0 || note_cnt > 30 || (is_rc_rs_unit() && note_cnt > 10)) && (!m_payout_flag)) /* M_Payout - Check later in function "check_payout_count_recycler()") */
						{
							_id003_send_host_invalid();
							/* 暗号化 コンテキスト更新のキャンセル */
							id003_enc_cancel_update_context();
						}
						/* check rc unit no. */
						else if(unit_no == 0 || unit_no > 2)
						{
							_id003_send_host_invalid();
							/* 暗号化 コンテキスト更新のキャンセル */
							id003_enc_cancel_update_context();
						}
						/* check not assign recycle denomination */
						else if(RecycleSettingInfo.DenomiInfo[unit_no - 1].Data1 == 0 && RecycleSettingInfo.DenomiInfo[unit_no - 1].Data2 == 0)
						{
							_id003_send_host_invalid();
							/* 暗号化 コンテキスト更新のキャンセル */
							id003_enc_cancel_update_context();
						}
						else if(_id003_rc_enable_check(unit_no) != TRUE)
						{
							_id003_send_host_invalid();
							/* 暗号化 コンテキスト更新のキャンセル */
							id003_enc_cancel_update_context();
						}
						else if(((ex_rc_status.sst31A.bit.u1_d1_empty && unit_no == 1)		/* RC-Twin drum1 is not empty */
							||	 (ex_rc_status.sst31A.bit.u1_d2_empty && unit_no == 2))		/* RC-Twin drum2 is not empty */
							&&  (!m_payout_flag))                                       	/* M_Payout - Check later in function "check_status_recycler()" */
						{
							_id003_send_host_invalid();
							/* 暗号化 コンテキスト更新のキャンセル */
							id003_enc_cancel_update_context();
						}
						else
						{
							OperationDenomi.count = note_cnt;
							OperationDenomi.unit  = unit_no;

							/* 2024-09-25 - Concurrent Dispensing from Multiple-payout command (M_Payout) */
							if(_id003_rx_buff.length == (0x07 + ID003_ADD_04) )
							{
								ConcurrentPayoutFlag = 1;   /* Enable concurrent payout flag */
								CurrentBoxNumberIndx = 0;

								memset(OperationDenomiCount, 0, sizeof(OperationDenomiCount));
								memset(OperationDenomiBoxNumber, 0, sizeof(OperationDenomiBoxNumber));

								dataIndx = 0;
								for(cnt = 0; cnt < (_id003_rx_buff.length - 3 - ID003_ADD_04) / 2 ; cnt++)
								{
									OperationDenomiCount[cnt]     = _id003_rx_buff.data[dataIndx+2];
									OperationDenomiBoxNumber[cnt] = _id003_rx_buff.data[dataIndx+3];
									m_total += OperationDenomiCount[cnt];
									dataIndx += 2;
								}

								if(is_rc_rs_unit())
								{
									if(m_total > 10)
									{
										clearConcurrentPayout();

										_id003_send_host_invalid();
										/* Encryption Cancel context update */
										id003_enc_cancel_update_context();
										return;
									}
								}

								/* Check abnormal */
								if(check_recieved_data() != TRUE)
								{
									clearConcurrentPayout();

									_id003_send_host_invalid();
									/* Encryption Cancel context update */
									id003_enc_cancel_update_context();
									return;
								}

								check_payout_zero_note();

								/* Update payout count and unit */
								OperationDenomi.count     = OperationDenomiCount[0];
								OperationDenomi.unit      = OperationDenomiBoxNumber[0];
							}
							else
							{
								clearConcurrentPayout();
 								/* if different 8-byte ecryption -> invalid */
								if(_id003_rx_buff.length != (0x05 + ID003_ADD_04) )
								{
									_id003_send_host_invalid();
									/* Encryption Cancel context update */
									id003_enc_cancel_update_context();
									return;
								}
							}
							/* --- End M_Payout --- */

							/* 出金元保存 */
							set_recovery_unit( OperationDenomi.unit, OperationDenomi.unit );
							OperationDenomi.remain	= OperationDenomi.count;

						//	fid003_change_mode( ID003_MODE_PAYOUT );
							ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYOUT;
							_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);


							s_id003_paying_info = (ID003_PAYING_INFO_BUSY|ID003_PAYING_INFO_WAIT_STS);
							_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_PAYOUT_REQ, 0, 0, 0, 0);
							if(!(is_rc_rs_unit()))
							{
								_cline_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_BLINK, 0, 1, 1, 1);
							}

							_id003_status_wait_clear();
							_id003_send_host_ack();
		                    /* 暗号化 コンテキスト更新 */
							id003_enc_update_context();
						}
					}
				}
				else
				{
					_id003_send_host_invalid();
				}
			}
		}
		else if(ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE
		||		ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT)
		{
			if((SENSOR_ENTRANCE) || (SENSOR_CENTERING) || !(SENSOR_VALIDATION_OFF) || (SENSOR_APB_IN) || (SENSOR_APB_OUT))
			{
				_id003_send_host_invalid();
			}
			else
			{
				_id003_illegal_credit();	// 不正クレジットと判断した
			}
		}
		else
		{
			id003_enc_cancel_update_context();
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
		/* 暗号化 コンテキスト更新のキャンセル */
		id003_enc_cancel_update_context();
	}
}
#endif

/*********************************************************************//**
 * @brief response collect (COLLECT)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _id003_rc_collect_cmd_proc(void) //ok ID-003コマンドでの回収コマンド受信
{
	//2025-01-17
	u8 unit_no;
	u8 collect_mode;

	ex_rs_payout_remain_flag = RS_NOTE_REMAIN_NONE;

	//if(_id003_rx_buff.length == 0x05)
	if(_id003_rx_buff.length == (0x05 + ID003_ADD_04))
	{
		if(ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
		{
			collect_mode = _id003_rx_buff.data[2];
			unit_no		 = _id003_rx_buff.data[3];

			/* RC-Quad */
			if(ex_rc_status.sst1A.bit.quad)
			{
				/* check collect mode */
				if(collect_mode > 1)
				{
					_id003_send_host_invalid();
				}
				/* check rc unit no. */
				else if(unit_no == 0 || unit_no > 4)
				{
					_id003_send_host_invalid();
				}
				/* check not assign recycle denomination */
				else if(RecycleSettingInfo.DenomiInfo[unit_no - 1].Data1 == 0 && RecycleSettingInfo.DenomiInfo[unit_no - 1].Data2 == 0)
				{
					_id003_send_host_invalid();
				}
				else if(_id003_rc_enable_check(unit_no) != TRUE)
				{
					_id003_send_host_invalid();
				}
				else if((ex_rc_status.sst31A.bit.u1_d1_empty && unit_no == 1)		/* RC-Twin drum1 is not empty */
					||  (ex_rc_status.sst31A.bit.u1_d2_empty && unit_no == 2)		/* RC-Twin drum2 is not empty */
					||  (ex_rc_status.sst32A.bit.u2_d1_empty && unit_no == 3)		/* RC-Quad drum1 is not empty */
					||  (ex_rc_status.sst32A.bit.u2_d2_empty && unit_no == 4))	/* RC-Quad drum2 is not empty */
				{
					_id003_send_host_invalid();
				}
				else
				{
					OperationDenomi.count	= collect_mode;
					OperationDenomi.unit	= unit_no;
					/* 回収元保存 */
					set_recovery_unit( OperationDenomi.unit, OperationDenomi.unit );

					ex_cline_status_tbl.line_task_mode = ID003_MODE_COLLECT;
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);


					s_id003_collecting_info = (ID003_COLLECTING_INFO_BUSY|ID003_COLLECTING_INFO_WAIT_STS);
					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_COLLECT_REQ, 0, 0, 0, 0);

					_id003_status_wait_clear();
					_id003_send_host_ack();

#if defined(A_PRO)	/* '23-06-22 */
					_id003_poll_timeout = 2000;
					_id003_poll_monitor = 1;
					_id003_poll_error = 0;
#endif
				}
			}
			/* RC-Twin */
			else
			{
				/* check collect mode */
				if(collect_mode > 1)
				{
					_id003_send_host_invalid();
				}
				/* check rc unit no. */
				else if(unit_no == 0 || unit_no > 2)
				{
					_id003_send_host_invalid();
				}
				/* check not assign recycle denomination */
				else if(RecycleSettingInfo.DenomiInfo[unit_no - 1].Data1 == 0 && RecycleSettingInfo.DenomiInfo[unit_no - 1].Data2 == 0)
				{
					_id003_send_host_invalid();
				}
				else if(_id003_rc_enable_check(unit_no) != TRUE)
				{
					_id003_send_host_invalid();
				}
				else if((ex_rc_status.sst31A.bit.u1_d1_empty && unit_no == 1)		/* RC-Twin drum1 is not empty */
					||  (ex_rc_status.sst31A.bit.u1_d2_empty && unit_no == 2))		/* RC-Twin drum2 is not empty */
				{
					_id003_send_host_invalid();
				}
				else
				{
					OperationDenomi.count	= collect_mode;
					OperationDenomi.unit	= unit_no;
					/* 回収元保存 */
					set_recovery_unit( OperationDenomi.unit, OperationDenomi.unit );

					ex_cline_status_tbl.line_task_mode = ID003_MODE_COLLECT;
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);

					s_id003_collecting_info = (ID003_COLLECTING_INFO_BUSY|ID003_COLLECTING_INFO_WAIT_STS);
					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_COLLECT_REQ, 0, 0, 0, 0);

					_id003_status_wait_clear();
					_id003_send_host_ack();

				#if defined(A_PRO)	/* '23-06-22 */
					_id003_poll_timeout = 2000;
					_id003_poll_monitor = 1;
					_id003_poll_error = 0;
				#endif
				}
			}
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}

}

static void _id003_rc_key_set_cmd_proc(void) //ok
{
	//2025-01-17
	if (_id003_rx_buff.length == (0x04 + ID003_ADD_04))
	{
		if(ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL				/* power init */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK		/* power init stack */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND		/* power init vend */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK	/* power init vend ack */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT		/* power init reject */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE		/* power init pause */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL						/* init */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK				/* init stack */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT				/* init reject */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE				/* init pause */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE						/* disable */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)				/* disable reject */
		{
			/* set key */
			RecycleSettingInfo.key = (_id003_rx_buff.data[2] & 0x01);

			/* resp to host */
			_id003_send_host_echoback();
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}

/*********************************************************************//**
 * @brief response key request (KEY REQ)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _id003_rc_key_req_cmd_proc() //ok
{
	//2025-01-17
	if(_id003_rx_buff.length == (0x03 + ID003_ADD_04))
	{
		_id003_tx_buff.length	= 4 + ID003_ADD_04;
		_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
	//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
		_is_set_unit_id();
		_id003_tx_buff.data[1]	= ID003_CMD_EXT_RC_KEY_REQ;
		_id003_tx_buff.data[2]	= RecycleSettingInfo.key;
		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}

static void _id003_rc_mc_req_cmd_proc() //ok
{
	//2025-01-17
	u8 *ptr;
	u8 cnt;
	u8 model;

	if(_id003_rx_buff.length == (0x03 + + ID003_ADD_04) )
	{
		_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
	//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
		_is_set_unit_id();
		_id003_tx_buff.data[1]	= ID003_CMD_EXT_RC_MC_REQ;

		ptr = &_id003_tx_buff.data[2];

		/* check model */
		if(ex_rc_status.sst1A.bit.quad)
		{
			_id003_tx_buff.length	= 11 + ID003_ADD_04;
			model = RC_MODEL_QUAD;
		}
		else
		{
			_id003_tx_buff.length	= 7 + ID003_ADD_04;
			model = RC_MODEL_TWIN;
		}

		for(cnt = 0; cnt < model; cnt++)
		{
			*(ptr + 0) = RecycleSettingInfo.DenomiInfo[cnt].Data1;
			*(ptr + 1) = RecycleSettingInfo.DenomiInfo[cnt].Data2;
			ptr += 2;
		}
		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}

static u8 checkbit(u8 data, u8 nbit)
{
	u8 mask = 0x01;

	mask <<= nbit;

	if((data & mask) != 0)
	{
		return(1);
	}
	else
	{
		return(0);
	}
}

static u8 set_recycle_currency(u8 model) //ok
{
	/* ex_line_status_tbl.recycle_denomi_mask1 *///UBA500はバックアップ領域にアサインしているが、起動時にクリアしている
	//その為、UBA500では ex_line_status_tbl.recycle_denomi_mask1の部分をrecycle_denomi_mask1 に置き換えても問題なし

	u8 rCnt, dCnt, bCnt;
	u8 *ptr;
	u8 bit = 0;
	u8 result = FALSE;
	u8 denomi[4][4];

	/* It sets the first address of the data part to the pointer. */
	ptr = &_id003_rx_buff.data[2];

	for (rCnt = 0; rCnt < model; rCnt++)
	{
		/* It confirms Data1, Data2. */
		for (dCnt = 0; dCnt < 2; dCnt++)
		{
			/* 金種確認用に一時保存 */
			denomi[rCnt][dCnt] = *ptr;

			switch (dCnt)
			{
			case 0:
				if (*ptr != 0 && (*ptr & recycle_denomi_mask1) == 0)
				{
					return (FALSE);
				}
				break;
			case 1:
				if (*ptr != 0 && (*ptr & recycle_denomi_mask2) == 0)
				{
					return (FALSE);
				}
				break;
			}

			/* It confirms whether or not the setting denomination doesn't overlap in one Recycle Box. */
			for (bCnt = 0; bCnt < 8; bCnt++)
			{
				result = checkbit(*ptr, bCnt);

				if (result == 1)
				{
					++bit;
				}
			}
			/* When overlapping, it sends a "INVALID" command. */
			if (bit > 1)
			{
				return (FALSE);
			}

			/* It sets the following address. */
			// bit = 0; // 2ヵ国のとき上位ビットと下位ビットで別の国の金種を設定できる不具合を修正(初期化を1BOXごとに変更) [2016-01-25] yuji
			++ptr;
		}
		bit = 0; // [2016-01-25] yuji
		/* It confirms in the number of usable Recycle Box. */
		if (*ptr > model || *ptr != (rCnt + 1))
		{
			return (FALSE);
		}

		/* It sets the following address. */
		++ptr;
	}

	/* 設定金種の確認 */
	for (rCnt = 0; rCnt < model; rCnt++)
	{
		/* Data1, Data2ともに"0"以外が設定されている場合 */
		if (denomi[rCnt][0] != 0 && denomi[rCnt][1] != 0)
		{
			/* Data1とData2を同じ金種としてリサイクルする場合 */
			/* Data1とData2が異なるビットが設定されている場合は無効とする */
			if (denomi[rCnt][0] != denomi[rCnt][1])
			{
				return (FALSE);
			}
			/* Data1とData2を同じ金種としてリサイクルしない場合 */
			/* Data1とData2が同じビットが設定されている場合は無効とする */
			if (denomi[rCnt][0] == denomi[rCnt][1])
			{
				return (FALSE);
			}
		}
	}

	return (TRUE);
}

/*********************************************************************//**
 * @brief response set currency (SET CURRENCY)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _id003_rc_mc_set_cmd_proc() //ok
{

	 //2024-11-04 生産で発生したリサイクル金種が変わる問題調査の為、UBA500と同じ処理にしておく

	u8 result = FALSE;
	u8 *ptr;
	u8 rCnt;
	u8 cnt;
	u8 rtn = 0;

	if(ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL
	|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK
	|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND
	|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK
	|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT
	|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE
	|| ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL
	|| ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK
	|| ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT
	|| ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE
	|| ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE
	|| ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	{
		/* RC-Quad */
		if(ex_rc_status.sst1A.bit.quad)
		{
			if(_id003_rx_buff.length == (0x0F + ID003_ADD_04) )
			{
				/* 何かしら紙幣がドラム内にある場合はInvalid Command応答する */
				if((!(ex_rc_status.sst31A.bit.u1_d1_empty) && !(ex_rc_status.sst31A.bit.u1_d2_empty) && !(ex_rc_status.sst32A.bit.u2_d1_empty) && !(ex_rc_status.sst32A.bit.u2_d2_empty))
				|| (RecycleSettingInfo.DenomiInfo[0].RecycleCurrent != 0 && RecycleSettingInfo.DenomiInfo[1].RecycleCurrent != 0 && RecycleSettingInfo.DenomiInfo[2].RecycleCurrent != 0 && RecycleSettingInfo.DenomiInfo[3].RecycleCurrent != 0))
				{
					/*ただし受信したデータが設定と全く同じ場合は、設定を行わないでEcobackしとく*/
					cnt = 2;

					for(rCnt = 0; rCnt < RC_MODEL_QUAD; rCnt++)
					{
						if(memcmp((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, (u8 *)&_id003_rx_buff.data[cnt], 2) != 0)
						{
							rtn = 1;
						}
						cnt += 3;
					}

					/* 全一致 */
					if(rtn == 0)
					{
						/* Echo Back */
						_id003_send_host_echoback();
					}
					/* 不一致 */
					else
					{
						_id003_send_host_invalid();
					}
				}
				else
				{
					result = set_recycle_currency(RC_MODEL_QUAD);

					if(result == TRUE)
					{
						/* Emptyでないドラムの設定チェック */
						for(rCnt = 0; rCnt < RC_MODEL_QUAD; rCnt++)
						{
							switch(rCnt)
							{
							case	0:		/* drum1 */
									if(memcmp((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, (u8 *)&_id003_rx_buff.data[2], 2) != 0)
									{
										if(!(ex_rc_status.sst31A.bit.u1_d1_empty) || RecycleSettingInfo.DenomiInfo[0].RecycleCurrent != 0)
										{
											_id003_send_host_invalid();
											return;
										}
									}
									break;
							case	1:		/* drum2 */
									if(memcmp((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, (u8 *)&_id003_rx_buff.data[5], 2) != 0)
									{
										if(!(ex_rc_status.sst31A.bit.u1_d2_empty) || RecycleSettingInfo.DenomiInfo[1].RecycleCurrent != 0)
										{
											_id003_send_host_invalid();
											return;
										}
									}
									break;
							case	2:		/* drum3 */
									if(memcmp((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, (u8 *)&_id003_rx_buff.data[8], 2) != 0)
									{
										if(!(ex_rc_status.sst32A.bit.u2_d1_empty) || RecycleSettingInfo.DenomiInfo[2].RecycleCurrent != 0)
										{
											_id003_send_host_invalid();
											return;
										}
									}
									break;
							case	3:		/* drum4 */
									if(memcmp((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, (u8 *)&_id003_rx_buff.data[11], 2) != 0)
									{
										if(!(ex_rc_status.sst32A.bit.u2_d2_empty) || RecycleSettingInfo.DenomiInfo[3].RecycleCurrent != 0)
										{
											_id003_send_host_invalid();
											return;
										}
									}
									break;
							default:
									break;
							}
						}

						/* 受信データをCOUNT SETTINGを保存 */
						ptr = &_id003_rx_buff.data[2];

						for(rCnt = 0; rCnt < RC_MODEL_QUAD; rCnt++)
						{
							switch(rCnt)
							{
							case	0:		/* drum1 */
									if(ex_rc_status.sst31A.bit.u1_d1_empty || RecycleSettingInfo.DenomiInfo[0].RecycleCurrent == 0)
									{
										RecycleSettingInfo.DenomiInfo[rCnt].BoxNumber = rCnt + 1;
										memcpy((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, ptr, 2);
										rc_twin_set_bill_info(rCnt);
									}
									break;
							case	1:		/* drum2 */
									if(ex_rc_status.sst31A.bit.u1_d2_empty || RecycleSettingInfo.DenomiInfo[1].RecycleCurrent == 0)
									{
										RecycleSettingInfo.DenomiInfo[rCnt].BoxNumber = rCnt + 1;
										memcpy((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, ptr, 2);
										rc_twin_set_bill_info(rCnt);
									}
									break;
							case	2:		/* drum3 */
									if(ex_rc_status.sst32A.bit.u2_d1_empty || RecycleSettingInfo.DenomiInfo[2].RecycleCurrent == 0)
									{
										RecycleSettingInfo.DenomiInfo[rCnt].BoxNumber = rCnt + 1;
										memcpy((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, ptr, 2);
										rc_twin_set_bill_info(rCnt);
									}
									break;
							case	3:		/* drum4 */
									if(ex_rc_status.sst32A.bit.u2_d2_empty || RecycleSettingInfo.DenomiInfo[3].RecycleCurrent == 0)
									{
										RecycleSettingInfo.DenomiInfo[rCnt].BoxNumber = rCnt + 1;
										memcpy((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, ptr, 2);
										rc_twin_set_bill_info(rCnt);
									}
									break;
							default:
									break;
							}
							ptr += 3;
						}

						/* Echo Back */
						_id003_send_host_echoback();

						/* リサイクル金種設定時はバックアップも更新 */
						memcpy((u8 *)&RecycleSettingInfo_bk.DenomiInfo[0], (u8 *)&RecycleSettingInfo.DenomiInfo[0], sizeof(RecycleSettingInfo));

			            //_cline_send_msg(ID_MAIN_MBX, TMSG_LINE_RC_INFO_REQ, 0, 0, 0, 0);
						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RC_INFO_REQ, 0, 0, 0, 0);
					}
					else
					{
						_id003_send_host_invalid();
					}
				}
			}
			else
			{
				_id003_send_host_invalid();
			}
		}
		/* RC-Twin */
		else
		{
			if(_id003_rx_buff.length == (0x09 + ID003_ADD_04) )
			{
				if((!(ex_rc_status.sst31A.bit.u1_d1_empty) && !(ex_rc_status.sst31A.bit.u1_d2_empty))
				|| (RecycleSettingInfo.DenomiInfo[0].RecycleCurrent != 0 && RecycleSettingInfo.DenomiInfo[1].RecycleCurrent != 0))
				{
					/*ただし受信したデータが設定と全く同じ場合は、設定を行わないでEcobackしとく*/
					cnt = 2;

					for(rCnt = 0; rCnt < RC_MODEL_TWIN; rCnt++)
					{
						if(memcmp((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, (u8 *)&_id003_rx_buff.data[cnt], 2) != 0)
						{
							rtn = 1;
						}
						cnt += 3;
					}

					/* 全一致 */
					if(rtn == 0)
					{
						/* Echo Back */
						_id003_send_host_echoback();
					}
					/* 不一致 */
					else
					{
						_id003_send_host_invalid();
					}
				}
				else
				{
					result = set_recycle_currency(RC_MODEL_TWIN);

					if(result == TRUE)
					{
						/* Emptyでないドラムの設定チェック */
						for(rCnt = 0; rCnt < RC_MODEL_TWIN; rCnt++)
						{
							switch(rCnt)
							{
							case	0:		/* drum1 */
									if(memcmp((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, (u8 *)&_id003_rx_buff.data[2], 2) != 0)
									{
										if(!(ex_rc_status.sst31A.bit.u1_d1_empty) || RecycleSettingInfo.DenomiInfo[0].RecycleCurrent != 0)
										{
											_id003_send_host_invalid();
											return;
										}
									}
									break;
							case	1:		/* drum2 */
									if(memcmp((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, (u8 *)&_id003_rx_buff.data[5], 2) != 0)
									{
										if(!(ex_rc_status.sst31A.bit.u1_d2_empty) || RecycleSettingInfo.DenomiInfo[1].RecycleCurrent != 0)
										{
											_id003_send_host_invalid();
											return;
										}
									}
									break;
							default:
									break;
							}
						}

						/* 受信データをCOUNT SETTINGを保存 */
						ptr = &_id003_rx_buff.data[2];

						for(rCnt = 0; rCnt < RC_MODEL_TWIN; rCnt++)
						{
							switch(rCnt)
							{
							case	0:		/* drum1 */
									if(ex_rc_status.sst31A.bit.u1_d1_empty || RecycleSettingInfo.DenomiInfo[0].RecycleCurrent == 0)
									{
										RecycleSettingInfo.DenomiInfo[rCnt].BoxNumber = rCnt + 1;
										memcpy((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, ptr, 2);
										rc_twin_set_bill_info(rCnt);
									}
									break;
							case	1:		/* drum2 */
									if(ex_rc_status.sst31A.bit.u1_d2_empty || RecycleSettingInfo.DenomiInfo[1].RecycleCurrent == 0)
									{
										RecycleSettingInfo.DenomiInfo[rCnt].BoxNumber = rCnt + 1;
										memcpy((u8 *)&RecycleSettingInfo.DenomiInfo[rCnt].Data1, ptr, 2);
										rc_twin_set_bill_info(rCnt);
									}
									break;
							default:
									break;
							}
							ptr += 3;
						}

						/* Echo Back */
						_id003_send_host_echoback();

						/* リサイクル金種設定時はバックアップも更新 */
						memcpy((u8 *)&RecycleSettingInfo_bk.DenomiInfo[0], (u8 *)&RecycleSettingInfo.DenomiInfo[0], sizeof(RecycleSettingInfo));
						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RC_INFO_REQ, 0, 0, 0, 0);
					}
					else
					{
						_id003_send_host_invalid();
					}
				}
			}
			else
			{
				_id003_send_host_invalid();
			}
		}
	}
	else
	{
		_id003_send_host_invalid();
	}

}

static u8 _id003_rc_status_error_drum_motor(u8 drum_id)
{
	u8 len = 0;
	if (drum_id == 0)
	{
		return len;
	}

	if (ex_rc_status.sst1A.bit.quad)
	{
		len = 0x12;
		for (u8 i = 0; i < RC_MODEL_QUAD; i++)
		{
			if (i == (drum_id - 1))
			{
				if(_id003_rc_enable_check(drum_id) != TRUE)
				{
					_id003_tx_buff.data[i*3 + 1]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[i*3 + 2]	= 0x00;
					_id003_tx_buff.data[i*3 + 3]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[i*3 + 1]	= ID003_STS_EXT_RC_MOTOR_ERROR;
					_id003_tx_buff.data[i*3 + 2]	= 0x00;
					_id003_tx_buff.data[i*3 + 3]	= 0x00;
				}
			}
			else
			{
				_id003_tx_buff.data[i * 3 + 1] = ID003_STS_EXT_RC_NORMAL;
				memcpy((u8 *)&_id003_tx_buff.data[i * 3 + 2], (u8 *)&RecycleSettingInfo.DenomiInfo[i].Data1, 2);
			}
		}
	}
	else
	{
		len = 0x0C;
		for (u8 i = 0; i < RC_MODEL_TWIN; i++)
		{
			if (i == (drum_id - 1))
			{
				if(_id003_rc_enable_check(drum_id) != TRUE)
				{
					_id003_tx_buff.data[i*3 + 1]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[i*3 + 2]	= 0x00;
					_id003_tx_buff.data[i*3 + 3]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[i*3 + 1]	= ID003_STS_EXT_RC_MOTOR_ERROR;
					_id003_tx_buff.data[i*3 + 2]	= 0x00;
					_id003_tx_buff.data[i*3 + 3]	= 0x00;
				}
			}
			else
			{
				_id003_tx_buff.data[i * 3 + 1] = ID003_STS_EXT_RC_NORMAL;
				memcpy((u8 *)&_id003_tx_buff.data[i * 3 + 2], (u8 *)&RecycleSettingInfo.DenomiInfo[i].Data1, 2);
			}
		}
	}
	return len;
}

static u8 _id003_rc_status_error_drum_jam(u8 drum_id)
{
	u8 len = 0;
	if (drum_id == 0)
	{
		return len;
	}

	if (ex_rc_status.sst1A.bit.quad)
	{
		len = 0x12;
		for (u8 i = 0; i < RC_MODEL_QUAD; i++)
		{
			if (i == (drum_id - 1))
			{
				if(_id003_rc_enable_check(drum_id) != TRUE)
				{
					_id003_tx_buff.data[i*3 + 1]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[i*3 + 2]	= 0x00;
					_id003_tx_buff.data[i*3 + 3]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[i*3 + 1]	= ID003_STS_EXT_RC_JAM;
					_id003_tx_buff.data[i*3 + 2]	= 0x00;
					_id003_tx_buff.data[i*3 + 3]	= 0x00;
				}
			}
			else
			{
				_id003_tx_buff.data[i * 3 + 1] = ID003_STS_EXT_RC_NORMAL;
				memcpy((u8 *)&_id003_tx_buff.data[i * 3 + 2], (u8 *)&RecycleSettingInfo.DenomiInfo[i].Data1, 2);
			}
		}
	}
	else
	{
		len = 0x0C;
		for (u8 i = 0; i < RC_MODEL_TWIN; i++)
		{
			if (i == (drum_id - 1))
			{
				if(_id003_rc_enable_check(drum_id) != TRUE)
				{
					_id003_tx_buff.data[i*3 + 1]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[i*3 + 2]	= 0x00;
					_id003_tx_buff.data[i*3 + 3]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[i*3 + 1]	= ID003_STS_EXT_RC_JAM;
					_id003_tx_buff.data[i*3 + 2]	= 0x00;
					_id003_tx_buff.data[i*3 + 3]	= 0x00;
				}
			}
			else
			{
				_id003_tx_buff.data[i * 3 + 1] = ID003_STS_EXT_RC_NORMAL;
				memcpy((u8 *)&_id003_tx_buff.data[i * 3 + 2], (u8 *)&RecycleSettingInfo.DenomiInfo[i].Data1, 2);
			}
		}
	}
	return len;
}


#if 1 //2025-01-16
static void id_recycler_error_exstatus(void)
{
	u8 boxno;
	u8 len;

	if(ex_rc_status.sst1A.bit.quad)
	{
		_id003_tx_buff.length	= (14 + ID003_ADD_04);
		
		len = 1;
		/* drum1 */
		switch(ex_rc_error_code)
		{
		case	ALARM_CODE_BOOTIF_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_BOOTIF_AREA;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RC_IF_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_IF_AREA;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
 				break;
		case	ALARM_CODE_FRAM_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FRAM_AREA;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_ILLIGAL_COMMAND:
		case	ALARM_CODE_ILLIGAL_OPERATION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FEED1_TIMEOUT:
				if((ex_rc_status.sst1B.bit.stat_bit0)
				&& (OperationDenomi.unit == 1 || OperationDenomi.unit == 2))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_TIMEOUT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
				}
				break;
		case	ALARM_CODE_FLAP1_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP1_TIMEOUT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_TWIN_DRUM1_TIMEOUT:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM1_TIMEOUT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_MOTOR_LOCK:
				if((ex_rc_status.sst1B.bit.stat_bit0)
				&& (OperationDenomi.unit == 1 || OperationDenomi.unit == 2))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_MOTOR_LOCK;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
				}
				break;
		case	ALARM_CODE_FLAP1_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP1_MOTOR_LOCK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FLAP1_LEVER_FAIL:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP1_LEVER_FAIL;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_TWIN_DRUM1_MOTOR_LOCK:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM1_MOTOR_LOCK;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_JAM_AT_TR:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_JAM_AT_TR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FEED1_JAM_AT_DR:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_JAM_AT_DR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
				}
				break;
		case	ALARM_CODE_TWIN_DRUM1_JAM:
		case	ALARM_CODE_TWIN_DRUM1_JAM_IN:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM1_JAM;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_SPEED_CHECK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_SPEED_CHECK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_TWIN_DRUM1_SPEED_SEL:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM1_SPEED_SEL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_TWIN_DRUM1_FULL:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM1_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_TWIN_DRUM1_EMPTY:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM1_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_DOUBLE_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_DOUBLE_BILL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED1_SHORT_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_SHORT_BILL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
				}
				break;
		case	ALARM_CODE_RS_FLAP_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_TIMEOUT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RS_FLAP_LEVER_FAIL:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_LEVER_FAIL;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RS_FLAP_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_MOTOR_LOCK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RFID_UNIT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_RFID_UNIT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RFID_ICB_COMMUNICTION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_RFID_COMM_ERROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		default:
				if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				/* RC disable */
				else if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
				}
				break;
		}
		len += 3;
		/* drum2 */
		switch(ex_rc_error_code)
		{
		case	ALARM_CODE_BOOTIF_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_BOOTIF_AREA;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RC_IF_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_IF_AREA;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FRAM_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FRAM_AREA;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_ILLIGAL_COMMAND:
		case	ALARM_CODE_ILLIGAL_OPERATION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FEED1_TIMEOUT:
				if((ex_rc_status.sst1B.bit.stat_bit0)
				&& (OperationDenomi.unit == 1 || OperationDenomi.unit == 2))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_TIMEOUT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
				}
				break;
		case	ALARM_CODE_FLAP1_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP1_TIMEOUT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_TWIN_DRUM2_TIMEOUT:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM2_TIMEOUT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_MOTOR_LOCK:
				if((ex_rc_status.sst1B.bit.stat_bit0)
				&& (OperationDenomi.unit == 1 || OperationDenomi.unit == 2))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_MOTOR_LOCK;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
				}
				break;
		case	ALARM_CODE_FLAP1_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP1_MOTOR_LOCK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FLAP1_LEVER_FAIL:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP1_LEVER_FAIL;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_TWIN_DRUM2_MOTOR_LOCK:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM2_MOTOR_LOCK;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_JAM_AT_TR:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_JAM_AT_TR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FEED1_JAM_AT_DR:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_JAM_AT_DR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
				}
				break;
		case	ALARM_CODE_TWIN_DRUM2_JAM:
		case	ALARM_CODE_TWIN_DRUM2_JAM_IN:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM2_JAM;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_SPEED_CHECK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_SPEED_CHECK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_TWIN_DRUM2_SPEED_SEL:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM2_SPEED_SEL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_TWIN_DRUM2_FULL:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM2_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_TWIN_DRUM2_EMPTY:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM2_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_DOUBLE_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_DOUBLE_BILL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED1_SHORT_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_SHORT_BILL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
				}
				break;
		case	ALARM_CODE_RS_FLAP_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_TIMEOUT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RS_FLAP_LEVER_FAIL:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_LEVER_FAIL;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RS_FLAP_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_MOTOR_LOCK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RFID_UNIT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_RFID_UNIT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RFID_ICB_COMMUNICTION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_RFID_COMM_ERROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		default:
				if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				/* RC disable */
				else if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
				}
				break;
		}
		len += 3;
		/* drum3 */
		switch(ex_rc_error_code)
		{
		case	ALARM_CODE_BOOTIF_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_BOOTIF_AREA;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RC_IF_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_IF_AREA;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FRAM_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FRAM_AREA;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_ILLIGAL_COMMAND:
		case	ALARM_CODE_ILLIGAL_OPERATION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FEED2_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_TIMEOUT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FLAP2_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP2_TIMEOUT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_QUAD_DRUM1_TIMEOUT:
				/* RC disable */
				if(_id003_rc_enable_check(3) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_QUAD_DRUM1_TIMEOUT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED2_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_MOTOR_LOCK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FLAP2_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP2_MOTOR_LOCK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FLAP2_LEVER_FAIL:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP2_LEVER_FAIL;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_QUAD_DRUM1_MOTOR_LOCK:
				/* RC disable */
				if(_id003_rc_enable_check(3) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_QUAD_DRUM1_MOTOR_LOCK;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED2_JAM_AT_TR:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_JAM_AT_TR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FEED2_JAM_AT_DR:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_JAM_AT_DR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[2].Data1, 2);
				}
				break;
		case	ALARM_CODE_QUAD_DRUM1_JAM:
		case	ALARM_CODE_QUAD_DRUM1_JAM_IN:
				/* RC disable */
				if(_id003_rc_enable_check(3) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_QUAD_DRUM1_JAM;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED2_SPEED_CHECK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_SPEED_CHECK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_QUAD_DRUM1_SPEED_SEL:
				/* RC disable */
				if(_id003_rc_enable_check(3) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_QUAD_DRUM1_SPEED_SEL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_QUAD_DRUM1_FULL:
				/* RC disable */
				if(_id003_rc_enable_check(3) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_QUAD_DRUM1_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_QUAD_DRUM1_EMPTY:
				/* RC disable */
				if(_id003_rc_enable_check(3) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_QUAD_DRUM1_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_TIMEOUT:
				if((ex_rc_status.sst1B.bit.stat_bit0)
				&& (OperationDenomi.unit == 3 || OperationDenomi.unit == 4))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_TIMEOUT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[2].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED1_JAM_AT_DR:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_JAM_AT_DR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[2].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED1_MOTOR_LOCK:
				if((ex_rc_status.sst1B.bit.stat_bit0)
				&& (OperationDenomi.unit == 3 || OperationDenomi.unit == 4))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_MOTOR_LOCK;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[2].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED1_DOUBLE_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_DOUBLE_BILL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[2].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED1_SHORT_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_SHORT_BILL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[2].Data1, 2);
				}
				break;

		case	ALARM_CODE_FEED2_DOUBLE_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_DOUBLE_BILL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[2].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED2_SHORT_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_SHORT_BILL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[2].Data1, 2);
				}
				break;
		case	ALARM_CODE_RS_FLAP_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_TIMEOUT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RS_FLAP_LEVER_FAIL:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_LEVER_FAIL;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RS_FLAP_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_MOTOR_LOCK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RFID_UNIT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_RFID_UNIT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RFID_ICB_COMMUNICTION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_RFID_COMM_ERROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		default:
				if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				/* RC disable */
				else if(_id003_rc_enable_check(3) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[2].Data1, 2);
				}
				break;
		}
		len += 3;
		/* drum4 */
		switch(ex_rc_error_code)
		{
		case	ALARM_CODE_BOOTIF_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_BOOTIF_AREA;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RC_IF_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_IF_AREA;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FRAM_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FRAM_AREA;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_ILLIGAL_COMMAND:
		case	ALARM_CODE_ILLIGAL_OPERATION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FEED2_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_TIMEOUT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FLAP2_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP2_TIMEOUT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_QUAD_DRUM2_TIMEOUT:
				/* RC disable */
				if(_id003_rc_enable_check(4) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_QUAD_DRUM2_TIMEOUT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED2_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_MOTOR_LOCK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FLAP2_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP2_MOTOR_LOCK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FLAP2_LEVER_FAIL:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP2_LEVER_FAIL;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_QUAD_DRUM2_MOTOR_LOCK:
				/* RC disable */
				if(_id003_rc_enable_check(4) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_QUAD_DRUM2_MOTOR_LOCK;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED2_JAM_AT_TR:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_JAM_AT_TR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FEED2_JAM_AT_DR:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_JAM_AT_DR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[3].Data1, 2);
				}
				break;
		case	ALARM_CODE_QUAD_DRUM2_JAM:
		case	ALARM_CODE_QUAD_DRUM2_JAM_IN:
				/* RC disable */
				if(_id003_rc_enable_check(4) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_QUAD_DRUM2_JAM;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED2_SPEED_CHECK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_SPEED_CHECK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_QUAD_DRUM2_SPEED_SEL:
				/* RC disable */
				if(_id003_rc_enable_check(4) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_QUAD_DRUM2_SPEED_SEL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_QUAD_DRUM2_FULL:
				/* RC disable */
				if(_id003_rc_enable_check(4) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_QUAD_DRUM2_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_QUAD_DRUM2_EMPTY:
				/* RC disable */
				if(_id003_rc_enable_check(4) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_QUAD_DRUM2_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_TIMEOUT:
				if((ex_rc_status.sst1B.bit.stat_bit0)
				&& (OperationDenomi.unit == 3 || OperationDenomi.unit == 4))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_TIMEOUT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[3].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED1_JAM_AT_DR:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_JAM_AT_DR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[3].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED1_MOTOR_LOCK:
				if((ex_rc_status.sst1B.bit.stat_bit0)
				&& (OperationDenomi.unit == 3 || OperationDenomi.unit == 4))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_MOTOR_LOCK;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[3].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED1_DOUBLE_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_DOUBLE_BILL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[3].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED1_SHORT_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_SHORT_BILL;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[3].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED2_DOUBLE_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_DOUBLE_BILL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[3].Data1, 2);
				}
				break;
		case	ALARM_CODE_FEED2_SHORT_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED2_SHORT_BILL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[3].Data1, 2);
				}
				break;
		case	ALARM_CODE_RS_FLAP_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_TIMEOUT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RS_FLAP_LEVER_FAIL:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_LEVER_FAIL;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RS_FLAP_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_MOTOR_LOCK;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RFID_UNIT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_RFID_UNIT;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RFID_ICB_COMMUNICTION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_RFID_COMM_ERROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		default:
				if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				/* RC disable */
				else if(_id003_rc_enable_check(4) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[3].Data1, 2);
				}
				break;
		}
		len += 3;
		
	}
	/* RC-Twin model */
	else
	{
		_id003_tx_buff.length	= (4 + ID003_ADD_04);
		len = 1;
		/* drum1 */
		switch(ex_rc_error_code)
		{
		case	ALARM_CODE_BOOTIF_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_BOOTIF_AREA;
				break;
		case	ALARM_CODE_RC_IF_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_IF_AREA;
				break;
		case	ALARM_CODE_FRAM_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FRAM_AREA;
				break;
		case	ALARM_CODE_ILLIGAL_COMMAND:
		case	ALARM_CODE_ILLIGAL_OPERATION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				break;
		case	ALARM_CODE_FEED1_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_TIMEOUT;
				break;
		case	ALARM_CODE_FLAP1_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP1_TIMEOUT;
				break;
		case	ALARM_CODE_FLAP1_LEVER_FAIL:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP1_LEVER_FAIL;
				break;
		case	ALARM_CODE_TWIN_DRUM1_TIMEOUT:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM1_TIMEOUT;
				}
				break;
		case	ALARM_CODE_FEED1_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_MOTOR_LOCK;
				break;
		case	ALARM_CODE_FLAP1_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP1_MOTOR_LOCK;
				break;
		case	ALARM_CODE_TWIN_DRUM1_MOTOR_LOCK:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM1_MOTOR_LOCK;
				}
				break;
		case	ALARM_CODE_FEED1_JAM_AT_TR:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_JAM_AT_TR;
				break;
		case	ALARM_CODE_FEED1_JAM_AT_DR:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_JAM_AT_DR;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
					len += 2;
				}
				break;
		case	ALARM_CODE_TWIN_DRUM1_JAM:
		case	ALARM_CODE_TWIN_DRUM1_JAM_IN:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM1_JAM;
				}
				break;
		case	ALARM_CODE_FEED1_SPEED_CHECK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_SPEED_CHECK;
				break;
		case	ALARM_CODE_TWIN_DRUM1_SPEED_SEL:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM1_SPEED_SEL;
				}
				break;
		case	ALARM_CODE_TWIN_DRUM1_FULL:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM1_FULL;
				}
				break;
		case	ALARM_CODE_TWIN_DRUM1_EMPTY:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM1_EMPTY;
				}
				break;
		case	ALARM_CODE_FEED1_DOUBLE_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_DOUBLE_BILL;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
					len += 2;
				}
				break;
		case	ALARM_CODE_FEED1_SHORT_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_SHORT_BILL;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
					len += 2;
				}
				break;
		case	ALARM_CODE_RS_FLAP_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_TIMEOUT;
				break;
		case	ALARM_CODE_RS_FLAP_LEVER_FAIL:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_LEVER_FAIL;
				break;
		case	ALARM_CODE_RS_FLAP_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_MOTOR_LOCK;
				break;
		case	ALARM_CODE_RFID_UNIT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_RFID_UNIT;
				break;
		case	ALARM_CODE_RFID_ICB_COMMUNICTION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_RFID_COMM_ERROR;
				break;
		default:
				if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				/* RC disable */
				else if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
					len += 2;
				}
				break;
		}
		len += 1;
		/* drum2 */
		switch(ex_rc_error_code)
		{
		case	ALARM_CODE_BOOTIF_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_BOOTIF_AREA;
				break;
		case	ALARM_CODE_RC_IF_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_IF_AREA;
				break;
		case	ALARM_CODE_FRAM_AREA:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FRAM_AREA;
				break;
		case	ALARM_CODE_ILLIGAL_COMMAND:
		case	ALARM_CODE_ILLIGAL_OPERATION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				break;
		case	ALARM_CODE_FEED1_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_TIMEOUT;
				break;
		case	ALARM_CODE_FLAP1_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP1_TIMEOUT;
				break;
		case	ALARM_CODE_TWIN_DRUM2_TIMEOUT:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM2_TIMEOUT;
				}
				break;
		case	ALARM_CODE_FEED1_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_MOTOR_LOCK;
				break;
		case	ALARM_CODE_FLAP1_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP1_MOTOR_LOCK;
				break;
		case	ALARM_CODE_FLAP1_LEVER_FAIL:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FLAP1_LEVER_FAIL;
				break;
		case	ALARM_CODE_TWIN_DRUM2_MOTOR_LOCK:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM2_MOTOR_LOCK;
				}
				break;
		case	ALARM_CODE_FEED1_JAM_AT_TR:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_JAM_AT_TR;
				break;
		case	ALARM_CODE_FEED1_JAM_AT_DR:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_JAM_AT_DR;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
					len += 2;
				}
				break;
		case	ALARM_CODE_TWIN_DRUM2_JAM:
		case	ALARM_CODE_TWIN_DRUM2_JAM_IN:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM2_JAM;
				}
				break;
		case	ALARM_CODE_FEED1_SPEED_CHECK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_SPEED_CHECK;
				break;
		case	ALARM_CODE_TWIN_DRUM2_SPEED_SEL:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM2_SPEED_SEL;
				}
				break;
		case	ALARM_CODE_TWIN_DRUM2_FULL:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM2_FULL;
				}
				break;
		case	ALARM_CODE_TWIN_DRUM2_EMPTY:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_TWIN_DRUM2_EMPTY;
				}
				break;
		case	ALARM_CODE_FEED1_DOUBLE_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_DOUBLE_BILL;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
					len += 2;
				}
				break;
		case	ALARM_CODE_FEED1_SHORT_BILL:
				if(ex_rc_status.sst1B.bit.stat_bit0 && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FEED1_SHORT_BILL;
				}
				else if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
					len += 2;
				}
				break;
		case	ALARM_CODE_RS_FLAP_TIMEOUT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_TIMEOUT;
				break;
		case	ALARM_CODE_RS_FLAP_LEVER_FAIL:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_LEVER_FAIL;
				break;
		case	ALARM_CODE_RS_FLAP_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RS_FLAP_MOTOR_LOCK;
				break;
		case	ALARM_CODE_RFID_UNIT:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_RFID_UNIT;
				break;
		case	ALARM_CODE_RFID_ICB_COMMUNICTION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_RFID_COMM_ERROR;
				break;
		default:
				if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				/* RC disable */
				else if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
					len += 2;
				}
				break;
		}
//		len += 1;
		_id003_tx_buff.length	= len + 2 + ID003_ADD_04;
	}
}

#else
static void _id003_rc_exStatus_error_proc() //id_recycler_error_exstatus
{
	
}
#endif

#if 1 //2025-01-16
static void id_recycler_error_status(void)
{
	u8 boxno;
	u8 len;

	/* RC-Quad model */
	if(ex_rc_status.sst1A.bit.quad)
	{
		_id003_tx_buff.length	= 14 + ID003_ADD_04;

		len = 1;

		/* drum1 */
		switch(ex_rc_error_code)
		{
		case	ALARM_CODE_BOOTIF_AREA:
		case	ALARM_CODE_RC_IF_AREA:
		case	ALARM_CODE_FRAM_AREA:
		case	ALARM_CODE_ILLIGAL_COMMAND:
		case	ALARM_CODE_ILLIGAL_OPERATION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FEED1_TIMEOUT:
		case	ALARM_CODE_FEED1_JAM_AT_TR:
		case	ALARM_CODE_FEED1_JAM_AT_DR:
		case	ALARM_CODE_FEED2_TIMEOUT:
		case	ALARM_CODE_FEED2_JAM_AT_TR:
		case	ALARM_CODE_FEED2_JAM_AT_DR:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_JAM;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_TWIN_DRUM1_TIMEOUT:
		case	ALARM_CODE_TWIN_DRUM1_JAM:
		case	ALARM_CODE_TWIN_DRUM1_JAM_IN:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_JAM;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_MOTOR_LOCK:
		case	ALARM_CODE_FEED1_SPEED_CHECK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_MOTOR_ERROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_TWIN_DRUM1_MOTOR_LOCK:
		case	ALARM_CODE_TWIN_DRUM1_SPEED_SEL:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_MOTOR_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FLAP1_TIMEOUT:
		case	ALARM_CODE_FLAP1_LEVER_FAIL:
		case	ALARM_CODE_FLAP1_MOTOR_LOCK:
		case	ALARM_CODE_FLAP2_TIMEOUT:
		case	ALARM_CODE_FLAP2_LEVER_FAIL:
		case	ALARM_CODE_FLAP2_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RS_FLAP_TIMEOUT:
		case	ALARM_CODE_RS_FLAP_LEVER_FAIL:
		case	ALARM_CODE_RS_FLAP_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RFID_UNIT:
		case	ALARM_CODE_RFID_ICB_COMMUNICTION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		default:
				if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				/* RC disable */
				else if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
				}
				break;
		}
		len += 3;

		/* drum2 */
		switch(ex_rc_error_code)
		{
		case	ALARM_CODE_BOOTIF_AREA:
		case	ALARM_CODE_RC_IF_AREA:
		case	ALARM_CODE_FRAM_AREA:
		case	ALARM_CODE_ILLIGAL_COMMAND:
		case	ALARM_CODE_ILLIGAL_OPERATION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FEED1_TIMEOUT:
		case	ALARM_CODE_FEED1_JAM_AT_TR:
		case	ALARM_CODE_FEED1_JAM_AT_DR:
		case	ALARM_CODE_FEED2_TIMEOUT:
		case	ALARM_CODE_FEED2_JAM_AT_TR:
		case	ALARM_CODE_FEED2_JAM_AT_DR:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_JAM;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_TWIN_DRUM2_TIMEOUT:
		case	ALARM_CODE_TWIN_DRUM2_JAM:
		case	ALARM_CODE_TWIN_DRUM2_JAM_IN:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_JAM;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_MOTOR_LOCK:
		case	ALARM_CODE_FEED1_SPEED_CHECK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_MOTOR_ERROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_TWIN_DRUM2_MOTOR_LOCK:
		case	ALARM_CODE_TWIN_DRUM2_SPEED_SEL:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_MOTOR_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FLAP1_TIMEOUT:
		case	ALARM_CODE_FLAP1_LEVER_FAIL:
		case	ALARM_CODE_FLAP1_MOTOR_LOCK:
		case	ALARM_CODE_FLAP2_TIMEOUT:
		case	ALARM_CODE_FLAP2_LEVER_FAIL:
		case	ALARM_CODE_FLAP2_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RS_FLAP_TIMEOUT:
		case	ALARM_CODE_RS_FLAP_LEVER_FAIL:
		case	ALARM_CODE_RS_FLAP_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RFID_UNIT:
		case	ALARM_CODE_RFID_ICB_COMMUNICTION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		default:
				if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				/* RC disable */
				else if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
				}
				break;
		}
		len += 3;

		/* drum3 */
		switch(ex_rc_error_code)
		{
		case	ALARM_CODE_BOOTIF_AREA:
		case	ALARM_CODE_RC_IF_AREA:
		case	ALARM_CODE_FRAM_AREA:
		case	ALARM_CODE_ILLIGAL_COMMAND:
		case	ALARM_CODE_ILLIGAL_OPERATION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FEED1_TIMEOUT:
		case	ALARM_CODE_FEED1_JAM_AT_TR:
		case	ALARM_CODE_FEED1_JAM_AT_DR:
		case	ALARM_CODE_FEED2_TIMEOUT:
		case	ALARM_CODE_FEED2_JAM_AT_TR:
		case	ALARM_CODE_FEED2_JAM_AT_DR:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_JAM;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_QUAD_DRUM1_TIMEOUT:
		case	ALARM_CODE_QUAD_DRUM1_JAM:
		case	ALARM_CODE_QUAD_DRUM1_JAM_IN:
				/* RC disable */
				if(_id003_rc_enable_check(3) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_JAM;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_MOTOR_LOCK:
		case	ALARM_CODE_FEED2_MOTOR_LOCK:
		case	ALARM_CODE_FEED2_SPEED_CHECK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_MOTOR_ERROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_QUAD_DRUM1_MOTOR_LOCK:
		case	ALARM_CODE_QUAD_DRUM1_SPEED_SEL:
				/* RC disable */
				if(_id003_rc_enable_check(3) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_MOTOR_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FLAP1_TIMEOUT:
		case	ALARM_CODE_FLAP1_LEVER_FAIL:
		case	ALARM_CODE_FLAP1_MOTOR_LOCK:
		case	ALARM_CODE_FLAP2_TIMEOUT:
		case	ALARM_CODE_FLAP2_LEVER_FAIL:
		case	ALARM_CODE_FLAP2_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RS_FLAP_TIMEOUT:
		case	ALARM_CODE_RS_FLAP_LEVER_FAIL:
		case	ALARM_CODE_RS_FLAP_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RFID_UNIT:
		case	ALARM_CODE_RFID_ICB_COMMUNICTION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		default:
				if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				/* RC disable */
				else if(_id003_rc_enable_check(3) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 3)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[2].Data1, 2);
				}
				break;
		}
		len += 3;

		/* drum4 */
		switch(ex_rc_error_code)
		{
		case	ALARM_CODE_BOOTIF_AREA:
		case	ALARM_CODE_RC_IF_AREA:
		case	ALARM_CODE_FRAM_AREA:
		case	ALARM_CODE_ILLIGAL_COMMAND:
		case	ALARM_CODE_ILLIGAL_OPERATION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_FEED1_TIMEOUT:
		case	ALARM_CODE_FEED1_JAM_AT_TR:
		case	ALARM_CODE_FEED1_JAM_AT_DR:
		case	ALARM_CODE_FEED2_TIMEOUT:
		case	ALARM_CODE_FEED2_JAM_AT_TR:
		case	ALARM_CODE_FEED2_JAM_AT_DR:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_JAM;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_QUAD_DRUM2_TIMEOUT:
		case	ALARM_CODE_QUAD_DRUM2_JAM:
		case	ALARM_CODE_QUAD_DRUM2_JAM_IN:
				/* RC disable */
				if(_id003_rc_enable_check(4) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_JAM;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FEED1_MOTOR_LOCK:
		case	ALARM_CODE_FEED2_MOTOR_LOCK:
		case	ALARM_CODE_FEED2_SPEED_CHECK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_MOTOR_ERROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_QUAD_DRUM2_MOTOR_LOCK:
		case	ALARM_CODE_QUAD_DRUM2_SPEED_SEL:
				/* RC disable */
				if(_id003_rc_enable_check(4) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_MOTOR_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				break;
		case	ALARM_CODE_FLAP1_TIMEOUT:
		case	ALARM_CODE_FLAP1_LEVER_FAIL:
		case	ALARM_CODE_FLAP1_MOTOR_LOCK:
		case	ALARM_CODE_FLAP2_TIMEOUT:
		case	ALARM_CODE_FLAP2_LEVER_FAIL:
		case	ALARM_CODE_FLAP2_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RS_FLAP_TIMEOUT:
		case	ALARM_CODE_RS_FLAP_LEVER_FAIL:
		case	ALARM_CODE_RS_FLAP_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		case	ALARM_CODE_RFID_UNIT:
		case	ALARM_CODE_RFID_ICB_COMMUNICTION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[len + 1]	= 0x00;
				_id003_tx_buff.data[len + 2]	= 0x00;
				break;
		default:
				if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				/* RC disable */
				else if(_id003_rc_enable_check(4) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 4)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else if(ex_rc_status.sst32A.bit.u2_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[3].Data1, 2);
				}
				break;
		}
		len += 3;
	}
	/* RC-Twin model */
	else
	{
		_id003_tx_buff.length	= 4 + ID003_ADD_04;

		len = 1;

		/* drum1 */
		switch(ex_rc_error_code)
		{
		case	ALARM_CODE_BOOTIF_AREA:
		case	ALARM_CODE_RC_IF_AREA:
		case	ALARM_CODE_FRAM_AREA:
		case	ALARM_CODE_ILLIGAL_COMMAND:
		case	ALARM_CODE_ILLIGAL_OPERATION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				break;
		case	ALARM_CODE_FEED1_TIMEOUT:
		case	ALARM_CODE_FEED1_JAM_AT_TR:
		case	ALARM_CODE_FEED1_JAM_AT_DR:
		case	ALARM_CODE_FEED2_TIMEOUT:
		case	ALARM_CODE_FEED2_JAM_AT_TR:
		case	ALARM_CODE_FEED2_JAM_AT_DR:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_JAM;
				break;
		case	ALARM_CODE_TWIN_DRUM1_TIMEOUT:
		case	ALARM_CODE_TWIN_DRUM1_JAM:
		case	ALARM_CODE_TWIN_DRUM1_JAM_IN:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_JAM;
				}
				break;
		case	ALARM_CODE_FEED1_MOTOR_LOCK:
		case	ALARM_CODE_FEED2_MOTOR_LOCK:
		case	ALARM_CODE_FEED1_SPEED_CHECK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_MOTOR_ERROR;
				break;
		case	ALARM_CODE_TWIN_DRUM1_MOTOR_LOCK:
		case	ALARM_CODE_TWIN_DRUM1_SPEED_SEL:
				/* RC disable */
				if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_MOTOR_ERROR;
				}
				break;
		case	ALARM_CODE_FLAP1_TIMEOUT:
		case	ALARM_CODE_FLAP1_LEVER_FAIL:
		case	ALARM_CODE_FLAP1_MOTOR_LOCK:
		case	ALARM_CODE_FLAP2_TIMEOUT:
		case	ALARM_CODE_FLAP2_LEVER_FAIL:
		case	ALARM_CODE_FLAP2_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				break;
		case	ALARM_CODE_RS_FLAP_TIMEOUT:
		case	ALARM_CODE_RS_FLAP_LEVER_FAIL:
		case	ALARM_CODE_RS_FLAP_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				break;
		case	ALARM_CODE_RFID_UNIT:
		case	ALARM_CODE_RFID_ICB_COMMUNICTION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				break;
		default:
				if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				/* RC disable */
				else if(_id003_rc_enable_check(1) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
				}
				else if(ex_rc_status.sst31A.bit.u1_d1_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
					len += 2;
				}
				break;
		}
		len += 1;

		/* drum2 */
		switch(ex_rc_error_code)
		{
		case	ALARM_CODE_BOOTIF_AREA:
		case	ALARM_CODE_RC_IF_AREA:
		case	ALARM_CODE_FRAM_AREA:
		case	ALARM_CODE_ILLIGAL_COMMAND:
		case	ALARM_CODE_ILLIGAL_OPERATION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				break;
		case	ALARM_CODE_FEED1_TIMEOUT:
		case	ALARM_CODE_FEED1_JAM_AT_TR:
		case	ALARM_CODE_FEED1_JAM_AT_DR:
		case	ALARM_CODE_FEED2_TIMEOUT:
		case	ALARM_CODE_FEED2_JAM_AT_TR:
		case	ALARM_CODE_FEED2_JAM_AT_DR:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_JAM;
				break;
		case	ALARM_CODE_TWIN_DRUM2_TIMEOUT:
		case	ALARM_CODE_TWIN_DRUM2_JAM:
		case	ALARM_CODE_TWIN_DRUM2_JAM_IN:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_JAM;
				}
				break;
		case	ALARM_CODE_FEED1_MOTOR_LOCK:
		case	ALARM_CODE_FEED2_MOTOR_LOCK:
		case	ALARM_CODE_FEED1_SPEED_CHECK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_MOTOR_ERROR;
				break;
		case	ALARM_CODE_TWIN_DRUM2_MOTOR_LOCK:
		case	ALARM_CODE_TWIN_DRUM2_SPEED_SEL:
				/* RC disable */
				if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else 
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_MOTOR_ERROR;
				}
				break;
		case	ALARM_CODE_FLAP1_TIMEOUT:
		case	ALARM_CODE_FLAP1_LEVER_FAIL:
		case	ALARM_CODE_FLAP1_MOTOR_LOCK:
		case	ALARM_CODE_FLAP2_TIMEOUT:
		case	ALARM_CODE_FLAP2_LEVER_FAIL:
		case	ALARM_CODE_FLAP2_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				break;
		case	ALARM_CODE_RS_FLAP_TIMEOUT:
		case	ALARM_CODE_RS_FLAP_LEVER_FAIL:
		case	ALARM_CODE_RS_FLAP_MOTOR_LOCK:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				break;
		case	ALARM_CODE_RFID_UNIT:
		case	ALARM_CODE_RFID_ICB_COMMUNICTION:
				_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
				break;
		default:
				if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
				|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EEROR;
					_id003_tx_buff.data[len + 1]	= 0x00;
					_id003_tx_buff.data[len + 2]	= 0x00;
				}
				/* RC disable */
				else if(_id003_rc_enable_check(2) != TRUE)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_DISCONNECT;
				}
				else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NOTE_ERROR;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_empty)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_EMPTY;
				}
				else if(ex_rc_status.sst31A.bit.u1_d2_full)
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_FULL;
				}
				else
				{
					_id003_tx_buff.data[len + 0]	= ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[len + 1], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
					len += 2;
				}
				break;
		}
//		len += 1;
		_id003_tx_buff.length	= len + 2 + ID003_ADD_04;
	}
}
#else 
static void _id003_rc_status_error_proc() //id_recycler_error_status
{
	u8 len;
	
	switch (ex_rc_error_code)
	{
	case ALARM_CODE_BOOTIF_AREA:
	case ALARM_CODE_RC_IF_AREA:
	case ALARM_CODE_FRAM_AREA:
	case ALARM_CODE_ILLIGAL_COMMAND:
	case ALARM_CODE_ILLIGAL_OPERATION:
		if (ex_rc_status.sst1A.bit.quad)
		{
			len = 0x12;
			for (u8 i = 0; i < RC_MODEL_QUAD; i++)
			{
				_id003_tx_buff.data[i*3 + 1] = ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[i*3 + 2] = 0x00;
				_id003_tx_buff.data[i*3 + 3] = 0x00;
			}
		}
		else
		{
			len = 0x0C;
			for (u8 i = 0; i < RC_MODEL_TWIN; i++)
			{
				_id003_tx_buff.data[i*3 + 1] = ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[i*3 + 2] = 0x00;
				_id003_tx_buff.data[i*3 + 3] = 0x00;
			}
		}
		break;
	case ALARM_CODE_FEED1_TIMEOUT:
	case ALARM_CODE_FEED2_TIMEOUT:
	// case ALARM_CODE_FEED1_MOTOR_LOCK:
	// case ALARM_CODE_FEED2_MOTOR_LOCK:
	case ALARM_CODE_FEED1_JAM_AT_TR:
	case ALARM_CODE_FEED2_JAM_AT_TR:
	case ALARM_CODE_FEED1_JAM_AT_DR:
	case ALARM_CODE_FEED2_JAM_AT_DR:
		if (ex_rc_status.sst1A.bit.quad)
		{
			len = 0x12;
			for (u8 i = 0; i < RC_MODEL_QUAD; i++)
			{
				_id003_tx_buff.data[i*3 + 1] = ID003_STS_EXT_RC_JAM;
				_id003_tx_buff.data[i*3 + 2] = 0x00;
				_id003_tx_buff.data[i*3 + 3] = 0x00;
			}
		}
		else
		{
			len = 0x0C;
			for (u8 i = 0; i < RC_MODEL_TWIN; i++)
			{
				_id003_tx_buff.data[i*3 + 1] = ID003_STS_EXT_RC_JAM;
				_id003_tx_buff.data[i*3 + 2] = 0x00;
				_id003_tx_buff.data[i*3 + 3] = 0x00;
			}
		}
		break;	
	/* JAM DRUM 1 */
	case ALARM_CODE_TWIN_DRUM1_TIMEOUT:
	case ALARM_CODE_TWIN_DRUM1_JAM:
	case ALARM_CODE_TWIN_DRUM1_JAM_IN:
		len = _id003_rc_status_error_drum_jam(1);
		break;
	/* JAM DRUM 2 */
	case ALARM_CODE_TWIN_DRUM2_TIMEOUT:
	case ALARM_CODE_TWIN_DRUM2_JAM:
	case ALARM_CODE_TWIN_DRUM2_JAM_IN:
		len = _id003_rc_status_error_drum_jam(2);
		break;
	/* JAM DRUM 3 */
	case ALARM_CODE_QUAD_DRUM1_TIMEOUT:
	case ALARM_CODE_QUAD_DRUM1_JAM:
	case ALARM_CODE_QUAD_DRUM1_JAM_IN:
		len = _id003_rc_status_error_drum_jam(3);
		break;
	/* JAM DRUM 4 */
	case ALARM_CODE_QUAD_DRUM2_TIMEOUT:
	case ALARM_CODE_QUAD_DRUM2_JAM:
	case ALARM_CODE_QUAD_DRUM2_JAM_IN:
		len = _id003_rc_status_error_drum_jam(4);
		break;
	/* MOTOR DRUM1 ERROR */
	case ALARM_CODE_TWIN_DRUM1_MOTOR_LOCK:
	case ALARM_CODE_TWIN_DRUM1_SPEED_SEL:
		len = _id003_rc_status_error_drum_motor(1);
		break;
	/* MOTOR DRUM2 ERROR */
	case ALARM_CODE_TWIN_DRUM2_MOTOR_LOCK:
	case ALARM_CODE_TWIN_DRUM2_SPEED_SEL:
		len = _id003_rc_status_error_drum_motor(2);
		break;
	/* MOTOR DRUM3 ERROR */
	case ALARM_CODE_QUAD_DRUM1_MOTOR_LOCK:
	case ALARM_CODE_QUAD_DRUM1_SPEED_SEL:
		len = _id003_rc_status_error_drum_motor(3);
		break;
	/* MOTOR DRUM4 ERROR */
	case ALARM_CODE_QUAD_DRUM2_MOTOR_LOCK:
	case ALARM_CODE_QUAD_DRUM2_SPEED_SEL:
		len = _id003_rc_status_error_drum_motor(4);
		break;
	case ALARM_CODE_FEED1_MOTOR_LOCK:
	case ALARM_CODE_FEED1_SPEED_CHECK:
		if (ex_rc_status.sst1A.bit.quad)
		{
			len = 0x12;
			/* RECYCLE 1, 2 MOTOR Error */
			for (u8 i = 0; i < RC_MODEL_QUAD; i++)
			{
				if (i == 0 || i == 1)
				{
					_id003_tx_buff.data[i*3 + 1] = ID003_STS_EXT_RC_MOTOR_ERROR;
					_id003_tx_buff.data[i*3 + 2] = 0x00;
					_id003_tx_buff.data[i*3 + 3] = 0x00;
				}
				else
				{
					_id003_tx_buff.data[i*3 + 1] = ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[i * 3 + 2], (u8 *)&RecycleSettingInfo.DenomiInfo[i].Data1, 2);
				}
				
			}
		}
		else
		{
			len = 0x0C;
			/* RECYCLE 1, 2 MOTOR Error */
			for (u8 i = 0; i < RC_MODEL_TWIN; i++)
			{
				_id003_tx_buff.data[i*3 + 1] = ID003_STS_EXT_RC_MOTOR_ERROR;
				_id003_tx_buff.data[i*3 + 2] = 0x00;
				_id003_tx_buff.data[i*3 + 3] = 0x00;
			}
		}
		break;
	
	case ALARM_CODE_FEED2_MOTOR_LOCK:
	case ALARM_CODE_FEED2_SPEED_CHECK:
		if (ex_rc_status.sst1A.bit.quad)
		{
			len = 0x12;
			/* RECYCLE 1, 2 MOTOR Error */
			for (u8 i = 0; i < RC_MODEL_QUAD; i++)
			{
				if (i == 2 || i == 3)
				{
					_id003_tx_buff.data[i*3 + 1] = ID003_STS_EXT_RC_MOTOR_ERROR;
					_id003_tx_buff.data[i*3 + 2] = 0x00;
					_id003_tx_buff.data[i*3 + 3] = 0x00;
				}
				else
				{
					_id003_tx_buff.data[i*3 + 1] = ID003_STS_EXT_RC_NORMAL;
					memcpy((u8 *)&_id003_tx_buff.data[i * 3 + 2], (u8 *)&RecycleSettingInfo.DenomiInfo[i].Data1, 2);
				}
				
			}
		}
		else
		{
			len = 0x0C;
			/* RECYCLE 1, 2 MOTOR Error */
			for (u8 i = 0; i < RC_MODEL_TWIN; i++)
			{
				_id003_tx_buff.data[i*3 + 1] = ID003_STS_EXT_RC_NORMAL;
				memcpy((u8 *)&_id003_tx_buff.data[i * 3 + 2], (u8 *)&RecycleSettingInfo.DenomiInfo[i].Data1, 2);
			}
		}
		break;
	case ALARM_CODE_FLAP1_TIMEOUT:
    case ALARM_CODE_FLAP1_LEVER_FAIL:
    case ALARM_CODE_FLAP1_MOTOR_LOCK:
    case ALARM_CODE_FLAP2_TIMEOUT:
    case ALARM_CODE_FLAP2_LEVER_FAIL:
    case ALARM_CODE_FLAP2_MOTOR_LOCK:
		if (ex_rc_status.sst1A.bit.quad)
		{
			len = 0x12;
			for (u8 i = 0; i < RC_MODEL_QUAD; i++)
			{
				_id003_tx_buff.data[i*3 + 1] = ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[i*3 + 2] = 0x00;
				_id003_tx_buff.data[i*3 + 3] = 0x00;
			}
		}
		else
		{
			len = 0x0C;
			for (u8 i = 0; i < RC_MODEL_TWIN; i++)
			{
				_id003_tx_buff.data[i*3 + 1] = ID003_STS_EXT_RC_EEROR;
				_id003_tx_buff.data[i*3 + 2] = 0x00;
				_id003_tx_buff.data[i*3 + 3] = 0x00;
			}
		}
		break;
	default:
		len = 0x08;
		_id003_tx_buff.data[0] = ID003_STS_EXT_RC_JAM;
		break;
	}
	/* resp to host */
	_id003_rc_resp_status_host_proc(len);
}
#endif


#if 1 //2025-01-16
static void id_set_recycle_box_status(void) //RTQ側を未検知状態
{
	/* RC-Quad model */
	if(ex_rc_status.sst1A.bit.quad)
	{
		_id003_tx_buff.length	= 14 + ID003_ADD_04;

		/* RC-Twin unit */
		if(ex_rc_status.sst21B.bit.u1_detect == 0)
		{
			/* drum1 */
			_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_BOX_OPEN;
			_id003_tx_buff.data[2]	= 0x00;
			_id003_tx_buff.data[3]	= 0x00;

			/* drum2 */
			_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_BOX_OPEN;
			_id003_tx_buff.data[5]	= 0x00;
			_id003_tx_buff.data[6]	= 0x00;
		}
		else
		{
			/* drum1 empty detect */
			if(ex_rc_status.sst31A.bit.u1_d1_empty)
			{
				_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_EMPTY;
				_id003_tx_buff.data[2]	= 0x00;
				_id003_tx_buff.data[3]	= 0x00;
			}
			/* drum1 full detect */
			else if(ex_rc_status.sst31A.bit.u1_d1_full)
			{
				_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_FULL;
				_id003_tx_buff.data[2]	= 0x00;
				_id003_tx_buff.data[3]	= 0x00;
			}
			/* normal status */
			else
			{
				_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_NORMAL;
				memcpy((u8 *)&_id003_tx_buff.data[2], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
			}

			/* drum2 empty detect */
			if(ex_rc_status.sst31A.bit.u1_d2_empty)
			{
				_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_EMPTY;
				_id003_tx_buff.data[5]	= 0x00;
				_id003_tx_buff.data[6]	= 0x00;
			}
			/* drum2 full detect */
			else if(ex_rc_status.sst31A.bit.u1_d2_full)
			{
				_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_FULL;
				_id003_tx_buff.data[5]	= 0x00;
				_id003_tx_buff.data[6]	= 0x00;
			}
			/* normal status */
			else
			{
				_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_NORMAL;
				memcpy((u8 *)&_id003_tx_buff.data[5], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
			}
		}

		/* RC-Quad unit */
		if(ex_rc_status.sst22B.bit.u2_detect == 0)
		{
			/* drum3 */
			_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_BOX_OPEN;
			_id003_tx_buff.data[8]	= 0x00;
			_id003_tx_buff.data[9]	= 0x00;

			/* drum4 */
			_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_BOX_OPEN;
			_id003_tx_buff.data[12]	= 0x00;
			_id003_tx_buff.data[11]	= 0x00;
		}
		else
		{
			/* drum3 empty detect */
			if(ex_rc_status.sst32A.bit.u2_d1_empty)
			{
				_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_EMPTY;
				_id003_tx_buff.data[8]	= 0x00;
				_id003_tx_buff.data[9]	= 0x00;
			}
			/* drum3 full detect */
			else if(ex_rc_status.sst32A.bit.u2_d1_full)
			{
				_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_FULL;
				_id003_tx_buff.data[8]	= 0x00;
				_id003_tx_buff.data[9]	= 0x00;
			}
			/* normal status */
			else
			{
				_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_NORMAL;
				memcpy((u8 *)&_id003_tx_buff.data[8], (u8 *)&RecycleSettingInfo.DenomiInfo[2].Data1, 2);
			}

			/* drum4 empty detect */
			if(ex_rc_status.sst32A.bit.u2_d2_empty)
			{
				_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_EMPTY;
				_id003_tx_buff.data[11]	= 0x00;
				_id003_tx_buff.data[12]	= 0x00;
			}
			/* drum4 full detect */
			else if(ex_rc_status.sst32A.bit.u2_d2_full)
			{
				_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_FULL;
				_id003_tx_buff.data[11]	= 0x00;
				_id003_tx_buff.data[12]	= 0x00;
			}
			/* normal status */
			else
			{
				_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_NORMAL;
				memcpy((u8 *)&_id003_tx_buff.data[11], (u8 *)&RecycleSettingInfo.DenomiInfo[3].Data1, 2);
			}
		}
	}
	/* RC-Twin model */
	else
	{
		/* RC-Twin unit disconnected */
		if(ex_rc_status.sst21B.bit.u1_detect == 0)
		{
			_id003_tx_buff.length	= 4 + ID003_ADD_04;

			/* drum1 */
			_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_BOX_OPEN;

			/* drum2 */
			_id003_tx_buff.data[2]	= ID003_STS_EXT_RC_BOX_OPEN;
		}
		else
		{
			_id003_tx_buff.length	= 8;

			/* drum1 */
			_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_NORMAL;
			memcpy((u8 *)&_id003_tx_buff.data[2], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);

			/* drum2 */
			_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_NORMAL;
			memcpy((u8 *)&_id003_tx_buff.data[5], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
		}
	}
}

#else
void _id003_rc_status_set_recycler_box_status() //id_set_recycle_box_status
{
	u8 len = 0;
	if (is_quad_model())
	{
		if (ex_rc_status.sst21B.bit.u1_detect == 0)   /* u1 not detected */
		{
			_id003_tx_buff.data[1] = ID003_STS_EXT_RC_BOX_OPEN;
			_id003_tx_buff.data[2] = 0x00;
			_id003_tx_buff.data[3] = 0x00;

			_id003_tx_buff.data[4] = ID003_STS_EXT_RC_BOX_OPEN;
			_id003_tx_buff.data[5] = 0x00;
			_id003_tx_buff.data[6] = 0x00;
		}
		else
		{
			_id003_tx_buff.data[1] = ID003_STS_EXT_RC_NORMAL;
			_id003_tx_buff.data[2] = RecycleSettingInfo.DenomiInfo[0].Data1;
			_id003_tx_buff.data[3] = RecycleSettingInfo.DenomiInfo[0].Data2;

			_id003_tx_buff.data[4] = ID003_STS_EXT_RC_NORMAL;
			_id003_tx_buff.data[5] = RecycleSettingInfo.DenomiInfo[1].Data1;
			_id003_tx_buff.data[6] = RecycleSettingInfo.DenomiInfo[1].Data2;
		}

		if (ex_rc_status.sst22B.bit.u2_detect == 0)   /* u2 not detected */
		{
			_id003_tx_buff.data[7] = ID003_STS_EXT_RC_BOX_OPEN;
			_id003_tx_buff.data[8] = 0x00;
			_id003_tx_buff.data[9] = 0x00;

			_id003_tx_buff.data[10] = ID003_STS_EXT_RC_BOX_OPEN;
			_id003_tx_buff.data[11] = 0x00;
			_id003_tx_buff.data[12] = 0x00;
		}
		else
		{
			_id003_tx_buff.data[7] = ID003_STS_EXT_RC_NORMAL;
			_id003_tx_buff.data[8] = RecycleSettingInfo.DenomiInfo[2].Data1;
			_id003_tx_buff.data[9] = RecycleSettingInfo.DenomiInfo[2].Data2;

			_id003_tx_buff.data[10] = ID003_STS_EXT_RC_NORMAL;
			_id003_tx_buff.data[11] = RecycleSettingInfo.DenomiInfo[3].Data1;
			_id003_tx_buff.data[12] = RecycleSettingInfo.DenomiInfo[3].Data2;
		}
		len = 0x12;
	}
	else
	{
		if (ex_rc_status.sst21B.bit.u1_detect == 0)   /* u1 not detected */
		{
			_id003_tx_buff.data[1] = ID003_STS_EXT_RC_BOX_OPEN;
			_id003_tx_buff.data[2] = 0x00;
			_id003_tx_buff.data[3] = 0x00;

			_id003_tx_buff.data[4] = ID003_STS_EXT_RC_BOX_OPEN;
			_id003_tx_buff.data[5] = 0x00;
			_id003_tx_buff.data[6] = 0x00;
		}
		else
		{
			_id003_tx_buff.data[1] = ID003_STS_EXT_RC_NORMAL;
			_id003_tx_buff.data[2] = RecycleSettingInfo.DenomiInfo[0].Data1;
			_id003_tx_buff.data[3] = RecycleSettingInfo.DenomiInfo[0].Data2;

			_id003_tx_buff.data[4] = ID003_STS_EXT_RC_NORMAL;
			_id003_tx_buff.data[5] = RecycleSettingInfo.DenomiInfo[1].Data1;
			_id003_tx_buff.data[6] = RecycleSettingInfo.DenomiInfo[1].Data2;
		}
		len = 0x0C;
	}
	_id003_rc_resp_status_host_proc(len);
}
#endif


//2025-01-16
static void id_set_recycler_status(void) //RTQ側検知状態
{
	u8 len;
	u8 boxno;

	/* RC-Quad model */
	if(ex_rc_status.sst1A.bit.quad)
	{
		_id003_tx_buff.length	= 14 + ID003_ADD_04;

		for(boxno = 0; boxno < RC_MODEL_QUAD; ++boxno)
		{
			switch(boxno)
			{
			/* drum1 */
			case	0:
					if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
					|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
					|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
					{
						_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_EEROR;
						_id003_tx_buff.data[2]	= 0x00;
						_id003_tx_buff.data[3]	= 0x00;
					}
					/* RC disable */
					else if(_id003_rc_enable_check(1) != TRUE)
					{
						_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_DISCONNECT;
						_id003_tx_buff.data[2]	= 0x00;
						_id003_tx_buff.data[3]	= 0x00;
					}
#if defined(A_PRO)	/* '22-09-27 */
					/* unit exchanged */
					else if((ex_rc_exchanged_unit & 0x01) != 0)
					{
						_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_EXCHANGED_UNIT;
						_id003_tx_buff.data[2]	= 0x00;
						_id003_tx_buff.data[3]	= 0x00;
					}
#else
					/* unit exchanged */
					else if(ex_rc_exchanged_unit == 1)
					{
						_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_EXCHANGED_UNIT;
						_id003_tx_buff.data[2]	= 0x00;
						_id003_tx_buff.data[3]	= 0x00;
					}
#endif
#if !defined(A_PRO)
					/* unit1 detect during power down */
					else if(_is_id003_check_rc_removed_pdw(RC_TWIN))
					{
						_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_REMOVED_PWROFF;
						_id003_tx_buff.data[2]	= 0x00;
						_id003_tx_buff.data[3]	= 0x00;
					}
					/* battery for optional function */
					else if(_is_id003_check_rc_low_battery())
					{
						_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_LOW_BATTERY;
						_id003_tx_buff.data[2]	= 0x00;
						_id003_tx_buff.data[3]	= 0x00;
					}
#endif
					/* double paper detect */
					else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
					{
						_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_NOTE_ERROR;
						_id003_tx_buff.data[2]	= 0x00;
						_id003_tx_buff.data[3]	= 0x00;
					}
					/* drum1 empty detect */
					else if(ex_rc_status.sst31A.bit.u1_d1_empty)
					{
						_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_EMPTY;
						_id003_tx_buff.data[2]	= 0x00;
						_id003_tx_buff.data[3]	= 0x00;
					}
					/* drum1 full detect */
					else if(ex_rc_status.sst31A.bit.u1_d1_full)
					{
						_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_FULL;
						_id003_tx_buff.data[2]	= 0x00;
						_id003_tx_buff.data[3]	= 0x00;
					}
					/* normal status */
					else
					{
						_id003_tx_buff.data[1]	= ID003_STS_EXT_RC_NORMAL;
						memcpy((u8 *)&_id003_tx_buff.data[2], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
					}
					break;
			/* drum2 */
			case	1:
					if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
					|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
					|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
					{
						_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_EEROR;
						_id003_tx_buff.data[5]	= 0x00;
						_id003_tx_buff.data[6]	= 0x00;
					}
					/* RC disable */
					else if(_id003_rc_enable_check(2) != TRUE)
					{
						_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_DISCONNECT;
						_id003_tx_buff.data[5]	= 0x00;
						_id003_tx_buff.data[6]	= 0x00;
					}
#if defined(A_PRO)	/* '22-09-27 */
					/* unit exchanged */
					else if((ex_rc_exchanged_unit & 0x01) != 0)
					{
						_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_EXCHANGED_UNIT;
						_id003_tx_buff.data[5]	= 0x00;
						_id003_tx_buff.data[6]	= 0x00;
					}
#else
					/* unit exchanged */
					else if(ex_rc_exchanged_unit == 1)
					{
						_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_EXCHANGED_UNIT;
						_id003_tx_buff.data[5]	= 0x00;
						_id003_tx_buff.data[6]	= 0x00;
					}
#endif
#if !defined(A_PRO)
					/* unit1 detect during power down */
					else if(_is_id003_check_rc_removed_pdw(RC_TWIN))
					{
						_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_REMOVED_PWROFF;
						_id003_tx_buff.data[5]	= 0x00;
						_id003_tx_buff.data[6]	= 0x00;
					}
					/* battery for optional function */
					else if(_is_id003_check_rc_low_battery())
					{
						_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_LOW_BATTERY;
						_id003_tx_buff.data[5]	= 0x00;
						_id003_tx_buff.data[6]	= 0x00;
					}
#endif
					/* double paper detect */
					else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
					{
						_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_NOTE_ERROR;
						_id003_tx_buff.data[5]	= 0x00;
						_id003_tx_buff.data[6]	= 0x00;
					}
					/* drum2 empty detect */
					else if(ex_rc_status.sst31A.bit.u1_d2_empty)
					{
						_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_EMPTY;
						_id003_tx_buff.data[5]	= 0x00;
						_id003_tx_buff.data[6]	= 0x00;
					}
					/* drum2 full detect */
					else if(ex_rc_status.sst31A.bit.u1_d2_full)
					{
						_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_FULL;
						_id003_tx_buff.data[5]	= 0x00;
						_id003_tx_buff.data[6]	= 0x00;
					}
					/* normal status */
					else
					{
						_id003_tx_buff.data[4]	= ID003_STS_EXT_RC_NORMAL;
						memcpy((u8 *)&_id003_tx_buff.data[5], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
					}
					break;
			/* drum3 */
			case	2:
					if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
					|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
					|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
					{
						_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_EEROR;
						_id003_tx_buff.data[8]	= 0x00;
						_id003_tx_buff.data[9]	= 0x00;
					}
					/* RC disable */
					else if(_id003_rc_enable_check(3) != TRUE)
					{
						_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_DISCONNECT;
						_id003_tx_buff.data[8]	= 0x00;
						_id003_tx_buff.data[9]	= 0x00;
					}
#if defined(A_PRO)	/* '22-09-27 */
					/* unit exchanged */
					else if((ex_rc_exchanged_unit & 0x02) != 0)
					{
						_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_EXCHANGED_UNIT;
						_id003_tx_buff.data[8]	= 0x00;
						_id003_tx_buff.data[9]	= 0x00;
					}
#else
					/* unit exchanged */
					else if(ex_rc_exchanged_unit == 1)
					{
						_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_EXCHANGED_UNIT;
						_id003_tx_buff.data[8]	= 0x00;
						_id003_tx_buff.data[9]	= 0x00;
					}
#endif
#if !defined(A_PRO)
					/* unit2 detect during power down */
					else if(_is_id003_check_rc_removed_pdw(RC_QUAD))
					{
						_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_REMOVED_PWROFF;
						_id003_tx_buff.data[8]	= 0x00;
						_id003_tx_buff.data[9]	= 0x00;
					}
					/* battery for optional function */
					else if(_is_id003_check_rc_low_battery())
					{
						_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_LOW_BATTERY;
						_id003_tx_buff.data[8]	= 0x00;
						_id003_tx_buff.data[9]	= 0x00;
					}
#endif
					/* double paper detect */
					else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 3)
					{
						_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_NOTE_ERROR;
						_id003_tx_buff.data[8]	= 0x00;
						_id003_tx_buff.data[9]	= 0x00;
					}
					/* drum1 empty detect */
					else if(ex_rc_status.sst32A.bit.u2_d1_empty)
					{
						_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_EMPTY;
						_id003_tx_buff.data[8]	= 0x00;
						_id003_tx_buff.data[9]	= 0x00;
					}
					/* drum1 full detect */
					else if(ex_rc_status.sst32A.bit.u2_d1_full)
					{
						_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_FULL;
						_id003_tx_buff.data[8]	= 0x00;
						_id003_tx_buff.data[9]	= 0x00;
					}
					/* normal status */
					else
					{
						_id003_tx_buff.data[7]	= ID003_STS_EXT_RC_NORMAL;
						memcpy((u8 *)&_id003_tx_buff.data[8], (u8 *)&RecycleSettingInfo.DenomiInfo[2].Data1, 2);
					}
					break;
			/* drum4 */
			case	3:
					if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
					|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
					|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
					{
						_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_EEROR;
						_id003_tx_buff.data[11]	= 0x00;
						_id003_tx_buff.data[12]	= 0x00;
					}
					/* RC disable */
					else if(_id003_rc_enable_check(4) != TRUE)
					{
						_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_DISCONNECT;
						_id003_tx_buff.data[11]	= 0x00;
						_id003_tx_buff.data[12]	= 0x00;
					}
#if defined(A_PRO)	/* '22-09-27 */
					/* unit exchanged */
					else if((ex_rc_exchanged_unit & 0x02) != 0)
					{
						_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_EXCHANGED_UNIT;
						_id003_tx_buff.data[11]	= 0x00;
						_id003_tx_buff.data[12]	= 0x00;
					}
#else
					/* unit exchanged */
					else if(ex_rc_exchanged_unit == 1)
					{
						_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_EXCHANGED_UNIT;
						_id003_tx_buff.data[11]	= 0x00;
						_id003_tx_buff.data[12]	= 0x00;
					}
#endif
#if !defined(A_PRO)
					/* unit2 detect during power down */
					else if(_is_id003_check_rc_removed_pdw(RC_QUAD))
					{
						_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_REMOVED_PWROFF;
						_id003_tx_buff.data[11]	= 0x00;
						_id003_tx_buff.data[12]	= 0x00;
					}
					/* battery for optional function */
					else if(_is_id003_check_rc_low_battery())
					{
						_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_LOW_BATTERY;
						_id003_tx_buff.data[11]	= 0x00;
						_id003_tx_buff.data[12]	= 0x00;
					}
#endif
					/* double paper detect */
					else if(ex_rc_status.sst22B.bit.u2_detect_dbl && OperationDenomi.unit == 4)
					{
						_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_NOTE_ERROR;
						_id003_tx_buff.data[11]	= 0x00;
						_id003_tx_buff.data[12]	= 0x00;
					}
					/* drum1 empty detect */
					else if(ex_rc_status.sst32A.bit.u2_d2_empty)
					{
						_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_EMPTY;
						_id003_tx_buff.data[11]	= 0x00;
						_id003_tx_buff.data[12]	= 0x00;
					}
					/* drum1 full detect */
					else if(ex_rc_status.sst32A.bit.u2_d2_full)
					{
						_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_FULL;
						_id003_tx_buff.data[11]	= 0x00;
						_id003_tx_buff.data[12]	= 0x00;
					}
					/* normal status */
					else
					{
						_id003_tx_buff.data[10]	= ID003_STS_EXT_RC_NORMAL;
						memcpy((u8 *)&_id003_tx_buff.data[11], (u8 *)&RecycleSettingInfo.DenomiInfo[3].Data1, 2);
					}
					break;
			default:
					break;
			}
		}
	}
	/*  RC-Twin model */
	else
	{
		len = 0;

		for(boxno = 0; boxno < RC_MODEL_TWIN; ++boxno)
		{
			switch(boxno)
			{
			/* drum1 */
			case	0:
					if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
					|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
					|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_EEROR;
					}
					/* RC disable */
					else if(_id003_rc_enable_check(1) != TRUE)
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_DISCONNECT;
					}
#if defined(A_PRO)	/* '22-09-27 */
					/* unit exchanged */
					else if(ex_rc_exchanged_unit != 0)
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_EXCHANGED_UNIT;
					}
#else
					/* unit exchanged */
					else if(ex_rc_exchanged_unit == 1)
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_EXCHANGED_UNIT;
					}
#endif
#if !defined(A_PRO)
					/* unit1 detect during power down */
					else if(_is_id003_check_rc_removed_pdw(RC_TWIN))
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_REMOVED_PWROFF;
					}
					/* battery for optional function */
					else if(_is_id003_check_rc_low_battery())
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_LOW_BATTERY;
					}
#endif
					/* double paper detect */
					else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 1)
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_NOTE_ERROR;
					}
					/* drum1 empty detect */
					else if(ex_rc_status.sst31A.bit.u1_d1_empty)
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_EMPTY;
					}
					/* drum1 full detect */
					else if(ex_rc_status.sst31A.bit.u1_d1_full)
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_FULL;
					}
					/* normal status */
					else
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_NORMAL;

						len += 1;
						memcpy((u8 *)&_id003_tx_buff.data[len], (u8 *)&RecycleSettingInfo.DenomiInfo[0].Data1, 2);
						len += 1;
					}
					break;
			/* drum2 */
			case	1:
					if((ex_cline_status_tbl.error_code == ALARM_CODE_RC_ROM)
					|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_COM)
					|| (ex_cline_status_tbl.error_code == ALARM_CODE_RC_DWERR))
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_EEROR;
					}
					/* RC disable */
					else if(_id003_rc_enable_check(2) != TRUE)
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_DISCONNECT;
					}
#if defined(A_PRO)	/* '22-90-27 */
					/* unit exchanged */
					else if(ex_rc_exchanged_unit != 0)
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_EXCHANGED_UNIT;
					}
#else
					/* unit exchanged */
					else if(ex_rc_exchanged_unit == 1)
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_EXCHANGED_UNIT;
					}
#endif
#if !defined(A_PRO)
					/* unit1 detect during power down */
					else if(_is_id003_check_rc_removed_pdw(RC_TWIN))
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_REMOVED_PWROFF;
					}
					/* battery for optional function */
					else if(_is_id003_check_rc_low_battery())
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_LOW_BATTERY;
					}
#endif
					/* double paper detect */
					else if(ex_rc_status.sst21B.bit.u1_detect_dbl && OperationDenomi.unit == 2)
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_NOTE_ERROR;
					}
					/* drum1 empty detect */
					else if(ex_rc_status.sst31A.bit.u1_d2_empty)
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_EMPTY;
					}
					/* drum1 full detect */
					else if(ex_rc_status.sst31A.bit.u1_d2_full)
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_FULL;
					}
					/* normal status */
					else
					{
						len += 1;
						_id003_tx_buff.data[len]	= ID003_STS_EXT_RC_NORMAL;

						len += 1;
						memcpy((u8 *)&_id003_tx_buff.data[len], (u8 *)&RecycleSettingInfo.DenomiInfo[1].Data1, 2);
						len += 1;
					}
					break;
			default:
					break;
			}
		}
		_id003_tx_buff.length	= len + 2 + ID003_ADD_04;
	}
}



/*********************************************************************//**
 * @brief response status (STATUS)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _id003_rc_status_cmd_proc()
{
	 //2025-01-16
	//if(_id003_rx_buff.length == 0x03)
	if(_id003_rx_buff.length == (0x03 + ID003_ADD_04))
	{
		_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
	//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
		_is_set_unit_id();

		if(ex_cline_status_tbl.line_task_mode == ID003_MODE_ERROR
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR)
		{
			/* In case of the error which was detected in Recycler. */
			if(ex_rc_status.sst1A.bit.error)
			{
				id_recycler_error_status();
			}
			/* In case of the error which was detected in UBA. */
			else
			{
				/* RC-Quad model */
				if(ex_rc_status.sst1A.bit.quad)
				{
					if(ex_rc_status.sst21B.bit.u1_detect == 0 || ex_rc_status.sst22B.bit.u2_detect == 0)
					{
						id_set_recycle_box_status();
					}
					else
					{
						id_set_recycler_status();
					}
				}
				/* RC-Twin model */
				else
				{
					if(ex_rc_status.sst21B.bit.u1_detect == 0)
					{
						id_set_recycle_box_status();
					}
					else
					{
						id_set_recycler_status();
					}
				}
			}
		}
		/* The warning occurs or the normal case. */
		else
		{
			id_set_recycler_status();
		}
		//2025-01-16 uart_send_id003((u8 *)&_id003_tx_buff, sizeof(_id003_tx_buff));
		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}

static void _id003_rc_clear_cmd_proc() //ok  not use only use RC200
{
	if (_id003_rx_buff.length == (0x04 + ID003_ADD_04) )
	{
		_id003_send_host_invalid();		
	}
	else
	{
		_id003_send_host_invalid();
	}
}

static void _id003_ex_estop_cmd_proc() //ok
{
	//2025-01-17
	if(is_rc_rs_unit())
	{
		_id003_send_host_invalid();
	}
	else
	{
		if(_id003_rx_buff.length == (0x03 + ID003_ADD_04))
		{
			if(ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYSTAY)
			{
				/* 2024-09-25 - Concurrent Dispensing from Multiple-payout command (M_Payout) */
				if(ConcurrentPayoutFlag == 1)
				{
					clearConcurrentPayout();
				} 

				/* 収納完了までにunitが変更されるため、退避しておく */
				OperationDenomi.unit_emergency = OperationDenomi.unit;

				ex_cline_status_tbl.line_task_mode = ID003_MODE_RETURN_TO_BOX;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_EMERGENCY_REQ, 0, 0, 0, 0);
				_id003_send_host_ack();
			}
			else
			{
				_id003_send_host_invalid();
			}
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
}

static u8 set_recycle_count_setting(u8 model) //ok
{
	u8 *ptr;

	ptr = &_id003_rx_buff.data[2];

	/* The validity confirmation of the box number. */
	if((*ptr > RECYCLER_BILL_MAX || (*(ptr + 1) != 0)) || (_id003_rx_buff.data[4] > model))
	{
		return(FALSE);
	}
	else
	{
		return(TRUE);
	}
}

static void _id003_rc_count_set_cmd_proc() //ok //recycle limit
{
	//2025-01-17
	u8 result;
	u8 model;

	if (_id003_rx_buff.length == (0x06 + ID003_ADD_04) )
	{
		if(ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL				/* power init */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK		/* power init stack */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND		/* power init vend */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK	/* power init vend ack */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT		/* power init reject */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE		/* power init pause */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL						/* init */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK				/* init stack */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT				/* init reject */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE				/* inti pause */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE						/* disable */
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)				/* disable reject */
		{
			/* check model */
			if(ex_rc_status.sst1A.bit.quad)
			{
				model = RC_MODEL_QUAD;
			}
			else
			{
				model = RC_MODEL_TWIN;
			}

			result = set_recycle_count_setting(model);
			if (result == TRUE)
			{
				ex_rc_data_lock = 1;

				if(_id003_rx_buff.data[2] == 0)
				{
					RecycleSettingInfo.DenomiInfo[_id003_rx_buff.data[4] - 1].RecycleLimit = RECYCLER_BILL_MAX;
				}
				else
				{
					RecycleSettingInfo.DenomiInfo[_id003_rx_buff.data[4] - 1].RecycleLimit = _id003_rx_buff.data[2];
				}
				/* リサイクル金種設定時はバックアップも更新 */
				memcpy((u8 *)&RecycleSettingInfo_bk.DenomiInfo[0], (u8 *)&RecycleSettingInfo.DenomiInfo[0], sizeof(RecycleSettingInfo));
				/* Update info to RC */
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RC_INFO_REQ, 0, 0, 0, 0);
				_id003_send_host_echoback();
			}
			else
			{
				_id003_send_host_invalid();
			}
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}

static void _id003_rc_refill_set_cmd_proc() //ok
{
	if (_id003_rx_buff.length == (0x04 + ID003_ADD_04)) //+4byte
	{
		if( ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE ||
			ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT)
		{
			OperationDenomi.mode = _id003_rx_buff.data[2]; //-2byte
			if (OperationDenomi.mode)
			{
				/* LED BEZEL BLINK */
				_cline_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_BLINK, 0, 3, 3, 3);
			}
			else
			{
				/* LED BEZEL ON */
				_cline_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_ON, 0, 0, 0, 0);
			}
			_id003_send_host_echoback();
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}


void _id003_rc_enable_set_cmd_proc(void) //ok //2024-11-20
{
	if(_id003_rx_buff.length == (0x04 + ID003_ADD_04))
	{
		if(ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_AT
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_SK
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAYVALID
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_ERROR)
		{
			ex_rc_enable = _id003_rx_buff.data[2];
			_id003_send_host_echoback();

			_cline_send_msg(ID_MAIN_MBX, TMSG_LINE_RC_ENABLE_REQ, _id003_rx_buff.data[2], 0, 0, 0);
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}

void _id003_rc_enable_req_cmd_proc(void)
{
	if(_id003_rx_buff.length == (0x03 + ID003_ADD_04))
	{
		_id003_tx_buff.length	= (4 + ID003_ADD_04);
		_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
	//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
		_is_set_unit_id();
		_id003_tx_buff.data[1]	= ID003_CMD_EXT_RC_ENABLE_REQ;
		_id003_tx_buff.data[2]	= ex_rc_enable;
		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}


static void _id003_rc_count_req_cmd_proc() //ok
{
	//2025-01-17
	u8 *ptr;
	u8 cnt;
	u8 model;

	_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
	_is_set_unit_id();
	_id003_tx_buff.data[1]	= ID003_CMD_EXT_RC_COUNT_REQ;

	ptr = &_id003_tx_buff.data[2];

	/* check model */
	if(ex_rc_status.sst1A.bit.quad)
	{
		_id003_tx_buff.length	= 11 + ID003_ADD_04;
		model = RC_MODEL_QUAD;
	}
	else
	{
		_id003_tx_buff.length	= 7 + ID003_ADD_04;
		model = RC_MODEL_TWIN;
	}

	for(cnt = 0; cnt < model; cnt++)
	{
		*(ptr + 0) = RecycleSettingInfo.DenomiInfo[cnt].RecycleLimit;
		*(ptr + 1) = (RecycleSettingInfo.DenomiInfo[cnt].RecycleLimit << 8);
		ptr += 2;
	}
	uart_send_id003(&_id003_tx_buff);
}

static void _id003_rc_version_req_cmd_proc()//ok
{
	//2025-01-17
	if(_id003_rx_buff.length == (0x03 + ID003_ADD_04))
	{
		_id003_tx_buff.length	= 21 + ID003_ADD_04;
		_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
	//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
		_is_set_unit_id();
		_id003_tx_buff.data[1]	= ID003_CMD_EXT_RC_VERSION_REQ;
		memcpy((u8 *)&_id003_tx_buff.data[2], (u8 *)&RecycleSoftInfo.FlashRomid[0], 18);

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}

static void _id003_rc_refill_req_cmd_proc() //ok
{
	//2025-01-17
	if(_id003_rx_buff.length == (0x03 + ID003_ADD_04))
	{
		_id003_tx_buff.length	= 4 + ID003_ADD_04;
		_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
	//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
		_is_set_unit_id();
		_id003_tx_buff.data[1]	= ID003_CMD_EXT_RC_REFILL_REQ;
		_id003_tx_buff.data[2]	= OperationDenomi.mode;

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}

static void _id003_rc_currency_req_cmd_proc() //ok
{
	//2025-01-17
	extern const u8 recycle_currency_table1[8][6];
	extern const u8 recycle_currency_table2[8][6];

	if(_id003_rx_buff.length == (0x03 + ID003_ADD_04))
	{
		_id003_tx_buff.length	= 99 + ID003_ADD_04;
		_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
	//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
		_is_set_unit_id();
		_id003_tx_buff.data[1]	= ID003_CMD_EXT_RC_CURRENCY;

		memcpy((u8 *)&_id003_tx_buff.data[2], (u8 *)&recycle_currency_table1[0][0], sizeof(recycle_currency_table1));
		memcpy((u8 *)&_id003_tx_buff.data[50], (u8 *)&recycle_currency_table2[0][0], sizeof(recycle_currency_table2));

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}


static void _id003_rc_total_cmd_proc() //ok
{
	//2025-01-17
	u8	cnt;
	u8	*ptr;
	u32	aCount = 0;
	u32	pCount = 0;
	u32	cCount = 0;
	u8 model;

	if(_id003_rx_buff.length == (0x03 + ID003_ADD_04) )
	{
		_id003_tx_buff.length	= 12 + ID003_ADD_04;
		_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
	//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
		_is_set_unit_id();
		_id003_tx_buff.data[1]	= ID003_CMD_EXT_RC_TOTAL;

		ptr = &_id003_tx_buff.data[2];

		/* check model */
		if(ex_rc_status.sst1A.bit.quad)
		{
			model = RC_MODEL_QUAD;
		}
		else
		{
			model = RC_MODEL_TWIN;
		}

		/* 全てのRecycle boxの枚数を加算 */
		for(cnt = 0; cnt < model; cnt++)
		{
			aCount += rcLogdatIF.rcLogIF[cnt].AcceptCount;
			pCount += rcLogdatIF.rcLogIF[cnt].PayoutCount;
			cCount += rcLogdatIF.rcLogIF[cnt].CollectCount;
		}

		/* データをセット */
		*ptr		= aCount;
		*(ptr + 1)	= aCount >> 8;
		*(ptr + 2)	= aCount >> 16;

		/* アドレスを移動 */
		ptr += 3;

		/* データをセット */
		*ptr		= pCount;
		*(ptr + 1)	= pCount >> 8;
		*(ptr + 2)	= pCount >> 16;

		/* アドレスを移動 */
		ptr += 3;

		/* データをセット */
		*ptr		= cCount;
		*(ptr + 1)	= cCount >> 8;
		*(ptr + 2)	= cCount >> 16;

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}

static void _id003_rc_total_clear_cmd_proc() //ok
{
	//2025-01-17
	u8	cnt;
	u8 model;

	if(_id003_rx_buff.length == (0x03 + ID003_ADD_04))
	{
		_id003_tx_buff.length	= 4 + ID003_ADD_04;
		_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
	//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
		_is_set_unit_id();
		_id003_tx_buff.data[1]	= ID003_CMD_EXT_RC_TOTAL_CLEAR;
		_id003_tx_buff.data[2]	= 0x00;

		/* check model */
		if(ex_rc_status.sst1A.bit.quad)
		{
			model = RC_MODEL_QUAD;
		}
		else
		{
			model = RC_MODEL_TWIN;
		}

		/* Total Countの AcceptCount, PayoutCount, CollectCount */
		for(cnt = 0; cnt < model; cnt++)
		{
			rcLogdatIF.rcLogIF[cnt].AcceptCount = 0;
			rcLogdatIF.rcLogIF[cnt].PayoutCount = 0;
			rcLogdatIF.rcLogIF[cnt].CollectCount = 0;
		}
		uart_send_id003(&_id003_tx_buff);
		set_recycle_total_count(TOTAL_CLEAR, 0);
	}
	else
	{
		_id003_send_host_invalid();
	}
}

static void _id003_rc_current_req_cmd_proc() //ok
{
	//2025-01-17
	u8 *ptr;
	u8 cnt;
	u8 model;

	if(_id003_rx_buff.length == (0x03 + ID003_ADD_04))
	{
		_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
	//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
		_is_set_unit_id();
		_id003_tx_buff.data[1]	= ID003_CMD_EXT_RC_CURRENT_REQ;

		ptr = &_id003_tx_buff.data[2];

		/* check model */
		if(ex_rc_status.sst1A.bit.quad)
		{
			_id003_tx_buff.length	= 11 + ID003_ADD_04;
			model = RC_MODEL_QUAD;
		}
		else
		{
			_id003_tx_buff.length	= 7 + ID003_ADD_04;
			model = RC_MODEL_TWIN;
		}

		for(cnt = 0; cnt < model; cnt++)
		{
			*(ptr + 0) = RecycleSettingInfo.DenomiInfo[cnt].RecycleCurrent;
			*(ptr + 1) = 0x00;
			ptr += 2;
		}
		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}

/*********************************************************************//**
 **********************************************************************/
static u8 set_current_count_setting(u8 model) //ok
{
	//2025-01-17
	u8 *ptr;

	ptr = &_id003_rx_buff.data[2];

	if((*ptr > RECYCLER_BILL_MAX || (*(ptr + 1) != 0)) || (_id003_rx_buff.data[4] > model))
	{
		return(FALSE);
	}
	else
	{
		return(TRUE);
	}
}

static void _id003_rc_current_cmd_proc() //ok
{
	u8 *ptr;
	uint32_t box_no = 0;
	uint32_t box_count = 0;
	uint8_t result;
	uint8_t model;
	
	if (_id003_rx_buff.length == (0x06 + ID003_ADD_04))
	{
		if(ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_AT
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_SK
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_ERROR
		// || ex_cline_status_tbl.line_task_mode == ID003_MODE_RC_ERROR
		)
		{
			/* check model */
			if(ex_rc_status.sst1A.bit.quad)
			{
				model = RC_MODEL_QUAD;
			}
			else
			{
				model = RC_MODEL_TWIN;
			}
			
			result = set_current_count_setting(model);

			if (result == TRUE)
			{
				ex_rc_data_lock = 1;

				box_no = _id003_rx_buff.data[4];
				box_count = _id003_rx_buff.data[2];

				ptr = (UB *)&RecycleSettingInfo.DenomiInfo[_id003_rx_buff.data[4] - 1].RecycleCurrent;	//2025-01-17
				*(ptr + 0) = _id003_rx_buff.data[2];

				/* リサイクル金種設定時はバックアップも更新 */
				memcpy((u8 *)&RecycleSettingInfo_bk.DenomiInfo[0], (u8 *)&RecycleSettingInfo.DenomiInfo[0], sizeof(RecycleSettingInfo));
				_id003_send_host_echoback();
				_cline_send_msg(ID_MAIN_MBX, TMSG_LINE_CURRENT_COUNT_REQ, box_no, box_count, 0, 0);
			}
			else
			{
				_id003_send_host_invalid();
			}
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}

static void _id003_rc_exstatus_cmd_proc() //ok
{
	//2025-01-16
	if(_id003_rx_buff.length == (0x03 + ID003_ADD_04))
	{
		_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
	//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
		_is_set_unit_id();

		if(ex_cline_status_tbl.line_task_mode == ID003_MODE_ERROR
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR)
		{
			/* In case of the error which was detected in Recycler. */
			if(ex_rc_status.sst1A.bit.error)
			{
				id_recycler_error_exstatus();
			}
			/* In case of the error which was detected in UBA. */
			else
			{
				/* RC-Quad model */
				if(ex_rc_status.sst1A.bit.quad)
				{
					if(ex_rc_status.sst21B.bit.u1_detect == 0 || ex_rc_status.sst22B.bit.u2_detect == 0)
					{
						id_set_recycle_box_status();
					}
					else
					{
						id_set_recycler_status();
					}
				}
				/* RC-Twin model */
				else
				{
					if(ex_rc_status.sst21B.bit.u1_detect == 0)
					{
						id_set_recycle_box_status();
					}
					else
					{
						id_set_recycler_status();
					}
				}
			}
		}
		/* The warning occurs or the normal case. */
		else
		{
			id_set_recycler_status();
		}
		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}

static void _id003_rc_battery_clear_cmd_proc() //ok
{
	//2025-01-17
	if(_id003_rx_buff.length == (0x03 + ID003_ADD_04))
	{
		if(ex_rc_status.sst22A.bit.battery_detect == 1)
		{
			_id003_send_host_ack();

			ex_cline_status_tbl.ex_rc_option_battery_low_detect = 0;
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}

void _id003_rc_cmd_proc(void)
{

	switch(_id003_rx_buff.data[1])	/* operation */
	{
	case	ID003_CMD_EXT_RC_STATUS:
			_id003_rc_status_cmd_proc(); //ok
			break;

	case ID003_CMD_EXT_RC_PAYOUT:				/* PAYOUT */
		_id003_rc_payout_cmd_proc(); //lengthがまだ
		break;
	case ID003_CMD_EXT_RC_COLLECT:				/* COLLECT */
		_id003_rc_collect_cmd_proc(); //ok
		break;
	case ID003_CMD_EXT_RC_CLEAR:				/* CLEAR ERROR */
		_id003_rc_clear_cmd_proc(); //ok
		break;				
	case ID003_CMD_EXT_RC_E_STOP:				/* EMERGENCY STOP */
		_id003_ex_estop_cmd_proc(); //ok
		break;
	case ID003_CMD_EXT_RC_MC_SET:			/* CURRENCY SET */
		_id003_rc_mc_set_cmd_proc(); //ok
		break;		
	case ID003_CMD_EXT_RC_KEY_SET:				/* KEY SET */
		_id003_rc_key_set_cmd_proc(); //ok
		break;			
	case ID003_CMD_EXT_RC_COUNT_SET:		/* COUTN SET *///recycle limit
		_id003_rc_count_set_cmd_proc(); //ok
		break;		
	case ID003_CMD_EXT_RC_REFILL_SET: /* REFILL SET */
		_id003_rc_refill_set_cmd_proc(); //ok
		break;
	case ID003_CMD_EXT_RC_ENABLE_SET:
		_id003_rc_enable_set_cmd_proc();
		break;
	case ID003_CMD_EXT_RC_CURRENT:			/* CURRENT COUNT SET*/
		_id003_rc_current_cmd_proc(); //ok
		break;
	case ID003_CMD_EXT_RC_MC_REQ:			/* CURRENCY REQ */
		_id003_rc_mc_req_cmd_proc(); //ok
		break;	
	case ID003_CMD_EXT_RC_KEY_REQ:				/* KEY REQ */
		_id003_rc_key_req_cmd_proc(); //ok
		break;
	case ID003_CMD_EXT_RC_COUNT_REQ:		/* COUNT REQ */
		_id003_rc_count_req_cmd_proc(); //ok
		break;
	case ID003_CMD_EXT_RC_VERSION_REQ:				/* VERSION GET */
		_id003_rc_version_req_cmd_proc(); //ok
		break;			
	case ID003_CMD_EXT_RC_REFILL_REQ:	/* REFILL MODE */
		_id003_rc_refill_req_cmd_proc(); //ok
		break;	
	case ID003_CMD_EXT_RC_CURRENCY:
		_id003_rc_currency_req_cmd_proc();	//ok
		break;	
	case	ID003_CMD_EXT_RC_ENABLE_REQ:
			_id003_rc_enable_req_cmd_proc();
			break;
	case ID003_CMD_EXT_RC_TOTAL:				/* TOTAL */
		_id003_rc_total_cmd_proc(); //ok
		break;				
	case ID003_CMD_EXT_RC_TOTAL_CLEAR:			/* TOTAL CLEAR */
		_id003_rc_total_clear_cmd_proc(); //ok
		break;		
	case ID003_CMD_EXT_RC_CURRENT_REQ:			/* CURRENT COUNT REQ*/
		_id003_rc_current_req_cmd_proc(); //ok
		break;		
	case ID003_CMD_EXT_RC_BATTERY_CLEAR:
		_id003_rc_battery_clear_cmd_proc(); //ok
		break;
	case ID003_CMD_EXT_RC_EXSTATUS:
		_id003_rc_exstatus_cmd_proc(); //ok
		break;	
	case ID003_CMD_EXT_RC_DLD_STATUS:
		_id003_rc_dldstatus_cmd_proc();
		break;
	default:
		_id003_send_host_invalid();
		break;	
	}
}

/*********************************************************************//**
 * @brief rc unit information
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_unit_info_cmd_proc()
{
	if (_id003_rx_buff.length == 0x05)
	{
		_id003_tx_buff.length = 3 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_UNIT_INFO;
	//	_id003_tx_buff.data[0] = ID003_UNIT_ID_RECYCLER; //0x20
		_is_set_unit_id();

		if (is_quad_model())
		{
			_id003_tx_buff.data[1] = RC_MODEL_QUAD;
		}
		else
		{
			_id003_tx_buff.data[1] = RC_MODEL_TWIN;
		}
		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}

#endif // UBA_RTQ


/*********************************************************************//**
 * @brief Stack1 Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stack1_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_ESCROW_WAIT_CMD)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD1)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD2))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK;
			write_status_table();

			set_recovery_step(RECOVERY_STEP_ESCORW);
			s_id003_stacking_info = (ID003_STACKING_INFO_BUSY|ID003_STACKING_INFO_WAIT_STS);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, VEND_POSITION_1, 0, 0, 0);

			if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
			 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
			{
				 id003_enc_update_context();
			}

			_id003_status_wait_clear();

			_id003_send_host_ack();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_REJECT)

		#if defined(UBA_RTQ)//2025-08-03 UBA500とは異なるがcheatではなくinvalid
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAYVALID)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_COLLECT)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_COLLECTED_WAIT_POLL)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_RETURN_TO_BOX)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYOUT)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYOUT_RETURN_NOTE)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYOUT_COLLECTED_WAIT_POLL)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYSTAY)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYSTAY_WAIT_POLL)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYVALID)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYVALID_ERROR)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_PAYVALID_ACK)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_PAYVALID_ACK_ERROR)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AFTER_PAYVALID_ACK_ENABLE)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AFTER_PAYVALID_ACK_DISABLE)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AFTER_PAYVALID_ACK_PAYOUT)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DIAGNOSTIC)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_RETURN_ERROR)				
		#endif
			  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_REJECT_WAIT_POLL))
		{
			_id003_send_host_invalid();
		}
		/* Bar CouponのリカバリーにおいてPower Up後 Status Request受信前に */
		/* Stackコマンドを受信した場合にはInvalidを応答する */
		else if ((s_id003_powerup_stsreq_flag == 1)
			  && ((s_id003_powerup_stat == POWERUP_STAT_RECOVER_FEED_STACK)
			   || (s_id003_powerup_stat == POWERUP_STAT_RECOVER_STACK)
			   )
			  && (ex_cline_status_tbl.escrow_code == BAR_INDX))
		{
			_id003_send_host_invalid();
		}
		/* リカバリーにおいてPower Up後 Status Request受信前に */
		/* Stackコマンドを受信した場合にはInvalidを応答する */
		else if ((s_id003_powerup_stsreq_flag == 1)
			  && (s_id003_powerup_stat == POWERUP_STAT_REJECT) )
		{
			_id003_send_host_invalid();
		}
		/* [Polling Mode] 不正クレジット */
		/*  Stackingステータス設定後の最初のPollを受信するまで */
		else if ((ex_cline_status_tbl.comm_mode == 0x0000)
			  && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_STACK)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND_FULL))
			  && ((s_id003_stacking_info & ID003_STACKING_INFO_WAIT_STS) == ID003_STACKING_INFO_WAIT_STS))
		{
			_id003_send_host_invalid();
		}
		/* [Interrupt2] 不正クレジット */
		/*  VALIDステータス送信後のACK受信まで */
		else if ((ex_cline_status_tbl.comm_mode == 0x0002)
			  && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_STACK)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND_FULL)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_VEND_ACK)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_VEND_ACK_FULL)))
		{
			_id003_send_host_invalid();
		}
		/* [Interrupt1] 不正クレジット */
		/*  Stackingステータス設定まで */
		else
		{
			_id003_illegal_credit();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Stack2 Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stack2_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_ESCROW_WAIT_CMD)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD1)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD2))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK;
			write_status_table();

			set_recovery_step(RECOVERY_STEP_ESCORW);
			s_id003_stacking_info = (ID003_STACKING_INFO_BUSY|ID003_STACKING_INFO_WAIT_STS);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, VEND_POSITION_2, 0, 0, 0);

			if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
			 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
			{
				 id003_enc_update_context();
			}

			_id003_status_wait_clear();

			_id003_send_host_ack();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_REJECT)
		#if defined(UBA_RTQ)//2025-08-03 UBA500とは異なるがcheatではなくinvalid
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAYVALID)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_COLLECT)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_COLLECTED_WAIT_POLL)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_RETURN_TO_BOX)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYOUT)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYOUT_RETURN_NOTE)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYOUT_COLLECTED_WAIT_POLL)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYSTAY)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYSTAY_WAIT_POLL)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYVALID)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYVALID_ERROR)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_PAYVALID_ACK)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_PAYVALID_ACK_ERROR)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AFTER_PAYVALID_ACK_ENABLE)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AFTER_PAYVALID_ACK_DISABLE)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AFTER_PAYVALID_ACK_PAYOUT)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DIAGNOSTIC)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_RETURN_ERROR)				
		#endif
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_REJECT_WAIT_POLL))
		{
			_id003_send_host_invalid();
		}
		/* Bar CouponのリカバリーにおいてPower Up後 Status Request受信前に */
		/* Stackコマンドを受信した場合にはInvalidを応答する */
		else if ((s_id003_powerup_stsreq_flag == 1)
			  && ((s_id003_powerup_stat == POWERUP_STAT_RECOVER_FEED_STACK)
			   || (s_id003_powerup_stat == POWERUP_STAT_RECOVER_STACK)
			   )
			  && (ex_cline_status_tbl.escrow_code == BAR_INDX))
		{
			_id003_send_host_invalid();
		}
		/* リカバリーにおいてPower Up後 Status Request受信前に */
		/* Stackコマンドを受信した場合にはInvalidを応答する */
		else if ((s_id003_powerup_stsreq_flag == 1)
			  && (s_id003_powerup_stat == POWERUP_STAT_REJECT) )
		{
			_id003_send_host_invalid();
		}
		/* [Polling Mode] 不正クレジット */
		/*  Stackingステータス設定後の最初のPollを受信するまで */
		else if ((ex_cline_status_tbl.comm_mode == 0x0000)
			  && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_STACK)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND_FULL))
			  && ((s_id003_stacking_info & ID003_STACKING_INFO_WAIT_STS) == ID003_STACKING_INFO_WAIT_STS))
		{
			_id003_send_host_invalid();
		}
		/* [Interrupt2] 不正クレジット */
		/*  VALIDステータス送信後のACK受信まで */
		else if ((ex_cline_status_tbl.comm_mode == 0x0002)
			  && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_STACK)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND_FULL)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_VEND_ACK)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_VEND_ACK_FULL)))
		{
			_id003_send_host_invalid();
		}
		/* [Interrupt1] 不正クレジット */
		/*  Stackingステータス設定まで */
		else
		{
			_id003_illegal_credit();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}

#if defined(UBA_RTQ)
/*********************************************************************//**
 * @brief Stack3 Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stack3_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_ESCROW_WAIT_CMD)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD1)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD2))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK;
			write_status_table();

			set_recovery_step(RECOVERY_STEP_ESCORW);
			s_id003_stacking_info = (ID003_STACKING_INFO_BUSY|ID003_STACKING_INFO_WAIT_STS);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, VEND_POSITION_2, 1, 0, 0); //引数1部分のみ stack2と異なる

			if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
			 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
			{
				 id003_enc_update_context();
			}

			_id003_status_wait_clear();

			_id003_send_host_ack();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_REJECT)
		#if defined(UBA_RTQ)//2025-08-03 UBA500とは異なるがcheatではなくinvalid
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAYVALID)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_COLLECT)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_COLLECTED_WAIT_POLL)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_RETURN_TO_BOX)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYOUT)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYOUT_RETURN_NOTE)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYOUT_COLLECTED_WAIT_POLL)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYSTAY)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYSTAY_WAIT_POLL)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYVALID)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYVALID_ERROR)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_PAYVALID_ACK)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_PAYVALID_ACK_ERROR)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AFTER_PAYVALID_ACK_ENABLE)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AFTER_PAYVALID_ACK_DISABLE)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AFTER_PAYVALID_ACK_PAYOUT)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DIAGNOSTIC)
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_RETURN_ERROR)				
		#endif
				|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_REJECT_WAIT_POLL))
		{
			_id003_send_host_invalid();
		}
		/* Bar CouponのリカバリーにおいてPower Up後 Status Request受信前に */
		/* Stackコマンドを受信した場合にはInvalidを応答する */
		else if ((s_id003_powerup_stsreq_flag == 1)
			  && ((s_id003_powerup_stat == POWERUP_STAT_RECOVER_FEED_STACK)
			   || (s_id003_powerup_stat == POWERUP_STAT_RECOVER_STACK)
			   )
			  && (ex_cline_status_tbl.escrow_code == BAR_INDX))
		{
			_id003_send_host_invalid();
		}
		/* リカバリーにおいてPower Up後 Status Request受信前に */
		/* Stackコマンドを受信した場合にはInvalidを応答する */
		else if ((s_id003_powerup_stsreq_flag == 1)
			  && (s_id003_powerup_stat == POWERUP_STAT_REJECT) )
		{
			_id003_send_host_invalid();
		}
		/* [Polling Mode] 不正クレジット */
		/*  Stackingステータス設定後の最初のPollを受信するまで */
		else if ((ex_cline_status_tbl.comm_mode == 0x0000)
			  && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_STACK)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND_FULL))
			  && ((s_id003_stacking_info & ID003_STACKING_INFO_WAIT_STS) == ID003_STACKING_INFO_WAIT_STS))
		{
			_id003_send_host_invalid();
		}
		/* [Interrupt2] 不正クレジット */
		/*  VALIDステータス送信後のACK受信まで */
		else if ((ex_cline_status_tbl.comm_mode == 0x0002)
			  && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_STACK)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND_FULL)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_VEND_ACK)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_VEND_ACK_FULL)))
		{
			_id003_send_host_invalid();
		}
		/* [Interrupt1] 不正クレジット */
		/*  Stackingステータス設定まで */
		else
		{
			_id003_illegal_credit();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}
#endif // UBA_RTQ

/*********************************************************************//**
 * @brief Return Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_return_cmd_proc(void)
{
	if ((_id003_rx_buff.length == 0x01 + 0x04)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_ESCROW_WAIT_CMD)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD1)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD2)))
	{
		_set_id003_reject(ID003_MODE_RETURN, REJECT_CODE_RETURN);

		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

		if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
		 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
		{
			 id003_enc_update_context();
		}

		_id003_status_wait_clear();

		_id003_send_host_ack();

		/* change status */
		_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Hold Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_hold_cmd_proc(void)
{
	u32 time;

	if ((_id003_rx_buff.length == 0x01 + 0x04)
	  || (_id003_rx_buff.length == 0x02 + 0x04))
	{
		if ((_id003_rx_buff.length == 0x02 + 0x04)
		 && (_id003_rx_buff.data[0] != 0x00))
		{
			time = ((u32)(_id003_rx_buff.data[0]) * 1000);
		}
		else
		{
			time = 1000;
		}

		if (ex_cline_status_tbl.line_task_mode == ID003_MODE_ESCROW_WAIT_CMD)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_HOLD1;
			write_status_table();

			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD2, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_HOLD1, time, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_ack();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else if (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD1)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_HOLD2;
			write_status_table();

			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD1, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_HOLD2, time, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_ack();
		}
		else if (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD2)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_HOLD1;
			write_status_table();

			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD2, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_HOLD1, time, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_ack();
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Wait Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_wait_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		if (s_id003_status_wait_flag == 1)
		{
			s_id003_status_wait_flag = 2;
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_STATUS_WAIT1, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_STATUS_WAIT2, WAIT_TIME_STATUS_WAIT, 0, 0);
		}
		else if (s_id003_status_wait_flag == 2)
		{
			s_id003_status_wait_flag = 1;
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_STATUS_WAIT2, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_STATUS_WAIT1, WAIT_TIME_STATUS_WAIT, 0, 0);
		}
		else
		{
			s_id003_status_wait_next_mode = ex_cline_status_tbl.line_task_mode;
			s_id003_status_wait_flag = 1;
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_STATUS_WAIT1, WAIT_TIME_STATUS_WAIT, 0, 0);
		}

		_id003_send_host_ack();
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Book Mark Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
void _id003_bookmark_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		switch (ex_cline_status_tbl.line_task_mode)
		{
		case ID003_MODE_DISABLE:
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE_BOOKMARK;
			write_status_table();
			break;
		case ID003_MODE_DISABLE_REJECT:
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE_REJECT_BOOKMARK;
			write_status_table();
			break;
		case ID003_MODE_ENABLE:
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_BOOKMARK;
			write_status_table();
			break;
		case ID003_MODE_ENABLE_REJECT:
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_REJECT_BOOKMARK;
			write_status_table();
			break;
		default:
			break;
		}
		_id003_send_host_ack();
	}
	else
	{
		_id003_send_host_invalid();
	}
}
#endif

/*********************************************************************//**
 * @brief Book Mark cancel Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
void _id003_bookmark_cancel_cmd_proc(void)
{
	if ((_id003_rx_buff.length == 0x01 + 0x04)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT_BOOKMARK))
	   )
	{
		switch (ex_cline_status_tbl.line_task_mode)
		{
		case ID003_MODE_DISABLE_BOOKMARK:
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
			write_status_table();
			break;
		case ID003_MODE_DISABLE_REJECT_BOOKMARK:
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE_REJECT;
			write_status_table();
			break;
		case ID003_MODE_ENABLE_BOOKMARK:
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
			write_status_table();
			break;
		case ID003_MODE_ENABLE_REJECT_BOOKMARK:
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_REJECT;
			write_status_table();
			break;
		default:
			break;
		}
		_id003_send_host_ack();
	}
	else
	{
		_id003_send_host_invalid();
	}
}
#endif

/*********************************************************************//**
 * @brief ACK receiving procedure (after vend valid)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_ack_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		if (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_VEND_ACK)
		{
		#if defined(UBA_RTQ)
			set_recycle_total_count(RC_MODE_STACK, OperationDenomi.unit);
		#endif // UBA_RTQ

			set_recovery_step(RECOVERY_STEP_NON);

			if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
			 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
			{
				id003_enc_vendack_receive();
			}
			if ((ex_cline_status_tbl.comm_mode == 0x0002)
			 && ((s_id003_stacking_info & ID003_STACKING_INFO_BUSY) != ID003_STACKING_INFO_BUSY))
			{
				if (ex_cline_status_tbl.error_code != ALARM_CODE_OK)
				{
					_set_id003_alarm(ID003_MODE_ERROR, ex_cline_status_tbl.error_code);
				}
				else if (_is_id003_enable())
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					write_status_table();
					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
					write_status_table();
					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
				}
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_STACKED;
				write_status_table();
			}

			_id003_status_wait_clear();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);

		//#if (ID003_UBA==1)
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_1ST_NOTE_DONE_REQ, 0, 0, 0, 0); //Vendに対するAck受信
		//#endif

		}
		else if (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_VEND_ACK_FULL)
		{
		#if defined(UBA_RTQ) //追加
			set_recycle_total_count(RC_MODE_STACK, OperationDenomi.unit);
		#endif // UBA_RTQ

			set_recovery_step(RECOVERY_STEP_NON);

			if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
			 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
			{
				id003_enc_vendack_receive();
			}
			if ((ex_cline_status_tbl.comm_mode == 0x0002)
			 && ((s_id003_stacking_info & ID003_STACKING_INFO_BUSY) != ID003_STACKING_INFO_BUSY))
			{
				_set_id003_alarm(ID003_MODE_ERROR, ex_cline_status_tbl.error_code);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_STACKED_FULL;
				write_status_table();
			}

			_id003_status_wait_clear();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else if (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK) //ok
		{
			set_recovery_step(RECOVERY_STEP_NON);
		#if defined(UBA_RTQ)
			set_recycle_total_count(RC_MODE_STACK, OperationDenomi.unit);
		#endif // UBA_RTQ

			if (((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
			 && (ex_cline_status_tbl.escrow_code != BAR_INDX))
			{
			#if 1 //#if defined(ID003_SPECK64) //UBA_ID003_ENC
				if (id003_enc_mode != PAYOUT16MODE_SPECK)
			#endif
				{
					id003_enc_vendack_receive();
				}
			}

			if (ex_cline_status_tbl.error_code == ALARM_CODE_STACKER_FULL)
			{
				_set_id003_alarm(ID003_MODE_ERROR, ALARM_CODE_STACKER_FULL);
			}
			else if (_is_id003_enable())
			{
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					write_status_table();
					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
					write_status_table();
				}
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
			}

			_id003_status_wait_clear();

			/* リカバリVend用変数クリア */
			s_id003_powerup_stat = 0;  //UBA500はここに存在する

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		#if defined(UBA_RTQ)
		else if (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_PAYVALID_ACK) //ok
		{
			set_recycle_total_count(RC_MODE_PAYOUT, OperationDenomi.unit);

			set_recovery_step(RECOVERY_STEP_NON);

			if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
			{
				id003_enc_vendack_receive();
			}
  
			if (--OperationDenomi.remain == 0 || 	/* if remain and */
				ex_main_emergency_flag == 1)		/* emergency actived */
			{
			//払い出し終了	
			    ex_pre_feed_after_jam = 0;

				if (ex_main_emergency_flag == 1)
				{
					ex_main_emergency_flag = 0; /* reset flag emergency */
					if(_is_id003_enable())		/* check previous status */
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					}
					else
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
						// _id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
						// TODO: BEZEL off
					}
				}
				else
				{
					if (ex_cline_status_tbl.error_code != ALARM_CODE_OK)
					{
						_set_id003_alarm(ID003_MODE_ERROR, ex_cline_status_tbl.error_code);
					}
					/* 2024-09-25 - Concurrent Dispensing from Multiple-payout command (M_Payout) */
					else if((ConcurrentPayoutFlag == 1)  
					&& (CurrentBoxNumberIndx + 1 < 4)
					&& ((OperationDenomiCount[CurrentBoxNumberIndx + 1] != 0) 
						|| (OperationDenomiBoxNumber[CurrentBoxNumberIndx + 1] != 0))) 
					{
						// Continuous payout next box
						OperationDenomi.count =   OperationDenomiCount[CurrentBoxNumberIndx + 1];
						OperationDenomi.unit  =   OperationDenomiBoxNumber[CurrentBoxNumberIndx + 1];
					
						set_recovery_unit( OperationDenomi.unit, OperationDenomi.count );
						OperationDenomi.remain	= OperationDenomi.count; 
			
						//fid003_change_mode( ID003_MODE_AFTER_PAYVALID_ACK_PAYOUT );
						ex_cline_status_tbl.line_task_mode = ID003_MODE_AFTER_PAYVALID_ACK_PAYOUT;//払い出し継続
						_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode,0);

						
						s_id003_paying_info = (ID003_PAYING_INFO_BUSY|ID003_PAYING_INFO_WAIT_STS);
						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_PAYVALID_REQ, 0, 0, 0, 0);
			
						CurrentBoxNumberIndx ++;  // Update current index of box number
			
						if(ex_rc_status.sst1A.bit.quad) /* RQ */
						{
							if(CurrentBoxNumberIndx > 3)  // Box Number Index can be 0, 1, 2, 3
							{
								/* Cleare all setting for M_Payout command */
								clearConcurrentPayout();
							}
						}
						else  /* RT */
						{
						/* Cleare all setting for M_Payout command */
							clearConcurrentPayout();
						}
						
					}
			/* --- End M_Payout --- */
					else if (_is_id003_enable())
					{
					   	ex_cline_status_tbl.line_task_mode = ID003_MODE_AFTER_PAYVALID_ACK_ENABLE;	//すべての払い出し完了後の状態,mode_payoutをまだ抜けていないのでのEnableとは分けている
						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_PAYVALID_REQ, 0, 0, 0, 0);
					}
					else
					{
					    ex_cline_status_tbl.line_task_mode = ID003_MODE_AFTER_PAYVALID_ACK_DISABLE;	//すべての払い出し完了後の状態,mode_payoutをまだ抜けていないのでのDisableとは分けている
						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_PAYVALID_REQ, 0, 0, 0, 0);
					}
				}
			}
			else
			{
			//払い出し継続
				ex_cline_status_tbl.line_task_mode = ID003_MODE_AFTER_PAYVALID_ACK_PAYOUT;//払い出し継続
				s_id003_paying_info = (ID003_PAYING_INFO_BUSY|ID003_PAYING_INFO_WAIT_STS);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_PAYVALID_REQ, 0, 0, 0, 0);
			}
			_id003_status_wait_clear(); //ok

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else if (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_PAYVALID_ACK_ERROR) //ok
		{
			set_recycle_total_count(RC_MODE_PAYOUT, OperationDenomi.unit);

			set_recovery_step(RECOVERY_STEP_NON);

			if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
			{
				id003_enc_vendack_receive();
			}

			_set_id003_alarm(ID003_MODE_ERROR, ex_cline_status_tbl.error_code);

			_id003_status_wait_clear();
			/* リカバリVend用変数クリア */
			s_id003_powerup_stat = 0;
			/* 2024-09-25 - Concurrent Dispensing from Multiple-payout command (M_Payout) */
			// In case concurrent flag is ON, clear all concurrent setting when error occured.
			if(ConcurrentPayoutFlag == 1)
			{
				clearConcurrentPayout();
			} 
		}
		else if (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK) //ok
		{
			set_recycle_total_count(RC_MODE_PAYOUT, ex_recovery_info.unit_count);

			set_recovery_step(RECOVERY_STEP_NON);
			if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
			{
				id003_enc_vendack_receive();
			}

			ex_pre_feed_after_jam = 0;

			if( ex_cline_status_tbl.error_code != ALARM_CODE_OK )
			{
				_set_id003_alarm(ID003_MODE_ERROR, ex_cline_status_tbl.error_code);
			}
			else if (_is_id003_enable())
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
				/* BEZEL ON */
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
				/* BEZEL OFF */
			}
			_id003_status_wait_clear();
			/* リカバリVend用変数クリア */
        	s_id003_powerup_stat = 0;

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		#endif // UBA_RTQ
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Download Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_download_cmd_proc(void)
{
	if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_AT)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_SK)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	// || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_BOOKMARK)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ERROR)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_SYSTEM_ERROR))
	{
		_id003_send_host_ack();

		if( ID003_CMD_DOWNLOAD_REQUEST == _id003_rx_buff.cmd )
		{
			/* ダウンロード開始要求フラグ */
			// 通常ダウンロード
			ex_download_start_flg = DL_START_NORMAL;
		}
		else
		{
			/* ダウンロード開始要求フラグ */
			// 差分ダウンロード
			ex_download_start_flg = DL_START_DIFFERENTIAL;
		}
		_kernel_synch_cache();
		tx_alt_cache_purge_all();
		while(UART_is_txd_active());
		// Jumb to BIF Download
		hal_all_led_off();

		terminate_main_sys();

		/* BIFへジャンプ */
		bif_if_download_smp();
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Enable/Disable Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_enable_cmd_proc(void)
{
	if ((_id003_rx_buff.length == 0x03 + 0x04)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT)
	  //|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_BOOKMARK)
	  //|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_BOOKMARK)
	  //|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_BOOKMARK)
	  //|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_2ND)
	    )
	){
		ex_cline_status_tbl.bill_disable = (_id003_rx_buff.data[0] & 0x00FF);
		ex_cline_status_tbl.bill_disable |= ((_id003_rx_buff.data[1] << 8) & 0xFF00);

		if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
		 && (_is_id003_enable())
		 && ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_echoback();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else if (((ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE) || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL))
			 &&  !(_is_id003_enable())
			 &&  ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_echoback();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
	#if 0
		else if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_BOOKMARK)
		 && (_is_id003_enable())
		 && ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_BOOKMARK;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_echoback();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_BOOKMARK)
			 && !(_is_id003_enable())
			 && ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE_BOOKMARK;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_echoback();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
	#endif
		else
		{
			write_status_table();

			_id003_send_host_echoback();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Security Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_security_cmd_proc(void)
{
	if ((_id003_rx_buff.length == 0x03 + 0x04)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT)
	  //|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_BOOKMARK)
	  //|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_BOOKMARK)
	  //|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_BOOKMARK)
	  //|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_2ND)
	  )
	){
		ex_cline_status_tbl.security_level = (_id003_rx_buff.data[0] & 0x00FF);
		ex_cline_status_tbl.security_level |= ((_id003_rx_buff.data[1] << 8) & 0xFF00);

		write_status_table();

		_id003_send_host_echoback();
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Communication Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_communication_cmd_proc(void)
{
	if ((_id003_rx_buff.length == 0x02 + 0x04)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	//UBA500は仕様外だが、下記も有効にしているので、
	//おそらく、イニシャル時に内部的にEnable,Disableになるタイミングと、ポーリングレスポンスのタイミング
      || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
      || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
      || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
      || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT) 
	  )
	){
		if ((_id003_rx_buff.data[0] == 0x00)
		 || (_id003_rx_buff.data[0] == 0x01)
		 || (_id003_rx_buff.data[0] == 0x02))
		{
			ex_cline_status_tbl.comm_mode = (_id003_rx_buff.data[0] & 0x03);

			write_status_table();

			_id003_send_host_echoback();
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Inhibit Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_inhibit_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x02 + 0x04)
	{
		ex_cline_status_tbl.accept_disable = (_id003_rx_buff.data[0] & 0x0001);

		if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
		 && (_is_id003_enable())
		 && ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();
#if defined(UBA_RTQ)
			 /* Keyが有効の場合は無効にする */
			if(RecycleSettingInfo.key == 1)
			{
				RecycleSettingInfo.key = 0;
			}
#endif // uBA_RTQ
 			_id003_send_host_echoback();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else if (((ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE) || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL))
			 &&  !(_is_id003_enable())
			 &&  ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{

			if(s_id003_2nd_note == 2)/* 識別完了、識別OK */
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT_2ND, REJECT_CODE_RETURN); //Enable->Disableステータス
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, REJECT_CODE_RETURN, 0, 0, 0);
				//rejectで書いてる write_status_table();
				_id003_status_wait_clear();
				_id003_send_host_echoback();
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
			}
			else if(s_id003_2nd_note == 5)/* 識別完了、識別NGなので次からReReject */
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT_2ND, REJECT_CODE_RETURN); //Enable->Disableステータス
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, REJECT_CODE_RETURN, 0, 0, 0);
				//rejectで書いてる write_status_table();
				_id003_status_wait_clear();
				_id003_send_host_echoback();
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
			}
			else if(s_id003_2nd_note == 1)
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT_2ND, REJECT_CODE_RETURN); //Enable->Disableステータス
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, REJECT_CODE_RETURN, 0, 0, 0);
				//rejectで書いてる write_status_table();
				_id003_status_wait_clear();
				_id003_send_host_echoback();
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
			}	
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
				_id003_status_wait_clear();
				_id003_send_host_echoback();
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
			}
		}
		else if (((ex_cline_status_tbl.line_task_mode == ID003_MODE_ACCEPT)
			/*   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ACCEPT_BOOKMARK) */
			   )
			  && (ex_cline_status_tbl.accept_disable))
		{
			if(s_id003_2nd_note == 2)/* 識別完了、識別OKなので次からEscrow */
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT_2ND, REJECT_CODE_RETURN); //Accepting->Disableステータス
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, REJECT_CODE_RETURN, 0, 0, 0);
				_id003_send_host_echoback();
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
			}
			else if(s_id003_2nd_note == 5)/* 識別完了、識別NGなので次からReReject */
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT_2ND, REJECT_CODE_RETURN); //Accepting->Disableステータス
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, REJECT_CODE_RETURN, 0, 0, 0);
				_id003_send_host_echoback();
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
			}
			else if(s_id003_2nd_note == 1)
			{
				_set_id003_reject(ID003_MODE_REJECT_WAIT_ACCEPT_RSP, REJECT_CODE_INHIBIT);
				_id003_status_wait_clear();
				_id003_send_host_echoback();
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
			}
			else
			{
				_set_id003_reject(ID003_MODE_REJECT_WAIT_ACCEPT_RSP, REJECT_CODE_INHIBIT);
				_id003_status_wait_clear();
				_id003_send_host_echoback();
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
			}
		}
		else if (((ex_cline_status_tbl.line_task_mode == ID003_MODE_ESCROW)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ESCROW_WAIT_CMD)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD1)
			   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD2))
			  && (ex_cline_status_tbl.accept_disable))
		{
			_set_id003_reject(ID003_MODE_REJECT, REJECT_CODE_INHIBIT);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			_id003_status_wait_clear();
			_id003_send_host_echoback();
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else
		{
			write_status_table();
 			_id003_send_host_echoback();
		}
#if defined(UBA_RTQ)		/* '18-12-17 */
		/* Refill modeでInhibit設定の時のみ無効にする */
		if(OperationDenomi.mode && _id003_rx_buff.data[0] != 0)
		{
			OperationDenomi.mode = 0;
		}
#endif

	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Direction Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_direction_cmd_proc(void)
{
	if ((_id003_rx_buff.length == 0x02 + 0x04)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT)
	  //|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_BOOKMARK)
	  //|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_BOOKMARK)
	  //|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_BOOKMARK)
	  //|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_2ND)
	  )
	){
		ex_cline_status_tbl.direction_disable = (_id003_rx_buff.data[0] & 0x000F);

		if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
		 && (_is_id003_enable())
		 && ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_echoback();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else if (((ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE) || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL))
			 && !(_is_id003_enable())
			 &&  ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_echoback();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
	#if 0
		else if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_BOOKMARK)
		 && (_is_id003_enable())
		 && ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_BOOKMARK;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_echoback();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_BOOKMARK)
			 && !(_is_id003_enable())
			 && ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE_BOOKMARK;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_echoback();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
	#endif
		else
		{
			write_status_table();

			_id003_send_host_echoback();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Optional Function Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_optional_func_cmd_proc(void)
{
	if ((_id003_rx_buff.length == 0x03 + 0x04)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	//2024-05-14  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	//2024-05-14  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	//2024-05-14  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL)
	//2024-05-14  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
	//2024-05-14  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_2ND)
	  )
	){
	#if defined(UBA_RTQ) //2025-04-23 //#if defined(EXTEND_ENCRYPTION_PAY) //UBA500SSは EXTEND_ENCRYPTION_PAY を定義していない様だ
		if(ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION)
		{
			ex_cline_status_tbl.option = (_id003_rx_buff.data[0] & 0x00FF) | ID003_OPTION_ENCRYPTION;
		}
		else
		{
			ex_cline_status_tbl.option = (_id003_rx_buff.data[0] & 0x00FF);
		}
		ex_cline_status_tbl.option |= ((_id003_rx_buff.data[1] << 8) & 0xFF00);
	#else
		ex_cline_status_tbl.option = (_id003_rx_buff.data[0] & 0x00FF);
		ex_cline_status_tbl.option |= ((_id003_rx_buff.data[1] << 8) & 0xFF00);
	#endif

		write_status_table();

		_id003_send_host_echoback();
	}
	/* for Scientific Games  old machine use optional funciton 1 bayte*//* 2020-09-29 *//* sama as UBA500*/
	else if ((_id003_rx_buff.length == 0x02 + 0x04)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_2ND)
	  )
	)
    {
	#if defined(UBA_RTQ) //2025-04-23 //#if defined(EXTEND_ENCRYPTION_PAY) //UBA500SSは EXTEND_ENCRYPTION_PAY を定義していない様だ
		if(ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION)
		{
			ex_cline_status_tbl.option = (_id003_rx_buff.data[0] & 0x00FF) | ID003_OPTION_ENCRYPTION;
		}
		else
		{
			ex_cline_status_tbl.option = (_id003_rx_buff.data[0] & 0x00FF);
		}
		ex_cline_status_tbl.option |= ((_id003_rx_buff.data[1] << 8) & 0xFF00);
	#else
		ex_cline_status_tbl.option = (_id003_rx_buff.data[0] & 0x00FF);
		ex_cline_status_tbl.option |= ((_id003_rx_buff.data[1] << 8) & 0xFF00);
	#endif

		write_status_table();

		_id003_send_host_echoback();
	}
	else
	{
		_id003_send_host_invalid();
	}
}

#if defined(UBA_RTQ)//#if defined(UBA_ID003_ENC)
void _id003_decrypt() //ok SPECK 通常 共用 Payout
{
	u8 result;
	/* 暗号化無効の場合 */
    if( (ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) != ID003_OPTION_ENCRYPTION )
    {
        _id003_send_host_invalid();
        return;
    }
	/* 暗号化されたコマンドを復号化する */
    result = id003_decryptcmd((u8 *) &(_id003_rx_buff)); //ok
    /* 復号化の結果がNGの場合はINVALID */
    if( result != TRUE )
    {
	//SPECKの両立できる //#if defined(ID003_SPECK64)
		if((ex_illigal_payout_command > 2) && (id003_enc_mode == PAYOUT16MODE_SPECK))
		{
			ex_illigal_payout_command = 0;
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);						
		}
		_id003_send_host_invalid();
	}
	else
	{
		_id003_rx_buff.length = _id003_rx_buff.length - 2;  //ここは +4 してはダメ、ここは純粋に暗号化の為に増えたサイズを減らしている
        _id003_rx_buff.cmd    = _id003_rx_buff.data[0];

		//CRC以外をずらす様だ、ソース的にはCRCまでずらしても問題ない様だ
		for(u8 i = 0; i < (_id003_rx_buff.length - ID003_ADD_04) ; i++)
        {
            _id003_rx_buff.data[i] = _id003_rx_buff.data[i+1];
        }
		//#if defined(ID003_SPECK64)
		if((_id003_rx_buff.cmd == 0x03) // ID003_CMD_DLE_KEY_NUM_SET
		&& (id003_enc_mode == PAYOUT16MODE_SPECK))
		{
			/* Encryptted Key, and Numberコマンド処理関数コール */
			_id003_set_encrypted_keynumber_cmd_proc(); //ok SPECK用
		}
		//#endif
		else if(_id003_rx_buff.cmd == ID003_CMD_EXT_RC)
		{
			/* Payoutコマンド処理関数コール */
			_id003_rc_payout_cmd_proc(); //中でAck設定している ok
		}
		else
		{
			id003_enc_cancel_update_context(); //ok
			_id003_send_host_invalid();
		}
	}
}
#endif // UBA_ID003_ENC


/*********************************************************************//**
 * @brief Encryption Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_encryption_cmd_proc(void) //ok 0x10 暗号化データ
{
	/* POWERUP,ENABLE,DISABLE,INITIALIZEｽﾃｰﾀｽ時のみ有効 */
	//2025-03-28
/*---------------------------------------------------------------------*/
/* 下記SPEC対応用*/
	if((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_AT
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_SK
		|| ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR)
		&& (id003_enc_mode == PAYOUT16MODE_SPECK) ) 
	{
		_id003_send_host_invalid();
	}
	else if((_id003_rx_buff.data[0] == ID003_CMD_DLE_ENCKEY_SET) && (_id003_rx_buff.length == (0x0A  + ID003_ADD_04) )
	 && (id003_enc_mode == PAYOUT16MODE_SPECK))
	{
		_id003_send_host_invalid();		
	}
	else if((_id003_rx_buff.data[0] == ID003_CMD_DLE_ENCNUM_SET) && (_id003_rx_buff.length == (0x03 + ID003_ADD_04))
	 && (id003_enc_mode == PAYOUT16MODE_SPECK))
	{
		_id003_send_host_invalid();
	}
/*---------------------------------------------------------------------*/
/* 下記SPEC対応、未対応 両方で併用可能 */
	//SPEC対応時に、id003_enc_mode確認が追加されている、SPEC未対応でもそのままの条件で問題ない
	//SSも使用している
	else if ((_id003_rx_buff.data[0] == ID003_CMD_DLE_ENCKEY_SET)				/* Make key */
		&& (_id003_rx_buff.length == (0x0A + ID003_ADD_04))
		&& (id003_enc_mode != PAYOUT16MODE_SPECK))
	{
	//SPEC対応前のコマンド、SPEC対応ソフトでもこの条件で問題ない
		_id003_encryption_key_cmd_proc();	//2025-03-28
	}
	//SSも使用している
	else if ((_id003_rx_buff.data[0] == ID003_CMD_DLE_ENCNUM_SET)			/* Set number */
			&& (_id003_rx_buff.length == (0x03 + ID003_ADD_04))
			&& (id003_enc_mode != PAYOUT16MODE_SPECK))
	{
	//SPEC対応前のコマンド、SPEC対応ソフトでもこの条件で問題ない
		_id003_encryption_set_number_cmd_proc();	//2025-03-28
	}
#if defined(UBA_RTQ)
	/* PAYOUT decrypt process */
	else if (((_id003_rx_buff.length == 0x07 + ID003_ADD_04) && (id003_enc_mode == PAYOUT8MODE))||  		// Encrypt 8-byte length = 0x0B
				((_id003_rx_buff.length == 0x0F + ID003_ADD_04) && (id003_enc_mode == PAYOUT16MODE))|| 		// Encrypt 16-byte length = 0x0F
				((_id003_rx_buff.length == 0x0F + ID003_ADD_04) && (id003_enc_mode == PAYOUT16MODE_SPECK))) 	// Encrypt SPECK 16-byte  length = 0x0F
	{
	//SPECにも対応
		/* Decrypted data */
		_id003_decrypt(); //中でAck設定している ok
	}
#endif
	else
	{
		_id003_send_host_invalid();
	}
}

#if 1//#if defined(ID003_SPECK64)
/*********************************************************************//**
 * @brief Encryption set key, number Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_set_encrypted_keynumber_cmd_proc(void) //ok SPECK用
{
	if((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_AT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_SK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	//  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	//  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	//  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL)
	//  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
	//  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT)
	  && (id003_enc_mode == PAYOUT16MODE_SPECK))
	{
		id003_enc_make_key(&_id003_rx_buff.data[1]);	 
		id003_enc_set_number(_id003_rx_buff.data[12]);
		_id003_send_host_ack();
		/* 暗号化 コンテキスト更新 */
		id003_enc_update_context(); // Key/nummer set  update the context

	}
	else
	{
		_id003_send_host_invalid(); 
		id003_enc_cancel_update_context();   
	}
}
#endif // UBA_ID003_ENC

/*********************************************************************//**
 * @brief Barcode Function Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_barcode_func_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x03 + 0x04)
	{
		ex_cline_status_tbl.barcode_type = _id003_rx_buff.data[0];
		ex_cline_status_tbl.barcode_length = _id003_rx_buff.data[1];

		write_status_table();

		_id003_send_host_echoback();
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Barcode Inhibit Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_barcode_inhibit_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x02 + 0x04)
	{
		ex_cline_status_tbl.barcode_inhibit = _id003_rx_buff.data[0];

		if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
		 && (_is_id003_enable())
		 && ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_echoback();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else if (((ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE) || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL))
			 &&  !(_is_id003_enable())
			 &&  ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			_id003_send_host_echoback();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 1);
		}
		else
		{
			write_status_table();

			_id003_send_host_echoback();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Enable/Disable Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_get_enable_cmd_proc(void)
{
#if (MULTI_COUNTRY == 0)
	extern const u8 id003_bill_info[18][6];
	extern const u16 id003_inhi_mask;
#else  /* MULTI_COUNTRY != 0 */
	extern const u8 id003_bill_info[MULTI_COUNTRY][18][6];
	extern const u16 id003_inhi_mask[MULTI_COUNTRY];
#endif /* MULTI_COUNTRY == 0 */
	extern const u16 dipsw_inhi[];
	extern const u8 id003_index_to_code[];
	u16 inhi = 0;
	u16 denomi_idx;
	u16 binfo_idx;
    u16 tmp_id003_inhi_mask;

/***/
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
#if (MULTI_COUNTRY == 0)
		for (denomi_idx = 0; denomi_idx < (DENOMI_SIZE / 4); denomi_idx++)
		{
			if ((ex_cline_status_tbl.dipsw_disable & dipsw_inhi[denomi_idx]) != 0)
			{
				for (binfo_idx = 0; ((id003_bill_info[binfo_idx][0] != 0) && (binfo_idx < 18)); binfo_idx++)
				{
					if (id003_index_to_code[denomi_idx] == id003_bill_info[binfo_idx][0])
					{
						inhi |= ((u16)(id003_bill_info[binfo_idx][4] << 8) | (u16)(id003_bill_info[binfo_idx][5]));
					}
				}
			}
		}
		if( (0xff00 & id003_inhi_mask) == 0 )
		{
		    tmp_id003_inhi_mask = (id003_inhi_mask | 0xFF00);
		}
		else
		{
			tmp_id003_inhi_mask = id003_inhi_mask;
		}
		inhi = ((ex_cline_status_tbl.bill_disable | inhi) | ~tmp_id003_inhi_mask);

#else  /* MULTI_COUNTRY != 0 */
		for (denomi_idx = 0; denomi_idx < (DENOMI_SIZE / 4); denomi_idx++)
		{
			if ((ex_cline_status_tbl.dipsw_disable & dipsw_inhi[denomi_idx]) != 0)
			{
				for (binfo_idx = 0; ((id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][0] != 0) && (binfo_idx < 18)); binfo_idx++)
				{
					if (id003_index_to_code[denomi_idx] == id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][0])
					{
						inhi |= ((u16)(id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][4] << 8) | (u16)(id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][5]));
					}
				}
			}
		}

		if( (0xff00 & id003_inhi_mask[ex_cline_status_tbl.country_setting]) == 0 )
		{
		    tmp_id003_inhi_mask = (id003_inhi_mask[ex_cline_status_tbl.country_setting] | 0xFF00);
		}
		inhi = ((ex_cline_status_tbl.bill_disable | inhi) | ~tmp_id003_inhi_mask);

#endif /* MULTI_COUNTRY == 0 */

		_id003_tx_buff.length = 3 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_ENABLE;
		_id003_tx_buff.data[0] = (u8)(inhi & 0x00FF);
		_id003_tx_buff.data[1] = (u8)((inhi >> 8) & 0x00FF);

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Security Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_get_security_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		_id003_tx_buff.length = 3 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_SECURITY;
		_id003_tx_buff.data[0] = (ex_cline_status_tbl.security_level & 0x00FF);
		_id003_tx_buff.data[1] = ((ex_cline_status_tbl.security_level & 0xFF00) >> 8);

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Communication Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_get_comm_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		_id003_tx_buff.length = 2 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_COMMUNICATION;
		_id003_tx_buff.data[0] = (ex_cline_status_tbl.comm_mode & 0x03);

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Inhibit Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_get_inhibit_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		_id003_tx_buff.length = 2 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_INHIBIT;
		_id003_tx_buff.data[0] = (ex_cline_status_tbl.accept_disable & 0x0001);

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Direction Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_get_direction_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		_id003_tx_buff.length = 2 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_DIRECTION;
		_id003_tx_buff.data[0] = (ex_cline_status_tbl.direction_disable & 0x000F);

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Optional Funcktion Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_get_option_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		_id003_tx_buff.length = 3 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_OPTIONAL_FUNC;
		_id003_tx_buff.data[0] = (ex_cline_status_tbl.option & 0x00FF);
		_id003_tx_buff.data[1] = ((ex_cline_status_tbl.option & 0xFF00) >> 8);

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Barcode Funcktion Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_get_barcode_func_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		_id003_tx_buff.length = 3 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_BARCODE_FUNC;
		_id003_tx_buff.data[0] = ex_cline_status_tbl.barcode_type;
		_id003_tx_buff.data[1] = ex_cline_status_tbl.barcode_length;

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Barcode Inhibit Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_get_barcode_inhibit_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		_id003_tx_buff.length = 2 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_BARCODE_INHIBIT;
		_id003_tx_buff.data[0] = ex_cline_status_tbl.barcode_inhibit;

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Version Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_version_cmd_proc(void)
{
	extern const u8 software_ver[];
	extern const u8 id003_protocol_ver[];
	u16 txbuff_index;
	u16 temp_index;
	u8 seq;
	u8 crc_char;

	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		_id003_version_cmd_proc_uba();
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Boot Version Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_boot_version_cmd_proc(void)
{
	u8 *ptr;

	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		ptr = (UB *)( (UW *)(0x00140540) );

		_id003_tx_buff.length = 5 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_BOOT_VERSION;
		_id003_tx_buff.data[0] = 'B';
		_id003_tx_buff.data[1] = ptr[7];
		_id003_tx_buff.data[2] = ptr[8];
		_id003_tx_buff.data[3] = 0;

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Currency Assing Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_currency_assing_cmd_proc(void)
{
#if (MULTI_COUNTRY == 0)
	extern const u8 id003_bill_info[18][6];
#else  /* MULTI_COUNTRY != 0 */
	extern const u8 id003_bill_info[MULTI_COUNTRY][18][6];
#endif /* MULTI_COUNTRY == 0 */
	u16 cnt;
	u16 data_len;

	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		_id003_tx_buff.cmd = ID003_CMD_GET_CURRENCY_ASSING;

		data_len = 0;
		for (cnt = 0; cnt < 18; cnt++)
		{
#if (MULTI_COUNTRY == 0)
			if (id003_bill_info[cnt][0] != 0x00)
			{
				_id003_tx_buff.data[data_len] = id003_bill_info[cnt][0]; /* escrow code */
				_id003_tx_buff.data[(data_len + 1)] = id003_bill_info[cnt][1]; /* country code */
				_id003_tx_buff.data[(data_len + 2)] = id003_bill_info[cnt][2]; /* denomination data */
				_id003_tx_buff.data[(data_len + 3)] = id003_bill_info[cnt][3]; /* denomination data */
				data_len += 4;
#else  /* MULTI_COUNTRY != 0 */
			if (id003_bill_info[ex_cline_status_tbl.country_setting][cnt][0] != 0x00)
			{
				_id003_tx_buff.data[data_len] = id003_bill_info[ex_cline_status_tbl.country_setting][cnt][0]; /* escrow code */
				_id003_tx_buff.data[(data_len + 1)] = id003_bill_info[ex_cline_status_tbl.country_setting][cnt][1]; /* country code */
				_id003_tx_buff.data[(data_len + 2)] = id003_bill_info[ex_cline_status_tbl.country_setting][cnt][2]; /* denomination data */
				_id003_tx_buff.data[(data_len + 3)] = id003_bill_info[ex_cline_status_tbl.country_setting][cnt][3]; /* denomination data */
				data_len += 4;
#endif /* MULTI_COUNTRY == 0 */
			}
		}
		_id003_tx_buff.length = (1 + data_len) + 4;

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}
/*********************************************************************//**
 * @brief Program Signature Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_program_signature_proc(void)
{
	if (((ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
		|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_ERROR))
		&& (_id003_rx_buff.length == 0x03 + 0x04))
	{
		/* 仕様書とHIGH,LOWが逆だが既存iVIZIONに合わせる */
		ex_crc16_seed = (u16)(_id003_rx_buff.data[1] << 8);
		ex_crc16_seed += (u16)(_id003_rx_buff.data[0]);
		_cline_send_msg(ID_DISCRIMINATION_MBX, TMSG_SIGNATURE_REQ, SIGNATURE_CRC16, ex_cline_status_tbl.line_task_mode, 0, 0);
		_id003_send_host_ack();
		ex_cline_status_tbl.line_task_mode = ID003_MODE_SIGNATURE_BUSY;
	}
	else if (((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP)
		|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_AT)
		|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_SK)
		|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR))
		&& (_id003_rx_buff.length == 0x03 + 0x04))
	{
		/* 仕様書とHIGH,LOWが逆だが既存iVIZIONに合わせる */
		ex_crc16_seed = (u16)(_id003_rx_buff.data[1] << 8);
		ex_crc16_seed += (u16)(_id003_rx_buff.data[0]);
		_cline_send_msg(ID_DISCRIMINATION_MBX, TMSG_SIGNATURE_REQ, SIGNATURE_CRC16, ex_cline_status_tbl.line_task_mode, 0, 0);
		_id003_send_host_ack();
		ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_SIGNATURE_BUSY;
	}
	else
	{
		_id003_send_host_invalid();
	}
}

/*********************************************************************//**
 * @brief Program SHA1 Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_sha1_hash_cmd_proc(void){
		
	//u16 seed;
	if (((ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_ERROR))
	&& (_id003_rx_buff.length == 0x17 + 0x04))
	{
		ex_sha1_seed[0] = (u32)(_id003_rx_buff.data[2] << 24)
						| (u32)(_id003_rx_buff.data[3] << 16)
						| (u32)(_id003_rx_buff.data[4] << 8)
						| (u32)_id003_rx_buff.data[5];//0x67452301;
		ex_sha1_seed[1] = (u32)(_id003_rx_buff.data[6] << 24)
						| (u32)(_id003_rx_buff.data[7] << 16)
						| (u32)(_id003_rx_buff.data[8] << 8)
						| (u32)_id003_rx_buff.data[9];//0xEFCDAB89;
		ex_sha1_seed[2] = (u32)(_id003_rx_buff.data[10] << 24)
						| (u32)(_id003_rx_buff.data[11] << 16)
						| (u32)(_id003_rx_buff.data[12] << 8)
						| (u32)_id003_rx_buff.data[13];//0x98BADCFE;
		ex_sha1_seed[3] = (u32)(_id003_rx_buff.data[14] << 24)
						| (u32)(_id003_rx_buff.data[15] << 16)
						| (u32)(_id003_rx_buff.data[16] << 8)
						| (u32)_id003_rx_buff.data[17];//0x10325476;
		ex_sha1_seed[4] = (u32)(_id003_rx_buff.data[18] << 24)
						| (u32)(_id003_rx_buff.data[19] << 16)
						| (u32)(_id003_rx_buff.data[20] << 8)
						| (u32)_id003_rx_buff.data[21];//0xC3D2E1F0;

		//ex_sha1_seed[0] = 0x67452301;
		//ex_sha1_seed[1] = 0xEFCDAB89;
		//ex_sha1_seed[2] = 0x98BADCFE;
		//ex_sha1_seed[3] = 0x10325476;
		//ex_sha1_seed[4] = 0xC3D2E1F0;

		//seed = (_id003_rx_buff.data[0] << 8)| _id003_rx_buff.data[1];
		_cline_send_msg(ID_DISCRIMINATION_MBX, TMSG_SIGNATURE_REQ, SIGNATURE_SHA1, ex_cline_status_tbl.line_task_mode, 0, 0);
		_id003_send_host_ack();
		ex_cline_status_tbl.line_task_mode = ID003_MODE_SHA1_HASH_BUSY;

	}
	else if (((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP)
		|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_AT)
		|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_SK)
		|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR))
		&& (_id003_rx_buff.length == 0x17 + 0x04))
	{
		ex_sha1_seed[0] = (u32)(_id003_rx_buff.data[2] << 24)
						| (u32)(_id003_rx_buff.data[3] << 16)
						| (u32)(_id003_rx_buff.data[4] << 8)
						| (u32)_id003_rx_buff.data[5];//0x67452301;
		ex_sha1_seed[1] = (u32)(_id003_rx_buff.data[6] << 24)
						| (u32)(_id003_rx_buff.data[7] << 16)
						| (u32)(_id003_rx_buff.data[8] << 8)
						| (u32)_id003_rx_buff.data[9];//0xEFCDAB89;
		ex_sha1_seed[2] = (u32)(_id003_rx_buff.data[10] << 24)
						| (u32)(_id003_rx_buff.data[11] << 16)
						| (u32)(_id003_rx_buff.data[12] << 8)
						| (u32)_id003_rx_buff.data[13];//0x98BADCFE;
		ex_sha1_seed[3] = (u32)(_id003_rx_buff.data[14] << 24)
						| (u32)(_id003_rx_buff.data[15] << 16)
						| (u32)(_id003_rx_buff.data[16] << 8)
						| (u32)_id003_rx_buff.data[17];//0x10325476;
		ex_sha1_seed[4] = (u32)(_id003_rx_buff.data[18] << 24)
						| (u32)(_id003_rx_buff.data[19] << 16)
						| (u32)(_id003_rx_buff.data[20] << 8)
						| (u32)_id003_rx_buff.data[21];//0xC3D2E1F0;

		//ex_sha1_seed[0] = 0x67452301;
		//ex_sha1_seed[1] = 0xEFCDAB89;
		//ex_sha1_seed[2] = 0x98BADCFE;
		//ex_sha1_seed[3] = 0x10325476;
		//ex_sha1_seed[4] = 0xC3D2E1F0;

		//seed = (_id003_rx_buff.data[0] << 8)| _id003_rx_buff.data[1];
		_cline_send_msg(ID_DISCRIMINATION_MBX, TMSG_SIGNATURE_REQ, SIGNATURE_SHA1, ex_cline_status_tbl.line_task_mode, 0, 0);
		_id003_send_host_ack();
		ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_SHA1_HASH_BUSY;
	}
	else
	{
		_id003_send_host_invalid();
	}
}
/*********************************************************************//**
 * @brief Secret Number Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_secret_number_cmd_proc(void)
{
	if (_id003_rx_buff.length == 0x01 + ID003_ADD_04)
	{
		/*** Xorshift RNGs **************************** '22-07-26 */
		/* create secret number from random seed value */
		create_secret_number();	//

		_id003_tx_buff.length = 4 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_SECRET_NUMBER;
		_id003_tx_buff.data[0] = Random_Secret_no[0];
		_id003_tx_buff.data[1] = Random_Secret_no[1];
		_id003_tx_buff.data[2] = Random_Secret_no[2];

		uart_send_id003(&_id003_tx_buff);
	}
	#if defined(UBA_RTQ)//#if defined(ID003_SPECK64) //defined(UBA_ID003_ENC) //SPECK専用コマンド 
	else if ((_id003_rx_buff.length == 0x02 + ID003_ADD_04) && _id003_rx_buff.data[0] == 0x01)
	{

		if( ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP
			&& ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_AT
			&& ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_SK
			&& ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_ERROR)
		{
			_id003_send_host_invalid();
		}
		else
		{
		//ID003シミュレータではこっちのSecret numberコマンドを使用してSPECKモードにしている
		//UBA500RTQ同様、このコマンドでOptional funciotnの暗号化も有効になる
		/* Encryption mode ON */
			ex_cline_status_tbl.option = ID003_OPTION_ENCRYPTION;
			/* Set encryption mode to SPECK */

			//SPECKモードになるコマンドは2つ
			//Secret numbertコマンドの拡張版 or _id003_enc_mode_cmd_proc
			id003_enc_mode = PAYOUT16MODE_SPECK;

			/*** Xorshift RNGs **************************** '22-07-26 */
			/* create secret number from random seed value */
			create_secret_number();
			//2025-03-28
			_id003_send_enc_msg(ID003_CMD_GET_SECRET_NUMBER, 0);
			/* Context更新のためのPoll�?ちフラグをON */
			_id003_sts_poll = 1;			 
			/* update random seed value */
			update_random_seed(0);
		}
	}
	#endif
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Serial Number Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_serial_number_cmd_proc(void)
{
	u16 index;
#if defined(UBA_RTQ)
	u8 *ptr;
#endif

	if (_id003_rx_buff.length == 0x01 + 0x04)
	{
		_id003_tx_buff.length = 13 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_SERIAL_NUMBER;

		for (index = 0; index < 12; index++)
		{
			_id003_tx_buff.data[index] = ex_adjustment_data.maintenance_info.serial_no[index];
		}

		uart_send_id003(&_id003_tx_buff);
	}
#if defined(UBA_RTQ)
	else if(_id003_rx_buff.length == 0x02 + 0x04)
	{
		if(_id003_rx_buff.data[0] > 3 || _id003_rx_buff.data[0] == 0 || (_id003_rx_buff.data[0] == 3 && ex_rc_status.sst1A.bit.quad == 0))
		{
			_id003_send_host_invalid();
		}
		else
		{
			_id003_tx_buff.length = 14 + 0x04;
			_id003_tx_buff.cmd = ID003_CMD_GET_SERIAL_NUMBER;
			_id003_tx_buff.data[0] = _id003_rx_buff.data[0];

			if(_id003_rx_buff.data[0] == 1)				/* main serial */
			{
				ptr = &read_mente_serailno_data[0].serial_no[0];
			}
			else if(_id003_rx_buff.data[0] == 2)		/* unit1 serial */
			{
				ptr = &read_mente_serailno_data[1].serial_no[0];
			}
			else if(_id003_rx_buff.data[0] == 3)		/* unit2 serial */
			{
				ptr = &read_mente_serailno_data[2].serial_no[0];
			}

			for(index = 0; index < 12; index++)
			{
				_id003_tx_buff.data[index + 1] = *ptr; 
				++ptr;
			}
			uart_send_id003(&_id003_tx_buff);
		}
	}
#endif
	else
	{
		_id003_send_host_invalid();
	}
}


#if (MULTI_COUNTRY != 0)
/*********************************************************************//**
 * @brief Country Type setting Command receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_country_type_cmd_proc(void)
{
	extern const u8 ex_multi_country_info[MULTI_COUNTRY][3];
	u8 cnt;

	if ((_id003_rx_buff.length == 0x03 + 0x04)
	 && (_id003_rx_buff.data[0] == 0x01)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_BOOKMARK)
	    )
	   )
	{
		for (cnt = 0; cnt < MULTI_COUNTRY; cnt++)
		{
			if (ex_multi_country_info[cnt][0] == _id003_rx_buff.data[1])
			{
				break;
			}
		}
		if (cnt < MULTI_COUNTRY)
		{
			ex_cline_status_tbl.country_setting = cnt;

			write_status_table();

			_id003_send_host_echoback();
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}


/*********************************************************************//**
 * @brief Country Type setting Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_get_country_type_cmd_proc(void)
{
	extern const u8 ex_multi_country_info[MULTI_COUNTRY][3];
	extern const u8 id003_bill_info[MULTI_COUNTRY][18][6];
	u8 index;
	u16 cnt;
	u16 data_len;

	if ((_id003_rx_buff.length == 0x02 + 0x04)
	 && (_id003_rx_buff.data[0] == 0x01)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ESCROW_WAIT_CMD)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD1)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD2)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT_BOOKMARK)
	    )
	   )
	{
		_id003_tx_buff.length = 3 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_COUNTRY_TYPE;
		_id003_tx_buff.data[0] = _id003_rx_buff.data[0];
		_id003_tx_buff.data[1] = ex_multi_country_info[ex_cline_status_tbl.country_setting][0];

		uart_send_id003(&_id003_tx_buff);
	}
	else if ((_id003_rx_buff.length == 0x02 + 0x04)
	 && (_id003_rx_buff.data[0] == 0x02)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT_BOOKMARK)
	    )
	   )
	{
		_id003_tx_buff.length = (2 + MULTI_COUNTRY) + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_COUNTRY_TYPE;
		_id003_tx_buff.data[0] = _id003_rx_buff.data[0];
		for (index = 0; index < MULTI_COUNTRY; index++)
		{
			_id003_tx_buff.data[(index + 1)] = ex_multi_country_info[index][0];
		}

		uart_send_id003(&_id003_tx_buff);
	}
	else if ((_id003_rx_buff.length == 0x03 + 0x04)
	 && (_id003_rx_buff.data[0] == 0x03)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_BOOKMARK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT_BOOKMARK)
	    )
	   )
	{
		for (index = 0; index < MULTI_COUNTRY; index++)
		{
			if (ex_multi_country_info[index][0] == _id003_rx_buff.data[1])
			{
				break;
			}
		}
		if (index < MULTI_COUNTRY)
		{
			_id003_tx_buff.cmd = ID003_CMD_GET_COUNTRY_TYPE;
			_id003_tx_buff.data[0] = _id003_rx_buff.data[0];

			data_len = 1;
			for (cnt = 0; cnt < 18; cnt++)
			{
				if (id003_bill_info[index][cnt][0] != 0x00)
				{
					_id003_tx_buff.data[data_len] = id003_bill_info[index][cnt][0]; /* escrow code */
					_id003_tx_buff.data[(data_len + 1)] = id003_bill_info[index][cnt][1]; /* country code */
					_id003_tx_buff.data[(data_len + 2)] = id003_bill_info[index][cnt][2]; /* denomination data */
					_id003_tx_buff.data[(data_len + 3)] = id003_bill_info[index][cnt][3]; /* denomination data */
					data_len += 4;
				}
			}
			_id003_tx_buff.length = (1 + data_len) + 4;

			uart_send_id003(&_id003_tx_buff);
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}
#endif /* MULTI_COUNTRY != 0 */

/*********************************************************************//**
 * @brief ICB Box Machine Number Setting Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_icb_mc_number_cmd_proc(void)
{
	int index;

	if ((_id003_rx_buff.length == 5 + 20) && (is_icb_enable())
		 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
		  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
		))
	{
		for (index = 0; index < 20; index++)
		{
			ex_ICB_gameno[index] = _id003_rx_buff.data[index];
		}
		_cline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_SETTING, 0, 0, 0);
		_id003_send_host_echoback();
	}
	else
	{
		_id003_send_host_invalid();
	}
}
/*********************************************************************//**
 * @brief ICB Function Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_icb_function_cmd_proc(void)
{
	if ((_id003_rx_buff.length == 0x02 + 0x04)
	 && (_id003_rx_buff.data[0] == ID003_SUBCMD_ICB_REQ)
	){
		_id003_tx_buff.length = (2 + 1) + 4;
		_id003_tx_buff.cmd = ID003_CMD_ICB_FUNCTION;
		_id003_tx_buff.data[0] = ID003_SUBCMD_ICB_REQ;
		if(is_icb_enable())
		{
			_id003_tx_buff.data[1] = ID003_ICB_ENABLE_STATUS;
		}
		else
		{
			_id003_tx_buff.data[1] = ID003_ICB_DISABLE_STATUS;
		}

		uart_send_id003(&_id003_tx_buff);
	}
#if 0 //iPRO UBA500では対応していないので、無効にしておく 0x8EのBox Numberと重複している 0x8Eは対応している
	else if ((_id003_rx_buff.length == 0x02 + 0x04)
	 && (_id003_rx_buff.data[0] == ID003_SUBCMD_MC_NO_REQ)
	){
		_id003_tx_buff.length = (2 + 20) + 4;
		_id003_tx_buff.cmd = ID003_CMD_ICB_FUNCTION;
		_id003_tx_buff.data[0] = ID003_SUBCMD_MC_NO_REQ;

		for (int index = 0; index < 20; index++)
		{
			_id003_tx_buff.data[1 + index] = ex_ICB_boxno[index];
		}

		uart_send_id003(&_id003_tx_buff);
	}
#endif
	else if ((_id003_rx_buff.length == 0x03 + 0x04)
	 && (_id003_rx_buff.data[0] == ID003_SUBCMD_ICB_SET)
	 && ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	  || (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	)){
		// initial status 時のみ
		if(_id003_rx_buff.data[1] == ID003_ICB_ENABLE_STATUS)
		{
			set_ICBenable_flag();
			_cline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_SETTING, 0, 0, 0);
		}
		else if(_id003_rx_buff.data[1] == ID003_ICB_DISABLE_STATUS)
		{
			set_ICBdisable_flag();
			_cline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_SETTING, 0, 0, 0);
		}
		_id003_send_host_echoback();
	}
	else
	{
		_id003_send_host_invalid();
	}
}

/*********************************************************************//**
 * @brief Machine Number Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_get_machine_number_cmd_proc(void)
{
	struct _Smrtdat *icb;
#if !defined(DIS_ICB)
	icb = (struct _Smrtdat *)&Smrtdat;
#endif

	if ((_id003_rx_buff.length == 0x01 + 0x04) && (is_icb_enable()))
	{
		_id003_tx_buff.length = (1 + 20) + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_MACHINE_NUMBER;

		for (int index = 0; index < 20; index++)
		{
			_id003_tx_buff.data[index] = ex_ICB_gameno[index];
		}

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}

/*********************************************************************//**
 * @brief ICB Box Number Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_get_box_number_cmd_proc(void)
{
	struct _Smrtdat *icb;
#if !defined(DIS_ICB)
	icb = (struct _Smrtdat *)&Smrtdat;
#endif
	if ((_id003_rx_buff.length == 0x01 + 0x04) && (is_icb_enable()))
	{
		_id003_tx_buff.length = (1 + 20) + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_BOX_NUMBER;

		for (int index = 0; index < 20; index++)
		{
			_id003_tx_buff.data[index] = ex_ICB_boxno[index];
		}

		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}

void _id003_revision_number_cmd_proc(void)
{

	if( _id003_rx_buff.length == 0x01 + 0x04 )
	{
#if defined(UBA_RTQ)
		/* RC-Quad model */
		if(ex_rc_status.sst1A.bit.quad)
		{
	    //    _id003_tx_buff.length = 5;
	        _id003_tx_buff.length = 5 + 4;

	        _id003_tx_buff.cmd = ID003_CMD_GET_REVISION_NUMBER;
			_id003_tx_buff.data[0] = read_editionno_data.head[0];
			_id003_tx_buff.data[1] = read_editionno_data.main[0];
			_id003_tx_buff.data[2] = read_editionno_data.twin[0];
			_id003_tx_buff.data[3] = read_editionno_data.quad[0];
		}
		/* RC-Twin model */
		else
		{
	    //    _id003_tx_buff.length = 4;
	        _id003_tx_buff.length = 4 + 4;

	        _id003_tx_buff.cmd = ID003_CMD_GET_REVISION_NUMBER;
			_id003_tx_buff.data[0] = read_editionno_data.head[0];
			_id003_tx_buff.data[1] = read_editionno_data.main[0];
			_id003_tx_buff.data[2] = read_editionno_data.twin[0];
		}
#else
		_id003_tx_buff.length = 2 + 4;
		_id003_tx_buff.cmd = ID003_CMD_GET_REVISION_NUMBER;
#endif
        uart_send_id003(&_id003_tx_buff);
    }
    else
    {
        _id003_send_host_invalid();
    }
}


/*********************************************************************//**
 * @brief authentication Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
void _id003_authentication_cmd_proc(void)
{
	u16 index;

	if(ex_Authentication.functionStatus == NORMAL_SETTING_AUTHENTICATION_NUMBER)
	{
		if (_id003_rx_buff.length == 0x01 + 0x04)
		{
			_id003_tx_buff.length = (1 + AUTHENTICATION_NUMBER_KEY_SIZE + AUTHENTICATION_CODE_SIZE) + 4;
			_id003_tx_buff.cmd = ID003_CMD_AUTHENTICATION;

			for (index = 0; index < AUTHENTICATION_NUMBER_KEY_SIZE; index++)
			{
				_id003_tx_buff.data[index] = ex_Authentication.numberKEY[index];
			}
			for (index = 0; index <  AUTHENTICATION_CODE_SIZE; index++)
			{
				_id003_tx_buff.data[AUTHENTICATION_NUMBER_KEY_SIZE + index] = ex_Authentication.code[index];
			}
			uart_send_id003(&_id003_tx_buff);
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}
#endif

#ifdef TWO_D_EXTERNED_BARCODE_TICKET
/*********************************************************************//**
 * @brief 2D barcode number Request receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_get_2d_barocode_cmd_proc(void)
{
	u16 index;
	u16 first_size;

	first_size = ex_bar_length[1];
	if(ex_bar_length[0]%2)
	{
		first_size++;
	}

	if ((_id003_rx_buff.length == 0x02 + 0x04)
	 && (_id003_rx_buff.data[0] == 0))
	{
		_id003_tx_buff.length = 1 + first_size + 4;
		_id003_tx_buff.cmd = ID003_CMD_RXD_2D_BARCODE;

		for(index = 0; index < first_size; index++)
		{
			_id003_tx_buff.data[index] = ex_barcode[index];
		}
		uart_send_id003(&_id003_tx_buff);
	}
	else if ((_id003_rx_buff.length == 0x02 + 0x04)
	 && (_id003_rx_buff.data[0] == 1))
	{
		_id003_tx_buff.length = 1 + ex_bar_length[0] - first_size + 4;
		_id003_tx_buff.cmd = ID003_CMD_RXD_2D_BARCODE;

		for(index = 0; index < ex_bar_length[0] - first_size; index++)
		{
			_id003_tx_buff.data[index] = ex_barcode[index + first_size];
		}
		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
		_id003_send_host_invalid();
	}
}
#endif

/*********************************************************************//**
 * @brief Send ACK
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_send_host_ack(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_CMD_ACK;
	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief Send Invalid
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_send_host_invalid(void)
{
	_id003_tx_buff.length = 1 + 4;
	_id003_tx_buff.cmd = ID003_CMD_INVALID;
	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief Send Echo back
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_send_host_echoback(void)
{
	u16 cnt;

	_id003_tx_buff.length = _id003_rx_buff.length;
	_id003_tx_buff.cmd = _id003_rx_buff.cmd;
	if (_id003_tx_buff.length - 5 >= 1)
	{
		for (cnt = 0; cnt < (_id003_tx_buff.length - 5); cnt++)
		{
			_id003_tx_buff.data[cnt] = _id003_rx_buff.data[cnt];
		}
	}
	uart_send_id003(&_id003_tx_buff);
}


/*********************************************************************//**
 * @brief Send communication error
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_send_host_commu_err(void)
{
	if (s_id003_illegal_credit == 0)
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_STS_COMMUNICATION_ERROR;
		uart_send_id003(&_id003_tx_buff);
	}
}


/*********************************************************************//**
 * @brief Send communication error
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_send_enc_msg(u8 status, u8 escrow_code)
{
	u8 crc1 = 0;
	u8 crc2 = 0;
	u8 enc_buff[8];

#if 1 //2025-03-28	
	if(status == ID003_CMD_GET_SECRET_NUMBER)
	{
	//SPECK用 特殊はSecret番号リクエストの場合、レスポンスも暗号化
		crccal( 0xFC, &crc1, &crc2 );                       /* CRC演算 */

		_id003_tx_buff.length = 0x0B; 	//ここは +4 しなくてよい、uart_send_plain_id003 が若干異なっているため
		crccal( _id003_tx_buff.length, &crc1, &crc2 );      /* CRC演算 */

		_id003_tx_buff.cmd = ID003_CMD_DLE;
		crccal( _id003_tx_buff.cmd, &crc1, &crc2 );         /* CRC演算 */

		enc_buff[0] = _id003_tx_buff.data[0] = status;
		crccal( _id003_tx_buff.data[0], &crc1, &crc2 );     /* CRC演算 */

		enc_buff[1] = _id003_tx_buff.data[1] = Random_Secret_no[0] ^ Random_Secret_no[2];
		crccal( _id003_tx_buff.data[1], &crc1, &crc2 );     /* CRC演算 */

		enc_buff[2] = _id003_tx_buff.data[2] = Random_Secret_no[1] ^ Random_Secret_no[2];
		crccal( _id003_tx_buff.data[2], &crc1, &crc2 );     /* CRC演算 */

		enc_buff[3] = _id003_tx_buff.data[3] = Random_Secret_no[0];
		crccal( _id003_tx_buff.data[3], &crc1, &crc2 );     /* CRC演算 */

		enc_buff[4] = _id003_tx_buff.data[4] = Random_Secret_no[1];
		crccal( _id003_tx_buff.data[4], &crc1, &crc2 );     /* CRC演算 */

		enc_buff[5] = _id003_tx_buff.data[5] = Random_Secret_no[2];
		crccal( _id003_tx_buff.data[5], &crc1, &crc2 );     /* CRC演算 */

		enc_buff[6] = _id003_tx_buff.data[6] = crc1;
		enc_buff[7] = _id003_tx_buff.data[7] = crc2;
	}
	else
	{
	//Normal 暗号化データ SSはこっちしか使用しない
		crccal( 0xFC, &crc1, &crc2 );                       /* CRC演算 */

		_id003_tx_buff.length = 0x0B; //ここは +4 しなくてよい、uart_send_plain_id003 が若干異なっているため
		crccal( _id003_tx_buff.length, &crc1, &crc2 );      /* CRC演算 */

		_id003_tx_buff.cmd = ID003_CMD_DLE;
		crccal( _id003_tx_buff.cmd, &crc1, &crc2 );         /* CRC演算 */

		enc_buff[0] = _id003_tx_buff.data[0] = status;
		crccal( _id003_tx_buff.data[0], &crc1, &crc2 );     /* CRC演算 */

		enc_buff[1] = _id003_tx_buff.data[1] = escrow_code;
		crccal( _id003_tx_buff.data[1], &crc1, &crc2 );     /* CRC演算 */

		enc_buff[2] = _id003_tx_buff.data[2] = Random_Secret_no[0];
		crccal( _id003_tx_buff.data[2], &crc1, &crc2 );     /* CRC演算 */

		enc_buff[3] = _id003_tx_buff.data[3] = Random_Secret_no[1];
		crccal( _id003_tx_buff.data[3], &crc1, &crc2 );     /* CRC演算 */

		enc_buff[4] = _id003_tx_buff.data[4] = Random_Secret_no[2];
		crccal( _id003_tx_buff.data[4], &crc1, &crc2 );     /* CRC演算 */

		enc_buff[5] = _id003_tx_buff.data[5] = id003_enc_get_number();
		crccal( _id003_tx_buff.data[5], &crc1, &crc2 );     /* CRC演算 */

		enc_buff[6] = _id003_tx_buff.data[6] = crc1;
		enc_buff[7] = _id003_tx_buff.data[7] = crc2;
	}

#else
	crccal(0xFC, &crc1, &crc2);							/* CRC演算 */
//V022 2024-09-11	_id003_tx_buff.length = 0x0B + 4;	//ここは +4 しなくてよい、uart_send_plain_id003 が若干異なっているため
	_id003_tx_buff.length = 0x0B;

	crccal(_id003_tx_buff.length, &crc1, &crc2);		/* CRC演算 */
	_id003_tx_buff.cmd = ID003_CMD_DLE;
	crccal(_id003_tx_buff.cmd, &crc1, &crc2);			/* CRC演算 */
	enc_buff[0] = _id003_tx_buff.data[0] = status;
	crccal(_id003_tx_buff.data[0], &crc1, &crc2);		/* CRC演算 */
	enc_buff[1] = _id003_tx_buff.data[1] = escrow_code;
	crccal(_id003_tx_buff.data[1], &crc1, &crc2);		/* CRC演算 */
	enc_buff[2] = _id003_tx_buff.data[2] = Random_Secret_no[0];
	crccal(_id003_tx_buff.data[2], &crc1, &crc2);		/* CRC演算 */
	enc_buff[3] = _id003_tx_buff.data[3] = Random_Secret_no[1];
	crccal(_id003_tx_buff.data[3], &crc1, &crc2);		/* CRC演算 */
	enc_buff[4] = _id003_tx_buff.data[4] = Random_Secret_no[2];
	crccal(_id003_tx_buff.data[4], &crc1, &crc2);		/* CRC演算 */
	enc_buff[5] = _id003_tx_buff.data[5] = id003_enc_get_number();
	crccal(_id003_tx_buff.data[5], &crc1, &crc2);		/* CRC演算 */
	enc_buff[6] = _id003_tx_buff.data[6] = crc1;
	enc_buff[7] = _id003_tx_buff.data[7] = crc2;

#endif

	id003_enc_encode(&enc_buff[0], &_id003_tx_buff.data[0]); //ok

	//2024-09-11 uart_send_id003(&_id003_tx_buff);
	uart_send_plain_id003(&_id003_tx_buff); //for encryption

}

void _id003_signature_rsp_msg_proc(void){
        
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_SIGNATURE_BUSY:
		if(cline_msg.arg2 == SIGNATURE_CRC16)
		{
			if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_SIGNATURE_END;
			}
			else
			{
				/* system error ? */
				_cline_system_error(0, 17);
				_id003_send_host_ack();
			}
		}
		break;
	case ID003_MODE_POWERUP_SIGNATURE_BUSY:
		if(cline_msg.arg2 == SIGNATURE_CRC16)
		{
			if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_SIGNATURE_END;
			}
			else
			{
				/* system error ? */
				_cline_system_error(0, 17);
				_id003_send_host_ack();
			}
		}
		break;
	case ID003_MODE_SHA1_HASH_BUSY:
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_SHA1_HASH_END;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 17);
			_id003_send_host_ack();
		}
		break;
	case ID003_MODE_POWERUP_SHA1_HASH_BUSY:
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_SHA1_HASH_END;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 17);
			_id003_send_host_ack();
		}
		break;
	default:					/* other */
		break;
	}
}
/*********************************************************************//**
 * @brief TMSG
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_msg_proc(void)
{
	switch (cline_msg.tmsg_code)
	{
	case TMSG_UART01CB_CALLBACK_INFO:
		_id003_callback_msg_proc();
		break;
	case TMSG_CLINE_STATUS_INFO:
		log_id003(1);	//2025-05-16
		_id003_status_info_msg_proc();
		break;
	case TMSG_CLINE_RESET_RSP:
		log_id003(2);	//2025-05-16
		_id003_reset_rsp_msg_proc();
		break;
	case TMSG_CLINE_DISABLE_RSP:
		log_id003(3);	//2025-05-16
		_id003_disable_rsp_msg_proc();
		break;
	case TMSG_CLINE_ENABLE_RSP:
		log_id003(4);	//2025-05-16
		_id003_enable_rsp_msg_proc();
		break;
	case TMSG_CLINE_ACCEPT_RSP:
		log_id003(5);	//2025-05-16
		_id003_accept_rsp_msg_proc();
		break;
	case TMSG_CLINE_STACK_RSP:
		log_id003(6);	//2025-05-16
		_id003_stack_rsp_msg_proc();
		break;
	case TMSG_CLINE_REJECT_RSP:
		log_id003(7);	//2025-05-16
		_id003_reject_rsp_msg_proc();
		break;
	case TMSG_TIMER_TIMES_UP:
		log_id003(8);	//2025-05-16
		_id003_times_up_msg_proc();
		break;
	case TMSG_SIGNATURE_RSP:
		log_id003(9);	//2025-05-16
		_id003_signature_rsp_msg_proc();
		break;
	case TMSG_FRAM_READ_RSP:
	case TMSG_FRAM_WRITE_RSP:
		// TODO:
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_COLLECT_RSP:
		log_id003(10);	//2025-05-16
		_id003_collect_rsp_msg_proc();
		break;
	case TMSG_CLINE_PAYOUT_RSP:
		log_id003(11);	//2025-05-16
		_id003_payout_rsp_msg_proc();
		break;
#endif // UBA_RTQ
	default:					/* other */
		break;
	}
}

/*********************************************************************//**
 * @brief UART1 callback message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_callback_msg_proc(void)
{
	switch (cline_msg.arg1)
	{
	case TMSG_SUB_RECEIVE:
		_id003_callback_receive();
		break;
	case TMSG_SUB_EMPTY:
		_id003_callback_empty();
		break;
	case TMSG_SUB_ALARM:
		_id003_callback_alarm();
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
void _id003_callback_empty(void)
{
}
/*********************************************************************//**
 * @brief UART1 callback RXD message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_callback_receive(void)
{
	u8 rtn;
	rtn = uart_listen_id003(&_id003_rx_buff);
	if (rtn == ID003_LISTEN_COMMAND)
	{
	/* Receiving command */
		_id003_cmd_proc();
	}
}
/*********************************************************************//**
 * @brief UART1 callback error message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_callback_alarm(void)
{
	s_id003_communication_stsreq_flag = 1;
}

/*********************************************************************//**
 * @brief Status infomation message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_status_info_msg_proc(void)
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_POWERUP:
	case ID003_MODE_POWERUP_BILLIN_AT:
	case ID003_MODE_POWERUP_BILLIN_SK:
	case ID003_MODE_POWERUP_ERROR:
		_id003_status_info_mode_powerup(); //エラー + success
		break;
	case ID003_MODE_ERROR:
		_id003_status_info_mode_error(); //エラー + success
		break;
	case ID003_MODE_SYSTEM_ERROR:
		// nothing todo
		break;
	default:					/* other */
		_id003_status_info_mode_def(); //エラー + success
		break;
	}
}


/*********************************************************************//**
 * @brief Status infomation message receiving [Mode : power-up relate]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_status_info_mode_powerup(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
		#if defined(UBA_RTQ)
			power_recover_003_rtq(cline_msg.arg2, 3);
		#else
			power_recover_003(cline_msg.arg2, 3); //2024-03-28
		#endif
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if ((ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_ERROR)
			 || (ex_cline_status_tbl.error_code != cline_msg.arg2))
			{
				s_id003_powerup_stat = 0;

				_set_id003_alarm(ID003_MODE_POWERUP_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 9);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
		#if defined(UBA_RTQ)
			power_recover_003_rtq(cline_msg.arg2, 4);
		#else
			power_recover_003(cline_msg.arg2, 4); //2024-03-28
		#endif
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_powerup_stat = 0;

			s_id003_status_wait_next_mode = ID003_MODE_POWERUP_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 10);
		}
	}
}


/*********************************************************************//**
 * @brief Status infomation message receiving [Mode : ESCROW]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_status_info_mode_escrow(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_cline_status_tbl.error_code != cline_msg.arg2)
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 11);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_reject = cline_msg.arg2;
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 12);
		}
	}
}


/*********************************************************************//**
 * @brief Status infomation message receiving [Mode : ESCROW]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_status_info_mode_vend(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if( cline_msg.arg2 == ALARM_CODE_BOX)
			{
				if (ex_cline_status_tbl.error_code != cline_msg.arg2)
				{
					_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
			}
			else
			{
				if (ex_cline_status_tbl.error_code != cline_msg.arg2)
				{
					ex_cline_status_tbl.error_code = cline_msg.arg2;
					write_status_table();
				}
			}
		}
		else if (cline_msg.arg1 != TMSG_SUB_REJECT)
		{
			/* system error ? */
			_cline_system_error(0, 62);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if( cline_msg.arg2 == ALARM_CODE_BOX)
			{
				if (ex_cline_status_tbl.error_code != cline_msg.arg2)
				{
					s_id003_status_wait_next_mode = ID003_MODE_ERROR;
					s_id003_status_wait_error = cline_msg.arg2;
				}
			}
			else
			{
				s_id003_status_wait_next_mode = ID003_MODE_ERROR;
				s_id003_status_wait_error = cline_msg.arg2;
			}
		}
		else if (cline_msg.arg1 != TMSG_SUB_REJECT)
		{
			/* system error ? */
			_cline_system_error(0, 63);
		}
	}
}


/*********************************************************************//**
 * @brief Status infomation message receiving [Mode : error]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_status_info_mode_error(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY;
			//_id003_status_tbl_reset();
			ex_cline_status_tbl.escrow_code = 0x0000;
			ex_cline_status_tbl.error_code = ALARM_CODE_OK;
			ex_cline_status_tbl.reject_code = 0x0000;
			ex_cline_status_tbl.accept_disable = 0x0001;
			write_status_table();

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);

			if (s_id003_illegal_credit != 0)
			{
				s_id003_illegal_credit = 0;
				uart_txd_active_id003();
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_cline_status_tbl.error_code != cline_msg.arg2)
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 != TMSG_SUB_REJECT)
		{
			/* system error ? */
			_cline_system_error(0, 13);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_status_wait_next_mode = ID003_MODE_AUTO_RECOVERY;
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else if (cline_msg.arg1 != TMSG_SUB_REJECT)
		{
			/* system error ? */
			_cline_system_error(0, 14);
		}
	}
}


/*********************************************************************//**
 * @brief Status infomation message receiving [Mode : default]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_status_info_mode_def(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_cline_status_tbl.error_code != cline_msg.arg2)
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if( cline_msg.arg1 == TMSG_SUB_SUCCESS ) //2023-11-28
		{
			/* Mainをwaitさせる */
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_WAIT, 0, 0, 0);
		}
		else if (cline_msg.arg1 != TMSG_SUB_REJECT)
		{
			/* system error ? */
			_cline_system_error(0, 15);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else if( cline_msg.arg1 == TMSG_SUB_SUCCESS ) //2023-11-28
		{
			/* Mainをwaitさせる */
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_WAIT, 0, 0, 0);
		}
		else if (cline_msg.arg1 != TMSG_SUB_REJECT)
		{
			/* system error ? */
			_cline_system_error(0, 16);
		}
	}
}


/*********************************************************************//**
 * @brief Reset response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_reset_rsp_msg_proc(void)
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_POWERUP_INITIAL:
		_id003_reset_rsp_mode_power_init(); //rejectあり ,TMSG_SUB_INTERIMなし ok
		break;
	case ID003_MODE_INITIAL:
		_id003_reset_rsp_mode_init(); //rejectあり,TMSG_SUB_INTERIMなし ok
		break;
	case ID003_MODE_AUTO_RECOVERY:
		_id003_reset_rsp_mode_recovery(); //rejectあり,TMSG_SUB_INTERIMなし ok
		break;
	/* イニシャル動作継続 */
	case ID003_MODE_POWERUP_INITIAL_STACK:
	case ID003_MODE_POWERUP_INITIAL_PAUSE:
		_id003_stack_rsp_mode_power_init(); //rejectなし,TMSG_SUB_INTERIMなし ok
		break;
	case ID003_MODE_INITIAL_STACK:
	case ID003_MODE_INITIAL_PAUSE:
		_id003_stack_rsp_mode_init(); //rejectなし,TMSG_SUB_INTERIMなし ok
		break;
	case ID003_MODE_AUTO_RECOVERY_STACK:
	case ID003_MODE_AUTO_RECOVERY_PAUSE:
		_id003_stack_rsp_mode_recovery(); //rejectなし,TMSG_SUB_INTERIMなし ok
		break;

		//2025-02-10  _id003_reject_rsp_msg_proc から移動
	case ID003_MODE_POWERUP_INITIAL_REJECT:
		_id003_reject_rsp_mode_power_init_rej(); //rejectあり,TMSG_SUB_INTERIMあり ok
		break;
//	case ID003_MODE_POWERUP_INITIAL_PAUSE:
//		_id003_reject_rsp_mode_power_init_pau(); //rejectあり,TMSG_SUB_INTERIMなし ok
//		break;
	case ID003_MODE_INITIAL_REJECT:
		_id003_reject_rsp_mode_init_rej(); //rejectあり,TMSG_SUB_INTERIMあり ok
		break;
//	case ID003_MODE_INITIAL_PAUSE:
//		_id003_reject_rsp_mode_init_pau(); //rejectあり,TMSG_SUB_INTERIMなし ok
//		break;
	case ID003_MODE_AUTO_RECOVERY_REJECT:
		_id003_reject_rsp_mode_recovery_rej(); //rejectあり,TMSG_SUB_INTERIMあり ok
		break;
//	case ID003_MODE_AUTO_RECOVERY_PAUSE:
//		_id003_reject_rsp_mode_recovery_pau(); //rejectあり,TMSG_SUB_INTERIMなし ok
//		break;

		default:					/* other */
		break;
	}
}


/*********************************************************************//**
 * @brief Reset response message receiving [Mode : power-up initial relate]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_reset_rsp_mode_power_init(void) //TMSG_CLINE_RESET_RSPから
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			if (((s_id003_powerup_stat == POWERUP_STAT_RECOVER_FEED_STACK)
			  || (s_id003_powerup_stat == POWERUP_STAT_RECOVER_STACK) //リカバリー Vend
			))
			{
				//2024-03-28
				if((ex_cline_status_tbl.option & ID003_OPTION_RECOVERY) == ID003_OPTION_RECOVERY)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_VEND;
					write_status_table();

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);

				}
				else
				{
					set_recovery_step(RECOVERY_STEP_NON);
					if (_is_id003_enable())
					{
						if (ex_cline_status_tbl.comm_mode == 0x0002)
						{
							ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
							write_status_table();

							_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
						}
						else
						{
							//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
							ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
							write_status_table();

							//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
						}
					}
					else
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
						write_status_table();

						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
					}
					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
			}
#if defined(UBA_RTQ)
			else if (s_id003_powerup_stat == POWERUP_STAT_RECOVER_PAYOUT)
			{
			//通常のリカバリでのイニシャル完了でのPay Valid
				ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_PAYVALID;
				write_status_table();
			}	
#endif // UBA_RTQ
			else
			{
				set_recovery_step(RECOVERY_STEP_NON);
				if (_is_id003_enable())
				{
					if (ex_cline_status_tbl.comm_mode == 0x0002)
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
						write_status_table();

						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
					}
					else
					{
						//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
						write_status_table();

						//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
					}
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
					write_status_table();

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
				}
				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_REMAIN)
		{
#if defined(UBA_RTQ)
			/* Bill Remain */
			if (cline_msg.arg2 == BILL_IN_STACKER)
			{
				if (cline_msg.arg3 == BILL_CHEAT)
				{
					/* set status */
					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
				}
				else
				{
					/* RC Stack */
					ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_STACK;
					write_status_table();

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RC_RECOVERY_REQ, cline_msg.arg3, cline_msg.arg4, 0, 0);
				}
			}
			else
			{
				/* Reject */
				_set_id003_reject(ID003_MODE_POWERUP_INITIAL_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			}
#else
			/* Bill Remain */
			if (cline_msg.arg2 == BILL_IN_STACKER)
			{
			/* Stack */
				ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_STACK;
				write_status_table();

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, 0, 0, 0, 0);
			}
			else
			{
			/* Reject */
				_set_id003_reject(ID003_MODE_POWERUP_INITIAL_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			}
#endif // UBA_RTQ	
		}
		//else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
		/* Reject */
			_set_id003_reject(ID003_MODE_POWERUP_INITIAL_REJECT, (u16)cline_msg.arg2);
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if ((cline_msg.arg2 == ALARM_CODE_STACKER_FULL)
			 && ((s_id003_powerup_stat == POWERUP_STAT_RECOVER_FEED_STACK)
			  || (s_id003_powerup_stat == POWERUP_STAT_RECOVER_STACK)// リカバリー Full Vend
			  ))
			{
				if((ex_cline_status_tbl.option & ID003_OPTION_RECOVERY) == ID003_OPTION_RECOVERY)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_VEND;
					ex_cline_status_tbl.error_code = cline_msg.arg2;
					write_status_table();

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
				else
				{
					_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
			}
		#if defined(UBA_RTQ)	//2025-05-22
			else if (s_id003_powerup_stat == POWERUP_STAT_RECOVER_PAYOUT)
			{
			//パワーリカバリでPayValid送信前のエラー用、PayValid優先
				ex_cline_status_tbl.error_code = (u16)cline_msg.arg2;
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYVALID_ERROR;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}	
		#endif
			else
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 17);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_status_wait_next_mode = ID003_MODE_DISABLE;
		}
		else if (cline_msg.arg1 == TMSG_SUB_REMAIN)
		/* Bill Remain */
		{
			if (cline_msg.arg2 == BILL_IN_STACKER)
			{
			/* Stack */
				s_id003_status_wait_next_mode = ID003_MODE_POWERUP_INITIAL_STACK;
			}
			else
			{
			/* Reject */
				s_id003_status_wait_next_mode = ID003_MODE_POWERUP_INITIAL_REJECT;
				s_id003_status_wait_reject = REJECT_CODE_ACCEPTOR_STAY_PAPER;
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			s_id003_status_wait_next_mode = ID003_MODE_POWERUP_INITIAL_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
		#if defined(UBA_RTQ)	//2025-05-22
			if (s_id003_powerup_stat == POWERUP_STAT_RECOVER_PAYOUT)
			{
			//パワーリカバリでPayValid送信前のエラー用、PayValid優先
				ex_cline_status_tbl.error_code = (u16)cline_msg.arg2;
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYVALID_ERROR;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				s_id003_status_wait_next_mode = ID003_MODE_ERROR;
				s_id003_status_wait_error = cline_msg.arg2;
			}	
		#else
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		#endif
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 18);
		}
	}
}


/*********************************************************************//**
 * @brief Reset response message receiving [Mode : initial relate]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_reset_rsp_mode_init(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else if (_is_id003_enable())
			{
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					write_status_table();

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
				else
				{
					//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
					write_status_table();
					//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
			}

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_REMAIN)
		{
		/* Bill Remain */
			if (cline_msg.arg2 == BILL_IN_STACKER)
			{
#if defined(UBA_RTQ)
				ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL_STACK;
				write_status_table();

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RC_RECOVERY_REQ, cline_msg.arg3, cline_msg.arg4, 0, 0);
#else
			/* Stack */
				ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL_STACK;
				write_status_table();

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, 0, 0, 0, 0);
#endif // UBA_RTQ	
			}
			else
			{
			/* Reject */
				_set_id003_reject(ID003_MODE_INITIAL_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			}
		}
		//else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
		/* Reject */
			_set_id003_reject(ID003_MODE_INITIAL_REJECT, (u16)cline_msg.arg2);
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 19);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_status_wait_next_mode = ID003_MODE_DISABLE;
		}
		else if (cline_msg.arg1 == TMSG_SUB_REMAIN)
		{
		/* Bill Remain */
			if (cline_msg.arg2 == BILL_IN_STACKER)
			{
			/* Stack */
				s_id003_status_wait_next_mode = ID003_MODE_INITIAL_STACK;
			}
			else
			{
			/* Reject */
				s_id003_status_wait_next_mode = ID003_MODE_INITIAL_REJECT;
				s_id003_status_wait_reject = REJECT_CODE_ACCEPTOR_STAY_PAPER;
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			s_id003_status_wait_next_mode = ID003_MODE_INITIAL_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 20);
		}
	}
}


/*********************************************************************//**
 * @brief Reset response message receiving [Mode : auto recovery relate]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_reset_rsp_mode_recovery(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else if (_is_id003_enable())
			{
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					write_status_table();

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
				else
				{
					//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
					write_status_table();
					//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
			}

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_REMAIN)
		{
		/* Bill Remain */
			if (cline_msg.arg2 == BILL_IN_STACKER)
			{
			/* Stack */
				ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY_STACK;
				write_status_table();

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, 0, 0, 0, 0);
			}
			else
			{
			/* Reject */
				_set_id003_reject(ID003_MODE_AUTO_RECOVERY_REJECT, REJECT_CODE_ACCEPTOR_STAY_PAPER);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			}
		}
		//else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
		/* Reject */
			_set_id003_reject(ID003_MODE_AUTO_RECOVERY_REJECT, (u16)cline_msg.arg2);
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 21);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_status_wait_next_mode = ID003_MODE_DISABLE;
		}
		else if (cline_msg.arg1 == TMSG_SUB_REMAIN)
		{
		/* Bill Remain */
			if (cline_msg.arg2 == BILL_IN_STACKER)
			{
			/* Stack */
				s_id003_status_wait_next_mode = ID003_MODE_AUTO_RECOVERY_STACK;
			}
			else
			{
			/* Reject */
				s_id003_status_wait_next_mode = ID003_MODE_AUTO_RECOVERY_REJECT;
				s_id003_status_wait_reject = REJECT_CODE_ACCEPTOR_STAY_PAPER;
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			s_id003_status_wait_next_mode = ID003_MODE_AUTO_RECOVERY_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 22);
		}
	}
}

#if defined(UBA_RTQ)
static void _id003_disable_rsp_mode_collect(void)//mainのDisableからの受信 //_id003_disable_mode_collect 使用しているか不明 2025-03-16 保留
{
	ex_cline_status_tbl.line_task_mode = ID003_MODE_COLLECT;
	/* change status */
	_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);

	
	if (!(is_rc_twin_d1_empty())) /* RC-Twin drum1 is not empty */
	{
		OperationDenomi.unit = 1;
	}
	else if (!(is_rc_twin_d2_empty())) /* RC-Twin drum2 is not empty */
	{
		OperationDenomi.unit = 2;
	}
	else if (!(is_rc_quad_d1_empty()) && ex_rc_status.sst1A.bit.quad) /* RC-Quad drum1 is not empty */
	{
		OperationDenomi.unit = 3;
	}
	else if (!(is_rc_quad_d2_empty()) && ex_rc_status.sst1A.bit.quad) /* RC-Quad drum2 is not empty */
	{
		OperationDenomi.unit = 4;
	}
	set_recovery_unit( OperationDenomi.unit, OperationDenomi.unit );
	
	OperationDenomi.count	= 0;
	s_id003_collecting_info = (ID003_COLLECTING_INFO_BUSY|ID003_COLLECTING_INFO_WAIT_STS);
	_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_COLLECT_REQ, 0, 0, 0, 0);
}
#endif // UBA_RTQ


/*********************************************************************//**
 * @brief Disable response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_disable_rsp_msg_proc(void) //2025-03-16 _id003_disable_mode_collect
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_DISABLE:
		if (s_id003_status_wait_flag == 0)
		{
		/* Normal (Not Status WAIT) */

			if (cline_msg.arg1 == TMSG_SUB_REJECT)
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT, (u16)cline_msg.arg2);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			}
			else if (cline_msg.arg1 == TMSG_SUB_ALARM)
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
#if defined(UBA_RTQ)
			else if (cline_msg.arg1 == TMSG_SUB_COLLECT)
			{
				//#if defined(UBA_RS)
				ex_rs_payout_remain_flag = RS_NOTE_REMAIN_NONE;
				//#endif
				_id003_disable_rsp_mode_collect();
			}
#endif // UBA_RTQ
			else
			{
				/* system error ? */
				_cline_system_error(0, 23);
			}
		}
		else
		{
		/* Status WAIT */
			if (cline_msg.arg1 == TMSG_SUB_REJECT)
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT, (u16)cline_msg.arg2);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			}
			else if (cline_msg.arg1 == TMSG_SUB_ALARM)
			{
				s_id003_status_wait_next_mode = ID003_MODE_ERROR;
				s_id003_status_wait_error = cline_msg.arg2;
			}
#if defined(UBA_RTQ)
			else if (cline_msg.arg1 == TMSG_SUB_COLLECT)
			{
				//#if defined(UBA_RS)
				ex_rs_payout_remain_flag = RS_NOTE_REMAIN_NONE;
				//#endif
				_id003_disable_rsp_mode_collect();
			}
#endif // UBA_RTQ
			else
			{
				/* system error ? */
				_cline_system_error(0, 24);
			}
		}
		break;

		//ID003_MODE_AFTER_PAYVALID_ACK_DISABLE	

	#if 0
	case ID003_MODE_DISABLE_BOOKMARK:
		if (s_id003_status_wait_flag == 0)
		{
		/* Normal (Not Status WAIT) */

			if (cline_msg.arg1 == TMSG_SUB_REJECT)
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT_BOOKMARK, (u16)cline_msg.arg2);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			}
			else if (cline_msg.arg1 == TMSG_SUB_ALARM)
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				/* system error ? */
				_cline_system_error(0, 23);
			}
		}
		else
		{
		/* Status WAIT */
			if (cline_msg.arg1 == TMSG_SUB_REJECT)
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT_BOOKMARK, (u16)cline_msg.arg2);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			}
			else if (cline_msg.arg1 == TMSG_SUB_ALARM)
			{
				s_id003_status_wait_next_mode = ID003_MODE_ERROR;
				s_id003_status_wait_error = cline_msg.arg2;
			}
			else
			{
				/* system error ? */
				_cline_system_error(0, 24);
			}
		}
		break;
	#endif
	default:					/* other */
		break;
	}
}

#if defined(UBA_RTQ)

static u8 _id003_rc_enable_check(u8 unit)
{
	u8 res = FALSE;

	switch (unit)
	{
	case 1: /* drum1 */
		if ((ex_rc_enable & 0x01) != 0)
		{
			res = TRUE;
		}
		break;
	case 2: /* drum2 */
		if ((ex_rc_enable & 0x02) != 0)
		{
			res = TRUE;
		}
		break;
	case 3: /* drum3 */
		if ((ex_rc_enable & 0x04) != 0)
		{
			res = TRUE;
		}
		break;
	case 4: /* drum4 */
		if ((ex_rc_enable & 0x08) != 0)
		{
			res = TRUE;
		}
		break;
	default:
		res = FALSE;
		break;
	}

	return (res);
}

/*********************************************************************//**
 * @brief Collect response message receiving [Mode : collect]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_collect_rsp_mode_collect(void)
{
	if (s_id003_status_wait_flag == 0)
	{
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_COLLECTED_WAIT_POLL;
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(cline_msg.arg2 == ALARM_CODE_STACKER_FULL)
			{
				s_id003_collecting_info &= ~(ID003_COLLECTING_INFO_BUSY);

				ex_cline_status_tbl.error_code = cline_msg.arg2;
				write_status_table();
				ex_cline_status_tbl.line_task_mode = ID003_MODE_COLLECTED_WAIT_POLL;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				
			}
			else
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);
				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}			
		}
		else
		{

		}
	}
	else
	{
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_COLLECTED_WAIT_POLL;
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
      		s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{

		}
	}
}

void _id003_collect_rsp_msg_proc()
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_COLLECT:
	case ID003_MODE_RETURN_TO_BOX:
	// case ID003_MODE_COLLECTED_WAIT_POLL:
		_id003_collect_rsp_mode_collect();
		break;
	default:					/* other */
		break;
	}
}

static void _id003_payout_rsp_mode_payout(void) //ok //mode_payout からのメッセージ
{

	if (s_id003_status_wait_flag == 0)
	{
		if (cline_msg.arg1 == TMSG_SUB_PAYSTAY)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYSTAY_WAIT_POLL;
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{

			if(((ex_cline_status_tbl.option & ID003_OPTION_CHANGE_PAYVALID_TIMING) == ID003_OPTION_CHANGE_PAYVALID_TIMING)
			|| ((ex_cline_status_tbl.option & ID003_OPTION_SPRAY_MODE) == ID003_OPTION_SPRAY_MODE)
		//#if defined(UBA_RS)
			|| (is_rc_rs_unit())
		//#endif
			)
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYOUT;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				s_id003_paying_info = (ID003_PAYING_INFO_BUSY|ID003_PAYING_INFO_WAIT_STS);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_PAYOUT_REQ, 0, 0, 0, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_RETRY_REQUEST)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYOUT;

			s_id003_paying_info = (ID003_PAYING_INFO_BUSY|ID003_PAYING_INFO_WAIT_STS);
			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_PAYOUT_REQ, 0, 0, 0, 0);
			_cline_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_BLINK, 0, 1, 1, 1);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			//2025-04-09
			if(ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYOUT_RETURN_NOTE && cline_msg.arg2 == ALARM_CODE_STACKER_FULL)
			{
				s_id003_paying_info &= ~(ID003_PAYING_INFO_BUSY);
				ex_cline_status_tbl.error_code = cline_msg.arg2;

				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYOUT_COLLECTED_WAIT_POLL;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				write_status_table();

				set_recycle_total_count(RC_MODE_COLLECT, OperationDenomi.unit_retry);
			}
			else if (cline_msg.arg3 == TMSG_SUB_PAYVALID_ERROR)//UBA500と異なり 第3引数にしたので
			{
			//現状ここに入る事はほぼないはず。 2025-05-22
			//通常のPayoutはPayValid完了までエラーに遷移しない、Disableモードになってからエラーになるので、ここではない
			//パワーリカバリでのPayValid送信前のエラーも別の箇所で行っている。ID-003のモードが ID003_MODE_POWERUP_INITIAL 
			//mode_payout変更でエラー時にPayvalidが出せなくなるのを防ぐ為、のこしておく
				ex_cline_status_tbl.error_code = (u16)cline_msg.arg2;
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYVALID_ERROR;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);
				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_RETURN_PAYOUT_NOTE)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYOUT_RETURN_NOTE;
			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_RETURN_PAYOUT_COLLECTED)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYOUT_COLLECTED_WAIT_POLL;
			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			set_recycle_total_count(RC_MODE_COLLECT, OperationDenomi.unit_retry);			
		}
	}
	else
	{
		if (cline_msg.arg1 == TMSG_SUB_PAYSTAY)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYSTAY_WAIT_POLL;
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if(((ex_cline_status_tbl.option & ID003_OPTION_CHANGE_PAYVALID_TIMING) == ID003_OPTION_CHANGE_PAYVALID_TIMING)
			|| ((ex_cline_status_tbl.option & ID003_OPTION_SPRAY_MODE) == ID003_OPTION_SPRAY_MODE)
		//#if defined(UBA_RS)
			|| (is_rc_rs_unit())
		//#endif
			)
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYOUT;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				s_id003_paying_info = (ID003_PAYING_INFO_BUSY|ID003_PAYING_INFO_WAIT_STS);
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_PAYOUT_REQ, 0, 0, 0, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			//2025-04-09
			if(ex_cline_status_tbl.line_task_mode == ID003_MODE_PAYOUT_RETURN_NOTE && cline_msg.arg2 == ALARM_CODE_STACKER_FULL)
			{
				s_id003_paying_info &= ~(ID003_PAYING_INFO_BUSY);
				ex_cline_status_tbl.error_code = cline_msg.arg2;

				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYOUT_COLLECTED_WAIT_POLL;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				write_status_table();

				set_recycle_total_count(RC_MODE_COLLECT, OperationDenomi.unit_retry);
			}
			else if (cline_msg.arg3 == TMSG_SUB_PAYVALID_ERROR)//UBA500と異なり 第3引数にしたので
			{
			//現状ここに入る事はほぼないはず。 2025-05-22
			//通常のPayoutはPayValid完了までエラーに遷移しない、Disableモードになってからエラーになるので、ここではない
			//パワーリカバリでのPayValid送信前のエラーも別の箇所で行っている。ID-003のモードが ID003_MODE_POWERUP_INITIAL 
			//mode_payout変更でエラー時にPayvalidが出せなくなるのを防ぐ為、のこしておく
				ex_cline_status_tbl.error_code = (u16)cline_msg.arg2;
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYVALID_ERROR;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);
				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
	}
}

static void _id003_payout_rsp_mode_payvalid(void) //ok //mode_payout からのメッセージ from main TMSG_CLINE_PAYOUT_REQ
{	
	if (s_id003_status_wait_flag == 0)
	{
        if( cline_msg.arg1 == TMSG_SUB_PAYSTAY )
        {
            ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYSTAY_WAIT_POLL;
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
        }
		else if(cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_paying_info &= ~(ID003_PAYING_INFO_BUSY);

			if(OperationDenomi.remain == 0)
			{
				 ex_pre_feed_after_jam = 0;

				if( ex_cline_status_tbl.error_code != ALARM_CODE_OK )
				{
				    _set_id003_alarm(ID003_MODE_ERROR, ex_cline_status_tbl.error_code);
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
				else if (_is_id003_enable())
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
					// TODO: BEZEL ON
					//UBA500はこのに処理がなくて、Poll受信でEnableにしている。
					//ただ本来は、ここでmainから、Enable,Disable,payout継続か質問の意味のsuccessなので
					//ここでメッセージを返すのがいい
					//ここは、払い出しがすべて完了した後のEnable,Disableなのでポールをまたずに、mainにメッセージを返していいはず
					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0); //UBA500はなぜかない 2025-08-02
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
					// TODO : BEZEL OFF
				    _cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
				}
			}
			else
			{
				s_id003_paying_info = (ID003_PAYING_INFO_BUSY|ID003_PAYING_INFO_WAIT_STS);
				//2025-02-05
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYOUT;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_PAYOUT_REQ, 0, 0, 0, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);
			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
	}
	else
	{
	/* Status WAIT */		
		if( cline_msg.arg1 == TMSG_SUB_PAYSTAY )
		{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYSTAY_WAIT_POLL;
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_paying_info &= ~(ID003_PAYING_INFO_BUSY);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);
			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
	}
}

static void _id003_payout_rsp_mode_paystay(void) //mode_payout からのメッセージ
{
	if (s_id003_status_wait_flag == 0)
	{
		if (cline_msg.arg1 == TMSG_SUB_PAYVALID)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYVALID;
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(cline_msg.arg3 == TMSG_SUB_PAYVALID_ERROR)//UBA500と異なり 第3引数にしたので
			{
			//現状ここに入る事はほぼないはず。 2025-05-22
			//通常のPayoutはPayValid完了までエラーに遷移しない、Disableモードになってからエラーになるので、ここではない
			//パワーリカバリでのPayValid送信前のエラーも別の箇所で行っている。ID-003のモードが ID003_MODE_POWERUP_INITIAL 
			//mode_payout変更でエラー時にPayvalidが出せなくなるのを防ぐ為、のこしておく
				ex_cline_status_tbl.error_code = (u16)cline_msg.arg2;
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYVALID_ERROR;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);
				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
	}
	else
	{
		if (cline_msg.arg1 == TMSG_SUB_PAYVALID)
		{
			s_id003_status_wait_next_mode = ID003_MODE_PAYVALID;
			// ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYVALID;
			// _id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if(cline_msg.arg3 == TMSG_SUB_PAYVALID_ERROR)//UBA500と異なり
			{
				ex_cline_status_tbl.error_code = (u16)cline_msg.arg2;
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYVALID_ERROR;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);
				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
	}
}

#if 0 //2025-02-05 上に統一できるので廃止
static void _id003_payout_rsp_mode_after_payvalid_ack_payout(void)
{

}
#endif


void _id003_payout_rsp_msg_proc(void) //mode_payout からのメッセージ
{
	switch(ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_PAYOUT:
	case ID003_MODE_PAYOUT_RETURN_NOTE:
	case ID003_MODE_PAYOUT_COLLECTED_WAIT_POLL:
		_id003_payout_rsp_mode_payout();
		break;
	case ID003_MODE_PAYSTAY:
		_id003_payout_rsp_mode_paystay();
		break;
	case ID003_MODE_PAYVALID:
	case ID003_MODE_PAYVALID_ERROR:
	case ID003_MODE_WAIT_PAYVALID_ACK:
	case ID003_MODE_AFTER_PAYVALID_ACK_ENABLE: //すべての払い出し完了後の状態,mode_payoutをまだ抜けていないのでのEnableとは分けている
	case ID003_MODE_AFTER_PAYVALID_ACK_DISABLE: //すべての払い出し完了後の状態,mode_payoutをまだ抜けていないのでのDisableとは分けている
	case ID003_MODE_AFTER_PAYVALID_ACK_PAYOUT://払い出し継続
		_id003_payout_rsp_mode_payvalid();
		break;
	#if 0 //2025-02-05 上に統一できるので廃止
	case ID003_MODE_AFTER_PAYVALID_ACK_PAYOUT:
		_id003_payout_rsp_mode_after_payvalid_ack_payout();
		break;
	#endif
	default:
		break;
	}
}

#if defined(UBA_RTQ)	//#if defined(UBA_ID003_ENC)
/*  */
void _id003_enc_mode_req_proc(void) //ok
{
	_id003_tx_buff.length   = 2 + 4;
    _id003_tx_buff.cmd      = ID003_CMD_GET_ENC_MODE;
    _id003_tx_buff.data[0]  = id003_enc_mode;

    uart_send_id003(&_id003_tx_buff);
}

/* */
void _id003_enc_mode_cmd_proc(void) //ok 0xC8
{
	if((_id003_rx_buff.data[0] == PAYOUT16MODE) && (id003_enc_mode != PAYOUT16MODE_SPECK))
	{
		id003_enc_mode = PAYOUT16MODE;
		_id003_send_host_echoback();		//See Spec ID03-320
	}
	#if 1//#if defined(ID003_SPECK64)//SPECK専用コマンド
	else if(_id003_rx_buff.data[0] == PAYOUT16MODE_SPECK)
	{	
		if(ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP
		&& ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_AT
		&& ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_SK
		&& ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_ERROR)
		{
			_id003_send_host_invalid();
		}
		else
		{
			//SPECKモードになるコマンドは2つ
			//Secret numbertコマンドの拡張版 or _id003_enc_mode_cmd_proc
			id003_enc_mode = PAYOUT16MODE_SPECK;
			_id003_send_host_echoback();	//See Spec ID03-320	
		}			
	}
	#endif
	else /* */
	{
		_id003_send_host_invalid();
	}
}
#endif // UBA_ID003_ENC

#endif // UBA_RTQ


/*********************************************************************//**
 * @brief Enable response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_enable_rsp_msg_proc(void)
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_ENABLE:
		if (s_id003_status_wait_flag == 0)
		{
		/* Normal (Not Status WAIT) */
			if (cline_msg.arg1 == TMSG_SUB_ACCEPT)
			{
				ex_cline_status_tbl.reject_code = 0x0000;

				ex_cline_status_tbl.line_task_mode = ID003_MODE_ACCEPT;
#if defined(UBA_RTQ)
				//#if defined(UBA_RS)
				ex_rs_payout_remain_flag = RS_NOTE_REMAIN_NONE;
				//#endif
				/* set recovery */
				set_recovery_step(RECOVERY_STEP_ACCEPT);
#endif // uBA_RTQ
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ACCEPT_REQ, 0, 0, 0, 0);
				_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 0, 0, 0);
				_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD1, 0, 0, 0);
				_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD2, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else if (cline_msg.arg1 == TMSG_SUB_REJECT)
			{
				_set_id003_reject(ID003_MODE_ENABLE_REJECT, (u16)cline_msg.arg2);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			}
			else if (cline_msg.arg1 == TMSG_SUB_ALARM)
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				/* system error ? */
				_cline_system_error(0, 25);
			}
		}
		else
		{
		/* Status WAIT */
			if (cline_msg.arg1 == TMSG_SUB_ACCEPT)
			{
				#if defined(UBA_RTQ) //)#if defined(UBA_RS)
				ex_rs_payout_remain_flag = RS_NOTE_REMAIN_NONE;
				#endif
				s_id003_status_wait_next_mode = ID003_MODE_ACCEPT;
			}
			else if (cline_msg.arg1 == TMSG_SUB_REJECT)
			{
				_set_id003_reject(ID003_MODE_ENABLE_REJECT, (u16)cline_msg.arg2);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			}
			else if (cline_msg.arg1 == TMSG_SUB_ALARM)
			{
				s_id003_status_wait_next_mode = ID003_MODE_ERROR;
				s_id003_status_wait_error = cline_msg.arg2;
			}
			else
			{
				/* system error ? */
				_cline_system_error(0, 26);
			}
		}
		break;
#if defined(UBA_RTQ)
	case ID003_MODE_RETURN_TO_BOX:
		set_recovery_step(RECOVERY_STEP_ACCEPT);
		ex_cline_status_tbl.reject_code = 0x0000;
		// ex_cline_status_tbl.line_task_mode = ID003_MODE_ACCEPT;

		/* bookmarkモードか */
		// if (ex_cline_status_tbl.bookmark_mode == 1)
		// {
		// 	_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ACCEPT_REQ, TMSG_SUB_BOOKMARK, 0, 0, 0);
		// }
		// else
		{
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ACCEPT_REQ, 0, 0, 0, 0);
		}
		/* change status */
		// _id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		break;
#endif // UBA_RTQ
	#if 0
	case ID003_MODE_ENABLE_BOOKMARK:
		if (s_id003_status_wait_flag == 0)
		{
		/* Normal (Not Status WAIT) */
			if (cline_msg.arg1 == TMSG_SUB_ACCEPT)
			{
				ex_cline_status_tbl.reject_code = 0x0000;

				ex_cline_status_tbl.line_task_mode = ID003_MODE_ACCEPT_BOOKMARK;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ACCEPT_REQ, TMSG_SUB_BOOKMARK, 0, 0, 0);
				_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 0, 0, 0);
				_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD1, 0, 0, 0);
				_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD2, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else if (cline_msg.arg1 == TMSG_SUB_REJECT)
			{
				_set_id003_reject(ID003_MODE_ENABLE_REJECT_BOOKMARK, (u16)cline_msg.arg2);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			}
			else if (cline_msg.arg1 == TMSG_SUB_ALARM)
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				/* system error ? */
				_cline_system_error(0, 25);
			}
		}
		else
		{
		/* Status WAIT */
			if (cline_msg.arg1 == TMSG_SUB_ACCEPT)
			{
				s_id003_status_wait_next_mode = ID003_MODE_ACCEPT;
			}
			else if (cline_msg.arg1 == TMSG_SUB_REJECT)
			{
				_set_id003_reject(ID003_MODE_ENABLE_REJECT_BOOKMARK, (u16)cline_msg.arg2);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			}
			else if (cline_msg.arg1 == TMSG_SUB_ALARM)
			{
				s_id003_status_wait_next_mode = ID003_MODE_ERROR;
				s_id003_status_wait_error = cline_msg.arg2;
			}
			else
			{
				/* system error ? */
				_cline_system_error(0, 26);
			}
		}
		break;
	#endif
	default:					/* other */
		break;
	}
}


/*********************************************************************//**
 * @brief Accept response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_accept_rsp_msg_proc(void)
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_ACCEPT:
		_id003_accept_rsp_mode_accept();
		break;
	#if 0
	case ID003_MODE_ACCEPT_BOOKMARK:
		_id003_accept_rsp_mode_accept_bookmark();
		break;
	#endif
	case ID003_MODE_REJECT_WAIT_ACCEPT_RSP:
		_id003_accept_rsp_mode_rej_wait_accrsp();
		break;

	//#if (ID003_UBA==1)
	//Vendに対するAck受信後しか、main側は識別処理へ遷移しない
	/* 搬送失敗メッセージ、エラーメッセージ */
	case ID003_MODE_STACK:
	case ID003_MODE_VEND:
	case ID003_MODE_WAIT_VEND_ACK:

	/* 搬送失敗メッセージ、エラーメッセージ */
	/* 識別成功、失敗のメッセージ*/
	case ID003_MODE_STACKED:
	case ID003_MODE_ENABLE_WAIT_POLL:
	case ID003_MODE_ENABLE:

	case ID003_MODE_DISABLE:
	case ID003_MODE_DISABLE_REJECT_2ND:
		_id003_accept_evt_success_2nd();
		break;
	#if defined(UBA_RTQ)
	case ID003_MODE_RETURN_TO_BOX:
		_id003_accept_rsp_mode_return_to_box();
		break;
	#endif // UBA_RTQ

	//_id003_status_info_msg_proc からこっちに移した
	case ID003_MODE_ESCROW:
	case ID003_MODE_ESCROW_WAIT_CMD:
	case ID003_MODE_HOLD1:
	case ID003_MODE_HOLD2:
		_id003_status_info_mode_escrow(); //エラー + Reject
		break;


	default:					/* other */
		break;
	}
}

#if defined(UBA_RTQ)
void _id003_accept_rsp_mode_return_to_box(void) //ID003_MODE_RETURN_TO_BOX
{
	if (s_id003_status_wait_flag == 0)
	{
		/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, VEND_POSITION_2, 1, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ACCEPTING)
		{
			/* trigger bezel off */
    		//_line_send_msg(ID_DISPLAY_MBX, TMSG_DISP_BEZEL_OFF, 0, 0, 0, 0);			
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			/* set reject *///2025-02-06
    		_set_id003_reject(ID003_MODE_RETURN_ERROR, (u16)cline_msg.arg2);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);			
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{

		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 27);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{

		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{

		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{

		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 28);
		}
	}
}
#endif // UBA_RTQ


/*********************************************************************//**
 * @brief Accept response message receiving [Mode : accept]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_accept_rsp_mode_accept(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_2nd_note = 0;
			if (cline_msg.arg2 == BAR_INDX)
			{
				if (_is_id003_bar_inhibit())
				{
				//2024-03-05
					if ((ex_cline_status_tbl.comm_mode == 0x0000)/*(ex_cline_status_tbl.comm_mode != 0x0002)*/ //2020-05-27 MM
					&& (ex_cline_status_tbl.line_task_mode == ID003_MODE_ACCEPT))
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ACCEPT_WAIT_POLL_FOR_REJECT;
						ex_cline_status_tbl.reject_code = (u16)REJECT_CODE_INHIBIT;
						write_status_table();
					}
					else
					{
						_set_id003_reject(ID003_MODE_REJECT, REJECT_CODE_INHIBIT);
						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
						/* change status */
						_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
					}
				}
				else
				{
				//2024-03-05
					if ((ex_cline_status_tbl.comm_mode == 0x0000)
					&& (ex_cline_status_tbl.line_task_mode == ID003_MODE_ACCEPT))
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ACCEPT_WAIT_POLL_FOR_ESCROW;
						ex_cline_status_tbl.escrow_code = cline_msg.arg2;
						write_status_table();
						_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 300, 0, 0);

					}
					else
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ESCROW;
						ex_cline_status_tbl.escrow_code = cline_msg.arg2;
						write_status_table();
						_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 300, 0, 0);

						/* change status */
						_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
					}
				}
			}
			else
			{
				if (_is_id003_denomi_inhibit(cline_msg.arg2, cline_msg.arg3))
				{
					//2024-03-05
					if ((ex_cline_status_tbl.comm_mode == 0x0000)/*(ex_cline_status_tbl.comm_mode != 0x0002)*/ //2020-05-27 MM
					&& (ex_cline_status_tbl.line_task_mode == ID003_MODE_ACCEPT))
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ACCEPT_WAIT_POLL_FOR_REJECT;
						ex_cline_status_tbl.reject_code = (u16)REJECT_CODE_INHIBIT;
						write_status_table();
					}
					else
					{
						_set_id003_reject(ID003_MODE_REJECT, REJECT_CODE_INHIBIT);

						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

						/* change status */
						_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
					}
				}
				else
				{
					//2024-03-05
					if ((ex_cline_status_tbl.comm_mode == 0x0000)
					&& (ex_cline_status_tbl.line_task_mode == ID003_MODE_ACCEPT))
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ACCEPT_WAIT_POLL_FOR_ESCROW;
						ex_cline_status_tbl.escrow_code = cline_msg.arg2;
						write_status_table();
						_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 300, 0, 0);
					}
					else
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ESCROW;
						ex_cline_status_tbl.escrow_code = cline_msg.arg2;
						write_status_table();
						_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 300, 0, 0);

						/* change status */
						_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
					}
				}
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			if ((ex_cline_status_tbl.comm_mode == 0x0000)/*(ex_cline_status_tbl.comm_mode != 0x0002)*/ //2020-05-27 MM
			&& (ex_cline_status_tbl.line_task_mode == ID003_MODE_ACCEPT))
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_ACCEPT_WAIT_POLL_FOR_REJECT;
				ex_cline_status_tbl.reject_code = (u16)cline_msg.arg2;
				write_status_table();
			}
			else
			{
				_set_id003_reject(ID003_MODE_REJECT, (u16)cline_msg.arg2);

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 27);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ESCROW;
			s_id003_status_wait_escrow = cline_msg.arg2;
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			s_id003_status_wait_next_mode = ID003_MODE_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 28);
		}
	}
}

/*********************************************************************//**
 * @brief Accept response message receiving [Mode : accept bookmark]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
void _id003_accept_rsp_mode_accept_bookmark(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK_BOOKMARK;
			write_status_table();

			set_recovery_step(RECOVERY_STEP_ESCORW);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, VEND_POSITION_1, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 63);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK_BOOKMARK;
			write_status_table();

			set_recovery_step(RECOVERY_STEP_ESCORW);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, VEND_POSITION_1, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			s_id003_status_wait_next_mode = ID003_MODE_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 64);
		}
	}
}
#endif
/*********************************************************************//**
 * @brief Accept response message receiving [Mode : reject wait accept response]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_accept_rsp_mode_rej_wait_accrsp(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_set_id003_reject(ID003_MODE_REJECT, REJECT_CODE_INHIBIT);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 29);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_set_id003_reject(ID003_MODE_REJECT, REJECT_CODE_INHIBIT);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 30);
		}
	}
}


/*********************************************************************//**
 * @brief Stack response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stack_rsp_msg_proc(void)
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_STACK:
	case ID003_MODE_WAIT_PAUSE:
	case ID003_MODE_PAUSE:
		_id003_stack_rsp_mode_stack();
		break;
#if 0
	case ID003_MODE_STACK_BOOKMARK:
	case ID003_MODE_PAUSE_BOOKMARK:
		_id003_stack_rsp_mode_stack_bookmark();
		break;
#endif

	case ID003_MODE_VEND:
	case ID003_MODE_WAIT_VEND_ACK:
		_id003_stack_rsp_mode_vend();
		break;

	case ID003_MODE_STACKED:
		_id003_stack_rsp_mode_stacked();
		break;

	/* _id003_status_info_msg_proc から移動 */
	case ID003_MODE_VEND_FULL:
	case ID003_MODE_WAIT_VEND_ACK_FULL:
	case ID003_MODE_STACKED_FULL:
		_id003_status_info_mode_vend(); //エラーのみ
		break;

		default:					/* other */
		break;
	}
}


/*********************************************************************//**
 * @brief Stack response message receiving [Mode : PowerUp Initialize]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stack_rsp_mode_power_init(void) //UBA500と異なり _id003_power_mode_success _id003_reset_rsp_mode_power_init も併用
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		//if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		if (cline_msg.arg1 == TMSG_SUB_END_INIT_STACK)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
			#if defined(UBA_RTQ)
				if( ((ex_cline_status_tbl.option & ID003_OPTION_RECOVERY) == ID003_OPTION_RECOVERY)
				 && ((s_id003_powerup_stat == POWERUP_STAT_RECOVER_FEED_STACK)
				  || (s_id003_powerup_stat == POWERUP_STAT_RECOVER_STACK)
				 ))
				{
					/* 収納は完了しているのでリカバリフラグはVEND状態にする。 */
					/* そうしないと、この後のイニシャル動作中に電源OFFした場合に */
					/* リカバリでチートになる */
					set_recovery_step( RECOVERY_STEP_VEND );
				}
				else if( s_id003_powerup_stat == POWERUP_STAT_FEED_STACK )
				{					
					set_recycle_total_count(RC_MODE_COLLECT, ex_recovery_info.unit_count);

					set_recovery_step(RECOVERY_STEP_NON);
					s_id003_powerup_stat = 0;
				}
			#endif		
				ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_VEND)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_PAUSE;
			write_status_table();
		}
		else if (cline_msg.arg1 == TMSG_SUB_RESUME)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_STACK;
			write_status_table();
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
		#if defined(UBA_RTQ)	//2025-05-22
			if (s_id003_powerup_stat == POWERUP_STAT_RECOVER_PAYOUT)
			{
			//パワーリカバリでPayValid送信前のエラー用、PayValid優先
				ex_cline_status_tbl.error_code = (u16)cline_msg.arg2;
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYVALID_ERROR;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);
				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		#else		
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);
			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		#endif
		}
		//2025-02-12 start
		//if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REMOVE)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		//else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_HANGING)
		{
			/*  */
		}
		//else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			_set_id003_reject(ID003_MODE_POWERUP_INITIAL_REJECT, (u16)cline_msg.arg2);
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		//2025-02-12 end
		else
		{
			/* system error ? */
			_cline_system_error(0, 31);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_END_INIT_STACK)
		{
			s_id003_status_wait_next_mode = ID003_MODE_POWERUP_INITIAL;
		}
		else if (cline_msg.arg1 == TMSG_SUB_VEND)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_RESUME)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
		#if defined(UBA_RTQ)	//2025-05-22
			if (s_id003_powerup_stat == POWERUP_STAT_RECOVER_PAYOUT)
			{
			//パワーリカバリでPayValid送信前のエラー用、PayValid優先
				ex_cline_status_tbl.error_code = (u16)cline_msg.arg2;
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYVALID_ERROR;
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				s_id003_status_wait_next_mode = ID003_MODE_ERROR;
				s_id003_status_wait_error = cline_msg.arg2;
			}
		#else		
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		#endif
		}
		//2025-02-12
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REMOVE)
		{
			s_id003_status_wait_next_mode = ID003_MODE_POWERUP_INITIAL;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_HANGING)
		{
			s_id003_status_wait_next_mode = ID003_MODE_POWERUP_INITIAL_PAUSE;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			s_id003_status_wait_next_mode = ID003_MODE_POWERUP_INITIAL_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 32);
		}
	}
}


/*********************************************************************//**
 * @brief Stack response message receiving [Mode : Initialize]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stack_rsp_mode_init(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		//if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		if (cline_msg.arg1 == TMSG_SUB_END_INIT_STACK)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_VEND)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
		{
			/*  */
			ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL_PAUSE;
			write_status_table();
		}
		else if (cline_msg.arg1 == TMSG_SUB_RESUME)
		{
			/*  */
			ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL_STACK;
			write_status_table();
		}
		//else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			_set_id003_reject(ID003_MODE_INITIAL_REJECT, (u16)cline_msg.arg2);
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		//2025-02-12 start
		//if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REMOVE)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL;
				write_status_table();

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		//else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_HANGING)
		{
			/*  */
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 33);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_END_INIT_STACK)
		{
			s_id003_status_wait_next_mode = ID003_MODE_INITIAL;
		}
		else if (cline_msg.arg1 == TMSG_SUB_VEND)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_RESUME)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		//2025-02-12 start
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REMOVE)
		{
			s_id003_status_wait_next_mode = ID003_MODE_INITIAL;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_HANGING)
		{
			s_id003_status_wait_next_mode = ID003_MODE_INITIAL_PAUSE;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			s_id003_status_wait_next_mode = ID003_MODE_INITIAL_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 34);
		}
	}
}


/*********************************************************************//**
 * @brief Stack response message receiving [Mode : Auto Recovery]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stack_rsp_mode_recovery(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		//if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		if (cline_msg.arg1 == TMSG_SUB_END_INIT_STACK)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY;
				write_status_table();

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_VEND)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
		{
			/*  */
			ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY_PAUSE;
			write_status_table();
		}
		else if (cline_msg.arg1 == TMSG_SUB_RESUME)
		{
			/*  */
			ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY_STACK;
			write_status_table();
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		//2025-02-12 start
		//if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REMOVE)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY;
				write_status_table();

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		//else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_HANGING)
		{
			/*  */
		}
		//else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			_set_id003_reject(ID003_MODE_AUTO_RECOVERY_REJECT, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 35);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_status_wait_next_mode = ID003_MODE_AUTO_RECOVERY;
		}
		else if (cline_msg.arg1 == TMSG_SUB_VEND)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_RESUME)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		//2025-02-12 start
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REMOVE)
		{
			s_id003_status_wait_next_mode = ID003_MODE_AUTO_RECOVERY;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_HANGING)
		{
			s_id003_status_wait_next_mode = ID003_MODE_AUTO_RECOVERY_PAUSE;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			s_id003_status_wait_next_mode = ID003_MODE_AUTO_RECOVERY_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 36);
		}
	}
}


/*********************************************************************//**
 * @brief Stack response message receiving [Mode : stack]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stack_rsp_mode_stack(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_VEND)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_VEND;
			write_status_table();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
		{
			if (ex_cline_status_tbl.line_task_mode == ID003_MODE_STACK)
			{
				if (!(ex_cline_status_tbl.comm_mode == 0x0002))
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_WAIT_PAUSE;
					write_status_table();
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_PAUSE;
					write_status_table();

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_RESUME)
		{
			if (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE)
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (cline_msg.arg2 == ALARM_CODE_STACKER_FULL)
			{
				/* VEND出力後エラー */
				ex_cline_status_tbl.line_task_mode = ID003_MODE_VEND_FULL;
				ex_cline_status_tbl.error_code = cline_msg.arg2;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
#if defined(UBA_RTQ)
		else if (cline_msg.arg1 == TMSG_SUB_COLLECT)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_COLLECT;
			//#if defined(UBA_RS)
			ex_rs_payout_remain_flag = RS_NOTE_REMAIN_NONE;
			//#endif 
			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
#endif // UBA_RTQ
		//#if (ID003_UBA==1) //mainのmode_stack関係からのメッセージ
		else if (cline_msg.arg1 == TMSG_SUB_STACKING_ENTRY_ON)
		{
		    s_id003_2nd_note = 1; /* 2枚目の取り込み開始 */
			s_id003_2nd_note_code = 0;
			ex_cline_status_tbl.reject_code = 0x0000;
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ACCEPT_REQ, 0, 0, 0, 0); /* 2枚目の取り込み許可*/
		}
		//#endif
		else
		{
			/* system error ? */
			_cline_system_error(0, 37);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_VEND)
		{
			s_id003_status_wait_next_mode = ID003_MODE_VEND;
		}
		else if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_stacking_info &= ~(ID003_STACKING_INFO_BUSY);
		}
		else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
		{
			if (ex_cline_status_tbl.line_task_mode == ID003_MODE_STACK)
			{
				s_id003_status_wait_next_mode = ID003_MODE_PAUSE;

				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAUSE;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_RESUME)
		{
			if (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE)
			{
				s_id003_status_wait_next_mode = ID003_MODE_STACK;

				ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			s_id003_status_wait_next_mode = ID003_MODE_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;

			if (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE)
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (s_id003_status_wait_next_mode == ID003_MODE_VEND)
			{
				s_id003_stacking_info &= ~(ID003_STACKING_INFO_BUSY);
				ex_cline_status_tbl.error_code = cline_msg.arg2;
				write_status_table();
			}
			else
			{
				if (cline_msg.arg2 == ALARM_CODE_STACKER_FULL)
				{
					/* VEND出力後エラー */
					s_id003_status_wait_next_mode = ID003_MODE_VEND_FULL;
					s_id003_stacking_info &= ~(ID003_STACKING_INFO_BUSY);
					ex_cline_status_tbl.error_code = cline_msg.arg2;
					write_status_table();
				}
				else
				{
					s_id003_status_wait_next_mode = ID003_MODE_ERROR;
					s_id003_status_wait_error = cline_msg.arg2;

					if (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE)
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK;
						write_status_table();

						/* change status */
						_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
					}
				}
			}
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 38);
		}
	}
}


/*********************************************************************//**
 * @brief Stack response message receiving [Mode : stack bookmark]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
void _id003_stack_rsp_mode_stack_bookmark(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_VEND)
		{
		}
		else if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
				/* [Interrupt Mode 2] don't wait status request in STACKED status */
					if (_is_id003_enable())
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
						write_status_table();
						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

						/* change status */
						_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
					}
					else
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
						write_status_table();
						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

						/* change status */
						_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
					}
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_STACKED_BOOKMARK;
					write_status_table();

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
		{
			if (ex_cline_status_tbl.line_task_mode == ID003_MODE_STACK_BOOKMARK)
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAUSE_BOOKMARK;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_RESUME)
		{
			if (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE_BOOKMARK)
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK_BOOKMARK;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (cline_msg.arg2 == ALARM_CODE_STACKER_FULL)
			{
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
				/* [Interrupt Mode 2] don't wait status request in STACKED status */
					_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
				else
				{
					ex_cline_status_tbl.error_code = cline_msg.arg2;
					write_status_table();
				}
			}
			else
			{
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 64);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_VEND)
		{
		}
		else if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if (ex_cline_status_tbl.comm_mode == 0x0002)
			{
				s_id003_status_wait_next_mode = ID003_MODE_DISABLE;
			}
			else
			{
				s_id003_status_wait_next_mode = ID003_MODE_STACKED_BOOKMARK;
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_PAUSE)
		{
			if (ex_cline_status_tbl.line_task_mode == ID003_MODE_STACK_BOOKMARK)
			{
				s_id003_status_wait_next_mode = ID003_MODE_PAUSE_BOOKMARK;

				ex_cline_status_tbl.line_task_mode = ID003_MODE_PAUSE_BOOKMARK;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_RESUME)
		{
			if (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE_BOOKMARK)
			{
				s_id003_status_wait_next_mode = ID003_MODE_STACK_BOOKMARK;

				ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK_BOOKMARK;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			s_id003_status_wait_next_mode = ID003_MODE_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;

			if (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE_BOOKMARK)
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK_BOOKMARK;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (cline_msg.arg2 == ALARM_CODE_STACKER_FULL)
			{
				ex_cline_status_tbl.error_code = cline_msg.arg2;
				write_status_table();
			}
			else
			{
				s_id003_status_wait_next_mode = ID003_MODE_ERROR;
				s_id003_status_wait_error = cline_msg.arg2;
			}
			if (ex_cline_status_tbl.line_task_mode == ID003_MODE_PAUSE_BOOKMARK)
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_STACK_BOOKMARK;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 65);
		}
	}
}
#endif

/*********************************************************************//**
 * @brief Stack response message receiving [Mode : vend relate]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stack_rsp_mode_vend(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
				s_id003_stacking_info &= ~(ID003_STACKING_INFO_BUSY);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_stacking_info &= ~(ID003_STACKING_INFO_BUSY);
			if( cline_msg.arg2 == ALARM_CODE_BOX)
			{
				if (ex_cline_status_tbl.error_code != cline_msg.arg2)
				{
					_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
			}
			else
			{
				if (ex_cline_status_tbl.error_code != cline_msg.arg2)
				{
					ex_cline_status_tbl.error_code = cline_msg.arg2;
					write_status_table();
				}
			}
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 39);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_stacking_info &= ~(ID003_STACKING_INFO_BUSY);
			if (ex_cline_status_tbl.comm_mode == 0x0002)
			{
				s_id003_status_wait_next_mode = ID003_MODE_DISABLE;
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_stacking_info &= ~(ID003_STACKING_INFO_BUSY);
			if( cline_msg.arg2 == ALARM_CODE_BOX)
			{
				if (ex_cline_status_tbl.error_code != cline_msg.arg2)
				{
					s_id003_status_wait_next_mode = ID003_MODE_ERROR;
					s_id003_status_wait_error = cline_msg.arg2;
				}
			}
			else
			{
				s_id003_status_wait_next_mode = ID003_MODE_ERROR;
				s_id003_status_wait_error = cline_msg.arg2;
			}
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 40);
		}
	}
}


/*********************************************************************//**
 * @brief Stack response message receiving [Mode : vend relate]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_stack_rsp_mode_stacked(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
				s_id003_stacking_info &= ~(ID003_STACKING_INFO_BUSY);
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
				/* [Interrupt Mode 2] don't wait status request in STACKED status */
					if (_is_id003_enable())
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
						write_status_table();
						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

						/* change status */
						_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
					}
					else
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
						write_status_table();
						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

						/* change status */
						_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
					}
				}
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_stacking_info &= ~(ID003_STACKING_INFO_BUSY);
			if (ex_cline_status_tbl.comm_mode == 0x0002)
			{
			/* [Interrupt Mode 2] don't wait status request in STACKED status */
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				ex_cline_status_tbl.error_code = cline_msg.arg2;
				write_status_table();
			}
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 41);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_stacking_info &= ~(ID003_STACKING_INFO_BUSY);
			if (ex_cline_status_tbl.comm_mode == 0x0002)
			{
				s_id003_status_wait_next_mode = ID003_MODE_DISABLE;
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_stacking_info &= ~(ID003_STACKING_INFO_BUSY);
			ex_cline_status_tbl.error_code = cline_msg.arg2;
			write_status_table();
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 42);
		}
	}
}


/*********************************************************************//**
 * @brief Reject response message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_reject_rsp_msg_proc(void)
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_REJECT:
		if (ex_cline_status_tbl.comm_mode == 0x0002)
		{
			_id003_reject_rsp_mode_wait_note_removed();
		}
		else
		{
			_id003_reject_rsp_mode_reject();
		}
		break;
	case ID003_MODE_REJECT_WAIT_NOTE_REMOVED:
	case ID003_MODE_RETURN:
		_id003_reject_rsp_mode_wait_note_removed();
		break;
	case ID003_MODE_DISABLE_REJECT:
	case ID003_MODE_ENABLE_REJECT:
//	case ID003_MODE_DISABLE_REJECT_BOOKMARK:
//	case ID003_MODE_ENABLE_REJECT_BOOKMARK:
	case ID003_MODE_DISABLE_REJECT_2ND:
		_id003_reject_rsp_mode_disable_rej();
		break;

	//#if (ID003_UBA==1) //2023-09-13
	case ID003_MODE_STACK:
	case ID003_MODE_VEND:
	case ID003_MODE_WAIT_VEND_ACK:
	/* 搬送失敗メッセージ、エラーメッセージ */
	/* 識別成功、失敗のメッセージ*/
	case ID003_MODE_STACKED:
	case ID003_MODE_ENABLE_WAIT_POLL:
	case ID003_MODE_ENABLE:
		if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		break;
	//#endif
#if defined(UBA_RTQ)	//2025-02-06
	case ID003_MODE_RETURN_ERROR:
		_id003_return_err_mode();
		break;
#endif

	default:					/* other */
		break;
	}
}


/*********************************************************************//**
 * @brief Reject response message receiving [Mode : PowerUp Initialize Reject]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_reject_rsp_mode_power_init_rej(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		//if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REMOVE)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
			}
		}
		//else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_HANGING)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_PAUSE;
			write_status_table();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		//else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			_set_id003_reject(ID003_MODE_POWERUP_INITIAL_REJECT, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 47);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REMOVE)
		{
			s_id003_status_wait_next_mode = ID003_MODE_POWERUP_INITIAL;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_HANGING)
		{
			s_id003_status_wait_next_mode = ID003_MODE_POWERUP_INITIAL_PAUSE;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			s_id003_status_wait_next_mode = ID003_MODE_POWERUP_INITIAL_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 48);
		}
	}
}


/*********************************************************************//**
 * @brief Reject response message receiving [Mode : Initialize Reject]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_reject_rsp_mode_init_rej(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		//if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REMOVE)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
			}
		}
		//else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_HANGING)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL_PAUSE;
			write_status_table();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		//else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			_set_id003_reject(ID003_MODE_INITIAL_REJECT, (u16)cline_msg.arg2);
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 51);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REMOVE)
		{
			s_id003_status_wait_next_mode = ID003_MODE_INITIAL;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_HANGING)
		{
			s_id003_status_wait_next_mode = ID003_MODE_INITIAL_PAUSE;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			s_id003_status_wait_next_mode = ID003_MODE_INITIAL_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 52);
		}
	}
}


/*********************************************************************//**
 * @brief Reject response message receiving [Mode : Auto Recovery Reject]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_reject_rsp_mode_recovery_rej(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		//if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REMOVE)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY;
				write_status_table();

				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
			}
		}
		//else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_HANGING)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY_PAUSE;
			write_status_table();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		//else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			_set_id003_reject(ID003_MODE_AUTO_RECOVERY_REJECT, (u16)cline_msg.arg2);
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 55);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REMOVE)
		{
			s_id003_status_wait_next_mode = ID003_MODE_AUTO_RECOVERY;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_HANGING)
		{
			s_id003_status_wait_next_mode = ID003_MODE_AUTO_RECOVERY_PAUSE;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INIT_REJECT_REQUEST)
		{
			s_id003_status_wait_next_mode = ID003_MODE_AUTO_RECOVERY_REJECT;
			s_id003_status_wait_reject = cline_msg.arg2;
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 56);
		}
	}
}


/*********************************************************************//**
 * @brief Reject response message receiving [Mode : reject]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_reject_rsp_mode_reject(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_REJECT_WAIT_POLL;
				write_status_table();
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ex_cline_status_tbl.line_task_mode, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 59);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_REJECT_WAIT_POLL;
			write_status_table();
		}
		else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ex_cline_status_tbl.line_task_mode, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 60);
		}
	}
}


/*********************************************************************//**
 * @brief Reject response message receiving [Mode : reject]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_reject_rsp_mode_wait_note_removed(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else if (_is_id003_enable())
			{
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					write_status_table();

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
				else
				{
					//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
					write_status_table();
					//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ex_cline_status_tbl.line_task_mode, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 59);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			s_id003_status_wait_next_mode = ID003_MODE_DISABLE;
		}
		else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ex_cline_status_tbl.line_task_mode, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 60);
		}
	}
}


/*********************************************************************//**
 * @brief Reject response message receiving [Mode : reject]
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_reject_rsp_mode_disable_rej(void)
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if(s_id003_illegal_credit == 1)
			{
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
			}
			else if (_is_id003_enable())
			{
//				if ((ID003_MODE_DISABLE_REJECT_BOOKMARK == ex_cline_status_tbl.line_task_mode)
//				 || (ID003_MODE_ENABLE_REJECT_BOOKMARK == ex_cline_status_tbl.line_task_mode))
//				{
//					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_BOOKMARK;
//				}
//				else
//				{
//					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
//				}

				ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;

				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
//				if ((ID003_MODE_DISABLE_REJECT_BOOKMARK == ex_cline_status_tbl.line_task_mode)
//				 || (ID003_MODE_ENABLE_REJECT_BOOKMARK == ex_cline_status_tbl.line_task_mode))
//				{
//					ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE_BOOKMARK;
//				}
//				else
//				{
//					ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
//				}
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;

				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ex_cline_status_tbl.line_task_mode, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 59);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
//			if ((ID003_MODE_DISABLE_REJECT_BOOKMARK == ex_cline_status_tbl.line_task_mode)
///			 || (ID003_MODE_ENABLE_REJECT_BOOKMARK == ex_cline_status_tbl.line_task_mode))
//			{
//				s_id003_status_wait_next_mode = ID003_MODE_DISABLE_BOOKMARK;
//			}
//			else
//			{
//				s_id003_status_wait_next_mode = ID003_MODE_DISABLE;
//			}

			s_id003_status_wait_next_mode = ID003_MODE_DISABLE;

		}
		else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ex_cline_status_tbl.line_task_mode, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 60);
		}
	}
}

/*********************************************************************//**
 * @brief Times up message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_times_up_msg_proc(void)
{
	switch (cline_msg.arg1)
	{
	case TIMER_ID_DIPSW_READ:
		_id003_timeup_dipsw_read_proc();
		break;
	case TIMER_ID_ESCROW_STATUS_REQ1:
		_id003_timeup_escrow_stsreq_proc();
		break;
	case TIMER_ID_ESCROW_HOLD1:
		_id003_timeup_escrow_hold1_proc();
		break;
	case TIMER_ID_ESCROW_HOLD2:
		_id003_timeup_escrow_hold2_proc();
		break;
	case TIMER_ID_ENQ_SEND:
		_id003_timeup_enq_send_proc();
		break;
	case TIMER_ID_STATUS_WAIT1:
		_id003_timeup_sts_wait1_proc();
		break;
	case TIMER_ID_STATUS_WAIT2:
		_id003_timeup_sts_wait2_proc();
		break;
	default:					/* other */
		/* system error ? */
		_cline_system_error(0, 61);
		break;
	}
}

/*********************************************************************//**
 * @brief Read dipsw message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_timeup_dipsw_read_proc(void)
{

	u8 data;

	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_DISABLE:
		ex_cline_status_tbl.dipsw_disable = _id003_dipsw_disable();

		if ((_is_id003_enable())
		 && ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			//2024-05-12
			if( ( (ex_cline_status_tbl.option & ID003_OPTION_ENTRANCE_SENSOR) == ID003_OPTION_ENTRANCE_SENSOR )&& 
			(ex_cline_status_tbl.comm_mode == 0x0001)
			)
			{
				data = SENSOR_ENTRANCE;
				if(s_id003_sensor_enq_status != data)
				{
					_id003_send_host_enq();
				}
			}

			write_status_table();
		}

		_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DIPSW_READ, WAIT_TIME_DIPSW_READ, 0, 0);
		break;
	case ID003_MODE_ENABLE:
		ex_cline_status_tbl.dipsw_disable = _id003_dipsw_disable();

		if (!(_is_id003_enable())
		 && ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			if( ( (ex_cline_status_tbl.option & ID003_OPTION_ENTRANCE_SENSOR) == ID003_OPTION_ENTRANCE_SENSOR ) && 
			(ex_cline_status_tbl.comm_mode == 0x0001)
			)
			{
				data = SENSOR_ENTRANCE;
				if(s_id003_sensor_enq_status != data)
				{
					_id003_send_host_enq();
				}
			}

			write_status_table();
		}

		_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DIPSW_READ, WAIT_TIME_DIPSW_READ, 0, 0);
		break;
#if 0
	case ID003_MODE_DISABLE_BOOKMARK:
		ex_cline_status_tbl.dipsw_disable = _id003_dipsw_disable();

		if ((_is_id003_enable())
		 && ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_BOOKMARK;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			write_status_table();
		}

		_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DIPSW_READ, WAIT_TIME_DIPSW_READ, 0, 0);
		break;

	case ID003_MODE_ENABLE_BOOKMARK:
		ex_cline_status_tbl.dipsw_disable = _id003_dipsw_disable();

		if (!(_is_id003_enable())
		 && ((s_id003_status_wait_flag == 0) || (s_id003_status_wait_next_mode == 0)))
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE_BOOKMARK;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

			_id003_status_wait_clear();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			write_status_table();
		}

		_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DIPSW_READ, WAIT_TIME_DIPSW_READ, 0, 0);
		break;
#endif
	default:					/* other */
		ex_cline_status_tbl.dipsw_disable = _id003_dipsw_disable();
		write_status_table();

		_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_DIPSW_READ, WAIT_TIME_DIPSW_READ, 0, 0);
		break;
	}
}


/*********************************************************************//**
 * @brief Status request times up message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_timeup_escrow_stsreq_proc(void)
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_ESCROW:
		if (s_id003_status_wait_flag == 0)
		{
		/* Normal (Not Status WAIT) */
			_set_id003_reject(ID003_MODE_REJECT, REJECT_CODE_INHIBIT);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
		/* Status WAIT */
			/* time out */
			s_id003_status_wait_next_mode = ID003_MODE_REJECT;
			s_id003_status_wait_reject = REJECT_CODE_INHIBIT;
		}
		break;
	default:					/* other */
		break;
	}
}


/*********************************************************************//**
 * @brief Hold1 times up message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_timeup_escrow_hold1_proc(void)
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_HOLD1:
		if (s_id003_status_wait_flag == 0)
		{
		/* Normal (Not Status WAIT) */
			_set_id003_reject(ID003_MODE_REJECT, REJECT_CODE_INHIBIT);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
		/* Status WAIT */
			/* time out */
			s_id003_status_wait_next_mode = ID003_MODE_REJECT;
			s_id003_status_wait_reject = REJECT_CODE_INHIBIT;
		}
		break;
	default:					/* other */
		break;
	}
}


/*********************************************************************//**
 * @brief Hold2 times up message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_timeup_escrow_hold2_proc(void)
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_ESCROW_WAIT_CMD:
	case ID003_MODE_HOLD2:
		if (s_id003_status_wait_flag == 0)
		{
		/* Normal (Not Status WAIT) */
			_set_id003_reject(ID003_MODE_REJECT, REJECT_CODE_INHIBIT);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
		/* Status WAIT */
			/* time out */
			s_id003_status_wait_next_mode = ID003_MODE_REJECT;
			s_id003_status_wait_reject = REJECT_CODE_INHIBIT;
		}
		break;
	default:					/* other */
		break;
	}
}


/*********************************************************************//**
 * @brief Send ENQ message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_timeup_enq_send_proc(void)
{
	if (((ex_cline_status_tbl.comm_mode == 0x0001)
	  || (ex_cline_status_tbl.comm_mode == 0x0002))
	 && (s_id003_enq_resend != 0))
	{
	/* [Interrupt Mode 1/2] Enq resend flag */
		_id003_send_host_enq();
		_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ENQ_SEND, WAIT_TIME_RESEND_ENQ, 0, 0);
	}
}


/*********************************************************************//**
 * @brief Wait1 times up message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_timeup_sts_wait1_proc(void)
{
	if (s_id003_status_wait_flag == 1)
	{
		/* wait 解除 */
		_id003_status_wait_release();
	}
}


/*********************************************************************//**
 * @brief Wait2 times up message receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_timeup_sts_wait2_proc(void)
{
	if (s_id003_status_wait_flag == 2)
	{
		/* wait 解除 */
		_id003_status_wait_release();
	}
}

/*********************************************************************//**
 * @brief Clear Status Wait
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_status_wait_clear(void) //ok
{
	if (s_id003_status_wait_flag == 1)
	{
		_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_STATUS_WAIT1, 0, 0, 0);
	}
	else if (s_id003_status_wait_flag == 2)
	{
		_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_STATUS_WAIT2, 0, 0, 0);
	}
	s_id003_status_wait_flag = 0;
	s_id003_status_wait_next_mode = 0;
	s_id003_status_wait_escrow = 0;
	s_id003_status_wait_reject = 0;
	s_id003_status_wait_error = 0;
}


/*********************************************************************//**
 * @brief Release Status Wait
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_status_wait_release(void)
{
	switch (ex_cline_status_tbl.line_task_mode)
	{
	case ID003_MODE_POWERUP:
	case ID003_MODE_POWERUP_BILLIN_AT:
	case ID003_MODE_POWERUP_BILLIN_SK:
	case ID003_MODE_POWERUP_ERROR:
		if (s_id003_status_wait_next_mode != 0)
		{
			if ((s_id003_status_wait_next_mode == ID003_MODE_POWERUP)
			 && (ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP))
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP;
				ex_cline_status_tbl.error_code = 0;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else if ((s_id003_status_wait_next_mode == ID003_MODE_POWERUP_BILLIN_AT)
				  && (ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_AT))
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
				ex_cline_status_tbl.error_code = 0;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else if ((s_id003_status_wait_next_mode == ID003_MODE_POWERUP_BILLIN_SK)
				  && (ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_SK))
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
				ex_cline_status_tbl.error_code = 0;
				write_status_table();

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else if (s_id003_status_wait_next_mode == ID003_MODE_POWERUP_ERROR)
			{
				if (ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_ERROR)
				{
					_set_id003_alarm(ID003_MODE_POWERUP_ERROR, s_id003_status_wait_error);

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
				else if (s_id003_status_wait_error != 0)
				{
					_set_id003_alarm(ID003_MODE_POWERUP_ERROR, s_id003_status_wait_error);
				}
			}
			else
			{
				/* didn't change status */
			}
		}
		else
		{
			/* didn't change status */
		}
		break;
	case ID003_MODE_POWERUP_INITIAL:
		if (s_id003_status_wait_next_mode == ID003_MODE_DISABLE)
		{
			if (((s_id003_powerup_stat == POWERUP_STAT_RECOVER_FEED_STACK)
			  || (s_id003_powerup_stat == POWERUP_STAT_RECOVER_STACK) // リカバリー Full Vend for wait
			))
			{
				if((ex_cline_status_tbl.option & ID003_OPTION_RECOVERY) == ID003_OPTION_RECOVERY)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_VEND;
					write_status_table();

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);

				}
				else
				{
					set_recovery_step(RECOVERY_STEP_NON);
					if (_is_id003_enable())
					{
						if (ex_cline_status_tbl.comm_mode == 0x0002)
						{
							ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
							write_status_table();

							_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
						}
						else
						{
							//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
							ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
							write_status_table();

							//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
						}
					}
					else
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
						write_status_table();

						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
					}
					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
			}
			else
			{
				set_recovery_step(RECOVERY_STEP_NON);
				if (_is_id003_enable())
				{
					if (ex_cline_status_tbl.comm_mode == 0x0002)
					{
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
						write_status_table();

						_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
					}
					else
					{
						//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
						ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
						write_status_table();

						//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
					}
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
					write_status_table();

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
				}
				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_POWERUP_INITIAL_STACK)
		{
		/* Stack */
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_STACK;
			write_status_table();

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_STACK_REQ, 0, 0, 0, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
		{
		/* Reject */
			_set_id003_reject(ID003_MODE_POWERUP_INITIAL_REJECT, s_id003_status_wait_reject);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			if ((s_id003_status_wait_error == ALARM_CODE_STACKER_FULL)
			 && ((s_id003_powerup_stat == POWERUP_STAT_RECOVER_FEED_STACK)
			  || (s_id003_powerup_stat == POWERUP_STAT_RECOVER_STACK) // リカバリー Full Vend for wait
			))
			{
				if((ex_cline_status_tbl.option & ID003_OPTION_RECOVERY) == ID003_OPTION_RECOVERY)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_VEND;
					ex_cline_status_tbl.error_code = s_id003_status_wait_error;
					write_status_table();

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
				else
				{
					_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
			}
			else
			{
				_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else
		{
			/* didn't change status */
		}
		break;
	case ID003_MODE_POWERUP_INITIAL_STACK:
		if (s_id003_status_wait_next_mode == ID003_MODE_POWERUP_INITIAL)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL;
			write_status_table();

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
		}
		else if (s_id003_status_wait_next_mode== ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* didn't change status */
		}
		break;
	// Wait処理必要なし
	//case ID003_MODE_POWERUP_INITIAL_VEND:
	//case ID003_MODE_POWERUP_INITIAL_VEND_ACK:
	//	break;
	case ID003_MODE_POWERUP_INITIAL_REJECT:
		if (s_id003_status_wait_next_mode == ID003_MODE_POWERUP_INITIAL)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL;
			write_status_table();

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL_PAUSE;
			write_status_table();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode== ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* didn't change status */
		}
		break;
	case ID003_MODE_POWERUP_INITIAL_PAUSE:
		if (s_id003_status_wait_next_mode == ID003_MODE_POWERUP_INITIAL)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_INITIAL;
			write_status_table();

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
		{
			_set_id003_reject(ID003_MODE_POWERUP_INITIAL_REJECT, s_id003_status_wait_reject);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode== ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* didn't change status */
		}
		break;
	case ID003_MODE_INITIAL:
		if (s_id003_status_wait_next_mode == ID003_MODE_DISABLE)
		{
			if (_is_id003_enable())
			{
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					write_status_table();

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
				else
				{
					//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
					write_status_table();
					//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
			}

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_INITIAL_PAUSE)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL_PAUSE;
			write_status_table();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* didn't change status */
		}
		break;
	case ID003_MODE_INITIAL_STACK:
		if (s_id003_status_wait_next_mode == ID003_MODE_INITIAL)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL;
			write_status_table();

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
		}
		else if (s_id003_status_wait_next_mode== ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* didn't change status */
		}
		break;
	case ID003_MODE_INITIAL_REJECT:
		if (s_id003_status_wait_next_mode == ID003_MODE_INITIAL)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL;
			write_status_table();

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_INITIAL_PAUSE)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL_PAUSE;
			write_status_table();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode== ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* didn't change status */
		}
		break;
	case ID003_MODE_INITIAL_PAUSE:
		if (s_id003_status_wait_next_mode == ID003_MODE_INITIAL)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_INITIAL;
			write_status_table();

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_INITIAL_REJECT)
		{
			_set_id003_reject(ID003_MODE_INITIAL_REJECT, s_id003_status_wait_reject);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode== ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* didn't change status */
		}
		break;
	case ID003_MODE_AUTO_RECOVERY:
		if (s_id003_status_wait_next_mode == ID003_MODE_DISABLE)
		{
			if (_is_id003_enable())
			{
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					write_status_table();

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
				else
				{
					//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
					write_status_table();
					//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);
			}

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY_PAUSE;
			write_status_table();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* didn't change status */
		}
		break;
	case ID003_MODE_AUTO_RECOVERY_STACK:
		if (s_id003_status_wait_next_mode == ID003_MODE_AUTO_RECOVERY)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY;
			write_status_table();

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
		}
		else if (s_id003_status_wait_next_mode== ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* didn't change status */
		}
		break;
	case ID003_MODE_AUTO_RECOVERY_REJECT:
		if (s_id003_status_wait_next_mode == ID003_MODE_AUTO_RECOVERY)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY;
			write_status_table();

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY_PAUSE;
			write_status_table();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode== ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* didn't change status */
		}
		break;
	case ID003_MODE_AUTO_RECOVERY_PAUSE:
		if (s_id003_status_wait_next_mode == ID003_MODE_AUTO_RECOVERY)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY;
			write_status_table();

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
		{
			_set_id003_reject(ID003_MODE_AUTO_RECOVERY_REJECT, s_id003_status_wait_reject);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode== ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* didn't change status */
		}
		break;
	case ID003_MODE_DISABLE:
		if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	#if 0
	case ID003_MODE_DISABLE_BOOKMARK:
		if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	#endif
	case ID003_MODE_ENABLE:
		if (s_id003_status_wait_next_mode == ID003_MODE_ACCEPT)
		{
			set_recovery_step(RECOVERY_STEP_ACCEPT);
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ACCEPT;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ACCEPT_REQ, 0, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD1, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD2, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		//else if (s_id003_status_wait_next_mode == ID003_MODE_ENABLE_REJECT)
		//{
		//	_set_id003_reject(ID003_MODE_ENABLE_REJECT, s_id003_status_wait_reject);
		//
		//	_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		//
		//	/* change status */
		//	_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		//}
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	#if 0
	case ID003_MODE_ENABLE_BOOKMARK:
		if (s_id003_status_wait_next_mode == ID003_MODE_ACCEPT)
		{
			set_recovery_step(RECOVERY_STEP_ACCEPT);
			ex_cline_status_tbl.line_task_mode = ID003_MODE_ACCEPT_BOOKMARK;
			write_status_table();
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ACCEPT_REQ, TMSG_SUB_BOOKMARK, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD1, 0, 0, 0);
			_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_ESCROW_HOLD2, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	#endif
	case ID003_MODE_ACCEPT:
		if (s_id003_status_wait_next_mode == ID003_MODE_ESCROW)
		{
			if (cline_msg.arg2 == BAR_INDX)
			{
				if (_is_id003_bar_inhibit())
				{
					_set_id003_reject(ID003_MODE_REJECT, REJECT_CODE_INHIBIT);

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ESCROW;
					ex_cline_status_tbl.escrow_code = s_id003_status_wait_escrow;
					write_status_table();
					_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 300, 0, 0);

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
			}
			else
			{
				if (_is_id003_denomi_inhibit(cline_msg.arg2, cline_msg.arg3))
				{
					_set_id003_reject(ID003_MODE_REJECT, REJECT_CODE_INHIBIT);

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ESCROW;
					ex_cline_status_tbl.escrow_code = s_id003_status_wait_escrow;
					write_status_table();
					_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 300, 0, 0);

					/* change status */
					_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
				}
			}
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, s_id003_status_wait_reject);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	case ID003_MODE_ESCROW:
		if (s_id003_status_wait_next_mode == ID003_MODE_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, s_id003_status_wait_reject);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	case ID003_MODE_ESCROW_WAIT_CMD:
		if (s_id003_status_wait_next_mode == ID003_MODE_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, s_id003_status_wait_reject);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	case ID003_MODE_HOLD1:
		if (s_id003_status_wait_next_mode == ID003_MODE_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, s_id003_status_wait_reject);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	case ID003_MODE_HOLD2:
		if (s_id003_status_wait_next_mode == ID003_MODE_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, s_id003_status_wait_reject);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	case ID003_MODE_STACK:
		if (s_id003_status_wait_next_mode == ID003_MODE_VEND)
		{
			ex_cline_status_tbl.line_task_mode = s_id003_status_wait_next_mode;
			write_status_table();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_VEND_FULL)
		{
			ex_cline_status_tbl.line_task_mode = s_id003_status_wait_next_mode;
			write_status_table();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_PAUSE)
		{
			ex_cline_status_tbl.line_task_mode = s_id003_status_wait_next_mode;
			write_status_table();

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, s_id003_status_wait_reject);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	#if 0
	case ID003_MODE_STACK_BOOKMARK:
		if (s_id003_status_wait_next_mode == ID003_MODE_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, s_id003_status_wait_reject);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	#endif
	case ID003_MODE_PAUSE:
		/* din't change status */
		break;
	//case ID003_MODE_PAUSE_BOOKMARK:
		/* din't change status */
		break;
	case ID003_MODE_VEND:
	case ID003_MODE_WAIT_VEND_ACK:
	case ID003_MODE_VEND_FULL:
	case ID003_MODE_WAIT_VEND_ACK_FULL:
		if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	case ID003_MODE_STACKED:
		if (s_id003_status_wait_next_mode == ID003_MODE_DISABLE)
		{
			if (ex_cline_status_tbl.error_code == ALARM_CODE_STACKER_FULL)
			{
				_set_id003_alarm(ID003_MODE_ERROR, ALARM_CODE_STACKER_FULL);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else if (_is_id003_enable())
			{
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					write_status_table();

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
				else
				{
					//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
					write_status_table();
					//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	case ID003_MODE_STACKED_FULL:
		if (ex_cline_status_tbl.error_code != ALARM_CODE_OK)
		{
			_set_id003_alarm(ID003_MODE_ERROR, ex_cline_status_tbl.error_code);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	#if 0
	case ID003_MODE_STACKED_BOOKMARK:
		if (s_id003_status_wait_next_mode == ID003_MODE_DISABLE)
		{
			if (ex_cline_status_tbl.error_code == ALARM_CODE_STACKER_FULL)
			{
				_set_id003_alarm(ID003_MODE_ERROR, ALARM_CODE_STACKER_FULL);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else if (_is_id003_enable())
			{
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					write_status_table();

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
				else
				{
					//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
					write_status_table();
					//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	#endif

	#if 0
	//case ID003_MODE_STACK_FINISH:
	//case ID003_MODE_NEARLY_FULL:
		if (s_id003_status_wait_next_mode == ID003_MODE_DISABLE)
		{
			if (ex_cline_status_tbl.error_code == ALARM_CODE_STACKER_FULL)
			{
				_set_id003_alarm(ID003_MODE_ERROR, ALARM_CODE_STACKER_FULL);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else if (_is_id003_enable())
			{
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					write_status_table();

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
				else
				{
					//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
					write_status_table();
					//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* don't change status */
		}
		break;
	#endif
	case ID003_MODE_REJECT_WAIT_ACCEPT_RSP:
		if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	case ID003_MODE_DISABLE_REJECT:
	case ID003_MODE_ENABLE_REJECT:
	case ID003_MODE_REJECT:
	case ID003_MODE_REJECT_WAIT_POLL:
	case ID003_MODE_REJECT_WAIT_NOTE_REMOVED:
	case ID003_MODE_RETURN:
	//case ID003_MODE_DISABLE_REJECT_BOOKMARK:
	//case ID003_MODE_ENABLE_REJECT_BOOKMARK:
		if (s_id003_status_wait_next_mode == ID003_MODE_DISABLE)
		{
			if (_is_id003_enable())
			{
				if (ex_cline_status_tbl.comm_mode == 0x0002)
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					write_status_table();

					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}
				else
				{
					//ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE;
					ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_WAIT_POLL;
					write_status_table();
					//_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);
				}

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
	#if 0
		else if (s_id003_status_wait_next_mode == ID003_MODE_DISABLE_BOOKMARK)
		{
			if (_is_id003_enable())
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_ENABLE_BOOKMARK;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_ENABLE_REQ, 0, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
			else
			{
				ex_cline_status_tbl.line_task_mode = ID003_MODE_DISABLE_BOOKMARK;
				write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_DISABLE_REQ, 0, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
	#endif
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else
		{
			/* din't change status */
		}
		break;
	case ID003_MODE_ERROR:
		if (s_id003_status_wait_next_mode == ID003_MODE_AUTO_RECOVERY)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_AUTO_RECOVERY;
			ex_cline_status_tbl.escrow_code = 0x0000;
			ex_cline_status_tbl.error_code = ALARM_CODE_OK;
			ex_cline_status_tbl.reject_code = 0x0000;
			ex_cline_status_tbl.accept_disable = 0x0001;
			write_status_table();

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_RESET_REQ, RESET_TYPE_NORMAL, 0, 0, 0);

			/* change status */
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (s_id003_status_wait_next_mode == ID003_MODE_ERROR)
		{
			if ((s_id003_status_wait_error != 0) && (s_id003_status_wait_error != ex_cline_status_tbl.error_code))
			{
				/* change error code */
				_set_id003_alarm(ID003_MODE_ERROR, s_id003_status_wait_error);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			}
		}
		else
		{
			/* din't change status */
		}
		break;
	#if defined(UBA_RTQ)
	case 	ID003_MODE_POWERUP_INITIAL_PAYVALID:
	case 	ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK:	
	case 	ID003_MODE_COLLECT:					/* 60 */
	case 	ID003_MODE_COLLECTED_WAIT_POLL:
	case 	ID003_MODE_RETURN_TO_BOX:			//エマージェンシーストップで紙幣取り込み中
	case 	ID003_MODE_PAYOUT:					/* 63 */
	case 	ID003_MODE_PAYOUT_RETURN_NOTE:
	case 	ID003_MODE_PAYOUT_COLLECTED_WAIT_POLL:
	case 	ID003_MODE_PAYSTAY:
	case 	ID003_MODE_PAYSTAY_WAIT_POLL:
	case 	ID003_MODE_PAYVALID:
	case 	ID003_MODE_PAYVALID_ERROR:
	case 	ID003_MODE_WAIT_PAYVALID_ACK:
	case 	ID003_MODE_WAIT_PAYVALID_ACK_ERROR:
	case 	ID003_MODE_AFTER_PAYVALID_ACK_ENABLE:
	case 	ID003_MODE_AFTER_PAYVALID_ACK_DISABLE:
	case 	ID003_MODE_AFTER_PAYVALID_ACK_PAYOUT:
	case 	ID003_MODE_DIAGNOSTIC:				/* 76 */
	case 	ID003_MODE_RETURN_ERROR:	//	Pay out紙幣がエマージョエンシ―ストップで、回収、識別エラーで返却中

		break;
	#endif
	}

	_id003_status_wait_clear();
}


void _id003_intr_mode_sub(u16 mode, u8 wait_flag) //not use wait_flag
{
	_id003_intr_mode_sub_uba(mode, wait_flag); //not use wait_flag
}


void _id003_illegal_credit(void)
{
	s_id003_illegal_credit = 1;
	uart_txd_stall_id003();

	if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_AT)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_SK)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_WAIT_POLL)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
	// || (ex_cline_status_tbl.line_task_mode == ID003_MODE_STACK_FINISH)
	// || (ex_cline_status_tbl.line_task_mode == ID003_MODE_NEARLY_FULL)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ERROR)
	// || (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_BOOKMARK)
	// || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_BOOKMARK)
	 )
	{
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
	}
	else if (((ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND)
		   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_VEND_ACK)
		   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_VEND_FULL)
		   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_WAIT_VEND_ACK_FULL)
		   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_STACKED)
		   || (ex_cline_status_tbl.line_task_mode == ID003_MODE_STACKED_FULL))
		  && ((s_id003_stacking_info & ID003_STACKING_INFO_BUSY) == ID003_STACKING_INFO_BUSY))
	{
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
	}
	else if ((ex_cline_status_tbl.line_task_mode == ID003_MODE_ESCROW)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ACCEPT_WAIT_POLL_FOR_ESCROW)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ACCEPT_WAIT_POLL_FOR_REJECT)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ESCROW_WAIT_CMD)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD1)
	 || (ex_cline_status_tbl.line_task_mode == ID003_MODE_HOLD2))
	{
		_set_id003_reject(ID003_MODE_REJECT, REJECT_CODE_OPERATION);

		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);

		_id003_status_wait_clear();
	}
	else if((ex_cline_status_tbl.line_task_mode == ID003_MODE_ACCEPT)
	/* || (ex_cline_status_tbl.line_task_mode == ID003_MODE_ACCEPT_BOOKMARK) */
	 )
	{
		_set_id003_reject(ID003_MODE_REJECT_WAIT_ACCEPT_RSP, REJECT_CODE_INHIBIT);
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_CHEAT, 0, 0);
	}
}

/*
void _id003_status_tbl_reset(void)
{
	ex_cline_status_tbl.escrow_code = 0x0000;
	ex_cline_status_tbl.error_code = ALARM_CODE_OK;
	ex_cline_status_tbl.reject_code = 0x0000;
	ex_cline_status_tbl.bill_disable = ~(ex_cline_status_tbl.bill_disable_mask);
	ex_cline_status_tbl.security_level = 0x0000;
	ex_cline_status_tbl.accept_disable = 0x0001;
	ex_cline_status_tbl.direction_disable = 0x0000;
	ex_cline_status_tbl.option = 0x0000;
}
*/


/*********************************************************************//**
 * @brief Send ENQ
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _id003_send_host_enq(void)
{
	if (s_id003_illegal_credit == 0)
	{
		_id003_tx_buff.length = 1 + 4;
		_id003_tx_buff.cmd = ID003_CMD_ENQ;
		uart_send_id003(&_id003_tx_buff);
	}
}


u16 _id003_dipsw_disable(void)
{
	u16 dipsw_disable;

	dipsw_disable = (ex_dipsw1 & 0x7F);

	return dipsw_disable;
}


void _set_id003_reject(u16 mode, u16 code)
{
	if (ex_recovery_info.step != RECOVERY_STEP_NON)
	{
		set_recovery_step(RECOVERY_STEP_NON);
	}

	ex_cline_status_tbl.line_task_mode = mode;
	ex_cline_status_tbl.reject_code = code;
	write_status_table();
	s_id003_2nd_note = 0;
}


void _set_id003_alarm(u16 mode, u16 code)
{
	if (ex_recovery_info.step != RECOVERY_STEP_NON)
	{
		set_recovery_step(RECOVERY_STEP_NON);
	}

	ex_cline_status_tbl.line_task_mode = mode;
	ex_cline_status_tbl.error_code = code;
	write_status_table();
	s_id003_2nd_note = 0;
}

/*********************************************************************//**
 * @brief is accept enable
 * @param[in]	None
 * @return true  : accept enable
 * @return false : accept disable
 **********************************************************************/
bool _is_id003_enable(void)
{
#if (MULTI_COUNTRY == 0)
	extern const u8 id003_bill_info[18][6];
	extern const u16 id003_inhi_mask;
#else
    extern const u8 id003_bill_info[MULTI_COUNTRY][18][6];
    extern const u16 id003_inhi_mask[MULTI_COUNTRY];
#endif
	extern const u16 dipsw_inhi[];
	extern const u8 id003_index_to_code[];
	u16 inhi = 0;
	u16 denomi_idx;
	u16 binfo_idx;
	bool rtn = true;

	extern const u16 dipsw_bar_inhi;
    bool bar_dip_disable = 0;

#if (MULTI_COUNTRY == 0)
/***/
	for (denomi_idx = 0; denomi_idx < (DENOMI_SIZE / 4); denomi_idx++)
	{
		if ((ex_cline_status_tbl.dipsw_disable & dipsw_inhi[denomi_idx]) != 0)
		{
			for (binfo_idx = 0; ((id003_bill_info[binfo_idx][0] != 0) && (binfo_idx < 18)); binfo_idx++)
			{
				if (id003_index_to_code[denomi_idx] == id003_bill_info[binfo_idx][0])
				{
					inhi |= ((u16)(id003_bill_info[binfo_idx][4] << 8) | (u16)(id003_bill_info[binfo_idx][5]));
				}
			}
		}
	}
	inhi = ((ex_cline_status_tbl.bill_disable | inhi) | ~id003_inhi_mask);
/***/
	if (ex_cline_status_tbl.accept_disable)
	{
		rtn = false;
	}
	if ((ex_cline_status_tbl.barcode_inhibit & 0x0003) == 0x0003)
	{
		rtn = false;
	}
	else
	{
        if ((ex_cline_status_tbl.barcode_inhibit == 0x0000)
		|| ((ex_cline_status_tbl.barcode_inhibit & 0x0003) == 0x0002)
		){
		/* 紙幣のみ受け取りの0xFE設定*/
            if( (inhi & id003_inhi_mask) == id003_inhi_mask)
            {
                rtn = false;
            }
            if( ex_cline_status_tbl.direction_disable == 0x000F )
            {
                rtn = false;
            }
        }
		else if((ex_cline_status_tbl.barcode_inhibit & 0x0003) == 0x0000)
		{
		/* 紙幣とTikcetの受け取りモード 0xFC */

			/* バーチケットDIP-SW Inhibit確認*/
			if( ex_dipsw1 & (u8)(dipsw_bar_inhi) )
			{
                bar_dip_disable = 1;
			}

            if( ( (inhi & id003_inhi_mask) == id003_inhi_mask ) && (bar_dip_disable == 1) )
            {
                rtn = false;
            }
            if( (ex_cline_status_tbl.direction_disable == 0x000F) && (bar_dip_disable == 1) )
            {
                rtn = false;
            }

		}
		else if((ex_cline_status_tbl.barcode_inhibit & 0x0003) == 0x0001)
		{
		/* Tikcetのみ受け取りモード 0xFD */
			/* バーチケットDIP-SW Inhibit確認*/
			if( ex_dipsw1 & (u8)(dipsw_bar_inhi) )
			{
                rtn = false;
			}
		}
	}


	/*  2021.12.16
		Bill validator should still be able to accept notes/
		ticket when either bill acceptance or ticket acceptance is enabled,  
		as long as the Barcode Inhibit (C7) command is set to 00 	
	*/
	if (ex_cline_status_tbl.barcode_inhibit == 0x0000)
	{	
		if(!(ex_dipsw1 & (u8)(dipsw_bar_inhi) )
		&& !(ex_cline_status_tbl.accept_disable)) // if unit isn't disabled via inhibit command
		{			
			rtn = true; // Keep bill validator idled
		}	
	}

	return rtn;

#else

	for (denomi_idx = 0; denomi_idx < DENOMI_SIZE; denomi_idx++)
	{
		if ((ex_cline_status_tbl.dipsw_disable & dipsw_inhi[denomi_idx]) != 0)
		{
			for (binfo_idx = 0; ((id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][0] != 0) && (binfo_idx < 18)); binfo_idx++)
			{
				if (id003_index_to_code[denomi_idx] == id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][0])
				{
					inhi |= ((u16)(id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][4] << 8) | (u16)(id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][5]));
				}
			}
		}
	}
	inhi = ((ex_cline_status_tbl.bill_disable | inhi) | ~id003_inhi_mask[ex_cline_status_tbl.country_setting]);
/***/
    if (ex_cline_status_tbl.accept_disable)
    {
        rtn = false;
    }
	
    if( (ex_cline_status_tbl.barcode_inhibit & 0x0003) == 0x0003 )
    {
        rtn = false;
    }
    else
    {
        /* 紙幣受け取るモードの場合 */
        if( (ex_cline_status_tbl.barcode_inhibit == 0x0000)            ||
            ((ex_cline_status_tbl.barcode_inhibit & 0x0003) == 0x0002) )
        {
            /* 全金種もしくは全方向受け取り禁止の場合inhibitにする */
            /* (Ticketだけ受け取らせたい場合はbarcode_inhibitを1にする) */
            if( (inhi & id003_inhi_mask[ex_cline_status_tbl.country_setting]) == id003_inhi_mask[ex_cline_status_tbl.country_setting] )
            {
                rtn = false;
            }
            if( ex_cline_status_tbl.direction_disable == 0x000F )
            {
                rtn = false;
            }
        }
    }

	return rtn;

#endif

}


/*********************************************************************//**
 * @brief is barcode inhibit
 * @param[in]	None
 * @return true  : inhibit
 * @return false : accept
 **********************************************************************/
bool _is_id003_bar_inhibit(void)
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

/*********************************************************************//**
 * @brief is denomi inhibit
 * @param[in]	None
 * @return true  : inhibit
 * @return false : accept
 **********************************************************************/
bool _is_id003_denomi_inhibit(u32 denomi_code, u32 direction)
{
#if (MULTI_COUNTRY == 0)
	extern const u8 id003_bill_info[18][6];
#else  /* MULTI_COUNTRY != 0 */
	extern const u8 id003_bill_info[MULTI_COUNTRY][18][6];
#endif /* MULTI_COUNTRY == 0 */
	extern const u16 dipsw_inhi[];
	bool rtn = false;
	u16 cmd_inhi = 0;
	u16 binfo_idx;
	u8 escrow_code;
	u8 dir_bit;

	//2023-11-30  
	//UBA500はdiscrimination_taskで処理しているが
	//アルゴが変わる可能性もあるので、ここで処理する
	//ex_Barcom.enableはチケットの識別処理で更新されるので、紙幣の場合処理を通らないので、
	//別変数を使用した方がいい
	//if((ex_Barcom.enable & 0x03) == 0x02)
	if((ex_cline_status_tbl.barcode_inhibit & 0x0001) == 0x0001)
	{
		/* 紙幣inhibit */
	//	ex_validation.reject_code = REJECT_CODE_INHIBIT;
		return true;
	}

	if((ex_system & BIT_SU_UNIT) == BIT_SU_UNIT)
	{
		/* SU setting, change A to C, B to D, C to A, D to B */
		if ((direction & 0x0000000F) == W3)
		{ /* B */
			dir_bit = 0x02;
		}
		else if ((direction & 0x0000000F) == W2)
		{ /* A */
			dir_bit = 0x01;
		}
		else if ((direction & 0x0000000F) == W1)
		{ /* D */
			dir_bit = 0x08;
		}
		else
		{ /* C */
			dir_bit = 0x04;
		}
	}
	else
	{
		if ((direction & 0x0000000F) == W3)
		{ /* D */
			dir_bit = 0x08;
		}
		else if ((direction & 0x0000000F) == W2)
		{ /* C */
			dir_bit = 0x04;
		}
		else if ((direction & 0x0000000F) == W1)
		{ /* B */
			dir_bit = 0x02;
		}
		else
		{ /* A */
			dir_bit = 0x01;
		}
	}

	escrow_code = _is_id003_escrow_data((u16)denomi_code);
	for (binfo_idx = 0; binfo_idx < 18; binfo_idx++)
	{
#if (MULTI_COUNTRY == 0)
		if (id003_bill_info[binfo_idx][0] == escrow_code)
		{
			cmd_inhi |= (u16)(id003_bill_info[binfo_idx][4]<<8) | (u16)(id003_bill_info[binfo_idx][5]);
			break;
		}
#else  /* MULTI_COUNTRY != 0 */
		if (id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][0] == escrow_code)
		{
			cmd_inhi |= (u16)(id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][4]<<8) | (u16)(id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][5]);
			break;
		}
#endif /* MULTI_COUNTRY == 0 */
	}

	/* Check DipSW Inhibit */
	if (binfo_idx >= 18)
	{
		rtn = true;
	}
	else if ((ex_cline_status_tbl.dipsw_disable & dipsw_inhi[denomi_code]) != 0)
	{
		rtn = true;
	}
	/* Check Command Inhibit */
	if ((ex_cline_status_tbl.bill_disable & cmd_inhi) != 0)
	{
		rtn = true;
	}
	/* Check Direction Inhibit */
	if ((dir_bit & ex_cline_status_tbl.direction_disable) == dir_bit)
	{
		rtn = true;
	}

	return rtn;
}


u8 _is_id003_escrow_data(u16 denomi_code)
{
	extern const u8 id003_index_to_code[];
	return id003_index_to_code[denomi_code];
}


u8 _is_id003_reject_data(u16 reject_code)
{
	u8 rtn;
	switch (reject_code)
	{
	case REJECT_CODE_INSERT_CANCEL:
	case REJECT_CODE_SKEW:
		rtn = ID003_REJECT_INSERTION;					/* 挿入異常 */
		break;
	case REJECT_CODE_MAG_PATTERN:
	case REJECT_CODE_MAG_AMOUNT:
		rtn = ID003_REJECT_MAG;							/* マグ異常 */
		break;
	case REJECT_CODE_ACCEPTOR_STAY_PAPER:
		rtn = ID003_REJECT_REMAINING_BILL_IN_ACCEPTOR;	/* 紙幣の残留(アクセプタ部) */
		break;
	case REJECT_CODE_XRATE:
		rtn = ID003_REJECT_XRATE;						/* 補正/振幅率異常 */
		break;
	case REJECT_CODE_FEED_SLIP:
	case REJECT_CODE_FEED_MOTOR_LOCK:
	case REJECT_CODE_FEED_TIMEOUT:
	case REJECT_CODE_BOX:
	case REJECT_CODE_STACKER_HOME:
	case REJECT_CODE_APB_HOME:
	case REJECT_CODE_CENTERING_HOME:
	case REJECT_CODE_STRING_DOWN:
	case REJECT_CODE_STRING_UP:
	case REJECT_CODE_SIDE_LOW_SENSITIVITY:
	case REJECT_CODE_SIDE_HIGH_SENSITIVITY:
	case REJECT_CODE_LOST_BILL:
		rtn = ID003_REJECT_FEED;						/* 搬送異常 */
		break;
	case REJECT_CODE_PRECOMP:
		rtn = ID003_REJECT_PRECOMP;						/* 金種判定異常 */
		break;
	case REJECT_CODE_PATTERN:
		rtn = ID003_REJECT_PATTERN1;					/* フォトパタン異常1 */
		break;
	case REJECT_CODE_PHOTO_LEVEL:
		rtn = ID003_REJECT_PHOTO_LEVEL;					/* フォトレベル異常 */
		break;
	case REJECT_CODE_INHIBIT:
	case REJECT_CODE_ESCROW_TIMEOUT:
	case REJECT_CODE_RETURN:
		rtn = ID003_REJECT_INHIBIT;						/* INHIBIT信号 or エスクロタイムアウト */
		break;
	case REJECT_CODE_OPERATION:
		rtn = ID003_REJECT_OPERATION;					/* オペレーション異常 */
		break;
	case REJECT_CODE_STACKER_STAY_PAPER:
		rtn = ID003_REJECT_REMAINING_BILL_IN_STACKER;	/* 紙幣の残留(スタッカー部) */
		break;
	case REJECT_CODE_LENGTH:
	case REJECT_CODE_PAPER_SHORT:
	case REJECT_CODE_PAPER_LONG:
		rtn = ID003_REJECT_LENGTH;						/* 長さエラー */
		break;
	case REJECT_CODE_FAKE_MCIR:
	case REJECT_CODE_FAKE_M3C:
	case REJECT_CODE_FAKE_M4C:
	case REJECT_CODE_FAKE_IR:
		rtn = ID003_REJECT_PATTERN2;					/* フォトパタン異常2 */
		break;
	case REJECT_CODE_COUNTERFEIT:
	case REJECT_CODE_THREAD:
	case REJECT_CODE_UV:
		rtn = ID003_REJECT_COUNTERFEIT;					/* 真券特徴異常 */
		break;
	case REJECT_CODE_BAR_NC:
		rtn = ID003_REJECT_BAR_NC; //0x91
		break;
	case REJECT_CODE_BAR_UN:
		rtn = ID003_REJECT_BAR_UN; //0x92
		break;
	case REJECT_CODE_BAR_SH:
		rtn = ID003_REJECT_BAR_SH; //0x93
		break;
	case REJECT_CODE_BAR_ST:
		rtn = ID003_REJECT_BAR_ST; //0x94
		break;
	case REJECT_CODE_BAR_SP:
		rtn = ID003_REJECT_BAR_SP;//0x95
		break;
	case REJECT_CODE_BAR_TP:
		rtn = ID003_REJECT_BAR_TP;//0x96
		break;
	case REJECT_CODE_BAR_XR:
		rtn = ID003_REJECT_BAR_XR;//0x97
		break;
	case REJECT_CODE_BAR_PHV:
		rtn = ID003_REJECT_BAR_PHV;//0x98
		break;
	case REJECT_CODE_BAR_DIN:
		rtn = ID003_REJECT_BAR_DIN;//0x9B
		break;
	case REJECT_CODE_BAR_LG:
		rtn = ID003_REJECT_BAR_LG;//0x9D
		break;
	case REJECT_CODE_BAR_NG:
		rtn = ID003_REJECT_PATTERN2; //0x7E
		break;
	case REJECT_CODE_BAR_MC:
		rtn = ID003_REJECT_COUNTERFEIT;//0x7F
		break;
	default:					/* other */
		rtn = ID003_REJECT_FEED;
		break;
	}
	return rtn;
}


u8 _is_id003_error_sts(u16 alarm_code, u8 *data)
{
	u8 rtn;
	switch (alarm_code)
	{
	case ALARM_CODE_FRAM:
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_EXT_ROM;
		break;
	case ALARM_CODE_STACKER_FULL:
		rtn = ID003_STS_STACKER_FULL;
		break;
	case ALARM_CODE_FEED_OTHER_SENSOR_SK:
	case ALARM_CODE_FEED_SLIP_SK:
	case ALARM_CODE_FEED_TIMEOUT_SK:
	case ALARM_CODE_FEED_LOST_BILL:
	 case ALARM_CODE_FEED_MOTOR_LOCK_SK: //2024-02-13
		rtn = ID003_STS_JAM_IN_SK;
		break;
	case ALARM_CODE_STACKER_MOTOR_LOCK:
	case ALARM_CODE_STACKER_GEAR:
	case ALARM_CODE_STACKER_TIMEOUT:
	case ALARM_CODE_STACKER_HOME:
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_STACKER_MOTOR;
		break;
	case ALARM_CODE_FEED_MOTOR_SPEED_LOW:
	case ALARM_CODE_FEED_MOTOR_SPEED_HIGH:
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_FEED_MOTOR_SPEED;
		break;
	case ALARM_CODE_FEED_MOTOR_LOCK:
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_FEED_MOTOR;
		break;
	case ALARM_CODE_FEED_OTHER_SENSOR_AT:
	case ALARM_CODE_FEED_SLIP_AT:
	case ALARM_CODE_FEED_TIMEOUT_AT:
	case ALARM_CODE_FEED_MOTOR_LOCK_AT:
	case ALARM_CODE_UV:
		rtn = ID003_STS_JAM_IN_AT;
		break;
	//2024-06-09
	case ALARM_CODE_FEED_CIS_AT:
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_HEAD;
		break;

	case ALARM_CODE_APB_HOME:
	case ALARM_CODE_APB_TIMEOUT:
	case ALARM_CODE_APB_HOME_STOP:
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_APB_UNIT; /* UBA10,iPRO,UBA500に合わせる 0xAF*/
		break;
	case ALARM_CODE_BOX:
	case ALARM_CODE_BOX_INIT:
		rtn = ID003_STS_STACKER_BOX_REMOVE;
		break;

	case ALARM_CODE_CHEAT:
		rtn = ID003_STS_CHEATED;
		break;
	case ALARM_CODE_CENTERING_TIMEOUT:
	case ALARM_CODE_CENTERING_HOME_STOP:
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_CENTERING;
		break;
	case ALARM_CODE_RFID_ICB_SETTING: 			/* ICB無効でICBユニットがある場合 */
	case ALARM_CODE_RFID_ICB_MC_INVALID:	 	/* ICB有効無効フラグ、マシンナンバー未設定 */
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_ICB_SETTING;
		break;
	case ALARM_CODE_RFID_UNIT_MAIN:					/* ICB有効でRFIDユニットがない場合 */
	case ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN: 		/* ICB読み込み、書込み失敗 */
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_ICB_RW;
		break;
	case ALARM_CODE_RFID_ICB_DATA: 				/* ICBチェックサムデータ異常 */
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_ICB_SUM;
		break;
	case ALARM_CODE_RFID_ICB_NUMBER_MISMATCH:	/* ICB別のゲーム機にセットされていたBOXです */
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_ICB_MC_NUMBER;
		break;
	case ALARM_CODE_RFID_ICB_NOT_INITIALIZE: 	/* ICB集計データセーブ済みのBOXです */
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_ICB_NOT_INITIALIZED;
		break;
#if defined(UBA_RTQ)
		//2024-12-12
		case ALARM_CODE_RC_REMOVED:
            rtn = ID003_STS_RC_ERROR;
			break;
		case ALARM_CODE_RC_ERROR:
            rtn = ID003_STS_RC_ERROR;
			break;
		case ALARM_CODE_RC_ROM:	//UBA500もコードはあるが使用していない可能性あり
            rtn = ID003_STS_FAILURE;
            *data = ID003_FAILURE_RC_ROM;
			break;
		case ALARM_CODE_RC_COM: //use
            rtn = ID003_STS_FAILURE;
            *data = ID003_FAILURE_RC_COMM;
			break;
		case ALARM_CODE_RC_DWERR: //use
            rtn = ID003_STS_FAILURE;
            *data = ID003_FAILURE_RC_ROM;
			break;
		case ALARM_CODE_RC_POS:
            rtn = ID003_STS_RC_ERROR;
			break;
		case ALARM_CODE_RC_TRANSPORT:
            rtn = ID003_STS_RC_ERROR;
			break;
		case ALARM_CODE_RC_TIMEOUT:
            rtn = ID003_STS_RC_ERROR;
			break;
		case ALARM_CODE_RC_DENOMINATION:
            rtn = ID003_STS_RC_ERROR;
			break;
		case ALARM_CODE_RC_EMPTY:
            rtn = ID003_STS_RC_ERROR;
			break;
		case ALARM_CODE_RC_DOUBLE:
            rtn = ID003_STS_RC_ERROR;
			break;
		case ALARM_CODE_RC_FULL:
            rtn = ID003_STS_RC_ERROR;
			break;
		case ALARM_CODE_RC_EXCHAGED:
            rtn = ID003_STS_RC_ERROR;
			break;
	#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)
		case ALARM_CODE_RC_RFID:
		//	rtn = ID003_STS_RC_ERROR;
			rtn = ID003_STS_FAILURE;
			*data = ID003_FAILURE_ICB_RTQ;
		break;
	#endif
#endif // UBA_RTQ
	default:					/* other */
		rtn = ID003_STS_FAILURE;
		*data = ID003_FAILURE_HEAD;
		break;
	}
	return rtn;
}


void interface_get_bar_info(u8 *type, u8 *length, u8 *length2, u8 *length3,u8 *length4,u8 *inhibit)//ok
{
	*type = ex_cline_status_tbl.barcode_type;						/* Barcode type */
	if (ex_cline_status_tbl.barcode_length == 0xFF)					/* Barcode length of charactor */
	{
		*length = 28;
		*length2 = 28;
		*length3 = 28;
		*length4 = 28;
	}
	else if ((ex_cline_status_tbl.option & ID003_OPTION_24CHAR_TICKET) == ID003_OPTION_24CHAR_TICKET)
	{
		*length = ex_cline_status_tbl.barcode_length;
		*length2 = 24;
		*length3 = 0;
		*length4 = 0;
	}
	else
	{
		*length = ex_cline_status_tbl.barcode_length;
		*length2 = 0;
		*length3 = 0;
		*length4 = 0;
	}
	*inhibit = ((~(ex_cline_status_tbl.barcode_inhibit)) & 0x03);	/* Barcode inhibit */
}

/*********************************************************************//**
 * @brief is denomi inhibit
 * @param[in]	None
 * @return true  : inhibit
 * @return false : accept
 **********************************************************************/
bool condition_denomi_inhibit_id003(u32 denomi_code, u32 direction)
{
	bool ret = false;
	u8 dir_bit;
	u16 cmd_inhi = 0;
	u16 binfo_idx;
	u8 escrow_code;

#if (MULTI_COUNTRY == 0)
	extern const u8 id003_bill_info[18][6];
#else  /* MULTI_COUNTRY != 0 */
	extern const u8 id003_bill_info[MULTI_COUNTRY][18][6];
#endif /* MULTI_COUNTRY == 0 */
	extern const u16 dipsw_inhi[];

	/* Check the DipSW Inhibit */
	if((ex_cline_status_tbl.dipsw_disable & dipsw_inhi[denomi_code]) != 0)
	{
		ret = true;
	}

	/* Check the Direction Inhibit */
	dir_bit = 0x0F;
	if((dir_bit & ex_cline_status_tbl.direction_disable) == dir_bit)
	{
		ret = true;
	}

	/* Check the Command Inhibit */
	escrow_code = _is_id003_escrow_data((u16)denomi_code);
	for(binfo_idx = 0; binfo_idx < 18; binfo_idx++)
	{
#if (MULTI_COUNTRY == 0)
		if(id003_bill_info[binfo_idx][0] == escrow_code)
		{
			cmd_inhi |= (u16)(id003_bill_info[binfo_idx][5]) | (u16)(id003_bill_info[binfo_idx][4]<<8);
			break;
		}
#else  /* MULTI_COUNTRY != 0 */
		if (id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][0] == escrow_code)
		{
			cmd_inhi |= (u16)(id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][5]) | (u16)(id003_bill_info[ex_cline_status_tbl.country_setting][binfo_idx][4]<<8);
			break;
		}
#endif /* MULTI_COUNTRY == 0 */
	}

	if((ex_cline_status_tbl.bill_disable & cmd_inhi) != 0)
	{
		ret = true;
	}

	return(ret);
}

#if !defined(UBA_RTQ)
bool is_security_mode(void)// true == no stacker 暗号化有効かの確認
{
    //TODO unsure what function is_security_mode() is supposed to achieve - investigate
	#if !defined(UBA_RTQ)
	if((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
	{
		return true; //enable
	}
	else
	{
		return false; //disable
	}

	#else
	return false; //disable
	#endif
}
#endif

void _id003_version_cmd_proc_uba(void)//2023-06-28
{
	extern const u8 Ctry_software_id[];
    extern const u8 Ctry_software_id_uba[];

    u16 txbuff_index;
    u16 temp_index;
    u8 seq;
    u8 crc_char;

	_id003_tx_buff.cmd = ID003_CMD_GET_VERSION;

	txbuff_index = 0;

	seq = 1;
	temp_index = 0;
	while( (seq != 0) && (txbuff_index < 250) )
	{
		switch( seq )
		{
			/* Set Model (Initial) */
			case 1:
				if( (0 != Ctry_software_id[temp_index]) && (is_legacy_mode_enable() == false))
                {
					_id003_tx_buff.data[txbuff_index++] = Ctry_software_id[temp_index];
					temp_index++;
				}
				else if( (0 != Ctry_software_id_uba[temp_index]) && (is_legacy_mode_enable() == true))
                {
					_id003_tx_buff.data[txbuff_index++] = Ctry_software_id_uba[temp_index];
					temp_index++;
				}
				else
				{
					seq = 0;
					temp_index = 0;
					/* add space	*/
					_id003_tx_buff.data[txbuff_index++] = 0x20;
				}
				break;
			default:
				seq = 0;
				break;
		}
	}
	crc_char = (u8)((ex_rom_crc >> 12) & 0xf);
	_id003_tx_buff.data[txbuff_index++] = (crc_char >= 10) ? (crc_char - 10 + 'A') : (crc_char + '0');
	crc_char = (u8)((ex_rom_crc >>  8) & 0xf);
	_id003_tx_buff.data[txbuff_index++] = (crc_char >= 10) ? (crc_char - 10 + 'A') : (crc_char + '0');
	crc_char = (u8)((ex_rom_crc >>  4) & 0xf);
	_id003_tx_buff.data[txbuff_index++] = (crc_char >= 10) ? (crc_char - 10 + 'A') : (crc_char + '0');
	crc_char = (u8)(ex_rom_crc & 0xf);
	_id003_tx_buff.data[txbuff_index++] = (crc_char >= 10) ? (crc_char - 10 + 'A') : (crc_char + '0');

	_id003_tx_buff.length = (txbuff_index + 1) + 4;

	uart_send_id003(&_id003_tx_buff);

}



void _id003_accept_evt_success_2nd(void) //mainのmode_accept関係からのメッセージ
{

	/* この関数が呼び出される時*/
	/* 識別処理は完了していて、Mainはラインタスクからの受け取り、返却命令待ち*/
	/* ラインタスク側は、Vendに対するAckは完了しているが、それ以降のいずれかのステータス*/
	/* (ホストのポーリング周期によって異なる)			*/
	/* 方針*/
	/* ポーリングモード、インタラプト1 */
	/* ステータスリクエスト受信をトリガに,Stacked,Idling,Accepting,Escrow*/
	/* と遷移させればいいので、ここではフラグのセットだけ	*/
	/* インタラプト2 */
	/* Escrow,Vend以外は送信が必修ではない*/
	/* すでに、Escrowを送信できるまで、処理が到達しているので、*/
	/* ここでEnqリスエストを送信して、ステータスに対してEscrowを送信できるようにする*/
	/* ここでEscrowに遷移させないと、ラインタスクから遷移させる手段がない事と*/
	/* (仕様上、Vend移行は、ステータスリクエストが来ない場合もありえる為)*/


//	if (s_id003_status_wait_flag == 0)
	if(1)
	{
	/* ここでは、シーケンスの遷移はさせない(インタラプト2以外)	*/
	/* フラグのみセットして、ポール受信シーケンス遷移させる		*/

		switch (ex_cline_status_tbl.line_task_mode)
		{
		/*---これも必要---*/
		/* 搬送失敗メッセージ、エラーメッセージ */
		case ID003_MODE_STACK:
		case ID003_MODE_VEND:
		case ID003_MODE_WAIT_VEND_ACK:

		/* 搬送失敗メッセージ、エラーメッセージ */
		/* 識別成功、失敗のメッセージ*/
		case ID003_MODE_STACKED:
		case ID003_MODE_ENABLE_WAIT_POLL:
		case ID003_MODE_ENABLE:

			if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
			{

				/* 識別完了、識別OKなので次からEscrow */
				s_id003_2nd_note_code = cline_msg.arg2;
				if(ex_cline_status_tbl.comm_mode == 2)
				{
					_id003_set_escrow_or_reject_2nd(1);
				}
				else
				{
					s_id003_2nd_note = 2;/* 2枚目の識別OK Escrowへは未遷移*/						
				}
			}
			else if (cline_msg.arg1 == TMSG_SUB_REJECT)
			{
				#if 1 //2023-09-13
				if(cline_msg.arg2 == REJECT_CODE_STACKER_HOME)
				{
				/*収納関係のエラーはVendに直結するので、そのまま取り込んだ紙幣の返却処理へ*/
					_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, REJECT_CODE_RETURN, 0, 0, 0); //2023-09-12 テスト的に入れる
				}
				else
				{
					/*それ以外はまたせる*/
					/* 識別完了、識別NGなので次からReReject */
					s_id003_2nd_note_code = cline_msg.arg2;
					if(ex_cline_status_tbl.comm_mode == 2)
					{					
						_id003_set_escrow_or_reject_2nd(2);
					}
					else
					{
						s_id003_2nd_note = 5;/* 2枚目の識別NG Rejectへは未遷移*/
					}
				}

				#else

				/* 識別完了、識別NGなので次からReReject */
				s_id003_2nd_note_code = cline_msg.arg2;
				if(ex_cline_status_tbl.comm_mode == 2)
				{					
					_id003_set_escrow_or_reject_2nd(2);
				}
				else
				{
					s_id003_2nd_note = 5;/* 2枚目の識別NG Rejectへは未遷移*/
				}


				#endif
			}
			else if (cline_msg.arg1 == TMSG_SUB_ALARM)
			{
			#if 1//2023-09-13
				_set_id003_alarm(ID003_MODE_ERROR, (u16)cline_msg.arg2);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
			#else	
			/* いきなりエラーにするのは問題*/
				s_id003_2nd_note = 10;	/* 2枚目処理中にError Errorへは未遷移*/
				s_id003_2nd_note_code = cline_msg.arg2;
			#endif
			}
			break;

		case ID003_MODE_DISABLE:
		case ID003_MODE_DISABLE_REJECT_2ND:

			if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT, REJECT_CODE_RETURN); //Disableステータス
				//rejectで書いてる write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, REJECT_CODE_RETURN, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);

			}
			else if (cline_msg.arg1 == TMSG_SUB_REJECT)
			{
				_set_id003_reject(ID003_MODE_DISABLE_REJECT, REJECT_CODE_RETURN); //Disableステータス
				//rejectで書いてる write_status_table();
				_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, REJECT_CODE_RETURN, 0, 0, 0);

				/* change status */
				_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);

			}
			break;

		default:					/* other */
			break;

		}


	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_set_id003_reject(ID003_MODE_REJECT, REJECT_CODE_INHIBIT);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{
			_set_id003_reject(ID003_MODE_REJECT, (u16)cline_msg.arg2);

			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{
			s_id003_status_wait_next_mode = ID003_MODE_ERROR;
			s_id003_status_wait_error = cline_msg.arg2;
		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 30);
		}
	}
}

void _id003_set_escrow_or_reject_2nd(u8 type)
{
	if(type == 1)
	{
		/* bill */
		ex_cline_status_tbl.escrow_code = s_id003_2nd_note_code;
		ex_cline_status_tbl.line_task_mode = ID003_MODE_ESCROW;
	}
	else if(type == 2)
	{
		/* ticket */
		ex_cline_status_tbl.reject_code = s_id003_2nd_note_code;
		_set_id003_reject(ID003_MODE_REJECT, (u16)ex_cline_status_tbl.reject_code);		
		_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_REJECT_REQ, ex_cline_status_tbl.reject_code, 0, 0, 0);
	}

	s_id003_2nd_note = 0;	/* Escrowへ処理完了 Flag clear */
	s_id003_2nd_note_code = 0;
	write_status_table();

	_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ESCROW_STATUS_REQ1, 300, 0, 0);

	/* change status */
	_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);

}

typedef struct _ID003_STS_TABLE
{
    u8 mode;
    void (* sts_req_func)(void);
    void (* sts_trans_func)(void);
    u8 sts_003;
    u8 wait_poll_mode[3];
	u8 force_enq;
} ID003_STS_TABLE;


ID003_STS_TABLE id003_sts_tbl[] =
{
/* 配列の並びはenum _ID003_MODEと一致させること。 */
	/* mode                                  Ststus request関数                    状態遷移時処理関数       003_Status                      wait */
	{ID003_MODE_POWERUP,                     NULL,	NULL,	ID003_STS_POWERUP,              {1, 1, 1}, 0}, 
	{ID003_MODE_POWERUP_BILLIN_AT,           NULL,	NULL,	ID003_STS_POWERUP_BILLIN_AT,    {1, 1, 1}, 0},
	{ID003_MODE_POWERUP_BILLIN_SK,           NULL,	NULL,	ID003_STS_POWERUP_BILLIN_SK,    {1, 1, 1}, 0},
	{ID003_MODE_POWERUP_ERROR,               NULL,	NULL,	ID003_STS_STACKER_FULL,         {1, 1, 1}, 0},
	{ID003_MODE_POWERUP_INITIAL,             NULL,	NULL,	ID003_STS_INITIALIZE,           {0, 1, 1}, 0},
	{ID003_MODE_POWERUP_INITIAL_STACK,       NULL,	NULL,	ID003_STS_INITIALIZE,           {1, 1, 1}, 0},
	{ID003_MODE_POWERUP_INITIAL_VEND,        NULL,	NULL,	ID003_STS_VEND_VALID,           {1, 1, 1}, 0},
	{ID003_MODE_POWERUP_INITIAL_VEND_ACK,    NULL,	NULL,	ID003_STS_VEND_VALID,           {1, 1, 1}, 0},
	{ID003_MODE_POWERUP_INITIAL_REJECT,      NULL,	NULL,	ID003_STS_PAUSE,      		    {1, 1, 1}, 0},
	{ID003_MODE_POWERUP_INITIAL_PAUSE,       NULL,	NULL,	ID003_STS_PAUSE,  	            {1, 1, 1}, 0},
	{ID003_MODE_INITIAL,                     NULL,	NULL,	ID003_STS_INITIALIZE,           {1, 1, 1}, 0},
	{ID003_MODE_INITIAL_STACK,               NULL,	NULL,	ID003_STS_INITIALIZE,           {1, 1, 1}, 0},
	{ID003_MODE_INITIAL_REJECT,              NULL,	NULL,	ID003_STS_PAUSE,                {1, 1, 1}, 0},
	{ID003_MODE_INITIAL_PAUSE,               NULL,	NULL,	ID003_STS_PAUSE,                {1, 1, 1}, 0},
	{ID003_MODE_AUTO_RECOVERY,               NULL,	NULL,	ID003_STS_INITIALIZE,           {1, 1, 1}, 0},
	{ID003_MODE_AUTO_RECOVERY_STACK,         NULL,	NULL,	ID003_STS_INITIALIZE,           {1, 1, 1}, 0},
	{ID003_MODE_AUTO_RECOVERY_REJECT,        NULL,	NULL,	ID003_STS_PAUSE,     		    {1, 1, 1}, 0},
	{ID003_MODE_AUTO_RECOVERY_PAUSE,         NULL,	NULL,	ID003_STS_PAUSE,                {1, 1, 1}, 0},
	{ID003_MODE_DISABLE,                     NULL,	NULL,	ID003_STS_DISABLE,              {1, 1, 0}, 0},
	{ID003_MODE_DISABLE_REJECT,              NULL,	NULL,	ID003_STS_DISABLE,              {0, 1, 0}, 0},
	{ID003_MODE_ENABLE_WAIT_POLL,            NULL,	NULL,	ID003_STS_ENABLE,              {1, 1, 0}, 0},//new 
	{ID003_MODE_ENABLE,                      NULL,	NULL,	ID003_STS_ENABLE,               {1, 1, 0}, 0},
	{ID003_MODE_ENABLE_REJECT,               NULL,	NULL,	ID003_STS_ENABLE,               {0, 1, 0}, 0},
	{ID003_MODE_ACCEPT,                      NULL,  NULL,	ID003_STS_ACCEPTING,            {1, 1, 0}, 0},
//	{ID003_MODE_ACCEPT_POLL_RECEIVED,        NULL,	NULL,	ID003_STS_ACCEPTING,              {1, 1, 0}, 0},//new 
	{ID003_MODE_ACCEPT_WAIT_POLL_FOR_REJECT, NULL,	NULL,	ID003_STS_ACCEPTING,              {1, 1, 0}, 0},//new 
	{ID003_MODE_ESCROW,                      NULL,	NULL,	ID003_STS_ESCROW,               {1, 1, 1}, 0},
	{ID003_MODE_ESCROW_WAIT_CMD,           	NULL,	NULL,	ID003_STS_ESCROW,              {1, 1, 1}, 0},//new
	{ID003_MODE_HOLD1,						NULL,	NULL,	ID003_STS_HOLDING,              {1, 1, 0}, 0},//new
	{ID003_MODE_HOLD2,						NULL,	NULL,	ID003_STS_HOLDING,              {1, 1, 0}, 0},//new 
	{ID003_MODE_STACK,                       NULL,	NULL,	ID003_STS_STACKING,             {1, 1, 0}, 0},
	{ID003_MODE_PAUSE,                       NULL,	NULL,	ID003_STS_PAUSE,                {0, 1, 1}, 0},
	{ID003_MODE_VEND,                        NULL,	NULL,	ID003_STS_VEND_VALID,           {1, 1, 1}, 0},
	{ID003_MODE_VEND_FULL,                   NULL,	NULL,	ID003_STS_VEND_VALID,           {1, 1, 1}, 0}, //uba500と異なる
	{ID003_MODE_WAIT_VEND_ACK,               NULL,	NULL,	ID003_STS_VEND_VALID,           {1, 1, 1}, 0},
	{ID003_MODE_WAIT_VEND_ACK_FULL,          NULL,	NULL,	ID003_STS_VEND_VALID,           {1, 1, 1}, 0},
	{ID003_MODE_STACKED,                     NULL,	NULL,	ID003_STS_STACKED,              {1, 1, 1}, 0},
	{ID003_MODE_STACKED_FULL,                NULL,	NULL,	ID003_STS_STACKED,              {1, 1, 1}, 0},//new 
//	{ID003_MODE_STACK_FINISH,                NULL,	NULL,	ID003_STS_STACKED,              {1, 1, 1} },
	{ID003_MODE_REJECT_WAIT_ACCEPT_RSP,      NULL, NULL,	ID003_STS_REJECTING,            {1, 1, 0}, 0}, /* 変えるかも、現状Rejectだが、このステータスの時は、返却動作前の取り込み中である為*/
	{ID003_MODE_REJECT,                      NULL,	NULL,	ID003_STS_REJECTING,            {1, 1, 0}, 0},
	{ID003_MODE_REJECT_WAIT_POLL,            NULL,	NULL,	ID003_STS_REJECTING,              {1, 1, 0}, 0},//new 
	{ID003_MODE_REJECT_WAIT_NOTE_REMOVED,    NULL,	NULL,	ID003_STS_REJECTING,              {1, 1, 0}, 0},//new 
	{ID003_MODE_RETURN,                      NULL,	NULL,	ID003_STS_RETURNING,            {1, 1, 0}, 0},
	{ID003_MODE_LOG_ACCESS,                  NULL,	NULL,	ID003_STS_EXT_LOG_ACCESS_READY, {0, 1, 0}, 0},
//	{ID003_MODE_DISABLE_BOOKMARK,            NULL,	NULL,	ID003_STS_DISABLE,              {1, 1, 0} },//new 
//	{ID003_MODE_DISABLE_REJECT_BOOKMARK,     NULL,	NULL,	ID003_STS_DISABLE,              {1, 1, 0} },//new 
//	{ID003_MODE_ENABLE_BOOKMARK,             NULL,	NULL,	ID003_STS_ENABLE,              {1, 1, 0} },//new 
//	{ID003_MODE_ENABLE_REJECT_BOOKMARK,      NULL,	NULL,	ID003_STS_ENABLE,              {1, 1, 0} },//new 
//	{ID003_MODE_ACCEPT_BOOKMARK,             NULL,	NULL,	ID003_STS_ACCEPTING,            {1, 1, 0} },
//	{ID003_MODE_STACK_BOOKMARK,              NULL,	NULL,	ID003_STS_ACCEPTING,            {1, 1, 0} },
//	{ID003_MODE_PAUSE_BOOKMARK,              NULL,	NULL,	ID003_STS_PAUSE,                {0, 1, 0} },
//	{ID003_MODE_STACKED_BOOKMARK,            NULL,	NULL,	ID003_STS_STACKED,              {1, 1, 0} },
//	{ID003_MODE_WAIT_POLL_BEFOR_FULL,        NULL,	NULL,	ID003_STS_POWERUP,              {1, 1, 1}, 0},//new //not use
//	{ID003_MODE_CLEANING,                    NULL,	NULL,	ID003_STS_POWERUP,              {1, 1, 1}, 0},//new //not use
	{ID003_MODE_ERROR,                       NULL,	NULL,	ID003_STS_FAILURE,              {1, 1, 1}, 1},
//	{ID003_MODE_NEARLY_FULL,       		     NULL,	NULL,	ID003_STS_NEARLY_FULL,          {1, 1, 1} },//not use
	{ID003_MODE_WAIT_PAUSE,                  NULL,	NULL,	ID003_STS_STACKING,             {1, 1, 0}, 0},//new //インタラプト2専用のようだ、若干怪しい
	{ID003_MODE_SIGNATURE_BUSY,              NULL,	NULL,	ID003_STS_SIGNATURE_BUSY,       {0, 1, 0}, 0},
	{ID003_MODE_SIGNATURE_END,               NULL,	NULL,	ID003_STS_SIGNATURE_END,			{1, 1, 1}, 0},//new 
	{ID003_MODE_SHA1_HASH_BUSY,              NULL,	NULL,	ID003_STS_SHA1_HASH_BUSY_OR_END,	{1, 1, 1}, 0},//new 
	{ID003_MODE_SHA1_HASH_END,               NULL,	NULL,	ID003_STS_SHA1_HASH_BUSY_OR_END,	{1, 1, 1}, 0},//new 
	{ID003_MODE_POWERUP_SIGNATURE_BUSY,      NULL,	NULL,	ID003_STS_SIGNATURE_BUSY,			{1, 1, 1}, 0},//new 
	{ID003_MODE_POWERUP_SIGNATURE_END,       NULL,	NULL,	ID003_STS_SIGNATURE_END,			{1, 1, 1}, 0},//new 
	{ID003_MODE_POWERUP_SHA1_HASH_BUSY,      NULL,	NULL,	ID003_STS_SHA1_HASH_BUSY_OR_END,{1, 1, 1}, 0},//new 
	{ID003_MODE_POWERUP_SHA1_HASH_END,       NULL,	NULL,	ID003_STS_SHA1_HASH_BUSY_OR_END,{1, 1, 1}, 0},//new 
	{ID003_MODE_SYSTEM_ERROR,                NULL,	NULL,	ID003_STS_FAILURE,              {1, 1, 1}, 0}, 
	{ID003_MODE_DISABLE_REJECT_2ND,          NULL,	NULL,	ID003_STS_DISABLE,              {0, 1, 0}, 0},
	{ID003_MODE_ACCEPT_WAIT_POLL_FOR_ESCROW, NULL,	NULL,	ID003_STS_ACCEPTING,              {1, 1, 0}, 0},//new 2024-03-05

#if defined(UBA_RTQ)
	{ID003_MODE_POWERUP_INITIAL_PAYVALID,    NULL,  NULL,   ID003_STS_PAY_VALID,            {1, 1, 1}, 0},
	{ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK,NULL,  NULL,   ID003_STS_PAY_VALID,            {1, 1, 0}, 0},

	{ID003_MODE_COLLECT,                     NULL,  NULL,   ID003_STS_COLLECTING,           {0, 1, 0}, 0},
	{ID003_MODE_COLLECTED_WAIT_POLL,         NULL,  NULL,   ID003_STS_COLLECTED,            {1, 1, 1}, 0},
	{ID003_MODE_RETURN_TO_BOX,               NULL,  NULL,   ID003_STS_RETURN_TO_BOX,        {0, 1, 0}, 0},

	{ID003_MODE_PAYOUT,                      NULL,  NULL,   ID003_STS_PAYING,               {1, 1, 0}, 0},
	{ID003_MODE_PAYOUT_RETURN_NOTE,          NULL,  NULL,   ID003_STS_RETURN_PAY_OUT,       {1, 1, 0}, 0},
	{ID003_MODE_PAYOUT_COLLECTED_WAIT_POLL,  NULL,  NULL,   ID003_STS_COLLECTED,            {1, 1, 0}, 0},
	{ID003_MODE_PAYSTAY,                     NULL,  NULL,   ID003_STS_PAY_STAY,             {1, 1, 0}, 0},
	{ID003_MODE_PAYSTAY_WAIT_POLL,           NULL,  NULL,   ID003_STS_PAY_STAY,             {1, 1, 1}, 0},
	{ID003_MODE_PAYVALID,                    NULL,  NULL,   ID003_STS_PAY_VALID,            {1, 1, 1}, 0},
	{ID003_MODE_PAYVALID_ERROR,              NULL,  NULL,   ID003_STS_PAY_VALID,            {1, 1, 1}, 0},
	{ID003_MODE_WAIT_PAYVALID_ACK,           NULL,  NULL,   ID003_STS_PAY_VALID,            {1, 1, 0}, 0},
	{ID003_MODE_WAIT_PAYVALID_ACK_ERROR,     NULL,  NULL,   ID003_STS_PAY_VALID,            {1, 1, 0}, 0},
	{ID003_MODE_AFTER_PAYVALID_ACK_ENABLE,   NULL,  NULL, 	ID003_STS_ENABLE,               {1, 1, 0}, 0},
	{ID003_MODE_AFTER_PAYVALID_ACK_DISABLE,  NULL,  NULL,	ID003_STS_DISABLE,              {1, 1, 0}, 0},
	{ID003_MODE_AFTER_PAYVALID_ACK_PAYOUT,	 NULL,  NULL,   ID003_STS_PAYING,               {1, 1, 0}, 0},
//	{ID003_MODE_RC_ERROR,					 NULL,	NULL,	ID003_MODE_RC_ERROR,            {1, 1, 0}, 0},
	{ID003_MODE_DIAGNOSTIC,					 NULL,	NULL,	ID003_STS_EXT_DIAG_READY,		{1, 1, 0}, 0},
	{ID003_MODE_RETURN_ERROR,                NULL,	NULL,	ID003_STS_RETURN_ERROR,			{0, 1, 0}, 0},
#endif // UBA_RTQ

	{ID003_MODE_END,                         NULL,	NULL,	NULL,                           {0, 0, 0}, 0},

//not use    {ID003_MODE_HOLD,		NULL,	NULL,	ID003_STS_HOLDING,              {0, 1, 0} },
//not use    {ID003_MODE_SHA1_BUSY,	NULL,	NULL,	ID003_STS_SHA1,                 {0, 1, 0} },

};

//ENQ送信必要か判断する関数
void _id003_intr_mode_sub_uba(u16 mode, u8 wait_flag) //not use wait_flag
{
    u8 array_num = 0;
	u8 ii=0;
	u8 send=0;

    if( (ex_cline_status_tbl.comm_mode == 0x0001) ||
        (ex_cline_status_tbl.comm_mode == 0x0002) )
    {
        while( ID003_MODE_END != id003_sts_tbl[array_num].mode )
        {
            if( id003_sts_tbl[array_num].mode == mode )
            {
                break;
            }
            array_num++;
        }

        if( ID003_MODE_END != id003_sts_tbl[array_num].mode )
        {
            /* 最後にENQを送信した003でのSTATUSと今の003でのSTATUSが異なる場合 */
        //    if( (1 == id003_sts_tbl[array_num].wait_poll_mode[ex_cline_status_tbl.comm_mode]) &&
        //        (s_id003_enq_last_status != id003_sts_tbl[array_num].sts_003)                  )
			if( 1 == id003_sts_tbl[array_num].wait_poll_mode[ex_cline_status_tbl.comm_mode] )
			{
				if(s_id003_enq_last_status != id003_sts_tbl[array_num].sts_003)
				{
				//前回ENQ送信時と異なる
					send=1; //need ENQ
				}
			#if 1
			//基本的にこれは、送信処理で呼び出されるわけではないので、_id003_tx_buffを参照しても、意味ない
			//_id003_tx_buffには、今このタイミングでは保存されていない。
				else if( id003_sts_tbl[array_num].force_enq == 1 )
				{
					send=1; //need ENQ
				}
			#else	
				else if(	
				(_id003_tx_buff_back_uba[1] != _id003_tx_buff.length)
				||
				(_id003_tx_buff_back_uba[2] != _id003_tx_buff.cmd)
				)
				{
				//最後にホストに通知したステータスと異なる
					send=1;
				}
				else
				{
				//最後にホストに通知したステータスのデータ部分が同じか確認する
					if(_id003_tx_buff.length > 5)
					{
						/* 前回送信したLenghtサイズ分比較 Data部*/ //_id003_tx_buff
						for(ii = 0; ii < (_id003_tx_buff_back_uba[1] - 5); ii++)
						{
							if(_id003_tx_buff_back_uba[ii + 3] != _id003_tx_buff.data[ii])
							{
							//異なる
								send=1;
							}
						}
					}
				}
			#endif
				if(send==1)
				{
					/* ENQ判断の関数自体、UBA500と異なり必要な時に呼び出す様になっているので*/
					/* ここの処理のs_id003_enq_last_statusの代入がなくても、問題なく動作するが*/
					/* 毎回呼び出す様に変更した場合同じステータスで、ENQを送信し続ける事になるので*/
					/* 保護処理として残しておく */
					/* Interrupt mode2は同じStatusが連続することがある。 */
					/* (Escrow,PayValidなど)                             */
					if( (ex_cline_status_tbl.comm_mode == 0x0002)                &&
						(id003_sts_tbl[array_num].sts_003 != ID003_STS_INITIALIZE ) )
					{
						s_id003_enq_last_status = 0;
					}
					else
					{
						s_id003_enq_last_status = id003_sts_tbl[array_num].sts_003;
					}

					/* [Interrupt Mode 1/2] Set enq resend flag */
					s_id003_enq_resend = 1;
					if( wait_flag == 1 )
					{
						/* ENQ送るため、一度タイマ起動させる。 */
						_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ENQ_SEND, WAIT_TIME_SEND_ENQ, 0, 0);
					}
					else
					{
						//UBA500に合わせる
						//処理ミスで、ENQとコマンドレスポンスを同時に設定して、上書きされる可能性があるのでここでのENQ送信を廃止
						//タイムアウトで、ENQ送信処理になるのでそれで対応する
						//_id003_send_host_enq();

						/* ENQ送るため、一度タイマ起動させる。 */
						_cline_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_ENQ_SEND, WAIT_TIME_SEND_ENQ, 0, 0);
					}
				}
            }
        }
    }
}

void save_send_status_003_uba(void)
{
	/* CRC以外をコピー*/
	memcpy( &_id003_tx_buff_back_uba[0], &_id003_tx_buff, (_id003_tx_buff.length - 2) );

}


bool _id003_check_opt_func(void)
{
	if ((ex_cline_status_tbl.option & ID003_OPTION_ENTRANCE_SENSOR) == ID003_OPTION_ENTRANCE_SENSOR)
	{
		return true;
	}
	return false;
}

#if !defined(UBA_RTQ)
void power_recover_003(u32 bill, u8 from) //2024-03-28
{
	/* from 2 電源ON後の紙幣探し後、from 3 Reset待ち状態で紙幣状態変化時、form 4 Reset待ち状態で紙幣状態変化時+wait mode*/

	u8 chnge_status=0;

	if (bill == BILL_IN_ACCEPTOR)
	{
	/* Powerup with bill in acceptor */
		s_id003_powerup_stat = POWERUP_STAT_REJECT;

		if(ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_AT)
		{
			chnge_status = 1;
		}
		if(from == 4)
		{
		/* wait mode */
			s_id003_status_wait_next_mode = ID003_MODE_POWERUP_BILLIN_AT;
			s_id003_status_wait_error = 0;
		}
		else
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
		}
	}
	else if (bill == BILL_IN_STACKER)
	{
	/* Powerup with bill in stacker */
		//書き方が異なるだけでUBA500と条件ほぼ同じ
		if ((ex_recovery_info.step >= RECOVERY_STEP_ESCORW)
			&& (ex_recovery_info.step <= RECOVERY_STEP_VEND))
		{
			s_id003_powerup_stat = POWERUP_STAT_RECOVER_FEED_STACK;
		}
		else
		{
			s_id003_powerup_stat = POWERUP_STAT_FEED_STACK;
		}
		if(ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_SK)
		{
			chnge_status = 1;
		}
		if(from == 4)
		{
		/* wait mode */
			s_id003_status_wait_next_mode = ID003_MODE_POWERUP_BILLIN_SK;
			s_id003_status_wait_error = 0;
		}
		else
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
		}
	}
	else
	{
	/* Non */
		if( from == 2 )
		{
		/* 電源ON後の紙幣探し後 */
		/* Powerup */
		/* GLI対応 */
			s_id003_powerup_stat = POWERUP_STAT_RECOVER_SEARCH_NON;
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_ERROR;
		#if !defined(UBA_RTQ) //2023-12-04
			ex_error_003_gli = 1;
			_cline_send_msg(ID_MAIN_MBX, TMSG_CLINE_SET_STATUS, TMSG_SUB_ALARM, ALARM_CODE_FEED_LOST_BILL, 0, 0);
		#endif
		}
		else
		{
			if( ex_recovery_info.step == RECOVERY_STEP_VEND
			||
			(ex_recovery_info.step == RECOVERY_STEP_ICB_ACCEPT) /* 2024-04-02 */
			||
			#if defined(UBAPRO_LD)
			(ex_recovery_info.step == RECOVERY_STEP_STACKING)
			#else
			( (ex_recovery_info.step == RECOVERY_STEP_STACKING) && ( 0 == SENSOR_STACKER_HOME ) )
			#endif
			|| (ex_recovery_info.step == RECOVERY_STEP_STACKING_BILL_IN_BOX)
			)
			{
				/* Powerup with bill in stacker */
				s_id003_powerup_stat = POWERUP_STAT_RECOVER_STACK;
				if(ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_SK)
				{
					chnge_status = 1;
				}

				if(from == 4)
				{
				/* wait mode */
					s_id003_status_wait_next_mode = ID003_MODE_POWERUP_BILLIN_SK;
					s_id003_status_wait_error = 0;
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
				}
			}
			else
			{
				/* ここで紙幣を探すと、次の処理でエラーにするか含めて判断する必要がでる*/
				/* UBA500もEscrow位置から紙幣が無くなった場合も特に処理していないので、同様にする*/	                        
				if(ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP)
				{
					chnge_status = 1;
				}

				/* Powerup */
				s_id003_powerup_stat = POWERUP_STAT_NORMAL;

				if(from == 4)
				{
				/* wait mode */
					s_id003_status_wait_next_mode = ID003_MODE_POWERUP;
					s_id003_status_wait_error = 0;
				}
				else
				{
					ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP;
				}
			}
		}
	}
	if( chnge_status == 1 && from == 3 )
	{
	/* Reset待ち状態で紙幣状態変化時 + 003のステータスが変わったので、インタラプトモードの為に関数コール */
		ex_cline_status_tbl.error_code = 0;
		write_status_table();
		/* change status */
		_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
	}
}
#endif


void _id003_encryption_set_number_cmd_proc(void) //ok
{
	/* POWERUP,ENABLE,DISABLE,INITIALIZEｽﾃｰﾀｽ時のみ有効 */
	if( ((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
	&& ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_AT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_SK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT))
	)
	{
		if( (_id003_rx_buff.data[0] == ID003_CMD_DLE_ENCNUM_SET) &&
			(_id003_rx_buff.length == (0x03 + ID003_ADD_04) ))
		{
			id003_enc_set_number(_id003_rx_buff.data[1]); //ok
			_id003_send_host_ack();
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}

void _id003_encryption_key_cmd_proc(void) //ok
{
	/* POWERUP,ENABLE,DISABLE,INITIALIZEｽﾃｰﾀｽ時のみ有効 */
	if( ((ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION)
	&& ((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_AT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_BILLIN_SK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_ERROR)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_DISABLE_REJECT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_ENABLE_REJECT))
	)
	{
		if( (_id003_rx_buff.data[0] == ID003_CMD_DLE_ENCKEY_SET)
		&& (_id003_rx_buff.length == (0x0A + ID003_ADD_04) ) )
		{
			id003_enc_make_key(&_id003_rx_buff.data[1]);
			_id003_send_host_ack();
		}
		else
		{
			_id003_send_host_invalid();
		}
	}
	else
	{
		_id003_send_host_invalid();
	}
}

void update_random_seed(unsigned int offset) //ok
{
	static u32 counter = 0;

	/* hopefully add some randomness */
	random_seed += offset;

	/* prevent zero */
	random_seed |= random_seed == 0;

	/* taken from https://www.jstatsoft.org/article/download/v008i14/916  (Xorshift RNGs - George Marsaglia)*/
	random_seed ^= (random_seed << 13);
	random_seed ^= (random_seed >> 17);
	random_seed ^= (random_seed << 5);

	/* scrambling (Weyl sequence)*/
	counter += 362437;

	/* Add data(check sum of log data) */
	counter += jdl_logdat_chksum;

	/* renewal random seed */
	random_seed += counter;
}

static void create_secret_number(void) //ok
{
	/* extract some bytes */
	Random_Secret_no[0] = ( random_seed >>  3 ) & 0xff;
	Random_Secret_no[1] = ( random_seed >> 11 ) & 0xff;
	Random_Secret_no[2] = ( random_seed >> 19 ) & 0xff;
}


void _secret_number_003(void) //ok resetコマンド受信時の状態によって 
{
	//resetコマンド受信時の状態によって、シークレット番号を変更するか判断
	if((ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_STACK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_VEND_ACK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_REJECT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAUSE)
#if defined(UBA_RTQ)	
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAYVALID)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_POWERUP_INITIAL_PAYVALID_ACK)
#endif
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_STACK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_REJECT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_INITIAL_PAUSE)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_STACK)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_REJECT)
	|| (ex_cline_status_tbl.line_task_mode == ID003_MODE_AUTO_RECOVERY_PAUSE))
	{
		ex_main_secret_number_change_flag=0;		
	}
	else
	{
		ex_main_secret_number_change_flag=1;
	}
}


#if defined(UBA_RTQ)

void power_recover_003_rtq(u32 bill, u8 from) //2025-03-26 _id003_status_info_mode_powerup, SSとRTQで見にくいので分ける
{
	/* from 2 電源ON後の紙幣探し後、from 3 Reset待ち状態で紙幣状態変化時、form 4 Reset待ち状態で紙幣状態変化時+wait mode*/

	u8 chnge_status=0;

	if (bill == BILL_IN_ACCEPTOR)
	{
	/* Powerup with bill in acceptor */
		memset((u8 *)&Smrtdat_fram_bk_power, 0, sizeof(Smrtdat_fram_bk_power)); //状態変化により リカバリをクリア
		s_id003_powerup_stat = POWERUP_STAT_REJECT;

		if(ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_AT)
		{
			chnge_status = 1;
		}
		if(from == 4)
		{
		/* wait mode */
			s_id003_status_wait_next_mode = ID003_MODE_POWERUP_BILLIN_AT;
			s_id003_status_wait_error = 0;
		}
		else
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
		}
	}
	else if (bill == BILL_IN_STACKER)
	{
	/* Powerup with bill in stacker */
		/* 現在既にPowerup with bill in stackerの場合は何もしない */
		if( ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_SK )
		{
			chnge_status = 1;
			switch( ex_recovery_info.step )
			{
				case RECOVERY_STEP_ACCEPT:
				case RECOVERY_STEP_ESCORW:
				case RECOVERY_STEP_STACKING:
				case RECOVERY_STEP_APB_IN:
				case RECOVERY_STEP_APB_OUT:
				case RECOVERY_STEP_VEND:
				case RECOVERY_STEP_STACK_TRANSPORT:
				case RECOVERY_STEP_STACK_DRUM:
					/* Vend Valid送信する。 */
					s_id003_powerup_stat = POWERUP_STAT_RECOVER_FEED_STACK;
					break;
				case RECOVERY_STEP_NON:
					/* Vend Valid送信しない、かつCountもしない */
					s_id003_powerup_stat = POWERUP_STAT_RECOVER_NO_COUNT;
					break;
				case RECOVERY_STEP_EMRGENCY_TRANSPORT:
				case RECOVERY_STEP_SWITCHBACK_TRANSPORT:
				case RECOVERY_STEP_PAYOUT_DRUM:
				case RECOVERY_STEP_PAYOUT_TRANSPORT:
				case RECOVERY_STEP_PAYOUT_POS1:
				case RECOVERY_STEP_PAYOUT_ESCROW:
				case RECOVERY_STEP_PAYOUT_RS_POS7:
				case RECOVERY_STEP_PAYOUT_RS_ESCROW:
				case RECOVERY_STEP_COLLECT_DRUM:
				case RECOVERY_STEP_COLLECT_TRANSPORT:
				case RECOVERY_STEP_COLLECT_STACKING:
					/* Vend Valid送信しない。 */
					s_id003_powerup_stat = POWERUP_STAT_FEED_STACK;
					break;
				case RECOVERY_STEP_PAYOUT_VALID:
					/* Pay Valid送信する */
					/* (出金した紙幣とは別の紙幣として扱う) */
					s_id003_powerup_stat = POWERUP_STAT_RECOVER_PAYOUT;
					break;
			}
			ex_cline_status_tbl.error_code = 0;
			if(from == 4)
			{
			/* wait mode */
				s_id003_status_wait_next_mode = ID003_MODE_POWERUP_BILLIN_SK;
				s_id003_status_wait_error = 0;
			}
			else
			{	
				//fid003_change_mode( ID003_MODE_POWERUP_BILLIN_SK );
				ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
			}
		}
	}

	/* 出金確定位置まで搬送していて入口センサのみONの場合、 */
	/* その紙幣は出金した紙幣として扱う                     */
	//else if((line_msg.arg3 == BILL_IN_ENTRANCE)
	else if(( bill == BILL_IN_ENTRANCE)  //UBA500は line_msg.arg3 になっているが、使用するなら line_msg.arg2 が正解
	&&		((ex_recovery_info.step == RECOVERY_STEP_PAYOUT_ESCROW) || (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW)
	||		 (ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID)))
	{
		if( ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_AT )
		{
			chnge_status = 1;
			/* Powerup with bill in acceptor */
			s_id003_powerup_stat = POWERUP_STAT_RECOVER_PAYOUT;

			if(from == 4)
			{
			/* wait mode */
				s_id003_status_wait_next_mode = ID003_MODE_POWERUP_BILLIN_AT;
				s_id003_status_wait_error = 0;
			}
			else
			{	
				//fid003_change_mode( ID003_MODE_POWERUP_BILLIN_AT );
				ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
			}
		}
	}
	else
	{
	/* Non */
		memset((u8 *)&Smrtdat_fram_bk_power, 0, sizeof(Smrtdat_fram_bk_power)); //状態変化により リカバリをクリア

		switch( ex_recovery_info.step )
		{
			case RECOVERY_STEP_STACKING:
			case RECOVERY_STEP_VEND:
				if (ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_SK)
				{
					chnge_status = 1;

					ex_cline_status_tbl.error_code = 0;
					/* Powerup with bill in stacker */
					s_id003_powerup_stat = POWERUP_STAT_RECOVER_STACK;
					if(from == 4)
					{
					/* wait mode */
						s_id003_status_wait_next_mode = ID003_MODE_POWERUP_BILLIN_SK;
						s_id003_status_wait_error = 0;
					}
					else
					{			
						//fid003_change_mode( ID003_MODE_POWERUP_BILLIN_SK );
						ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_SK;
					}
				}
				break;
			case RECOVERY_STEP_PAYOUT_VALID:
			case RECOVERY_STEP_PAYOUT_ESCROW:
			case RECOVERY_STEP_PAYOUT_RS_ESCROW:
				if (ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP_BILLIN_AT)
				{
					chnge_status = 1;

					ex_cline_status_tbl.error_code = 0;
					/* Powerup with bill in acceptor */
					s_id003_powerup_stat = POWERUP_STAT_RECOVER_PAYOUT;
					if(from == 4)
					{
					/* wait mode */
						s_id003_status_wait_next_mode = ID003_MODE_POWERUP_BILLIN_AT;
						s_id003_status_wait_error = 0;
					}
					else
					{			
						//fid003_change_mode( ID003_MODE_POWERUP_BILLIN_AT );
						ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP_BILLIN_AT;
					}
				}
				break;
			case RECOVERY_STEP_ESCORW:
			case RECOVERY_STEP_APB_IN:
			case RECOVERY_STEP_APB_OUT:
			case RECOVERY_STEP_STACK_DRUM:
			case RECOVERY_STEP_EMRGENCY_TRANSPORT:
			case RECOVERY_STEP_SWITCHBACK_TRANSPORT:
			case RECOVERY_STEP_PAYOUT_DRUM:
			case RECOVERY_STEP_PAYOUT_TRANSPORT:
			case RECOVERY_STEP_PAYOUT_POS1:
			case RECOVERY_STEP_PAYOUT_RS_POS7:
			case RECOVERY_STEP_COLLECT_DRUM:
			case RECOVERY_STEP_COLLECT_TRANSPORT:
			case RECOVERY_STEP_COLLECT_STACKING:
			case RECOVERY_STEP_STACK_TRANSPORT:
			default:
				/* Powerup */
				if( ex_cline_status_tbl.line_task_mode != ID003_MODE_POWERUP)
				{
					chnge_status = 1;

					s_id003_powerup_stat = POWERUP_STAT_NORMAL;

					ex_cline_status_tbl.error_code = 0;
					if(from == 4)
					{
					/* wait mode */
						s_id003_status_wait_next_mode = ID003_MODE_POWERUP;
						s_id003_status_wait_error = 0;
					}
					else
					{			
						//	fid003_change_mode( ID003_MODE_POWERUP );
						ex_cline_status_tbl.line_task_mode = ID003_MODE_POWERUP;
					}
				}
				break;
		}

	}

	if( chnge_status == 1 && from == 3 )
	{
	/* Reset待ち状態で紙幣状態変化時 + 003のステータスが変わったので、インタラプトモードの為に関数コール */
		ex_cline_status_tbl.error_code = 0;
		write_status_table();
		/* change status */
		_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
	}
}

void id003_set_escrow_for_payout( u8 escrow_code )
{
//	id003_escrow_payout = escrow_code;
	ex_cline_status_tbl.id003_escrow_payout = escrow_code;
}

/*********************************************************************//**
 * @brief is check low battery
 * @param[in]	rc unit
 * @return true  : low battery
 * @return false : not low battery
 **********************************************************************/
static bool _is_id003_check_rc_low_battery(void)
{
//	battery_detect	0:無し		1:有り
//	battery_low		0:Low		1:正常

	if(ex_rc_status.sst22A.bit.battery_detect == 1)
	{
		if(ex_rc_status.sst21A.bit.battery_low == 0 || (ex_rc_status.sst21A.bit.battery_low != 0 && ex_cline_status_tbl.ex_rc_option_battery_low_detect == 1))
		{
			/* Low battery */
			return(true);
		}
		else
		{
			/* Not low battery */
			return(false);
		}
	}
	else
	{
		/* Not battery */
		return(false);
	}
}

static bool _is_id003_check_rc_removed_pdw(u8 unit)
{
//	u1_detect_pdw	0:履歴無し	1:履歴有り
//	u2_detect_pdw	0:履歴無し	1:履歴有り
//	battery_detect	0:無し		1:有り

	if(ex_rc_status.sst22A.bit.battery_detect == 1)
	{
		/* RC-Twin */
		if(unit == RC_TWIN)
		{
			if(ex_rc_status.sst21A.bit.u1_detect_pdw == 1)
			{
				/* Removed */
				return(true);
			}
			else
			{
				/* Not Removed */
				return(false);
			}
		}
		/* RC-Quad */
		else
		{
			if(ex_rc_status.sst22A.bit.u2_detect_pdw == 1)
			{
				/* Removed */
				return(true);
			}
			else
			{
				/* Not Removed */
				return(false);
			}
		}
	}
	else
	{
		/* Not battery */
		return(false);
	}
}

/*********************************************************************//**
 * @brief is check rs unit
 * @param[in]	rc unit
 * @return true  : low battery
 * @return false : not low battery
 **********************************************************************/
#if 0 // is_rc_rs_unit に統一
 static bool _is_rc_rs_unit(void)// is_rc_rs_unit に統一
{
}
#endif

void clearConcurrentPayout(void)
{
  ConcurrentPayoutFlag = 0;
  CurrentBoxNumberIndx = 0;
  memset(OperationDenomiCount, 0, sizeof(OperationDenomiCount));
  memset(OperationDenomiBoxNumber, 0, sizeof(OperationDenomiBoxNumber));
}

static void check_payout_zero_note(void)
{
  int tmpCnt = 0;
  int tmpSkip = 0;
  int boxCount;
  int dataIndx;
  UB  tmpDenomiCount[4];
  UB  tmpDenomiBoxNumber[4];

  if(ex_rc_status.sst1A.bit.quad) /* RQ */
  {
    boxCount = 4;
  }
  else  /* RT */
  {
    boxCount = 2;
  }

  memset(tmpDenomiCount, 0, sizeof(tmpDenomiCount));
  memset(tmpDenomiBoxNumber, 0, sizeof(tmpDenomiBoxNumber));

   for(dataIndx = 0; dataIndx < boxCount; dataIndx++)
   {
      if((OperationDenomiCount[dataIndx] == 0) && (OperationDenomiBoxNumber[dataIndx] != 0)) // e.g. 0001010200030104 => 010210400000000
      {
        /* skip */
        tmpSkip++;
        continue;
      }
      else
      {
        tmpDenomiCount[tmpCnt]        = OperationDenomiCount[dataIndx];
        tmpDenomiBoxNumber [tmpCnt]   = OperationDenomiBoxNumber[dataIndx];
        tmpCnt++;
      }
   }

  if(tmpSkip > 0)  /* There is Payout 0 note from some Box */
  {
      memcpy(OperationDenomiCount, tmpDenomiCount, 4);
      memcpy(OperationDenomiBoxNumber, tmpDenomiBoxNumber, 4);
  }

}

static u8 check_recieved_data(void)
{
    if( (check_setting_box_and_count()    != TRUE)
      ||(check_setting_denomi_recycler()  != TRUE)
      ||(check_status_recycler()          != TRUE)
      ||(check_payout_count_recycler()    != TRUE))
    {
      return(FALSE);
    }

    return(TRUE);
}

/* Check Box No. Setting and denomi. count */
static u8	check_setting_box_and_count(void)
{
  if(ex_rc_status.sst1A.bit.quad) /* RQ */
  {
    if( (OperationDenomiBoxNumber[0]  > 4 || OperationDenomiBoxNumber[1]  > 4 || OperationDenomiBoxNumber[2]  > 4 || OperationDenomiBoxNumber[3]  > 4) 
      ||(OperationDenomiCount[0] == 0  && OperationDenomiCount[1] == 0 && OperationDenomiCount[2] == 0 && OperationDenomiCount[2] == 0))
		{
			return(FALSE);
		}
  }
  else  /* RT */
  {
    if( (OperationDenomiBoxNumber[0]  > 2 || OperationDenomiBoxNumber[1]  > 2) 
      ||(OperationDenomiCount[0]== 0  && OperationDenomiCount[1] 	== 0 ))
		{
			return(FALSE);
		}
  }
    
    return(TRUE);
}

/* Check Denomination Setting : Setting by Currency(f) command */
static u8	check_setting_denomi_recycler(void)
{

		if( OperationDenomiCount[0] != 0 )
		{
			if(RecycleSettingInfo.DenomiInfo[ OperationDenomiBoxNumber[0] - 1 ].Data1 == 0
			&& RecycleSettingInfo.DenomiInfo[ OperationDenomiBoxNumber[0] - 1 ].Data2 == 0)
			{
				return(FALSE);
			}
		}
	
		if( OperationDenomiCount[1] != 0 )
		{
			if(RecycleSettingInfo.DenomiInfo[ OperationDenomiBoxNumber[1] - 1 ].Data1 == 0
			&& RecycleSettingInfo.DenomiInfo[ OperationDenomiBoxNumber[1] - 1 ].Data2 == 0)
			{
				return(FALSE);
			}
		}

  if(ex_rc_status.sst1A.bit.quad) /* RQ */
  {
    if( OperationDenomiCount[2] != 0 )
		{
			if(RecycleSettingInfo.DenomiInfo[ OperationDenomiBoxNumber[2] - 1 ].Data1 == 0
			&& RecycleSettingInfo.DenomiInfo[ OperationDenomiBoxNumber[2] - 1 ].Data2 == 0)
			{
				return(FALSE);
			}
		}

    if( OperationDenomiCount[3] != 0 )
		{
			if(RecycleSettingInfo.DenomiInfo[ OperationDenomiBoxNumber[3] - 1 ].Data1 == 0
			&& RecycleSettingInfo.DenomiInfo[ OperationDenomiBoxNumber[3] - 1 ].Data2 == 0)
			{
				return(FALSE);
			}
		}

  }

    return(TRUE);
}

/* Check Recycler Status : RC is empty or not? */
static u8	check_status_recycler(void)
{
  if(ex_rc_status.sst1A.bit.quad) /* RQ */
  {
    if( OperationDenomiCount[0] != 0 )
		{
			if( (ex_rc_status.sst31A.bit.u1_d1_empty && OperationDenomiBoxNumber[0] == 0x01)      /* RC1 is not empty */
			  ||(ex_rc_status.sst31A.bit.u1_d2_empty && OperationDenomiBoxNumber[0] == 0x02)      /* RC2 is not empty */
        ||(ex_rc_status.sst32A.bit.u2_d1_empty && OperationDenomiBoxNumber[0] == 0x03)      /* RC3 is not empty */
        ||(ex_rc_status.sst32A.bit.u2_d2_empty && OperationDenomiBoxNumber[0] == 0x04))     /* RC4 is not empty */
			{
				return(FALSE);
			}
		}
	
    if( OperationDenomiCount[1] != 0 )
		{
			if( (ex_rc_status.sst31A.bit.u1_d1_empty && OperationDenomiBoxNumber[1] == 0x01)      /* RC1 is not empty */
			  ||(ex_rc_status.sst31A.bit.u1_d2_empty && OperationDenomiBoxNumber[1] == 0x02)      /* RC2 is not empty */
        ||(ex_rc_status.sst32A.bit.u2_d1_empty && OperationDenomiBoxNumber[1] == 0x03)      /* RC3 is not empty */
        ||(ex_rc_status.sst32A.bit.u2_d2_empty && OperationDenomiBoxNumber[1] == 0x04))     /* RC4 is not empty */
			{
				return(FALSE);
			}
		}
	
    if( OperationDenomiCount[2] != 0 )
		{
			if( (ex_rc_status.sst31A.bit.u1_d1_empty && OperationDenomiBoxNumber[2] == 0x01)      /* RC1 is not empty */
			  ||(ex_rc_status.sst31A.bit.u1_d2_empty && OperationDenomiBoxNumber[2] == 0x02)      /* RC2 is not empty */
        ||(ex_rc_status.sst32A.bit.u2_d1_empty && OperationDenomiBoxNumber[2] == 0x03)      /* RC3 is not empty */
        ||(ex_rc_status.sst32A.bit.u2_d2_empty && OperationDenomiBoxNumber[2] == 0x04))     /* RC4 is not empty */
			{
				return(FALSE);
			}
		}

    if( OperationDenomiCount[3] != 0 )
		{
			if( (ex_rc_status.sst31A.bit.u1_d1_empty && OperationDenomiBoxNumber[3] == 0x01)      /* RC1 is not empty */
			  ||(ex_rc_status.sst31A.bit.u1_d2_empty && OperationDenomiBoxNumber[3] == 0x02)      /* RC2 is not empty */
        ||(ex_rc_status.sst32A.bit.u2_d1_empty && OperationDenomiBoxNumber[3] == 0x03)      /* RC3 is not empty */
        ||(ex_rc_status.sst32A.bit.u2_d2_empty && OperationDenomiBoxNumber[3] == 0x04))     /* RC4 is not empty */
			{
				return(FALSE);
			}
		}
  }
  else  /* RT */
  {
    if( OperationDenomiCount[0] != 0 )
		{
			if( (ex_rc_status.sst31A.bit.u1_d1_empty && OperationDenomiBoxNumber[0] == 0x01)      /* RC1 is not empty */
			  ||(ex_rc_status.sst31A.bit.u1_d2_empty && OperationDenomiBoxNumber[0] == 0x02))     /* RC2 is not empty */
			{
				return(FALSE);
			}
		}
	
		if( OperationDenomiCount[1] != 0 )
		{
			if( (ex_rc_status.sst31A.bit.u1_d1_empty && OperationDenomiBoxNumber[1] == 0x01)      /* RC1 is not empty */
			  ||(ex_rc_status.sst31A.bit.u1_d2_empty && OperationDenomiBoxNumber[1] == 0x02))     /* RC2 is not empty */
			{
				return(FALSE);
			}
		}
  }

	  return(TRUE);
}

/* Check Number of Bill to Payout */
static u8	check_payout_count_recycler(void)
{
  if(ex_rc_status.sst1A.bit.quad) /* RQ */
  {
    /* Payout 0 or more than 30 notes is invalid */
		if( ( OperationDenomiCount[0]     == 0 
          && OperationDenomiCount[1]  == 0 
          && OperationDenomiCount[2]  == 0 
          && OperationDenomiCount[3]  == 0)
		  ||( OperationDenomiCount[0]     > RECYCLER_BILL_MAX 
          && OperationDenomiCount[1]  > RECYCLER_BILL_MAX 
          && OperationDenomiCount[2]  > RECYCLER_BILL_MAX 
          && OperationDenomiCount[3]  > RECYCLER_BILL_MAX ))
		{
			return(FALSE);
		}
  }
  else  /* RT */
  {
    /* Payout 0 or more than 30 notes is invalid */
		if( ( OperationDenomiCount[0] == 0 && OperationDenomiCount[1] == 0 )
		 || ( OperationDenomiCount[0]  > RECYCLER_BILL_MAX && OperationDenomiCount[1]  > RECYCLER_BILL_MAX ) )
		{
			return(FALSE);
		}
  }

		/* Check that number of bill to pay more than  RC limit or not : 1st Box */
		if( OperationDenomiCount[0] != 0 )
		{
			if((RecycleSettingInfo.DenomiInfo[ OperationDenomiBoxNumber[0] - 1 ].RecycleLimit != 0
			  && RecycleSettingInfo.DenomiInfo[ OperationDenomiBoxNumber[0] - 1 ].RecycleLimit < OperationDenomiCount[0]))
			{
				return(FALSE);
			}
		}

		/* Check that number of bill to pay more than  RC limit or not : 2nd Box */
		if( OperationDenomiCount[1] != 0 )
		{
			if((RecycleSettingInfo.DenomiInfo[OperationDenomiBoxNumber[1] - 1].RecycleLimit != 0 
			    && RecycleSettingInfo.DenomiInfo[OperationDenomiBoxNumber[1] - 1].RecycleLimit < OperationDenomiCount[1]))
			{
				return(FALSE);
			}
		}

  if(ex_rc_status.sst1A.bit.quad) /* RQ */
  {
    /* Check that number of bill to pay more than  RC limit or not : 3rd Box */
		if( OperationDenomiCount[2] != 0 )
		{
			if((RecycleSettingInfo.DenomiInfo[OperationDenomiBoxNumber[2] - 1].RecycleLimit != 0 
			    && RecycleSettingInfo.DenomiInfo[OperationDenomiBoxNumber[2] - 1].RecycleLimit < OperationDenomiCount[2]))
			{
				return(FALSE);
			}
		}

    /* Check that number of bill to pay more than  RC limit or not : 4th Box */
		if( OperationDenomiCount[3] != 0 )
		{
			if((RecycleSettingInfo.DenomiInfo[OperationDenomiBoxNumber[3] - 1].RecycleLimit != 0 
			    && RecycleSettingInfo.DenomiInfo[OperationDenomiBoxNumber[3] - 1].RecycleLimit < OperationDenomiCount[3]))
			{
				return(FALSE);
			}
		}
  }

    return(TRUE);
}

void _id003_return_err_mode(void)	//2025-02-06 Pay out紙幣がエマージョエンシ―ストップで、回収、識別エラーで返却中
{
	if (s_id003_status_wait_flag == 0)
	{
	/* Normal (Not Status WAIT) */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYVALID;
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{

		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{

		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 59);
		}
	}
	else
	{
	/* Status WAIT */
		if (cline_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			ex_cline_status_tbl.line_task_mode = ID003_MODE_PAYVALID;
			_id003_intr_mode_sub(ex_cline_status_tbl.line_task_mode, 0);
		}
		else if (cline_msg.arg1 == TMSG_SUB_INTERIM)
		{
			/*  */
		}
		else if (cline_msg.arg1 == TMSG_SUB_REJECT)
		{

		}
		else if (cline_msg.arg1 == TMSG_SUB_ALARM)
		{

		}
		else
		{
			/* system error ? */
			_cline_system_error(0, 60);
		}
	}
}

//#if defined(ID003_SENSOR)
void _id003_get_sensor_status_cmd_proc(void)
{
    if( _id003_rx_buff.length == (0x01 + ID003_ADD_04) )
    {
		if(is_rc_rs_unit())
		{
	        _id003_tx_buff.length  = (5 + ID003_ADD_04);
	        _id003_tx_buff.cmd     = ID003_CMD_GET_SENSOR_STATUS;
	        _id003_tx_buff.data[0] = (u8)ex_position_dirt[0];
	        _id003_tx_buff.data[1] = (u8)ex_position_dirt[1];
	        _id003_tx_buff.data[2] = (u8)ex_position_dirt[2];
	        _id003_tx_buff.data[3] = (u8)ex_position_dirt[3];
		}
		else
		{
	        _id003_tx_buff.length  = (4 + ID003_ADD_04);
	        _id003_tx_buff.cmd     = ID003_CMD_GET_SENSOR_STATUS;
	        _id003_tx_buff.data[0] = (u8)ex_position_dirt[0];
	        _id003_tx_buff.data[1] = (u8)ex_position_dirt[1];
	        _id003_tx_buff.data[2] = (u8)ex_position_dirt[2];
		}
		uart_send_id003(&_id003_tx_buff);
    }
    else
    {
        _id003_send_host_invalid();
    }
}
//#endif

void _id003_rc_dldstatus_cmd_proc(void)
{
	if(_id003_rx_buff.cmd == ID003_CMD_EXT_RC && _id003_rx_buff.data[1] == ID003_CMD_EXT_RC_DLD_STATUS)
	{
		_id003_tx_buff.length	= (4 + ID003_ADD_04);
		_id003_tx_buff.cmd		= ID003_CMD_EXT_RC;
	//	_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
		_is_set_unit_id();
		_id003_tx_buff.data[1]	= ID003_CMD_EXT_RC_DLD_STATUS;

		if(ex_rc_download_stat == 1)
		{
			_id003_tx_buff.data[2]	= ID003_STS_EXT_RC_DOWNLOAD_BUSY;
		}
		else
		{
			_id003_tx_buff.data[2]	= ID003_STS_EXT_RC_DOWNLOAD_READY;
		}
		uart_send_id003(&_id003_tx_buff);
	}
	else
	{
//		_id003_send_host_invalid();
	}
}

static void _is_set_unit_id(void)
{
	if(is_rc_rs_unit())
	{
		_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER_RSUNIT;
	}
	else
	{
		_id003_tx_buff.data[0]	= ID003_UNIT_ID_RECYCLER;
	}
}

#endif


