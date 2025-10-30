/************************************************************************************************/
/*                                                                                              */
/*                              Copyright(C) 2020 Grape Systems, Inc.                           */
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
/*      test_apl.h                                                              0.90            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      STM32Fシリーズにインタラプト転送を追加するための、調査用のサンプルアプリケーション      */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2020/04/17  V0.90                                                           */
/*                            - Created beta version                                            */
/*                                                                                              */
/************************************************************************************************/

#ifndef _HOST_APL_H_
#define _HOST_APL_H_

/**** INTERNAL DATA DEFINES *********************************************************************/
/* error code */
#define     USB_HOST_OK           0
#define     USB_HOST_NG        -1

// Default Value???
//#define TEST_VENDOR_ID              0x04B8          /* venderID                                 */
//#define TEST_PRODUCT_ID             0x0839          /* productID                                */
// Elecom USB HUB
#define TEST_VENDOR_ID              0x0409          /* venderID                                 */
#define TEST_PRODUCT_ID             0x0059          /* productID                                */
// Sony USB Memory
//#define TEST_VENDOR_ID              0x054c          /* venderID                                 */
//#define TEST_PRODUCT_ID             0x05ba          /* productID                                */

#define TEST_CONNECT                0x01
#define TEST_DISCONNECT             0x80

/* Prototype declaration */
int GRUSB_Test_Stack_Init(void);

#define USB_TEST_OK                     0
#define USB_TEST_NG                     -1


#endif /* _HOST_APL_H_ */
