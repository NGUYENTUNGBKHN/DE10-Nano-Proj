/******************************************************************************/
/*! @addtogroup Group1
    @file       architecture.h
    @brief      architecture header
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
#pragma once

#include "common.h"
/*----------------------------------------------------------*/
/*			Related Definitions mechanism					*/
/*----------------------------------------------------------*/
#define PITCH	0.254

#define CONV_PULSE(x)	( (x) / PITCH )	/* mm -> pulse count */


/* 入口センサからの距離*/
/* 幅よせ  38mm -> 36.6 */
/* 識別    71mm -> 67.1 */
/* PB IN  131mm -> 130.6 */
/* PB OUT 188mm -> 187.2 */
/* EXIT   223mm -> 222.6*/

/* ESP Escrowポジション */
#define DIST_ENT_TO_CEN		(36.6)										/* distance of entrance-sensor to centering-sensor */
#define DIST_ENT_TO_VAL1	(67.1)										/* distance of entrance-sensor to validation-sensor */
#define DIST_ENT_TO_ESP		(118.2)										/* distance of entrance-sensor to mag-sensor *//* ESP Escrowポジション */
#define DIST_ENT_TO_PBI		(130.6)										/* distance of entrance-sensor to APB-IN-sensor */
#define DIST_ENT_TO_PBO		(187.2)										/* distance of entrance-sensor to APB-OUT-sensor */
#define DIST_ENT_TO_EXI		(222.6)										/* distance of entrance-sensor to exit-sensor */

#define DIST_EXI_TO_STP		50									/* distance of exit-sensor to stack-position (LD-mode) */
#define DIST_SKI_TO_STP	    52											/* distance of Stacker-IN-sensor to stack-position (LD-mode) */
#define DIST_CEN_TO_REP		10		/*ハンギング位置、幅よせOFFから*/									/* distance of validation-sensor to reject-position */
#define DIST_CEN_TO_PAP		50									/* distance of validation-sensor to payout hold position */
#define DIST_EXI_TO_END		(193.5)										/* distance of exit-sensor to box-plate */

#define DIST_CEN_TO_ESP		(DIST_ENT_TO_ESP-DIST_ENT_TO_CEN)			/* distance of centering-sensor to validation-sensor */
#define DIST_CEN_TO_VAL 	(DIST_ENT_TO_VAL1-DIST_ENT_TO_CEN)			/* distance of centering-sensor to CIS start positon */
#define DIST_CEN_TO_PBI		(DIST_ENT_TO_PBI-DIST_ENT_TO_CEN)			/* distance of centering-sensor to APB-IN-sensor *//*  130.6 - 36.6 = 94*/
#define DIST_CEN_TO_EXI		(DIST_ENT_TO_EXI-DIST_ENT_TO_CEN)			/* distance of centering-sensor to exit-sensor *//* 222.6 - 36.6 = 186 */
#define DIST_PBI_TO_PBO		(DIST_ENT_TO_PBO-DIST_ENT_TO_PBI)			/* distance of APB-IN-sensor to APB-OUT-sensor *//* 187.2 - 130.6 = 56.6 */
#define DIST_PBO_TO_EXI		(DIST_ENT_TO_EXI-DIST_ENT_TO_PBO)			/* distance of APB-OUT-sensor to exit-sensor *//* 222.6 - 187.2  = 35.4 */

#define LENGTH_LOWER_LIMIT	(120)										/* lower limit of bill length *///2024-03-08 110->120
#define LENGTH_UPPER_LIMIT	(180)										/* upper limit of bill length *///2024-03-08 175->180

#define DIST_ESP_TO_PBI 	(DIST_ENT_TO_PBI-DIST_ENT_TO_ESP)			/* distance of centering-sensor to CIS start positon */


#define PLS_ENT_TO_CEN		(CONV_PULSE(DIST_ENT_TO_CEN))				/* distance of entrance-sensor to centering-sensor */
#define PLS_ENT_TO_PBI		(CONV_PULSE(DIST_ENT_TO_PBI))				/* distance of entrance-sensor to APB-IN-sensor */
#define PLS_ENT_TO_PBO		(CONV_PULSE(DIST_ENT_TO_PBO))				/* distance of entrance-sensor to APB-OUT-sensor */
#define PLS_ENT_TO_EXI		(CONV_PULSE(DIST_ENT_TO_EXI))				/* distance of entrance-sensor to exit-sensor */


#define PLS_EXI_TO_ENT		(PLS_ENT_TO_EXI)							/* distance of entrance-sensor to entrance-sensor */
#define PLS_EXI_TO_CEN		(PLS_ENT_TO_EXI-PLS_ENT_TO_CEN)				/* distance of entrance-sensor to centering-sensor */
#define PLS_EXI_TO_PBI		(PLS_ENT_TO_EXI-PLS_ENT_TO_PBI)				/* distance of entrance-sensor to APB-IN-sensor */
#define PLS_EXI_TO_PBO		(PLS_ENT_TO_EXI-PLS_ENT_TO_PBO)				/* distance of entrance-sensor to APB-OUT-sensor */
#define PLS_EXI_TO_EXI		0											/* distance of entrance-sensor to exit-sensor */

