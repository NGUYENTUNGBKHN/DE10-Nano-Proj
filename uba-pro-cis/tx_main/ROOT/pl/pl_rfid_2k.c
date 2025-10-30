/******************************************************************************/
/*! @addtogroup Group1
    @file       pl_rfid.c
    @brief      RFID(UART) control
    @date       2018/02/26
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018-2019 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
******************************************************************************/
/*! @ingroup hal_rfid hal_rfid */
/* @{ */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"

#include "common.h"
//#include "custom.h"

#include "pl_rfid.h"
#include "pl_rfid_2k.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

/************************** Private Definitions *****************************/

/* [Character code] */
#define		RFID_HEADER					0x40		/* @ */

// 2kコマンド
#define		RFID_CMD_READ_S_2K			0x30
#define 	RFID_CMD_WRITE_S_2K			0x31
#define		RFID_CMD_READ_M_2K			0x33
#define 	RFID_CMD_READ_M_UNLIM_2K	0xB5
#define		RFID_CMD_F_READ_S_2K		0xD0
#define		RFID_CMD_F_WRITE_S_2K		0xD1
#define		RFID_CMD_F_READ_M_2K		0xD3
#define		RFID_CMD_F_READ_M_UNLIM_2K	0xD5

/*--------------------------------------------------------------*/
/**
 * @brief RFID 2k tag read sblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void	rfid_2k_read_sblock_proc(u8 block_start)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 0x000b;
	/**/
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_READ_S_2K;		//30h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x22;						//04h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[0];		//05h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[1];		//06h
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[2];		//07h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[3];		//08h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[4];		//09h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[5];		//0Ah
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[6];		//0Bh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[7];		//0Ch
	bcc ^= data;
	lword.BIT.DATA1 = data = block_start;				//0Dh
	bcc ^= data;
	lword.BIT.DATA2 = bcc;								//0Eh
	lword.BIT.DATA3 = 0;								//0Fh
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(EX_RFID_2K_BLOCK_SIZE+5);
	rfid_tx_data_send_proc(length+4);
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID 2k tag read mblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void	rfid_2k_read_mblock_proc(u8 block_start, u8 count)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 0x000c;
	/**/
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_READ_M_2K;		//33h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x22;						//04h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[0];			//05h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[1];			//06h
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[2];			//07h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[3];			//08h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[4];			//09h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[5];			//0Ah
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[6];			//0Bh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[7];			//0Ch
	bcc ^= data;
	lword.BIT.DATA1 = data = block_start;			//0Dh
	bcc ^= data;
	lword.BIT.DATA2 = data = count - 1;		//0Eh
	bcc ^= data;
	lword.BIT.DATA3 = bcc;								//0Fh
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(count*EX_RFID_2K_BLOCK_SIZE+5);
	rfid_tx_data_send_proc(length+4);
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID 2k tag FAST read sblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void rfid_2k_fast_read_sblock_proc(u8 block_start)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 0x000b;
	/**/
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_F_READ_S_2K;		//D0h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x22;						//04h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[0];		//05h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[1];		//06h
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[2];		//07h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[3];		//08h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[4];		//09h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[5];		//0Ah
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[6];		//0Bh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[7];		//0Ch
	bcc ^= data;
	lword.BIT.DATA1 = data = block_start;				//0Dh
	bcc ^= data;
	lword.BIT.DATA2 = bcc;								//0Eh
	lword.BIT.DATA3 = 0;								//0Fh
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(EX_RFID_2K_BLOCK_SIZE+5);
	rfid_tx_data_send_proc(length+4);
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID 2k tag FAST read mblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void	rfid_2k_fast_read_mblock_proc(u8 block_start, u8 count)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 0x000c;
	/**/
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_F_READ_M_2K;		//D3h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x22;						//04h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[0];			//05h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[1];			//06h
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[2];			//07h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[3];			//08h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[4];			//09h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[5];			//0Ah
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[6];			//0Bh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[7];			//0Ch
	bcc ^= data;
	lword.BIT.DATA1 = data = block_start;			//0Dh
	bcc ^= data;
	lword.BIT.DATA2 = data = count - 1;		//0Eh
	bcc ^= data;
	lword.BIT.DATA3 = bcc;								//0Fh
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(count*EX_RFID_2K_BLOCK_SIZE+5);
	rfid_tx_data_send_proc(length+4);
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID 2k tag read mblock unlimited command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void	rfid_2k_read_mblock_unlimited_proc(u8 block_start, u8 count)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 0x000c;
	/**/
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_READ_M_UNLIM_2K;		//B5h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x22;						//04h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[0];			//05h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[1];			//06h
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[2];			//07h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[3];			//08h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[4];			//09h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[5];			//0Ah
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[6];			//0Bh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[7];			//0Ch
	bcc ^= data;
	lword.BIT.DATA1 = data = block_start;			//0Dh
	bcc ^= data;
	lword.BIT.DATA2 = data = count - 1;		//0Eh
	bcc ^= data;
	lword.BIT.DATA3 = bcc;								//0Fh
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(count*EX_RFID_2K_BLOCK_SIZE+5);
	rfid_tx_data_send_proc(length+4);
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID 2k tag FAST read mblock unlimited command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void	rfid_2k_fast_read_mblock_unlimited_proc(u8 block_start, u8 count)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 0x000c;
	/**/
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_F_READ_M_UNLIM_2K;		//D5h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x22;						//04h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[0];			//05h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[1];			//06h
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[2];			//07h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[3];			//08h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[4];			//09h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[5];			//0Ah
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[6];			//0Bh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[7];			//0Ch
	bcc ^= data;
	lword.BIT.DATA1 = data = block_start;			//0Dh
	bcc ^= data;
	lword.BIT.DATA2 = data = count - 1;		//0Eh
	bcc ^= data;
	lword.BIT.DATA3 = bcc;								//0Fh
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(count*EX_RFID_2K_BLOCK_SIZE+5);
	rfid_tx_data_send_proc(length+4);
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID 2k tag read sblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
u8 rfid_2k_read_sblock_receive_data_proc(u8 block_start)
{
	UINT32 block_offset;

	if(ex_rfid_rx_data[3] == 0)
	{
		block_offset = block_start - rfid_tx_rmb_no;

		ex_rfid_2k_read_data_bufer[(block_offset)*EX_RFID_2K_BLOCK_SIZE]     = ex_rfid_rx_data[4];
		ex_rfid_2k_read_data_bufer[(block_offset)*EX_RFID_2K_BLOCK_SIZE + 1] = ex_rfid_rx_data[5];
		ex_rfid_2k_read_data_bufer[(block_offset)*EX_RFID_2K_BLOCK_SIZE + 2] = ex_rfid_rx_data[6];
		ex_rfid_2k_read_data_bufer[(block_offset)*EX_RFID_2K_BLOCK_SIZE + 3] = ex_rfid_rx_data[7];
		ex_rfid_2k_read_data_bufer[(block_offset)*EX_RFID_2K_BLOCK_SIZE + 4] = ex_rfid_rx_data[8];
		ex_rfid_2k_read_data_bufer[(block_offset)*EX_RFID_2K_BLOCK_SIZE + 5] = ex_rfid_rx_data[9];
		ex_rfid_2k_read_data_bufer[(block_offset)*EX_RFID_2K_BLOCK_SIZE + 6] = ex_rfid_rx_data[10];
		ex_rfid_2k_read_data_bufer[(block_offset)*EX_RFID_2K_BLOCK_SIZE + 7] = ex_rfid_rx_data[11];
		return 0;
	}
	return 1;
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID read mblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
u8 rfid_2k_read_mblock_receive_data_proc(u8 block_start, u8 length)
{
	UINT32 block_offset;

	if(ex_rfid_rx_data[3] == 0)
	{
		block_offset = block_start - rfid_tx_rmb_no;

		for(u8 block = 0; block < length;block++)
		{
			ex_rfid_2k_read_data_bufer[(block_offset+block)*EX_RFID_2K_BLOCK_SIZE]     = ex_rfid_rx_data[4 + block*EX_RFID_2K_BLOCK_SIZE];
			ex_rfid_2k_read_data_bufer[(block_offset+block)*EX_RFID_2K_BLOCK_SIZE + 1] = ex_rfid_rx_data[5 + block*EX_RFID_2K_BLOCK_SIZE];
			ex_rfid_2k_read_data_bufer[(block_offset+block)*EX_RFID_2K_BLOCK_SIZE + 2] = ex_rfid_rx_data[6 + block*EX_RFID_2K_BLOCK_SIZE];
			ex_rfid_2k_read_data_bufer[(block_offset+block)*EX_RFID_2K_BLOCK_SIZE + 3] = ex_rfid_rx_data[7 + block*EX_RFID_2K_BLOCK_SIZE];
			ex_rfid_2k_read_data_bufer[(block_offset+block)*EX_RFID_2K_BLOCK_SIZE + 4] = ex_rfid_rx_data[8 + block*EX_RFID_2K_BLOCK_SIZE];
			ex_rfid_2k_read_data_bufer[(block_offset+block)*EX_RFID_2K_BLOCK_SIZE + 5] = ex_rfid_rx_data[9 + block*EX_RFID_2K_BLOCK_SIZE];
			ex_rfid_2k_read_data_bufer[(block_offset+block)*EX_RFID_2K_BLOCK_SIZE + 6] = ex_rfid_rx_data[10 + block*EX_RFID_2K_BLOCK_SIZE];
			ex_rfid_2k_read_data_bufer[(block_offset+block)*EX_RFID_2K_BLOCK_SIZE + 7] = ex_rfid_rx_data[11 + block*EX_RFID_2K_BLOCK_SIZE];
		}
		return 0;
	}
	return 1;
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID 2k tag write sblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void rfid_2k_write_sblock_proc(void)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 0x0013;
	/**/
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_WRITE_S_2K;		//31h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x22;						//04h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[0];			//05h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[1];			//06h
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[2];			//07h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[3];			//08h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[4];			//09h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[5];			//0Ah
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[6];			//0Bh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[7];			//0Ch
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][0];		//0Dh
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][1];		//0Eh
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][2];		//0Fh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][3];		//10h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][4];		//11h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][5];		//12h
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][6];		//13h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][7];		//14h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][8];		//15h
	bcc ^= data;
	lword.BIT.DATA2 = bcc;								//--h
	lword.BIT.DATA3 = 0x00;								//--h
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(5);
	rfid_tx_data_send_proc(length+4);
}

/*--------------------------------------------------------------*/
/**
 * @brief RFID 2k tag FAST write sblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void rfid_2k_fast_write_sblock_proc(void)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 0x0013;
	/**/
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_F_WRITE_S_2K;		//D1h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x22;						//04h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[0];			//05h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[1];			//06h
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[2];			//07h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[3];			//08h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_info.uid[4];			//09h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_info.uid[5];			//0Ah
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_info.uid[6];			//0Bh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_info.uid[7];			//0Ch
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][0];		//0Dh
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][1];		//0Eh
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][2];		//0Fh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][3];		//10h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][4];		//11h
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][5];		//12h
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][6];		//13h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][7];		//14h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_2k_wsb_set_data[ex_rfid_wsb_write_cnt][8];		//15h
	bcc ^= data;
	lword.BIT.DATA2 = bcc;								//--h
	lword.BIT.DATA3 = 0x00;								//--h
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(5);
	rfid_tx_data_send_proc(length+4);
}


/* EOF */

