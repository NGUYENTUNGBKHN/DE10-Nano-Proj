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
 * @file system_timer.h
 * @brief システムタイマ ヘッダファイル
 * @date 2018.03.05 Created
 */
/****************************************************************************/
#if !defined(__SYSTEM_TIMER_H_INCLUDEED__)
#define __SYSTEM_TIMER_H_INCLUDEED__

enum {
	SYSTEM_TIMER_TICK_MAX = 10,
};

/****************************************************************/
/*						関数宣言								*/
/****************************************************************/
u64 system_timer_read(void);
s64 system_timer_get_local_us(void);
u32 system_timer_get_timestamp(void);		// 20/03/19
struct tm *system_timer_get_localtime(struct tm *time);

#endif /* __SYSTEM_TIMER_H_INCLUDEED__ */

/* End of file */

