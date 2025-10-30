/******************************************************************************/
/*! @addtogroup Group1
    @file       cyc.h
    @brief      cycle handler header
    @date       2018/02/26
    @author     Development Dept at Tokyo
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
#pragma once

/*----------------------------------------------------------*/
/*			Event Flag										*/
/*----------------------------------------------------------*/
/*----- Feed Motor control ---------------------------------*/
#define EVT_ALL_BIT						0xFFFFFFFF

#define FEED_MOTOR_LOCK_TIME			400					/* 400msec */

#define EVT_FEED_SENSOR					0x00000001			/* sensor detection */
#define EVT_FEED_OVER_PULSE				0x00000002			/* over specified pulse */
#define EVT_FEED_STRING_DOWN			0x00000010			/* straing datection (ad down) */
#define EVT_FEED_STRING_UP				0x00000020			/* straing datection (ad up) */
#define EVT_FEED_MOTOR_STOP				0x00000100			/* feed motor stop */
#define EVT_FEED_TIMEOUT				0x00010000			/* time out */
#define EVT_FEED_MOTOR_LOCK				0x00020000			/* feed motor lock */
#define EVT_FEED_MOTOR_RUNAWAY			0x00040000			/* feed motor runaway */


/*----- Centering Motor control ----------------------------*/
#define EVT_CENTERING_SENSOR			0x00000001			/* sensor detection */
#define EVT_CENTERING_HOME_INTR			0x00000004			/* home interrupt */
#if !defined(PRJ_IVIZION2)
	#define EVT_CENTERING_RUN_TIMEOUT	0x00000002			
	#define EVT_CENTERING_HOME_OUT_INTR	0x00000008			/* home out */
	#define EVT_CENTERING_HOME_OUT_INTR_10MSEC	0x00000020			/* 10msec home out *//* 幅よせclose専用 *//* 2021-05-27	*/
#endif

#define EVT_CENTERING_MOTOR_STOP		0x00000100			/* stacker motor stop */
#define EVT_CENTERING_TIMEOUT			0x00010000			/* time out */
#define EVT_CENTERING_MOTOR_RUNAWAY		0x00040000			/* stacker motor runaway */

/*----- APB Motor control ----------------------------------*/
#define EVT_APB_SENSOR					0x00000001			/* sensor detection */
#if !defined(PRJ_IVIZION2)
#define EVT_APB_OVER_PULSE				0x00000002			/* over specified pulse */
#endif
#define EVT_APB_HOME_INTR				0x00000004			/* home interrupt */
#define EVT_APB_FIRST_HOME_INTR			0x00000008			/* first home interrupt */
#define EVT_APB_MOTOR_STOP				0x00000100			/* stacker motor stop */
#define EVT_APB_TIMEOUT					0x00010000			/* time out */
#define EVT_APB_MOTOR_RUNAWAY			0x00040000			/* stacker motor runaway */



#if !defined(PRJ_IVIZION2)
/*----- Shutter Motor control ----------------------------------*/
#define EVT_SHUTTER_SENSOR					0x00000001			/* sensor detection */
#define EVT_SHUTTER_OVER_PULSE				0x00000002			/* over specified pulse */
//#define EVT_SHUTTER_CLOSE_INTR				0x00000004			/* home interrupt *///割り込み検出を止める
#define EVT_SHUTTER_FIRST_HOME_INTR			0x00000008			/* first home interrupt */
#define EVT_SHUTTER_MOTOR_STOP				0x00000100			/* stacker motor stop */
#define EVT_SHUTTER_TIMEOUT					0x00010000			/* time out */
#define EVT_SHUTTER_MOTOR_RUNAWAY			0x00040000			/* stacker motor runaway */
#endif


/*----- Sensor control -------------------------------------*/
#define	EVT_SENSOR_INIT					0x00000001			/* sensor init */
#define	EVT_SENSOR_SHIFT				0x00000002			/* sensor shift */
#define	EVT_SENSOR_POSITION_AD			0x00000004			/* done position ad */
#define	EVT_SENSOR_RESET_DA				0x00000008			/* reset position da */
#define	EVT_SENSOR_SIDE_LOW				0x00000100			/* side sensor low sensitivity */
#define	EVT_SENSOR_SIDE_HIGH			0x00000200			/* side sensor high sensitivity */
#define	EVT_SENSOR_SIDE_PLUNGE			0x00000400			/* side sensor level plunge */
#define	EVT_SENSOR_SIDE_SURGE			0x00000800			/* side sensor level surge */
#define	EVT_SENSOR_SIDE_RAW_PLUNGE		0x00001000			/* side sensor raw level plunge */
#define	EVT_SENSOR_SIDE_RAW_SURGE		0x00002000			/* side sensor raw level surge */
#define	EVT_SENSOR_SIDE_SAMPLED			0x00004000			/* side sensor ad sampled */

/*----- Motor Encoder Pulse Control ------------------------*/
#define EVT_PULSE_COUNT					0x00000001			/* Motor Encoder */


/*----- Side control -------------------------------------*/
#define	EVT_SIDE_INIT					0x00000001			/* side init */
#define	EVT_SIDE_SHIFT					0x00000002			/* side shift */
#define	EVT_SIDE_POSITION_AD			0x00000004			/* done position ad */
#define	EVT_SIDE_RESET_DA				0x00000008			/* reset position da */
#define	EVT_SIDE_LOW					0x00000100			/* side sensor low sensitivity */
#define	EVT_SIDE_HIGH					0x00000200			/* side sensor high sensitivity */
#define	EVT_SIDE_PLUNGE					0x00000400			/* side sensor level plunge */
#define	EVT_SIDE_SURGE					0x00000800			/* side sensor level surge */
#define	EVT_SIDE_RAW_PLUNGE				0x00001000			/* side sensor raw level plunge */
#define	EVT_SIDE_RAW_SURGE				0x00002000			/* side sensor raw level surge */
#define	EVT_SIDE_SAMPLED				0x00004000			/* side sensor ad sampled */
#define	EVT_SIDE_FEED_PULSE				0x00008000			/* side sensor ad sampling trigger pulse */

/*----------------------------------------------------------*/
/*			Motor Control Sequence Code						*/
/*----------------------------------------------------------*/
#define SEQ_PHASE_BIT			0xFF00

/*----- Feed Sequence Code----------------------------------*/

#if 0
/*----- Stacker Sequence Code ------------------------------*/
#define STK_PLS_STD						(u16)224
/* Stacker full check point */
//#define STK_FUL_TM					(u8)2	/* Stacker FUll detecting time 2ms 		*/
#define STK_FUL_TM						(float)1.8	/* Stacker FUll detecting time 1.8ms 		*/
#define STK_FUL_ST1						(u8)61	/* Stacker FUll detecting start point 	*/
#define STK_FUL_ED1						(u8)70	/* Stacker FUll detecting end point 	*/
#define STK_FUL_ST2						(u8)215	/* Stacker FUll detecting start point	*/
#define STK_FUL_ED2						(u8)219	/* Stacker FUll detecting end point 	*/
/* Stacker status */
#define STK_AVAILABLE					(u8)0x0	/* Stacker FUll detecting end point 	*/
#define STK_FULL						(u8)0x1	/* Stacker FUll detecting end point 	*/
#endif

/*----- Centering Sequence Code ----------------------------*/

/*----- APB Sequence Code ----------------------------------*/

/*----- Sensor Sequence Code ----------------------------------*/
#define SENSOR_SEQ_INIT					0x4100
#define SENSOR_SEQ_LED_ON				0x4200
#define SENSOR_SEQ_POSI_AD				0x4300
#define SENSOR_SEQ_LED_BLINK			0x4400
//#define SENSOR_SEQ_RESET_DA				0x4500
#define SENSOR_SEQ_POSI_ADJ				0x4600
#define SENSOR_SEQ_POSI_ADJ_DETECT		0x4700

/*----- SIDE Sequence Code ----------------------------------*/
#define SIDE_SEQ_INIT				0x5100
#define SIDE_SEQ_LED_ON				0x5200
#define SIDE_SEQ_POSI_AD			0x5300
#define SIDE_SEQ_LED_BLINK			0x5400
#define SIDE_SEQ_ADJ				0x5600
#define SIDE_SEQ_STR_CHECK			0x5700
#define SIDE_SEQ_FEED_PULSE			0x5800

/*----------------------------------------------------------*/
/*			Public Functions								*/
/*----------------------------------------------------------*/
void systimer_init(void);

void _cyc_sensor_proc(void);
void _sensor_ctrl_config(u16 seq_no, u16 off_time, u16 on_time);

void _cyc_side_proc(void);
void _side_ctrl_config(u16 seq_no, u16 off_time, u16 on_time);

/* EOF */
