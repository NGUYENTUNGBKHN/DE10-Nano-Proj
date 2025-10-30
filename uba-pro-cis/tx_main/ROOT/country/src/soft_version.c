/******************************************************************************/
/*! @addtogroup Main
    @file       soft_version.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * soft_version.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */
#include "custom.h"
#include "common.h"

#if defined(UBA)
// ソフトウェアバージョン文字列ファイル。                            0123456789ABCDEF
//const unsigned char software_ver[64] = "UBA700  USA  ID-003 V000-4C 24APR23                            \0";
//												01234567890123456789012345678901234567890123456789012345678901234
	//	const unsigned char software_ver[64] = "UBA700         EUR ID-003 V010-4C 19MAY23                      \0";
	//	const unsigned char software_ver[64] = "UBA-700-SS EUR5 ID-003 V001-10 15MAR23                         \0"; //これだと700の前のハイフンが消える
	//	const unsigned char software_ver[64] = "UBA-700-SS EUR ID-003 V001-00 10JUL23                          \0";

#elif (_DEBUG_UART_LOOPBACK==1)
		// ソフトウェアバージョン文字列ファイル。          0123456789ABCDEF0123456789ABCDEF
		const unsigned char software_ver[64] = "iVIZION-200-SS USA ID-003 V008-06 19DEC22(LOOPBACK TEST)       \0";

#elif (DEBUG_POLYMER_DEBUG==1)
		// ソフトウェアバージョン文字列ファイル。          0123456789ABCDEF0123456789ABCDEF
		//const unsigned char software_ver[64] = "iVIZION2 USA ID-003 V001-01 22SEP22                            \0";
		const unsigned char software_ver[64] = "iVIZION-200-SS USA ID-003 V006-02 31OCT22(DEBUG_POLYMER)       \0";
#elif (DEBUG_POLYMER_BELT_ACCEPT==1)
		// ソフトウェアバージョン文字列ファイル。          0123456789ABCDEF0123456789ABCDEF
		//const unsigned char software_ver[64] = "iVIZION2 USA ID-003 V001-01 22SEP22                            \0";
		const unsigned char software_ver[64] = "iVIZION-200-SS USA ID-003 V006-02 31OCT22(DEBUG_BELT_AGING)    \0";
#elif (SH_BOX)
	#if defined(_PROTOCOL_ENABLE_ID003)
		// ソフトウェアバージョン文字列ファイル。          0123456789ABCDEF0123456789ABCDEF
		const unsigned char software_ver[64] = "iVIZION-200-SH USA ID-003 V011-10 15MAR23                      \0";
	#elif defined(_PROTOCOL_ENABLE_ID0G8)
		// ソフトウェアバージョン文字列ファイル。          0123456789ABCDEF0123456789ABCDEF
		const unsigned char software_ver[64] = "iVIZION2 USA ID-0G8 V006-03 15NOV22                            \0";
	#else
		// ソフトウェアバージョン文字列ファイル。          0123456789ABCDEF0123456789ABCDEF
		const unsigned char software_ver[64] = "iVIZION2 USA ID-000 V001-01 22SEP22                            \0";
	#endif

#else
	// iVIZION2 Version
	// Vxxx-xx
	//   |   |
	//   |  Interface Version
	//   |
	//  Other Version(template, operation etc)
	#if defined(_PROTOCOL_ENABLE_ID003)
		// ソフトウェアバージョン文字列ファイル。          0123456789ABCDEF0123456789ABCDEF
		const unsigned char software_ver[64] = "iVIZION-200-SS EUR ID-003 V100-01 19MAY23                      \0";
	#elif defined(_PROTOCOL_ENABLE_ID0G8)
		// ソフトウェアバージョン文字列ファイル。          0123456789ABCDEF0123456789ABCDEF
		const unsigned char software_ver[64] = "iVIZION2 USA ID-0G8 V101-01 11MAY23                            \0";
	#else
		// ソフトウェアバージョン文字列ファイル。          0123456789ABCDEF0123456789ABCDEF
		const unsigned char software_ver[64] = "iVIZION2 USA ID-000 V001-01 22SEP22                            \0";
	#endif

#endif
/* EOF */
