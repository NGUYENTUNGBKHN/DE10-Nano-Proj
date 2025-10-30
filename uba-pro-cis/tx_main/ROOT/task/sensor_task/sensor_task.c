/******************************************************************************/
/*! @addtogroup Main
    @file       sensor_task.c
    @brief      sensor task process
    @file       sensor_task.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2012/09/20 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
// "システムタイマー内からのイベント通知を監視する。センサー状態の変化をメインタスクに通知する。センサーの点灯、消灯を制御する。"

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "hal.h"
#include "hal_bezel.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"
#include "cis_ram.c"
#include "jsl_ram.c"

#include "jdl_conf.h"

#include "sensor.h"
#include "sensor_ad.h"
#include "cyc.h"

/************************** Function Prototypes ******************************/
void sensor_task(VP_INT exinf);
void _sensor_temp_adj_resume_posi(void);
/************************** External functions *******************************/
#if !defined(DEBUG_VALIDATION_DISABLE)//yuji 2019-08-29
extern void _pl_cis_oneshot_sequence(u16 cismode);
extern u8 _cis_temp_adjust_pga(void);
#endif

/************************** Variable declaration *****************************/

/************************** PRIVATE DEFINITIONS *************************/
enum _SENSOR_MODE
{
	SENSOR_MODE_INIT = 1,
	SENSOR_MODE_ACTIVE,
	SENSOR_MODE_STANDBY,
	SENSOR_MODE_POSI_AD,
	SENSOR_MODE_TEMP_ADJ,
	SENSOR_MODE_TEMP_ADJ_POS_DETECT,
	SENSOR_MODE_TEMP_ADJ_CIS,
	SENSOR_MODE_TEMP_ADJ_FINAL,
};

enum _SENSOR_STATE
{
	SENSOR_ST_UNKNOWN = 0,
	SENSOR_ST_ON = 0x55,
	SENSOR_ST_OFF = 0xAA,
};

/************************** PRIVATE VARIABLES *************************/
static T_MSG_BASIC sensor_msg;

static u8 s_sens_temp_result;
static u16 s_sens_temp_cnt;
static u16 s_sens_temp_pos_check;
static u16 s_sens_temp_pos_detect;
static u16 s_sens_temp_pos_value;


#if !defined(DEBUG_VALIDATION_DISABLE)//yuji 2019-08-29
/* CIS Sensor */
typedef struct _CIS_SENS_TEMP_ADJ_INFO
{
	u8 end_flag;
	u8 count;
	float backup_pga;
} CIS_SENS_TEMP_ADJ_INFO;
static CIS_SENS_TEMP_ADJ_INFO s_sens_tempadj_cis[20];
#endif

#if !defined(DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE)
/* Position Sensor Temperature Adjustment */
typedef struct _SENS_TEMP_ADJ_INFO
{
	u8 end_flag;
	u8 reverse_flag;
	u16 count;
	u8 backup_da;
	u8 backup_ga;
	u8 last_st;
} SENS_TEMP_ADJ_INFO;

typedef struct _SENS_TEMP_ADJ_REF
{
	u8 da1;
	u8 ga1;
} SENS_TEMP_ADJ_REF;

/* Entrance Sensor */
static SENS_TEMP_ADJ_INFO s_sens_tempadj_entr;
static SENS_TEMP_ADJ_REF s_sens_entr_ref;

/* Centering Sensor */
static SENS_TEMP_ADJ_INFO s_sens_tempadj_cent;
static SENS_TEMP_ADJ_REF s_sens_cent_ref;

/* APB-IN Sensor */
static SENS_TEMP_ADJ_INFO s_sens_tempadj_apbi;
static SENS_TEMP_ADJ_REF s_sens_apbi_ref;

/* APB-OUT Sensor */
static SENS_TEMP_ADJ_INFO s_sens_tempadj_apbo;
static SENS_TEMP_ADJ_REF s_sens_apbo_ref;

/* Exit Sensor */
static SENS_TEMP_ADJ_INFO s_sens_tempadj_exit;
static SENS_TEMP_ADJ_REF s_sens_exit_ref;
#endif /* defined(DEBUG_POS_SENSOR_ADJUSTMENT) */

/************************** PRIVATE FUNCTIONS *************************/
void _sensor_initialize_proc(void);
void _sensor_msg_proc(void);

void _sensor_temp_adj_init_vali(void);
void _sensor_temp_adj_init_posi(void);

void _sensor_temp_adj_restore_vali(void);
void _sensor_temp_adj_restore_posi(void);

void _sensor_temp_adj_success_vali(void);
void _sensor_temp_adj_success_posi(void);

void _sensor_temp_adj_over_vali(void);
void _sensor_temp_adj_over_posi(void);

u16 _sensor_temp_adj_vali(void);
u16 _sensor_temp_adj_posi(void);

bool _is_sensor_position_all_off(void);


void _sensor_set_mode(u16 mode);
void _sensor_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _sensor_system_error(u8 fatal_err, u8 code);

#if defined(UBA_RTQ)
u8 check_sensor_level();
#endif

/************************** EXTERN FUNCTIONS *************************/

/*******************************
        sensor_task
 *******************************/
void sensor_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;
	FLGPTN current_flag = 0;
	u16 end_cnt;

	/* Task Initialize */
	_sensor_initialize_proc();

	while (1)
	{
		ercd = twai_flg(ID_SENSOR_FLAG, EVT_ALL_BIT, TWF_ORW, &current_flag, TASK_WAIT_TIME_SENSOR);
		if (ercd == E_OK)
		{
			clr_flg(ID_SENSOR_FLAG, ~EVT_ALL_BIT);
			switch (ex_sensor_task_mode)
			{
			case SENSOR_MODE_ACTIVE:
				if (current_flag & EVT_SENSOR_SHIFT)
				{
					_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_STATUS_INFO, TMSG_SUB_SUCCESS, ex_position_sensor, 0, 0);
				}
				break;
			case SENSOR_MODE_STANDBY:
				if (current_flag & EVT_SENSOR_SHIFT)
				{
					_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_STATUS_INFO, TMSG_SUB_SUCCESS, ex_position_sensor, 0, 0);
				}
				break;
			case SENSOR_MODE_TEMP_ADJ:
				// ポジションセンサー補正中
				// ポジションセンサーチェックしない
				if (current_flag & EVT_SENSOR_POSITION_AD)
				{

					if (s_sens_temp_cnt != 0)
					{
						s_sens_temp_cnt--;
					}
					end_cnt = _sensor_temp_adj_posi();	/* 調整結果の確認 */
					if (s_sens_temp_cnt == 0)
					{
						/*指定回数オーバー */
						_sensor_temp_adj_over_posi();
						_sensor_set_mode(SENSOR_MODE_TEMP_ADJ_FINAL);
					}
					else if (end_cnt >= POSITION_SENSOR_NUM)
					{ /* POS調整完 */
						_sensor_temp_adj_success_posi();	/* 調整成功 */
						_sensor_set_mode(SENSOR_MODE_TEMP_ADJ_CIS);
					}
					else
					{
						_sensor_temp_adj_restore_posi();	/* 通常監視用のDA,Gainに戻す */
						_sensor_set_mode(SENSOR_MODE_TEMP_ADJ_POS_DETECT);
					}
					_sensor_ctrl_config(SENSOR_SEQ_POSI_ADJ_DETECT, 0, SENSOR_POSITION_RISE_TIME);

				}
				break;
			case SENSOR_MODE_TEMP_ADJ_POS_DETECT:
				// ポジションセンサー紙幣検知中
				// ポジションセンサーチェックする
				if (current_flag & EVT_SENSOR_POSITION_AD)
				{
				
					s_sens_temp_pos_check++;
					if (!_is_sensor_position_all_off())
					{
						// some sensor(s) on
						s_sens_temp_pos_detect++;
					}
					if((s_sens_temp_pos_detect >= 2) && (s_sens_temp_pos_value & ex_position_sensor))
					{
						_sensor_temp_adj_restore_vali();
						_sensor_temp_adj_restore_posi();	/* 通常監視用のDA,Gainに戻す */
						s_sens_temp_result = TEMP_ADJ_SENSOR_SHIFT;
						_sensor_ctrl_config(SENSOR_SEQ_LED_BLINK, SENSOR_STANDBY_OFF_INTERVAL, SENSOR_STANDBY_ON_INTERVAL);
						_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_TEMP_ADJ_RSP, s_sens_temp_result, ex_position_sensor, 0, 0);
						_sensor_set_mode(SENSOR_MODE_STANDBY);
					}
					else
					{
						if (s_sens_temp_cnt != 0)
						{
							s_sens_temp_cnt--;
						}
						if(s_sens_temp_pos_detect != 1)
						{
							_sensor_temp_adj_resume_posi();	/* 調整用のDA,Gainに戻す */
							_sensor_set_mode(SENSOR_MODE_TEMP_ADJ);
							_sensor_ctrl_config(SENSOR_SEQ_POSI_ADJ, 0, SENSOR_POSITION_RISE_TIME);
						}
					}
					s_sens_temp_pos_value = ex_position_sensor;

				}
				break;
			case SENSOR_MODE_TEMP_ADJ_CIS:
				// CISセンサー補正中
				// ポジションセンサーチェックする
				if (current_flag & EVT_SENSOR_POSITION_AD)
				{
				
					s_sens_temp_pos_check++;
					if (!_is_sensor_position_all_off())
					{
						// some sensor(s) on
						s_sens_temp_pos_detect++;
					}
					else
					{
						// clear
						s_sens_temp_pos_detect = 0;
					}
					if((s_sens_temp_pos_detect >= 2) && (s_sens_temp_pos_value & ex_position_sensor))
					{ /*  */
						_sensor_temp_adj_restore_vali();
						_sensor_temp_adj_restore_posi();	/* 通常監視用のDA,Gainに戻す */
						s_sens_temp_result = TEMP_ADJ_SENSOR_SHIFT;
						_sensor_ctrl_config(SENSOR_SEQ_LED_BLINK, SENSOR_STANDBY_OFF_INTERVAL, SENSOR_STANDBY_ON_INTERVAL);
						_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_TEMP_ADJ_RSP, s_sens_temp_result, ex_position_sensor, 0, 0);
						_sensor_set_mode(SENSOR_MODE_STANDBY);
					}
					else
					{
						if (s_sens_temp_cnt != 0)
						{
							s_sens_temp_cnt--;
						}
						end_cnt = _sensor_temp_adj_vali();
						if ((end_cnt >= ADJUSTMENT_SEQUENCE_END) || (s_sens_temp_cnt == 0))
						{ /* 調整完 || 指定回数オーバー */
							_sensor_set_mode(SENSOR_MODE_TEMP_ADJ_FINAL);
						}
					}
					s_sens_temp_pos_value = ex_position_sensor;
				
				}
				break;
			case SENSOR_MODE_TEMP_ADJ_FINAL:
				if (current_flag & EVT_SENSOR_POSITION_AD)
				{
					
					if (s_sens_temp_cnt != 0)
					{/* 正常終了 */
						_sensor_temp_adj_success_vali();
						s_sens_temp_result = TEMP_ADJ_SUCCESS;
						_sensor_ctrl_config(SENSOR_SEQ_LED_BLINK, SENSOR_STANDBY_OFF_INTERVAL, SENSOR_STANDBY_ON_INTERVAL);
						_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_TEMP_ADJ_RSP, s_sens_temp_result, 0, 0, 0);
					}
					else
					{ /* 指定回数オーバー */
						_sensor_temp_adj_over_vali();
						s_sens_temp_result = TEMP_ADJ_OVER_RUN;
						_sensor_ctrl_config(SENSOR_SEQ_LED_BLINK, SENSOR_STANDBY_OFF_INTERVAL, SENSOR_STANDBY_ON_INTERVAL);
						_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_TEMP_ADJ_RSP, s_sens_temp_result, 0, 0, 0);
					}
					_sensor_set_mode(SENSOR_MODE_STANDBY);
					
				}
				break;
			default:
				break;
			}
		}
		ercd = prcv_mbx(ID_SENSOR_MBX, (T_MSG **)&tmsg_pt);
		if (ercd == E_OK)
		{
			memcpy(&sensor_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(sensor_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_sensor_system_error(1, 3);
			}
			_sensor_msg_proc();
		}
	}
}


void _sensor_initialize_proc(void)
{
	T_MSG_BASIC *tmsg_pt;
	FLGPTN current_flag = 0;
	ER ercd;
	u8 seq = 0;
	u16 tmp_value = 0;

	tx_thread_vfp_enable();
//#if (_DEBUG_CIS_AS_A_POSITION==1)
	_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_DISABLE);
//#endif

#if !defined(DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE)
	memset(&s_sens_entr_ref, 0, sizeof(s_sens_entr_ref));
	memset(&s_sens_cent_ref, 0, sizeof(s_sens_cent_ref));
	memset(&s_sens_apbi_ref, 0, sizeof(s_sens_apbi_ref));
	memset(&s_sens_apbo_ref, 0, sizeof(s_sens_apbo_ref));
	memset(&s_sens_exit_ref, 0, sizeof(s_sens_exit_ref));
#endif /* defined(DEBUG_POS_SENSOR_ADJUSTMENT) */

	_sensor_set_mode(SENSOR_MODE_INIT);
	while (seq != 2)
	{
		switch (seq)
		{
		case 0:
			ercd = rcv_mbx(ID_SENSOR_MBX, (T_MSG **)&tmsg_pt);
			if (ercd == E_OK)
			{
				memcpy(&sensor_msg, tmsg_pt, sizeof(T_MSG_BASIC));
				if ((rel_mpf(sensor_msg.mpf_id, tmsg_pt)) != E_OK)
				{
					/* system error */
					_sensor_system_error(1, 4);
				}
				if (sensor_msg.tmsg_code == TMSG_SENSOR_INIT_REQ)
				{
					clr_flg(ID_SENSOR_FLAG, ~EVT_ALL_BIT);

					_sensor_ctrl_config(SENSOR_SEQ_INIT, SENSOR_ACTIVE_OFF_INTERVAL, SENSOR_ACTIVE_ON_INTERVAL);
					seq++;
				}
			}
			break;
		case 1:
			ercd = wai_flg(ID_SENSOR_FLAG, EVT_ALL_BIT, TWF_ORW, &current_flag);
			if (ercd == E_OK)
			{
				if (current_flag & EVT_SENSOR_INIT)
				{
					//2024-12-02
					//入口
					//case1 +17でDA255以内の場合 DA +17
					// +17でDA255を超える場合、
					//case2 すでのHigh Gainの場合、DA 255
					//case3 それ以外、DAそのままでGainをHigh Gain設定にする
					if(ex_position_da.entrance <= SENSOR_ENT_ADJUST_DA_MAX - SENSOR_POSI_DA_POWER_UP)
					{
					//case1 DAを上げる
						ex_position_da.entrance = ex_position_da.entrance + SENSOR_POSI_DA_POWER_UP;
					}
					// Gainを上げる
					else if(IS_HIGH_GAIN_ENTRANCE)
					{
					//case2 High Gainの場合はDA MAX
						ex_position_da.entrance = SENSOR_ENT_ADJUST_DA_MAX;
					}
					else
					{
					/*case3  High Gainを使用していないので、High Gain設定, DAはそのまま */
						ex_position_ga |= POSI_ENTRANCE;
					}
				/*--------------------------------------*/
					//幅よせ
					if(ex_position_da.centering <= SENSOR_CEN_ADJUST_DA_MAX - SENSOR_POSI_DA_POWER_UP)
					{
					//case1 DAを上げる
						ex_position_da.centering = ex_position_da.centering + SENSOR_POSI_DA_POWER_UP;
					}
					// Gainを上げる
					else if(IS_HIGH_GAIN_CENTERING)
					{
					//case2 High Gainの場合はDA MAX
						ex_position_da.centering = SENSOR_CEN_ADJUST_DA_MAX;
					}
					else
					{
					/*case3  High Gainを使用していないので、High Gain設定, DAはそのまま */
						ex_position_ga |= POSI_CENTERING;
					}
				/*--------------------------------------*/

				/*--------------------------------------*/
					//PB IN
					if(ex_position_da.apb_in <= SENSOR_PBI_ADJUST_DA_MAX - SENSOR_POSI_DA_POWER_UP)
					{
					//case1 DAを上げる
						ex_position_da.apb_in = ex_position_da.apb_in + SENSOR_POSI_DA_POWER_UP;
					}
					// Gainを上げる
					else if(IS_HIGH_GAIN_APB_IN)
					{
					//case2 High Gainの場合はDA MAX
						ex_position_da.apb_in = SENSOR_PBI_ADJUST_DA_MAX;
					}
					else
					{
					/*case3  High Gainを使用していないので、High Gain設定, DAはそのまま */
						ex_position_ga |= POSI_APB_IN;
					}
				/*--------------------------------------*/

				/*--------------------------------------*/
					//PB OUT
					if(ex_position_da.apb_out <= SENSOR_PBO_ADJUST_DA_MAX - SENSOR_POSI_DA_POWER_UP)
					{
					//case1 DAを上げる
						ex_position_da.apb_out = ex_position_da.apb_out + SENSOR_POSI_DA_POWER_UP;
					}
					// Gainを上げる
					else if(IS_HIGH_GAIN_APB_OUT)
					{
					//case2 High Gainの場合はDA MAX
						ex_position_da.apb_out = SENSOR_PBO_ADJUST_DA_MAX;
					}
					else
					{
					/*case3  High Gainを使用していないので、High Gain設定, DAはそのまま */
						ex_position_ga |= POSI_APB_OUT;
					}
				/*--------------------------------------*/

				/*--------------------------------------*/
					//EXIT
					if(ex_position_da.exit <= SENSOR_EXT_ADJUST_DA_MAX - SENSOR_POSI_DA_POWER_UP)
					{
					//case1 DAを上げる
						ex_position_da.exit = ex_position_da.exit + SENSOR_POSI_DA_POWER_UP;
					}
					// Gainを上げる
					else if(IS_HIGH_GAIN_EXIT)
					{
					//case2 High Gainの場合はDA MAX
						ex_position_da.exit = SENSOR_EXT_ADJUST_DA_MAX;
					}
					else
					{
					/*case3  High Gainを使用していないので、High Gain設定, DAはそのまま */
						ex_position_ga |= POSI_EXIT;
					}
				/*--------------------------------------*/

					set_position_da();
					set_position_ga(); //Gainも追加

					_sensor_ctrl_config(SENSOR_SEQ_INIT, SENSOR_ACTIVE_OFF_INTERVAL, SENSOR_ACTIVE_ON_INTERVAL);
					seq = 3;
				}
			}
			break;
		//(_DEBUG_POWERUP_POSITION_SENSOR==1)
		case 3:
			ercd = wai_flg(ID_SENSOR_FLAG, EVT_ALL_BIT, TWF_ORW, &current_flag);
			if (ercd == E_OK)
			{
				if (current_flag & EVT_SENSOR_INIT)
				{
					_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_INIT_RSP, ex_position_sensor, 0, 0, 0);
					seq = 2;
				}
			}
			break;
		default:
			seq = 0;
		}
	}
	_sensor_set_mode(SENSOR_MODE_ACTIVE);
}

extern const MOTOR_LIMIT_STACKER_TABLE motor_limit_stacker_table[STACKER_AD_NUMBER];

void _sensor_temp_ic_uba(void)//スタッカーの電流制限設定
{
	u8 cnt = 0;
	u16 motor_limit = 0;

	for (cnt = 0; cnt < STACKER_AD_NUMBER; cnt++)
	{
		if ((s16)ex_temperature.outer <= motor_limit_stacker_table[cnt].tempic_ad)
		{
			// 使用テーブル決定
			motor_limit_stacker_table_index = cnt;
			break;
		}
	}
}

void _sensor_msg_proc(void)
{
	switch (sensor_msg.tmsg_code)
	{
	case TMSG_SENSOR_ACTIVE_REQ:
		_sensor_set_mode(SENSOR_MODE_ACTIVE);
		_sensor_ctrl_config(SENSOR_SEQ_LED_ON, SENSOR_ACTIVE_OFF_INTERVAL, SENSOR_ACTIVE_ON_INTERVAL);
		_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_ACTIVE_RSP, 0, 0, 0, 0);
		break;
	case TMSG_SENSOR_STANDBY_REQ:
		if((ex_sensor_task_mode != SENSOR_MODE_ACTIVE)
		 && (ex_sensor_task_mode != SENSOR_MODE_STANDBY))
		{
			_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_STANDBY_RSP, TMSG_SUB_ALARM, ex_position_sensor, 0, 0);
			program_error();
		}
		else
		{
			_sensor_set_mode(SENSOR_MODE_STANDBY);
			_sensor_ctrl_config(SENSOR_SEQ_LED_BLINK, SENSOR_STANDBY_OFF_INTERVAL, SENSOR_STANDBY_ON_INTERVAL);
		}
		break;
	case TMSG_SENSOR_STATUS_REQ:
		_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_STATUS_INFO, TMSG_SUB_SUCCESS, ex_position_sensor, 0, 0);
		break;
	case TMSG_SENSOR_TEMP_ADJ_REQ:
		_sensor_temp_ic_uba();//現状の温度ICは頻繁に更新されているので、参照のみ
		//2022-10-18 待機時センサ調整無効
	//	s_sens_temp_result = TEMP_ADJ_SUCCESS;
	//	_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_TEMP_ADJ_RSP, s_sens_temp_result, 0, 0, 0);
	//	break;
	
	#if 0//#if (DEBUG_ADJUSTMENT_DISABLE==1)
		s_sens_temp_result = TEMP_ADJ_SUCCESS;
		_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_TEMP_ADJ_RSP, s_sens_temp_result, 0, 0, 0);
		break;
	#else
		_sensor_set_mode(SENSOR_MODE_TEMP_ADJ);
		//tmp_adj_da(400);
		//_sensor_temp_adj_init(500);
		s_sens_temp_result = 0;
		s_sens_temp_cnt = 500;
		_sensor_temp_adj_init_vali();
		_sensor_temp_adj_init_posi();
		_sensor_ctrl_config(SENSOR_SEQ_POSI_ADJ, 0, SENSOR_POSITION_RISE_TIME);
		break;
	#endif
	case TMSG_TIMER_TIMES_UP:
		break;
	case TMSG_SENSOR_CIS_ACTIVE_REQ:
	//#if (_DEBUG_CIS_AS_A_POSITION==1)
		_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_RUN);
		_sensor_send_msg(ID_MAIN_MBX, TMSG_SENSOR_CIS_ACTIVE_RSP, 0, 0, 0, 0);
	//#endif
		break;

	default:					/* other */
		/* system error ? */
		_sensor_system_error(0, 5);
		break;
	}
}


void _sensor_temp_adj_init_vali(void)
{
#if !defined(DEBUG_VALIDATION_DISABLE)//yuji 2019-08-29
	// initialize CIS variables
	u8 led;
	float *p_pga;

	p_pga = (float *)&ex_cis_adjustment_tmp.cis_pga;

	memset(&s_sens_tempadj_cis, 0, sizeof(s_sens_tempadj_cis));
	memset(&ex_calc_pga, 0, sizeof(ex_calc_pga));

	for(led = 0; led < 20; led++)
	{
		s_sens_tempadj_cis[led].backup_pga = p_pga[led];
		p_pga[led] = 1.0f;
	}
	ex_cis_adjustment_tbl.weit = 1;
	ex_cis_adjustment_tbl.sequence = ADJUSTMENT_PL_INIT;
#endif
}
void _sensor_temp_adj_restore_pos_ga(void)
{
#if !defined(DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE)
	// POS GAIN
	ex_position_ga = 0;
	if(s_sens_tempadj_entr.backup_ga)
	{
		ex_position_ga |= POSI_ENTRANCE;
	}
	if(s_sens_tempadj_cent.backup_ga)
	{
		ex_position_ga |= POSI_CENTERING;
	}
	if(s_sens_tempadj_apbi.backup_ga)
	{
		ex_position_ga |= POSI_APB_IN;
	}
	if(s_sens_tempadj_apbo.backup_ga)
	{
		ex_position_ga |= POSI_APB_OUT;
	}
	if(s_sens_tempadj_exit.backup_ga)
	{
		ex_position_ga |= POSI_EXIT;
	}
	//GAIN値ｾｯﾄ
	if (0 != set_position_ga())
	{
		/* system error */
		_sensor_system_error(1, 6);
	}
#endif /* defined(DEBUG_POS_SENSOR_ADJUSTMENT) */
}


void _sensor_temp_adj_init_posi(void)
{
#if !defined(DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE)
	memset(&s_sens_tempadj_entr, 0, sizeof(s_sens_tempadj_entr));
	memset(&s_sens_tempadj_cent, 0, sizeof(s_sens_tempadj_cent));
	memset(&s_sens_tempadj_apbi, 0, sizeof(s_sens_tempadj_apbi));
	memset(&s_sens_tempadj_apbo, 0, sizeof(s_sens_tempadj_apbo));
	memset(&s_sens_tempadj_exit, 0, sizeof(s_sens_tempadj_exit));

	s_sens_tempadj_entr.backup_da = ex_position_da.entrance;
	s_sens_tempadj_cent.backup_da = ex_position_da.centering;
	s_sens_tempadj_apbi.backup_da = ex_position_da.apb_in;
	s_sens_tempadj_apbo.backup_da = ex_position_da.apb_out;
	s_sens_tempadj_exit.backup_da = ex_position_da.exit;

	// Set Threshold 2.0V (155)
	ex_position_da.ent_threshold = POSITION_THRESHOLD_ADJ;
	ex_position_da.ext_threshold = POSITION_THRESHOLD_ADJ;

	//入口センサのADのみ、動作時2.5倍程度された値で動作させているので、
	//オリジナルの設定値から調整開始
	ex_position_da.entrance = ex_position_da_adj.entrance;

	s_sens_tempadj_entr.backup_ga = GAIN_ENTRANCE;	/* これはgainではなくenable flag */
	s_sens_tempadj_cent.backup_ga = GAIN_CENTERING;
	s_sens_tempadj_apbi.backup_ga = GAIN_APB_IN;
	s_sens_tempadj_apbo.backup_ga = GAIN_APB_OUT;
	s_sens_tempadj_exit.backup_ga = GAIN_EXIT;

	set_position_da();

	memset(&s_sens_entr_ref, 0, sizeof(s_sens_entr_ref));
	memset(&s_sens_cent_ref, 0, sizeof(s_sens_cent_ref));
	memset(&s_sens_apbi_ref, 0, sizeof(s_sens_apbi_ref));
	memset(&s_sens_apbo_ref, 0, sizeof(s_sens_apbo_ref));
	memset(&s_sens_exit_ref, 0, sizeof(s_sens_exit_ref));
#endif /* defined(DEBUG_POS_SENSOR_ADJUSTMENT) */
}

void _sensor_temp_adj_restore_vali(void)
{
	// restore CIS variables
#if !defined(DEBUG_VALIDATION_DISABLE)// yuji debug 2019-08-29
	u8 led;
	float *p_pga;
	p_pga = (float *)&ex_cis_adjustment_tmp.cis_pga;

	for(led = 0; led < 20; led++)
	{
		p_pga[led] = s_sens_tempadj_cis[led].backup_pga;
	}
#endif
}

void _sensor_temp_adj_resume_posi(void)	/* 調整用のDA,Gainに戻す */
{
#if !defined(DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE)
	/* 前回待機調整値時のDA基準値①を代入 */
	if (s_sens_entr_ref.da1 != 0)
	{
		ex_position_da.entrance = s_sens_entr_ref.da1;
		if(s_sens_entr_ref.ga1)
		{
			ex_position_ga |= POSI_ENTRANCE;
		}
		else
		{
			ex_position_ga &= ~POSI_ENTRANCE;
		}
	}
	if (s_sens_cent_ref.da1 != 0)
	{
		ex_position_da.centering = s_sens_cent_ref.da1;
		if(s_sens_cent_ref.ga1)
		{
			ex_position_ga |= POSI_CENTERING;
		}
		else
		{
			ex_position_ga &= ~POSI_CENTERING;
		}
	}
	if (s_sens_apbi_ref.da1 != 0)
	{
		ex_position_da.apb_in = s_sens_apbi_ref.da1;
		if(s_sens_apbi_ref.ga1)
		{
			ex_position_ga |= POSI_APB_IN;
		}
		else
		{
			ex_position_ga &= ~POSI_APB_IN;
		}
	}
	if (s_sens_apbo_ref.da1 != 0)
	{
		ex_position_da.apb_out = s_sens_apbo_ref.da1;
		if(s_sens_apbo_ref.ga1)
		{
			ex_position_ga |= POSI_APB_OUT;
		}
		else
		{
			ex_position_ga &= ~POSI_APB_OUT;
		}
	}
	if (s_sens_exit_ref.da1 != 0)
	{
		ex_position_da.exit = s_sens_exit_ref.da1;
		if(s_sens_exit_ref.ga1)
		{
			ex_position_ga |= POSI_EXIT;
		}
		else
		{
			ex_position_ga &= ~POSI_EXIT;
		}
	}
	//GAIN値ｾｯﾄ
	if (0 != set_position_ga())
	{
		/* system error */
		_sensor_system_error(1, 6);
	}
	// Set Threshold 2.0V (155)
	ex_position_da.ent_threshold = POSITION_THRESHOLD_ADJ;
	ex_position_da.ext_threshold = POSITION_THRESHOLD_ADJ;

	set_position_da();
#endif
}

void _sensor_temp_adj_restore_posi(void)
{
	s_sens_temp_pos_check = 0;
	s_sens_temp_pos_detect = 0;
#if !defined(DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE)
	ex_position_da.entrance = s_sens_tempadj_entr.backup_da;
	ex_position_da.centering = s_sens_tempadj_cent.backup_da;
	ex_position_da.apb_in = s_sens_tempadj_apbi.backup_da;
	ex_position_da.apb_out = s_sens_tempadj_apbo.backup_da;
	ex_position_da.exit = s_sens_tempadj_exit.backup_da;
	// Set Threshold bill detection 0.4V (31)
	ex_position_da.ent_threshold = POSITION_THRESHOLD_DETECTION;
	ex_position_da.ext_threshold = POSITION_THRESHOLD_DETECTION;

	_sensor_temp_adj_restore_pos_ga();
	/* D/A値セット */
	set_position_da();
#endif
}


void _sensor_temp_adj_success_vali(void)
{
#if !defined(DEBUG_VALIDATION_DISABLE)
	memcpy(&ex_cis_adjustment_tmp.cis_pga, &ex_calc_pga, sizeof(CIS_ADJUSTMENT_PGA));
#endif
	// PL off
    //_main_set_pl_active(PL_DISABLE);
#if (_ENABLE_JDL==1)
    jdl_sens_update_cor_val();
    jdl_ener_time();
#endif /* _ENABLE_JDL */

}

void _sensor_temp_adj_success_posi(void)	/* 調整成功 */
{
#if !defined(DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE)
	memcpy(&ex_position_da_adj, &ex_position_da, sizeof(POSITION_SENSOR));

	ex_position_da.ent_threshold = POSITION_THRESHOLD_DETECTION;
	ex_position_da.ext_threshold = POSITION_THRESHOLD_DETECTION;

	set_position_da();
	#if defined(UBA_RTQ) //#if defined(ID003_SENSOR)	/* 2021-01-19 */
	check_sensor_level();
	#endif

#endif
}

void _sensor_temp_adj_over_vali(void)
{
	// Over CIS variables
#if !defined(DEBUG_VALIDATION_DISABLE)
	u8 led;
	float *p_pga;
	p_pga = (float *)&ex_cis_adjustment_tmp.cis_pga;

	for(led = 0; led < 20; led++)
	{
		p_pga[led] = s_sens_tempadj_cis[led].backup_pga;
	}
#endif
}


void _sensor_temp_adj_over_posi(void)
{
#if !defined(DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE)
	if (s_sens_entr_ref.da1 == 0)
	{
		s_sens_entr_ref.da1 = ex_position_da.entrance;
		s_sens_entr_ref.ga1 = GAIN_ENTRANCE;
	}
	//ex_position_da.entrance = s_sens_tempadj_entr.backup_da;
	if (s_sens_cent_ref.da1 == 0)
	{
		s_sens_cent_ref.da1 = ex_position_da.centering;
		s_sens_cent_ref.ga1 = GAIN_CENTERING;
	}
	//ex_position_da.centering = s_sens_tempadj_cent.backup_da;
	if (s_sens_apbi_ref.da1 == 0)
	{
		s_sens_apbi_ref.da1 = ex_position_da.apb_in;
		s_sens_apbi_ref.ga1 = GAIN_APB_IN;
	}
	//ex_position_da.apb_in = s_sens_tempadj_apbi.backup_da;
	if (s_sens_apbo_ref.da1 == 0)
	{
		s_sens_apbo_ref.da1 = ex_position_da.apb_out;
		s_sens_apbo_ref.ga1 = GAIN_APB_OUT;
	}
	//ex_position_da.apb_out = s_sens_tempadj_apbo.backup_da;
	if (s_sens_exit_ref.da1 == 0)
	{
		s_sens_exit_ref.da1 = ex_position_da.exit;
		s_sens_exit_ref.ga1 = GAIN_EXIT;
	}
	//ex_position_da.exit = s_sens_tempadj_exit.backup_da;

	// Set Threshold bill detection 0.4V (31)
	memcpy(&ex_position_da_adj, &ex_position_da, sizeof(POSITION_SENSOR));
	ex_position_da.ent_threshold = POSITION_THRESHOLD_DETECTION;
	ex_position_da.ext_threshold = POSITION_THRESHOLD_DETECTION;

	set_position_da();
#endif
}


u16 _sensor_temp_adj_vali(void)
{
	u16 end_cnt = 0;
#if !defined(DEBUG_VALIDATION_DISABLE)//yuji 2019-08-29
	_pl_cis_oneshot_sequence(CIS_MODE_BC_NON_PAPER);
	dly_tsk(ex_cis_adjustment_tbl.weit);

	end_cnt = ex_cis_adjustment_tbl.sequence;

	if(ex_cis_adjustment_tbl.sequence >= ADJUSTMENT_SEQUENCE_END)
	{
		if(!_cis_temp_adjust_pga())
		{
			//Calcration PGA error
			s_sens_temp_cnt = 0;
		}
	}
#else
	end_cnt = ADJUSTMENT_SEQUENCE_END;
#endif
	return end_cnt;
}


u16 _sensor_temp_adj_posi(void)	/* 調整結果の確認 */
{
	u16 end_cnt = 0;
#if !defined(DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE)
	if ((s_sens_tempadj_entr.last_st == SENSOR_ST_UNKNOWN)
	 && (s_sens_tempadj_cent.last_st == SENSOR_ST_UNKNOWN)
	 && (s_sens_tempadj_apbi.last_st == SENSOR_ST_UNKNOWN)
	 && (s_sens_tempadj_apbo.last_st == SENSOR_ST_UNKNOWN)
	 && (s_sens_tempadj_exit.last_st == SENSOR_ST_UNKNOWN))
	{
		/* 待機時補正の最初のみ呼び出される */
		if(SENSOR_ADJ_ENTRANCE)
		{
		/* 札ありと判定 */
			s_sens_tempadj_entr.last_st = SENSOR_ST_ON;
		}
		else
		{
			s_sens_tempadj_entr.last_st = SENSOR_ST_OFF;
		}
		if(SENSOR_ADJ_CENTERING)
		{
			s_sens_tempadj_cent.last_st = SENSOR_ST_ON;
		}
		else
		{
			s_sens_tempadj_cent.last_st = SENSOR_ST_OFF;
		}
		if(SENSOR_ADJ_APB_IN)
		{
			s_sens_tempadj_apbi.last_st = SENSOR_ST_ON;
		}
		else
		{
			s_sens_tempadj_apbi.last_st = SENSOR_ST_OFF;
		}
		if(SENSOR_ADJ_APB_OUT)
		{
			s_sens_tempadj_apbo.last_st = SENSOR_ST_ON;
		}
		else
		{
			s_sens_tempadj_apbo.last_st = SENSOR_ST_OFF;
		}
		if(SENSOR_ADJ_EXIT)
		{
			s_sens_tempadj_exit.last_st = SENSOR_ST_ON;
		}
		else
		{
			s_sens_tempadj_exit.last_st = SENSOR_ST_OFF;
		}
		/* 現在の調整DA値を代入 */
		s_sens_entr_ref.da1 = ex_position_da.entrance;
		s_sens_cent_ref.da1 = ex_position_da.centering;
		s_sens_apbi_ref.da1 = ex_position_da.apb_in;
		s_sens_apbo_ref.da1 = ex_position_da.apb_out;
		s_sens_exit_ref.da1 = ex_position_da.exit;
		/* 現在の調整GA値を代入 */
		s_sens_entr_ref.ga1 = GAIN_ENTRANCE;
		s_sens_cent_ref.ga1 = GAIN_CENTERING;
		s_sens_apbi_ref.ga1 = GAIN_APB_IN;
		s_sens_apbo_ref.ga1 = GAIN_APB_OUT;
		s_sens_exit_ref.ga1 = GAIN_EXIT;

		//2024-01-11
		s_sens_tempadj_entr.end_flag = 0;
		s_sens_tempadj_cent.end_flag = 0;
		s_sens_tempadj_apbi.end_flag = 0;
		s_sens_tempadj_apbo.end_flag = 0;
		s_sens_tempadj_exit.end_flag = 0;	

		return end_cnt;
	}
	/* Entrance Sensor */
	if(s_sens_tempadj_entr.end_flag)
	{
		end_cnt++;	/*調整完了 */
	}
	else if(s_sens_tempadj_entr.last_st == SENSOR_ST_ON)
	{
	/* 調整開始時札ありと判定 */
		// start status Low
		if(SENSOR_ADJ_ENTRANCE)
		{
		/* 札ありと判定されているので、DAを上げる必要あり */
		//same
			if (ex_position_da.entrance < SENSOR_ENT_ADJUST_DA_MAX)
			{
				ex_position_da.entrance += 1;
			}
			else
			{
				if(IS_HIGH_GAIN_ENTRANCE)
				{
					ex_position_da.entrance = SENSOR_ENT_ADJUST_DA_MAX;
				}
				else
				{
				/* High Gainを使用していないので、High Gain設定にして DAを下からもう一度試す */
					ex_position_ga |= POSI_ENTRANCE;
					ex_position_da.entrance = SENSOR_ENT_ADJUST_DA_INI;
				}
			}
			s_sens_entr_ref.da1 = ex_position_da.entrance;
			s_sens_entr_ref.ga1 = GAIN_ENTRANCE;
			s_sens_tempadj_entr.end_flag = 0;
			s_sens_tempadj_entr.count = 0;
		}
		else
		{
		/* 調整開始時紙あり、今回紙なし */
		/* 調整開始時に札ありの時の調整の最終フェーズ*/
			if(s_sens_tempadj_entr.count)
			{
				s_sens_tempadj_entr.end_flag = 1;	/*調整完了 */
				end_cnt++;	/*調整完了 */
			}
			s_sens_tempadj_entr.count ++;
		}
	}
	else
	{
	/* 調整開始時札なしと判定 */
		// start status High
		if(!SENSOR_ADJ_ENTRANCE)
		{
		/* 札なしと判定 */
			if(s_sens_tempadj_entr.reverse_flag)
			{
			/* 調整開始時に札なしの時の調整の最終フェーズ*/
				if(s_sens_tempadj_entr.count)
				{
					s_sens_tempadj_entr.end_flag = 1; 	/*調整完了 */
					end_cnt++;
				}
				s_sens_tempadj_entr.count ++;
			}
			else
			{
			/* 調整開始時に札なしなので、一度札ありにしたい為、DAを下げ続ける処理*/
			/* DAを最低まで下げても、札なしのままの場合は、Gainを下げるようにもしたいが*/
			/* そうすると、場合によってHigh Gainを使用していてもDAを最低にする前に札ありとなり*/
			/* High GainでDAを上げる処理に遷移してしまう*/
			/* 実際はDAが消灯に近い状態になっている可能性がある*/
			/* High Gainの場合は、DAを下げきる前に見切りをつけてGainを先に変えた方がいい*/
			#if 1 //2024-01-17

				if(IS_HIGH_GAIN_ENTRANCE)//if(ex_position_ga & POSI_ENTRANCE)
				{
				/* useing High gain*/
					if (ex_position_da.entrance > SENSOR_ENT_ADJUST_DA_MIN + 30)
					{
						ex_position_da.entrance -= 1;
					}
					else
					{
					/* High geinを使用しているので、Low gainに変更して確認 */
						ex_position_ga &= ~POSI_ENTRANCE;
						ex_position_da.entrance = SENSOR_ENT_ADJUST_DA_MAX;
					}
				}
				else
				{
					if (ex_position_da.entrance > SENSOR_ENT_ADJUST_DA_MIN)
					{
						ex_position_da.entrance -= 1;
					}
					else
					{
					/* useing Low gain*/
						ex_position_da.entrance = SENSOR_ENT_ADJUST_DA_MIN;

						//2024-01-11
						/* DA最小にしても、札なしなので、札ありには到達しないので、このDAで調整終了 */
						s_sens_tempadj_entr.end_flag = 1;	/*調整完了 */
						end_cnt++;	/*調整完了 */
					}
				}
				s_sens_entr_ref.da1 = ex_position_da.entrance;
				s_sens_entr_ref.ga1 = GAIN_ENTRANCE;
				s_sens_tempadj_entr.end_flag = 0;
				s_sens_tempadj_entr.count = 0;

			#else
				if (ex_position_da.entrance > SENSOR_ENT_ADJUST_DA_MIN)
				{
					ex_position_da.entrance -= 1;
				}
				else
				{
					if(IS_HIGH_GAIN_ENTRANCE)//if(ex_position_ga & POSI_ENTRANCE)
					{
					/* useing High gain*/
					/* High geinを使用しているので、Low gainに変更して確認 */
						ex_position_ga &= ~POSI_ENTRANCE;
						ex_position_da.entrance = SENSOR_ENT_ADJUST_DA_MAX;
					}
					else
					{
						/* useing Low gain*/
						ex_position_da.entrance = SENSOR_ENT_ADJUST_DA_MIN;

						//2024-01-11
						/* DA最小にしても、札なしなので、札ありには到達しないので、このDAで調整終了 */
						s_sens_tempadj_entr.end_flag = 1;	/*調整完了 */
						end_cnt++;	/*調整完了 */

					}
				}
				s_sens_entr_ref.da1 = ex_position_da.entrance;
				s_sens_entr_ref.ga1 = GAIN_ENTRANCE;
				s_sens_tempadj_entr.end_flag = 0;
				s_sens_tempadj_entr.count = 0;
			#endif
			}
		}
		else
		{
		/* 調整開始時札なしと判定、今回紙ありと判定 */
		/* DAを下げ続けた結果、札ありとなったので、今度は札なしまでDAを上げていく */
			if (ex_position_da.entrance < SENSOR_ENT_ADJUST_DA_MAX)
			{
				ex_position_da.entrance += 1;
			}
			else
			{
				if(IS_HIGH_GAIN_ENTRANCE)
				{
					ex_position_da.entrance = SENSOR_ENT_ADJUST_DA_MAX;
				}
				else
				{
				/* High Gainを使用していないので、High Gain設定にして DAを下からもう一度試す */
					ex_position_ga |= POSI_ENTRANCE;
					ex_position_da.entrance = SENSOR_ENT_ADJUST_DA_INI;
				}
			}
			s_sens_entr_ref.da1 = ex_position_da.entrance;
			s_sens_entr_ref.ga1 = GAIN_ENTRANCE;
			s_sens_tempadj_entr.reverse_flag = 1;
			s_sens_tempadj_entr.count = 0;
		}
	}

	/* Centering Sensor */
	if(s_sens_tempadj_cent.end_flag)
	{
		end_cnt++;
	}
	else if(s_sens_tempadj_cent.last_st == SENSOR_ST_ON)
	{
	/* 調整開始時札ありと判定 */
		// start status Low
		if(SENSOR_ADJ_CENTERING)
		{
		/* 札ありと判定されているので、DAを上げる必要あり */
			if (ex_position_da.centering < SENSOR_CEN_ADJUST_DA_MAX)
			{
				ex_position_da.centering += 1;
			}
			else
			{
				if(IS_HIGH_GAIN_CENTERING)
				{
					ex_position_da.centering = SENSOR_CEN_ADJUST_DA_MAX;
				}
				else
				{
					ex_position_ga |= POSI_CENTERING;
					ex_position_da.centering = SENSOR_CEN_ADJUST_DA_INI;
				}
			}
			s_sens_cent_ref.da1 = ex_position_da.centering;
			s_sens_cent_ref.ga1 = GAIN_CENTERING;
			s_sens_tempadj_cent.end_flag = 0;
			s_sens_tempadj_cent.count = 0;
		}
		else
		{
		/* 調整開始時紙あり、今回紙なし */	
			if(s_sens_tempadj_cent.count)
			{
				s_sens_tempadj_cent.end_flag = 1;
				end_cnt++;
			}
			s_sens_tempadj_cent.count ++;
		}
	}
	else
	{
	/* 調整開始時札なしと判定 */
		// start status High
		if(!SENSOR_ADJ_CENTERING)
		{
		/* 札なしと判定 */	
			if(s_sens_tempadj_cent.reverse_flag)
			{
				if(s_sens_tempadj_cent.count)
				{
					s_sens_tempadj_cent.end_flag = 1;
					end_cnt++;
				}
				s_sens_tempadj_cent.count ++;
			}
			else
			{
				//2024-01-17
				if(IS_HIGH_GAIN_CENTERING)
				{
					if (ex_position_da.centering > SENSOR_CEN_ADJUST_DA_MIN + 30)
					{
						ex_position_da.centering -= 1;
					}
					else
					{
					/* High geinを使用しているので、Low gainに変更して確認 */	
						ex_position_ga &= ~POSI_CENTERING;
						ex_position_da.centering = SENSOR_CEN_ADJUST_DA_MAX;
					}
				}
				else
				{
					if (ex_position_da.centering > SENSOR_CEN_ADJUST_DA_MIN)
					{
						ex_position_da.centering -= 1;
					}
					else
					{
						/* useing Low gain*/
						ex_position_da.centering = SENSOR_CEN_ADJUST_DA_MIN;

						//2024-01-11
						/* DA最小にしても、札なしなので、札ありには到達しないので、このDAで調整終了 */
						s_sens_tempadj_cent.end_flag = 1;	/*調整完了 */
						end_cnt++;	/*調整完了 */
					}
				}
				s_sens_cent_ref.da1 = ex_position_da.centering;
				s_sens_cent_ref.ga1 = GAIN_CENTERING;
				s_sens_tempadj_cent.end_flag = 0;
				s_sens_tempadj_cent.count = 0;
			}
		}
		else
		{
		/* 調整開始時札なしと判定、今回紙ありと判定 */
			if (ex_position_da.centering < SENSOR_CEN_ADJUST_DA_MAX)
			{
				ex_position_da.centering += 1;
			}
			else
			{
				if(IS_HIGH_GAIN_CENTERING)
				{
					ex_position_da.centering = SENSOR_CEN_ADJUST_DA_MAX;
				}
				else
				{
				/* High Gainを使用していないので、High Gain設定にして DAを下からもう一度試す */
					ex_position_ga |= POSI_CENTERING;
					ex_position_da.centering = SENSOR_CEN_ADJUST_DA_INI;
				}
			}
			s_sens_cent_ref.da1 = ex_position_da.centering;
			s_sens_cent_ref.ga1 = GAIN_CENTERING;
			s_sens_tempadj_cent.reverse_flag = 1;
			s_sens_tempadj_cent.count = 0;
		}
	}

	/* APB-IN Sensor */
	if(s_sens_tempadj_apbi.end_flag)
	{
		end_cnt++;
	}
	else if(s_sens_tempadj_apbi.last_st == SENSOR_ST_ON)
	{
	/* 調整開始時札ありと判定 */
		// start status Low
		if(SENSOR_ADJ_APB_IN)
		{
		/* 札ありと判定されているので、DAを上げる必要あり */
			if (ex_position_da.apb_in < SENSOR_PBI_ADJUST_DA_MAX)
			{
				ex_position_da.apb_in += 1;
			}
			else
			{
				if(IS_HIGH_GAIN_APB_IN)
				{
					ex_position_da.apb_in = SENSOR_PBI_ADJUST_DA_MAX;
				}
				else
				{
					ex_position_ga |= POSI_APB_IN;
					ex_position_da.apb_in = SENSOR_PBI_ADJUST_DA_INI;
				}
			}
			s_sens_apbi_ref.da1 = ex_position_da.apb_in;
			s_sens_apbi_ref.ga1 = GAIN_APB_IN;
			s_sens_tempadj_apbi.end_flag = 0;
			s_sens_tempadj_apbi.count = 0;
		}
		else
		{
		/* 調整開始時紙あり、今回紙なし */
			if(s_sens_tempadj_apbi.count)
			{
				s_sens_tempadj_apbi.end_flag = 1;
				end_cnt++;
			}
			s_sens_tempadj_apbi.count ++;
		}
	}
	else
	{
	/* 調整開始時札なしと判定 */
		// start status High
		if(!SENSOR_ADJ_APB_IN)
		{
		/* 札なしと判定 */
			if(s_sens_tempadj_apbi.reverse_flag)
			{
				if(s_sens_tempadj_apbi.count)
				{
					s_sens_tempadj_apbi.end_flag = 1;
					end_cnt++;
				}
				s_sens_tempadj_apbi.count ++;
			}
			else
			{
				//2024-01-17
				if(IS_HIGH_GAIN_APB_IN)
				{
				/* useing High gain*/
					if (ex_position_da.apb_in > SENSOR_PBI_ADJUST_DA_MIN + 30 )
					{
						ex_position_da.apb_in -= 1;
					}
					else
					{
					/* High geinを使用しているので、Low gainに変更して確認 */
						ex_position_ga &= ~POSI_APB_IN;
						ex_position_da.apb_in = SENSOR_PBI_ADJUST_DA_MAX;
					}
				}
				else
				{
					/* useing Low gain*/
					if (ex_position_da.apb_in > SENSOR_PBI_ADJUST_DA_MIN)
					{
						ex_position_da.apb_in -= 1;
					}
					else
					{
						ex_position_da.apb_in = SENSOR_PBI_ADJUST_DA_MIN;

						//2024-01-11
						/* DA最小にしても、札なしなので、札ありには到達しないので、このDAで調整終了 */
						s_sens_tempadj_apbi.end_flag = 1;	/*調整完了 */
						end_cnt++;	/*調整完了 */
					}
				}
				s_sens_apbi_ref.da1 = ex_position_da.apb_in;
				s_sens_apbi_ref.ga1 = GAIN_APB_IN;
				s_sens_tempadj_apbi.end_flag = 0;
				s_sens_tempadj_apbi.count = 0;
			}
		}
		else
		{
		/* 調整開始時札なしと判定、今回紙ありと判定 */
			if (ex_position_da.apb_in < SENSOR_PBI_ADJUST_DA_MAX)
			{
				ex_position_da.apb_in += 1;
			}
			else
			{
				if(IS_HIGH_GAIN_APB_IN)
				{
					ex_position_da.apb_in = SENSOR_PBI_ADJUST_DA_MAX;
				}
				else
				{
				/* High Gainを使用していないので、High Gain設定にして DAを下からもう一度試す */
					ex_position_ga |= POSI_APB_IN;
					ex_position_da.apb_in = SENSOR_PBI_ADJUST_DA_MIN;
				}
			}
			s_sens_apbi_ref.da1 = ex_position_da.apb_in;
			s_sens_apbi_ref.ga1 = GAIN_APB_IN;
			s_sens_tempadj_apbi.reverse_flag = 1;
			s_sens_tempadj_apbi.count = 0;
		}
	}

	/* APB-OUT Sensor */
	if(s_sens_tempadj_apbo.end_flag)
	{
		end_cnt++;
	}
	else if(s_sens_tempadj_apbo.last_st == SENSOR_ST_ON)
	{
	/* 調整開始時札ありと判定 */
		// start status Low
		if(SENSOR_ADJ_APB_OUT)
		{
		/* 札ありと判定されているので、DAを上げる必要あり */
			if (ex_position_da.apb_out < SENSOR_PBO_ADJUST_DA_MAX)
			{
				ex_position_da.apb_out += 1;
			}
			else
			{
				if(IS_HIGH_GAIN_APB_OUT)
				{
					ex_position_da.apb_out = SENSOR_PBO_ADJUST_DA_MAX;
				}
				else
				{
					ex_position_ga |= POSI_APB_OUT;
					ex_position_da.apb_out = SENSOR_PBO_ADJUST_DA_INI;
				}
			}
			s_sens_apbo_ref.da1 = ex_position_da.apb_out;
			s_sens_apbo_ref.ga1 = GAIN_APB_OUT;
			s_sens_tempadj_apbo.end_flag = 0;
			s_sens_tempadj_apbo.count = 0;
		}
		else
		{
		/* 調整開始時紙あり、今回紙なし */
			if(s_sens_tempadj_apbo.count)
			{
				s_sens_tempadj_apbo.end_flag = 1;
				end_cnt++;
			}
			s_sens_tempadj_apbo.count ++;
		}
	}
	else
	{
	/* 調整開始時札なしと判定 */
		// start status High
		if(!SENSOR_ADJ_APB_OUT)
		{
		/* 札なしと判定 */	
			if(s_sens_tempadj_apbo.reverse_flag)
			{
				if(s_sens_tempadj_apbo.count)
				{
					s_sens_tempadj_apbo.end_flag = 1;
					end_cnt++;
				}
				s_sens_tempadj_apbo.count ++;
			}
			else
			{
			//2024-01-17
				if(IS_HIGH_GAIN_APB_OUT)
				{
				/* useing High gain*/
					if (ex_position_da.apb_out > SENSOR_PBO_ADJUST_DA_MIN + 30)
					{
						ex_position_da.apb_out -= 1;
					}
					else
					{
					/* High geinを使用しているので、Low gainに変更して確認 */
						ex_position_ga &= ~POSI_APB_OUT;
						ex_position_da.apb_out = SENSOR_PBO_ADJUST_DA_MAX;
					}
				}
				else
				{
					if (ex_position_da.apb_out > SENSOR_PBO_ADJUST_DA_MIN)
					{
						ex_position_da.apb_out -= 1;
					}
					else
					{
						/* useing Low gain*/
						ex_position_da.apb_out = SENSOR_PBO_ADJUST_DA_MIN;

						//2024-01-11
						/* DA最小にしても、札なしなので、札ありには到達しないので、このDAで調整終了 */
						s_sens_tempadj_apbo.end_flag = 1;	/*調整完了 */
						end_cnt++;	/*調整完了 */		
					}
				}
				s_sens_apbo_ref.da1 = ex_position_da.apb_out;
				s_sens_apbo_ref.ga1 = GAIN_APB_OUT;
				s_sens_tempadj_apbo.end_flag = 0;
				s_sens_tempadj_apbo.count = 0;
			}
		}
		else
		{
		/* 調整開始時札なしと判定、今回紙ありと判定 */
			if (ex_position_da.apb_out < SENSOR_PBO_ADJUST_DA_MAX)
			{
				ex_position_da.apb_out += 1;
			}
			else
			{
				if(IS_HIGH_GAIN_APB_OUT)
				{
					ex_position_da.apb_out = SENSOR_PBO_ADJUST_DA_MAX;
				}
				else
				{
				/* High Gainを使用していないので、High Gain設定にして DAを下からもう一度試す */
					ex_position_ga |= POSI_APB_OUT;
					ex_position_da.apb_out = SENSOR_PBO_ADJUST_DA_MIN;
				}
			}
			s_sens_apbo_ref.da1 = ex_position_da.apb_out;
			s_sens_apbo_ref.ga1 = GAIN_APB_OUT;
			s_sens_tempadj_apbo.reverse_flag = 1;
			s_sens_tempadj_apbo.count = 0;
		}
	}

	/* Exit Sensor */
	if(s_sens_tempadj_exit.end_flag)
	{
		end_cnt++;
	}
	else if(s_sens_tempadj_exit.last_st == SENSOR_ST_ON)
	{
	/* 調整開始時札ありと判定 */
		// start status Low
		if(SENSOR_ADJ_EXIT)
		{
			if (ex_position_da.exit < SENSOR_EXT_ADJUST_DA_MAX)
			{
				ex_position_da.exit += 1;
			}
			else
			{
				if(IS_HIGH_GAIN_EXIT)
				{
					ex_position_da.exit = SENSOR_EXT_ADJUST_DA_MAX;
				}
				else
				{
					ex_position_ga |= POSI_EXIT;
					ex_position_da.exit = SENSOR_EXT_ADJUST_DA_INI;
				}
			}
			s_sens_exit_ref.da1 = ex_position_da.exit;
			s_sens_exit_ref.ga1 = GAIN_EXIT;
			s_sens_tempadj_exit.end_flag = 0;
			s_sens_tempadj_exit.count = 0;
		}
		else
		{
		/* 調整開始時紙あり、今回紙なし */
			if(s_sens_tempadj_exit.count)
			{
				s_sens_tempadj_exit.end_flag = 1;
				end_cnt++;
			}
			s_sens_tempadj_exit.count ++;
		}
	}
	else
	{
	/* 調整開始時札なしと判定 */
		// start status High
		if(!SENSOR_ADJ_EXIT)
		{
		/* 札なしと判定 */
			if(s_sens_tempadj_exit.reverse_flag)
			{
				if(s_sens_tempadj_exit.count)
				{
					s_sens_tempadj_exit.end_flag = 1;
					end_cnt++;
				}
				s_sens_tempadj_exit.count ++;
			}
			else
			{
				//2024-01-17
				if(IS_HIGH_GAIN_EXIT)
				{
				/* useing High gain*/
					if (ex_position_da.exit > SENSOR_EXT_ADJUST_DA_MIN + 30)
					{
						ex_position_da.exit -= 1;
					}
					else
					{
						ex_position_ga &= ~POSI_EXIT;
						ex_position_da.exit = SENSOR_EXT_ADJUST_DA_MAX;
					}
				}
				else
				{
					/* useing Low gain*/
					if (ex_position_da.exit > SENSOR_EXT_ADJUST_DA_MIN)
					{
						ex_position_da.exit -= 1;
					}
					else
					{
						/* useing Low gain*/
						ex_position_da.exit = SENSOR_EXT_ADJUST_DA_MIN;

						//2024-01-11
						/* DA最小にしても、札なしなので、札ありには到達しないので、このDAで調整終了 */
						s_sens_tempadj_exit.end_flag = 1;	/*調整完了 */
						end_cnt++;	/*調整完了 */
					}
				}
				s_sens_exit_ref.da1 = ex_position_da.exit;
				s_sens_exit_ref.ga1 = GAIN_EXIT;
				s_sens_tempadj_exit.end_flag = 0;
				s_sens_tempadj_exit.count = 0;
			}
		}
		else
		{
		/* 調整開始時札なしと判定、今回紙ありと判定 */
			if (ex_position_da.exit < SENSOR_EXT_ADJUST_DA_MAX)
			{
				ex_position_da.exit += 1;
			}
			else
			{
				if(IS_HIGH_GAIN_EXIT)
				{
					ex_position_da.exit = SENSOR_EXT_ADJUST_DA_MAX;
				}
				else
				{
				/* High Gainを使用していないので、High Gain設定にして DAを下からもう一度試す */
					ex_position_ga |= POSI_EXIT;
					ex_position_da.exit = SENSOR_EXT_ADJUST_DA_MIN;
				}
			}
			s_sens_exit_ref.da1 = ex_position_da.exit;
			s_sens_exit_ref.ga1 = GAIN_EXIT;
			s_sens_tempadj_exit.reverse_flag = 1;
			s_sens_tempadj_exit.count = 0;
		}
	}
	//GAIN値ｾｯﾄ
	if (0 != set_position_ga())
	{
		/* system error */
		_sensor_system_error(1, 6);
	}
#else
	/* Entrance Sensor adj */
	end_cnt++;

	/* Centering Sensor adj */
	end_cnt++;

	/* APB-IN Sensor adj */
	end_cnt++;

	/* APB-OUT Sensor adj */
	end_cnt++;

	/* Exit Sensor adj */
	end_cnt++;

#endif

	//D/A値ｾｯﾄ
	set_position_da();

	return end_cnt;
}



bool _is_sensor_position_all_off(void)
{
	if ((SENSOR_ENTRANCE) || (SENSOR_CENTERING)
	 || (SENSOR_APB_IN) || (SENSOR_APB_OUT) || (SENSOR_EXIT))
	{
		return false;
	}
	else
	{
		return true;
	}
}

void _sensor_set_mode(u16 mode)
{
	ex_sensor_task_mode = mode;

#if (_ENABLE_JDL==1)
//    jdl_add_trace(ID_SENSOR_TASK, ((ex_sensor_task_mode >> 8) & 0xFF), (ex_sensor_task_mode & 0xFF), s_sens_temp_result, (u8)((s_sens_temp_cnt >> 8) & 0xFF), (u8)(s_sens_temp_cnt & 0xFF));
#endif /* _ENABLE_JDL */
}

u32 is_position_sensor_da_and_gain_max() //現状 特殊 Toolでのみ使用
{
	switch(ex_sensor_task_mode)
	{
	case SENSOR_MODE_TEMP_ADJ:
	case SENSOR_MODE_TEMP_ADJ_POS_DETECT:
	case SENSOR_MODE_TEMP_ADJ_CIS:
	case SENSOR_MODE_TEMP_ADJ_FINAL:
#if !defined(DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE)
		/* s_sens_tempadj_entr.backup_da */
		if((!s_sens_tempadj_entr.backup_ga) && (s_sens_tempadj_entr.backup_da >= SENSOR_ENT_CLEANING_DA_THD))
		{
			return 1;
		}
		/* s_sens_tempadj_cent.backup_da */
		if((!s_sens_tempadj_cent.backup_ga) && (s_sens_tempadj_cent.backup_da >= SENSOR_CEN_CLEANING_DA_THD))
		{
			return 1;
		}
		/* s_sens_tempadj_apbi.backup_da */
		if((!s_sens_tempadj_apbi.backup_ga) && (s_sens_tempadj_apbi.backup_da >= SENSOR_PBI_CLEANING_DA_THD))
		{
			return 1;
		}
		/* s_sens_tempadj_apbo.backup_da */
		if((!s_sens_tempadj_apbo.backup_ga) && (s_sens_tempadj_apbo.backup_da >= SENSOR_PBO_CLEANING_DA_THD))
		{
			return 1;
		}
		/* s_sens_tempadj_exit.backup_da */
		if((!s_sens_tempadj_exit.backup_ga) && (s_sens_tempadj_exit.backup_da >= SENSOR_EXT_CLEANING_DA_THD))
		{
			return 1;
		}
#endif
		break;
	default:
		/* ex_position_da.entrance */
		if((IS_HIGH_GAIN_ENTRANCE) && (ex_position_da.entrance >= SENSOR_ENT_CLEANING_DA_THD))
		{
			return 1;
		}
		/* ex_position_da.centering */
		if((IS_HIGH_GAIN_CENTERING) && (ex_position_da.centering >= SENSOR_CEN_CLEANING_DA_THD))
		{
			return 1;
		}
		/* ex_position_da.apb_in */
		if((IS_HIGH_GAIN_APB_IN) && (ex_position_da.apb_in >= SENSOR_PBI_CLEANING_DA_THD))
		{
			return 1;
		}
		/* ex_position_da.apb_out */
		if((IS_HIGH_GAIN_APB_OUT) && (ex_position_da.apb_out >= SENSOR_PBO_CLEANING_DA_THD))
		{
			return 1;
		}
		/* ex_position_da.exit */
		if((IS_HIGH_GAIN_EXIT) && (ex_position_da.exit >= SENSOR_EXT_CLEANING_DA_THD))
		{
			return 1;
		}
		break;
	}
	return 0;
}
u32 is_cis_pga_max()  //現状 特殊 Toolでのみ使用
{
	u32 led;
	float *p_pga;
	switch(ex_sensor_task_mode)
	{
	case SENSOR_MODE_TEMP_ADJ:
	case SENSOR_MODE_TEMP_ADJ_POS_DETECT:
	case SENSOR_MODE_TEMP_ADJ_CIS:
	case SENSOR_MODE_TEMP_ADJ_FINAL:
#if !defined(DEBUG_VALIDATION_DISABLE)
		//TOOD:check s_sens_tempadj_cis[led].backup_pga
		for(led = 0; led < 20; led++)
		{
			/* ratio (2.0f) */
			if(s_sens_tempadj_cis[led].backup_pga > 2.0f)
			{
				return 1;
			}
		}
#endif
		break;
	default:
		//TOOD:ex_cis_adjustment_tmp.cis_pga
		p_pga = (float *)&ex_cis_adjustment_tmp.cis_pga.red_ref_pga_u;
		for(led = 0; led < 20; led++)
		{
			/* ratio (2.0f) */
			if(*p_pga > 2.0f)
			{
				return 1;
			}
			p_pga++;
		}
		break;
	}
	return 0;
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
void _sensor_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_SENSOR_TASK;
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
			_sensor_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_sensor_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _sensor_system_error(u8 fatal_err, u8 code)
{
#ifdef _DEBUG_SYSTEM_ERROR
	//_sensor_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_sensor_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_SENSOR_TASK, (u16)code, (u16)sensor_msg.tmsg_code, (u16)sensor_msg.arg1, fatal_err);
}


//#if defined(ID003_SENSOR)	/* 2021-01-19 */
#if defined(UBA_RTQ) //2025-02-17
u8 check_sensor_level()
{
	/* 3.75倍した後のDAでリミット確認	*/
//	#define	POSI_DA_LIMIT_LEVEL		3616
	#define	POSI_DA_LIMIT_LEVEL		224 //UBA500はMAXの88%を閾値としている

	ex_position_dirt[0] = 0;

	if( ex_position_da.entrance > POSI_DA_LIMIT_LEVEL )
	{
	//High Gainがあるのでこの部分がUBA500と異なる
		if(IS_HIGH_GAIN_ENTRANCE)
		{
			ex_position_dirt[0] = ex_position_dirt[0] | (u8)POSI_ENTRANCE;	/* entrance */
		}
	}

	if( ex_position_da.centering > POSI_DA_LIMIT_LEVEL )
	{
		if(IS_HIGH_GAIN_CENTERING)
		{
			ex_position_dirt[0] = ex_position_dirt[0] | (u8)POSI_CENTERING;	/* centering */
		}
	}

	if( ex_position_da.apb_in > POSI_DA_LIMIT_LEVEL )
	{
		if(SENSOR_ADJ_APB_IN)
		{
			ex_position_dirt[0] = ex_position_dirt[0] | (u8)POSI_APB_IN;		/* apb_in */
		}
	}

	if( ex_position_da.apb_out > POSI_DA_LIMIT_LEVEL )
	{
		if(SENSOR_ADJ_APB_OUT)
		{
			ex_position_dirt[0] = ex_position_dirt[0] | (u8)POSI_APB_OUT;		/* apb_out */
		}
	}

	if( ex_position_da.exit > POSI_DA_LIMIT_LEVEL )
	{
		if(IS_HIGH_GAIN_EXIT)
		{
			ex_position_dirt[0] = ex_position_dirt[0] | (u8)POSI_EXIT;		/* exit */
		}
	}

	//ex_position_dirt_head_check = 1;

}
#endif

//#endif


/* EOF */
