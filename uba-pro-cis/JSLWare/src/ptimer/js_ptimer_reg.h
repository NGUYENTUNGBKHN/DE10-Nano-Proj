/*==============================================================================*/
/* Copyright (C) 2014 JSL Technology. All right reserved.						*/
/* Tittle: Private Timer Register												*/
/* Comment:																		*/
/*==============================================================================*/

#ifndef __J_PTIM_REG__
#define __J_PTIM_REG__


#define PTIM_GLOBAL_COUNTER		   	0x200
#define PTIM_GLOBAL_CTRL		   	0x208
#define PTIM_GLOBAL_INTSTAT		   	0x20C
#define PTIM_GLOBAL_COMPVAL		   	0x210
#define PTIM_GLOBAL_AUTOINC		   	0x218

#define PTIM_PRIVATE_TIMERLOAD   	0x600
#define PTIM_PRIVATE_TIMERCOUNTER  	0x604
#define PTIM_PRIVATE_TIMERCTRL   	0x608
#define PTIM_PRIVATE_TIMERINTSTAT	0x60C

#define PTIM_WDT_LOAD		   		0x620
#define PTIM_WDT_COUNTER		   	0x624
#define PTIM_WDT_CTRL   			0x628
#define PTIM_WDT_INTSTAT		   	0x62C
#define PTIM_WDT_RESETSTAT		  	0x630
#define PTIM_WDT_DISABLE		  	0x634


#endif /* __J_PTIM_REG__ */




