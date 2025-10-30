/******************************************************************************/
/*! @addtogroup BIF
    @file       jsl_ram.c
    @brief      JSL ware variable
    @date       2021/03/15
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2021 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2021/03/05 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/

/***************************** Include Files *********************************/
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

/*==============================================================================*/
/* UARTドライバ設定用コンフィグレーション定数									*/
/*==============================================================================*/
const UINT32 CFG_UART_SYSCLK[] =		/* ペリフェラルシステムクロック周波数(Hz) */
{
	100000000,	/* L4_SP_CLK:100MHz */
	100000000,	/* L4_SP_CLK:100MHz */
};
const UINT32 CFG_UART_BPS[] =			/* UART目標bps */
{
	9600,
	9600
};
const UINT16 CFG_UART_FMT[] =			/* UARTフォーマット */
{
	(UART_FMT_RXEN|UART_FMT_TXEN|UART_FMT_8BIT|UART_FMT_STOPBIT1|UART_FMT_PARITY_EVEN),
	(UART_FMT_RXEN|UART_FMT_TXEN|UART_FMT_8BIT|UART_FMT_STOPBIT1|UART_FMT_PARITY_EVEN)
};

/*==============================================================================*/
/* SPIコールバック																*/
/*==============================================================================*/
static INT8 spi_send_sub( UINT8 n, UINT8 *buf, UINT16 len, UINT8 smode )
{
	SPIM_SEND pack;

	memset( (void *)&pack, 0, sizeof(pack) );
	pack.opt = 0;
	pack.len = len;

	if( smode & SPI_FRAM_SMODE_READ ){
		pack.recv = buf;
	}
	if( smode & SPI_FRAM_SMODE_WRITE ){
		pack.send = buf;
	}
	if( smode & SPI_FRAM_SMODE_PACKEND ){
		pack.opt |= SPIM_OPT_PACK_END;
	}
	if( Spi_send( &hSpi, 0, &pack ) == FALSE ){
		return( FALSE );
	}

	return( TRUE );
}
/*==============================================================================*/
/* SPI Flashドライバ設定用コンフィグレーション定数								*/
/*==============================================================================*/
static const UINT32 spi_fram0[] =
{
	/*  Size	 */
	128*1024,
	0				/* <-End */
};
const UINT32 *CFG_SPI_FRAM_INFO[] =	/* SPI F-RAMセクター情報 */
{
	spi_fram0,		/* Flash0 */
	0				/* <-End */
};
const spi_send_func CFG_SPI_FRAM_SEND_FUNC[] =
{
	spi_send_sub
};
/* EOF */
