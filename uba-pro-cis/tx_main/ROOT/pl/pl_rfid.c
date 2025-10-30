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

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

/************************** Private Definitions *****************************/
#define FPGA_RFIDTX_IRQ_INTR_ID			OSW_INT_FPGA_IRQ24
#define FPGA_RFIDRX_IRQ_INTR_ID			OSW_INT_FPGA_IRQ25
/*=============================*/
/*===  Communication memory ===*/
/*=============================*/
/* [Baud rate] */
#define		RFID_BPS9600				129			/*   9600  */
#define		RFID_BPS19200				64			/*  19200  */
#define		RFID_BPS38400				31			/*  38400  */

/* [Character code] */
#define		RFID_HEADER					0x40		/* @ */

/* [Command code] */
#define		RFID_CMD_READ_VER			0x40		/* Firmware version acquisition */
#define		RFID_CMD_RF_CONTROL			0x41		/* RF control */
#define		RFID_CMD_INVENTORY			0x01		/* Inventory */
#define		RFID_CMD_READ_M				0x23		/* 16-block read */
#define		RFID_CMD_WRITE_S			0x21		/* 1-block write */


// 以下はiVIZIONでは未使用のコマンド
#define		RFID_CMD_BAUDRATE			0x43
#define		RFID_CMD_READ_S				0x20
#define		RFID_CMD_READ_S_2K			0x30
#define 	RFID_CMD_WRITE_S_2K			0x31
#define		RFID_CMD_READ_M				0x23
#define		RFID_CMD_READ_M_2K			0x33
#define		RFID_CMD_GET_SYS_INFO		0x2B
#define		RFID_CMD_F_INVENTORY		0xB1
#define 	RFID_CMD_READ_M_UNLIM_2K	0xB5
#define		RFID_CMD_F_READ_S_2K		0xD0
#define		RFID_CMD_F_WRITE_S_2K		0xD1
#define		RFID_CMD_F_READ_M			0xC3
#define		RFID_CMD_F_READ_M_2K		0xD3
#define		RFID_CMD_F_READ_M_UNLIM_2K	0xD5

#define		EX_CNT_RFID_SYS_RW_VER		4
/************************** External Functions *****************************/
extern ER set_int_typ(INTNO intno, irq_type_t type);		///< 割込みタイプ設定

/************************** External Variables *****************************/
/************************** Private Variables *****************************/
OSW_ISR_HANDLE hplIsr24;
OSW_ISR_HANDLE hplIsr25;
/* [Write data] */
//#define		EX_RFID_TAG_BLOCK			60			/* Tag block */
//#define		EX_RFID_TAG_DATA			5			/* Tag block address(1byte) +  Tag data save(4byte) */

#define RFID_RX_SIZE 2048



void _intr_rfid_tready(void )
{
	iset_flg(ID_RFID_FLAG, EVT_RFID_EMPTY);
}
/*--------------------------------------------------*/
/*	BCC計算	(bcc caluculate)					*/
/*                                                  */
/*  引き数：データ                                  */
/*  戻り値：なし                                    */
/*  更新する外部変数：BCC                    */
/*--------------------------------------------------*/
void bcccal(u8* data,u16 len, u8 *bcc)
{
	u8 tmp;

	tmp = *data;
	for(int cnt = 1; cnt < len;cnt++)
	{
		tmp ^= *(data + cnt);
	}
	*bcc = tmp;
}
void _intr_rfid_receive(void )
{
	// FORMAT:
	//      38400 bps
	//      1 start bit
	//      8 data bits
	//      none parity bit
	//      1 stop bit
	// seq0:Received SYNC
	// seq1:Received LENGTH(LOW)
	// seq1:Received LENGTH(HIGH)
	// seq2:Received COMMAND & Data
	// seq3:Received BCC check
	u32 *p_Dst,*p_Src;
	u8 err = 0;
	u8 bcc;
	u16 cnt = 0;
	u16 length;
	RUART_RX_UNION ruart_rx = {0};
	p_Src = (u32 *)FPGA_ADDR_RUARRTDAT;
	p_Dst = (u32 *)ex_rfid_rx_data;
	ruart_rx.LWORD = FPGA_REG.RUART_RX.LWORD;
	if(ruart_rx.BIT.ERR_F == 1)
	{
#if (HAL_STATUS_ENABLE==1)
		ex_hal_status.rfid = HAL_STATUS_NG;
#endif
		err = 1;
		iset_flg(ID_RFID_FLAG, EVT_RFID_ERR);
	}
	else
	{
		length = ruart_rx.BIT.PTR;
	}
	/* copy */
	if(err == 0)
	{
		while(cnt < length)
		{
			*p_Dst = *p_Src;
			p_Dst++;
			p_Src++;
			cnt += 4;
		}
		bcccal(&ex_rfid_rx_data[1], length -2, &bcc);
	}
	/* SYNC */
	if(err == 0)
	{
		if (ex_rfid_rx_data[0] !=  RFID_HEADER)
		{
			err = 1;
		}
	}
	/* LENGTH */
	if(err == 0)
	{
		if(ex_rfid_rx_data[1] + ex_rfid_rx_data[2] * 0x100 + 4 != length)
		{
			err = 1;
		}
	}
	/* BCC check */
	if(err == 0)
	{
		if(ex_rfid_rx_data[length-1] != bcc)
		{
			err = 1;
		}
	}
	/* set result */
	if(err == 0)
	{
#if (HAL_STATUS_ENABLE==1)
		ex_hal_status.rfid = HAL_STATUS_OK;
#endif
		iset_flg(ID_RFID_FLAG, EVT_RFID_RCV);
	}
	else
	{
#if (HAL_STATUS_ENABLE==1)
		ex_hal_status.rfid = HAL_STATUS_NG;
#endif
		iset_flg(ID_RFID_FLAG, EVT_RFID_ERR);
	}
}
/*********************************************************************//**
 * @brief		RFID-TX interrupt
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_intr_rfid_txd(void)
{
	// Disable UART interrupts
	OSW_ISR_disable(FPGA_RFIDTX_IRQ_INTR_ID);
	_intr_rfid_tready();
	OSW_ISR_enable(FPGA_RFIDTX_IRQ_INTR_ID);
}
/*********************************************************************//**
 * @brief		RFID-RX interrupt
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_intr_rfid_rxd(void)
{
	// Disable UART interrupts
	OSW_ISR_disable(FPGA_RFIDRX_IRQ_INTR_ID);
	_intr_rfid_receive();
	OSW_ISR_enable(FPGA_RFIDRX_IRQ_INTR_ID);
}

/*--------------------------------------------------------------*/
/**
 * @brief RFID reset on processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void __pl_rfid_reset_on_proc(void)
{
	RFID_UNION rfid;

	rfid.LWORD = (u32)FPGA_REG.RFID.LWORD;
	rfid.BIT.RST = 1;

	FPGA_REG.RFID.LWORD = rfid.LWORD;
}

/*--------------------------------------------------------------*/
/**
 * @brief RFID reset off processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void __pl_rfid_reset_off_proc(void)
{
	RFID_UNION rfid;

	rfid.LWORD = (u32)FPGA_REG.RFID.LWORD;
	rfid.BIT.RST = 0;

	FPGA_REG.RFID.LWORD = rfid.LWORD;
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID baudrate setting processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void __pl_rfid_baudrate_proc(u8 highspeed)
{
	RFID_UNION rfid;

	rfid.LWORD = (u32)FPGA_REG.RFID.LWORD;
	rfid.BIT.BAUDRATE = highspeed;

	FPGA_REG.RFID.LWORD = rfid.LWORD;
	OSW_TSK_sleep(1);
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID received response processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void rfid_tx_data_receive_proc(u16 length)
{
	RUART_RX_UNION ruart_rx = {0};

	ruart_rx.LWORD = (u32)FPGA_REG.RUART_RX.LWORD;
	ruart_rx.BIT.EN = 1;
	ruart_rx.BIT.INTP = length - 1;
	ruart_rx.BIT.ERR_F = 1;
	ruart_rx.BIT.PCLR = 1;		// 1:clr
	FPGA_REG.RUART_RX.LWORD = ruart_rx.LWORD;
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID send command processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void rfid_tx_data_send_proc(u16 length)
{
	RUART_TX_UNION ruart_tx = {0};

	ruart_tx.LWORD = (u32)FPGA_REG.RUART_TX.LWORD;
	ruart_tx.BIT.SZ = length - 1;		// (転送数 - 1)を設定
	FPGA_REG.RUART_TX.LWORD = ruart_tx.LWORD;
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID get version command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void rfid_read_ver_proc(void)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 1;
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_READ_VER;			//03h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = bcc;								//04h
	lword.BIT.DATA1 = 0;								//--h
	lword.BIT.DATA2 = 0;								//--h
	lword.BIT.DATA3 = 0;								//--h
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(EX_CNT_RFID_SYS_RW_VER+5);
	rfid_tx_data_send_proc(length+4);
}

/*--------------------------------------------------------------*/
/**
 * @brief RFID get version command receive processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
u8 rfid_read_ver_receive_data_proc(void)
{
	ex_rfid_unit.usr_ver = ex_rfid_rx_data[4] | (ex_rfid_rx_data[5]<<8);
	ex_rfid_unit.mkr_ver = ex_rfid_rx_data[6] | (ex_rfid_rx_data[7]<<8);

	return 0;
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID rf control command set processing
 * @param[in] carrier : Carrier ON(0x01),OFF(0x00)
 * @retval none
 */
/*--------------------------------------------------------------*/
void	rfid_rf_control_proc(u8 carrier)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 2;
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_RF_CONTROL;		//03h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = data = carrier;					//04h
	bcc ^= data;
	lword.BIT.DATA1 = bcc;								//05h
	lword.BIT.DATA2 = 0;								//--h
	lword.BIT.DATA3 = 0;								//--h
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(5);
	rfid_tx_data_send_proc(length+4);
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID Baudrate command set processing
 * @param[in] baudrate :
 * 				0x00 :  38,400bps
 * 				0x04 : 115,200bps
 * 				other : not allowed
 * @retval none
 */
/*--------------------------------------------------------------*/
void	rfid_baudrate_proc(u8 baudrate)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 2;
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_BAUDRATE;			//03h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = data = baudrate;					//04h
	bcc ^= data;
	lword.BIT.DATA1 = bcc;								//05h
	lword.BIT.DATA2 = 0;								//--h
	lword.BIT.DATA3 = 0;								//--h
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(5);
	rfid_tx_data_send_proc(length+4);
}


/*--------------------------------------------------------------*/
/**
 * @brief RFID inventory command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void	rfid_inventory_proc(void)
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
	lword.BIT.DATA3 = data = RFID_CMD_INVENTORY;		//03h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x26;						//04h
	bcc ^= data;
	lword.BIT.DATA1 = data = 0x00;						//05h
	bcc ^= data;
	lword.BIT.DATA2 = data = 0x00;						//06h
	bcc ^= data;
	lword.BIT.DATA3 = data = 0x00;						//07h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x00;						//08h
	bcc ^= data;
	lword.BIT.DATA1 = data = 0x00;						//09h
	bcc ^= data;
	lword.BIT.DATA2 = data = 0x00;						//0Ah
	bcc ^= data;
	lword.BIT.DATA3 = data = 0x00;						//0Bh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x00;						//0Ch
	bcc ^= data;
	lword.BIT.DATA1 = data = 0x00;						//0Dh
	bcc ^= data;
	lword.BIT.DATA2 = data = 0x00;						//0Eh
	bcc ^= data;
	lword.BIT.DATA3 = bcc;								//0Fh
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(15);
	rfid_tx_data_send_proc(length+4);
}


/*--------------------------------------------------------------*/
/**
 * @brief RFID FAST inventory command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void rfid_fast_inventory_proc(void)
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
	lword.BIT.DATA3 = data = RFID_CMD_F_INVENTORY;		//03h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x26;						//04h
	bcc ^= data;
	lword.BIT.DATA1 = data = 0x00;						//05h
	bcc ^= data;
	lword.BIT.DATA2 = data = 0x00;						//06h
	bcc ^= data;
	lword.BIT.DATA3 = data = 0x00;						//07h
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x00;						//08h
	bcc ^= data;
	lword.BIT.DATA1 = data = 0x00;						//09h
	bcc ^= data;
	lword.BIT.DATA2 = data = 0x00;						//0Ah
	bcc ^= data;
	lword.BIT.DATA3 = data = 0x00;						//0Bh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = 0x00;						//0Ch
	bcc ^= data;
	lword.BIT.DATA1 = data = 0x00;						//0Dh
	bcc ^= data;
	lword.BIT.DATA2 = data = 0x00;						//0Eh
	bcc ^= data;
	lword.BIT.DATA3 = bcc;								//0Fh
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(15);
	rfid_tx_data_send_proc(length+4);
}

/*--------------------------------------------------------------*/
/**
 * @brief RFID inventory command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
u8 rfid_inventory_receive_data_proc(void)
{
	if(ex_rfid_rx_data[4] == 0)
	{
		ex_rfid_info.dsfid = ex_rfid_rx_data[5];
		ex_rfid_info.uid[0] = ex_rfid_rx_data[6];
		ex_rfid_info.uid[1] = ex_rfid_rx_data[7];
		ex_rfid_info.uid[2] = ex_rfid_rx_data[8];
		ex_rfid_info.uid[3] = ex_rfid_rx_data[9];
		ex_rfid_info.uid[4] = ex_rfid_rx_data[10];
		ex_rfid_info.uid[5] = ex_rfid_rx_data[11];
		ex_rfid_info.uid[6] = ex_rfid_rx_data[12];
		ex_rfid_info.uid[7] = ex_rfid_rx_data[13];
		return 0;
	}
	return 1;
}


/*--------------------------------------------------------------*/
/**
 * @brief RFID get system information command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void	rfid_get_system_information_proc(void)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 0x000a;
	/**/
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_GET_SYS_INFO;		//03h
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
	lword.BIT.DATA1 = bcc;								//0Dh
	lword.BIT.DATA2 = 0x00;								//--h
	lword.BIT.DATA3 = 0x00;								//--h
	/**/
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(19);
	rfid_tx_data_send_proc(length+4);
}

/*--------------------------------------------------------------*/
/**
 * @brief RFID inventory command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
u8 rfid_get_system_information_receive_data_proc(void)
{
	if(ex_rfid_rx_data[3] == 0)
	{
		if(ex_rfid_rx_data[4] & 0x01)
		{
			// bit dsfid on
			ex_rfid_info.dsfid = ex_rfid_rx_data[13];
		}
		else
		{
			ex_rfid_info.dsfid = 0;
		}
		if(ex_rfid_rx_data[4] & 0x02)
		{
			// bit afi on
			ex_rfid_info.afi = ex_rfid_rx_data[14];
		}
		else
		{
			ex_rfid_info.afi = 0;
		}
		if(ex_rfid_rx_data[4] & 0x04)
		{
			// bit tag size on
			ex_rfid_info.blocknum = ex_rfid_rx_data[15];
			ex_rfid_info.blocksize = ex_rfid_rx_data[16];
		}
		else
		{
			ex_rfid_info.blocknum = 0;
			ex_rfid_info.blocksize = 0;
		}
		if(ex_rfid_rx_data[4] & 0x08)
		{
			// bit IC ref on
			ex_rfid_info.icref = ex_rfid_rx_data[17];
		}
		else
		{
			ex_rfid_info.icref = 0;
		}
		return 0;
	}
	return 1;
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID read sblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void	rfid_read_sblock_proc(u8 block_start)
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
	lword.BIT.DATA3 = data = RFID_CMD_READ_S;			//20h
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
	lword.BIT.DATA1 = data = block_start;					//0Dh
	bcc ^= data;
	lword.BIT.DATA2 = bcc;									//0Eh
	lword.BIT.DATA3 = 0;									//0Fh
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(EX_RFID_BLOCK_SIZE+5);
	rfid_tx_data_send_proc(length+4);
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID read mblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void	rfid_read_mblock_proc(u8 block_start, u8 count)
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
	lword.BIT.DATA3 = data = RFID_CMD_READ_M;			//03h
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
	rfid_tx_data_receive_proc(count*EX_RFID_BLOCK_SIZE+5);
	rfid_tx_data_send_proc(length+4);
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID FAST read mblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void	rfid_fast_read_mblock_proc(u8 block_start, u8 count)
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
	lword.BIT.DATA3 = data = RFID_CMD_F_READ_M;			//03h
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
	rfid_tx_data_receive_proc(count*EX_RFID_BLOCK_SIZE+5);
	rfid_tx_data_send_proc(length+4);
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID read sblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void rfid_read_sblock_receive_err_response_proc(void)
{
	RUART_RX_UNION ruart_rx = {0};
	UINT32 length,cnt;

	u32 *p_Dst,*p_Src;
	UINT8 buffer[16];
	UINT8 res_code, err_code;

	p_Src = (u32 *)FPGA_ADDR_RUARRTDAT;
	p_Dst = (u32 *)buffer;
	ruart_rx.LWORD = FPGA_REG.RUART_RX.LWORD;
	length = ruart_rx.BIT.PTR;
	if(length > 4)
	{
		/* copy */
		while(cnt < length)
		{
			*p_Dst = *p_Src;
			p_Dst++;
			p_Src++;
			cnt += 4;
		}
		res_code = buffer[3];
		err_code = buffer[4];
	}
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID read mblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
void rfid_read_mblock_receive_err_response_proc(void)
{
	RUART_RX_UNION ruart_rx = {0};
	UINT32 length,cnt;

	u32 *p_Dst,*p_Src;
	UINT8 buffer[16];
	UINT8 res_code, err_code;

	p_Src = (u32 *)FPGA_ADDR_RUARRTDAT;
	p_Dst = (u32 *)buffer;
	ruart_rx.LWORD = FPGA_REG.RUART_RX.LWORD;
	length = ruart_rx.BIT.PTR;
	if(length > 4)
	{
		/* copy */
		while(cnt < length)
		{
			*p_Dst = *p_Src;
			p_Dst++;
			p_Src++;
			cnt += 4;
		}
		res_code = buffer[3];
		err_code = buffer[4];
	}
}
/*--------------------------------------------------------------*/
/**
 * @brief RFID read sblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
#if 0
u8 rfid_read_sblock_receive_data_proc(u8 block_start)
{
	UINT32 block_offset;

	if(ex_rfid_rx_data[3] == 0)
	{
		block_offset = block_start - rfid_tx_rmb_no;

		ex_ICB_rfid_read_data_bufer[(block_offset)*EX_RFID_BLOCK_SIZE]     = ex_rfid_rx_data[4];
		ex_ICB_rfid_read_data_bufer[(block_offset)*EX_RFID_BLOCK_SIZE + 1] = ex_rfid_rx_data[5];
		ex_ICB_rfid_read_data_bufer[(block_offset)*EX_RFID_BLOCK_SIZE + 2] = ex_rfid_rx_data[6];
		ex_ICB_rfid_read_data_bufer[(block_offset)*EX_RFID_BLOCK_SIZE + 3] = ex_rfid_rx_data[7];
		return 0;
	}
	return 1;
}
#endif	
/*--------------------------------------------------------------*/
/**
 * @brief RFID read mblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
#if 0
u8 rfid_read_mblock_receive_data_proc(u8 block_start, u8 length)
{
	UINT32 block_offset;

	if(ex_rfid_rx_data[3] == 0)
	{
		block_offset = block_start - rfid_tx_rmb_no;

		for(u8 block = 0; block < length;block++)
		{
			ex_ICB_rfid_read_data_bufer[(block_offset+block)*EX_RFID_BLOCK_SIZE]     = ex_rfid_rx_data[4 + block*EX_RFID_BLOCK_SIZE];
			ex_ICB_rfid_read_data_bufer[(block_offset+block)*EX_RFID_BLOCK_SIZE + 1] = ex_rfid_rx_data[5 + block*EX_RFID_BLOCK_SIZE];
			ex_ICB_rfid_read_data_bufer[(block_offset+block)*EX_RFID_BLOCK_SIZE + 2] = ex_rfid_rx_data[6 + block*EX_RFID_BLOCK_SIZE];
			ex_ICB_rfid_read_data_bufer[(block_offset+block)*EX_RFID_BLOCK_SIZE + 3] = ex_rfid_rx_data[7 + block*EX_RFID_BLOCK_SIZE];
		}
		return 0;
	}
	return 1;
}
#endif	
/*--------------------------------------------------------------*/
/**
 * @brief RFID write sblock command set processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
#if 0 //RFID_TYPE_232 2024-06-19
void rfid_write_sblock_proc(void)
{
	u32 offset = 0;
	u32 *p_Word;
	u16 length;
	u8 data;
	u8 bcc = 0;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_RUARTTDAT;
	offset = 0;

	length = 0x000F;
	/**/
	lword.BIT.DATA0 = RFID_HEADER;						//00h
	lword.BIT.DATA1 = data = (length & 0xFF);			//01h
	bcc ^= data;
	lword.BIT.DATA2 = data = ((length >> 8) & 0xFF);	//02h
	bcc ^= data;
	lword.BIT.DATA3 = data = RFID_CMD_WRITE_S;			//03h
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
	lword.BIT.DATA1 = data = ex_rfid_wsb_set_data[ex_rfid_wsb_write_cnt][0];		//0Dh
	bcc ^= data;
	lword.BIT.DATA2 = data = ex_rfid_wsb_set_data[ex_rfid_wsb_write_cnt][1];		//0Eh
	bcc ^= data;
	lword.BIT.DATA3 = data = ex_rfid_wsb_set_data[ex_rfid_wsb_write_cnt][2];		//0Fh
	bcc ^= data;
	*(p_Word + offset++) = lword.LWORD;
	/**/
	lword.BIT.DATA0 = data = ex_rfid_wsb_set_data[ex_rfid_wsb_write_cnt][3];		//10h
	bcc ^= data;
	lword.BIT.DATA1 = data = ex_rfid_wsb_set_data[ex_rfid_wsb_write_cnt][4];		//11h
	bcc ^= data;
	lword.BIT.DATA2 = bcc;								//12h
	lword.BIT.DATA3 = 0x00;								//--h
	*(p_Word + offset++) = lword.LWORD;
	/*#*/
	rfid_tx_data_receive_proc(5);
	rfid_tx_data_send_proc(length+4);
}
#endif
/*--------------------------------------------------------------*/
/**
 * @brief RFID write sblock command receive processing
 * @param[in,out] none
 * @retval none
 */
/*--------------------------------------------------------------*/
u8 rfid_write_sblock_receive_data_proc(void)
{
	if(ex_rfid_rx_data[3] == 0)
	{
		return 0;
	}
	return 1;
}

/*********************************************************************//**
 * @brief		RFID initialize
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_rfid_init(void)
{
	// UART-TX IRQ Handler
	if(hplIsr24.hdl == 0)
	{
		OSW_ISR_create( &hplIsr24, FPGA_RFIDTX_IRQ_INTR_ID, _pl_intr_rfid_txd);
		OSW_ISR_set_priority(FPGA_RFIDTX_IRQ_INTR_ID, IPL_USER_NORMAL);
		set_int_typ(FPGA_RFIDTX_IRQ_INTR_ID, IRQ_EDGE_RISING);
	}
	// Disable Interrupt RFID TXD
	OSW_ISR_disable(FPGA_RFIDTX_IRQ_INTR_ID);


	// UART-RX IRQ Handler
	if(hplIsr25.hdl == 0)
	{
		OSW_ISR_create( &hplIsr25, FPGA_RFIDRX_IRQ_INTR_ID, _pl_intr_rfid_rxd);
		OSW_ISR_set_priority(FPGA_RFIDRX_IRQ_INTR_ID, IPL_USER_NORMAL);
		set_int_typ(FPGA_RFIDRX_IRQ_INTR_ID, IRQ_EDGE_RISING);
	}
	// Disable Interrupt RFID RXD
	OSW_ISR_disable(FPGA_RFIDRX_IRQ_INTR_ID);
}
/*********************************************************************//**
 * @brief		RFID-TX enable
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_rfid_txd_enable(void)
{
	// Disable Interrupt RFID TXD
	OSW_ISR_enable(FPGA_RFIDTX_IRQ_INTR_ID);
}
/*********************************************************************//**
 * @brief		RFID-TX disable
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_rfid_txd_disable(void)
{
	// Disable Interrupt RFID TXD
	OSW_ISR_disable(FPGA_RFIDTX_IRQ_INTR_ID);
}
/*********************************************************************//**
 * @brief		RFID-RX enable
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_rfid_rxd_enable(void)
{
	// Enable Interrupt RFID RXD
	OSW_ISR_enable(FPGA_RFIDRX_IRQ_INTR_ID);
}
/*********************************************************************//**
 * @brief		RFID-RX disable
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_rfid_rxd_disable(void)
{
	// Disable Interrupt RFID RXD
	OSW_ISR_disable(FPGA_RFIDRX_IRQ_INTR_ID);
}


/*********************************************************************//**
 * @brief		RFID finalize
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_rfid_final(void)
{
	OSW_ISR_disable(FPGA_RFIDTX_IRQ_INTR_ID);
	OSW_ISR_disable(FPGA_RFIDRX_IRQ_INTR_ID);
	// RFID-TX IRQ Handler
	OSW_ISR_delete( &hplIsr24);
	hplIsr24.hdl = 0;
	hplIsr24.interrupt_id = 0;
	// RFID-RX IRQ Handler
	OSW_ISR_delete( &hplIsr25);
	hplIsr25.hdl = 0;
	hplIsr25.interrupt_id = 0;
}


/* EOF */

