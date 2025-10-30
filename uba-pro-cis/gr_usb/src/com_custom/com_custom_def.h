/****************************************************************************/
/*                                                                          */
/*                   Copyright(C) 2021 Grape Systems, Inc.                  */
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
/*      com_custom_def.h                                          1.00      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This file performs Abstract Control Model of Communication Device   */
/*      Class. (Custom version with multiple COM ports.)                    */
/*      Based on GR-USB/DEVICE Communication Function Driver V1.42.         */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*   NAME        DATE        REMARKS                                        */
/*                                                                          */
/*   T.Yamaguchi 2021/09/08  V1.00                                          */
/*                           Created initial version                        */
/*                                                                          */
/****************************************************************************/
#ifndef     _COM_CUSTOM_DEF_H_
#define     _COM_CUSTOM_DEF_H_

/**** INCLUDE FILES *********************************************************/
#include    "grusbtyp.h"
#include    "perid.h"
#include    "com_custom_cnf.h"

/**** INTERNAL DATA DEFINES *************************************************/
/* Transfer completion status option */
/* [Remarks]                                                           */
/*   Enable this definition when using the transfer completion status. */
/*#define GRCOMD_CUSTOM_COMP_STATUS_USE*/

#define GRCOMD_CUSTOM_DCOM_MAX                     (2)

/* 1.30 Endian Setting */
#define GRCOMD_CUSTOM_LITTLEENDIAN                 (0x00)
#define GRCOMD_CUSTOM_BIGENDIAN                    (0x01)
#define GRCOMD_CUSTOM_ENDIAN                       GRCOMD_CUSTOM_LITTLEENDIAN

/* Endpoint Setting */
#define GRCOMD_CUSTOM_BULKIN                       (0x00)
#define GRCOMD_CUSTOM_BULKOUT                      (0x01)
#define GRCOMD_CUSTOM_EP2_TYPE                     GRCOMD_CUSTOM_BULKIN
//#define GRCOMD_CUSTOM_EP2_TYPE                     GRCOMD_CUSTOM_BULKOUT
#if (GRCOMD_CUSTOM_EP2_TYPE == GRCOMD_CUSTOM_BULKIN)
#define GRCOMD_CUSTOM_EP5_TYPE                     GRCOMD_CUSTOM_BULKIN
#else
#define GRCOMD_CUSTOM_EP5_TYPE                     GRCOMD_CUSTOM_BULKOUT
#endif

#define GRCOMD_CUSTOM_MAC_SWAP16(x, y)             ((UINT16) ( (((UINT16)(x)<<8) & 0xFF00) \
                                                      | (((UINT16)(y))    & 0x00FF) ) )
#if (GRCOMD_CUSTOM_ENDIAN == GRCOMD_CUSTOM_BIGENDIAN)
#define GRCOMD_CUSTOM_MAC_SWAPUS(x)                ((UINT16) ( (((UINT16)(x)<<8) & 0xFF00) \
                                                      | (((UINT16)(x)>>8) & 0x00FF) ))
#else
#define GRCOMD_CUSTOM_MAC_SWAPUS(x)                (x)
#endif

/* Interface number */
#define GRCOMD_CUSTOM_MANAGEMENT_IF                (0x00)     /* Request Interface number         */
#define GRCOMD_CUSTOM_NOTIFICATION_IF              (0x00)     /* Notification Interface number    */
#define GRCOMD_CUSTOM_DATA_IF                      (0x01)     /* Data Interface number            */
#define GRCOMD_CUSTOM_MANAGEMENT_IF_1              (0x02)     /* Request Interface number         */
#define GRCOMD_CUSTOM_NOTIFICATION_IF_1            (0x02)     /* Notification Interface number    */
#define GRCOMD_CUSTOM_DATA_IF_1                    (0x03)     /* Data Interface number            */

/* Abstract Model Request send data size */
/* GET_COMM_FEATURE */
#define GRCOMD_CUSTOM_FEATURE_STATUS_SIZE          (0x02)     /* Feature Status data size         */
/* GET_LINE_CODING */
#define GRCOMD_CUSTOM_LINE_CODING_SIZE             (0x07)     /* Line Coding Structure data size  */

/* Abstract Model Request code */
#define GRCOMD_CUSTOM_SEND_ENCAPSULATED_COMMAND    (0x00)
#define GRCOMD_CUSTOM_GET_ENCAPSULATED_RESPONSE    (0x01)
#define GRCOMD_CUSTOM_SET_COMM_FEATURE             (0x02)
#define GRCOMD_CUSTOM_GET_COMM_FEATURE             (0x03)
#define GRCOMD_CUSTOM_CLEAR_COMM_FEATURE           (0x04)
#define GRCOMD_CUSTOM_SET_LINE_CODING              (0x20)
#define GRCOMD_CUSTOM_GET_LINE_CODING              (0x21)
#define GRCOMD_CUSTOM_SET_CONTROL_LINE_STATE       (0x22)
#define GRCOMD_CUSTOM_SEND_BREAK                   (0x23)

/* Abstract Model Notification parameter */
/* NETWORK_CONNECTION */
#define GRCOMD_CUSTOM_NETWORK_CONNECTION_LENGTH    (0x00)
/* RESPONSE_AVAILABLE */
#define GRCOMD_CUSTOM_RESPONSE_AVAILABLE_VALUE     (0x00)
#define GRCOMD_CUSTOM_RESPONSE_AVAILABLE_LENGTH    (0x00)
/* SERIAL_STATE */
#define GRCOMD_CUSTOM_SERIAL_STATE_VALUE           (0x00)
#define GRCOMD_CUSTOM_SERIAL_STATE_LENGTH          (0x02)

/* Abstract Model Notification code */
#define GRCOMD_CUSTOM_NETWORK_CONNECTION           (0x00)
#define GRCOMD_CUSTOM_RESPONSE_AVAILABLE           (0x01)
#define GRCOMD_CUSTOM_SERIAL_STATE                 (0x20)

/* Abstract Model Feature Selector code */
#define GRCOMD_CUSTOM_ABSTRACT_STATE               (0x01)

/* Discriptor Type */
#define GRCOMD_CUSTOM_DESCTYPE_CS_INTERFACE        (0x24)                                 /* CS_INTERFACE */

/* Descriptor SubType */
#define GRCOMD_CUSTOM_DECSSUBTYPE_HFDESC           (0x00)     /* Header Functional Descriptor             */
#define GRCOMD_CUSTOM_DECSSUBTYPE_CMFDESC          (0x01)     /* Call Management Functional Descriptor    */
#define GRCOMD_CUSTOM_DESCSUBTYPE_ACMFDESC         (0x02)     /* Abstract Control Management Descriptor   */
#define GRCOMD_CUSTOM_DESCSUBTYPE_UFDESC           (0x06)     /* Union Functional Descriptor              */

/* The bmCapabilities bitmap value of Abstract Control Management Functional Descriptor */
#define GRCOMD_CUSTOM_BM_CAPA_NETWORK              (0x08)     /* NetworkConnection is supported       */
#define GRCOMD_CUSTOM_BM_CAPA_BREAK                (0x04)     /* SendBreak is supported               */
#define GRCOMD_CUSTOM_BM_CAPA_LINE_SERIAL          (0x02)     /* xx_Line_xx,SeriaState is supported   */
#define GRCOMD_CUSTOM_BM_CAPA_FEATURE              (0x01)     /* xx_CommFeature is supported          */

/* The bmCapabilities bitmap value of Call Management Functional Descriptor */
#define GRCOMD_CUSTOM_BM_CAPA_DATA_CLASS           (0x02)     /* Data Class Interface is supported    */
#define GRCOMD_CUSTOM_BM_CAPA_CALL_MANAGEMENT      (0x01)     /* Call Management itself is supported  */

/* Initial setting value */
#define GRCOMD_CUSTOM_BOUNDERY                     (0x04)     /* Definition for Boundary              */
#define GRCOMD_CUSTOM_MAX_DT_SZ                    (0x20)     /* Definition for Boundary              */
#define GRCOMD_CUSTOM_NUM_CONFIGURATIONS           (0x01)     /* The Configuration number to define   */
#define GRCOMD_CUSTOM_CTRLAREASIZE                 (0x0100)   /* Management Buffer Size               */
#define GRCOMD_CUSTOM_STRING_DESC_NUM              (0x04)     /* Number of String Descriptor          */
#define GRCOMD_CUSTOM_EP0_MAXPKTSIZE_FS            (0x40)     /* EP0 Max Packet Size (FS)             */
#define GRCOMD_CUSTOM_EP0_MAXPKTSIZE_HS            (0x40)     /* EP0 Max Packet Size (HS)             */
#define GRCOMD_CUSTOM_EP3_MAXPKTSIZE_FS            GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_H_FS, \
                                                              GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_L_FS)                /* V1.22 */
#define GRCOMD_CUSTOM_EP3_MAXPKTSIZE_HS            GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_H_HS, \
                                                              GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_L_HS)                /* V1.22 */
                                                            /* EP1 Max Packet Size                  */
#define GRCOMD_CUSTOM_EP6_MAXPKTSIZE_FS            GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_H_FS, \
                                                              GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_L_FS)                /* V1.22 */
#define GRCOMD_CUSTOM_EP6_MAXPKTSIZE_HS            GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_H_HS, \
                                                              GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_L_HS)                /* V1.22 */
                                                            /* EP1 Max Packet Size                  */

#define GRCOMD_CUSTOM_EP0_PAGENUM                  (0x01)     /* FIFO Size(1 Page)                    */
#define GRCOMD_CUSTOM_EP1_PAGENUM                  (0x01)     /* FIFO Size(1 Page)                    */
#define GRCOMD_CUSTOM_EP2_PAGENUM                  (0x01)     /* FIFO Size(1 Page)                    */
#define GRCOMD_CUSTOM_EP3_PAGENUM                  (0x01)     /* FIFO Size(1 Page)                    */
#define GRCOMD_CUSTOM_EP4_PAGENUM                  (0x01)     /* FIFO Size(1 Page)                    */
#define GRCOMD_CUSTOM_EP5_PAGENUM                  (0x01)     /* FIFO Size(1 Page)                    */
#define GRCOMD_CUSTOM_EP6_PAGENUM                  (0x01)     /* FIFO Size(1 Page)                    */
#define GRCOMD_CUSTOM_CTRL_BUF                     (0x01)     /* The number of Control transfer buffer*/
#define GRCOMD_CUSTOM_EP1_DATABUF                  (0x01)     /* The number of Data transfer buffer   */
#define GRCOMD_CUSTOM_EP2_DATABUF                  (0x01)     /* The number of Data transfer buffer   */
#define GRCOMD_CUSTOM_EP3_DATABUF                  (0x01)     /* The number of Data transfer buffer   */
#define GRCOMD_CUSTOM_EP4_DATABUF                  (0x01)     /* The number of Data transfer buffer   */
#define GRCOMD_CUSTOM_EP5_DATABUF                  (0x01)     /* The number of Data transfer buffer   */
#define GRCOMD_CUSTOM_EP6_DATABUF                  (0x01)     /* The number of Data transfer buffer   */

/* Endpoint number */
#define GRCOMD_CUSTOM_EP0                          (0x00)     /* Endpoint0    */
#define GRCOMD_CUSTOM_EP1                          (0x01)     /* Endpoint1    */
#define GRCOMD_CUSTOM_EP2                          (0x02)     /* Endpoint2    */
#define GRCOMD_CUSTOM_EP3                          (0x03)     /* Endpoint3    */
#define GRCOMD_CUSTOM_EP4                          (0x04)     /* Endpoint4    */
#define GRCOMD_CUSTOM_EP5                          (0x05)     /* Endpoint5    */
#define GRCOMD_CUSTOM_EP6                          (0x06)     /* Endpoint6    */


/* Device Descriptor */
#define GRCOMD_CUSTOM_DEVDESC_LENGTH               (0x12)     /* bLength              */
#define GRCOMD_CUSTOM_DEVDESC_DESCRIPTORTYPE       (0x01)     /* bDescriptorType      */
#define GRCOMD_CUSTOM_DEVDESC_DEVICECLASS          (0x02)     /* bDeviceClass         */
#define GRCOMD_CUSTOM_DEVDESC_DEVICESUBCLASS       (0x00)     /* bDeviceSubClass      */
#define GRCOMD_CUSTOM_DEVDESC_DEVICEPROTOCOL       (0x00)     /* bDeviceProtocol      */
#define GRCOMD_CUSTOM_DEVDESC_MAXPKTSIZE0_FS       GRCOMD_CUSTOM_EP0_MAXPKTSIZE_FS        /* wMaxPacketSize0 (FS) */
#define GRCOMD_CUSTOM_DEVDESC_MAXPKTSIZE0_HS       GRCOMD_CUSTOM_EP0_MAXPKTSIZE_HS        /* wMaxPacketSize0 (HS) */
#define GRCOMD_CUSTOM_DEVDESC_NUMCONFIGURATIONS    (0x01)                          /* bNumConfigurations   */

/* Configuration Descriptor */
#define GRCOMD_CUSTOM_CFGDESC_LENGTH               (0x09)      /* bLength                  */
#define GRCOMD_CUSTOM_CFGDESC_DESCRIPTORTYPE       (0x02)      /* bDescriptorType          */
#define GRCOMD_CUSTOM_CFGDESC_CONFIGURATIONVALUE   (0x01)      /* bConfigurationValue      */
#define GRCOMD_CUSTOM_CFGDESC_CONFIGURATION        (0x00)      /* iConfiguration           */

/* When D1 bit of bmCapabilities of Call Management Functional Descriptor is 1 */
#if( ( GRCOMD_CUSTOM_CNF_CMF_CAPABILITIES & GRCOMD_CUSTOM_BM_CAPA_DATA_CLASS ) == GRCOMD_CUSTOM_BM_CAPA_DATA_CLASS )
    #define GRCOMD_CUSTOM_CFGDESC_NUMINTERFACES    (0x04)      /* bNumInterfaces           */
#else
    #define GRCOMD_CUSTOM_CFGDESC_NUMINTERFACES    (0x02)      /* bNumInterfaces           */
#endif

/* Interface Association Descriptor */
#define GRCOMD_CUSTOM_IFADESC_LENGTH               (0x08)      /* bLength                      */
#define GRCOMD_CUSTOM_IFADESC_DESCRIPTORTYPE       (0x0B)      /* bDescriptorType      (0x0B)  */
#define GRCOMD_CUSTOM_IFADESC_INTERFACE_COUNT      (0x02)      /* bInterfacceCount             */
#define GRCOMD_CUSTOM_IFADESC_FUNCTION_CLASS       (0x02)      /* bFunctionClass               */
#define GRCOMD_CUSTOM_IFADESC_FUNCTION_SUB_CLASS   (0x02)      /* bFunctionSubClass            */
#define GRCOMD_CUSTOM_IFADESC_FUNCTION_PROTOCOL    (0x00)      /* bFunctionProtocol            */
#define GRCOMD_CUSTOM_IFADESC_FIRST_INTERFACE      (0x00)      /* bFirstInterface              */
#define GRCOMD_CUSTOM_IFADESC_FIRST_INTERFACE2     (0x02)      /* bFirstInterface              */
#define GRCOMD_CUSTOM_IFADESC_FUNCTION             (0x00)      /* iFunction                    */

/* Interface Descriptor */
#define GRCOMD_CUSTOM_IFDESC_LENGTH                (0x09)      /* bLength              */
#define GRCOMD_CUSTOM_IFDESC_DESCRIPTORTYPE        (0x04)      /* bDescriptorType      */
#define GRCOMD_CUSTOM_IFDESC_ALTERNATESETTING      (0x00)      /* bAlternateSetting    */
#define GRCOMD_CUSTOM_IFDESC_INTERFACEPROTOCOL     (0x00)      /* bInterfaceProtocol   */
#define GRCOMD_CUSTOM_IFDESC_INTERFACE             (0x00)      /* iInterface           */
/* Interface Descriptor 0 specific */
#define GRCOMD_CUSTOM_IFDESC0_INTERFACENUMBER      (0x00)      /* bInterfaceNumber     */
#define GRCOMD_CUSTOM_IFDESC0_NUMENDPOINTS         (0x01)      /* bNumEndpoints        */
#define GRCOMD_CUSTOM_IFDESC0_INTERFACECLASS       (0x02)      /* bInterfaceClass      */
#define GRCOMD_CUSTOM_IFDESC0_INTERFACESUBCLASS    (0x02)      /* bInterfaceSubClass   */
                                                               /* 0x02 is Abstract Control Model   */
/* Interface Descriptor 1 specific */
#define GRCOMD_CUSTOM_IFDESC1_INTERFACENUMBER      (0x01)      /* bInterfaceNumber     */
#define GRCOMD_CUSTOM_IFDESC1_NUMENDPOINTS         (0x02)      /* bNumEndpoints        */
#define GRCOMD_CUSTOM_IFDESC1_INTERFACECLASS       (0x0A)      /* bInterfaceClass      */
#define GRCOMD_CUSTOM_IFDESC1_INTERFACESUBCLASS    (0x00)      /* bInterfaceSubClass   */

/* Interface Descriptor 2 specific */
#define GRCOMD_CUSTOM_IFDESC2_INTERFACENUMBER      (0x02)      /* bInterfaceNumber     */
#define GRCOMD_CUSTOM_IFDESC2_NUMENDPOINTS         (0x01)      /* bNumEndpoints        */
#define GRCOMD_CUSTOM_IFDESC2_INTERFACECLASS       (0x02)      /* bInterfaceClass      */
#define GRCOMD_CUSTOM_IFDESC2_INTERFACESUBCLASS    (0x02)      /* bInterfaceSubClass   */
                                                               /* 0x02 is Abstract Control Model   */
/* Interface Descriptor 3 specific */
#define GRCOMD_CUSTOM_IFDESC3_INTERFACENUMBER      (0x03)      /* bInterfaceNumber     */
#define GRCOMD_CUSTOM_IFDESC3_NUMENDPOINTS         (0x02)      /* bNumEndpoints        */
#define GRCOMD_CUSTOM_IFDESC3_INTERFACECLASS       (0x0A)      /* bInterfaceClass      */
#define GRCOMD_CUSTOM_IFDESC3_INTERFACESUBCLASS    (0x00)      /* bInterfaceSubClass   */

/* Endpoint Descriptor */
#define GRCOMD_CUSTOM_EPDESC_LENGTH                (0x07)      /* bLength                  */
#define GRCOMD_CUSTOM_EPDESC_DESCRIPTORTYPE        (0x05)      /* bDescriptorType          */
#define GRCOMD_CUSTOM_EPDESC_ATTRIBUTES_BULK       (0x02)      /* bmAttributes(Bulk)       */
#define GRCOMD_CUSTOM_EPDESC_ATTRIBUTES_INT        (0x03)      /* bmAttributes(Interrupt)  */
#define GRCOMD_CUSTOM_EPDESC_INTERVAL_BULK         (0x00)      /* bInterval                */
/* Endpoint3(Interrupt In) Address */
#define GRCOMD_CUSTOM_EPDESC_EP3_ADDRESS           (0x83)      /* bEndpointAddress */                       /* V1.22 */
/* Endpoint3(Interrupt In) maxpacket size */
#define GRCOMD_CUSTOM_EPDESC_EP3_MAXPKTSIZE_L_FS   GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_L_FS
                                                                    /* wMaxPacketSize(Low-Byte) */          /* V1.22 */
#define GRCOMD_CUSTOM_EPDESC_EP3_MAXPKTSIZE_H_FS   GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_H_FS
                                                                    /* wMaxPacketSize(High-Byte)*/          /* V1.22 */
#define GRCOMD_CUSTOM_EPDESC_EP3_MAXPKTSIZE_L_HS   GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_L_HS
                                                                    /* wMaxPacketSize(Low-Byte) */          /* V1.22 */
#define GRCOMD_CUSTOM_EPDESC_EP3_MAXPKTSIZE_H_HS   GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_H_HS
                                                                    /* wMaxPacketSize(High-Byte)*/          /* V1.22 */
/* Endpoint6(Interrupt In) Address */
#define GRCOMD_CUSTOM_EPDESC_EP6_ADDRESS           (0x86)      /* bEndpointAddress */          /* V1.22 */
/* Endpoint6(Interrupt In) maxpacket size */
#define GRCOMD_CUSTOM_EPDESC_EP6_MAXPKTSIZE_L_FS   GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_L_FS
                                                                    /* wMaxPacketSize(Low-Byte) */          /* V1.22 */
#define GRCOMD_CUSTOM_EPDESC_EP6_MAXPKTSIZE_H_FS   GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_H_FS
                                                                    /* wMaxPacketSize(High-Byte)*/          /* V1.22 */
#define GRCOMD_CUSTOM_EPDESC_EP6_MAXPKTSIZE_L_HS   GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_L_HS
                                                                    /* wMaxPacketSize(Low-Byte) */          /* V1.22 */
#define GRCOMD_CUSTOM_EPDESC_EP6_MAXPKTSIZE_H_HS   GRCOMD_CUSTOM_CNF_INT_IN_MAXPKTSIZE_H_HS
                                                                    /* wMaxPacketSize(High-Byte)*/          /* V1.22 */

#if( GRCOMD_CUSTOM_EP2_TYPE == GRCOMD_CUSTOM_BULKIN )
    #define GRCOMD_CUSTOM_EP2_MAXPKTSIZE_FS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_FS, \
                                                              GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_FS)
                                                                            /* EP2 Max Packet Size  */
    #define GRCOMD_CUSTOM_EP1_MAXPKTSIZE_FS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_FS, \
                                                              GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_FS)
                                                                            /* EP1 Max Packet Size  */      /* V1.22 */
    #define GRCOMD_CUSTOM_EP2_MAXPKTSIZE_HS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_HS, \
                                                              GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_HS)
                                                                            /* EP2 Max Packet Size  */
    #define GRCOMD_CUSTOM_EP1_MAXPKTSIZE_HS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_HS, \
                                                              GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_HS)
                                                                            /* EP1 Max Packet Size  */      /* V1.22 */
    #define GRCOMD_CUSTOM_EP2_EPTYPE               EPTYPE_IN   /* EP2 Type             */
    #define GRCOMD_CUSTOM_EP1_EPTYPE               EPTYPE_OUT  /* EP1 Type             */                   /* V1.22 */

    #define GRCOMD_CUSTOM_BULKIN_EP_NUMBER         GRCOMD_CUSTOM_EP2  /* BULK-In EP Number    */
    #define GRCOMD_CUSTOM_BULKOUT_EP_NUMBER        GRCOMD_CUSTOM_EP1  /* BULK-Out EP Number   */            /* V1.22 */

    /* Endpoint2(Bulk IN) Address */
    #define GRCOMD_CUSTOM_EPDESC_EP2_ADDRESS       (0x82)      /* bEndpointAddress */
    /* Endpoint1(Bulk OUT) Address */
    #define GRCOMD_CUSTOM_EPDESC_EP1_ADDRESS       (0x01)      /* bEndpointAddress */          /* V1.22 */
    /* Endpoint2(Bulk In) maxpacket size */
    #define GRCOMD_CUSTOM_EPDESC_EP2_MAXPKTSIZE_L_FS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_FS   /* wMaxPacketSize(Low-Byte) */
    #define GRCOMD_CUSTOM_EPDESC_EP2_MAXPKTSIZE_H_FS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_FS   /* wMaxPacketSize(High-Byte)*/
    #define GRCOMD_CUSTOM_EPDESC_EP2_MAXPKTSIZE_L_HS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_HS   /* wMaxPacketSize(Low-Byte) */
    #define GRCOMD_CUSTOM_EPDESC_EP2_MAXPKTSIZE_H_HS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_HS   /* wMaxPacketSize(High-Byte)*/
    /* Endpoint1(Bulk Out) maxpacket size */
    #define GRCOMD_CUSTOM_EPDESC_EP1_MAXPKTSIZE_L_FS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_FS  /* wMaxPacketSize(Low-Byte) */  /* V1.22 */
    #define GRCOMD_CUSTOM_EPDESC_EP1_MAXPKTSIZE_H_FS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_FS  /* wMaxPacketSize(High-Byte)*/  /* V1.22 */
    #define GRCOMD_CUSTOM_EPDESC_EP1_MAXPKTSIZE_L_HS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_HS  /* wMaxPacketSize(Low-Byte) */  /* V1.22 */
    #define GRCOMD_CUSTOM_EPDESC_EP1_MAXPKTSIZE_H_HS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_HS  /* wMaxPacketSize(High-Byte)*/  /* V1.22 */

#else
    #define GRCOMD_CUSTOM_EP2_MAXPKTSIZE_FS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_FS, \
                                                              GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_FS)
                                                                            /* EP2 Max Packet Size  */
    #define GRCOMD_CUSTOM_EP1_MAXPKTSIZE_FS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_FS, \
                                                              GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_FS)
                                                                            /* EP1 Max Packet Size  */      /* V1.22 */
    #define GRCOMD_CUSTOM_EP2_MAXPKTSIZE_HS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_HS, \
                                                              GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_HS)
                                                                            /* EP2 Max Packet Size  */
    #define GRCOMD_CUSTOM_EP1_MAXPKTSIZE_HS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_HS, \
                                                              GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_HS)
                                                                            /* EP1 Max Packet Size  */      /* V1.22 */
    #define GRCOMD_CUSTOM_EP2_EPTYPE               EPTYPE_OUT         /* EP2 Type             */
    #define GRCOMD_CUSTOM_EP1_EPTYPE               EPTYPE_IN          /* EP1 Type             */            /* V1.22 */

    #define GRCOMD_CUSTOM_BULKIN_EP_NUMBER         GRCOMD_CUSTOM_EP1  /* BULK-In EP Number    */            /* V1.22 */
    #define GRCOMD_CUSTOM_BULKOUT_EP_NUMBER        GRCOMD_CUSTOM_EP2  /* BULK-Out EP Number   */

    /* Endpoint2(Bulk OUT) Address */
    #define GRCOMD_CUSTOM_EPDESC_EP2_ADDRESS       (0x02)             /* bEndpointAddress */
    /* Endpoint1(Bulk IN) Address */
    #define GRCOMD_CUSTOM_EPDESC_EP1_ADDRESS       (0x81)             /* bEndpointAddress */               /* V1.22 */
    /* Endpoint2(Bulk Out) maxpacket size */
    #define GRCOMD_CUSTOM_EPDESC_EP2_MAXPKTSIZE_L_FS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_FS  /* wMaxPacketSize(Low-Byte) */
    #define GRCOMD_CUSTOM_EPDESC_EP2_MAXPKTSIZE_H_FS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_FS  /* wMaxPacketSize(High-Byte)*/
    #define GRCOMD_CUSTOM_EPDESC_EP2_MAXPKTSIZE_L_HS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_HS  /* wMaxPacketSize(Low-Byte) */
    #define GRCOMD_CUSTOM_EPDESC_EP2_MAXPKTSIZE_H_HS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_HS  /* wMaxPacketSize(High-Byte)*/
    /* Endpoint1(Bulk In) maxpacket size */
    #define GRCOMD_CUSTOM_EPDESC_EP1_MAXPKTSIZE_L_FS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_FS   /* wMaxPacketSize(Low-Byte) */  /* V1.22 */
    #define GRCOMD_CUSTOM_EPDESC_EP1_MAXPKTSIZE_H_FS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_FS   /* wMaxPacketSize(High-Byte)*/  /* V1.22 */
    #define GRCOMD_CUSTOM_EPDESC_EP1_MAXPKTSIZE_L_HS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_HS   /* wMaxPacketSize(Low-Byte) */  /* V1.22 */
    #define GRCOMD_CUSTOM_EPDESC_EP1_MAXPKTSIZE_H_HS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_HS   /* wMaxPacketSize(High-Byte)*/  /* V1.22 */

#endif

#if( GRCOMD_CUSTOM_EP5_TYPE == GRCOMD_CUSTOM_BULKIN )
    #define GRCOMD_CUSTOM_EP5_MAXPKTSIZE_FS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_FS, \
                                                              GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_FS)
                                                                            /* EP5 Max Packet Size  */
    #define GRCOMD_CUSTOM_EP4_MAXPKTSIZE_FS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_FS, \
                                                              GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_FS)
                                                                            /* EP4 Max Packet Size  */      /* V1.22 */
    #define GRCOMD_CUSTOM_EP5_MAXPKTSIZE_HS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_HS, \
                                                              GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_HS)
                                                                            /* EP5 Max Packet Size  */
    #define GRCOMD_CUSTOM_EP4_MAXPKTSIZE_HS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_HS, \
                                                              GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_HS)
                                                                            /* EP4 Max Packet Size  */      /* V1.22 */
    #define GRCOMD_CUSTOM_EP5_EPTYPE               EPTYPE_IN                /* EP5 Type             */
    #define GRCOMD_CUSTOM_EP4_EPTYPE               EPTYPE_OUT               /* EP4 Type             */      /* V1.22 */

    /* Endpoint2(Bulk IN) Address */
    #define GRCOMD_CUSTOM_EPDESC_EP5_ADDRESS       (0x85)             /* bEndpointAddress */
    /* Endpoint1(Bulk OUT) Address */
    #define GRCOMD_CUSTOM_EPDESC_EP4_ADDRESS       (0x04)             /* bEndpointAddress */          /* V1.22 */
    /* Endpoint2(Bulk In) maxpacket size */
    #define GRCOMD_CUSTOM_EPDESC_EP5_MAXPKTSIZE_L_FS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_FS   /* wMaxPacketSize(Low-Byte) */
    #define GRCOMD_CUSTOM_EPDESC_EP5_MAXPKTSIZE_H_FS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_FS   /* wMaxPacketSize(High-Byte)*/
    #define GRCOMD_CUSTOM_EPDESC_EP5_MAXPKTSIZE_L_HS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_HS   /* wMaxPacketSize(Low-Byte) */
    #define GRCOMD_CUSTOM_EPDESC_EP5_MAXPKTSIZE_H_HS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_HS   /* wMaxPacketSize(High-Byte)*/
    /* Endpoint1(Bulk Out) maxpacket size */
    #define GRCOMD_CUSTOM_EPDESC_EP4_MAXPKTSIZE_L_FS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_FS  /* wMaxPacketSize(Low-Byte) */  /* V1.22 */
    #define GRCOMD_CUSTOM_EPDESC_EP4_MAXPKTSIZE_H_FS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_FS  /* wMaxPacketSize(High-Byte)*/  /* V1.22 */
    #define GRCOMD_CUSTOM_EPDESC_EP4_MAXPKTSIZE_L_HS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_HS  /* wMaxPacketSize(Low-Byte) */  /* V1.22 */
    #define GRCOMD_CUSTOM_EPDESC_EP4_MAXPKTSIZE_H_HS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_HS  /* wMaxPacketSize(High-Byte)*/  /* V1.22 */

#else
    #define GRCOMD_CUSTOM_EP5_MAXPKTSIZE_FS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_FS, \
                                                              GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_FS)
                                                                            /* EP5 Max Packet Size  */
    #define GRCOMD_CUSTOM_EP4_MAXPKTSIZE_FS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_FS, \
                                                              GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_FS)
                                                                            /* EP4 Max Packet Size  */      /* V1.22 */
    #define GRCOMD_CUSTOM_EP5_MAXPKTSIZE_HS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_HS, \
                                                              GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_HS)
                                                                            /* EP5 Max Packet Size  */
    #define GRCOMD_CUSTOM_EP4_MAXPKTSIZE_HS        GRCOMD_CUSTOM_MAC_SWAP16(GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_HS, \
                                                              GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_HS)
                                                                            /* EP4 Max Packet Size  */      /* V1.22 */
    #define GRCOMD_CUSTOM_EP5_EPTYPE               EPTYPE_OUT          /* EP5 Type             */
    #define GRCOMD_CUSTOM_EP4_EPTYPE               EPTYPE_IN           /* EP4 Type             */           /* V1.22 */

    /* Endpoint2(Bulk OUT) Address */
    #define GRCOMD_CUSTOM_EPDESC_EP5_ADDRESS       (0x05)              /* bEndpointAddress */
    /* Endpoint1(Bulk IN) Address */
    #define GRCOMD_CUSTOM_EPDESC_EP4_ADDRESS       (0x84)              /* bEndpointAddress */               /* V1.22 */
    /* Endpoint2(Bulk Out) maxpacket size */
    #define GRCOMD_CUSTOM_EPDESC_EP5_MAXPKTSIZE_L_FS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_FS  /* wMaxPacketSize(Low-Byte) */
    #define GRCOMD_CUSTOM_EPDESC_EP5_MAXPKTSIZE_H_FS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_FS  /* wMaxPacketSize(High-Byte)*/
    #define GRCOMD_CUSTOM_EPDESC_EP5_MAXPKTSIZE_L_HS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_L_HS  /* wMaxPacketSize(Low-Byte) */
    #define GRCOMD_CUSTOM_EPDESC_EP5_MAXPKTSIZE_H_HS   GRCOMD_CUSTOM_CNF_OUT_MAXPKTSIZE_H_HS  /* wMaxPacketSize(High-Byte)*/
    /* Endpoint1(Bulk In) maxpacket size */
    #define GRCOMD_CUSTOM_EPDESC_EP4_MAXPKTSIZE_L_FS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_FS   /* wMaxPacketSize(Low-Byte) */  /* V1.22 */
    #define GRCOMD_CUSTOM_EPDESC_EP4_MAXPKTSIZE_H_FS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_FS   /* wMaxPacketSize(High-Byte)*/  /* V1.22 */
    #define GRCOMD_CUSTOM_EPDESC_EP4_MAXPKTSIZE_L_HS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_L_HS   /* wMaxPacketSize(Low-Byte) */  /* V1.22 */
    #define GRCOMD_CUSTOM_EPDESC_EP4_MAXPKTSIZE_H_HS   GRCOMD_CUSTOM_CNF_IN_MAXPKTSIZE_H_HS   /* wMaxPacketSize(High-Byte)*/  /* V1.22 */

#endif

/* When Interface number is 1 */
#if( GRCOMD_CUSTOM_CFGDESC_NUMINTERFACES == 0x02 )
    #define GRCOMD_CUSTOM_CFGDESC_TOTALLENGTH_H    (0x00)    /* bTotalLength(High-Byte)      */
    #define GRCOMD_CUSTOM_CFGDESC_TOTALLENGTH_L    (0x69)    /* bTotalLength(Low-Byte)       */

    #define GRCOMD_CUSTOM_NUM_ENDPOINTS            (0x03)    /* Number of useing Endpoints   */
/* When Interface number is not 1 */
#else
    #define GRCOMD_CUSTOM_CFGDESC_TOTALLENGTH_H    (0x00)    /* bTotalLength(High-Byte)      */
    #define GRCOMD_CUSTOM_CFGDESC_TOTALLENGTH_L    (0x97)    /* bTotalLength(Low-Byte)       */

    #define GRCOMD_CUSTOM_NUM_ENDPOINTS            (0x07)    /* Number of useing Endpoints   */
#endif

/* Header Functional Descriptor */
#define GRCOMD_CUSTOM_HFDESC_FUNCLENGTH            (0x05)                                 /* bFunctionLength      */
#define GRCOMD_CUSTOM_HFDESC_DESCTYPE              GRCOMD_CUSTOM_DESCTYPE_CS_INTERFACE    /* bDescriptorType      */
#define GRCOMD_CUSTOM_HFDESC_DESCSUBTYPE           GRCOMD_CUSTOM_DECSSUBTYPE_HFDESC       /* bDescriptorSubtype   */

/* Call Management Functional Descriptor */
#define GRCOMD_CUSTOM_CMFDESC_FUNCLENGTH           (0x05)                                 /* bFunctionLength      */
#define GRCOMD_CUSTOM_CMFDESC_DESCTYPE             GRCOMD_CUSTOM_DESCTYPE_CS_INTERFACE    /* bDescriptorType      */
#define GRCOMD_CUSTOM_CMFDESC_DESCSUBTYPE          GRCOMD_CUSTOM_DECSSUBTYPE_CMFDESC      /* bDescriptorSubtype   */

/* Abstract Control Management Functional Descriptor */
#define GRCOMD_CUSTOM_ACMFDESC_FUNCLENGTH          (0x04)                                 /* bFunctionLength      */
#define GRCOMD_CUSTOM_ACMFDESC_DESCTYPE            GRCOMD_CUSTOM_DESCTYPE_CS_INTERFACE    /* bDescriptorType      */
#define GRCOMD_CUSTOM_ACMFDESC_DESCSUBTYPE         GRCOMD_CUSTOM_DESCSUBTYPE_ACMFDESC     /* bDescriptorSubtype   */

/* Union Functional Descriptor */
#define GRCOMD_CUSTOM_UFDESC_FUNCLENGTH            (0x05)                                 /* bFunctionLength      */
#define GRCOMD_CUSTOM_UFDESC_DESCTYPE              GRCOMD_CUSTOM_DESCTYPE_CS_INTERFACE    /* bDescriptorType      */
#define GRCOMD_CUSTOM_UFDESC_DESCSUBTYPE           GRCOMD_CUSTOM_DESCSUBTYPE_UFDESC       /* bDescriptorSubtype   */
#define GRCOMD_CUSTOM_UFDESC_MASTERIF              (0x00)                                 /* bMasterInterface     */
#define GRCOMD_CUSTOM_UFDESC_SLAVEIF0              (0x01)                                 /* bSlaveInterface0     */

#define GRCOMD_CUSTOM_UFDESC1_MASTERIF             (0x02)                                 /* bMasterInterface     */
#define GRCOMD_CUSTOM_UFDESC1_SLAVEIF0             (0x03)                                 /* bSlaveInterface0     */


/* Structure of Notification Request */
typedef struct GRCOMD_CUSTOM_NOTIFICATION
{
    UINT8   bmRequestType;              /* Request Type      */
    UINT8   bNotification;              /* Notification Code */
    UINT16  wValue;                     /* Value             */
    UINT16  wIndex;                     /* Index             */
    UINT16  wLength;                    /* Data Phase Length */
} GRCOMD_CUSTOM_NOTIFICATION;
#define GRCOMD_CUSTOM_NOTIFICATION_SIZE        sizeof(GRCOMD_CUSTOM_NOTIFICATION)

/* Structure of The data of a Notification Request is included */
typedef struct GRCOMD_CUSTOM_NOTIFICATION_SERIAL_STATE
{
    GRCOMD_CUSTOM_NOTIFICATION     tNotice;    /* Notification Request structure */
    UINT16                  usState;    /* State */
} GRCOMD_CUSTOM_NOTIFICATION_SERIAL_STATE;
#define GRCOMD_CUSTOM_NOTIFICATION_SERIAL_STATE_SIZE       sizeof(GRCOMD_CUSTOM_NOTIFICATION_SERIAL_STATE)

/* Structure of Interface Descriptor */
typedef struct grusb_dev_interface_acc_desc_tag
{
    UINT8   uc_bLength;                 /* bLength              */
    UINT8   uc_bDescriptorType;         /* bDescriptorType      */
    UINT8   uc_bFirstInterface;         /* bFirstInterface      */
    UINT8   uc_bInterfaceCount;         /* bInterfaceCount      */
    UINT8   uc_bFunctionClass;          /* bFunctionClass       */
    UINT8   uc_bFunctionSubClass;       /* bFunctionSubClass    */
    UINT8   uc_bFunctionProtocol;       /* bFunctionProtocol    */
    UINT8   uc_iFunction;               /* iFunction            */
} GRUSB_DEV_INTERFACE_ACC_DESC;

/* Structure of Header Functional Descriptor */
typedef struct _header_func_desc_tag {
    UINT8   bFunctionLength;
    UINT8   bDescriptorType;
    UINT8   bDescriptorSubtype;
    UINT8   bcdCDC1;
    UINT8   bcdCDC2;
} GRCOMD_CUSTOM_HEADER_DESC;

/* Structure of Call Management Functional Descriptor */
typedef struct _call_func_desc_tag {
    UINT8   bFunctionLength;
    UINT8   bDescriptorType;
    UINT8   bDescriptorSubtype;
    UINT8   bmCapabilities;
    UINT8   bDataInterface;
} GRCOMD_CUSTOM_CALL_DESC;

/* Structure of Abstract Control Management Functional Descriptor */
typedef struct _abstract_func_desc_tag {
    UINT8   bFunctionLength;
    UINT8   bDescriptorType;
    UINT8   bDescriptorSubtype;
    UINT8   bmCapabilities;
} GRCOMD_CUSTOM_ABSTRACT_DESC;

/* Structure of Union Functional Descriptor */
typedef struct _union_func_desc_tag {
    UINT8   bFunctionLength;
    UINT8   bDescriptorType;
    UINT8   bDescriptorSubtype;
    UINT8   bMasterInterface;
    UINT8   bSlaveInterface[1];
} GRCOMD_CUSTOM_UNION_DESC;

/* Structure of Descriptor used by Communication Class */
typedef struct _com_class_desc_tag {
    GRUSB_DEV_CONFIG_DESC           tConfig;
    GRUSB_DEV_INTERFACE_ACC_DESC    tInterAcc;
    GRUSB_DEV_INTERFACE_DESC        tInterface0;
    GRCOMD_CUSTOM_HEADER_DESC       tHeader0;
    GRCOMD_CUSTOM_ABSTRACT_DESC     tAbstract;
    GRCOMD_CUSTOM_UNION_DESC        tUnion;
    GRCOMD_CUSTOM_CALL_DESC         tCall;
    GRUSB_DEV_ENDPOINT_DESC         tEndPoint1;
/* If Interface number is not 1, the following data is also used */
#if( GRCOMD_CUSTOM_CFGDESC_NUMINTERFACES != 0x01 )
    GRUSB_DEV_INTERFACE_DESC        tInterface1;
    GRCOMD_CUSTOM_HEADER_DESC       tHeader1;
    GRUSB_DEV_ENDPOINT_DESC         tEndPoint2;
    GRUSB_DEV_ENDPOINT_DESC         tEndPoint3;
#endif
} GRCOMD_CUSTOM_CLASS_DESC;

/**** INTERNAL FUNCTION PROTOTYPES ******************************************/
EXTERN GRUSB_DEV_STRING_DESC*   GRUSB_COMD_CUSTOM_GetStrDesc( VOID );

#endif  /* _COM_CUSTOM_DEF_H_ */
