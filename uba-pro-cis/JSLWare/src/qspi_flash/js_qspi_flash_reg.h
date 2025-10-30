/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: QSPI Flash Register													*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_QSPI_REG__
#define __J_QSPI_REG__


#define QSPI_CFG				0x0
#define QSPI_DEVRD				0x4
#define QSPI_DEVWR				0x8
#define QSPI_DELAY				0xC
#define QSPI_RDDATACAP			0x10
#define QSPI_DEVSZ				0x14
#define QSPI_SRAMPART			0x18
#define QSPI_INDADDRTRIG		0x1C
#define QSPI_DMAPER				0x20
#define QSPI_REMAPADDR			0x24
#define QSPI_MODEBIT			0x28
#define QSPI_SRAMFILL			0x2C
#define QSPI_TXTHRESH			0x30
#define QSPI_RXTHRESH			0x34
#define QSPI_IRQSTAT			0x40
#define QSPI_IRQMASK			0x44
#define QSPI_LOWWRPROT			0x50
#define QSPI_UPPWRPROT			0x54
#define QSPI_WRPROT				0x58
#define QSPI_INDRD				0x60
#define QSPI_INDRDWATER			0x64
#define QSPI_INDRDSTADDR		0x68
#define QSPI_INDRDCNT			0x6C
#define QSPI_INDWR				0x70
#define QSPI_INDWRWATER			0x74
#define QSPI_INDWRSTADDR		0x78
#define QSPI_INDWRCNT			0x7C
#define QSPI_FLASHCMD			0x90
#define QSPI_FLASHCMDADDR		0x94
#define QSPI_FLASHCMDRDDATALO	0xA0
#define QSPI_FLASHCMDRDDATAUP	0xA4
#define QSPI_FLASHCMDWRDATALO	0xA8
#define QSPI_FLASHCMDWRDATAUP	0xAC
#define QSPI_MODULEID			0xFC


#endif /* __J_QSPI_REG__ */




