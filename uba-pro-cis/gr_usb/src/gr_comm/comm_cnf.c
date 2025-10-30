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
/*      comm_cnf.c                                                1.41      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This file perfoems Abstract Control Model of Communication Device   */
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
/*                          version was updated                             */
/*   K.Takagi   2006/08/31  V1.22                                           */
/*                          version was updated                             */
/*   K.Handa    2006/09/26  V1.23                                           */
/*                          version was updated                             */
/*   K.handa    2006/12/27  V1.30                                           */
/*                          version was updated                             */
/*   K.handa    2007/06/12  V1.31                                           */
/*                          version was updated                             */
/*   M.Suzuki   2015/03/04  V1.40                                           */
/*                          Added the definition of size to String          */
/*                          Descriptor structure.                           */
/*                          Deleted the const of uData union.               */
/*                          Added the sample data of String Descriptor.     */
/*   M.Suzuki   2017/11/24  V1.41                                           */
/*                          Modified the macro definition for debug log.    */
/*                          - GRCOMD_CNF_LOG                                */
/*                                                                          */
/****************************************************************************/

/**** INCLUDE FILES *********************************************************/
#include    "comm_def.h"

#ifdef GRCOMD_CNF_DEBUG
#include    "dbg_mdl.h"
#define GRCOMD_CNF_LOG(m,n,x,y)  GRDBG_TRACE(m,n,x,y)
#else
#define GRCOMD_CNF_LOG(m,n,x,y)
#define GRDBG_COMD_CNF                  (0x00)
#define END_FUNC                        (0x00)
#endif

/* Number of characters for Manufacturer Name */
#define GRCOMD_MANUFACTURER_CHARS       (5)
/* Number of characters for Product Name */
#define GRCOMD_PRODUCT_CHARS            (8) 
/* Number of characters for Serial Number */
#define GRCOMD_SERIALNUMBER_CHARS       (12)

/* String Descriptor 0 Structure */
#define GRCOMD_STRDESC_0_LANGID_SIZE    (2)
typedef struct grusb_comd_string_desc0_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_STRDESC_0_LANGID_SIZE];   /* bString(UNICODE encoded string)  */
} GRUSB_COMD_STRING_DESC0;

/* String Descriptor 1 Structure */
#define GRCOMD_STRDESC_1_STR_SIZE       (GRCOMD_MANUFACTURER_CHARS * 2)
typedef struct grusb_comd_string_desc1_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_STRDESC_1_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_COMD_STRING_DESC1;

/* String Descriptor 2 Structure */
#define GRCOMD_STRDESC_2_STR_SIZE       (GRCOMD_PRODUCT_CHARS * 2)
typedef struct grusb_comd_string_desc2_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_STRDESC_2_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_COMD_STRING_DESC2;

/* String Descriptor 3 Structure */
#define GRCOMD_STRDESC_3_STR_SIZE       (GRCOMD_SERIALNUMBER_CHARS * 2)
typedef struct grusb_comd_string_desc3_tag
{
    UINT8       ucStr1;                                 /* bLength                          */
    UINT8       ucStr2;                                 /* bDescriptorType                  */
    UINT8       ucStr3[GRCOMD_STRDESC_3_STR_SIZE];      /* bString(UNICODE encoded string)  */
} GRUSB_COMD_STRING_DESC3;

/****************************************************************************/
/* FUNCTION   : GRUSB_COMD_GetStrDesc                                       */
/*                                                                          */
/* DESCRIPTION: String Descriptor is returned.                              */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : uData.tDesc             String descriptor information       */
/*                                                                          */
/****************************************************************************/
GRUSB_DEV_STRING_DESC* GRUSB_COMD_GetStrDesc( VOID )
{
    DLOCAL union {
        struct {
        GRUSB_COMD_STRING_DESC0     tStr0;
        GRUSB_COMD_STRING_DESC1     tStr1;
        GRUSB_COMD_STRING_DESC2     tStr2;
        GRUSB_COMD_STRING_DESC3     tStr3;
        } tDesc;
        UINT32                      dummy;
    } uData = {
/* >>> Sample Data >>> */
        /* String 0 */
        0x04,
        0x03, 
        0x09, 0x04,
        /* String 1 */
        0x0C, 
        0x03, 
        0x47, 0x00, 0x72, 0x00, 0x61, 0x00, 0x70, 0x00,         /* Grape         */
        0x65, 0x00, 
        /* String 2 */
        0x12, 
        0x03, 
        0x43, 0x00, 0x4F, 0x00, 0x4D, 0x00, 0x20, 0x00,         /* COM Port      */
        0x50, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x74, 0x00, 
        /* String 3 */
        0x1A, 
        0x03, 
        0x30, 0x00, 0x31, 0x00, 0x31, 0x00, 0x34, 0x00,         /* 0114a  */
        0x61, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
/* <<< Sample Data <<< */
    };
    
    GRCOMD_CNF_LOG( GRDBG_COMD_CNF, 0x00, 0x00, END_FUNC );
    
    return  (GRUSB_DEV_STRING_DESC*)(&uData.tDesc);
}
