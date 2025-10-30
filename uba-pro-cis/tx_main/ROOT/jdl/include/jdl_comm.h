/****************************************************************************/
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2017                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
 * MODEL NAME : （モデル名）
 * @file jdl.h
 * @brief  JCM Device Log Header
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/
#pragma once

#include "typedefine.h"

/*==========================================================================*/
/*==========================================================================*/
/* JDL Common Version                                                       */
/*==========================================================================*/
#define JDL_COMM_VER  0x0001 /* Ver 0.0.0.1 */


/*==========================================================================*/
/*==========================================================================*/
/* Error code                                                               */
/*==========================================================================*/
#define JDL_E_OK       0    /*  */

#define JDL_E_REVCHG   0x01  /* 0x01 : Revision changed */
#define JDL_E_IDCHG    0x02  /* 0x02 : ID changed */
#define JDL_E_DCORR    0x03  /* 0x03 : Data corrupted */
#define JDL_E_VERCHG   0x04  /* 0x04 : Version changed */

#define JDL_E_MACV     0x20  /* 0x20 : Memory access violation */
#define JDL_E_SAVE     0x21  /* 0x21 : Save Error */

#define JDL_E_PARAM    0x30  /* 0x30 : Parameter Error */

#define JDL_E_COVR     0x50  /* 0x50 : Counter overflow */
#define JDL_E_CMAX     0x51  /* 0x51 : Counter was maximum value */
#define JDL_E_BFULL    0x53  /* 0x53 : Buffer was full */
#define JDL_E_BEMPTY   0x54  /* 0x54 : Buffer was empty */

#define JDL_E_DDATA    0x60  /* 0x60 : Data different */
#define JDL_E_DISABLE  0x61  /* 0x61 : JDL disable */

#define JDL_E_CONF     0xF0  /* 0xF0 : Configuration error */

#define JDL_E_BUSY     0xFF  /* 0xFF : Busy */


/*==========================================================================*/
/*==========================================================================*/
/* JDL Mode                                                                 */
/*==========================================================================*/
#define JDL_MODE_DISABLE  0x00
#define JDL_MODE_ENABLE   0x01


/*==========================================================================*/
/*==========================================================================*/
/* Data Type & Size                                                         */
/*==========================================================================*/
#define JDL_DATA_TYPE_SIZE_BYTE   1
#define JDL_DATA_TYPE_SIZE_WORD   2
#define JDL_DATA_TYPE_SIZE_DWORD  4
#define JDL_DATA_TYPE_SIZE_QWORD  8


/*==========================================================================*/
/*----------------------------------------------------------*/
/* NULL Pointer                                             */
/*----------------------------------------------------------*/
#define JDL_NULL ((void *) 0)

/*==========================================================================*/
/*----------------------------------------------------------*/
/* Check sum size.                                          */
/*----------------------------------------------------------*/
#define JDL_SIZE_CHECKSUM 2

/*==========================================================================*/
/*----------------------------------------------------------*/
/* CRC 16 size.                                             */
/*----------------------------------------------------------*/
#define JDL_SIZE_CRC16 2

/*==========================================================================*/
/*----------------------------------------------------------*/
/* CRC 32 size.                                             */
/*----------------------------------------------------------*/
#define JDL_SIZE_CRC32 4

/*==========================================================================*/
/*----------------------------------------------------------*/
/* Model name size                                          */
/*----------------------------------------------------------*/
#define JDL_SIZE_MODEL_NAME 32


/*==========================================================================*/
/*----------------------------------------------------------*/
/* Serial Number size                                       */
/*----------------------------------------------------------*/
#define JDL_SIZE_SERIAL_NO 12


/*==========================================================================*/
/*----------------------------------------------------------*/
/* Firmware version size                                    */
/*----------------------------------------------------------*/
#define JDL_SIZE_FIRM_VER 64


/*==========================================================================*/
/*----------------------------------------------------------*/
/* Firmware version size                                    */
/*----------------------------------------------------------*/
#define JDL_SIZE_BOOT_VER 16


/*==========================================================================*/
/*----------------------------------------------------------*/
/* Protocol ID size                                         */
/*----------------------------------------------------------*/
#define JDL_SIZE_PROTOCOL_ID 4


/*==========================================================================*/
/*----------------------------------------------------------*/
/* Buffer round counter size                                */
/*----------------------------------------------------------*/
#define JDL_SIZE_BUFF_ROUND 1


/*==========================================================================*/
/*----------------------------------------------------------*/
/* Event code size                                          */
/*----------------------------------------------------------*/
#define JDL_SIZE_EVEN_CORD 1


/*==========================================================================*/
/*----------------------------------------------------------*/
/* Task ID size                                             */
/*----------------------------------------------------------*/
#define JDL_SIZE_TID 1



/*==========================================================================*/
/*==========================================================================*/
/* Struct                                                                   */
/*==========================================================================*/
/*----------------------------------------------------------*/
/* File Header                                              */
/*----------------------------------------------------------*/
#define JDL_SIZE_FILE_HEADER     64            /* [File Header SIZE:64] */
#define JDL_FHEAD_SIZE_NAME      16                        /* [SIZE:16] */
#define JDL_FHEAD_SIZE_REV       JDL_DATA_TYPE_SIZE_WORD   /* [SIZE: 2] */
#define JDL_FHEAD_SIZE_FSIZE     JDL_DATA_TYPE_SIZE_DWORD  /* [SIZE: 4] */
#define JDL_FHEAD_SIZE_PID       JDL_DATA_TYPE_SIZE_WORD   /* [SIZE: 2] */

#define JDL_FHEAD_ADR_NAME       0                                            /*   0 */
#define JDL_FHEAD_ADR_REV        (JDL_FHEAD_ADR_NAME + JDL_FHEAD_SIZE_NAME)   /*  16 */
#define JDL_FHEAD_ADR_FSIZE      (JDL_FHEAD_ADR_REV + JDL_FHEAD_SIZE_REV)     /*  18 */
#define JDL_FHEAD_ADR_PID        (JDL_FHEAD_ADR_FSIZE + JDL_FHEAD_SIZE_FSIZE) /*  22 */
#define JDL_FHEAD_ADR_RESERVED   (JDL_FHEAD_ADR_PID + JDL_FHEAD_SIZE_PID)     /*  24 */


/*----------------------------------------------------------*/
/* Category Header                                          */
/*----------------------------------------------------------*/
#define JDL_SIZE_CATEGORY_HEADER 32        /* [Category Header SIZE:32] */
#define JDL_CHEAD_SIZE_NAME      16                        /* [SIZE:16] */
#define JDL_CHEAD_SIZE_ID        JDL_DATA_TYPE_SIZE_WORD   /* [SIZE: 2] */
#define JDL_CHEAD_SIZE_REV       JDL_DATA_TYPE_SIZE_WORD   /* [SIZE: 2] */
#define JDL_CHEAD_SIZE_CSIZE     JDL_DATA_TYPE_SIZE_DWORD  /* [SIZE: 4] */

/* Address of each item */
#define JDL_CHEAD_ADR_NAME       0                                            /*   0 */
#define JDL_CHEAD_ADR_ID         (JDL_CHEAD_ADR_NAME + JDL_CHEAD_SIZE_NAME)   /*  16 */
#define JDL_CHEAD_ADR_REV        (JDL_CHEAD_ADR_ID + JDL_CHEAD_SIZE_ID)       /*  18 */
#define JDL_CHEAD_ADR_CSIZE      (JDL_CHEAD_ADR_REV + JDL_CHEAD_SIZE_REV)     /*  20 */
#define JDL_CHEAD_ADR_RESERVED   (JDL_CHEAD_ADR_CSIZE + JDL_CHEAD_SIZE_CSIZE) /*  24 */


/*----------------------------------------------------------*/
/* Time structure                                           */
/*----------------------------------------------------------*/
#define JDL_SIZE_TIME 8
typedef struct _JDL_TIME
{
    u32 high;
    u32 low;
} JDL_TIME;


/*==========================================================================*/
/*==========================================================================*/
/* Common functions                                                         */
/*==========================================================================*/
extern void jdl_tick(void);
extern u8 _jdl_init(void);
extern u8 _jdl_set_mode(u8 mode);
extern u8 _jdl_get_mode(void);
extern void _jdl_get_rev(u16 *rev);
extern void _jdl_get_10m_etime(u32 *time);
extern void _jdl_get_100m_etime(u32 *time);
extern void _jdl_get_1s_ctime(JDL_TIME *ctime);
extern void _jdl_get_time(JDL_TIME *stime, JDL_TIME *etime);
extern void _jdl_set_time(JDL_TIME *stime, JDL_TIME *etime);
extern void _jdl_draw_ener_time(u32 *time);
extern u8 _jdl_inc_8bit(u32 offset, u8 *data);
extern u8 _jdl_inc_16bit(u32 offset, u16 *data);
extern u8 _jdl_inc_32bit(u32 offset, u32 *data);
//extern u8 _jdl_inc_64bit(u32 offset, u32 *data, u32 *data_2);
extern u8 _jdl_add_8bit(u32 offset, u8 add_data, u8 *sum_data);
extern u8 _jdl_add_16bit(u32 offset, u16 add_data, u16 *sum_data);
extern u8 _jdl_add_32bit(u32 offset, u32 add_data, u32 *sum_data);
//extern u8 _jdl_add_64bit(u32 offset, u32 add_data1, u32 add_data2, u32 *sum_data1, u32 *sum_data2);
extern void _jdl_memset(u8 *ptr, u8 data, u32 size);
extern void _jdl_memcpy(u8 *dptr, u8 *sptr, u32 size);







