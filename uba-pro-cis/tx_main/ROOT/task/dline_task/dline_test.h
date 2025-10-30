/******************************************************************************/
/*! @addtogroup Main
    @file       usb_testmode.h
    @brief      jcm usb test mode header file
    @date       2018/03/19
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan Cash Machine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/19 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/

#ifndef SRC_MAIN_DLINE_DLINE_TEST_H_
#define SRC_MAIN_DLINE_DLINE_TEST_H_


/* ====	define					==== */
/*<<	USB communication buffer	>>*/
enum TEST_MODE
{
	TEST_NON_ACTION				= 0x00,
	TEST_USB_CONTROL			= 0x11,
	TEST_SW_CONTROL_REDY		= 0x21,
	TEST_SW_CONTROL				= 0x22,
	TEST_SW_SELECT_MISS			= 0x23,
	TEST_RESET_WAIT				= 0x2e,
	TEST_SW_SELECT_NON_CONNECT	= 0x2f,
	TEST_BT_CONTROL				= 0x31
};

enum OPERATION_TEST_PHASE_NUMBER
{
	PHASE_ACCEPT_TEST_SS 					= 0x01,
	PHASE_ACCEPT_TEST 						= 0x02,
	PHASE_NO_JUDGE_ACCEPT_TEST_SS		 	= 0x03,
	PHASE_NO_JUDGE_ACCEPT_TEST 				= 0x04,
	PHASE_NO_JUDGE_REJECT_TEST_SS 			= 0x05,
	PHASE_NO_JUDGE_REJECT_TEST 				= 0x06,
	PHASE_COLLECTION_REJECT_CENT_OPEN_TEST_SS = 0x07, //2024-12-01 生産CIS初期流動に使用する
//	PHASE_COLLECTION_REJECT_CENT_OPEN_TEST	= 0x08,
	PHASE_TRANSMIT_MOTOR_TEST				= 0x11,
	PHASE_STACKER_MOTOR_TEST				= 0x12,
	PHASE_WIDTH_MOTOR_TEST					= 0x13,
//	PHASE_APB_MOTOR_TEST					= 0x14,
//	PHASE_LED_TEST							= 0x20,
//	PHASE_ICB_TEST							= 0x30,
	PHASE_SENSOR_TEST						= 0x40,
//	PHASE_SIDE_TEST							= 0x41,
	PHASE_AGING_TEST_SS						= 0x70,
//	PHASE_AGING_TEST						= 0x71,
	PHASE_MOTOR_SPEED_TEST					= 0x80,
//	PHASE_MOTOR_SPEED_SETTING_VALUE_TEST	= 0x81,
	PHASE_WIDTH_SPEED_TEST					= 0x82,
//	PHASE_WIDTH_SPEED_SETTING_VALUE_TEST	= 0x83,
	PHASE_PB_SPEED_TEST						= 0x84,
//	PHASE_PB_SPEED_SETTING_VALUE_TEST		= 0x85,
	PHASE_STACKER_SPEED_TEST				= 0x86,
//	PHASE_STACKER_SPEED_SETTING_VALUE_TEST	= 0x87,
//	PHASE_STACKER_CURRENT_VALUE_TEST		= 0x88,
	PHASE_SHUTTER_SPEED_TEST				= 0x89,
	PHASE_DIPSWITCH_TEST					= 0x90,
	PHASE_DENOMI_TEST						= 0xA0, //for debug
	PHASE_STACK_TEST						= 0xB0,
	PHASE_PB_TEST							= 0xB1,
	PHASE_WIDTH_TEST						= 0xB2,	
	PHASE_SHUTTER_TEST						= 0xB5,
	#if defined(UBA_LOG)
	PHASE_GET_UBA_LOG_TEST					= 0xC5,	//for debug
	#endif
	
	PHASE_GET_SS_RC_TYPE 					= 0xE0,	//Tool suiteなどで使用

	/* 0xEX are SPECIAL command, using for DEBUG  */
	PHASE_MAG_EMI_TEST						= 0xE1,	//for debug
	PHASE_SYSTEM_ERROR						= 0xF1,	//for debug
	PHASE_FRAM_LOG							= 0xF2,	//for debug
	PHASE_TASK_SEQ							= 0xF5,	//for debug
	PHASE_CLOCK								= 0xF6,	//for debug
	PHASE_HW_STATE							= 0xF8, //for debug
	PHASE_END								= 0xFF
};

enum TESTMODE_OPTION
{
	BIT_NONE				= 0x00,
	BIT_WITH_STACKER		= 0x01,
	BIT_REJECT				= 0x02,
	BIT_NO_JUDGE			= 0x04
};

enum MOTOR_CMD_NUMBER
{
	CMD_MOTOR_FWD			= 0x31,
	CMD_MOTOR_REV			= 0x32
};

enum TESTMODE_CMD_NUMBER
{
	CMD_NONE				= 0x00,
	CMD_RUN					= 0x01,
	CMD_ENQ					= 0x05,
#if defined(UBA_RTQ)
	CMD_ACK					= 0x06,
	CMD_NACK				= 0x15,
	CMD_ERR					= 0x20,
	CMD_END					= 0xff,
#endif // UBA_RTQ	
	CMD_POLL				= 0x11,
	CMD_STOP				= 0xff
};
enum DIP_SWITCH_CMD_NUMBER
{
	CMD_DIP_SWITCH1			= 0x31,
	CMD_DIP_SWITCH2			= 0x32
};

enum DENOMI_CMD_NUMBER
{
	CMD_DENOMI_NG			= 0x00,
	CMD_DENOMI_OK			= 0x01,
	CMD_TICKET_OK			= 0x02
};

enum IF_CMD_NUMBER
{
	CMD_IF_RS232C			= 0x31,
	CMD_IF_TTL				= 0x32,
	CMD_IF_PHOTOCOUPLER		= 0x33,
	CMD_IF_ID044			= 0x34,
	CMD_IF_WAKEUP			= 0x35,
};

enum TESTMODE_RES_NUMBER
{
	RES_DATA	= 0x00,
	RES_NG		= 0x80,
	RES_BUSY	= 0x01,
	RES_ACK		= 0x06,
	RES_NAK		= 0x15,
};

extern void front_usb_testmode_request(void);
extern u8 reset_dline_sw_test(void);
extern u8 select_dline_sw_test(void);

#endif /* SRC_MAIN_DLINE_DLINE_TEST_H_ */
