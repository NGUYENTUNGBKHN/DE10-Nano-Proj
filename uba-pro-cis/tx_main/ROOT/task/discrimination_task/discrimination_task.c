/******************************************************************************/
/*! @addtogroup Main
    @file       discrimination_task.c
    @brief      calculate task
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

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "custom.h"
#include "operation.h"
#include "sub_functions.h"
#include "sensor.h"
#include "sensor_ad.h"
#include "pl/pl_cis.h"
#include "hal_clk.h"
#include "alt_watchdog.h" //2024-12-03

#define EXT
#include "../common/global.h"
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

#include "jdl_conf.h"

/************************** Function Prototypes ******************************/
void discrimination_task(VP_INT exinf);
/************************** External functions *******************************/

/************************** Variable declaration *****************************/

/************************** PRIVATE DEFINITIONS *************************/
enum _DISCRIMINATION_MODE
{
	DISCRIMINATION_MODE_IDLE = 0,
	DISCRIMINATION_MODE_COMPARE,
	DISCRIMINATION_MODE_REV_COMPARE,
	DISCRIMINATION_MODE_SIGNATURE,
	DISCRIMINATION_MODE_GENERATION,
	DISCRIMINATION_MODE_CIS_INITIALIZE,
	DISCRIMINATION_MODE_IMAGE_INITIALIZE,
};

/************************** PRIVATE VARIABLES *************************/
T_MSG_BASIC discrimination_msg;


/************************** PRIVATE FUNCTIONS *************************/
void _discrimination_initialize_proc(void);
void _discrimination_msg_proc(void);
void _discrimination_set_mode(u16 mode);
void _discrimination_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _discrimination_system_error(u8 fatal_err, u8 code);

static u16 calc_crc(UB *data, UW start_adr, UW end_adr, UH seed);
#if defined(_PROTOCOL_ENABLE_ID0G8)
static u32 _calc_crc32(u8 *data, u32 start_adr, u32 end_adr, u32 value);
#endif

/************************** EXTERN FUNCTIONS *************************/
extern int skew_check(u8 buf_num);
extern int bar_skew_check(u8 buf_num);
extern int compare(void);
extern int block_compare(void);
extern int reverse_compare(void);
extern int is_tape_detected(void);
extern void set_reject_code(s16 result);
UB bar_check(void);
bool is_barcode_ticket_disabled(void);
bool is_banknote_disabled(void);
extern s16 edge_detect(u8 buf_num);
#if BANKNOTE_CYCLIC_ENABLE
extern void cyclic_data_fix(u8 buf_num);
#endif


/*******************************
        discrimination_task
 *******************************/
void discrimination_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	_discrimination_initialize_proc();
#if (_DEBUG_VARIABLE_CLOCK==1)
	change_mpu_clock(MPU_CLOCK_LOW);
#endif

	while(1)
	{
		ercd = trcv_mbx(ID_DISCRIMINATION_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME);
		if (ercd == E_OK)
		{
			memcpy(&discrimination_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(discrimination_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_discrimination_system_error(1, 3);
			}
#if (_DEBUG_VARIABLE_CLOCK==1)
			change_mpu_clock(MPU_CLOCK_HIGH);
			_discrimination_msg_proc();
			change_mpu_clock(MPU_CLOCK_LOW);
#else
			_discrimination_msg_proc();
#endif
		}
		//_discrimination_idel_proc();
	}
}


void _discrimination_initialize_proc(void)
{
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbs = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS *pbs = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif
	tx_thread_vfp_enable();
	_discrimination_set_mode(DISCRIMINATION_MODE_IDLE);
	parameter_set(pbs);
}


#if (_DEBUG_CIS_MULTI_IMAGE==1)
static void _cis_image_initialize(void)
{
	u32 *p_Data;
	u32 *end;
	u32 index = (ex_cis_image_control.current + 1)%BILL_NOTE_IMAGE_MAX_COUNT;
	ST_BS *pbs = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[index];

	if(	ex_cis_image_control.image_status[index] != CIS_IMAGE_INITIALIZED)
	{
		p_Data = (u32 *)&pbs->proc_num;
		end = (u32 *)(&pbs->sens_dt[0] - 1);
		while(p_Data < end)
		{
			*p_Data = (u32)0;
			p_Data ++;
		}
		p_Data = (u32 *)&pbs->sens_dt[0];
		end = (u32 *)&pbs->sens_dt[CAP_SNSDAT_SIZE-1];
		while(p_Data < end)
		{
			*p_Data = (u32)0xFFFFFFFF;
			p_Data ++;
		}
		ex_cis_image_control.image_status[index] = CIS_IMAGE_INITIALIZED;
		parameter_set(pbs);
	}
}
#endif
void _discrimination_msg_proc(void)
{
	s16 result = 0;

#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbs = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS *pbs = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif

	switch (discrimination_msg.tmsg_code)
	{
	case TMSG_DATA_COLLECTION_REQ:
		_discrimination_set_mode(DISCRIMINATION_MODE_COMPARE);
#if BANKNOTE_CYCLIC_ENABLE
		//サイクリックデータずれ修正処理
		cyclic_data_fix(0);
#endif
		result = edge_detect(0);
		if(result)
		{
			ex_collection_data.data_result = DATA_EDGE_ERROR;
			ex_validation.reject_code = REJECT_CODE_SKEW;
		}
#if (_DEBUG_CIS_MULTI_IMAGE==1)
		ex_cis_image_control.last = ex_cis_image_control.current;
#endif

		_discrimination_set_mode(DISCRIMINATION_MODE_IDLE);
		ex_val_watch.end = OSW_TIM_value() - ex_val_watch.start;
		if(ex_validation.reject_code)
		{
			_discrimination_send_msg(ID_MAIN_MBX, TMSG_VALIDATION_RSP, TMSG_SUB_REJECT, ex_validation.reject_code, 0, 0);
		}
		else
		{
			ex_collection_data.data_exist = DATA_EXIST;
			_discrimination_send_msg(ID_MAIN_MBX, TMSG_VALIDATION_RSP, TMSG_SUB_SUCCESS, 0,0,0);//ex_validation.denomi, ex_validation.direction, 0);
		}
		break;
	case TMSG_VALIDATION_REQ:
		ex_val_watch.start = OSW_TIM_value();
		_discrimination_set_mode(DISCRIMINATION_MODE_COMPARE);
		ex_validation.reject_code = REJECT_CODE_OK;
		memset(&ex_validation.denomi, 0, sizeof(BV_MEMORY)-8);
#if BANKNOTE_CYCLIC_ENABLE
		//サイクリックデータずれ修正処理
		cyclic_data_fix(0);
#endif
		// stop timer 0
		//TIM_Cmd(LPC_TIMER0,DISABLE);
#if (_DEBUG_ALL_ACCEPT==1)
		/* 紙幣識別処理 */
		ex_validation.start = VALIDATION_STRT;

		//テンプレート処理開始
		result = compare();
		ex_validation.bill_length = (u8)(ex_validation.bill_length * PITCH / 2 + 0.5); //2024-12-24 共通識別内は触れないでの、ここで変換
		ex_validation.denomi = 0;
		ex_validation.direction = 0;
		ex_validation.reject_code = REJECT_CODE_OK;
		//set_reject_code();//reject_code挿入
#else
		ex_validation.start = VALIDATION_STRT;

		//テンプレート処理開始
		result = compare();
		ex_validation.bill_length = (u8)(ex_validation.bill_length * PITCH / 2 + 0.5); //2024-12-24 共通識別内は触れないでの、ここで変換
	#if defined(UBA_RTQ)
		//RTQの無鑑別は、リサイクル先を札長で決める為、識別を動かす必要がある。
		//札長以外いらないので抜ける
		if(is_test_mode()) //2025-05-13a 
		{
			if( ex_dipsw1 == DIPSW1_ACCEPT_ALLACC_TEST) //ok	
			{
			//必要なのは札長なので、成功で返す
			//札長が上手く取れてなくても、回収庫にいくだけ
			//2025-01-20 	_discrimination_send_msg(ID_MAIN_MBX, TMSG_VALIDATION_RSP, TMSG_SUB_SUCCESS, ex_validation.denomi, ex_validation.direction, 0);
				_discrimination_send_msg(ID_MAIN_MBX, TMSG_VALIDATION_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);

				_discrimination_set_mode(DISCRIMINATION_MODE_IDLE);
				return;
			}
		}
	#endif
	
	#if 1 //2025-02-17
		if(result == 0)
		{
			result = skew_check(0);
			if(result == 0)
			{
				//add validation function
				result = block_compare();
			}
		}
	#else
		if(ex_validation.bill_length > 125-5 && ex_validation.bill_length < 125+5)
		{
			//GBR5.6 125mm
			ex_validation.denomi = 0;
			ex_validation.direction = 0;
			result = 0;
		}
		else if(ex_validation.bill_length > 152-5 && ex_validation.bill_length < 152+5)
		{
			ex_validation.denomi = 1;
			ex_validation.direction = 0;
			result = 0;
		}
	#endif
		if(result)//エラー
		{
			if(bar_skew_check(0) == 0)
			{
				/* バーコード識別処理 */
				bar_check();
				if(!is_banknote_disabled())
				{
					/* 紙幣の返却コードを優先 */
					/* チケット判定に成功している場合の返却のみチケットのコードを優先 */
					if(ex_bar_reject_code == 0)
					{
						// TICKET ACCEPT
						ex_validation.reject_code = REJECT_CODE_OK;
						ex_validation.denomi = BAR_INDX;
					}
				#if (DEBUG_1D_TICKET_RETURN_EXCEPT_SPECIFIC_NUMBER==1)
					else if(ex_bar_reject_code != 0)
				#elif (DEBUG_2D_TICKET_RETURN_EXCEPT_SPECIFIC_NUMBER==1)
					else if(ex_bar_reject_code != 0)
				#else
					else if((ex_bar_reject_code == REJECT_CODE_BAR_DIN)
						|| (ex_bar_reject_code == REJECT_CODE_BAR_SH)
						|| (ex_bar_reject_code == REJECT_CODE_INHIBIT)
						|| (ex_bar_reject_code == REJECT_CODE_BAR_PHV))
				#endif
					{
						// TICKET REJECT
						ex_validation.reject_code = ex_bar_reject_code;
						ex_validation.denomi = BAR_INDX;
					}
				}
				else if(!is_barcode_ticket_disabled())
				{
					/* 紙幣受取禁止、チケットが受取の場合のみ、チケットの返却コードを優先 */
					if(ex_bar_reject_code == 0)
					{
						// TICKET ACCEPT
						ex_validation.reject_code = REJECT_CODE_OK;
						ex_validation.denomi = BAR_INDX;
					}
					else
					{
						// TICKET REJECT
						ex_validation.reject_code = ex_bar_reject_code;
						ex_validation.denomi = BAR_INDX;
					}
				}
				else
				{
					/* 紙幣、チケット受取禁止（DISABLE STATUS) */
				}
			}
		}

		if(ex_validation.denomi != BAR_INDX)
		{
			set_reject_code(result);//reject_code挿入
		}
		else
		{
			if(is_tape_detected())
			{
				/* 2017-06-12 adp RBA-40 possible manipulation */
				/* reject banknote if tape detected */
				ex_validation.reject_code = REJECT_CODE_LENGTH;
			}
		}
#endif
		pbs->result_e_code = ex_validation.reject_code;
		ex_monitor_info.data_exist = true;
#if (_DEBUG_CIS_MULTI_IMAGE==1)
		ex_cis_image_control.last = ex_cis_image_control.current;
#endif
	#if 1 //2024-05-28
		#if defined(QA_TEST_AZ) || defined(QA_TEST_EMC_EMI)
		//動作インターバル優先なので、無視
		
		#else
			is_temperature_warn();
		#endif
	#else	
		if(is_temperature_warn() == TMP_WARN_HIGH) //2024-05-28
		{
			/* 20231023 温度上昇対策 5000ms 待ち*/
			/* CIS OFFはMGU READ完了時に行われるので待ち時間はDiscrimination内で行っても問題ない */
			OSW_TSK_sleep(MOT_TEMP_WARN_ESCROW_INTERVAL_HIGH);
		}
		else if(is_temperature_warn() == TMP_WARN_LOW)
		{
			/* 20231023 温度上昇対策 2000ms待ち*/
			/* CIS OFFはMGU READ完了時に行われるので待ち時間はDiscrimination内で行っても問題ない */
			OSW_TSK_sleep(MOT_TEMP_WARN_ESCROW_INTERVAL_LOW);
		}
	#endif

		_discrimination_set_mode(DISCRIMINATION_MODE_IDLE);
		ex_val_watch.end = OSW_TIM_value() - ex_val_watch.start;
		if(ex_validation.reject_code)
		{
			_discrimination_send_msg(ID_MAIN_MBX, TMSG_VALIDATION_RSP, TMSG_SUB_REJECT, ex_validation.reject_code, 0, 0);
		}
		else
		{
			_discrimination_send_msg(ID_MAIN_MBX, TMSG_VALIDATION_RSP, TMSG_SUB_SUCCESS, ex_validation.denomi, ex_validation.direction, 0);
		}
		break;
	case TMSG_VALIDATION_REVERSE_REQ:
		_discrimination_set_mode(DISCRIMINATION_MODE_REV_COMPARE);
		// validation memory clear
		memset(&ex_validation, 0, sizeof(BV_MEMORY));

		ex_validation.reject_code = REJECT_CODE_OK;

#if (_DEBUG_ALL_ACCEPT==1)
		/* 紙幣識別処理 */
		ex_validation.denomi = 0;
		ex_validation.direction = 0;
		ex_validation.reject_code = REJECT_CODE_OK;
		//set_reject_code();//reject_code挿入
#else
		/* 逆鑑別処理 */
		reverse_compare();
#endif
#if (_DEBUG_REV_COMP_LENGTH==1)
		ex_monitor_info.data_exist = true;
#endif
		_discrimination_set_mode(DISCRIMINATION_MODE_IDLE);
		if(ex_validation.reject_code)
		{
			_discrimination_send_msg(ID_MAIN_MBX, TMSG_VALIDATION_REVERSE_RSP, TMSG_SUB_REJECT, ex_validation.reject_code, 0, 0);
		}
		else
		{
			_discrimination_send_msg(ID_MAIN_MBX, TMSG_VALIDATION_REVERSE_RSP, TMSG_SUB_SUCCESS, ex_validation.denomi, 0, 0);
		}
		break;
	case TMSG_SIGNATURE_REQ:
#if defined(_PROTOCOL_ENABLE_ID003)
		if(discrimination_msg.arg1 == SIGNATURE_CRC16)
		{
			_discrimination_set_mode(DISCRIMINATION_MODE_SIGNATURE);

			alt_wdog_stop(ALT_WDOG1); //2024-12-03

            ex_rom_crc16 = calc_crc( (u8 *)ROM_ALIAS_START_ADDRESS,
                                     ROM_ALIAS_START_ADDRESS,
                                     ROM_ALIAS_END_ADDRESS,
									 ex_crc16_seed);

			alt_wdog_start(ALT_WDOG1); //2024-12-03

			_discrimination_set_mode(DISCRIMINATION_MODE_IDLE);
			_discrimination_send_msg(ID_CLINE_MBX, TMSG_SIGNATURE_RSP, TMSG_SUB_SUCCESS, SIGNATURE_CRC16, 0, 0);
		}
		else if(discrimination_msg.arg1 == SIGNATURE_SHA1)
		{
			_discrimination_set_mode(DISCRIMINATION_MODE_SIGNATURE);

			alt_wdog_stop(ALT_WDOG1); //2024-12-03
			calc_sha1_reset((u8 *)ROM_ALIAS_START_ADDRESS,
							 (u8 *)ROM_ALIAS_END_ADDRESS + 1,
							 (u8 *)&ex_sha1_seed[0],
							 (u8 *)&ex_rom_sha1[0]);
			while(calc_sha1_func()==false);
			alt_wdog_start(ALT_WDOG1); //2024-12-03

			_discrimination_set_mode(DISCRIMINATION_MODE_IDLE);
			_discrimination_send_msg(ID_CLINE_MBX, TMSG_SIGNATURE_RSP, TMSG_SUB_SUCCESS, SIGNATURE_SHA1, 0, 0);
		}
		break;

#elif defined(_PROTOCOL_ENABLE_ID0G8)
		if(discrimination_msg.arg1 == SIGNATURE_CRC16)
		{
            ex_rom_crc16 = calc_crc( (u8 *)ROM_ALIAS_START_ADDRESS,
                                     ROM_ALIAS_START_ADDRESS,
                                     ROM_ALIAS_END_ADDRESS,
                                     ex_seed_crc16);
			_discrimination_set_mode(DISCRIMINATION_MODE_IDLE);
			_discrimination_send_msg(ID_CLINE_MBX, TMSG_SIGNATURE_RSP, TMSG_SUB_SUCCESS, SIGNATURE_CRC16, 0, 0);
		}
		else if(discrimination_msg.arg1 == SIGNATURE_CRC32)
		{
            ex_rom_crc32 = _calc_crc32( (u8 *)ROM_ALIAS_START_ADDRESS,
                                        ROM_ALIAS_START_ADDRESS,
                                     	ROM_ALIAS_END_ADDRESS,
                                        ex_seed_crc32);

			_discrimination_set_mode(DISCRIMINATION_MODE_SIGNATURE);
			_discrimination_set_mode(DISCRIMINATION_MODE_IDLE);
			_discrimination_send_msg(ID_CLINE_MBX, TMSG_SIGNATURE_RSP, TMSG_SUB_SUCCESS, SIGNATURE_CRC32, 0, 0);
		}
	
	

		break;

#else
		/* system error */
		_discrimination_system_error(1, 5);
		break;
#endif
	case TMSG_CIS_INITIALIZE_REQ:
		_discrimination_set_mode(DISCRIMINATION_MODE_CIS_INITIALIZE);
		switch(discrimination_msg.arg1)
		{
		case AD_MODE_PAY_OUT:
    		//OSW_TSK_sleep(50); /* 20-08-26 wait CIS stability time after FPGA ON */
			change_ad_sampling_mode(AD_MODE_PAY_OUT);
			if(!is_cisa_lvds_lock())
			{
#if (HAL_STATUS_ENABLE==1)
				ex_hal_status.cisa = HAL_STATUS_NG;
#endif
				_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_ALARM, ALARM_CODE_CISA_OFF, 0, 0);
			}
			else if(!is_cisb_lvds_lock())
			{
#if (HAL_STATUS_ENABLE==1)
				ex_hal_status.cisb = HAL_STATUS_NG;
#endif
				_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_ALARM, ALARM_CODE_CISB_OFF, 0, 0);
			}
			else
			{
#if (HAL_STATUS_ENABLE==1)
				ex_hal_status.cisa = HAL_STATUS_OK;
				ex_hal_status.cisb = HAL_STATUS_OK;
#endif
				_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			}
			break;
		case AD_MODE_BILL_IN:

			for(int retry = 0; retry < 5; retry++)
			{
				change_ad_sampling_mode(AD_MODE_BILL_IN);
				// Clear main validation result memory using LVDS lock time
				memset(&ex_validation, 0, sizeof(ex_validation));
				if(!is_cisa_lvds_lock())
				{
					ex_hal_status.cisa = HAL_STATUS_NG;
				}
				else
				{
					ex_hal_status.cisa = HAL_STATUS_OK;
				}
				if(!is_cisb_lvds_lock())
				{
					ex_hal_status.cisb = HAL_STATUS_NG;
				}
				else
				{
					ex_hal_status.cisb = HAL_STATUS_OK;
				}
				if((ex_hal_status.cisa == HAL_STATUS_OK)&&(ex_hal_status.cisb == HAL_STATUS_OK))
				{
					break;
				}
				_pl_cis_enable_set(0);
				OSW_TSK_sleep(100);
				_pl_cis_enable_set(1);
				OSW_TSK_sleep(100);
			}
			if(ex_hal_status.cisa == HAL_STATUS_NG)
			{
				_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_ALARM, ALARM_CODE_CISA_OFF, 0, 0);
			}
			else if(ex_hal_status.cisb == HAL_STATUS_NG)
			{
				_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_ALARM, ALARM_CODE_CISB_OFF, 0, 0);
			}
			else
			{
				_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			}
			break;
		case AD_MODE_ONE_SHOT:
			ex_cis_adjustment_tbl.weit = 1;
			ex_cis_adjustment_tbl.sequence = ADJUSTMENT_PL_INIT;

			while(ex_cis_adjustment_tbl.sequence < ADJUSTMENT_SEQUENCE_END)
			{
				_pl_cis_oneshot_sequence(discrimination_msg.arg2);
				OSW_TSK_sleep(ex_cis_adjustment_tbl.weit);
			}
			ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_NONE;
			break;
#if (_DEBUG_EMI_IMAGE_CHECK==1)
		case AD_MODE_AGING:
			for(int retry = 0; retry < 5; retry++)
			{
				change_ad_sampling_mode(AD_MODE_AGING);
				// Clear main validation result memory using LVDS lock time
				memset(&ex_validation, 0, sizeof(ex_validation));
				if(!is_cisa_lvds_lock())
				{
					ex_hal_status.cisa = HAL_STATUS_NG;
				}
				else
				{
					ex_hal_status.cisa = HAL_STATUS_OK;
				}
				if(!is_cisb_lvds_lock())
				{
					ex_hal_status.cisb = HAL_STATUS_NG;
				}
				else
				{
					ex_hal_status.cisb = HAL_STATUS_OK;
				}
				if((ex_hal_status.cisa == HAL_STATUS_OK)&&(ex_hal_status.cisb == HAL_STATUS_OK))
				{
					break;
				}
				_pl_cis_enable_set(0);
				OSW_TSK_sleep(100);
				_pl_cis_enable_set(1);
				OSW_TSK_sleep(100);
			}
			if(ex_hal_status.cisa == HAL_STATUS_NG)
			{
				_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_ALARM, ALARM_CODE_CISA_OFF, 0, 0);
			}
			else if(ex_hal_status.cisb == HAL_STATUS_NG)
			{
				_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_ALARM, ALARM_CODE_CISB_OFF, 0, 0);
			}
			else
			{
				_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			}
			break;
#endif
	
	#if MAG1_ENABLE
		case AD_MODE_MAG_ADJUSTMENT:
			// UBA700,710確認
			
			if( set_mag_gain() )
			{
			//Error = UBA700
				ex_uba710 = 0;
				_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
			}
			else
			{
				ex_uba710 = 1;
				
				ex_cis_adjustment_tbl.weit = 1;
				ex_cis_adjustment_tbl.sequence = ADJUSTMENT_PL_INIT;

				while(ex_cis_adjustment_tbl.sequence < ADJUSTMENT_SEQUENCE_END)
				{
					_pl_cis_oneshot_sequence(discrimination_msg.arg2);
					OSW_TSK_sleep(ex_cis_adjustment_tbl.weit);
				}
				//
				if(0)//if(is_uv_led_check_uba())	//2023-01-13
				{
					_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_ALARM, 0, 0, 0);
				}
				else
				{
					ex_cis_adjustment_tbl.weit = 1;
					ex_cis_adjustment_tbl.sequence = ADJUSTMENT_PL_INIT;

					while(ex_cis_adjustment_tbl.sequence < ADJUSTMENT_SEQUENCE_END)
					{
						_pl_mag_adjustment_sequence();
						OSW_TSK_sleep(ex_cis_adjustment_tbl.weit);
					}

					ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_NONE;
					if(discrimination_msg.arg2 == MAG_INIT_MODE_POWERUP)
					{
						if( (ex_mag_adj.ul_gain == MAG_GAIN_MAX && ex_mag_adj.ul_adj_max <= MAG_AD_ERROR )
						||  (ex_mag_adj.ur_gain == MAG_GAIN_MAX && ex_mag_adj.ur_adj_max <= MAG_AD_ERROR )
						)
						{
						/* 磁気あり基板 + 磁気なし機械の場合、ソフト的には磁気ありソフトとして動作するので */
						/* センサ調整Toolを使用する場合にはエラーになる*/
						/* 実動作だと磁気を識別に使用して受け取り不良になる*/
						/* これらを対策する為、磁気なし機械に磁気あり基板をのせた場合はエラーとする*/
						/* ソフト自動判別なので、逆の組合せ 磁気なし基板 + 磁気あり機械の場合は対策不可能で*/
						/* この場合は、磁気なしソフトとして動作する、エラーにはならない */
							_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_ALARM, ALARM_CODE_MAG, 0, 0);

						}
						else
						{
							_discrimination_send_msg(ID_MAIN_MBX, TMSG_CIS_INITIALIZE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
						}
					}
				}
			}

			break;
	#endif
	
		default:
			/* system error */
			_discrimination_system_error(1, 7);
			break;
		}
		_discrimination_set_mode(DISCRIMINATION_MODE_IDLE);
		break;
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	case TMSG_IMAGE_INITIALIZE_REQ:
		_discrimination_set_mode(DISCRIMINATION_MODE_IMAGE_INITIALIZE);
		_cis_image_initialize();
		_discrimination_set_mode(DISCRIMINATION_MODE_IDLE);
		break;
#endif

	default:					/* other */
		/* system error ? */
		_discrimination_system_error(0, 6);
		break;
	}
}



/*********************************************************************//**
 * @brief set task mode
 * @param[in]	mode : task mode
 * @return 		None
 **********************************************************************/
void _discrimination_set_mode(u16 mode)
{
	ex_discrimination_task_mode = mode;

#ifdef _ENABLE_JDL
    jdl_add_trace(ID_DISCRIMINATION_TASK, ((ex_discrimination_task_mode >> 8) & 0xFF), (ex_discrimination_task_mode & 0xFF), ex_validation.denomi, ex_validation.direction, ex_validation.reject_code);
#endif /* _ENABLE_JDL */
}


void _discrimination_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_DISCRIMINATION_TASK;
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
			_discrimination_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_discrimination_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _discrimination_system_error(u8 fatal_err, u8 code)
{
#ifdef _DEBUG_SYSTEM_ERROR
	//_discrimination_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_discrimination_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_DISCRIMINATION_TASK, (u16)code, (u16)discrimination_msg.tmsg_code, (u16)discrimination_msg.arg1, fatal_err);
}


/*------------------------------------------------------*/
/*=== CRC計算ﾙｰﾁﾝ										*/
/*-- <引数>	なし										*/
/*------------------------------------------------------*/
static const UH fcstab[256] = {
   0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
   0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
   0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
   0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
   0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
   0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
   0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
   0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
   0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
   0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
   0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
   0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
   0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
   0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
   0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
   0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
   0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
   0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
   0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
   0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
   0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
   0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
   0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
   0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
   0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
   0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
   0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
   0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
   0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
   0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
   0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
   0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};
/*-OnFROM-----------------------------------------------*/
/*=== CRC計算ﾙｰﾁﾝ										*/
/*--- *data		:	CRC計算する対象のﾃﾞｰﾀへの先頭ｱﾄﾞﾚｽ	*/
/*--- start_adr	:	CRC計算する対象のｱﾄﾞﾚｽ(開始ｱﾄﾞﾚｽ)	*/
/*---				( *dataを先頭としたoffset)			*/
/*--- end_adr	:	CRC計算する対象のｱﾄﾞﾚｽ(終了ｱﾄﾞﾚｽ)	*/
/*--- seed		:	CRC計算初期値(seed)					*/
/*---			    (初めて計算するときは0を指定)		*/
/*-- <引数>	なし										*/
/*------------------------------------------------------*/
u16 calc_crc(UB *data, UW start_adr, UW end_adr, UH seed)
{
    UW	len;
    UB	vdata;
    UH vcrc;

    vcrc = seed;

    if( start_adr <= end_adr )
    {
        len = end_adr - start_adr + 1;
    }
    else
    {
        return(0);		/* ｴﾗｰ */
    }

#if !defined(ICE)
//    WDTEnable_WDT1( FALSE );
#endif
    while (len--)
    {
        vdata = *data++;
        vcrc = (vcrc >> 8) ^ fcstab[(vcrc ^ vdata) & 0xff];
    }
#if !defined(ICE)
 //   WDTEnable_WDT1( TRUE );
#endif
    return(vcrc);
}


#if defined(_PROTOCOL_ENABLE_ID0G8)
const unsigned long int ex_crctbl_32[256] = 
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
/*--------------------------------------------------------------
* CRC32 calculation												
*																
*---------------------------------------------------------------
*	引数	:u32 start_address	(CRC計算する対象の終了ｱﾄﾞﾚｽ)	
*			 u32 end_address	(CRC計算する対象の先頭ｱﾄﾞﾚｽ)	
*			 int value			(初期値)						
*---------------------------------------------------------------
*	戻値	:value												
*--------------------------------------------------------------*/
u32 _calc_crc32(u8 *data, u32 start_adr, u32 end_adr, u32 value)
{
	u32	len;
	u32	vdata;

	if(start_adr <= end_adr)
	{
		len = (end_adr - start_adr) + 1;
	    do
	    {
	   	    vdata = *data++;
        	value = (value >> 8) ^ ex_crctbl_32[(u8)((value & 0xff) ^ vdata)];
			len -= 1;
    	}while(len != 0);
    }
    return(value);
}
#endif

