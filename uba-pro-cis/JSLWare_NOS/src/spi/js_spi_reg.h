/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: SPI Register															*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_SPI_REG__
#define __J_SPI_REG__


#define SPI_CTRLR0				0x0		/* master & slave */
#define SPI_CTRLR1				0x4		/* master */
#define SPI_SPIENR				0x8		/* master & slave */
#define SPI_NWCR				0xC		/* master & slave */
#define SPI_SER					0x10	/* master */
#define SPI_BAUDR				0x14	/* master */
#define SPI_TXFTLR				0x18	/* master & slave */
#define SPI_RXFTLR				0x1C	/* master & slave */
#define SPI_TXFLR				0x20	/* master & slave */
#define SPI_RXFLR				0x24	/* master & slave */
#define SPI_SR					0x28	/* master & slave */
#define SPI_IMR					0x2C	/* master & slave */
#define SPI_ISR					0x30	/* master & slave */
#define SPI_RISR				0x34	/* master & slave */
#define SPI_TXOICR				0x38	/* master & slave */
#define SPI_RXOICR				0x3C	/* master & slave */
#define SPI_RXUICR				0x40	/* master & slave */
#define SPI_ICR					0x48	/* master & slave */
#define SPI_DMACR				0x4C	/* master & slave */
#define SPI_DMATDIR				0x50	/* master & slave */
#define SPI_DMARDIR				0x54	/* master & slave */
#define SPI_IDR					0x58	/* master & slave */
#define SPI_VERSION_ID			0x5C	/* master & slave */
#define SPI_DR					0x60	/* master & slave */
#define SPI_RX_SAMPLE_DRY		0xFC	/* master */


#endif /* __J_SPI_REG__ */




