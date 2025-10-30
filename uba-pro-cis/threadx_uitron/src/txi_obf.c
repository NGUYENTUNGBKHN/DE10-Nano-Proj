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
/*  txi_obf.c                                                                   */
/*  uITRON Library for ThreadX.(GR-TXi4)                                        */
/*                                                                              */
/*  Find free object index                                                      */
/*                                                                              */
/********************************************************************************/

#include "txi_knl.h"

/********************************************************************************/
/*                                                                              */
/* FUNCTION                                                                     */
/*      _kernel_object_find_index                                               */
/*                                                                              */
/* AUTHOR                                                                       */
/*      M. Miyashita, Grape Systems Inc.                                        */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*      Find free object index.                                                 */
/*                                                                              */
/* INPUT                                                                        */
/*      id_list             ID list                                             */
/*      maxid               List size                                           */
/*                                                                              */
/* OUTPUT                                                                       */
/*      index or none(0)                                                        */
/*                                                                              */
/* CALLS                                                                        */
/*      NONE                                                                    */
/*                                                                              */
/* CALLED BY                                                                    */
/*      Internal                                                                */
/*                                                                              */
/* ALLOWED FROM                                                                 */
/*      acre_alm                                                                */
/*      acre_cyc                                                                */
/*      acre_dtq                                                                */
/*      acre_mbx                                                                */
/*      acre_mpf                                                                */
/*      acre_mpl                                                                */
/*      acre_mtx                                                                */
/*      acre_tsk                                                                */
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
ID _kernel_object_find(VP id_list[], INT maxid)
{
    ID index;

    for(index=0; index<maxid; index++)
    {
        if(id_list[index]==NULL)
        {
            return index+1;
        }
    }

    return (ID)0;
}

