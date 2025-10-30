/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: DMA Register															*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_DMA_REG__
#define __J_DMA_REG__


#define DMA_DSR	   				0x000
#define DMA_DPC					0x004
#define DMA_INTEN				0x020
#define DMA_INT_EVENT_RIS		0x024
#define DMA_INTMIS	   			0x028
#define DMA_INTCLR		 	  	0x02C
#define DMA_FSRD   				0x030
#define DMA_FSRC   				0x034
#define DMA_FTRD		 	  	0x038
#define DMA_FTRD(n)		 	  	(0x040*(n*4))
#define DMA_CSR(n)		 	  	(0x100*(n*8))
#define DMA_CPC(n)		 	  	(0x104*(n*8))
#define DMA_SAR(n)		 	  	(0x400*(n*0x20))
#define DMA_DAR(n)		 	  	(0x404*(n*0x20))
#define DMA_CCR(n)		 	  	(0x408*(n*0x20))
#define DMA_LC0(n)		 	  	(0x40C*(n*0x20))
#define DMA_LC1(n)		 	  	(0x410*(n*0x20))
#define DMA_DBGSTATUS		 	0xD00
#define	DMA_DBGCMD				0xD04
#define	DMA_DBGINST0			0xD08
#define	DMA_DBGINST1			0xD0C
#define DMA_CR(n)		 	  	(0xE00*(n*0x4))
#define	DMA_CRD					0xE14
#define	DMA_WD					0xE80
#define	DMA_PERIPH_ID			0xFE0
#define	DMA_PCELL_ID			0xFF0


#endif /* __J_DMA_REG__ */




