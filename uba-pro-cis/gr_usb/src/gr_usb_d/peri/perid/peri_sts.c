/****************************************************************************/
/*                                                                          */
/*              Copyright(C) 2002-2008 Grape Systems, Inc.                  */
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
/*      peri_sts.c                                              1.42        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      State Control FUNCTION                                              */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME        DATE        REMARKS                                         */
/*                                                                          */
/*  S.Baba      2002/12/06  Created initial version 0.00                    */
/*  Y.Sato      2003/04/18  Beta Version 0.90                               */
/*  K.Takagi    2003/07/16  version 0.92, modified variable names.          */
/*  Y.Sato      2003/07/25  version 1.00                                    */
/*  K.Takagi    2004/02/27  V 1.01                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/09  V 1.02                                          */
/*                              added Callback function that is used to     */
/*                              change status.                              */
/*                              - GRUSB_DEV_StateInit                       */
/*                              - GRUSB_DEV_StateSetConfigure               */
/*  K.Takagi    2004/04/23  V 1.03                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/05/12  V 1.10                                          */
/*                              Add logging routine.                        */
/*  K.Handa     2006/12/27  V 1.20                                          */
/*                              version was updated.                        */
/*  S.Tomizawa  2007/01/24  V 1.30                                          */
/*                              version was updated.                        */
/*  K.Handa     2007/03/12  V 1.40                                          */
/*                              version was updated.                        */
/*  K.Handa     2007/06/12  V 1.41                                          */
/*                              version was updated.                        */
/*  K.Handa     2008/01/21  V 1.42                                          */
/*                              version was updated.                        */
/*                                                                          */
/****************************************************************************/

/**** INCLUDE FILES *********************************************************/
#include    "perid.h"
#include    "peri_sts.h"

/* Logging routine */
#ifdef GRUSB_STS_DEBUG
#include    "dbg_mdl.h"
#define     GRSTS_DBG_TRACE(m,n,x,y)    GRDBG_TRACE(m,n,x,y)
#else
#define     GRSTS_DBG_TRACE(m,n,x,y)
#endif

/**** INTERNAL DATA DEFINES *************************************************/
DLOCAL  GRUSB_DEV_STATE     l_eDevState;
DLOCAL  INT                 l_iInterfaceNo;
DLOCAL  BOOLEAN             l_iSuspend;
DLOCAL  BOOLEAN             l_aiStallStat[GRUSB_DEV_MAX_EP];
DLOCAL  GRUSB_Notice        l_pfnCngStateNoConfigured;

#define MAX_TRACE       256                         /* Trace Buffer number  */
INT     g_iGrDebugCnt = 0;                          /* counter              */
UINT16  g_usGrDebugTbl[MAX_TRACE][2];               /* Trace Buffer         */


/************************************************************************/
/*  FUNCTION    : GRUSB_DEV_StateInit                                   */
/*                                                                      */
/*  DESCRIPTION : Initialization of a state management module.          */
/*----------------------------------------------------------------------*/
/*  INPUT       : pfnCngStateNoConfigured   callback function when      */
/*                                          device status chages from   */
/*                                          CONFIGURED.                 */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  Results     : Nono                                                  */
/************************************************************************/
VOID GRUSB_DEV_StateInit(GRUSB_Notice   pfnCngStateNoConfigured)
{
    int i;

    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x01, 0, 0 );

    /* initialize parameters */
    l_eDevState    = GRUSB_DEV_STATE_IDLE;  /* set IDLE state           */
    l_iSuspend     = GRUSB_FALSE;           /* clear SUSPEND state      */
    l_iInterfaceNo = 0;                     /* clear Interface number   */

    /* All Stall states are canceled. */
    for (i=0; i<GRUSB_DEV_MAX_EP; ++i)
        l_aiStallStat[i] = GRUSB_FALSE;

    /* set callback function pointer */
    l_pfnCngStateNoConfigured = pfnCngStateNoConfigured;

    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x01, 0, END_FUNC );
}

/************************************************************************/
/*  FUNCTION    : GRUSB_DEV_StateIdle                                   */
/*                                                                      */
/*  DESCRIPTION : A state is set as an IDEL state.                      */
/*----------------------------------------------------------------------*/
/*  INPUT       : Nono                                                  */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  Results     : Nono                                                  */
/************************************************************************/
VOID GRUSB_DEV_StateIdle(VOID)
{
    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x02, 0, 0 );

    /* set IDLE state */
    l_eDevState = GRUSB_DEV_STATE_IDLE;

    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x02, 0, END_FUNC );
}

/************************************************************************/
/*  FUNCTION    : GRUSB_DEV_StateReset                                  */
/*                                                                      */
/*  DESCRIPTION : BusReset is notified.                                 */
/*----------------------------------------------------------------------*/
/*  INPUT       : Nono                                                  */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  Results     : Nono                                                  */
/************************************************************************/
VOID GRUSB_DEV_StateReset(VOID)
{
    int i;

    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x03, 0, 0 );

    /* All Stall states are canceled. */
    for (i=0; i<GRUSB_DEV_MAX_EP; ++i)
        l_aiStallStat[i] = GRUSB_FALSE;

    /* set DEFAULT state */
    l_eDevState = GRUSB_DEV_STATE_DEFAULT;

    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x03, 0, END_FUNC );
}

/************************************************************************/
/*  Function    : GRUSB_DEV_StateSetAddress                             */
/*                                                                      */
/*  DESCRIPTION : SetAddress is notified.                               */
/*----------------------------------------------------------------------*/
/*  INPUT   : iAddress      device address                              */
/*  OUTPUt  : None                                                      */
/*                                                                      */
/*  RESULT  : Nono                                                      */
/************************************************************************/
VOID GRUSB_DEV_StateSetAddress( INT     iAddress)
{
    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x04, 0, 0 );

    if (iAddress == 0)
        l_eDevState = GRUSB_DEV_STATE_DEFAULT;  /* set DEFAULT state */
    else
        l_eDevState = GRUSB_DEV_STATE_ADDRESS;  /* set ADDRESS state */

    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x04, 0, l_eDevState );
    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x04, 0, END_FUNC );
}

/************************************************************************/
/*  Function    : GRUSB_DEV_StateSetConfigure                           */
/*                                                                      */
/*  DESCRIPTION : SetConfigure is notified.                             */
/*----------------------------------------------------------------------*/
/*  INPUT       : iConfig       Configuration value                     */
/*  OUTPUt      : None                                                  */
/*                                                                      */
/*  RESULT      : Nono                                                  */
/************************************************************************/
VOID GRUSB_DEV_StateSetConfigure( INT       iConfig)
{
    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x05, 0, 0 );

    if (iConfig == 0) {
        l_eDevState = GRUSB_DEV_STATE_ADDRESS;      /* set ADDRESS state */

        /* Notice to upper module if callback function is set */
        if (l_pfnCngStateNoConfigured)
        {
            GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x05, 0x01, 0 );

            (* l_pfnCngStateNoConfigured)();
        }
    }
    else
        l_eDevState = GRUSB_DEV_STATE_CONFIGURED;   /* set CONFIGURED state */

    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x05, 0, l_eDevState );
    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x05, 0, END_FUNC );
}

/************************************************************************/
/*  Function    : GRUSB_DEV_StateSuspend                                */
/*                                                                      */
/*  DESCRIPTION : Suspend is notified.                                  */
/*----------------------------------------------------------------------*/
/*  INPUT       : iSuspend      GRUSB_TRUE  = Suspned                   */
/*                              GRUSB_FALSE = not Suspend               */
/*  OUTPUt      : None                                                  */
/*                                                                      */
/*  RESULT      : Nono                                                  */
/************************************************************************/
VOID GRUSB_DEV_StateSuspend( BOOLEAN        iSuspend)
{
    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x06, 0, 0 );

    /* set SUSPEND state to the suspend flag */
    l_iSuspend = iSuspend;

    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x06, 0, END_FUNC );
}

/************************************************************************/
/*  Function    : GRUSB_DEV_StateGetDeviceState                         */
/*                                                                      */
/*  DESCRIPTION : The present DeviceState is acquired.                  */
/*----------------------------------------------------------------------*/
/*  INPUT       : None                                                  */
/*  OUTPUt      : None                                                  */
/*                                                                      */
/*  RESULT      : l_eDevState       the present Device State            */
/************************************************************************/
GRUSB_DEV_STATE GRUSB_DEV_StateGetDeviceState(VOID)
{
    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x07, 0, 0 );

    /* is state Suspend? */
    if (l_iSuspend == GRUSB_TRUE)
    {
        GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x07, 0x01, END_FUNC );

        return GRUSB_DEV_STATE_SUSPEND;     /* return Suspend state */
    }
    else
    {
        GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x07, 0x02, END_FUNC );

        return l_eDevState;                 /* return present Device state */
    }
}

/************************************************************************/
/*  Function    : GRUSB_DEV_StateSetStall                               */
/*                                                                      */
/*  DESCRIPTION :A Stall state is set up.                               */
/*----------------------------------------------------------------------*/
/*  INPUT       : iEpNo         endpoint number                         */
/*                iStall        GRUSB_TRUE  = set up to Stall           */
/*                              GRUSB_FALSE = release to Stall          */
/*  OUTPUt      : None                                                  */
/*                                                                      */
/*  RESULT      : None                                                  */
/************************************************************************/
VOID GRUSB_DEV_StateSetStall( INT           iEpNo,
                              BOOLEAN       iStall)
{
    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x08, 0, 0 );

    /* check endpoint number */
    if ((0 <= iEpNo)
     && (iEpNo < GRUSB_DEV_MAX_EP))
    {
        GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x08, 0x01, 0 );

        l_aiStallStat[iEpNo] = iStall;  /* A STALL state is set as specified endpoint. */
    }

    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x08, 0, END_FUNC );
}

/************************************************************************/
/*  FUNCTION    : GRUSB_DEV_StateGetStall                               */
/*                                                                      */
/*  DESCRIPTION : The present Stall state is acquired.                  */
/*----------------------------------------------------------------------*/
/*  INPUT       : iEpNo         endpoint number                         */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  RESULT      : l_aiStallStat[iEpNo]   state of endpoint              */
/************************************************************************/
BOOLEAN GRUSB_DEV_StateGetStall( INT        iEpNo)
{
    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x09, 0, 0 );

    /* check endpoint number */
    if ((0 <= iEpNo)
     && (iEpNo < GRUSB_DEV_MAX_EP))
    {
        GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x09, 0x01, END_FUNC );

        return (l_aiStallStat[iEpNo]);  /* The STALL state of specified endPoint */
                                        /* is returned.                          */
    }
    else
    {
        GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x09, 0x02, END_FUNC );

        return GRUSB_FALSE;
    }
}

/************************************************************************/
/*  FUNCTION    : GRUSB_DEV_StateGetInterface                           */
/*                                                                      */
/*  DESCRIPTION : The present interface number is acquired.             */
/*----------------------------------------------------------------------*/
/*  INPUT       : None                                                  */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  RESULT      : l_iInterfaceNo        interface number                */
/************************************************************************/
INT GRUSB_DEV_StateGetInterface(VOID)
{
    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x0A, 0, l_iInterfaceNo );

    return l_iInterfaceNo;          /* The present interface number is returned. */
}

/************************************************************************/
/*  FUNCTION    : GRUSB_DEV_StateSetInterface                           */
/*                                                                      */
/*  DESCRIPTION : An interface number is set up.                        */
/*----------------------------------------------------------------------*/
/*  INPUT       : iInterface        interface number                    */
/*  OUTPUT      : None                                                  */
/*                                                                      */
/*  RESULT      : None                                                  */
/************************************************************************/
VOID GRUSB_DEV_StateSetInterface( INT       iInterface)
{
    GRSTS_DBG_TRACE( GRDBG_PERI_STS, 0x0B, 0, iInterface );

    l_iInterfaceNo = iInterface;
}

