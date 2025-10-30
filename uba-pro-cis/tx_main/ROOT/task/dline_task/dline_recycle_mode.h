/******************************************************************************/
/*! @addtogroup Group2
    @file       dline_recycle_mode.h
    @brief      
    @date       2024/03/07
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    Japan CashMachine Co, Limited. All rights reserved.
******************************************************************************/
#ifndef _DLINE_RECYCLE_MODE_H_
#define _DLINE_RECYCLE_MODE_H_
#ifdef __cplusplus
extern "C"
{
#endif

#if defined(UBA_RTQ)
/* CODE */


#define RC_ENQ		0x05		/*	Enquiry 				*/
#define RC_ACK		0x06		/*	Acknowledge 			*/
#define RC_NAK		0x15		/*	Negative Acknowledge 	*/

//#if defined(RC_BOARD_GREEN)
#define	RC_BUSY	0x01		/*	Busy					*/
//#endif //

enum RECYCLE_TEST_PHASE_NUMBER
{
	PHASE_PAYOUT 							= 0x01,
	PHASE_COLLECT 							= 0x02,
	PHASE_SET_RECYCLE_DENOMI			 	= 0x03, 
	PHASE_SET_RECYCLE_LIMIT 				= 0x04,
	PHASE_SET_RECYCLE_COUNT 				= 0x05, 

	PHASE_TWIN_TR_POS_SENSOR_TEST			= 0x06,
	PHASE_TWIN_RC_POS_SENSOR_TEST			= 0x07,
	PHASE_TWIN_SOLENOID_TEST				= 0x08,
	PHASE_TWIN_DRUM1_MOTOR_TEST				= 0x09,
	PHASE_TWIN_DRUM2_MOTOR_TEST				= 0x0A,
	PHASE_QUAD_FEED_MOTOR_TEST				= 0x0B,
	PHASE_QUAD_FLAP_MOTOR_TEST				= 0x0C,
	PHASE_QUAD_TR_POS_SENSOR_TEST			= 0x0D,
	PHASE_QUAD_RC_POS_SENSOR_TEST			= 0x0E,
	PHASE_QUAD_SOLENOID_TEST				= 0x0F,

	PHASE_GET_RECYCLE_LOG 					= 0x10,
	PHASE_CLEAR_RECYCLE_LOG 				= 0x11,

	PHASE_TWIN_TAPE_SENS_TEST				= 0x12,
	PHASE_QUAD_TAPE_SENS_TEST				= 0x13,

	PHASE_SET_STACKER_VALUE					= 0x20,
	PHASE_GET_STACKER_VALUE					= 0x21,

	PHASE_RC_LED_TEST						= 0x31,
	PHASE_DIPSW_TEST						= 0x32,
	PHASE_EXBOX_SOLENOID_TEST				= 0x33,
	PHASE_TWIN_FEED_MOTOR_TEST				= 0x34,
	PHASE_TWIN_FLAP_MOTOR_TEST				= 0x35,
	PHASE_COMMUNICATION						= 0x36,
	
	PHASE_QUAD_DRUM1_MOTOR_TEST				= 0x40,
	PHASE_QUAD_DRUM2_MOTOR_TEST				= 0x41,
	
	PHASE_SERIAL_NO							= 0x50,
	PHASE_SENS_ADJ_FRAM						= 0x51,
	PHASE_PERFORM_TEST_FRAM					= 0x52,
	
	PHASE_TWIN_FEED_GET_DATA				= 0x60,
	PHASE_QUAD_FEED_GET_DATA				= 0x61,

	PHASE_SET_RECYCLE_LENGTH 				= 0x70,
	PHASE_GET_RECYCLE_COUNT 				= 0x71, 
	
	PHASE_SENS_ADJ_START					= 0xB0,
	PHASE_SENS_ADJ_GET_VAL					= 0xB1,
	
	PHASE_DRUM1_TAPE_POS_ADJ				= 0xC0,
	PHASE_DRUM2_TAPE_POS_ADJ				= 0xC1,
	
	PHASE_CHECK_FRAM						= 0xC2,

//#if defined(RC_BOARD_GREEN)		
	PHASE_GET_RS_POS_ONOFF					= 0xD0,
	PHASE_SET_RS_POS_DA						= 0xD1,
	PHASE_SET_RS_POS_GAIN					= 0xD2,
//#endif
	
	PHASE_CHECK_TWIN_OR_QUAD				= 0xE0, //既存のTool suteやセンサ調整Toolで使用している

//#if defined(RC_BOARD_GREEN)
//#if defined(UBA_RS)			
	PHASE_RS_FLAP_MOTOR_TEST				= 0xF0,
	PHASE_RS_POS_SENSOR_TEST				= 0xF1	
//#endif
//#endif
};

enum SERIAL_NO_CMD_NUMBER
{
	CMD_SERIAL_NO_READ_FACTORY			= 0x00,
	CMD_SERIAL_NO_WRITE_FACTORY			= 0x01,
	CMD_SERIAL_NO_READ_MAINTENANCE		= 0x10,
	CMD_SERIAL_NO_WRITE_MAINTENANCE		= 0x11,
};

enum EDITION_NUMBER
{
	CMD_EDITION_NO_READ_FACTORY			=0x02,
	CMD_EDITION_NO_WRITE_FACTORY		=0x03,
	CMD_EDITION_NO_READ_MAINTENANCE		=0x20,
	CMD_EDITION_NO_WRITE_MAINTENANCE	=0x21,
};

enum SENS_ADJ_CMD_NUMBER
{
	CMD_SENS_ADJ_START_FACTORY			= 0x00,
	CMD_SENS_ADJ_READ_FACTORY			= 0x01,
	CMD_SENS_ADJ_START_MAINTENANCE		= 0x10,
	CMD_SENS_ADJ_READ_MAINTENANCE		= 0x11,
};

enum SENS_ADJ_FRAM_CMD_NUMBER
{
	CMD_SENS_ADJ_FRAM_READ_FACTORY		= 0x00,
	CMD_SENS_ADJ_FRAM_WRITE_FACTORY		= 0x01,
	CMD_SENS_ADJ_FRAM_READ_MAINTENANCE	= 0x10,
	CMD_SENS_ADJ_FRAM_WRITE_MAINTENANCE	= 0x11,
};

enum PERFORM_TEST_FRAM_CMD_NUMBER
{
	CMD_PERFORM_TEST_FRAM_READ			= 0x00,
	CMD_PERFORM_TEST_FRAM_WRITE			= 0x01,
};

extern testmode ex_testmode;

extern void front_usb_recycle_request();

#endif 

#ifdef __cplusplus
}
#endif
#endif
// end
