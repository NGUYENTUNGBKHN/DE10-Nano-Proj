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
#if 1
	9600,
	9600
#else
	115200,
	115200
#endif
};
const UINT16 CFG_UART_FMT[] =			/* UARTフォーマット */
{
#if 1
	(UART_FMT_RXEN|UART_FMT_TXEN|UART_FMT_8BIT|UART_FMT_STOPBIT1|UART_FMT_PARITY_EVEN),
	(UART_FMT_RXEN|UART_FMT_TXEN|UART_FMT_8BIT|UART_FMT_STOPBIT1|UART_FMT_PARITY_EVEN)
#elif 0
	(UART_FMT_RXEN|UART_FMT_TXEN|UART_FMT_8BIT|UART_FMT_STOPBIT1|UART_FMT_PARITY_MARK),
	(UART_FMT_RXEN|UART_FMT_TXEN|UART_FMT_8BIT|UART_FMT_STOPBIT1|UART_FMT_PARITY_MARK)
#else
	(UART_FMT_RXEN|UART_FMT_TXEN|UART_FMT_8BIT|UART_FMT_STOPBIT1),
	(UART_FMT_RXEN|UART_FMT_TXEN|UART_FMT_8BIT|UART_FMT_STOPBIT1)
#endif
};

/*==============================================================================*/
/* SPIドライバ設定用コンフィグレーション定数									*/
/*==============================================================================*/
#if (_DEBUG_SPI_FRAM_JCM==1)
#else
const UINT32 CFG_SPIM_SYSCLK[] =		/* ペリフェラルシステムクロック周波数(Hz) */
{
	200000000,
	200000000,
};
const UINT8 CFG_SPIM_STB[] =			/* STB端子有効bit */
{
	(1<<0),
	(1<<0)
};


const UINT16 spim0_stb_gpio[] = 				/* I/Oテーブル */
{
	GPIO_SPI0_STB, 									/* CS0のGPIO */
	0xFFFF 										/* テーブル終端記述 */
};
const UINT16 *CFG_SPIM_STBGPIOID[] = 	/* STB端子用GPIO_ID */
{
	spim0_stb_gpio
};
const UINT8 CFG_SPIM_FMT[] =			/* SPI出力フォーマット */
{
	(SPIM_FMT_STB_L_ACTIVE | SPIM_FMT_CLK_L_ACTIVE),
	(SPIM_FMT_STB_L_ACTIVE | SPIM_FMT_CLK_L_ACTIVE)
};
const UINT32 CFG_SPIM_CLK[] =			/* SPI目標CLK周波数 */
{
	/* FM25V10 (Read, Write 40MHz) */
	/* 200MHz/8=25MHz */
#if 1
	40000000,
	40000000
#else
	2500000,
	2500000
#endif
};
#endif


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
