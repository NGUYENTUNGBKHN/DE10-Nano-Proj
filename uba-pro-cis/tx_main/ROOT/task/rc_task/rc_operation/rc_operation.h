/******************************************************************************/
/*! @addtogroup Group2
    @file       rc_operation.h
    @brief      
    @date       2024/05/22
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    Japan CashMachine Co, Limited. All rights reserved.
******************************************************************************/

#ifndef _RC_OPERATION_H_
#define _RC_OPERATION_H_
#ifdef __cplusplus
extern "C"
{
#endif

/* CODE */

extern u8 *Rom2str;									/* ダウンロード先頭アドレス	*/
extern u8 *Rom2end;									/* ダウンロード終了アドレス */
extern u8 pol_time;

extern u8 rc_last_send;
extern u8 ex_rc_rfid_src_req;

extern void  _rc_set_seq(u16 seq);
/* RC Send command line function */
extern void rc_send_command_proc(void);

extern void rc_send_polling_proc(void);
/* RC Recevied command line function */
extern void rc_received_check_proc(void);

/* DOWNLOAD */
extern void rc_dl_data_command(void);
/* Download command */
extern void rc_dl_start_command(void);
extern void rc_dl_data_command(void);
extern void rc_dl_check_command(void);
extern void rc_dl_end_command(void);
/* Download response */
extern void rc_dl_start_response(void);
extern void rc_dl_data_response(void);
extern void rc_dl_check_response(void);
extern void rc_dl_end_response(void);
/* ---------*/

/* RFID*/
extern void rc_rfid_read(u16 addr, u16 length, u8 *data);
extern void rc_rfid_write(u16 addr, u16 length, u8 *data);
extern void rc_rfid_reset(void);

#ifdef __cplusplus
}
#endif
#endif
