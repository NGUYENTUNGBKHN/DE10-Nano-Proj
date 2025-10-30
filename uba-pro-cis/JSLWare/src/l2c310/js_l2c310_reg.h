/*==============================================================================*/
/* Copyright (C) 2015 JSL Technology. All right reserved.						*/
/* Tittle: L2C-310 Register														*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_SRC_REG__
#define __J_SRC_REG__


#define L2C_REG0_CACHE_ID	  			0x000
#define L2C_REG0_CACHE_TYPE		   		0x004
#define L2C_REG1_CONTROL		   		0x100
#define L2C_REG1_AUX	   				0x104
#define L2C_REG1_TAG_RAM_CONTROL		0x108
#define L2C_REG1_DATA_RAM_CONTROL   	0x10C
#define L2C_REG2_EV_COUNTER_CTRL		0x200
#define L2C_REG2_EV_COUNTER1_CFG		0x204
#define L2C_REG2_EV_COUNTER0_CFG		0x208
#define L2C_REG2_EV_COUNTER1   			0x20C
#define L2C_REG2_EV_COUNTER0			0x210
#define L2C_REG2_INT_MASK   			0x214
#define L2C_REG2_INT_MASK_STATUS		0x218
#define L2C_REG2_INT_RAW_STATUS			0x21C
#define L2C_REG2_INT_CLEAR	   			0x220
#define L2C_REG7_CACHE_SYNC   			0x730
#define L2C_REG7_INV_PA		  			0x770
#define L2C_REG7_INV_WAY	  			0x77C
#define L2C_REG7_CLEAN_PA	  			0x7B0
#define L2C_REG7_CLEAN_INDEX  			0x7B8
#define L2C_REG7_CLEAN_WAY				0x7BC
#define L2C_REG7_CLEAN_INV_PA			0x7F0
#define L2C_REG7_CLEAN_INV_INDEX		0x7F8
#define L2C_REG7_CLEAN_INV_WAY			0x7FC
#define L2C_REG9_D_LOCKDOWN0			0x900
#define L2C_REG9_I_LOCKDOWN0			0x904
#define L2C_REG9_D_LOCKDOWN1			0x908
#define L2C_REG9_I_LOCKDOWN1			0x90C
#define L2C_REG9_D_LOCKDOWN2			0x910
#define L2C_REG9_I_LOCKDOWN2			0x914
#define L2C_REG9_D_LOCKDOWN3			0x918
#define L2C_REG9_I_LOCKDOWN3			0x91C
#define L2C_REG9_D_LOCKDOWN4			0x920
#define L2C_REG9_I_LOCKDOWN4			0x924
#define L2C_REG9_D_LOCKDOWN5			0x928
#define L2C_REG9_I_LOCKDOWN5			0x92C
#define L2C_REG9_D_LOCKDOWN6			0x930
#define L2C_REG9_I_LOCKDOWN6			0x934
#define L2C_REG9_D_LOCKDOWN7			0x938
#define L2C_REG9_I_LOCKDOWN7			0x93C
#define L2C_REG9_LOCK_LINE_EN8			0x950
#define L2C_REG9_UNLOCK_WAY8			0x954
#define L2C_REG12_ADDR_FILTERING_START	0xC00
#define L2C_REG12_ADDR_FILTERING_END	0xC04
#define L2C_REG15_DEBUG_CTRL			0xF40
#define L2C_REG15_PREFETCH_CTRL			0xF60
#define L2C_REG15_PWER_CTRL				0xF80


#endif /* __J_SRC_REG__ */




