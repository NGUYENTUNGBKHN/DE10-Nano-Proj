/******************************************************************************/
/*! @addtogroup Group2
    @file       rc_rsp.c
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

u8 *Rom2str;									/* ダウンロード先頭アドレス	*/
u8 *Rom2end;									/* ダウンロード終了アドレス */
#if defined(UBA_RTQ_ICB)//#if defined(RFID_RECOVER)
	u8 *rc_rfid_read_buff;
#else
	static u8 *rc_rfid_read_buff;
#endif

/*********************************************************************//**
 * @brief status request response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_status_response(void)
{
	/* set current count, limit count */
	if(ex_rc_data_lock == 0)
	{
		if(ex_rc_status.sst1A.bit.quad)		/* RC-Quad */
		{
			RecycleSettingInfo.DenomiInfo[0].RecycleLimit = _rc_rx_buff.data[0];
			RecycleSettingInfo.DenomiInfo[1].RecycleLimit = _rc_rx_buff.data[1];
			RecycleSettingInfo.DenomiInfo[2].RecycleLimit = _rc_rx_buff.data[2];
			RecycleSettingInfo.DenomiInfo[3].RecycleLimit = _rc_rx_buff.data[3];

			RecycleSettingInfo.DenomiInfo[0].RecycleCurrent = _rc_rx_buff.data[4];
			RecycleSettingInfo.DenomiInfo[1].RecycleCurrent = _rc_rx_buff.data[5];
			RecycleSettingInfo.DenomiInfo[2].RecycleCurrent = _rc_rx_buff.data[6];
			RecycleSettingInfo.DenomiInfo[3].RecycleCurrent = _rc_rx_buff.data[7];
		}
		else				/* RC-Twin */
		{
			RecycleSettingInfo.DenomiInfo[0].RecycleLimit = _rc_rx_buff.data[0];
			RecycleSettingInfo.DenomiInfo[1].RecycleLimit = _rc_rx_buff.data[1];
			RecycleSettingInfo.DenomiInfo[2].RecycleLimit = 0;
			RecycleSettingInfo.DenomiInfo[3].RecycleLimit = 0;

			RecycleSettingInfo.DenomiInfo[0].RecycleCurrent = _rc_rx_buff.data[4];
			RecycleSettingInfo.DenomiInfo[1].RecycleCurrent = _rc_rx_buff.data[5];
			RecycleSettingInfo.DenomiInfo[2].RecycleCurrent = 0;
			RecycleSettingInfo.DenomiInfo[3].RecycleCurrent = 0;
		}
	}
}

/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_reset_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_RESET_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_RESET_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_reset_skip_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_RESET_SKIP_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);	/* To main_task(TMSG_RC_RESET_SKIP_RSP) */
}

/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_cancel_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_CANCEL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_CANCEL_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_reject_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_REJECT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_REJECT_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_pause_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_PAUSE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_PAUSE_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_payout_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_PAYOUT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_PAYOUT_RSP) */
}

/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_encryption_key_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DEL_RSP, TMSG_RC_ENC_KEY, TMSG_SUB_SUCCESS, 0, 0);		/* To main_task(TMSG_RC_GET_MOTOR_SPEED_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_encryption_num_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DEL_RSP, TMSG_RC_ENC_NUM, TMSG_SUB_SUCCESS, 0, 0);		/* To main_task(TMSG_RC_GET_MOTOR_SPEED_RSP) */
}



/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_stack_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_STACK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_STACK_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_collect_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_COLLECT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_STORE_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_sol_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_SOL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);			/* To main_task(TMSG_RC_SOL_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_feed_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_FEED_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);			/* To main_task(TMSG_RC_FEED_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_flapper_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_FLAPPER_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_FLAPPER_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_sensor_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_SENSOR_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);			/* To main_task(TMSG_RC_DRUM_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_drum_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DRUM_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);			/* To main_task(TMSG_RC_DRUM_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_display_response(void)
{
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_wu_reset_response(void)
{
}


/*********************************************************************//**
 * @brief sensor active response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_sensor_active_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_SENSOR_ACTIVE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);	/* To main_task(TMSG_RC_SENSOR_ACTIVE_RSP) */
}

void rc_sensor_condition_response(void)
{
	ex_position_dirt[1] = _rc_rx_buff.data[0];	/* Twin */
	ex_position_dirt[2] = _rc_rx_buff.data[1];	/* Quad */
	ex_position_dirt[3] = _rc_rx_buff.data[2];	/* RS	*/

	//not use memcpy((u8 *)&ex_rc_position_current_da[0], (u8 *)&_rc_rx_buff.data[3], sizeof(ex_rc_position_current_da));			// 16byte
	//not use memcpy((u8 *)&ex_rc_position_shipment_da[0], (u8 *)&_rc_rx_buff.data[19], sizeof(ex_rc_position_shipment_da));		// 16byte

	//not use ex_position_dirt_rc_check = 1;
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_write_serial_no_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_WRITE_SERIALNO_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);	/* To main_task(TMSG_RC_READ_SHIP_SERIALNO_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_read_serial_no_response(void)
{
	switch(read_serailno_data.fram_param)
	{
	case	0:
			memcpy((u8 *)&read_serailno_data.version[0], (u8 *)&_rc_rx_buff.data[0], sizeof(read_serailno_data.version));			// 2byte
			memcpy((u8 *)&read_serailno_data.date[0], (u8 *)&_rc_rx_buff.data[2], sizeof(read_serailno_data.date));				// 8byte
			memcpy((u8 *)&read_serailno_data.serial_no[0], (u8 *)&_rc_rx_buff.data[10], sizeof(read_serailno_data.serial_no));	// 12byte
			break;
	case	1:
			memcpy((u8 *)&read_serailno_data.version[0], (u8 *)&_rc_rx_buff.data[22], sizeof(read_serailno_data.version));		// 2byte
			memcpy((u8 *)&read_serailno_data.date[0], (u8 *)&_rc_rx_buff.data[24], sizeof(read_serailno_data.date));				// 8byte
			memcpy((u8 *)&read_serailno_data.serial_no[0], (u8 *)&_rc_rx_buff.data[32], sizeof(read_serailno_data.serial_no));	// 12byte
			break;
	case	2:
			memcpy((u8 *)&read_serailno_data.version[0], (u8 *)&_rc_rx_buff.data[44], sizeof(read_serailno_data.version));		// 2byte
			memcpy((u8 *)&read_serailno_data.date[0], (u8 *)&_rc_rx_buff.data[46], sizeof(read_serailno_data.date));				// 8byte
			memcpy((u8 *)&read_serailno_data.serial_no[0], (u8 *)&_rc_rx_buff.data[54], sizeof(read_serailno_data.serial_no));	// 12byte
			break;
	}

	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_READ_SERIALNO_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);	/* To main_task(TMSG_RC_READ_SERIALNO_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_write_mainte_serial_no_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_WRITE_MAINTE_SERIALNO_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);	/* To main_task(TMSG_RC_READ_SHIP_SERIALNO_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_read_mainte_serial_no_response(void)
{
	/* Main FRAM Serial No. */
	memcpy((u8 *)&read_mente_serailno_data[0].version[0], (u8 *)&_rc_rx_buff.data[0], sizeof(read_mente_serailno_data[0].version));			// 2byte
	memcpy((u8 *)&read_mente_serailno_data[0].date[0], (u8 *)&_rc_rx_buff.data[2], sizeof(read_mente_serailno_data[0].date));				// 8byte
	memcpy((u8 *)&read_mente_serailno_data[0].serial_no[0], (u8 *)&_rc_rx_buff.data[10], sizeof(read_mente_serailno_data[0].serial_no));	// 12byte

	/* Twin FRAM Serial No. */
	memcpy((u8 *)&read_mente_serailno_data[1].version[0], (u8 *)&_rc_rx_buff.data[22], sizeof(read_mente_serailno_data[1].version));		// 2byte
	memcpy((u8 *)&read_mente_serailno_data[1].date[0], (u8 *)&_rc_rx_buff.data[24], sizeof(read_mente_serailno_data[1].date));				// 8byte
	memcpy((u8 *)&read_mente_serailno_data[1].serial_no[0], (u8 *)&_rc_rx_buff.data[32], sizeof(read_mente_serailno_data[1].serial_no));	// 12byte

	/* Quad FRAM Serial No. */
	memcpy((u8 *)&read_mente_serailno_data[2].version[0], (u8 *)&_rc_rx_buff.data[44], sizeof(read_mente_serailno_data[2].version));		// 2byte
	memcpy((u8 *)&read_mente_serailno_data[2].date[0], (u8 *)&_rc_rx_buff.data[46], sizeof(read_mente_serailno_data[2].date));				// 8byte
	memcpy((u8 *)&read_mente_serailno_data[2].serial_no[0], (u8 *)&_rc_rx_buff.data[54], sizeof(read_mente_serailno_data[2].serial_no));	// 12byte

	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_READ_MAINTE_SERIALNO_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);	/* To main_task(TMSG_RC_READ_MAINTE_SERIALNO_RSP) */
}

/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_write_edition_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_WRITE_EDITION_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);	/* To main_task(TMSG_RC_READ_SHIP_SERIALNO_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_read_edition_response(void)
{
#if 1
	memcpy((u8 *)&read_editionno_data.main[0], (u8 *)&_rc_rx_buff.data[0], sizeof(read_editionno_data.main));
	memcpy((u8 *)&read_editionno_data.twin[0], (u8 *)&_rc_rx_buff.data[1], sizeof(read_editionno_data.twin));
	memcpy((u8 *)&read_editionno_data.quad[0], (u8 *)&_rc_rx_buff.data[2], sizeof(read_editionno_data.quad));
#endif
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_READ_EDITION_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);	/* To main_task(TMSG_RC_READ_SERIALNO_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_diag_end_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DIAG_END_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);	/* To main_task(TMSG_RC_DIAG_END_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_diag_mot_fwd_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DIAG_MOT_FWD_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);	/* To main_task(TMSG_RC_DIAG_MOT_FWD_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_get_recycle_setting_response(void) //RTQからの設定受信
{
	/* set recycle denomi information */
	memcpy((u8 *)&RecycleSettingInfo.DenomiInfo[0].BoxNumber, (u8 *)&_rc_rx_buff.data[0], 8);			/* RC-Twin drum1	*/
	memcpy((u8 *)&RecycleSettingInfo.DenomiInfo[1].BoxNumber, (u8 *)&_rc_rx_buff.data[8], 8);			/* RC-Twin drum2	*/

	if(ex_rc_status.sst1A.bit.quad)		/* RC-Quad */
	{
		memcpy((u8 *)&RecycleSettingInfo.DenomiInfo[2].BoxNumber, (u8 *)&_rc_rx_buff.data[16], 8);			/* RC-Quad drum1	*/
		memcpy((u8 *)&RecycleSettingInfo.DenomiInfo[3].BoxNumber, (u8 *)&_rc_rx_buff.data[24], 8);			/* RC-Quad drum2	*/
	}
	else
	{
	    memset((u8 *)&RecycleSettingInfo.DenomiInfo[2].BoxNumber, 0, 8);
	    memset((u8 *)&RecycleSettingInfo.DenomiInfo[3].BoxNumber, 0, 8);
	}

	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_GET_RECYCLE_SETTING_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_GET_RECYCLE_SETTING_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_resume_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_RESUME_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_GET_RECYCLE_SETTING_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_version_response(void)
{
	u8 count;
	u8 *ptr;
	int ret;
#if 0
	u8 id[18];
#endif 
	memcpy((u8 *)&RecycleSoftInfo.FlashRomid[0], (u8 *)&_rc_rx_buff.data[0], 18);

#if 1
	ret = memcmp((u8 *)RC_DOWNLOAD_DATA_ROMID, (u8 *)&RecycleSoftInfo.FlashRomid[0], 18);
#else
	ret = 0;
	memcpy((u8 *)&id[0], (u8 *)RC_DOWNLOAD_DATA_ROMID, 18);
#endif 

	if(ret != 0)
	{
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_VERSION_RSP, TMSG_SUB_ALARM, 0, 0, 0);		/* To main_task(TMSG_RC_VERSION_RSP) */
	}
	else
	{
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_VERSION_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_VERSION_RSP) */
	}
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_error_clear_response(void)
{
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_sw_clear_response(void)
{

}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_error_detail_response(void)
{
	ex_rc_error_code = (u16)_rc_rx_buff.data[0];
	ex_rc_error_code = ex_rc_error_code << 8 | _rc_rx_buff.data[1];

	switch (ex_rc_error_code)
	{
	case ALARM_CODE_FEED1_TIMEOUT:
	case ALARM_CODE_FEED2_TIMEOUT:
	case ALARM_CODE_FEED1_MOTOR_LOCK:
	case ALARM_CODE_FEED2_MOTOR_LOCK:
	case ALARM_CODE_FEED1_JAM_AT_TR:
	case ALARM_CODE_FEED2_JAM_AT_TR:
	case ALARM_CODE_FEED1_JAM_AT_DR:
	case ALARM_CODE_FEED2_JAM_AT_DR:
	case ALARM_CODE_FEED1_DOUBLE_BILL:
	case ALARM_CODE_FEED2_DOUBLE_BILL:
	case ALARM_CODE_TWIN_DRUM1_TIMEOUT:
	case ALARM_CODE_TWIN_DRUM2_TIMEOUT:
	case ALARM_CODE_QUAD_DRUM1_TIMEOUT:
	case ALARM_CODE_QUAD_DRUM2_TIMEOUT:
	case ALARM_CODE_TWIN_DRUM1_MOTOR_LOCK:
	case ALARM_CODE_TWIN_DRUM2_MOTOR_LOCK:
	case ALARM_CODE_QUAD_DRUM1_MOTOR_LOCK:
	case ALARM_CODE_QUAD_DRUM2_MOTOR_LOCK:
	case ALARM_CODE_TWIN_DRUM1_JAM:
	case ALARM_CODE_TWIN_DRUM2_JAM:
	case ALARM_CODE_QUAD_DRUM1_JAM:
	case ALARM_CODE_QUAD_DRUM2_JAM:
		ex_cline_status_tbl.ex_rc_after_jam = 1;
		break;
	case ALARM_CODE_DISCONNECT_TWIN:
	case ALARM_CODE_DISCONNECT_QUAD:
		break;
	default:
		ex_cline_status_tbl.ex_rc_after_jam = 0;
		break;
	}

	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_ERROR_DETAIL_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_ERROR_DETAIL_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_get_dipsw_response(void)
{
	ex_rc_dip_sw = _rc_rx_buff.data[0];
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_GET_DIPSW_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_GET_DIPSW_RSP) */
}


/*********************************************************************//**
* @brief mode response receiving procedure
* @param[in]	None
* @return 		None
**********************************************************************/
void rc_boot_version_response(void)
{
	memcpy((u8 *)&RecycleSoftInfo.BootRomid[0], (u8 *)&_rc_rx_buff.data[0], 28);
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_BOOT_VERSION_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_BOOT_VERSION_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_set_recycle_setting_response(void)
{
	ex_rc_data_lock = 0;
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_RECYCLE_SETTING_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_RECYCLE_SETTING_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_set_enable_drum_response(void)
{
	ex_rc_enable = _rc_rx_buff.data[0];
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_set_current_count_setting_response(void)
{
	ex_rc_data_lock = 0;
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_CURRENT_COUNT_SETTING_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_CURRENT_COUNT_SETTING_RSP */
}



/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_set_motor_speed_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_SET_MOTOR_SPEED_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_SET_MOTOR_SPEED_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_retry_bill_dir_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_RETRY_BILL_DIR_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_RETRY_BILL_DIR_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_feed_box_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_FEED_BOX_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);				/* To main_task(TMSG_RC_FEED_BOX_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_drum_gap_adj_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DRUM_GAP_ADJ_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);				/* To main_task(TMSG_RC_DRUM_GAP_ADJ_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_billback_drum_payout_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_BILLBACK_DRUM_PAYOUT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_BILLBACK_DRUM_PAYOUT_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_feed_box_drum_payout_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_FEEDBOX_DRUM_PAYOUT_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_FEEDBOX_DRUM_PAYOUT_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_search_response(void)
{
    if(_rc_rx_buff.data[0] == CMD_RC_SEARCH_PRM_OUT_DRUM)
    {
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_SEARCH_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);				/* To main_task(TMSG_RC_BOX_SEARCH_RSP) */
	}
	else
	{
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_SEARCH_RSP, TMSG_SUB_SEARCH_STILL_BOX, 0, 0, 0);
	}
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_prefeed_stack_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_PREFEED_STACK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);				/* To main_task(TMSG_RC_PREFEED_STACK_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_last_feed_cashbox_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_LAST_FEED_CASHBOX_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);			/* To main_task(TMSG_RC_LAST_FEED_CASHBOX_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_force_stack_drum_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_FORCE_STACK_DRUM_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);			/* To main_task(TMSG_RC_FORCE_STACK_DRUM_RSP) */
}
	

/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_billback_response(void)
{
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_set_state_response(void)
{
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_set_mode_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_MODE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_MODE_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_fram_read_response(void)
{
	if(ex_rc_configuration.board_type == RC_OLD_BOARD)
	{
		rc_fram_log.read_length = _rc_rx_buff.length - 0x0C;
	}
	else
	{
		rc_fram_log.read_length = _rc_rx_buff.length - 0x0E;
	}
	rc_fram_log.wait_flg = FALSE;
	memcpy((u8 *)&rc_fram_log.read_data[0], (u8 *)&_rc_rx_buff.data[0], rc_fram_log.read_length);
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_fram_check_response(void)
{

	ex_fram_check = _rc_rx_buff.data[0];

	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_FRAM_CHECK_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);	/* To main_task(TMSG_RC_FRAM_CHECK_RSP) */
}

/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_drum_tape_pos_adj_response(void)
{	
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_DRUM_TAPE_POS_ADJ_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);	/* To main_task(TMSG_RC_DRUM_TAPE_POS_ADJ_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_start_adj_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_START_SENS_ADJ_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_START_SENS_ADJ_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_get_adj_data_response(void)
{
	u8 cnt;
	
	ex_sens_adj_err_data[0] = _rc_rx_buff.data[0];
	ex_sens_adj_err_data[1] = _rc_rx_buff.data[1];
	ex_sens_adj_err_data[2] = _rc_rx_buff.data[2];
	ex_sens_adj_err_data[3] = _rc_rx_buff.data[3];
	ex_sens_adj_err_data[4] = _rc_rx_buff.data[4];

	for(cnt = 0; cnt < RC_SENSOR_MAX; cnt++)
	{
		ex_rc_adj_data[cnt].ad600da  = _rc_rx_buff.data[5 + (cnt * 9)];
		ex_rc_adj_data[cnt].ad800da  = _rc_rx_buff.data[6 + (cnt * 9)];
		ex_rc_adj_data[cnt].da       = _rc_rx_buff.data[7 + (cnt * 9)];
		ex_rc_adj_data[cnt].ad600[0] = _rc_rx_buff.data[8 + (cnt * 9)];
		ex_rc_adj_data[cnt].ad600[1] = _rc_rx_buff.data[9 + (cnt * 9)];
		ex_rc_adj_data[cnt].ad800[0] = _rc_rx_buff.data[10 + (cnt * 9)];
		ex_rc_adj_data[cnt].ad800[1] = _rc_rx_buff.data[11 + (cnt * 9)];
		ex_rc_adj_data[cnt].ad[0]    = _rc_rx_buff.data[12 + (cnt * 9)];
		ex_rc_adj_data[cnt].ad[1]    = _rc_rx_buff.data[13 + (cnt * 9)];
	}

	if( (_rc_rx_buff.data[0] == 0) &&
		(_rc_rx_buff.data[1] == 0) &&
		(_rc_rx_buff.data[2] == 0) &&
		(_rc_rx_buff.data[3] == 0) &&
		(_rc_rx_buff.data[4] == 0))
	{
		
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_READ_SENS_ADJ_DATA_RSP, 
					TMSG_SUB_SUCCESS, 0, 0, 0);					/* To main_task(TMSG_RC_READ_SENS_ADJ_DATA_RSP) */
	}
	else
	{
		_rc_send_msg(ID_MAIN_MBX, TMSG_RC_READ_SENS_ADJ_DATA_RSP, 
					TMSG_SUB_ALARM, 0, 0, 0);					/* To main_task(TMSG_RC_READ_SENS_ADJ_DATA_RSP) */
	}
}
	
	
/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_sens_adj_write_fram_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_SENS_ADJ_WRITE_FRAM_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_SENS_ADJ_WRITE_FRAM_RSP) */
}

	
/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_sens_adj_read_fram_response(void)
{
	memcpy((u8 *)&ex_rc_adj_data, (u8 *)&_rc_rx_buff.data[0], sizeof(ex_rc_adj_data));
	
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_SENS_ADJ_READ_FRAM_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_SENS_ADJ_READ_FRAM_RSP) */
}
	
	
/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_perform_test_write_fram_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_PERFORM_TEST_WRITE_FRAM_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_PERFORM_TEST_WRITE_FRAM_RSP) */
}
	
	
/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_perform_test_read_fram_response(void)
{
	memcpy((u8 *)&ex_perform_test_data, (u8 *)&_rc_rx_buff.data[0], sizeof(ex_perform_test_data));
	
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_PERFORM_TEST_READ_FRAM_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_PERFORM_TEST_READ_FRAM_RSP) */
}

#if defined(UBA_RTQ) //#if defined(RC_BOARD_GREEN)
/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_get_pos_onoff_response(void)
{
	memcpy((u8 *)&ex_rc_new_adjustment_data.pos[0], (u8 *)&_rc_rx_buff.data[0], 3);

	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_GET_POS_ONOFF_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_GET_POS_ONOFF_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_set_pos_da_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_GET_POS_DA_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_GET_POS_DA_RSP) */
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_set_pos_gain_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_GET_POS_GAIN_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_GET_POS_GAIN_RSP) */
}

/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_get_confguration_response(void)
{
	/*  set rc confguration */
	memcpy((u8 *)&ex_rc_configuration.unit_type, (u8 *)&_rc_rx_buff.data[0], 3); //ここで新旧が判別できる

	/* 旧基板の場合は強制的にRSユニット無し/RFID無しに設定する */
	if(ex_rc_configuration.board_type == 0)
	{
		ex_rc_configuration.unit_type = 0;
		ex_rc_configuration.rfid_module = 0;
	}

	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_GET_CONFIGURATION_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_GET_CONFIGURATION_RSP) */
}
//#endif // RC_BOARD_GREEN

//#if defined(UBA_RTQ_ICB)
/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_rfid_test_response(void) //生産のテストモードに使用
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_RFID_TEST_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RC_RFID_TEST_RSP) */
}

/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
//#if defined(UBA_RS)
void rs_flapper_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RS_FLAPPER_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RS_FLAPPER_RSP) */
}

/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rs_display_response(void)
{
	_rc_send_msg(ID_MAIN_MBX, TMSG_RS_DISPLAY_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);		/* To main_task(TMSG_RS_FLAPPER_RSP) */
}
//#endif // UBA_RS

void rc_rfid_reset_response(void)
{
	/* ENQに対するACK受信の場合 */
	if( RFID_ENQ == rc_last_send )
	{
		/* 完了をメールBOXに通知 */
		if (ex_rc_rfid_src_req == ID_MAIN_TASK)
		{
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_RFID_RESET_RSP, TMSG_SUB_SUCCESS, _rc_rx_buff.data[0], 0, 0);
		}
		else
		{
			_rc_send_msg(ID_ICB_MBX, TMSG_RC_RFID_RESET_RSP, TMSG_SUB_SUCCESS, _rc_rx_buff.data[0], 0, 0);
		}
	}
	else
	{
		//RC task
		_rc_send_msg(ID_RC_MBX, TMSG_RC_RFID_RESET_REQ, RFID_ENQ, 0, 0, 0); //2024-12-18
	}
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_rfid_read_response(void)
{
	/* ENQに対するACK受信の場合 */
	if( RFID_ENQ == rc_last_send )
	{
		if (ex_rc_rfid_src_req == ID_MAIN_TASK)
		{
			memcpy(rc_rfid_read_buff, (u8 *)&_rc_rx_buff.data[1], _rc_rx_buff.length - 0x0D);
			/* 完了をICBタスクに通知 */
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_RFID_READ_RSP, TMSG_SUB_SUCCESS, _rc_rx_buff.data[0], 0, 0);

		}
		else
		{
			memcpy(rc_rfid_read_buff, (u8 *)&_rc_rx_buff.data[1], _rc_rx_buff.length - 0x0D);
			/* 完了をICBタスクに通知 */
			_rc_send_msg(ID_ICB_MBX, TMSG_RC_RFID_READ_RSP, TMSG_SUB_SUCCESS, _rc_rx_buff.data[0], 0, 0);	
		}
	}
	else
	{
	//こっちの処理は、RCタスクからENQを送信させたいので、RCタスクに対してメッセージを送りたい
		_rc_send_msg(ID_RC_MBX, TMSG_RC_RFID_READ_REQ, RFID_ENQ, 0, 0, 0);//2024-12-18		
	}
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_rfid_write_response(void)
{
	/* ENQに対するACK受信の場合 */
	if( RFID_ENQ == rc_last_send )
	{
		if (ex_rc_rfid_src_req == ID_MAIN_TASK)
		{
			/* 完了を通知 */
			ex_rtq_rfid_data = 0; //202-07-10
			_rc_send_msg(ID_MAIN_MBX, TMSG_RC_RFID_WRITE_RSP, TMSG_SUB_SUCCESS, _rc_rx_buff.data[0], 0, 0);
			#if defined(UBA_RTQ_ICB)
			_hal_status_led_orange(0); //2025-03-25
			#endif	
		}
		else
		{
			/* 完了を通知 */
			_rc_send_msg(ID_ICB_MBX, TMSG_RC_RFID_WRITE_RSP, TMSG_SUB_SUCCESS, _rc_rx_buff.data[0], 0, 0);
		}
	}
	else
	{
	//こっちの処理は、RCタスクからENQを送信させたいので、RCタスクに対してメッセージを送りたい
		_rc_send_msg(ID_RC_MBX, TMSG_RC_RFID_WRITE_REQ, RFID_ENQ, 0, 0, 0);//2024-12-18
	}
}
#endif // UBA_RTQ_ICB

/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_error_count_clear_response(void)
{
}


/*********************************************************************//**
 * @brief mode response receiving procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_get_motor_speed_response(void)
{
	u8 cnt;
	
	for(cnt = 0; cnt < 7; cnt++)
	{
		ex_rc_motor_speed[cnt][0] = _rc_rx_buff.data[0 + (cnt * 2)];
		ex_rc_motor_speed[cnt][1] = _rc_rx_buff.data[1 + (cnt * 2)];
		
		ex_rc_motor_duty[cnt] = _rc_rx_buff.data[14 + cnt];
	}

	_rc_send_msg(ID_MAIN_MBX, TMSG_RC_GET_MOTOR_SPEED_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);			/* To main_task(TMSG_RC_GET_MOTOR_SPEED_RSP) */
}

/*********************************************************************//**
 * @brief Command receiving procedure
 * @param[in]	_rc_rx_buff.cmd : command code
 * @return 		None
 **********************************************************************/
void rc_received_check_proc(void)
{
	_set_rc_status();

	if (_rc_rx_buff.del_cmd != CMD_RC_DEL_RSP)
	{
		switch (_rc_rx_buff.cmd)
		{
		case CMD_RC_STATUS_RSP:
			rc_status_response();
			break;
		case CMD_RC_RESET_SKIP_RSP:
			rc_reset_skip_response();
			break;
		case CMD_RC_REJECT_RSP:
			rc_reject_response();
			break;
		case CMD_RC_PAUSE_RSP:
			rc_pause_response();
			break;
		case CMD_RC_RESUME_RSP:
			rc_resume_response();
			break;
		case CMD_RC_BOOT_VERSION_RSP:
			rc_boot_version_response();
			break;
		case CMD_RC_VERSION_RSP:
			rc_version_response();
			break;
		case CMD_RC_READ_EDITION_RSP:
			rc_read_edition_response();
			break;
		case CMD_RC_PREFEED_STACK_RSP:
			rc_prefeed_stack_response();
			break;
		case CMD_RC_GET_RECYCLE_SETTING_RSP:
			rc_get_recycle_setting_response();
			break;
		case CMD_RC_RECYCLE_SETTING_RSP:
			rc_set_recycle_setting_response();
			break;
		case CMD_RC_MODE_RSP:
			rc_set_mode_response();
			break;
		case CMD_RC_READ_MAINTENANCE_SERIAL_NO_RSP:
			rc_read_mainte_serial_no_response();
			break;
		case CMD_RC_GET_DIPSW_RSP:
			rc_get_dipsw_response();
			break;
		case CMD_RC_STATE_RSP:
			rc_set_state_response();
			break;
		case CMD_RC_DISPLAY_RSP:
			rc_display_response();
			break;
		case CMD_RC_SW_CLEAR_RSP:
			rc_sw_clear_response();
			break;
		case CMD_RC_FEED_RSP:
			rc_feed_response();
			break;
		case CMD_RC_GET_MOTOR_SPEED_RSP:
			rc_get_motor_speed_response();
			break;
		case CMD_RC_DRUM_RSP:
			rc_drum_response();
			break;
		case CMD_RC_SENSOR_RSP:
			rc_sensor_response();
			break;
		case CMD_RC_FLAPPER_RSP:
			rc_flapper_response();
			break;
		case CMD_RC_SOL_RSP:
			rc_sol_response();
			break;
		case CMD_RC_ENABLE_DRUM_RSP:
			rc_set_enable_drum_response();
			break;
		case CMD_RC_SET_MOTOR_SPEED_RSP:
			rc_set_motor_speed_response();
			break;
		case CMD_RC_RESET_RSP:
			rc_reset_response();
			break;
		case CMD_RC_STACK_RSP:
			rc_stack_response();
			break;
		case CMD_RC_PAYOUT_RSP:
			rc_payout_response();
			break;
		case CMD_RC_COLLECT_RSP:
			rc_collect_response();
			break;
		case CMD_RC_LAST_FEED_CASHBOX_RSP:
			rc_last_feed_cashbox_response();
			break;
		case CMD_RC_SEARCH_RSP:
			rc_search_response();
			break;
		case CMD_RC_ERROR_CLEAR_RSP:
			rc_error_clear_response();
			break;
		case CMD_RC_SET_CURRENT_COUNT_RSP:
			rc_set_current_count_setting_response();
			break;
		case CMD_RC_ERROR_DETAIL_RSP:
			rc_error_detail_response();
			break;
		case CMD_RC_CANCEL_RSP:
			rc_cancel_response();
			break;
		case CMD_RC_RETRY_BILL_DIR_RSP:
			rc_retry_bill_dir_response();
			break;
		case CMD_RC_FEED_BOX_RSP:
			rc_feed_box_response();
			break;
		case CMD_RC_WRITE_SERIAL_NO_RSP:
			rc_write_serial_no_response();
			break;
		case CMD_RC_READ_SERIAL_NO_RSP:
			rc_read_serial_no_response();
			break;
		case CMD_RC_START_ADJ_RSP:
			rc_start_adj_response();
			break;
		case CMD_RC_GET_ADJ_DATA_RSP:
			rc_get_adj_data_response();
			break;
		case CMD_RC_DRUM_TAPE_ADJ_RSP:
			rc_drum_tape_pos_adj_response();
			break;
		case CMD_RC_FRAM_CHECK_RSP:
			rc_fram_check_response();
			break;
		case CMD_RC_SENS_ADJ_WRITE_FRAM_RSP:
			rc_sens_adj_write_fram_response();
			break;
		case CMD_RC_SENS_ADJ_READ_FRAM_RSP:
			rc_sens_adj_read_fram_response();
			break;
		case CMD_RC_PERFORM_TEST_WRITE_FRAM_RSP:
			rc_perform_test_write_fram_response();
			break;
		case CMD_RC_PERFORM_TEST_READ_FRAM_RSP:
			rc_perform_test_read_fram_response();
			break;
		case CMD_RC_FRAM_READ_RSP:
			rc_fram_read_response();
			break;
		case CMD_RC_BILLBACK_DRUM_PAYOUT_RSP:
			rc_billback_drum_payout_response();
			break;
		case CMD_RC_FEED_BOX_DRUM_PAYOUT_RSP:
			rc_feed_box_drum_payout_response();
			break;
		case CMD_RC_FORCE_STACK_DRUM_RSP:
			rc_force_stack_drum_response();
			break;
		case CMD_RC_BILLBACK_RSP:
			rc_billback_response();
			break;
		case CMD_RC_SENSOR_ACTIVE_RSP:
			rc_sensor_active_response();
			break;
		case CMD_RC_SENSOR_CONDITION_RSP:
			rc_sensor_condition_response();
			break;
		case CMD_RC_ERROR_COUNT_CLEAR_RSP:
			rc_error_count_clear_response();
			break;
		case CMD_RC_DRUM_GAP_ADJ_RSP:
			rc_drum_gap_adj_response();
			break;
		case CMD_RC_DIAG_END_RSP:
			rc_diag_end_response();
			break;
		case CMD_RC_DIAG_MOT_FWD_RSP:
			rc_diag_mot_fwd_response();
			break;
		case CMD_RC_WRITE_EDITION_RSP:
			rc_write_edition_response();
			break;
	//#if defined(RC_BOARD_GREEN)
		case CMD_RC_GET_POS_ONOFF_RSP:
			rc_get_pos_onoff_response();
			break;
		case CMD_RC_SET_POS_DA_RSP:
			rc_set_pos_da_response();
			break;
		case CMD_RC_SET_POS_GAIN_RSP:
			rc_set_pos_gain_response();
			break;
		case CMD_RC_GET_CONFIGURATION_RSP:
			rc_get_confguration_response();
			break;
	//#endif // RC_BOARD_GREEN
	
 	//#if defined(UBA_RS)
		case CMD_RS_FLAPPER_RSP:
			rs_flapper_response();
			break;
		case CMD_RS_DISPLAY_RSP:
			rs_display_response();
			break;
 	//#endif

	//#if defined(UBA_RTQ_ICB)
		case CMD_RC_RFID_TEST_RSP:
			rc_rfid_test_response();
			break;
		case CMD_RC_RFID_READ_RSP:
			rc_rfid_read_response();
			break;
		case CMD_RC_RFID_WRITE_RSP:
			rc_rfid_write_response();
			break;
		case CMD_RC_RFID_RESET_RSP:
			rc_rfid_reset_response();
			break;
	//#endif
		/* Download fw*/
		case CMD_RC_DL_START_RSP:
			rc_dl_start_response();
			break;
		case CMD_RC_DL_DATA_RSP:
			rc_dl_data_response();
			break;
		case CMD_RC_DL_CHECK_RSP:
			rc_dl_check_response();
			break;
		case CMD_RC_DL_END_RSP:
			rc_dl_end_response();
		/* -------------------------- */
			break;
		default:
			break;
		}
	}
	else
	{
		switch(_rc_rx_buff.cmd)
		{
		case CMD_RC_ENC_KEY:
			rc_encryption_key_response();
			break;
		case CMD_RC_ENC_NUM:
			rc_encryption_num_response();
			break;
		case CMD_RC_PAYOUT_RSP:
			rc_payout_response();
			break;
		default:
			break;
		}
	}

	if(_rc_rx_buff.cmd == CMD_RC_DL_START_RSP)
	{
		/* 【RC_SEQ_DL_IDLE : 0x5200】 */
		_rc_set_seq(RC_SEQ_DL_IDLE);
	}
	else
	{
		/* 【RC_SEQ_IDLE : 0x5100】 */
		_rc_set_seq(RC_SEQ_IDLE);
	}
}

/*********************************************************************//**
 * @brief rfid read
 * @param[in]   None
 * @return      None
 **********************************************************************/
void rc_rfid_read(u16 addr, u16 length, u8 *data) //ICB task
{
	rc_rfid_read_buff = data;
	_icb_send_msg(ID_RC_MBX, TMSG_RC_RFID_READ_REQ, RFID_RUN, addr, length, 0); //2024-12-18 
}


#endif // UBA_RTQ
