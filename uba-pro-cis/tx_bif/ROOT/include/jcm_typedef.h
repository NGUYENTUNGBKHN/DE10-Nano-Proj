/****************************************************************************/
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
#if !defined(_JCM_TYPEDEF_H_INCLUDED_)
#define _JCM_TYPEDEF_H_INCLUDED_

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef	signed long long	s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef	unsigned long long 	u64;

#if !defined(__cplusplus)
#ifndef bool
typedef unsigned int bool;
#endif
#endif

typedef unsigned int bool32;
typedef float float32;
typedef unsigned int UArg32;
typedef void* HANDLE;

#if !defined(true)
#define	true	1
#endif

#if !defined(false)
#define	false	0
#endif

#if !defined(null)
#define null (0)
#endif

#if !defined(NULL)
#define NULL (null)
#endif

#if !defined(S32_SUCCESS)
#define S32_SUCCESS 	(0)
#endif

#if !defined(S32_ERROR)
#define S32_ERROR		(-1)
#endif

#if !defined(INFINITE)
#define INFINITE		(unsigned int)(-1)
#endif


#define ON			(1)							/* ON */
#define OFF			(0)							/* OFF */



#endif /* _JCM_TYPEDEF_H_INCLUDED_ */

/* End of File */

