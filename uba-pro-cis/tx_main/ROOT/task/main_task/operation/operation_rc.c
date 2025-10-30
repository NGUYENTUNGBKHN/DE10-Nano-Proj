/******************************************************************************/
/*! @addtogroup Group2
    @file       operation_sub.c
    @brief      operation sub process
    @date       2013/03/23
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2013/03/23 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
#if defined(UBA_RTQ)

#include <string.h>
#include "common.h"
#include "cyc.h"
#include "sensor_ad.h"
#include "architecture.h"
#include "custom.h"
#include "country/src/country_custom.h"

#include "js_io.h"
#include "js_oswapi.h"
#include "hal_gpio.h"
#include "hal_gpio_reg.h"

#include "operation.h"
#include "if_rc.h"

#include "sub_functions.h"

#include "status_tbl.h"
#include "jdl_conf.h"
#include "jdl.h"

#define EXT
#include "com_ram.c"


#include  "kernel.h"    // CCS用に追加 ITORN特有の変数関数の型宣言
#include  "kernel_inc.h" // タスクID Toolで自動的に設定された各タスク、メッセージBOXのID番号

/************************** PRIVATE DEFINITIONS *************************/

/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/
u8 check_bill_denomi(void);
u8 check_bill_length(void);
u8 check_recycle_full_sensor(void);
u8 check_recycle_limit_count(void);
bool check_bill_rc_inh(void);


bool is_twin_drum1_full_check(void);
bool is_twin_drum2_full_check(void);
bool is_quad_drum1_full_check(void);
bool is_quad_drum2_full_check(void);

static bool check_drum_enable(u8 drum);


/************************** EXTERN FUNCTIONS *************************/
extern const u8 rcTbl_dt1[8][2];
extern const u8 rcTbl_dt2[8][2];
extern const u8 rctblFileno[100];
extern const u8 rcinhFileno[100];
extern const u8 rcLength_dt1[8];
extern const u8 rcLength_dt2[8];
extern const u8	rcMax_dt1[8];
extern const u8	rcMax_dt2[8];
extern const u8 rcMin_dt1[8];
extern const u8 rcMin_dt2[8];
extern const u8 rcSkew_dt[100];

extern const u8 encryption_serial_number[9];		/* GC2 Encryption S/N */
extern const u8 encryption_key_initial_value[8];	/* GC2 Encryption key initil value */
extern const u16 GC2tbl[0x100];


/*********************************************************************//**
 * @brief set recycle denomination information
 * @param[in]	None
 * @return 		None
 **********************************************************************/
// 3つで使用している
void is_recycle_set_test_denomi(void)//テストモードで8 OFF前に呼び出される //UBA500では 電源ON時のDIP-SWでのエージング、生産エージング、DIP 1,3,4,5,6のモード
{
	u8	cnt;
	u8	*ptr_dt1, *ptr_dt2;

	u8	unit_type;
	u8	dipsw = 0;

	/* initialize buffer */
	memset((u8 *)&RecycleSettingInfo.DenomiInfo[0], 0, sizeof(RecycleSettingInfo));

	if(is_quad_model())
	{
		unit_type = 4;
	}
	else
	{
		unit_type = 2;
	}

	//ここに入る処理は3つ
	//1無鑑別のテストモード - 強制的にその国のDenomi順でリサイクル設定
	//2エージングモード(後ろのDIP-SW OFF) - 強制的に全て EUR10
	//3エージングモード(後ろのDIP-SW 有効) - 後ろのDIP-SWの設定で全てのリサイクルが設定 (UBA500RTQはデバッグ用なので、UBA700RTQは対応しない)

	//ここには入ってこないが
	//識別ありのテストモード - リサイクル設定はID-003モードをそのまま使用 (大切 客先要望)

#if defined(RTQ_FACTORY)	//生産用のソフトのみ、無鑑別はEUR設定でリサイクル、それ以外の国は、無鑑別はID-003の設定を引く次ぐのでこの関数コールしない	
	if( ex_dipsw1 == (DIPSW1_ACCEPT_ALLACC_TEST | DIPSW1_PERFORMANCE_TEST) )
#else
	if(0)
#endif
	{
	//大切 生産でもEUR模擬紙幣を使用しているので、EUR固定にかえてもいいかも
	//1無鑑別のテストモード - 強制的にその国のDenomi順でリサイクル設定
		for(cnt = 0; cnt < unit_type; cnt++)
		{
			/* 金種の選定 */
			switch(cnt)
			{
			case	0:		/* 5 Euro */
					ptr_dt1 = (u8 *)&rcTbl_dt1[RC1_DENOMI][0];
					ptr_dt2 = (u8 *)&rcTbl_dt2[RC1_DENOMI][0];
					break;
			case	1:		/* 10 Euro */
					ptr_dt1 = (u8 *)&rcTbl_dt1[RC2_DENOMI][0];
					ptr_dt2 = (u8 *)&rcTbl_dt2[RC2_DENOMI][0];
					break;
			case	2:		/* 20 Euro */
					ptr_dt1 = (u8 *)&rcTbl_dt1[RC3_DENOMI][0];
					ptr_dt2 = (u8 *)&rcTbl_dt2[RC3_DENOMI][0];
					break;
			case	3:		/* 50 Euro */
					ptr_dt1 = (u8 *)&rcTbl_dt1[RC4_DENOMI][0];
					ptr_dt2 = (u8 *)&rcTbl_dt2[RC4_DENOMI][0];
					break;
			default:
					break;
			}
			/* リサイクル金種を確定させる */
			RecycleSettingInfo.DenomiInfo[cnt].Data1 = *ptr_dt1;
			RecycleSettingInfo.DenomiInfo[cnt].Data2 = *ptr_dt2;
			RecycleSettingInfo.DenomiInfo[cnt].BoxNumber = cnt + 1;
			RecycleSettingInfo.DenomiInfo[cnt].RecycleLimit = 30;

			/* 紙幣情報を設定 */
			rc_twin_set_bill_info(cnt);
		}
	}
	else if(
	//	(ex_rc_dip_sw == 0x00)
	//	&&
		(	
		ex_dipsw1 == (DIPSW1_RC_AGING_TEST | DIPSW1_PERFORMANCE_TEST)
		||
		ex_dipsw1 == (DIPSW1_RC_AGING_FACTORY | DIPSW1_PERFORMANCE_TEST)
		)
	)
	{
	//大切
	//2エージングモード(後ろのDIP-SW OFF) - 強制的に全て EUR10 生産で使用
		for(cnt = 0; cnt < unit_type; cnt++)
		{
			/* リサイクル金種を確定させる */
			RecycleSettingInfo.DenomiInfo[cnt].Data1 = 0x04;
			RecycleSettingInfo.DenomiInfo[cnt].Data2 = 0x00;
			RecycleSettingInfo.DenomiInfo[cnt].BoxNumber = cnt + 1;
			RecycleSettingInfo.DenomiInfo[cnt].RecycleLimit = 30;

			/* 紙幣情報を設定 */
			/* The length of the bill. */
			RecycleSettingInfo.DenomiInfo[cnt].BillInfo.Length = 127;

			/* The minimum permission by the length of the bil. */
			RecycleSettingInfo.DenomiInfo[cnt].BillInfo.Min = 123;

			/* The maximum permission by the length of the bil. */
			RecycleSettingInfo.DenomiInfo[cnt].BillInfo.Max = 131;

			/* 払出し用のエスクロコードを設定する */
			RecycleSettingInfo.DenomiInfo[cnt].PayoutCode = 0x63;
		}
	}
	else
	{
	//基本使用されていない、削除でもいいかも
	#if 0
	//RC側のDIP-SWでリサイクル金種設定
		switch(ex_rc_dip_sw)
		{
		case	0x01:	/*  */
				ptr_dt1 = (u8 *)&rcTbl_dt1[RC1_DENOMI][0];
				ptr_dt2 = (u8 *)&rcTbl_dt2[RC1_DENOMI][0];
				break;
		case	0x02:
				ptr_dt1 = (u8 *)&rcTbl_dt1[RC2_DENOMI][0];
				ptr_dt2 = (u8 *)&rcTbl_dt2[RC2_DENOMI][0];
				break;
		case	0x04:
				ptr_dt1 = (u8 *)&rcTbl_dt1[RC3_DENOMI][0];
				ptr_dt2 = (u8 *)&rcTbl_dt2[RC3_DENOMI][0];
				break;
		case	0x08:
				ptr_dt1 = (u8 *)&rcTbl_dt1[RC4_DENOMI][0];
				ptr_dt2 = (u8 *)&rcTbl_dt2[RC4_DENOMI][0];
				break;
		case	0x10:
				ptr_dt1 = (u8 *)&rcTbl_dt1[RC5_DENOMI][0];
				ptr_dt2 = (u8 *)&rcTbl_dt2[RC5_DENOMI][0];
				break;
		case	0x20:
				ptr_dt1 = (u8 *)&rcTbl_dt1[RC6_DENOMI][0];
				ptr_dt2 = (u8 *)&rcTbl_dt2[RC6_DENOMI][0];
				break;
		case	0x40:
				ptr_dt1 = (u8 *)&rcTbl_dt1[RC7_DENOMI][0];
				ptr_dt2 = (u8 *)&rcTbl_dt2[RC7_DENOMI][0];
				break;
		case	0x80:
				ptr_dt1 = (u8 *)&rcTbl_dt1[RC8_DENOMI][0];
				ptr_dt2 = (u8 *)&rcTbl_dt2[RC8_DENOMI][0];
				break;
		default:
				ptr_dt1 = (u8 *)&rcTbl_dt1[RC1_DENOMI][0];
				ptr_dt2 = (u8 *)&rcTbl_dt2[RC1_DENOMI][0];
				break;
		}

		for(cnt = 0; cnt < unit_type; cnt++)
		{
			/* リサイクル金種を確定させる */
			RecycleSettingInfo.DenomiInfo[cnt].Data1 = *ptr_dt1;
			RecycleSettingInfo.DenomiInfo[cnt].Data2 = *ptr_dt2;
			RecycleSettingInfo.DenomiInfo[cnt].BoxNumber = cnt + 1;
			RecycleSettingInfo.DenomiInfo[cnt].RecycleLimit = 30;

			/* 紙幣情報を設定 */
			rc_twin_set_bill_info(cnt);
		}
		/* copy to backup area */
		memcpy((u8 *)&RecycleSettingInfo_bk.DenomiInfo[0], (u8 *)&RecycleSettingInfo.DenomiInfo[0], sizeof(RecycleSettingInfo));
	#endif
	}
}


/*******************************************************************************************
* ﾓｼﾞｭｰﾙ概要	:	リサイクル金種設定処理(PC Application)
*-------------------------------------------------------------------------------------------
* 宣言			:	void rc_twin_set_usb_test_denomi(void)
*-------------------------------------------------------------------------------------------
* 機能			:	PC Applicationからの指定されたリサイクル金種を設定する
*-------------------------------------------------------------------------------------------
* 引数			:	UB kinshu		設定金種
*					UB cnt			RC No.
*-------------------------------------------------------------------------------------------
* 戻り値       	:	なし
*-------------------------------------------------------------------------------------------
* 注意事項		:
*******************************************************************************************/
void is_recycle_set_usb_test_denomi(u8 kinshu, u8 cnt)
{
	u8	*ptr_dt1, *ptr_dt2;

	/* 金種の選定 */
#if 1	/* '19-01-15 */
	ptr_dt1 = (u8 *)&rcTbl_dt1[kinshu][0];
	ptr_dt2 = (u8 *)&rcTbl_dt2[kinshu][0];
#else
	switch(kinshu)
	{
	case	0x05:	/* 5 Euro */
			ptr_dt1 = (u8 *)&rcTbl_dt1[0][0];
			ptr_dt2 = (u8 *)&rcTbl_dt2[0][0];
			break;
	case	0x0A:	/* 10 Euro */
			ptr_dt1 = (u8 *)&rcTbl_dt1[1][0];
			ptr_dt2 = (u8 *)&rcTbl_dt2[1][0];
			break;
	case	0x14:	/* 20 Euro */
			ptr_dt1 = (u8 *)&rcTbl_dt1[2][0];
			ptr_dt2 = (u8 *)&rcTbl_dt2[2][0];
			break;
	case	0x32:	/* 50 Euro */
			ptr_dt1 = (u8 *)&rcTbl_dt1[3][0];
			ptr_dt2 = (u8 *)&rcTbl_dt2[3][0];
			break;
	case	0x64:	/* 100 Euro */
			ptr_dt1 = (u8 *)&rcTbl_dt1[4][0];
			ptr_dt2 = (u8 *)&rcTbl_dt2[4][0];
			break;
	case	0xC8:	/* 200 Euro */
			ptr_dt1 = (u8 *)&rcTbl_dt1[5][0];
			ptr_dt2 = (u8 *)&rcTbl_dt2[5][0];
			break;
	case	0xF4:	/* 500 Euro */
			ptr_dt1 = (u8 *)&rcTbl_dt1[6][0];
			ptr_dt2 = (u8 *)&rcTbl_dt2[6][0];
			break;
	default:
			break;
	}
#endif

	/* リサイクル金種を確定させる */
	RecycleSettingInfo.DenomiInfo[cnt].Data1 = *ptr_dt1;
	RecycleSettingInfo.DenomiInfo[cnt].Data2 = *ptr_dt2;
	RecycleSettingInfo.DenomiInfo[cnt].BoxNumber = cnt + 1;

	/* 紙幣情報を設定 */
	rc_twin_set_bill_info(cnt);

	/* copy to backup area */
	memcpy((u8 *)&RecycleSettingInfo_bk.DenomiInfo[0], (u8 *)&RecycleSettingInfo.DenomiInfo[0], sizeof(RecycleSettingInfo));
}


/*********************************************************************//**
 * @brief set recycle bill information
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rc_twin_set_bill_info(u8 cnt)
{
	u8	ii;
	u8	no;
	u8	flag = 0;

	/* 金種が設定されていない場合紙幣情報を初期化 */
	if(RecycleSettingInfo.DenomiInfo[cnt].Data1 == 0 && RecycleSettingInfo.DenomiInfo[cnt].Data2 == 0)
	{
		memset((u8 *)&RecycleSettingInfo.DenomiInfo[cnt], 0, 3);
		memset((u8 *)&RecycleSettingInfo_bk.DenomiInfo[cnt], 0, 3);
		memset((u8 *)&RecycleSettingInfo.DenomiInfo[cnt].BillInfo, 0, sizeof(struct _billInfo));
		memset((u8 *)&RecycleSettingInfo_bk.DenomiInfo[cnt].BillInfo, 0, sizeof(struct _billInfo));
		RecycleSettingInfo.DenomiInfo[cnt].PayoutCode = 0;
		RecycleSettingInfo_bk.DenomiInfo[cnt].PayoutCode = 0;
		RecycleSettingInfo.DenomiInfo[cnt].BoxNumber = cnt + 1;
		return;
	}

	for(ii = 0; ii < 100; ii++)
	{
		/* リサイクル設定可能金種の場合 */
		if(rcinhFileno[ii] == 1)
		{
			/* Indexを取得 */
			no = rctblFileno[ii];

			/* Data1の場合 */
			if(no < 8)
			{
				/* 設定金種と一致の場合 */
				if(RecycleSettingInfo.DenomiInfo[cnt].Data1 == rcTbl_dt1[no][0])
				{
					flag = 1;
					break;
				}
			}
			/* Data2の場合 */
			else
			{
				/* 設定金種と一致の場合 */
				if(RecycleSettingInfo.DenomiInfo[cnt].Data2 == rcTbl_dt2[no - 8][0])
				{
					flag = 1;
					break;
				}
			}
		}
	}

	/* 該当金種が見つからない場合はリターン! */
	if(flag == 0)
	{
		return;
	}
	/* 該当金種が見つかった場合は紙幣情報を設定する */
	else
	{
		/* Data1の場合 */
		if(no < 8)
		{
			/* The length of the bill. */
			RecycleSettingInfo.DenomiInfo[cnt].BillInfo.Length = rcLength_dt1[no];

			/* The minimum permission by the length of the bil. */
			RecycleSettingInfo.DenomiInfo[cnt].BillInfo.Min = rcLength_dt1[no] - rcMin_dt1[no];

			/* The maximum permission by the length of the bil. */
			RecycleSettingInfo.DenomiInfo[cnt].BillInfo.Max = rcLength_dt1[no] + rcMax_dt1[no];

			/* 払出し用のエスクロコードを設定する */
			RecycleSettingInfo.DenomiInfo[cnt].PayoutCode = rcTbl_dt1[no][1];

			/* 上限枚数を設定 */
//			RecycleSettingInfo.DenomiInfo[cnt].RecycleLimit = 30;
		}
		/* Data2の場合 */
		else
		{
			/* The length of the bill. */
			RecycleSettingInfo.DenomiInfo[cnt].BillInfo.Length = rcLength_dt2[no - 8];

			/* The minimum permission by the length of the bil. */
			RecycleSettingInfo.DenomiInfo[cnt].BillInfo.Min = rcLength_dt2[no - 8] - rcMin_dt2[no - 8];

			/* The maximum permission by the length of the bil. */
			RecycleSettingInfo.DenomiInfo[cnt].BillInfo.Max = rcLength_dt2[no - 8] + rcMax_dt2[no - 8];

			/* 払出し用のエスクロコードを設定する */
			RecycleSettingInfo.DenomiInfo[cnt].PayoutCode = rcTbl_dt2[no - 8][1];

//			RecycleSettingInfo.DenomiInfo[cnt].RecycleLimit = 30;
		}
	}

	/* Recycle box No.の確認 */
	if(RecycleSettingInfo.DenomiInfo[cnt].BoxNumber == 0)
	{
		/* Recycle box No.が"0"の場合はNo.を設定する */
		RecycleSettingInfo.DenomiInfo[cnt].BoxNumber = cnt + 1;
	}
}


/*********************************************************************//**
 * @brief judge rc or cash box
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void is_recycle_denomi_check(void) //動作中に呼び出されていて、テストモードと通常モードの両方で呼ばれているので注意 //2箇所で使用 Refill modeでリサイクル対象か確認、受け取りのStackコマンド受信後
{
	u8	no;

	if(ex_main_test_no == TEST_RC_AGING || ex_main_test_no == TEST_RC_AGING_FACTORY)
	{
		return;
	}

	/* 受取テスト(識別有)もしくはI/Fモード */
	if( 
		(
		(ex_dipsw1 == DIPSW1_ACCEPT_TEST) //修正した ok 
		&&
		(is_test_mode())
		)

	|| ex_operating_mode == OPERATING_MODE_NORMAL)
	{
		if(ex_validation.denomi == BAR_INDX)
		{
			OperationDenomi.unit_bk = RC_CASH_BOX;
		}
		else
		{
			if(check_bill_rc_inh())
			{
				OperationDenomi.unit_bk = RC_CASH_BOX;
			}
			else
			{
				/* check bill denomi */
				no = check_bill_denomi();

				if(no < 8)
				{
					if(rcTbl_dt1[no][0] == 0x00)
					{
						OperationDenomi.unit_bk = RC_CASH_BOX;
					}
					else
					{
						if(RecycleSettingInfo.DenomiInfo[0].Data1 == rcTbl_dt1[no][0] && (ex_rc_enable & 0x01) != 0)
						{
							OperationDenomi.unit_bk = RC_TWIN_DRUM1;
						}
						else if(RecycleSettingInfo.DenomiInfo[1].Data1 == rcTbl_dt1[no][0] && (ex_rc_enable & 0x02) != 0)
						{
							OperationDenomi.unit_bk = RC_TWIN_DRUM2;
						}
						else if(RecycleSettingInfo.DenomiInfo[2].Data1 == rcTbl_dt1[no][0] && (ex_rc_enable & 0x04) != 0)
						{
							OperationDenomi.unit_bk = RC_QUAD_DRUM1;
						}
						else if(RecycleSettingInfo.DenomiInfo[3].Data1 == rcTbl_dt1[no][0] && (ex_rc_enable & 0x08) != 0)
						{
							OperationDenomi.unit_bk = RC_QUAD_DRUM2;
						}
						else
						{
							OperationDenomi.unit_bk = RC_CASH_BOX;
						}
					}
				}
				else
				{
					if(rcTbl_dt2[no - 8][0] == 0x00)
					{
						OperationDenomi.unit_bk = RC_CASH_BOX;
					}
					else
					{
						if(RecycleSettingInfo.DenomiInfo[0].Data2 == rcTbl_dt2[no - 8][0] && (ex_rc_enable & 0x01) != 0)
						{
							OperationDenomi.unit_bk = RC_TWIN_DRUM1;
						}
						else if(RecycleSettingInfo.DenomiInfo[1].Data2 == rcTbl_dt2[no - 8][0] && (ex_rc_enable & 0x02) != 0)
						{
							OperationDenomi.unit_bk = RC_TWIN_DRUM2;
						}
						else if(RecycleSettingInfo.DenomiInfo[2].Data2 == rcTbl_dt2[no - 8][0] && (ex_rc_enable & 0x04) != 0)
						{
							OperationDenomi.unit_bk = RC_QUAD_DRUM1;
						}
						else if(RecycleSettingInfo.DenomiInfo[3].Data2 == rcTbl_dt2[no - 8][0] && (ex_rc_enable & 0x08) != 0)
						{
							OperationDenomi.unit_bk = RC_QUAD_DRUM2;
						}
						else
						{
							OperationDenomi.unit_bk = RC_CASH_BOX;
						}
					}
				}
			}
		}
	}
	/* それ以外 */
	else
	{
	/* 受取テスト(識別無) *//*長さで判定 */
		if(ex_validation.denomi == BAR_INDX)
		{
			OperationDenomi.unit_bk = RC_CASH_BOX;
		}
		else
		{
			/* check bill length */
			no = check_bill_length();

			switch(no)
			{
			case	0:
					OperationDenomi.unit_bk = RC_TWIN_DRUM1;
					break;
			case	1:
					OperationDenomi.unit_bk = RC_TWIN_DRUM2;
					break;
			case	2:
					OperationDenomi.unit_bk = RC_QUAD_DRUM1;
					break;
			case	3:
					OperationDenomi.unit_bk = RC_QUAD_DRUM2;
					break;
			default:
					OperationDenomi.unit_bk = RC_CASH_BOX;
					break;
			}
		}
	}

	/* リサイクルボックスFull状態の確認 */
	if(check_recycle_full_sensor() != TRUE || check_recycle_limit_count() != TRUE)
	{
		OperationDenomi.unit_bk = RC_CASH_BOX;
	}

	/* Skew紙幣の処理 */
//#if defined(RC_SKEW_DETECT)		/* '19-07-05 */
	if((check_recycle_skew_detect() == TRUE) && 
		ex_main_emergency_flag == 0 && 
		ex_validation.denomi != BAR_INDX && 
		OperationDenomi.unit_bk != RC_CASH_BOX)
//#else
//	if(ex_rc_skew_detect != 0 && ex_main_emergency_flag == 0)
//#endif
	{
		OperationDenomi.unit_bk = RC_CASH_BOX;

#if 1//#ifdef _ENABLE_JDL	/* '20-01-19 */
		jdl_reject(REJECT_CODE_NO_RECYCLE, ex_validation.start, ex_validation.denomi, 0, 0, 0, 0);
//		s_jdl_ins_flag = 1;
#endif /* _ENABLE_JDL */
	}

	/* drum enable check */
	if(!(check_drum_enable(OperationDenomi.unit_bk)))
	{
		OperationDenomi.unit_bk = RC_CASH_BOX;
	}

	/* 収納先保存 */
	set_recovery_unit( OperationDenomi.unit_bk, OperationDenomi.unit_bk );
}


/*********************************************************************//**
 * @brief confirm denomination to use bill denomi
 * @param[in]	None
 * @return 		denomination no.
 **********************************************************************/
u8 check_bill_denomi(void)
{
	return(rctblFileno[ex_validation.denomi]);
}


/*********************************************************************//**
 * @brief confirm denomination to use bill denomi
 * @param[in]	None
 * @return 		true : rc inhibit
				false: not rc inhibit
 **********************************************************************/
bool check_bill_rc_inh(void)
{
	if(rcinhFileno[ex_validation.denomi] == 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*********************************************************************//**
 * @brief confirm denomination to use bill length(for test)
 * @param[in]	None
 * @return 		denomination no.
 **********************************************************************/
u8 check_bill_length(void)
{
	u8 ii;
	u8 denomi = 0xff;
	u8 model;
	u8 lengh_mm;

	#if 0	//2024-12-24 移動した、compare();　直後に移動　通常の回収庫への搬送も必要なので、　ex_validation.bill_lengthに上書き保持する。　
	lengh_mm = (u8)(ex_validation.bill_length * PITCH / 2 + 0.5);
	#endif

#if 0
	for(ii = 0; ii < 7; ii++)
	{
		if((rcLength_dt1[ii] - 3 <= ex_validation.bill_length) && (rcLength_dt1[ii] + 3 >= ex_validation.bill_length))
		{
			denomi = ii;
			break;
		}
	}
#else
	if(is_quad_model())
	{
		model = 4;
	}
	else
	{
		model = 2;
	}

	for(ii = 0; ii < model; ii++)
	{
	#if 1 //2025-01-20 無鑑別テストモードでもリサイクラへ搬送させる
		if((RecycleSettingInfo.DenomiInfo[ii].BillInfo.Min <= (u8)ex_validation.bill_length) && 
			(RecycleSettingInfo.DenomiInfo[ii].BillInfo.Max >= (u8)ex_validation.bill_length))
	#else
		if((RecycleSettingInfo.DenomiInfo[ii].BillInfo.Min <= lengh_mm) && 
			(RecycleSettingInfo.DenomiInfo[ii].BillInfo.Max >= lengh_mm))
	#endif
		{
			denomi = ii;
			break;
		}
	}
#endif

	return(denomi);
}


//#if defined(RC_SKEW_DETECT)		/* '19-07-05 */
/*********************************************************************//**
 * @brief check recycle skew
 * @param[in]	None
 * @return 		TRUE  : skew(Go to Cash box)
				FALSE : not skew(Go to RC)
 **********************************************************************/
u8 check_recycle_skew_detect(void)
{

	if(ex_rc_skew_pulse > rcSkew_dt[ex_validation.denomi])
	{
		return(TRUE);
	}
	else
	{
		return(FALSE);
	}
}
//#endif


/*********************************************************************//**
 * @brief check recycle full sensor
 * @param[in]	None
 * @return 		TRUE  : not full
				FALSE : full
 **********************************************************************/
u8 check_recycle_full_sensor(void)////動作中に呼び出されていて、テストモードと通常モードの両方で呼ばれているので注意 //ok 2024-12-25 is_recycle_denomi_check(void) でのみ使用でその関数は 2箇所で使用 Refill modeでリサイクル対象か確認、受け取りのStackコマンド受信後
{
	UB	result = TRUE;

	switch(OperationDenomi.unit_bk)
	{

	case	RC_TWIN_DRUM1:
			if(is_twin_drum1_full_check())
			{
				if(is_quad_model())
				{
					/* RC-Twin Drum1とRC-Twin Durm2が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					if(((RecycleSettingInfo.DenomiInfo[0].Data1 == RecycleSettingInfo.DenomiInfo[1].Data1) && RecycleSettingInfo.DenomiInfo[0].Data1 != 0 && !(is_twin_drum2_full_check()))
					|| ((RecycleSettingInfo.DenomiInfo[0].Data2 == RecycleSettingInfo.DenomiInfo[1].Data2) && RecycleSettingInfo.DenomiInfo[0].Data2 != 0 && !(is_twin_drum2_full_check())))
					{
						OperationDenomi.unit_bk = RC_TWIN_DRUM2;
					}
					/* RC-Twin Drum1とRC-Quad Durm1が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					else if(((RecycleSettingInfo.DenomiInfo[0].Data1 == RecycleSettingInfo.DenomiInfo[2].Data1) && RecycleSettingInfo.DenomiInfo[0].Data1 != 0 && !(is_quad_drum1_full_check()))
					||		((RecycleSettingInfo.DenomiInfo[0].Data2 == RecycleSettingInfo.DenomiInfo[2].Data2) && RecycleSettingInfo.DenomiInfo[0].Data2 != 0 && !(is_quad_drum1_full_check())))
					{
						OperationDenomi.unit_bk = RC_QUAD_DRUM1;
					}
					/* RC-Twin Drum1とRC-Quad Durm2が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					else if(((RecycleSettingInfo.DenomiInfo[0].Data1 == RecycleSettingInfo.DenomiInfo[3].Data1) && RecycleSettingInfo.DenomiInfo[0].Data1 != 0 && !(is_quad_drum2_full_check()))
					||		((RecycleSettingInfo.DenomiInfo[0].Data2 == RecycleSettingInfo.DenomiInfo[3].Data2) && RecycleSettingInfo.DenomiInfo[0].Data2 != 0 && !(is_quad_drum2_full_check())))
					{
						OperationDenomi.unit_bk = RC_QUAD_DRUM2;
					}
					else
					{
						result = FALSE;
					}
				}
				else
				{
					/* RC-Twin Drum1とRC-Twin Durm2が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					if(((RecycleSettingInfo.DenomiInfo[0].Data1 == RecycleSettingInfo.DenomiInfo[1].Data1) && RecycleSettingInfo.DenomiInfo[0].Data1 != 0 && !(is_twin_drum2_full_check()))
					|| ((RecycleSettingInfo.DenomiInfo[0].Data2 == RecycleSettingInfo.DenomiInfo[1].Data2) && RecycleSettingInfo.DenomiInfo[0].Data2 != 0 && !(is_twin_drum2_full_check())))
					{
						OperationDenomi.unit_bk = RC_TWIN_DRUM2;
					}
					else
					{
						result = FALSE;
					}
				}
			}
			break;
	case	RC_TWIN_DRUM2:
			if(is_twin_drum2_full_check())
			{
				if(is_quad_model())
				{
					/* RC-Twin Drum2とRC-Twin Durm1が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					if(((RecycleSettingInfo.DenomiInfo[1].Data1 == RecycleSettingInfo.DenomiInfo[0].Data1) && RecycleSettingInfo.DenomiInfo[1].Data1 != 0 && !(is_twin_drum1_full_check()))
					|| ((RecycleSettingInfo.DenomiInfo[1].Data2 == RecycleSettingInfo.DenomiInfo[0].Data2) && RecycleSettingInfo.DenomiInfo[1].Data2 != 0 && !(is_twin_drum1_full_check())))
					{
						OperationDenomi.unit_bk = RC_TWIN_DRUM1;
					}
					/* RC-Twin Drum2とRC-Quad Durm1が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					else if(((RecycleSettingInfo.DenomiInfo[1].Data1 == RecycleSettingInfo.DenomiInfo[2].Data1) && RecycleSettingInfo.DenomiInfo[1].Data1 != 0 && !(is_quad_drum1_full_check()))
					||		((RecycleSettingInfo.DenomiInfo[1].Data2 == RecycleSettingInfo.DenomiInfo[2].Data2) && RecycleSettingInfo.DenomiInfo[1].Data2 != 0 && !(is_quad_drum1_full_check())))
					{
						OperationDenomi.unit_bk = RC_QUAD_DRUM1;
					}
					/* RC-Twin Drum1とRC-Quad Durm2が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					else if(((RecycleSettingInfo.DenomiInfo[1].Data1 == RecycleSettingInfo.DenomiInfo[3].Data1) && RecycleSettingInfo.DenomiInfo[1].Data1 != 0 && !(is_quad_drum2_full_check()))
					||		((RecycleSettingInfo.DenomiInfo[1].Data2 == RecycleSettingInfo.DenomiInfo[3].Data2) && RecycleSettingInfo.DenomiInfo[1].Data2 != 0 && !(is_quad_drum2_full_check())))
					{
						OperationDenomi.unit_bk = RC_QUAD_DRUM2;
					}
					else
					{
						result = FALSE;
					}
				}
				else
				{
					/* RC-Twin Drum2とRC-Twin Durm1が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					if(((RecycleSettingInfo.DenomiInfo[1].Data1 == RecycleSettingInfo.DenomiInfo[0].Data1) && RecycleSettingInfo.DenomiInfo[1].Data1 != 0 && !(is_twin_drum1_full_check()))
					|| ((RecycleSettingInfo.DenomiInfo[1].Data2 == RecycleSettingInfo.DenomiInfo[0].Data2) && RecycleSettingInfo.DenomiInfo[1].Data2 != 0 && !(is_twin_drum1_full_check())))
					{
						OperationDenomi.unit_bk = RC_TWIN_DRUM1;
					}
					else
					{
						result = FALSE;
					}
				}
			}
			break;
	case	RC_QUAD_DRUM1:
			if(is_quad_drum1_full_check())
			{
				if(is_quad_model())
				{
					/* RC-Quad Drum1とRC-Twin Durm1が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					if(((RecycleSettingInfo.DenomiInfo[2].Data1 == RecycleSettingInfo.DenomiInfo[0].Data1) && RecycleSettingInfo.DenomiInfo[2].Data1 != 0 && !(is_twin_drum1_full_check()))
					|| ((RecycleSettingInfo.DenomiInfo[2].Data2 == RecycleSettingInfo.DenomiInfo[0].Data2) && RecycleSettingInfo.DenomiInfo[2].Data2 != 0 && !(is_twin_drum1_full_check())))
					{
						OperationDenomi.unit_bk = RC_TWIN_DRUM1;
					}
					/* RC-Quad Drum1とRC-Twin Durm2が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					else if(((RecycleSettingInfo.DenomiInfo[2].Data1 == RecycleSettingInfo.DenomiInfo[1].Data1) && RecycleSettingInfo.DenomiInfo[2].Data1 != 0 && !(is_twin_drum2_full_check()))
					||		((RecycleSettingInfo.DenomiInfo[2].Data2 == RecycleSettingInfo.DenomiInfo[1].Data2) && RecycleSettingInfo.DenomiInfo[2].Data2 != 0 && !(is_twin_drum2_full_check())))
					{
						OperationDenomi.unit_bk = RC_TWIN_DRUM2;
					}
					/* RC-Twin Drum1とRC-Quad Durm2が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					else if(((RecycleSettingInfo.DenomiInfo[2].Data1 == RecycleSettingInfo.DenomiInfo[3].Data1) && RecycleSettingInfo.DenomiInfo[2].Data1 != 0 && !(is_quad_drum2_full_check()))
					||		((RecycleSettingInfo.DenomiInfo[2].Data2 == RecycleSettingInfo.DenomiInfo[3].Data2) && RecycleSettingInfo.DenomiInfo[2].Data2 != 0 && !(is_quad_drum2_full_check())))
					{
						OperationDenomi.unit_bk = RC_QUAD_DRUM2;
					}
					else
					{
						result = FALSE;
					}
				}
				else
				{
					result = FALSE;
				}
			}
			break;
	case	RC_QUAD_DRUM2:
			if(is_quad_drum2_full_check())
			{
				if(is_quad_model())
				{
					/* RC-Quad Drum2とRC-Twin Durm1が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					if(((RecycleSettingInfo.DenomiInfo[3].Data1 == RecycleSettingInfo.DenomiInfo[0].Data1) && RecycleSettingInfo.DenomiInfo[3].Data1 != 0 && !(is_twin_drum2_full_check()))
					|| ((RecycleSettingInfo.DenomiInfo[3].Data2 == RecycleSettingInfo.DenomiInfo[0].Data2) && RecycleSettingInfo.DenomiInfo[3].Data2 != 0 && !(is_twin_drum2_full_check())))
					{
						OperationDenomi.unit_bk = RC_TWIN_DRUM1;
					}
					/* RC-Quad Drum2とRC-Twin Durm2が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					else if(((RecycleSettingInfo.DenomiInfo[3].Data1 == RecycleSettingInfo.DenomiInfo[1].Data1) && RecycleSettingInfo.DenomiInfo[3].Data1 != 0 && !(is_twin_drum2_full_check()))
					||		((RecycleSettingInfo.DenomiInfo[3].Data2 == RecycleSettingInfo.DenomiInfo[1].Data2) && RecycleSettingInfo.DenomiInfo[3].Data2 != 0 && !(is_twin_drum2_full_check())))
					{
						OperationDenomi.unit_bk = RC_TWIN_DRUM2;
					}
					/* RC-Quad Drum2とRC-Quad Durm1が同じ金種設定の場合(0x00設定ではなくFullでもない) */
					else if(((RecycleSettingInfo.DenomiInfo[3].Data1 == RecycleSettingInfo.DenomiInfo[2].Data1) && RecycleSettingInfo.DenomiInfo[3].Data1 != 0 && !(is_quad_drum1_full_check()))
					||		((RecycleSettingInfo.DenomiInfo[3].Data2 == RecycleSettingInfo.DenomiInfo[2].Data2) && RecycleSettingInfo.DenomiInfo[3].Data2 != 0 && !(is_quad_drum1_full_check())))
					{
						OperationDenomi.unit_bk = RC_QUAD_DRUM1;
					}
					else
					{
						result = FALSE;
					}
				}
				else
				{
					result = FALSE;
				}
			}
			break;
	default:
			break;
	}

	return(result);
}


/*********************************************************************//**
 * @brief check recycle limit count
 * @param[in]	None
 * @return 		TRUE  : not full
				FALSE : full
 **********************************************************************/
u8 check_recycle_limit_count(void) //ok UBA500現状常にTRUEで返しているので意味ない
{
	u8	result = TRUE;

#if 0
	if(((ex_dipsw1 & DIPSW1_ACCEPT_TEST) == DIPSW1_ACCEPT_TEST || (ex_dipsw1 & DIPSW1_ACCEPT_ALLACC_TEST) == DIPSW1_ACCEPT_ALLACC_TEST) && ex_rc_dip_sw != 0x00)
	{
		return(result);
	}

	switch(OperationDenomi.unit_bk)
	{
	case	RC_TWIN_DRUM1:
			/* 設定値を上回る場合は収納庫へ搬送先変更 */
			if((RecycleSettingInfo.DenomiInfo[0].RecycleLimit != 0)
			&& (RecycleSettingInfo.DenomiInfo[0].RecycleLimit <= RecycleSettingInfo.DenomiInfo[0].RecycleCurrent))
			{
				result = FALSE;
			}
			break;
	case	RC_TWIN_DRUM2:		/* RC-Twinドラム2 */
			/* 設定値を上回る場合は収納庫へ搬送先変更 */
			if((RecycleSettingInfo.DenomiInfo[1].RecycleLimit != 0)
			&& (RecycleSettingInfo.DenomiInfo[1].RecycleLimit <= RecycleSettingInfo.DenomiInfo[1].RecycleCurrent))
			{
				result = FALSE;
			}
			break;
	case	RC_QUAD_DRUM1:		/* RC-Quadドラム1 */
			/* 設定値を上回る場合は収納庫へ搬送先変更 */
			if((RecycleSettingInfo.DenomiInfo[2].RecycleLimit != 0)
			&& (RecycleSettingInfo.DenomiInfo[2].RecycleLimit <= RecycleSettingInfo.DenomiInfo[2].RecycleCurrent))
			{
				result = FALSE;
			}
			break;
	case	RC_QUAD_DRUM2:		/* RC-Quadドラム2 */
			/* 設定値を上回る場合は収納庫へ搬送先変更 */
			if((RecycleSettingInfo.DenomiInfo[3].RecycleLimit != 0)
			&& (RecycleSettingInfo.DenomiInfo[3].RecycleLimit <= RecycleSettingInfo.DenomiInfo[3].RecycleCurrent))
			{
				result = FALSE;
			}
			break;
	default:
			break;
	}
#endif

	return(result);
}


void is_recycle_aging_accept(void)
{
	if(ex_main_test_no == TEST_RC_AGING_FACTORY)
	{
		if(is_quad_model())
		{
			if(++ex_rc_aging_counter > 200)
			{
			#if defined(QA_TEST_SAFE) || defined(QA_TEST_EMC_EMI)	//2025-02-20
			//製品安全評価は無限に動かし続ける
			#else
				ex_rc_aging_seq = 8;
			#endif
			}
		}
		else
		{
			if(++ex_rc_aging_counter > 100)
			{
			#if defined(QA_TEST_SAFE) || defined(QA_TEST_EMC_EMI)	//2025-02-20
			//製品安全評価は無限に動かし続ける
			#else
					ex_rc_aging_seq = 8;
			#endif
			}
		}
	}

	switch(ex_rc_aging_seq)
	{
	case	0:		/* In --> Out  (RC-Twin drum1) */
#if 0
		// hsuzuki おそらく外部カウンター
			GPIOPinWrite(SOC_GPIO1_REG, 15, GPIO_PIN_HIGH);
			dly_tsk(50);
			GPIOPinWrite(SOC_GPIO1_REG, 15, GPIO_PIN_LOW);
#endif
		#if defined(QA_TEST_AZ)
			Gpio_out( GPIO_28, 0 );
			dly_tsk(100);
			Gpio_out( GPIO_28, 1 );
		#endif
			OperationDenomi.unit = RC_TWIN_DRUM1;
			OperationDenomi.count = 1;

			ex_rc_aging_seq = 1;
			break;
	case	1:
			break;
	case	2:		/* In  --> Out (RC-Twin drum2) */
			OperationDenomi.unit = RC_TWIN_DRUM2;
			OperationDenomi.count = 1;

			ex_rc_aging_seq = 3;
			break;
	case	3:
			break;
	case	4:		/* In  --> Out  (RC-Quad drum1) */
			OperationDenomi.unit = RC_QUAD_DRUM1;
			OperationDenomi.count = 1;

			ex_rc_aging_seq = 5;
			break;
	case	5:
			break;
	case	6:		/* In  --> Out  (RC-Quad drum2) */
			OperationDenomi.unit = RC_QUAD_DRUM2;
			OperationDenomi.count = 1;

			ex_rc_aging_seq = 7;
			break;
	case	7:
			break;
	default:
			OperationDenomi.unit = RC_CASH_BOX;
			break;
	}
}


void is_recycle_aging_payout(void)
{
	switch(ex_rc_aging_seq)
	{
	case	1:		/* Out --> In  (RC-Twin drum1) */
			OperationDenomi.unit = RC_TWIN_DRUM1;
			OperationDenomi.count = 1;
			OperationDenomi.remain = 1;

			_main_set_mode(MODE1_ENABLE, ENABLE_MODE2_WAIT_REQ);
			_main_send_connection_task(TMSG_CONN_ENABLE, TMSG_SUB_PAYOUT, 0, 0, 0);

			ex_rc_aging_seq = 2;
			break;
	case	3:		/* Out --> In  (RC-Twin drum2) */
			OperationDenomi.unit = RC_TWIN_DRUM2;
			OperationDenomi.count = 1;
			OperationDenomi.remain = 1;

			_main_set_mode(MODE1_ENABLE, ENABLE_MODE2_WAIT_REQ);
			_main_send_connection_task(TMSG_CONN_ENABLE, TMSG_SUB_PAYOUT, 0, 0, 0);

			if(is_quad_model())
			{
				ex_rc_aging_seq = 4;
			}
			else
			{
				ex_rc_aging_seq = 0;
			}
			break;
	case	5:		/* Out --> In   (RC-Quad drum1) */
			OperationDenomi.unit = RC_QUAD_DRUM1;
			OperationDenomi.count = 1;
			OperationDenomi.remain = 1;

			_main_set_mode(MODE1_ENABLE, ENABLE_MODE2_WAIT_REQ);
			_main_send_connection_task(TMSG_CONN_ENABLE, TMSG_SUB_PAYOUT, 0, 0, 0);

			ex_rc_aging_seq = 6;
			break;
	case	7:		/* Out --> In   (RC-Quad drum2) */
			OperationDenomi.unit = RC_QUAD_DRUM2;
			OperationDenomi.count = 1;
			OperationDenomi.remain = 1;

			_main_set_mode(MODE1_ENABLE, ENABLE_MODE2_WAIT_REQ);
			_main_send_connection_task(TMSG_CONN_ENABLE, TMSG_SUB_PAYOUT, 0, 0, 0);

			ex_rc_aging_seq = 0;
			break;
	default:
			break;
	}
}


bool is_quad_model(void)
{
	if(ex_rc_status.sst1A.bit.quad)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool rc_busy_status(void)
{
	if(ex_rc_status.sst1A.bit.busy == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool rc_warning_status(void)
{
	if(ex_rc_status.sst1A.bit.warning == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool rc_initial_status(void)
{
	if(ex_rc_status.sst1A.bit.initial == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_detect_rc_twin(void)
{
	if(ex_rc_status.sst21B.bit.u1_detect == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_detect_rc_quad(void)
{
	if(is_quad_model())
	{
		if(ex_rc_status.sst22B.bit.u2_detect == 1)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return true;
	}
}


bool is_flapper1_head_to_twin_pos(void)
{
	if(ex_rc_status.sst21A.bit.flap1_senA == 1 &&  ex_rc_status.sst21A.bit.flap1_senRC1 == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_flapper1_head_to_box_pos(void)
{
	if(ex_rc_status.sst21A.bit.flap1_senA == 1 && ex_rc_status.sst21A.bit.flap1_senB == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_flapper1_twin_to_box_pos(void)
{
	if(ex_rc_status.sst21A.bit.flap1_senRC1 == 1 && ex_rc_status.sst21A.bit.flap1_senB == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_flapper2_head_to_quad_pos(void)
{
	if(ex_rc_status.sst22A.bit.flap2_senD == 1 && ex_rc_status.sst22A.bit.flap2_senRC2 == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_flapper2_head_to_box_pos(void)
{
	if(ex_rc_status.sst22A.bit.flap2_senC == 1 && ex_rc_status.sst22A.bit.flap2_senD == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_flapper2_quad_to_box_pos(void)
{
	if(ex_rc_status.sst22A.bit.flap2_senRC2 == 1 && ex_rc_status.sst22A.bit.flap2_senC == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_rc_twin_d1_empty(void)
{
	if(ex_rc_status.sst31A.bit.u1_d1_empty)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_rc_twin_d2_empty(void)
{
	if(ex_rc_status.sst31A.bit.u1_d2_empty)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_rc_quad_d1_empty(void)
{
	if(is_quad_model())
	{
		if(ex_rc_status.sst32A.bit.u2_d1_empty)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}


bool is_rc_quad_d2_empty(void)
{
	if(is_quad_model())
	{
		if(ex_rc_status.sst32A.bit.u2_d2_empty)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

u8 is_pre_feed_check(void)
{
	if(ex_pre_feed_after_jam)
	{
		return true;
	}
	else
	{
		if(ex_main_task_mode1 == MODE1_PAYOUT)
		{
			if((OperationDenomi.remain > 1) && (RecycleSettingInfo.DenomiInfo[OperationDenomi.unit - 1].RecycleCurrent > 1))
			{
				return false;
			}
			else
			{
				return true;
			}
		}
		else if(ex_main_task_mode1 == MODE1_COLLECT)
		{
			if(OperationDenomi.count == 1)
			{
				return true;
			}
			else
			{
				if(RecycleSettingInfo.DenomiInfo[OperationDenomi.unit - 1].RecycleCurrent > 1)
				{
					return false;
				}
				else
				{
					return true;
				}
			}
		}
		else
		{
			return true;
		}
	}
}


bool is_detect_rc_jam_check(void)
{
	if(ex_cline_status_tbl.ex_rc_after_jam)
	{
		if(is_quad_model())
		{
			if( (ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) == 0 &&
			    (ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) == 0 &&
				(ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) == 0 &&
				(ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) == 0 &&
				!(SENSOR_APB_IN)									 &&
				!(SENSOR_APB_OUT)									 &&
				!(SENSOR_EXIT))
			{
				/* jam形跡なし */
				ex_cline_status_tbl.ex_rc_after_jam = 0;
				return true;
			}
			else
			{
				/* jam未解除の可能性あり */
				return false;
			}
		}
		else
		{
			if( (ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) == 0 &&
			    (ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) == 0 &&
				!(SENSOR_APB_IN)									 &&
				!(SENSOR_APB_OUT)									 &&
				!(SENSOR_EXIT))
			{
				/* jam形跡なし */
				ex_cline_status_tbl.ex_rc_after_jam = 0;
				return true;
			}
			else
			{
				/* jam未解除の可能性あり */
				return false;
			}
		}
	}
	else
	{
		/* jam形跡なし */
		ex_cline_status_tbl.ex_rc_after_jam = 0;
		return true;
	}
}


//#if defined(RC_INTERNAL_JAM)		/* '19-09-19 */
u8 is_detect_rc_internal_jam(void)
{
	u8 res = FALSE;
	
	if(ex_rc_internal_jam_flag == 0)
	{
		res = TRUE;
	}
	else
	{
		if(ex_rc_internal_jam_flag == RC_TWIN_DRUM1 && !(check_drum_enable(RC_TWIN_DRUM1)))
		{
			/* drum1 disable */
			res = TRUE;
		}
		if(ex_rc_internal_jam_flag == RC_TWIN_DRUM2 && !(check_drum_enable(RC_TWIN_DRUM2)))
		{
			/* drum2 disable */
			res = TRUE;
		}
		if(ex_rc_internal_jam_flag == RC_QUAD_DRUM1 && !(check_drum_enable(RC_QUAD_DRUM1)))
		{
			/* drum3 disable */
			res = TRUE;
		}
		if(ex_rc_internal_jam_flag == RC_QUAD_DRUM2 && !(check_drum_enable(RC_QUAD_DRUM2)))
		{
			/* drum4 disable */
			res = TRUE;
		}
	}

	return(res);
}


u8 is_detect_rc_internal_jam_initial(void)
{
	u8 res = FALSE;
	
	if(ex_rc_internal_jam_flag_bk == 0)
	{
		res = TRUE;
	}
	else
	{
		if(ex_rc_internal_jam_flag_bk == RC_TWIN_DRUM1 && !(check_drum_enable(RC_TWIN_DRUM1)))
		{
			/* drum1 disable */
			res = TRUE;
		}
		if(ex_rc_internal_jam_flag_bk == RC_TWIN_DRUM2 && !(check_drum_enable(RC_TWIN_DRUM2)))
		{
			/* drum2 disable */
			res = TRUE;
		}
		if(ex_rc_internal_jam_flag_bk == RC_QUAD_DRUM1 && !(check_drum_enable(RC_QUAD_DRUM1)))
		{
			/* drum3 disable */
			res = TRUE;
		}
		if(ex_rc_internal_jam_flag_bk == RC_QUAD_DRUM2 && !(check_drum_enable(RC_QUAD_DRUM2)))
		{
			/* drum4 disable */
			res = TRUE;
		}
	}

	return(res);
}
//#endif

//#if defined(UBA_RS)
u8 is_rs_mode_remain_note_check()
{
	if(is_rc_rs_unit())
	{
		if(ex_recovery_info.step == RECOVERY_STEP_PAYOUT_DRUM
		|| ex_recovery_info.step == RECOVERY_STEP_PAYOUT_TRANSPORT
		|| ex_recovery_info.step == RECOVERY_STEP_PAYOUT_POS1
		|| ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_POS7
		|| ex_recovery_info.step == RECOVERY_STEP_PAYOUT_RS_ESCROW
		|| ex_recovery_info.step == RECOVERY_STEP_PAYOUT_VALID
		|| ex_recovery_info.step == RECOVERY_STEP_SWITCHBACK_TRANSPORT)
		{
			if((SENSOR_CENTERING)
			|| !(SENSOR_VALIDATION_OFF)
			|| (SENSOR_APB_IN)
			|| (SENSOR_APB_OUT)
			|| (SENSOR_EXIT))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false; // RS unit not exist
	}
}

bool is_payout_remain_check(void)
{
	if(OperationDenomi.remain == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool is_rs_remain_note(void) //RSのみの機能
{
	if(ex_rs_payout_remain_flag == RS_NOTE_REMAIN_CHECK)
	{
		if((is_rc_rs_unit()) && (RS_REMAIN_ON))
		{
			/* remain note */
			return(true);
		}
		else
		{
			/* not remain note */
			return(false);
		}
	}
	else
	{
		/* not remain note */
		return(false);
	}
}
//#endif // UBA_RS

bool is_rc_error_check(void)   //ok 2024-12-25
{
	if(ex_main_task_mode1_alarm_back == MODE1_ALARM
	&& (ex_main_task_mode2_alarm_back == ALARM_MODE2_CONFIRM_RC_UNIT || ex_main_task_mode2_alarm_back == ALARM_MODE2_RC_ERROR))
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_twin_drum1_full_check(void)  //ok 2024-12-25
{
	if(
		((ex_dipsw1 & DIPSW1_ACCEPT_TEST) == DIPSW1_ACCEPT_TEST || (ex_dipsw1 & DIPSW1_ACCEPT_ALLACC_TEST) == DIPSW1_ACCEPT_ALLACC_TEST)
		 && ex_rc_dip_sw != 0x00) //ok
	//条件設定は若干よくないがUBA500に合わせてこのままにしておく
	//通常動作とテストモードの動作時に呼び出される
	//通常動作でDIP-SWを設定すると、ex_dipsw1の条件を満たす可能性あり、ex_rc_dip_sw の条件があるのでOK	 
	{
		if(ex_rc_status.sst31A.bit.u1_d1_full)
		{
			// full
			return true;
		}
		else
		{
			// not full
			return false;
		}
	}
	else
	{
		/* 上限未設定 */
		if(RecycleSettingInfo.DenomiInfo[0].RecycleLimit == 0)
		{
			if(RecycleSettingInfo.DenomiInfo[0].RecycleCurrent >= 30 || ex_rc_status.sst31A.bit.u1_d1_full)
			{
				// full
				return true;
			}
			else
			{
				// not full
				return false;
			}
		}
		/* 上限設定 */
		else
		{
			if((RecycleSettingInfo.DenomiInfo[0].RecycleLimit <= RecycleSettingInfo.DenomiInfo[0].RecycleCurrent) || ex_rc_status.sst31A.bit.u1_d1_full)
			{
				// full
				return true;
			}
			else
			{
				// not full
				return false;
			}
		}
	}
}


bool is_twin_drum2_full_check(void) //ok 2024-12-25
{
	if(((ex_dipsw1 & DIPSW1_ACCEPT_TEST) == DIPSW1_ACCEPT_TEST || (ex_dipsw1 & DIPSW1_ACCEPT_ALLACC_TEST) == DIPSW1_ACCEPT_ALLACC_TEST) && ex_rc_dip_sw != 0x00)
	{
		if(ex_rc_status.sst31A.bit.u1_d2_full)
		{
			// full
			return true;
		}
		else
		{
			// not full
			return false;
		}
	}
	else
	{
		/* 上限未設定 */
		if(RecycleSettingInfo.DenomiInfo[1].RecycleLimit == 0)
		{
			if(RecycleSettingInfo.DenomiInfo[1].RecycleCurrent >= 30 || ex_rc_status.sst31A.bit.u1_d2_full)
			{
				// full
				return true;
			}
			else
			{
				// not full
				return false;
			}
		}
		/* 上限設定 */
		else
		{
			if((RecycleSettingInfo.DenomiInfo[1].RecycleLimit <= RecycleSettingInfo.DenomiInfo[1].RecycleCurrent) || ex_rc_status.sst31A.bit.u1_d2_full)
			{
				// full
				return true;
			}
			else
			{
				// not full
				return false;
			}
		}
	}
}


bool is_quad_drum1_full_check(void) //ok 2024-12-25
{
	if(((ex_dipsw1 & DIPSW1_ACCEPT_TEST) == DIPSW1_ACCEPT_TEST || (ex_dipsw1 & DIPSW1_ACCEPT_ALLACC_TEST) == DIPSW1_ACCEPT_ALLACC_TEST) && ex_rc_dip_sw != 0x00)
	{
		if(ex_rc_status.sst32A.bit.u2_d1_full)
		{
			// full
			return true;
		}
		else
		{
			// not full
			return false;
		}
	}
	else
	{
		/* 上限未設定 */
		if(RecycleSettingInfo.DenomiInfo[2].RecycleLimit == 0)
		{
			if(RecycleSettingInfo.DenomiInfo[2].RecycleCurrent >= 30 || ex_rc_status.sst32A.bit.u2_d1_full)
			{
				// full
				return true;
			}
			else
			{
				// not full
				return false;
			}
		}
		/* 上限設定 */
		else
		{
			if((RecycleSettingInfo.DenomiInfo[2].RecycleLimit <= RecycleSettingInfo.DenomiInfo[2].RecycleCurrent) || ex_rc_status.sst32A.bit.u2_d1_full)
			{
				// full
				return true;
			}
			else
			{
				// not full
				return false;
			}
		}
	}
}


bool is_quad_drum2_full_check(void) //ok 2024-12-25
{
	if(((ex_dipsw1 & DIPSW1_ACCEPT_TEST) == DIPSW1_ACCEPT_TEST || (ex_dipsw1 & DIPSW1_ACCEPT_ALLACC_TEST) == DIPSW1_ACCEPT_ALLACC_TEST) && ex_rc_dip_sw != 0x00)
	{
		if(ex_rc_status.sst32A.bit.u2_d2_full)
		{
			// full
			return true;
		}
		else
		{
			// not full
			return false;
		}
	}
	else
	{
		/* 上限未設定 */
		if(RecycleSettingInfo.DenomiInfo[3].RecycleLimit == 0)
		{
			if(RecycleSettingInfo.DenomiInfo[3].RecycleCurrent >= 30 || ex_rc_status.sst32A.bit.u2_d2_full)
			{
				// full
				return true;
			}
			else
			{
				// not full
				return false;
			}
		}
		/* 上限設定 */
		else
		{
			if((RecycleSettingInfo.DenomiInfo[3].RecycleLimit <= RecycleSettingInfo.DenomiInfo[3].RecycleCurrent) || ex_rc_status.sst32A.bit.u2_d2_full)
			{
				// full
				return true;
			}
			else
			{
				// not full
				return false;
			}
		}
	}
}


bool is_rc_serial_no_check(void)
{
	u8 ret1 = 0;
	u8 ret2 = 0;

	if(is_quad_model())
	{
		/* RC-Twinの確認 */
		ret1 = memcmp((u8 *)&read_mente_serailno_data[1].serial_no[0], (u8 *)&read_mente_serailno_data_bk[1].serial_no[0], 12);

		/* RC-Quadの確認 */
		ret2 = memcmp((u8 *)&read_mente_serailno_data[2].serial_no[0], (u8 *)&read_mente_serailno_data_bk[2].serial_no[0], 12);

		/* 判定後にバックアップを更新 */
		/* Twin FRAM Serial No. */
		memcpy((u8 *)&read_mente_serailno_data_bk[1].version[0],	(u8 *)&read_mente_serailno_data[1].version[0],		sizeof(read_mente_serailno_data[1].version));		// 2byte
		memcpy((u8 *)&read_mente_serailno_data_bk[1].date[0],		(u8 *)&read_mente_serailno_data[1].date[0],			sizeof(read_mente_serailno_data[1].date));			// 8byte
		memcpy((u8 *)&read_mente_serailno_data_bk[1].serial_no[0],	(u8 *)&read_mente_serailno_data[1].serial_no[0],	sizeof(read_mente_serailno_data[1].serial_no));		// 12byte

		/* Quad FRAM Serial No. */
		memcpy((u8 *)&read_mente_serailno_data_bk[2].version[0],	(u8 *)&read_mente_serailno_data[2].version[0],		sizeof(read_mente_serailno_data[2].version));		// 2byte
		memcpy((u8 *)&read_mente_serailno_data_bk[2].date[0],		(u8 *)&read_mente_serailno_data[2].date[0],			sizeof(read_mente_serailno_data[2].date));			// 8byte
		memcpy((u8 *)&read_mente_serailno_data_bk[2].serial_no[0],	(u8 *)&read_mente_serailno_data[2].serial_no[0],	sizeof(read_mente_serailno_data[2].serial_no));		// 12byte

		/* write fram */
		_main_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_RTQ, FRAM_RC_MENTEN_SERI, 0, 0);
		
		if(ret1 == 0 && ret2 == 0)
		{
			/* 正常 */
			return true;
		}
		else
		{
			/* 上下入れ替わっている可能性あり */
			return false;
		}
	}
	else
	{
		return true;
	}
}


#if defined(A_PRO)	/* '22-09-27 */
bool is_rc_serial_no_check2(void)
{
	u8 ret1 = 0;
	u8 ret2 = 0;

	if(is_quad_model())
	{
		/* RC-Twinの確認 */
		ret1 = memcmp((u8 *)&read_mente_serailno_data[1].serial_no[0], (u8 *)&store_mente_serailno_data[1].serial_no[0], 12);

		/* RC-Quadの確認 */
		ret2 = memcmp((u8 *)&read_mente_serailno_data[2].serial_no[0], (u8 *)&store_mente_serailno_data[2].serial_no[0], 12);

		if(ret1 == 0 && ret2 == 0)
		{
			/* 正常 */
			return true;
		}
		else
		{
			if(ret1 != 0)
			{
				ex_rc_exchanged_unit |= 0x01;
			}
			if(ret2 != 0)
			{
				ex_rc_exchanged_unit |= 0x02;
			}

			/* 上下入れ替わっている可能性あり */
			return false;
		}
	}
	else
	{
		return true;
	}
}
#endif


bool is_rc_unit_error(void)
{
	if(ex_rc_error_flag != 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}


static bool check_drum_enable(u8 drum)
{
	if(drum == RC_TWIN_DRUM1)
	{
		if((ex_rc_enable & 0x01) != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(drum == RC_TWIN_DRUM2)
	{
		if((ex_rc_enable & 0x02) != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(drum == RC_QUAD_DRUM1)
	{
		if((ex_rc_enable & 0x04) != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else if(drum == RC_QUAD_DRUM2)
	{
		if((ex_rc_enable & 0x08) != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}


u8 get_rc_recovery_search_info( void )
{
    u8 ret;

    switch( ex_recovery_info.step )
    {
        /* このケースは探さないが探すなら出金方向 */
        case RECOVERY_STEP_VEND:
        case RECOVERY_STEP_PAYOUT_VALID:
        case RECOVERY_STEP_PAYOUT_ESCROW:
	//#if defined(UBA_RS)
		case RECOVERY_STEP_PAYOUT_RS_ESCROW:
	//#endif
        /* Cash Boxへ搬送中のため出金方向 */
        case RECOVERY_STEP_EMRGENCY_TRANSPORT:
        case RECOVERY_STEP_SWITCHBACK_TRANSPORT:
            ret = SEARCH_BILL_OUT;
            break;
        case RECOVERY_STEP_ESCORW:
        case RECOVERY_STEP_APB_IN:
        case RECOVERY_STEP_APB_OUT:
        case RECOVERY_STEP_STACK_DRUM:
        case RECOVERY_STEP_PAYOUT_DRUM:
        case RECOVERY_STEP_PAYOUT_TRANSPORT:
        case RECOVERY_STEP_PAYOUT_POS1:
        case RECOVERY_STEP_COLLECT_DRUM:
        case RECOVERY_STEP_COLLECT_TRANSPORT:
	//#if defined(UBA_RS)
		case RECOVERY_STEP_PAYOUT_RS_POS7:
	//#endif
            ret = SEARCH_BILL_IN;
            break;
        case RECOVERY_STEP_STACKING:
        case RECOVERY_STEP_COLLECT_STACKING:
        case RECOVERY_STEP_STACK_TRANSPORT:
            if( ex_recovery_info.unit == RC_CASH_BOX )
            {
                ret = SEARCH_BILL_OUT;
            }
            else
            {
                ret = SEARCH_BILL_IN;
            }
            break;
        default:
            ret = SEARCH_BILL_OUT;
            break;
	}
    return( ret );
}


u16 get_rc_recovery_status(u8 *unit, u8 *sts, u8 seach ) //ID-003側とmain側と両方で使用しているので注意 戻り値は使用する時がある、 unitは使用していない stsはcheatを出す為に使用している時がある。
{
	//この関数現状は、戻り値が BILL_IN_RC になるかと
	//sts が　BILL_CHEAT　になるか
	u16 rtn;
	*unit = 0;
	*sts = 0;

	// 全センサーOFF
	if (
	//#if defined(UBA_RS)
		((is_feed_quad_model()) &&											/* QUAD RS */
		 (((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) == 0) &&
		  ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) == 0) &&
		  ((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) == 0) &&
		  ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) == 0) && 
		  ((ex_rc_status.sst4A.byte & RS_POS1_POS2_POS3_POSRD) == 0 && (is_rc_rs_unit()) ) && 
		  (!(SENSOR_EXIT)) &&
		  (!(SENSOR_APB_OUT)) &&
		  (!(SENSOR_APB_IN))))
		  ||
		((!is_feed_quad_model()) &&											/* TWIN RS */
		 (((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) == 0) &&
		  ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) == 0) &&
		  ((ex_rc_status.sst4A.byte & RS_POS1_POS2_POS3_POSRD) == 0 && (is_rc_rs_unit()) ) && 
		  (!(SENSOR_EXIT)) &&
		  (!(SENSOR_APB_OUT)) &&
		  (!(SENSOR_APB_IN))))
		  ||
	//#endif
		  ((is_feed_quad_model()) &&											/* QUAD  */
		 (((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) == 0) &&
		  ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) == 0) &&
		  ((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) == 0) &&
		  ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) == 0) && 
		  ( !(is_rc_rs_unit()) )			 &&
		  (!(SENSOR_EXIT)) &&
		  (!(SENSOR_APB_OUT)) &&
		  (!(SENSOR_APB_IN)))) 
		  ||
		((!is_feed_quad_model()) &&											/* TWIN  */
		 (((ex_rc_status.sst21A.byte & RC1_POS1_POS2_POS3) == 0) &&
		  ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) == 0) &&
		  (!(is_rc_rs_unit()))			 &&
		  (!(SENSOR_EXIT)) &&
		  (!(SENSOR_APB_OUT)) &&
		  (!(SENSOR_APB_IN))))
		)
	{
		switch (ex_recovery_info.step)
		{
		case RECOVERY_STEP_STACK_DRUM:
			if (ex_recovery_info.unit != RC_CASH_BOX)
			{
				if ((ex_recovery_info.count + 1) == RecycleSettingInfo.DenomiInfo[ex_recovery_info.unit - 1].RecycleCurrent)
				{
					rtn = BILL_IN_RC; //use
				}
				else
				{
					/* DRUMに入りきったこと未確認なので */
					/* チートにする */
					rtn = BILL_IN_NON;
					*sts = BILL_CHEAT; //use ID003, main_task
				}
			}
			else
			{
				/* DRUMに入りきったこと未確認なので */
				/* チートにする */
				rtn = BILL_IN_NON;
				*sts = BILL_CHEAT; //use ID003, main_task
			}
			break;
		case RECOVERY_STEP_PAYOUT_DRUM:
			if (ex_recovery_info.unit != RC_CASH_BOX)
			{
			#if 0	//UBA500 not use
				if( (ex_recovery_info.count) == RecycleSettingInfo.DenomiInfo[ex_recovery_info.unit-1].RecycleCurrent )
				{
					rtn = BILL_IN_NON;
				}
				else
			#endif
				{
					/* チートにする */
					rtn = BILL_IN_NON;
					*sts = BILL_CHEAT;//use ID003, main_task
				}
			}
			else
			{
				/* DRUMに入りきったこと未確認なので */
				/* チートにする */
				rtn = BILL_IN_NON;
				*sts = BILL_CHEAT;//use ID003, main_task
			}
			break;
		case RECOVERY_STEP_COLLECT_DRUM:
			if (ex_recovery_info.unit != RC_CASH_BOX)
			{
			#if 0	//UBA500 not use
				if( (ex_recovery_info.count) == RecycleSettingInfo.DenomiInfo[ex_recovery_info.unit-1].RecycleCurrent )
				{
					rtn = BILL_IN_NON;
				}
				else
			#endif
				{
					/* チートにする */
					rtn = BILL_IN_NON;
					*sts = BILL_CHEAT;//use ID003, main_task
				}
			}
			else
			{
				/* DRUMに入りきったこと未確認なので */
				/* チートにする */
				rtn = BILL_IN_NON;
				*sts = BILL_CHEAT;//use ID003, main_task
			}
			break;
		/*  */
		case RECOVERY_STEP_ESCORW:
		case RECOVERY_STEP_APB_IN:
		case RECOVERY_STEP_APB_OUT:
		case RECOVERY_STEP_STACK_TRANSPORT:
		case RECOVERY_STEP_EMRGENCY_TRANSPORT:
		case RECOVERY_STEP_SWITCHBACK_TRANSPORT:
		case RECOVERY_STEP_PAYOUT_TRANSPORT:
		case RECOVERY_STEP_PAYOUT_POS1:
		case RECOVERY_STEP_COLLECT_TRANSPORT:
	//#if defined(UBA_RS)
		case RECOVERY_STEP_PAYOUT_RS_POS7:
	//#endif
			rtn = BILL_IN_NON;
			/* チートにする */
			*sts = BILL_CHEAT;//use ID003, main_task
			break;
		case RECOVERY_STEP_ACCEPT:
		case RECOVERY_STEP_STACKING:
		case RECOVERY_STEP_VEND:
		case RECOVERY_STEP_PAYOUT_ESCROW:
		case RECOVERY_STEP_PAYOUT_VALID:
	//#if defined(UBA_RS)
		case RECOVERY_STEP_PAYOUT_RS_ESCROW:
	//#endif
		case RECOVERY_STEP_COLLECT_STACKING:
		default:
			rtn = BILL_IN_NON;
			*sts = 0;
			break;
		}
	}
	else
	{
		switch (ex_recovery_info.step)
		{
		case RECOVERY_STEP_STACK_TRANSPORT:
			rtn = BILL_IN_RC; //use
			*unit = ex_recovery_info.unit;//not use
			if (ex_recovery_info.unit != RC_CASH_BOX)
			{
				if (((RC_POSB_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM1)) ||
					((RC_POSC_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM2)) ||
					((RC_POSE_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM1)) ||
					((RC_POSF_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM2)))
				{
					*sts = BILLBACK_PAYDRUM_FEED_BOX_FOR_STACK; //use main_task
				}
				else
				{
					/* スイッチバックさせる */
					*sts = BILLBACK_FEEDBOX;//use main_task
				}
			}
			else
			{
				*sts = FEED_BOX;//use main_task
			}
			break;
		case RECOVERY_STEP_STACK_DRUM:
			rtn = BILL_IN_RC; //use
			*unit = ex_recovery_info.unit;//not use
			if (((RC_POSB_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM1)) ||
				((RC_POSC_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM2)) ||
				((RC_POSE_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM1)) ||
				((RC_POSF_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM2)))
			{
				*sts = BILLBACK_PAYDRUM_FEED_BOX_FOR_STACK;//use main_task
			}
			else
			{
				if (((RC_POSA_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM1)) ||
					((RC_POSD_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM1)))
				{
					*sts = BILLBACK_PAYDRUM_FEED_BOX_FOR_STACK;//use main_task
				}
				else if (ex_recovery_info.unit != RC_CASH_BOX)
				{
					*sts = BILLBACK_FEEDBOX;//use main_task
				}
				else
				{
					*sts = FEED_BOX;//use main_task
				}
			}
			break;
		case RECOVERY_STEP_PAYOUT_DRUM:
			rtn = BILL_IN_RC; //use
			*unit = ex_recovery_info.unit;//not use
			if (((RC_POSB_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM1)) ||
				((RC_POSC_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM2)) ||
				((RC_POSE_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM1)) ||
				((RC_POSF_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM2)))
			{
				*sts = BILLBACK_PAYDRUM_FEED_BOX_FOR_PAYOUT;//use main_task
			}
			else
			{
				if (((RC_POSA_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM1)) ||
					((RC_POSD_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM1)))
				{
					*sts = BILLBACK_PAYDRUM_FEED_BOX_FOR_PAYOUT;//use main_task
				}
				else
				{
					*sts = BILLBACK_FEEDBOX;//use main_task
				}
			}
			break;
		case RECOVERY_STEP_PAYOUT_TRANSPORT:
			rtn = BILL_IN_RC; //use
			*unit = ex_recovery_info.unit;//not use
			if (((RC_POSB_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM1)) ||
				((RC_POSC_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM2)) ||
				((RC_POSE_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM1)) ||
				((RC_POSF_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM2)))
			{
				*sts = BILLBACK_PAYDRUM_FEED_BOX_FOR_PAYOUT;//use main_task
			}
			else
			{
				*sts = BILLBACK_FEEDBOX;//use main_task
			}
			break;
		case RECOVERY_STEP_PAYOUT_POS1:
	//#if defined(UBA_RS)
		case RECOVERY_STEP_PAYOUT_RS_POS7:
	//#endif
			if (RS_POS1_ON || RC_POS1_ON || RC_POS2_ON)
			{
				rtn = BILL_IN_RC; //use
				*unit = ex_recovery_info.unit;//not use
				*sts = BILLBACK_FEEDBOX;	//use main_task
			}
			else
			{
				rtn = BILL_IN_RC; //use
				*unit = RC_CASH_BOX;//not use
				*sts = FEED_BOX;//use main_task
			}
			break;
		case RECOVERY_STEP_COLLECT_DRUM:
		case RECOVERY_STEP_COLLECT_TRANSPORT:
			rtn = BILL_IN_RC; //use
			*unit = ex_recovery_info.unit;//not use
			if (((RC_POSB_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM1)) ||
				((RC_POSC_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM2)) ||
				((RC_POSE_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM1)) ||
				((RC_POSF_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM2)))
			{
				*sts = PAYDRUM_FEED_BOX;//use main_task
			}
			/* AのみON.（BはクリアウィンドウのためOFFしている。*/
			/* 3もOFFなので、大半はDrumに紙幣残っている */
			else if ((ex_recovery_info.unit == RC_TWIN_DRUM1) &&
					 (!RC_POSB_ON) && (RC_POSA_ON) && (!RC_POS3_ON))
			{
				*sts = PAYDRUM_FEED_BOX;//use main_task
			}
			/* DのみON.（EはクリアウィンドウのためOFFしている。*/
			/* 4もOFFなので、大半はDrumに紙幣残っている */
			else if ((ex_recovery_info.unit == RC_QUAD_DRUM1) &&
					 (!RC_POSE_ON) && (RC_POSD_ON) && (!RC_POS4_ON))
			{
				*sts = PAYDRUM_FEED_BOX;//use main_task
			}
			else
			{
				*sts = FEED_BOX;//use main_task
			}
			break;
		case RECOVERY_STEP_EMRGENCY_TRANSPORT:
		case RECOVERY_STEP_SWITCHBACK_TRANSPORT:
		case RECOVERY_STEP_PAYOUT_ESCROW:	 /* TBD チート? *///2025-05-24
			rtn = BILL_IN_RC; //use
			*unit = RC_CASH_BOX;//not use
			*sts = FEED_BOX;//use main_task
			break;
		case RECOVERY_STEP_ACCEPT:			 /* 通常はBill in Acceptor */
		case RECOVERY_STEP_ESCORW:			 /* 通常はBill in Acceptor */
		case RECOVERY_STEP_APB_IN:			 /* 通常はBill in Acceptor */
		case RECOVERY_STEP_APB_OUT:			 /* 通常はBill in Acceptor */
		case RECOVERY_STEP_STACKING:		 /* TBD チート? */
		case RECOVERY_STEP_VEND:			 /* TBD チート? */
	//2025-05-24	case RECOVERY_STEP_PAYOUT_ESCROW:	 /* TBD チート? */
		case RECOVERY_STEP_PAYOUT_VALID:	 /* TBD チート? */
	//#if defined(UBA_RS)
		case RECOVERY_STEP_PAYOUT_RS_ESCROW:
	//#endif
		case RECOVERY_STEP_COLLECT_STACKING: /* TBD チート? */
		default:
			if(is_rc_rs_unit())
			{
				rtn = BILL_IN_RC; //use
				*unit = ex_recovery_info.unit;//not use

				// 紙幣はあるけど履歴なし
				if ((is_feed_quad_model()) && ((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0))
				{
				#if 1 //2025-10-21 テスト的に変更
					rtn = BILL_IN_RC;
					*unit = RC_CASH_BOX;
					*sts = FEED_BOX;
				#else
					rtn = BILL_IN_RC; //use
					*unit = RC_CASH_BOX;//not use
					*sts = BILLBACK_FEEDBOX;	//use main_task
				#endif
				}
				else if ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0)
				{
					if (((RC_POSB_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM1)) || 
						((RC_POSC_ON) && (ex_recovery_info.unit == RC_TWIN_DRUM2)))
					{
						*sts = BILLBACK_PAYDRUM_FEED_BOX_FOR_PAYOUT;//use main_task
					}
					else
					{
						*unit = RC_TWIN_DRUM1;//not use
						*sts = BILLBACK_FEEDBOX; // BILL_DRUM_JAM;//use main_task
					}
				}
				else if ((is_feed_quad_model()) && ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0))
				{
					if (((RC_POSE_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM1)) || 
						((RC_POSF_ON) && (ex_recovery_info.unit == RC_QUAD_DRUM2)))
					{
						*sts = BILLBACK_PAYDRUM_FEED_BOX_FOR_PAYOUT; //use main_task
					}
					else
					{
						*unit = RC_QUAD_DRUM1;//not use
						*sts = BILLBACK_FEEDBOX; // BILL_DRUM_JAM;//use main_task
					}
				}
				else
				{
					rtn = BILL_IN_RC; //use
					*unit = RC_CASH_BOX;//not use
					*sts = BILLBACK_FEEDBOX;	//use main_task
				}
			}
			else
			{
				// 紙幣はあるけど履歴なし
				if ((is_feed_quad_model()) && ((ex_rc_status.sst22A.byte & RC2_POS4_POS5_POS6) != 0))
				{
					#if 1 //2025-10-21 テスト的に変更
					rtn = BILL_IN_RC;
					*unit = RC_CASH_BOX;
					*sts = FEED_BOX;
					#else
					rtn = BILL_IN_RC; //use
					*unit = RC_CASH_BOX;//not use
					*sts = BILLBACK_FEEDBOX;	//use main_task
					#endif
				}
				else if ((ex_rc_status.sst31A.byte & RC1_POSA_POSB_POSC) != 0)
				{
					rtn = BILL_IN_RC; //use
					*unit = RC_TWIN_DRUM1;//not use
					*sts = BILLBACK_FEEDBOX; // BILL_DRUM_JAM;//use main_task
				}
				else if ((is_feed_quad_model()) && ((ex_rc_status.sst32A.byte & RC2_POSD_POSE_POSF) != 0))
				{
					rtn = BILL_IN_RC; //use
					*unit = RC_QUAD_DRUM1;//not use
					*sts = BILLBACK_FEEDBOX; // BILL_DRUM_JAM;//use main_task
				}
				else
				{
					rtn = BILL_IN_RC; //use
					*unit = RC_CASH_BOX;	//not use
					*sts = BILLBACK_FEEDBOX;//use main_task
				}
			}
			break;
		}
	}
	return rtn;
}

/*********************************************************************//**
 * @brief initialize_encryption
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void initialize_encryption(void)
{
	set_encryption_number(0);
	set_encryption_key(0);
	
	GC2Init(&encryption_serial_number[0], &ex_cbc_context[0]);
}


/*********************************************************************//**
 * @brief initialize_encryption
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void renewal_cbc_context(void)
{
	set_encryption_number(1);
	set_encryption_key(1);
	
	GC2Init(&encryption_serial_number[0], &ex_cbc_context[0]);
}


/*********************************************************************//**
 * @brief set Encryption Number
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void set_encryption_number(u8 type)
{
	u8 cnt;
	u32 powerup_count;
	u32 stack_count;
	u32 payout_count;
	u32 collect_count;
	
	switch(type)
	{
	case	0:
			ex_encryption_number = 1;
			break;
	case	1:	// ここでcntを可変な値にする為POWER ON回数等を使用する
			_jdl_load_dword(JDL_STAT_ADR_MOV_POWERUP, &powerup_count);
			_jdl_load_dword(JDL_STAT_ADR_RC_STACK, &stack_count);
			_jdl_load_dword(JDL_STAT_ADR_RC_PAYOUT, &payout_count);
			_jdl_load_dword(JDL_STAT_ADR_RC_COLLECT, &collect_count);
			
			cnt = (u8)powerup_count + (u8)stack_count + (u8)payout_count + (u8)collect_count;
			
			ex_encryption_number = cnt;
			
			/* "0"の場合は"1"にする */
			if(ex_encryption_number == 0)
			{
				ex_encryption_number = 1;
			}
			break;
	}
}


/*********************************************************************//**
 * @brief set Encryption Key
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void set_encryption_key(u8 type)
{
	u8 cnt;
	u32 powerup_count;
	u32 stack_count;
	u32 payout_count;
	u32 collect_count;
	u8 ii = 0;
	
	switch(type)
	{
	case	0:
			for(cnt = 0; cnt < 8; cnt++)
			{
				ex_encryption_key[cnt] = encryption_key_initial_value[cnt];
			}
		
			// ex_encryption_key_no = 0;		iProでは0に初期化しているが不要では？
			break;
	case	1:	// ここでcntを可変な値にする為POWER ON回数等を使用する
			_jdl_load_dword(JDL_STAT_ADR_MOV_POWERUP, &powerup_count);
			_jdl_load_dword(JDL_STAT_ADR_RC_STACK, &stack_count);
			_jdl_load_dword(JDL_STAT_ADR_RC_PAYOUT, &payout_count);
			_jdl_load_dword(JDL_STAT_ADR_RC_COLLECT, &collect_count);
			
			cnt = (u8)powerup_count + (u8)stack_count + (u8)payout_count + (u8)collect_count;
			
			ex_encryption_key_no++;
			
			if(ex_encryption_key_no > 4)
			{
				ex_encryption_key_no = 0;
			}
			else
			{
				cnt += (ex_encryption_key_no * 4);
			}
			for(ii = 0; ii < 8; ii += 2)
			{
				ex_encryption_key[ii + 0] = (u8)(GC2tbl[cnt]);
				ex_encryption_key[ii + 1] = (u8)(GC2tbl[cnt] >> 8);

				if(cnt > 0xFE)
				{
					cnt = 0;
				}
				else
				{
					cnt += 1;
				}
			}
			break;
	}
}


/*********************************************************************//**
 * @brief initialze GC2
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void GC2Init(const u8 *pSerialNum, u8 *pCBC)
{
	u8 i;
	u8 j;
	
	for(i = 0; i < RC_GC2_BLOCK_LEN; i++)
	{
		pCBC[i] = 0;

		for(j = 0; j < RC_GC2_BLOCK_LEN; j++)
		{
			pCBC[i] ^= pSerialNum[j];
			pCBC[i] = RC_GC2_BARREL_L(pCBC[i]);	// cbcの上位5bitと下位3bitを交換
			pCBC[i] += ex_encryption_key[i];
		}
	}
}



/*********************************************************************//**
 * @brief feed control sub function
 *  all sensor status
 * @param[in]	None
 * @return 		true  : all sensor off
 * 				false : any sensor on
 **********************************************************************/
bool is_feed_quad_model(void)
{
	if(ex_rc_status.sst1A.bit.quad)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool is_rc_rs_unit(void)
{
	if(ex_rc_configuration.unit_type == RS_CONNECT)
	{
		return(true);
	}
	else
	{
		return(false);
	}
}

bool is_rc_error(void)
{
	if(ex_rc_status.sst1A.bit.error || ex_rc_status.sst1A.bit.warning)
	{
		return true;
	}
	else
	{
		return false;
	}
}


#pragma SET_CODE_SECTION()
#pragma SET_DATA_SECTION()
#endif
