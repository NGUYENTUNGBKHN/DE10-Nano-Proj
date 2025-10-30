/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
 * @file main_task.c
 * @brief メインタスクを格納しています。
 * @date 2018.01.05
 */
/****************************************************************************/
#include <string.h>
#include "systemdef.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "alt_cache.h"
#include "memorymap.h"
#include "operation.h"
#include "sub_functions.h"
#include "sensor.h"
#include "sensor_ad.h"
#include "status_tbl.h"
#include "motor_ctrl.h"
#include "hal.h"
#include "hal_clk.h"
#include "hal_i2c_iox.h"
#include "pl/pl.h"
#include "pl/pl_evrec.h"

#include "jdl_conf.h"
#include "jdl.h"

#define EXT
#include "../common/global.h"
#include "com_ram.c"
#include "com_ram_ncache.c"
#include "cis_ram.c"

#include "jsl_ram.c"	//2025-09-26

#undef EXTERN
#define EXTERN
#include "main_task.h"

#include "hal_spi_fram.h"


/************************** Function Prototypes ******************************/
void _main_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _main_system_error(u8 fatal_err, u8 code);
void _main_init_var(void);
void _main_start_cyc(void);
void _main_act_peripheral_task(void);
void operation_main(void);
extern void usb_operation_main(void);


/************************** Variable declaration *****************************/
T_MSG_BASIC ex_main_msg;
/*******************************
        activate task
 *******************************/

void _undef(void){program_error();};
void _prefetch(void){program_error();};
void _abort(void){program_error();};

void cyc_timer_clock(void)//2025-09-26 1ms周期 タイマ作成
{
	int32_t clock = 0;
	int8_t prescaler = 0;

	// get actual frequency from register
	alt_clk_freq_get(ALT_CLK_MPU_PERIPH, (alt_freq_t *)&clock);
	prescaler = 8;
	clock /= prescaler*1000;		/* mpu_periph/prescaler -> 1000hz */

	if (0 != clock) {
		clock--;
	}

	IOREG32(PTIME_BASE, PTIM_PRIVATE_TIMERLOAD) = clock;
	IOREG32(PTIME_BASE, PTIM_PRIVATE_TIMERINTSTAT) = 0x1;
	//Bit[0] = 1 → タイマ有効, Bit[1] = 1 → 自動リロード, Bit[2] = 1 → 割り込み有効 
	IOREG32(PTIME_BASE, PTIM_PRIVATE_TIMERCTRL) = 0x7 | ((prescaler - 1) << 8); // enable, auto-reload, irq enable
}

static void osw_timer_isr(void)	//2025-09-26 1ms周期 割り込み
{
	IOREG32(PTIME_BASE, PTIM_PRIVATE_TIMERINTSTAT) = 0x1; // 割り込みフラグクリア

	if (_ir_feed_motor_ctrl.speed_check_time != 0)	//2025-09-24
	{
		_ir_feed_motor_ctrl.speed_check_time--;
	}
}


void create_1ms_timer(void)	//2025-09-26 
{
	//Arm側の内部タイマの方が割り込みが早く、誤差が小さいので搬送速度測定の100msecに使用、
	//通常のタイマだと100msecで誤差3msec程度発生し、搬送速度設定に影響が出ている
	OSW_ISR_HANDLE isr_handle = {0};

	/* 周期タイマ割込みハンドラ登録 */
	OSW_ISR_create(&isr_handle, OSW_INT_PRIVATE_TIMER, osw_timer_isr);
	OSW_ISR_set_priority(OSW_INT_PRIVATE_TIMER, IPL_KERNEL_NORMAL); //OSで使用しているタイマを優先度同じ

	cyc_timer_clock(); 		/* 1ms周期(1000Hz)タイマ作成 */

	/* 周期タイマ割込み許可 */
	OSW_ISR_enable(OSW_INT_PRIVATE_TIMER);

}



/*********************************************************************//**
 * @brief		Calculate ROM file CRC.
 * @param[in]	None
 * @return 		Succeeded or Failure
 * @retval 		true Succeeded
 * @retval 		false Failure
 **********************************************************************/
u16 _calc_crc_initial_value(u8 *data, u32 length, u16 initial);
static void _set_program_crc(void) {
	// ROM program start address=0x00180000, end address=0x010FFFFD
    u16 tmp_crc;
    u32 end_addr;

    end_addr = ROM_ALIAS_END_ADDRESS;
    /* Flashに保存されているCRC処理 */
    tmp_crc = *(u8 *)(end_addr-1);
    tmp_crc = (tmp_crc << 8) & 0xff00;
    tmp_crc = tmp_crc + *(u8 *)end_addr;
    ex_rom_crc =  _calc_crc_initial_value(
    							(u8 *)(end_addr - 1),
								(u32)2,
								tmp_crc);
}
/******************************************************************************/
/*! @brief set version information
    @return         none
    @exception      none
******************************************************************************/
void _main_set_version_info(void)
{
	extern const u8 software_ver[];
	u16 temp_index;
	u16 temp_index2;
	u8 seq;
	/* calc and set main rom crc */
	_set_program_crc();

	temp_index = 0;
	temp_index2 = 0;
	seq = 1;
	while ((software_ver[temp_index] != '\0') && (seq != 0))
	{
		switch (seq)
		{
		case 1:
			/* Set Model */
			if (software_ver[temp_index] == ' ')
			{
				seq++;
				temp_index2 = 0;
			}
			else if (temp_index2 < MODEL_LENGTH)
			{
				ex_model[temp_index2] = software_ver[temp_index];
				temp_index2++;
			}
			break;
		case 2:
			/* Set Country */
			if (software_ver[temp_index] == ' ')
			{
				seq++;
				temp_index2 = 0;
			}
			else if (temp_index2 < COUNTRY_LENGTH)
			{
				ex_country[temp_index2] = software_ver[temp_index];
				temp_index2++;
			}
			break;
		case 3:
			/* Search Protocol */
			if (((software_ver[(temp_index - 1)] == 'i') || (software_ver[(temp_index - 1)] == 'I'))
			 && ((software_ver[temp_index] == 'd') || (software_ver[temp_index] == 'D')))
			{
				seq++;
			}
			break;
		case 4:
			/* Set Protocol */
			if (software_ver[temp_index] == ' ')
			{
				seq++;
				temp_index2 = 0;
			}
			else if (temp_index2 < PROTOCOL_LENGTH)
			{
				ex_protocol[temp_index2] = software_ver[temp_index];
				temp_index2++;
			}
			break;
		case 5:
			/* Search Version */
			if ((software_ver[temp_index] == 'v') || (software_ver[temp_index] == 'V'))
			{
				seq++;
			}
			break;
		case 6:
			/* Set Version */
			if (software_ver[temp_index] == ' ')
			{
				seq++;
				temp_index2 = 0;
			}
			else if ((temp_index2 < VERSION_LENGTH) && (software_ver[temp_index] != '-'))
			{
				ex_version[temp_index2] = software_ver[temp_index];
				temp_index2++;
			}
			break;
		case 7:
			/* Set Date */
			if (software_ver[temp_index] == ' ')
			{
				seq = 0;
				temp_index2 = 0;
			}
			else if (temp_index2 < DATE_LENGTH)
			{
				ex_date[temp_index2] = software_ver[temp_index];
				temp_index2++;
			}
			break;
		default:
			seq = 0;
			break;
		}
		temp_index++;
	}
}

#if 0//#if defined(UBA_ENABLE_FIX_CIS)
/*******************************
     load cis adjustment value
 *******************************/
void _main_load_adjustment()
{
	//debug
	memset(&ex_cis_adjustment_data, 0x00, sizeof(ex_cis_adjustment_data));
	ex_cis_adjustment_data.afe_again.shg0_u[0] = 0x00;
	ex_cis_adjustment_data.afe_again.shg0_u[1] = 0x00;
	ex_cis_adjustment_data.afe_again.shg0_u[2] = 0x00;
	ex_cis_adjustment_data.afe_again.shg0_u[3] = 0x00;
	ex_cis_adjustment_data.afe_again.shg0_d[0] = 0x00;
	ex_cis_adjustment_data.afe_again.shg0_d[1] = 0x00;
	ex_cis_adjustment_data.afe_again.shg0_d[2] = 0x00;
	ex_cis_adjustment_data.afe_again.shg0_d[3] = 0x00;
	ex_cis_adjustment_data.afe_aoffset.offdac_u[0] = 0x0A;
	ex_cis_adjustment_data.afe_aoffset.offdac_u[1] = 0x0A;
	ex_cis_adjustment_data.afe_aoffset.offdac_u[2] = 0x0A;
	ex_cis_adjustment_data.afe_aoffset.offdac_u[3] = 0x0A;
	ex_cis_adjustment_data.afe_aoffset.offdac_d[0] = 0x0A;
	ex_cis_adjustment_data.afe_aoffset.offdac_d[1] = 0x0A;
	ex_cis_adjustment_data.afe_aoffset.offdac_d[2] = 0x0A;
	ex_cis_adjustment_data.afe_aoffset.offdac_d[3] = 0x0A;
	ex_cis_adjustment_data.afe_dgain.dgain_u[0] = 0x20;
	ex_cis_adjustment_data.afe_dgain.dgain_u[1] = 0x20;
	ex_cis_adjustment_data.afe_dgain.dgain_u[2] = 0x20;
	ex_cis_adjustment_data.afe_dgain.dgain_u[3] = 0x20;
	ex_cis_adjustment_data.afe_dgain.dgain_d[0] = 0x20;
	ex_cis_adjustment_data.afe_dgain.dgain_d[1] = 0x20;
	ex_cis_adjustment_data.afe_dgain.dgain_d[2] = 0x20;
	ex_cis_adjustment_data.afe_dgain.dgain_d[3] = 0x20;
	// cis_time (1000)ex_cis_adjustment_data.cis_time
	ex_cis_adjustment_data.cis_time. red_ref_time_u = 500;
	ex_cis_adjustment_data.cis_time. gre_ref_time_u = 500;
	ex_cis_adjustment_data.cis_time. blu_ref_time_u = 500;
	ex_cis_adjustment_data.cis_time. ir1_ref_time_u = 500;
	ex_cis_adjustment_data.cis_time. ir2_ref_time_u = 1000;
	ex_cis_adjustment_data.cis_time. fl_ref_time_u = 2000;
	ex_cis_adjustment_data.cis_time. red_pen_time_u = 500;
	ex_cis_adjustment_data.cis_time. gre_pen_time_u = 500;
	ex_cis_adjustment_data.cis_time. ir1_pen_time_u = 500;
	ex_cis_adjustment_data.cis_time. ir2_pen_time_u = 1000;
	/* inside */
	ex_cis_adjustment_data.cis_time. red_ref_time_d = 500;
	ex_cis_adjustment_data.cis_time. gre_ref_time_d = 500;
	ex_cis_adjustment_data.cis_time. blu_ref_time_d = 500;
	ex_cis_adjustment_data.cis_time. ir1_ref_time_d = 500;
	ex_cis_adjustment_data.cis_time. ir2_ref_time_d = 1000;
	ex_cis_adjustment_data.cis_time. fl_ref_time_d = 2000;
	ex_cis_adjustment_data.cis_time. red_pen_time_d = 500;
	ex_cis_adjustment_data.cis_time. gre_pen_time_d = 500;
	ex_cis_adjustment_data.cis_time. ir1_pen_time_d = 500;
	ex_cis_adjustment_data.cis_time. ir2_pen_time_d = 1000;
	// cis_sled
	ex_cis_adjustment_data.cis_sled. red_ref_da_u = 0;
	ex_cis_adjustment_data.cis_sled. gre_ref_da_u = 0;
	ex_cis_adjustment_data.cis_sled. blu_ref_da_u = 0;
	ex_cis_adjustment_data.cis_sled. ir1_ref_da_u = 0;
	ex_cis_adjustment_data.cis_sled. ir2_ref_da_u = 1;
	ex_cis_adjustment_data.cis_sled. fl_ref_da_u = 3;
	ex_cis_adjustment_data.cis_sled. red_pen_da_u = 1;
	ex_cis_adjustment_data.cis_sled. gre_pen_da_u = 0;
	ex_cis_adjustment_data.cis_sled. ir1_pen_da_u = 1;
	ex_cis_adjustment_data.cis_sled. ir2_pen_da_u = 1;
	/* inside */
	ex_cis_adjustment_data.cis_sled. red_ref_da_d = 0;
	ex_cis_adjustment_data.cis_sled. gre_ref_da_d = 0;
	ex_cis_adjustment_data.cis_sled. blu_ref_da_d = 0;
	ex_cis_adjustment_data.cis_sled. ir1_ref_da_d = 0;
	ex_cis_adjustment_data.cis_sled. ir2_ref_da_d = 1;
	ex_cis_adjustment_data.cis_sled. fl_ref_da_d = 3;
	ex_cis_adjustment_data.cis_sled. red_pen_da_d = 1;
	ex_cis_adjustment_data.cis_sled. gre_pen_da_d = 0;
	ex_cis_adjustment_data.cis_sled. ir1_pen_da_d = 1;
	ex_cis_adjustment_data.cis_sled. ir2_pen_da_d = 1;
	// cis_da
	ex_cis_adjustment_data.cis_da.red_ref_da_u = 140;
	ex_cis_adjustment_data.cis_da.gre_ref_da_u = 160;
	ex_cis_adjustment_data.cis_da.blu_ref_da_u = 80;
	ex_cis_adjustment_data.cis_da.ir1_ref_da_u = 140;
	ex_cis_adjustment_data.cis_da.ir2_ref_da_u = 115;
	ex_cis_adjustment_data.cis_da.fl_ref_da_u = 220;
	ex_cis_adjustment_data.cis_da.red_pen_da_u = 130;
	ex_cis_adjustment_data.cis_da.gre_pen_da_u = 140;
	ex_cis_adjustment_data.cis_da.ir1_pen_da_u = 115;
	ex_cis_adjustment_data.cis_da.ir2_pen_da_u = 150;
	/* inside */
	ex_cis_adjustment_data.cis_da.red_ref_da_d = 140;
	ex_cis_adjustment_data.cis_da.gre_ref_da_d = 160;
	ex_cis_adjustment_data.cis_da.blu_ref_da_d = 80;
	ex_cis_adjustment_data.cis_da.ir1_ref_da_d = 140;
	ex_cis_adjustment_data.cis_da.ir2_ref_da_d = 115;
	ex_cis_adjustment_data.cis_da.fl_ref_da_d = 220;
	ex_cis_adjustment_data.cis_da.red_pen_da_d = 130;
	ex_cis_adjustment_data.cis_da.gre_pen_da_d = 140;
	ex_cis_adjustment_data.cis_da.ir1_pen_da_d = 115;
	ex_cis_adjustment_data.cis_da.ir2_pen_da_d = 150;
	// cis_tmp_time (1000)ex_cis_adjustment_tmp.cis_tmp_time
	memcpy(&ex_cis_adjustment_tmp.cis_tmp_time, &ex_cis_adjustment_data.cis_time, sizeof(CIS_ADJUSTMENT_TIME));
	// cis_tmp_da (0)ex_cis_adjustment_tmp.cis_tmp_da
	memcpy(&ex_cis_adjustment_tmp.cis_tmp_da, &ex_cis_adjustment_data.cis_da, sizeof(CIS_ADJUSTMENT_DA));
	// cis_tmp_sled (0)ex_cis_adjustment_tmp.cis_tmp_iled
	memcpy(&ex_cis_adjustment_tmp.cis_tmp_sled, &ex_cis_adjustment_data.cis_sled, sizeof(CIS_ADJUSTMENT_DA));
	// cis_pga (1.0f)ex_cis_adjustment_tmp.cis_pga
	/* outside */
	ex_cis_adjustment_tmp.cis_pga.red_ref_pga_u = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.gre_ref_pga_u = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.blu_ref_pga_u = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.ir1_ref_pga_u = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.ir2_ref_pga_u = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.fl_ref_pga_u = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.red_pen_pga_u = 1.0f; // not used
	ex_cis_adjustment_tmp.cis_pga.gre_pen_pga_u = 1.0f; // not used
	ex_cis_adjustment_tmp.cis_pga.ir1_pen_pga_u = 1.0f; // not used
	ex_cis_adjustment_tmp.cis_pga.ir2_pen_pga_u = 1.0f; // not used
	/* inside */
	ex_cis_adjustment_tmp.cis_pga.red_ref_pga_d = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.gre_ref_pga_d = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.blu_ref_pga_d = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.ir1_ref_pga_d = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.ir2_ref_pga_d = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.fl_ref_pga_d = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.red_pen_pga_d = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.gre_pen_pga_d = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.ir1_pen_pga_d = 1.0f;
	ex_cis_adjustment_tmp.cis_pga.ir2_pen_pga_d = 1.0f;


	memset(&ex_cis_adjustment_data.cis_bc, 0, sizeof(BC_DATA));
	memset(&ex_cis_adjustment_data.cis_wc, 0, sizeof(WC_DATA));
	for (int i = 0; i < MAIN_SCAN_LINE; i++)
	{
		/* black */
		ex_cis_adjustment_data.cis_bc.black_data1_u[i] = 50;
		ex_cis_adjustment_data.cis_bc.black_data1_d[i] = 50;
		ex_cis_adjustment_data.cis_bc.black_data2_u[i] = 50;
		ex_cis_adjustment_data.cis_bc.black_data2_d[i] = 50;
		/* white */
		/* outside */
		ex_cis_adjustment_data.cis_wc.red_ref_u[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.gre_ref_u[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.blu_ref_u[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.ir1_ref_u[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.ir2_ref_u[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.fl_ref_u[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.c6_led_u[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.c7_led_u[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.c8_led_u[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.c9_led_u[i] = 0x100;
		/* inside */
		ex_cis_adjustment_data.cis_wc.red_ref_d[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.gre_ref_d[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.blu_ref_d[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.ir1_ref_d[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.ir2_ref_d[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.fl_ref_d[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.red_pen_d[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.gre_pen_d[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.ir1_pen_d[i] = 0x100;
		ex_cis_adjustment_data.cis_wc.ir2_pen_d[i] = 0x100;
	}
	set_position_ga();
}
#endif


/****************************************************************/
/**
 * @brief メインタスクエントリー
 */
/****************************************************************/
void main_task(void)
{
	// DONE:モデル、バージョンのセット
	_main_set_version_info();
	_main_init_var();

	ex_uba710 = 1;
	// MPU CLK set 400mhz
	set_mpu_clock(_DEBUG_MPU_CLOCK);
	get_pll_info();

	//Fornt USBのみの給電の場合、このまま処理を進めると Power up bill in Acceptorとなる
	//後ろの電源ON後もAcceptor JAMが解除できない状態となる、解除する為には電源OFFが必須となる
	//市場ではこのような使用の可能性が低いが、対策としてFront USBでのみの給電の場合、
	//後ろの電源ON待ち、電源ON検知でソフトリセットを行う
	if(__hal_5v_dect_read())
	{

	// Front UBSでの給電のみの場合、後ろの電源検知待ち、後ろの電源ONでソフトリセット
	// ダウンロード機能のみ有効も考えたが、通常タスクを幾つか起動させる必要がある
	// FGPAを展開していないので、FGPA関係をアクセスしている場合CPUエラーとなる
	// 今後のソースコード変更で動かなくなるリストもあるので、最初からサポートしない
	// 例) WDTタスクの処理で現状モータ処理のフェースセーフ関係をPFGA経由でHigh Lowさせている。その処理を削除しないとエラーになる
		usb_operation_main();
	}


#if (_DEBUG_FPGA_FRAM==1) //2023-07-22
	ex_fram_log_enable = 0;
#endif

//#if (_DEBUG_FPGA_CLOCK_NOT_STOP==1)
    ex_fpga_dummy_clk = 0;
//#endif
	create_1ms_timer();	//RTQの搬送速度測定用 //2025-09-26
    /* Initialize FPGA */
	initialize_pl();
	_main_set_pl_active(PL_ENABLE);

	ex_fpga_version = FPGA_REG.FVER.LWORD;
#if (FPGA_LOG_ENABLE==1)
	_pl_evrec_stop();
	_pl_evrec_init();
	_pl_evrec_start(1);
#endif /* FPGA_LOG_ENABLE */


	_hal_i2c3_init_iox();	//2023-05-11 BootでIN OUT設定が0x83の場合があるので
	// position sensor GAIN
	_main_set_position_gain();
	// position sensoer D/A
	_main_set_position_da();

	unit_style_check();
	//for debug _pl_subboard_type();	//2025-08-18

	init_motor_max_current();
	_kernel_synch_cache();
	_main_start_cyc();
	//　ベゼルタスク、DIPタスクの前に初期化
	_main_act_peripheral_task();

	do
	{
		dly_tsk(100);
	}while(ex_dipsw1_run == 0);
	//2025-09-03 ivizion2とは変える、RTQなのでRTQとの通信エラーなどをID-003に通知する為、
	//UBA500 SSもRTQは早い段階で通信設定をおこなっている。
	//ここで行わないと、_rc_initial_msg_proc 内でのエラーをID-003に通知できない
	if ((ex_dipsw1 & DIPSW1_PERFORMANCE_TEST) == 0)
	{
		_main_select_protocol();
	}

#if defined(UBA_RTQ)
	//2025-02-21 DIP-SWタスクが動き始めたばかり、
	//SSはmode_powerで使用しているが、
	//RTQはテストモードなどDIP-SWの設定でリサイクル設定をクリアする場合があるので、
	//ここでRTQが完全に動作するのを待つ必票がある

	//2025-07-15
	//RTQにResetをかける為、これは必要
	//UBA500はBootでReset信号Highにしているが、
	//UBA700はBootのミスで信号Lowにしている。
	//UBA700はIFの最初でHighにしているが、時間確保の為、ここにディレイを追加
	dly_tsk(3000); //ok 50でもオシロ上ではResetかけれる

	if(_rc_initial_msg_proc() != 0)
	{
		_main_set_mode(MODE1_RCINIT, RCINIT_MODE2_DL_START);
	}
#endif
	
	// センサ調整値の読み出し
	//2022-03-08	
	#if 0//#if defined(UBA_ENABLE_FIX_CIS)
	_main_load_adjustment();
	#endif


	#if 0 //2024-02-29
	u32 size[40];
	/* Statistics */
	size[1] = JDL_STAT_ADR_BUFF_BASE; //162 162 start JDL 0スタートの実アドレス
	size[2] = JDL_STAT_ADR_MOV_FRAM; //192 192
	
	size[3] = JDL_STAT_ADR_ACC_BASE; //210 210
	size[4] = JDL_STAT_ADR_RC_BASE; //232 232
	size[5] = JDL_STAT_ADR_REJ_BASE; //254 254
	size[6] = JDL_STAT_ADR_REJ_LENGTH; //294 294
	size[7] = JDL_STAT_ADR_REJ_BAR_NC; //332 332
	size[8] = JDL_STAT_ADR_ERR_BASE; //362 362
	size[9] = JDL_STAT_ADR_ERR_POSI_AT; //390 390
	size[10] = JDL_STAT_ADR_ERR_CENT_O_RUN; //410 410

	size[11] = JDL_STAT_ADR_STAT_BLK_SIZE; //430 480
	size[12] = JDL_STAT_ADR_STAT_BASE; //434 484

	size[13] = JDL_STAT_OFS_STAT_ACC_BAR; //2 2
	size[14] = JDL_STAT_OFS_STAT_ERR_FEED; //28 28
 
	size[15] = JDL_STAT_ADR_FOR_RESERVED; //854-> 1224 end

	size[16] = JDL_STAT_SEND_TOTAL; //690 1060
	size[17] = JDL_STAT_BUFF_TOTAL; //692 1062

	/* sensor */
	size[18] = JDL_SENS_ADR_BUFF_BASE; //854-> 1224 start
	size[19] = JDL_SENS_ADR_ADJ_VAL; //870 1240
	size[20] = JDL_SENS_ADR_PRE_COR_VAL; //898 1268


	/* communication */
	size[21] = JDL_COMM_ADR_BUFF_BASE; //918 1291 start

	/* Event */
	size[22] = JDL_EVEN_ADR_BUFF_BASE; //5537 5910 start


	/* Error */
	size[23] = JDL_ERR_ADR_BUFF_BASE; //15546 17419 start

	/* Acceptance */
	size[24] = JDL_ACC_ADR_BUFF_BASE; //24775 29720 start

	/* Position */
	size[25] = JDL_PANA_ADR_BUFF_BASE; //57231 62176 start
	size[26] = JDL_PANA_ADR_FOR_NEXT_CATEGORY; //86877 Additional RC start

	/* Additional RC *//* ここエリアまでHead FRAMに保存*/
	size[27] = JDL_RC_ADR_BUFF_BASE; //86877 Additional RC start
	size[28] = JDL_RC_BUFF_TOTAL; //450 ok

	size[32] = JDL_BUFF_TOTAL; //87327 Additional RC end
	size[33] = JDL_RC_ADR_FOR_NEXT_CATEGORY; //87327 Additional RC end
	size[34] = JDL_RC_ADR_FOR_RESERVED; //87327 Additional RC end

	/* Option RC *//* FRAM外 */
	size[29] = JDL_OPRC_ADR_BUFF_BASE; //87327
	size[30] = JDL_OPRC_BUFF_TOTAL; //64002

	size[31] = JDL_OPRC_ADR_FOR_NEXT_CATEGORY; //151329
	//size[32] = FRAM_JDL_SIZE;

	//#define JDL_BUFF_TOTAL  JDL_RC_ADR_FOR_NEXT_CATEGORY  /* [Buffer Total Size] */
    //#define JDL_RC_ADR_FOR_NEXT_CATEGORY (JDL_RC_ADR_FOR_RESERVED + 0)                 /* [Address for the next category] BADR: 87327, CADR:   450  (Reserved area = 0) */
    //#define JDL_RC_ADR_FOR_RESERVED  (JDL_RC_ADR_UINFO_BASE + JDL_RC_UNIFO_TOTAL_SIZE) /* [Address for the reserved] BADR: 87327, CADR:   450 */

	size[35] = JDL_RC_ADR_UINFO_BASE; //21347 86883
	size[36] = JDL_RC_UNIFO_TOTAL_SIZE; //444 444
	#endif

	#if 0 //調査用
	u32 size[30];
	/* Statistics */
	size[0] = FRAM_ADJ_OFFSET; //0
	size[1] = FRAM_ADJ_TMP_OFFSET; //35840
	size[2] = FRAM_PWR_OFFSET; //36352
	size[3] = FRAM_IF_OFFSET; //36864
	size[4] = FRAM_ICB_OFFSET; //37888
	size[5] = FRAM_LOG_OFFSET; //38912


	#endif

#if 0	//2025-09-08
	u32 size2[30];
	size2[0] = sizeof(ADJUSTMENT_DATA); 		//128
	size2[1] = sizeof(CIS_ADJUSTMENT_DATA);	//34676
	size2[2] = sizeof(CIS_ADJUSTMENT_TMP);	//216

	size2[3] = sizeof(CIS_ADJUSTMENT_TMP);	//216
	size2[4] = sizeof(ex_icb_recovery_blank); //516

#endif


	operation_main();
}

/*********************************************************************//**
 * @brief main_task initialze Global Variables
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_init_var(void)
{
	ST_BS *pbill_data;
	// initialize global variables
	ex_main_test_no = TEST_STANDBY;
	memset((u8 *)&ex_collection_data, 0, sizeof(DATA_COLLECTION_INFO));
	//note parameter setting
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	for(int i = 0; i < BILL_NOTE_IMAGE_MAX_COUNT; i++)
	{
		pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[i];
		parameter_set(pbill_data);
	}
#else
	pbill_data = (ST_BS *)BILL_NOTE_IMAGE_TOP;
	parameter_set(pbill_data);
#endif

}
/*********************************************************************//**
 * @brief main_task select Host I/F procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_select_protocol(void)
{
	u8 if_select;
	u8 protocol_select;

#if (LOOPBACK_UBA==1)
	ex_loopback_error = 0;
#endif

#if defined(_PROTOCOL_ENABLE_ID003)
	ex_smrt_id = 3;
	if_select = IF_SELECT_TTL;
	protocol_select = PROTOCOL_SELECT_ID003;
#elif defined(_PROTOCOL_ENABLE_ID0G8)
	ex_smrt_id = 240;
	if_select = IF_SELECT_USB;
	protocol_select = PROTOCOL_SELECT_ID0G8;
#else
	ex_smrt_id = 3;
	if_select = IF_SELECT_TTL;
	protocol_select = PROTOCOL_SELECT_ID003;
#endif /* _PROTOCOL_ENABLE_ID003 */
#if defined(_DEBUG_UART_LOOPBACK)
	ex_smrt_id = 3;
	if_select = IF_SELECT_TTL;
	protocol_select = PROTOCOL_SELECT_TEST;
#endif
	read_status_table();
	if (protocol_select != get_protocol())
	{ /* last time isn't same protocol or version */
		init_status_table();
	}

	set_protocol(protocol_select);
	ex_cline_status_tbl.if_select = if_select;

	write_status_table();
	_kernel_synch_cache();

	act_tsk(ID_CLINE_TASK);	//2025-09-03 上記のread_status_table 後にあとでないとだめなので、セットにするため、ここに配置

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
void _main_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MAIN_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_MAIN_TASK;
		t_msg->mpf_id = ID_MBX_MAIN_MPF;
		t_msg->tmsg_code = tmsg_code;
		t_msg->arg1 = arg1;
		t_msg->arg2 = arg2;
		t_msg->arg3 = arg3;
		t_msg->arg4 = arg4;
		ercd = snd_mbx(receiver_id, (T_MSG *)t_msg);
		if (ercd != E_OK)
		{
			/* system error */
			_main_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_main_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _main_system_error(u8 fatal_err, u8 code)
{
#ifdef _DEBUG_SYSTEM_ERROR
//	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_MAIN_TASK, (u16)code, (u16)ex_main_msg.tmsg_code, (u16)ex_main_msg.arg1, fatal_err);
}

#if 1 //2024-05-07
void _main_system_alarm(u16 alarm)
{

	u8 l_dipsw;

	_hal_read_dipsw1(&l_dipsw);


	if (l_dipsw & DIPSW1_PERFORMANCE_TEST)
	{
		ex_operating_mode |= OPERATING_MODE_TEST;
	}

#if 1//#ifdef _ENABLE_JDL
	jdl_error(alarm, 0xF000, ex_main_task_mode1, ex_main_task_mode2, 0);
#endif /* _ENABLE_JDL */

	_main_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_LED_OFF, 0, 0, 0, 0);
	_main_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, (u32)alarm, 0, 0, 0);

    /* I/Fで通信を行うためにI/F初期化 */
	_main_send_connection_task(TMSG_CONN_INITIAL, ex_operating_mode, TMSG_SUB_ALARM, (u32)alarm, 0 );

	/* Support USB download and monitor. */
	act_tsk(ID_DLINE_TASK);
	_main_send_msg(ID_DLINE_MBX, TMSG_DLINE_INITIAL_REQ, ex_operating_mode, TMSG_SUB_ALARM, alarm, 0);

	while (1)
	{
		dly_tsk(500);
	}
}
#endif

//#endif

/* EOF */
