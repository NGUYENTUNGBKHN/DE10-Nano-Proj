/*
 * subline_download.h
 *
 *  Created on: 2019/07/26
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_SUBLINE_SUBLINE_DOWNLOAD_H_
#define SRC_SUBLINE_SUBLINE_DOWNLOAD_H_
//#include "boot_typedefine.h"

/***** コマンドID *****/
/* Status RequestコマンドID */
/* Download StartコマンドID */
/* Download DataコマンドID */
/* Download End ConfirmコマンドID */
/* Version RequestコマンドID */
/* ResetコマンドID */
#define CMD_ID_STATUS			0xD0
#define CMD_ID_DNLOAD_START		0xD1
#define CMD_ID_DNLOAD_DATA		0xD2
#define CMD_ID_DNLOAD_END_CONF	0xD3
#define CMD_ID_VERSION			0xD4
#define CMD_ID_RESET			0x40
/***** コマンドLENフィールド *****/
#define CMD_LEN_STATUS					0x00
#define CMD_LEN_DNLOAD_START			0x00
#define CMD_LEN_DNLOAD_DATA_DEFAULT		DNLOAD_DATA_BLK_SIZE
#define CMD_LEN_DNLOAD_END_CONF			0x00
#define CMD_LEN_VERSION					0x00
#define CMD_LEN_RESET					0x00
/* ﾚｽﾎﾟﾝｽ */
#define RES_ID_STATUS			0xE0
#define RES_ID_DNLOAD_START		0xE1
#define RES_ID_DNLOAD_DATA		0xE2
#define RES_ID_DNLOAD_END_CONF	0xE3
#define RES_ID_VERSION			0xE4

enum _SUBLINE_MODE
{
	/* Normal Operation(I/F) Mode */
	SUBLINE_MODE_NORMAL = 1,
	SUBLINE_MODE_DOWNLOAD_WAIT_RSP,
	SUBLINE_MODE_HOST_DOWNLOAD_WAIT_RSP,
	/* Download Mode */
	SUBLINE_MODE_DOWNLOAD,
	SUBLINE_MODE_DOWNLOAD_ERROR,
	SUBLINE_MODE_DOWNLOAD_ILLEGAL_FILE_ERROR,
	SUBLINE_MODE_DOWNLOAD_WAIT_RESET,
};

enum SUBLINE_STATE {
	SUBLINE_NO_DATA = 0,
	SUBLINE_RECEIVE_OK,
	SUBLINE_RECEIVE_COMPLETE,
	SUBLINE_PACKET_ERR = 0xFF,
};
enum SUBLINE_PROGRAMID {
	SUBLINE_ANALYZE,
	SUBLINE_ANALYZE_SUITE,
};
#endif /* SRC_SUBLINE_SUBLINE_DOWNLOAD_H_ */
