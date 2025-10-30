/****************************************************************************/
/*                                                                          */
/*               Copyright(C) 2003-2019 Grape Systems, Inc.                 */
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
/*      com_dfu_def.h                                             1.00      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This file performs Abstract Control Model of DFU Class              */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*   NAME       DATE        REMARKS                                         */
/*                                                                          */
/*   JCM-HQ     2022/06/21  V1.00                                           */
/*                          Created initial version                         */
/*                                                                          */
/****************************************************************************/
#ifndef     _COM_DFU_DEF_H_
#define     _COM_DFU_DEF_H_

/**** INCLUDE FILES *********************************************************/
#include    "grusbtyp.h"
#include    "perid.h"
#include    "com_dfu_cnf.h"

/**** INTERNAL DATA DEFINES *************************************************/
/* Transfer completion status option */
/* [Remarks]                                                           */
/*   Enable this definition when using the transfer completion status. */
/*#define GRCOMD_COMP_STATUS_USE*/

/* 1.30 Endian Setting */
#define GRCOMD_DFU_LITTLEENDIAN                 (0x00)
#define GRCOMD_DFU_BIGENDIAN                    (0x01)
#define GRCOMD_DFU_ENDIAN                       GRCOMD_DFU_LITTLEENDIAN

/* Endpoint Setting */
#define GRCOMD_DFU_BULKIN                       (0x00)
#define GRCOMD_DFU_BULKOUT                      (0x01)
//#define GRCOMD_DFU_EP2_TYPE                     GRCOMD_DFU_BULKIN
#define GRCOMD_DFU_EP2_TYPE                     GRCOMD_DFU_BULKOUT

#define GRCOMD_DFU_MAC_SWAP16(x, y)             ((UINT16) ( (((UINT16)(x)<<8) & 0xFF00) \
                                                      | (((UINT16)(y))    & 0x00FF) ) )
#if (GRCOMD_DFU_ENDIAN == GRCOMD_DFU_BIGENDIAN)
#define GRCOMD_DFU_MAC_SWAPUS(x)                ((UINT16) ( (((UINT16)(x)<<8) & 0xFF00) \
                                                      | (((UINT16)(x)>>8) & 0x00FF) ))
#else
#define GRCOMD_DFU_MAC_SWAPUS(x)                (x)
#endif

/* Interface number */
#define GRCOMD_DFU_MANAGEMENT_IF                (0x00)              /* Request Interface number         */
#define GRCOMD_DFU_NOTIFICATION_IF              (0x00)              /* Notification Interface number    */

/* Abstract Model Request send data size */
/* GET_COMM_FEATURE */
#define GRCOMD_DFU_FEATURE_STATUS_SIZE          (0x02)              /* Feature Status data size         */
/* GET_LINE_CODING */
#define GRCOMD_DFU_LINE_CODING_SIZE             (0x07)              /* Line Coding Structure data size  */

/* DFU Request */
#define GRUSB_DFU_DETACH                        (0x00)              /*             (use)                */
#define GRUSB_DFU_DNLOAD                        (0x01)              /*             (not use)            */
#define GRUSB_DFU_UPLOAD                        (0x02)              /*             (not use)            */
#define GRUSB_DFU_GETSTATUS                     (0x03)              /*             (not use)            */
#define GRUSB_DFU_CLRSTATUS                     (0x04)              /*             (not use)            */
#define GRUSB_DFU_GETSTATE                      (0x05)              /*             (not use)            */
#define GRUSB_DFU_ABORT                         (0x06)              /*             (use)                */
//#define GRUSB_DFU_SET_IDLE                      (0x0A)              /* SET_IDLE    (use)                */
//#define GRUSB_DFU_SET_PROTOCOL                  (0x0B)              /*             (not use)            */

/* GRUSB_DFU_GETSTATUS */
#define GRCOMD_DFU_GETSTATUS_SIZE               (0x06)              /* Get Status data size             */

/* GRUSB_DFU_GETSTATE */
#define GRCOMD_DFU_GETSTATE_SIZE                (0x01)              /* Get State data size              */

/* Abstract Model Request code */
#define GRCOMD_DFU_SEND_ENCAPSULATED_COMMAND    (0x00)
#define GRCOMD_DFU_GET_ENCAPSULATED_RESPONSE    (0x01)
#define GRCOMD_DFU_SET_COMM_FEATURE             (0x02)
#define GRCOMD_DFU_GET_COMM_FEATURE             (0x03)
#define GRCOMD_DFU_CLEAR_COMM_FEATURE           (0x04)
#define GRCOMD_DFU_SET_LINE_CODING              (0x20)
#define GRCOMD_DFU_GET_LINE_CODING              (0x21)
#define GRCOMD_DFU_SET_CONTROL_LINE_STATE       (0x22)
#define GRCOMD_DFU_SEND_BREAK                   (0x23)

/* Abstract Model Notification parameter */
/* NETWORK_CONNECTION */
#define GRCOMD_DFU_NETWORK_CONNECTION_LENGTH    (0x00)
/* RESPONSE_AVAILABLE */
#define GRCOMD_DFU_RESPONSE_AVAILABLE_VALUE     (0x00)
#define GRCOMD_DFU_RESPONSE_AVAILABLE_LENGTH    (0x00)
/* SERIAL_STATE */
#define GRCOMD_DFU_SERIAL_STATE_VALUE           (0x00)
#define GRCOMD_DFU_SERIAL_STATE_LENGTH          (0x02)

/* Abstract Model Notification code */
#define GRCOMD_DFU_NETWORK_CONNECTION           (0x00)
#define GRCOMD_DFU_RESPONSE_AVAILABLE           (0x01)
#define GRCOMD_DFU_SERIAL_STATE                 (0x20)

/* Abstract Model Feature Selector code */
#define GRCOMD_DFU_ABSTRACT_STATE               (0x01)

/* Discriptor Type */
#define GRCOMD_DFU_DESCTYPE_CS_INTERFACE        (0x24)                                  /* CS_INTERFACE */

/* Descriptor SubType */
#define GRCOMD_DFU_DECSSUBTYPE_HFDESC           (0x00)      /* Header Functional Descriptor             */
#define GRCOMD_DFU_DECSSUBTYPE_CMFDESC          (0x01)      /* Call Management Functional Descriptor    */
#define GRCOMD_DFU_DESCSUBTYPE_ACMFDESC         (0x02)      /* Abstract Control Management Descriptor   */
#define GRCOMD_DFU_DESCSUBTYPE_UFDESC           (0x06)      /* Union Functional Descriptor              */

/* The bmCapabilities bitmap value of Abstract Control Management Functional Descriptor */
#define GRCOMD_DFU_BM_CAPA_NETWORK              (0x08)          /* NetworkConnection is supported       */
#define GRCOMD_DFU_BM_CAPA_BREAK                (0x04)          /* SendBreak is supported               */
#define GRCOMD_DFU_BM_CAPA_LINE_SERIAL          (0x02)          /* xx_Line_xx,SeriaState is supported   */
#define GRCOMD_DFU_BM_CAPA_FEATURE              (0x01)          /* xx_CommFeature is supported          */

/* The bmCapabilities bitmap value of Call Management Functional Descriptor */
#define GRCOMD_DFU_BM_CAPA_DATA_CLASS           (0x02)          /* Data Class Interface is supported    */
#define GRCOMD_DFU_BM_CAPA_CALL_MANAGEMENT      (0x01)          /* Call Management itself is supported  */

/* Initial setting value */
#define GRCOMD_DFU_BOUNDERY                     (0x04)          /* Definition for Boundary              */
#define GRCOMD_DFU_MAX_DT_SZ                    (0x20)          /* Definition for Boundary              */
#define GRCOMD_DFU_NUM_CONFIGURATIONS           (0x01)          /* The Configuration number to define   */
#define GRCOMD_DFU_CTRLAREASIZE                 (0x0100)        /* Management Buffer Size               */
#define GRCOMD_DFU_STRING_DESC_NUM              (0x06)          /* Number of String Descriptor          */
#define GRCOMD_DFU_EP0_MAXPKTSIZE_FS            (0x40)          /* EP0 Max Packet Size (FS)             */
#define GRCOMD_DFU_EP0_MAXPKTSIZE_HS            (0x40)          /* EP0 Max Packet Size (HS)             */
#define GRCOMD_DFU_EP3_MAXPKTSIZE_FS            GRCOMD_DFU_MAC_SWAP16(GRCOMD_DFU_CNF_INT_IN_MAXPKTSIZE_H_FS, \
                                                                      GRCOMD_DFU_CNF_INT_IN_MAXPKTSIZE_L_FS)                /* V1.22 */
#define GRCOMD_DFU_EP3_MAXPKTSIZE_HS            GRCOMD_DFU_MAC_SWAP16(GRCOMD_DFU_CNF_INT_IN_MAXPKTSIZE_H_HS, \
                                                                      GRCOMD_DFU_CNF_INT_IN_MAXPKTSIZE_L_HS)                /* V1.22 */
                                                            /* EP1 Max Packet Size                  */
#define GRCOMD_DFU_EP0_PAGENUM                  (0x01)          /* FIFO Size(1 Page)                    */
#define GRCOMD_DFU_EP1_PAGENUM                  (0x01)          /* FIFO Size(1 Page)                    */
#define GRCOMD_DFU_EP2_PAGENUM                  (0x01)          /* FIFO Size(1 Page)                    */
#define GRCOMD_DFU_EP3_PAGENUM                  (0x01)          /* FIFO Size(1 Page)                    */
#define GRCOMD_DFU_CTRL_BUF                     (0x01)          /* The number of Control transfer buffer*/
#define GRCOMD_DFU_EP1_DATABUF                  (0x01)          /* The number of Data transfer buffer   */
#define GRCOMD_DFU_EP2_DATABUF                  (0x01)          /* The number of Data transfer buffer   */
#define GRCOMD_DFU_EP3_DATABUF                  (0x01)          /* The number of Data transfer buffer   */

/* Endpoint number */
#define GRCOMD_DFU_EP0                          (0x00)                                  /* Endpoint0    */
#define GRCOMD_DFU_EP1                          (0x01)                                  /* Endpoint1    */
#define GRCOMD_DFU_EP2                          (0x02)                                  /* Endpoint2    */
#define GRCOMD_DFU_EP3                          (0x03)                                  /* Endpoint3    */

/* Device Descriptor */
#define GRCOMD_DFU_DEVDESC_LENGTH               (0x12)                          /* bLength              */
#define GRCOMD_DFU_DEVDESC_DESCRIPTORTYPE       (0x01)                          /* bDescriptorType      */
#define GRCOMD_DFU_DEVDESC_DEVICECLASS          (0x00)                          /* bDeviceClass         */
#define GRCOMD_DFU_DEVDESC_DEVICESUBCLASS       (0x00)                          /* bDeviceSubClass      */
#define GRCOMD_DFU_DEVDESC_DEVICEPROTOCOL       (0x00)                          /* bDeviceProtocol      */
#define GRCOMD_DFU_DEVDESC_MAXPKTSIZE0_FS       GRCOMD_DFU_EP0_MAXPKTSIZE_FS    /* wMaxPacketSize0 (FS) */
#define GRCOMD_DFU_DEVDESC_MAXPKTSIZE0_HS       GRCOMD_DFU_EP0_MAXPKTSIZE_HS    /* wMaxPacketSize0 (HS) */
#define GRCOMD_DFU_DEVDESC_NUMCONFIGURATIONS    (0x01)                          /* bNumConfigurations   */

/* Configuration Descriptor */
#define GRCOMD_DFU_CFGDESC_LENGTH               (0x09)                      /* bLength                  */
#define GRCOMD_DFU_CFGDESC_DESCRIPTORTYPE       (0x02)                      /* bDescriptorType          */
#define GRCOMD_DFU_CFGDESC_CONFIGURATIONVALUE   (0x01)                      /* bConfigurationValue      */
#define GRCOMD_DFU_CFGDESC_CONFIGURATION        (0x04)                      /* iConfiguration           */

/* When D1 bit of bmCapabilities of Call Management Functional Descriptor is 1 */
#if( ( GRCOMD_DFU_CNF_CMF_CAPABILITIES & GRCOMD_DFU_BM_CAPA_DATA_CLASS ) == GRCOMD_DFU_BM_CAPA_DATA_CLASS )
    #define GRCOMD_DFU_CFGDESC_NUMINTERFACES    (0x02)                      /* bNumInterfaces           */
#else
    #define GRCOMD_DFU_CFGDESC_NUMINTERFACES    (0x01)                      /* bNumInterfaces           */
#endif

/* Interface Descriptor */
#define GRCOMD_DFU_IFDESC_LENGTH                (0x09)                          /* bLength              */
#define GRCOMD_DFU_IFDESC_DESCRIPTORTYPE        (0x04)                          /* bDescriptorType      */
#define GRCOMD_DFU_IFDESC_ALTERNATESETTING      (0x00)                          /* bAlternateSetting    */
#define GRCOMD_DFU_IFDESC_INTERFACEPROTOCOL     (0x00)                          /* bInterfaceProtocol   */
#define GRCOMD_DFU_IFDESC_INTERFACE             (0x00)                          /* iInterface           */
/* Interface Descriptor 0 specific */
#define GRCOMD_DFU_IFDESC0_INTERFACENUMBER      (0x00)                          /* bInterfaceNumber     */
#define GRCOMD_DFU_IFDESC0_NUMENDPOINTS         (0x00)                          /* bNumEndpoints        */
#define GRCOMD_DFU_IFDESC0_INTERFACECLASS       (0xFE)                          /* bInterfaceClass      */
#define GRCOMD_DFU_IFDESC0_INTERFACESUBCLASS    (0x01)                          /* bInterfaceSubClass   */
#define GRCOMD_DFU_IFDESC0_INTERFACEPROTOCOL    (0x01)                          /* bInterfaceProtocol   */
#define GRCOMD_DFU_IFDESC0_INTERFACE            (0x05)                          /* iInterface           */
                                                                                /* 0x02 is Abstract Control Model   */
/* Interface Descriptor 1 specific */
#define GRCOMD_DFU_IFDESC1_INTERFACENUMBER      (0x01)                          /* bInterfaceNumber     */
#define GRCOMD_DFU_IFDESC1_NUMENDPOINTS         (0x01)                          /* bNumEndpoints        */
#define GRCOMD_DFU_IFDESC1_INTERFACECLASS       (0x03)                          /* bInterfaceClass      */
#define GRCOMD_DFU_IFDESC1_INTERFACESUBCLASS    (0x00)                          /* bInterfaceSubClass   */
#define GRCOMD_DFU_IFDESC1_INTERFACE            (0x06)                          /* iInterface           */

/* Endpoint Descriptor */
#define GRCOMD_DFU_EPDESC_LENGTH                (0x07)                      /* bLength                  */
#define GRCOMD_DFU_EPDESC_DESCRIPTORTYPE        (0x05)                      /* bDescriptorType          */
#define GRCOMD_DFU_EPDESC_ATTRIBUTES_BULK       (0x02)                      /* bmAttributes(Bulk)       */
#define GRCOMD_DFU_EPDESC_ATTRIBUTES_INT        (0x03)                      /* bmAttributes(Interrupt)  */
#define GRCOMD_DFU_EPDESC_INTERVAL_BULK         (0x00)                      /* bInterval                */
/* Endpoint3(Interrupt In) Address */
#define GRCOMD_DFU_EPDESC_EP1_ADDRESS           (0x81)                              /* bEndpointAddress */          /* V1.22 */
/* Endpoint3(Interrupt In) maxpacket size */
#define GRCOMD_DFU_EPDESC_EP1_MAXPKTSIZE_L_FS       GRCOMD_DFU_CNF_INT_IN_MAXPKTSIZE_L_FS
                                                                        /* wMaxPacketSize(Low-Byte) */          /* V1.22 */
#define GRCOMD_DFU_EPDESC_EP1_MAXPKTSIZE_H_FS       GRCOMD_DFU_CNF_INT_IN_MAXPKTSIZE_H_FS
                                                                        /* wMaxPacketSize(High-Byte)*/          /* V1.22 */
#define GRCOMD_DFU_EPDESC_EP1_MAXPKTSIZE_L_HS       GRCOMD_DFU_CNF_INT_IN_MAXPKTSIZE_L_HS
                                                                        /* wMaxPacketSize(Low-Byte) */          /* V1.22 */
#define GRCOMD_DFU_EPDESC_EP1_MAXPKTSIZE_H_HS       GRCOMD_DFU_CNF_INT_IN_MAXPKTSIZE_H_HS
                                                                        /* wMaxPacketSize(High-Byte)*/          /* V1.22 */

#if( GRCOMD_DFU_EP2_TYPE == GRCOMD_DFU_BULKIN )
    #define GRCOMD_DFU_EP2_MAXPKTSIZE_FS        GRCOMD_DFU_MAC_SWAP16(GRCOMD_DFU_CNF_IN_MAXPKTSIZE_H_FS, \
                                                                      GRCOMD_DFU_CNF_IN_MAXPKTSIZE_L_FS)
                                                                            /* EP2 Max Packet Size  */
    #define GRCOMD_DFU_EP1_MAXPKTSIZE_FS        GRCOMD_DFU_MAC_SWAP16(GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_H_FS, \
                                                                      GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_L_FS)
                                                                            /* EP1 Max Packet Size  */          /* V1.22 */
    #define GRCOMD_DFU_EP2_MAXPKTSIZE_HS        GRCOMD_DFU_MAC_SWAP16(GRCOMD_DFU_CNF_IN_MAXPKTSIZE_H_HS, \
                                                                      GRCOMD_DFU_CNF_IN_MAXPKTSIZE_L_HS)
                                                                            /* EP2 Max Packet Size  */
    #define GRCOMD_DFU_EP1_MAXPKTSIZE_HS        GRCOMD_DFU_MAC_SWAP16(GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_H_HS, \
                                                                      GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_L_HS)
                                                                            /* EP1 Max Packet Size  */          /* V1.22 */
    #define GRCOMD_DFU_EP2_EPTYPE               EPTYPE_DFU_IN                       /* EP2 Type             */
    #define GRCOMD_DFU_EP1_EPTYPE               EPTYPE_DFU_OUT                      /* EP1 Type             */          /* V1.22 */

    #define GRCOMD_DFU_BULKIN_EP_NUMBER         GRCOMD_DFU_EP2                      /* BULK-In EP Number    */
    #define GRCOMD_DFU_BULKOUT_EP_NUMBER        GRCOMD_DFU_EP1                      /* BULK-Out EP Number   */          /* V1.22 */

    /* Endpoint2(Bulk IN) Address */
    #define GRCOMD_DFU_EPDESC_EP2_ADDRESS       (0x82)                              /* bEndpointAddress */
    /* Endpoint1(Bulk OUT) Address */
    #define GRCOMD_DFU_EPDESC_EP1_ADDRESS       (0x01)                              /* bEndpointAddress */          /* V1.22 */
    /* Endpoint2(Bulk In) maxpacket size */
    #define GRCOMD_DFU_EPDESC_EP2_MAXPKTSIZE_L_FS   GRCOMD_DFU_CNF_IN_MAXPKTSIZE_L_FS   /* wMaxPacketSize(Low-Byte) */
    #define GRCOMD_DFU_EPDESC_EP2_MAXPKTSIZE_H_FS   GRCOMD_DFU_CNF_IN_MAXPKTSIZE_H_FS   /* wMaxPacketSize(High-Byte)*/
    #define GRCOMD_DFU_EPDESC_EP2_MAXPKTSIZE_L_HS   GRCOMD_DFU_CNF_IN_MAXPKTSIZE_L_HS   /* wMaxPacketSize(Low-Byte) */
    #define GRCOMD_DFU_EPDESC_EP2_MAXPKTSIZE_H_HS   GRCOMD_DFU_CNF_IN_MAXPKTSIZE_H_HS   /* wMaxPacketSize(High-Byte)*/
    /* Endpoint1(Bulk Out) maxpacket size */
    #define GRCOMD_DFU_EPDESC_EP1_MAXPKTSIZE_L_FS   GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_L_FS  /* wMaxPacketSize(Low-Byte) */  /* V1.22 */
    #define GRCOMD_DFU_EPDESC_EP1_MAXPKTSIZE_H_FS   GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_H_FS  /* wMaxPacketSize(High-Byte)*/  /* V1.22 */
    #define GRCOMD_DFU_EPDESC_EP1_MAXPKTSIZE_L_HS   GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_L_HS  /* wMaxPacketSize(Low-Byte) */  /* V1.22 */
    #define GRCOMD_DFU_EPDESC_EP1_MAXPKTSIZE_H_HS   GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_H_HS  /* wMaxPacketSize(High-Byte)*/  /* V1.22 */

#else
    #define GRCOMD_DFU_EP2_MAXPKTSIZE_FS        GRCOMD_DFU_MAC_SWAP16(GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_H_FS, \
                                                                      GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_L_FS)
                                                                            /* EP2 Max Packet Size  */
    #define GRCOMD_DFU_EP1_MAXPKTSIZE_FS        GRCOMD_DFU_MAC_SWAP16(GRCOMD_DFU_CNF_IN_MAXPKTSIZE_H_FS, \
                                                                      GRCOMD_DFU_CNF_IN_MAXPKTSIZE_L_FS)
                                                                            /* EP1 Max Packet Size  */          /* V1.22 */
    #define GRCOMD_DFU_EP2_MAXPKTSIZE_HS        GRCOMD_DFU_MAC_SWAP16(GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_H_HS, \
                                                                      GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_L_HS)
                                                                            /* EP2 Max Packet Size  */
    #define GRCOMD_DFU_EP1_MAXPKTSIZE_HS        GRCOMD_DFU_MAC_SWAP16(GRCOMD_DFU_CNF_IN_MAXPKTSIZE_H_HS, \
                                                                      GRCOMD_DFU_CNF_IN_MAXPKTSIZE_L_HS)
                                                                            /* EP1 Max Packet Size  */          /* V1.22 */
    #define GRCOMD_DFU_EP2_EPTYPE               EPTYPE_DFU_OUT                      /* EP2 Type             */
    #define GRCOMD_DFU_EP1_EPTYPE               EPTYPE_DFU_IN                       /* EP1 Type             */          /* V1.22 */

    #define GRCOMD_DFU_BULKIN_EP_NUMBER         GRCOMD_DFU_EP1                      /* BULK-In EP Number    */          /* V1.22 */
    #define GRCOMD_DFU_BULKOUT_EP_NUMBER        GRCOMD_DFU_EP2                      /* BULK-Out EP Number   */

    /* Endpoint2(Bulk OUT) Address */
    #define GRCOMD_DFU_EPDESC_EP2_ADDRESS       (0x02)                              /* bEndpointAddress */
    /* Endpoint1(Bulk IN) Address */
    #define GRCOMD_DFU_EPDESC_EP1_ADDRESS       (0x81)                              /* bEndpointAddress */          /* V1.22 */
    /* Endpoint2(Bulk Out) maxpacket size */
    #define GRCOMD_DFU_EPDESC_EP2_MAXPKTSIZE_L_FS   GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_L_FS  /* wMaxPacketSize(Low-Byte) */
    #define GRCOMD_DFU_EPDESC_EP2_MAXPKTSIZE_H_FS   GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_H_FS  /* wMaxPacketSize(High-Byte)*/
    #define GRCOMD_DFU_EPDESC_EP2_MAXPKTSIZE_L_HS   GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_L_HS  /* wMaxPacketSize(Low-Byte) */
    #define GRCOMD_DFU_EPDESC_EP2_MAXPKTSIZE_H_HS   GRCOMD_DFU_CNF_OUT_MAXPKTSIZE_H_HS  /* wMaxPacketSize(High-Byte)*/
    /* Endpoint1(Bulk In) maxpacket size */
    #define GRCOMD_DFU_EPDESC_EP1_MAXPKTSIZE_L_FS   GRCOMD_DFU_CNF_IN_MAXPKTSIZE_L_FS   /* wMaxPacketSize(Low-Byte) */  /* V1.22 */
    #define GRCOMD_DFU_EPDESC_EP1_MAXPKTSIZE_H_FS   GRCOMD_DFU_CNF_IN_MAXPKTSIZE_H_FS   /* wMaxPacketSize(High-Byte)*/  /* V1.22 */
    #define GRCOMD_DFU_EPDESC_EP1_MAXPKTSIZE_L_HS   GRCOMD_DFU_CNF_IN_MAXPKTSIZE_L_HS   /* wMaxPacketSize(Low-Byte) */  /* V1.22 */
    #define GRCOMD_DFU_EPDESC_EP1_MAXPKTSIZE_H_HS   GRCOMD_DFU_CNF_IN_MAXPKTSIZE_H_HS   /* wMaxPacketSize(High-Byte)*/  /* V1.22 */

#endif

/* When Interface number is 1 */
#if( GRCOMD_DFU_CFGDESC_NUMINTERFACES == 0x01 )
    #define GRCOMD_DFU_CFGDESC_TOTALLENGTH_H    (0x00)                  /* bTotalLength(High-Byte)      */
    #define GRCOMD_DFU_CFGDESC_TOTALLENGTH_L    (0x34)                  /* bTotalLength(Low-Byte)       */

    #define GRCOMD_DFU_NUM_ENDPOINTS            (0x01)                  /* Number of useing Endpoints   */
/* When Interface number is not 1 */
#else
    #define GRCOMD_DFU_CFGDESC_TOTALLENGTH_H    (0x00)                  /* bTotalLength(High-Byte)      */
    #define GRCOMD_DFU_CFGDESC_TOTALLENGTH_L    (0x34)                  /* bTotalLength(Low-Byte)       */

    #define GRCOMD_DFU_NUM_ENDPOINTS            (0x01)                  /* Number of useing Endpoints   */
#endif
#if 1  /* DFU */
#define GRCOMD_DFU_CFGDESC_TOTALLENGTH_H        (0x00)                  /* bTotalLength(High-Byte)      */
#define GRCOMD_DFU_CFGDESC_TOTALLENGTH_L        (0x1B)                  /* bTotalLength(Low-Byte)       */

#define GRCOMD_DFU_CFGDESC_NUMINTERFACES        (0x01)                  /* bNumInterfaces           */

#define GRCOMD_DFU_IFDESC0_INTERFACEPROTOCOL    (0x02)                  /* bInterfaceProtocol   */
#endif /* DFU */

/* Header Functional Descriptor */
#define GRCOMD_DFU_HFDESC_FUNCLENGTH            (0x05)                           /* bFunctionLength      */
#define GRCOMD_DFU_HFDESC_DESCTYPE              GRCOMD_DFU_DESCTYPE_CS_INTERFACE /* bDescriptorType      */
#define GRCOMD_DFU_HFDESC_DESCSUBTYPE           GRCOMD_DFU_DECSSUBTYPE_HFDESC    /* bDescriptorSubtype   */

/* Call Management Functional Descriptor */
#define GRCOMD_DFU_CMFDESC_FUNCLENGTH           (0x05)                           /* bFunctionLength      */
#define GRCOMD_DFU_CMFDESC_DESCTYPE             GRCOMD_DFU_DESCTYPE_CS_INTERFACE /* bDescriptorType      */
#define GRCOMD_DFU_CMFDESC_DESCSUBTYPE          GRCOMD_DFU_DECSSUBTYPE_CMFDESC   /* bDescriptorSubtype   */

/* Abstract Control Management Functional Descriptor */
#define GRCOMD_DFU_ACMFDESC_FUNCLENGTH          (0x04)                           /* bFunctionLength      */
#define GRCOMD_DFU_ACMFDESC_DESCTYPE            GRCOMD_DFU_DESCTYPE_CS_INTERFACE /* bDescriptorType      */
#define GRCOMD_DFU_ACMFDESC_DESCSUBTYPE         GRCOMD_DFU_DESCSUBTYPE_ACMFDESC  /* bDescriptorSubtype   */

/* Union Functional Descriptor */
#define GRCOMD_DFU_UFDESC_FUNCLENGTH            (0x05)                           /* bFunctionLength      */
#define GRCOMD_DFU_UFDESC_DESCTYPE              GRCOMD_DFU_DESCTYPE_CS_INTERFACE /* bDescriptorType      */
#define GRCOMD_DFU_UFDESC_DESCSUBTYPE           GRCOMD_DFU_DESCSUBTYPE_UFDESC    /* bDescriptorSubtype   */
#define GRCOMD_DFU_UFDESC_MASTERIF              (0x00)                           /* bMasterInterface     */
#define GRCOMD_DFU_UFDESC_SLAVEIF0              (0x01)                           /* bSlaveInterface0     */

/* DFU Functional Descriptor */
#define GRCOMD_DFU_DFUFDESC_LENGTH              (0x09)                           /* bFunctionLength       */
#define GRCOMD_DFU_DFUFDESC_DESCRIPTORTYPE      (0x21)                           /* bDescriptorType       */
#define GRCOMD_DFU_DFUFDESC_ATTRIBUTES          (0x03)                           /* bmAttributes          */
#define GRCOMD_DFU_DFUFDESC_DETACHTIMEOUT_L     (0x10)                           /* wDetachTimeOut_lo     */
#define GRCOMD_DFU_DFUFDESC_DETACHTIMEOUT_H     (0x27)                           /* wDetachTimeOut_hi     */
#define GRCOMD_DFU_DFUFDESC_TRANFERSIZE_L       (0x00)                           /* wMaxTranferSize_lo    */
#define GRCOMD_DFU_DFUFDESC_TRANFERSIZE_H       (0x10)                           /* wMaxTranferSize_hi    */
#define GRCOMD_DFU_DFUFDESC_DFUVERSION_L        (0x10)                           /* bcdDFUVersion_lo      */
#define GRCOMD_DFU_DFUFDESC_DFUVERSION_H        (0x01)                           /* bcdDFUVersion_hi      */


/* Number of characters for DFU I/F */
#define GRCOMD_DFU_IF_CHARS                     (126)


/* Structure of Notification Request */
typedef struct GRCOMD_DFU_NOTIFICATION
{
    UINT8   bmRequestType;              /* Request Type      */
    UINT8   bNotification;              /* Notification Code */
    UINT16  wValue;                     /* Value             */
    UINT16  wIndex;                     /* Index             */
    UINT16  wLength;                    /* Data Phase Length */
} GRCOMD_DFU_NOTIFICATION;
#define GRCOMD_DFU_NOTIFICATION_SIZE        sizeof(GRCOMD_DFU_NOTIFICATION)

/* Structure of The data of a Notification Request is included */
typedef struct GRCOMD_DFU_NOTIFICATION_SERIAL_STATE
{
    GRCOMD_DFU_NOTIFICATION     tNotice;    /* Notification Request structure */
    UINT16                  usState;    /* State */
} GRCOMD_DFU_NOTIFICATION_SERIAL_STATE;
#define GRCOMD_DFU_NOTIFICATION_SERIAL_STATE_SIZE       sizeof(GRCOMD_DFU_NOTIFICATION_SERIAL_STATE)

/* Structure of DUF Functional Descriptor */
typedef struct grusb_dev_dfu_desc_tag
{
    UINT8       uc_bLength;                     /* bLength              */
    UINT8       uc_bDescriptorType;             /* bDescriptorType      */
    UINT8       uc_bmAttributes;                /* bmAttributes         */
    UINT8       uc_wDetachTimeOut_lo;           /* wDetachTimeOut_lo    */
    UINT8       uc_wDetachTimeOut_hi;           /* wDetachTimeOut_hi    */
    UINT8       uc_wMaxTranferSize_lo;          /* wMaxTranferSize_lo   */
    UINT8       uc_wMaxTranferSize_hi;          /* bInterfaceSubClass   */
    UINT8       uc_bcdDFUVersion_lo;            /* bcdDFUVersion_lo     */
    UINT8       uc_bcdDFUVersion_hi;            /* bcdDFUVersion_hi     */
} GRUSB_DFU_DEV_DFU_DESC;

/* Structure of Header Functional Descriptor */
typedef struct header_func_desc_tag {
    UINT8   bFunctionLength;
    UINT8   bDescriptorType;
    UINT8   bDescriptorSubtype;
    UINT8   bcdCDC1;
    UINT8   bcdCDC2;
} GRCOMD_DFU_HEADER_DESC;

/* Structure of Call Management Functional Descriptor */
typedef struct call_func_desc_tag {
    UINT8   bFunctionLength;
    UINT8   bDescriptorType;
    UINT8   bDescriptorSubtype;
    UINT8   bmCapabilities;
    UINT8   bDataInterface;
} GRCOMD_DFU_CALL_DESC;

/* Structure of Abstract Control Management Functional Descriptor */
typedef struct abstract_func_desc_tag {
    UINT8   bFunctionLength;
    UINT8   bDescriptorType;
    UINT8   bDescriptorSubtype;
    UINT8   bmCapabilities;
} GRCOMD_DFU_ABSTRACT_DESC;

/* Structure of Union Functional Descriptor */
typedef struct union_func_desc_tag {
    UINT8   bFunctionLength;
    UINT8   bDescriptorType;
    UINT8   bDescriptorSubtype;
    UINT8   bMasterInterface;
    UINT8   bSlaveInterface[1];
} GRCOMD_DFU_UNION_DESC;

/* Structure of Descriptor used by Communication Class */
typedef struct com_class_desc_tag {
    GRUSB_DEV_CONFIG_DESC       tConfig;
    GRUSB_DEV_INTERFACE_DESC    tInterface0;
    GRCOMD_DFU_HEADER_DESC          tHeader0;
    GRCOMD_DFU_ABSTRACT_DESC        tAbstract;
    GRCOMD_DFU_UNION_DESC           tUnion;
    GRCOMD_DFU_CALL_DESC            tCall;
    GRUSB_DEV_ENDPOINT_DESC     tEndPoint1;
/* If Interface number is not 1, the following data is also used */
#if( GRCOMD_DFU_CFGDESC_NUMINTERFACES != 0x01 )
    GRUSB_DEV_INTERFACE_DESC    tInterface1;
    GRCOMD_DFU_HEADER_DESC          tHeader1;
    GRUSB_DEV_ENDPOINT_DESC     tEndPoint2;
    GRUSB_DEV_ENDPOINT_DESC     tEndPoint3;
#endif
} GRCOMD_DFU_CLASS_DESC;

/**** INTERNAL FUNCTION PROTOTYPES ******************************************/
EXTERN GRUSB_DEV_STRING_DESC*   GRUSB_COMD_DFU_GetStrDesc( VOID );

EXTERN GRUSB_DEV_STRING_DESC*   GRUSB_COMD_DFU_GetStrDesc2( VOID );

#endif  /* _COM_HID_DEF_H_ */
