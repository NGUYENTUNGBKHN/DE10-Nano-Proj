/****************************************************************************/
/*                                                                          */
/*               Copyright(C) 2003-2015 Grape Systems, Inc.                 */
/*                            All Rights Reserved                           */
/*                                                                          */
/* This software is furnished under a license and may be used and copied    */
/* only in accordance with the terms of such license and with the inclusion */
/* of the above copyright notice. No title to and ownership of the software */
/* is transferred.                                                          */
/* Grape Systems Inc. makes no representation or warranties with respect to */
/* the performance of this computer program, and specifically disclaims any */
/* responsibility for any damages, special or consequential, connected with */
/* the use of this program.                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* FILE NAME                                                    VERSION     */
/*                                                                          */
/*      comm_cnf.h                                                1.40      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This file performs Abstract Control Model of Communication Device   */
/*      Class. (User dependence)                                            */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*   NAME       DATE        REMARKS                                         */
/*                                                                          */
/*   M.Suzuki   2004/05/12  V1.00                                           */
/*                          Created initial version                         */
/*   M.Suzuki   2004/05/27  V1.11                                           */
/*                          version is updated                              */
/*   S.Tomizawa 2005/08/02  V1.20                                           */
/*                          follow new perid(support USB 2.0 Hi-Speed)      */
/*   K.Takagi   2006/08/31  V1.22                                           */
/*                          Modified the max packet size of Interrutp Xfer. */
/*   K.Handa    2006/09/26  V1.23                                           */
/*                          version was updated                             */
/*   K.handa    2006/12/27  V1.30                                           */
/*                          version was updated                             */
/*   K.handa    2007/06/12  V1.31                                           */
/*                          version was updated                             */
/*   M.Suzuki   2015/03/04  V1.40                                           */
/*                          Modified some of the comments.                  */
/*                          Modified the default value of the index of      */
/*                          String Descriptor.                              */
/*                          - GRCOMD_CNF_MANUFACTURER                       */
/*                          - GRCOMD_CNF_PRODUCT                            */
/*                          - GRCOMD_CNF_SERIALNUMBER                       */
/*                          Modified the default value of the maximum bus   */
/*                          power consumption.                              */
/*                          - GRCOMD_CNF_MAXPOWER                           */
/*                                                                          */
/****************************************************************************/
#ifndef     _COMM_CNF_H_
#define     _COMM_CNF_H_

/**** INTERNAL DATA DEFINES *************************************************/
/*----- Device Descriptor informations -----*/
/* The release number of USB specification */
#define GRCOMD_CNF_USB_MSB                  (0x02)  /* bcdUSB               */
#define GRCOMD_CNF_USB_LSB                  (0x00)  /* Version 2.00         */
/* Vender ID */
#define GRCOMD_CNF_VID_MSB                  (0x24)  /* idVendor             */
#define GRCOMD_CNF_VID_LSB                  (0x75)  /* vendor ID 0x2475     */
#if 0
/* Product ID(USB:0) */
#define GRCOMD_CNF_PID_MSB                  (0x01)  /* idProduct            */
#define GRCOMD_CNF_PID_LSB                  (0x17)  /* product ID 0x0117    */
/* Product ID(USB:1) */
#define GRCOMD_CNF_PID_MSB2                 (0x01)  /* idProduct            */
#define GRCOMD_CNF_PID_LSB2                 (0x18)  /* product ID 0x0118    */
#endif
/* The release number of a device */
#define GRCOMD_CNF_DEV_MSB                  (0x01)  /* bcdDevice            */
#define GRCOMD_CNF_DEV_LSB                  (0x00)  /* version 1.00         */
/* Index of String Descriptor */
#define GRCOMD_CNF_MANUFACTURER             (0x01)  /* iManufactuer         */
#define GRCOMD_CNF_PRODUCT                  (0x02)  /* iProduct             */
#define GRCOMD_CNF_SERIALNUMBER             (0x03)  /* iSerialNumber        */

/*----- Configuration Descriptor informations -----*/
/* Configuration of attributes */
#define GRCOMD_CNF_ATTRIBUTES               (0xC0)      /* bmAttributes     */
/* Maximum bus power consumption */
#define GRCOMD_CNF_MAXPOWER                 (0x32)      /* bMaxPower        */

/*----- Endpoint Descriptor informations -----*/
/* Polling interval (for Interrupt In Endpoint) */
/* for Full Speed */
#define GRCOMD_CNF_INTERVAL_INTERRUPT_FS    (0x20)             /* bInterval */
/* for High Speed */
#define GRCOMD_CNF_INTERVAL_INTERRUPT_HS    (0x09)             /* bInterval */
/* Interrupt In Endpoint maxpacket size */
/* for Full Speed */
#define GRCOMD_CNF_INT_IN_MAXPKTSIZE_L_FS   (0x08)  /* wMaxPacketSize(Low-Byte)  */  /* V1.22 */
#define GRCOMD_CNF_INT_IN_MAXPKTSIZE_H_FS   (0x00)  /* wMaxPacketSize(High-Byte) */
/* for High Speed */
#define GRCOMD_CNF_INT_IN_MAXPKTSIZE_L_HS   (0x20)  /* wMaxPacketSize(Low-Byte)  */
#define GRCOMD_CNF_INT_IN_MAXPKTSIZE_H_HS   (0x00)  /* wMaxPacketSize(High-Byte) */
/* Bulk Out Endpoint maxpacket size */
/* for Full Speed */
#define GRCOMD_CNF_OUT_MAXPKTSIZE_L_FS      (0x40)  /* wMaxPacketSize(Low-Byte)  */
#define GRCOMD_CNF_OUT_MAXPKTSIZE_H_FS      (0x00)  /* wMaxPacketSize(High-Byte) */
/* for High Speed */
#define GRCOMD_CNF_OUT_MAXPKTSIZE_L_HS      (0x00)  /* wMaxPacketSize(Low-Byte)  */
#define GRCOMD_CNF_OUT_MAXPKTSIZE_H_HS      (0x02)  /* wMaxPacketSize(High-Byte) */
/* Bulk In Endpoint maxpacket size */
/* for Full Speed */
#define GRCOMD_CNF_IN_MAXPKTSIZE_L_FS       (0x40)  /* wMaxPacketSize(Low-Byte)  */
#define GRCOMD_CNF_IN_MAXPKTSIZE_H_FS       (0x00)  /* wMaxPacketSize(High-Byte) */
/* for High Speed */
#define GRCOMD_CNF_IN_MAXPKTSIZE_L_HS       (0x00)  /* wMaxPacketSize(Low-Byte)  */
#define GRCOMD_CNF_IN_MAXPKTSIZE_H_HS       (0x02)  /* wMaxPacketSize(High-Byte) */

/*----- Header Functional Descriptor informations -----*/
/* USB Class Definitions for Communication Devices Specification release number */
#define GRCOMD_CNF_HF_CDC_MSB           (0x01)      /* bcdCDC               */
#define GRCOMD_CNF_HF_CDC_LSB           (0x10)      /* version 1.10         */

/*----- Call Management Functional Descriptor informations -----*/
/* Capabilities that this configuration supports */
#define GRCOMD_CNF_CMF_CAPABILITIES     (0x03)      /* bmCapabilities       */
/* Interface number of Data Class interface */
#define GRCOMD_CNF_CMF_DATAIF           (0x01)      /* bDataInterface       */

/*----- Abstract Control Management Functional Descriptor informations -----*/
/* Capabilities that this configuration supports */
#define GRCOMD_CNF_ACMF_CAPABILITIES    (0x0F)      /* bmCapabilities       */

#endif  /* _COMM_CNF_H_ */
