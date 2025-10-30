/****************************************************************************/
/*                                                                          */
/*               Copyright(C) 2003-2017 Grape Systems, Inc.                 */
/*                        All Rights Reserved                               */
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
/*      com_hid_cnf.c                                             1.00      */
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

/**** INCLUDE FILES *********************************************************/
#include    "com_hid_def.h"

#ifdef GRCOMD_HID_CNF_DEBUG
#include    "dbg_mdl.h"
#define GRCOMD_HID_CNF_LOG2(m,n,x,y)  GRDBG_TRACE2(m,n,x,y)
#else
#define GRCOMD_HID_CNF_LOG2(m,n,x,y)
#define GRDBG_COMD_CNF                  (0x00)
#define END_FUNC                        (0x00)
#endif

/* Number of characters for Manufacturer Name */
#define GRCOMD_HID_MANUFACTURER_CHARS       (3)
/* Number of characters for Product Name */
#define GRCOMD_HID_PRODUCT_CHARS            (7) 
/* Number of characters for Serial Number */
#define GRCOMD_HID_SERIALNUMBER_CHARS       (12)

/* Number of characters for ROM File */
#define GRCOMD_HID_ROMFILE_CHARS            (10)

/* Number of characters for DFU I/F */
#define GRCOMD_HID_DFU_IF_CHARS             (7)

/* Number of characters for HID I/F */
#define GRCOMD_HID_IF_CHARS                 (126)

/* Size of Report Descriptor */
#define GRCOMD_HID_REPORT_DESC_SIZE         (0x035E)


/* String Descriptor 0 Structure */
#define GRCOMD_HID_STRDESC_0_LANGID_SIZE    (2)
typedef struct grusb_comd_string_desc0_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_HID_STRDESC_0_LANGID_SIZE];   /* bString(UNICODE encoded string)  */
} GRUSB_HID_COMD_STRING_DESC0;

/* String Descriptor 1 Structure */
#define GRCOMD_HID_STRDESC_1_STR_SIZE       (GRCOMD_HID_MANUFACTURER_CHARS * 2)
typedef struct grusb_comd_string_desc1_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_HID_STRDESC_1_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_HID_COMD_STRING_DESC1;

/* String Descriptor 2 Structure */
#define GRCOMD_HID_STRDESC_2_STR_SIZE       (GRCOMD_HID_PRODUCT_CHARS * 2)
typedef struct grusb_comd_string_desc2_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_HID_STRDESC_2_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_HID_COMD_STRING_DESC2;

/* String Descriptor 3 Structure */
#define GRCOMD_HID_STRDESC_3_STR_SIZE       (GRCOMD_HID_SERIALNUMBER_CHARS * 2)
typedef struct grusb_comd_string_desc3_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_HID_STRDESC_3_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_HID_COMD_STRING_DESC3;

/* String Descriptor 4 Structure */
#define GRCOMD_HID_STRDESC_4_STR_SIZE       (GRCOMD_HID_ROMFILE_CHARS * 2)
typedef struct grusb_comd_string_desc4_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_HID_STRDESC_4_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_HID_COMD_STRING_DESC4;

/* String Descriptor 5 Structure */
#define GRCOMD_HID_STRDESC_5_STR_SIZE       (GRCOMD_HID_DFU_IF_CHARS * 2)
typedef struct grusb_comd_string_desc5_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_HID_STRDESC_5_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_HID_COMD_STRING_DESC5;

/* String Descriptor 6 Structure */
#define GRCOMD_HID_STRDESC_6_STR_SIZE       (GRCOMD_HID_IF_CHARS * 2)
typedef struct grusb_comd_string_desc6_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_HID_STRDESC_6_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_HID_COMD_STRING_DESC6;

DLOCAL UINT8 HidReportDesc_tbl_GSA[GRCOMD_HID_REPORT_DESC_SIZE] =
{
/* Report Descriptor */
    0x05, 0x92,			/*	0x0000 : USAGE_PAGE (GSA Gaming Device) */
    0x09, 0x12,			/*	0x0002 : USAGE (Note/Ticket Acceptor Device) */
    0xA1, 0x01,			/*	0x0004 : COLLECTION (Application) */

    /* ACK (合計：42bytes) */
    0x09, 0x40,			/*	0x0006 : USAGE - ACK (0x40) */
    0x85, 0x01,			/*	0x0008 : REPORT_ID (0x01) */
    0xA1, 0x02,			/*	0x000A : COLLECTION (Logical) */
    /*--- Resync : 1bit ---*/
    0x15, 0x00,			/*	0x000C : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x000E : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x0010 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0012 : REPORT_COUNT (1) */
    0x09, 0x90,			/*	0x0014 : USAGE - Resync (0x90) */
    0xB1, 0x02,			/*	0x0016 : FEATURE (Data, Var, Abs) */
    /*--- Reserved bits : 7bits ---*/
    0x15, 0x00,			/*	0x0018 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x001A : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x001C : REPORT_SIZE (1) */
    0x95, 0x07,			/*	0x001E : REPORT_COUNT (7) */
    0xB1, 0x03,			/*	0x0020 : FEATURE (Cnst, Var, Abs) */
    /*--- Transaction ID : 8bits ---*/
    0x15, 0x00,			/*	0x0022 : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x0024 : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x0027 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x0029 : REPORT_COUNT (1) */
    0x09, 0x60,			/*	0x002B : USAGE - Transaction ID (0x60) */
    0xB1, 0x02,			/*	0x002D : FEATURE (Data, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x002F : END_COLLECTION (Logical) */

    /* Enable (合計：10bytes) */
    /*--- Enable : 8bits ---*/
    0x09, 0x41,			/*	0x0030 : USAGE - Enable (0x41) */
    0x85, 0x02,			/*	0x0032 : REPORT_ID (0x02) */
    0x75, 0x08,			/*	0x0034 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x0036 : REPORT_COUNT (1) */
    0xB1, 0x03,			/*	0x0038 : FEATURE (Cnst, Var, Abs) */

    /* Disable (合計：10bytes) */
    /*--- Disable : 8bits ---*/
    0x09, 0x42,			/*	0x003A : USAGE - Disable (0x42) */
    0x85, 0x03,			/*	0x003C : REPORT_ID (0x03) */
    0x75, 0x08,			/*	0x003E : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x0040 : REPORT_COUNT (1) */
    0xB1, 0x03,			/*	0x0042 : FEATURE (Cnst, Var, Abs) */

    /* Self Test (合計：29bytes) */
    0x09, 0x43,			/*	0x0044 : USAGE - Self Test (0x43) */
    0x85, 0x04,			/*	0x0046 : REPORT_ID (0x04) */
    0xA1, 0x02,			/*	0x0048 : COLLECTION (Logical) */
    /*--- NVM : 1bit ---*/
    0x15, 0x00,			/*	0x004A : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x004C : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x004E : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0050 : REPORT_COUNT (1) */
    0x09, 0x93,			/*	0x0052 : USAGE - NVM (0x93) */
    0xB1, 0x02,			/*	0x0054 : FEATURE (Data, Var, Abs) */
    /*--- Reserved bits : 7bits ---*/
    0x15, 0x00,			/*	0x0056 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x0058 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x005A : REPORT_SIZE (1) */
    0x95, 0x07,			/*	0x005C : REPORT_COUNT (7) */
    0xB1, 0x03,			/*	0x005E : FEATURE (Cnst, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x0060 : END_COLLECTION (Logical) */

    /* Calculate CRC (合計：20bytes) */
    0x09, 0x47,			/*	0x0061 : USAGE - Calculate CRC (0x47) */
    0x85, 0x08,			/*	0x0063 : REPORT_ID (0x08) */
    0xA1, 0x02,			/*	0x0065 : COLLECTION (Logical) */
    /*--- Seed : 8bits X 4 ---*/
    0x15, 0x00,			/*	0x0067 : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x0069 : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x006C : REPORT_SIZE (8) */
    0x95, 0x04,			/*	0x006E : REPORT_COUNT (4) */
    0x09, 0x63,			/*	0x0070 : USAGE - Seed (0x63) */
    0xB1, 0x02,			/*	0x0072 : FEATURE (Data, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x0074 : END_COLLECTION (Logical) */

    /* CRC Data (合計：20bytes) */
    0x09, 0x48,			/*	0x0075 : USAGE - CRC Data (0x48) */
    0x85, 0x09,			/*	0x0077 : REPORT_ID (0x09) */
    0xA1, 0x02,			/*	0x0079 : COLLECTION (Logical) */
    /*--- Seed : 8bits X 4 ---*/
    0x15, 0x00,			/*	0x007B : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x007B : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x0080 : REPORT_SIZE (8) */
    0x95, 0x04,			/*	0x0082 : REPORT_COUNT (4) */
    0x09, 0x64,			/*	0x0084 : USAGE - Result (0x64) */
    0x81, 0x02,			/*	0x0086 : INPUT (Data, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x0088 : END_COLLECTION (Logical) */

    /* Device State (合計：41bytes) */
    0x09, 0x49,			/*	0x0089 : USAGE - Device State (0x49) */
    0x85, 0x0A,			/*	0x008B : REPORT_ID (0x0A) */
    0xA1, 0x02,			/*	0x008D : COLLECTION (Logical) */
    /*--- Enable : 1bit ---*/
    0x15, 0x00,			/*	0x008F : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x0091 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x0093 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0095 : REPORT_COUNT (1) */
    0x09, 0x94,			/*	0x0097 : USAGE - Enable (0x94) */
    0x81, 0x02,			/*	0x0099 : INPUT (Data, Var, Abs) */
    /*--- Disable : 1bit ---*/
    0x15, 0x00,			/*	0x009B : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x009D : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x009F : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x00A1 : REPORT_COUNT (1) */
    0x09, 0x95,			/*	0x00A3 : USAGE - Disable (0x95) */
    0x81, 0x02,			/*	0x00A5 : INPUT (Data, Var, Abs) */
    /*--- Reserved bits : 6bits ---*/
    0x15, 0x00,			/*	0x00A7 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x00A9 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x00AB : REPORT_SIZE (1) */
    0x95, 0x06,			/*	0x00AD : REPORT_COUNT (6) */
    0x81, 0x03,			/*	0x00AF : INPUT (Cnst, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x00B1 : END_COLLECTION (Logical) */
    /* Number of Note Data Entries Command (合計：11bytes) */
    /*--- Number of Note Data Entries : 8bits ---*/
    0x0A, 0x10, 0x02,	/*	0x00B2 : USAGE - Number of Note Data Entries (0x0210) */
    0x85, 0x80,			/*	0x00B5 : REPORT_ID (0x80) */
    0x75, 0x08,			/*	0x00B7 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x00B9 : REPORT_COUNT (1) */
    0xB1, 0x03,			/*	0x00BB : FEATURE (Cnst, Var, Abs) */

    /* Number of Note Data Entries Response (合計：22bytes) */
    0x0A, 0x10, 0x02,	/*	0x00BD : USAGE - Number of Note Data Entries Response (0x0210) */
    0x85, 0x80,			/*	0x00C0 : REPORT_ID (0x80) */
    0xA1, 0x02,			/*	0x00C2 : COLLECTION (Logical) */
    /*--- Number : 8bits ---*/
    0x15, 0x00,			/*	0x00C4 : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x00C6 : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x00C9 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x00CB : REPORT_COUNT (1) */
    0x0A, 0x30, 0x02,	/*	0x00CD : USAGE - Number of Reports (0x0230) */
    0x81, 0x02,			/*	0x00D0 : INPUT (Data, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x00D2 : END_COLLECTION (Logical) */

    /* Read Note Table Command (合計：11bytes) */
    /*--- Read Note Table : 8bits ---*/
    0x0A, 0x11, 0x02,	/*	0x00D3 : USAGE - Read Note Table (0x0211) */
    0x85, 0x81,			/*	0x00D6 : REPORT_ID (0x81) */
    0x75, 0x08,			/*	0x00D8 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x00DA : REPORT_COUNT (1) */
    0xB1, 0x03,			/*	0x00DC : FEATURE (Cnst, Var, Abs) */

    /* Read Note Table Response (合計：91bytes) */
    0x0A, 0x11, 0x02,	/*	0x00DE : USAGE - Read Note Table (0x0211) */
    0x85, 0x81,			/*	0x00E1 : REPORT_ID (0x81) */
    0xA1, 0x02,			/*	0x00E3 : COLLECTION (Logical) */
    /*--- Entry Number : 8bits ---*/
    0x15, 0x00,			/*	0x00E5 : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x00E7 : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x00EA : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x00EC : REPORT_COUNT (1) */
    0x0A, 0x37, 0x02,	/*	0x00EE : USAGE - Entry Number (0x0237) */
    0x81, 0x02,			/*	0x00F1 : INPUT (Data, Var, Abs) */
    /*--- Country : 8bits X 3 ---*/
    0x15, 0x00,			/*	0x00F3 : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x00F5 : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x00F8 : REPORT_SIZE (8) */
    0x95, 0x03,			/*	0x00FA : REPORT_COUNT (3) */
    0x0A, 0x31, 0x02,	/*	0x00FC : USAGE - Country (0x0231) */
    0x81, 0x02,			/*	0x00FF : INPUT (Data, Var, Abs) */
    /*--- Value : 8bits X 2 ---*/
    0x15, 0x00,			/*	0x0101 : LOGICAL_MINIMUM (0) */
    0x27, 0xFF, 0xFF,
    0x00, 0x00,			/*	0x0103 : LOGICAL_MAXIMUM (65535) */
    0x75, 0x08,			/*	0x0108 : REPORT_SIZE (8) */
    0x95, 0x02,			/*	0x010A : REPORT_COUNT (2) */
    0x0A, 0x32, 0x02,	/*	0x010C : USAGE - Value (0x0232) */
    0x81, 0x02,			/*	0x010F : INPUT (Data, Var, Abs) */
    /*--- Scalar : 7bits ---*/
    0x15, 0x00,			/*	0x0111 : LOGICAL_MINIMUM (0) */
    0x25, 0x7F,			/*	0x0113 : LOGICAL_MAXIMUM (127) */
    0x75, 0x07,			/*	0x0115 : REPORT_SIZE (7) */
    0x95, 0x01,			/*	0x0117 : REPORT_COUNT (1) */
    0x0A, 0x60, 0x02,	/*	0x0119 : USAGE - Scalar (0x0260) */
    0x81, 0x02,			/*	0x011C : INPUT (Data, Var, Abs) */
    /*--- Sign : 1bit ---*/
    0x15, 0x00,			/*	0x011E : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x0120 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x0122 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0124 : REPORT_COUNT (1) */
    0x0A, 0x61, 0x02,	/*	0x0126 : USAGE - Sign (0x0261) */
    0x81, 0x02,			/*	0x0129 : INPUT (Data, Var, Abs) */
    /*--- Version : 8bits ---*/
    0x15, 0x00,			/*	0x012B : LOGICAL_MINIMUM (0) */
    0x25, 0x02,			/*	0x012D : LOGICAL_MAXIMUM (2) */
    0x75, 0x08,			/*	0x012F : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x0131 : REPORT_COUNT (1) */
    0x0A, 0x33, 0x02,	/*	0x0133 : USAGE - Version (0x0233) */
    0x81, 0x02,			/*	0x0136 : INPUT (Data, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x0138 : END_COLLECTION (Logical) */

    /* Extend TimeOut (合計：11bytes) */
    /*--- Extend TimeOut : 8bits ---*/
    0x0A, 0x12, 0x02,	/*	0x0139 : USAGE - Extend TimeOut (0x0212) */
    0x85, 0x82,			/*	0x013C : REPORT_ID (0x82) */
    0x75, 0x08,			/*	0x013E : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x0140 : REPORT_COUNT (1) */
    0xB1, 0x03,			/*	0x0142 : FEATURE (Cnst, Var, Abs) */

    /* Accept Note/Ticket (合計：11bytes) */
    /*--- Accept Note/Ticket : 8bits ---*/
    0x0A, 0x13, 0x02,	/*	0x0144 : USAGE - Accept Note/Ticket (0x0213) */
    0x85, 0x83,			/*	0x0147 : REPORT_ID (0x83) */
    0x75, 0x08,			/*	0x0149 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x014B : REPORT_COUNT (1) */
    0xB1, 0x03,			/*	0x014D : FEATURE (Cnst, Var, Abs) */

    /* Return Note/Ticket (合計：11bytes) */
    /*--- Return Note/Ticket : 8bits ---*/
    0x0A, 0x14, 0x02,	/*	0x014F : USAGE - Return Note/Ticket (0x0214) */
    0x85, 0x84,			/*	0x0152 : REPORT_ID (0x84) */
    0x75, 0x08,			/*	0x0154 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x0156 : REPORT_COUNT (1) */
    0xB1, 0x03,			/*	0x0158 : FEATURE (Cnst, Var, Abs) */

    /* Failure Status (合計：126bytes) */
    0x0A, 0x15, 0x02,	/*	0x015A : USAGE - Failure Status (0x0215) */
    0x85, 0x85,			/*	0x015D : REPORT_ID (0x85) */
    0xA1, 0x02,			/*	0x015F : COLLECTION (Logical) */
    /*--- Firmware : 1bit ---*/
    0x15, 0x00,			/*	0x0161 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x0163 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x0165 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0167 : REPORT_COUNT (1) */
    0x0A, 0x62, 0x02,	/*	0x0169 : USAGE - Firmware (0x0262) */
    0x81, 0x02,			/*	0x016C : INPUT (Data, Var, Abs) */
    /*--- Mechanical : 1bit ---*/
    0x15, 0x00,			/*	0x016E : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x0170 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x0172 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0174 : REPORT_COUNT (1) */
    0x0A, 0x63, 0x02,	/*	0x0176 : USAGE - Mechanical (0x0263) */
    0x81, 0x02,			/*	0x0179 : INPUT (Data, Var, Abs) */
    /*--- Optical : 1bit ---*/
    0x15, 0x00,			/*	0x017B : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x017D : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x017F : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0181 : REPORT_COUNT (1) */
    0x0A, 0x64, 0x02,	/*	0x0183 : USAGE - Optical (0x0264) */
    0x81, 0x02,			/*	0x0186 : INPUT (Data, Var, Abs) */
    /*--- Component : 1bit ---*/
    0x15, 0x00,			/*	0x0188 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x018A : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x018C : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x018E : REPORT_COUNT (1) */
    0x0A, 0x65, 0x02,	/*	0x0190 : USAGE - Component (0x0265) */
    0x81, 0x02,			/*	0x0193 : INPUT (Data, Var, Abs) */
    /*--- NVM : 1bit ---*/
    0x15, 0x00,			/*	0x0195 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x0197 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x0199 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x019B : REPORT_COUNT (1) */
    0x0A, 0x66, 0x02,	/*	0x019D : USAGE - NVM (0x0266) */
    0x81, 0x02,			/*	0x01A0 : INPUT (Data, Var, Abs) */
    /*--- Unused_0 : 1bit ---*/
    0x15, 0x00,			/*	0x01A2 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x01A4 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x01A6 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x01A8 : REPORT_COUNT (1) */
    0x0A, 0x67, 0x02,	/*	0x01AA : USAGE - Unused_0 (0x0267) */
    0x81, 0x02,			/*	0x01AD : INPUT (Data, Var, Abs) */
    /*--- Unused_1 : 1bit ---*/
    0x15, 0x00,			/*	0x01AF : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x01B1 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x01B3 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x01B5 : REPORT_COUNT (1) */
    0x0A, 0x68, 0x02,	/*	0x01B7 : USAGE - Unused_1 (0x0268) */
    0x81, 0x02,			/*	0x01BA : INPUT (Data, Var, Abs) */
    /*--- Other : 1bit ---*/
    0x15, 0x00,			/*	0x01BC : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x01BE : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x01C0 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x01C2 : REPORT_COUNT (1) */
    0x0A, 0x69, 0x02,	/*	0x01C4 : USAGE - Other (0x0269) */
    0x81, 0x02,			/*	0x01C7 : INPUT (Data, Var, Abs) */
    /*--- Diagnostics : 8bits ---*/
    0x15, 0x00,			/*	0x01C9 : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x01CB : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x01CE : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x01D0 : REPORT_COUNT (1) */
    0x0A, 0x34, 0x02,	/*	0x01D2 : USAGE - Diagnostics (0x0234) */
    0x81, 0x02,			/*	0x01D5 : INPUT (Data, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x01D7 : END_COLLECTION (Logical) */

/* Note Validated (合計：35bytes) */
    0x0A, 0x16, 0x02,	/*	0x01D8 : USAGE - Note Validated (0x0216) */
    0x85, 0x86,			/*	0x01DB : REPORT_ID (0x86) */
    0xA1, 0x02,			/*	0x01DD : COLLECTION (Logical) */
    /*--- Transaction ID : 8bits ---*/
    0x15, 0x00,			/*	0x01DF : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x01E1 : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x01E4 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x01E6 : REPORT_COUNT (1) */
    0x09, 0x60,			/*	0x01E8 : USAGE - Transaction ID (0x60) */
    0x81, 0x02,			/*	0x01EA : INPUT (Data, Var, Abs) */
    /*--- Note ID : 8bits ---*/
    0x15, 0x00,			/*	0x01EC : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x01EE : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x01F1 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x01F3 : REPORT_COUNT (1) */
    0x0A, 0x35, 0x02,	/*	0x01F5 : USAGE - Note ID (0x0235) */
    0x81, 0x02,			/*	0x01F8 : INPUT (Data, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x01FA : END_COLLECTION (Logical) */

    /* Ticket Validated (合計：49bytes) */
    0x0A, 0x17, 0x02,	/*	0x01FB : USAGE - Ticket Validated (0x0217) */
    0x85, 0x87,			/*	0x01FE : REPORT_ID (0x87) */
    0xA1, 0x02,			/*	0x0200 : COLLECTION (Logical) */
    /*--- Transaction ID : 8bits ---*/
    0x15, 0x00,			/*	0x0202 : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x0204 : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x0207 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x0209 : REPORT_COUNT (1) */
    0x09, 0x60,			/*	0x020B : USAGE - Transaction ID (0x60) */
    0x81, 0x02,			/*	0x020D : INPUT (Data, Var, Abs) */
    /*--- Length : 8bits ---*/
    0x15, 0x00,			/*	0x020F : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x0211 : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x0214 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x0216 : REPORT_COUNT (1) */
    0x0A, 0x36, 0x02,	/*	0x0218 : USAGE - Length (0x0236) */
    0x81, 0x02,			/*	0x021B : INPUT (Data, Var, Abs) */
    /*--- Code : 8bits X 24 ---*/
    0x15, 0x00,			/*	0x021D : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x021F : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x0222 : REPORT_SIZE (8) */
    0x95, 0x18,			/*	0x0224 : REPORT_COUNT (24) */
    0x0A, 0x80, 0x02,	/*	0x0226 : USAGE - Code (0x0280) */
    0x81, 0x02,			/*	0x0229 : INPUT (Data, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x022B : END_COLLECTION (Logical) */

    /* Note/Ticket Status (合計：125bytes) */
    0x0A, 0x18, 0x02,	/*	0x022C : USAGE - Note/Ticket Status (0x0218) */
    0x85, 0x88,			/*	0x022F : REPORT_ID (0x88) */
    0xA1, 0x02,			/*	0x0231 : COLLECTION (Logical) */
    /*--- Transaction ID : 8bits ---*/
    0x15, 0x00,			/*	0x0233 : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x0235 : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x0238 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x023A : REPORT_COUNT (1) */
    0x09, 0x60,			/*	0x023C : USAGE - Transaction ID (0x60) */
    0x81, 0x02,			/*	0x023E : INPUT (Data, Var, Abs) */
    /*--- Accepted : 1bit ---*/
    0x15, 0x00,			/*	0x0240 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x0242 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x0244 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0246 : REPORT_COUNT (1) */
    0x0A, 0x6A, 0x02,	/*	0x0248 : USAGE - Accepted (0x026A) */
    0x81, 0x02,			/*	0x024B : INPUT (Data, Var, Abs) */
    /*--- Returned : 1bit ---*/
    0x15, 0x00,			/*	0x024D : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x024F : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x0251 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0253 : REPORT_COUNT (1) */
    0x0A, 0x6B, 0x02,	/*	0x0255 : USAGE - Returned (0x026B) */
    0x81, 0x02,			/*	0x0258 : INPUT (Data, Var, Abs) */
    /*--- Rejected : 1bit ---*/
    0x15, 0x00,			/*	0x025A : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x025C : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x025E : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0260 : REPORT_COUNT (1) */
    0x0A, 0x6C, 0x02,	/*	0x0262 : USAGE - Rejected (0x026C) */
    0x81, 0x02,			/*	0x0265 : INPUT (Data, Var, Abs) */
    /*--- Removed : 1bit ---*/
    0x15, 0x00,			/*	0x0267 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x0269 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x026B : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x026D : REPORT_COUNT (1) */
    0x0A, 0x75, 0x02,	/*	0x026F : USAGE - Removed (0x0275) */
    0x81, 0x02,			/*	0x0272 : INPUT (Data, Var, Abs) */
    /*--- Note Path Clear : 1bit ---*/
    0x15, 0x00,			/*	0x0274 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x0276 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x0278 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x027A : REPORT_COUNT (1) */
    0x0A, 0x76, 0x02,	/*	0x027C : USAGE - Note Path Clear (0x0276) */
    0x81, 0x02,			/*	0x027F : INPUT (Data, Var, Abs) */
    /*--- Unused_2 : 1bit ---*/
    0x15, 0x00,			/*	0x0281 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x0283 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x0285 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0287 : REPORT_COUNT (1) */
    0x0A, 0x6D, 0x02,	/*	0x0289 : USAGE - Unused_2 (0x026D) */
    0x81, 0x03,			/*	0x028C : INPUT (Cnst, Var, Abs) */
    /*--- Cheat : 1bit ---*/
    0x15, 0x00,			/*	0x028E : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x0290 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x0292 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0294 : REPORT_COUNT (1) */
    0x0A, 0x6E, 0x02,	/*	0x0296 : USAGE - Cheat (0x026E) */
    0x81, 0x02,			/*	0x0299 : INPUT (Data, Var, Abs) */
    /*--- Jam : 1bit ---*/
    0x15, 0x00,			/*	0x029B : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x029D : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x029F : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x02A1 : REPORT_COUNT (1) */
    0x0A, 0x6F, 0x02,	/*	0x02A3 : USAGE - Jam (0x026F) */
    0x81, 0x02,			/*	0x02A6 : INPUT (Data, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x02A8 : END_COLLECTION (Logical) */

    /* Stacker Status (合計：125bytes) */
    0x0A, 0x19, 0x02,	/*	0x02A9 : USAGE - Stacker Status (0x0219) */
    0x85, 0x89,			/*	0x02AC : REPORT_ID (0x89) */
    0xA1, 0x02,			/*	0x02AE : COLLECTION (Logical) */
    /*--- Transaction ID : 8bits ---*/
    0x15, 0x00,			/*	0x02B0 : LOGICAL_MINIMUM (0) */
    0x26, 0xFF, 0x00,	/*	0x02B2 : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x02B5 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x02B7 : REPORT_COUNT (1) */
    0x09, 0x60,			/*	0x02B9 : USAGE - Transaction ID (0x60) */
    0x81, 0x02,			/*	0x02BB : INPUT (Data, Var, Abs) */
    /*--- Disconnected : 1bit ---*/
    0x15, 0x00,			/*	0x02BD : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x02BF : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x02C1 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x02C3 : REPORT_COUNT (1) */
    0x0A, 0x70, 0x02,	/*	0x02C5 : USAGE - Disconnected (0x0270) */
    0x81, 0x02,			/*	0x02C8 : INPUT (Data, Var, Abs) */
    /*--- Full : 1bit ---*/
    0x15, 0x00,			/*	0x02CA : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x02CC : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x02CE : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x02D0 : REPORT_COUNT (1) */
    0x0A, 0x71, 0x02,	/*	0x02D2 : USAGE - Full (0x0271) */
    0x81, 0x02,			/*	0x02D5 : INPUT (Data, Var, Abs) */
    /*--- Jam : 1bit ---*/
    0x15, 0x00,			/*	0x02D7 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x02D9 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x02DB : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x02DD : REPORT_COUNT (1) */
    0x0A, 0x72, 0x02,	/*	0x02DF : USAGE - Jam (0x0272) */
    0x81, 0x02,			/*	0x02E2 : INPUT (Data, Var, Abs) */
    /*--- Unused_0 : 1bit ---*/
    0x15, 0x00,			/*	0x02E4 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x02E6 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x02E8 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x02EA : REPORT_COUNT (1) */
    0x0A, 0x67, 0x02,	/*	0x02EC : USAGE - Unused_0 (0x0267) */
    0x81, 0x03,			/*	0x02EF : INPUT (Cnst, Var, Abs) */
    /*--- Unused_1 : 1bit ---*/
    0x15, 0x00,			/*	0x02F1 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x02F3 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x02F5 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x02F7 : REPORT_COUNT (1) */
    0x0A, 0x68, 0x02,	/*	0x02F9 : USAGE - Unused_1 (0x0268) */
    0x81, 0x03,			/*	0x02FC : INPUT (Cnst, Var, Abs) */
    /*--- Unused_2 : 1bit ---*/
    0x15, 0x00,			/*	0x02FE : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x0300 : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x0302 : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0304 : REPORT_COUNT (1) */
    0x0A, 0x6D, 0x02,	/*	0x0306 : USAGE - Unused_2 (0x026D) */
    0x81, 0x03,			/*	0x0309 : INPUT (Cnst, Var, Abs) */
    /*--- Unused_3 : 1bit ---*/
    0x15, 0x00,			/*	0x030B : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x030D : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x030F : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x0311 : REPORT_COUNT (1) */
    0x0A, 0x73, 0x02,	/*	0x0313 : USAGE - Unused_3 (0x0273) */
    0x81, 0x03,			/*	0x0316 : INPUT (Cnst, Var, Abs) */
    /*--- Fault : 1bit ---*/
    0x15, 0x00,			/*	0x0318 : LOGICAL_MINIMUM (0) */
    0x25, 0x01,			/*	0x031A : LOGICAL_MAXIMUM (1) */
    0x75, 0x01,			/*	0x031C : REPORT_SIZE (1) */
    0x95, 0x01,			/*	0x031E : REPORT_COUNT (1) */
    0x0A, 0x74, 0x02,	/*	0x0320 : USAGE - Fault (0x0274) */
    0x81, 0x02,			/*	0x0323 : INPUT (Data, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x0325 : END_COLLECTION (Logical) */

    /* Request GAT Report (合計：10bytes) */
    /*--- GAT Report : 8bits ---*/
    0x09, 0x44,			/*	0x0326 : USAGE - Request GAT Report (0x44) */
    0x85, 0x05,			/*	0x0328 : REPORT_ID (0x05) */
    0x75, 0x08,			/*	0x032A : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x032C : REPORT_COUNT (1) */
    0xB1, 0x03,			/*	0x032E : FEATURE (Cnst, Var, Abs) */

    /* GAT Data (合計：45bytes) */
    0x09, 0x46,			/*	0x0330 : USAGE - GAT Data (0x46) */
    0x85, 0x07,			/*	0x0332 : REPORT_ID (0x07) */
    0xA1, 0x02,			/*	0x0334 : COLLECTION (Logical) */
    /*--- Index : 8bits ---*/
    0x15, 0x01,			/*	0x0336 : LOGICAL_MINIMUM (1) */
    0x26, 0xFF, 0x00,	/*	0x0338 : LOGICAL_MAXIMUM (255) */
    0x75, 0x08,			/*	0x033B : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x033D : REPORT_COUNT (1) */
    0x09, 0x61,			/*	0x033F : USAGE - GAT Index number (0x61) */
    0x81, 0x02,			/*	0x0341 : INPUT (Data, Var, Abs) */
    /*--- Size : 8bits ---*/
    0x15, 0x00,			/*	0x0343 : LOGICAL_MINIMUM (0) */
    0x25, 0x3D,			/*	0x0345 : LOGICAL_MAXIMUM (61) */
    0x75, 0x08,			/*	0x0347 : REPORT_SIZE (8) */
    0x95, 0x01,			/*	0x0349 : REPORT_COUNT (1) */
    0x09, 0x62,			/*	0x034B : USAGE - GAT content size (0x62) */
    0x81, 0x02,			/*	0x034D : INPUT (Data, Var, Abs) */
    /*--- Data : 8bits X 61 ---*/
    0x15, 0x0A,			/*	0x034F : LOGICAL_MINIMUM (10) */
    0x26, 0x7E, 0x00,	/*	0x0351 : LOGICAL_MAXIMUM (126) */
    0x75, 0x08,			/*	0x0354 : REPORT_SIZE (8) */
#if 0 /* 1_SEGMENT_GAT_DATA */
   	0x95, 0x3A,			/*	0x0356 : REPORT_COUNT (58) */
#else
    0x95, 0x3D,			/*	0x0356 : REPORT_COUNT (61) */
#endif
    0x09, 0xB0,			/*	0x0358 : USAGE - Data dump (0xB0) */
    0x81, 0x02,			/*	0x035A : INPUT (Data, Var, Abs) */
    /*------------------------------*/
    0xC0,				/*	0x035C : END_COLLECTION (Logical) */
    0xC0				/*	0x035D : END_COLLECTION (Application) */
};


/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_GetStrDesc2                                      */
/*                                                                          */
/* DESCRIPTION: String Descriptor is returned.                              */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : uData.tDesc             String descriptor information       */
/*                                                                          */
/****************************************************************************/
GRUSB_DEV_STRING_DESC* GRUSB_COMD_HID_GetStrDesc2( VOID )
{

    DLOCAL union {
        struct {
        GRUSB_HID_COMD_STRING_DESC0     tStr0;
        GRUSB_HID_COMD_STRING_DESC1     tStr1;
        GRUSB_HID_COMD_STRING_DESC2     tStr2;
        GRUSB_HID_COMD_STRING_DESC3     tStr3;
        GRUSB_HID_COMD_STRING_DESC4     tStr4;
        GRUSB_HID_COMD_STRING_DESC5     tStr5;
        GRUSB_HID_COMD_STRING_DESC6     tStr6;
        } tDesc;
        UINT32                      dummy;
    } uData = {
/* >>> Sample Data >>> */
        /* String 0 */
        0x04,
        0x03, 
        0x09, 0x04,
        /* String 1 */
        0x08, 
        0x03, 
        0x4A, 0x00, 0x43, 0x00, 0x4D, 0x00,                 /* JCM                   */
        /* String 2 */
        0x10, 
        0x03, 
        'i', 0x00, 'V', 0x00, 'I', 0x00, 'Z', 0x00,         /* iVIZION               */
        'I', 0x00, 'O', 0x00, 'N', 0x00,
        /* String 3 */
        0x1A, 
        0x03, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* (Serial No)           */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        /* String 4 */
        0x16, 
        0x03, 
        0x30, 0x00, 0x78, 0x00, 0x39, 0x00, 0x32, 0x00,     /* 0x92, 0x12            */
        0x2C, 0x00, 0x20, 0x00, 0x30, 0x00, 0x78, 0x00,
        0x31, 0x00, 0x32, 0x00,
        /* String 5 */
        0x10, 
        0x03, 
        0x44, 0x00, 0x46, 0x00, 0x55, 0x00, 0x20, 0x00,     /* DFU I/F               */
        0x49, 0x00, 0x2F, 0x00, 0x46, 0x00,
        /* String 6 */
        0x7E, 
        0x03, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,     /* (Firmware Informaion) */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
        
/* <<< Sample Data <<< */
    };
    
    GRCOMD_HID_CNF_LOG2( GRDBG_COMD_CNF, 0x00, 0x00, END_FUNC );
    
    return  (GRUSB_DEV_STRING_DESC*)(&uData.tDesc);
}

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_HID_GetReportDesc2                               */
/*                                                                          */
/* DESCRIPTION: String Descriptor is returned.                              */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : uData.tDesc             String descriptor information       */
/*                                                                          */
/****************************************************************************/
UINT8* GRUSB_COMD_HID_GetReportDesc2( UINT16* pusSize )
{

    *pusSize = GRCOMD_HID_REPORT_DESC_SIZE;

    return HidReportDesc_tbl_GSA;
};

