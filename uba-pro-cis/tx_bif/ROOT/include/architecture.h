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

/*----------------------------------------------------------*/
/*			Related Definitions mechanism					*/
/*----------------------------------------------------------*/
#define PITCH	(0.25)

#define CONV_PULSE(x)	( (x) / PITCH )	/* mm -> pulse count */

/*----- Feed Motor control ---------------------------------*/

