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
 * @file MGU_struct.h
 * @brief 共通構造体定義
 * @date 2021/12/23 Created.
 */
/****************************************************************************/
#ifndef __MGU_STRUCT_H_INCLUDED__
#define __MGU_STRUCT_H_INCLUDED__

/**** STRUCTURES ********************************************************************************/
#pragma push
#pragma pack(1)


/*----------------------------------------------------------------------*/
/* MGU memory struct	                                                */
/*----------------------------------------------------------------------*/

typedef struct _MGU_MEMORY_BLOCK0
{
    					/*  address,  size */
	u16 revision;		/*       0,      2 */
	u32 reset_time;		/*       2,      4 */
	u8 flag;			/*       6,      1 */
	u8 hm_flag;			/*       7,      1 */
	u8 sm_flag;			/*       8,      1 */
	u8 reserved;		/*       9,      1 */
	u32 hm_cur_cnt;		/*      10,      4 */
	u32 hm_total_cnt;	/*      14,      4 */
	u32 sm_cur_cnt;		/*      18,      4 */
	u32 sm_total_cnt;	/*      22,      4 */
	u8 sum;
}  __attribute__((packed)) MGU_MEMORY_BLOCK0;
typedef struct _MGU_MEMORY_BLOCK1
{
    					/*  address,  size */
	u8 product_sn[20];	/*      20,      2 */
	u8 sum;
}  __attribute__((packed)) MGU_MEMORY_BLOCK1;
typedef struct _MGU_MEMORY_BLOCK2
{
    					/*  address,  size */
	u32 hm_tmp_log[7];
	u32 sm_tmp_log[7];
	u32 hm_ope_log[10];
	u32 sm_ope_log[10];
	u8 sum[10];
}  __attribute__((packed)) MGU_MEMORY_BLOCK2;

typedef struct _MGU_MEMORY_AREA
{
	/* 生産時に書き込み */
	u8 mgu_sn[20];
	/* メンテ時に書き込み */
	MGU_MEMORY_BLOCK0[10];
	/* 製品起動時に書込み */
	MGU_MEMORY_BLOCK1[10];
	/* 通券時に書き込み */
	MGU_MEMORY_BLOCK2[10];
}  __attribute__((packed)) MGU_MEMORY_AREA;
#endif
