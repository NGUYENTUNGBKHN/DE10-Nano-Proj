/******************************************************************************/
/*! @addtogroup Main
    @file       usb_utiity.h
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


/* Mode */
enum UTILITY_PHASE_NUMBER
{
	UTILITY_MODE_ICB_SETTINGS   = 0x72,
	UTILITY_MODE_IF_SETTINGS    = 0x74,
	UTILITY_MODE_HW_SETTINGS    = 0x75,
};

/* Phase */
enum ICB_FUNCTION_PHASE_NUMBER
{
	UTILITY_PHASE_ICB_STATUS   	= 0x01,
	UTILITY_PHASE_ICB_MCNUMBER	= 0x02,
	UTILITY_PHASE_ICB_INHIBIT   = 0x03
};
enum HASH_FUNCTION_PHASE_NUMBER
{
	UTILITY_PHASE_HASH_SHA1   	= 0x01,
	UTILITY_PHASE_HASH_CRC16	= 0x02,
	UTILITY_PHASE_HASH_CRC32    = 0x03
};
enum TILT_FUNCTION_PHASE_NUMBER
{
	UTILITY_PHASE_TILT			= 0x01
};

/* Coomand */
enum ICB_FUNCTION_CMD_NUMBER
{
	UTILITY_CMD_ICB_ENABLE      = 0x01,
	UTILITY_CMD_ICB_DISABLE     = 0xff,
	UTILITY_CMD_STATUS_REQUEST	= 0x05,
	UTILITY_CMD_ICB_INHIBIT     = 0x55
};
#define	UTILITY_CMD_IF_CRC16     	0x01
#define	UTILITY_CMD_IF_CRC32     	0x02
#define	UTILITY_CMD_HASH_STARTCULC 	0x01
#define	UTILITY_CMD_TILT_SET 		0x01
#define	UTILITY_CMD_TILT_CLEAR 		0x02
#define	UTILITY_CMD_BUSY			0xFF

#define UTILITY_ICB_MCNUMBER_LENGTH 14
#define UTILITY_HASH_SHA1_LENGTH 20
#define UTILITY_HASH_CRC16_LENGTH 2
#define UTILITY_HASH_CRC32_LENGTH 4


//extern void	front_usb_utility_request(void);
/*!
 * JCM USB Tool Suite Utility ICB function request
 *
 * Read/write setting of enable/disable of ICB function.
 * Read/write Read/write setting of ICB machine number.
 *
 * \param       None
 *
 * \retval      NONE
 *
 * \internal
 * \endinternal
 */
extern void front_usb_icb_function_request(void);

