/******************************************************************************/
/*! @addtogroup Main
    @file       com_ram_ncache.c
    @brief      common variable
    @date       2018/03/05
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/05 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/

#ifndef SRC_MAIN_INCLUDE_COM_RAM_NCACHE_H_
#define SRC_MAIN_INCLUDE_COM_RAM_NCACHE_H_

#include "soc_cv_av/alt_clock_manager.h"
#include "custom.h"
#include "common.h"
#include "struct.h"
#include "fpga.h"

#if defined(EXT)
	#define EXTERN extern
#else
	#undef EXTERN
	#define EXTERN
#endif


#endif /* SRC_MAIN_INCLUDE_COM_RAM_NCACHE_H_ */
/* EOF */
