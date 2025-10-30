/******************************************************************************/
/*! @addtogroup Main
    @file       mode_powerup.c
    @brief      powerup mode of main task
    @date       2018/03/05
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/05 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/
#include <string.h>
#include "systemdef.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "operation.h"
#include "sensor.h"
#include "sub_functions.h"
#include "status_tbl.h"
#ifdef _ENABLE_JDL
#include "jdl.h"
#endif	/* _ENABLE_JDL */

#if defined(UBA_RTQ)
#include "if_rc.h"
#endif // UBA_RTQ

#define EXT
#include "com_ram.c"

/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/
#if defined(UBA_RTQ)
static u8 main_search_unit;
static u8 main_search_direction;
static u8 main_search_in_drum;
#endif
/************************** PRIVATE FUNCTIONS *************************/

/************************** EXTERN FUNCTIONS *************************/

/************************** EXTERNAL VARIABLES *************************/
extern const unsigned char software_ver[64];
extern const u8 logid[];

/*********************************************************************//**
 * @brief powerup message procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_msg_proc(void)
{
	switch (ex_main_task_mode2)
	{
	case POWERUP_MODE2_FRAM_READ:
		powerup_fram_read();
		break;
	case POWERUP_MODE2_CLEAR_ICB_SETTING:
		powerup_clear_icb_setting();
		break;
	case POWERUP_MODE2_MGU_READ:
		powerup_mgu_read();
		break;
	case POWERUP_MODE2_DIPSW_INIT:
		powerup_dipsw_init();
		break;
	case POWERUP_MODE2_SENSOR_INIT:
		powerup_sensor_init();	//use _main_send_connection_task
		break;
	case POWERUP_MODE2_WAIT_REQ:
		powerup_wait_req(); //おそらく TMSG_CONN_STATUS
		break;
	case POWERUP_MODE2_SENSOR_ACTIVE: //lineタスクからの命令で紙幣を探す為に、センサアクティブ待ち
		powerup_sensor_active();
		break;
	case POWERUP_MODE2_STACKER_HOME: //紙幣を探す前にスタックHome命令
		powerup_stacker_home(); //use _main_send_connection_task
		break;
	case POWERUP_MODE2_SEARCH_BILL:
		powerup_search_bill(); //use _main_send_connection_task
		break;
	case POWERUP_MODE2_ALARM_BOX:
		powerup_alarm_box();//use _main_send_connection_task
		break;
	case POWERUP_MODE2_ALARM_CONFIRM_BOX:
		powerup_alarm_confirm_box();//use _main_send_connection_task
		break;
	case POWERUP_MODE2_MAG_INIT:
		powerup_mag_init();
		break;
	#if !defined(UBA_RTQ)//#if defined(GLI)
	case POWERUP_MODE2_ALARM_RECEIVE_RESET_GLI:
		powerup_alarm_receive_reset_gli();//use _main_send_connection_task
		break;
	#endif
#if defined(UBA_RTQ)
	case POWERUP_MODE2_RC_SENSOR_ACTIVE:
		powerup_rc_sensor_active();
		break;
	case POWERUP_MODE2_RC_SEARCH_BILL:
		powerup_rc_search_bill();
		break;
	case POWERUP_MODE2_ALARM_RC_UNIT:
		powerup_alarm_rc_unit();
		break;
	case POWERUP_MODE2_ALARM_CONFIRM_RC_UNIT:
		powerup_alarm_confirm_rc_unit();
		break;
#endif // UBA_RTQ
	default:
		/* system error ? */
		_main_system_error(0, 124);
		break;
	}
}


/*********************************************************************//**
 * @brief powerup read fram adjustment data procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_fram_read(void) //TMSG_CONN_INITIAL
{
#ifdef _ENABLE_JDL
	u8 dummy[8];
#endif

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_FRAM_READ_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			/* ここで確認しているsumはポジションセンサの待機時補正の待機時補正のバックアップ値*/
			/* CISや調整時のポジションセンサ、シリアル番号のまとめたsumは行っていない		*/
			/* CISなどの調整値のsum確認関数は _main_check_fram_adj_sum() */
			/* 調整値のsumエラーにするかの分岐はID-003モードとテストモードによって変える為、シーケンスの後半で行う */
			/* ID-003モードでのエラーはここで行ってもいいが、調整値のsum確認を一か所にする為、シーケンスの後半 */
            if(ALARM_CODE_OK != check_fram_tmpadj_sum())
            {
                // load from adj
				#if 1// //2022-11-21
                ex_position_da_adj.entrance = 128;
                ex_position_da_adj.centering = 128;
                ex_position_da_adj.apb_in = 128;
                ex_position_da_adj.apb_out = 128;
                ex_position_da_adj.exit = 128;
                ex_position_ga = 0;
				#else
                ex_position_da_adj.entrance = ex_adjustment_data.maintenance_value.pos_entrance_da;
                ex_position_da_adj.centering = ex_adjustment_data.maintenance_value.pos_centering_da;
                ex_position_da_adj.apb_in = ex_adjustment_data.maintenance_value.pos_apb_in_da;
                ex_position_da_adj.apb_out = ex_adjustment_data.maintenance_value.pos_apb_out_da;
                ex_position_da_adj.exit = ex_adjustment_data.maintenance_value.pos_exit_da;
                ex_position_ga = ex_adjustment_data.maintenance_value.pos_gain;
				#endif
            }
            ex_position_da.entrance = ex_position_da_adj.entrance;
            ex_position_da.centering = ex_position_da_adj.centering;
            ex_position_da.apb_in = ex_position_da_adj.apb_in;
            ex_position_da.apb_out = ex_position_da_adj.apb_out;
            ex_position_da.exit = ex_position_da_adj.exit;
		    ex_position_da.ent_threshold = POSITION_THRESHOLD_DETECTION;
            ex_position_da.ext_threshold = POSITION_THRESHOLD_DETECTION;

		#if MAG1_ENABLE
			//調整されていないの不定な値でも、MAGがついているかの通信に使う為問題なし
			//MAGがついているかの確認は、センサ調整時の自動判別に必要
			memcpy(&ex_mag_adj, &ex_adjustment_data.mag_adj_value, sizeof(MAG_SENSOR_VAL));

			//2024-04-24 保護処理をしなくても使用時に1 byteにキャストしているが念のため
			if(ex_mag_adj.ul_gain > MAG_GAIN_MAX)
			{
				ex_mag_adj.ul_gain = MAG_GAIN_MAX;
			}
			if(ex_mag_adj.ur_gain > MAG_GAIN_MAX)
			{
				ex_mag_adj.ur_gain = MAG_GAIN_MAX;
			}
		#endif

			// position sensor GAIN
			_main_set_position_gain();
			// position sensoer D/A
			_main_set_position_da();

			
		if ((ex_dipsw1 & 0x40) == 0x40)
		{
		//	jdl_init(1);//2025-10-02 デバッグ用
		}

		#ifdef _ENABLE_JDL
			// DONE:ログ初期化ルーチン
			memset(dummy,0,8);
			jdl_init(0);
			jdl_powerup();
			jdl_set_jdl_time(&dummy[0]);
			ex_start_jdl = 1;
		#endif	/* _ENABLE_JDL */
		#if defined(UBA_RTQ)
			/* 24-10-30 For RC set version and set rc setting */
			jdl_rc_set_version();
			jdl_rc_set_rc_setting();
		#endif
			/* For ICB setting */
			/* UBA-700-SS EUR ID-003 */
			/* バージョン名より前の文字列が異なる場合(Model name, Countory code, ID )、ICB無効*/
		    if ((JDL_E_DDATA == _jdl_sys_cmp(JDL_SYS_ADR_FIRM_VER, (u8 *)&software_ver[0], icb_get_version_address()))
		    && (logid[0] != '0')
			&& (logid[0] != '2'))
		    {
			/* ICB Disable and write ICB Disable FRAM*/
				set_ICBdisable_flag();
				_main_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ICB_SETTING, 0, 0, 0);
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_CLEAR_ICB_SETTING);
		    }
		    else
		    {
				_main_send_msg(ID_MGU_MBX, TMSG_MGU_READ_REQ, MGU_LOG, 0, 0, 0);
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_MGU_READ);
		    }
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
		//ここでのFRAMエラーはハード的に通信できないエラーなので、ホストに通知しても意味があまりない、
		//この時点では、通常ではDIP-SW処理もラインタスク処理も動いていないので、
		//ここでホスト通信することはやめる、DIP-SW処理が動いていないので、テストモードはインタフェースモードかも不明
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ex_main_msg.arg2, 0, 0, 0);
			_main_set_mode(MODE1_ALARM, ALARM_MODE2_FRAM);
		}
		else
		{
			/* system error ? */
			_main_system_error(1, 125);
		}
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	#if defined(UBA_RTQ)
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
		case TMSG_LINE_RC_ENABLE_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		break;
	
	#endif
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_INITIAL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief powerup write fram icb setting data procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_clear_icb_setting(void)	//TMSG_CONN_INITIAL
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_FRAM_WRITE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_MGU_MBX, TMSG_MGU_READ_REQ, MGU_LOG, 0, 0, 0);
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_MGU_READ);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_INITIAL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(1, 125);
		}
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	#if defined(UBA_RTQ)
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;	
	#endif

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_INITIAL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief powerup read mgu eeprom data procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_mgu_read(void)	//TMSG_CONN_INITIAL	
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_MGU_READ_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
	#if MAG1_ENABLE
			_main_set_pl_active(PL_ENABLE);
		
			_pl_cis_enable_set(1);
		
			_main_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_MAG_ADJUSTMENT, MAG_INIT_MODE_POWERUP, 0, 0);
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_MAG_INIT);
	#else
			_main_send_msg(ID_DIPSW_MBX, TMSG_DIPSW_INIT_REQ, 0, 0, 0, 0);
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_DIPSW_INIT);
	#endif
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_INITIAL, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			/* system error ? */
			_main_system_error(1, 125);
		}
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	#if defined(UBA_RTQ)
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	#endif

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_INITIAL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief powerup read dipsw procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_dipsw_init(void) //TMSG_CONN_INITIAL
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_DIPSW_INIT_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
		#if (DATA_COLLECTION_DEBUG==1) //2024-01-18
			if(1)
		#else
			if (ex_dipsw1 & DIPSW1_PERFORMANCE_TEST) //ok check
		#endif
			{
			#if defined(_DEBUG_UART_LOOPBACK)
				_main_select_protocol();
				/* wakeup ID_CLINE_TASK */
			#else
				_main_init_performance_test_mio();
			#endif
				init_status_table();
				/*	set TEST Mode status	*/
				set_test_mode();
				_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_INIT_REQ, 0, 0, 0, 0);
				_main_set_mode(MODE1_TEST_STANDBY, TEST_STANDBY_MODE2_SENSOR_INIT);
			#if defined(UBA_RTQ)
				//UBA500とは異なるが、メカが単体の収納テストなどを行うので、ポーリング周期早いのにしておく	
				_main_send_msg(ID_RC_MBX, TMSG_RC_POLL_CHANGE_REQ, 1, 0, 0, 0);	// change polling time in RC_TASK
			#endif
			}
			else
			{

				#if 1//2024-04-24 候補3 2024-05-07
				// テストモード以外は、調整値を確認してsumエラーの場合、表示LEDで通知
				// DIP-SW 8 ONのみのテストモード以外のテストモードもsumチェックして、表示LED通知を検討したが
				// DIP-SW 8 以外を後で手動操作される場合と、いろいろなテストモードがあるので、表示LEDで上手くsumエラーを
				// 通知できなくなる可能性があるので、客先で使用する時のみ通知する様にする
				if(_main_check_fram_adj_sum() != ALARM_CODE_OK)
				{
					_main_system_alarm(ALARM_CODE_FRAM); //無限ループ
				}
				#endif
			#if defined(UBA_RTQ)
			//RTQはRTQ側へのダウンロードを行った場合、ID-003でステータスを通知する為
			//通常より通信開始がはやい場合がある
				if(ex_rc_download_flag==1)//2025-06-04
				{
				//起動時にRTQへのダウンロードを行った場合
					ex_rc_download_flag = 0;
					dly_tsk(100);
				}
				else
				{
				//通常のパワーアップ

				}
			#else

			#endif
				_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_INIT_REQ, 0, 0, 0, 0);
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_SENSOR_INIT);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_INITIAL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(1, 125);
		}
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	#if defined(UBA_RTQ)
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;	
	#endif

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_INITIAL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}



/*********************************************************************//**
 * @brief powerup sensor init procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_sensor_init(void) //use //use _main_send_connection_task TMSG_CONN_INITIAL
{

	u16 bill_remain;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_INIT_RSP:
		/* ex_main_msg.arg1 はex_position_sensor */
		/* Set FPGA mode : standby */
		_main_set_pl_active(PL_DISABLE);
		_main_set_sensor_active(0);
		if (!(is_box_set()))
		{
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_BOX);
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_BOX, 0, 0, 0);
			_main_send_connection_task(TMSG_CONN_INITIAL, ex_operating_mode, TMSG_SUB_ALARM, ALARM_CODE_BOX, 0);
		}
#if defined(UBA_RTQ)
		else if (!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
		{
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_RC_UNIT);
			_main_send_connection_task(TMSG_CONN_INITIAL, ex_operating_mode, TMSG_SUB_ALARM, ALARM_CODE_RC_REMOVED, 0);
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_RC_REMOVED, 0, 0, 0);
			_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
		}
#endif // UBA_RTQ
		else
		{
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_WAIT_REQ);
			_main_display_powerup();
			bill_remain = _main_bill_remain();
			_main_send_connection_task(TMSG_CONN_INITIAL, ex_operating_mode, TMSG_SUB_SUCCESS, bill_remain, 0); //2024-03-13 ラインタスクへの最初の1メッセージ
		}
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM && 
			ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_INITIAL, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_LINE_CURRENT_COUNT_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
		break;
	case TMSG_LINE_RC_ENABLE_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		break;
#endif // UBA_RTQ

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_INITIAL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief wait reset request procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_wait_req(void) ////use _main_send_connection_task
{
	u16 bill_remain;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_SEARCH_BILL_REQ:
	case TMSG_DLINE_SEARCH_BILL_REQ:
		ex_main_search_flag = ex_main_msg.arg1;
		_main_set_powerup_search();
		break;
#if defined(UBA_RTQ)
	case TMSG_CLINE_RC_SEARCH_BILL_REQ:
	case TMSG_DLINE_RC_SEARCH_BILL_REQ:
		main_search_unit = ex_main_msg.arg1;

		main_search_direction = ex_main_msg.arg2;

		_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_RC_SENSOR_ACTIVE);
		_main_set_sensor_active(1);
		break;
#endif // UBA_RTQ
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
	#if !defined(UBA_RTQ)	//2024-03-18
		else if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_FEED_LOST_BILL))
		{
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ex_main_msg.arg2, 0, 0, 0);
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_RECEIVE_RESET_GLI);
		}
	#endif
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_SENSOR_STATUS_INFO:
		if (!(is_box_set()))
		{
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_BOX);
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_BOX, 0, 0, 0);
			_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ALARM_CODE_BOX, 0, 0);
		}
		else
		{
			bill_remain = _main_bill_remain();
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, bill_remain, 0, 0);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_RC_UNIT);
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ALARM_CODE_RC_REMOVED, 0, 0);
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_RC_REMOVED, 0, 0, 0);
				_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM && 
			ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_LINE_CURRENT_COUNT_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
		break;
	case	TMSG_LINE_RC_ENABLE_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		break;
#endif // UBA_RTQ

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 126);
		}
		break;
	}
}



/*********************************************************************//**
 * @brief sensor active
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_sensor_active(void) //lineタスクからの命令で紙幣を探す為に、センサアクティブ待ち
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_ACTIVE_RSP:
		ex_multi_job.busy &= ~TASK_ST_SENSOR;
		if (!(ex_multi_job.busy))
		{ /* all job end */
			if (!(is_box_set()))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_SEARCH_BILL, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
			}
			else if (!is_ld_mode())
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_STACKER_HOME);
				_main_send_msg(ID_STACKER_MBX, TMSG_STACKER_HOME_REQ, 0, 0, 0, 0);
			}
			else if (ex_main_search_flag == SEARCH_TYPE_WINDOW)
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_SEARCH_BILL);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_SEARCH_REQ, FEED_SEARCH_OPTION_WINDOW, 0, 0, 0);
			}
			else
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_SEARCH_BILL);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_SEARCH_REQ, FEED_SEARCH_OPTION_NORMAL, 0, 0, 0);
			}
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		break;
	case TMSG_SENSOR_CIS_ACTIVE_RSP:
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM && 
			ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_SEARCH_BILL, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_LINE_CURRENT_COUNT_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
		break;
	case	TMSG_LINE_RC_ENABLE_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		break;
#endif // UBA_RTQ

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 127);
		}
		break;
	}
}



/*********************************************************************//**
 * @brief stacker home procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_stacker_home(void) //use _main_send_connection_task
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_STACKER_HOME_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			if (!(is_box_set()))
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_BOX);
				_main_send_connection_task(TMSG_CONN_SEARCH_BILL, TMSG_SUB_ALARM, ALARM_CODE_BOX, 0, 0);
			}
			else if (ex_main_search_flag == SEARCH_TYPE_WINDOW)
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_SEARCH_BILL);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_SEARCH_REQ, FEED_SEARCH_OPTION_WINDOW, 0, 0, 0);
			}
			else
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_SEARCH_BILL);
				_main_send_msg(ID_FEED_MBX, TMSG_FEED_SEARCH_REQ, FEED_SEARCH_OPTION_NORMAL, 0, 0, 0);
			}
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_SEARCH_BILL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 17);
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	#if defined(UBA_RTQ)
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	#endif

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 128);
		}
		break;
	}
}



/*********************************************************************//**
 * @brief search bill procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_search_bill(void) //use _main_send_connection_task
{
	u16 bill_remain;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_FEED_SEARCH_RSP:
		//2024-03-16 UBA500に合わせる
		//紙幣を探す時にFeedモータロックなどになる可能性もあるが、インタフェース的にPowser up前にエラーを通知する使用がないので、
		//BOX OPEN以外のエラーは無視する
		if (!(is_box_set()))
		{
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_BOX);
			_main_send_connection_task(TMSG_CONN_SEARCH_BILL, TMSG_SUB_ALARM, ALARM_CODE_BOX, 0, 0);
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_BOX, 0, 0, 0);			
		}
#if defined(UBA_RTQ)
		else if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
		{
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_RC_UNIT);
			_main_send_connection_task(TMSG_CONN_SEARCH_BILL, TMSG_SUB_ALARM, ALARM_CODE_RC_REMOVED, 0, 0);
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_RC_REMOVED, 0, 0, 0);			
			_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
		}
#endif // UBA_RTQ
		else
		{
			bill_remain = _main_bill_remain();
			/* Set FPGA mode : standby */
			_main_set_pl_active(PL_DISABLE);
			/* ex_main_msg.arg1 はex_position_sensor */
			_main_set_sensor_active(0);
			_main_send_connection_task(TMSG_CONN_SEARCH_BILL, TMSG_SUB_SUCCESS, bill_remain, 0, 0);
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_WAIT_REQ);
			/* Set LED and Bezel display when return to porweup */
			_main_display_powerup();
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM && 
			ex_rc_error_flag == 0)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_SEARCH_BILL, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_LINE_CURRENT_COUNT_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
		break;
	case	TMSG_LINE_RC_ENABLE_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		break;
#endif // UBA_RTQ
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 128);
		}
		break;
	}
}


/*********************************************************************//**
 * @brief unset box procedure (at power up state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_alarm_box(void)//use _main_send_connection_task
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
#if defined(_PROTOCOL_ENABLE_ID0G8)
		/* Does not recover */
#else
		if (is_box_set())
		{
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_CONFIRM_BOX);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RECOVER_WAIT, WAIT_TIME_RECOVER, 0, 0);
		#if defined(UBA_RTQ)
			/* box open中のbox open検出 */
			ex_rc_detect_next_box_open = 0;
		#endif
		}
#endif
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if((!(is_detect_rc_twin()) || !(is_detect_rc_quad())) && ex_rc_detect_next_box_open == 0)
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_RC_UNIT);
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ALARM_CODE_RC_REMOVED, 0, 0);
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_RC_REMOVED, 0, 0, 0);				
				_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
				/* box open中のbox open検出 */
				ex_rc_detect_next_box_open = 1;
			}
			else if (is_detect_rc_twin() && is_detect_rc_quad())
			{
				/* box open中のbox open検出 */
				ex_rc_detect_next_box_open = 0;
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM && 
			ex_rc_error_flag == 0)
		{
			ex_rc_error_flag = ex_main_msg.arg2;
		}
		break;
	case TMSG_LINE_CURRENT_COUNT_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
		break;
	case TMSG_LINE_RC_ENABLE_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		break;
#endif // UBA_RTQ
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 129);
		}
		break;
	}
}



/*********************************************************************//**
 * @brief confirm box procedure (at power up state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_alarm_confirm_box(void)//use _main_send_connection_task
{
	u16 bill_remain;

	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
		if (!(is_box_set()))
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RECOVER_WAIT, 0, 0, 0);
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_BOX);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RECOVER_WAIT)
		{
		#if defined(UBA_RTQ)
			if(ex_rc_error_flag)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STATUS, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
				break;
			}
		#endif
			if (!(is_box_set()))
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_CANCEL_TIMER, TIMER_ID_RECOVER_WAIT, 0, 0, 0);
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_BOX);
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_BOX, 0, 0, 0);
			}
			else
			{
			#if defined(UBA_RTQ)
				if((!(is_detect_rc_twin()) || !(is_detect_rc_quad())) && ex_rc_detect_next_box_open == 0)
				{
					_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_RC_UNIT);
					_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ALARM_CODE_RC_REMOVED, 0, 0);
					_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_RC_REMOVED, 0, 0, 0);
					_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);

					/* box open中のbox open検出 */
					ex_rc_detect_next_box_open = 1;
				}
				else
				{
					_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_WAIT_REQ);
					_main_display_powerup();
					bill_remain = _main_bill_remain();
					_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, bill_remain, 0, 0);
					/* Set LED and Bezel display when return to porweup */
				}
			#else
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_WAIT_REQ);
				_main_display_powerup();
				bill_remain = _main_bill_remain();
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, bill_remain, 0, 0);
			#endif // UBA_RTQ	
			}
		}
#if defined(UBA_RTQ)
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_CIS_INITIALIZE_RSP:
		break;
#if defined(UBA_RTQ)
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM && 
			ex_rc_error_flag == 0)
		{
			ex_rc_error_flag = ex_main_msg.arg2;
		}
		break;
	case TMSG_LINE_CURRENT_COUNT_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
		break;
	case TMSG_LINE_RC_ENABLE_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		break;
#endif // UBA_RTQ
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 130);
		}
		break;
	}
}
/*********************************************************************//**
 * @brief confirm box procedure (at power up state)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_mag_init(void)
{
	switch(ex_main_msg.tmsg_code)
	{
	case TMSG_CIS_INITIALIZE_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_DIPSW_MBX, TMSG_DIPSW_INIT_REQ, 0, 0, 0, 0);
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_DIPSW_INIT);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
		/* ラインタスクは完全には動作させる前なので、ホストには通信しない */
		/* 完全なハードエラーなので、他のタスクはまだ動いていない状態だが、そのままエラーにジャンプ*/
		/* 現状はMag側のエラー(例)磁気あり基板 + 磁気なし実機*/
			_main_alarm_sub(0, 0, TMSG_CONN_INITIAL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			//_main_alarm_sub(0, 0, TMSG_CONN_INITIAL, ALARM_CODE_UV, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(1, 125);
		}
		break;
	case TMSG_DLINE_TEST_FINISH_REQ:
		break;
	case TMSG_TIMER_TIMES_UP:
#if defined(UBA_RTQ)
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
#endif // UBA_RTQ
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	#if defined(UBA_RTQ)
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;
	#endif

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS)
		 && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_INITIAL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 125);
		}
		break;
	}
}


#if !defined(UBA_RTQ)//#if defined(GLI) //2023-12-04
void powerup_alarm_receive_reset_gli(void)//use _main_send_connection_task
{
	
	switch(ex_main_msg.tmsg_code)
	{
	case	TMSG_CLINE_SEARCH_BILL_REQ:
	case	TMSG_DLINE_SEARCH_BILL_REQ:
			break;
	//2024-03-18
	case 	TMSG_CLINE_RESET_REQ:
	case 	TMSG_DLINE_RESET_REQ:
			_main_set_init();
			break;

	case	TMSG_CLINE_SET_STATUS:
			if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
			}
			break;
	case	TMSG_SENSOR_ACTIVE_RSP:
	case	TMSG_SENSOR_STATUS_INFO:
			if (!(is_box_set()))
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_BOX);
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_BOX, 0, 0, 0);
				_main_send_msg(ID_SENSOR_MBX, TMSG_SENSOR_STATUS_REQ, 0, 0, 0, 0);
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ALARM_CODE_BOX, 0, 0);
			}
			break;
	#if defined(UBA_RTQ)
	case	TMSG_RC_SW_COLLECT_RSP:
			_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
			ex_rc_collect_sw = 0;
			break;
	case	TMSG_LINE_CURRENT_COUNT_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
			break;
	case TMSG_LINE_RC_ENABLE_REQ:
			_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
			break;	
	#endif

	default:
			if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
			{
				_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
			}
			else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
			{
				/* system error ? */
				_main_system_error(0, 7);
			}
			break;
	}
}
#endif // end GLI

#if defined(UBA_RTQ)
void powerup_rc_sensor_active()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_SENSOR_ACTIVE_RSP:
		if (!(is_box_set()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_SEARCH_BILL, ALARM_CODE_BOX, _main_conv_seq(), ex_position_sensor);
		}
		else if (!(is_detect_rc_twin()) || 
				!(is_detect_rc_quad()))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_SEARCH_BILL, ALARM_CODE_RC_REMOVED, _main_conv_seq(), ex_position_sensor);
		}
		else
		{
			main_search_in_drum = 0;
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_RC_SEARCH_BILL);
			_main_send_msg(ID_FEED_MBX, TMSG_FEED_SEARCH_REQ, main_search_direction, 0, 0, 0);
			_main_send_msg(ID_RC_MBX, TMSG_RC_SEARCH_REQ, main_search_unit, main_search_direction, 0, 0);
		}
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_SEARCH_BILL, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_LINE_CURRENT_COUNT_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
		break;
	case TMSG_LINE_RC_ENABLE_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		break;

	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_SEARCH_BILL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 8);
		}
		break;
	}
}

/*********************************************************************//**
 * @brief search bill procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void powerup_rc_search_bill()
{
	u16 bill_remain;

	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_SENSOR_STATUS_INFO:
		break;
	case TMSG_FEED_SEARCH_RSP:
		ex_multi_job.busy &= ~(TASK_ST_FEED);
		//	_main_send_msg(ID_RC_MBX, TMSG_RC_CANCEL_REQ, 0, 0, 0, 0);
		_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
		break;
	case TMSG_RC_SEARCH_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
		}
		/* ドラム内に紙幣ある */
		else if (ex_main_msg.arg1 == TMSG_SUB_SEARCH_STILL_BOX)
		{
			main_search_in_drum = 1;
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			_main_alarm_sub(0, 0, TMSG_CONN_RC_SEARCH_BILL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 10);
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_CHECK)
		{
			if (!(rc_busy_status()))
			{
				ex_multi_job.busy &= ~(TASK_ST_RC);
				if ((ex_multi_job.busy & TASK_ST_FEED) == 0)
				{
					if (!(is_box_set()))
					{
						_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_BOX);
						_main_send_connection_task(TMSG_CONN_RC_SEARCH_BILL, TMSG_SUB_ALARM, ALARM_CODE_BOX, 0, 0);
						_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_BOX, 0, 0, 0);
					}
					else if (!(is_detect_rc_twin()) || !(is_detect_rc_quad()))
					{
						_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_RC_UNIT);
						_main_send_connection_task(TMSG_CONN_RC_SEARCH_BILL, TMSG_SUB_ALARM, ALARM_CODE_RC_REMOVED, 0, 0);
						_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_RC_REMOVED, 0, 0, 0);
						_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
					}
					else
					{
						_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_WAIT_REQ);
						/* Set LED and Bezel display when return to porweup */
						_main_display_powerup();
						bill_remain = _main_bill_remain();
						if ((1 == main_search_in_drum) &&
							((BILL_IN_NON == bill_remain) || (BILL_IN_ENTRANCE == bill_remain)))
						{
							_main_send_connection_task(TMSG_CONN_RC_SEARCH_BILL, TMSG_SUB_SUCCESS, BILL_IN_RC, 0, 0);
						}
						else
						{
							_main_send_connection_task(TMSG_CONN_RC_SEARCH_BILL, TMSG_SUB_SUCCESS, bill_remain, 0, 0);
						}
					}
				}
				else
				{
					_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
				}
			}
			else
			{
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_CHECK, 20, 0, 0);
			}
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM )
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_SEARCH_BILL, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_LINE_CURRENT_COUNT_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
		break;
	case TMSG_LINE_RC_ENABLE_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		break;
	default:
		if (((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) == TMSG_COMMON_STATUS) && (ex_main_msg.arg1 == TMSG_SUB_ALARM))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_SEARCH_BILL, ex_main_msg.arg2, ex_main_msg.arg3, ex_main_msg.arg4);
		}
		else if ((ex_main_msg.tmsg_code & TMSG_MCODE_MASK) != TMSG_COMMON_STATUS)
		{
			/* system error ? */
			_main_system_error(0, 9);
		}
		break;
	}
}

void powerup_alarm_rc_unit()
{
	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_SENSOR_ACTIVE_RSP:
	case TMSG_SENSOR_STATUS_INFO:
		if (!(is_box_set()) && ex_rc_detect_next_box_open == 0)
		{
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_BOX);
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ALARM_CODE_BOX, 0, 0);
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_BOX, 0, 0, 0);

			/* box open中のbox open検出 */
			ex_rc_detect_next_box_open = 1;

			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM)
		{
			//UBA500と異なるが、UBA500は結果的にはここでチェックしていない事になっている
			// ex_rc_error_flag = ex_main_msg.arg2;
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);			
		}
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			if (is_detect_rc_twin() && is_detect_rc_quad())
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_CONFIRM_RC_UNIT);
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RECOVER_WAIT, WAIT_TIME_RECOVER, 0, 0);
				_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_ON, COLOR_GREEN, 0, 0);
				/* box open中のbox open検出 */
				ex_rc_detect_next_box_open = 0;
			}
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case TMSG_LINE_CURRENT_COUNT_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
		break;
	case TMSG_LINE_RC_ENABLE_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		break;
	default:
		break;
	}
}

void powerup_alarm_confirm_rc_unit()
{
	u16 bill_remain;

	switch (ex_main_msg.tmsg_code)
	{
	case TMSG_CLINE_RESET_REQ:
	case TMSG_DLINE_RESET_REQ:
		_main_set_init();
		break;
	case TMSG_CLINE_SET_STATUS:
		if ((ex_main_msg.arg1 == TMSG_SUB_ALARM) && (ex_main_msg.arg2 == ALARM_CODE_CHEAT))
		{
			_main_alarm_sub(0, 0, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_SENSOR_STATUS_INFO:
	case TMSG_SENSOR_ACTIVE_RSP:
		if (!(is_box_set()) && ex_rc_detect_next_box_open == 0)
		{
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_BOX);
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ALARM_CODE_BOX, 0, 0);
			_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_BOX, 0, 0, 0);
			/* box open中のbox open検出 */
			ex_rc_detect_next_box_open = 1;
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);

		}
		break;
	case TMSG_RC_SW_COLLECT_RSP:
		_main_send_msg(ID_RC_MBX, TMSG_RC_SW_COLLECT_REQ, 0, 0, 0, 0);
		ex_rc_collect_sw = 0;
		break;
	case TMSG_TIMER_TIMES_UP:
		if (ex_main_msg.arg1 == TIMER_ID_RECOVER_WAIT)
		{
			if (ex_rc_error_flag)
			{
				_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STATUS, ex_rc_error_flag, _main_conv_seq(), ex_position_sensor);
			}
			else if(!(is_box_set()) && ex_rc_detect_next_box_open == 0)
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_BOX);
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ALARM_CODE_BOX, 0, 0);

			}
			else if((!(is_detect_rc_twin()) || !(is_detect_rc_quad())) && ex_rc_detect_next_box_open == 0)
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_RC_UNIT);
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ALARM_CODE_RC_REMOVED, 0, 0);
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_RC_REMOVED, 0, 0, 0);
				/* display led */
				_main_send_msg(ID_RC_MBX, TMSG_RC_DISPLAY_REQ, DISP_OFF, 0, 0, 0);
				/* box open中のbox open検出 */
				ex_rc_detect_next_box_open = 1;
			}
			else
			{
				_main_send_msg(ID_RC_MBX, TMSG_RC_GET_RECYCLE_SETTING_REQ, 0, 0, 0, 0);
			}

			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		else if (ex_main_msg.arg1 == TIMER_ID_RC_STATUS)
		{
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		}
		break;
	case TMSG_RC_GET_RECYCLE_SETTING_RSP:
		if (ex_main_msg.arg1 == TMSG_SUB_SUCCESS)
		{
		#if 1
			if(!(is_box_set()) && ex_rc_detect_next_box_open == 0)
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_ALARM_BOX);
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_ALARM, ALARM_CODE_BOX, 0, 0);
				_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_BOX, 0, 0, 0);

				if(is_detect_rc_twin() && is_detect_rc_quad())
				{
					/* box open中のbox open検出 */
					ex_rc_detect_next_box_open = 0;
				}
				else
				{
					/* box open中のbox open検出 */
					ex_rc_detect_next_box_open = 1;
				}
			}
			else
			{
				_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_WAIT_REQ);
				_main_display_powerup();
				bill_remain = _main_bill_remain();
				_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, bill_remain, 0, 0);
				/* Set LED and Bezel display when return to porweup */
				_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
			}
		#else
			_main_set_mode(MODE1_POWERUP, POWERUP_MODE2_WAIT_REQ);
			_main_display_powerup();
			bill_remain = _main_bill_remain();
			_main_send_connection_task(TMSG_CONN_STATUS, TMSG_SUB_SUCCESS, bill_remain, 0, 0);
			_main_send_msg(ID_TIMER_MBX, TMSG_TIMER_SET_TIMER, TIMER_ID_RC_STATUS, 20, 0, 0);
		#endif
		}
		else
		{
			/* system error ? */
			_main_system_error(0, 11);
		}
		break;
	case TMSG_LINE_CURRENT_COUNT_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_CURRENT_COUNT_SETTING_REQ, ex_main_msg.arg1, ex_main_msg.arg2, 0, 0);
		break;
	case TMSG_RC_STATUS_INFO:
		if (ex_main_msg.arg1 == TMSG_SUB_ALARM )
		{
			_main_alarm_sub(MODE1_ALARM, ALARM_MODE2_RC_ERROR, TMSG_CONN_STATUS, ex_main_msg.arg2, _main_conv_seq(), ex_position_sensor);
		}
		break;
	case TMSG_LINE_RC_ENABLE_REQ:
		_main_send_msg(ID_RC_MBX, TMSG_RC_RECYCLE_ENABLE_REQ, ex_main_msg.arg1, 0, 0, 0);
		break;
	default:
		break;
	}
}
#endif // UBA_RTQ



/* EOF */

