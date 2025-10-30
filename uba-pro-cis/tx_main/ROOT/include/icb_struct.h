/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
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
 * @file icb_struct.h
 * @brief 共通構造体定義
 * @date 2021/12/23 Created.
 */
/****************************************************************************/
#ifndef __ICB_STRUCT_H_INCLUDED__
#define __ICB_STRUCT_H_INCLUDED__

/**** STRUCTURES ********************************************************************************/
#pragma push
#pragma pack(1)


/*----------------------------------------------------------------------*/
/* ICB memory struct	                                                */
/*----------------------------------------------------------------------*/
typedef struct _ICB_MEMORY_AREA1
{
                       /*  address, block,  size */
    u16 denomi[20];    /*        0,     0,    40 */
    u16 reerved;       /*       40,    10,     2 */
    u16 total;         /*       42,    10,     2 */
    u16 coupon;        /*       44,    11,     2 */
    u16 totalin;       /*       46,    11,     2 */
    u8  cinfo[20];     /*       48,    12,    20 */
    u8  err[20];       /*       68,    17,    20 */
    u8  gameno[20];    /*       88,    22,    20 */
    u8  boxno[20];     /*      108,    27,    20 */
    u8  ver[8];        /*      128,    32,     8 */
    u8  rw_ver[20];    /*      136,    34,    20 */
    u32 reset_time;    /*      156,    39,     4 */
    u32 set_time;      /*      160,    40,     4 */
    u32 init_time;     /*      164,    41,     4 */
    u8  flg;           /*      168,    42,     1 */
    u8  assign;        /*      169,    42,     1 */
    u8  id;            /*      170,    42,     1 */
    u8  sum;           /*      171,    42,     1 */
}  __attribute__((packed)) ICB_MEMORY_AREA1;
typedef struct _ICB_MEMORY_AREA2
{
                       /*  address, block,  size */
    u8  rej[3][5];     /*      172,    43,    15 */
    u8  reerved;       /*      187,    46,     1 */
    u8  crncy[20];     /*      188,    47,    20 */
    u8  assign[4];     /*      208,    52,     4 */
    u8  ticket_rej[8]; /*      212,    53,     8 */
    u8  model;         /*      220,    55,     3 */
    u8  revision;      /*      223,    55,     1 */
    u8  serial[6];     /*      224,    56,     6 */
    u8  box;           /*      230,    57,     1 */
    u8  sum2;          /*      231,    57,     1 */
}  __attribute__((packed)) ICB_MEMORY_AREA2;
typedef struct _ICB_MEMORY
{
	ICB_MEMORY_AREA1 area1;
	ICB_MEMORY_AREA2 area2;
}  __attribute__((packed)) ICB_MEMORY;
#pragma pop
#endif /* __ICB_STRUCT_H_INCLUDED__ */

