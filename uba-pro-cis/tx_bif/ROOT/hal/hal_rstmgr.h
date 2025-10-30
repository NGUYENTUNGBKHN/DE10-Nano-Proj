/******************************************************************************/
/*! @addtogroup Group1
    @file       hal_rstmgr.h
    @brief      Reset manager header
    @date       2021/05/02
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2021 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2021/05/21 H.Suzuki
      -# Initial Version
******************************************************************************/

#ifndef _HAL_RSTMGR_H_
#define _HAL_RSTMGR_H_
/* Public Functions ----------------------------------------------------------- */
extern u32 uart1_reset(void);
extern u32 i2c0_reset(void);
extern u32 i2c3_reset(void);

#endif /* _HAL_RSTMGR_H_ */
/* EOF */
