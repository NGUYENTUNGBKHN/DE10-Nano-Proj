/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2018 Grape Systems, Inc.                          */
/*                                     All Rights Reserved.                                     */
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
/*      grp_usbc_dbg.c                                                          1.08            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# debug module                                                               */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  0.01                                                            */
/*                            - Created first version 1.00  based on the 1.00                   */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/29  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   M.Suzuki       2018/01/19  V1.08                                                           */
/*                            - Modified the "FILE NAME" and "DESCRIPTION" of the file header.  */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include    "grp_usbc_dbg.h"


/**** INTERNAL DATA DEFINES *********************************************************************/
#ifdef GRP_USBC_DBG_MDL

grp_si      g_iDbgMdlCnt = 0;                                           /* counter              */
grp_u8      g_aucDbgMdlTbl[GRP_DBG_MAX_TRACE][4];                       /* Trace Buffer         */


/************************************************************************************************/
/* FUNCTION     : GRDBG_DebugTrace                                                              */
/*                                                                                              */
/* DESCRIPTION  : The Trace information for Debug is set.                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ucCode                        Module Code                                     */
/*                ucFncCode                     Parameter 1                                     */
/*                ucPrm1                        Parameter 2                                     */
/*                ucPrm2                        Parameter 3                                     */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : none                                                                          */
/************************************************************************************************/
void    GRDBG_DebugTrace( grp_u8 ucCode, grp_u8 ucFncCode, grp_u8 ucPrm1, grp_u8 ucPrm2 )
{
    /* Store data on memory */
    g_aucDbgMdlTbl[g_iDbgMdlCnt][0] = ucCode;
    g_aucDbgMdlTbl[g_iDbgMdlCnt][1] = ucFncCode;
    g_aucDbgMdlTbl[g_iDbgMdlCnt][2] = ucPrm1;
    g_aucDbgMdlTbl[g_iDbgMdlCnt][3] = ucPrm2;

    /* counter increment */
    g_iDbgMdlCnt = (g_iDbgMdlCnt+1) % GRP_DBG_MAX_TRACE;
}

#endif  /* GRP_USBC_DBG_MDL */
