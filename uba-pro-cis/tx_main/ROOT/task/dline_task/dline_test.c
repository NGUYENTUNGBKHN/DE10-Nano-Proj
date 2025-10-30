/******************************************************************************/
/*! @addtogroup Main
    @file       dline_test.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * dline_test.c
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
#include "motor_ctrl.h"
#include "dline_suite.h"
#include "dline_test.h"
#include "hal.h"
#include "hal_clk.h"

#if defined(UBA_RTQ)
#include "if_rc.h"
#endif // UBA_RTQ

#define EXT
#include "com_ram.c"
#include "com_ram_ncache.c"
#include "usb_ram.c"

#if defined(UBA_RTQ_AZ_LOG) 
u32 _hal_write_fram_debug_log_uba(void); //2023-06-09
#endif

extern u8 bkex_status_tbl_buff[256];


/************************** PRIVATE DEFINITIONS ******************************/

/*===== JCM Device Log (JDL) definition =====*/
typedef struct _currency_information{
		u8 country_code;
		u8 denomi_base;
		u8 denomi_exp;
		u8 info0;
		u8 info1;
		u8 info2;
} currency_infomation;
extern const currency_infomation currency_info[DENOMI_SIZE];

/************************** Function Prototypes ******************************/
static u8 phase_dipswitch_test(void);
static u8 phase_sensor_test(void);


static u8 phase_motor_speed_test(void);

static u8 phase_width_speed_test(void);
static u8 phase_stacker_speed_test(void);
static u8 phase_pb_speed_test(void);
static u8 phase_shutter_speed_test(void);

static u8 phase_denomi_test(void);
static u8 phase_stack_test(void);
static u8 phase_width_test(void);

static u8 phase_shutter_motor_test(void);
#if defined(UBA_LOG)
	static u8 phase_get_uba_log_test(void);
#endif

#if defined(_ENABLE_MAG_AGING_EMI)
	static u8 phase_mag_emi_test(void);
#endif /* _ENABLE_MAG_AGING_EMI */

static u8 phase_system_error_info(void);
static u8 phase_hw_state(void);
u8 phase_end_test(void);

#if (_DEBUG_FPGA_FRAM==1) //2023-07-22
static u8 phase_fram_log(void);
#endif

static u8 phase_task_seq(void);
static u8 phase_clock(void);

/************************** External functions *******************************/

extern void _dline_set_mode(u16 mode);
extern void _dline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void set_response_1data(u8 cmd);
extern void data_collection_send( u32 usb_write_size );
void phase_get_ss_rc_type(void);

/************************** Variable declaration *****************************/

/************************** EXTERNAL VARIABLES *******************************/
extern u16 ex_dline_task_mode;

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/
extern struct _testmode ex_dline_testmode;

static u8 reset_dline_test(void)
{
	ex_dline_testmode.action = TEST_NON_ACTION;
	ex_dline_testmode.test_no = TEST_STANDBY;
	_dline_set_mode(DLINE_MODE_TEST_STANDBY);

	return 0;
}
/*********************************************************************//**
 * @brief dipsw select test mode
 * @param[in]	dipsw   : dipsw1 setting
 * @return 		test_no : select test no
 **********************************************************************/
u16 _dlise_dipsw_select_test(u8 dipsw)
{
	u16 test_no;
	switch (dipsw)
	{
	case DIPSW1_FEED_MOTOR_FWD_TEST:				/* DIP 1 .*/
		test_no = TEST_FEED_MOTOR_FWD;
		break;
	case DIPSW1_FEED_MOTOR_REV_TEST:				/* DIP 2 .*/
		test_no = TEST_FEED_MOTOR_REV;
		break;
	case DIPSW1_STACK_TEST:							/* DIP 3 .*/
		test_no = TEST_STACKER;
		break;
	case DIPSW1_STACKER_MOTOR_FWD_TEST:				/* DIP 1, 3 .*/
		test_no = TEST_STACKER_MOTOR_FWD;
		break;
	case DIPSW1_AGING_TEST:							/* DIP 4 */
		test_no = TEST_AGING;
		break;
	//case DIPSW1_AGING_LD_TEST:
	//	test_no = TEST_AGING_LD;
	//	break;
	case DIPSW1_APB_TEST:							/* DIP 5 */
		test_no = TEST_APB;
		break;
	case DIPSW1_APB_CLOSE_TEST:						/* DIP 5, 7 */
		test_no = TEST_APB_CLOSE;
		break;
	case DIPSW1_CENTERING_TEST:						/* DIP 1, 5 */
		test_no = TEST_CENTERING;
		break;
	case DIPSW1_CENTERING_CLOSE_TEST:				/* DIP 1, 3, 5 */
		test_no = TEST_CENTERING_CLOSE;
		break;
	case DIPSW1_SENSOR_TEST:						/* DIP 7 */
		test_no = TEST_SENSOR;
		break;
	case DIPSW1_ACCEPT_TEST:						/* DIP 1, 2, 3, 4 .*/
		test_no = TEST_ACCEPT;
		break;
	case DIPSW1_ACCEPT_LD_TEST:						/* DIP 1, 2, 3 */
		test_no = TEST_ACCEPT_LD;
		break;
	case DIPSW1_ACCEPT_ALLACC_TEST:					/* DIP 1, 2, 3, 4, 6 */
		test_no = TEST_ACCEPT_ALLACC;
		break;
	case DIPSW1_ACCEPT_LD_ALLACC_TEST:				/* DIP 1, 2, 3, 4, 5 */
		test_no = TEST_ACCEPT_LD_ALLACC;
		break;
	case DIPSW1_REJECT_TEST:						/* DIP 1, 2, 3, 4, 5, 7 .*/
		test_no = TEST_REJECT;
		break;
	case DIPSW1_REJECT_LD_TEST:						/* DIP 1, 2, 3, 4, 5, 6 */
		test_no = TEST_REJECT_LD;
		break;
	case DIPSW1_DIPSW1_TEST:						/* DIP 1, 2, 3, 4, 5, 6, 7 .*/
		test_no = TEST_DIPSW1;
		break;

	case DIPSW1_SHUTTER_TEST:	/* DIP-5,6 */
		test_no = TEST_SHUTTER;
		break;
	case DIPSW1_SHUTTER_CLOSE_TEST:					/* DIP 1, 6 .*/
		test_no = TEST_SHUTTER_CLOSE;
		break;
	case DIPSW1_RFID_UBA_TEST:						/* DIP 5, 6, 7 */
		test_no = TEST_RFID_UBA;
		break;
#if defined(UBA_RTQ)
	case DIPSW1_RC_AGING_TEST:						/* DIP 1, 2, 3, 4, 7 */
		//2025-06-05 #if defined(UBA_RS)
		//UBA500RTQと変えた 2025-09-12
		//UBA500の場合、エージング中に一度DIP-SW 8 ONにしてエージングを停止させて
		//再度 DIP-SW8 OFFにしてエージングを再開した場合、RSモデルの場合、RSなしモデルとしてエージング動作する(エラーにはならないが)
		//UBA500RTQとしては、途中のDIP-SW 8 ONには対応していないとの事
		//UBA700RTQは修正できそうなので修正した
		if(ex_rc_configuration.unit_type_bk != 0) //この条件を新規追加
		{
			//RSで、すでに一度エージングを開始しているので、設定を維持
		}
		else
		{
			//RSモデルでないか、まだエージングを1度も行っていない可能性
			if(is_rc_rs_unit())
			{
				ex_rc_configuration.unit_type_bk = ex_rc_configuration.unit_type;
				ex_rc_configuration.unit_type = 0;
			}
			else
			{
				ex_rc_configuration.unit_type_bk = 0;
			}			
		}
		test_no = TEST_RC_AGING;
		break;
	case DIPSW1_RC_AGING_FACTORY:					/* DIP 1, 2, 3, 4, 6, 7 */
		//2025-06-05 #if defined(UBA_RS)
		//UBA500RTQと変えた 2025-09-12
		//UBA500の場合、エージング中に一度DIP-SW 8 ONにしてエージングを停止させて
		//再度 DIP-SW8 OFFにしてエージングを再開した場合、RSモデルの場合、RSなしモデルとしてエージング動作する(エラーにはならないが)
		//UBA500RTQとしては、途中のDIP-SW 8 ONには対応していないとの事
		//UBA700RTQは修正できそうなので修正した
		if(ex_rc_configuration.unit_type_bk != 0) //この条件を新規追加
		{
			//RSで、すでに一度エージングを開始しているので、設定を維持
		}
		else
		{
			//RSモデルでないか、まだエージングを1度も行っていない可能性
			if(is_rc_rs_unit())
			{
				ex_rc_configuration.unit_type_bk = ex_rc_configuration.unit_type;
				ex_rc_configuration.unit_type = 0;
			}
			else
			{
				ex_rc_configuration.unit_type_bk = 0;
			}			
		}
		test_no = TEST_RC_AGING_FACTORY;
		break;
	case DIPSW1_RC_TEST1:							/* DIP 1, 2, 5 */
		test_no = TEST_RC1;							/* DIP 1, 2, 5 */
		break;
	case DIPSW1_RC_TEST2:							/* DIP 1, 2, 6 */
		test_no = TEST_RC2;							/* DIP 1, 2, 6 */
		break;
	//#if defined(UBA_RTQ_ICB)
	case DIPSW1_RS_TEST:							/* DIP 1, 2, 5, 6 *///生産で使用 test rfid
		test_no = TEST_RS;
		break;
	//#endif
#endif // UBA_RTQ

	default:
		test_no = TEST_STANDBY;
		break;
	}

	return test_no;
}

u8 reset_dline_sw_test(void)
{
	return reset_dline_test();
}
u8 select_dline_sw_test(void)
{
	if (!(ex_dipsw1 & DIPSW1_PERFORMANCE_TEST))	//check
	{
		ex_dline_testmode.test_no = _dlise_dipsw_select_test(ex_dipsw1);

		switch(ex_dline_testmode.test_no)
		{
		case TEST_ACCEPT:
		case TEST_ACCEPT_LD:
		case TEST_ACCEPT_ALLACC:
		case TEST_ACCEPT_LD_ALLACC:
		case TEST_REJECT:
		case TEST_REJECT_LD:
		case TEST_REJECT_CENT_OPEN: //2024-12-01 生産CIS初期流動に使用
			/* accept test */
			/* accept test (ld mode) */
			/* accept test (all accept mode) */
			/* accept test (ld & all accept mode) */
			/* reject test */
			/* reject test (ld mode) */
			ex_dline_testmode.action = TEST_SW_CONTROL;
			_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			break;

		case TEST_AGING:
		//case TEST_AGING_LD:
		case TEST_FEED_MOTOR_FWD:
		case TEST_FEED_MOTOR_REV:
		case TEST_STACKER_MOTOR_FWD:
		case TEST_STACKER:
		case TEST_STACKER_HOME:
		case TEST_APB:
		case TEST_CENTERING:
		case TEST_CENTERING_CLOSE:
		case TEST_SENSOR:
		case TEST_DIPSW1:
		case TEST_APB_CLOSE:
		case TEST_SHUTTER:
		case TEST_SHUTTER_CLOSE:
		case TEST_RFID_UBA:
			ex_dline_testmode.action = TEST_SW_CONTROL;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			break;
#if defined(UBA_RTQ)
		#if 0	//2025-09-04 UBA500と合わせるならここではなく、DIP-SWのメッセージ受信の場所のはず
		case TEST_RC1:	/* DIP 1, 2, 5 */
			ex_dline_testmode.action = TEST_SW_CONTROL;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			break;
		case TEST_RC2:	/* DIP 1, 2, 6 */
			ex_dline_testmode.action = TEST_SW_CONTROL;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			break;
		#endif
		case TEST_RC_AGING_FACTORY:
			ex_dline_testmode.action = TEST_SW_CONTROL;
			_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			break;
		case TEST_RC_AGING:
			ex_dline_testmode.action = TEST_SW_CONTROL;
			_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			break;
	//#if defined(UBA_RTQ_ICB)
	#if 0	//2025-09-04 UBA500と合わせるならここではなく、DIP-SWのメッセージ受信の場所のはず
		case TEST_RS:
			ex_dline_testmode.action = TEST_SW_CONTROL;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			break;
	#endif		
	//#endif
		#if 1	//2025-09-04 UBA500と合わせるならこっちのはず
		case TEST_RC1:
		case TEST_RC2:
		case TEST_RS:
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_DIPSW_REQ, 0, 0, 0, 0);
			break;
		#endif
#endif // UBA_RTQ
		case TEST_STANDBY:
		default:
			/* standby */
			break;
		}
	}
#if defined(UBA_RTQ)
	else
	{	
	#if 0	////2025-09-04
		/* 03/04/2025 Add check RC dipsw timeout 200ms */
		if (ex_dipsw1 == (DIPSW1_RC_TEST1 | DIPSW1_PERFORMANCE_TEST) || 
			ex_dipsw1 == (DIPSW1_RC_TEST2 | DIPSW1_PERFORMANCE_TEST) ||
			ex_dipsw1 == (DIPSW1_RS_TEST  | DIPSW1_PERFORMANCE_TEST)) //check
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_DIPSW_REQ, 0, 0, 0, 0);
		}
	#endif
	}
#endif // UBA_RTQ
	return 0;
}
static u8 phase_accept_test_ss(void)
{
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_ACCEPT;
			_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_STOP:
		if (((ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
		 || (ex_dline_task_mode == DLINE_MODE_ATEST_ERROR))
		 && (ex_dline_testmode.test_no == TEST_ACCEPT)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}
static u8 phase_accept_test(void)
{
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_ACCEPT_LD;
			_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_STOP:
		if (((ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
		 || (ex_dline_task_mode == DLINE_MODE_ATEST_ERROR))
		 && (ex_dline_testmode.test_no == TEST_ACCEPT_LD)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}
static u8 phase_no_judge_accept_test_ss(void)
{
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_ACCEPT_ALLACC;
			_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_STOP:
		if (((ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
		 || (ex_dline_task_mode == DLINE_MODE_ATEST_ERROR))
		 && (ex_dline_testmode.test_no == TEST_ACCEPT_ALLACC)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}
static u8 phase_no_judge_accept_test(void)
{
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_ACCEPT_LD_ALLACC;
			_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_STOP:
		if (((ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
		 || (ex_dline_task_mode == DLINE_MODE_ATEST_ERROR))
		 && (ex_dline_testmode.test_no == TEST_ACCEPT_LD_ALLACC)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}
static u8 phase_no_judge_reject_test_ss(void)
{
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_REJECT;
			_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_STOP:
		if (((ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
		 || (ex_dline_task_mode == DLINE_MODE_ATEST_ERROR))
		 && (ex_dline_testmode.test_no == TEST_REJECT)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}
static u8 phase_no_judge_reject_test(void)
{
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_REJECT_LD;
			_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_STOP:
		if (((ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
		 || (ex_dline_task_mode == DLINE_MODE_ATEST_ERROR))
		 && (ex_dline_testmode.test_no == TEST_REJECT_LD)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

static u8 phase_collection_reject_cent_open_test_ss(void) //2024-12-01 生産CIS初期流動に使用
{
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_collection_data.enable = true;
			ex_collection_data.data_exist = DATA_NONE;
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_REJECT_CENT_OPEN;
			_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_STOP:
		if (((ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
		 || (ex_dline_task_mode == DLINE_MODE_ATEST_ERROR))
		 && (ex_dline_testmode.test_no == TEST_REJECT_CENT_OPEN)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			ex_collection_data.enable = false;
			ex_collection_data.data_exist = DATA_NONE;
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

static u8 phase_transmit_motor_test(void)
{
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_MOTOR_FWD:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_feed_motor_test_speed = 0;
			/* Setting output & input GPIO */
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_FEED_MOTOR_FWD;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			if (ex_front_usb.pc.mess.length > 6)
			{
			/* set speed */
				if (ex_usb_read_buffer[6] < MOTOR_MAX_SPEED)
				{
					_ir_feed_motor_ctrl.pwm = ex_usb_read_buffer[6];
					ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed = FEED_MOTOR_SPEED_PWM;
				}
				else
				{
					_ir_feed_motor_ctrl.pwm = MOTOR_MAX_SPEED;
					ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed = FEED_MOTOR_SPEED_FULL;
				}
			}
			else
			{
				ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed = FEED_MOTOR_SPEED_FULL;
			}
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_MOTOR_REV:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_feed_motor_test_speed = 0;
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_FEED_MOTOR_REV;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			if (ex_front_usb.pc.mess.length > 6)
			{
			/* set speed */
				if (ex_usb_read_buffer[6] < MOTOR_MAX_SPEED)
				{
					_ir_feed_motor_ctrl.pwm = ex_usb_read_buffer[6];
					ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed = FEED_MOTOR_SPEED_PWM;
				}
				else
				{
					_ir_feed_motor_ctrl.pwm = MOTOR_MAX_SPEED;
					ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed = FEED_MOTOR_SPEED_FULL;
				}
			}
			else
			{
				ex_feed_motor_speed[FEED_SPEED_FREE_RUN].set_speed = FEED_MOTOR_SPEED_FULL;
			}
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		&& ((ex_dline_testmode.test_no == TEST_FEED_MOTOR_FWD)
		 || (ex_dline_testmode.test_no == TEST_FEED_MOTOR_REV))
		&&  (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}
static u8 phase_stacker_motor_test(void)
{
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_MOTOR_FWD:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_feed_motor_test_speed = 0;
			/* Setting output & input GPIO */
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_STACKER_MOTOR_FWD;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			if (ex_front_usb.pc.mess.length > 6)
			{
			/* set speed */

			}
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_MOTOR_REV:
		set_response_1data(NAK);
		break;
	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		&& (ex_dline_testmode.test_no == TEST_STACKER_MOTOR_FWD)
		&&  (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}
static u8 phase_width_motor_test(void) /* 0x13 */// おそらくUBA700では使用してない　phase_width_test　とほぼ同じこっちはcloseがない
{
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_MOTOR_FWD:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_CENTERING;

			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_MOTOR_REV: //not use
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			set_response_1data(NAK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		&& (
		   (ex_dline_testmode.test_no == TEST_CENTERING)
		   )
		&&  (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	
	case 0x44:	/* close */
		/* 生産の初期流動で使用している */
		//if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		if(1)
		{
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_CENTERING_CLOSE;
			ex_dline_testmode.test_result = TEST_NOT_YET;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

/******************************************************************************/
/*! @brief
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
static u8 phase_sensor_test(void)
{
#if defined(UBA_RTQ)
	u16 data;
#endif	
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_SENSOR;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_ENQ:
		//常にAckを付けているので、サイズが異なるどちらに合わせるか保留
		//UBA500とセンサデータの上位下位が逆なので、注意
		ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x09;
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		// add
		*(ex_usb_write_buffer + 5) = 0x06;	/* エコーバックではなくAckを使用する	*/
		*(ex_usb_write_buffer + 6) = TEST_RESULT_OK;
	#if defined(UBA_RTQ)
		//UBA500RTQは対応していない様だが、UBA700RTQは対応する
		data = ex_position_sensor;
		if(ex_rc_status.sst1B.bit.stacker_home == 1)
		{
			data = (data | POSI_BOX_HOM);
		}
		else
		{
			data = (data & (~POSI_BOX_HOM));
		}
		if(ex_rc_status.sst1B.bit.box_detect == 1)
		{
			data = (data | POSI_500BOX);
		}
		else
		{
			data = (data & (~POSI_500BOX));
		}
		*(ex_usb_write_buffer + 7) = (u8)((data & 0xff00) >> 8);
		*(ex_usb_write_buffer + 8) = (u8)(data & 0xff);
	#else
		*(ex_usb_write_buffer + 7) = (u8)((ex_position_sensor & 0xff00) >> 8);
		*(ex_usb_write_buffer + 8) = (u8)(ex_position_sensor & 0xff);
	#endif
		ex_usb_write_size = 9;
		break;
	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && (ex_dline_testmode.test_no == TEST_SENSOR)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}



/******************************************************************************/
/*! @brief dip switch test
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @par            Modify
    - 変更するグローバル変数 ex_dline_testmode
    @return         none
    @exception      none
******************************************************************************/
static u8 phase_dipswitch_test(void)
{
	u8 dip_switch_value = 0;

	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_DIP_SWITCH1:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			/* clear pre phase & request */
			ex_dline_testmode.dline.phase = 0;
			ex_dline_testmode.dline.request = 0;
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_DIPSW1;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_DIP_SWITCH2:
		set_response_1data(NAK);
		break;

	case CMD_ENQ:

		dip_switch_value = ex_dipsw1;
		
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x07;
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)((dip_switch_value & 0xff00) >> 8);
		*(ex_usb_write_buffer + 6) = (u8)(dip_switch_value & 0xff);
		ex_usb_write_size = 7;				/* send 7 Byte			*/
		break;
	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		&& ((ex_dline_testmode.test_no == TEST_DIPSW1)
		 )
		&&  (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			/* clear pre phase & request */
			ex_dline_testmode.dline.phase = 0;
			ex_dline_testmode.dline.request = 0;
			/* add test mode end flag */
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

static u8 phase_aging_test_ss(void)
{
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_AGING;
			_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_ATEST_INITIAL)
		 && (ex_dline_testmode.test_no == TEST_AGING)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

static u8 phase_denomi_test(void)
{
	/*<<	clear command waiting flag	>>*/
	ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
	/* clear pre phase & request */
	ex_dline_testmode.dline.phase = 0;
	ex_dline_testmode.dline.request = 0;

	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_ACCEPT_LD;
			_dline_set_mode(DLINE_MODE_ATEST_INITIAL);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_ENQ:
 		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
 		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x09;
 		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
 		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		if(ex_validation.reject_code != REJECT_CODE_OK)
		{
			*(ex_usb_write_buffer + 5) = CMD_DENOMI_NG;
			*(ex_usb_write_buffer + 6) = ex_validation.reject_code;
			*(ex_usb_write_buffer + 7) = 0;
			*(ex_usb_write_buffer + 8) = 0;
		}
		else if(ex_validation.denomi == BAR_INDX)
		{
			*(ex_usb_write_buffer + 5) = CMD_TICKET_OK;
			*(ex_usb_write_buffer + 6) = ex_validation.reject_code;
			*(ex_usb_write_buffer + 7) = 0;
			*(ex_usb_write_buffer + 8) = 0;
		}
		else
		{
			*(ex_usb_write_buffer + 5) = CMD_DENOMI_OK;
			*(ex_usb_write_buffer + 6) = ex_validation.reject_code;
			*(ex_usb_write_buffer + 7) = currency_info[ex_validation.denomi].denomi_base;
			*(ex_usb_write_buffer + 8) = currency_info[ex_validation.denomi].denomi_exp;
		}
		ex_usb_write_size = 9;				/* send 9 Byte			*/
		break;
	case CMD_STOP:
		if (((ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
		 || (ex_dline_task_mode == DLINE_MODE_ATEST_ERROR))
		 && (ex_dline_testmode.test_no == TEST_ACCEPT_LD)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}


#if defined(UBA_RTQ)
static void uba_rtq_feed_speed()
{
	ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
	*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
	*(ex_usb_write_buffer + 1) = 0x00;
	*(ex_usb_write_buffer + 2) = 0x09;
	*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
	*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
	*(ex_usb_write_buffer + 5) = 0x06;	/* エコーバックではなくAckを使用する	*/

	if( ex_dline_testmode.test_result == TEST_NOT_YET )
	{
	// wait
		*(ex_usb_write_buffer + 6) = TEST_NOT_YET;	/* not yet	*/
		*(ex_usb_write_buffer + 7) = 0;
		*(ex_usb_write_buffer + 8) = 0;
	}
	else if( ex_dline_testmode.test_result == TEST_RESULT_OK )
	{
	// OK
		*(ex_usb_write_buffer + 6) = TEST_RESULT_OK;	/* 0x10	*/
		// Feed
#if 1
		*(ex_usb_write_buffer + 7) = (u8)((ex_feed_motor_test_speed/10) & 0xff);
		*(ex_usb_write_buffer + 8) = (u8)(((ex_feed_motor_test_speed/10) & 0xff00) >> 8);
#else
		*(ex_usb_write_buffer + 7) = (u8)((ex_feed_motor_test_speed/10 - 30) & 0xff);
		*(ex_usb_write_buffer + 8) = (u8)(((ex_feed_motor_test_speed/10 - 30) & 0xff00) >> 8);
#endif 
		
	}
	else
	{
	// NG
		*(ex_usb_write_buffer + 6) = TEST_RESULT_NG;	/* NG	*/
		*(ex_usb_write_buffer + 7) = 0;
		*(ex_usb_write_buffer + 8) = 0;
	}

	ex_usb_write_size = 9;
}
#endif // UBA_RTQ


/******************************************************************************/
/*! @brief motor speed test
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @par            Modify
    - 変更するグローバル変数 ex_dline_testmode
    @return         none
    @exception      none
******************************************************************************/
static u8 phase_motor_speed_test(void)
{
	/*<<	clear command waiting flag	>>*/
	ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
	/* clear pre phase & request */
	ex_dline_testmode.dline.phase = 0;
	ex_dline_testmode.dline.request = 0;

	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && ((ex_dline_testmode.test_no == TEST_FEED_MOTOR_FWD) || (ex_dline_testmode.test_no == TEST_FEED_MOTOR_REV))
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			ex_dline_testmode.test_result = TEST_NOT_YET;
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_ENQ:
		#if defined(UBA_RTQ)  /* FOR UBA RTQ */
		uba_rtq_feed_speed();
		#else
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x0c;
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = RES_DATA;
		*(ex_usb_write_buffer + 6) = (u8)((ex_feed_motor_test_speed & 0xff00) >> 8);
		*(ex_usb_write_buffer + 7) = (u8)(ex_feed_motor_test_speed & 0xff);
		*(ex_usb_write_buffer + 8) = (u8)((0 & 0xff00) >> 8);
		*(ex_usb_write_buffer + 9) = (u8)(0 & 0xff);
		*(ex_usb_write_buffer + 10) = (u8)((0 & 0xff00) >> 8);
		*(ex_usb_write_buffer + 11) = (u8)(0 & 0xff);
 		ex_usb_write_size = 12;				/* send 12 Byte			*/
		#endif // UBA_RTQ
		break;

	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && ((ex_dline_testmode.test_no == TEST_FEED_MOTOR_FWD)
		  || (ex_dline_testmode.test_no == TEST_FEED_MOTOR_REV))
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}


static u8 phase_width_speed_test(void)	/* 0x82 */
{
	/*<<	clear command waiting flag	>>*/
	ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
	/* clear pre phase & request */
	ex_dline_testmode.dline.phase = 0;
	ex_dline_testmode.dline.request = 0;
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && ((ex_dline_testmode.test_no == TEST_CENTERING))
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;

	case CMD_ENQ:
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x0c;
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = RES_DATA;
		*(ex_usb_write_buffer + 6) = (u8)((ex_centering_count & 0xff00) >> 8);
		*(ex_usb_write_buffer + 7) = (u8)(ex_centering_count & 0xff);
		*(ex_usb_write_buffer + 8) = (u8)((ex_dline_testmode.time1 & 0xff00) >> 8);
		*(ex_usb_write_buffer + 9) = (u8)(ex_dline_testmode.time1 & 0xff);
		*(ex_usb_write_buffer + 10) = (u8)((0 & 0xff00) >> 8);
		*(ex_usb_write_buffer + 11) = (u8)(0 & 0xff);
		*(ex_usb_write_buffer + 12) = (u8)((0 & 0xff00) >> 8);
		*(ex_usb_write_buffer + 13) = (u8)(0 & 0xff);
 		ex_usb_write_size = 14;				/* send 14 Byte			*/
		break;

	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && ((ex_dline_testmode.test_no == TEST_CENTERING))
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

/******************************************************************************/
/*! @brief motor speed test
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @par            Modify
    - 変更するグローバル変数 ex_dline_testmode
    @return         none
    @exception      none
******************************************************************************/
static u8 phase_pb_speed_test(void)
{
	/*<<	clear command waiting flag	>>*/
	ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
	/* clear pre phase & request */
	ex_dline_testmode.dline.phase = 0;
	ex_dline_testmode.dline.request = 0;
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && ((ex_dline_testmode.test_no == TEST_APB))
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_ENQ:
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x0A;
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = RES_DATA;
		*(ex_usb_write_buffer + 6) = (u8)((ex_apb_count & 0xff00) >> 8);
		*(ex_usb_write_buffer + 7) = (u8)(ex_apb_count & 0xff);
		*(ex_usb_write_buffer + 8) = (u8)((ex_dline_testmode.time1 & 0xff00) >> 8);
		*(ex_usb_write_buffer + 9) = (u8)(ex_dline_testmode.time1 & 0xff);
 		ex_usb_write_size = 10;				/* send 10 Byte			*/
		break;
	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && (ex_dline_testmode.test_no == TEST_APB)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

/******************************************************************************/
/*! @brief motor speed test
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @par            Modify
    - 変更するグローバル変数 ex_dline_testmode
    @return         none
    @exception      none
******************************************************************************/
static u8 phase_stacker_speed_test(void)	/* 0x86 */
{
	/*<<	clear command waiting flag	>>*/
	ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
	/* clear pre phase & request */
	ex_dline_testmode.dline.phase = 0;
	ex_dline_testmode.dline.request = 0;
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && ((ex_dline_testmode.test_no == TEST_STACKER_MOTOR_FWD) || (ex_dline_testmode.test_no == TEST_STACKER))
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
#if defined(UBA_RTQ)
		case CMD_ENQ:
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x0C;
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = RES_DATA;

		*(ex_usb_write_buffer + 6) = (u8)((ex_stacker_count & 0xff00) >> 8);
		*(ex_usb_write_buffer + 7) = (u8)(ex_stacker_count & 0xff);

		*(ex_usb_write_buffer + 8) = (u8)((ex_dline_testmode.time1 & 0xff00) >> 8);
		*(ex_usb_write_buffer + 9) = (u8)(ex_dline_testmode.time1 & 0xff);
		*(ex_usb_write_buffer + 10) = (u8)((ex_dline_testmode.time2 & 0xff00) >> 8);
		*(ex_usb_write_buffer + 11) = (u8)(ex_dline_testmode.time2 & 0xff);

 		ex_usb_write_size = 12;				/* send 12 Byte			*/
		break;
#else
	case CMD_ENQ:
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x0C;
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = RES_DATA;

		*(ex_usb_write_buffer + 6) = (u8)((ex_stacker_count & 0xff00) >> 8);
		*(ex_usb_write_buffer + 7) = (u8)(ex_stacker_count & 0xff);

		*(ex_usb_write_buffer + 8) = (u8)((ex_dline_testmode.time1 & 0xff00) >> 8);
		*(ex_usb_write_buffer + 9) = (u8)(ex_dline_testmode.time1 & 0xff);
		*(ex_usb_write_buffer + 10) = (u8)((ex_dline_testmode.time2 & 0xff00) >> 8);
		*(ex_usb_write_buffer + 11) = (u8)(ex_dline_testmode.time2 & 0xff);

 		ex_usb_write_size = 12;				/* send 12 Byte			*/
		break;
#endif 
	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && ((ex_dline_testmode.test_no == TEST_STACKER_MOTOR_FWD)
		  || (ex_dline_testmode.test_no == TEST_STACKER))
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

/******************************************************************************/
/*! @brief shutter speed test
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @par            Modify
    - 変更するグローバル変数 ex_dline_testmode
    @return         none
    @exception      none
******************************************************************************/
static u8 phase_shutter_speed_test(void)	/* 0x89 */
{
	/*<<	clear command waiting flag	>>*/
	ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
	/* clear pre phase & request */
	ex_dline_testmode.dline.phase = 0;
	ex_dline_testmode.dline.request = 0;
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && (ex_dline_testmode.test_no == TEST_SHUTTER)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;

	case CMD_ENQ:
		ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x0A;
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = RES_DATA;

		*(ex_usb_write_buffer + 6) = (u8)((ex_shutter_count & 0xff00) >> 8);
		*(ex_usb_write_buffer + 7) = (u8)(ex_shutter_count & 0xff);

		*(ex_usb_write_buffer + 8) = (u8)((ex_dline_testmode.time1 & 0xff00) >> 8);
		*(ex_usb_write_buffer + 9) = (u8)(ex_dline_testmode.time1 & 0xff);

 		ex_usb_write_size = 10;				/* send 10 Byte			*/
		break;

	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		&& ((ex_dline_testmode.test_no == TEST_SHUTTER)
		|| (ex_dline_testmode.test_no == TEST_SHUTTER_CLOSE))
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

static u8 phase_stack_test(void)	/* 0xB0 */
{
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_stacker_count = 0;
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_STACKER;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
			
			ex_dline_testmode.test_result = TEST_NOT_YET;
			
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
#if defined(UBA_RTQ)
	case CMD_ENQ:
		// reset = 1;

		ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x0B;
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = 0x06;	/* エコーバックではなくAckを使用する	*/
		ex_usb_write_size = 11;				/* send 8 Byte			*/

		if( ex_dline_testmode.test_result == TEST_NOT_YET )
		{
		// wait
			*(ex_usb_write_buffer + 6) = TEST_NOT_YET;	/* not yet	*/

			*(ex_usb_write_buffer + 7) =  0x00;
			*(ex_usb_write_buffer + 8) =  0x00;
			*(ex_usb_write_buffer + 9) =  0x00;
			*(ex_usb_write_buffer + 10) = 0x00;

		}
		else if( ex_dline_testmode.test_result == TEST_RESULT_OK )
		{
		// OK
			*(ex_usb_write_buffer + 6) = TEST_RESULT_OK;	/* 0x10	*/

			*(ex_usb_write_buffer + 7) =  (u8)ex_dline_testmode.time1;
			*(ex_usb_write_buffer + 8) =  (u8)(ex_dline_testmode.time1>>8);
			*(ex_usb_write_buffer + 9) =  (u8)ex_dline_testmode.time2;
			*(ex_usb_write_buffer + 10) = (u8)(ex_dline_testmode.time2>>8);
		}
		else
		{
		// NG
			*(ex_usb_write_buffer + 6) = TEST_RESULT_NG;	/* NG	*/

			*(ex_usb_write_buffer + 7) =  0x00;
			*(ex_usb_write_buffer + 8) =  0x00;
			*(ex_usb_write_buffer + 9) =  0x00;
			*(ex_usb_write_buffer + 10) = 0x00;
		}
		break;
#endif // UBA_RTQ
	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && (ex_dline_testmode.test_no == TEST_STACKER)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			ex_stacker_count = 0;
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

#if 1//
static u8 phase_width_test(void)
{
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_centering_count = 0;
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_CENTERING;
			
			ex_dline_testmode.test_result = TEST_NOT_YET;
			
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;

	case 0x44:	/* close */
		/* 生産の初期流動で使用している */
		//if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		if(1)
		{
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_CENTERING_CLOSE;
			ex_dline_testmode.test_result = TEST_NOT_YET;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;

	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && ((ex_dline_testmode.test_no == TEST_CENTERING) || (ex_dline_testmode.test_no = TEST_CENTERING_CLOSE))
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			ex_centering_count = 0;
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}
#else
static u8 phase_width_test(void)
{
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_centering_count = 0;
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_CENTERING;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && (ex_dline_testmode.test_no == TEST_CENTERING)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			ex_centering_count = 0;
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}
#endif


static u8 phase_pb_test(void)
{
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_apb_count = 0;
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_APB;
			ex_dline_testmode.test_result = TEST_NOT_YET;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
		
	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 && (ex_dline_testmode.test_no == TEST_APB)
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			ex_apb_count = 0;
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}


static u8 phase_end_test(void)
{
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_NONE:
		if (((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		 ||  (ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
		 ||  (ex_dline_task_mode == DLINE_MODE_ATEST_ERROR))
		 && (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	case CMD_STOP:
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
		set_response_1data(ACK);
		reset_dline_test();
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

/******************************************************************************/
/*! @brief Test mode request from PC
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
void front_usb_testmode_request( void )
{
	ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_TESTMODE;
	if (!is_test_mode())
	{
		if((u8)ex_front_usb.pc.mess.modeID == (u8)MODE_TESTMODE_REQUEST)
		{
			/* デバッグコマンドのみ応答 */
			switch (ex_front_usb.pc.mess.phase)
			{
			case PHASE_SYSTEM_ERROR:	//for debug
				phase_system_error_info();
				break;
			case PHASE_HW_STATE://for debug
				phase_hw_state();
				break;
			case PHASE_SENSOR_TEST:	/* 0x40 *///SS Factory(Performance) 動作開始時に呼び出しているがTool側はチェックしてない
				phase_sensor_test();
				break;

		#if (_DEBUG_FPGA_FRAM==1) //2023-07-22
			case PHASE_FRAM_LOG://for debug
				phase_fram_log();
				break;
		#endif
			case PHASE_TASK_SEQ:	//for debug
				phase_task_seq();
				break;
			case PHASE_CLOCK:	//for debug
				phase_clock();
				break;
		#if defined(UBA_LOG)
			case PHASE_GET_UBA_LOG_TEST:	//0xC5 //for debug
				phase_get_uba_log_test();
				break;
		#endif
			default:
				set_response_1data(NAK);
			}
		}
	}
/*<<	テストモード及び各モードのフェーズ確認		>>*/
	else if((u8)ex_front_usb.pc.mess.modeID == (u8)MODE_TESTMODE_REQUEST)
	{
		switch (ex_front_usb.pc.mess.phase)
		{
		case PHASE_ACCEPT_TEST_SS:	/* 0x01 */
			phase_accept_test_ss();
			break;
		case PHASE_ACCEPT_TEST:		/* 0x02 */
			phase_accept_test();
			break;
		case PHASE_NO_JUDGE_ACCEPT_TEST_SS:	/* 0x03 */
			phase_no_judge_accept_test_ss();
			break;
		case PHASE_NO_JUDGE_ACCEPT_TEST:	/* 0x04 */
			phase_no_judge_accept_test();
			break;
		case PHASE_NO_JUDGE_REJECT_TEST_SS:	/* 0x05 */
			phase_no_judge_reject_test_ss();
			break;
		case PHASE_NO_JUDGE_REJECT_TEST:	/* 0x06 */
			phase_no_judge_reject_test();
			break;
		case PHASE_COLLECTION_REJECT_CENT_OPEN_TEST_SS: /* 0x07 *///2024-12-01 生産CIS初期流動に使用する
			phase_collection_reject_cent_open_test_ss();
			break;
		case PHASE_SENSOR_TEST:	/* 0x40 *///SS Factory(Performance) 動作開始時に呼び出しているがTool側はチェックしてない
			phase_sensor_test();
			break;
		case PHASE_DIPSWITCH_TEST:	/* 0x90 *///SS  Factory,Performance */
			phase_dipswitch_test();
			break;
		case PHASE_AGING_TEST_SS:	/* 0x70 */
			phase_aging_test_ss();
			break;
		case PHASE_DENOMI_TEST:		/* 0xA0 *///for debug
			phase_denomi_test();
			break;
		/*------------------------------*/
		/* 0x11で起動して、0x80で時間取得 */
		case PHASE_TRANSMIT_MOTOR_TEST:	/* 0x11 *///SS,RTQ  Factory, Performance 起動
			phase_transmit_motor_test();
			break;
		case PHASE_MOTOR_SPEED_TEST:	/* 0x80 *///SS,RTQ  Factory, Performance 時間取得
			phase_motor_speed_test();
			break;
		/*------------------------------*/
		/* 0xB1で起動して、0x84で動作時間取得 */
		case PHASE_PB_TEST:			/* 0xB1*/ //SS  Factory,Performance 起動, RTQ  Performance 起動 
			phase_pb_test();
			break;
		case PHASE_PB_SPEED_TEST:	/* 0x84 */ //SS  Factory,Performance 時間取得,RTQ  Performance 時間取得
			phase_pb_speed_test();
			break;
		/*------------------------------*/
		/* 0xB5で起動して、0x89で動作時間取得 */
		case PHASE_SHUTTER_TEST:		//0xB5	//SS  Factory,Performance 起動, RTQ  Performance 起動 
			phase_shutter_motor_test();
			break;
		case PHASE_SHUTTER_SPEED_TEST:	/* 0x89	*/ //SS  Factory,Performance 時間取得,RTQ  Performance 時間取得
			phase_shutter_speed_test();
			break;
		/*------------------------------*/
		/* 0xB0で起動して、SSは0x86で動作時間取得、RTQは0xB0で動作時間取得 */
		case PHASE_STACK_TEST:			/* 0xB0	*/	//SS  Factory,Performance 起動, RTQ  Factory,Performance 起動,時間取得
			phase_stack_test();			//起動、RTQは時間も
			break;
		case PHASE_STACKER_SPEED_TEST:	/* 0x86	*///SS  Factory,Performance 時間取得(SSのみこっちで時間を取得している)
			phase_stacker_speed_test();	//時間
			break;

		case PHASE_STACKER_MOTOR_TEST:	/* 0x12 */	//*SS,RTQ  Performance　for Free run
			phase_stacker_motor_test();	//起動のみ
			break;

		/*------------------------------*/	
		case PHASE_WIDTH_TEST:			/* 0xB2 *//* 0x13との違いはこっちはcloseがある *///SS  Factory,Performance 起動, RTQ  Performance 起動
			phase_width_test();			//起動とclose
			break;
		case PHASE_WIDTH_SPEED_TEST:	/* 0x82 */	//SS  Factory,Performance 時間取得, RTQ  Performance 時間取得
			phase_width_speed_test();	//時間
			break;

		case PHASE_WIDTH_MOTOR_TEST:	/* 0x13 *//* 0xB2との違い不明*///not use おそらくUBA700では使用してない
			phase_width_motor_test();		//起動のみ おそらくUBA700では使用してない.Closeを使用する為、0xB2を使用
			break;
		/*------------------------------*/	

		case PHASE_SYSTEM_ERROR:	//for debug
			phase_system_error_info();
			break;
		case PHASE_HW_STATE://for debug
			phase_hw_state();
			break;
	#if (_DEBUG_FPGA_FRAM==1) //2023-07-22
		case PHASE_FRAM_LOG://for debug
			phase_fram_log();
			break;
	#endif
		case PHASE_TASK_SEQ:	//for debug
			phase_task_seq();
			break;
		case PHASE_CLOCK:	//for debug
			phase_clock();
			break;
		case PHASE_END:
			phase_end_test();
			break;
	#if defined(_ENABLE_MAG_AGING_EMI)
		case PHASE_MAG_EMI_TEST:		/* 0xE1 */
			phase_mag_emi_test();
			break;
	#endif /* _ENABLE_MAG_AGING_EMI */


		#if defined(UBA_LOG)
		case PHASE_GET_UBA_LOG_TEST:	//0xC5 //for debug
			phase_get_uba_log_test();
			break;
		#endif

		case PHASE_GET_SS_RC_TYPE:	/* 0xE0 *///2025-04-07 //Tool suiteなどで使用
				phase_get_ss_rc_type(); //Tool suiteなどで使用
				break;
		default:
			/*<<	clear command waiting flag	>>*/
			ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
			/*	*/
			set_response_1data(NAK);
			break;
		}
	}
	else
	{
		set_response_1data(NAK);
	}
}

static u8 phase_shutter_motor_test(void)	//0xB5
{
	switch(ex_front_usb.pc.mess.command)
	{
	case CMD_MOTOR_FWD:
		break;
	case CMD_MOTOR_REV:
		break;
	case CMD_RUN:
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_shutter_count = 0;
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_SHUTTER;
			ex_dline_testmode.test_result = TEST_NOT_YET;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;

	case 0x44:	/* close */
		/* 生産の初期流動で使用している */
		if (ex_dline_task_mode == DLINE_MODE_TEST_STANDBY)
		{
			ex_dline_testmode.action = TEST_USB_CONTROL;
			ex_dline_testmode.test_no = TEST_SHUTTER_CLOSE;
			_dline_set_mode(DLINE_MODE_TEST_EXEC);
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 0, 0, 0);
			set_response_1data(ACK);
		}
		else
		{
			set_response_1data(NAK);
		}
		break;

	case CMD_STOP:
		if ((ex_dline_task_mode == DLINE_MODE_TEST_EXEC)
		&& ((ex_dline_testmode.test_no == TEST_SHUTTER)
		   || (ex_dline_testmode.test_no == TEST_SHUTTER_CLOSE))
		&&  (ex_dline_testmode.action == TEST_USB_CONTROL))
		{
			ex_shutter_count = 0;
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
			set_response_1data(ACK);
			reset_dline_test();
		}
		else
		{
			set_response_1data(NAK);
		}
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

#if defined(_ENABLE_MAG_AGING_EMI)
/******************************************************************************/
/*! @brief MAG EMI test
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @par            Modify
    - 変更するグローバル変数 ex_dline_testmode
    @return         none
    @exception      none
******************************************************************************/
static u8 phase_mag_emi_test(void)
{
	/*<<	clear command waiting flag	>>*/
	ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_NONE:
		ex_usb_write_size = 6 + sizeof(ex_mag_emi_result) + sizeof(ex_mag_data);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x00);
		*(ex_usb_write_buffer + 6) = (u8)(ex_mag_emi_result & 0xff);
	/*<--->*/
		memcpy((u8 *)(ex_usb_write_buffer + 7) , (u8 *)ex_mag_data, sizeof(ex_mag_data));
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return(response);
}
#endif /*_ENABLE_MAG_AGING_EMI*/


struct _SYSTEM_ERROR
{
	u16 taskid;
	u16 errno;
	u16 tmsg;
	u16 submsg;
};
extern struct _SYSTEM_ERROR _debug_system_error_val;
static u8 phase_system_error_info(void)//UBA_LOG Main
{
	u32 value = 0;
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_NONE:
		ex_usb_write_size = 48;
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x00);
	/*<--->*/
		memcpy((u8 *)(ex_usb_write_buffer + 6) , (u8 *)&_debug_system_error_val, sizeof(struct _SYSTEM_ERROR));
		memcpy((u8 *)(ex_usb_write_buffer + 6 + 8) , (u8 *)&ex_multi_job, 32);
		memcpy((u8 *)(ex_usb_write_buffer + 6 + 8 + 32) , (u8 *)&ex_main_task_mode1, 1);
		memcpy((u8 *)(ex_usb_write_buffer + 6 + 8 + 32 + 1) , (u8 *)&ex_main_task_mode2, 1);
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}

static u8 phase_hw_state(void)
{
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_NONE:
		ex_usb_write_size = 6 + sizeof(ex_hal_status);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x00);
	/*<--->*/
		memcpy((u8 *)(ex_usb_write_buffer + 6) , (u8 *)&ex_hal_status, sizeof(ex_hal_status));
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
}


#if (_DEBUG_FPGA_FRAM==1) //2023-07-22
#include "js_oswapi.h"
#include "js_spi_fram.h"
#include "fram_drv.h"
u32 _hal_write_fram_debug_log(void)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM); //2023-06-29a
	if(ercd != E_OK)
	{
		program_error();
	}

	spi_buf.buf = (u8 *)&exbk_fram_log;
	spi_buf.addr =  (128 * 1024) - sizeof(exbk_fram_log);
	spi_buf.len = sizeof(exbk_fram_log);
	spi_buf.byte_count = &byte;
	if( (spi_buf.addr + spi_buf.len) > (128 * 1024) )
	{
		ret = ERROR;
	};
	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}

	ercd = sig_sem(ID_SPI_SEM); //2023-06-29a
	if(ercd != E_OK)
	{
		program_error();
	}

	return ret;
}
u32 _hal_read_fram_debug_log(void)
{
	u32 ret = SUCCESS;
	SPI_BUF_INFO spi_buf;

	spi_buf.buf = (u8 *)&exbk_fram_log;
	spi_buf.addr =  (128 * 1024) - sizeof(exbk_fram_log);
	spi_buf.len = sizeof(exbk_fram_log);
	if( (spi_buf.addr + spi_buf.len) > (128 * 1024) )
	{
		ret = ERROR;
	};
	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}
void load_phase_fram_log(void)
{
	_hal_read_fram_debug_log();
}
void save_phase_fram_log(void) //use
{
	u32 gpio_ext_porta, stat;
	if(ex_fram_log_enable == 1)  //テストモード開始後1
	{
		stat = (u32)*(u32 *)(0xFF706000);
		gpio_ext_porta = (u32)*(u32 *)(0xFF706850);
		exbk_fram_log.main_task_mode1 = ex_main_task_mode1;
		exbk_fram_log.main_task_mode2 = ex_main_task_mode2;
		//exbk_fram_log.dummy = 0;
		exbk_fram_log.stat = (u16)(0xFFFF & stat);
		exbk_fram_log.port = (u16)(0xFFFF & gpio_ext_porta);

		_hal_write_fram_debug_log();
	}
}
void save_debug_fram_log(int num)
{
	u32 gpio_ext_porta, stat;
	stat = (u32)*(u32 *)(0xFF706000);
	gpio_ext_porta = (u32)*(u32 *)(0xFF706850);

	if(ex_fram_log_enable == 1)  //テストモード開始後1
	{
		exbk_fram_log.main_task_mode1 = ex_main_task_mode1;	//2023-06-29a 
		exbk_fram_log.main_task_mode2 = ex_main_task_mode2;	//2023-06-29a 

		exbk_fram_log.dummy = num;;
		//exbk_fram_log.dummy = (u16)(0xFFFF & alt_fpga_gpi_read(0xFFFFFFFF));
		exbk_fram_log.stat = (u16)(0xFFFF & stat);
		exbk_fram_log.port = (u16)(0xFFFF & gpio_ext_porta);
		_hal_write_fram_debug_log();
	}
}
u8 phase_fram_log(void)
{
	u32 gpio_ext_porta, stat;
	stat = (u32)*(u32 *)(0xFF706000);
	gpio_ext_porta = (u32)*(u32 *)(0xFF706850);
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_NONE:
		if(ex_fram_log_enable == 1) //テストモード開始後1
		{
			exbk_fram_log.stat = (u16)(0xFFFF & stat);
			exbk_fram_log.port = (u16)(0xFFFF & gpio_ext_porta);
		}
		ex_usb_write_size =  6 + sizeof(exbk_fram_log);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x00);
		/*<--->*/
		memcpy((u8 *)(ex_usb_write_buffer + 6) , (u8 *)&exbk_fram_log, sizeof(exbk_fram_log));
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;
};
#endif


#if (_DEBUG_FPGA_FRAM==1) //2023-07-22
void save_stack_fram_log(u16 seq) //use
{
	u32 gpio_ext_porta, stat;
	if(ex_fram_log_enable == 1)  //テストモード開始後1
	{
		exbk_fram_log.main_task_mode1 = ex_main_task_mode1;
		exbk_fram_log.main_task_mode2 = ex_main_task_mode2;

		exbk_fram_log.dummy = seq;

		exbk_fram_log.stat = (u16)(0xFFFF & stat);
		exbk_fram_log.port = (u16)(0xFFFF & gpio_ext_porta);


		_hal_write_fram_debug_log();
	}
}
#endif

#define TASK_COUN 64

static u8 phase_task_seq(void)
{
	int task;
	u16 dummy = 0;
	ex_usb_write_size = 6 + 2 * TASK_COUN;				/* send 10 Byte			*/
	*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
	*(ex_usb_write_buffer + 1) = 0x00;
	*(ex_usb_write_buffer + 2) = 0x06;
	*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
	*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
	*(ex_usb_write_buffer + 5) = ex_front_usb.pc.mess.command;		/* set illegal status	*/

	//0	ID_MAIN_TASK	ex_main_task_mode
	task = 0;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_main_task_mode, sizeof(u16));
	//1	ID_DLINE_TASK,	ex_dline_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_dline_task_mode, sizeof(u16));
	//2	ID_CLINE_TASK,	ex_cline_task_mode
	task++;
	dummy = ex_cline_status_tbl.line_task_mode;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &dummy, sizeof(u16));
	//3	ID_TIMER_TASK,	ex_timer_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_timer_task_mode, sizeof(u16));
	//4	ID_DISPLAY_TASK,	ex_display_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_display_task_mode, sizeof(u16));
	//5	ID_IDLE_TASK,	0x0000
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &dummy, sizeof(u16));
	//6	ID_BEZEL_TASK,	ex_bezel_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_bezel_task_mode, sizeof(u16));
	//7	ID_OTG_TASK,	ex_otg_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_otg_task_mode, sizeof(u16));
	//8	ID_SUBLINE_TASK,	ex_subline_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_subline_task_mode, sizeof(u16));
	//9	ID_DIPSW_TASK,	ex_dipsw_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_dipsw_task_mode, sizeof(u16));
	//10	ID_SENSOR_TASK,	ex_sensor_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_sensor_task_mode, sizeof(u16));
	//11	ID_FRAM_TASK,	ex_fram_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_fram_task_mode, sizeof(u16));
	//12	ID_MOTOR_TASK,	ex_motor_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_motor_task_mode, sizeof(u16));
	//13	ID_MGU_TASK,	ex_mgu_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_mgu_task_mode, sizeof(u16));
	//14	ID_DISCRIMINATION_TASK,	ex_discrimination_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_discrimination_task_mode, sizeof(u16));
	//16	ID_RFID_TASK,	ex_rfid_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_rfid_task_mode, sizeof(u16));
	//17	ID_ICB_TASK,	ex_icb_task_seq
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_icb_task_seq, sizeof(u16));
	//18	ID_FEED_TASK,	ex_feed_task_seq
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_feed_task_seq, sizeof(u16));
	//19	ID_CENTERING_TASK,	ex_centering_task_seq
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_centering_task_seq, sizeof(u16));
	//20	ID_APB_TASK,	ex_apb_task_seq
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_apb_task_seq, sizeof(u16));
	//21	ID_STACKER_TASK,	ex_stacker_task_seq
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_stacker_task_seq, sizeof(u16));
	//22	ID_UART01_CB_TASK,	ex_uart01_cb_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_uart01_cb_task_mode, sizeof(u16));
	//23	ID_USB0_CB_TASK,	ex_usb0_cb_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_usb0_cb_task_mode, sizeof(u16));
	//24	ID_USB2_CB_TASK,	ex_usb2_cb_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_usb2_cb_task_mode, sizeof(u16));
	//25	ID_FUSB_DET_TASK,	ex_fusb_det_task
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_fusb_det_task, sizeof(u16));
	//27	ID_POWER_TASK,	ex_power_task_mode
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_power_task_mode, sizeof(u16));
	//29	ID_SHUTTER_TASK,	ex_shutter_task_seq
	task++;
	memcpy(ex_usb_write_buffer + 6 + sizeof(u16) * task, &ex_shutter_task_seq, sizeof(u16));
	//30	-	-

	return 0;
}
static u8 phase_clock(void)
{
	u8 index;
	switch (ex_front_usb.pc.mess.command)
	{
	case CMD_NONE:
		// グローバル変数clock_frequencyにクロックを格納
		get_pll_info();

		index = ex_usb_read_buffer[6];

		ex_usb_write_size = 6 + 1 + 4;				/* send 11 Byte			*/
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = 0x00;
		*(ex_usb_write_buffer + 2) = 0x06 + 5;
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = ex_front_usb.pc.mess.command;		/* set illegal status	*/
		*(ex_usb_write_buffer + 6) = index;
		*(ex_usb_write_buffer + 7) = (u8)((clock_frequency[index] & 0xff));
		*(ex_usb_write_buffer + 8) = (u8)((clock_frequency[index] >> 8) & 0xff);
		*(ex_usb_write_buffer + 9) = (u8)((clock_frequency[index] >> 16) & 0xff);
		*(ex_usb_write_buffer + 10) = (u8)((clock_frequency[index] >> 24) & 0xff);
		break;
	case CMD_STOP:
		set_response_1data(ACK);
		break;
	default:
		set_response_1data(NAK);
		break;
	}

	return 0;
}

#if defined(UBA_LOG)
extern void make_position_uba_data(void); /* 2022-06-15 */
static u8 phase_get_uba_log_test(void)//UBA_LOG Main
{

	u16 send_size = 0;

	switch (ex_front_usb.pc.mess.command)
	{
	case 0x21:	/* backup_log *///ID-003 パワーリカバリ 残す
		/* backup_log_read */
		ex_usb_write_size = 6 + sizeof(ex_free_uba);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x21);
	/*<--->*/
		memcpy((u8 *)(ex_usb_write_buffer + 6) , (u8 *)&ex_free_uba[0], sizeof(ex_free_uba));
		break;

	case 0x25:	/* backup_log *///2023-12-11 残す
		ex_usb_write_size = 6 + sizeof(bkex_status_tbl_buff);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x25);
		memcpy((u8 *)(ex_usb_write_buffer + 6) , (u8 *)&bkex_status_tbl_buff[0], sizeof(bkex_status_tbl_buff));
		break;


	case 0x30:	//2025-02-13 温度 + 収納テーブル 残す
		ex_usb_write_size = 6 + 4;
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0xFF);
		*(ex_usb_write_buffer + 6) = (u8)(ex_temperature.cis_a & 0xff);
		*(ex_usb_write_buffer + 7) = (u8)(ex_temperature.cis_b & 0xff);
		*(ex_usb_write_buffer + 8) = (u8)(ex_temperature.outer & 0xff);
		*(ex_usb_write_buffer + 9) = (u8)(motor_limit_stacker_table_index & 0xff);
		break;
		
	case 0x31:	/* backup_log *///2025-02-13 skew 残す
		ex_usb_write_size = 6 + 1;
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0xFF);

		*(ex_usb_write_buffer + 6) = (u8)(ex_rc_skew_pulse & 0xff);
		break;

	case 0x33: //RTQへ通知した搬送速度 残す
		ex_usb_write_size = 6 + 4;
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0xFF);

	#if defined(UBA_RTQ)
		*(ex_usb_write_buffer + 6) = (u8)((uba_feed_speed_fwd & 0xff00) >> 8); //RTQのイニシャル550ｍｍに近い値をRTQへ通知情報
		*(ex_usb_write_buffer + 7) = (u8)(uba_feed_speed_fwd & 0xff);
		*(ex_usb_write_buffer + 8) = (u8)((uba_feed_speed_rev & 0xff00) >> 8);
		*(ex_usb_write_buffer + 9) = (u8)(uba_feed_speed_rev & 0xff);
	#endif
		break;


	case 0x41: //ID-003用 残す
		ex_usb_write_size = 6 + sizeof(ex_free_uba_data1);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x41);
	/*<--->*/
		memcpy((u8 *)(ex_usb_write_buffer + 6) , (u8 *)&ex_free_uba_data1[0], sizeof(ex_free_uba_data1));
		break;

	case 0x42: //ID-003用 残す
		ex_usb_write_size = 6 + sizeof(ex_free_uba_data2);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x42);
	/*<--->*/
		memcpy((u8 *)(ex_usb_write_buffer + 6) , (u8 *)&ex_free_uba_data2[0], sizeof(ex_free_uba_data2));
		break;

	case 0x43: //ID-003用 残す
		ex_usb_write_size = 6 + sizeof(ex_free_uba_data3);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x43);
	/*<--->*/
		memcpy((u8 *)(ex_usb_write_buffer + 6) , (u8 *)&ex_free_uba_data3[0], sizeof(ex_free_uba_data3));
		break;
#if defined(UBA_RTQ)//#if defined(RFID_RECOVER)//#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)

	extern u8 rc_rfid_data[255];
	case 0x45:	//HeadのRFID情報 残すかも
		ex_usb_write_size = 6 + sizeof(rc_rfid_data);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x45);
	/*<--->*/
		memcpy((u8 *)(ex_usb_write_buffer + 6) , (u8 *)&rc_rfid_data[0], sizeof(rc_rfid_data));
		break;

	case 0x46:	//HeadのRFID情報 残すかも
		ex_usb_write_size = 6 + sizeof(Smrtdat_fram);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x46);
	/*<--->*/
		memcpy((u8 *)(ex_usb_write_buffer + 6) , (u8 *)&Smrtdat_fram, sizeof(Smrtdat_fram));
		break;
		
	extern	u8 rc_rfid_data_read_rtq[255];
	extern u8 *rc_rfid_read_buff;
	case 0x47:	//RTQのRFIDのRead 2025-07-23
		ex_usb_write_size = 6 + 0;
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x47);
	/*<--->*/
	//	*(ex_usb_write_buffer + 6) = (u8)ex_rtq_rfid_write_disable;
		rc_rfid_read_buff = &rc_rfid_data_read_rtq[0]; //保存先アドレスを指定する必要あり
	//	_main_send_msg(ID_RC_MBX, TMSG_RC_RFID_READ_REQ, RFID_RUN, 0, 8, 0); //8byteなのに10byte帰ってくるような
		_main_send_msg(ID_RC_MBX, TMSG_RC_RFID_READ_REQ, RFID_RUN, 0, 40, 0); //8byteなのに10byte帰ってくるような

		//	_dline_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_DATA_WAIT, 0, 0, 0);
		break;

	case 0x48:	//RTQのRFID入手 2025-07-23
		ex_usb_write_size = 6 + 40;
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x48);
	/*<--->*/
		memcpy((u8 *)(ex_usb_write_buffer + 6) , (u8 *)&rc_rfid_data_read_rtq[0], 40);
		break;
	#endif //end (UBA_RTQ)

	case 0x49: //パワーリカバリ用のフラグ遷移確認用 残す予定 SS RTQ
		ex_usb_write_size = 6 + 1 + sizeof(ex_back_log);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0xFF);
	/*<--->*/
		*(ex_usb_write_buffer + 6) = ex_back_log_index;
		memcpy((u8 *)(ex_usb_write_buffer + 7) , (u8 *)&ex_back_log, sizeof(ex_back_log));
		break;

	case 0x50:	//パワーリカバリ用のフラグ遷移確認用 残す予定 SS RTQ
		set_response_1data(ACK);
		ex_back_log_index = 0;
		break;


	case 0x51: //mode確認用 残す予定
		ex_usb_write_size = 6 + 1 + sizeof(ex_mode_log);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0xFF);
	/*<--->*/
		*(ex_usb_write_buffer + 6) = ex_mode_log_index;
		memcpy((u8 *)(ex_usb_write_buffer + 7) , (u8 *)&ex_mode_log, sizeof(ex_mode_log));
		break;

	case 0x52:	//mode確認用 残す予定
		set_response_1data(ACK);
		ex_mode_log_index = 0;
		break;

	#if defined(EEPROM_TEST)
	case 0x54:	//2025-08-05
		set_response_1data(ACK);
		_dline_send_msg(ID_MGU_MBX, TMSG_MGU_WRITE_REQ, MGU_LOG, 0, 0, 0);
		break;
	#endif


	#if defined(UBA_RTQ_AZ_LOG) 
	case 0x50: //2023-09-05 残す
		ex_usb_write_size =  6 + sizeof(ex_fram_uba);
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = (u8)(0x00);
		/*<--->*/
		memcpy((u8 *)(ex_usb_write_buffer + 6) , (u8 *)&ex_fram_uba, sizeof(ex_fram_uba));
		break;
	#endif

	case 0x70: //RTQ搬送速度調査
	// 	ex_usb_write_size = 6 + 4;
	// 	*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
	// 	*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
	// 	*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
	// 	*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
	// 	*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
	// 	*(ex_usb_write_buffer + 5) = (u8)(0xFF);
	// /*<--->*/
	// 	*(ex_usb_write_buffer + 6) = (u8)((uba_feed_speed_fwd & 0xff00) >> 8);
	// 	*(ex_usb_write_buffer + 7) = (u8)(uba_feed_speed_fwd & 0xff);
	// 	*(ex_usb_write_buffer + 8) = (u8)((uba_feed_speed_rev & 0xff00) >> 8);
	// 	*(ex_usb_write_buffer + 9) = (u8)(uba_feed_speed_rev & 0xff);
		break;

	case 0x07:	/* 2022-06-15 */// 残す
		make_position_uba_data();
		break;
	default:
		set_response_1data(NAK);
		break;
	}
	return 0;


}


void make_position_uba_data(void) //2022-06-15
{
	u16 send_size = 0;

	*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
	*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
	*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
	*(ex_usb_write_buffer + 5) = (u8)(0x04);

	*(ex_usb_write_buffer + 6) = ex_position_da.entrance;
	send_size += sizeof(ex_position_da.entrance);

	*(ex_usb_write_buffer + 6 + send_size) = ex_position_da.centering;
	send_size += sizeof(ex_position_da.centering);

	*(ex_usb_write_buffer + 6 + send_size) = ex_position_da.apb_in;
	send_size += sizeof(ex_position_da.apb_in);

	*(ex_usb_write_buffer + 6 + send_size) = ex_position_da.apb_out;
	send_size += sizeof(ex_position_da.apb_out);

	*(ex_usb_write_buffer + 6 + send_size) = ex_position_da.exit;
	send_size += sizeof(ex_position_da.exit);

	*(ex_usb_write_buffer + 6 + send_size) = ex_position_ga;
	send_size += sizeof(ex_position_ga);

	ex_usb_write_size = 6 + send_size;
	*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size & 0xff00) >> 8);
	*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);

}
#endif

//		case PHASE_GET_SS_RC_TYPE:
void phase_get_ss_rc_type(void) //Tool suiteなどで使用
{

/*<<	clear command waiting flag	>>*/
	ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
	*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
	*(ex_usb_write_buffer + 1) = 0x00;
	*(ex_usb_write_buffer + 2) = 0x07;
	*(ex_usb_write_buffer + 3) = ex_front_usb.pc.mess.modeID;
	*(ex_usb_write_buffer + 4) = ex_front_usb.pc.mess.phase;
	*(ex_usb_write_buffer + 5) = 0x06;	/* エコーバックではなくAckを使用する	*/

#if defined(UBA_RTQ)
	*(ex_usb_write_buffer + 6) = 0x10;	/* RC-TQ	*/
#else
	*(ex_usb_write_buffer + 6) = 0x00;	/* SS/SU	*/
#endif

	ex_usb_write_size = 7;				/* send 7 Byte			*/

}

#if defined(UBA_RTQ_AZ_LOG)  //2023-09-05
#include "js_oswapi.h"
#include "js_spi_fram.h"
#include "fram_drv.h"

u32 _hal_write_fram_debug_log_uba(void)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM); //2023-06-29a
	if(ercd != E_OK)
	{
		program_error();
	}

	spi_buf.buf = (u8 *)&ex_fram_uba;
	spi_buf.addr = 100000;
	spi_buf.len = sizeof(ex_fram_uba);
	spi_buf.byte_count = &byte;

	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}

	ercd = sig_sem(ID_SPI_SEM); //2023-06-29a
	if(ercd != E_OK)
	{
		program_error();
	}

	return ret;
}

u32 _hal_read_fram_debug_log_uba(void)
{
	u32 ret = SUCCESS;
	SPI_BUF_INFO spi_buf;

	spi_buf.buf = (u8 *)&ex_fram_uba;
	spi_buf.addr = 100000;
	spi_buf.len = sizeof(ex_fram_uba);

	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}

#endif



/* EOF */
