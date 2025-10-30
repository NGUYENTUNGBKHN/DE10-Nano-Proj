/********************************************************************************/
/*                Copyright (C) 2002-2017 Grape Systems, Inc.                   */
/*                     All Rights Reserved.                                     */
/*                                                                              */
/*  This software is furnished under a license and may be used and copied only  */
/*  in accordance with the terms of such license and with the inclusion of the  */
/*  above copyright notice. No title to and ownership of the software is        */
/*  transferred.                                                                */
/*  Grape Systems Inc. makes no warranties with respect to the performance of   */
/*  this computer program, and specifically disclaims any responsibility for    */
/*  any damages, special or consequential, connected with the use of this       */
/*  program.                                                                    */
/*                                                                              */
/********************************************************************************/

/********************************************************************************/
/*  acp_por.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Accrept rendezvous                                                          */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      acp_por                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Accrept rendezvous.                                                     */
/*                                                                              */
/* INPUT                                                                        */
/*      porid               ID number of the rendezvous port.                   */
/*      acpptn              Accept bit pattern.                                 */
/*      p_rdvno             Pointer for rendezvous number                       */
/*      msg                 Data pointer to be call                             */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_CTX               Context error                                       */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      received size                                                           */
/*                                                                              */
/* CALLS                                                                        */
/*      __tacp_por                  Accrept rendezvous(Internal function)       */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Application                                                             */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      Task context                                                            */
/*                                                                              */
/* RELEASE HISTORY                                                              */
/*                                                                              */
/*  DATE            NAME                    DESCRIPTION                         */
/*                                                                              */
/*  2006/08/31      M. Miyashita    Initial version 2.0                         */
/*  2007/03/05      M. Miyashita    version 2.1, Modified comment(s).           */
/*  2009/07/10      M. Miyashita    version 2.2, Modified comment(s).           */
/*  2017/10/11      T. Shibuya      version 2.3, Modified comment(s).           */
/*                                                                              */
/********************************************************************************/
ER_UINT acp_por(ID porid, RDVPTN acpptn, RDVNO* p_rdvno, VP msg)
{
    CHECK_ID(1, _kernel_por_maxid, porid)
    CHECK_PARAM(acpptn!=0 && p_rdvno!=NULL && msg!=NULL)
    CHECK_TSK_CONTEXT()

    return __tacp_por(porid, acpptn, p_rdvno, msg, TMO_FEVR);
}

