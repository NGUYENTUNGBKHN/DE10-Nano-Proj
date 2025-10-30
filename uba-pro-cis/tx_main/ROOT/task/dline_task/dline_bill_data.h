/******************************************************************************/
/*! @addtogroup Main
    @file       usb_bill_data.h
    @brief      jcm usb test mode header file
    @date       2019/08/29
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan Cash Machine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2019/08/29 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/

#ifndef SRC_MAIN_DLINE_DLINE_BILL_DATA_H_
#define SRC_MAIN_DLINE_DLINE_BILL_DATA_H_

enum OPERATION_BILL_DATA_PHASE_NUMBER
{
	PHASE_ACCEPT_STATUS			= 0x21,
};

extern void front_usb_bill_data_request(void);

#endif /* SRC_MAIN_DLINE_DLINE_BILL_DATA_H_ */
