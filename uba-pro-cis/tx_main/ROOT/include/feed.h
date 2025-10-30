/******************************************************************************/
/*! @addtogroup Group2
    @file       
    @brief      
    @date       2024/05/15
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    Japan CashMachine Co, Limited. All rights reserved.
******************************************************************************/
/************************** PRIVATE DEFINITIONS *************************/
#ifndef _FEED_H_
#define _FEED_H_
#ifdef __cplusplus
extern "C"
{
#endif

/*------------------------------------------------------------------------------------*/
extern T_MSG_BASIC feed_msg;
extern u16 s_feed_task_wait_seq;
extern u16 s_feed_task_pause_seq;
extern u8 s_feed_alarm_code;
extern u8 s_feed_alarm_retry;
extern u8 s_feed_reject_code;
extern u8 s_feed_reject_option;
extern u16 s_feed_sensor_backup; //not use UBA500では強制収納の時にセンサ状態に矛盾があった場合のエラーに使用しているが、大切ではないので使用しない
extern u8 s_feed_aging_no;
extern u8 s_feed_freerun_dir;
extern u8 s_feed_search_option;
extern u8 s_feed_pullback_off;	//2023-11-01
extern bool ex_reject_escrow;	/* 0x04XX	*/
extern u16 backup_feed_apb_sequence; // use

/*------------------------------------------------------------------------------------*/
#define FEED_SPEED_LIMIT_MAX		0xFFFF					/* MAX SPEED */
#define CONV_SPEED(x)	(( (x) / PITCH )/10)	/* mm -> pulse count /100ms */
#define FEED_SPEED_200MM			(80)
#define FEED_SPEED_300MM			(120)
#define FEED_SPEED_450MM			(180)
#define FEED_SPEED_600MM			(240)
#define FEED_SPEED_750MM			(300)
#define FEED_SPEED_900MM			(360)
#define FEED_SPEED_1200MM			(480)

//#define FEED_SPEED_ERR_LOWER_LIMIT	29					/* 200mm/sec :  29pulse/100msec = (200 / 0.686) / 10 */
//#define FEED_SPEED_ERR_UPPER_LIMIT	102					/* 700mm/sec : 102pulse/100msec = (800 / 0.686) / 10 */
#define FEED_SPEED_ERR_LOWER_LIMIT	FEED_SPEED_200MM		/* 200mm/sec :  80pulse/100msec = (200 / 0.25) / 10 */
#define FEED_SPEED_ERR_UPPER_LIMIT	FEED_SPEED_1200MM
enum _FEED_SEQ
{
	FEED_SEQ_IDLE			= 0x0000,
	FEED_SEQ_FORCE_QUIT		= 0x0001,
	FEED_SEQ_INITIAL		= 0x0100,
	FEED_SEQ_CENTERING		= 0x0200,
	FEED_SEQ_ESCROW			= 0x0300,
	FEED_SEQ_APB			= 0x0400,
#if !defined(UBA_RTQ)
	FEED_SEQ_REV_CHECK_BILL	= 0x0500, //#if defined(HIGH_SECURITY_MODE) //Check for remaining bills on the box transport path after stack
#endif
	FEED_SEQ_FORCE_STACK	= 0x0600,
	FEED_SEQ_REJECT			= 0x0700,
	FEED_SEQ_FORCE_REV		= 0x0800, //2024-03-18a ID-003 Disable, Forced return of bills detected at entrance

	FEED_SEQ_SEARCH			= 0x0A00,
#if defined(UBA_RTQ)
	ENTRY_SEQ_BACK			= 0x0B00,
#endif
	FEED_SEQ_AGING			= 0x0E00,
	FEED_SEQ_FREERUN		= 0x0F00,
#if defined(UBA_RTQ)
	FEED_SEQ_RC_STACK		= 0x1200,
	FEED_SEQ_RC_PAYOUT		= 0x1300,
	FEED_SEQ_RC_COLLECT		= 0x1400,
	FEED_SEQ_RC_FORCE_PAYOUT= 0x1600, //payout以外でも使用している。Payoutというより、ドラムから背面搬送まで戻す処理、背面搬送のポジションセンサ1を紙幣が完全に通過するまで、払い出し方法に回す
	FEED_SEQ_RC_FORCE_STACK	= 0x1700, //背面搬送で紙幣が検知できなくなるまで、取り込み方法に回す,フラッパ動作させてないので、おそらくドラムの方へ行く
	FEED_SEQ_RC_BILL_BACK	= 0x1800,
	FEED_SEQ_RC_PAYOUT_STOP	= 0x1900,
	FEED_SEQ_RC_CLEANING	= 0x2000,
	//#if defined(UBA_RS)
	FEED_SEQ_RS_PAYOUT 		= 0x2300,
	FEED_SEQ_RS_FORCE_PAYOUT = 0x2400,
	//#endif // uBA_RS
#endif // UBA_RTQ
};

enum _ALARM_CODE_FEED
{
	FEED_OTHER_SENSOR = 0,
	FEED_SLIP,
	FEED_TIMEOUT,
	FEED_MOTOR_LOCK,
};

#define FEED_SEQ_TIMEOUT				3000	/* 3sec */
#if defined(UBA_RTQ)
	#define FEED_RC_SEQ_TIMEOUT				3000	/* 3sec */
	#define FEED_SEQ_RC_TIMEOUT				8000	/* シーケンスタイムアウト(8sec) */
	#define	FEED_SEQ_RC_STACK_TIMEOUT		1500	/* シーケンスタイムアウト(1.5sec) */
	#define	FEED_SEQ_RS_STACK_TIMEOUT		2000	/* シーケンスタイムアウト(2.0sec) */
#endif // UBA_RTQ

#define	FEED_SPEED_CHECK_1ST_TIME		400		/* 400msec */
#define	FEED_SPEED_CHECK_TIME			100		/* 100msec */
#define FEED_PAUSE_CHECK_TIME			100		/* 100msec */
#define FEED_FREERUN_CHECK_TIME			1000	/* 1sec */
#define FEED_APB_PAUSE_TIMEOUT			10000	/* 5sec->10s *//* UBA500 10s*/
#define FEED_PAYOUT_PAUSE_TIMEOUT		5000	/* 5sec */
#define FEED_SERCH_REV_TIMEOUT			1000	/* 1sec */
#define FEED_SERCH_FWD_TIMEOUT			250		/* 250msec */
#define FEED_LAST_FEED_TIMEOUT			500		/* 500msec */


//RBA-40 CIS edge falling , rising
#define FEED_MOTOR_SLIP_LIMIT			CONV_PULSE(300)		/* 300mm (1200*0.25) */
#define FEED_MOTOR_SLIP_LIMIT_SPG		CONV_PULSE(50)		/* 50mm (ad sampling) */

#define FEED_STACK_RETRY_COUNT			5
#define FEED_STACK_RETRY_REV_PULSE		CONV_PULSE(20)		/* 10  => 20 UBA500同様に*///2024-04-10

#define FEED_REJECT_RETRY_COUNT			5
#define FEED_REJECT_NONBILL_LPULSE		CONV_PULSE(100)		/* 100mm (400*0.25) */
#define FEED_REJECT_SHORT_LPULSE		CONV_PULSE(400)		/* 400mm (1600*0.25) */
#define FEED_REJECT_RETRY_FWD_PULSE		CONV_PULSE(50)		/* 50mm (200*0.25) */
#define FEED_REJECT_RETRY_FWD_TIME		250					/* 250msec */

#define FEED_SEARCH_PULSE				CONV_PULSE(5)		/* 5mm (20*0.25) */
#define FEED_MARGIN_PULSE				CONV_PULSE(10)		/* 10mm (40*0.25) */

#define FEED_INIT_RETRY_COUNT			10

#define FEED_SET_SLIP(x)				((CONV_PULSE(x))+FEED_MOTOR_SLIP_LIMIT_SPG)
#define FEED_NEXT_SEQ(ftbl)				(ftbl[(ex_feed_task_seq & 0x000F)].next_seq)
#define FEED_NEXT_PULSE(ftbl)			(ftbl[(FEED_NEXT_SEQ(ftbl) & 0x000F)].pulse)

#define IS_FEED_EVT_SENSOR(x)			((x & EVT_FEED_SENSOR) == EVT_FEED_SENSOR)
#define IS_FEED_EVT_OVER_PULSE(x)		((x & EVT_FEED_OVER_PULSE) == EVT_FEED_OVER_PULSE)

#define IS_FEED_EVT_MOTOR_STOP(x)		((x & EVT_FEED_MOTOR_STOP) == EVT_FEED_MOTOR_STOP)
#define IS_FEED_EVT_TIMEOUT(x)			((x & EVT_FEED_TIMEOUT) == EVT_FEED_TIMEOUT)
#define IS_FEED_EVT_MOTOR_LOCK(x)		((x & EVT_FEED_MOTOR_LOCK) == EVT_FEED_MOTOR_LOCK)
#define IS_FEED_EVT_CIS_SENSOR(x)		((x & EVT_FEED_CIS) == EVT_FEED_CIS) //2024-01-30

#define FEED_HOLE_START_MIN				(27*PITCH)		/* 27mm (pulse)  */
#define FEED_HOLE_DISTANCE_LIMIT		(25*PITCH)		/* 25mm (pulse) */

#define FEED_SEQ_ENTRANCE_TIMEOUT		4000	/* 4sec */

#define IS_FEED_EVT_CIS_SKEW(x)			((x & EVT_FEED_CIS_SKEW) == EVT_FEED_CIS_SKEW)
#define IS_FEED_EVT_CIS_MLT(x)			((x & EVT_FEED_CIS_MLT) == EVT_FEED_CIS_MLT)
#define IS_FEED_CIS_SHORT_EDGE(x)		((x & EVT_FEED_CIS_SHORT_EDGE) == EVT_FEED_CIS_SHORT_EDGE)
#define FEED_ACCEPT_RETRY_COUNT			5
#define FEED_REJECT_RETRY_FWD_PULSE1	CONV_PULSE(10)		/* 10mm (200*0.25) *//* iVIZION同等、PBIN,PBOUT,EXITセンサーON or LOSTの場合 */
#define FEED_REJECT_RETRY_FWD_PULSE2	CONV_PULSE(25)		/* 25mm (200*0.25) *//* iVIZION同等、CIS or CENTセンサーONの場合 */
#define FEED_REJECT_RETRY_FWD_PULSE3	CONV_PULSE(60)		/* 60mm (200*0.25) *//* iVIZION同等、入口センサーONの場合 */

//トリガ使用 厳しく /* 停止状態からの返却の最終搬送、紙幣ハンギング位置まで */
#define FEED_HUNGING_POSITION_START		(u16)(50/PITCH)

//最長設定、トリガではない タイムアウト用 緩く/* 最長はBOXから、幅よせONまで(EXIT～幅寄せ185, + 札長180+マージン180) */
#define FEED_REJECT_CENTERING_ON_TO_OFF	(u16)(545/PITCH)
// Reject時のリトライ正転距離
#define FEED_REJECT_RETRY1_FWD_SHORT_PULSE	(u16)(20/PITCH)
#define FEED_REJECT_RETRY2_FWD_SHORT_PULSE	(u16)(40/PITCH)
#define FEED_REJECT_RETRY1_FWD_LONG_PULSE	(u16)(70/PITCH)
#define FEED_REJECT_RETRY2_FWD_LONG_PULSE	(u16)(90/PITCH)

// Reject時の搬送
//最長設定、トリガではない 緩く/* 幅よせはONしていて、入口センサのみONまでの返却パルス(幅よせOFFまで) */
#define FEED_REJECT_CENTERING						(u16)(180/PITCH)
#define FEED_HANGING_POSITION_NEW					(u16)(10/PITCH) /* 20->10 *//* CISと搬送惰性により*/
#define FEED_NO_HANGING_POSITION_SHORT_TICKET		(u16)(70/PITCH)
#define FEED_HANGING_POSITION_SHORT_TICKET			(u16)(10/PITCH)
#define FEED_SEQ_STOP_CONF_TIMEOUT		200	//use/* 200sec *//* モータ惰性を考えて紙幣位置安定まで待つ*/

//#define PITCH	0.254
// Escrowから収納までの搬送 2022-02-14 Escrow位置がかわったので変更する
#define WAIT_PB_IN_OFF		(u16)(13/PITCH)	//トリガ使用 最低XXmm ESCROW位置から最低XX搬送しないとPB IN OFFにならないという条件 13mm ok
#define WAIT_PB_OUT_OFF		(u16)(57/PITCH)	//トリガ使用 最低XXmm PB IN OFF位置から最低XX搬送しないとPB OUT OFFにならないという条件 57mm ok
#define WAIT_EXIT_OFF		(u16)(35/PITCH)	//トリガ使用 最低XXmm PB OUT OFF位置から最低XX搬送しないとEXIT OFFにならないという条件 35mm ok
//トリガ使用 	厳しく/ 幅よせセンサがONしてから停止命令までの搬送距離 (理論値23mm) // itou seq6 メカ要望のトリガ
#define STOP_WID			(u16)(16/PITCH) //ok 18->16
#define	FEED_REV_CHECK_BILL				500		//use//

#define WAIT_LAST_FEED				(u16)(55/PITCH)	//not use RTQ, only use SS トリガ使用 EXIT OFF位置から収納開始位置までの搬送距離 // itou seq28 メカの要望 BOX内のばらつきの為、既存機種より伸ばす トリガ
#define WAIT_LAST_FEED_FORCE		(u16)(75/PITCH)	//maybe not use RTQ, only use SS トリガ使用 EXIT OFF位置から収納開始位置までの搬送距離 // itou seq28 メカの要望 BOX内のばらつきの為、既存機種より伸ばす トリガ


/*-- only RTQ --------------------------------------------------------*/
#define FEED_SEARCH_IN_PULSE			(u16)(60/PITCH)





/*-- only RTQ --------------------------------------------------------*/


#ifdef __cplusplus
}
#endif
#endif
