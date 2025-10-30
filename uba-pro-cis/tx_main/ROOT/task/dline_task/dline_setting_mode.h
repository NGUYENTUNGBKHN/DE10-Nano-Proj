/******************************************************************************/
/*! @addtogroup Main
    @file       usb_usetting_mode.h
    @brief      jcm usb test mode header file
    @date       2020/02/25
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2020 Japan Cash Machine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2020/02/25 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/

enum	SETTING_PHASE_NUMBER
{
//	PHASE_SETTING_ENCRYPTION		= 0x21,	//予約済み
//	PHASE_SETTING_VIRTUAL_DIPSW		= 0x22,	//予約済み
	PHASE_SETTING_MOTOR_SPEED		= 0x23,
	PHASE_SETTING_MOTOR_CURRENT		= 0x24,
	PHASE_SETTING_STACKER_FULL_THRESHOLD	= 0x25,
	PHASE_SETTING_TEMPERATURE	= 0x26,
	PHASE_SETTING_SPEED_MODE		= 0x30,
	PHASE_SETTING_SSSU_MODE			= 0x31,
};
enum	HW_SETTING_PHASE_NUMBER
{
	HW_PHASE_SETTING_VERSION_SET		= 0x01,
};
enum SETTING_DATA_CMD_NUMBER
{
	//CMD_READ	= 0x00,
	//CMD_WRITE	= 0x01,
	CMD_READ_HMOT	= 0x00,
	CMD_WRITE_HMOT	= 0x01,
	CMD_READ_SMOT	= 0x10,
	CMD_WRITE_SMOT	= 0x11,
	CMD_READ_CMOT	= 0x20,
	CMD_WRITE_CMOT	= 0x21,
};
enum SETTING_CMD_VER_NUMBER
{
	UTILITY_CMD_VER_UBA700 = 0x00,
	UTILITY_CMD_VER_UBA = 0x01,
	UTILITY_CMD_GET_VER = 0x05,
};
enum	SETTING_DATA_RES_NUMBER
{
	//RES_OK					= 0x00,
	RES_END					= 0xFF,
	RES_STS_ENABLE_08_DE	= 0x01,
	RES_STS_ENABLE_DES		= 0x02,
	RES_STS_DISABLE			= 0xFF,
};
#define RES_DATA	0x00

void	front_usb_setting_request(void);


/*--- End of File ---*/
