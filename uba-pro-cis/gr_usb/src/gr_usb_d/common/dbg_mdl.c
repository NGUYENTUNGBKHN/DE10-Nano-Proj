/****************************************************************************/
/*                                                                          */
/*              Copyright(C) 2002-2006 Grape Systems, Inc.                  */
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
/*      dbg_mdl.c                                                 1.10      */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      デバッグ用の関数ログ機能                                            */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*                                                                          */
/*  K.Takagi    2003/11/26  Created initial version 1.00                    */
/*  K.Handa     2006/12/27  Version was updated to 1.10                     */
/*                                                                          */
/****************************************************************************/

/**** INCLUDE FILES *********************************************************/
#include    "dbg_mdl.h"

/**** INTERNAL DATA DEFINES *************************************************/
#ifdef GR_DBG_MDL

#if GR_DBG_MDL_8BIT_MODE
/*----- 8bitのデータを保存することが出来ます -----*/
int     g_iDbgMdlCnt = 0;                           /* counter              */
unsigned char   g_aucDbgMblTbl[DBG_MAX_TRACE][4];   /* Trace Buffer         */

#elif GR_DBG_MDL_16BIT_MODE
/*----- 16bitのデータを保存することが出来ます -----*/
int     g_iDbgMdlCnt = 0;                           /* counter              */
unsigned short  g_ausDbgMblTbl[DBG_MAX_TRACE][4];   /* Trace Buffer         */

#elif GR_DBG_MDL_32BIT_MODE
/*----- 32bitのデータを保存することが出来ます-----*/
int     g_iDbgMdlCnt = 0;                           /* counter              */
unsigned long   g_aulDbgMblTbl[DBG_MAX_TRACE][4];   /* Trace Buffer         */

#endif  /* MODE設定 */
#endif  /* GR_DBG_MDL */

#ifdef GR_DBG_MDL
#if GR_DBG_MDL_8BIT_MODE
/************************************************************************/
/*  FUNCTION    : GRDBG_DebugTrace                                      */
/*                                                                      */
/*  DESCRIPTION : The Trace information for Debug is set.               */
/*----------------------------------------------------------------------*/
/*  INPUT       : ucCode        Module Code                             */
/*                ucFncCode     Parameter 1                             */
/*                ucPrm1        Parameter 2                             */
/*                ucPrm2        Parameter 3                             */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  RESULT      : None                                                  */
/************************************************************************/
void GRDBG_DebugTrace( unsigned char    ucCode,
                       unsigned char    ucFncCode,
                       unsigned char    ucPrm1,
                       unsigned char    ucPrm2)
{
    /* データをメモリ上に保存 */
    g_aucDbgMblTbl[g_iDbgMdlCnt][0] = ucCode;
    g_aucDbgMblTbl[g_iDbgMdlCnt][1] = ucFncCode;
    g_aucDbgMblTbl[g_iDbgMdlCnt][2] = ucPrm1;
    g_aucDbgMblTbl[g_iDbgMdlCnt][3] = ucPrm2;

    /* カウンタの更新 */
    g_iDbgMdlCnt = (g_iDbgMdlCnt+1) % DBG_MAX_TRACE;
}

#elif GR_DBG_MDL_16BIT_MODE
/************************************************************************/
/*  FUNCTION    : GRDBG_DebugTrace                                      */
/*                                                                      */
/*  DESCRIPTION : The Trace information for Debug is set.               */
/*----------------------------------------------------------------------*/
/*  INPUT       : usCode        Module Code                             */
/*                usFncCode     Parameter 1                             */
/*                usPrm1        Parameter 2                             */
/*                usPrm2        Parameter 3                             */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  RESULT      : None                                                  */
/************************************************************************/
void GRDBG_DebugTrace( unsigned short   usCode,
                       unsigned short   usFncCode,
                       unsigned short   usPrm1,
                       unsigned short   usPrm2)
{
    /* データをメモリ上に保存 */
    g_ausDbgMblTbl[g_iDbgMdlCnt][0] = usCode;
    g_ausDbgMblTbl[g_iDbgMdlCnt][1] = usFncCode;
    g_ausDbgMblTbl[g_iDbgMdlCnt][2] = usPrm1;
    g_ausDbgMblTbl[g_iDbgMdlCnt][3] = usPrm2;

    /* カウンタの更新 */
    g_iDbgMdlCnt = (g_iDbgMdlCnt+1) % DBG_MAX_TRACE;
}

#elif GR_DBG_MDL_32BIT_MODE
/************************************************************************/
/*  FUNCTION    : GRDBG_DebugTrace                                      */
/*                                                                      */
/*  DESCRIPTION : The Trace information for Debug is set.               */
/*----------------------------------------------------------------------*/
/*  INPUT       : ulCode        Module Code                             */
/*                ulFncCode     Parameter 1                             */
/*                ulPrm1        Parameter 2                             */
/*                ulPrm2        Parameter 3                             */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  RESULT      : None                                                  */
/************************************************************************/
void GRDBG_DebugTrace( unsigned long            ulCode,
                       unsigned long            ulFncCode,
                       unsigned long            ulPrm1,
                       unsigned long            ulPrm2)
{
    /* データをメモリ上に保存 */
    g_aulDbgMblTbl[g_iDbgMdlCnt][0] = ulCode;
    g_aulDbgMblTbl[g_iDbgMdlCnt][1] = ulFncCode;
    g_aulDbgMblTbl[g_iDbgMdlCnt][2] = ulPrm1;
    g_aulDbgMblTbl[g_iDbgMdlCnt][3] = ulPrm2;

    /* カウンタの更新 */
    g_iDbgMdlCnt = (g_iDbgMdlCnt+1) % DBG_MAX_TRACE;
}
#endif  /* MODE設定 */
#endif  /* GR_DBG_MDL */
