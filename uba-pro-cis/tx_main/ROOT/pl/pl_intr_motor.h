/******************************************************************************/
/*! @addtogroup Group1
    @file       pl_intr_motor.h
    @brief      motor control header
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


/*----------------------------------------------------------*/
/*			Public Functions								*/
/*----------------------------------------------------------*/
extern void _pl_intr_feed_encooder(void);
extern void _pl_intr_stacker_encooder(void);
extern void _pl_intr_pb_encooder(void);

/* EOF */
