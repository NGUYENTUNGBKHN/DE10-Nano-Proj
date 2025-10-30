/******************************************************************************/
/*! @addtogroup Main
    @file       dline_jdl.h
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * dline_jdl.h
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_DLINE_DLINE_JDL_H_
#define SRC_DLINE_DLINE_JDL_H_



enum _USB_JDL_PHASE_NUMBER
{
	USB_JDL_PHASE_START = 0x01,
	USB_JDL_PHASE_DATA = 0x02,
	USB_JDL_PHASE_END = 0x03,
	USB_JDL_PHASE_CLEAR = 0x10,
};


enum _USB_JDL_HOST_CMD_NUMBER
{
	USB_JDL_HOST_CMD_REQ = 0x01,
	USB_JDL_HOST_CMD_ENQ = 0x05,
};


enum _USB_JDL_RES_NUMBER
{
	USB_JDL_RES_OK = 0x00,
	USB_JDL_RES_BUSY = 0x01,
	USB_JDL_RES_ACK = 0x06,
	USB_JDL_RES_NAK = 0x15,
};

#define USB_JDL_ADDR_SIZE	4 /* 4Byte */
#define USB_JDL_PACKT_SIZE	(1023 - (FUSB_HEADER_SIZE + USB_JDL_ADDR_SIZE))

extern void front_usb_jdl_request(void);

#endif /* SRC_DLINE_DLINE_JDL_H_ */

/*--- End of File ---*/
