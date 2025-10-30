/*
 * icb.h
 *
 *  Created on: 2022/03/14
 *      Author: suzuki-hiroyuki
 */

#ifndef ROOT_USER_ICB_ICB_H_
#define ROOT_USER_ICB_ICB_H_

enum _ICB_FLAG
{
	ICB_NO_DATA = 0,				//Read write Toolが設定		Read Write Toolで初期化されたBOX
	ICB_INSTALLED = 1,				//Headが設定				初期化済みBOXでHeadとのイニシャル動作完了
	ICB_DATA_EXIST = 2,				//Headが設定				データを何か更新済み
	ICB_DATA_HAS_BEEN_READ = 3,		//Read write Toolが設定		Readのみ完了、エラーにする
	ICB_AUTO_INITIAL_REQUEST = 4,	//Read write Toolが設定		強制初期化(ivizion) ICB_NO_DATA と同じ扱いで初期化する
	ICB_SYSTEM_INHIBIT = 0x55
};
#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
	#define	SIZE_OF_1BLK	8
#else
	#define	SIZE_OF_1BLK	4
#endif

#if !defined(UBA_RTQ)
	typedef struct _icb_blk_info
	{
		int	address;
		int	size;
		u8	*buffer;
	} icb_blk_info;
	extern const struct _icb_blk_info	send_blk_info[];
#endif

void icb_totalInsert_counter(void);
bool is_box_flag_no_data(void);
bool is_box_flag_installed(void);
bool is_box_flag_data_exist(void);
bool is_box_flag_read(void);
bool is_box_flag_initial_request(void);
bool is_box_flag_inhibit(void);
int icb_check_machineNo(void);
int is_icb_checksum_error(void);
int culc_BLK1_checksum(void);
int culc_BLK2_checksum(void);
#if !defined(UBA_RTQ)
	void icb_update_reject_counter(int code);
	void icb_send_buffer_initial_info(void);
	BOOL icb_check_recovery_data(void);
	bool is_ICBsend_flag_broken(void);
	void icb_set_initial_all_data(void);
	int set_ICBrecovery_data(int num);
	int icb_update_error_counter(int code);
	RTC_INFO get_date_from_RTC(void);
	int icb_update_denomi_counter(u8 ex_denomi);
	int icb_update_ticket_counter(void);
	int icb_update_total_counter(void);
#endif

//int MCnumberTicket(void);
//int ICBenableTicket(void);
//int ICBdisableTicket(void);

int icb_BLKnumber(u8 req);
int icb_update_ticket_number(void);
int icb_machine_number_is_valid(void);
void renewal_ICBsend_flag(void);
u8 icb_get_version_address(void);
void icb_set_initial_info(void);
u8 icb_savnosum(void);
int check_ICBflag(void);

void set_ICBdisable_flag(void);
void set_ICBenable_flag(void);
void Check_Savgameno(void); //2023-11-09
void set_MCnumber(u8 *number);
void icb_clear_recovery_flag(int mode);
void icb_update_ticket_reject_counter(int code);
u32	 make_serial_time(RTC_INFO *_date);
RTC_INFO Convert32bitToTimeUnix(u32 time_32bit);
u32 ConvertTimeUnixTo32bit(RTC_INFO time_unix);

void set_box_flag(enum _ICB_FLAG flg_value);
void  fill_memo32( u32 data, u32 *ptr, u32 size);

#if defined(UBA_RTQ)
void update_checksum2();
#endif 

#endif /* ROOT_USER_ICB_ICB_H_ */
