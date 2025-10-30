/******************************************************************************/
/*! @addtogroup Group2
    @file       mode_rcinit.c
    @brief      initialize mode of main task
    @date       2018/01/15
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/01/15 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
#if defined(UBA_RTQ)

#include <string.h>
#include "common.h"
#include "cyc.h"
#include "sensor_ad.h"

#include "operation.h"
#include "if_rc.h"


#include "js_io.h"
#include "js_oswapi.h"
#include "hal_gpio.h"
#include "hal_gpio_reg.h"


#include "sub_functions.h"
#include "status_tbl.h"
#include "jdl_conf.h"
#include "jdl.h"


#define EXT
#include "com_ram.c"




//extern void sysinit_2nd_cpu(void);
//extern void sysinit_2nd_dl(void);
//extern void sysinit_2nd_error(void);


#include  "kernel.h"    // CCS用に追加 ITORN特有の変数関数の型宣言

/************************** PRIVATE DEFINITIONS *************************/
#define	TWIN_ALL_DRUM_ENABLE			0x03
#define	QUAD_ALL_DRUM_ENABLE			0x0F


/************************** PRIVATE VARIABLES *************************/
const u8 encryption_serial_number[9]     = "JCM-RC01";									/* GC2 Encryption S/N */
const u8 encryption_key_initial_value[8] = {0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF};	/* GC2 Encryption key initil value */

const u16 GC2tbl[0x100]=
{
/*  0       1       2       3       4       5       6       7				 */
	0x0000, 0x8911, 0x1223, 0x9B32, 0x2446, 0xAD57, 0x3665, 0xBF74,	/* 00-07 */
	0x488C, 0xC19D, 0x5AAF, 0xD3BE, 0x6CCA, 0xE5DB, 0x7EE9, 0xF7F8,	/* 08-0F */
	0x8110, 0x0801, 0x9333, 0x1A22, 0xA556, 0x2C47, 0xB775, 0x3E64,	/* 10-17 */
	0xC99C, 0x408D, 0xDBBF, 0x52AE, 0xEDDA, 0x64CB, 0xFFF9, 0x76E8,	/* 18-1F */
	0x0221, 0x8B30, 0x1002, 0x9913, 0x2667, 0xAF76, 0x3444, 0xBD55,	/* 20-27 */
	0x4AAD, 0xC3BC, 0x588E, 0xD19F, 0x6EEB, 0xE7FA, 0x7CC8, 0xF5D9,	/* 28-2F */
	0x8331, 0x0A20, 0x9112, 0x1803, 0xA777, 0x2E66, 0xB554, 0x3C45,	/* 30-37 */
	0xCBBD, 0x42AC, 0xD99E, 0x508F, 0xEFFB, 0x66EA, 0xFDD8, 0x74C9,	/* 38-3F */
	0x0442, 0x8D53, 0x1661, 0x9F70, 0x2004, 0xA915, 0x3227, 0xBB36,	/* 40-47 */
	0x4CCE, 0xC5DF, 0x5EED, 0xD7FC, 0x6888, 0xE199, 0x7AAB, 0xF3BA,	/* 48-4F */
	0x8552, 0x0C43, 0x9771, 0x1E60, 0xA114, 0x2805, 0xB337, 0x3A26,	/* 50-57 */
	0xCDDE, 0x44CF, 0xDFFD, 0x56EC, 0xE998, 0x6089, 0xFBBB, 0x72AA,	/* 58-5F */
	0x0663, 0x8F72, 0x1440, 0x9D51, 0x2225, 0xAB34, 0x3006, 0xB917,	/* 60-67 */
	0x4EEF, 0xC7FE, 0x5CCC, 0xD5DD, 0x6AA9, 0xE3B8, 0x788A, 0xF19B,	/* 68-6F */
	0x8773, 0x0E62, 0x9550, 0x1C41, 0xA335, 0x2A24, 0xB116, 0x3807,	/* 70-77 */
	0xCFFF, 0x46EE, 0xDDDC, 0x54CD, 0xEBB9, 0x62A8, 0xF99A, 0x708B,	/* 78-7F */
	0x0884, 0x8195, 0x1AA7, 0x93B6, 0x2CC2, 0xA5D3, 0x3EE1, 0xB7F0,	/* 80-87 */
	0x4008, 0xC919, 0x522B, 0xDB3A, 0x644E, 0xED5F, 0x766D, 0xFF7C,	/* 88-8F */
	0x8994, 0x0085, 0x9BB7, 0x12A6, 0xADD2, 0x24C3, 0xBFF1, 0x36E0,	/* 90-97 */
	0xC118, 0x4809, 0xD33B, 0x5A2A, 0xE55E, 0x6C4F, 0xF77D, 0x7E6C,	/* 98-9F */
	0x0AA5, 0x83B4, 0x1886, 0x9197, 0x2EE3, 0xA7F2, 0x3CC0, 0xB5D1,	/* A0-A7 */
	0x4229, 0xCB38, 0x500A, 0xD91B, 0x666F, 0xEF7E, 0x744C, 0xFD5D,	/* A8-AF */
	0x8BB5, 0x02A4, 0x9996, 0x1087, 0xAFF3, 0x26E2, 0xBDD0, 0x34C1,	/* B0-B7 */
	0xC339, 0x4A28, 0xD11A, 0x580B, 0xE77F, 0x6E6E, 0xF55C, 0x7C4D,	/* B8-BF */
	0x0CC6, 0x85D7, 0x1EE5, 0x97F4, 0x2880, 0xA191, 0x3AA3, 0xB3B2,	/* C0-C7 */
	0x444A, 0xCD5B, 0x5669, 0xDF78, 0x600C, 0xE91D, 0x722F, 0xFB3E,	/* C8-CF */
	0x8DD6, 0x04C7, 0x9FF5, 0x16E4, 0xA990, 0x2081, 0xBBB3, 0x32A2,	/* D0-D7 */
	0xC55A, 0x4C4B, 0xD779, 0x5E68, 0xE11C, 0x680D, 0xF33F, 0x7A2E,	/* D8-DF */
	0x0EE7, 0x87F6, 0x1CC4, 0x95D5, 0x2AA1, 0xA3B0, 0x3882, 0xB193,	/* E0-E7 */
	0x466B, 0xCF7A, 0x5448, 0xDD59, 0x622D, 0xEB3C, 0x700E, 0xF91F,	/* E8-EF */
	0x8FF7, 0x06E6, 0x9DD4, 0x14C5, 0xABB1, 0x22A0, 0xB992, 0x3083,	/* F0-F7 */
	0xC77B, 0x4E6A, 0xD558, 0x5C49, 0xE33D, 0x6A2C, 0xF11E, 0x780F	/* F8-FF */
};

/************************** PRIVATE FUNCTIONS *************************/
static void rcinit_cpu_reset(void);
static void rcinit_fw_check(void);
static void rcinit_encryption_init(void);
static void rcinit_prefeed_stack(void);
static void rcinit_wait_setting(void);
static void rcinit_box_removed_check(void);
static void rcinit_dl_start(void);
static void rcinit_dl_data(void);
static void rcinit_dl_end(void);

static void set_recycler_operation_mode(u8 mode);
static u8 rc_check_connected(void);

static u8 loop;

/************************** EXTERN FUNCTIONS *************************/

static void _main_system_alarm(u16 alarm)
{
	if (ex_dipsw1 & DIPSW1_PERFORMANCE_TEST)
	{
		ex_operating_mode |= OPERATING_MODE_TEST;
	}

#if 1//#ifdef _ENABLE_JDL
	jdl_error(alarm, 0xF000, ex_main_task_mode1, ex_main_task_mode2, 0);
#endif /* _ENABLE_JDL */

	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, alarm, 0, 0, 0);
	//_main_send_msg(ID_ERRDISP_MBX, TMSG_ERRDISP_LED_ALARM, alarm, 0, 0, 0);
    /* I/Fで通信を行うためにI/F初期化 */
	_main_send_connection_task(TMSG_CONN_INITIAL, ex_operating_mode, TMSG_SUB_ALARM, alarm, 0 );

	/* Support USB download and monitor. */

	act_tsk(ID_DLINE_TASK);
	_main_send_msg(ID_DLINE_MBX, TMSG_DLINE_INITIAL_REQ, ex_operating_mode, TMSG_SUB_ALARM, alarm, 0);

	while (1)
	{
		dly_tsk(500);
	}
}


/*********************************************************************//**
 * @brief initialize of rc task message procedure
 * @param[in]	None
 * @return 		rc_error		0 : success
 *								1 : error
 **********************************************************************/
u8 _rc_initial_msg_proc(void)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	u8 rc_error = 0;
	u8 ret = 0;
	u8 result = 0;
	loop = 1;

	ex_rc_retry_flg = 0;

#if defined(USE_RLINE_TASK) && !defined(_PROTOCOL_ENABLE_ID008)
	_main_send_msg(ID_RLINE_MBX, TMSG_RLINE_INITIAL_REQ, ex_operating_mode, TMSG_SUB_SUCCESS, 0, 0);
#endif

	result = rc_check_connected();

	if(result != 0)
	{
		/* エラー処理 */
		ex_rc_powerup_error = 1;
		_main_system_alarm(result);	/* 致命的なエラーなので無限ループ      */
	}
	else
	{
		ex_rc_powerup_error = 0;
	}

	_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_CPU_RESET);

	/* send message to rc_task (TMSG_RC_INIT_REQ) */
	_main_send_msg(ID_RC_MBX, TMSG_RC_INIT_REQ, 0, 0, 0, 0);

	/* waiting the complecation of rc */
	while(loop)
	{
		/* check the recieve message */
		ercd = rcv_mbx(ID_MAIN_MBX, (T_MSG **)&tmsg_pt);

		if(ercd == E_OK)
		{
			/* copy the data to buffer */
			memcpy(&ex_main_msg, tmsg_pt, sizeof(T_MSG_BASIC));

			if((rel_mpf(ex_main_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_main_system_error(0, 100);
			}

			switch(ex_main_task_mode2)
			{
			case RCINIT_MODE2_CPU_RESET: /* (MODE1_RCINIT, RCINIT_MODE2_CPU_RESET)		*/
				rcinit_cpu_reset();
				break;
			case RCINIT_MODE2_FW_CHECK: /* (MODE1_RCINIT, RCINIT_MODE2_FW_CHECK)		*/
				rcinit_fw_check();
				break;
			case RCINIT_MODE2_ENCRYPTION_INIT:
				rcinit_encryption_init();
				break;
			case RCINIT_MODE2_PREFEED_STACK:
				rcinit_prefeed_stack();
				break;
			case RCINIT_MODE2_WAIT_SETTING: /* (MODE1_RCINIT, RCINIT_MODE2_WAIT_SETTING)	*/
				rcinit_wait_setting();
				break;
			case RCINIT_MODE2_BOX_REMOVED_CHECK:
				rcinit_box_removed_check();
				break;
			case RCINIT_MODE2_DL_START:
				rcinit_dl_start();
				break;
			case RCINIT_MODE2_DL_DATA:
				rcinit_dl_data();
				break;
			case RCINIT_MODE2_DL_END:
				rcinit_dl_end();
				break;
			default:
				/* system error */
				_main_system_error(0, 126);
				break;
			}
		}
	}
	return(rc_error);
}


/*********************************************************************//**
 * @brief accept message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rcinit_msg_proc(void)
{
}


/*********************************************************************//**
 * @brief accept message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rcinit_cpu_reset(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_RC_INIT_RSP:
		/* success the line between main cpu and rc cpu */
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* waiting rc_task */
			dly_tsk(300);

			_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_FW_CHECK);

			/* send message to rc_task (TMSG_RC_BOOT_VERSION_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_BOOT_VERSION_REQ, 0, 0, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* エラー処理 */
			_main_system_alarm(ex_main_msg.arg2); /* 致命的なエラーなので無限ループ      */
		}
		else
		{
			/* system error */
			_main_system_error(0, 101);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* エラー処理 */
			if (ex_main_msg.arg2 == ALARM_CODE_RC_ROM || ex_main_msg.arg2 == ALARM_CODE_RC_COM)
			{
				_main_system_alarm(ex_main_msg.arg2); /* 致命的なエラーなので無限ループ      */
			}
			else
			{
				/* system error */
				_main_system_error(0, 102);
			}
		}
		else
		{
			/* system error */
			_main_system_error(0, 103);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		/* system error */
		_main_system_error(0, 104);
		break;
	}
}


/*********************************************************************//**
 * @brief accept message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rcinit_fw_check(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_RC_BOOT_VERSION_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* copy to I/F area */
			memcpy((u8 *)&RecycleSoftInfo_bk.BootRomid[0], (u8 *)&RecycleSoftInfo.BootRomid[0], 10);

			/* send message to rc_task (TMSG_RC_VERSION_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_VERSION_REQ, 0, 0, 0, 0);
		}
		else
		{
			/* system error */
			_main_system_error(0, 105);
		}
		break;
	case TMSG_RC_VERSION_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* version is ok */
			/* copy to I/F area */
			memcpy((u8 *)&RecycleSoftInfo_bk.FlashRomid[0], (u8 *)&RecycleSoftInfo.FlashRomid[0], 28);

			/* send message to rc_task (TMSG_RC_DL_CHECK_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_DL_CHECK_REQ, 0, 0, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* version is ng */
			ex_rc_download_flag = 1;	/* RC download mode ON */
			ex_rc_download_stat = 1;	/* RC download Busy */
			_main_send_connection_task(TMSG_CONN_LINE_OPEN, TMSG_SUB_SUCCESS, 0, 0, 0); //TMSG_CLINE_UART_OPEM_REQ

			/* go to download moe */
			_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_DL_START);

			/* send message to rc_task (TMSG_RC_DL_START_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_DL_START_REQ, 0, 0, 0, 0);
		}
		else
		{
			/* system error */
			_main_system_error(0, 106);
		}
		break;
	case TMSG_RC_DL_CHECK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* checksum is ok */
			/* copy to I/F area */
			memcpy((u8 *)&RecycleSoftInfo_bk.FlashCheckSum[0], (u8 *)&RecycleSoftInfo.FlashCheckSum[0], 2);

// read_eeprom_value( EPROM_MAINTENANCE_EDITION, EPROM_MAINTENANCE_EDITION_SIZE, (u8*)&read_editionno_data.head[0]);
		#if 1//#if defined(RC_BOARD_GREEN)  //新基板、旧基板、RS、RFIDかの確認コマンド
			_main_send_msg(ID_RC_MBX, TMSG_RC_GET_CONFIGURATION_REQ, 0, 0, 0, 0); //新基板、旧基板、RS、RFIDかの確認コマンド
		#else
			_main_send_msg(ID_RC_MBX, TMSG_RC_READ_EDITION_REQ, 1, 0, 0, 0);
		#endif // RC_BOARD_GREEN
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* checksum is ng */
			ex_rc_download_stat = 1;	/* RC download Busy */
			ex_rc_download_flag = 1;	/* RC download mode ON */
			_main_send_connection_task(TMSG_CONN_LINE_OPEN, TMSG_SUB_SUCCESS, 0, 0, 0); //TMSG_CLINE_UART_OPEM_REQ

			/* go to download moe */
			_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_DL_START);

			/* send message to rc_task (TMSG_RC_DL_START_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_DL_START_REQ, 0, 0, 0, 0);
		}
		else
		{
			/* system error */
			_main_system_error(0, 107);
		}
		break;
#if 1// #if defined(RC_BOARD_GREEN)  //新基板、旧基板、RS、RFIDかの確認コマンド受信完了
	case TMSG_RC_GET_CONFIGURATION_RSP:  //新基板、旧基板、RS、RFIDかの確認コマンド受信完了
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			// TODO: Save maintenance edition
			_main_send_msg(ID_RC_MBX, TMSG_RC_READ_EDITION_REQ, 1, 0, 0, 0);
		}
		else
		{
			/* system error */
			_main_system_error(0, 107);
		}
		break;
#endif
	case TMSG_RC_READ_EDITION_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_RC_MBX, TMSG_RC_GET_DIPSW_REQ, 0, 0, 0, 0);
		}
		else
		{
			/* system error */
			_main_system_error(0, 107);
		}
		break;
	case TMSG_RC_GET_DIPSW_RSP:
#if defined(RC_ENCRYPTION)
		// 暗号化初期化
		initialize_encryption();
		renewal_cbc_context();

		_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_ENCRYPTION_INIT);

		/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_KEY) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_KEY, 0, 0, 0);
#else
		_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_PREFEED_STACK);
		/* send message to rc_task (TMSG_RC_PREFEED_STACK_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_PREFEED_STACK_REQ, 0, 0, 0, 0);
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
#endif
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* エラー処理 */
			if (ex_main_msg.arg2 == ALARM_CODE_RC_ROM || ex_main_msg.arg2 == ALARM_CODE_RC_COM)
			{
				_main_system_alarm(ex_main_msg.arg2); /* 致命的なエラーなので無限ループ      */
			}
			else
			{
				/* system error */
				_main_system_error(0, 108);
			}
		}
		else
		{
			/* system error */
			_main_system_error(0, 109);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		/* system error */
		_main_system_error(0, 110);
		break;
	}
}

static void rcinit_encryption_init()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_RC_DEL_RSP:
		switch (ex_main_msg.arg1)
		{
		case TMSG_RC_ENC_KEY:
			/* send message to rc_task (TMSG_RC_DEL_REQ  TMSG_RC_ENC_NUM) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_DEL_REQ, TMSG_RC_ENC_NUM, 0, 0, 0);
			break;
		case TMSG_RC_ENC_NUM:
			_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_PREFEED_STACK);
			/* send message to rc_task (TMSG_RC_PREFEED_STACK_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_PREFEED_STACK_REQ, 0, 0, 0, 0);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_TIMEOUT, WAIT_TIME_RC_TIMEOUT, 0, 0);
			break;
		default:
			/* system error */
			_main_system_error(0, 111);
			break;
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* エラー処理 */
			if (ex_main_msg.arg2 == ALARM_CODE_RC_ROM || ex_main_msg.arg2 == ALARM_CODE_RC_COM)
			{
				_main_system_alarm(ex_main_msg.arg2); /* 致命的なエラーなので無限ループ      */
			}
			else
			{
				/* system error */
				_main_system_error(0, 112);
			}
		}
		else
		{
			/* system error */
			_main_system_error(0, 113);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		/* system error */
		_main_system_error(0, 114);
		break;
	}
}

static void rcinit_prefeed_stack()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_RC_PREFEED_STACK_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* system error */
			_main_system_error(0, 114);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (rc_warning_status())
			{
				/* send message to rc_task (TMSG_RC_PREFEED_STACK_REQ) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_PREFEED_STACK_REQ, 0, 0, 0, 0);
			}
			else if (!(rc_busy_status()))
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RC_TIMEOUT, 0, 0, 0);
				/* リサイクル金種,Total Countを初期化する */
				_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_WAIT_SETTING);

				// /* send message to rc_task (TMSG_RC_GET_RECYCLE_SETTING_REQ) */
				// _main_send_msg(ID_RC_MBX, TMSG_RC_GET_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
				/* Read fram */
				/* => removed fram rc all was read by _fram_read_all_data_proc */
				_main_send_msg(ID_FRAM_MBX, TMSG_FRAM_READ_REQ, FRAM_RTQ, FRAM_RC_ALL, 0, 0);
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, WAIT_TIME_RC_BUSY_CHECK, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_TIMEOUT)
		{
			/* system error */
			_main_system_error(0, 114);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* エラー処理 */
			if (ex_main_msg.arg2 == ALARM_CODE_RC_ROM || ex_main_msg.arg2 == ALARM_CODE_RC_COM)
			{
				_main_system_alarm(ex_main_msg.arg2); /* 致命的なエラーなので無限ループ      */
			}
			else
			{
				/* system error */
				_main_system_error(0, 112);
			}
		}
		else
		{
			/* system error */
			_main_system_error(0, 113);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		/* system error */
		_main_system_error(0, 114);
		break;
	}
}


/*********************************************************************//**
 * @brief accept message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rcinit_wait_setting(void)
{
	static u8 cnt;
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_FRAM_READ_RSP:	//step1
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			//RTQ側で保持しているリサイクル情報をリクエスト
			_main_send_msg(ID_RC_MBX, TMSG_RC_GET_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			//TODO: temporary 
			_main_system_alarm(ALARM_MODE2_FRAM);
		}
		break;
	case TMSG_FRAM_WRITE_RSP:
		
		break;
	case TMSG_RC_GET_RECYCLE_SETTING_RSP:	//step2
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			//この時点でRTQ側で保持しているリサイクル情報を取得済み
			// DIP-SW読み取りがUBA700ではセマフォ処理になっているので、ここで改めて読むより
			// この前の処理で ex_dipsw1_run == 1 になるのを待って、1度は読めているので、ここで再度読み込みは行わない

			/* エージングモード(Aging mode) */
			if (((ex_dipsw1 & DIPSW1_RC_AGING_TEST) == DIPSW1_RC_AGING_TEST || (ex_dipsw1 & DIPSW1_RC_AGING_FACTORY) == DIPSW1_RC_AGING_FACTORY
		#if defined(RTQ_FACTORY) //生産用のソフトのみ、無鑑別はEUR設定でリサイクル、それ以外の国は、無鑑別はID-003の設定を引く次ぐのでこの関数コールしない	
				|| (ex_dipsw1 & DIPSW1_ACCEPT_ALLACC_TEST) == DIPSW1_ACCEPT_ALLACC_TEST //2024-10-04 生産でEUR以外の国が使用された場合問題になるので、復活させるかも
		#endif //
				 ) &&
				(ex_dipsw1 & DIPSW1_PERFORMANCE_TEST) != 0)
			{
				memset((u8 *)&RecycleSettingInfo.DenomiInfo[0].BoxNumber, 0, sizeof(RecycleSettingInfo));

				/* RCのDipSWから金種を設定(Read and set the setting from DipSW of RC side) */
				is_recycle_set_test_denomi();

				//リサイクル設定を変更したのでRTQへの通知が必要
				_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
			}
			else
			{
				/* 前回ソフトと今回ソフトの情報が同じ場合(Model/Protocol/Country check) */
				if (software_version_check())
				{
				//前回ソフトと同じ場合
				//本来ここでは、TMSG_RC_RECYCLE_SETTING_REQ は必要ないはず、
				//理由はリサイクル設定を変更しないので
					if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
					{
					//RTQが外れている場合
						#if 0 // #ifdef _ENABLE_JDL
						jdl_rc_set_version();
						jdl_rc_set_rc_setting();
						#endif /* _ENABLE_JDL */
						/* 前回がテストモードの場合 */
						if (rc_before_mode == HEAD_TEST_MODE)
						{
							/* 今回のモードに関わらずリカバリーフラグを初期化する */
							ex_recovery_info.step = RECOVERY_STEP_NON;
						}
						/* 前回がI/Fモードの場合 */
						else if (rc_before_mode == HEAD_IF_MODE)
						{
							if (ex_dipsw1 & DIPSW1_PERFORMANCE_TEST)
							{
								/* 今回がテストモードのみリカバリーフラグを初期化する */
								ex_recovery_info.step = RECOVERY_STEP_NON;
							}
						}
						else
						{
							/* その他 */
							ex_recovery_info.step = RECOVERY_STEP_NON;
						}

						/* test mode */
						if (ex_dipsw1 & DIPSW1_PERFORMANCE_TEST)
						{
							/* 動作モードを保存 */
							set_recycler_operation_mode(HEAD_TEST_MODE);

							if ((ex_dipsw1 & DIPSW1_ACCEPT_TEST) == DIPSW1_ACCEPT_TEST || (ex_dipsw1 & DIPSW1_ACCEPT_ALLACC_TEST) == DIPSW1_ACCEPT_ALLACC_TEST
								|| (ex_dipsw1 & DIPSW1_RC_AGING_TEST) == DIPSW1_RC_AGING_TEST || (ex_dipsw1 & DIPSW1_RC_AGING_FACTORY) == DIPSW1_RC_AGING_FACTORY)
							{
								/* send message to rc_task (TMSG_RC_MODE_REQ) */
								//RSユニット対応から、引数に DIPSW1_RC_AGING_FACTORY を追加する事になった
								//エージング動作で通常では動かさないタイミングでフラッパ動作を行う為、
								//RTQ側の混乱をさける為、引数で通知する事になった。
							//2025-01-28
								if((ex_dipsw1 & DIPSW1_RC_AGING_TEST) == DIPSW1_RC_AGING_TEST)
								{
									/* send message to rc_task (TMSG_RC_MODE_REQ) */
									_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, DIPSW1_RC_AGING_TEST, 0, 0);
								}
								else if((ex_dipsw1 & DIPSW1_RC_AGING_FACTORY) == DIPSW1_RC_AGING_FACTORY)
								{
									/* send message to rc_task (TMSG_RC_MODE_REQ) */
									_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, DIPSW1_RC_AGING_FACTORY, 0, 0);
								}
								else
								{
									/* send message to rc_task (TMSG_RC_MODE_REQ) */
									_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
								}
							}
							else
							{
								/* send message to rc_task (TMSG_RC_MODE_REQ) */
								_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_TEST_MODE, 0, 0, 0);
							}
						}
						/* I/F mode */
						else
						{
							/* 動作モードを保存 */
							set_recycler_operation_mode(HEAD_IF_MODE);

							/* send message to rc_task (TMSG_RC_MODE_REQ) */
							_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
						}
					}
					else
					{
					//通常はここ
					//通常のID-003で、ソフト変更されていない場合、RTQが外れていない状態
						//ここは本来下記は必要ない気がするが、UBA500RTQと同じにする
						//このケースの場合、本来下記のコマンドは必要ないかも
						//このコマンドを省略して、次のステップに遷移させていいはず
						/* RCへリサイクル金種設定(set the setting from mamory of RC side) */
						_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
					}
				}
				/* 前回ソフトと今回ソフトの情報が異なる場合 */
				else
				{
					/* 前回のモードが異なる場合はリカバリーフラグを初期化する */
					ex_recovery_info.step = RECOVERY_STEP_NON;

					/* リサイクル金種,Total Countを初期化する */
					memset((u8 *)&RecycleSettingInfo.DenomiInfo[0].BoxNumber, 0, sizeof(RecycleSettingInfo));
					memset((u8 *)&rcLogdatIF.rcLogIF[0].AcceptCount, 0, sizeof(rcLogdatIF));

					/* write fram */
					_main_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_RTQ, FRAM_RC_LOG_IF, 0, 0);

					RecycleSettingInfo.DenomiInfo[0].BoxNumber = 1;
					RecycleSettingInfo.DenomiInfo[1].BoxNumber = 2;
					RecycleSettingInfo.DenomiInfo[2].BoxNumber = 3;
					RecycleSettingInfo.DenomiInfo[3].BoxNumber = 4;

					for (cnt = 0; cnt < 4; cnt++)
					{
						RecycleSettingInfo.DenomiInfo[cnt].RecycleLimit = 30;
						RecycleSettingInfo_bk.DenomiInfo[cnt].RecycleLimit = 30;
					}
					//リサイクル設定を変更したのでRTQへの通知が必要
					_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_SETTING_REQ, 0xFF, 0, 0, 0); //引数は0xFFが望ましい
					/* Write fram */
					_main_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_RTQ, FRAM_RC_SOFT_INFO, 0, 0);
				}
			}
		}
		else
		{
			/* system error */
			_main_system_error(0, 115);
		}
		break;
	case TMSG_RC_RECYCLE_SETTING_RSP:	//setp3 場合によっては step2からstep4へ遷移する場合あり //RTQ側へリサイクル金種を設定中
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
#if 0 // #ifdef _ENABLE_JDL
			jdl_rc_set_version();
			jdl_rc_set_rc_setting();
#endif /* _ENABLE_JDL */
			/* 前回がテストモードの場合 */
			if (rc_before_mode == HEAD_TEST_MODE)
			{
				/* 今回のモードに関わらずリカバリーフラグを初期化する */
				ex_recovery_info.step = RECOVERY_STEP_NON;
			}
			/* 前回がI/Fモードの場合 */
			else if (rc_before_mode == HEAD_IF_MODE)
			{
				if (ex_dipsw1 & DIPSW1_PERFORMANCE_TEST)
				{
					/* 今回がテストモードのみリカバリーフラグを初期化する */
					ex_recovery_info.step = RECOVERY_STEP_NON;
				}
			}
			else
			{
				/* その他 */
				ex_recovery_info.step = RECOVERY_STEP_NON;
			}

			/* test mode */
			if (ex_dipsw1 & DIPSW1_PERFORMANCE_TEST)
			{
				/* 動作モードを保存 */
				set_recycler_operation_mode(HEAD_TEST_MODE);

				if ((ex_dipsw1 & DIPSW1_ACCEPT_TEST) == DIPSW1_ACCEPT_TEST || (ex_dipsw1 & DIPSW1_ACCEPT_ALLACC_TEST) == DIPSW1_ACCEPT_ALLACC_TEST
					|| (ex_dipsw1 & DIPSW1_RC_AGING_TEST) == DIPSW1_RC_AGING_TEST || (ex_dipsw1 & DIPSW1_RC_AGING_FACTORY) == DIPSW1_RC_AGING_FACTORY)
				{
					/* send message to rc_task (TMSG_RC_MODE_REQ) */
					//RSユニット対応から、引数に DIPSW1_RC_AGING_FACTORY を追加する事になった
					//エージング動作で通常では動かさないタイミングでフラッパ動作を行う為、
					//RTQ側の混乱をさける為、引数で通知する事になった。
					//2025-01-28
					if((ex_dipsw1 & DIPSW1_RC_AGING_TEST) == DIPSW1_RC_AGING_TEST)
					{
						/* send message to rc_task (TMSG_RC_MODE_REQ) */
						_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, DIPSW1_RC_AGING_TEST, 0, 0);
					}
					else if((ex_dipsw1 & DIPSW1_RC_AGING_FACTORY) == DIPSW1_RC_AGING_FACTORY)
					{
						/* send message to rc_task (TMSG_RC_MODE_REQ) */
						_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, DIPSW1_RC_AGING_FACTORY, 0, 0);
					}
					else
					{
						/* send message to rc_task (TMSG_RC_MODE_REQ) */
						_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
					}
				}
				else
				{
					/* send message to rc_task (TMSG_RC_MODE_REQ) */
					_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_TEST_MODE, 0, 0, 0);
				}
			}
			/* I/F mode */
			else
			{
				/* 動作モードを保存 */
				set_recycler_operation_mode(HEAD_IF_MODE);

				/* send message to rc_task (TMSG_RC_MODE_REQ) */
				_main_send_msg(ID_RC_MBX, TMSG_RC_MODE_REQ, HEAD_IF_MODE, 0, 0, 0);
			}
		}
		else
		{
			/* system error */
			_main_system_error(0, 116);
		}
		break;
	case TMSG_RC_MODE_RSP: //step4
		/* リサイクル金種設定時はバックアップも更新 */
		memcpy((u8 *)&RecycleSettingInfo_bk.DenomiInfo[0], (u8 *)&RecycleSettingInfo.DenomiInfo[0], sizeof(RecycleSettingInfo));

		_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_BOX_REMOVED_CHECK);

		/* send message to rc_task (TMSG_RC_READ_MAINTE_SERIALNO_REQ) */
		_main_send_msg(ID_RC_MBX, TMSG_RC_READ_MAINTE_SERIALNO_REQ, 0, 0, 0, 0);

		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_main_msg.arg2 == ALARM_CODE_RC_ROM || ex_main_msg.arg2 == ALARM_CODE_RC_COM)
			{
				_main_system_alarm(ex_main_msg.arg2); /* 致命的なエラーなので無限ループ      */
			}
		}
		else
		{
			/* system error */
			_main_system_error(0, 117);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		/* system error */
		_main_system_error(0, 118);
		break;
	}
}

static void rcinit_box_removed_check()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_RC_READ_MAINTE_SERIALNO_RSP:
		/* Serial No. check */
		if (!(is_rc_serial_no_check()))
		{
			ex_rc_exchanged_unit_powerup = 1;
		}
		else
		{
			ex_rc_exchanged_unit_powerup = 0;

			/* battery detect */
			if (ex_rc_status.sst22A.bit.battery_detect)
			{
				ex_rc_option_battery = 1;
			}
			/* battery not detect */
			else
			{
				ex_rc_option_battery = 0;
			}
		}

		if (ex_rc_status.sst1A.bit.quad)
		{
			ex_rc_enable = QUAD_ALL_DRUM_ENABLE;
		}
		else
		{
			ex_rc_enable = TWIN_ALL_DRUM_ENABLE;
		}
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_rc_enable, 0, 0, 0);
		loop = 0;	//これで起動時のループ抜ける
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* エラー処理 */
			if (ex_main_msg.arg2 == ALARM_CODE_RC_ROM || ex_main_msg.arg2 == ALARM_CODE_RC_COM)
			{
				_main_system_alarm(ex_main_msg.arg2); /* 致命的なエラーなので無限ループ      */
			}
			else
			{
				/* system error */
				_main_system_error(0, 102);
			}
		}
		else
		{
			/* system error */
			_main_system_error(0, 103);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		/* system error */
		_main_system_error(0, 104);
		break;
	}
}


/*********************************************************************//**
 * @brief accept message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rcinit_dl_start(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_RC_DL_START_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_DL_DATA);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* エラー処理 */
			_main_system_alarm(ALARM_CODE_RC_DWERR); /* 致命的なエラーなので無限ループ      */
		}
		else
		{
			/* system error */
			_main_system_error(0, 102);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_main_msg.arg2 == ALARM_CODE_RC_ROM || ex_main_msg.arg2 == ALARM_CODE_RC_COM)
			{
				_main_system_alarm(ex_main_msg.arg2); /* 致命的なエラーなので無限ループ      */
			}
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		/* system error */
		_main_system_error(0, 119);
		break;
	}
}


/*********************************************************************//**
 * @brief accept message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rcinit_dl_data(void)
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_RC_DL_DATA_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_DL_END);

			/* send message to rc_task (TMSG_RC_DL_END_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_DL_END_REQ, 0, 0, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* エラー処理 */
			_main_system_alarm(ALARM_CODE_RC_DWERR); /* 致命的なエラーなので無限ループ      */
		}
		else
		{
			/* system error */
			_main_system_error(0, 120);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_main_msg.arg2 == ALARM_CODE_RC_ROM || ex_main_msg.arg2 == ALARM_CODE_RC_COM)
			{
				_main_system_alarm(ex_main_msg.arg2); /* 致命的なエラーなので無限ループ      */
			}
		}
		else
		{
			/* system error */
			_main_system_error(0, 121);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		/* system error */
		_main_system_error(0, 122);
		break;
	}
}


/*********************************************************************//**
 * @brief accept message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void rcinit_dl_end(void)
{
	u8 result = 0;
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_RC_DL_END_RSP:
		ex_rc_download_stat = 0;	/* RC download Ready */	

		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			result = rc_check_connected();

			if (result != 0)
			{
				/* エラー処理 */
				ex_rc_powerup_error = 1;
				_main_system_alarm(result); /* 致命的なエラーなので無限ループ      */
			}
			else
			{
				ex_rc_powerup_error = 0;
			}

			_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_CPU_RESET);

			/* send message to rc_task (TMSG_RC_INIT_REQ) */
			_main_send_msg(ID_RC_MBX, TMSG_RC_INIT_REQ, 0, 0, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			/* エラー処理 */
			_main_system_alarm(ALARM_CODE_RC_DWERR); /* 致命的なエラーなので無限ループ      */
		}
		else
		{
			/* system error */
			_main_system_error(0, 123);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			if (ex_main_msg.arg2 == ALARM_CODE_RC_ROM || ex_main_msg.arg2 == ALARM_CODE_RC_COM)
			{
				_main_system_alarm(ex_main_msg.arg2); /* 致命的なエラーなので無限ループ      */
			}
		}
		else
		{
			/* system error */
			_main_system_error(0, 124);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	default:
		/* system error */
		_main_system_error(0, 125);
		break;
	}
}


/*********************************************************************//**
 * @brief software information check
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8 software_version_check(void)
{
	u8 result = TRUE;
	u8 ii;

	for(ii = 0; ii < 3; ii++)
	{
		switch(ii)
		{
		case	0:
				result = memcmp((u8 *)&ex_model[0], (u8 *)&ex_model_bk[0], 16);
				break;
		case	1:
				result = memcmp((u8 *)&ex_protocol[0], (u8 *)&ex_protocol_bk[0], 8);
				break;
		case	2:
				result = memcmp((u8 *)&ex_country[0], (u8 *)&ex_country_bk[0], 8);
				break;
		default:
				break;
		}

		/* 情報不一致の場合はチェック中止 */
		if(result != 0)
		{
			break;
		}
	}

	/* 情報を更新 */
	memcpy((u8 *)&ex_model_bk[0], (u8 *)&ex_model[0], 16);
	memcpy((u8 *)&ex_protocol_bk[0], (u8 *)&ex_protocol[0], 8);
	memcpy((u8 *)&ex_country_bk[0], (u8 *)&ex_country[0], 8);

	/* I/Fモードの場合は前回が2金種か4金種設定かも確認する */
	if(rc_before_model != ex_rc_status.sst1A.bit.quad)
	{
		rc_before_model = ex_rc_status.sst1A.bit.quad;
		return(FALSE);
	}
	else
	{
		rc_before_model = ex_rc_status.sst1A.bit.quad;
	}

	if(result != 0)
	{
		return(FALSE);
	}
	else
	{
		return(TRUE);
	}
}


/*********************************************************************//**
 * @brief set before operation mode
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void set_recycler_operation_mode(u8 mode)
{
	rc_before_mode = mode;
	/* Write fram */
	_main_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_RTQ, FRAM_RC_BEFORE_STA, 0, 0);
}


/*********************************************************************//**
 * @brief check rc unit connected
 * @param[in]	None
 * @return 		== 0	OK
				!= 0	Error
 **********************************************************************/
static u8 rc_check_connected(void)
{
	u8 seq = 1;
	u8	result = 0;
	u8 re_retry_count = 2;
	u32 result_gpio3 = 0x000000;
	
//#if defined(USE_RC_ICE)
#if 0
	ex_rc_ready_timeout = 3000;			/* set timeout 3.0sec */

	seq = 4;
#endif

	while(seq != 0)
	{
		switch(seq)
		{
		case	1:		/* RC_SEQ_CPU_RESET */
				/* RC CPUをリセット */
				//GPIOPinWrite(SOC_GPIO1_REG, 28, GPIO_PIN_LOW);
				Gpio_out( GPIO_RC_RESET, GPIO_PIN_LOW );	//Reset開始

				ex_rc_detect_time = 100;		/* set timeout 100msec */

				seq = 2;
				break;
		case	2:		/* RC_SEQ_WAIT_NOT_READY */
				if(ex_rc_detect_time == 0)
				{
					seq = 3;
					ex_rc_detect_time = 3000;	/* set timeout 3.0sec */
				}
				break;
		case	3:		/* RC_SEQ_CONFIRM_RESET */
				if(ex_rc_detect_time > 0)
				{
					//result_gpio3 = GPIOPinRead(SOC_GPIO1_REG, 31);
					result_gpio3 = Gpio_in( GPIO_26 );

					if(result_gpio3 == 0) //Ready信号がLowになるのを待つ Low == Not Rady (Resetか効くのをまっている)
					{
					//RTQ Dead
						//GPIOPinWrite(SOC_GPIO1_REG, 28, GPIO_PIN_HIGH);
						Gpio_out( GPIO_RC_RESET, GPIO_PIN_HIGH ); //Reset解除 (RTQに対してResetが効いたのでResetを解除)

						ex_rc_ready_timeout = 3000;			/* set timeout 3.0sec */

						seq = 4;
					}
				}
				else
				{
					/* 通信エラー */
					seq = 0;
					result = ALARM_CODE_RC_COM;
				}
				break;
		case	4:		/* RC_SEQ_WAIT_READY */
				/* RC READY信号受信待ち */
				if(ex_rc_ready_timeout > 0)
				{
					//result_gpio3 = GPIOPinRead(SOC_GPIO1_REG, 31);
					result_gpio3 = Gpio_in( GPIO_26 );	//HighでReady == RTQが動作できる状態

					if(result_gpio3 != 0)
					{
					//RTQ OK
					//RTQが動作できる状態になったので、次のシーケンスで再度確認
						ex_rc_detect_time = 5;		/* set timeout 5msec */

						seq = 5;
					}
				}
				else
				{
					/* 通信エラー */
					seq = 0;
					result = ALARM_CODE_RC_COM;
				}
				break;
		case	5:		/* RC_SEQ_CONFIRM_READY */
				//result_gpio3 = GPIOPinRead(SOC_GPIO1_REG, 31);
				result_gpio3 = Gpio_in( GPIO_26 );

				if(result_gpio3 != 0)
				{
				//RTQ OK
				//RTQが動作できる状態	
					if(ex_rc_detect_time == 0)
					{
						seq = 0;
					}
				}
				else
				{
					if(--re_retry_count == 0)
					{
						/* 通信エラー */
						result = ALARM_CODE_RC_COM;
						seq = 0;
					}
					else
					{
						seq = 1;
					}
				}
				break;
		default:
				break;
		}
	}

	return(result);
}
#endif
