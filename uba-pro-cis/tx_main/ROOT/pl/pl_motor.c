/******************************************************************************/
/*! @addtogroup Group1
    @file       pl_motor.c
    @brief      Motor cotrol(feed/centering/APB/stacker)
    @date       2018/02/26
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018-2019 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
******************************************************************************/
/* Motor Driver : TB67H400AFTG
 *  Mixed decay mode:37.5% (6clk/16clk)
 *
 *  Pin Driven List
 * PWM IN1 IN2 OUT+ OUT- Function
 * --------------------------------
 *   L   L   L  OFF  OFF standby
 *   H   L   L  OFF  OFF STOP(OFF)
 * --------------------------------
 *   L   L   H    L    L short-brake
 *   H   L   H    L    H counterclockwise
 * --------------------------------
 *   L   H   L    L    L short-brake
 *   H   H   L    H    L clockwise
 * --------------------------------
 *   L   H   H    L    L short-brake
 *   H   H   H    L    L short-brake
 * --------------------------------
 *
 * CW Operation
 * PWM IN1 IN2 OUT+ OUT- Function
 * --------------------------------
 *   H   H   H    L    L short-brake
 *   H   H   L    H    L clockwise
 * --------------------------------
 *
 * CCW Operation
 * PWM IN1 IN2 OUT+ OUT- Function
 * --------------------------------
 *   H   H   H    L    L short-brake
 *   H   L   H    L    H counterclockwise
 * --------------------------------
 *
 */
/*! @ingroup hal_motor hal_motor */
/* @{ */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"

#include "common.h"
//#include "custom.h"

#include "pl_gpio.h"
#include "pl_motor.h"
#include "pl_intr_motor.h"

#define EXT
#include "com_ram.c"
#include "com_ram_ncache.c"
#include "jsl_ram.c"
#include "cis_ram.c"

/************************** Private Definitions *****************************/
extern const MOTOR_LIMIT_STACKER_TABLE motor_limit_stacker_table[STACKER_AD_NUMBER];

/* 搬送 */
#if defined(UBA_RTQ)
	//RTQは搬送固定値
	static const u8 hmot_dac_table[0x100] =	//new feed1 RTQ
	{
	//	0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
		198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,198,
	};
	static const u8 hmot_dac_table_rs[0x100] = //RS
	{
	//	0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	};
#else	//SS
	static const u8 hmot_dac_table[0x100] =	//new feed1
	{
	//	0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15
		//1.8A-1.1A
		//2024-07-29
		//計算ミスにより、予定より低い設定値となっていた為、低温で速度が遅いユニットが発生
		//適切な設定値に修正
		177,177,177,177,177,177,177,177,177,177,177,177,177,177,177,177,
		177,177,177,177,177,177,177,177,177,177,177,177,177,179,180,182,
		183,185,186,187,188,190,191,192,193,194,195,196,197,198,198,199,
		200,201,202,203,204,205,205,206,207,208,209,209,210,211,211,212,
		212,213,213,214,214,215,215,216,216,217,217,218,218,218,219,219,
		219,220,220,220,221,221,221,222,222,222,223,223,223,223,224,224,
		224,224,225,225,225,225,225,226,226,226,226,226,227,227,227,227,
		227,227,228,228,228,228,228,228,228,229,229,229,229,229,229,229,
		230,230,230,230,230,230,230,230,230,231,231,231,231,231,231,231,
		231,231,232,232,232,232,232,232,232,232,232,232,232,232,233,233,
		233,233,233,233,233,233,233,233,233,233,233,234,234,234,234,234,
		234,234,234,234,234,234,234,234,234,234,234,235,235,235,235,235,
		235,235,235,235,235,235,235,235,235,235,235,235,235,235,235,236,
		236,236,236,236,236,236,236,236,236,236,236,236,236,236,236,236,
		236,236,236,236,236,236,236,237,237,237,237,237,237,237,237,237,
		237,237,237,237,237,237,237,237,237,237,237,237,237,237,237,238,
	};
#endif


#if !defined(UBA_RTQ)	//SS 収納初動
static const u8 smot_dac_table_uba_1st_0[0x100] =
{
	//1.2-0.7
	92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,
	92,92,92,96,99,103,105,108,110,112,114,114,115,117,118,119,
	120,121,122,122,123,124,125,126,126,127,128,129,129,130,130,131,
	132,132,133,134,134,135,135,136,137,137,138,138,139,139,140,140,
	140,141,141,142,142,142,143,143,143,144,144,144,144,145,145,145,
	145,146,146,146,146,147,147,147,147,147,148,148,148,148,148,149,
	149,149,149,149,149,150,150,150,150,150,150,150,150,151,151,151,
	151,151,151,151,151,152,152,152,152,152,152,152,152,152,152,153,
	153,153,153,153,153,153,153,153,153,153,153,154,154,154,154,154,
	154,154,154,154,154,154,154,154,154,155,155,155,155,155,155,155,
	155,155,155,155,155,155,155,155,155,155,155,155,156,156,156,156,
	156,156,156,156,156,156,156,156,156,156,156,156,156,156,156,156,
	156,156,156,157,157,157,157,157,157,157,157,157,157,157,157,157,
	157,157,157,157,157,157,157,157,157,157,157,157,157,157,157,157,
	157,157,157,158,158,158,158,158,158,158,158,158,158,158,158,158,
	158,158,158,158,158,158,158,158,158,158,158,158,158,158,158,158,
};
#endif

/************************** External Functions *****************************/
extern ER set_int_typ(INTNO intno, irq_type_t type);		///< 割込みタイプ設定

/************************** External Variables *****************************/
/************************** Private Variables *****************************/
OSW_ISR_HANDLE hplIsr36;
OSW_ISR_HANDLE hplIsr37;

OSW_ISR_HANDLE hplIsr38;

/*********************************************************************//**
 * @brief		Enable Feed encoder interrupt
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_feed_enc_interrupt_enable(void)
{
	OSW_ISR_enable(FEED_ENC_IRQ_INTR_ID);
}
/*********************************************************************//**
 * @brief		Disable Feed encoder interrupt
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_feed_enc_interrupt_disable(void)
{
	OSW_ISR_disable(FEED_ENC_IRQ_INTR_ID);
}
/*********************************************************************//**
 * @brief		Enable Stacker encoder interrupt
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_stacker_enc_interrupt_enable(void)
{
	OSW_ISR_enable(STACKER_ENC_IRQ_INTR_ID);
}
/*********************************************************************//**
 * @brief		Disable Stacker encoder interrupt
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_stacker_enc_interrupt_disable(void)
{
	OSW_ISR_disable(STACKER_ENC_IRQ_INTR_ID);
}


void _pl_pb_enc_interrupt_enable(void)
{
	GpioIsr_enable(__HAL_PB_ENC);
}
void _pl_pb_enc_interrupt_disable(void)
{
	GpioIsr_disable(__HAL_PB_ENC);
}


/*********************************************************************//**
 * @brief		motor initialize(Feed, ADP Encoder)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_motor_init(void)
{
	#if 0 //ブレーキさせっぱなし 2023-01-26a これを有効にすると、半押し動作後直後の温度補正で、モータフリーとなる
	HMOT_CTL_UNION reg_hmot;
	SMOT_CTL_UNION reg_smot;
	ACT_EN_UNION reg_act_en;
	
	LMOT_CTL_UNION reg_shutter_mot;
	PMOT_CTL_UNION reg_pb_mot;
	

	reg_hmot.LWORD = 0;
	reg_smot.LWORD = 0;

	/* feed */
	reg_hmot.BIT.DUTY = 0;
	reg_hmot.BIT.CMD = MOT_CNT_OFF;
	reg_hmot.BIT.ENCDAC = 1;		// new feed 1
	FPGA_REG.HMOT_CTL = reg_hmot;

	/* Stacker */
	reg_smot.BIT.DUTY = 0;
	reg_smot.BIT.CMD = MOT_CNT_OFF;	
	reg_smot.BIT.ENCDAC = 1; //2023-01-25 //new stack 1
	FPGA_REG.SMOT_CTL = reg_smot;

	/* shutter */
	reg_shutter_mot.BIT.DUTY = 0;
	reg_shutter_mot.BIT.CMD = MOT_CNT_OFF;
	FPGA_REG.LMOT_CTL = reg_shutter_mot;

	/* PB */
	reg_pb_mot.BIT.DUTY = 0;
	reg_pb_mot.BIT.CMD = MOT_CNT_OFF;
	FPGA_REG.PMOT_CTL = reg_pb_mot;

	_pl_set_hmot_cur_uba();//feed new feed1
	_pl_set_smot_cur_uba(0);//stacker new stack

	reg_act_en.BIT.EN = 0;
	FPGA_REG.ACT_EN = reg_act_en;

	#endif

	// Feed IRQ Handler
	if(hplIsr36.hdl == 0)
	{
		OSW_ISR_create( &hplIsr36, FEED_ENC_IRQ_INTR_ID, _pl_intr_feed_encooder);
		OSW_ISR_set_priority(FEED_ENC_IRQ_INTR_ID, IPL_USER_HIGHEST);
		set_int_typ(FEED_ENC_IRQ_INTR_ID, IRQ_EDGE_RISING);
	}
	// Disable Interrupt Feed Encoder
    _pl_feed_enc_interrupt_disable();

	// Stacker IRQ Handler
	if(hplIsr37.hdl == 0)
	{
		OSW_ISR_create( &hplIsr37, STACKER_ENC_IRQ_INTR_ID, _pl_intr_stacker_encooder);
		OSW_ISR_set_priority(STACKER_ENC_IRQ_INTR_ID, IPL_USER_HIGHEST);
		set_int_typ(STACKER_ENC_IRQ_INTR_ID, IRQ_EDGE_RISING);
	}
	// Disable Interrupt Feed Encoder
    _pl_stacker_enc_interrupt_disable();

	//PB encoder UBA_GPIO
	// UBA_WS FGPAの問題のようなとりあえず有効 #if defined(UBA_ES) //UBA_WS1  PB encoder FPGA割り込み　廃止 2022-09-27a
	#if 0 //2024-05-20
	if(hplIsr38.hdl == 0)
	{
		OSW_ISR_create( &hplIsr38, PB_ENC_IRQ_INTR_ID, _pl_intr_pb_encooder);
		OSW_ISR_set_priority(PB_ENC_IRQ_INTR_ID, IPL_USER_HIGHEST);
		set_int_typ(PB_ENC_IRQ_INTR_ID, IRQ_EDGE_RISING);
	}

	// Disable Interrupt PB Encoder
    _pl_pb_enc_interrupt_disable();
	#endif


}

void _pl_change_staker_da_uba(u8 mode)
{
	SMOT_CTL_UNION smot_ctl;
	smot_ctl = FPGA_REG.SMOT_CTL;	/* 現在の設定値を読み込み */

	if(mode==0)
	{
		smot_ctl.BIT.REFSEL = 0;	/* disable stacker full DA	*/
	}
	else if(mode==1)
	{
		smot_ctl.BIT.REFSEL = 1;	/* enable stacker full DA	*/
	}
	FPGA_REG.SMOT_CTL = smot_ctl;
}

/*********************************************************************//**
 * @brief		motor finalize(Feed, ADP Encoder)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_motor_final(void)
{
    _pl_feed_enc_interrupt_disable();
	// Feed IRQ Handler
	OSW_ISR_delete( &hplIsr36);
	hplIsr36.hdl = 0;
	hplIsr36.interrupt_id = 0;
	// Stacker IRQ Handler
	OSW_ISR_delete( &hplIsr37);
	hplIsr37.hdl = 0;
	hplIsr37.interrupt_id = 0;

	// pb IRQ Handler(encoder)
	OSW_ISR_delete( &hplIsr38); //not use
	hplIsr38.hdl = 0;
	hplIsr38.interrupt_id = 0;

}


/*********************************************************************//**
 * @brief		Write ENMT
 * @param[in]	u8 set output high(1) or low(1)
 * @return 		None
 **********************************************************************/
void _pl_gpio_set_enmt_uba(u8 set)
{

	ACT_EN_UNION act_en;

	// EMNT
	act_en.BIT.EN = set;
	act_en.BIT.SLP_N = 1;
	FPGA_REG.ACT_EN = act_en;

	dly_tsk(1);
}


/*********************************************************************//**
 * @brief		Adjustment for APB motor speed
 * @param[in]	dir : set motor direction FWD or REV [0～100]
 * @param[in]	speed : set motor speed [0～100]
 * @return 		None
 **********************************************************************/
void _pl_apb_motor_speed_conrol(u8 pwm, MOT_CMD_T mode) /*2021-12-29 *//* この関数、中と外のでFalseセーフ設定している、1つにまとめる */
{
	PMOT_CTL_UNION hmot_ctl;
	DUTY_UNION duty;

	duty.BIT.DAT = 0x7FF * pwm / 100;

	FPGA_MOTPRM_REG.M2[0].BIT.DUTY_INIT.LWORD = duty.LWORD;
	FPGA_MOTPRM_REG.M2[0].BIT.DUTY_LL.LWORD = duty.LWORD;
	FPGA_MOTPRM_REG.M2[0].BIT.DUTY_UL.LWORD = duty.LWORD;

	hmot_ctl.LWORD = 0;
	hmot_ctl.BIT.CMD = mode;
	FPGA_REG.PMOT_CTL = hmot_ctl;

}

/*********************************************************************//**
 * @brief		Adjustment for feed motor speed
 * @param[in]	speed : [0～100] PWM
 * 				mode : [0～3] MOT_CNT_T
 * @return 		None
 **********************************************************************/
void _pl_feed_motor_speed_conrol(u8 speed, MOT_CMD_T mode)
{
	HMOT_CTL_UNION hmot_ctl;
	DUTY_UNION duty;

	duty.BIT.DAT = 0x7FF * speed / 100;

	FPGA_MOTPRM_REG.M0[0].BIT.DUTY_INIT.LWORD = duty.LWORD;
	FPGA_MOTPRM_REG.M0[0].BIT.DUTY_LL.LWORD = duty.LWORD;
	FPGA_MOTPRM_REG.M0[0].BIT.DUTY_UL.LWORD = duty.LWORD;

	hmot_ctl.LWORD = 0;
	//new feed 1
	hmot_ctl.BIT.ENCDAC = 1;

	hmot_ctl.BIT.CMD = mode;
	FPGA_REG.HMOT_CTL = hmot_ctl;
}

/*********************************************************************//**
 * @brief		Adjustment for feed motor speed
 * @param[in]	cycle : speed cycle [1/10ns]
 * 				mode : [0～3] MOT_CNT_T
 * @return 		None
 **********************************************************************/
void _pl_feed_motor_target_speed_conrol(u32 cycle, MOT_CMD_T mode)
{
#if 0//
	//const u32 feed_coe = 134217728;
	const u32 feed_coe = 67108864;

	HMOT_CTL_UNION hmot_ctl;
	u16 value;

	value = feed_coe/cycle;

	/* 2023-02-01 ezawa irai */
	// DGAIN 0x2000->0x1000
	// DUTY_LL 0x400->0x534(65%)
	FPGA_MOTPRM_REG.M0[0].BIT.SPD_OB.LWORD = value;		//	004
	FPGA_MOTPRM_REG.M0[0].BIT.DGAIN.LWORD = 0x1000;		//	008
	FPGA_MOTPRM_REG.M0[0].BIT.PGAIN.LWORD = 0x0800;		//	00C
	FPGA_MOTPRM_REG.M0[0].BIT.IDC.LWORD = 0x0E66;		//	010
	FPGA_MOTPRM_REG.M0[0].BIT.ISM.LWORD = 0x019A;		//	014
	FPGA_MOTPRM_REG.M0[0].BIT.IGAIN.LWORD = 0x0000;		//	018
	FPGA_MOTPRM_REG.M0[0].BIT.FB_LL.LWORD = 0x0000;		//	01C
	FPGA_MOTPRM_REG.M0[0].BIT.FB_UL.LWORD = 0x7FFF;		//	020
	FPGA_MOTPRM_REG.M0[0].BIT.UD_FRQ.LWORD = 0x0000;	//	024
	FPGA_MOTPRM_REG.M0[0].BIT.CNT2NXT.LWORD = 0x0000;	//	030
	FPGA_MOTPRM_REG.M0[0].BIT.DUTY_INIT.LWORD = 0x7FF;
	FPGA_MOTPRM_REG.M0[0].BIT.DUTY_LL.LWORD = 0x534;
	FPGA_MOTPRM_REG.M0[0].BIT.DUTY_UL.LWORD = 0x7FF;

	hmot_ctl.LWORD = 0;

	hmot_ctl.BIT.ENCDAC = 1;

	hmot_ctl.BIT.CMD = mode;
	FPGA_REG.HMOT_CTL = hmot_ctl;

#else //

	//const u32 feed_coe = 134217728;
	const u32 feed_coe = 67108864;
	HMOT_CTL_UNION hmot_ctl;
	u16 value;

	value = feed_coe/cycle;

	FPGA_MOTPRM_REG.M0[0].BIT.SPD_OB.LWORD = value;		//	004

	#if 1 //new feed 1 2022-12-21
	FPGA_MOTPRM_REG.M0[0].BIT.DGAIN.LWORD = 0x1000;		//	008 // 
	#else
	FPGA_MOTPRM_REG.M0[0].BIT.DGAIN.LWORD = 0x2000;		//	008
	#endif

	FPGA_MOTPRM_REG.M0[0].BIT.PGAIN.LWORD = 0x0800;		//	00C
	FPGA_MOTPRM_REG.M0[0].BIT.IDC.LWORD = 0x0E66;		//	010
	FPGA_MOTPRM_REG.M0[0].BIT.ISM.LWORD = 0x019A;		//	014
	FPGA_MOTPRM_REG.M0[0].BIT.IGAIN.LWORD = 0x0000;		//	018
	FPGA_MOTPRM_REG.M0[0].BIT.FB_LL.LWORD = 0x0000;		//	01C
	FPGA_MOTPRM_REG.M0[0].BIT.FB_UL.LWORD = 0x7FFF;		//	020
	FPGA_MOTPRM_REG.M0[0].BIT.UD_FRQ.LWORD = 0x0000;	//	024
	FPGA_MOTPRM_REG.M0[0].BIT.CNT2NXT.LWORD = 0x0000;	//	030
	FPGA_MOTPRM_REG.M0[0].BIT.DUTY_INIT.LWORD = 0x7FF;

#if 0 //new feed 1 2022-12-21
	FPGA_MOTPRM_REG.M0[0].BIT.DUTY_LL.LWORD = 0x600;	//PWM下限75%
#else //2025-02-12 搬送速度を550mm/sceに落とす為 PWM下限を50%
	FPGA_MOTPRM_REG.M0[0].BIT.DUTY_LL.LWORD = 0x400;	//PWM下限を50%
#endif

	FPGA_MOTPRM_REG.M0[0].BIT.DUTY_UL.LWORD = 0x7FF;

	hmot_ctl.LWORD = 0;
	#if 1//2022-12-21 new feed 1 
	hmot_ctl.BIT.ENCDAC = 1;
	#endif
	hmot_ctl.BIT.CMD = mode;
	FPGA_REG.HMOT_CTL = hmot_ctl;

#endif //end 
}

/*********************************************************************//**
 * @brief		Adjustment for centering motor speed
 * @param[in]	speed : [0～100] PWM
 * @return 		None
 **********************************************************************/
void _pl_centering_motor_speed_conrol(u8 speed, MOT_CMD_T mode)
{
	CMOT_CTL_UNION cmot_ctl;
	DUTY_UNION duty;

	duty.BIT.DAT = 0x7FF * speed / 100;

	cmot_ctl.LWORD = 0;
	cmot_ctl.BIT.DUTY = duty.BIT.DAT;
	cmot_ctl.BIT.CMD = mode;
	FPGA_REG.CMOT_CTL = cmot_ctl;
}

void _pl_stacker_motor_speed_conrol_uba(u8 speed, MOT_CMD_T mode)
{

	SMOT_CTL_UNION smot_ctl;
	DUTY_UNION duty;

	duty.BIT.DAT = 0x7FF * speed / 100;

	FPGA_MOTPRM_REG.M1[0].BIT.DUTY_INIT.LWORD = duty.LWORD;
	FPGA_MOTPRM_REG.M1[0].BIT.DUTY_LL.LWORD = duty.LWORD;
	FPGA_MOTPRM_REG.M1[0].BIT.DUTY_UL.LWORD = duty.LWORD;

	smot_ctl = FPGA_REG.SMOT_CTL;
	smot_ctl.BIT.PMSET = 0;
	smot_ctl.BIT.ENCDAC = 1;
	smot_ctl.BIT.CMD = mode;
	FPGA_REG.SMOT_CTL = smot_ctl;
}

/*********************************************************************//**
 * @brief		Stop feed motor
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_feed_motor_stop(void)
{

	__disable_irq();

	_pl_feed_motor_speed_conrol(0, MOT_CNT_STOP);

	__enable_irq();

	dly_tsk(1);
}


/*********************************************************************//**
 * @brief		Start feed motor for clockwise
 * @param[in]	speed : [0～100] PWM
 * @return 		None
 **********************************************************************/
void _pl_feed_motor_cw(u8 speed)
{

	__disable_irq();

	switch(speed)
	{
	case FEED_MOTOR_SPEED_PWM:
		/* PWM */
		_pl_feed_motor_speed_conrol(_ir_feed_motor_ctrl.pwm, MOT_CNT_CW);
		break;
	case FEED_MOTOR_SPEED_FULL:
		_pl_feed_motor_speed_conrol(MOTOR_MAX_SPEED, MOT_CNT_CW);
		break;
	case FEED_MOTOR_SPEED_600MM:
		_pl_feed_motor_target_speed_conrol(TARGET_SPEED_CYCLE_600MM, MOT_CNT_CW);
		break;
	case FEED_MOTOR_SPEED_300MM:
		_pl_feed_motor_target_speed_conrol(TARGET_SPEED_CYCLE_300MM, MOT_CNT_CW);
		break;
	case FEED_MOTOR_SPEED_350MM:
		_pl_feed_motor_target_speed_conrol(TARGET_SPEED_CYCLE_350MM, MOT_CNT_CW);
		break;
#if defined(UBA_RTQ)
	case FEED_MOTOR_SPEED_550M:
		_pl_feed_motor_target_speed_conrol(TARGET_SPEED_CYCLE_5500MM, MOT_CNT_CW);
		break;
#endif // UBA_RTQ
	case FEED_MOTOR_SPEED_STOP:
	default:
		_pl_feed_motor_speed_conrol(MOTOR_MIN_SPEED, MOT_CNT_CW);
		break;
	}

	__enable_irq();

	dly_tsk(1);

}


/*********************************************************************//**
 * @brief		Start feed motor for counter clockwise
 * @param[in]	speed : [1～16] PWM
 * @return 		None
 **********************************************************************/
void _pl_feed_motor_ccw(u8 speed)
{

	__disable_irq();

	switch(speed)
	{
	case FEED_MOTOR_SPEED_PWM:
		/* PWM */
		_pl_feed_motor_speed_conrol(_ir_feed_motor_ctrl.pwm, MOT_CNT_CCW);
		break;
	case FEED_MOTOR_SPEED_FULL:
		_pl_feed_motor_speed_conrol(MOTOR_MAX_SPEED, MOT_CNT_CCW);
		break;
	case FEED_MOTOR_SPEED_600MM:
		_pl_feed_motor_target_speed_conrol(TARGET_SPEED_CYCLE_600MM, MOT_CNT_CCW);
		break;
	case FEED_MOTOR_SPEED_300MM:
		_pl_feed_motor_target_speed_conrol(TARGET_SPEED_CYCLE_300MM, MOT_CNT_CCW);
		break;
	case FEED_MOTOR_SPEED_350MM:
		_pl_feed_motor_target_speed_conrol(TARGET_SPEED_CYCLE_350MM, MOT_CNT_CCW);
		break;
#if defined(UBA_RTQ)
	case FEED_MOTOR_SPEED_550M:
		_pl_feed_motor_target_speed_conrol(TARGET_SPEED_CYCLE_5500MM, MOT_CNT_CCW);
		break;
#endif // UBA_RTQ
	case FEED_MOTOR_SPEED_STOP:
	default:
		_pl_feed_motor_speed_conrol(MOTOR_MIN_SPEED, MOT_CNT_CCW);
		break;
	}

	__enable_irq();

	dly_tsk(1);
}


/*********************************************************************//**
 * @brief		APB motor stop
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_apb_motor_stop(void)
{
	__disable_irq();

	/* Enable motor control signal */
	_pl_apb_motor_speed_conrol(0, MOT_CNT_STOP);

	/* Enable motor control signal */
	__enable_irq();
}

/*********************************************************************//**
 * @brief		Start APB motor for clockwise
 * @param[in]	speed : [0～100] PWM
 * @return 		None
 **********************************************************************/
void _pl_apb_motor_cw(u8 pwm)	/* すでにpwm設定されているので、引数の必要なし */
{
	__disable_irq();

	/* Enable motor control signal */


	#if 1	/* FPGA側での速度調整しないので、変数pwmで統一 */
	if( _ir_apb_motor_ctrl.pwm >= 0 || _ir_apb_motor_ctrl.pwm <= 100)
	{
		_pl_apb_motor_speed_conrol(_ir_apb_motor_ctrl.pwm, MOT_CNT_CW);	
	}
	else
	{
		_pl_apb_motor_speed_conrol(MOTOR_MIN_SPEED, MOT_CNT_CW);
	}
	#else
	switch(speed)
	{
	case UBA_MOTOR_SPEED_PWM:
		/* PWM */
		_pl_apb_motor_speed_conrol(_ir_apb_motor_ctrl.pwm, MOT_CNT_CW);
		break;
	case UBA_MOTOR_SPEED_FULL:
		_pl_apb_motor_speed_conrol(MOTOR_MAX_SPEED, MOT_CNT_CW);
		break;
	case UBA_MOTOR_SPEED_STOP:
	default:
		_pl_apb_motor_speed_conrol(MOTOR_MIN_SPEED, MOT_CNT_CW);
		break;
	}
	#endif

	/* Enable motor control signal */
	__enable_irq();
}


/*********************************************************************//**
 * @brief		Start APB motor for counter clockwise
 * @param[in]	speed : [0～100] PWM
 * @return 		None
 **********************************************************************/
void _pl_apb_motor_ccw(u8 pwm)	/* すでにpwm設定されているので、引数の必要なし */
{
	__disable_irq();

	/* Enable motor control signal */


	#if 1	/* FPGA側での速度調整しないので、変数pwmで統一 */
	if( _ir_apb_motor_ctrl.pwm >= 0 || _ir_apb_motor_ctrl.pwm <= 100)
	{
		_pl_apb_motor_speed_conrol(_ir_apb_motor_ctrl.pwm, MOT_CNT_CCW);
	}
	else
	{
		_pl_apb_motor_speed_conrol(MOTOR_MIN_SPEED, MOT_CNT_CCW);
	}
	#else
	switch(speed)
	{
	case UBA_MOTOR_SPEED_PWM:
		/* PWM */
		_pl_apb_motor_speed_conrol(_ir_apb_motor_ctrl.pwm, MOT_CNT_CCW);
		break;
	case UBA_MOTOR_SPEED_FULL:
		_pl_apb_motor_speed_conrol(MOTOR_MAX_SPEED, MOT_CNT_CCW);
		break;
	case UBA_MOTOR_SPEED_STOP:
	default:
		_pl_apb_motor_speed_conrol(MOTOR_MIN_SPEED, MOT_CNT_CCW);
		break;
	}
	#endif

	/* Enable motor control signal */
	__enable_irq();
}

/*********************************************************************//**
 * @brief		Centering motor stop
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_centering_motor_stop(void)
{

	ex_centor_motor_run = 0;

	__disable_irq();

	_pl_centering_motor_speed_conrol(0, MOT_CNT_STOP);

	__enable_irq();

	dly_tsk(1);

}


/*********************************************************************//**
 * @brief		Start Centering motor for clockwise
 * @param[in]	speed : [0～100] PWM
 * @return 		None
 **********************************************************************/
void _pl_centering_motor_cw(u8 speed)
{

	ex_centor_motor_run = 1;

	__disable_irq();

	if(((s32)speed >= MOTOR_MIN_SPEED) && ((s32)speed <= MOTOR_MAX_SPEED))
	{
		/* PWM */
		_pl_centering_motor_speed_conrol(speed, MOT_CNT_CW);
	}

	__enable_irq();

	dly_tsk(1);

}

/*********************************************************************//**
 * @brief		Start Centering motor for counter clockwise
 * @param[in]	speed : [0～100] PWM
 * @return 		None
 **********************************************************************/
void _pl_centering_motor_ccw(u8 speed)
{

	ex_centor_motor_run = 1;

	__disable_irq();

	if(((s32)speed >= MOTOR_MIN_SPEED) && ((s32)speed <= MOTOR_MAX_SPEED))
	{
		/* PWM */
		_pl_centering_motor_speed_conrol(speed, MOT_CNT_CCW);
	}

	__enable_irq();

	dly_tsk(1);

}

void _pl_stacker_motor_cw_uba(void)
{
	__disable_irq();

	/* Enable motor control signal */
	_pl_stacker_motor_speed_conrol_uba(MOTOR_MAX_SPEED, MOT_CNT_CW);
	/* Enable motor control signal */

	__enable_irq();
	dly_tsk(1);
	/* Enable motor control signal */
}


/*********************************************************************//**
 * @brief		Start stacker motor for counter clockwise
 * @param[in]	speed : [1～16] PWM
 * @return 		None
 **********************************************************************/
void _pl_stacker_motor_ccw_uba(void)
{
	__disable_irq();

	_pl_stacker_motor_speed_conrol_uba(MOTOR_MAX_SPEED, MOT_CNT_CCW);

	__enable_irq();

	dly_tsk(1);
}


/*********************************************************************//**
 * @brief		Stacker motor stop
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_stacker_motor_stop(void)
{

	__disable_irq();
	_pl_stacker_motor_speed_conrol_uba(0, MOT_CNT_STOP);
	__enable_irq();

	dly_tsk(1);

}
/*****************************************************************************/
/**
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_set_cmot_cur(void)
{
	FPGA_DAC_REG.DAC_DATA[DAC_INDEX_CMOT_CUR].LWORD = (u32)_ir_centering_motor_ctrl.max_current;

	return ALARM_CODE_OK;
}

u8 _pl_set_smot_cur_uba(u8 mode)
{
	//new stack
	u32 index;
	SMOT_CTL_UNION smot_ctl;

	smot_ctl = FPGA_REG.SMOT_CTL;
	smot_ctl.BIT.PMSET = 0;
	
	if(mode == MOTOR_STATE_FWD)
	{
	//FWD
		smot_ctl.BIT.ENCDAC = 1;
		for(index = 0; index < 0x100; index++)
		{
		#if defined(UBA_RTQ)
			//RTQは押し込み初動は電圧固定で、温度によって変える
			FPGA_DAC_REG.SMOT_DAC0[index] = (u32)motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st;
		#else
			//SSは押し込み初動は電圧可変で、温度による違いはなし
			FPGA_DAC_REG.SMOT_DAC0[index] = (u32)smot_dac_table_uba_1st_0[index];
		#endif
			//押し込み2段目は、電圧固定で、温度によって変える
			FPGA_DAC_REG.SMOT_DAC1[index] = (u32)motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_2nd;
		}
	}
	else if(mode == MOTOR_STATE_REV)
	{
	//REV
		smot_ctl.BIT.ENCDAC = 1;
		for(index = 0; index < 0x100; index++)
		{
			FPGA_DAC_REG.SMOT_DAC0[index] = (u32)motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st;
			FPGA_DAC_REG.SMOT_DAC1[index] = (u32)motor_limit_stacker_table[motor_limit_stacker_table_index].limit2_1st;
		}
	}
	else if(mode == 0xFF)
	{
	//
		smot_ctl.BIT.ENCDAC = 1;
		for(index = 0; index < 0x100; index++)
		{
		#if defined(UBA_RTQ)
			FPGA_DAC_REG.SMOT_DAC0[index] = (u32)motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st;
		#else
			FPGA_DAC_REG.SMOT_DAC0[index] = (u32)smot_dac_table_uba_1st_0[index];
		#endif
			FPGA_DAC_REG.SMOT_DAC1[index] = (u32)motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_2nd;
		}
	}
	else
	{
	//その他は変更しない
		smot_ctl.BIT.ENCDAC = 1;
		for(index = 0; index < 0x100; index++)
		{
		#if defined(UBA_RTQ)
			FPGA_DAC_REG.SMOT_DAC0[index] = (u32)motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_1st;
		#else
			FPGA_DAC_REG.SMOT_DAC0[index] = (u32)smot_dac_table_uba_1st_0[index];
		#endif
			FPGA_DAC_REG.SMOT_DAC1[index] = (u32)motor_limit_stacker_table[motor_limit_stacker_table_index].limit1_2nd;
		}
	}
	FPGA_REG.SMOT_CTL = smot_ctl;
}

u8 _pl_set_hmot_cur_uba(void)
{
	//new feed
 	u32 index;
	for(index = 0; index < 0x100; index++)
	{
	#if defined(UBA_RTQ) //2025-06-06
		if(is_rc_rs_unit())
		{
			FPGA_DAC_REG.HMOT_DAC[index] = (u32)hmot_dac_table_rs[index];
		}
		else
		{
			FPGA_DAC_REG.HMOT_DAC[index] = (u32)hmot_dac_table[index];
		}
	#else
		FPGA_DAC_REG.HMOT_DAC[index] = (u32)hmot_dac_table[index];
	#endif
	}
	return ALARM_CODE_OK;
}

void _pl_shutter_motor_speed_conrol(u8 pwm, MOT_CMD_T mode)
{

	LMOT_CTL_UNION hmot_ctl;
	DUTY_UNION duty;

	duty.BIT.DAT = 0x7FF * pwm / 100;

	hmot_ctl.LWORD = 0;
	hmot_ctl.BIT.DUTY = duty.BIT.DAT;
	hmot_ctl.BIT.CMD = mode;
	FPGA_REG.LMOT_CTL = hmot_ctl;

}

void _pl_shutter_motor_cw(u8 pwm)
{
	__disable_irq();

	/* Enable motor control signal */
	_pl_shutter_motor_speed_conrol(_ir_shutter_motor_ctrl.pwm, MOT_CNT_CW);

	/* Enable motor control signal */
	__enable_irq();

}

void _pl_shutter_motor_ccw(u8 pwm)
{
	__disable_irq();

	/* Enable motor control signal */
	_pl_shutter_motor_speed_conrol(_ir_shutter_motor_ctrl.pwm, MOT_CNT_CCW);

	/* Enable motor control signal */
	__enable_irq();

}

/*****************************************************************************/
/**PB
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_set_pbmot_cur(void)
{

	FPGA_DAC_REG.DAC_DATA[DAC_INDEX_PBMOT_CUR].LWORD = (u32)_ir_apb_motor_ctrl.max_current;

	return ALARM_CODE_OK;
}

/*****************************************************************************/
/**Shutter
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_set_shutter_mot_cur(void)
{

	FPGA_DAC_REG.DAC_DATA[DAC_INDEX_SHUTTER_MOT_CUR].LWORD = (u32)_ir_shutter_motor_ctrl.max_current;

	return ALARM_CODE_OK;
}

void _pl_shutter_motor_stop(void)
{
	__disable_irq();

	/* Enable motor control signal */
	_pl_shutter_motor_speed_conrol(0, MOT_CNT_STOP);

	/* Enable motor control signal */
	__enable_irq();
}

/* EOF */

