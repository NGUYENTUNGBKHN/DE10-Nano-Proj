/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2015 Grape Systems, Inc.                          */
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
/*      grp_fsif                                                                1.03            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      This file composes the File System Interface between a file system and GR-USB/HOST#     */
/*      Mass storage class modules.                                                             */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2006/12/28  V0.01                                                           */
/*                            - Created initial version                                         */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Takagi       2008/10/31  V1.01                                                           */
/*                            - Correction by member change in structure.                       */
/*                              - grp_msc_reg                                                   */
/*                                  pfnCallback -> pfnMscEvCallback                             */
/*                              - grp_fsif_init_prm                                             */
/*                                  pfnEventNotification -> pfnFsifNotification                 */
/*                            - Changed parameter types.                                        */
/*                              - _grp_fsif_Notify                                              */
/*                            - Warning measures                                                */
/*                              - _grp_fsif_InitUnitInfo                                        */
/*                            - Added the retrying processing of the error.                     */
/*                              - _grp_fsif_MediaCheck                                          */
/*                            - Deleted the retrying processing of the error.                   */
/*                              - _grp_fsif_Inquiry                                             */
/*                              - _grp_fsif_ReadCapacity                                        */
/*                              - _grp_fsif_ModeSense                                           */
/*                              - _grp_fsif_TestUnitReady                                       */
/*                            - Processing to write ASC and the ASCQ code in the trace log to   */
/*                              acquire a detailed error code when debugging it (At the user    */
/*                              porting) is added.                                              */
/*                              - _grp_fsif_GetErrorCode                                        */
/*                            - Addition of initialization to uninitalization data.             */
/*                              - _grp_fsif_NoticeListInit                                      */
/*   K.Kaneko       2015/12/07  V1.03                                                           */
/*                            - Prepared MSC command management area as a global variable.      */
/*                              - _grp_fsif_Read                                                */
/*                              - _grp_fsif_Write                                               */
/*                              - grp_fsif_Reset                                                */
/*                              - _grp_fsif_PollingCheck                                        */
/*                              - _grp_fsif_ConnectProcess                                      */
/*                              - _grp_fsif_DisconnectProcess                                   */
/*                              - _grp_fsif_GetMaxLun                                           */
/*                            - Corrected the name of the semaphore to "sFSIF6."                */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_usbc_types.h"
#include "grp_std_tools.h"                                                      /* for memset   */
#include "grp_usbd.h"
#include "grp_cmem.h"
#include "grp_msc.h"
#include "grp_fsif.h"
#include "grp_fsif_local.h"


/**** INTERNAL DATA DEFINES *********************************************************************/
/* Structure of Internal informations                                                           */
DLOCAL  grp_fsif_init_prm               l_tInitPrm;

/* Structure of Internal informations                                                           */
DLOCAL  _grp_fsif_unit_info             l_tFsifCB[GRP_FSIF_MAX_UINT];

/* Structure of Internal informations                                                           */
DLOCAL  _grp_fsif_notice                l_tFsifNtc[GRP_FSIF_MAX_NTC_QUEUE];

/* Structure of Internal informations                                                           */
DLOCAL  _grp_fsif_list                  l_tFreeList;
DLOCAL  _grp_fsif_list                  l_tAccessList;
DLOCAL  _grp_fsif_list                  l_tNoticeList;

/* Structure of Internal informations                                                           */
DLOCAL  _grp_fsif_local_vos_table       l_tFsifVos;

/* Structure of Internal informations                                                           */
DLOCAL  _grp_fsif_access_mng            l_tAccessMng;

/* Data buffer for the MAX_LUN */
DLOCAL  grp_u8                          *l_pucMaxLun;

/* MSC command management area */
DLOCAL  grp_msc_cmd                     l_tConMscCmd;
DLOCAL  grp_msc_cmd                     l_tDisMscCmd;
DLOCAL  grp_msc_cmd                     l_tPolMscCmd;
DLOCAL  grp_msc_cmd                     l_tLunMscCmd;

/************************************************************************************************/
/* The definition of the combination the Class and Subclass                                     */
DLOCAL  grp_u8  l_aucRegistClass[GRP_FSIF_MAX_CLASS][GRP_FSIF_MAX_SUBCLASS] = {
    /*          SCSI                ATAPI               SFF8070i            UFI         */
    /* BOT */  {GRP_FSIF_SCS_BOT,   GRP_FSIF_ATA_BOT,   GRP_FSIF_SFF_BOT,   GRP_FSIF_UFI_BOT},
    /* CBI */  {GRP_FSIF_SCS_CBI,   GRP_FSIF_ATA_CBI,   GRP_FSIF_SFF_CBI,   GRP_FSIF_UFI_CBI},
    /* CB  */  {GRP_FSIF_SCS_CB,    GRP_FSIF_ATA_CB,    GRP_FSIF_SFF_CB,    GRP_FSIF_UFI_CB}};


/************************************************************************************************/
/* MACRO for exclusive control                                                                  */
/*                                                                                              */
#define GRP_FSIF_LOCK_OK    GRP_VOS_POS_RESULT

#define GRP_FSIF_LIST_LOCK()        (grp_vos_GetSemaphore( l_tFsifVos.ptListSem, GRP_VOS_INFINITE))
#define GRP_FSIF_LIST_UNLOCK()      (grp_vos_ReleaseSemaphore( l_tFsifVos.ptListSem))

#define GRP_FSIF_NTC_LIST_LOCK()    (grp_vos_GetSemaphore( l_tFsifVos.ptNoticeSem, GRP_VOS_INFINITE))
#define GRP_FSIF_NTC_LIST_UNLOCK()  (grp_vos_ReleaseSemaphore( l_tFsifVos.ptNoticeSem))

#define GRP_FSIF_GET_SEM()          (grp_vos_GetSemaphore( l_tFsifVos.ptLockTaskSem, GRP_VOS_INFINITE))
#define GRP_FSIF_REL_SEM()          (grp_vos_ReleaseSemaphore( l_tFsifVos.ptLockTaskSem))

#define GRP_FSIF_UINT_INFO_LOCK()   (grp_vos_GetSemaphore( l_tFsifVos.ptUnitInfoSem, GRP_VOS_INFINITE))
#define GRP_FSIF_UINT_INFO_UNLOCK() (grp_vos_ReleaseSemaphore( l_tFsifVos.ptUnitInfoSem))

#define GRP_FSIF_BUF_LOCK()        (grp_vos_GetSemaphore( l_tFsifVos.ptBufSem, GRP_VOS_INFINITE))
#define GRP_FSIF_BUF_UNLOCK()      (grp_vos_ReleaseSemaphore( l_tFsifVos.ptBufSem))

/*                                                                                              */
/************************************************************************************************/


/************************************************************************************************/
/* MACRO for making 32bit data from 8bit array data                                             */
/*                                                                                              */
#define _GRP_FSIF_UINT32( pucArray)                                                             \
    (grp_u32)( (((grp_u32)*(pucArray+0))<<24)                                                   \
             | (((grp_u32)*(pucArray+1))<<16)                                                   \
             | (((grp_u32)*(pucArray+2))<< 8)                                                   \
             | (((grp_u32)*(pucArray+3))    )                                                   \
             )
/*                                                                                              */
/************************************************************************************************/


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL grp_s32 _grp_fsif_InitUnitInfo(void);
LOCAL grp_s32 _grp_fsif_CreateVos(void);
LOCAL grp_s32 _grp_fsif_RegistSubClass(void);

LOCAL grp_s32 _grp_fsif_NoticeListInit(void);
LOCAL void    _grp_fsif_Notify( _grp_fsif_unit_info *ptUnitInfo, grp_u32 ulNotifyEvent);
LOCAL grp_s32 _grp_fsif_NoticeListGet( grp_msc_notify *ptMscNotify);
LOCAL grp_s32 _grp_fsif_NoticeListSet( grp_msc_notify *ptMscNotify);
LOCAL grp_s32 _grp_fsif_FreeListInit(void);
LOCAL grp_s32 _grp_fsif_FreeListGet( _grp_fsif_unit_info **pptUnitInfo);
LOCAL grp_s32 _grp_fsif_FreeListRelease( _grp_fsif_unit_info *ptUnitInfo);
LOCAL grp_s32 _grp_fsif_AccessListInit(void);
LOCAL grp_s32 _grp_fsif_AccessListGet( grp_msc_hdr ptMscHdr, _grp_fsif_unit_info **pptUnitInfo);
LOCAL grp_s32 _grp_fsif_AccessListSet( _grp_fsif_unit_info *ptUnitInfo);
LOCAL grp_s32 _grp_fsif_PollingCheck(void);
LOCAL grp_s32 _grp_fsif_LockToPollingTask( _grp_fsif_unit_info *ptUnitInfo);
LOCAL grp_s32 _grp_fsif_UnlockToPollingTask( _grp_fsif_unit_info *ptUnitInfo);
LOCAL grp_s32 _grp_fsif_LockToAppTask( _grp_fsif_unit_info *ptUnitInfo);
LOCAL grp_s32 _grp_fsif_UnlockToAppTask( _grp_fsif_unit_info *ptUnitInfo);

LOCAL grp_s32 _grp_fsif_AccessQueInit(void);
LOCAL grp_s32 _grp_fsif_AccessQueGet( _grp_fsif_access_queue **pptAccessQue);
LOCAL grp_s32 _grp_fsif_AccessQueRelease( _grp_fsif_access_queue *ptAccessQue);

LOCAL grp_s32 _grp_fsif_Read( _grp_fsif_unit_info *ptUnitInfo, grp_u32 ulStartSector, grp_u32 ulNumOfSector, grp_u8 *pucDataBuf, grp_vos_t_queue *ptQueue);
LOCAL grp_s32 _grp_fsif_ReadCommand( _grp_fsif_unit_info *ptUnitInfo, grp_u32 ulStartSector, grp_u32 ulNumOfSector, grp_u8 *pucDataBuf, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_Write( _grp_fsif_unit_info *ptUnitInfo, grp_u32 ulStartSector, grp_u32 ulNumOfSector, grp_u8 *pucDataBuf, grp_vos_t_queue *ptQueue);
LOCAL grp_s32 _grp_fsif_WriteCommand( _grp_fsif_unit_info *ptUnitInfo, grp_u32 ulStartSector, grp_u32 ulNumOfSector, grp_u8 *pucDataBuf, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd);

LOCAL grp_s32 _grp_fsif_Notification( grp_msc_notify *ptNotify);
LOCAL void    _grp_fsif_MainTask( grp_u32 ulValue);
LOCAL grp_s32 _grp_fsif_ConnectProcess( grp_msc_notify *ptMscNotify);
LOCAL grp_s32 _grp_fsif_DisconnectProcess( grp_msc_notify *ptMscNotify);
LOCAL grp_s32 _grp_fsif_MscClose( grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_GetMaxLun( grp_msc_notify *ptMscNotify, grp_vos_t_queue *ptQueue, grp_u8 *pucMaxLun);
LOCAL grp_s32 _grp_fsif_GetMaxLunRequest( grp_msc_cmd *ptMscCmd, grp_vos_t_queue *ptQueue);
LOCAL grp_s32 _grp_fsif_RequestCallBack( grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_MediaCheck( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_Inquiry( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_InquiryCommand( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_ReadCapacity( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_ReadCapacityCommand( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_ModeSense( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_ModeSenseCommand( _grp_fsif_unit_info *ptUnitInfo, grp_u8  ucPegeCode, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_TestUnitReady( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_TestUnitReadyCommand( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_RequestSenseCommand( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd);
LOCAL grp_s32 _grp_fsif_GetErrorCode( _grp_fsif_unit_info *ptUnitInfo);


/************************************************************************************************/
/* FUNCTION   : grp_fsif_Init                                                                   */
/*                                                                                              */
/* DESCRIPTION: Initialize of thie modules                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptPrm                           initialized parameter                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_fsif_Init( grp_fsif_init_prm *ptPrm)
{
grp_s32                         lStat = GRP_FSIF_OK;

    _TRACE_FSIF_(0x00, 0x00, 0x00);

    /* set initialize paramter */
    l_tInitPrm.pfnFsifNotification = ptPrm->pfnFsifNotification;

    /* initialize of the management blocks */
    lStat = _grp_fsif_InitUnitInfo();
    if (lStat != GRP_FSIF_OK) {
        /* error */
        return GRP_FSIF_NG;
    }

    /* initialize of the list modules */
    _grp_fsif_NoticeListInit();
    _grp_fsif_FreeListInit();
    _grp_fsif_AccessListInit();

    /* initialize of the access queue management module */
    lStat = _grp_fsif_AccessQueInit();
    if (lStat != GRP_FSIF_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    /* create VOS resource */
    lStat = _grp_fsif_CreateVos();
    if (lStat != GRP_VOS_POS_RESULT) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    /* Registration function */
    lStat = _grp_fsif_RegistSubClass();
    if (lStat != GRP_MSC_OK) {
        /* error */
        return GRP_FSIF_NG;
    }

    _TRACE_FSIF_(0x00, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_InitUnitInfo                                                          */
/*                                                                                              */
/* DESCRIPTION: Get several information's buffer area                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_InitUnitInfo(void)
{
grp_si                          i;
grp_s32                         lStat;
grp_u8                          *pucCmemArea;

    _TRACE_FSIF_(0x01, 0x00, 0x00);

    /* size check */
    lStat = GRP_CMEM_BSIZE_FSIF;    /* for compile informations */
    if ((lStat < GRP_FSIF_INQUIRY_DATA_SZ)
     || (lStat < GRP_FSIF_READ_CAPACITY_DATA_SZ)
     || (lStat < GRP_FSIF_MODE_SENSE_DATA_SZ)
     || (lStat < GRP_FSIF_REQUEST_SENSE_DATA_SZ)) {
        /* error */
        return GRP_FSIF_NG;
    }

    /* create Commont memory for the MAX LUN */
    lStat = grp_cmem_BlkGet( GRP_CMEM_ID_FSIF_MAX_LUN, (void **)&l_pucMaxLun);
    if (lStat != GRP_CMEM_OK) {
        /* error */
        _TRACE_FSIF_(0x01, 0x01, _F_END);
        return GRP_FSIF_CMEM_ERROR;
    }

    /* clear all data */
    grp_std_memset( &l_tFsifCB[0], 0, sizeof(l_tFsifCB[GRP_FSIF_MAX_UINT]));

    /* create Common memory for units */
    for (i=0; i<GRP_FSIF_MAX_UINT; i++) {
        lStat = grp_cmem_BlkGet( GRP_CMEM_ID_FSIF, (void **)&pucCmemArea);
        if (lStat != GRP_CMEM_OK) {
            /* error */
            _TRACE_FSIF_(0x01, 0x02, _F_END);
            return GRP_FSIF_CMEM_ERROR;
        }

        l_tFsifCB[i].pucCmemArea = pucCmemArea;
    }

    _TRACE_FSIF_(0x01, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_CreateVos                                                             */
/*                                                                                              */
/* DESCRIPTION: Create VOS resource                                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_VOS_POS_RESULT              Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_CreateVos(void)
{
grp_s32                         lStat = GRP_VOS_POS_RESULT;

    _TRACE_FSIF_(0x02, 0x00, 0x00);

    /* create event flag */
    lStat = grp_vos_CreateFlag( &l_tFsifVos.ptNoticeFlg,        /* Event Flag Infomation Block  */
                                (grp_u8 *)"fFSIF");             /* Event Flag Name              */

    /* create semahphore */
    if (lStat == GRP_VOS_POS_RESULT) {
        lStat = grp_vos_CreateSemaphore(
                                    &l_tFsifVos.ptNoticeSem,    /* Semaphore Infomation Block   */
                                    (grp_u8 *)"sFSIF1",         /* Semaphore Name               */
                                    1);                         /* Initial Count                */
    }
    if (lStat == GRP_VOS_POS_RESULT) {
        lStat = grp_vos_CreateSemaphore(
                                    &l_tFsifVos.ptListSem,      /* Semaphore Infomation Block   */
                                    (grp_u8 *)"sFSIF2",         /* Semaphore Name               */
                                    1);                         /* Initial Count                */
    }
    if (lStat == GRP_VOS_POS_RESULT) {
        lStat = grp_vos_CreateSemaphore(
                                &l_tFsifVos.ptLockTaskSem,      /* Semaphore Infomation Block   */
                                (grp_u8 *)"sFSIF3",             /* Semaphore Name               */
                                1);                             /* Initial Count                */
    }
    if (lStat == GRP_VOS_POS_RESULT) {
        lStat = grp_vos_CreateSemaphore(
                                &l_tFsifVos.ptUnitInfoSem,      /* Semaphore Infomation Block   */
                                (grp_u8 *)"sFSIF4",             /* Semaphore Name               */
                                1);                             /* Initial Count                */
    }
    if (lStat == GRP_VOS_POS_RESULT) {
        lStat = grp_vos_CreateSemaphore(
                                &l_tFsifVos.ptBufSem,           /* Semaphore Infomation Block   */
                                (grp_u8 *)"sFSIF5",             /* Semaphore Name               */
                                1);                             /* Initial Count                */
    }

    /* create queue */
    if (lStat == GRP_VOS_POS_RESULT) {
        lStat = grp_vos_CreateQueue(
                                    &l_tFsifVos.ptMainTskQue,       /* Queue Infomation Block   */
                                    (grp_u8 *)"qFSIF1",             /* Queue name               */
                                    GRP_FSIF_MAIN_QUE_SIZE,         /* Message size(byte)       */
                                    GRP_FSIF_MAIN_QUE_CNT);         /* Message Count            */
    }
    if (lStat == GRP_VOS_POS_RESULT) {
        lStat = grp_vos_CreateQueue(
                                    &l_tFsifVos.ptPollTskQue,       /* Queue Infomation Block   */
                                    (grp_u8 *)"qFSIF2",             /* Queue name               */
                                    GRP_FSIF_POLL_QUE_SIZE,         /* Message size(byte)       */
                                    GRP_FSIF_POLL_QUE_CNT);         /* Message Count            */
    }

    /* create task */
    if (lStat == GRP_VOS_POS_RESULT) {
        lStat = grp_vos_CreateTask(
                            &l_tFsifVos.ptMainTsk,      /* Task Infomation Block Pointer Buffer */
                            (grp_u8 *)"tFSIF1",         /* Task Name                            */
                            _grp_fsif_MainTask,         /* Start Address                        */
                            GRP_FSIF_MAIN_TASK_STK,     /* Stack Size                           */
                            GRP_FSIF_MAIN_TASK_PRI,     /* Task Priority                        */
                            GRP_VOS_READY,              /* Task Initial Stat                    */
                            0);                         /* Task Start Parameter                 */
    }

    _TRACE_FSIF_(0x02, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_RegistSubClass                                                        */
/*                                                                                              */
/* DESCRIPTION: Register sub class to the mass storage modules                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_MSC_OK                      Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_RegistSubClass(void)
{
grp_msc_reg                     tMscReg;
grp_s32                         lStat;
grp_si                          i;
grp_si                          j;

    _TRACE_FSIF_(0x03, 0x00, 0x00);

    /* initialize of the lower modules */
    lStat = grp_msc_Init(GRP_USB_NULL);
    if (lStat != GRP_MSC_OK) {
        _TRACE_FSIF_(0x03, 0x01, _F_END);
        return lStat;
    }

    tMscReg.ucMode           = GRP_MSC_REG_PROTOCOL;
    tMscReg.pfnMscEvCallback = _grp_fsif_Notification;
    tMscReg.pvUserRef        = GRP_USB_NULL;
    tMscReg.usVendorId       = 0;
    tMscReg.usProductId      = 0;

    /* set subclass on BOT module */
    for (i=0; i<(grp_si)GRP_FSIF_MAX_CLASS; i++) {
        switch (i) {
        case GRP_FSIF_BOT_IDX:
            tMscReg.ucProtocol  = GRP_MSC_BOT_CODE;
            break;
        case GRP_FSIF_CBI_IDX:
            tMscReg.ucProtocol  = GRP_MSC_CBI_CODE;
            break;
        case GRP_FSIF_CB_IDX:
            tMscReg.ucProtocol  = GRP_MSC_CB_CODE;
            break;
        default:
            /* DIRECT RETURN */
            _TRACE_FSIF_(0x03, 0x02, _F_END);
            return GRP_FSIF_ILLEAGAL_ERROR;
        }

        for (j=0; j<(grp_si)GRP_FSIF_MAX_SUBCLASS; j++) {
            if (l_aucRegistClass[i][j]) {
                switch (j) {
                case GRP_FSIF_SCSI_IDX:
                    tMscReg.ucSubClass = GRP_MSC_SCSI_CODE;
                    break;
                case GRP_FSIF_ATAPI_IDX:
                    tMscReg.ucSubClass = GRP_MSC_ATAPI_CODE;
                    break;
                case GRP_FSIF_SFF8070I_IDX:
                    tMscReg.ucSubClass = GRP_MSC_SFF8070I_CODE;
                    break;
                case GRP_FSIF_UFI_IDX:
                    tMscReg.ucSubClass = GRP_MSC_UFI_CODE;
                    break;
                default:
                    /* DIRECT RETURN */
                    _TRACE_FSIF_(0x03, 0x03, _F_END);
                    return GRP_FSIF_ILLEAGAL_ERROR;
                }
                lStat = grp_msc_Register( &tMscReg);
                if (lStat != GRP_MSC_OK) {
                    _TRACE_FSIF_(0x03, 0x04, _F_END);
                    return lStat;
                }
            }
        }
    }

    _TRACE_FSIF_(0x03, 0x00, _F_END);

    return lStat;
}

/*----------------------------------------------------------------------------------------------*/
/*  Connect/Dosconnect notification management list                                             */
/*----------------------------------------------------------------------------------------------*/
/************************************************************************************************/
/* FUNCTION   : _grp_fsif_NoticeListInit                                                        */
/*                                                                                              */
/* DESCRIPTION: Initialize notice list                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success only                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_NoticeListInit(void)
{
    _TRACE_FSIF_(0x04, 0x00, 0x00);

    /* set initial value */
    l_tNoticeList.ptPrev = &l_tNoticeList;
    l_tNoticeList.ptNext = &l_tNoticeList;

    /* clear all data */
    grp_std_memset( &l_tFsifNtc[0], 0, sizeof(l_tFsifNtc));

    _TRACE_FSIF_(0x04, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_Notify                                                                */
/*                                                                                              */
/* DESCRIPTION: Event notify to the upper layer.                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ulNotifyEvent                   event (attach or detache)                       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_fsif_Notify( _grp_fsif_unit_info *ptUnitInfo, grp_u32 ulNotifyEvent)
{
    if (ulNotifyEvent == GRP_FSIF_ATTACHED_MEDIA) {
        if (ptUnitInfo->ucNotifyStat == GRP_FSIF_NS_DISCONNECT) {
            /* Notify attaced media */
            if (l_tInitPrm.pfnFsifNotification != GRP_USB_NULL) {
                (*l_tInitPrm.pfnFsifNotification)( ulNotifyEvent, (void*)ptUnitInfo, ptUnitInfo->ucIndex);
            }
            /* change the status */
            ptUnitInfo->ucNotifyStat = GRP_FSIF_NS_CONNECT;
        }
    }
    else {
        if (ptUnitInfo->ucNotifyStat == GRP_FSIF_NS_CONNECT) {
            /* Notify attaced media */
            if (l_tInitPrm.pfnFsifNotification != GRP_USB_NULL) {
                (*l_tInitPrm.pfnFsifNotification)( ulNotifyEvent, (void*)ptUnitInfo, ptUnitInfo->ucIndex);
            }
            /* change the status */
            ptUnitInfo->ucNotifyStat = GRP_FSIF_NS_DISCONNECT;
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_NoticeListGet                                                         */
/*                                                                                              */
/* DESCRIPTION: Get free list                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMscNotify                     pointer of ms_c structure                       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_NG                     no free list                                    */
/*              GRP_FSIF_VOS_ERROR              VOS error                                       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_NoticeListGet( grp_msc_notify *ptMscNotify)
{
_grp_fsif_list                  *ptList;
_grp_fsif_notice                *ptGetList;
grp_s32                         lStat = GRP_FSIF_OK;

    _TRACE_FSIF_(0x05, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if (GRP_FSIF_NTC_LIST_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    ptList = l_tNoticeList.ptNext;
    if (ptList == &l_tNoticeList) {
        /* no list */
        lStat = GRP_FSIF_NG;
    }
    else {
        l_tNoticeList.ptNext    = ptList->ptNext;
        ptList->ptNext->ptPrev  = &l_tNoticeList;

        ptGetList = (_grp_fsif_notice *)ptList;
        /* clear infromations */
        ptGetList->ptPrev = GRP_USB_NULL;
        ptGetList->ptNext = GRP_USB_NULL;
        ptGetList->lFlag  = GRP_FSIF_NO_USE;
        /* set parameters */
        ptMscNotify->iEvent     = ptGetList->tMscNotify.iEvent;
        ptMscNotify->hMscHdr    = ptGetList->tMscNotify.hMscHdr;
        ptMscNotify->ucSubClass = ptGetList->tMscNotify.ucSubClass;
        ptMscNotify->ucProtocol = ptGetList->tMscNotify.ucProtocol;
        ptMscNotify->pvUserRef  = ptGetList->tMscNotify.pvUserRef;
    }

    /* unlock */                                                                        /* }    */
    if (GRP_FSIF_NTC_LIST_UNLOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    _TRACE_FSIF_(0x05, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_NoticeListSet                                                         */
/*                                                                                              */
/* DESCRIPTION: Set specified block to the list                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMscNotify                     pointer of ms_c structure                       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_NG                     don't set to the list                           */
/*              GRP_FSIF_VOS_ERROR              VOS error                                       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_NoticeListSet( grp_msc_notify *ptMscNotify)
{
_grp_fsif_list                  *ptList;
_grp_fsif_notice                *ptTmpList  = GRP_USB_NULL;
_grp_fsif_notice                *ptSetList  = GRP_USB_NULL;
grp_s32                         lStat       = GRP_FSIF_NG;
grp_si                          i;

    _TRACE_FSIF_(0x06, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if (GRP_FSIF_NTC_LIST_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    /* check same information block */
    if (ptMscNotify->iEvent == GRP_MSC_DETACHED) {
        /* disconnect */
        ptList  = l_tNoticeList.ptNext;
        lStat   = GRP_FSIF_NG;

        for (i=0; i<GRP_FSIF_MAX_NTC_QUEUE; i++) {
            if (ptList == (_grp_fsif_list *)&l_tNoticeList) {
                /* no same informations block */
                lStat  = GRP_FSIF_OK;
                break;
            }

            /* check same device informations */
            ptTmpList = (_grp_fsif_notice *)ptList;

            if ((ptTmpList->tMscNotify.iEvent  == GRP_MSC_ATTACHED)
             && (ptTmpList->tMscNotify.hMscHdr == ptMscNotify->hMscHdr)) {
                /* deleted block */
                lStat = GRP_FSIF_NG;

                ptList->ptPrev->ptNext  = ptList->ptNext;
                ptList->ptNext->ptPrev  = ptList->ptPrev;

                ptList->ptPrev = GRP_USB_NULL;
                ptList->ptNext = GRP_USB_NULL;

                /* release block */
                ptTmpList->lFlag  = GRP_FSIF_NO_USE;
                break;
            }

            ptList = ptList->ptNext;
        }
    }
    else if (ptMscNotify->iEvent == GRP_MSC_ATTACHED) {
        lStat = GRP_FSIF_OK;
    }

    if (lStat == GRP_FSIF_OK) {
        /* search no used block */
        lStat = GRP_FSIF_NG;
        for (i=0; i<GRP_FSIF_MAX_NTC_QUEUE; i++) {
            if (l_tFsifNtc[i].lFlag == GRP_FSIF_NO_USE) {
                /* discovered */
                l_tFsifNtc[i].lFlag = GRP_FSIF_USED;

                ptSetList   = &l_tFsifNtc[i];
                lStat       = GRP_FSIF_OK;
                /* set parameters */
                ptSetList->tMscNotify.iEvent        = ptMscNotify->iEvent;
                ptSetList->tMscNotify.hMscHdr       = ptMscNotify->hMscHdr;
                ptSetList->tMscNotify.ucSubClass    = ptMscNotify->ucSubClass;
                ptSetList->tMscNotify.ucProtocol    = ptMscNotify->ucProtocol;
                ptSetList->tMscNotify.pvUserRef     = ptMscNotify->pvUserRef;
                break;
            }
        }

        if (lStat == GRP_FSIF_OK) {
            /* chaine the list */
            ptList = l_tNoticeList.ptPrev;

            ptList->ptNext          = (_grp_fsif_list *)ptSetList;
            l_tNoticeList.ptPrev    = (_grp_fsif_list *)ptSetList;

            ptSetList->ptNext   = &l_tNoticeList;
            ptSetList->ptPrev   = ptList;
        }
    }

    /* unlock */                                                                        /* }    */
    if (GRP_FSIF_NTC_LIST_UNLOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    _TRACE_FSIF_(0x06, 0x00, _F_END);

    return lStat;
}

/*----------------------------------------------------------------------------------------------*/
/*  Free block management list                                                                  */
/*----------------------------------------------------------------------------------------------*/
/************************************************************************************************/
/* FUNCTION   : _grp_fsif_FreeListInit                                                          */
/*                                                                                              */
/* DESCRIPTION: Initialize free list                                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success only                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_FreeListInit(void)
{
_grp_fsif_list                  *ptList;
grp_si                          i;

    _TRACE_FSIF_(0x07, 0x00, 0x00);

    /* set initial value */
    l_tFreeList.ptPrev = &l_tFreeList;
    l_tFreeList.ptNext = &l_tFreeList;

    for (i=0; i<GRP_FSIF_MAX_UINT; i++) {
        l_tFsifCB[i].ucIndex = (grp_u8)i;

        ptList = (_grp_fsif_list *)&l_tFsifCB[i];

        ptList->ptPrev = l_tFreeList.ptPrev;
        ptList->ptNext = &l_tFreeList;

        l_tFreeList.ptPrev->ptNext  = ptList;
        l_tFreeList.ptPrev          = ptList;
    }

    _TRACE_FSIF_(0x07, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_FreeListGet                                                           */
/*                                                                                              */
/* DESCRIPTION: Get free list                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : pptUnitInfo                     pointer of unit information block               */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_NG                     not free list                                   */
/*              GRP_FSIF_VOS_ERROR              VOS error                                       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_FreeListGet( _grp_fsif_unit_info **pptUnitInfo)
{
_grp_fsif_list                  *ptGetList;
grp_s32                         lStat = GRP_FSIF_OK;

    _TRACE_FSIF_(0x08, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if (GRP_FSIF_LIST_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    ptGetList = l_tFreeList.ptNext;

    if (ptGetList == &l_tFreeList) {
        /* no list */
        *pptUnitInfo = GRP_USB_NULL;
        lStat        = GRP_FSIF_NG;
    }
    else {
        l_tFreeList.ptNext          = ptGetList->ptNext;
        ptGetList->ptNext->ptPrev   = &l_tFreeList;

        ptGetList->ptPrev = GRP_USB_NULL;
        ptGetList->ptNext = GRP_USB_NULL;

        *pptUnitInfo = (_grp_fsif_unit_info*)ptGetList;
    }

    /* unlock */                                                                        /* }    */
    if (GRP_FSIF_LIST_UNLOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    _TRACE_FSIF_(0x08, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_FreeListRelease                                                       */
/*                                                                                              */
/* DESCRIPTION: Release free list                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_VOS_ERROR              VOS error                                       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_FreeListRelease( _grp_fsif_unit_info *ptUnitInfo)
{
_grp_fsif_list                  *ptListEnd;
_grp_fsif_list                  *ptRelList = (_grp_fsif_list *)ptUnitInfo;

    _TRACE_FSIF_(0x09, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if (GRP_FSIF_LIST_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    ptListEnd = l_tFreeList.ptPrev;

    ptListEnd->ptNext   = ptRelList;
    l_tFreeList.ptPrev  = ptRelList;

    ptRelList->ptNext   = &l_tFreeList;
    ptRelList->ptPrev   = ptListEnd;

    /* unlock */                                                                        /* }    */
    if (GRP_FSIF_LIST_UNLOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    _TRACE_FSIF_(0x09, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/*----------------------------------------------------------------------------------------------*/
/*  Access block management list                                                                */
/*----------------------------------------------------------------------------------------------*/
/************************************************************************************************/
/* FUNCTION   : _grp_fsif_AccessListInit                                                        */
/*                                                                                              */
/* DESCRIPTION: Initialize access list                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success only                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_AccessListInit(void)
{
    _TRACE_FSIF_(0x0A, 0x00, 0x00);

    /* set initial value */
    l_tAccessList.ptPrev = &l_tAccessList;
    l_tAccessList.ptNext = &l_tAccessList;

    _TRACE_FSIF_(0x0A, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_AccessListGet                                                         */
/*                                                                                              */
/* DESCRIPTION: Search access list                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMscHdr                        MSC module handler                              */
/* OUTPUT     : pptUnitInfo                     pointer of unit information block               */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_NG                     no same list                                    */
/*              GRP_FSIF_VOS_ERROR              VOS error                                       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_AccessListGet( grp_msc_hdr ptMscHdr, _grp_fsif_unit_info **pptUnitInfo)
{
_grp_fsif_list                  *ptSearchList;
_grp_fsif_unit_info             *ptTmp;
_grp_fsif_unit_info             *ptRet  = GRP_USB_NULL;
grp_s32                         lStat   = GRP_FSIF_OK;

    _TRACE_FSIF_(0x0B, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if (GRP_FSIF_LIST_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    ptSearchList = l_tAccessList.ptNext;

    /* search same handler */
    while (ptSearchList != &l_tAccessList) {
        ptTmp = (_grp_fsif_unit_info *)ptSearchList;
        if (ptTmp->ptMscHdr == ptMscHdr) {
            /* corresponded information */
            ptRet = ptTmp;
            break;
        }
        else {
            /* next information */
            ptSearchList = ptSearchList->ptNext;
        }
    }

    if (ptRet != GRP_USB_NULL) {
        ptSearchList->ptPrev->ptNext    = ptSearchList->ptNext;
        ptSearchList->ptNext->ptPrev    = ptSearchList->ptPrev;

        ptSearchList->ptPrev = GRP_USB_NULL;
        ptSearchList->ptNext = GRP_USB_NULL;

        *pptUnitInfo = ptRet;
    }
    else {
        *pptUnitInfo    = GRP_USB_NULL;
        lStat           = GRP_FSIF_NG;
    }

    /* unlock */                                                                        /* }    */
    if (GRP_FSIF_LIST_UNLOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    _TRACE_FSIF_(0x0B, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_AccessListSet                                                         */
/*                                                                                              */
/* DESCRIPTION: Set access list                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_VOS_ERROR              VOS error                                       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_AccessListSet( _grp_fsif_unit_info *ptUnitInfo)
{
_grp_fsif_list                  *ptListEnd;
_grp_fsif_list                  *ptSetList = (_grp_fsif_list *)ptUnitInfo;

    _TRACE_FSIF_(0x0C, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if (GRP_FSIF_LIST_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    ptListEnd = l_tAccessList.ptPrev;

    ptListEnd->ptNext       = ptSetList;
    l_tAccessList.ptPrev    = ptSetList;

    ptSetList->ptNext   = &l_tAccessList;
    ptSetList->ptPrev   = ptListEnd;

    /* unlock */                                                                        /* }    */
    if (GRP_FSIF_LIST_UNLOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    _TRACE_FSIF_(0x0C, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_PollingCheck                                                          */
/*                                                                                              */
/* DESCRIPTION: Execute Test Unit Ready command to check a media existence.                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_PollingCheck(void)
{
_grp_fsif_unit_info             *ptUnitInfo;
_grp_fsif_list                  *ptList = l_tAccessList.ptNext;
grp_msc_cmd                     *ptMscCmd   = &l_tPolMscCmd;
grp_s32                         lStat;

    _TRACE_FSIF_(0x0D, 0x00, 0x00);

    for (;;) {
        if (ptList == &l_tAccessList) {
            /* no list */
            break;                                                      /* exit this FOR loop   */
        }

        ptUnitInfo = (_grp_fsif_unit_info*)ptList;

        if (ptUnitInfo->ucRmb & GRP_FSIF_RMB) {

            lStat = _grp_fsif_LockToAppTask( ptUnitInfo);                               /* {    */
            if (lStat == GRP_FSIF_OK) {
                lStat = _grp_fsif_TestUnitReady( ptUnitInfo, l_tFsifVos.ptPollTskQue, ptMscCmd);
                if (lStat == GRP_FSIF_OK) {
                    if (ptUnitInfo->ucNotifyStat == GRP_FSIF_NS_DISCONNECT) {
                        /* media check sequence */
                        lStat = _grp_fsif_MediaCheck( ptUnitInfo, l_tFsifVos.ptPollTskQue, ptMscCmd);
                        if (lStat == GRP_FSIF_OK) {
                            /* attached media */
                            _grp_fsif_Notify( ptUnitInfo, GRP_FSIF_ATTACHED_MEDIA);
                        }
                    }
                }
                else if (lStat == GRP_FSIF_ILLEAGAL_ERROR) {
                    if (_grp_fsif_UnlockToAppTask( ptUnitInfo) != GRP_FSIF_OK) {        /* }    */
                        /* illegal error */
                    }
                    /* device is detected */
                    break;                                              /* exit this FOR loop   */
                }
                else {
                    if (ptUnitInfo->ucNotifyStat == GRP_FSIF_NS_CONNECT) {
                        /* detached media */
                        _grp_fsif_Notify( ptUnitInfo, GRP_FSIF_DETACHED_MEDIA);
                    }
                }

                lStat = _grp_fsif_UnlockToAppTask( ptUnitInfo);                         /* }    */
                if (lStat != GRP_FSIF_OK) {
                    /* illegal error */
                    break;
                }
            }
            else if (lStat != GRP_FSIF_BUSY) {
                /* illegal error */
                break;
            }
        }
        /* renew next pointer */
        if (ptList->ptNext) {
            ptList = ptList->ptNext;
        }
        else {
            ptList = l_tAccessList.ptNext;
        }
    }
    _TRACE_FSIF_(0x0D, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/*----------------------------------------------------------------------------------------------*/
/*  Exclusive control of Polling task and Application task                                      */
/*----------------------------------------------------------------------------------------------*/
/************************************************************************************************/
/* FUNCTION   : _grp_fsif_LockToPollingTask                                                     */
/*                                                                                              */
/* DESCRIPTION: Lock to the polling task                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_VOS_ERROR              VOS error                                       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_LockToPollingTask( _grp_fsif_unit_info *ptUnitInfo)
{
    _TRACE_FSIF_(0x0E, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if (GRP_FSIF_UINT_INFO_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    if (ptUnitInfo->ucPriorityFlag == GRP_FSIF_PF_ALREADY_USE) {
        /* unlock */                                                                    /* }    */
        if (GRP_FSIF_UINT_INFO_UNLOCK() != GRP_FSIF_LOCK_OK) {
            /* error */
            return GRP_FSIF_VOS_ERROR;
        }

        if (GRP_FSIF_GET_SEM() != GRP_FSIF_LOCK_OK) {
            /* error */
            return GRP_FSIF_VOS_ERROR;
        }

        ptUnitInfo->ucPriorityFlag = GRP_FSIF_PF_ALREADY_USE;

        if (GRP_FSIF_REL_SEM() != GRP_FSIF_LOCK_OK) {
            /* error */
            return GRP_FSIF_VOS_ERROR;
        }
    }
    else {
        ptUnitInfo->ucPriorityFlag = GRP_FSIF_PF_ALREADY_USE;

        /* unlock */                                                                    /* }    */
        if (GRP_FSIF_UINT_INFO_UNLOCK() != GRP_FSIF_LOCK_OK) {
            /* error */
            return GRP_FSIF_VOS_ERROR;
        }
    }

    _TRACE_FSIF_(0x0E, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_UnlockToPollingTask                                                   */
/*                                                                                              */
/* DESCRIPTION: Unlocke the polling task.                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_VOS_ERROR              VOS error                                       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_UnlockToPollingTask( _grp_fsif_unit_info *ptUnitInfo)
{
    _TRACE_FSIF_(0x0F, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if (GRP_FSIF_UINT_INFO_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    ptUnitInfo->ucPriorityFlag = GRP_FSIF_PF_NO_USE;

    /* unlock */                                                                        /* }    */
    if (GRP_FSIF_UINT_INFO_UNLOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    _TRACE_FSIF_(0x0F, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_LockToAppTask                                                         */
/*                                                                                              */
/* DESCRIPTION: Lock to the application task.                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_BUSY                   already used (not error)                        */
/*              GRP_FSIF_VOS_ERROR              VOS error                                       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_LockToAppTask( _grp_fsif_unit_info *ptUnitInfo)
{
    _TRACE_FSIF_(0x10, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if (GRP_FSIF_UINT_INFO_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_VOS_ERROR;
    }

    if (ptUnitInfo->ucPriorityFlag == GRP_FSIF_PF_ALREADY_USE) {
        _TRACE_FSIF_(0x10, 0x01, 0x00);
        /* unlock */                                                                    /* }    */
        if (GRP_FSIF_UINT_INFO_UNLOCK() != GRP_FSIF_LOCK_OK) {
            /* error */
            return GRP_FSIF_VOS_ERROR;
        }

        /* already use */
        return GRP_FSIF_BUSY;
    }
    else {
        _TRACE_FSIF_(0x10, 0x02, 0x00);
        /* get semaphore */
        if (GRP_FSIF_GET_SEM() != GRP_FSIF_LOCK_OK) {
            /* error */
            return GRP_FSIF_VOS_ERROR;
        }

        ptUnitInfo->ucPriorityFlag = GRP_FSIF_PF_ALREADY_USE;

        /* unlock */                                                                    /* }    */
        if (GRP_FSIF_UINT_INFO_UNLOCK() != GRP_FSIF_LOCK_OK) {
            /* error */
            return GRP_FSIF_VOS_ERROR;
        }
    }

    _TRACE_FSIF_(0x10, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_UnlockToAppTask                                                       */
/*                                                                                              */
/* DESCRIPTION: Unlock the application task.                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_VOS_ERROR              VOS error                                       */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_UnlockToAppTask( _grp_fsif_unit_info *ptUnitInfo)
{
    _TRACE_FSIF_(0x11, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if (GRP_FSIF_UINT_INFO_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_NG;
    }

    ptUnitInfo->ucPriorityFlag = GRP_FSIF_PF_NO_USE;

    /* unlock */                                                                        /* }    */
    if (GRP_FSIF_UINT_INFO_UNLOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_NG;
    }

    if (GRP_FSIF_REL_SEM() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_NG;
    }

    _TRACE_FSIF_(0x11, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/*----------------------------------------------------------------------------------------------*/
/*  Access queue management modules                                                             */
/*----------------------------------------------------------------------------------------------*/
/************************************************************************************************/
/* FUNCTION   : _grp_fsif_AccessQueInit                                                         */
/*                                                                                              */
/* DESCRIPTION: Initialize of access queue management module                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_AccessQueInit(void)
{
grp_si                          i;
grp_s32                         lStat;
grp_u8                          aucQueName[] = "qFSIFMX0";

    _TRACE_FSIF_(0x12, 0x00, 0x00);

    /* create semaphore */
    lStat = grp_vos_CreateSemaphore(
                                &l_tAccessMng.ptAccessCntSem,   /* Semaphore Infomation Block   */
                                (grp_u8 *)"sFSIF6",             /* Semaphore Name               */
                                GRP_FSIF_MAX_ACCESS);           /* Initial Count                */
    if (lStat != GRP_VOS_POS_RESULT) {
        /* error */
        return lStat;
    }

    for (i=0; i<GRP_FSIF_MAX_ACCESS; i++) {
        /* create unique queue name */
        if (i<10) {
            aucQueName[7] = (grp_u8)('0' + i);
        }
        else {
            aucQueName[7] = (grp_u8)('A' + i-10);
        }

        l_tAccessMng.tAccessQue[i].lFlag = GRP_FSIF_NO_USE;
        /* create queue */
        lStat = grp_vos_CreateQueue(
                        &l_tAccessMng.tAccessQue[i].ptAccessQue,    /* Queue Infomation Block   */
                        aucQueName,                                 /* Queue name               */
                        GRP_FSIF_ACCESS_QUE_SIZE,                   /* Message size(byte)       */
                        GRP_FSIF_ACCESS_QUE_CNT);                   /* Message Count            */
        if (lStat != GRP_VOS_POS_RESULT) {
            /* error */
            return lStat;
        }
    }

    _TRACE_FSIF_(0x12, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_AccessQueGet                                                          */
/*                                                                                              */
/* DESCRIPTION: get queue informations                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : pptAccessQue                    queue information pointer                       */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_AccessQueGet( _grp_fsif_access_queue  **pptAccessQue)
{
grp_si                          i;
grp_s32                         lStat = GRP_FSIF_NG;

    _TRACE_FSIF_(0x13, 0x00, 0x00);

    *pptAccessQue = GRP_USB_NULL;

    /* get semaphore */
    if (grp_vos_GetSemaphore( l_tAccessMng.ptAccessCntSem, GRP_VOS_INFINITE) != GRP_VOS_POS_RESULT) {
        /* error */
        return GRP_FSIF_NG;
    }

    /* lock */                                                                          /* {    */
    if (GRP_FSIF_LIST_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_NG;
    }

    /* search no_used block */
    for (i=0; i<GRP_FSIF_MAX_ACCESS; i++) {
        if (l_tAccessMng.tAccessQue[i].lFlag == GRP_FSIF_NO_USE) {
            /* discoverd */
            lStat           = GRP_FSIF_OK;
            *pptAccessQue   = &l_tAccessMng.tAccessQue[i];
            /* set used flag */
            l_tAccessMng.tAccessQue[i].lFlag = GRP_FSIF_USED;
            break;
        }
    }

    /* unlock */                                                                        /* }    */
    if (GRP_FSIF_LIST_UNLOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_NG;
    }

    _TRACE_FSIF_(0x13, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_AccessQueRelease                                                      */
/*                                                                                              */
/* DESCRIPTION: release queue informations                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptAccessQue                     queue information pointer                       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_AccessQueRelease( _grp_fsif_access_queue *ptAccessQue)
{
    _TRACE_FSIF_(0x14, 0x00, 0x00);

    /* lock */                                                                          /* {    */
    if (GRP_FSIF_LIST_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_NG;
    }

    /* set no_use flag */
    ptAccessQue->lFlag = GRP_FSIF_NO_USE;

    /* unlock */                                                                        /* }    */
    if (GRP_FSIF_LIST_UNLOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_NG;
    }

    /* get semaphore */
    if (grp_vos_ReleaseSemaphore( l_tAccessMng.ptAccessCntSem) != GRP_VOS_POS_RESULT) {
        /* error */
        return GRP_FSIF_NG;
    }

    _TRACE_FSIF_(0x14, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_fsif_ReadSector                                                             */
/*                                                                                              */
/* DESCRIPTION: read sector                                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pvHdr                           handler pointer                                 */
/*              ulStartSector                   start sector number to read                     */
/*              ulNumOfSector                   read sector size                                */
/*              pucDataBuf                      data buffer pointer                             */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_fsif_ReadSector( void *pvHdr, grp_u32 ulStartSector, grp_u32 ulNumOfSector, grp_u8* pucDataBuf)
{
_grp_fsif_access_queue          *ptQueueInfo;
_grp_fsif_unit_info             *ptUnitInfo     = (_grp_fsif_unit_info*)pvHdr;
grp_s32                         lStat           = GRP_FSIF_OK;
grp_u32                         ulIndex;
grp_u32                         ulReadSector;
grp_u32                         ulReadSectorNum;
grp_u32                         ulRemainCnt;
grp_u32                         ulSectorSize;

    _TRACE_FSIF_(0x15, 0x00, 0x00);

    if (ulNumOfSector == 0) {
        return GRP_FSIF_PARAM_ERROR;
    }

    /* get queue information */
    if (_grp_fsif_AccessQueGet( &ptQueueInfo) != GRP_FSIF_OK) {
        return GRP_FSIF_QUEINFO_ERROR;
    }

    /* set initial parameters */
    ulReadSector    = ulStartSector;
    ulRemainCnt     = ulNumOfSector;
    ulIndex         = 0;
    ulSectorSize    = ptUnitInfo->ulSectorSize;

    if (ulSectorSize != 0) {
        while (ulRemainCnt != 0) {
            if (ptUnitInfo->ucUnitStatus & GRP_FSIF_US_ATTACHED) {
                /* calcurate read count */
                if (ulRemainCnt > (GRP_FSIF_MAX_DATA_LEN / ulSectorSize)) {
                    ulReadSectorNum  = GRP_FSIF_MAX_DATA_LEN / ulSectorSize;
                    ulRemainCnt     -= ulReadSectorNum;
                }
                else {
                    ulReadSectorNum  = ulRemainCnt;
                    ulRemainCnt      = 0;
                }

                /* read sector */
                lStat = _grp_fsif_Read( ptUnitInfo,                 
                                        ulReadSector,               /* Start sector number  */
                                        ulReadSectorNum,            /* Read sector count    */
                                        &pucDataBuf[ulIndex],       /* Pointer of buffer    */
                                        ptQueueInfo->ptAccessQue);  
                if (lStat != GRP_FSIF_OK) {
                    /* error */
                    break;
                }

                /* renew counters */
                ulIndex         += (ulReadSectorNum * ulSectorSize);
                ulReadSector    += ulReadSectorNum;
            }
            else {
                lStat = GRP_FSIF_NG;
                break;
            }
        }
    }
    else {
        lStat = GRP_FSIF_NG;
    }

    /* release queue information */
    if (_grp_fsif_AccessQueRelease( ptQueueInfo) != GRP_FSIF_OK) {
        return GRP_FSIF_QUEINFO_ERROR;
    }

    _TRACE_FSIF_(0x15, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_Read                                                                  */
/*                                                                                              */
/* DESCRIPTION: Execute read command                                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ulStartSector                   start sector number to read                     */
/*              ulNumOfSector                   read sector size                                */
/*              pucDataBuf                      data buffer pointer                             */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out error                                  */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_Read( _grp_fsif_unit_info *ptUnitInfo, grp_u32 ulStartSector, grp_u32 ulNumOfSector, grp_u8 *pucDataBuf, grp_vos_t_queue *ptQueue)
{
grp_s32                         lStat;

    _TRACE_FSIF_(0x16, 0x00, 0x00);

    if (_grp_fsif_LockToPollingTask( ptUnitInfo) != GRP_FSIF_OK) {
        /* illegal error */
        return GRP_FSIF_ILLEAGAL_ERROR;
    }

    /* Test Unit Ready command */
    lStat = _grp_fsif_ReadCommand( ptUnitInfo, ulStartSector, ulNumOfSector, pucDataBuf, ptQueue, &ptUnitInfo->tMscCmd);
    switch (lStat) {
    case GRP_FSIF_OK:
        /* NO BREAK */
    case GRP_FSIF_TMOUT:
        break;

    case GRP_FSIF_CHECK_CONDITION:
        /* Request Sense command */
        lStat = _grp_fsif_RequestSenseCommand( ptUnitInfo, ptQueue, &ptUnitInfo->tMscCmd);
        if (lStat == GRP_FSIF_OK) {
            _TRACE_FSIF_(0x16, 0x01, 0x00);
            lStat = _grp_fsif_GetErrorCode( ptUnitInfo);

            if (lStat == GRP_FSIF_EC_NOT_RDY) {
                /* detached media */
                _grp_fsif_Notify( ptUnitInfo, GRP_FSIF_DETACHED_MEDIA);
            }
        }
        else {
            _TRACE_FSIF_(0x16, 0x02, 0x00);
        }
        break;
    default:
        lStat = GRP_FSIF_ILLEAGAL_ERROR;
        break;
    }

    if (_grp_fsif_UnlockToPollingTask( ptUnitInfo) != GRP_FSIF_OK) {
        /* illegal error */
        return GRP_FSIF_ILLEAGAL_ERROR;
    }

    _TRACE_FSIF_(0x16, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_ReadCommand                                                           */
/*                                                                                              */
/* DESCRIPTION: Execute read command                                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ulStartSector                   start sector number to read                     */
/*              ulNumOfSector                   read sector size                                */
/*              pucDataBuf                      data buffer pointer                             */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out error                                  */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_ReadCommand( _grp_fsif_unit_info *ptUnitInfo, grp_u32 ulStartSector, grp_u32 ulNumOfSector, grp_u8 *pucDataBuf, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd)
{
grp_s32                         lStat;
grp_s32                         lTmpStat;
grp_u32                         ulMsg;

    _TRACE_FSIF_(0x17, 0x00, 0x00);

    /*  clear queue */
    while (grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_VOS_NOWAIT) == GRP_VOS_POS_RESULT);

    /* set parameters */
    ptMscCmd->hMscHdr       = ptUnitInfo->ptMscHdr;
    ptMscCmd->ucLun         = ptUnitInfo->ucOwnLun;
    ptMscCmd->pucReqBuffer  = pucDataBuf;
    ptMscCmd->ulReqLength   = (ulNumOfSector * ptUnitInfo->ulSectorSize);
    ptMscCmd->pfnCallback   = _grp_fsif_RequestCallBack;
    ptMscCmd->pvUserRef     = (void *)ptQueue;

    lStat = grp_msc_ReadSector( ptMscCmd, ulStartSector, ulNumOfSector);
    if (lStat == GRP_MSC_OK) {
        /* wait queue */
        lStat = grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_FSIF_WAIT_TIME);
        if (lStat == GRP_VOS_POS_RESULT) {
            /* check status */
            if (ptMscCmd->lStatus == GRP_MSC_NO_FAIL) {
                /* command success */
                lStat = GRP_FSIF_OK;
            }
            else if (ptMscCmd->lStatus == GRP_MSC_CHECK_CONDITION) {
                /* need Request Sense */
                lStat = GRP_FSIF_CHECK_CONDITION;
            }
            else {
                /* command failed */
                lStat = GRP_FSIF_NG;
            }
        }
        else {
            if (lStat == GRP_VOS_NEG_TIMEOUT) {
                /* time out */
                lStat = GRP_FSIF_TMOUT;
            }
            else {
                /* illeagal error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }

        if (lStat == GRP_FSIF_TMOUT) {
            /* cancel request */
            lTmpStat = grp_msc_Cancel( ptMscCmd);
            if (lTmpStat != GRP_FSIF_OK) {
                /* error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }
    }

    _TRACE_FSIF_(0x17, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : grp_fsif_WriteSector                                                            */
/*                                                                                              */
/* DESCRIPTION: write sector                                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pvHdr                           handler pointer                                 */
/*              ulStartSector                   start sector number to write                    */
/*              ulNumOfSector                   write sector size                               */
/*              pucDataBuf                      data buffer pointer                             */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_fsif_WriteSector( void *pvHdr, grp_u32 ulStartSector, grp_u32 ulNumOfSector, grp_u8 *pucDataBuf)
{
_grp_fsif_access_queue          *ptQueueInfo;
_grp_fsif_unit_info             *ptUnitInfo     = (_grp_fsif_unit_info*)pvHdr;
grp_s32                         lStat           = GRP_FSIF_OK;
grp_u32                         ulIndex;
grp_u32                         ulWriteSector;
grp_u32                         ulWriteSectorNum;
grp_u32                         ulRemainCnt;
grp_u32                         ulSectorSize;

    _TRACE_FSIF_(0x18, 0x00, 0x00);

    if (ulNumOfSector == 0) {
        _TRACE_FSIF_(0x18, 0x01, _F_END);
        return GRP_FSIF_PARAM_ERROR;
    }

    /* get queue information */
    if (_grp_fsif_AccessQueGet( &ptQueueInfo) != GRP_FSIF_OK) {
        return GRP_FSIF_QUEINFO_ERROR;
    }

    /* set initial parameters */
    ulWriteSector   = ulStartSector;
    ulRemainCnt     = ulNumOfSector;
    ulIndex         = 0;
    ulSectorSize    = ptUnitInfo->ulSectorSize;

    if (ulSectorSize != 0) {
        while (ulRemainCnt != 0) {
            if (ptUnitInfo->ucUnitStatus &  GRP_FSIF_US_ATTACHED) {
                /* calcurate write count */
                if (ulRemainCnt > (GRP_FSIF_MAX_DATA_LEN / ulSectorSize)) {
                    ulWriteSectorNum = GRP_FSIF_MAX_DATA_LEN / ulSectorSize;
                    ulRemainCnt     -= ulWriteSectorNum;
                }
                else {
                    ulWriteSectorNum = ulRemainCnt;
                    ulRemainCnt      = 0;
                }

                /* write sector */
                lStat = _grp_fsif_Write(    ptUnitInfo,                 
                                            ulWriteSector,              /* Start sector number  */
                                            ulWriteSectorNum,           /* Write sector count   */
                                            &pucDataBuf[ulIndex],       /* Pointer of buffer    */
                                            ptQueueInfo->ptAccessQue);  
                if (lStat != GRP_FSIF_OK) {
                    /* error */
                    break;
                }

                /* renew counters */
                ulIndex         += (ulWriteSectorNum * ulSectorSize);
                ulWriteSector   += ulWriteSectorNum;
            }
            else {
                lStat = GRP_FSIF_NG;
                break;
            }
        }
    }
    else {
        lStat = GRP_FSIF_NG;
    }

    /* release queue information */
    if (_grp_fsif_AccessQueRelease( ptQueueInfo) != GRP_FSIF_OK) {
        return GRP_FSIF_QUEINFO_ERROR;
    }

    _TRACE_FSIF_(0x18, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_Write                                                                 */
/*                                                                                              */
/* DESCRIPTION: Execute write command                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ulStartSector                   start sector number to write                    */
/*              ulNumOfSector                   writre sector size                              */
/*              pucDataBuf                      data buffer pointer                             */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out error                                  */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_Write( _grp_fsif_unit_info *ptUnitInfo, grp_u32 ulStartSector, grp_u32 ulNumOfSector, grp_u8 *pucDataBuf, grp_vos_t_queue *ptQueue)
{
grp_s32                         lStat;

    _TRACE_FSIF_(0x19, 0x00, 0x00);

    if (_grp_fsif_LockToPollingTask( ptUnitInfo) != GRP_FSIF_OK) {
        /* illegal error */
        return GRP_FSIF_ILLEAGAL_ERROR;
    }

    /* Test Unit Readey command */
    lStat = _grp_fsif_WriteCommand( ptUnitInfo, ulStartSector, ulNumOfSector, pucDataBuf, ptQueue, &ptUnitInfo->tMscCmd);
    switch (lStat) {
    case GRP_FSIF_OK:
        /* NO BREAK */
    case GRP_FSIF_TMOUT:
        break;

    case GRP_FSIF_CHECK_CONDITION:
        /* Request Sense command */
        lStat = _grp_fsif_RequestSenseCommand( ptUnitInfo, ptQueue, &ptUnitInfo->tMscCmd);
        if (lStat == GRP_FSIF_OK) {
            _TRACE_FSIF_(0x19, 0x01, 0x00);
            lStat = _grp_fsif_GetErrorCode( ptUnitInfo);

            if (lStat == GRP_FSIF_EC_NOT_RDY) {
                /* detached media */
                _grp_fsif_Notify( ptUnitInfo, GRP_FSIF_DETACHED_MEDIA);
            }
        }
        else {
            _TRACE_FSIF_(0x19, 0x02, 0x00);
        }
        break;

    default:
        lStat = GRP_FSIF_ILLEAGAL_ERROR;
        break;
    }

    if (_grp_fsif_UnlockToPollingTask( ptUnitInfo) != GRP_FSIF_OK) {
        /* illegal error */
        return GRP_FSIF_ILLEAGAL_ERROR;
    }

    _TRACE_FSIF_(0x19, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_WriteCommand                                                          */
/*                                                                                              */
/* DESCRIPTION: Execute write command                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ulStartSector                   start sector number to write                    */
/*              ulNumOfSector                   writre sector size                              */
/*              pucDataBuf                      data buffer pointer                             */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out error                                  */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_WriteCommand( _grp_fsif_unit_info *ptUnitInfo, grp_u32 ulStartSector, grp_u32 ulNumOfSector, grp_u8 *pucDataBuf, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd)
{
grp_s32                         lStat;
grp_s32                         lTmpStat;
grp_u32                         ulMsg;

    _TRACE_FSIF_(0x1A, 0x00, 0x00);

    /*  clear queue */
    while (grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_VOS_NOWAIT) == GRP_VOS_POS_RESULT);

    /* set parameters */
    ptMscCmd->hMscHdr       = ptUnitInfo->ptMscHdr;
    ptMscCmd->ucLun         = ptUnitInfo->ucOwnLun;
    ptMscCmd->pucReqBuffer  = pucDataBuf;
    ptMscCmd->ulReqLength   = (ulNumOfSector * ptUnitInfo->ulSectorSize);
    ptMscCmd->pfnCallback   = _grp_fsif_RequestCallBack;
    ptMscCmd->pvUserRef     = (void *)ptQueue;

    lStat = grp_msc_WriteSector( ptMscCmd, ulStartSector, ulNumOfSector);
    if (lStat == GRP_MSC_OK) {
        /* wait queue */
        lStat = grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_FSIF_WAIT_TIME);
        if (lStat == GRP_VOS_POS_RESULT) {
            /* check status */
            if (ptMscCmd->lStatus == GRP_MSC_NO_FAIL) {
                /* command success */
                lStat = GRP_FSIF_OK;
            }
            else if (ptMscCmd->lStatus == GRP_MSC_CHECK_CONDITION) {
                /* need Request Sense */
                lStat = GRP_FSIF_CHECK_CONDITION;
            }
            else {
                /* command failed */
                lStat = GRP_FSIF_NG;
            }
        }
        else {
            if (lStat == GRP_VOS_NEG_TIMEOUT) {
                /* time out */
                lStat = GRP_FSIF_TMOUT;
            }
            else {
                /* illeagal error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }

        if (lStat == GRP_FSIF_TMOUT) {
            /* cancel request */
            lTmpStat = grp_msc_Cancel( ptMscCmd);
            if (lTmpStat != GRP_FSIF_OK) {
                /* error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }
    }

    _TRACE_FSIF_(0x1A, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : grp_fsif_GetMediaInfo                                                           */
/*                                                                                              */
/* DESCRIPTION: Get media informations                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pvHdr                           handler pointer                                 */
/*              ptMediaInfo                     structure of the media information              */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_ILLEAGAL_STATE         Illegal status                                  */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_fsif_GetMediaInfo( void *pvHdr, grp_fsif_media_info *ptMediaInfo)
{
_grp_fsif_unit_info             *ptUnitInfo = (_grp_fsif_unit_info*)pvHdr;

    _TRACE_FSIF_(0x1B, 0x00, 0x00);

    /* set parameters */
    if ((ptUnitInfo->ucUnitStatus & GRP_FSIF_US_ATTACHED) == 0) {
        /* Media is no exist */
        ptMediaInfo->ulSectorSize   = 0;
        ptMediaInfo->ulSectorNum    = 0;
        ptMediaInfo->ucPeriDevType  = GRP_FSIF_PDT_NORMAL;
    }
    else {
        ptMediaInfo->ulSectorSize   = ptUnitInfo->ulSectorSize;
        ptMediaInfo->ulSectorNum    = ptUnitInfo->ulSectorNum;
        ptMediaInfo->ucPeriDevType  = ptUnitInfo->ucPeriDevType;
    }
    ptMediaInfo->ucLun          = ptUnitInfo->ucOwnLun;
    ptMediaInfo->usUsbDevId     = ptUnitInfo->ptMscHdr->usUsbDevId;

    _TRACE_FSIF_(0x1B, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_Notification                                                          */
/*                                                                                              */
/* DESCRIPTION: Notification of connect or disconnect the device                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMscNotify                     pointer of ms_c structure                       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_Notification( grp_msc_notify *ptNotify)
{
grp_s32                         lStat;

    _TRACE_FSIF_(0x1C, 0x00, 0x00);

    lStat = _grp_fsif_NoticeListSet( ptNotify);
    if (lStat == GRP_FSIF_OK) {
        /* notification to the main task */
        lStat = grp_vos_SetFlag( l_tFsifVos.ptNoticeFlg, GRP_FSIF_EVENT);
        if (lStat != GRP_VOS_POS_RESULT) {
            /* error */
        }
    }

    _TRACE_FSIF_(0x1C, 0x00, _F_END);

    return GRP_MSC_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_MainTask                                                              */
/*                                                                                              */
/* DESCRIPTION: Connect/Disconnect proceessing task                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulValue                         reserved                                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_fsif_MainTask( grp_u32 ulValue)
{
grp_msc_notify                  tMscNotify;
grp_s32                         lStat;
grp_u32                         ulEventFlag;

    ulValue = GRP_USB_NULL;     /* Warning measures */

    _TRACE_FSIF_(0x1D, 0x00, 0x00);

    for(;;) {
        /* wait event */
        lStat = grp_vos_WaitFlag( l_tFsifVos.ptNoticeFlg, GRP_FSIF_EVENT, &ulEventFlag, GRP_FSIF_TEST_UNIT_WAIT_TIME);
        if (lStat == GRP_VOS_POS_RESULT) {
            /* connect or disconnect event occure */
            lStat = grp_vos_ClearFlag( l_tFsifVos.ptNoticeFlg, GRP_FSIF_EVENT_CLEAR);
            if (lStat != GRP_VOS_POS_RESULT) {
                /* error */
            }

            for (;;) {
                lStat = _grp_fsif_NoticeListGet( &tMscNotify);
                if (lStat != GRP_FSIF_OK) {
                    /* exit this loop */
                    break;
                }

                switch (tMscNotify.iEvent) {
                case GRP_MSC_ATTACHED:
                    _grp_fsif_ConnectProcess( &tMscNotify);
                    break;

                case GRP_MSC_DETACHED:
                    _grp_fsif_DisconnectProcess( &tMscNotify);
                    break;

                default:
                    _TRACE_FSIF_(0x1D, 0x01, _F_END);
                    /* error */
                    break;
                }
            }
        }
        else if (lStat == GRP_VOS_NEG_TIMEOUT) {
              /* if waiting event is finished with TimeOut error, then it is check a media. */
            _grp_fsif_PollingCheck();
        }
    }

    _TRACE_FSIF_(0x1D, 0x00, _F_END);
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_ConnectProcess                                                        */
/*                                                                                              */
/* DESCRIPTION: Connect proceessing                                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMscNotify                     pointer of ms_c structure                       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_ConnectProcess( grp_msc_notify *ptMscNotify)
{
grp_msc_cmd                     *ptMscCmd   = &l_tConMscCmd;
_grp_fsif_unit_info             *ptUnitInfo;
grp_s32                         lStat;
grp_u8                          ucCnt;

    _TRACE_FSIF_(0x1E, 0x00, 0x00);

    /* pipe open */
    ptMscCmd->hMscHdr = ptMscNotify->hMscHdr;

    lStat = grp_msc_Open( ptMscCmd);
    if (lStat != GRP_MSC_OK) {
        /* error */
        _TRACE_FSIF_(0x1E, 0x01, _F_END);
        return GRP_FSIF_NG;
    }

    /* check protocol */
    if (ptMscNotify->ucProtocol == GRP_MSC_BOT_CODE) {
        /* get max LUN */
        lStat = _grp_fsif_GetMaxLun( ptMscNotify, l_tFsifVos.ptMainTskQue, l_pucMaxLun);
        if (lStat != GRP_FSIF_OK) {
            /* pipe close */
            _grp_fsif_MscClose( ptMscCmd);
            /* error */
            _TRACE_FSIF_(0x1E, 0x02, _F_END);
            return GRP_FSIF_NG;
        }
    }
    else {
        /* only unit device */
        *l_pucMaxLun = 0;
    }

    for (ucCnt=0; ucCnt<(*l_pucMaxLun + 1); ucCnt++) {
        /* get management block */
        lStat = _grp_fsif_FreeListGet( &ptUnitInfo);
        if (lStat != GRP_FSIF_OK) {
            /* no free block */
            if (ucCnt == 0) {
                /* pipe close */
                _grp_fsif_MscClose( ptMscCmd);
            }
            _TRACE_FSIF_(0x1E, 0x03, _F_END);
            return GRP_FSIF_NG;
        }

        /* set parameters */
        ptUnitInfo->ptMscHdr        = ptMscNotify->hMscHdr;

        ptUnitInfo->ucUnitStatus    = GRP_FSIF_US_NOMEDIA;
        ptUnitInfo->ucNotifyStat    = GRP_FSIF_NS_DISCONNECT;
        ptUnitInfo->ucRmb           = GRP_FSIF_RMB;
        ptUnitInfo->ucMaxLun        = *l_pucMaxLun;
        ptUnitInfo->ucOwnLun        = ucCnt;
        ptUnitInfo->ucPeriDevType   = GRP_FSIF_PDT_NORMAL;
        ptUnitInfo->ucPriorityFlag  = GRP_FSIF_PF_NO_USE;
        ptUnitInfo->ulSectorSize    = 0;
        ptUnitInfo->ulSectorNum     = 0;

        lStat = _grp_fsif_LockToAppTask( ptUnitInfo);                                   /* {    */
        if (lStat == GRP_FSIF_OK) {
            /* media check sequence */
            lStat = _grp_fsif_MediaCheck( ptUnitInfo, l_tFsifVos.ptMainTskQue, ptMscCmd);
            if (lStat == GRP_FSIF_OK) {
                /* attached media */
                _grp_fsif_Notify( ptUnitInfo, GRP_FSIF_ATTACHED_MEDIA);
            }
            if (_grp_fsif_UnlockToAppTask( ptUnitInfo) != GRP_FSIF_OK) {                /* }    */
                        /* illegal error */
            }
        }

        /* set Access list */
        lStat = _grp_fsif_AccessListSet( ptUnitInfo);
        if (lStat != GRP_FSIF_OK) {
            return GRP_FSIF_NG;
        }
    }

    _TRACE_FSIF_(0x1E, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_DisconnectProcess                                                     */
/*                                                                                              */
/* DESCRIPTION: Disconnect proceessing                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMscNotify                     pointer of ms_c structure                       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_DisconnectProcess( grp_msc_notify *ptMscNotify)
{
grp_msc_cmd                     *ptMscCmd   = &l_tDisMscCmd;
_grp_fsif_unit_info             *ptUnitInfo;
grp_si                          i;
grp_s32                         lStat       = GRP_FSIF_OK;

    _TRACE_FSIF_(0x1F, 0x00, 0x00);

    for (i=0; i<GRP_FSIF_MAX_UINT; i++) {
        lStat = _grp_fsif_AccessListGet( ptMscNotify->hMscHdr, &ptUnitInfo);
        if (lStat != GRP_FSIF_OK) {
            /* no information */
            break;
        }
        else {
            if (ptUnitInfo->ucUnitStatus  & GRP_FSIF_US_ATTACHED) {
                /* detached media */
                _grp_fsif_Notify( ptUnitInfo, GRP_FSIF_DETACHED_MEDIA);
            }

            ptUnitInfo->ucUnitStatus = GRP_FSIF_US_NOMEDIA;

            if (ptUnitInfo->ucOwnLun == 0) {
                /* pipe close */
                ptMscCmd->hMscHdr = ptUnitInfo->ptMscHdr;
                _grp_fsif_MscClose( ptMscCmd);
            }

            _grp_fsif_FreeListRelease( ptUnitInfo);

        }
    }

    _TRACE_FSIF_(0x1F, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_MscClose                                                              */
/*                                                                                              */
/* DESCRIPTION: Close request to the MSC driver. If return value mean the bussy, then close     */
/*              requests are retyried.                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMscCmd                        MSC command handler                             */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_MscClose( grp_msc_cmd *ptMscCmd)
{
grp_si                          i;
grp_s32                         lStat = GRP_MSC_OK;

    _TRACE_FSIF_(0x20, 0x00, 0x00);

    for (i=0; i<GRP_FSIF_CLS_RETRY_CNT; i++) {
        /* close request */
        lStat = grp_msc_Close( ptMscCmd);
        if (lStat != GRP_MSC_BUSY) {
            /* exit this loop */
            break;
        }
        /* wait */
        grp_vos_DelayTask( GRP_FSIF_CLOSE_WAIT);
    }

    _TRACE_FSIF_(0x20, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_GetMaxLun                                                             */
/*                                                                                              */
/* DESCRIPTION: Get Max LUN                                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMscNotify                     pointer of ms_c structure                       */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : pucMaxLun                       Max LUN                                         */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_GetMaxLun( grp_msc_notify *ptMscNotify, grp_vos_t_queue *ptQueue, grp_u8 *pucMaxLun)
{
grp_msc_cmd                     *ptMscCmd   = &l_tLunMscCmd;
grp_s32                         lStat;

    _TRACE_FSIF_(0x21, 0x00, 0x00);

    /* set parameters */
    ptMscCmd->hMscHdr         = ptMscNotify->hMscHdr;
    ptMscCmd->pucReqBuffer    = pucMaxLun;
    ptMscCmd->pfnCallback     = _grp_fsif_RequestCallBack;
    ptMscCmd->pvUserRef       = ptQueue;

    /* Get Max LUN request */
    lStat = _grp_fsif_GetMaxLunRequest( ptMscCmd, ptQueue);

    switch (lStat) {
    case GRP_FSIF_OK:
        /* success */
        break;

    case GRP_FSIF_NG:
        /* retry */
        lStat = _grp_fsif_GetMaxLunRequest( ptMscCmd, ptQueue);
        break;

    case GRP_FSIF_TMOUT:
        /* NO BREAK */
    case GRP_FSIF_ILLEAGAL_ERROR:
        /* NO BREAK */
    default:
        break;
    }

    _TRACE_FSIF_(0x21, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_GetMaxLunRequest                                                      */
/*                                                                                              */
/* DESCRIPTION: Execute Get Max LUN request                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMscCmd                        MSC command handler                             */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out                                        */
/*              other                                                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_GetMaxLunRequest( grp_msc_cmd *ptMscCmd, grp_vos_t_queue *ptQueue)
{
grp_s32                         lStat;
grp_s32                         lTmpStat;
grp_u32                         ulMsg;

    _TRACE_FSIF_(0x22, 0x00, 0x00);

    /*  clear queue */
    while (grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_VOS_NOWAIT) == GRP_VOS_POS_RESULT);

    /* request */
    lStat = grp_msc_GetMaxLun( ptMscCmd);
    if (lStat == GRP_MSC_OK) {
        /* wait queue */
        lStat = grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_FSIF_WAIT_TIME);
        if (lStat == GRP_VOS_POS_RESULT) {
            /* check status */
            if (ptMscCmd->lStatus == GRP_MSC_NO_FAIL) {
                /* command success */
                lStat = GRP_FSIF_OK;
            }
            else {
                /* command failed */
                lStat = GRP_FSIF_NG;
            }
        }
        else {
            if (lStat == GRP_VOS_NEG_TIMEOUT) {
                /* time out */
                lStat = GRP_FSIF_TMOUT;
            }
            else {
                /* illeagal error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }

        if (lStat == GRP_FSIF_TMOUT) {
            /* cancel */
            lTmpStat = grp_msc_GetMaxLunCancel( ptMscCmd);
            if (lTmpStat != GRP_FSIF_OK) {
                /* error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }
    }

    _TRACE_FSIF_(0x22, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_RequestCallBack                                                       */
/*                                                                                              */
/* DESCRIPTION: Call back function                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMscCmd                        MSC command handler                             */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success only                                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_RequestCallBack( grp_msc_cmd *ptMscCmd)
{
grp_vos_t_queue                 *ptQueue    = (grp_vos_t_queue *)ptMscCmd->pvUserRef;
grp_s32                         lStat;
grp_u32                         ulMsg       = GRP_FSIF_RCV;

    _TRACE_FSIF_(0x23, 0x00, 0x00);

    lStat = grp_vos_SendQueue( ptQueue, &ulMsg, GRP_VOS_NOWAIT);
    if (lStat != GRP_VOS_POS_RESULT) {
        /* error */
    }

    _TRACE_FSIF_(0x23, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_MediaCheck                                                            */
/*                                                                                              */
/* DESCRIPTION: Media check proccess                                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_MediaCheck( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd)
{
grp_s32                         lStat;
grp_si                          i;

    _TRACE_FSIF_(0x24, 0x00, 0x00);

    /* temporary setting */
    ptUnitInfo->ucUnitStatus = GRP_FSIF_US_NOMEDIA;

    for (i=0; i<GRP_FSIF_MC_RETRY_CNT; i++) {
        /* Inquiry */
        lStat = _grp_fsif_Inquiry( ptUnitInfo, ptQueue, ptMscCmd);
        if (lStat != GRP_FSIF_OK) {
            /* error */
            _TRACE_FSIF_(0x24, 0x01, _F_END);
            continue;
        }

        /* Read Capacity */
        lStat = _grp_fsif_ReadCapacity( ptUnitInfo, ptQueue, ptMscCmd);
        if (lStat != GRP_FSIF_OK) {
            /* error */
            _TRACE_FSIF_(0x24, 0x02, _F_END);
            continue;
        }

        /* Mode Sense */
        lStat = _grp_fsif_ModeSense( ptUnitInfo, ptQueue, ptMscCmd);
        if ((lStat != GRP_FSIF_OK)
         && (lStat != GRP_FSIF_EC_ILLEGL_REQ)) {
            /* error */
            _TRACE_FSIF_(0x24, 0x03, _F_END);
            continue;
        }

        /* Test Unit Ready */
        lStat = _grp_fsif_TestUnitReady( ptUnitInfo, ptQueue, ptMscCmd);
        if (lStat != GRP_FSIF_OK) {
            /* error */
            _TRACE_FSIF_(0x24, 0x04, _F_END);
            continue;
        }

        /* connected media */
        ptUnitInfo->ucUnitStatus |= GRP_FSIF_US_ATTACHED;

        /* success */
        lStat = GRP_FSIF_OK;
        break;
    }

    _TRACE_FSIF_(0x24, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_Inquiry                                                               */
/*                                                                                              */
/* DESCRIPTION: Execute inquiry command                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out                                        */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_Inquiry( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd)
{
grp_s32                         lStat;

    _TRACE_FSIF_(0x25, 0x00, 0x00);

    /* Inquiry command */
    lStat = _grp_fsif_InquiryCommand( ptUnitInfo, ptQueue, ptMscCmd);
    switch (lStat) {
    case GRP_FSIF_OK:
        /* NO BREAK */
    case GRP_FSIF_TMOUT:
        break;

    case GRP_FSIF_CHECK_CONDITION:
        /* Request Sense command */
        lStat = _grp_fsif_RequestSenseCommand( ptUnitInfo, ptQueue, ptMscCmd);
        if (lStat == GRP_FSIF_OK) {
            /* check error status */
            lStat = _grp_fsif_GetErrorCode( ptUnitInfo);
        }
        break;

    default:
        lStat = GRP_FSIF_ILLEAGAL_ERROR;
        break;
    }

    if (lStat == GRP_FSIF_OK) {
#ifndef GRP_FSIF_LOW_MEMORY
        /* save inquiry data */
        grp_std_memcpy( ptUnitInfo->aucInquiryData, ptUnitInfo->pucCmemArea, GRP_FSIF_INQUIRY_DATA_SZ);
#endif  /* GRP_FSIF_LOW_MEMORY */

        /* check Multi-Media logical unit */
        if ((*ptUnitInfo->pucCmemArea & GRP_FSIF_MM_FLAG) == GRP_FSIF_MM_FLAG) {
            /* set informations */
            ptUnitInfo->ucPeriDevType           = GRP_FSIF_PDT_MUILT;
            ptUnitInfo->ptMscHdr->ucSubClass    = GRP_MSC_ATAPI_CODE;
        }
        /* check removable medium bit */
        if ((*(ptUnitInfo->pucCmemArea + 1) & GRP_FSIF_INQ_RM_BIT) != GRP_FSIF_INQ_RM_BIT) {
            /* media is not removable */
            ptUnitInfo->ucRmb = GRP_FSIF_NOT_RMB;
        }
    }

    _TRACE_FSIF_(0x25, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_InquiryCommand                                                        */
/*                                                                                              */
/* DESCRIPTION: Execute inquiry command                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out                                        */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_InquiryCommand( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd)
{
grp_s32                         lStat;
grp_s32                         lTmpStat;
grp_u32                         ulMsg;

    _TRACE_FSIF_(0x26, 0x00, 0x00);

    /*  clear queue */
    while (grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_VOS_NOWAIT) == GRP_VOS_POS_RESULT);

    /* set parameters */
    ptMscCmd->hMscHdr       = ptUnitInfo->ptMscHdr;
    ptMscCmd->ucLun         = ptUnitInfo->ucOwnLun;
    ptMscCmd->pucReqBuffer  = ptUnitInfo->pucCmemArea;
    ptMscCmd->ulReqLength   = GRP_FSIF_INQUIRY_DATA_SZ;
    ptMscCmd->pfnCallback   = _grp_fsif_RequestCallBack;
    ptMscCmd->pvUserRef     = (void *)ptQueue;

    lStat = grp_msc_Inquiry( ptMscCmd);
    if (lStat == GRP_MSC_OK) {
        /* wait queue */
        lStat = grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_FSIF_WAIT_TIME);
        if (lStat == GRP_VOS_POS_RESULT) {
            /* check status */
            if (ptMscCmd->lStatus == GRP_MSC_NO_FAIL) {
                /* command success */
                lStat = GRP_FSIF_OK;
            }
            else if (ptMscCmd->lStatus == GRP_MSC_CHECK_CONDITION) {
                /* need Request Sense */
                lStat = GRP_FSIF_CHECK_CONDITION;
            }
            else {
                /* command failed */
                lStat = GRP_FSIF_NG;
            }
        }
        else {
            if (lStat == GRP_VOS_NEG_TIMEOUT) {
                /* time out */
                lStat = GRP_FSIF_TMOUT;
            }
            else {
                /* illeagal error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }

        if (lStat == GRP_FSIF_TMOUT) {
            /* cancel */
            lTmpStat = grp_msc_Cancel( ptMscCmd);
            if (lTmpStat != GRP_FSIF_OK) {
                /* error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }
    }

    _TRACE_FSIF_(0x26, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_ReadCapacity                                                          */
/*                                                                                              */
/* DESCRIPTION: Execute read capacity command                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out                                        */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_ReadCapacity( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd)
{
grp_s32                         lStat;

    _TRACE_FSIF_(0x27, 0x00, 0x00);

    /* Read Capacity command */
    lStat = _grp_fsif_ReadCapacityCommand( ptUnitInfo, ptQueue, ptMscCmd);
    switch (lStat) {
    case GRP_FSIF_OK:
        /* NO BREAK */
    case GRP_FSIF_TMOUT:
        break;

    case GRP_FSIF_CHECK_CONDITION:
        /* Request Sense command */
        lStat = _grp_fsif_RequestSenseCommand( ptUnitInfo, ptQueue, ptMscCmd);
        if (lStat == GRP_FSIF_OK) {
            /* check error status */
            lStat = _grp_fsif_GetErrorCode( ptUnitInfo);
        }
        break;

    default:
        lStat = GRP_FSIF_ILLEAGAL_ERROR;
        break;
    }

    /* set sector size and total number of sectors */
    if (lStat == GRP_FSIF_OK) {
        ptUnitInfo->ulSectorNum     = _GRP_FSIF_UINT32(ptUnitInfo->pucCmemArea) + 1;
        ptUnitInfo->ulSectorSize    = _GRP_FSIF_UINT32(ptUnitInfo->pucCmemArea + 4);
    }
    else {
        ptUnitInfo->ulSectorNum     = 0;
        ptUnitInfo->ulSectorSize    = 0;
    }

    _TRACE_FSIF_(0x27, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_ReadCapacityCommand                                                   */
/*                                                                                              */
/* DESCRIPTION: Execute read capacity command                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out                                        */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_ReadCapacityCommand( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd)
{
grp_s32                         lStat;
grp_s32                         lTmpStat;
grp_u32                         ulMsg;

    _TRACE_FSIF_(0x28, 0x00, 0x00);

    /*  clear queue */
    while (grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_VOS_NOWAIT) == GRP_VOS_POS_RESULT);

    /* set parameters */
    ptMscCmd->hMscHdr       = ptUnitInfo->ptMscHdr;
    ptMscCmd->ucLun         = ptUnitInfo->ucOwnLun;
    ptMscCmd->pucReqBuffer  = ptUnitInfo->pucCmemArea;
    ptMscCmd->ulReqLength   = GRP_FSIF_READ_CAPACITY_DATA_SZ;
    ptMscCmd->pfnCallback   = _grp_fsif_RequestCallBack;
    ptMscCmd->pvUserRef     = (void *)ptQueue;

    lStat = grp_msc_ReadCapacity( ptMscCmd);
    if (lStat == GRP_MSC_OK) {
        /* wait queue */
        lStat = grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_FSIF_WAIT_TIME);
        if (lStat == GRP_VOS_POS_RESULT) {
            /* check status */
            if (ptMscCmd->lStatus == GRP_MSC_NO_FAIL) {
                /* command success */
                lStat = GRP_FSIF_OK;
            }
            else if (ptMscCmd->lStatus == GRP_MSC_CHECK_CONDITION) {
                /* need Request Sense */
                lStat = GRP_FSIF_CHECK_CONDITION;
            }
            else {
                /* command failed */
                lStat = GRP_FSIF_NG;
            }
        }
        else {
            if (lStat == GRP_VOS_NEG_TIMEOUT) {
                /* time out */
                lStat = GRP_FSIF_TMOUT;
            }
            else {
                /* illeagal error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }

        if (lStat == GRP_FSIF_TMOUT) {
            /* cancel */
            lTmpStat = grp_msc_Cancel( ptMscCmd);
            if (lTmpStat != GRP_FSIF_OK) {
                /* error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }
    }

    _TRACE_FSIF_(0x28, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_ModeSense                                                             */
/*                                                                                              */
/* DESCRIPTION: Execute mode sense command                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out                                        */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_ModeSense( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd)
{
grp_s32                         lStat;

    _TRACE_FSIF_(0x29, 0x00, 0x00);

    /* Mode Sense command */
    lStat = _grp_fsif_ModeSenseCommand( ptUnitInfo, GRP_FSIF_MS_IEC_PC, ptQueue, ptMscCmd);
    switch (lStat) {
    case GRP_FSIF_OK:
        /* NO BREAK */
    case GRP_FSIF_TMOUT:
        break;

    case GRP_FSIF_CHECK_CONDITION:
        /* Request Sense command */
        lStat = _grp_fsif_RequestSenseCommand( ptUnitInfo, ptQueue, ptMscCmd);
        if (lStat == GRP_FSIF_OK) {
            /* check error status */
            lStat = _grp_fsif_GetErrorCode( ptUnitInfo);
        }
        break;

    case GRP_FSIF_NO_CHECK_CONDITION:
        lStat = GRP_FSIF_EC_ILLEGL_REQ;
        break;

    default:
        lStat = GRP_FSIF_ILLEAGAL_ERROR;
        break;
    }

    /* check write protect */
    if ((lStat == GRP_FSIF_OK)
     && (ptUnitInfo->ucPeriDevType == GRP_FSIF_PDT_NORMAL)) {
        /* check */
        if (*(ptUnitInfo->pucCmemArea + 0) == 3) {
            if (*(ptUnitInfo->pucCmemArea + 2) &  GRP_FSIF_MS_WP_BIT) {
                ptUnitInfo->ucUnitStatus |= GRP_FSIF_US_WR_PROTECT;
            }
        }
        else if (*(ptUnitInfo->pucCmemArea + 0) == 6) {
            if (*(ptUnitInfo->pucCmemArea + 3) &  GRP_FSIF_MS_WP_BIT) {
                ptUnitInfo->ucUnitStatus |= GRP_FSIF_US_WR_PROTECT;
            }
        }
    }

    _TRACE_FSIF_(0x29, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_ModeSenseCommand                                                      */
/*                                                                                              */
/* DESCRIPTION: Execute mode sense command                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ucPegeCode                      page code                                       */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out                                        */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_ModeSenseCommand( _grp_fsif_unit_info *ptUnitInfo, grp_u8 ucPegeCode, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd)
{
grp_s32                         lStat;
grp_s32                         lTmpStat;
grp_u32                         ulMsg;

    _TRACE_FSIF_(0x2A, 0x00, 0x00);

    /*  clear queue */
    while (grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_VOS_NOWAIT) == GRP_VOS_POS_RESULT);

    /* set parameters */
    ptMscCmd->hMscHdr       = ptUnitInfo->ptMscHdr;
    ptMscCmd->ucLun         = ptUnitInfo->ucOwnLun;
    ptMscCmd->pucReqBuffer  = ptUnitInfo->pucCmemArea;
    ptMscCmd->ulReqLength   = GRP_FSIF_MODE_SENSE_DATA_SZ;
    ptMscCmd->pfnCallback   = _grp_fsif_RequestCallBack;
    ptMscCmd->pvUserRef     = (void *)ptQueue;

    lStat = grp_msc_ModeSense( ptMscCmd, ucPegeCode);
    if (lStat == GRP_MSC_OK) {
        /* wait queue */
        lStat = grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_FSIF_WAIT_TIME);
        if (lStat == GRP_VOS_POS_RESULT) {
            /* check status */
            if (ptMscCmd->lStatus == GRP_MSC_NO_FAIL) {
                /* command success */
                lStat = GRP_FSIF_OK;
            }
            else if (ptMscCmd->lStatus == GRP_MSC_CHECK_CONDITION) {
                /* need Request Sense */
                lStat = GRP_FSIF_CHECK_CONDITION;
            }
            else if (ptMscCmd->lStatus == GRP_MSC_STALL) {
                /* command pass */
                lStat = GRP_FSIF_NO_CHECK_CONDITION;
            }
            else {
                /* command failed */
                lStat = GRP_FSIF_NG;
            }
        }
        else {
            if (lStat == GRP_VOS_NEG_TIMEOUT) {
                /* time out */
                lStat = GRP_FSIF_TMOUT;
            }
            else {
                /* illeagal error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }

        if (lStat == GRP_FSIF_TMOUT) {
            /* cancel */
            lTmpStat = grp_msc_Cancel( ptMscCmd);
            if (lTmpStat != GRP_FSIF_OK) {
                /* error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }
    }

    _TRACE_FSIF_(0x2A, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_TestUnitReady                                                         */
/*                                                                                              */
/* DESCRIPTION: Execute test unit ready command                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out                                        */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_TestUnitReady( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd)
{
grp_s32                         lStat;

    _TRACE_FSIF_(0x2B, 0x00, 0x00);

    /* Test Unit Ready command */
    lStat = _grp_fsif_TestUnitReadyCommand( ptUnitInfo, ptQueue, ptMscCmd);
    switch (lStat) {
    case GRP_FSIF_OK:
        /* NO BREAK */
    case GRP_FSIF_TMOUT:
        break;

    case GRP_FSIF_CHECK_CONDITION:
        /* Request Sense command */
        lStat = _grp_fsif_RequestSenseCommand( ptUnitInfo, ptQueue, ptMscCmd);
        if (lStat == GRP_FSIF_OK) {
            /* check error status */
            lStat = _grp_fsif_GetErrorCode( ptUnitInfo);
        }
        break;

    default:
        lStat = GRP_FSIF_ILLEAGAL_ERROR;
        break;
    }

    _TRACE_FSIF_(0x2B, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_TestUnitReadyCommand                                                  */
/*                                                                                              */
/* DESCRIPTION: Execute test unit ready command                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out                                        */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_TestUnitReadyCommand( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd)
{
grp_s32                         lStat;
grp_s32                         lTmpStat;
grp_u32                         ulMsg;

    _TRACE_FSIF_(0x2C, 0x00, 0x00);

    /*  clear queue */
    while (grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_VOS_NOWAIT) == GRP_VOS_POS_RESULT);

    /* set parameters */
    ptMscCmd->hMscHdr       = ptUnitInfo->ptMscHdr;
    ptMscCmd->ucLun         = ptUnitInfo->ucOwnLun;
    ptMscCmd->pucReqBuffer  = GRP_USB_NULL;
    ptMscCmd->ulReqLength   = 0;
    ptMscCmd->pfnCallback   = _grp_fsif_RequestCallBack;
    ptMscCmd->pvUserRef     = (void *)ptQueue;

    lStat = grp_msc_TestUnitReady( ptMscCmd);
    if (lStat == GRP_MSC_OK) {
        /* wait queue */
        lStat = grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_FSIF_WAIT_TIME);
        if (lStat == GRP_VOS_POS_RESULT) {
            /* check status */
            if (ptMscCmd->lStatus == GRP_MSC_NO_FAIL) {
                /* command success */
                lStat = GRP_FSIF_OK;
            }
            else if (ptMscCmd->lStatus == GRP_MSC_CHECK_CONDITION) {
                /* need Request Sense */
                lStat = GRP_FSIF_CHECK_CONDITION;
            }
            else {
                /* command failed */
                lStat = GRP_FSIF_NG;
            }
        }
        else {
            if (lStat == GRP_VOS_NEG_TIMEOUT) {
                /* time out */
                lStat = GRP_FSIF_TMOUT;
            }
            else {
                /* illeagal error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }

        if (lStat == GRP_FSIF_TMOUT) {
            /* cancel */
            lTmpStat = grp_msc_Cancel( ptMscCmd);
            if (lTmpStat != GRP_FSIF_OK) {
                /* error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }
    }

    _TRACE_FSIF_(0x2C, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_RequestSenseCommand                                                   */
/*                                                                                              */
/* DESCRIPTION: Execute request sense command                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/*              ptQueue                         queue handler                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_TMOUT                  Time out                                        */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_RequestSenseCommand( _grp_fsif_unit_info *ptUnitInfo, grp_vos_t_queue *ptQueue, grp_msc_cmd *ptMscCmd)
{
grp_s32                         lStat;
grp_s32                         lTmpStat;
grp_u32                         ulMsg;

    _TRACE_FSIF_(0x2D, 0x00, 0x00);

    /*  clear queue */
    while (grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_VOS_NOWAIT) == GRP_VOS_POS_RESULT);

    /* set parameters */
    ptMscCmd->hMscHdr       = ptUnitInfo->ptMscHdr;
    ptMscCmd->ucLun         = ptUnitInfo->ucOwnLun;
    ptMscCmd->pucReqBuffer  = ptUnitInfo->pucCmemArea;
    ptMscCmd->ulReqLength   = GRP_FSIF_REQUEST_SENSE_DATA_SZ;
    ptMscCmd->pfnCallback   = _grp_fsif_RequestCallBack;
    ptMscCmd->pvUserRef     = (void *)ptQueue;

    lStat = grp_msc_RequestSense( ptMscCmd);
    if (lStat == GRP_MSC_OK) {
        /* wait queue */
        lStat = grp_vos_ReceiveQueue( ptQueue, (void *)&ulMsg, GRP_FSIF_WAIT_TIME);
        if (lStat == GRP_VOS_POS_RESULT) {
            /* check status */
            if (ptMscCmd->lStatus == GRP_MSC_NO_FAIL) {
                /* command success */
                lStat = GRP_FSIF_OK;
            }
            else {
                /* command failed */
                lStat = GRP_FSIF_NG;
            }
        }
        else {
            if (lStat == GRP_VOS_NEG_TIMEOUT) {
                /* time out */
                lStat = GRP_FSIF_TMOUT;
            }
            else {
                /* illeagal error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }

        if (lStat == GRP_FSIF_TMOUT) {
            /* cancel */
            lTmpStat = grp_msc_Cancel( ptMscCmd);
            if (lTmpStat != GRP_FSIF_OK) {
                /* error */
                lStat = GRP_FSIF_ILLEAGAL_ERROR;
            }
        }
    }
    else {
        /* set unit status */
        ptUnitInfo->ucUnitStatus |= GRP_FSIF_US_UNKNOWN;
    }

    _TRACE_FSIF_(0x2D, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_fsif_GetErrorCode                                                          */
/*                                                                                              */
/* DESCRIPTION: Get error code from request sense data                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptUnitInfo                      pointer of unit information block               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_EC_NO_ERR              Success                                         */
/*              GRP_FSIF_EC_NOT_RDY             Media not ready                                 */
/*              GRP_FSIF_EC_HW_ERR              Non recoverable error                           */
/*              GRP_FSIF_EC_ILLEGL_REQ          Illegal request                                 */
/*              GRP_FSIF_EC_OTHER_ERR           Other errors                                    */
/*              GRP_FSIF_EC_WR_PROTECT          Write protect                                   */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_fsif_GetErrorCode( _grp_fsif_unit_info *ptUnitInfo)
{
grp_s32                         lStat       = GRP_FSIF_EC_NO_ERR;
grp_u8*                         pucUnitStat = &ptUnitInfo->ucUnitStatus;
grp_u8                          ucSenseKey  = *(ptUnitInfo->pucCmemArea + 2);
grp_u8                          ucAsc       = *(ptUnitInfo->pucCmemArea + 12);  /* for debug    */
grp_u8                          ucAscq      = *(ptUnitInfo->pucCmemArea + 13);  /* for debug    */

    _TRACE_FSIF_(0x2E, 0x00, 0x00);

    /* for debug */
    _TRACE_FSIF_(0x2E, ucAsc, ucAscq);

    switch (ucSenseKey) {
    /*-----------------------------------------------------------------------------------------*/
    case GRP_FSIF_NO_SENSE:
        /* NO BREAK */
    case GRP_FSIF_RECOVERED_ERROR:
        /* Success */
        lStat            = GRP_FSIF_EC_NO_ERR;
        _TRACE_FSIF_(0x2E, 0x01, 0x00);
        break;
    /*-----------------------------------------------------------------------------------------*/
    case GRP_FSIF_UNIT_ATTANTION:
        lStat           = GRP_FSIF_EC_NOT_RDY;
        _TRACE_FSIF_(0x2E, 0x02, 0x00);
        break;
    /*-----------------------------------------------------------------------------------------*/
    case GRP_FSIF_NOT_READY:
        *pucUnitStat    = GRP_FSIF_US_NOMEDIA;
        lStat           = GRP_FSIF_EC_NOT_RDY;
        _TRACE_FSIF_(0x2E, 0x03, 0x00);
        break;
    /*-----------------------------------------------------------------------------------------*/
    case GRP_FSIF_MEDIUM_ERROR:
        /* NO BREAK */
    case GRP_FSIF_HARDWARE_ERROR:
        /* non-recoverable error */
        *pucUnitStat    |= GRP_FSIF_US_UNKNOWN;
        lStat            = GRP_FSIF_EC_HW_ERR;
        _TRACE_FSIF_(0x2E, 0x04, 0x00);
        break;
    /*-----------------------------------------------------------------------------------------*/
    case GRP_FSIF_ILLEGAL_REQUEST:
        lStat            = GRP_FSIF_EC_ILLEGL_REQ;
        _TRACE_FSIF_(0x2E, 0x05, 0x00);
        break;
    /*-----------------------------------------------------------------------------------------*/
    case GRP_FSIF_DATA_PROTECT:
        *pucUnitStat    |= GRP_FSIF_US_WR_PROTECT;
        lStat            = GRP_FSIF_EC_WR_PROTECT;
        _TRACE_FSIF_(0x2E, 0x06, 0x00);
        break;
    /*-----------------------------------------------------------------------------------------*/
    default:
        /* illegal error */
        *pucUnitStat    |= GRP_FSIF_US_UNKNOWN;
        lStat            = GRP_FSIF_EC_OTHER_ERR;
        _TRACE_FSIF_(0x2E, 0x07, 0x00);
        break;
    }

    _TRACE_FSIF_(0x2E, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : grp_fsif_GetNonCacheBuffer                                                      */
/*                                                                                              */
/* DESCRIPTION: Get non-cache buffer to use getting the boot sector.                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ppucBuf                         get buffer area                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_fsif_GetNonCacheBuffer( grp_u8 **ppucBuf)
{
grp_s32                         lStat = GRP_FSIF_OK;

    _TRACE_FSIF_(0x2F, 0x00, 0x00);

    /* set initialized value */
    *ppucBuf = GRP_USB_NULL;

    /* lock */
    if (GRP_FSIF_BUF_LOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        return GRP_FSIF_NG;
    }

    /* get buffer */
    lStat = grp_cmem_BlkGet( GRP_CMEM_ID_FSIF_NC_BUF, (void **)ppucBuf);
    if (lStat != GRP_CMEM_OK) {
        /* unlock */                                                                    /* }    */
        if (GRP_FSIF_BUF_UNLOCK() != GRP_FSIF_LOCK_OK) {
            /* error */
            return GRP_FSIF_NG;
        }
        /* error */
        return GRP_FSIF_CMEM_ERROR;
    }

    _TRACE_FSIF_(0x2F, 0x00, _F_END);

    return GRP_FSIF_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_fsif_RelNonCacheBuffer                                                      */
/*                                                                                              */
/* DESCRIPTION: Release non-cache buffer.                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ppucBuf                         get buffer area                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_fsif_RelNonCacheBuffer( grp_u8 *pucBuf)
{
grp_s32                         lStat = GRP_FSIF_OK;

    _TRACE_FSIF_(0x30, 0x00, 0x00);

    /* release buffer */
    lStat = grp_cmem_BlkRel( pucBuf);
    if (lStat != GRP_CMEM_OK) {
        /* error */
        lStat = GRP_FSIF_CMEM_ERROR;
    }

    /* unlock */                                                                        /* }    */
    if (GRP_FSIF_BUF_UNLOCK() != GRP_FSIF_LOCK_OK) {
        /* error */
        lStat = GRP_FSIF_NG;
    }

    _TRACE_FSIF_(0x30, 0x00, _F_END);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : grp_fsif_Reset                                                                  */
/*                                                                                              */
/* DESCRIPTION: Reset to the media or USB device                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pvHdr                           handler pointer                                 */
/*              ulMode                          Reset mode                                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_FSIF_OK                     Success                                         */
/*              GRP_FSIF_ILLEAGAL_STATE         Illegal status                                  */
/*              others                          Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_fsif_Reset( void *pvHdr, grp_u32 ulMode)
{
_grp_fsif_access_queue          *ptQueueInfo;
_grp_fsif_unit_info             *ptUnitInfo = (_grp_fsif_unit_info*)pvHdr;
grp_msc_cmd                     *ptMscCmd   = &ptUnitInfo->tMscCmd;
grp_s32                         lStat       = GRP_FSIF_OK;
grp_u32                         ulMsg;

    _TRACE_FSIF_(0x31, 0x00, 0x00);

    /* get queue information */
    if (_grp_fsif_AccessQueGet( &ptQueueInfo) != GRP_FSIF_OK) {
        return GRP_FSIF_QUEINFO_ERROR;
    }

    if ((ptUnitInfo->ucUnitStatus & GRP_FSIF_US_ATTACHED) == 0) {
        /* error */
        _TRACE_FSIF_(0x31, 0x01, _F_END);
        return GRP_FSIF_ILLEAGAL_STATE;
    }

    switch (ulMode) {
    case GRP_FSIF_MSC_RESET:
        /* set parameters */
        ptMscCmd->hMscHdr     = ptUnitInfo->ptMscHdr;
        ptMscCmd->ucLun       = ptUnitInfo->ucOwnLun;
        ptMscCmd->pfnCallback = _grp_fsif_RequestCallBack;
        ptMscCmd->pvUserRef   = (void *)ptQueueInfo->ptAccessQue;

        lStat = grp_msc_Reset( ptMscCmd, GRP_MSC_RESET_MASS);

        if (lStat == GRP_MSC_OK) {
            /* wait queue */
            lStat = grp_vos_ReceiveQueue( ptQueueInfo->ptAccessQue, (void *)&ulMsg, GRP_FSIF_WAIT_TIME);
            if (lStat == GRP_VOS_POS_RESULT) {
                /* check status */
                if (ptMscCmd->lStatus == GRP_MSC_NO_FAIL) {
                    /* command success */
                    lStat = GRP_FSIF_OK;
                }
                else {
                    /* command failed */
                    lStat = GRP_FSIF_NG;
                }
            }
            else {
                if (lStat == GRP_VOS_NEG_TIMEOUT) {
                    /* time out */
                    lStat = GRP_FSIF_TMOUT;
                }
                else {
                    /* illeagal error */
                    lStat = GRP_FSIF_ILLEAGAL_ERROR;
                }
            }
        }
        break;

    case GRP_FSIF_RE_ENUMERATION:
        ptMscCmd->hMscHdr = ptUnitInfo->ptMscHdr;
        lStat = grp_msc_Reset( ptMscCmd, GRP_MSC_RESET_ENUM);
        break;

    default:
        lStat = GRP_FSIF_PARAM_ERROR;
        break;
    }

    _TRACE_FSIF_(0x31, 0x00, _F_END);

    /* release queue information */
    if (_grp_fsif_AccessQueRelease( ptQueueInfo) != GRP_FSIF_OK) {
        return GRP_FSIF_QUEINFO_ERROR;
    }

    return lStat;
}

