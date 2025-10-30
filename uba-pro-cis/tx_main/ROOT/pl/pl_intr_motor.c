/******************************************************************************/
/*! @addtogroup Main
    @file       pl_int_motor.c
    @brief      Interrupt functions for Feed motor encoder and MD-100 encoder
    @date       2018/03/01
    @author     H. Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/01 H.Suzuki
      -# Initial Version
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"

#include "common.h"
//#include "custom.h"

#include "pl_gpio.h"
#include "pl_motor.h"
#include "sensor_ad.h"
#include "cyc.h"	//2023-12-04

#define EXT
#include "com_ram.c"
#include "com_ram_ncache.c"
#include "jsl_ram.c"
#include "cis_ram.c"

/************************** Private Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/
u8 ex_debug_recycler_encoder_pulse;


/*********************************************************************//**
 * @brief		Feed Encoder interrupt
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_intr_feed_encooder(void)
{
#if 1//#if (_DEBUG_DELETE_DISABLE_IRQ==1)
	__disable_irq();
#else
	// Disable GPIO interrupts
	OSW_ISR_disable(FEED_ENC_IRQ_INTR_ID);
#endif
	_ir_feed_motor_ctrl.lock_count = 0;			/* clear motor-lock-count */
	_ir_feed_motor_ctrl.pulse++;			/* pulse count */
	if (_ir_feed_motor_ctrl.drive_pulse > 0)
	{
		if (_ir_feed_motor_ctrl.pulse >= _ir_feed_motor_ctrl.drive_pulse)
		{
			_ir_feed_motor_ctrl.drive_pulse = 0;
	
			_ir_feed_motor_ctrl.over_pulse = 1;
	
			iset_flg(ID_FEED_CTRL_FLAG, EVT_FEED_OVER_PULSE);
		}
	}
	_ir_ad_feed_motor_pulse();

	if(_ir_feed_motor_ctrl.speed_check_time > 1)	//2025-09-24
	{
		_ir_feed_motor_ctrl.speed_check_pulse++;	/* pulse count */
	}

#if 1//#if (_DEBUG_DELETE_DISABLE_IRQ==1)
	__enable_irq();
#else
	// Enable GPIO interrupts
	OSW_ISR_enable(FEED_ENC_IRQ_INTR_ID);
#endif
}

/*********************************************************************//**
 * @brief		Stacker Encoder interrupt
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_intr_stacker_encooder(void)
{

	// Disable GPIO interrupts
	//2024-05-20 OSW_ISR_disable(STACKER_ENC_IRQ_INTR_ID);
	__disable_irq(); //ivizion2はこっちを使用

#if !defined(UBA_RTQ)
	if(SENSOR_STACKER_HOME)
	{
		ex_recovery_info.back_fwd_pulse = 0;
	}
	else if( _ir_stacker_motor_ctrl.mode == MOTOR_FWD )
	{
		ex_recovery_info.back_fwd_pulse++;
		//2023-12-04 ここで押し込みパスルをバックアップする
		if( ex_recovery_info.back_fwd_pulse == 150 ||
 			ex_recovery_info.back_fwd_pulse == 240 ||
			ex_recovery_info.back_fwd_pulse == 520
		)
		{
			iset_flg(ID_FRAM_FLAG, EVT_FRAM_WRITE);
		}
	}
#else
	if( _ir_stacker_motor_ctrl.mode == MOTOR_FWD )
	{
		ex_recovery_info.back_fwd_pulse++;
		//ここで押し込みパスルをバックアップする
		if( ex_recovery_info.back_fwd_pulse == 150 ||
 			ex_recovery_info.back_fwd_pulse == 240 ||
			ex_recovery_info.back_fwd_pulse == 520
		)
		{
			iset_flg(ID_FRAM_FLAG, EVT_FRAM_WRITE);
		}
	}
	else if( _ir_stacker_motor_ctrl.mode == MOTOR_REV )
	{
		if(ex_recovery_info.back_fwd_pulse != 0)
		{
			ex_recovery_info.back_fwd_pulse--;
		}
	}
#endif

	//#if (_DEBUG_DELETE_DISABLE_IRQ==1) //
	if((_ir_stacker_motor_ctrl.init_flag != 0) && (_ir_stacker_motor_ctrl.init_value != 0))
	{
		_ir_stacker_motor_ctrl.pulse = 0;					// カウンタを必ず先にクリアパルスカウンタクリア
		_ir_stacker_motor_ctrl.event_pulse = 0;
		_ir_stacker_motor_ctrl.drive_pulse = _ir_stacker_motor_ctrl.init_value; 	//設定値になった場合にイベント発生 リミット値設定
		_ir_stacker_motor_ctrl.init_flag = 0;
		_ir_stacker_motor_ctrl.init_value = 0;
	}
	//#endif
	_ir_stacker_motor_ctrl.lock_count = 0;			/* clear motor-lock-count */
	_ir_stacker_motor_ctrl.pulse++;			/* pulse count */

	if ( _ir_stacker_motor_ctrl.drive_pulse > 0 )
	{
	// 規定パルス動作したか確認
		// FULL確認用
		if (_ir_stacker_motor_ctrl.full_check)
		{
			if (_ir_stacker_motor_ctrl.pulse == STACKER_FULL_DA )
			{
				_ir_stacker_motor_ctrl.peakload_time = 0;		// 押し終わり付近のタイマ初期化
				_ir_stacker_motor_ctrl.peakload_flag = 1;		// 押し終わり付近の計測有効
			}
			else if (_ir_stacker_motor_ctrl.pulse >= STACKER_TOP_PULSE)
			{
				_ir_stacker_motor_ctrl.peakload_flag = 0;		// 押し終わり付近の計測終了
			}
		}

	#if defined(UBA_RTQ)		/* '21-03-01 */
		if(_ir_stacker_motor_ctrl.mode == MOTOR_FWD)
		{
			_ir_stacker_motor_ctrl.event_pulse_up++;
		}
		else
		{
			_ir_stacker_motor_ctrl.event_pulse_down++;
		}
	#else
		if(!(SENSOR_STACKER_HOME))
		{
		// Homeを外れた
			if(_ir_stacker_motor_ctrl.mode == MOTOR_FWD)
			{
				_ir_stacker_motor_ctrl.event_pulse_up++;
			}
			else
			{
				_ir_stacker_motor_ctrl.event_pulse_down++;
			}
		}
	#endif
		if (_ir_stacker_motor_ctrl.pulse >= _ir_stacker_motor_ctrl.drive_pulse)
		{
		/* 実際のカウント >= limit	*/
			_ir_stacker_motor_ctrl.drive_pulse = 0;
			iset_flg(ID_STACKER_CTRL_FLAG, EVT_STACKER_DRIVE_PULSE_OVER);
		}

		if (_ir_stacker_motor_ctrl.pulse == STACKER_FULL_DA && _ir_stacker_motor_ctrl.mode == MOTOR_FWD )
		{
		/* 実際のカウント >= limit	*/
			_pl_change_staker_da_uba(1); /* DA cahnge*/
		}
	}

	// Enable GPIO interrupts
	//2024-05-20 OSW_ISR_enable(STACKER_ENC_IRQ_INTR_ID);
	__enable_irq();  //ivizion2はこっちを使用

}



//PB encoder
/*********************************************************************//**
 * @brief		Feed Encoder interrupt
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_intr_pb_encooder(void) //_intr_apb_motor_encoder
{

	#if 0	// UBA_WS FGPAの問題のような

	// Disable GPIO interrupts
	OSW_ISR_disable(PB_ENC_IRQ_INTR_ID);

	_ir_apb_motor_ctrl.pulse++;			/* pulse count */
	ex_pb_encoder_count++;

	if (_ir_apb_motor_ctrl.drive_pulse > 0)
	{
		if (_ir_apb_motor_ctrl.pulse >= _ir_apb_motor_ctrl.drive_pulse)
		{
			_ir_apb_motor_ctrl.event_pulse = _ir_apb_motor_ctrl.pulse;
			_ir_apb_motor_ctrl.drive_pulse = 0;
			iset_flg(ID_APB_CTRL_FLAG, EVT_APB_OVER_PULSE);
		}
	}

	// Enable GPIO interrupts
	OSW_ISR_enable(PB_ENC_IRQ_INTR_ID);

	#endif

}





/* EOF */
