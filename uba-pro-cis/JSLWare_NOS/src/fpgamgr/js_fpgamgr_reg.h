/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: FPGA Manager Register												*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_FPGAMGR_REG__
#define __J_FPGAMGR_REG__


#define FPGAMGR_STAT						0x00
#define FPGAMGR_CTRL						0x04
#define FPGAMGR_DCLKCNT						0x08
#define FPGAMGR_DCLKSTAT					0x0C
#define FPGAMGR_GPO							0x10
#define FPGAMGR_GPI							0x14
#define FPGAMGR_MISC						0x18
#define FPGAMGR_MON							0x800
#define FPGAMGR_MON_GPIO_INTEN				0x830
#define FPGAMGR_MON_GPIO_INTMASK			0x834
#define FPGAMGR_MON_GPIO_INTTYPE_LEVEL		0x838
#define FPGAMGR_MON_GPIO_INTTYPE_POLARITY	0x83C
#define FPGAMGR_MON_GPIO_INTSTATUS			0x840
#define FPGAMGR_MON_GPIO_RAW_INTSTATUS		0x844
#define FPGAMGR_MON_GPIO_PORTA_EOI			0x84C
#define FPGAMGR_MON_GPIO_EXT_PORTA			0x850
#define FPGAMGR_MON_GPIO_IS_SYNC			0x860
#define FPGAMGR_MON_GPIO_VER_ID_CODE		0x86C
#define FPGAMGR_MON_GPIO_CONFIG_REG2		0x870
#define FPGAMGR_MON_GPIO_CONFIG_REG1		0x874


#define FPGAMGR_DATA						0x48A000


#endif /* __J_FPGAMGR_REG__ */



