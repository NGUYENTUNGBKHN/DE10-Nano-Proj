/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: I2C Register															*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_I2C_REG__
#define __J_I2C_REG__


#define	I2C_CON					0x00
#define I2C_TAR					0x04
#define I2C_SAR					0x08
#define I2C_DATA_CMD			0x10
#define I2C_SS_SCL_HCNT			0x14
#define I2C_SS_SCL_LCNT			0x18
#define I2C_FS_SCL_HCNT			0x1C
#define I2C_FS_SCL_LCNT			0x20
#define I2C_INTR_STAT			0x2C
#define I2C_INTR_MASK			0x30
#define I2C_RAW_INTR_STAT		0x34
#define I2C_RX_TL				0x38
#define I2C_TX_TL				0x3C
#define I2C_CLR_INTR			0x40
#define I2C_CLR_RX_UNDER		0x44
#define I2C_CLR_RX_OVER			0x48
#define I2C_CLR_TX_OVER			0x4C
#define I2C_CLR_RD_REQ			0x50
#define I2C_CLR_TX_ABRT			0x54
#define I2C_CLR_RX_DONE			0x58
#define I2C_CLR_ACTIVITY		0x5C
#define I2C_CLR_STOP_DET		0x60
#define I2C_CLR_START_DET		0x64
#define I2C_CLR_GEN_CALL		0x68
#define I2C_ENABLE				0x6C
#define I2C_STATUS				0x70
#define I2C_TXFLR				0x74
#define I2C_RXFLR				0x78
#define I2C_SDA_HOLD			0x7C
#define I2C_TX_ABRT_SOURCE		0x80
#define I2C_SLV_DATA_NACK_ONLY	0x84
#define I2C_DMA_CR				0x88
#define I2C_DMA_TDLR			0x8C
#define I2C_DMA_RDLR			0x90
#define I2C_SDA_SETUP			0x94
#define I2C_ACK_GENERAL_CALL	0x98
#define I2C_ENABLE_STATUS		0x9C
#define I2C_FS_SPKLEN			0xA0
#define I2C_COMP_PARAM_1		0xF4
#define I2C_COMP_VERSION		0xF8
#define I2C_COMP_TYPE			0xFC


#endif /* __J_I2C_REG__ */





