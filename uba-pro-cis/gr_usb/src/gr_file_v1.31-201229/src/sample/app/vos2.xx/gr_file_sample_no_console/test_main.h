/************************************************************************************************/
/*                                                                                              */
/*                              Copyright(C) 2011 Grape Systems, Inc.                           */
/*                                       All Rights Reserved                                    */
/*                                                                                              */
/* This software is furnished under a license and may be used and copied only in accordance     */
/* with the terms of such license and with the inclusion of the above copyright notice.         */
/* No title to and ownership of the software is transferred. Grape Systems Inc. makes no        */
/* representation or warranties with respect to the performance of this computer program, and   */
/* specifically disclaims any responsibility for any damages, special or consequential,         */
/* connected with the use of this program.                                                      */
/*                                                                                              */
/************************************************************************************************/
/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      test_main.h                                                             0.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      USBマスストレージ統合キット用のテスト環境のヘッダファイル                               */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2011/06/21  V0.01                                                           */
/*                              Created initial version                                         */
/*                                                                                              */
/************************************************************************************************/

#ifndef _TEST_MAIN_H_
#define _TEST_MAIN_H_

extern int GRUSB_Test_Stack_Init(void);

#define USB_TEST_OK                     0
#define USB_TEST_NG                     -1

#endif /*_TEST_MAIN_H_*/
