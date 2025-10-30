/******************************************************************************/
/*! @addtogroup Group1
    @file       pl_evrec.h
    @brief      Motor encoder control header
    @date       2021/07/14
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2021 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2021/07/14 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/

/* Public Functions ----------------------------------------------------------- */
void _pl_evrec_init(void);
void _pl_evrec_start(u8 cyc);
void _pl_evrec_stop(void);

/* EOF */
