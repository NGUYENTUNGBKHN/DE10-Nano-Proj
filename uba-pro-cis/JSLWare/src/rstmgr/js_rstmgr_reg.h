/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: Reset Manager Register												*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_RSTMGR_REG__
#define __J_RSTMGR_REG__


#define RSTMGR_STAT							0x00
#define RSTMGR_CTRL							0x04
#define RSTMGR_COUNTS						0x08
#define RSTMGR_MPUMODRST					0x10
#define RSTMGR_PERMODRST					0x14
#define RSTMGR_PER2MODRST					0x18
#define RSTMGR_BRGMODRST					0x1C
#define RSTMGR_MISCMODRST					0x20



// レジスタ定義, 19/04/05
typedef union {
	UINT32 lword;
	struct {
		UINT32 emac0:1;
		UINT32 emac1:1;
		UINT32 usb0:1;
		UINT32 usb1:1;
		UINT32 nand:1;
		UINT32 qspi:1;
		UINT32 l4wd0:1;
		UINT32 l4wd1:1;
		UINT32 osc1timer0:1;
		UINT32 osc1timer1:1;
		UINT32 sptimer0:1;
		UINT32 sptimer1:1;
		UINT32 i2c0:1;
		UINT32 i2c1:1;
		UINT32 i2c2:1;
		UINT32 i2c3:1;
		UINT32 uart0:1;
		UINT32 uart1:1;
		UINT32 spim0:1;
		UINT32 spim1:1;
		UINT32 spis0:1;
		UINT32 spis1:1;
		UINT32 sdmmc:1;
		UINT32 can0:1;
		UINT32 can1:1;
		UINT32 gpio0:1;
		UINT32 gpio1:1;
		UINT32 gpio2:1;
		UINT32 dma:1;
		UINT32 sdr:1;
		UINT32 :2;
	} bit;
} RSTMGR_PERMODRST_T;



#endif /* __J_RSTMGR_REG__ */



