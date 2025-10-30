/******************************************************************************/
/*! @addtogroup Main
    @file       subline_usb.h
    @brief      jcm usb header file
    @date       2018/03/19
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/19 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/

#ifndef _SRC_TASK_SUBLINE_USB_H_
#define _SRC_TASK_SUBLINE_USB_H_

extern void subline_suite(void);
extern u8 subline_receive_data(void);
extern void subline_send_data(void);

#endif /* _SRC_TASK_SUBLINE_USB_H_ */
/* EOF */
