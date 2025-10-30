/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
 * @file cyclonev_fram_drv.c
 * @brief FRAMドライバ
 * @date 2019.03.05 Created.
 */
/****************************************************************************/
#include <string.h>


#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

#include "jcm_typedef.h"
#include "fram_drv.h"

#include "memory.h"
/****************************************************************/
/**
 * @brief SPIハンドル構造体
 */
/****************************************************************/
typedef struct {
	s32 ch;
	SPI_HANDLE handle;
} SPI_CP;

struct {
	SPI_CP cp;		// SPIハンドル構造体
} fram_data;		// 18/11/12

const UINT32 CFG_SPIM_SYSCLK[] =				/* ペリフェラルシステムクロック周波数(Hz) */
{
	200000000
};

const UINT16 spim0_stb_gpio[] = 				/* I/Oテーブル */
{
	GPIO_SPI0_STB, 									/* CS0のGPIO */
	0xFFFF 										/* テーブル終端記述 */
};

const UINT16 *CFG_SPIM_STBGPIOID[] =			/* STB端子用GPIO_ID */
{
	spim0_stb_gpio
};

const UINT8 CFG_SPIM_FMT[] =					/* SPI出力フォーマット */
{
	(SPIM_FMT_STB_L_ACTIVE | SPIM_FMT_CLK_L_ACTIVE)
};

const UINT32 CFG_SPIM_CLK[] = 					/* SPI目標ビットCLK周波数 */
{
#if (_DEBUG_FRAM_CLK_40MHZ==1)
	40000000
#else
	25000000
#endif
};

/****************************************************************/
/*							FRAMコマンド						*/
/****************************************************************/
enum {
	SPICMD_WREN = 0x06,
	SPICMD_WRITE = 0x02,
	SPICMD_READ = 0x03,
	SPICMD_WRDI = 0x04,
	SPICMD_RDSR = 0x05,
	SPICMD_WRSR = 0x01,

	//MEMORY_OPERATION_LENGTH = 3,		// コマンド1BYTE + アドレス2BYTES (FM25V05), 18/11/30
	MEMORY_OPERATION_LENGTH = 4,		// コマンド1BYTE + アドレス3BYTES (FM25V10-128KBにつきアドレス増加), 21/08/30
};
#define LOCAL_BUFFER_SIZE 0x400

static u8 ex_spi_local_buff_large[LOCAL_BUFFER_SIZE+MEMORY_OPERATION_LENGTH];
static u8 ex_spi_local_buff_small[LOCAL_BUFFER_SIZE+MEMORY_OPERATION_LENGTH];

/****************************************************************/
/**
 * @brief リードステータス
 */
/****************************************************************/
FRAM_DRV_ER fram_drv_read_status(u8 *pdata)
{
	INT8 result;
	SPIM_SEND send;
	u8 cmd = SPICMD_RDSR;
	u8 rcv[2];
	
	send.opt = (SPIM_OPT_PACK_END | SPIM_8BIT_GRAN);
	send.send = &cmd;
	send.recv = rcv;
	send.len = 2;

	result = Spi_send(&fram_data.cp.handle, fram_data.cp.ch, &send);

	*pdata = rcv[1];

	return (result == FALSE ? FRAM_DRV_WREN_ERROR : FRAM_DRV_SUCCESS);
}
/****************************************************************/
/**
 * @brief ライトステータス
 */
/****************************************************************/
FRAM_DRV_ER fram_drv_write_status(u8 data)
{
	INT8 result;
	SPIM_SEND send;
	u8 cmd[2];

	cmd[0] = SPICMD_WRSR;
	cmd[1] = data;
	send.opt = (SPIM_OPT_PACK_END | SPIM_8BIT_GRAN);
	send.send = cmd;
	send.recv = 0;
	send.len = 2;

	fram_drv_write_enable();

	result = Spi_send(&fram_data.cp.handle, fram_data.cp.ch, &send);

	return (result == FALSE ? FRAM_DRV_WREN_ERROR : FRAM_DRV_SUCCESS);
}
/****************************************************************/
/**
 * @brief ライトイネーブル
 */
/****************************************************************/
FRAM_DRV_ER fram_drv_write_enable(void)
{
	INT8 result;
	SPIM_SEND send;
	u8 cmd = SPICMD_WREN;
	u8 rcv = 0;

	send.opt = (SPIM_OPT_PACK_END | SPIM_8BIT_GRAN);
	send.send = &cmd;
	send.recv = &rcv;
	send.len = 1;

	result = Spi_send(&fram_data.cp.handle, fram_data.cp.ch, &send);

	return (result == FALSE ? FRAM_DRV_WREN_ERROR : FRAM_DRV_SUCCESS);
}

/****************************************************************/
/**
 * @brief SPIドライバ初期設定
 */
/****************************************************************/
FRAM_DRV_ER fram_drv_init(void)
{
	INT8 result;
	FRAM_DRV_ER fram_er = FRAM_DRV_SUCCESS;
	
	memset(&fram_data, 0, sizeof(fram_data));		// モジュール変数初期化, 18/12/04
	fram_data.cp.ch = -1;		// FRAM SPIチャネルクリア
	
	result = Spi_init(NULL);		// ドライバ初期化
	
	fram_er = (result == FALSE ? FRAM_DRV_INIT_ERROR : FRAM_DRV_SUCCESS);
	
	return fram_er;
}

/****************************************************************/
/**
 * @brief SPIドライバオープン
 * @param ch SPIのチャネルを指定します。
 */
/****************************************************************/
FRAM_DRV_ER fram_drv_open(s32 ch)
{
	INT8 result;
	SPI_PARAM param;
	SPI_HANDLE handle;
	param.port = ch;
	param.mode = SPI_MODE_MASTER;
	
	result = Spi_open(&handle, &param);		// 指定ポートオープン
	if (result == FALSE)		// オープンに失敗の時
	{
		fram_data.cp.ch = -1;		// SPIハンドルをクリア, 18/12/04
		return FRAM_DRV_OPEN_ERROR;
	}
	
	fram_data.cp.handle = handle;
	fram_data.cp.ch = ch;
	
	return FRAM_DRV_SUCCESS;
}

/****************************************************************/
/**
 * @brief SPIリード
 */
/****************************************************************/
FRAM_DRV_ER fram_drv_read(u32 address, u8* pread_buff, u32 len)
{
	INT8 result;
	SPIM_SEND send;
	u8* psend_buff;
	u8* pspi_read_buff;
	INT32 block,cnt,sub_len;
	UINT32 sub_address;


	block = len / LOCAL_BUFFER_SIZE;
	if(len%LOCAL_BUFFER_SIZE) block++;

	for(cnt = 0;cnt < block; cnt++)
	{
		sub_address = address + LOCAL_BUFFER_SIZE*cnt;
		if(LOCAL_BUFFER_SIZE*(cnt+1) > len)
		{
			sub_len = len%LOCAL_BUFFER_SIZE;
		}
		else
		{
			sub_len = LOCAL_BUFFER_SIZE;
		}

		psend_buff = &ex_spi_local_buff_small[0];
		pspi_read_buff = &ex_spi_local_buff_large[0];
	
		psend_buff[0] = SPICMD_READ;
		psend_buff[1] = (u8)((sub_address>>16)&0xFF);
		psend_buff[2] = (u8)((sub_address>>8)&0xFF);
		psend_buff[3] = (u8)(sub_address&0xFF);

		send.opt = (SPIM_OPT_PACK_END | SPIM_8BIT_GRAN);
		send.send = psend_buff;
		send.recv = pspi_read_buff;
		send.len = sub_len + MEMORY_OPERATION_LENGTH;

		result = Spi_send(&fram_data.cp.handle, fram_data.cp.ch, &send);

		if (result == FALSE){
			return FRAM_DRV_READ_ERROR;
		}
		copy_memory(pread_buff + LOCAL_BUFFER_SIZE*cnt, pspi_read_buff + MEMORY_OPERATION_LENGTH, sub_len);
	}

	return FRAM_DRV_SUCCESS;
}


/****************************************************************/
/**
 * @brief SPIライト
 */
/****************************************************************/
FRAM_DRV_ER fram_drv_write(u32 address, u8* pwrite_buff, u32 len)
{
	INT8 result;
	SPIM_SEND send;
	u8* pspi_write_buff;

	INT32 block,cnt,sub_len;
	UINT32 sub_address;
#if 1
	UINT32 retry = 0;
	u8* psend_buff;
	u8* pspi_read_buff;
	fram_drv_write_status(0x40);
	result = TRUE;
#endif

	block = len / LOCAL_BUFFER_SIZE;
	if(len%LOCAL_BUFFER_SIZE) block++;

	for(cnt = 0;cnt < block; cnt++)
	{
		if(result == FALSE)
		{
			if(retry++ > 5)
			{
				return FRAM_DRV_WRITE_ERROR;
			}
			// 前のブロックを再度実行
			cnt--;
		}
		sub_address = address + LOCAL_BUFFER_SIZE*cnt;
		if(LOCAL_BUFFER_SIZE*(cnt+1) > len)
		{
			sub_len = len%LOCAL_BUFFER_SIZE;
		}
		else
		{
			sub_len = LOCAL_BUFFER_SIZE;
		}
		pspi_write_buff = &ex_spi_local_buff_large[0];

		pspi_write_buff[0] = SPICMD_WRITE;
		pspi_write_buff[1] = (u8)((sub_address>>16)&0xFF);
		pspi_write_buff[2] = (u8)((sub_address>>8)&0xFF);
		pspi_write_buff[3] = (u8)(sub_address&0xFF);

		copy_memory(pspi_write_buff + MEMORY_OPERATION_LENGTH, pwrite_buff + LOCAL_BUFFER_SIZE*cnt, sub_len);

		send.opt = (SPIM_OPT_PACK_END | SPIM_8BIT_GRAN);
		send.send = pspi_write_buff;
		send.recv = NULL;
		send.len = sub_len + MEMORY_OPERATION_LENGTH;

		fram_drv_write_enable();

		result = Spi_send(&fram_data.cp.handle, fram_data.cp.ch, &send);

		if (result == FALSE)
		{
			return FRAM_DRV_WRITE_ERROR;
		}

#if 1
		fill_memory(ex_spi_local_buff_large,0,sub_len);
		psend_buff = &ex_spi_local_buff_small[0];
		pspi_read_buff = &ex_spi_local_buff_large[0];

		psend_buff[0] = SPICMD_READ;
		psend_buff[1] = (u8)((sub_address>>16)&0xFF);
		psend_buff[2] = (u8)((sub_address>>8)&0xFF);
		psend_buff[3] = (u8)(sub_address&0xFF);

		send.opt = (SPIM_OPT_PACK_END | SPIM_8BIT_GRAN);
		send.send = psend_buff;
		send.recv = pspi_read_buff;
		send.len = sub_len + MEMORY_OPERATION_LENGTH;

		result = Spi_send(&fram_data.cp.handle, fram_data.cp.ch, &send);

		if (result == FALSE){
			return FRAM_DRV_READ_ERROR;
		}
		if (!compare_memory(pwrite_buff + LOCAL_BUFFER_SIZE*cnt, pspi_read_buff + MEMORY_OPERATION_LENGTH, sub_len))
		{
			result = FALSE;
			continue;
		}
		result = TRUE;
#endif
	}

	return FRAM_DRV_SUCCESS;
}


/****************************************************************/
/**
 * @brief SPIドライバクローズ
 * @param ch SPIのチャネルをクローズします。
 */
/****************************************************************/
void fram_drv_close(void)
{
	if (fram_data.cp.ch != -1) {		// SPIハンドル獲得済みのとき, 18/12/04
		Spi_close(&fram_data.cp.handle);								/* SPIドライバをクローズ */
		fram_data.cp.ch = -1;		// SPIハンドルをクリア, 18/12/04
	}
}


/* End of File */

