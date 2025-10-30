/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: GPIO Register														*/
/* Comment:																		*/
/* 	・初期化設定はドライバ側では行われません									*/
/*==============================================================================*/

#ifndef __J_GPIO_REG__
#define __J_GPIO_REG__


#define GPIO_SWPORTA_DR				0x00
#define GPIO_SWPORTA_DDR			0x04
#define GPIO_INTEN					0x30
#define GPIO_INTMASK				0x34
#define GPIO_INTTYPE_LEVEL			0x38
#define GPIO_INT_POLARITY			0x3C
#define GPIO_INTSTATUS				0x40
#define GPIO_RAW_INTSTATUS			0x44
#define GPIO_DEBOUNCE				0x48
#define GPIO_PORTA_EOI				0x4C
#define GPIO_EXT_PORTA				0x50
#define GPIO_LS_SYNC				0x60
#define GPIO_ID_CODE				0x64
#define GPIO_VER_ID_CODE			0x6C
#define GPIO_CONFIG_REG2			0x70
#define GPIO_CONFIG_REG1			0x74


#endif /* __J_GPIO_REG__ */



