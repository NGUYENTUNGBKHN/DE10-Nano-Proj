/******************************************************************************/
/*! @addtogroup Group2
    @file       status_tbl.c
    @brief      status table
    @date       2018/02/26
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/

#include <string.h>
#include "common.h"
#include "status_tbl.h"
#include "sub_functions.h"
#include "hal.h"
#include "hal_spi_fram.h"

#define EXT
#include "com_ram.c"

u8 bkex_status_tbl_buff[256]; // ex_cline_status_tbl, ex_recovery_info を bkex_status_tbl_buff にコピーしてFRAMへ読み書きしているので、サイズを変えないこと
/*	[000]		ex_cline_status_tbl.protocol_select		*/
/*	[001]		ex_cline_status_tbl.if_select			*/
/*	[002]~[003]	ex_cline_status_tbl.line_task_mode		*/
/*	[004]~[005]	ex_cline_status_tbl.status				*/
/*	[006]~[007]	ex_cline_status_tbl.escrow_code			*/
/*	[008]~[009]	ex_cline_status_tbl.reject_code			*/
/*	[010]~[011]	ex_cline_status_tbl.error_code			*/
/*	[012]~[013]	ex_cline_status_tbl.dipsw_disable		*/
/*	[014]~[017]	ex_cline_status_tbl.bill_disable		*/
/*	[018]~[021]	ex_cline_status_tbl.bill_disable_mask	*/
/*	[022]~[025]	ex_cline_status_tbl.bill_escrow			*/
/*	[026]~[029]	ex_cline_status_tbl.security_level		*/
/*	[030]~[031]	ex_cline_status_tbl.comm_mode			*/
/*	[032]~[033]	ex_cline_status_tbl.accept_disable		*/
/*	[034]~[035]	ex_cline_status_tbl.direction_disable	*/
/*	[036]~[037]	ex_cline_status_tbl.option				*/
/*	[038]		ex_cline_status_tbl.barcode_type		*/
/*	[039]		ex_cline_status_tbl.barcode_length		*/
/*	[040]		ex_cline_status_tbl.barcode_inhibit		*/
/*	[041]~[044]	ex_cline_status_tbl.comm_addr			*/
/*	[045]		ex_cline_status_tbl.encryption			*/
/*	[046]~[063]	<<<<<<<<<<<<<<< reserve1 >>>>>>>>>>>>>>	*/
/*	[064]		ex_recovery_info.recovery				*/
/*	[065]		ex_recovery_info.feed_position			*/
/*	[066]		ex_recovery_info.stack_start			*/
/*	[067]~[253]	<<<<<<<<<<<<<<< reserve2 >>>>>>>>>>>>>>	*/
/*	[254]~[255]	check sum								*/


/************************** Function Prototypes ******************************/
u8 fram_write_status_tbl(void);
u8 fram_read_status_tbl(void);
void set_recovery_count( u8 unit ); //ok
/*********************************************************************//**
 * @brief init status table
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_status_table(void)
{
	ER ercd;

	ercd = wai_sem(ID_STATE_TBL_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
	//// ex_cline_status_tbl, ex_recovery_info を bkex_status_tbl_buff にコピーしてFRAMへ読み書きしているので、サイズを変えないこと
	memset(bkex_status_tbl_buff, 0, sizeof(bkex_status_tbl_buff));
	fram_write_status_tbl();
	memcpy((void *)&ex_cline_status_tbl, &bkex_status_tbl_buff[CLINE_STATUS_TBL_OFFSET], sizeof(ex_cline_status_tbl));
	memcpy((void *)&ex_recovery_info, &bkex_status_tbl_buff[RECOVERY_INFO_OFFSET], sizeof(ex_recovery_info));

	ercd = sig_sem(ID_STATE_TBL_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
}


/*********************************************************************//**
 * @brief read status table
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void read_status_table(void)
{
	ER ercd;
	u16 sum;
	u16 sum2;

	ercd = wai_sem(ID_STATE_TBL_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
	// ex_cline_status_tbl, ex_recovery_info を bkex_status_tbl_buff にコピーしてFRAMへ読み書きしているので、サイズを変えないこと
	fram_read_status_tbl(); //256サイズ読み込んでいるので、ex_cline_status_tbl と ex_recovery_info の両方

	memcpy((void *)&sum, &bkex_status_tbl_buff[STATUS_TBL_SUM_OFFSET], sizeof(sum));

	sum2 = calc_check_sum(bkex_status_tbl_buff, (sizeof(bkex_status_tbl_buff) - 2));

	if (sum2 != sum)
	{
		memset(bkex_status_tbl_buff, 0, sizeof(bkex_status_tbl_buff));
	}

	memcpy((void *)&ex_cline_status_tbl, &bkex_status_tbl_buff[CLINE_STATUS_TBL_OFFSET], sizeof(ex_cline_status_tbl));
	memcpy((void *)&ex_recovery_info, &bkex_status_tbl_buff[RECOVERY_INFO_OFFSET], sizeof(ex_recovery_info));

	ercd = sig_sem(ID_STATE_TBL_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
}
/*********************************************************************//**
 * @brief read protocol
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8 get_protocol(void)
{
	return ex_cline_status_tbl.protocol_select;
}
/*********************************************************************//**
 * @brief write protocol
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void set_protocol(u8 protocol)
{
	ex_cline_status_tbl.protocol_select = protocol;
}

/*********************************************************************//**
 * @brief write status table
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void write_status_table(void)
{
	ER ercd;
	u16 sum;

	ercd = wai_sem(ID_STATE_TBL_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	// ex_cline_status_tbl, ex_recovery_info を bkex_status_tbl_buff にコピーしてFRAMへ読み書きしているので、サイズを変えないこと
#if 1 //2023-11-01 infoも追加 2023-12-04
	memcpy(&bkex_status_tbl_buff[CLINE_STATUS_TBL_OFFSET], &ex_cline_status_tbl, sizeof(ex_cline_status_tbl));
	memcpy(&bkex_status_tbl_buff[RECOVERY_INFO_OFFSET], &ex_recovery_info, sizeof(ex_recovery_info)); //info

	sum = calc_check_sum(bkex_status_tbl_buff, (sizeof(bkex_status_tbl_buff) - 2));
	memcpy(&bkex_status_tbl_buff[STATUS_TBL_SUM_OFFSET], (void *)&sum, sizeof(sum));
#else
	memcpy(&bkex_status_tbl_buff[CLINE_STATUS_TBL_OFFSET], &ex_cline_status_tbl, sizeof(ex_cline_status_tbl));
	sum = calc_check_sum(bkex_status_tbl_buff, (sizeof(bkex_status_tbl_buff) - 2));
	memcpy(&bkex_status_tbl_buff[STATUS_TBL_SUM_OFFSET], (void *)&sum, sizeof(sum));
#endif

	fram_write_status_tbl();

	ercd = sig_sem(ID_STATE_TBL_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
}


/*********************************************************************//**
 * @brief write status table
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void set_recovery_step(u8 step)
{
	ER ercd;
	u16 sum;

	if (is_test_mode())
	{
	/* テストモードは行わない*/
		return;
	}
	//2025-07-31
	if( ex_recovery_info.step == step )
	{
	//前回と同じ場合 RTQがPayout時などで同じ値を送信する事が多いので、不要な場合抜けるようにする
		return;
	}
	//2025-07-31 UBA500と処理の順番を変える、更新しない場合はセマフォ取得前に抜ける
#if defined(UBA_RTQ) //2025-04-21
	if( (1 == ex_main_emergency_flag) && ( RECOVERY_STEP_ACCEPT == step ) )
	{
		return;
	}
	/* 入金、出金共通で使用する関数（スイッチバック位置まで搬送処理）    */
	/* でRECOVERY_STEP_PAYOUT_POS1を設定している。入金時は設定しないよう */
	/* ここでチェックする */
	else if(RECOVERY_STEP_PAYOUT_POS1 == step || RECOVERY_STEP_PAYOUT_RS_POS7 == step)
	{
		if( (RECOVERY_STEP_PAYOUT_DRUM      != ex_recovery_info.step) &&
			(RECOVERY_STEP_PAYOUT_TRANSPORT != ex_recovery_info.step) )
		{		
			return;
		}
	}
	/* Emergency Stop中にSTACK_TRANSPORT(入金搬送中)を設定しないようにする */
	else if( RECOVERY_STEP_STACK_TRANSPORT == step )
	{
		if( RECOVERY_STEP_EMRGENCY_TRANSPORT == ex_recovery_info.step)
		{		
			return;
		}
	}
#endif

	/* Normal Operation */
	ercd = wai_sem(ID_STATE_TBL_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

#if defined(UBA_RTQ) //2025-04-21
	if( RECOVERY_STEP_NON == step )
	{
		set_recovery_count(ex_recovery_info.unit);
	}
#endif

	ex_recovery_info.step = step;

	memcpy(&bkex_status_tbl_buff[RECOVERY_INFO_OFFSET], (void *)&ex_recovery_info, sizeof(ex_recovery_info));
	sum = calc_check_sum(bkex_status_tbl_buff, (sizeof(bkex_status_tbl_buff) - 2));
	memcpy(&bkex_status_tbl_buff[STATUS_TBL_SUM_OFFSET], (void *)&sum, sizeof(sum));

	fram_write_status_tbl();

#if defined(UBA_LOG)	//2025-07-30
	ex_back_log[ex_back_log_index] = step;
	ex_back_log_index++;
	ex_back_log_index = ex_back_log_index%EX_BACK_LOG;
#endif

	ercd = sig_sem(ID_STATE_TBL_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
}

/*********************************************************************//**
 * @brief write status table to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8 fram_write_status_tbl(void)
{
	/// TODO: FRAM処理、関数名eeprom,fram変更
	u8 err = 0;
	u8 *ptr;

	/* status table */
	ptr = (u8 *)&bkex_status_tbl_buff;

	err = _hal_write_fram_power_recover( FRAM_STATUS_TBL_ADR, ptr, FRAM_STATUS_TBL_SIZE);
	if( !err )
	{
		err = 1;
	}
	return 0;
}
/*********************************************************************//**
 * @brief write status table to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8 fram_read_status_tbl(void)
{
	/// TODO: FRAM処理、関数名eeprom,fram変更
	u8 err = 0;
	u8 *ptr;

	/* status table */
	//// ex_cline_status_tbl, ex_recovery_info を bkex_status_tbl_buff にコピーしてFRAMへ読み書きしているので、サイズを変えないこと
	ptr = (u8 *)&bkex_status_tbl_buff[0];

	err = _hal_read_fram_power_recover( FRAM_STATUS_TBL_ADR, ptr, FRAM_STATUS_TBL_SIZE);
	if( !err )
	{
		err = 1;
	}

	return(err);
}

//#if defined(_PROTOCOL_ENABLE_ID0G8)	//2023-04-28
void check_ticket_char(void)
{
	u8 cnt,ii=0;
	u8 size;
	
	size = sizeof(ex_cline_status_tbl.ex_Barcode_recovery_icb);


	for( cnt = 0; cnt < size; cnt++ )
	{
		if( ex_cline_status_tbl.ex_Barcode_recovery_icb[cnt] >= 0x30 && ex_cline_status_tbl.ex_Barcode_recovery_icb[cnt] <= 0x39 )
		{
		/* ok*/
			ii++;
		}
		else
		{
		/* clear */
			break;
		}
	}
	ex_barcode_charactor_count = ii;
}
//#endif


#if !defined(UBA_RTQ)
u32 is_icb_recovery_info_stack(u8 *denomi) /* 本来はパワーリカバリ用だが、動作時のResetによるイニシャルでも呼び出される*/
{
	/* Stackコマンド受信～FRAMバックアップ前のリカバリ */
	ER ercd;
	u32 result = 0;

	ercd = wai_sem(ID_STATE_TBL_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	/* Escrow~ICBカウントアップまで */
	/* RECOVERY_STEP_ICB_ACCEPTはICBのカウントアップ処理済みなので含まない */
	if ((ex_recovery_info.step >= RECOVERY_STEP_ESCORW)
	 && (ex_recovery_info.step < RECOVERY_STEP_ICB_ACCEPT))
	{
		*denomi = ex_cline_status_tbl.escrow_code;
		result = 1;

		//#if defined(_PROTOCOL_ENABLE_ID0G8)	//2023-04-28 2023-12-12
		if(ex_cline_status_tbl.escrow_code == BAR_INDX)
		{
			memo_copy(  &ex_barcode[0], (u8*)&ex_cline_status_tbl.ex_Barcode_recovery_icb[0], sizeof(ex_cline_status_tbl.ex_Barcode_recovery_icb));
			check_ticket_char();	/* set ticket char */
		}
		//#endif

	}

	ercd = sig_sem(ID_STATE_TBL_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
	return result;
}
#endif


#if defined(UBA_RTQ)
void set_recovery_unit( u8 unit, u8 unit_count ) //ok
{
	if(!(is_test_mode()))
	{
		ex_recovery_info.unit_count = unit_count;
		ex_recovery_info.unit       = unit;

	}
	return;
}

void clear_recovery_unit(void) //ok only use A_PRO
{
	if(!(is_test_mode()))
	{
		ex_recovery_info.unit = 0;
	}
	return;
}

void set_recovery_count( u8 unit ) //ok
{
	if( 0 == unit )
	{
		return;
	}

	if(!(is_test_mode()))
	{
		ex_recovery_info.count = RecycleSettingInfo.DenomiInfo[unit-1].RecycleCurrent;
	}
	return;
}

/* '19-09-17 */
void set_stack_mode(u8 mode)
{
	ex_recovery_info.stack_mode = mode;
}
#endif
/* EOF */
