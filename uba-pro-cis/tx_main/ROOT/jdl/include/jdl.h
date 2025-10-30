/****************************************************************************/
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2017                          */
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
 * MODEL NAME : （モデル名）
 * @file jdl.h
 * @brief  JCM Device Log Header
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/
#pragma once

#include "jdl_comm.h"
#include "jdl_conf.h"
#include "jdl_category_id.h"
#include "jdl_event_code.h"

/*==========================================================================*/
/*==========================================================================*/
/* Byte swap                                                                */
/*==========================================================================*/
#if defined(_JDL_LITTLE_ENDIAN)
#define _JDL_SWAP_16(val) ( (((u16)val & 0xff00) >> 8) | (((u16)val & 0x00ff) << 8) )
#define _JDL_SWAP_32(val) ( (((u32)val & 0xff000000) >> 24) | (((u32)val & 0x00ff0000) >> 8) | (((u32)val & 0x0000ff00) << 8) | (((u32)val & 0x000000ff) << 24) )
#else  /* _JDL_BIG_ENDIAN */
#define _JDL_SWAP_16(val) ( val )
#define _JDL_SWAP_32(val) ( val )
#endif /* _JDL_LITTLE_ENDIAN */


/*==========================================================================*/
/*==========================================================================*/
/* System category functions                                                */
/*==========================================================================*/
extern u8 _jdl_sys_init(u8 clear);
extern u8 _jdl_sys_req(u32 s_offset, u32 buff_size);
extern u8 _jdl_sys_set(u32 offset, u8 *data, u32 size);
extern u8 _jdl_sys_cmp(u32 offset, u8 *data, u32 size);
extern u8 _jdl_sys_set_tim(void);
extern u8 _jdl_sys_get(u8 *buff, u32 buff_size, u32 offset, u32 *g_size);


/*==========================================================================*/
/*==========================================================================*/
/* Statistics category functions                                            */
/*==========================================================================*/
extern u8 _jdl_stat_init(u8 clear);
extern u8 _jdl_stat_req(u32 s_offset, u32 buff_size);
extern u8 _jdl_stat_inc_mov(u32 offset, u8 type);
extern u8 _jdl_stat_add_mov(u32 offset, u8 type, u32 val1, u32 val2);
extern u8 _jdl_stat_get_mov(u32 offset, u8 type, u32 *val, u32 *val2);
//extern u8 _jdl_stat_motor(u32 offset, u8 cnt_type, u8 tim_type, u32 add_tim1, u32 add_tim2);
extern u8 _jdl_stat_motor_new(u32 offset, u8 cnt_type, u8 tim_type, u32 add_tim1, u32 add_tim2);
#if defined(_JDL_STAT_USE_ACC_CNT)
extern u8 _jdl_stat_insert(void);
extern u8 _jdl_stat_bill_accept(void);
extern u8 _jdl_stat_ticket_accept(void);
#endif  /* _JDL_STAT_USE_ACC_CNT */
#if defined(_JDL_STAT_USE_REJ_CNT)
extern u8 _jdl_stat_bill_reject(u32 rej_offset, u32 stat_offset);
extern u8 _jdl_stat_ticket_reject(u32 rej_offset, u32 stat_offset);
#endif  /* _JDL_STAT_USE_REJ_CNT */
#if defined(_JDL_STAT_USE_ERR_CNT)
extern u8 _jdl_stat_err(u32 err_offset, u32 stat_offset);
#endif  /* _JDL_STAT_USE_ERR_CNT */
extern u8 _jdl_stat_get(u8 *buff, u32 buff_size, u32 offset, u32 *g_size);


/*==========================================================================*/
/*==========================================================================*/
/* Sensor category functions                                                */
/*==========================================================================*/
extern u8 _jdl_sens_init(u8 clear);
extern u8 _jdl_sens_req(u32 s_offset, u32 buff_size);
extern u8 _jdl_sens_set_val(u32 offset, u8 type, u32 val);
extern u8 _jdl_sens_set_blk(u32 offset, u8 *data, u32 size);
extern u8 _jdl_sens_update_cor(void);
extern u8 _jdl_sens_set_cur_val(u32 offset, u8 type, u32 val);
extern u8 _jdl_sens_set_cur_blk(u32 offset, u8 *data, u32 size);
extern u8 _jdl_sens_get(u8 *buff, u32 buff_size, u32 offset, u32 *g_size);


/*==========================================================================*/
/*==========================================================================*/
/* Communication category functions                                         */
/*==========================================================================*/
extern u8 _jdl_comm_init(u8 clear, u8 *pid);
extern u8 _jdl_comm_req(u32 s_offset, u32 buff_size);
extern void _jdl_comm_tick(void);
extern u8 _jdl_comm_add_data(u8 *data, u8 size, u8 tx_flag);
extern u8 _jdl_comm_get(u8 *buff, u32 buff_size, u32 offset, u32 *g_size);
extern void _jdl_comm_get_idx(u32 *index, u8 *round);
extern u8 _jdl_comm_get_err_code(void);


/*==========================================================================*/
/*==========================================================================*/
/* Event category functions                                                 */
/*==========================================================================*/
extern u8 _jdl_event_init(u8 clear);
extern u8 _jdl_event_req(u32 s_offset, u32 buff_size);
extern u8 _jdl_event_set(u8 *data);
extern u8 _jdl_event_get(u8 *buff, u32 buff_size, u32 offset, u32 *g_size);
extern void _jdl_event_link_clear(void);
extern void _jdl_event_get_idx(u16 *index, u8 *round);
extern u8 _jdl_event_get_err_code(void);


/*==========================================================================*/
/*==========================================================================*/
/* Error category functions                                                 */
/*==========================================================================*/
extern u8 _jdl_err_init(u8 clear);
extern u8 _jdl_err_req(u32 s_offset, u32 buff_size);
extern u8 _jdl_err_set(u8 *data);
extern u8 _jdl_err_update(u8 *data);
extern u8 _jdl_err_add_trace(void *data);
extern u8 _jdl_err_get(u8 *buff, u32 buff_size, u32 offset, u32 *g_size);
extern void _jdl_err_link_clear(void);
extern void _jdl_err_get_idx(u16 *index, u8 *round);
extern u8 _jdl_err_get_err_code(void);


/*==========================================================================*/
/*==========================================================================*/
/* Acceptance category functions                                            */
/*==========================================================================*/
extern u8 _jdl_acc_init(u8 clear);
extern u8 _jdl_acc_req(u32 s_offset, u32 buff_size);
extern u8 _jdl_acc_insert(void);
extern u8 _jdl_acc_bill_accept(u16 dinfo_idx);
extern u8 _jdl_acc_ticket_accept(u16 tinfo_idx);
extern u8 _jdl_acc_bill_reject(u16 dinfo_idx, u32 rej_offset, u32 stat_offset);
extern u8 _jdl_acc_ticket_reject(u16 tinfo_idx, u32 rej_offset, u32 stat_offset);
extern u8 _jdl_acc_get(u8 *buff, u32 buff_size, u32 offset, u32 *g_size);


/*==========================================================================*/
/*==========================================================================*/
/* Position analysis category functions                                     */
/*==========================================================================*/
extern u8 _jdl_posiana_init(u8 clear);
extern u8 _jdl_posiana_req(u32 s_offset, u32 buff_size);
extern u8 _jdl_posiana_set(void);
extern u8 _jdl_posiana_set_prev(void);
extern u8 _jdl_posiana_update_state(u8 *data);
extern u8 _jdl_posiana_get(u8 *buff, u32 buff_size, u32 offset, u32 *g_size);
extern void _jdl_posiana_link_clear(void);
extern u8 _jdl_posiana_get_err_code(void);


#if defined(_JDL_USE_RC)
/*==========================================================================*/
/*==========================================================================*/
/* RC category functions                                                    */
/*==========================================================================*/
extern u8 _jdl_rc_init(u8 clear);
extern u8 _jdl_rc_req(u32 s_offset, u32 buff_size);
extern u8 _jdl_rc_set_uinfo(u16 unit_no, u32 uinfo_offset, u8 *data, u32 size);
extern u8 _jdl_rc_get(u8 *buff, u32 buff_size, u32 offset, u32 *g_size);
#endif /* _JDL_USE_RC */


#if defined(UBA_RC) || defined(UBA_RTQ)
/*==========================================================================*/
/*==========================================================================*/
/* OPRC category functions                                                  */
/*==========================================================================*/
extern u8 _jdl_oprc_init(u8 clear);
extern u8 _jdl_oprc_req(u32 s_offset, u32 buff_size);
extern u8 _jdl_oprc_get(u8 *buff, u32 buff_size, u32 s_offset, u32 *g_size);
#endif

/*
extern 
extern 
extern 
*/







