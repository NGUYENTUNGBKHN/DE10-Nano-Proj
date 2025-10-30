/******************************************************************************/
/*! @addtogroup Group2
    @file       feed_rc_unit.h
    @brief      
    @date       2024/05/15
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    Japan CashMachine Co, Limited. All rights reserved.
******************************************************************************/

#ifndef _FEED_RC_UNIT_H_
#define _FEED_RC_UNIT_H_
#ifdef __cplusplus
extern "C"
{
#endif
#if defined(UBA_RTQ)
#define FEED_RC_SEQ_TIMEOUT                 3000


#define FEED_HUNGING_POSITION_START 				(u16)(50/PITCH)
#define FEED_REJECT_CENTERING						(u16)(180/PITCH)
#define FEED_REJECT_CENTERING_ON_TO_OFF				(u16)(545/PITCH)
#define FEED_HANGING_POSITION_SHORT_TICKET			(u16)(10/PITCH)


enum _STACK_OPTION
{
    NON = 0,
	STACK,
	COLLECT,
	PAYOUT,
	PAYREJECT,
};

/* STACK */
extern void _feed_rc_stack_seq_proc(u32 flag);

/* STACK FORCE PAYOUT */
extern void _feed_rc_fpayout_seq_proc(u32 flag);

/* PAYOUT */
extern void _feed_rc_payout_seq_proc(u32 flag);
extern void _feed_set_feed_payout_retry(u32 alarm_code);

/* PAYOUT FORCE STACK */
extern void _feed_rc_force_stack_proc(u32 flag);

/*COLECCT */
extern void _feed_rc_collect_seq_proc(u32 flag);

/* SEARCH BILL */
extern void _feed_rc_search_bill_seq_proc(u32 flag);

/* BILL BACK */
extern void _feed_rc_bill_back_seq_proc(u32 flag);

/* Setup param  */
extern void feed_rc_set_unit(u8 unit);
extern void feed_rc_set_payout_option(u8 option);
extern void feed_rc_set_recover(u8 recover);
extern void feed_rc_set_search_dir(u8 dir);

extern void feed_rc_set_sensor_bk(u8 status);

//#if defined(UBA_RS)
	/* RS PAYOUT */
	extern void _feed_rs_payout_seq_proc(u32 flag);
	/* RS FORCE PAYOUT */
	extern void _feed_rs_fpayout_seq_proc(u32 flag);
	extern void _feed_rs_fpayout_2400_seq_start();
	extern void feed_rc_set_payout_last(u8 data);
//#endif

#endif // UBA_RTQ

#ifdef __cplusplus
}
#endif
#endif

