/******************************************************************************/
/*! @addtogroup Main
    @file       display_task.c
    @brief      control status led task function
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "sub_functions.h"
#include "hal.h"

#include "systemdef.h"					//2023-05-11 test
#include "cyclonev_sysmgr_reg_def.h"	//2023-05-11 test
#include "hal_gpio_reg.h"				//2023-05-11 test

#include "cyc.h"


#define EXT
#include "com_ram.c"
/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

static T_MSG_BASIC errdisp_msg;
static u16 s_errdisp_led_ctrl_time;			//LED点滅用のインターバルカウンタ
static u16 s_errdisp_led_ctrl_count_red;	// 実際に点滅した回数
static u16 s_errdisp_led_ctrl_count_green;	// 実際に点滅した回数


enum _ERRDISP_MODE
{
	ERRDISP_MODE_OFF = 1,
	ERRDISP_MODE_ON,
	ERRDISP_MODE_BLINK_ON,
	ERRDISP_MODE_BLINK_LAST,
	ERRDISP_MODE_BLINK_1SHOT,	//only test mode denomi flash
	ERRDISP_MODE_IDLE,
};

struct DISP_LED_CTRL_VAL
{
	u16 mode;
	u8 code;		// エラーコード
	u8 red_count;
	u8 red_mode;
	u8 green_count;
	u8 green_mode;
	u16 interval;	// 1回の点滅周期
	u8 red_current_count;
	u8 green_current_count;
	u16 round_wait;
};
extern struct DISP_LED_CTRL_VAL s_errdisp_ctrl;
struct DISP_LED_CTRL_VAL s_errdisp_ctrl;



#define DISP_INTERVAL_MIDDLE	3
#define DISP_ROUNDWAIT_MIDDLE	20		/* 2sec */// 点滅の次の点滅までの待ち時間
#define DISP_DISABLE_UBA	0

struct ERRDISP_LED_SET_TBL
{
	u8 code;		// エラーコード
	u8 red_count;
	u8 red_mode;
	u8 green_count;
	u8 green_mode;
	u16 interval;
};

struct ERRDISP_LED_SET_TBL2
{
	u8 code;		// エラーコード
	u8 red_count;
	u8 red_mode;
	u8 green_count;
	u8 green_mode;
	u16 interval;
	u8 rfid_index;
};


const struct ERRDISP_LED_SET_TBL ex_status_led_ctrl_tbl[]=
{
/*--- led control code  ---*/
	{DISP_CTRL_TEST_STANDBY,			1,  STATUS_LED_ON,		1,	STATUS_LED_ON, 		DISP_INTERVAL_MIDDLE} /* perfomance test standby *///use
#if defined(UBA_RTQ)
	,{DISP_CTRL_TEST_RC_GREEN,			0,  STATUS_LED_OFF,		1,	STATUS_LED_ON, 		DISP_INTERVAL_MIDDLE} /* LED Green on *///use
	,{DISP_CTRL_TEST_RC_RED,			1,  STATUS_LED_ON,		0,	STATUS_LED_OFF, 	DISP_INTERVAL_MIDDLE} /* LED red on *///use
	,{DISP_CTRL_TEST_RC_ON,				1,  STATUS_LED_ON,		1,	STATUS_LED_ON, 	    DISP_INTERVAL_MIDDLE} /* 2 LED  on *///use
	,{DISP_CTRL_TEST_RC_OFF,			0,  STATUS_LED_OFF,		0,	STATUS_LED_OFF, 	DISP_INTERVAL_MIDDLE} /* 2 LED  off *///use
#endif // UBA_RTQ
};


const struct ERRDISP_LED_SET_TBL2 ex_errdisp_reject_tbl[]=
{

	/* 基本ivizon2に合わせる,ivizon2はUVで2,MAGは存在しないがコード上2だがUBA700はUV2 MAG 12とする*/
	/* UBA500で使用していない、点滅はなるべく使用しないようにする10,11を9に統一*/
	/* ICBは、2,4,6,7,8,9,10,13,14,15しか使えない */
	/* ICBのReject項目にない、1,3,5はICBカウントさせない */
	{REJECT_CODE_SKEW,					0,0,	1,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0} /* skew *///ok
	,{REJECT_CODE_MAG_PATTERN,			0,0,	12,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,10} /* mag pattern *///ok not use
	,{REJECT_CODE_MAG_AMOUNT,			0,0,	12,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,10} /* mag amount *///ok
	,{REJECT_CODE_ACCEPTOR_STAY_PAPER,	0,0,	3,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0} /* acceptor stay paper *///ok
	,{REJECT_CODE_XRATE,				0,0,	4,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,4} /* x-rate *///ok not use
	,{REJECT_CODE_INSERT_CANCEL,		0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0} /* insert cancel */
	,{REJECT_CODE_FEED_SLIP,			0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0} /* feed slip *///ok
	,{REJECT_CODE_FEED_MOTOR_LOCK,		0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0} /* feed motor lock *///ok
	,{REJECT_CODE_FEED_TIMEOUT,			0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0} /* feed time out */
	,{REJECT_CODE_APB_HOME,				0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0} /* apb home position error *///ok
	,{REJECT_CODE_CENTERING_HOME,		0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0} /* centering home position error *///ok not use
	,{REJECT_CODE_LOST_BILL,			0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0} /* lost bill *///ok
	,{REJECT_CODE_PRECOMP,				0,0,	6,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,6} /* precompare *///ok
	,{REJECT_CODE_PATTERN,				0,0,	7,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,7} /* photo pattern *///ok not use
	,{REJECT_CODE_PHOTO_LEVEL,			0,0,	8,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,8} /* photo level *///ok not use
	,{REJECT_CODE_INHIBIT,				0,0,	9,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,9} /* inhibit *///ok
	,{REJECT_CODE_ESCROW_TIMEOUT,		0,0,	9,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,9} /* escrow time out *///ok not use
	,{REJECT_CODE_RETURN,				0,0,	9,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,9} /* receive return *///wait ivizon10
	,{REJECT_CODE_OPERATION,			0,0,	9,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,9} /* operation error *///wait ivizion11
	,{REJECT_CODE_LENGTH,				0,0,	13,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,13} /* paper length out range *///ok not use
	,{REJECT_CODE_PAPER_SHORT,			0,0,	13,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,13} /* paper too short *///ok
	,{REJECT_CODE_PAPER_LONG,			0,0,	13,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,13} /* paper too long *///ok
	,{REJECT_CODE_SYNC,					0,0,	14,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,14} /* Sync error */// not use
	,{REJECT_CODE_COUNTERFEIT,			0,0,	15,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,15} /* Rejected by a Custom & NN1 & NN2 check *///ok
	,{REJECT_CODE_FAKE_MCIR,			0,0,	15,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,15} /* Rejected by a MCIR check *///wait 14
	,{REJECT_CODE_FAKE_M3C,				0,0,	15,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,15} /* Rejected by a M3C check *///wait 14
	,{REJECT_CODE_FAKE_M4C,				0,0,	15,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,15}	/* Rejected by a M4C check *///wait 14
	,{REJECT_CODE_FAKE_IR,				0,0,	15,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,15}	/* Rejected by a IR check *///wait 14
	,{REJECT_CODE_HOLE,					0,0,	16,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,15}	/* hole note *///ivizion16
	,{REJECT_CODE_TEAR,					0,0,	17,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,15}	/* tear note *///ivizion17
	,{REJECT_CODE_DOG_EAR,				0,0,	18,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,15}	/* dog ear *///ivizion18
	,{REJECT_CODE_DYENOTE,				0,0,	19,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,15} /* dye note *///ivizion19
	,{REJECT_CODE_UV,					0,0,	2,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,2}	/* uv *///wait 15
	,{REJECT_CODE_THREAD,				0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0}	/* security thread *///wait 15
	,{REJECT_CODE_LOST_BILL,			0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0}	/* lost bill */
	,{REJECT_CODE_BOX,					0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0}	/* box set error *///ok
	,{REJECT_CODE_STACKER_HOME,			0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0}	/* stacker pusher home position error *///ok
	,{REJECT_CODE_STACKER_STAY_PAPER,	0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0}	/* stacker stay paper *///wait not use

	/* ivizon2に合わせる*/
	/* バーチケット裏面はUBAシリーズは9回だが、iviozn2に合わせて11回*/
	/* BAR coupon *//* RFID側の足りないので、2に一部統一*/
	/* 3,6,7はRFIDのアサインがないので、カウントできないので11に統一 */
	,{REJECT_CODE_BAR_NC,				0,0,	1,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,1}
	 /* [BAR] need configuration *///ok 0x91 ITF_BARCODE_RESULT_EDGE_ERROR = BAR_NC
	//BAR_NC

	//ITF_BARCODE_RESULT_EDGE_ERROR
	//ITF_BARCODE_RESULT_AREA_ERROR
	//ITF_BARCODE_RESULT_4WAY_ERROR
	,{REJECT_CODE_BAR_UN,				0,0,	2,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,2}
	 /* [BAR] unknown code *///ok 0x92 = BAR_UN
	//BAR_UN
	//ITF_BARCODE_RESULT_WIDTH_ERROR
	//ITF_BARCODE_RESULT_COUNT_ERROR

	,{REJECT_CODE_BAR_SH,				0,0,	3,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,11}
	 /* [BAR] under or over read *///ok 0x93 存在しな�?可能性があ�?
	//BAR_SH
	//char error
	
	,{REJECT_CODE_BAR_ST,				0,0,	4,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,4}
	 /* [BAR] start bit missing *///ok 0x94
	//BAR_ST
	//ITF_BARCODE_RESULT_START_STOP_CODE_ERROR

	,{REJECT_CODE_BAR_SP,				0,0,	5,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,5}
	 /* [BAR] stop bit missing *///ok 0x95
	//BAR_SP
	//ITF_BARCODE_RESULT_UNKNOWN_DIGITS_ERROR

	,{REJECT_CODE_BAR_TP,				0,0,	6,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,11}
	 /* [BAR] type not enable *///ok 0x96
	//BAR_TP
	//ITF_BARCODE_RESULT_ICB_ATTRIBUTE_ERROR
	//ITF_BARCODE_RESULT_ICB_CHECK_ERROR

	,{REJECT_CODE_BAR_XR,				0,0,	7,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0}
	 /* [BAR] x-rate *///ok 0x97 存在しな�?可能性があ�?
	//not use ok

	,{REJECT_CODE_BAR_PHV,				0,0,	8,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,8}
	 /* [BAR] photo level *///ok 0x98
	//BAR_PHV
	//ITF_BARCODE_RESULT_DOUBLE

//	,{REJECT_CODE_BAR_DIN,				0,0,	9,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,9}
	,{REJECT_CODE_BAR_DIN,				0,0,	0xB,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,9} //11

	 /* [BAR] reverse *///wait 0x9B 使用している
	//BAR_DIR_MISS
	//ITF_BARCODE_RESULT_UP_SIDE_DOWN_ERROR

//	,{REJECT_CODE_BAR_LG,				0,0,	10,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,10}
	,{REJECT_CODE_BAR_LG,				0,0,	0xD,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,10}//13

	 /* [BAR] length out of range *///wait 0x9D 使用している
	//BAR_LG 0xD
	//ITF_BARCODE_RESULT_SIZE_ERROR

//	,{REJECT_CODE_BAR_NG,				0,0,	0,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0}
	,{REJECT_CODE_BAR_NG,				0,0,	0xE,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0} //14

	 /* [BAR] invalid coupon *///wait 0x7E not use
	//BAR_NG
	//not use ok

//	,{REJECT_CODE_BAR_MC,				0,0,	0,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0}
	,{REJECT_CODE_BAR_MC,				0,0,	0xE,	STATUS_LED_BLINK, DISP_INTERVAL_MIDDLE,0} //14

	 /* [BAR] setting coupon *///wait 0x7F not use
	//BAR_MC
	//not use ok




};

const struct ERRDISP_LED_SET_TBL2 ex_errdisp_alarm_tbl[] =
{
	/*--- abnomal signal on (wait reset) ---*/

	{ALARM_CODE_FRAM,					8, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} /* FRAM error */// use
	,{ALARM_CODE_MAG,					21, STATUS_LED_BLINK, 1, STATUS_LED_ON,  DISP_INTERVAL_MIDDLE,0} /* MAG setting error */// use
	,{ALARM_CODE_I2C,					16, STATUS_LED_BLINK, 1, STATUS_LED_ON,  DISP_INTERVAL_MIDDLE,0} /* I2C communication error(起動時のFRAM通信失敗)(致命的) */// use
	,{ALARM_CODE_TMP_I2C,				16, STATUS_LED_BLINK, 1, STATUS_LED_ON,  DISP_INTERVAL_MIDDLE,0} /* I2C communication error(Escrow時の温度IC通信失敗)(致命的ではない) */
	,{ALARM_CODE_SPI,					8, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} /* SPI communication error */// FRAM SPI 現状エラーにはしていない
	,{ALARM_CODE_PL_SPI,				14, STATUS_LED_BLINK, 1, STATUS_LED_ON,  DISP_INTERVAL_MIDDLE,0} /* PL SPI communication error */// not use 登録はしているが、実際はエラー条件は存在しないはず
	,{ALARM_CODE_CISA_OFF,				17, STATUS_LED_BLINK, 1, STATUS_LED_ON,  DISP_INTERVAL_MIDDLE,0} /* PL CISA communication error */
	,{ALARM_CODE_CISB_OFF,				17, STATUS_LED_BLINK, 1, STATUS_LED_ON,  DISP_INTERVAL_MIDDLE,0} /* PL CISB communication error */
	,{ALARM_CODE_UV,					18, STATUS_LED_BLINK, 1, STATUS_LED_ON,  DISP_INTERVAL_MIDDLE,0}	/* (UV error *//* not use MAGなりなしソフト1本化の為*/
	,{ALARM_CODE_CIS_ENCODER,			19, STATUS_LED_BLINK, 1, STATUS_LED_ON,  DISP_INTERVAL_MIDDLE,0} /* PL CIS encoder count error */

	,{ALARM_CODE_CIS_TEMPERATURE,		20, STATUS_LED_BLINK, 1, STATUS_LED_ON,  DISP_INTERVAL_MIDDLE,0} //2024-05-28


	/* SYSTEM Error 1 (wait reset) ---*/
	,{ALARM_CODE_STACKER_FULL,			1, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,1} /* stacker Full */// use
	,{ALARM_CODE_STACKER_MOTOR_LOCK,	7, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,7} /* stacker motor lock */// use
	,{ALARM_CODE_STACKER_GEAR,			7, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,7} /* stacker gear */// use
	,{ALARM_CODE_STACKER_TIMEOUT,		7, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,7} /* stacker time out */// use
	,{ALARM_CODE_STACKER_HOME,			2, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,2} /* BOX home position error */

	,{ALARM_CODE_FEED_OTHER_SENSOR_SK,	3, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,3} /* stacker JAM */// use
	,{ALARM_CODE_FEED_SLIP_SK,			3, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,3} /* stacker JAM (feed slip) *///use
	,{ALARM_CODE_FEED_TIMEOUT_SK,		3, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,3} /* stacker JAM (feed time out) *///use
	,{ALARM_CODE_FEED_LOST_BILL,		3, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,3} /* lost bill */// use
	,{ALARM_CODE_FEED_MOTOR_LOCK_SK,			3, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,3} /* stacker JAM 2024-02-13  *///use


	,{ALARM_CODE_FEED_OTHER_SENSOR_AT,	4, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,4} /* acceptor JAM (other sensor ON) */// use
	,{ALARM_CODE_FEED_SLIP_AT,			4, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,4} /* acceptor JAM (feed slip) */// use
	,{ALARM_CODE_FEED_TIMEOUT_AT,		4, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,4} /* acceptor JAM (feed time out) */// use
	,{ALARM_CODE_FEED_MOTOR_LOCK_AT,	4, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,4} /* acceptor JAM (feed motor lock) */// use

	,{ALARM_CODE_FEED_MOTOR_SPEED_LOW,	5, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,5} /* feed motor low speed */// use
	,{ALARM_CODE_FEED_MOTOR_SPEED_HIGH,	5, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,5} /* feed motor high speed */// use

	,{ALARM_CODE_FEED_MOTOR_LOCK,		6, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,6} /* feed motor lock */// use

	,{ALARM_CODE_APB_TIMEOUT,			9, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,9} /* APB time out */// use
	,{ALARM_CODE_APB_HOME,				9, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,9} /* APB home position error */// use
	,{ALARM_CODE_APB_HOME_STOP,			9, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,9} /* APB home position stop error */// use

	,{ALARM_CODE_BOX,					10, STATUS_LED_BLINK, 0, 0, DISP_INTERVAL_MIDDLE,0} /* BOX set error */// use
	,{ALARM_CODE_BOX_INIT,				10, STATUS_LED_BLINK, 0, 0, DISP_INTERVAL_MIDDLE,0} /* BOX set error (Box not set when initialize) */// use

	,{ALARM_CODE_CHEAT,					12, STATUS_LED_BLINK, 0, 0, DISP_INTERVAL_MIDDLE,12} /* cheat detection */// use

	,{ALARM_CODE_CENTERING_TIMEOUT,		14, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,14} /* centering time out */// use
	,{ALARM_CODE_CENTERING_HOME_STOP,	14, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,14} /* centering home position stop error */// use

	,{ALARM_CODE_SHUTTER_TIMEOUT,		16, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,16} /* cheat detection */// use
	,{ALARM_CODE_SHUTTER_HOME_STOP,		16, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,16} /* cheat detection */// use

	//ICB
	//TAGとの通信処理なので、RTQは使用していない
	,{ALARM_CODE_RFID_UNIT_MAIN,					20, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} /* RFID Unit *//* RFID基板との問題 *///new only use SS, not use RTQ

	//HeadとBOXの設定不一致 (UBA700RTQは現状ICB無効時ICB通信させないので、このエラーは発生しない)
	//ICB有効でTAG側はシステムinhibitしている(UBA700RTQは現状、TAGにICB無効設定を書く予定がないので発生しない)(ivizonのようにTAGに無効するようになれば使用する事になる)
	,{ALARM_CODE_RFID_ICB_SETTING,			17, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} /* ICB無効でICBユニットがある場合 *///OK only use SS, not use RTQ

	//ICB機能を有効にして、RFID無しBOXを使用してイニシャル動作完了時
	,{ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN,		11, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} /* ICB communication *///ok use SS and RTQ

	//sum error
	,{ALARM_CODE_RFID_ICB_DATA,				18, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} /* ICB data *///ok use SS and RTQ

	//Utility ToolにEnable設定でMC設定(1)を行う。
	//別のHeadでUtility ToolにEnable設定でMC設定(2)を行う
	//RFID初期化済みBoxを使用して、MC設定1を行った、Headを使用して、ID-003で電源ONからResetによるイニシャル動作を行う。
	//電源OFF後、そのBoxを使用して、MC設定2を行ったHeadを使用して、ID-003で電源ONからResetによるイニシャルで発生
	,{ALARM_CODE_RFID_ICB_NUMBER_MISMATCH,	13, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} /* ICB machine number *///ok use SS and RTQ

	//HeadがICB有効状態で、RFID付きBOXでイニシャル動作をさせる、紙幣1枚受け取り後、BOXを取り外して、BOXをR/WツールにてRead実行。 ReadのみでICBイニシャルさせない
	//その後、BOXを元のHeadに戻すと、自動イニシャル後エラー発生
	,{ALARM_CODE_RFID_ICB_NOT_INITIALIZE,	19, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} /* ICB data *///ok
	,{ALARM_CODE_ICB_FORCED_QUIT,			22, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} /* ICB seq *///new

	//マシン番号が14桁全てブランクの場合のエラー
	,{ALARM_CODE_RFID_ICB_MC_INVALID,		21, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} /* ICB seq マシンナンバー未設定 *///new

	,{ALARM_CODE_EXTERNAL_RESET,		0, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} //new not use 表示には使用していない only JDL
	,{ALARM_CODE_PLL_LOCK,				0, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} //new not use 表示には使用していない only JDL
	,{ALARM_CODE_POWER_OFF,				0, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,0} //new not use 表示には使用していない only JDL

	,{ALARM_CODE_FEED_FORCED_QUIT,		6, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,6} /* feed forced quit */// use
	,{ALARM_CODE_STACKER_FORCED_QUIT,	7, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,7} /* stacker forced quit */// use
	,{ALARM_CODE_APB_FORCED_QUIT,		9, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,9} /* APB forced quit */// use
	,{ALARM_CODE_CENTERING_FORCED_QUIT,	14, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,14} /* centering forced quit */// use
	,{ALARM_CODE_SHUTTER_FORCED_QUIT,	16, STATUS_LED_BLINK, 0, 0,  DISP_INTERVAL_MIDDLE,16} /* centering forced quit */// use

#if defined(UBA_RTQ)
	,{ALARM_CODE_RC_ERROR, 			5, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0}
	,{ALARM_CODE_RC_ROM, 			6, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0} //not use
	,{ALARM_CODE_RC_REMOVED, 		7, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0}
	,{ALARM_CODE_RC_COM, 			8, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0}
	,{ALARM_CODE_RC_DWERR, 			9, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0}
	,{ALARM_CODE_RC_POS, 			10, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0} //not use
	,{ALARM_CODE_RC_TRANSPORT, 		12, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0}
	,{ALARM_CODE_RC_TIMEOUT, 		14, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0}
	,{ALARM_CODE_RC_DENOMINATION, 	15, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0} //not use
	,{ALARM_CODE_RC_EMPTY, 			1, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0}
	,{ALARM_CODE_RC_DOUBLE, 		2, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0}
	,{ALARM_CODE_RC_FULL, 			3, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0} //not use
	,{ALARM_CODE_RC_EXCHAGED, 		4, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0}	//not use, only A_PRO
	,{ALARM_CODE_RC_FORCED_QUIT, 	0, STATUS_LED_BLINK, 0, 0, DISP_INTERVAL_MIDDLE, 0}	//not use		        	/* RC forced quit */
	#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)
	,{ALARM_CODE_RC_RFID,	 		22, STATUS_LED_BLINK, 1, STATUS_LED_ON, DISP_INTERVAL_MIDDLE, 0}	//new error
	#endif
#endif // UBA_RTQ


};



#define ERRDISP_LED_REJECT_TBL_MAX	(sizeof(ex_errdisp_reject_tbl) / sizeof(struct ERRDISP_LED_SET_TBL))
#define ERRDISP_LED_ALARM_TBL_MAX	(sizeof(ex_errdisp_alarm_tbl) / sizeof(struct ERRDISP_LED_SET_TBL))
#define STATUS_LED_CTRL_TBL_MAX	(sizeof(ex_status_led_ctrl_tbl) / sizeof(struct ERRDISP_LED_SET_TBL))



/************************** PRIVATE FUNCTIONS *************************/
void display_task(VP_INT exinf);
void _display_msg_proc(void);
void _display_idel_proc(void);
void _display_status_led_off(void);
void _display_status_led_on(void);
void _display_status_led_set_reject(void);
void _display_status_led_set_alarm(void);
void _display_status_led_set_denomi(void);
void _display_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _display_system_error(u8 fatal_err, u8 code);
void _errdisp_blink_on_proc(void);
void _errdisp_blink_last_proc(void);

bool _is_errdisp_led_ctrl_timeup(u16 time);
bool _is_errdisp_led_ctrl_count_over_red(u16 count);
bool _is_errdisp_led_ctrl_count_over_green(u16 count);
void _errdisp_set_mode(u16 mode);


void _errdisp_initialize_proc(void);



/************************** EXTERN FUNCTIONS *************************/

u8 flash_orenge_uba_on_time;
u8 flash_orenge_uba_off_time;


/*******************************
        display_task
 *******************************/
void display_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	ER ercd;

	ex_uba_ore_current = 0xFF; /* initial */
	_errdisp_initialize_proc();				/* error disp task initialize */

	#if 1 //2023-01-31
	_hal_status_led(DISP_COLOR_INIT);
	#endif

	while(1)
	{
		ercd = trcv_mbx(ID_DISPLAY_MBX, (T_MSG **)&tmsg_pt, 100);
		if (ercd == E_OK)
		{
			memcpy(&errdisp_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(errdisp_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_display_system_error(1, 3);
			}
			_display_msg_proc();
		}
		_display_idel_proc();

	}
}



void _display_msg_proc(void)
{
	switch (errdisp_msg.tmsg_code)
	{
	case TMSG_DISP_LED_OFF:
		_display_status_led_off();
		break;
	case TMSG_DISP_LED_ON:
		_display_status_led_on(); //もともとはdisplay_msg.arg1まで
		break;
	case TMSG_DISP_LED_DENOMI:
		_display_status_led_set_denomi();
		break;
	case TMSG_DISP_LED_REJECT:
		_display_status_led_set_reject();
		break;
	case TMSG_DISP_LED_ALARM:
		_display_status_led_set_alarm();
		break;
	case TMSG_DISP_LED_DOWNLOAD:

		break;
	case TMSG_DISP_LED_DOWNLOAD_COMPLETE:

		break;
	default:					/* other */
		/* system error ? */
		_display_system_error(0, 4);
		break;
	}
}



/*********************************************************************//**
 * @brief		status LED display off
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _display_status_led_off(void)
{
	memset(&s_errdisp_ctrl, 0, sizeof(s_errdisp_ctrl));
	s_errdisp_ctrl.mode = ERRDISP_MODE_OFF;
	s_errdisp_led_ctrl_time = 0;			//LED点滅用のインターバルカウンタ
	s_errdisp_led_ctrl_count_red = 0;
	s_errdisp_led_ctrl_count_green = 0;

	_hal_status_led(DISP_COLOR_RED_OFF);
	_hal_status_led(DISP_COLOR_GREEN_OFF);

	_errdisp_set_mode(ERRDISP_MODE_OFF);
}


/*********************************************************************//**
 * @brief		status LED display on
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _display_status_led_on(void) //もともとはdisplay_msg.arg1まで
{
	u16 ii;
	u8 code;

	code = (u8)errdisp_msg.arg1;
	s_errdisp_led_ctrl_time = STATUS_LED_CTRL_TBL_MAX;

	for(ii = 0; ii < STATUS_LED_CTRL_TBL_MAX; ii++)
	{
		if(ex_status_led_ctrl_tbl[ii].code == code)
		{
			if( ex_status_led_ctrl_tbl[ii].interval == DISP_DISABLE_UBA )
			{
				/* 無効 */
				return;
			}

			memset(&s_errdisp_ctrl, 0, sizeof(s_errdisp_ctrl));
			s_errdisp_ctrl.mode = ERRDISP_MODE_ON;

			s_errdisp_led_ctrl_time = 0;		//LED点滅用のインターバルカウンタ
			s_errdisp_led_ctrl_count_red = 0;
			s_errdisp_led_ctrl_count_green = 0;
			s_errdisp_ctrl.red_count = ex_status_led_ctrl_tbl[ii].red_count;
			s_errdisp_ctrl.red_mode = ex_status_led_ctrl_tbl[ii].red_mode;
			s_errdisp_ctrl.green_count = ex_status_led_ctrl_tbl[ii].green_count;
			s_errdisp_ctrl.green_mode = ex_status_led_ctrl_tbl[ii].green_mode;
			s_errdisp_ctrl.interval =  ex_status_led_ctrl_tbl[ii].interval;
			s_errdisp_ctrl.round_wait = DISP_ROUNDWAIT_MIDDLE;


			// red
			if( s_errdisp_ctrl.red_mode == STATUS_LED_OFF )
			{
				_hal_status_led(DISP_COLOR_RED_OFF);
			}
			else
			{
				_hal_status_led(DISP_COLOR_RED);
			}

			// green
			if( s_errdisp_ctrl.green_mode == STATUS_LED_OFF )
			{
				_hal_status_led(DISP_COLOR_GREEN_OFF);
			}
			else
			{
				_hal_status_led(DISP_COLOR_GREEN);
			}

			if( s_errdisp_ctrl.red_mode == STATUS_LED_BLINK ||
					s_errdisp_ctrl.green_mode == STATUS_LED_BLINK )
			{
				_errdisp_set_mode(ERRDISP_MODE_BLINK_ON);
			}
			else
			{
				_errdisp_set_mode(ERRDISP_MODE_IDLE);
			}
			break;
		}
	}
}


/*********************************************************************//**
 * @brief		Set status LED display
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _display_status_led_set_reject(void)
{

	u16 cnt;

	for (cnt = 0; cnt < ERRDISP_LED_REJECT_TBL_MAX; cnt++)
	{
		if (ex_errdisp_reject_tbl[cnt].code == errdisp_msg.arg1)
		{
			memset(&s_errdisp_ctrl, 0, sizeof(s_errdisp_ctrl));
			s_errdisp_led_ctrl_time = 0;			//LED点滅用のインターバルカウンタ
			s_errdisp_led_ctrl_count_red = 0;
			s_errdisp_led_ctrl_count_green = 0;

			s_errdisp_ctrl.mode = ERRDISP_MODE_BLINK_ON;
			s_errdisp_ctrl.code = ex_errdisp_reject_tbl[cnt].code;
			s_errdisp_ctrl.red_count = ex_errdisp_reject_tbl[cnt].red_count;
			s_errdisp_ctrl.red_mode = ex_errdisp_reject_tbl[cnt].red_mode;
			s_errdisp_ctrl.green_count = ex_errdisp_reject_tbl[cnt].green_count;
			s_errdisp_ctrl.green_mode = ex_errdisp_reject_tbl[cnt].green_mode;
			s_errdisp_ctrl.interval = ex_errdisp_reject_tbl[cnt].interval;
			s_errdisp_ctrl.round_wait = DISP_ROUNDWAIT_MIDDLE;

			if( STATUS_LED_ON == s_errdisp_ctrl.red_mode )
			{
				_hal_status_led(DISP_COLOR_RED);
			}
			else
			{
				_hal_status_led(DISP_COLOR_RED_OFF);
			}

			if( STATUS_LED_ON == s_errdisp_ctrl.green_mode )
			{
				_hal_status_led(DISP_COLOR_GREEN);
			}
			else
			{
				_hal_status_led(DISP_COLOR_GREEN_OFF);
			}


			if( s_errdisp_ctrl.red_mode == STATUS_LED_BLINK || s_errdisp_ctrl.green_mode == STATUS_LED_BLINK )
			{
				_errdisp_set_mode(ERRDISP_MODE_BLINK_ON);
			}
			else
			{
    			_errdisp_set_mode(ERRDISP_MODE_IDLE);
			}
			break;
		}
	}
}


/*********************************************************************//**
 * @brief		Set status LED display
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _display_status_led_set_alarm(void)
{
	u16 cnt;

	for (cnt = 0; cnt < ERRDISP_LED_ALARM_TBL_MAX; cnt++)
	{
		if (ex_errdisp_alarm_tbl[cnt].code == errdisp_msg.arg1)	// 引数としては3番目
		{
			memset(&s_errdisp_ctrl, 0, sizeof(s_errdisp_ctrl));
			s_errdisp_led_ctrl_time = 0;			//LED点滅用のインターバルカウンタ
			s_errdisp_led_ctrl_count_red = 0;
			s_errdisp_led_ctrl_count_green = 0;
			s_errdisp_ctrl.mode = ERRDISP_MODE_BLINK_ON;
			s_errdisp_ctrl.code = ex_errdisp_alarm_tbl[cnt].code;
			s_errdisp_ctrl.red_count = ex_errdisp_alarm_tbl[cnt].red_count;
			s_errdisp_ctrl.red_mode = ex_errdisp_alarm_tbl[cnt].red_mode;
			s_errdisp_ctrl.green_count = ex_errdisp_alarm_tbl[cnt].green_count;
			s_errdisp_ctrl.green_mode = ex_errdisp_alarm_tbl[cnt].green_mode;
			s_errdisp_ctrl.interval = ex_errdisp_alarm_tbl[cnt].interval;
			s_errdisp_ctrl.round_wait = DISP_ROUNDWAIT_MIDDLE;

			if( STATUS_LED_ON == s_errdisp_ctrl.red_mode )
			{
				_hal_status_led(DISP_COLOR_RED); // red on keep
			}
			else
			{
				_hal_status_led(DISP_COLOR_RED_OFF);// red off start
			}

			if( STATUS_LED_ON == s_errdisp_ctrl.green_mode )
			{
				_hal_status_led(DISP_COLOR_GREEN); // green on keep
			}
			else
			{
				_hal_status_led(DISP_COLOR_GREEN_OFF); // green off start
			}

			if( s_errdisp_ctrl.red_mode == STATUS_LED_BLINK || s_errdisp_ctrl.green_mode == STATUS_LED_BLINK )
			{
				_errdisp_set_mode(ERRDISP_MODE_BLINK_ON);
			}
			else
			{
    			_errdisp_set_mode(ERRDISP_MODE_IDLE);
			}

			break;
		}
	}
}


/*********************************************************************//**
 * @brief		Set LED display for denomination
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _display_status_led_set_denomi(void)
{

	memset(&s_errdisp_ctrl, 0, sizeof(s_errdisp_ctrl));
	s_errdisp_ctrl.mode = ERRDISP_MODE_BLINK_ON;
	s_errdisp_led_ctrl_time = 0;		//LED点滅用のインターバルカウンタ
	s_errdisp_led_ctrl_count_red = 0;
	s_errdisp_led_ctrl_count_green = 0;
	s_errdisp_ctrl.red_count = 0;
	s_errdisp_ctrl.red_mode = 0;
	s_errdisp_ctrl.green_count = (u8)errdisp_msg.arg1;
	s_errdisp_ctrl.green_mode = STATUS_LED_BLINK;
	s_errdisp_ctrl.interval = DISP_INTERVAL_MIDDLE;
	s_errdisp_ctrl.round_wait = DISP_ROUNDWAIT_MIDDLE;

	// red
	if( s_errdisp_ctrl.red_mode == STATUS_LED_OFF )
	{
		_hal_status_led(DISP_COLOR_RED_OFF);
	}
	else
	{
		_hal_status_led(DISP_COLOR_RED);
	}

	// green
	if( s_errdisp_ctrl.green_mode == STATUS_LED_OFF )
	{
		_hal_status_led(DISP_COLOR_GREEN_OFF);
	}
	else
	{
		_hal_status_led(DISP_COLOR_GREEN);
	}

	if( s_errdisp_ctrl.red_mode == STATUS_LED_BLINK ||
			s_errdisp_ctrl.green_mode == STATUS_LED_BLINK )
	{
		_errdisp_set_mode(ERRDISP_MODE_BLINK_ON);
	}
	else
	{
		_errdisp_set_mode(ERRDISP_MODE_IDLE);
	}
}


/*********************************************************************//**
 * @brief		display task idel procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _display_idel_proc(void)
{
	switch(ex_display_task_mode)
	{
    case    ERRDISP_MODE_IDLE:
	case	ERRDISP_MODE_ON:			/* Error LED : ON			*/
	case	ERRDISP_MODE_OFF:			/* Error LED : OFF			*/
			break;

	case	ERRDISP_MODE_BLINK_ON:		/* Error LED : Blink ON		*/// use
	case	ERRDISP_MODE_BLINK_1SHOT:	// now only test mode denomi flash
			_errdisp_blink_on_proc();	// use
			break;

	case	ERRDISP_MODE_BLINK_LAST:	/* Error LED : Blink Last	*/
			_errdisp_blink_last_proc();
			break;
	default:
		/* system error ? */
		_display_system_error(0, 5);
		break;
	}
	//UBA_ORE_LED
#if defined(UBA_RTQ)	//2023-04-20 幅よせHome通知のOrenge点灯を廃止
	#if 0
	if(ex_centor_home_out)
	{
		/* Home out *//* LED OFF */
		_hal_status_led_orange(0);
	}
	else
	{
		/* Home  *//* LED ON */
		_hal_status_led_orange(1);
	}
	#else //degub

	#if (LOOPBACK_UBA==1)//#if defined(QA_TEST_EMC_EMI)
		if(ex_loopback_error != 0) //2025-04-09
		{
			_hal_status_led_orange(1);/* LED ON */
		}
		else
		{
			_hal_status_led_orange(0);/* LED OFF */
		}
	#else
		if(_cyc_validation_mode == VALIDATION_CHECK_MODE_RUN)
		{
	//		_hal_status_led_orange(1);/* LED ON */
		}
		else
		{
	//		_hal_status_led_orange(0);/* LED OFF */
		}
	#endif

	#endif
#else
	//Hihg speedモードは点灯 それ以外は点滅
	if(is_uba_mode())
	{
	/* 点滅 Low mode*/
		if( flash_orenge_uba_on_time == 0 && flash_orenge_uba_off_time == 0 )
		{
			flash_orenge_uba_on_time = 5;
		}

		if(flash_orenge_uba_on_time > 0)
		{
		/* ON */
			_hal_status_led_orange(1);
			flash_orenge_uba_on_time--;
			if(flash_orenge_uba_on_time == 0)
			{
				flash_orenge_uba_off_time = 10;
			}
		}
		else
		{
		/* OFF */
			_hal_status_led_orange(0);
			flash_orenge_uba_off_time--;
			if(flash_orenge_uba_off_time == 0)
			{
				flash_orenge_uba_on_time = 5;
			}
		}
	}
	else
	{
	/* 点灯 High mode*/
		_hal_status_led_orange(1);
		
		flash_orenge_uba_on_time = 0;
		flash_orenge_uba_off_time = 0;
	}
#endif	

}

/*********************************************************************//**
 * @brief send task message
 * @param[in]	receiver task id
 * 				task message code
 * 				argument 1
 * 				argument 2
 * 				argument 3
 * 				argument 4
 * @return 		None
 **********************************************************************/
void _display_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_DISPLAY_TASK;
		t_msg->mpf_id = ID_MBX_MPF;
		t_msg->tmsg_code = tmsg_code;
		t_msg->arg1 = arg1;
		t_msg->arg2 = arg2;
		t_msg->arg3 = arg3;
		t_msg->arg4 = arg4;
		ercd = snd_mbx(receiver_id, (T_MSG *)t_msg);
		if (ercd != E_OK)
		{
			/* system error */
			_display_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_display_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _display_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_display_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	//if (fatal_err)
	//{
	//	_display_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	//}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_DISPLAY_TASK, (u16)code, (u16)errdisp_msg.tmsg_code, (u16)errdisp_msg.arg1, fatal_err);
}


u8 get_reject_code_icb(u8 reject)
{
	u8 reject_code_icb = 0;
	u8 cnt;

	for(cnt = 0; cnt < ERRDISP_LED_REJECT_TBL_MAX; cnt++)
	{
		if( ex_errdisp_reject_tbl[cnt].code == reject )
		{
//			reject_code_icb = ex_errdisp_reject_tbl[cnt].green_count;
			reject_code_icb = ex_errdisp_reject_tbl[cnt].rfid_index;
			break;
		}
		else if(ex_errdisp_reject_tbl[cnt].code == 0 )
		{
			break;
		}
	}
	return(reject_code_icb);
}

u32 get_icb_error_code(u8 alarm_code)
{
	u32 alarm_code_icb = 0;
	u8 cnt;

	for(cnt = 0; cnt < ERRDISP_LED_ALARM_TBL_MAX; cnt++)
	{
		if( ex_errdisp_alarm_tbl[cnt].code == alarm_code )
		{
//			alarm_code_icb = ex_errdisp_alarm_tbl[cnt].green_count;
			alarm_code_icb = (u32)ex_errdisp_alarm_tbl[cnt].rfid_index;
			break;
		}
		else if(ex_errdisp_alarm_tbl[cnt].code == 0 )
		{
			break;
		}
	}
	return(alarm_code_icb);
}


void _errdisp_blink_last_proc(void)
{

	if (_is_errdisp_led_ctrl_timeup(s_errdisp_ctrl.round_wait))
	{
		// green
		if ( s_errdisp_ctrl.green_mode == STATUS_LED_BLINK )
		{
			s_errdisp_led_ctrl_count_green = 0;
		}

		//red
		if ( s_errdisp_ctrl.red_mode == STATUS_LED_BLINK )
		{
			s_errdisp_led_ctrl_count_red = 0;
		}
		/* 必要以上のログがログが更新される可能性がある為、ログ保存処理はスキップ	*/
		ex_display_task_mode = ERRDISP_MODE_BLINK_ON;
		s_errdisp_led_ctrl_time = 0;		//LED点滅用のインターバルカウンタ
	}
}


void _errdisp_blink_on_proc(void)
{

	if (_is_errdisp_led_ctrl_timeup(s_errdisp_ctrl.interval))	// 点滅インターバル時間が経過したか確認
	{
		// 緑確認
		if ( s_errdisp_ctrl.green_mode == STATUS_LED_BLINK )
		{
			if (_is_errdisp_led_ctrl_count_over_green(s_errdisp_ctrl.green_count))	// 点滅回数の最後か確認
			{
				_hal_status_led(DISP_COLOR_GREEN_OFF);
			}
			else
			{
				if( green_on == 0 )
				{
					_hal_status_led(DISP_COLOR_GREEN);
				}
				else
				{
					_hal_status_led(DISP_COLOR_GREEN_OFF);
					s_errdisp_led_ctrl_count_green++;
				}
			}
		}

		// 赤確認
		if ( s_errdisp_ctrl.red_mode == STATUS_LED_BLINK )
		{
			if (_is_errdisp_led_ctrl_count_over_red(s_errdisp_ctrl.red_count))
			{
				_hal_status_led(DISP_COLOR_RED_OFF);
			}
			else
			{
				if( red_on == 0 )
				{
					_hal_status_led(DISP_COLOR_RED);
				}
				else
				{
					_hal_status_led(DISP_COLOR_RED_OFF);
					s_errdisp_led_ctrl_count_red++;
				}
			}
		}

		if(
		((_is_errdisp_led_ctrl_count_over_green(s_errdisp_ctrl.green_count) )
		|| s_errdisp_ctrl.green_mode != STATUS_LED_BLINK )
		&&
		( (_is_errdisp_led_ctrl_count_over_red(s_errdisp_ctrl.red_count))
		|| s_errdisp_ctrl.red_mode != STATUS_LED_BLINK )
		)
		{
			if( ex_display_task_mode == ERRDISP_MODE_BLINK_ON )
			{
			/* 必要以上のログがログが更新される可能性がある為、ログ保存処理はスキップ	*/
				ex_display_task_mode = ERRDISP_MODE_BLINK_LAST;/* 最後の点滅完了状態	*/
			}
			else if( ex_display_task_mode == ERRDISP_MODE_BLINK_1SHOT )
			{
			/* 必要以上のログがログが更新される可能性がある為、ログ保存処理はスキップ	*/
				ex_display_task_mode = ERRDISP_MODE_IDLE;
			}
		}
		s_errdisp_led_ctrl_time = 0;			//LED点滅用のインターバルカウンタ
	}
}



void _errdisp_set_mode(u16 mode)
{

	ex_display_task_mode = mode;

#if 1//#ifdef _ENABLE_JDL
//	jdl_add_trace(ID_ERRDISP_TASK, ((ex_display_task_mode >> 8) & 0xFF), (ex_display_task_mode & 0xFF), (s_errdisp_ctrl.mode & 0xFF), s_errdisp_ctrl.red_mode, s_errdisp_ctrl.green_mode);
#endif /* _ENABLE_JDL */

}

/*********************************************************************//**
 * @brief is led control timeup
 * @param[in]	limit time
 * @return 		None
 **********************************************************************/
bool _is_errdisp_led_ctrl_timeup(u16 time)	// 緑、赤共通
{
	s_errdisp_led_ctrl_time++;			//LED点滅用のインターバルカウンタ
	return (s_errdisp_led_ctrl_time >= time) ? true : false;
}


/*********************************************************************//**
 * @brief is led control count over
 * @param[in]	None
 * @return 		None
 **********************************************************************/
bool _is_errdisp_led_ctrl_count_over_red(u16 count)
{

	return (s_errdisp_led_ctrl_count_red >= count) ? true : false;
}


/*********************************************************************//**
 * @brief is led control count over
 * @param[in]	None
 * @return 		None
 **********************************************************************/
bool _is_errdisp_led_ctrl_count_over_green(u16 count)
{

	return (s_errdisp_led_ctrl_count_green >= count) ? true : false;
}


void _errdisp_initialize_proc(void)
{

	memset(&s_errdisp_ctrl, 0, sizeof(s_errdisp_ctrl));

	s_errdisp_led_ctrl_time = 0;		//LED点滅用のインターバルカウンタ
	s_errdisp_led_ctrl_count_red = 0;
	s_errdisp_led_ctrl_count_green = 0;

	_errdisp_set_mode(ERRDISP_MODE_OFF);
}

/* EOF */
