/******************************************************************************/
/*! @addtogroup Group1
    @file       operation.h
    @brief      main operation header
    @date       2021/04/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2021 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2021/04/24 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/

#ifndef _SRC_INCLUDE_OPERATION_H_
#define _SRC_INCLUDE_OPERATION_H_

/*############################################################################*/
/*#                                                                          #*/
/*#  Functions                                                               #*/
/*#                                                                          #*/
/*############################################################################*/
/*============================================================================*/
/* ----- Operation Main ----------------------------------------------------- */
/*============================================================================*/
extern void operation_main(void);
extern void operation_main_msg_proc(void);

/*============================================================================*/
/* ----- Power Up Mode (MODE1_POWERUP) -------------------------------------- */
/*============================================================================*/
extern void powerup_msg_proc(void);
extern void powerup_fram_read(void);
extern void powerup_mgu_read(void);
extern void powerup_dipsw_init(void);
extern void powerup_rfid_unit_init(void);
extern void powerup_sensor_init(void);
extern void powerup_wait_req(void);
extern void powerup_side_init(void);

/*============================================================================*/
/* ----- Initialise Mod (MODE1_INIT) ---------------------------------------- */
/*============================================================================*/
extern void initialize_msg_proc(void);
extern void init_sensor_active(void);

extern void init_feed(void);
extern void init_stacker_home(void);
extern void init_stacker(void);
extern void init_centering_close(void);
extern void init_side_ajust(void);
#if defined(PRJ_IVIZION2)
    extern void init_centering_open(void);
    extern void init_read_temperature(void);
    extern void init_read_rfid(void);
#else
    extern void init_centering_home(void);
    extern void init_centering(void);
    extern void init_apb(void);
    extern void init_apb_home(void);
    extern void init_force_stack(void);
    extern void init_stack_exec_retry(void);
    extern void init_icb(void);   
    extern void	_feed_bill_over_window(void);
    extern void _feed_bill_over_window_fwd(void);
    extern void init_shutter(void);
    extern void init_initial_position(void);
#endif

extern void init_wait_req(void);
extern void init_force_feed_stack(void);
extern void init_wait_remain_req(void);
extern void init_reject_apb_home(void);
extern void init_feed_reject(void);
extern void init_note_stay(void);
extern void init_wait_reset_req(void);
extern void init_apb_close_sensor_active(void);
extern void init_apb_close(void);



/*============================================================================*/
/* ----- Disable Mode (MODE1_DISABLE) --------------------------------------- */
/*============================================================================*/
extern void disable_msg_proc(void);
extern void disable_wait_req(void);
extern void disable_wait_reject_req(void);

/*============================================================================*/
/* ----- Enable Mode (MODE1_ENABLE) ----------------------------------------- */
/*============================================================================*/
extern void enable_msg_proc(void);
extern void enable_wait_bill_in(void);
extern void enable_wait_req(void);
extern void enable_wait_reject_req(void);

/*============================================================================*/
/* ----- Active Disable Mode (MODE1_ACTIVE_DISABLE) ------------------------- */
/*============================================================================*/
extern void active_disable_msg_proc(void);
extern void active_disable_wait_req(void);
extern void active_disable_wait_reject_req(void);
extern void active_disable_stacker_half(void);
#if !defined(PRJ_IVIZION2)
extern void active_disable_centering_home(void);
extern void active_disable_shutter_open(void);
#endif

/*============================================================================*/
/* ----- Active Enable Mode (MODE1_ACTIVE_ENABLE) --------------------------- */
/*============================================================================*/
extern void active_enable_msg_proc(void);
extern void active_enable_wait_bill_in(void);
extern void active_enable_wait_req(void);
extern void active_enable_wait_reject_req(void);
extern void active_enable_stacker_half(void);
#if !defined(PRJ_IVIZION2)
extern void active_enable_centering_home(void);
extern void active_enable_shutter_open(void);
#endif
/*============================================================================*/
/* ----- Adjust Mode (MODE1_ADJUST) ----------------------------------------- */
/*============================================================================*/
extern void adjust_msg_proc(void);
extern void adjust_temp_adj(void);

/*============================================================================*/
/* ----- Accepting Mode (MODE1_ACCEPT) -------------------------------------- */
/*============================================================================*/
#if !defined(PRJ_IVIZION2)
extern void pb_centering_home(void);
extern void wait_pb_start(void);
extern void wait_feed_start(void);
extern void accept_wait_reject_centering_shutter_home(void);
#endif

/*============================================================================*/
/* ----- Stacking Mode (MODE1_STACK) ---------------------------------------- */
/*============================================================================*/
extern void stack_msg_proc(void);
#if defined(PRJ_IVIZION2)
    extern void feed_apb(void);
    extern void apb_exec(void);
    extern void feed_stack(void);
    extern void stack_exec(void);
    extern void stack_wait_req(void);
    extern void stack_ld_wait_req(void);
    extern void stack_wait_reject_req(void);
#else
    extern void stack_msg_proc(void);
    extern void feed_stack(void);
    extern void wait_stack_start(void);
    extern void stack_wait_stack_top(void);
    extern void stack_wait_stack_home(void);
    extern void stack_wait_stack_home_wait_note(void);//連続動作用,入口センサ監視中
    extern void stack_wait_req_first(void);
    extern void stack_exec_retry(void);
    extern void stack_wait_req(void);
    extern void stack_feed_rev_req(void);
    #if !defined(UBA_RC)//#if defined(LOW_POWER_3)
    //extern void wait_pd_done(void);
    extern void stack_icb_req(void);
    #endif
    extern void stack_wait_reject_req(void);
    extern void stack_wait_reject_shutter_open(void);
    extern void stack_wait_shutter_open_ld_mode(void);
#endif

/*============================================================================*/
/* ----- Rejecting Mode (MODE1_REJECT) -------------------------------------- */
/*============================================================================*/
extern void reject_msg_proc(void);
extern void reject_sensor_active(void);
extern void feed_reject(void);
extern void stacker_half(void);
extern void reject_wait_req(void);

#if defined(PRJ_IVIZION2)
extern void reject_stacker_half(void);
extern void reject_stack(void);
extern void reject_centering_home(void);
extern void apb_close(void);
extern void apb_close_wait_alarm(void);
extern void reject_centering_open(void);
extern void reject_centering_close_sensor_active(void);
extern void reject_centering_close(void);
#else
extern void reject_reject_stop_wait_wid(void);
extern void reject_wait_wid(void);
extern void reject_icb_req(void);
extern void reject_note_removed_wait_sensor_active(void);
#endif



/*============================================================================*/
/* ----- Rejecting Standby Mode (MODE1_REJECT_STANDBY) ---------------------- */
/*============================================================================*/
extern void reject_standby_msg_proc(void);
extern void reject_standby_note_stay(void);
extern void reject_standby_confirm_note_stay(void);
extern void reject_standby_wait_req(void);
extern void reject_standby_wait_reject_req(void);

/*============================================================================*/
/* ----- Alarm Mode (MODE1_ALARM) ------------------------------------------- */
/*============================================================================*/
extern void download_msg_proc(void);
extern void alarm_msg_proc(void);
extern void alarm_wait_req(void);
extern void alarm_stacker_full(void);
extern void alarm_stacker_jam(void);
extern void alarm_stacker_fail(void);

extern void alarm_check_fishing(void);
extern void alarm_lost_bill(void);
extern void alarm_feed_speed(void);
extern void alarm_feed_fail(void);
extern void alarm_pusher_home(void);
extern void alarm_acceptor_jam(void);
extern void alarm_side_acceptor_jam(void);
extern void alarm_confirm_at_jam(void);
extern void alarm_apb_fail(void);
extern void alarm_box(void);
extern void alarm_confirm_box(void);
extern void alarm_stacker_con(void);
extern void alarm_cheat(void);
extern void alarm_centering_fail(void);
extern void alarm_wait_reject_req(void);
extern void alarm_wait_sensor_active(void);
extern void alarm_force_reject(void);
extern void alarm_confirm_stacker_full(void);
extern void alarm_confirm_stacker_con(void);

#if defined(PRJ_IVIZION2)
extern void alarm_confirm_stacker_jam(void);
extern void alarm_rfid(void);
extern void alarm_cpu_board(void);
#else
extern void alarm_pusher_home(void);
extern void alarm_wait_sensor_active(void);
extern void alarm_icb(void);
extern void alarm_write_icb(void);
#endif

/*============================================================================*/
/* ----- Test Mode (MODE1_TEST) --------------------------------------------- */
/*============================================================================*/
extern void test_standby_msg_proc(void);
extern void test_active_msg_proc(void);
extern void test_init(void);
extern void test_standby(void);
extern void test_sensor_active(void);
extern void test_feed_motor(void);
extern void test_stacker(void);
extern void test_stacker_home(void);
extern void test_stacker_motor(void);
extern void test_centering(void);
extern void test_side_init(void);

#if defined(PRJ_IVIZION2)
extern void test_centering_open(void);
extern void test_centering_close(void);
#else
extern void test_centering_open_uba(void);
extern void test_centering_close_uba(void);
extern void test_apb(void);
extern void test_apb_motor(void);
#endif

extern void test_centering_motor(void);
extern void test_sensor(void);
extern void test_interface(void);
extern void test_led(void);
extern void test_mag_write(void);
extern void test_display_setting(void);
extern void test_aging_sensor_init_wait(void);
extern void test_aging_sensor_init(void);
extern void test_aging_cis_init(void);
extern void test_aging_sensor_active(void);
#if defined(PRJ_IVIZION2)
extern void test_aging_stack_home(void);
extern void test_aging_feed_escrow(void);
extern void test_aging_feed_stack(void);
extern void test_aging_stack_exec(void);
extern void test_aging_temperature(void);
#else
extern void test_aging_feed_centering_uba(void);
extern void test_aging_centering_uba(void);
extern void test_aging_feed_escrow_uba(void);
extern void test_aging_feed_apb_uba(void);
extern void test_aging_apb_exec_uba(void);
extern void test_aging_stack_exec_uba(void);
extern void test_aging_stack_exec_home_uba(void);	
extern void  test_aging_shutter_close_uba(void);
extern void test_aging_shutter_open_uba(void);
#endif

#if !defined(PRJ_IVIZION2)
extern void test_shutter_motor(void);
extern void test_shutter_motor_close(void);
#endif
extern void clear_ex_multi_job(void);   /* 2022-02-21 */

extern void test_wait(void);
extern void test_alarm(void);
/*============================================================================*/
/* ----- Operation Sub (sub functions) -------------------------------------- */
/*============================================================================*/
//extern void musb_maintenance_sub_proc(void);
/* Set Mode */
extern void _main_select_protocol(void);
extern void _main_set_init(void);
extern void _main_set_disable(void);
extern void _main_set_enable(void);
extern void _main_set_active_disable(void);
extern void _main_set_active_enable(void);
extern void _main_set_accept(void);
extern void _main_set_reject(void);
extern void _main_set_adjustment(void);
#if defined(PRJ_IVIZION2)
extern void _main_set_reject_centering_close(void);
#else
extern void _main_set_active_enable_uba(u8 mode);
extern void _main_set_active_disable_uba(u8 mode);
extern  u16 _main_stay_bill_check(void);
#endif

extern void _main_set_reject_standby_note_stay(void);
extern void _main_set_reject_standby_wait_req(void);
extern void _main_set_sensor_active( u8 enable);
extern void _main_set_pl_active(u8 active);
extern void _main_set_test_standby(void);
extern void _main_set_test_active(void);
extern void _main_set_test_wait(void);
extern void _main_set_payout(void);
extern void _main_alarm_sub(u32 mode1, u32 mode2, u32 rsp_msg, u32 code, u32 seq, u32 sensor);
extern void _main_reject_sub(u32 mode1, u32 mode2, u32 rsp_msg, u32 code, u32 seq, u32 sensor);
extern void _main_reject_req(u32 mode1, u32 mode2, u32 code, u32 seq, u32 sensor);
/* Display */
extern void _main_display_powerup(void);
extern void _main_display_init(void);
extern void _main_display_adj(void);
extern void _main_display_disable(void);
extern void _main_display_enable(void);
extern void _main_display_pause(void);
extern void _main_display_resume(void);
extern void _main_display_test_standby(void);
extern void _main_display_denomination(void);
extern void _main_display_manual_setting(u8 st_num);
/* Convert Mode to Sequence */
extern u16 _main_conv_seq(void);
/* Read State */
extern u16 _main_get_aging_time(void);
extern u16 _main_bill_remain(void);
extern u16 _main_bill_in(void);
extern bool _is_main_position_all_off(void);
extern bool _is_main_accept_denomi(u16 denomi_code);
extern u32 convert_index_to_denomi(u16 denomi_code);
extern bool _is_main_bookmark_rejected(u8 reject_code);
/* Set Main Task Mode */
extern void _main_set_mode(u8 mode1, u8 mode2);
extern u8 alarm_index(void);
extern u8 reject_index(void);
/* Send task message */
extern void _main_send_connection_task(u32 msg, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _main_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _main_system_error(u8 fatal_err, u8 code);

extern T_MSG_BASIC ex_main_msg;
#endif /* _SRC_INCLUDE_OPERATION_H_ */
/*--- End of File ---*/
