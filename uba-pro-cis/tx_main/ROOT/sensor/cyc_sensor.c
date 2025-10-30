/******************************************************************************/
/*! @addtogroup Group2
 @file       cyc_sensor.c
 @brief      sensor process of cycle handler
 @date       2018/02/26
 @author     suzuki-hiroyuki
 @par        Revision
 @par        Copyright (C)
 2018 Japan CashMachine Co, Limited. All rights reserved.
 *******************************************************************************
 @par        History
 - 2018/02/26 Development Dept at Tokyo
 -# Initial Version
 ******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"

#include "common.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

#include "hal_sensor.h"
#include "sensor.h"
#include "cyc.h"

/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/
u16 _cyc_sensor_val[SENSOR_CHATTERING_ELIMINATE];
u16 _cyc_sensor_ctrl_seq;
u16 _cyc_sensor_tick_count;
u16 _cyc_sensor_off_time;
u16 _cyc_sensor_on_time;

typedef struct _cyc_sensor_task_setting
{
	u16 ctrl_seq;
	u16 off_time;
	u16 on_time;
} cyc_sensor_task_setting;
cyc_sensor_task_setting _cyc_sensor_task_setting_value;
/************************** PRIVATE FUNCTIONS *************************/
void _cyc_sampling_position_sensor(void);
u32 _cyc_check_position_sensor(void);
u32 _cyc_1st_check_position_sensor(void);
static void _update_sensor_ctrl_from_task(void);

/************************** EXTERN FUNCTIONS *************************/

/******************************************************************************/
/*! @brief check position sensor procedure
 @return         none
 @exception      none
 ******************************************************************************/
void _cyc_sensor_proc(void)
{
	u32 evt_flag = 0;

	_update_sensor_ctrl_from_task();
	switch (_cyc_sensor_ctrl_seq)
	{
	case 0x4100: /* SENSOR_SEQ_INIT */
	case 0x4200: /* SENSOR_SEQ_LED_ON */
		_hal_position_sensor_on(); /* position & home senosr ON */
		_cyc_sensor_tick_count = 0;		/*サンプリング結果保存バッファアドレス再設定 */
		_cyc_sensor_ctrl_seq++;
		break;
	case 0x4101:
		_hal_position_intr_on();
		_cyc_sampling_position_sensor();
		_cyc_sensor_ctrl_seq++;
		_cyc_sensor_tick_count++;	/*サンプリング結果保存バッファアドレス+1*/
		break;
	case 0x4102:
		_cyc_sampling_position_sensor();
		_cyc_sensor_tick_count++;	/*サンプリング結果保存バッファアドレス+1*/

		if (_cyc_sensor_tick_count >= SENSOR_CHATTERING_ELIMINATE) {
			_cyc_sensor_tick_count = 0;			/*サンプリング結果保存バッファアドレス再設定 */
			evt_flag = _cyc_check_position_sensor();
			evt_flag |= EVT_SENSOR_INIT;
			iset_flg(ID_SENSOR_FLAG, evt_flag); /* set flag. */
			_cyc_sensor_ctrl_seq = 0x4202;
		}
		break;
	/* 常時点灯処理 */
	case 0x4201:
		_hal_position_intr_on();
		_cyc_sampling_position_sensor();
		_cyc_sensor_ctrl_seq++;
		_cyc_sensor_tick_count++;	/*サンプリング結果保存バッファアドレス+1*/
		break;

	case 0x4202:
		_cyc_sampling_position_sensor();
		_cyc_sensor_tick_count++;	/*サンプリング結果保存バッファアドレス+1*/
		if (_cyc_sensor_tick_count >= SENSOR_CHATTERING_ELIMINATE)
		{
			_cyc_sensor_tick_count = 0;			/*サンプリング結果保存バッファアドレス再設定 */
			evt_flag = _cyc_check_position_sensor();
			if (evt_flag) {
				iset_flg(ID_SENSOR_FLAG, evt_flag); /* set flag. */
				iset_flg(ID_FEED_CTRL_FLAG, EVT_FEED_SENSOR);
				iset_flg(ID_STACKER_CTRL_FLAG, EVT_STACKER_SENSOR);
				iset_flg(ID_CENTERING_CTRL_FLAG, EVT_CENTERING_SENSOR);
				iset_flg(ID_APB_CTRL_FLAG, EVT_APB_SENSOR);
				iset_flg(ID_SHUTTER_CTRL_FLAG, EVT_SHUTTER_SENSOR);			
			}
		}
		break;
	/* 点滅処理 */
	case 0x4400: /* SENSOR_SEQ_LED_BLINK */
		_hal_position_sensor_off(); /* position & home senosr OFF */
		_cyc_sensor_tick_count = 0;	/*サンプリング結果保存バッファアドレス再設定 */
		_cyc_sensor_ctrl_seq++;
		break;
	case 0x4401:
		_cyc_sensor_tick_count++;	/*サンプリング結果保存バッファアドレス+1*/
		if (_cyc_sensor_tick_count >= _cyc_sensor_off_time) {
			_hal_position_sensor_on(); /* position & home senosr ON */
			_cyc_sensor_tick_count = 0;	/*サンプリング結果保存バッファアドレス再設定 */
			_cyc_sensor_ctrl_seq++;
		}
		break;
	case 0x4402:
		_hal_position_intr_on();
		_cyc_sampling_position_sensor();	//1回目
		_cyc_sensor_tick_count++;	/*サンプリング結果保存バッファアドレス+1*/
		if (_cyc_1st_check_position_sensor())
		{
			// next:final check
			_cyc_sensor_ctrl_seq++;
		}
		else
		{
			_hal_position_sensor_off(); /* position & home senosr OFF */
			_cyc_sensor_tick_count = 0;	/*サンプリング結果保存バッファアドレス再設定 */
			_cyc_sensor_ctrl_seq = 0x4401;
		}
		break;
	case 0x4403:
		_cyc_sampling_position_sensor();	//2回目
		//2023-12-05 この処理いらない、どうせ次に0にする _cyc_sensor_tick_count++;	/*サンプリング結果保存バッファアドレス+1*/
		evt_flag = _cyc_check_position_sensor();
		_hal_position_sensor_off(); /* position & home senosr OFF */
		_cyc_sensor_tick_count = 0;	/*サンプリング結果保存バッファアドレス再設定 */
		_cyc_sensor_ctrl_seq = 0x4401;
		if (evt_flag) {
			iset_flg(ID_SENSOR_FLAG, evt_flag); /* set flag. */
		}
		break;

	case 0x4600: /* SENSOR_SEQ_POSI_ADJ */
	case 0x4700: /* SENSOR_SEQ_POSI_ADJ_DTCT */
		_hal_position_sensor_on(); /* position & home senosr ON */
		_cyc_sensor_tick_count = 0;	/*サンプリング結果保存バッファアドレス再設定 */
		_cyc_sensor_ctrl_seq++;
		break;
	case 0x4601:
	case 0x4701:
		_hal_position_intr_on();
		_cyc_sampling_position_sensor();
		_cyc_sensor_tick_count++;	/*サンプリング結果保存バッファアドレス+1*/
		_cyc_sensor_ctrl_seq++;
		break;
	case 0x4602:
		_cyc_sampling_position_sensor();
		_cyc_sensor_tick_count++;	/*サンプリング結果保存バッファアドレス+1*/

		if (_cyc_sensor_tick_count >= SENSOR_CHATTERING_ELIMINATE) {
			_cyc_sensor_ctrl_seq++;
			_cyc_sensor_tick_count = 0;		/*サンプリング結果保存バッファアドレス再設定 */
			ex_adjust_position_sensor = _cyc_sensor_val[SENSOR_CHATTERING_ELIMINATE - 1];
			evt_flag |= EVT_SENSOR_POSITION_AD;
			if (evt_flag) {
				iset_flg(ID_SENSOR_FLAG, evt_flag); /* set flag. */
			}
		}
		break;
	case 0x4603:
		// only wait
		break;
	case 0x4702:
		_cyc_sampling_position_sensor();
		evt_flag = _cyc_check_position_sensor();
		_cyc_sensor_val[0] = _cyc_sensor_val[1];
		_cyc_sensor_tick_count = 1;

		evt_flag |= EVT_SENSOR_POSITION_AD;
		if (evt_flag) {
			iset_flg(ID_SENSOR_FLAG, evt_flag); /* set flag. */
		}
		break;
	default:
		break;
	}
}

void _cyc_sampling_position_sensor(void)
{
	if(_cyc_sensor_tick_count < SENSOR_CHATTERING_ELIMINATE)
	{
		_cyc_sensor_val[_cyc_sensor_tick_count] = _hal_sen_pos_sensor_read();
	}
	return;
}


u32 _cyc_check_position_sensor(void)
{
	u32 evt_flag = 0;
	u16 sensor_val;
	u8 cnt;
	u8 ent_cnt = 0;
	u8 cent_cnt = 0;
	u8 apb_ent_cnt = 0;
	u8 apb_exit_cnt = 0;
	u8 exit_cnt = 0;
	u8 box_home_cnt = 0;
	u8 apb_home_cnt = 0;
	u8 cent_home_cnt = 0;
	u16 centering_sensor = 0;
	u8 shutter_cnt = 0;
	u16 sensor_val_a = 0;
	u16 sensor_val_b = 0;
	u8 box1_cnt = 0;

	for (cnt = 0; cnt < SENSOR_CHATTERING_ELIMINATE; cnt++) {
		sensor_val = _cyc_sensor_val[cnt];

		if ((sensor_val & POSI_ENTRANCE)
				!= (ex_position_sensor & POSI_ENTRANCE)) {
			ent_cnt++;
		}
		if ((sensor_val & POSI_CENTERING)
				!= (ex_position_sensor & POSI_CENTERING)) {
			cent_cnt++;
		}
		if ((sensor_val & POSI_APB_IN) != (ex_position_sensor & POSI_APB_IN)) {
			apb_ent_cnt++;
		}
		if ((sensor_val & POSI_APB_OUT)
				!= (ex_position_sensor & POSI_APB_OUT)) {
			apb_exit_cnt++;
		}
		if ((sensor_val & POSI_BOX_HOM)		/* 2022-01-13 */
				!= (ex_position_sensor & POSI_BOX_HOM)) {
			box_home_cnt++;
		}
		if ((sensor_val & POSI_EXIT) != (ex_position_sensor & POSI_EXIT)) {
			exit_cnt++;
		}
		if ((sensor_val & POSI_APB_HOME)
				!= (ex_position_sensor & POSI_APB_HOME)) {
			apb_home_cnt++;
		}
		if ((sensor_val & POSI_CENTERING_HOME)
				!= (ex_position_sensor & POSI_CENTERING_HOME)) {
			cent_home_cnt++;
		}
		if ((sensor_val & POSI_SHUTTER_OPEN)
				!= (ex_position_sensor & POSI_SHUTTER_OPEN)) {
			shutter_cnt++;
		}
		if ((sensor_val & POSI_500BOX)		/* Boxありセンサ */
				!= (ex_position_sensor & POSI_500BOX)) {
			box1_cnt++;
		}
	}

	if (ent_cnt == SENSOR_CHATTERING_ELIMINATE) {
		ex_position_sensor = (ex_position_sensor ^ POSI_ENTRANCE);
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if (cent_cnt == SENSOR_CHATTERING_ELIMINATE) {
		ex_position_sensor = (ex_position_sensor ^ POSI_CENTERING);
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if (apb_ent_cnt == SENSOR_CHATTERING_ELIMINATE) {
		ex_position_sensor = (ex_position_sensor ^ POSI_APB_IN);
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if (apb_exit_cnt == SENSOR_CHATTERING_ELIMINATE) {
		ex_position_sensor = (ex_position_sensor ^ POSI_APB_OUT);
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if (box_home_cnt == SENSOR_CHATTERING_ELIMINATE) {
		ex_position_sensor = (ex_position_sensor ^ POSI_BOX_HOM);
		evt_flag |= EVT_SENSOR_SHIFT;
	}

	if (exit_cnt == SENSOR_CHATTERING_ELIMINATE) {
		ex_position_sensor = (ex_position_sensor ^ POSI_EXIT);
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if (apb_home_cnt == SENSOR_CHATTERING_ELIMINATE) {
		ex_position_sensor = (ex_position_sensor ^ POSI_APB_HOME);
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if (cent_home_cnt == SENSOR_CHATTERING_ELIMINATE) {
		ex_position_sensor = (ex_position_sensor ^ POSI_CENTERING_HOME);
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if (shutter_cnt == SENSOR_CHATTERING_ELIMINATE) {
		ex_position_sensor = (ex_position_sensor ^ POSI_SHUTTER_OPEN);
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if (box1_cnt == SENSOR_CHATTERING_ELIMINATE) {	/* Boxありセンサ */
		ex_position_sensor = (ex_position_sensor ^ POSI_500BOX);
		evt_flag |= EVT_SENSOR_SHIFT;
	}

	/* Centering Home */ 
	sensor_val_a = (_cyc_sensor_val[0] & POSI_CENTERING_HOME); /* 幅よせのみ確認 */
	sensor_val_b = (_cyc_sensor_val[1] & POSI_CENTERING_HOME); /* 幅よせのみ確認 */

	if(sensor_val_a == sensor_val_b)
	{
		/* 2回ともデータが同じ データが安定している */
		if(sensor_val_a)
		{
			centering_sensor = 1;	/* Home */
			ex_centor_home_out = 0;	/* Home */
		}
		else
		{
			centering_sensor = 0;	/* Home out */
			ex_centor_home_out = 1;	/* Home out */
		}
	}

#if 1	//2024-02-14
	if(ex_validation_sensor == (VALIDATION_1ST_SENSOR | VALIDATION_2ED_SENSOR))
	{
		ex_position_sensor |= POSI_VALIDATION;
	}
	else
	{
		ex_position_sensor &= ~POSI_VALIDATION;
	}
#endif

#if defined(UBA_RTQ)
	/* stacker home detect *///2025-09-11
	if((ex_line_stacker_home == 0 && ex_rc_status.sst1B.bit.stacker_home == 1)
	|| (ex_line_stacker_home == 1 && ex_rc_status.sst1B.bit.stacker_home == 0))
	{
		/* change sensor status */
		evt_flag |= EVT_SENSOR_SHIFT;	//UBA500は使用していないが、DIP-SWでのセンサテストの時、Stacker homeの状態通知は、これを使用しない場合は、タイマ作成など改善が必要
	}

	if(ex_rc_status.sst1B.bit.stacker_home == 1)
	{
		ex_line_stacker_home = 1;	/* stacker home position		*/
	}
	else
	{
		ex_line_stacker_home = 0;	/* stacker home out position	*/
	}

	 //2024-07-16
	/* box detect */
	if((ex_line_box_detect == 0 && ex_rc_status.sst1B.bit.box_detect == 1)
	|| (ex_line_box_detect == 1 && ex_rc_status.sst1B.bit.box_detect == 0))
	{
		/* change sensor status */
		evt_flag |= EVT_SENSOR_SHIFT;
	}

	if(ex_rc_status.sst1B.bit.box_detect == 1)
	{
		ex_line_box_detect = 1;		/* box detect		*/
	}
	else
	{
		ex_line_box_detect = 0;		/* box not detect	*/
	}
#endif

	return evt_flag;
}

u32 _cyc_1st_check_position_sensor(void) //待機時の点灯処理で、点灯を継続するか消灯するかの分岐
{
	u32 evt_flag = 0;
	u16 sensor_val;
	sensor_val = _cyc_sensor_val[0];
	if ((sensor_val & POSI_ENTRANCE)
			!= (ex_position_sensor & POSI_ENTRANCE)) {
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if ((sensor_val & POSI_CENTERING)
			!= (ex_position_sensor & POSI_CENTERING)) {
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if ((sensor_val & POSI_APB_IN) != (ex_position_sensor & POSI_APB_IN)) {
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if ((sensor_val & POSI_APB_OUT)
			!= (ex_position_sensor & POSI_APB_OUT)) {
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if ((sensor_val & POSI_EXIT) != (ex_position_sensor & POSI_EXIT)) {
		evt_flag |= EVT_SENSOR_SHIFT;
	}

	if ((sensor_val & POSI_BOX_HOM)		/* 2022-01-13 */
			!= (ex_position_sensor & POSI_BOX_HOM)) {
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if ((sensor_val & POSI_APB_HOME)
			!= (ex_position_sensor & POSI_APB_HOME)) {
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if ((sensor_val & POSI_CENTERING_HOME)
			!= (ex_position_sensor & POSI_CENTERING_HOME)) {
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if ((sensor_val & POSI_SHUTTER_OPEN)
			!= (ex_position_sensor & POSI_SHUTTER_OPEN)) {
		evt_flag |= EVT_SENSOR_SHIFT;
	}
	if ((sensor_val & POSI_500BOX)
			!= (ex_position_sensor & POSI_500BOX)) {
		evt_flag |= EVT_SENSOR_SHIFT;
	}

#if 1	//2024-02-14
	if(ex_validation_sensor == (VALIDATION_1ST_SENSOR | VALIDATION_2ED_SENSOR))
	{
		ex_position_sensor |= POSI_VALIDATION;
	}
	else
	{
		ex_position_sensor &= ~POSI_VALIDATION;
	}
#endif

#if defined(UBA_RTQ)
	/* stacker home detect *///2025-09-11
	if((ex_line_stacker_home == 0 && ex_rc_status.sst1B.bit.stacker_home == 1)
	|| (ex_line_stacker_home == 1 && ex_rc_status.sst1B.bit.stacker_home == 0))
	{
		/* change sensor status */
		evt_flag |= EVT_SENSOR_SHIFT;
	}

	/* box detect */ //2024-07-16
	if((ex_line_box_detect == 0 && ex_rc_status.sst1B.bit.box_detect == 1)
	|| (ex_line_box_detect == 1 && ex_rc_status.sst1B.bit.box_detect == 0))
	{
		/* この関数では、戻り値でイベントを発生させているのではなく、点灯の継続かの判断のみ*/
		/* イベント処理は_cyc_check_position_sensor(void) */
		/* change sensor status */
		evt_flag |= EVT_SENSOR_SHIFT;
	}
#endif

	return evt_flag;
}

static void _update_sensor_ctrl_from_task(void)
{
	switch(_cyc_sensor_task_setting_value.ctrl_seq)
	{
	case SENSOR_SEQ_INIT:
	case SENSOR_SEQ_LED_ON:
	case SENSOR_SEQ_LED_BLINK:
	case SENSOR_SEQ_POSI_ADJ:
	case SENSOR_SEQ_POSI_ADJ_DETECT:
		_cyc_sensor_ctrl_seq = _cyc_sensor_task_setting_value.ctrl_seq;
		_cyc_sensor_off_time = _cyc_sensor_task_setting_value.off_time;
		_cyc_sensor_on_time = _cyc_sensor_task_setting_value.on_time;
		memset(&_cyc_sensor_task_setting_value, 0, sizeof(_cyc_sensor_task_setting_value));
		break;
	default:
		break;
	}
}
void _sensor_ctrl_config(u16 seq_no, u16 off_time, u16 on_time)
{
	if (seq_no == SENSOR_SEQ_LED_BLINK)
	{
		_cyc_sensor_task_setting_value.off_time = off_time;
		if ((off_time != 0) && (on_time < SENSOR_POSITION_RISE_TIME)) {
			_cyc_sensor_task_setting_value.on_time = SENSOR_POSITION_RISE_TIME;
		} else {
			_cyc_sensor_task_setting_value.on_time = on_time;
		}
		_cyc_sensor_task_setting_value.ctrl_seq = seq_no;
	}
	else
	{
		_cyc_sensor_task_setting_value.off_time = off_time;
		_cyc_sensor_task_setting_value.on_time = on_time;
		_cyc_sensor_task_setting_value.ctrl_seq = seq_no;
	}
}
/* EOF */
