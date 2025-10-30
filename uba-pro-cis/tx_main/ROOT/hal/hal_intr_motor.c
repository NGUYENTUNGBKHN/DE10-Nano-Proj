/******************************************************************************/
/*! @addtogroup Main
    @file       hal_int_motor.c
    @brief      Interrupt functions for PB Home sensor and Centering Home sensor
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
#include "kernel.h"
#include "kernel_inc.h"
#include "js_io.h"
#include "js_oswapi.h"
#include "common.h"
#include "custom.h"
#include "cyc.h"
#include "sensor_ad.h"

#define EXT
#include "com_ram.c"
#include "com_ram_ncache.c"
#include "jsl_ram.c"

/************************** Private Definitions *****************************/

/************************** Function Prototypes ******************************/


/************************** External functions *******************************/

/************************** Variable declaration *****************************/

#if 0//#if(NEW_PB==0) //割り込みは使用しない
void _intr_apb_motor_home(void *arg)
{
	
}
#endif //end NEW_PB


void _intr_pb_encooder(void *arg) //_intr_apb_motor_encoder
{
	// Disable GPIO interrupts
	GpioIsr_disable(__HAL_PB_ENC);
	GpioIsr_clear(__HAL_PB_ENC);

	#if 1 //2022-10-03
	if((_ir_apb_motor_ctrl.init_flag != 0) && (_ir_apb_motor_ctrl.init_value != 0))
	{
		_ir_apb_motor_ctrl.pulse = 0;					// カウンタを必ず先にクリアパルスカウンタクリア
		_ir_apb_motor_ctrl.event_pulse = 0;
		_ir_apb_motor_ctrl.drive_pulse = _ir_apb_motor_ctrl.init_value; 	//設定値になった場合にイベント発生 リミット値設定
		_ir_apb_motor_ctrl.init_flag = 0;
		_ir_apb_motor_ctrl.init_value = 0;
	}
	#endif

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
	GpioIsr_enable(__HAL_PB_ENC);

}


/* EOF */
