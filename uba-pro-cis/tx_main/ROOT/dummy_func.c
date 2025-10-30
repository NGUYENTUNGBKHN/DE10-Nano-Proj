/* dummy temp func */
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "custom.h"
#include "sub_functions.h"
#include "./pl/pl.h"
#include "./pl/pl_encoder.h"
#include "./pl/pl_motor.h"
#include "hal.h"
#include "hal_sensor.h"
#include "hal_clk.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "usb_ram.c"
#include "cis_ram.c"


/****************************************************************/
/**
 * @brief メインメッセージ処理
 */
/****************************************************************/
void on_rcv_main_msg_proc(void)
{
	// TODO:implement
}
/* pl_adc.c */
u16 _pl_sen_mag_adj_ref_ad_read(void)
{
	return 0;
}
/* id003.c*/
void _id003_dummy_cmd_proc(void)
{
	return;
};
/* sub_function.c */
void terminate_main_sys(void)
{
	ER er;
	// DISABLE TASK DISPATCH
	dis_dsp();
	er = ter_tsk(ID_DISPLAY_TASK);
	if(er != E_OK)
	{
		program_error();
	}
	er = ter_tsk(ID_BEZEL_TASK);
	if(er != E_OK)
	{
		program_error();
	}
	er = ter_tsk(ID_DIPSW_TASK);
	if(er != E_OK)
	{
		program_error();
	}

	// if ENABLE PL
	if(get_pl_state())
	{
		/* FEED LED OFF */
		_pl_sen_feed_encoder_LED_set(0);
		/* PL ADC PowerDown */
		/* Feed Motor 初期化 */
		_pl_motor_final();
		/* FPGAをStandbyに設定 */
		enable_pl(0);
	}
	/* Stop OS timer */
	// DISABLE ALL INTERRUPT
	for(int i = 0; i < OSW_INT_NUM; i++)
	{
		OSW_ISR_disable( i );
	}
	// BEZEL LED OFF
	// LED OFF
	_hal_position_sensor_off(); /* position & home senosr OFF */

	/* Disable GPIO and peripheral */

	// USB Pull-UP Down
	/* Pullup OFF */
	GRUSB_DEV_ApReqPullupRegister( GRUSB_FALSE );
	GRUSB_DEV_ApReqPullupRegister2( GRUSB_FALSE );

}

#if (_DEBUG_EMI_IMAGE_CHECK==1)
#define EXT
#include "../common/global.h"
u32 emi_image_check_black_line(void)
{
	u32 er = RET_OK;
	int cnt,offset;
	unsigned char* data_pointer = NULL;
	unsigned char* block_data = NULL;
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *p_data_collection = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS *p_data_collection = (ST_BS *)BILL_NOTE_IMAGE_TOP;
#endif
	data_pointer = (unsigned char*)(&block_data[CISB_T_G_OFFSET]);
	// black line
	// 外形検知の閾値は255
	// このチェックは254
	for(cnt = 0;cnt < p_data_collection->block_count; cnt++)
	{
		block_data = &p_data_collection->sens_dt[cnt * BLOCK_BYTE_SIZE];
		data_pointer = (unsigned char*)(&block_data[CISB_T_G_OFFSET]);
		for(offset = 1; offset < 7; offset++)
		{
			if(*(data_pointer + (offset * 100)) < 254)
			{
				er = RET_NG;
			}
		}
		data_pointer += PERIOD_100DPI;
		for(offset = 1; offset < 7; offset++)
		{
			if(*(data_pointer + (offset * 100)) < 254)
			{
				er = RET_NG;
			}
		}
	}

	return er;
}
#endif
#if (_DEBUG_EMI_IMAGE_CHECK_TEST==1)
void emi_image_set_black_line(void)
{
	int cnt;
	unsigned char* data_pointer = NULL;
	unsigned char* block_data = NULL;
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *p_data_collection = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS *p_data_collection = (ST_BS *)BILL_NOTE_IMAGE_TOP;
#endif
	block_data = &p_data_collection->sens_dt[p_data_collection->block_count/2 * BLOCK_BYTE_SIZE];
	data_pointer = (unsigned char*)(&block_data[CISA_R_R_OFFSET]);
	// white line
	for(cnt = 20;cnt < 728 - 20; cnt++)
	{
		*(data_pointer + cnt) = 0xff;
	}
	data_pointer = (unsigned char*)(&block_data[CISB_T_G_OFFSET]);
	// black line
	for(cnt = 20;cnt < 728 - 20; cnt++)
	{
		*(data_pointer + cnt) = 0x0;
	}
}
#endif
/* EOF */
