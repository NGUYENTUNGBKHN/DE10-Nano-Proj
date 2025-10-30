/******************************************************************************/
/*! @addtogroup Group2
    @file       rc_cmd.c
    @brief      
    @date       2024/05/22
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    Japan CashMachine Co, Limited. All rights reserved.
******************************************************************************/
#if defined(UBA_RTQ)
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "sub_functions.h"
#include "hal.h"
#include "hal_uart0.h"
#include "rc_task.h"

#include "rc_operation.h"
#include "if_rc.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

#ifdef _ENABLE_JDL
#include "jdl_conf.h"
#endif /* _ENABLE_JDL */

//#if defined(UBA_RTQ_ICB)
u8 rc_rfid_data[255];
#if defined(UBA_LOG)
	u8 rc_rfid_data_read_rtq[255];	//2025-07-23 for debug
#endif
u8 rc_last_send;
u8 ex_rc_rfid_src_req; // determine clearly source request connect to rfid 
//#endif


static void rc_task_log_jdl()
{
	jdl_add_trace(ID_RC_TASK, ((ex_rc_task_seq >> 8) & 0xFF), 
				(ex_rc_task_seq & 0xFF), (rc_msg.tmsg_code & 0xFF), (rc_msg.arg1 & 0xFF), (rc_msg.arg2 & 0xFF));

}

/*********************************************************************//**
 * @brief status request sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_status_request(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;				/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_STATUS_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief reset command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_reset_command(void)
{
	ex_rc_error_status = 0;
	ex_rc_warning_status = 0;

	_rc_tx_buff.start_code	= RC_SYNC;				/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_RESET_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* reset type	*/
	
	ex_rc_rewait_rdy_flg = REWAIT_RDY_OFF;
	// ex_rc_rewait_rdy_exec_command = 0; //UBA_MUST
}

/*********************************************************************//**
 * @brief ger boot version command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_get_boot_version_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;						/* header		*/
	_rc_tx_buff.length		= 0x06;							/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_BOOT_VERSION_REQ;
	_set_head_status();										/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief set recycle setting command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_set_recycle_setting_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;						/* header		*/
	_rc_tx_buff.length		= 0x26;							/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_RECYCLE_SETTING_REQ;
	_set_head_status();										/* sst1 - sst32	*/

	/* 初期化かどうかを通知 */
	if(rc_msg.arg1 == 0xFF)
	{
		_rc_tx_buff.sst[4] = rc_msg.arg1;
	}
#if defined(RC_TASK_DEBUG_NO_BILL)
	rc_test_setup_recycle();
#endif 	
	memcpy((u8 *)&_rc_tx_buff.data[0], (u8 *)&RecycleSettingInfo.DenomiInfo[0].BoxNumber, 8);			/* RC-Twin drum1	*/
	memcpy((u8 *)&_rc_tx_buff.data[8], (u8 *)&RecycleSettingInfo.DenomiInfo[1].BoxNumber, 8);			/* RC-Twin drum2	*/
	memcpy((u8 *)&_rc_tx_buff.data[16], (u8 *)&RecycleSettingInfo.DenomiInfo[2].BoxNumber, 8);			/* RC-Quad drum1	*/
	memcpy((u8 *)&_rc_tx_buff.data[24], (u8 *)&RecycleSettingInfo.DenomiInfo[3].BoxNumber, 8);			/* RC-Quad drum1	*/
}

/*********************************************************************/
/**
 * @brief get version command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_get_version_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;						/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_VERSION_REQ;
	_set_head_status();									/* sst1 - sst32	*/
}

/*********************************************************************/
/**
 * @brief get recycle setting command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_get_recycle_setting_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_GET_RECYCLE_SETTING_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief reset command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_reset_skip_command(void) //UBA700では対応していない、待機時の静電気によるリカバリ用
{
	ex_rc_error_status = 0;
	ex_rc_warning_status = 0;

	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_RESET_SKIP_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}


/*********************************************************************//**
 * @brief reject command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_reject_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x08;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_REJECT_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* unit			*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* drum			*/
}

/*********************************************************************//**
 * @brief pause command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_pause_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_PAUSE_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* pause status	*/
}


/*********************************************************************//**
 * @brief get version command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_resume_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_RESUME_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************/
/**
 * @brief WU reset command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_read_edition_no_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_READ_EDITION_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* mode	*/
}

/*********************************************************************//**
 * @brief rc box search command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_prefeed_stack_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_PREFEED_STACK_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief set mode command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_set_mode_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x08;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_MODE_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* mode	 		*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* mode	 		*/
}


/*********************************************************************//**
 * @brief WU reset command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_read_mainte_serial_no_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_READ_MAINTE_SERIAL_NO_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief get dipsw command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_get_dipsw_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_GET_DIPSW_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief set state command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_set_state_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_STATE_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* state	 	*/

	if(rc_msg.arg1 == RC_STATE_IDLE || rc_msg.arg1 == RC_STATE_ABNORMAL)
	{
		pol_time = 100;			/* set polling timer 100msec */
	}
	else
	{
//		pol_time = 100;			/* set polling timer 100msec */
		pol_time = 30;			/* set polling timer 30msec */
	}
}

/*********************************************************************//**
 * @brief rc last feed cashbox command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_last_feed_cashbox_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_LAST_FEED_CASHBOX_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief rc box search command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_box_search_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x08;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_BOX_SEARCH_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* unit			*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* dirction		*/
}

/*********************************************************************//**
 * @brief rc billback drum payout command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_billback_drum_pauout_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_BILLBACK_DRUM_PAYOUT_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* pause status	*/
}

/*********************************************************************//**
 * @brief rc feed box drum payout command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_feed_box_drum_pauout_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_FEED_BOX_DRUM_PAYOUT_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* pause status	*/
}

/*********************************************************************//**
 * @brief rc force stack drum command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_force_stack_drum_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_FORCE_STACK_DRUM_REQ;
	_set_head_status();								/* sst1 - sst32	*/

	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* box number	*/
}

/*********************************************************************//**
 * @brief rc billback command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_billback2_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_BILLBACK_REQ;
	_set_head_status();								/* sst1 - sst32	*/

	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* box number	*/
}

/*********************************************************************//**
 * @brief set head status
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _set_head_status_enc_buff()
{
	/* 情報が確定するまでとりあえず"0x00"を設定しておく */
	_rc_tx_enc_buff.sst[0] = 0x00;

	if(ex_rc_configuration.board_type == RC_NEW_BOARD)
	{
		_rc_tx_enc_buff.sst[0] |= 0x01;
	}
	if(is_rc_rs_unit())
	{
		_rc_tx_enc_buff.sst[0] |= 0x02;
	}
	if(ex_rc_configuration.rfid_module == CONNECT_RFID)
	{
		_rc_tx_enc_buff.sst[0] |= 0x04;
	}

	_rc_tx_enc_buff.sst[1] = 0xFF;
#if 1 //2025-01-20
	/* TEST type *///必要か不明だが通常と同じにしておく
	if (ex_main_test_no == TEST_RC_AGING)
	{
		_rc_tx_enc_buff.sst[2] = DIPSW1_RC_AGING_TEST;
	}
	else if (ex_main_test_no == TEST_RC_AGING_FACTORY)
	{
		_rc_tx_enc_buff.sst[2] = DIPSW1_RC_AGING_FACTORY;
	}
	else
	{
		_rc_tx_enc_buff.sst[2] = 0x00;
	}	

	if(SENSOR_ENTRANCE)
	{
		_rc_tx_enc_buff.sst[3] = 0x01;
	}
	else
	{
		_rc_tx_enc_buff.sst[3] = 0x00;
	}
#else
	_rc_tx_enc_buff.sst[2] = 0x00;
	_rc_tx_enc_buff.sst[3] = 0x00;
#endif
	_rc_tx_enc_buff.sst[4] = 0x00;
}

/*********************************************************************//**
 * @brief sensor active command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_sensor_active_command(void)
{
	/* None */
}


/*********************************************************************//**
 * @brief sensor condition command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_sensor_condition_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_SENSOR_CONDITION_REQ;
	_set_head_status();								/* sst1 - sst32	*/

}

/*********************************************************************//**
 * @brief rc drum gap adj command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_drum_gap_adj_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_DRUM_GAP_ADJ_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* pause status	*/
}

/*********************************************************************//**
 * @brief set mode command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_error_count_clear_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_ERROR_COUNT_CLEAR_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief WU reset command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_diag_end_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_DIAG_END_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}


/*********************************************************************//**
 * @brief WU reset command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_diag_mot_fwd_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_DIAG_MOT_FWD_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief set del command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_set_del_command(void)
{
	u8 cnt;

	switch (rc_msg.arg1)
	{
	case TMSG_RC_ENC_KEY:
		_rc_tx_enc_buff.start_code = 0x24; 			/* header		*/
		_rc_tx_enc_buff.length = 0x0F;	   			/* length		*/
		_rc_tx_enc_buff.del_cmd = CMD_RC_DEL_REQ;
		_rc_tx_enc_buff.cmd = CMD_RC_ENC_KEY;
		_set_head_status_enc_buff(); 				/* sst1 - sst32	*/
		for (cnt = 0; cnt < 8; cnt++)
		{
			_rc_tx_enc_buff.data[cnt] = ex_encryption_key[cnt];
		}
		break;
	case TMSG_RC_ENC_NUM:
		_rc_tx_enc_buff.start_code = 0x24; 			/* header		*/
		_rc_tx_enc_buff.length = 0x08;	   			/* length		*/
		_rc_tx_enc_buff.del_cmd = CMD_RC_DEL_REQ;
		_rc_tx_enc_buff.cmd = CMD_RC_ENC_NUM;
		_set_head_status_enc_buff(); 				/* sst1 - sst32	*/
		_rc_tx_enc_buff.data[0] = ex_encryption_number;
		break;
	case TMSG_RC_PAYOUT_REQ:
		_rc_tx_enc_buff.start_code = 0x24; 			/* header		*/
		_rc_tx_enc_buff.length = 0x0B;	   			/* length		*/
		_rc_tx_enc_buff.del_cmd = CMD_RC_DEL_REQ;
		_rc_tx_enc_buff.cmd = CMD_RC_PAYOUT_REQ;
		_set_head_status_enc_buff();		   		/* sst1 - sst32	*/
		_rc_tx_enc_buff.data[0] = rc_msg.arg2; 		/* drum			*/
		_rc_tx_enc_buff.data[1] = rc_msg.arg3; 		/* remain count	*/
		_rc_tx_enc_buff.data[2] = ex_encryption_number;
		_rc_tx_enc_buff.data[3] = ~(ex_encryption_number);

		ex_last_encryption_cmd = CMD_RC_PAYOUT_REQ;
		break;
	default:
		break;
	}
}

/*********************************************************************//**
 * @brief display command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_display_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x0A;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_DISPLAY_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* mode			*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* color		*/
	_rc_tx_buff.data[2]		= rc_msg.arg3;			/* count		*/
	_rc_tx_buff.data[3]		= rc_msg.arg4;			/* unit			*/
}

/*********************************************************************//**
 * @brief sw clear command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
//static void rc_sw_clear_command(void)
void rc_sw_clear_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_SW_CLEAR_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief get motor speed command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_get_motor_speed_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_GET_MOTOR_SPEED_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* motor	 	*/
}

/*********************************************************************//**
 * @brief feed command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_feed_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x08;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_FEED_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* unit			*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* operation	*/
}

/*********************************************************************//**
 * @brief payout command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_payout_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x08;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_PAYOUT_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* drum			*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* remain count	*/
}

/*********************************************************************//**
 * @brief collect command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_collect_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x08;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_COLLECT_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* drum			*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* remain count	*/
}

/*********************************************************************//**
 * @brief error clear command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_error_clear_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_ERROR_CLEAR_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief error detail command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_get_error_detail_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_ERROR_DETAIL_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief set recycle current count setting command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_set_current_count_setting_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x08;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_SET_CURRENT_COUNT_REQ;
	_set_head_status();								/* sst1 - sst32	*/

	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* box number	*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* count		*/
}

/*********************************************************************//**
 * @brief WU reset command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_wu_reset_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_WU_RESET_REQ;
	_set_head_status();								/* sst1 - sst32	*/

}

/*********************************************************************//**
 * @brief cancel command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_cancel_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_CANCEL_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief retry bill direction command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_retry_bill_dir_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x08;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_RETRY_BILL_DIR_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* state		*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* state		*/
}

/*********************************************************************//**
 * @brief rc feed box command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_feed_box_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x08;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_FEED_BOX_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* state		*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* state		*/
}

/*********************************************************************//**
 * @brief WU reset command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_write_serial_no_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x1E;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_WRITE_SERIAL_NO_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= write_serailno_data.fram_param;	/* fram_type	*/
	memcpy((u8 *)&_rc_tx_buff.data[1],  (u8 *)&write_serailno_data.version[0],   sizeof(write_serailno_data.version));		// 2byte
	memcpy((u8 *)&_rc_tx_buff.data[3],  (u8 *)&write_serailno_data.date[0], 	  sizeof(write_serailno_data.date));			// 8byte
	memcpy((u8 *)&_rc_tx_buff.data[11], (u8 *)&write_serailno_data.serial_no[0], sizeof(write_serailno_data.serial_no));		// 12byte
	_rc_tx_buff.data[23]	= rc_msg.arg1;			/* mode	*/
}
/*********************************************************************//**
 * @brief WU reset command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_read_serial_no_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_READ_SERIAL_NO_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* mode	*/
}


/*********************************************************************//**
 * @brief WU reset command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_write_mainte_serial_no_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x1E;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_WRITE_MAINTE_SERIAL_NO_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= write_serailno_data.fram_param;	/* fram_type	*/
	memcpy((u8 *)&_rc_tx_buff.data[1],  (u8 *)&write_serailno_data.version[0],   sizeof(write_serailno_data.version));		// 2byte
	memcpy((u8 *)&_rc_tx_buff.data[3],  (u8 *)&write_serailno_data.date[0], 	  sizeof(write_serailno_data.date));			// 8byte
	memcpy((u8 *)&_rc_tx_buff.data[11], (u8 *)&write_serailno_data.serial_no[0], sizeof(write_serailno_data.serial_no));		// 12byte
	_rc_tx_buff.data[23]	= rc_msg.arg1;			/* mode	*/
}


/*********************************************************************//**
 * @brief WU reset command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_write_edition_no_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x0A;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_WRITE_EDITION_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	memcpy((u8 *)&_rc_tx_buff.data[0], (u8 *)&write_editionno_data.main[0], sizeof(write_editionno_data.main)); // 1byte
	memcpy((u8 *)&_rc_tx_buff.data[1], (u8 *)&write_editionno_data.twin[0], sizeof(write_editionno_data.twin)); // 1byte
	memcpy((u8 *)&_rc_tx_buff.data[2], (u8 *)&write_editionno_data.quad[0], sizeof(write_editionno_data.quad)); // 1byte

	_rc_tx_buff.data[3]	= rc_msg.arg1;			/* mode	*/
}


/*********************************************************************//**
 * @brief set mode command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_fram_read_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x0A;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_FRAM_READ_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* offset	 	*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* offset	 	*/
	_rc_tx_buff.data[2]		= rc_msg.arg3;			/* offset	 	*/
	_rc_tx_buff.data[3]		= rc_msg.arg4;			/* offset	 	*/
}


/*********************************************************************//**
 * @brief set mode command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_fram_check_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_FRAM_CHECK_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief set mode command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_start_sensor_adj_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_START_ADJ_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* mode	 	*/
}


/*********************************************************************//**
 * @brief set mode command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_read_sensor_adj_data_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_GET_ADJ_DATA_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief set mode command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_drum_tape_pos_adj_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_DRUM_TAPE_ADJ_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* drum	 	*/
}

/*********************************************************************//**
 * @brief set mode command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_sens_adj_write_fram_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0xA0;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_SENS_ADJ_WRITE_FRAM_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* mode	 	*/

	memcpy((u8 *)&_rc_tx_buff.data[1], (u8 *)&ex_rc_adj_data, sizeof(ex_rc_adj_data));		/* ex_rc_adj_data 	*/
}


/*********************************************************************//**
 * @brief set mode command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_sens_adj_read_fram_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_SENS_ADJ_READ_FRAM_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* mode	 	*/
}


/*********************************************************************//**
 * @brief set mode command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_perform_test_write_fram_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;									/* header		*/
	if (ex_rc_configuration.board_type == RC_OLD_BOARD)
	{
		_rc_tx_buff.length		= 0x2A;										/* length		*/
	}
	else
	{
		_rc_tx_buff.length		= 0x2C;										/* length		*/
	}	
	_rc_tx_buff.cmd			= CMD_RC_PERFORM_TEST_WRITE_FRAM_REQ;
	_set_head_status();													/* sst1 - sst32	*/
	memcpy((u8 *)&_rc_tx_buff.data[0], (u8 *)&ex_perform_test_data, sizeof(ex_perform_test_data));		/* ex_perform_test_data 	*/
}


/*********************************************************************//**
 * @brief set mode command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_perform_test_read_fram_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_PERFORM_TEST_READ_FRAM_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

/*********************************************************************//**
 * @brief polling change command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_poll_change_command(void)
{
	if(rc_msg.arg1 == 1)
	{
		pol_time = 30;			/* set polling timer 30msec */
	}
	else
	{
		pol_time = 100;			/* set polling timer 100msec */
	}
}

//#if defined(RC_BOARD_GREEN)
static void rc_get_configuration_command()
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_GET_CONFIGURATION_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

static void rc_get_pos_onoff_command()
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x06;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_GET_POS_ONOFF_REQ;
	_set_head_status();								/* sst1 - sst32	*/
}

static void rc_set_pos_gain_command()
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x09;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_SET_POS_GAIN_REQ;
	_set_head_status();								/* sst1 - sst32	*/

	memcpy((u8 *)&_rc_tx_buff.data[0], (u8 *)&ex_rc_new_adjustment_data.gain[0], 3);
}

static void rc_set_pos_da_command()
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x20;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_SET_POS_DA_REQ;
	_set_head_status();								/* sst1 - sst32	*/

	memcpy((u8 *)&_rc_tx_buff.data[0], (u8 *)&ex_rc_new_adjustment_data.da[0], 26);

}
//#endif

//#if defined(UBA_RTQ_ICB)
/*********************************************************************//**
 * @brief display command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_rfid_test_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_RFID_TEST_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* mode			*/
}

/*********************************************************************//**
 * @brief Header send resquest RC RFID read data command
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_rfid_read_command(void)
{
	rc_last_send = rc_msg.arg1;
	if(rc_msg.sender_id != ID_RC_TASK)
	{
	//通信成功後、どのタスクにメッセージを返せばいいかのバックアップ
	//常に更新するとENQやメッセージ再送の時に、	ID_RC_TASK で更新されるので
	//ID_RC_TASK 以外を条件にする
		ex_rc_rfid_src_req = rc_msg.sender_id;
	}

	_rc_tx_buff.start_code 		= RC_SYNC;
	_rc_tx_buff.cmd				= CMD_RC_RFID_READ_REQ;
	

	if (rc_msg.arg1 == RFID_RUN) /* */
	{
		_rc_tx_buff.length		= 0x0A;					/* length		*/
		// アドレス(2byte)
		_rc_tx_buff.data[0]		= rc_msg.arg1;	
		_rc_tx_buff.data[1] 	= (u8)(rc_msg.arg2 & 0xff);
		_rc_tx_buff.data[2] 	= (u8)((rc_msg.arg2 >> 8) & 0xff);
		// サイズ
		_rc_tx_buff.data[3] 	= (u8)(rc_msg.arg3);
		_set_head_status();	
	}
	else
	{
		_rc_tx_buff.length		= 0x07;					/* length		*/
		_rc_tx_buff.data[0]		= rc_msg.arg1;	
		_set_head_status();								/* sst1 - sst32	*/
	}
}

/*********************************************************************//**
 * @brief Header send resquest RC RFID write data command
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_rfid_write_command(void)
{
	u32 index = 0;
	rc_last_send = rc_msg.arg1;
	if(rc_msg.sender_id != ID_RC_TASK)
	{
	//通信成功後、どのタスクにメッセージを返せばいいかのバックアップ
	//常に更新するとENQやメッセージ再送の時に、	ID_RC_TASK で更新されるので
	//ID_RC_TASK 以外を条件にする
		ex_rc_rfid_src_req = rc_msg.sender_id;
	}

	_rc_tx_buff.start_code 		= RC_SYNC;
	_rc_tx_buff.cmd				= CMD_RC_RFID_WRITE_REQ;

	if (rc_msg.arg1 == RFID_RUN)
	{
		_rc_tx_buff.length		= 0x0A + rc_msg.arg3;	/* length		*/
		_rc_tx_buff.data[0]		= rc_msg.arg1;
		// アドレス(2byte)
		_rc_tx_buff.data[1] 	= (u8)(rc_msg.arg2 & 0xff);
		_rc_tx_buff.data[2] 	= (u8)((rc_msg.arg2 >> 8) & 0xff);
		// サイズ
		_rc_tx_buff.data[3] 	= (u8)(rc_msg.arg3);

		while( index < rc_msg.arg3 )
		{
			_rc_tx_buff.data[4 + index] = rc_rfid_data[index];
			index++;
		}
		_set_head_status();								/* sst1 - sst32	*/
	}
	else
	{
	//ENQ
		_rc_tx_buff.length		= 0x07;					/* length		*/
		_rc_tx_buff.data[0]		= rc_msg.arg1;			/* mode			*/
		_set_head_status();								/* sst1 - sst32	*/

		#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)	//2025-07-04
		ex_rtq_rfid_res_totaltime = 0;
		#endif
	}
	#if defined(UBA_RTQ_ICB)
	_hal_status_led_orange(1);	//2025-03-25
	#endif
}

/*********************************************************************//**
 * @brief display command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_rfid_reset_command(void)
{
	rc_last_send = rc_msg.arg1;
	if(rc_msg.sender_id != ID_RC_TASK)
	{
		//通信成功後、どのタスクにメッセージを返せばいいかのバックアップ
		//常に更新するとENQやメッセージ再送の時に、	ID_RC_TASK で更新されるので
		//ID_RC_TASK 以外を条件にする
		ex_rc_rfid_src_req = rc_msg.sender_id;
	}

	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* request type	*/
	/* 読み込み要求 */
	if(RFID_RUN == rc_msg.arg1)
	{
		_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
		_rc_tx_buff.length		= 0x07;						/* length		*/
		_rc_tx_buff.cmd			= CMD_RC_RFID_RESET_REQ;
		_set_head_status();	
	}
	/* ENQ */
	else
	{
		_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
		_rc_tx_buff.length		= 0x07;						/* length		*/
		_rc_tx_buff.cmd			= CMD_RC_RFID_RESET_REQ;
		_set_head_status();									/* sst1 - sst32	*/
	}
}
//#endif // UBA_RTQ_ICB

/*********************************************************************//**
 * @brief sensor_command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_sensor_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x08;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_SENSOR_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* operation	*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* sensor type	*/
}

/*********************************************************************//**
 * @brief flapper command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_flapper_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x09;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_FLAPPER_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* unit			*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* position		*/
	_rc_tx_buff.data[2]		= rc_msg.arg3;			/* usb mode		*/
}


// /*********************************************************************//**
//  * @brief sensor_command sending procedure
//  * @param[in]	None
//  * @return 		None
//  **********************************************************************/
// static void rc_sensor_command(void)
// {
// 	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
// 	_rc_tx_buff.length		= 0x08;					/* length		*/
// 	_rc_tx_buff.cmd			= CMD_RC_SENSOR_REQ;
// 	_set_head_status();								/* sst1 - sst32	*/
// 	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* operation	*/
// 	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* sensor type	*/
// }


/*********************************************************************//**
 * @brief drum command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_drum_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x08;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_DRUM_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* drum			*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* operation	*/
}

/*********************************************************************//**
 * @brief set drum enable command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_set_enable_drum_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;					/* header		*/
	_rc_tx_buff.length		= 0x07;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_ENABLE_DRUM_REQ;
	_set_head_status();								/* sst1 - sst32	*/

	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* box number	*/
}

/*********************************************************************//**
 * @brief set motor speed command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_set_motor_speed_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;						/* header		*/
	_rc_tx_buff.length		= 0x0A;							/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_SET_MOTOR_SPEED_REQ;
	_set_head_status();										/* sst1 - sst32	*/

	memcpy((u8 *)&_rc_tx_buff.data[0], (u8 *)&uba_feed_speed_fwd, sizeof(uba_feed_speed_fwd));		/* fwd speed 	*/
	memcpy((u8 *)&_rc_tx_buff.data[2], (u8 *)&uba_feed_speed_rev, sizeof(uba_feed_speed_rev));		/* rev speed 	*/
}

/*********************************************************************//**
 * @brief stack command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_stack_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;							/* header		*/
	_rc_tx_buff.length		= 0x09;							/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_STACK_REQ;
	_set_head_status();										/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;					/* drum			*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;					/* escrow code	*/
	_rc_tx_buff.data[2]		= rc_msg.arg3;					/* bill length	*/
}

/*********************************************************************//**
 * @brief sol command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rc_sol_command(void)
{
	_rc_tx_buff.start_code	= RC_SYNC;				/* header		*/
	_rc_tx_buff.length		= 0x08;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RC_SOL_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* unit			*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* operation	*/
}

//#if defined(UBA_RS)
/*********************************************************************//**
 * @brief RS flapper command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rs_flapper_command()
{
	_rc_tx_buff.start_code	= RC_SYNC;				/* header		*/
	_rc_tx_buff.length		= 0x09;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RS_FLAPPER_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* position		*/
	_rc_tx_buff.data[1]		= 0x00;					/* 				*/
	_rc_tx_buff.data[2]		= rc_msg.arg2;			/* mode	 		*/
}

/*********************************************************************//**
 * @brief RS display command sending procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rs_display_command(void)
{
	_rc_tx_buff.start_code	= 0x24;					/* header		*/
	_rc_tx_buff.length		= 0x09;					/* length		*/
	_rc_tx_buff.cmd			= CMD_RS_DISPLAY_REQ;
	_set_head_status();								/* sst1 - sst32	*/
	_rc_tx_buff.data[0]		= rc_msg.arg1;			/* mode			*/
	_rc_tx_buff.data[1]		= rc_msg.arg2;			/* color		*/
	_rc_tx_buff.data[2]		= rc_msg.arg3;			/* count		*/
}

//#endif // uBA_RS

/*********************************************************************//**
 * @brief MBX message procedure
 *  rc task idle send polling
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_send_polling_proc(void) //ok
{
	rc_status_request();

	uart_send_rc(&_rc_tx_buff);

	/* 【RC_SEQ_WAIT_RESPONSE : 0x5101】 */
	_rc_set_seq(RC_SEQ_WAIT_RESPONSE);

}

/*********************************************************************//**
 * @brief GC2 encode
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void GC2Encode(u8 *pInput, u8 *pEncrypted, u8 length)
{
	u8 i, j;

	for(i = 0; i < length; i++)
	{
		pEncrypted[i] = pInput[i] ^ pEncrypted[i];

		for(j = 0; j < RC_GC2_ROUNDS; j++)
		{
			pEncrypted[i] = RC_GC2_BARREL_L(pEncrypted[i]) + ex_encryption_key[i];	// 上位5bitと下位3bitを交換したものにrc_encryption_keyを加える
		}
	}
}

/*********************************************************************//**
 * @brief encrypt message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void encrypt_message(u8 length)
{
	u8 *pSend_data;
	u8 encryption_cnt;
	
	pSend_data = (u8 *)&_rc_tx_enc_buff.cmd;
	encryption_cnt = 0;
	
	/* CBC ContextをWorkエリアにCopy */
	memcpy((u8 *)&ex_cbc_context_work[0], (u8 *)&ex_cbc_context[0], sizeof(ex_cbc_context_work));
	
	while(encryption_cnt < length)
	{		
		//暗号化残りが8バイト以上
		if(length - encryption_cnt >= 8)
		{
			/* 平文から暗号文に変換 */
			GC2Encode(pSend_data + encryption_cnt, (u8 *)&ex_cbc_context_work[0], 8);
			
			/* 暗号文を送信バッファに展開 */
			memcpy((u8 *)pSend_data + encryption_cnt, (u8 *)&ex_cbc_context_work[0], 8);
			
			encryption_cnt += 8;
		}
		else
		{
			/* 平文から暗号文に変換 */
			GC2Encode(pSend_data + encryption_cnt, (u8 *)&ex_cbc_context_work[0], length - encryption_cnt);
			
			/* 暗号文を送信バッファに展開 */
			memcpy((u8 *)pSend_data + encryption_cnt, (u8 *)&ex_cbc_context_work[0], length - encryption_cnt);
			
			encryption_cnt += length - encryption_cnt;
		}
	}
}

/*********************************************************************//**
 * @brief MBX message procedure
 *  rc task idle
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_send_command_proc(void) //だいたいokのはず
{
	switch (rc_msg.tmsg_code)
	{
	case TMSG_RC_INIT_REQ:
		rc_task_log_jdl();
		break;
	#if 0 //not use
	case TMSG_RC_STATUS_REQ: 						/* status request					*/
		break;
	#endif	
	case TMSG_RC_BOOT_VERSION_REQ: 					/* boot version request command		*/
		rc_get_boot_version_command();
		break;
	case TMSG_RC_RECYCLE_SETTING_REQ:
		rc_set_recycle_setting_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_VERSION_REQ:						/* version request command			*/
		rc_get_version_command();
		break;
	case TMSG_RC_READ_EDITION_REQ:					/* read Edition no command			*/
		rc_read_edition_no_command();
		break;
	case TMSG_RC_GET_RECYCLE_SETTING_REQ:
		rc_get_recycle_setting_command();
		break;
	case TMSG_RC_PREFEED_STACK_REQ:					/* prefeed stack command			*/
		rc_prefeed_stack_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_MODE_REQ:							/* mode setting command				*/
		rc_set_mode_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_READ_MAINTE_SERIALNO_REQ:			/* read Serial no command			*/
		rc_read_mainte_serial_no_command();
		break;
	case TMSG_RC_GET_DIPSW_REQ:						/* dipsw request command			*/
		rc_get_dipsw_command();
		break;
	case TMSG_RC_STATE_REQ:
		rc_set_state_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_DISPLAY_REQ:
		rc_display_command();
		break;
	case TMSG_RC_SW_COLLECT_REQ:
		rc_sw_clear_command();
		break;
	case TMSG_RC_GET_MOTOR_SPEED_REQ:
		rc_get_motor_speed_command();
		break;
	case TMSG_RC_FEED_REQ:
		rc_feed_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_FLAPPER_REQ:
		rc_flapper_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_SENSOR_REQ:
		rc_sensor_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_DRUM_REQ:
		rc_drum_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_SOL_REQ:
		rc_sol_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_RECYCLE_ENABLE_REQ:
		rc_set_enable_drum_command();
		break;
	case TMSG_RC_RESET_REQ:								/* RESET cmd */
		rc_reset_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_SET_MOTOR_SPEED_REQ:
		rc_set_motor_speed_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_STACK_REQ:
		rc_stack_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_PAYOUT_REQ:
		rc_payout_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_COLLECT_REQ:
		rc_collect_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_LAST_FEED_CASHBOX_REQ:
		rc_last_feed_cashbox_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_ERROR_CLEAR_REQ:
		rc_error_clear_command();
		break;
	case TMSG_RC_ERROR_DETAIL_REQ:
		rc_get_error_detail_command();
		break;
	case TMSG_RC_CURRENT_COUNT_SETTING_REQ:
		rc_set_current_count_setting_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_WU_RESET_REQ:
		rc_wu_reset_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_CANCEL_REQ:
		rc_cancel_command();
		rc_task_log_jdl();		
		break;
	case TMSG_RC_RETRY_BILL_DIR_REQ:			/* retry bill direction command		*/
		rc_retry_bill_dir_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_FEED_BOX_REQ:
		rc_feed_box_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_WRITE_SERIALNO_REQ:
		rc_write_serial_no_command();
		break;
	case TMSG_RC_READ_SERIALNO_REQ: 			/* read Serial no command			*/
		rc_read_serial_no_command();
		break;
	case TMSG_RC_WRITE_MAINTE_SERIALNO_REQ: 	/* write Serial no command			*/
		rc_write_mainte_serial_no_command();
		break;
	case TMSG_RC_WRITE_EDITION_REQ:		 		/* write Edition no command			*/
		rc_write_edition_no_command();
		break;
	case TMSG_RC_FRAM_READ_REQ: 				/* fram read command				*/
		rc_fram_read_command();
		break;
	case TMSG_RC_FRAM_CHECK_REQ: 				/* fram check command				*/
		rc_fram_check_command();
		break;
	case TMSG_RC_START_SENS_ADJ_REQ: 			/* sensor adj start request command	*/
		rc_start_sensor_adj_command();
		break;
	case TMSG_RC_READ_SENS_ADJ_DATA_REQ: 		/* sensor _adj read request command	*/
		rc_read_sensor_adj_data_command();
		break;
	case TMSG_RC_DRUM_TAPE_POS_ADJ_REQ: 		/* drum tape position adj command	*/
		rc_drum_tape_pos_adj_command();
		break;
	case TMSG_RC_SENS_ADJ_WRITE_FRAM_REQ: /* sensor _adj write fram request command	*/
		rc_sens_adj_write_fram_command();
		break;
	case TMSG_RC_SENS_ADJ_READ_FRAM_REQ: /* sensor _adj read fram request command	*/
		rc_sens_adj_read_fram_command();
		break;
	case TMSG_RC_PERFORM_TEST_WRITE_FRAM_REQ: /* performance test write fram request command	*/
		rc_perform_test_write_fram_command();
		break;
	case TMSG_RC_PERFORM_TEST_READ_FRAM_REQ: /* performance test read fram request command	*/
		rc_perform_test_read_fram_command();
		break;
	case TMSG_RC_POLL_CHANGE_REQ: 				/* polling change commanf			*/
		rc_poll_change_command();
		break;
	case TMSG_RC_SEARCH_REQ:
		rc_box_search_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_BILLBACK_DRUM_PAYOUT_REQ:
		rc_billback_drum_pauout_command();
		break;
	case TMSG_RC_FEEDBOX_DRUM_PAYOUT_REQ:
		rc_feed_box_drum_pauout_command();
		break;
	case TMSG_RC_FORCE_STACK_DRUM_REQ:
		rc_force_stack_drum_command();
		break;
	case TMSG_RC_BILLBACK2_REQ:
		rc_billback2_command();
		break;
	case TMSG_RC_DEL_REQ:
		rc_set_del_command();
		rc_task_log_jdl();
		break;
	case TMSG_RC_SENSOR_ACTIVE_REQ:
		rc_sensor_active_command();
		break;
	case TMSG_RC_SENSOR_CONDITION_REQ:
		rc_sensor_condition_command();
		break;
	case TMSG_RC_DRUM_GAP_ADJ_REQ:
		rc_drum_gap_adj_command();
		break;		
	case TMSG_RC_ERROR_COUNT_CLEAR_REQ:
		rc_error_count_clear_command();
		break;
	case TMSG_RC_DIAG_END_REQ:
		rc_diag_end_command();
		break;
	case TMSG_RC_DIAG_MOT_FWD_REQ:
		rc_diag_mot_fwd_command();
		break;
//#if defined(RC_BOARD_GREEN)
	case TMSG_RC_GET_CONFIGURATION_REQ:
		rc_get_configuration_command();
		break;
	/* sensor adjust */
	case  TMSG_RC_GET_POS_ONOFF_REQ:
		rc_get_pos_onoff_command();
		break;
	case TMSG_RC_SET_POS_DA_REQ:
		rc_set_pos_da_command();
		break;
	case TMSG_RC_GET_POS_GAIN_REQ:
		rc_set_pos_gain_command();
		break;
	/**/
//#endif // UBA_RS
	//#if defined(UBA_RS)
	case TMSG_RS_FLAPPER_REQ:
		rs_flapper_command();
		break;
	case TMSG_RS_DISPLAY_REQ:
		rs_display_command();
		break;
	//#endif // UBA_RS
	case TMSG_RC_RFID_TEST_REQ:
		rc_rfid_test_command();
		break;
	case TMSG_RC_RFID_READ_REQ:
		rc_rfid_read_command();
		break;
	case TMSG_RC_RFID_WRITE_REQ:
		rc_rfid_write_command();
		break;
	case TMSG_RC_RFID_RESET_REQ:
		rc_rfid_reset_command();
		break;
	/*  ---- Download Fw ---------------- */
	case TMSG_RC_DL_START_REQ:					/* download start command			*/
		rc_dl_start_command();
		break;
	case TMSG_RC_DL_DATA_REQ:					/* download data command			*/
		rc_dl_data_command();
		break;
	case TMSG_RC_DL_CHECK_REQ:					/* download check command			*/
		rc_dl_check_command();
		break;
	case TMSG_RC_DL_END_REQ:					/* download end command				*/
		rc_dl_end_command();
		break;
	/* ---------------------------------- */
	default:
		/* system error */
		_rc_system_error(1, 1);
		break;
	}
	if(rc_msg.tmsg_code != TMSG_RC_INIT_REQ)
	{
		// TMSG_RC_DEL_REQ
		if(rc_msg.tmsg_code == TMSG_RC_DEL_REQ)
		{
			/* 払出しコマンドの場合は暗号化する */
			if(rc_msg.arg1 == TMSG_RC_PAYOUT_REQ)
			{
				encrypt_message(_rc_tx_enc_buff.length - 1);
			}
			uart_send_rc_encode(&_rc_tx_enc_buff);
		}
		else
		{
			uart_send_rc(&_rc_tx_buff);
		}

		/* 【RC_SEQ_WAIT_RESPONSE : 0x5101】 */
		_rc_set_seq(RC_SEQ_WAIT_RESPONSE);
	}
}

/*********************************************************************//**
 * @brief rfid reset
 * @param[in]   None
 * @return      None
 **********************************************************************/
void rc_rfid_reset(void) //ICB task //2024-12-18
{
	_icb_send_msg(ID_RC_MBX, TMSG_RC_RFID_RESET_REQ, RFID_RUN, 0, 0, 0);
}


/*********************************************************************//**
 * @brief rfid write
 * @param[in]   None
 * @return      None
 **********************************************************************/
void rc_rfid_write(u16 addr, u16 length, u8 *data) //2024-12-18 //ICB task
{
	memcpy(rc_rfid_data, data, length);
	_icb_send_msg(ID_RC_MBX, TMSG_RC_RFID_WRITE_REQ, RFID_RUN, addr, length, 0);
}

#if defined(QA_TEST_SAFE) || defined(QA_TEST_EMC_EMI)	//2025-02-20
void rc_rfid_write_test(void) //2024-12-18 //ICB task
{
	rc_rfid_data[0] = 1;
	rc_rfid_data[1] = 1;
	rc_rfid_data[2] = 1;
	rc_rfid_data[3] = 1;
	rc_rfid_data[4] = 1;
	rc_rfid_data[5] = 1;
	rc_rfid_data[6] = 1;
	rc_rfid_data[7] = 1;

}
#endif

#if defined(UBA_RTQ_ICB)
void _main_rtq_rfid(void)
{

	memcpy(rc_rfid_data, &Smrtdat_fram, sizeof(Smrtdat_fram));
	_main_send_msg(ID_RC_MBX, TMSG_RC_RFID_WRITE_REQ, RFID_RUN, 0, sizeof(Smrtdat_fram), 0);	
	//#if defined(NEW_RFID)	//2025-07-04
	ex_rtq_rfid_write_disable = 1;
	//#endif
}
#endif

#endif // UBA_RTQ
