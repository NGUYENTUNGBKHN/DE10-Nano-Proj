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
 * @file jdl_event_code.h
 * @brief  JCM Device Log Event Code Header
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/
#pragma once

/*----------------------------------------------------------*/
/* Event Code                                               */
/*----------------------------------------------------------*/
#define JDL_EVEN_CODE_POWERUP    0x01

#define JDL_EVEN_CODE_RESET      0x10

#define JDL_EVEN_CODE_ACC        0x20
#define JDL_EVEN_CODE_REJ        0x21

#define JDL_EVEN_CODE_RC_ACC     0xA0
#define JDL_EVEN_CODE_RC_PAY     0xA1
#define JDL_EVEN_CODE_RC_COL     0xA2
#define JDL_EVEN_CODE_RC_RETRY   0xA3

#define JDL_EVEN_CODE_ESC_ACC    0xB0
#define JDL_EVEN_CODE_ESC_CAN    0xB1
#define JDL_EVEN_CODE_ESC_STK    0xB2

#define JDL_EVEN_CODE_ERR        0xF0
#define JDL_EVEN_CODE_ERR_RC     0xF1
#define JDL_EVEN_CODE_ERR_ESC    0xF2

#define JDL_EVEN_CODE_ERR_WDT    0xFF









