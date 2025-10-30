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
extern void powerup_clear_icb_setting(void);
extern void powerup_mgu_read(void);
extern void powerup_rfid_unit_init(void);
extern void powerup_dipsw_init(void);
extern void powerup_sensor_init(void);
extern void pwerup_sensor_active(void);
extern void powerup_wait_req(void);
extern void powerup_sensor_active(void);
extern void powerup_stacker_home(void);
extern void powerup_search_bill(void);
extern void powerup_alarm_box(void);
extern void powerup_alarm_confirm_box(void);
#if defined(UBA_RTQ)
extern void powerup_rc_sensor_active(void);
extern void powerup_rc_search_bill(void);
extern void powerup_alarm_rc_unit(void);
extern void powerup_alarm_confirm_rc_unit(void);
#endif // UBA_RTQ

extern void powerup_mag_init(void);
#if !defined(UBA_RTQ)
	void powerup_alarm_receive_reset_gli(void);
#endif

#if defined(UBA_RTQ)
extern u8 _rc_initial_msg_proc(void);
extern u8 software_version_check(void);
#endif

/*============================================================================*/
/* ----- Initialise Mod (MODE1_INIT) ---------------------------------------- */
/*============================================================================*/
extern void initialize_msg_proc(void);
extern void init_sensor_active(void);
extern void init_cis_init(void);

extern void init_feed(void);
extern void init_stacker(void);
extern void init_stacker_half(void);

extern void init_centering(void);
extern void init_apb(void);
extern void init_force_stack(void);

extern void init_icb(void);   
extern void	_feed_bill_over_window_rev(void);
extern void _feed_bill_over_window_fwd(void);
extern void init_shutter(void);
extern void init_initial_position(void);

extern void init_icb(void);
extern void init_wait_req(void);
extern void init_force_feed_stack(void);
extern void init_force_stack(void);
extern void init_wait_remain_req(void);
extern void init_wait_ent_off(void);
extern void init_reject_apb_home(void);
extern void init_feed_reject(void);
extern void init_note_stay(void);
extern void init_wait_reset_req(void);
extern void init_apb_close_sensor_active(void);
extern void init_apb_close(void);

#if defined(UBA_RTQ)
extern void init_rc();
extern void init_rc_wait_recovery_drum_gap_adj();
extern void init_rc_wait_last_feed();
extern void init_rc_wait_recovery_back();
extern void init_rc_wait_recovery_stack_home();
extern void init_rc_wait_recovery_init_rc();
extern void init_rc_wait_recovery_back_box();
extern void init_rc_wait_recovery_force_stack_drum(void);
extern void init_rc_wait_recovery_front_back_box__front(void);
extern void init_rc_wait_recovery_front_back_box__init_rc1(void);
extern void init_rc_wait_recovery_paydrum_box__paydrum_box(void);
extern void init_rc_wait_recovery_box_search_box__box_search(void);
extern void init_rc_wait_recovery_box_search_box__box(void);
extern void init_rc_wait_recovery_stack(void);
extern void init_rc_wait_recovery_bill_back(void);
extern void init_rc_wait_last_feed(void);
#endif

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

extern void active_disable_wait_reject_req(void);
extern void active_disable_stacker_half(void);
extern void active_disable_centering_home(void);
extern void active_disable_shutter_open(void);
extern void active_disable_feed_reject(void);
extern void active_disable_pb_close(void);

/*============================================================================*/
/* ----- Active Enable Mode (MODE1_ACTIVE_ENABLE) --------------------------- */
/*============================================================================*/
extern void active_enable_msg_proc(void);
extern void active_enable_wait_req(void);
extern void active_enable_wait_reject_req(void);
extern void active_enable_stacker_half(void);
extern void active_enable_centering_home(void);
extern void active_enable_shutter_open(void);
extern void active_enable_pb_close(void);
/*============================================================================*/
/* ----- Adjust Mode (MODE1_ADJUST) ----------------------------------------- */
/*============================================================================*/
extern void adjust_msg_proc(void);
extern void adjust_temp_adj(void);

/*============================================================================*/
/* ----- Accepting Mode (MODE1_ACCEPT) -------------------------------------- */
/*============================================================================*/
extern void accept_msg_proc(void);
extern void accept_sensor_active(void);
extern void stacker_home(void);
//extern void centering_home(void);
extern void feed_centering(void);
extern void centering_exec(void);
extern void stacker_home(void);
extern void feed_escrow(void);
extern void discrimination(void);
extern void accept_wait_req(void);
extern void accept_wait_reject_req(void);
extern void pb_centering_home(void);
extern void wait_pb_start(void);
extern void wait_feed_start(void);

#if defined(UBA_RTQ)
extern void accept_init_rc(void);
extern void accept_wait_rc_rsp(void);
#endif // UBA_RTQ

/*============================================================================*/
/* ----- Stacking Mode (MODE1_STACK) ---------------------------------------- */
/*============================================================================*/
extern void stack_msg_proc(void);
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
extern void stack_wait_reject_req(void);
extern void stack_wait_reject_shutter_open(void);
extern void stack_wait_shutter_open_ld_mode(void);
#if (DATA_COLLECTION_DEBUG==1)
extern void stack_feed_data_collection(void);	//only Data collection
#endif
#if defined(UBA_RTQ)
    extern void stack_feed_rc_stack(void);
    extern void stack_rc_feed_box(void);
    extern void stack_rc_retry_prefeed_stack(void);
    extern void stack_rc_retry_rev(void);
    extern void stack_rc_retry_stack_home(void);
    extern void stack_rc_retry_init_rc(void);
    extern void stack_rc_retry_feed_box(void);
    extern void stack_rc_retry_feed_box_revvoid(void);
    extern void stack_rc_retry_feed_box_rev(void);
#endif // UBA_RTQ

/*============================================================================*/
/* ----- Rejecting Mode (MODE1_REJECT) -------------------------------------- */
/*============================================================================*/
extern void reject_msg_proc(void);
extern void reject_sensor_active(void);
extern void feed_reject(void);
extern void stacker_half_pb_close(void);
extern void reject_wait_req(void);
extern void reject_wait_wid(void);
extern void reject_note_removed_wait_sensor_active(void);

/*============================================================================*/
/* ----- Rejecting Standby Mode (MODE1_REJECT_STANDBY) ---------------------- */
/*============================================================================*/
extern void reject_standby_msg_proc(void);
extern void reject_standby_note_stay(void);
extern void reject_standby_confirm_note_stay(void);
extern void reject_standby_wait_reject_req(void);


/*============================================================================*/
/* ----- Pay out Mode (MODE1_PAYOUT) ---------------------------------------- */
/*============================================================================*/
#if defined(UBA_RTQ)	
	extern void payout_msg_proc(void);
	extern void payout_sensor_active(void);
	extern void payout_init_transport(void);
	extern void payout_init_rc(void);
	extern void payout_wait_rc_rsp(void);
	extern void payout_feed_rc_payout(void);
	extern void payout_note_stay(void);
	extern void payout_reject_stop_wait_wid(void);
	extern void payout_wait_stack_start(void);
	extern void payout_wait_stack_top(void);
	extern void payout_wait_stack_home(void);
	extern void payout_exec_retry(void);
	extern void payout_dummy_feed(void);
	extern void payout_rc_retry_prefeed_stack(void);
	extern void payout_wait_req(void);
	extern void payout_rc_retry_fwd(void);
	extern void payout_rc_retry_rev(void);
	extern void payout_rc_retry_stack_home(void);
	extern void payout_rc_retry_init_rc(void);
	extern void payout_rc_retry_feed_box(void);

	extern void payout_wait_req(void);
	//#if defined(ID003_SPECK64)
	extern void payout_wait_pb_close(void);
	//#endif

#endif

/*============================================================================*/
/* ----- Collect Mode (MODE1_COLLECT) --------------------------------------- */
/*============================================================================*/
#if defined(UBA_RTQ)		
	extern void collect_msg_proc(void);
	extern void collect_sensor_active(void);
	extern void collect_init_transport(void);
	extern void collect_init_rc(void);
	extern void collect_wait_rc_rsp(void);
	extern void collect_wait_stack_start(void);
	extern void collect_wait_stack_top(void);
	extern void collect_wait_stack_home(void);
	extern void collect_exec_retry(void);
	extern void collect_wait_req(void);
	extern void collect_rc_retry_prefeed_stack(void);
	extern void collect_rc_retry_fwd(void);
	extern void collect_rc_retry_stack_home(void);
	extern void collect_rc_retry_init_rc(void);
	extern void collect_rc_retry_feed_box(void);
#endif // UBA_RTQ

/*============================================================================*/
/* ----- Alarm Mode (MODE1_ALARM) ------------------------------------------- */
/*============================================================================*/
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
extern void alarm_acceptor_cis_jam(void); //2024-06-09
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

extern void alarm_confirm_stacker_con(void);

extern void alarm_rfid(void);
extern void alarm_cpu_board(void);
extern void alarm_cpu_board_wait_reset(void);

extern void alarm_shutter_fail(void);
extern void alarm_pusher_home(void);
extern void alarm_wait_sensor_active(void);
extern void alarm_cis_temperature(void); //2024-05-28
extern void active_alarm_tmp_wait_read(void); //2024-05-28

#if defined(UBA_RTQ)
	extern void alarm_rc_error(void);
	extern void alarm_confirm_rc_error(void);
	extern void alarm_confirm_rc_unit(void);
#endif // UBA_RTQ
/*============================================================================*/
/* ----- Test Mode (MODE1_TEST) --------------------------------------------- */
/*============================================================================*/
extern void active_alarm_msg_proc(void);
extern void test_standby_msg_proc(void);
extern void test_active_msg_proc(void);
extern void test_init(void);
extern void test_standby(void);
extern void test_sensor_active(void);
extern void test_feed_motor(void);
extern void test_stacker(void);
extern void test_stacker_home(void);
extern void test_stacker_motor(void);

extern void test_stacker_retry(void);
extern void test_centering_open_uba(void);
extern void test_centering_close_uba(void);
extern void test_apb(void);
extern void test_sensor(void);

extern void test_mag_write(void);
extern void test_dipsw(void);
extern void test_aging_sensor_init_wait(void);
extern void test_aging_sensor_init(void);
extern void test_aging_cis_init(void);
extern void test_aging_sensor_active(void);

extern void test_aging_feed_centering_uba(void);
extern void test_aging_centering_uba(void);
extern void test_aging_feed_escrow_uba(void);
extern void test_aging_feed_apb_uba(void);
extern void test_aging_apb_close_uba(void);
extern void test_aging_apb_open_uba(void);
extern void test_aging_stack_exec_uba(void);
extern void test_aging_stack_exec_home_uba(void);	
extern void  test_aging_shutter_close_uba(void);
extern void test_aging_shutter_open_uba(void);
extern void test_shutter_motor(void);
extern void test_shutter_motor_close(void);
extern void test_rfid_uba(void);


extern void clear_ex_multi_job(void);   /* 2022-02-21 */
#if defined(UBA_RTQ)
	extern void test_rc_communication(void);
	extern void test_rc_dipsw(void);
	extern void test_rc_sw_led(void);

	extern void test_rc_feed(void);
	extern void test_rc_flap(void);
	extern void test_rc_sensor(void);
	extern void test_rc_sol(void);
	extern void test_rc_drum(void);

	extern void test_rc_serial_no(void);
	extern void test_rc_sens_adj(void);
	extern void test_rc_flap_usb(void);
	extern void test_rc_drum_tape_pos_adj(void);
	extern void test_rc_fram_check(void);
	extern void test_rc_sens_adj_write_fram(void);
	extern void test_rc_sens_adj_read_fram(void);
	extern void test_rc_perform_test_write_fram(void);
	extern void test_rc_perform_test_read_fram(void);
	extern void test_rc_edition_no(void);
	//#if defined(RC_BOARD_GREEN)
	extern void test_rc_wait_sensor_adj(void);
		//#if defined(UBA_RS)
		extern void test_rc_rs_flap(void);
		extern void test_rc_rs_flap_usb(void);
		//#endif
	//#endif
	//#if defined(UBA_RTQ_ICB)
	extern void test_rc_rfid(void);
	//#endif
#endif 

extern void test_wait(void);
extern void test_alarm(void);
/*============================================================================*/
/* ----- Operation Sub (sub functions) -------------------------------------- */
/*============================================================================*/
//extern void musb_maintenance_sub_proc(void);
/* Set Mode */
extern void _main_select_protocol(void);
extern void _main_set_init(void);
extern void _main_set_powerup_search(void);
extern void _main_set_powerup_alarm_box(void);
extern void _main_set_disable(void);
extern void _main_set_enable(void);

extern void _main_set_accept(void);
extern void _main_set_adjustment(void);

extern u8 is_uv_led_check_uba(void);
extern void _main_set_reject(void);
extern void _main_set_active_enable_uba(u8 mode);
extern void _main_set_active_disable_uba(u8 mode);
extern  u16 _main_stay_bill_check(void);

extern void _main_set_reject_standby_note_stay(void);
extern void _main_set_reject_standby_wait_req(void);
extern void _main_set_sensor_active( u8 enable);
extern void _main_set_pl_active(u8 active);
extern void _main_set_test_standby(void);
extern void _main_set_test_active(void);
extern void _main_set_test_wait(void);

extern void _main_alarm_sub(u32 mode1, u32 mode2, u32 rsp_msg, u32 code, u32 seq, u32 sensor);
extern void _main_reject_sub(u32 mode1, u32 mode2, u32 rsp_msg, u32 code, u32 seq, u32 sensor);
extern void _main_reject_req(u32 mode1, u32 mode2, u32 code, u32 seq, u32 sensor);
/* Display */

extern void _main_display_init(void);

extern void _main_display_adj(void);
extern void _main_display_disable(void);
extern void _main_display_enable(void);
extern void _main_display_powerup(void);

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
extern bool _is_main_bookmark_rejected(u8 reject_code);


extern void _main_set_position_da(void);
extern void _main_set_position_gain(void);

/* Set Main Task Mode */
extern void _main_set_mode(u8 mode1, u8 mode2);
extern u8 alarm_index(void);
extern u8 reject_index(void);
/* Send task message */
extern void _main_send_connection_task(u32 msg, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _main_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _main_system_error(u8 fatal_err, u8 code);

extern T_MSG_BASIC ex_main_msg;



#if defined(UBA_RTQ)
	extern void _main_set_payout_or_collect(u8 type); //2024-11-13
	/* active rc1 */
	extern void mode_test_active_rc1_proc(void);
	/* active rc2 */
	extern void mode_test_active_rc2_proc(void);
	/* active rc3 */
	extern void mode_test_active_rc3_proc(void);

	//#if defined(RC_BOARD_GREEN)
	extern void mode_test_active_rs_sh3_usb_proc(void);
	//#endif // RC_BOARD_GREEN
	//#if defined(UBA_RTQ_ICB)
	extern void mode_test_active_rs_sh3_proc(void);
	//#endif // RC_RIFD_MODULE

	/*============================================================================*/
	/* ----- Operation Sub (sub functions for RC) ------------------------------- */
	/*============================================================================*/
	extern void is_recycle_denomi_check(void);
	extern bool rc_busy_status(void);
	extern bool rc_warning_status(void);
	extern bool rc_initial_status(void);
	extern bool is_quad_model(void);
	extern bool is_detect_rc_twin(void);
	extern bool is_detect_rc_quad(void);
	extern bool is_flapper1_head_to_twin_pos(void);
	extern bool is_flapper1_head_to_box_pos(void);
	extern bool is_flapper1_twin_to_box_pos(void);
	extern bool is_flapper2_head_to_quad_pos(void);
	extern bool is_flapper2_head_to_box_pos(void);
	extern bool is_flapper2_quad_to_box_pos(void);

	//#if defined(RC_SKEW_DETECT)		/* '19-07-05 */
	extern u8 check_recycle_skew_detect(void);
	//#endif
	extern u8 check_recycle_limit_count(void);
	extern u8 check_recycle_full_sensor(void);
	extern u8 check_bill_length(void);
	extern void is_recycle_denomi_check(void);
	extern void rc_twin_set_bill_info(u8 cnt);
	extern void is_recycle_set_test_denomi(void);
	extern void is_recycle_store_serial_no(void);
	extern void is_recycle_set_usb_test_denomi(u8 kinshu, u8 cnt);
	extern void rc_twin_set_bill_info(u8 cnt);
	extern void is_recycle_aging_accept(void);
	extern void is_recycle_aging_payout(void);

	extern bool is_rc_twin_d1_empty(void);
	extern bool is_rc_twin_d2_empty(void);
	extern bool is_rc_quad_d1_empty(void);
	extern bool is_rc_quad_d2_empty(void);
	extern u8 is_pre_feed_check(void);
	extern bool is_detect_rc_jam_check(void);
	//#if defined(RC_INTERNALJAM)		/* '19-09-19 */
	extern u8 is_detect_rc_internal_jam(void);
	extern u8 is_detect_rc_internal_jam_initial(void);
	//#endif
	//#if defined(UBA_RS)
	extern u8 is_rs_mode_remain_note_check();
	//#endif
	extern bool is_rc_error_check(void);
	extern bool is_rc_unit_error(void);
	extern bool is_rc_serial_no_check(void);
	#if defined(A_PRO)
	extern bool is_rc_serial_no_check2(void);
	#endif
	extern u16 get_rc_recovery_status(u8 *unit, u8 *sts, u8 seach);
	extern void initialize_encryption(void);
	extern void renewal_cbc_context(void);
	extern void set_encryption_number(u8 type);
	extern void set_encryption_key(u8 type);
	extern void GC2Init(const u8 *pSerialNum, u8 *pCBC);
	extern bool is_feed_quad_model(void);
	extern bool is_rc_rs_unit(void);
	extern bool is_rc_error(void);
#endif

#if defined(UBA_RTQ_ICB)
void _main_rtq_rfid(void);
#endif

#endif /* _SRC_INCLUDE_OPERATION_H_ */
/*--- End of File ---*/
