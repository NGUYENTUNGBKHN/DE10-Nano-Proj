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
/*  wai_flg.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Wait event flag                                                             */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      wai_flg                                                                 */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Wait event flag.                                                        */
/*                                                                              */
/* INPUT                                                                        */
/*      flgid               ID number of the event flag                         */
/*      waiptn              Waiting bit pattern                                 */
/*      wfmode              Waiting flag mode                                   */
/*      p_flgptn            Pointer of flag pattern                             */
/*                                                                              */
/* OUTPUT                                                                       */
/*      E_CTX               Context error                                       */
/*      E_ID                Invalid ID number                                   */
/*      E_PAR               Parameter error                                     */
/*      E_RLWAI             Forced release from waiting                         */
/*      E_NOEXS             Non-existent object                                 */
/*      E_ILUSE             Illegal using error                                 */
/*      E_OK                Normal completion                                   */
/*                                                                              */
/* CALLS                                                                        */
/*      __twai_flg                  Wait event flag.                            */
/*                                  (Internal function)                         */
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
ER wai_flg(ID flgid, FLGPTN waiptn, MODE wfmode, FLGPTN* p_flgptn)
{
    CHECK_ID(1, _kernel_flg_maxid, flgid)
    CHECK_PARAM(waiptn!=0 && wfmode==(wfmode&(TWF_ANDW|TWF_ORW)) && p_flgptn!=NULL);
    CHECK_TSK_CONTEXT()

    return __twai_flg(flgid, waiptn, wfmode, p_flgptn, TMO_FEVR);
}

