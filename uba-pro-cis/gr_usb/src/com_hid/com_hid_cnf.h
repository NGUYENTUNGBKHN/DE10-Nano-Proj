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
/*      com_hid_cnf.h                                             1.00      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This file performs Abstract Control Model of HID Class              */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*   NAME       DATE        REMARKS                                         */
/*                                                                          */
/*   JCM-HQ     2022/06/21  V1.00                                           */
/*                          Created initial version                         */
/*                                                                          */
/****************************************************************************/
#ifndef     _COM_HID_CNF_H_
#define     _COM_HID_CNF_H_

/**** INTERNAL DATA DEFINES *************************************************/
/*----- Device Descriptor informations -----*/
/* The release number of USB specification */
#define GRCOMD_HID_CNF_USB_MSB                  (0x02)  /* bcdUSB               */
#define GRCOMD_HID_CNF_USB_LSB                  (0x00)  /* Version 2.00         */
/* Vender ID */
#define GRCOMD_HID_CNF_VID_MSB                  (0x24)  /* idVendor             */
#define GRCOMD_HID_CNF_VID_LSB                  (0x75)  /* vendor ID 0x2475     */
/* Product ID(USB:0) */
#define GRCOMD_HID_CNF_PID_MSB                  (0x01)  /* idProduct            */
#define GRCOMD_HID_CNF_PID_LSB                  (0x17)  /* product ID 0x0117    */

/* Product ID(USB:1) */
#if 1 //2022-09-21
#define GRCOMD_HID_CNF_PID_MSB2                 (0x01)  /* idProduct            */
#define GRCOMD_HID_CNF_PID_LSB2                 (0x19)  /* product ID 0x0119    */
#else
#define GRCOMD_HID_CNF_PID_MSB2                 (0x01)  /* idProduct            */
#define GRCOMD_HID_CNF_PID_LSB2                 (0x05)  /* product ID 0x0105    */
#endif


#if 0//2022-09-21 こっちは参照してないのでは #if 1  /* DFU */
#define GRCOMD_DFU_CNF_PID_MSB2                 (0x01)  /* idProduct            */
#define GRCOMD_DFU_CNF_PID_LSB2                 (0x09)  /* product ID 0x0105    */
#endif /* DFU */

/* The release number of a device */
#define GRCOMD_HID_CNF_DEV_MSB                  (0x01)  /* bcdDevice            */
#define GRCOMD_HID_CNF_DEV_LSB                  (0x00)  /* version 1.00         */
/* Index of String Descriptor */
#define GRCOMD_HID_CNF_MANUFACTURER             (0x01)  /* iManufactuer         */
#define GRCOMD_HID_CNF_PRODUCT                  (0x02)  /* iProduct             */
#define GRCOMD_HID_CNF_SERIALNUMBER             (0x03)  /* iSerialNumber        */
#define GRCOMD_HID_CNF_INTERFACE                (0x06)  /* iInterface           */

/*----- Configuration Descriptor informations -----*/
/* Configuration of attributes */
#define GRCOMD_HID_CNF_ATTRIBUTES               (0xC0)      /* bmAttributes     */
/* Maximum bus power consumption */
#define GRCOMD_HID_CNF_MAXPOWER                 (0x01)      /* bMaxPower        */

/*----- Endpoint Descriptor informations -----*/
/* Polling interval (for Interrupt In Endpoint) */
/* for Full Speed */
#define GRCOMD_HID_CNF_INTERVAL_INTERRUPT_FS    (0x0A)             /* bInterval */
/* for High Speed */
#define GRCOMD_HID_CNF_INTERVAL_INTERRUPT_HS    (0x09)             /* bInterval */
/* Interrupt In Endpoint maxpacket size */
/* for Full Speed */
#define GRCOMD_HID_CNF_INT_IN_MAXPKTSIZE_L_FS   (0x40)  /* wMaxPacketSize(Low-Byte)  */  /* V1.22 */
#define GRCOMD_HID_CNF_INT_IN_MAXPKTSIZE_H_FS   (0x00)  /* wMaxPacketSize(High-Byte) */
/* for High Speed */
#define GRCOMD_HID_CNF_INT_IN_MAXPKTSIZE_L_HS   (0x20)  /* wMaxPacketSize(Low-Byte)  */
#define GRCOMD_HID_CNF_INT_IN_MAXPKTSIZE_H_HS   (0x00)  /* wMaxPacketSize(High-Byte) */
/* Bulk Out Endpoint maxpacket size */
/* for Full Speed */
#define GRCOMD_HID_CNF_OUT_MAXPKTSIZE_L_FS      (0x40)  /* wMaxPacketSize(Low-Byte)  */
#define GRCOMD_HID_CNF_OUT_MAXPKTSIZE_H_FS      (0x00)  /* wMaxPacketSize(High-Byte) */
/* for High Speed */
#define GRCOMD_HID_CNF_OUT_MAXPKTSIZE_L_HS      (0x00)  /* wMaxPacketSize(Low-Byte)  */
#define GRCOMD_HID_CNF_OUT_MAXPKTSIZE_H_HS      (0x02)  /* wMaxPacketSize(High-Byte) */
/* Bulk In Endpoint maxpacket size */
/* for Full Speed */
#define GRCOMD_HID_CNF_IN_MAXPKTSIZE_L_FS       (0x40)  /* wMaxPacketSize(Low-Byte)  */
#define GRCOMD_HID_CNF_IN_MAXPKTSIZE_H_FS       (0x00)  /* wMaxPacketSize(High-Byte) */
/* for High Speed */
#define GRCOMD_HID_CNF_IN_MAXPKTSIZE_L_HS       (0x00)  /* wMaxPacketSize(Low-Byte)  */
#define GRCOMD_HID_CNF_IN_MAXPKTSIZE_H_HS       (0x02)  /* wMaxPacketSize(High-Byte) */

/*----- Header Functional Descriptor informations -----*/
/* USB Class Definitions for Communication Devices Specification release number */
#define GRCOMD_HID_CNF_HF_CDC_MSB           (0x01)      /* bcdCDC               */
#define GRCOMD_HID_CNF_HF_CDC_LSB           (0x10)      /* version 1.10         */

/*----- Call Management Functional Descriptor informations -----*/
/* Capabilities that this configuration supports */
#define GRCOMD_HID_CNF_CMF_CAPABILITIES     (0x03)      /* bmCapabilities       */
/* Interface number of Data Class interface */
#define GRCOMD_HID_CNF_CMF_DATAIF           (0x01)      /* bDataInterface       */

/*----- Abstract Control Management Functional Descriptor informations -----*/
/* Capabilities that this configuration supports */
#define GRCOMD_HID_CNF_ACMF_CAPABILITIES    (0x0F)      /* bmCapabilities       */

#endif  /* _COM_HID_CNF_H_ */
