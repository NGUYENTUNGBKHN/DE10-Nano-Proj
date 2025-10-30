/****************************************************************************/
/*                                                                          */
/*              Copyright(C) 2002-2014 Grape Systems, Inc.                  */
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
/*      peri_prm.c                                              1.43        */
/*                                                                          */
/* DESCRIPTION:                                                             */
/*                                                                          */
/*      This module performs parameter management.                          */
/*                                                                          */
/* HISTORY                                                                  */
/*                                                                          */
/*  NAME          DATE        REMARKS                                       */
/*                                                                          */
/*  Y.Sano      2002/12/13    Created initial version 0.01                  */
/*  Y.Sato      2003/04/18    Beta Version 0.90                             */
/*  K.Takagi    2003/07/16    version 0.92, modified variable names.        */
/*  Y.Sato      2003/07/25    version 1.00                                  */
/*  K.Takagi    2004/02/27  V 1.01                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/09  V 1.02                                          */
/*                              version is updated.                         */
/*  K.Takagi    2004/04/23  V 1.03                                          */
/*                              mdified following function.                 */
/*                              - GRUSB_Prm_GetStringDescriptor             */
/*  K.Takagi    2004/05/12  V 1.10                                          */
/*                              Add logging routine.                        */
/*  K.Handa     2005/03/18  V 1.11                                          */
/*                              below descriptors supported.                */
/*                              - Device Qualifier Descriptor               */
/*                              - Other Speed Configuration Descriptor      */
/*  K.Handa     2005/03/18  V 1.12                                          */
/*                              hybrid version (full/high-speed)            */
/*  K.Handa     2005/03/18  V 1.13                                          */
/*                              for override maxpower                       */
/*  K.Handa     2005/04/08  V 1.14                                          */
/*                              for supporting test mode                    */
/*  K.Handa     2005/11/10  V 1.15                                          */
/*                              updated illegal comment                     */
/*  K.Handa     2006/12/27  V 1.20                                          */
/*                              version was updated.                        */
/*  S.Tomizawa  2007/01/24  V 1.30                                          */
/*                              changed static parameter to global one      */
/*                              - l_atEpInfo                                */
/*                              to support MTP,                             */
/*                              - added some parameters                     */
/*                              - added code that process a received the    */
/*                                request for getting a String Descriptor   */
/*  K.Handa     2007/03/12  V 1.40                                          */
/*                              version was updated.                        */
/*  K.Handa     2007/06/12  V 1.41                                          */
/*                              bmAttributes update timing was changed.     */
/*  K.Handa     2008/01/21  V 1.42                                          */
/*                              version was updated.                        */
/*  M.Suzuki    2014/06/24  V 1.43                                          */
/*                              added check for NULL pointer when           */
/*                              overriding Configuration Descriptor.        */
/*                                                                          */
/****************************************************************************/

/**** INCLUDE FILES *********************************************************/
#include    <string.h>
#include    "perid.h"
#include    "peri_sts.h"
#include    "peri_prm.h"
#include    "peri_ctl.h"

/* Logging routine */
#ifdef GRUSB_PRM_DEBUG
#include    "dbg_mdl.h"
#define     GRPRM_DBG_TRACE(m,n,x,y)    GRDBG_TRACE(m,n,x,y)
#else
#define     GRPRM_DBG_TRACE(m,n,x,y)
#endif

/**** INTERNAL CONSTANT DEFINE **********************************************/
#define MAX_FIFOBUFFER          (5120)      /* 5.0 K bytes of total amount of FIFO Buffer   */ /* V 1.15 */

/**** INTERNAL FUNCTION PROTOTYPES ******************************************/
/* bus information */
        UINT8                   l_ucBusSpd;         /* bus speed (0:full/1:high)        */
/* endpoint information */
DLOCAL  INT                     l_iEpNum;                       /* number of endpoint   */
        GRUSB_EndPointInfo      l_atEpInfo[GRUSB_DEV_MAX_EP];   /* endpoint information */ /* 2007/01/24 [1.30] */

/* descriptor information */
DLOCAL  GRUSB_DEV_DEVICE_DESC*  l_ptDeviceDesc[2];  /* Device Descriptor            [0]FS [1]HS */
DLOCAL  UINT8                   l_ucStringDescNum;  /* number of String Descriptor          */
DLOCAL  GRUSB_DEV_STRING_DESC*  l_pStringDesc;      /* array of String Descriptor           */
DLOCAL  UINT16                  l_usConfigDescNum;  /* number of Configuration Descriptor   */
DLOCAL  GRUSB_DEV_CONFIG_DESC*  l_ptConfigDesc[2];  /* Configuration Desciriptor    [0]FS [1]HS */

DLOCAL  GRUSB_DEV_DEVICE_QUALIFIER_DESC     l_tDeviceQualifierDesc;
DLOCAL  UINT8                               l_tConfDescOverride[GRUSB_PRM_ORCDBS];

/* 2007/01/24 [1.30] Added for MTP */
#ifdef GRUSB_MTP_MS_VENDORCODE
EXTERN  BOOLEAN g_bMTPSupport;
DLOCAL const union {
    UINT8    uacData[0x12];
    UINT32   ulDummy;       /* for 32bits alignment */
} l_uOSString = {
    0x12,                                                                               /* bLength                */
    0x03,                                                                               /* bDescriptorType        */
    0x4D, 0x00, 0x53, 0x00, 0x46, 0x00, 0x54, 0x00, 0x31, 0x00, 0x30, 0x00, 0x30, 0x00, /* qwSignature('MSFT100') */
    GRUSB_MTP_MS_VENDORCODE,                                                            /* bMS_VendorCode         */
    0x00                                                                                /* bPad                   */
};
#endif

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_Init                                              */
/*                                                                          */
/* DESCRIPTION: Initialization in the Parameter management Module           */
/*--------------------------------------------------------------------------*/
/* INPUT      : ptInitInfo              Initialization Parameter            */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_DEV_SUCCESS       Success                             */
/*              GRUSB_DEV_INVALID_PARAM Parameter Error                     */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : l_iEpNum                endpoint number                     */
/*              l_atEpInfo              endpoint information                */
/*              l_ptDeviceDesc          DeviceDescriptor information        */
/*              l_ucStringDescNum       StringDescriptor number             */
/*              l_pStringDesc           StringDescriptor information        */
/*              l_usConfigDescNum       ConfigurationDescriptor number      */
/*              l_pucConfigDescriptor   ConfigurationDescriptor information */
/*                                                                          */
/****************************************************************************/
INT GRUSB_Prm_Init( GRUSB_DEV_INITINFO*         ptInitInfo)
{
    INT                     iEpCnt;
    INT                     iCnt;
    INT                     iQueCnt;
    INT32                   lFifoSz = 0;
    UINT8*                  pucCntrlArea;
    GRUSB_EndPointInfo*     ptEpInfo;
    GRUSB_DEV_EPINITINFO*   ptEpInit;
    UINT16                  usMaxPktSz;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x01, 0, 0 );

    GRUSB_Prm_SetBusSpeed( GRUSB_FALSE );       /* set default (full speed) */
    l_iEpNum = ptInitInfo->iEpNum;                          /* set endpoint number  */

    pucCntrlArea = ptInitInfo->pucCntrlArea;    /* get pointer of management amount */

    if ((l_iEpNum > GRUSB_DEV_MAX_EP)
     || (l_iEpNum < 0))                                         /* invalid endpoint */
    {
        GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x01, 0x01, END_FUNC );

        return  GRUSB_DEV_INVALID_PARAM;                        /* parameter error  */
    }

    for (iEpCnt=0, iCnt=0; iEpCnt<l_iEpNum; iEpCnt++)
    {
        /* clear endpoint information */
        memset( &l_atEpInfo[iEpCnt], 0, sizeof(GRUSB_EndPointInfo));
        ptEpInfo        = l_atEpInfo + iEpCnt;
        ptEpInfo->iEpNo = (UINT8)iEpCnt;

        ptEpInit             = ptInitInfo->atEpInfo + iCnt;
        ptEpInfo->ucEpType   = ptEpInit->ucEpType;              /* endpoint type    */
        ptEpInfo->usMaxPktSz[0] = ptEpInit->usMaxPktSz[0];      /* maximum packet size (FS) */
        ptEpInfo->usMaxPktSz[1] = ptEpInit->usMaxPktSz[1];      /* maximum packet size (HS) */
        ptEpInfo->ucPages    = ptEpInit->ucPages;                   /* pages number */

        if (GRUSB_DEV_MAC_IsEpTypeIN(ptEpInit->ucEpType)) /* if endpoint type is IN */
            ptEpInfo->uInfo.tSndInfo.pfnFunc = ptEpInit->pfnTransEnd;
                                                /* then callback function is set up */

        ptEpInfo->pfnTrnsErrFnc = ptEpInit->pfnTransErr;

        iQueCnt = ptEpInit->usNumOfQue;
        iCnt++;

        usMaxPktSz = (ptEpInfo->usMaxPktSz[0] > ptEpInfo->usMaxPktSz[1]) ? ptEpInfo->usMaxPktSz[0] : ptEpInfo->usMaxPktSz[1];
        lFifoSz += (ptEpInfo->ucPages * usMaxPktSz);
        if (lFifoSz > MAX_FIFOBUFFER)
        {
            GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x01, 0x02, END_FUNC );

            return  GRUSB_DEV_INVALID_PARAM;                    /* Parameter Error  */
        }

        ptEpInfo->ptTrnsInfo = (GRUSB_TransmitInfo*)pucCntrlArea;
                                       /* Pointer to a management domain is set up. */

        /* Initialization of Cyclic Buffer */
        GRLIB_Cyclic_Init( &ptEpInfo->tCycBufInfo, iQueCnt + 1);
        pucCntrlArea += ((iQueCnt + 1) * sizeof(GRUSB_TransmitInfo));
                                       /* Renewal of Pointer to a management domain */
    }

    if( ptInitInfo->usConfigDescNum == 0 )                                              /* 1.41 */
    {                                                                                   /* 1.41 */
        return  GRUSB_DEV_INVALID_PARAM;                        /* parameter error  */  /* 1.41 */
    }                                                                                   /* 1.41 */

    /* Preservation of each descriptor */
    l_ptDeviceDesc[0] = ptInitInfo->ptDeviceDesc[0];
    l_ptDeviceDesc[1] = ptInitInfo->ptDeviceDesc[1];
    l_ucStringDescNum = ptInitInfo->ucStringDescNum;
    l_pStringDesc     = ptInitInfo->pStringDesc;
    l_usConfigDescNum = ptInitInfo->usConfigDescNum;
    l_ptConfigDesc[0] = ptInitInfo->ptConfigDesc[0];
    l_ptConfigDesc[1] = ptInitInfo->ptConfigDesc[1];

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x01, 0, END_FUNC );

    return  GRUSB_DEV_SUCCESS;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_ReInit                                            */
/*                                                                          */
/* DESCRIPTION: Re-initialization in the Parameter management Module        */
/*              is performed.                                               */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : l_iEpNum                endpoint numbe                      */
/* MODIFY     : l_atEpInfo              endpoint information                */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_Prm_ReInit(VOID)
{
    INT                 i;
    GRUSB_EndPointInfo  *ptEpInfo;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x02, 0, 0 );

    for (i=0; i<l_iEpNum; i++)
    {
        ptEpInfo = l_atEpInfo + i;

        /* initialization of cyclic buffer */
        GRLIB_Cyclic_Init( &ptEpInfo->tCycBufInfo, (ptEpInfo->tCycBufInfo.iMax));

        if (GRUSB_DEV_MAC_IsEpTypeIN(ptEpInfo->ucEpType))
        {   /* receive information */
            ptEpInfo->uInfo.tRcvInfo.ulRcvSize = 0;
        }
        else
        {   /* send informaiton */
            ptEpInfo->uInfo.tSndInfo.iFlag     = GRUSB_FALSE;
            ptEpInfo->uInfo.tSndInfo.ulSndSize = 0;
        }
    }

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x02, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_SetSntFunc                                        */
/*                                                                          */
/* DESCRIPTION: The CallBack function notified at the time of               */
/*              the completion of transmitting is registered.               */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/*              pfnFunc             The CallBack function notified at the   */
/*                                  time of the completion of transmitting  */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_TRUE              valid endpoint                      */
/*              GRUSB_FALSE             invalid endpoint                    */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : l_atEpInfo              endpoint information                */
/*                                                                          */
/****************************************************************************/
BOOLEAN GRUSB_Prm_SetSntFunc( INT                   iEpNo,
                              GRUSB_NoticeEndPoint  pfnFunc)
{
    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x03, 0, 0 );

    /* check endpoint number */
    if ((iEpNo >= l_iEpNum)
     || (iEpNo < 0))
    {
        GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x03, 0x01, END_FUNC );

        return  GRUSB_FALSE;                            /* Parameter Error */
    }

    l_atEpInfo[iEpNo].uInfo.tSndInfo.pfnFunc = pfnFunc;
                                        /* the callback function is set up */

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x03, 0, END_FUNC );

    return  GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_GetEndPointInfo                                   */
/*                                                                          */
/* DESCRIPTION: The endpoint information on specified endpoint is acquired. */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo           endpoint number                             */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : not GRUSB_NULL          endpoint information                */
/*              GRUSB_NULL              invalide endpoint information       */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : l_atEpInfo              endpoint information                */
/*                                                                          */
/****************************************************************************/
GRUSB_EndPointInfo* GRUSB_Prm_GetEndPointInfo( INT      iEpNo)
{
    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x04, 0, 0 );

    /* check endpoint number */
    if ((iEpNo >= l_iEpNum)
     || (iEpNo < 0))
    {
        GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x04, 0x01, END_FUNC );

        return  GRUSB_NULL;                             /* Parameter Error  */
    }

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x04, 0, END_FUNC );

    return  &l_atEpInfo[iEpNo];
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_GetFIFOInfo                                       */
/*                                                                          */
/* DESCRIPTION: The FIFO information on specified endpoint is acquired.     */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/* OUTPUT     : pucPages            FIFO Page number                        */
/*              pusMaxPacketSize    FIFO Max Packet Size                    */
/*                                                                          */
/* RESULTS    : GRUSB_TRUE          valid endpoint                          */
/*              GRUSB_FALSE         invalid endpoint                        */
/*--------------------------------------------------------------------------*/
/* REFER      : l_atEpInfo          endpoint information                    */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
BOOLEAN GRUSB_Prm_GetFIFOInfo( INT          iEpNo,
                               UINT8*       pucPages,
                               UINT16*      pusMaxPktSz)
{
    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x05, 0, 0 );

    /* check endpoint number */
    if ((iEpNo >= l_iEpNum)
     || (iEpNo < 0))
    {
        GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x05, 0x01, END_FUNC );

        return  GRUSB_FALSE;                            /* Parameter Error  */
    }

    *pucPages    = l_atEpInfo[iEpNo].ucPages;
    *pusMaxPktSz = l_atEpInfo[iEpNo].usMaxPktSz[l_ucBusSpd];

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x05, 0, END_FUNC );

    return  GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_GetEndPointType                                   */
/*                                                                          */
/* DESCRIPTION: The specified transmission direction of endpoint and        */
/*              transmission classification are acquired.                   */
/*--------------------------------------------------------------------------*/
/* INPUT      : iEpNo               endpoint number                         */
/* OUTPUT     : ptEpType            endpoint transmission type              */
/*                                                                          */
/* RESULTS    : GRUSB_TRUE          valid endpoint                          */
/*              GRUSB_FALSE         invalid endpoint                        */
/*--------------------------------------------------------------------------*/
/* REFER      : l_atEpInfo          endpoint information                    */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
BOOLEAN GRUSB_Prm_GetEndPointType( INT                  iEpNo,
                                   GRUSB_DEV_EP_TYPE*   pucEpType)
{
    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x06, 0, 0 );

    if ((iEpNo >= l_iEpNum)
     || (iEpNo < 0))
    {
        GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x06, 0x01, END_FUNC );

        return  GRUSB_FALSE;                            /* Parameter Error  */
    }

    *pucEpType = l_atEpInfo[iEpNo].ucEpType;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x06, 0, END_FUNC );

    return  GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_GetDeviceDescriptor                               */
/*                                                                          */
/* DESCRIPTION: Device descriptor information is acquired.                  */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : pusSize             length of descriptor                    */
/*                                                                          */
/* RESULTS    : not GRUSB_NULL      valid descriptor                        */
/*              GRUSB_NULL          invalid descriptor                      */
/*--------------------------------------------------------------------------*/
/* REFER      : l_ptDeviceDesc      DeviceDescriptor information            */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
UINT8* GRUSB_Prm_GetDeviceDescriptor( UINT16*       pusSize)
{
    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x07, 0, 0 );

    /* check parameter */
    if (l_ptDeviceDesc[l_ucBusSpd]->uc_bLength <= 0)    /* if length is less than 0 */
    {
        GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x07, 0x01, END_FUNC );

        return  GRUSB_NULL;                     /* return null pointer      */
    }

    /* get length of the Device Descriptor */
    *pusSize = l_ptDeviceDesc[l_ucBusSpd]->uc_bLength;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x07, 0, END_FUNC );

    return  (UINT8*)l_ptDeviceDesc[l_ucBusSpd]; /* return pointer */
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_GetStringDescriptor                               */
/*                                                                          */
/* DESCRIPTION: String descriptor information is acquired.                  */
/*--------------------------------------------------------------------------*/
/* INPUT      : ucIndex             index value                             */
/* OUTPUT     : pusSize             length of descriptor                    */
/*                                                                          */
/* RESULTS    : not GRUSB_NULL      valid String descriprot                 */
/*              GRUSB_NULL          invalid String descriptor               */
/*--------------------------------------------------------------------------*/
/* REFER      : l_ucStringDescNum   StringDescriptor number                 */
/*              l_pStringDesc       StringDescriptor information            */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
UINT8* GRUSB_Prm_GetStringDescriptor( UINT8         ucIndex,
                                      UINT16*       pusSize )
{
    UINT8*                  pucStrngDesc;
    UINT8                   ucSize;
    INT                     i;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x08, 0, 0 );

/* 2007/01/24 [1.30] Added for MTP */
#ifdef GRUSB_MTP_MS_VENDORCODE
    if( g_bMTPSupport )
    {
        if( ucIndex == 0xEE )
        {
            pucStrngDesc = (UINT8*)&l_uOSString.uacData[0];
            *pusSize =(UINT16)pucStrngDesc[0];

            return  pucStrngDesc;
        }
    }
#endif
    /* check parameter */
    if (ucIndex > l_ucStringDescNum)    /* if the index number is invalid   */
    {
        GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x08, 0x01, END_FUNC );

        return  GRUSB_NULL;             /* return null pointer              */
    }

    /* get pointer of the String Descriptor */
    pucStrngDesc = (UINT8*)l_pStringDesc;

    for (i = 0; i < (ucIndex + 1); i++)
    {
        if (i != 0)
        {
            /* get pointer of the String Descriptor */
            pucStrngDesc = pucStrngDesc + ucSize;
        }
        /* get length of the String Descriptor */
        ucSize = *pucStrngDesc;
    }

    /* get length of the String Descriptor */
    *pusSize = (UINT16)ucSize;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x08, ucSize, pucStrngDesc );
    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x08, 0, END_FUNC );

    return  pucStrngDesc;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_GetConfigurationDescriptor                        */
/*                                                                          */
/* DESCRIPTION: Configuration descriptor information is acquired.           */
/*--------------------------------------------------------------------------*/
/* INPUT      : ucIndex                 index value                         */
/* OUTPUT     : pusSize                 length of descriprot                */
/*                                                                          */
/* RESULTS    : not GRUSB_NULL          valid ConfigurationDescriptor       */
/*              GRUSB_NULL              invalid ConfigurationDescriptor     */
/*--------------------------------------------------------------------------*/
/* REFER      : l_usConfigDescNum       ConfigurationDescriptor number      */
/*              l_pucConfigDescriptor   ConfigurationDescriptor information */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
UINT8* GRUSB_Prm_GetConfigurationDescriptor( UINT8          ucIndex,
                                             UINT16*        pusSize)
{
    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x09, 0, 0 );

    /* check parameter */
    if (ucIndex >= l_usConfigDescNum)   /* if the index number is invalid   */
    {
        GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x09, 0x01, END_FUNC );

        return  GRUSB_NULL;             /* return null pointer              */
    }

    /* get length of the Configuration Descriptor */
    *pusSize = ((UINT16)(l_ptConfigDesc[l_ucBusSpd]->auc_wTotalLength[0]));

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x09, 0, END_FUNC );

    return (UINT8*)l_ptConfigDesc[l_ucBusSpd];  /* return pointer */
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_GetInterface                                      */
/*                                                                          */
/* DESCRIPTION: Interface information is set up.                            */
/*--------------------------------------------------------------------------*/
/* INPUT      : usIndex             specified Interface number              */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_TRUE          valid Interface number                  */
/*              GRUSB_FALSE         invlaide Interface number               */
/*--------------------------------------------------------------------------*/
/* REFER      : l_ptConfigDesc      ConfigurationDescriptor information     */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
BOOLEAN GRUSB_Prm_GetInterface( UINT16      usIndex,
                                UINT8*      pucRetBuf)
{
    INT     iStatus;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0A, 0, 0 );

    /* check index value */
    if (usIndex > l_ptConfigDesc[l_ucBusSpd]->uc_bNumInterfaces)
    {
        GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0A, 0x01, END_FUNC );

        return  GRUSB_FALSE;
    }

    iStatus = GRUSB_DEV_StateGetInterface();

    *pucRetBuf = (UINT8)iStatus;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0A, 0x02, END_FUNC );

    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_SetInterface                                      */
/*                                                                          */
/* DESCRIPTION: Interface information is set up.                            */
/*--------------------------------------------------------------------------*/
/* INPUT      : usIndex             specified Interface number              */
/*              usValue             alternative setting value               */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : GRUSB_TRUE          success                                 */
/*              GRUSB_FALSE         invalid Interface number                */
/*--------------------------------------------------------------------------*/
/* REFER      : l_ptConfigDesc      ConfigurationDescriptor information     */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
BOOLEAN GRUSB_Prm_SetInterface( UINT16          usIndex,
                                UINT16          usValue)
{
    INT     iAltSet;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0B, 0, 0 );

    /* check index value */
    if (usIndex > l_ptConfigDesc[l_ucBusSpd]->uc_bNumInterfaces)
    {
        GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0B, 0x01, END_FUNC );

        return  GRUSB_FALSE;
    }

    iAltSet = (INT)usValue;

    GRUSB_DEV_StateSetInterface( iAltSet );

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0B, 0x02, END_FUNC );

    return GRUSB_TRUE;
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_GetDeviceQualifierDescriptor                      */
/*                                                                          */
/* DESCRIPTION: Device qualifier descriptor information is acquired.        */
/*--------------------------------------------------------------------------*/
/* INPUT      : none                                                        */
/* OUTPUT     : pusSize             length of descriptor                    */
/*                                                                          */
/* RESULTS    : not GRUSB_NULL      valid descriptor                        */
/*              GRUSB_NULL          invalid descriptor                      */
/*--------------------------------------------------------------------------*/
/* REFER      : l_ptDeviceDesc      DeviceDescriptor information            */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
UINT8* GRUSB_Prm_GetDeviceQualifierDescriptor( UINT16*      pusSize)
{
    UINT8       ucOtherSpd;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0C, 0, 0 );

    /* select other speed */
    if(l_ucBusSpd == GRUSB_PRM_BUS_FS)
    {
        /* FS -> Other is HS */
        ucOtherSpd = GRUSB_PRM_BUS_HS;
    }
    else
    {
        /* HS -> Other is FS */
        ucOtherSpd = GRUSB_PRM_BUS_FS;
    }

    /* check parameter */
    if (l_ptDeviceDesc[ucOtherSpd]->uc_bLength <= 0)    /* if length is less than 0 */
    {
        GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0C, 0x01, END_FUNC );
        return  GRUSB_NULL;                             /* return null pointer      */
    }

    /* make qualifier from other speed descriptor */
    l_tDeviceQualifierDesc.uc_bLength            = 10;                                                  /* bLength              */
    l_tDeviceQualifierDesc.uc_bDescriptorType    = GRUSB_DEV_DEVICE_QUALIFIER;                          /* bDescriptorType      */
    l_tDeviceQualifierDesc.uc_bcdUSB[0]          = l_ptDeviceDesc[ucOtherSpd]->uc_bcdUSB[0];            /* bcdUSB               */
    l_tDeviceQualifierDesc.uc_bcdUSB[1]          = l_ptDeviceDesc[ucOtherSpd]->uc_bcdUSB[1];            /* bcdUSB               */
    l_tDeviceQualifierDesc.uc_bDeviceClass       = l_ptDeviceDesc[ucOtherSpd]->uc_bDeviceClass;         /* bDeviceClass         */
    l_tDeviceQualifierDesc.uc_bDeviceSubClass    = l_ptDeviceDesc[ucOtherSpd]->uc_bDeviceSubClass;      /* bDeviceSubClass      */
    l_tDeviceQualifierDesc.uc_bDeviceProtocol    = l_ptDeviceDesc[ucOtherSpd]->uc_bDeviceProtocol;      /* bDeviceProtocol      */
    l_tDeviceQualifierDesc.uc_bMaxPacketSize0    = l_ptDeviceDesc[ucOtherSpd]->uc_bMaxPacketSize0;      /* bMaxPacketSize0      */
    l_tDeviceQualifierDesc.uc_bNumConfigurations = l_ptDeviceDesc[ucOtherSpd]->uc_bNumConfigurations;   /* bNumConfigurations   */
    l_tDeviceQualifierDesc.uc_Reserved           = 0;                                                   /* Reserved             */

    /* get length of the Device Descriptor */
    *pusSize = l_tDeviceQualifierDesc.uc_bLength;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0C, 0, END_FUNC );

    return  (UINT8*)&l_tDeviceQualifierDesc;    /* return pointer */
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_GetOtherSpeedConfigurationDescriptor              */
/*                                                                          */
/* DESCRIPTION: Other Speed Configuration descriptor information is acquired*/
/*--------------------------------------------------------------------------*/
/* INPUT      : ucIndex                 index value                         */
/* OUTPUT     : pusSize                 length of descriprot                */
/*                                                                          */
/* RESULTS    : not GRUSB_NULL          valid ConfigurationDescriptor       */
/*              GRUSB_NULL              invalid ConfigurationDescriptor     */
/*--------------------------------------------------------------------------*/
/* REFER      : l_usConfigDescNum       ConfigurationDescriptor number      */
/*              l_pucConfigDescriptor   ConfigurationDescriptor information */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
UINT8* GRUSB_Prm_GetOtherSpeedConfigurationDescriptor( UINT8        ucIndex,
                                                       UINT16*      pusSize)
{
    UINT8                   ucOtherSpd;
    GRUSB_DEV_CONFIG_DESC   *ptConfDesc;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0D, 0, 0 );

    /* select other speed */
    if(l_ucBusSpd == GRUSB_PRM_BUS_FS)
    {
        /* FS -> Other is HS */
        ucOtherSpd = GRUSB_PRM_BUS_HS;
    }
    else
    {
        /* HS -> Other is FS */
        ucOtherSpd = GRUSB_PRM_BUS_FS;
    }

    /* check parameter */
    if (ucIndex >= l_usConfigDescNum)   /* if the index number is invalid   */
    {
        GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0D, 0x01, END_FUNC );

        return  GRUSB_NULL;             /* return null pointer              */
    }

    /* copy other speed configuration descriptor */
    memcpy( l_tConfDescOverride,
            l_ptConfigDesc[ucOtherSpd],
            l_ptConfigDesc[ucOtherSpd]->auc_wTotalLength[0] );

    /* override descriptor type */
    ptConfDesc = (GRUSB_DEV_CONFIG_DESC*)&l_tConfDescOverride;
    ptConfDesc->uc_bDescriptorType = GRUSB_DEV_OTHER_SPEED_CONFIGURATION;

    /* get length of the Configuration Descriptor */
    *pusSize = ((UINT16)(ptConfDesc->auc_wTotalLength[0]));

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0D, 0, END_FUNC );

    return (UINT8*)ptConfDesc;  /* return pointer */
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_SetBusSpeed                                       */
/*                                                                          */
/* DESCRIPTION: Set Bus Speed.                                              */
/*--------------------------------------------------------------------------*/
/* INPUT      : bBusSpd             GRUSB_FALSE:FS  GRUSB_TRUE:HS           */
/* OUTPUT     : none                                                        */
/*                                                                          */
/* RESULTS    : none                                                        */
/*--------------------------------------------------------------------------*/
/* REFER      : none                                                        */
/* MODIFY     : l_ucBusSpd          Bus Speed                               */
/*                                                                          */
/****************************************************************************/
VOID GRUSB_Prm_SetBusSpeed( BOOLEAN bBusSpd )
{
    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0E, 0, 0 );

    l_ucBusSpd = (bBusSpd == GRUSB_TRUE) ? GRUSB_PRM_BUS_HS : GRUSB_PRM_BUS_FS;

    GRPRM_DBG_TRACE( GRDBG_PERI_PRM, 0x0E, 0, END_FUNC );
}

/****************************************************************************/
/* FUNCTION   : GRUSB_Prm_GetConfigurationDescriptorOverride                */
/*                                                                          */
/* DESCRIPTION: Configuration descriptor information is acquired.           */
/*--------------------------------------------------------------------------*/
/* INPUT      : ucORMode    override mode                                   */
/*              pucDesc     pointer for original configuration descriptor   */
/*              usActLength length for original configuration descriptor    */
/* OUTPUT     : pusSize                 length of descriprot                */
/*                                                                          */
/* RESULTS    : none                                                        */
/*                                                                          */
/*--------------------------------------------------------------------------*/
/* REFER      : l_usConfigDescNum       ConfigurationDescriptor number      */
/*              l_pucConfigDescriptor   ConfigurationDescriptor information */
/* MODIFY     : none                                                        */
/*                                                                          */
/****************************************************************************/
UINT8* GRUSB_Prm_GetConfigurationDescriptorOverride( UINT8 *pucDesc,
                                                     UINT16 usActLength )
{
    GRUSB_DEV_CONFIG_DESC   *ptConfDescOverride = GRUSB_NULL;

    if( GRUSB_NULL != pucDesc )
    {
        /* copy for override */
        memcpy(l_tConfDescOverride, pucDesc, usActLength);
        ptConfDescOverride = (GRUSB_DEV_CONFIG_DESC*)l_tConfDescOverride;
    }

    return  (UINT8*)ptConfDescOverride;     /* return pointer */
}
