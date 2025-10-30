/******************************************************************************/
/*! @addtogroup Group1
    @file       sensor_ad.c
    @brief      A/D cycle routine
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
#include "kernel.h"
#include "itron.h"
#include "common.h"
#include "custom.h"
#include <string.h>

#include "alt_interrupt.h"
#include "cyc.h"
#include "operation.h"
#include "sub_functions.h"
#include "sensor.h"
#include "hal_sensor.h"
#include "hal.h"
#include "pl/pl.h"
#include "pl/pl_cis.h"
#include "pl/pl_gpio.h"
#include "alt_cache.h"

#define EXT
#include "../common/global.h"
#include "com_ram.c"
#include "com_ram_ncache.c"
#include "jsl_ram.c"
#include "cis_ram.c"

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE DEFINITIONS ***********************/
/************************** Function Prototypes ***********************/
void start_ad(void);
void stop_ad(void);

/************************** EXTERNAL FUNCTIONS ***********************/
extern void _kernel_synch_cache(void);
/************************** Variable declaration **********************/

void start_ad(void)
{
#if BANKNOTE_EDGE_SKEW_ENABLE || BANKNOTE_EDGE_LENGTH_ENABLE
	/* clear pending interrupts */
	alt_int_dist_pending_clear((ALT_INT_INTERRUPT_t)FPGA_SCANBIN_IRQ_INTR_ID);
	OSW_ISR_enable(FPGA_SCANBIN_IRQ_INTR_ID);
#endif
#if BANKNOTE_MLT_PAPER_ENABLE
	/* clear pending interrupts */
	alt_int_dist_pending_clear(FPGA_SCANMLT_IRQ_INTR_ID);
	OSW_ISR_enable(FPGA_SCANMLT_IRQ_INTR_ID);
#endif
	/* DONE: check PL enable or not */
	if(get_pl_state())
	{
		_pl_cis_scanst(1);
	}





#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ex_cis_image_control.image_status[ex_cis_image_control.current] = CIS_IMAGE_USED;
#endif
	// validation memory clear
	ex_validation.reject_code = REJECT_CODE_OK;
	memset(&ex_validation, 0, sizeof(BV_MEMORY));
}

void stop_ad(void)
{
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS* pbill_data = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif

	/* DONE: check PL enable or not */
	if(get_pl_state())
	{
		pbill_data->block_count = CAP_INFO_ARRAY[0].CAPST.BIT.BLK;		// 有効ブロック数 (採取済みブロック数 - 1) - 10
		_pl_cis_scanst(0);
		_pl_cis_scan_enable(0);

		//set debug data
		pbill_data->st_model_area.cap_info_array_base = CAP_INFO_ARRAY[0].BASE;
		pbill_data->st_model_area.cap_info_array_set = CAP_INFO_ARRAY[0].SET.LWORD;
		pbill_data->st_model_area.cap_info_array_blid = CAP_INFO_ARRAY[0].BLID.LWORD;
		pbill_data->st_model_area.cap_info_array_capst = CAP_INFO_ARRAY[0].CAPST.LWORD;
	}
#if BANKNOTE_CYCLIC_ENABLE
	//サイクリックデータずれ修正処理用
	pbill_data->st_model_area.cisdt_set_prebk = (u8)(FPGA_REG.CISDT_SET.BIT.PREBK);
	pbill_data->st_model_area.cisdt_st_oldestbk = (u8)(FPGA_REG.CISDT_ST.BIT.OLDESTBK);
#endif

	OSW_ISR_disable(FPGA_SCANBIN_IRQ_INTR_ID);
	OSW_ISR_disable(FPGA_SCANMLT_IRQ_INTR_ID);
	alt_cache_l1_data_clean_all();
    _kernel_synch_cache();
}


/******************************************************************************/
/*! @brief Count up feed motor encoder pulse
    @param       	none
    @par            Refer
    @par            Modify
    - 変更するグローバル変数 ex_total_pulse_count
    @return        	none
    @exception      none
******************************************************************************/
void _ir_ad_feed_motor_pulse(void)
{
	if ((_ir_feed_motor_ctrl.mode == MOTOR_FWD)
	 || (_ir_feed_motor_ctrl.mode == MOTOR_BRAKE_FWD)
	) {
		ex_feed_pulse_count.fwd++;
	}
	else if ((_ir_feed_motor_ctrl.mode == MOTOR_REV)
	 || (_ir_feed_motor_ctrl.mode == MOTOR_BRAKE_REV)
	) {
		ex_feed_pulse_count.rev++;
	}
	if(ex_feed_pulse_count.total < 0xffff)
	{
		ex_feed_pulse_count.total++;
	}
	if(SENSOR_ENTRANCE)
	{
		if(ex_position_pulse_count.entrance.start == 0)
		{
			ex_position_pulse_count.entrance.start = ex_feed_pulse_count.total;
		}
		ex_position_pulse_count.entrance.end = ex_feed_pulse_count.total;
	}
	if(SENSOR_CENTERING)
	{
		if(ex_position_pulse_count.centering.start == 0)
		{
			ex_position_pulse_count.centering.start = ex_feed_pulse_count.total;
		}
		ex_position_pulse_count.centering.end = ex_feed_pulse_count.total;
	}
	if(SENSOR_APB_IN)
	{
		if(ex_position_pulse_count.apb_in.start == 0)
		{
			ex_position_pulse_count.apb_in.start = ex_feed_pulse_count.total;
		}
		ex_position_pulse_count.apb_in.end = ex_feed_pulse_count.total;
	}
	if(SENSOR_APB_OUT)
	{
		if(ex_position_pulse_count.apb_out.start == 0)
		{
			ex_position_pulse_count.apb_out.start = ex_feed_pulse_count.total;
		}
		ex_position_pulse_count.apb_out.end = ex_feed_pulse_count.total;
	}
	if(SENSOR_EXIT)
	{
		if(ex_position_pulse_count.exit.start == 0)
		{
			ex_position_pulse_count.exit.start = ex_feed_pulse_count.total;
		}
		ex_position_pulse_count.exit.end = ex_feed_pulse_count.total;
	}
}


void change_ad_sampling_mode(u16 mode)
{
	ex_ad_sequence_mode = mode;

	switch(mode)
	{
	case AD_MODE_IDLE:
		/*2019-07-05 add*/
		// Scan flag init
		_pl_cis_scan_enable(0);
		// scanst init
		_pl_cis_scanst(0);
		/* mdst(sr register) initialize */
		_pl_cis_init_fpga();
		//1 紙幣サンプリング用シーケンス情報初期化
		_pl_sampling_cis_seq();
	#if POINT_UV1_ENABLE || POINT_UV2_ENABLE
		if(ex_uba710 == 1)
		{
			// UV DA,GAINセット
			set_uv_adj();
		}
	#endif
	#if MAG1_ENABLE
		if(ex_uba710 == 1)
		{
			set_mag_gain();
		}
	#endif
		// 紙幣サンプリング用その他シーケンス情報初期化
		_pl_sampling_ss_seq();
		_pl_sampling_regist_data();
		/* afe initialize */
		_pl_cis_init_afe();
		alt_cache_l1_data_clean_all();
	    _kernel_synch_cache();
		break;
	case AD_MODE_BILL_IN:

	//#if (_DEBUG_CIS_AS_A_POSITION==1) //2024-02-15 連続取り込みの場合もあるので、これは必要 
		//連続取り込みの場合もあるので、これは必要
		_validation_ctrl_set_mode(VALIDATION_CHECK_MODE_DISABLE);
		dly_tsk(5);
	//#endif
		// Scan flag init
		_pl_cis_scan_enable(0);
		// scanst init
		_pl_cis_scanst(0);

		/* (fpga register, set BC WC) initialize */
		_pl_cis_init_fpga();

		_pl_cis_init_ad(CIS_MODE_ADJ_PAPER);
		//1 紙幣サンプリング用シーケンス情報初期化
		_pl_sampling_cis_seq();
	#if POINT_UV1_ENABLE || POINT_UV2_ENABLE
		if(ex_uba710 == 1)
		{
		// UV DA,GAINセット
			set_uv_adj();
		}
	#endif
	#if MAG1_ENABLE
		if(ex_uba710 == 1)
		{
			set_mag_gain();
		}
	#endif
		// 紙幣サンプリング用その他シーケンス情報初期化
		_pl_sampling_ss_seq();
		_pl_sampling_regist_data();
		/* afe initialize */
		_pl_cis_init_afe();
		alt_cache_l1_data_clean_all();
	    _kernel_synch_cache();
		_pl_cis_scan_enable(1);

		#if MAG1_ENABLE	//2022-06-29
		if(ex_uba710 == 1)
		{
			_hal_i2c3_write_mag_cntl(1);
		}
		#endif

		break;
	case AD_MODE_PAY_OUT:
		// Scan flag init
		_pl_cis_scan_enable(0);
		// scanst init
		_pl_cis_scanst(0);

		_pl_cis_init_ad(CIS_MODE_ADJ_PAPER);
		//1 紙幣サンプリング用シーケンス情報初期化
		_pl_sampling_cis_seq();
	#if POINT_UV1_ENABLE || POINT_UV2_ENABLE
		if(ex_uba710 == 1)
		{
			// UV DA,GAINセット
			set_uv_adj();	
		}
	#endif

	#if MAG1_ENABLE
		if(ex_uba710 == 1)
		{
			set_mag_gain();
		}
	#endif
		// 紙幣サンプリング用その他シーケンス情報初期化
		_pl_sampling_ss_seq();
		_pl_sampling_regist_data();
		/* afe initialize */
		_pl_cis_init_afe();
	    _kernel_synch_cache();
		_pl_cis_scan_enable(1);
		break;
	case AD_MODE_ONE_SHOT:
		break;

	case AD_MODE_MAG_OFF:
	#if MAG1_ENABLE	//2022-06-29
		if(ex_uba710 == 1)
		{
			_hal_i2c3_write_mag_cntl(0);
		}
	#endif
		break;

#if (_DEBUG_EMI_IMAGE_CHECK==1)
	case AD_MODE_AGING:
		// Scan flag init
		_pl_cis_scan_enable(0);
		// scanst init
		_pl_cis_scanst(0);

		/* (fpga register, set BC WC) initialize */
		_pl_cis_init_fpga();

		_pl_cis_init_ad(CIS_MODE_ADJ_PAPER);
		//1 紙幣サンプリング用シーケンス情報初期化
		_pl_sampling_cis_seq();
	#if POINT_UV1_ENABLE || POINT_UV2_ENABLE
		if(ex_uba710 == 1)
		{
		// UV DA,GAINセット
			set_uv_adj();
		}
	#endif
	#if MAG1_ENABLE
		if(ex_uba710 == 1)
		{
			set_mag_gain();
		}
	#endif
		// 紙幣サンプリング用その他シーケンス情報初期化
		_pl_sampling_ss_seq();
		_pl_sampling_regist_data();
		_pl_sampling_cyclic_off();
		/* afe initialize */
		_pl_cis_init_afe();
		alt_cache_l1_data_clean_all();
	    _kernel_synch_cache();
		_pl_cis_scan_enable(1);

		#if MAG1_ENABLE	//2022-06-29
		if(ex_uba710 == 1)
		{
			_hal_i2c3_write_mag_cntl(1);
		}
		#endif

		break;
#endif
	case AD_MODE_VALIDATION_CHECK:
	//#if (_DEBUG_CIS_AS_A_POSITION==1)
		// Scan flag init
		_pl_cis_scan_enable(0);
		// scanst init
		_pl_cis_scanst(0);
		_pl_cis_init_ad(CIS_MODE_VALIDATION_CHECK);
		_pl_validation_sensor_seq();
		_pl_validation_sensor_ss_seq();
		_pl_validation_sensor_regist_data();

		_pl_cis_init_afe(); //2024-02-26

		alt_cache_l1_data_clean_all();
		_kernel_synch_cache();
		_pl_cis_scan_enable(1);
	//#endif
		break;
	default:
		/* error:reset */
		program_error();
	}
}


/******************************************************************************/
/*! @brief check A/D sampling finish.
    @par            Refer
    - 参照するグローバル変数 ex_ad_save_flag
    @par            Modify
    - 変更するグローバル変数 none
	@arg            none
    @return         0~		: a/d sampling is finished
					0		: a/d sampling is not finished
    @exception      none
******************************************************************************/
u8 check_ad_sampling_complete(void)
{
	u8 result;
	// TODOP: check A/D sampling finish.
	result = 0;

	return result;
}
/* EOF */
