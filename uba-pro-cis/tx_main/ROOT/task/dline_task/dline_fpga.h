/******************************************************************************/
/*! @addtogroup Main
    @file       usb_fpga.h
    @brief      jcm usb fpga mode header file
    @date       2018/03/19
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan Cash Machine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/03/19 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/

#ifndef SRC_MAIN_DLINE_DLINE_FPGA_H_
#define SRC_MAIN_DLINE_DLINE_FPGA_H_


/* ====	define					==== */
/*<<	USB communication buffer	>>*/
enum FPGA_PHASE_NUMBER
{
	PHASE_FPGA_EVENT_LOG 				= 0x01,
	PHASE_FPGA_VERSION 					= 0x11,
};


extern void front_usb_fpga_event_log_request(void);

#endif /* SRC_MAIN_DLINE_DLINE_FPGA_H_ */
