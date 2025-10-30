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
/*      com_dfu_cnf.c                                             1.00      */
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

/**** INCLUDE FILES *********************************************************/
#include    "com_dfu_def.h"

#ifdef GRCOMD_DFU_CNF_DEBUG
#include    "dbg_mdl.h"
#define GRCOMD_DFU_CNF_LOG2(m,n,x,y)  GRDBG_TRACE2(m,n,x,y)
#else
#define GRCOMD_DFU_CNF_LOG2(m,n,x,y)
#define GRDBG_COMD_CNF                  (0x00)
#define END_FUNC                        (0x00)
#endif

/* Number of characters for Manufacturer Name */
#define GRCOMD_DFU_MANUFACTURER_CHARS       (3)
/* Number of characters for Product Name */
#define GRCOMD_DFU_PRODUCT_CHARS            (7) 
/* Number of characters for Serial Number */
#define GRCOMD_DFU_SERIALNUMBER_CHARS       (12)

/* Number of characters for ROM File */
#define GRCOMD_DFU_ROMFILE_CHARS            (16)

/* Number of characters for DFU I/F */
#define GRCOMD_DFU_DFU_IF_CHARS             (7)

/* Number of characters for DFU I/F */
#define GRCOMD_DFU_IF_CHARS                 (124)


/* String Descriptor 0 Structure */
#define GRCOMD_DFU_STRDESC_0_LANGID_SIZE    (2)
typedef struct grusb_comd_string_desc0_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_DFU_STRDESC_0_LANGID_SIZE];   /* bString(UNICODE encoded string)  */
} GRUSB_DFU_COMD_STRING_DESC0;

/* String Descriptor 1 Structure */
#define GRCOMD_DFU_STRDESC_1_STR_SIZE       (GRCOMD_DFU_MANUFACTURER_CHARS * 2)
typedef struct grusb_comd_string_desc1_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_DFU_STRDESC_1_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_DFU_COMD_STRING_DESC1;

/* String Descriptor 2 Structure */
#define GRCOMD_DFU_STRDESC_2_STR_SIZE       (GRCOMD_DFU_PRODUCT_CHARS * 2)
typedef struct grusb_comd_string_desc2_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_DFU_STRDESC_2_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_DFU_COMD_STRING_DESC2;

/* String Descriptor 3 Structure */
#define GRCOMD_DFU_STRDESC_3_STR_SIZE       (GRCOMD_DFU_SERIALNUMBER_CHARS * 2)
typedef struct grusb_comd_string_desc3_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_DFU_STRDESC_3_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_DFU_COMD_STRING_DESC3;

/* String Descriptor 4 Structure */
#define GRCOMD_DFU_STRDESC_4_STR_SIZE       (GRCOMD_DFU_ROMFILE_CHARS * 2)
typedef struct grusb_comd_string_desc4_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_DFU_STRDESC_4_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_DFU_COMD_STRING_DESC4;

/* String Descriptor 5 Structure */
#define GRCOMD_DFU_STRDESC_5_STR_SIZE       (GRCOMD_DFU_DFU_IF_CHARS * 2)
typedef struct grusb_comd_string_desc5_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_DFU_STRDESC_5_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_DFU_COMD_STRING_DESC5;

/* String Descriptor 6 Structure */
#define GRCOMD_DFU_STRDESC_6_STR_SIZE       (GRCOMD_DFU_IF_CHARS * 2)
typedef struct grusb_comd_string_desc6_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_DFU_STRDESC_6_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_DFU_COMD_STRING_DESC6;


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
GRUSB_DEV_STRING_DESC* GRUSB_COMD_DFU_GetStrDesc2( VOID )
{

    DLOCAL union {
        struct {
        GRUSB_DFU_COMD_STRING_DESC0     tStr0;
        GRUSB_DFU_COMD_STRING_DESC1     tStr1;
        GRUSB_DFU_COMD_STRING_DESC2     tStr2;
        GRUSB_DFU_COMD_STRING_DESC3     tStr3;
        GRUSB_DFU_COMD_STRING_DESC4     tStr4;
        GRUSB_DFU_COMD_STRING_DESC5     tStr5;
        GRUSB_DFU_COMD_STRING_DESC6     tStr6;
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
        0x31, 0x00, 0x38, 0x00, 0x30, 0x00, 0x31, 0x00,     /* (Serial No)           */
        0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00,
        0x30, 0x00, 0x31, 0x00, 0x30, 0x00, 0x33, 0x00,
        /* String 4 */
        0x22, 
        0x03, 
        0x44, 0x00, 0x46, 0x00, 0x55, 0x00, 0x20, 0x00,     /* DFU Program Mode      */
        0x50, 0x00, 0x72, 0x00, 0x6F, 0x00, 0x67, 0x00,
        0x72, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x20, 0x00,
        0x4D, 0x00, 0x6F, 0x00, 0x64, 0x00, 0x65, 0x00,
        /* String 5 */
        0x10, 
        0x03, 
        0x44, 0x00, 0x46, 0x00, 0x55, 0x00, 0x20, 0x00,     /* DFU I/F               */
        0x49, 0x00, 0x2F, 0x00, 0x46, 0x00,
        /* String 6 */
        0x7E, 
        0x03, 
        0x31, 0x00, 0x2E, 0x00, 0x31, 0x00, 0x2E, 0x00,     /* (Firmware Informaion) */
        0x32, 0x00, 0x2C, 0x00, 0x69, 0x00, 0x56, 0x00,
        0x49, 0x00, 0x5A, 0x00, 0x49, 0x00, 0x4F, 0x00,
        0x4E, 0x00, 0x2D, 0x00, 0x31, 0x00, 0x30, 0x00,
        0x30, 0x00, 0x2D, 0x00, 0x53, 0x00, 0x53, 0x00,
        0x2C, 0x00, 0x69, 0x00, 0x28, 0x00, 0x55, 0x00,
        0x53, 0x00, 0x41, 0x00, 0x29, 0x00, 0x31, 0x00,
        0x30, 0x00, 0x30, 0x00, 0x2D, 0x00, 0x53, 0x00,
        0x53, 0x00, 0x20, 0x00, 0x49, 0x00, 0x44, 0x00,
        0x30, 0x00, 0x47, 0x00, 0x38, 0x00, 0x2D, 0x00,
        0x30, 0x00, 0x31, 0x00, 0x2C, 0x00, 0x56, 0x00,
        0x32, 0x00, 0x2E, 0x00, 0x38, 0x00, 0x39, 0x00,
        0x2D, 0x00, 0x34, 0x00, 0x32, 0x00, 0x2C, 0x00,
        0x32, 0x00, 0x30, 0x00, 0x32, 0x00, 0x31, 0x00,
        0x2D, 0x00, 0x30, 0x00, 0x38, 0x00, 0x2D, 0x00,
        0x33, 0x00, 0x31, 0x00
        
/* <<< Sample Data <<< */
    };
    
    GRCOMD_DFU_CNF_LOG2( GRDBG_COMD_CNF, 0x00, 0x00, END_FUNC );
    
    return  (GRUSB_DEV_STRING_DESC*)(&uData.tDesc);
}


