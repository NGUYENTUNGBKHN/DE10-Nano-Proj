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
 * @file jdl_category_id.h
 * @brief  JCM Device Log Category ID Header
 * @date 2018.10.29
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/
#pragma once

/*----------------------------------------------------------*/
/* Category ID                                              */
/*----------------------------------------------------------*/
#define JDL_CATE_ID_ALL           0x0000
/* Standard Category */
#define JDL_CATE_ID_SYSTEM        0x0001
#define JDL_CATE_ID_STATISTICS    0x0002
#define JDL_CATE_ID_SENSOR        0x0003
#define JDL_CATE_ID_COMMUNICATION 0x0004
#define JDL_CATE_ID_EVENT         0x0005
#define JDL_CATE_ID_ERROR         0x0006
#define JDL_CATE_ID_ACCEPTANCE    0x0007
#define JDL_CATE_ID_POSIANA       0x0008

/* Additional Category */
#define JDL_CATE_ID_AD_RC         0x0100
#define JDL_CATE_ID_AD_ESCROW     0x0101


#if defined(UBA_RC) || defined(UBA_RTQ)
/* Option Unit Category */
#define JDL_CATE_ID_OP_RC         0x0200
#define JDL_CATE_ID_OP_ESCROW     0x0201
#endif

/* Hidden command for JDL clear */
#define JDL_CATE_ID_CLEAR_JDL     0xFFFF



