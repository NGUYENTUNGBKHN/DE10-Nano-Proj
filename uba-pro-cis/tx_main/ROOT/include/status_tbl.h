
/******************************************************************************/
/*! @addtogroup Group1
    @file       status_tbl.h
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
#pragma once


enum _RECOVERY_STEP
{
	RECOVERY_STEP_NON = 0,
	RECOVERY_STEP_ACCEPT,
#if defined(_PROTOCOL_ENABLE_ID0G8)
	RECOVERY_STEP_ESCORW_WAIT_ACK,
	RECOVERY_STEP_ENABLE_NOTE_PATH_CLER,	// use escrow位置をnote path clearの分岐にしたいが、すでに似た名前がある為(Escrow)
#endif
	RECOVERY_STEP_ESCORW,
	RECOVERY_STEP_APB_IN,
	RECOVERY_STEP_APB_OUT,	// only use SS not use RTQ
	RECOVERY_STEP_EXIT,		// not use SS and RTQ
	RECOVERY_STEP_SK_IN,		// not use SS and RTQ
	RECOVERY_STEP_STACK,
	RECOVERY_STEP_STACKING,
// UBA_WS
	RECOVERY_STEP_STACKING_BILL_IN_BOX,	// use 紙幣はBOXに入ったはず
//
	RECOVERY_STEP_ICB_ACCEPT,
	RECOVERY_STEP_VEND,
#if defined(UBA_RTQ)
	RECOVERY_STEP_STACK_TRANSPORT,		// 7  feedタスクで設定。センサ1で検知時(多分不要) //ok
	RECOVERY_STEP_STACK_DRUM,			// 8  feedタスクで設定。センサB,C,E,Fで検知時 //ok
										//    リカバリ時にDRUM動作の要不要チェックのため必要
	RECOVERY_STEP_COLLECT_DRUM,			// 9 feedタスクで設定。搬送モータ駆動開始時 //ok
										//    回収開始保存のため必要。
	RECOVERY_STEP_COLLECT_TRANSPORT,	// 10 feedタスクで設定。センサB,C,E,Fで検知時 //ok
	RECOVERY_STEP_COLLECT_STACKING,		// use StackerタスクへStack要求実施時 //ok
   	RECOVERY_STEP_EMRGENCY_TRANSPORT = 0x80, // 128  識別OK時に設定 //ok
	RECOVERY_STEP_SWITCHBACK_TRANSPORT, // 129 スイッチバック時に設定 //ok
										//	   再搬送時にCashBoxへ搬送するだけであることを示す。
	RECOVERY_STEP_PAYOUT_DRUM,			// 130 feedタスクで設定。搬送モータ駆動開始時 //ok
										//     出金開始保存のため必要。
	RECOVERY_STEP_PAYOUT_TRANSPORT,		// 131 feedタスクで設定。センサB,C,E,Fを紙幣が通過時（OFF時）//ok
										//     リカバリ時にDRUM動作の要不要チェックのため必要
	RECOVERY_STEP_PAYOUT_POS1,			// 132 feedタスクで設定。センサ1 ON時に設定 //ok

	RECOVERY_STEP_PAYOUT_ESCROW, //ok
	RECOVERY_STEP_PAYOUT_VALID,	//ok
	RECOVERY_STEP_PAYOUT_RS_POS7 = 30,	//#if defined(UBA_RS) //ok
	RECOVERY_STEP_PAYOUT_RS_ESCROW	//#if defined(UBA_RS) //ok
#endif
};

/* offset */
#define CLINE_STATUS_TBL_OFFSET	0
//2023-12-12 #define RECOVERY_INFO_OFFSET	64
#define RECOVERY_INFO_OFFSET	92

#define STATUS_TBL_SUM_OFFSET	254

extern void init_status_table(void);
extern void read_status_table(void);
extern void write_status_table(void);
extern void set_protocol(u8 protocol);
extern u8 get_protocol(void);
extern void set_recovery_step(u8 step);
extern u32 is_icb_recovery_info_stack(u8 *denomi);
