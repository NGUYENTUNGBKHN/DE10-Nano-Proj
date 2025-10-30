/******************************************************************************/
/*! @addtogroup Group1
    @file       sensor.h
    @brief      sensro header
    @date       2018/02/26
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/

#ifndef _SRC_INCLUDE_SENSOR_H_
#define _SRC_INCLUDE_SENSOR_H_
/* ad sequence mode */
#define AD_MODE_IDLE					0x0001
//#define AD_MODE_ON_OFF					0x0002
#define AD_MODE_BILL_IN					0x0004
#define AD_MODE_BILL_IN_SIDE_CLEAR		0x0008
//#define AD_MODE_WATCH					0x0010
//#define AD_MODE_WAIT_EDGE_DETECTION		0x0020
//#define AD_MODE_SAMPLING				0x0040
#define AD_MODE_FINISH					0x0080
#define AD_MODE_ONE_SHOT				0x0100
#define AD_MODE_PAY_OUT					0x0200
#define AD_MODE_AGING					0x0400
#define AD_MODE_MAG_ADJUSTMENT			0x0800
#define AD_MODE_VALIDATION_CHECK		0x1000
#define AD_MODE_MAG_OFF					0x0300


#define CIS_MODE_ADJ_PAPER				0x0001	// bill in sampling
#define CIS_MODE_ADJ_NON_PAPER			0x0002	// not used
#define CIS_MODE_AD_PAPER				0x0004	// adjustment ad
#define CIS_MODE_AD_NON_PAPER			0x0008	// adjustment non paper ad
#define CIS_MODE_BC_PAPER				0x0010	// adjustment black shading ad
#define CIS_MODE_BC_NON_PAPER			0x0020	// adjustment non paper
#define CIS_MODE_WC_PAPER				0x0040	// adjustment white shading ad
#define CIS_MODE_WC_NON_PAPER			0x0080	// adjustment non paper
#define CIS_MODE_TMP_NON_PAPER			0x0100	// adjustment tmp ad non paper
#define MAG_MODE_ADJUSTMENT				0x0200
#define CIS_MODE_VALIDATION_CHECK		0x0400

/*----------------------------------------------------------*/
/*		Position Sensor										*/
/*----------------------------------------------------------*/
#if (_DEBUG_POS_INTERVAL_1000MS==1)
#define SENSOR_CHATTERING_ELIMINATE		2		/*  */
#define SENSOR_POSITION_RISE_TIME		1		/* position sensor rise time : 1msec */
#define SENSOR_STANDBY_OFF_INTERVAL		1000	/* [standby] position sensor OFF time : 20msec <- 38msec */
#define SENSOR_STANDBY_ON_INTERVAL		1000	/* [standby] position sensor ON  time :  1msec <- 2msec */
#else
#define SENSOR_CHATTERING_ELIMINATE		2		/*  */
#define SENSOR_POSITION_RISE_TIME		1		/* position sensor rise time : 1msec */
#define SENSOR_STANDBY_OFF_INTERVAL		20	/* [standby] position sensor OFF time : 20msec <- 38msec */
#define SENSOR_STANDBY_ON_INTERVAL		1	/* [standby] position sensor ON  time :  1msec <- 2msec */
#endif
#define SENSOR_ACTIVE_OFF_INTERVAL		0		/* [active ] position sensor OFF time :  0msec */
#define SENSOR_ACTIVE_ON_INTERVAL		0		/* [active ] position sensor ON  time : ∞msec */
#define SENSOR_DA_MAX_8BIT		        255
#define SENSOR_DA_MIN_8BIT              11				/* 0.6V */
#define SENSOR_ENT_ACTUAL_DA_MAX	251

// 補正時MIN
#define SENSOR_ENT_ADJUST_DA_MIN	54
#define SENSOR_CEN_ADJUST_DA_MIN	54
#define SENSOR_PBI_ADJUST_DA_MIN	54
#define SENSOR_PBO_ADJUST_DA_MIN	54
#define SENSOR_EXT_ADJUST_DA_MIN	54

// 補正時MAX    /* 2022-11-21 */
#define SENSOR_ENT_ADJUST_DA_MAX	255
#define SENSOR_CEN_ADJUST_DA_MAX	255
#define SENSOR_PBI_ADJUST_DA_MAX	255
#define SENSOR_PBO_ADJUST_DA_MAX	255
#define SENSOR_EXT_ADJUST_DA_MAX	255
#define SENSOR_POSI_DA_POWER_UP		17	//2024-12-02

// 補正時INITIAL VALUE
#define SENSOR_ENT_ADJUST_DA_INI	54
#define SENSOR_CEN_ADJUST_DA_INI	54
#define SENSOR_PBI_ADJUST_DA_INI	54
#define SENSOR_PBO_ADJUST_DA_INI	54
#define SENSOR_EXT_ADJUST_DA_INI	54

// クリーニングお知らせ閾値 not use
#define SENSOR_ENT_CLEANING_DA_THD	200
#define SENSOR_CEN_CLEANING_DA_THD	200
#define SENSOR_PBI_CLEANING_DA_THD	200
#define SENSOR_PBO_CLEANING_DA_THD	200
#define SENSOR_EXT_CLEANING_DA_THD	200

#define POSITION_SENSOR_NUM             5

#define POSITION_THRESHOLD_ADJ			155			/* 2.0V(155) */
#define POSITION_THRESHOLD_DETECTION	31			/* 0.4V(31) */


extern u8 set_uv_adj(void);
extern void _sensor_ctrl_config(u16 seq_no, u16 off_time, u16 on_time);
#endif
/*--- End of File ---*/
