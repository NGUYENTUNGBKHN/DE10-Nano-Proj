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
/*      test_apl.c                                                              0.90            */
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

/**** INCLUDE FILES *****************************************************************************/

#include "grp_vos.h"
#include "grp_std_tools.h"
#include "grp_usbc.h"
#include "grp_bus.h"

#include "fat.h"
#include "grp_mem_vl_pool.h"
#include "grp_fat_format.h"
#include "grp_stdio.h"
#include "grp_fsif.h"

#include "kernel_inc.h"
#include "itron.h"
#include "js_oswapi.h"
#include "memorymap.h"
#include "common.h"
#include "host_apl.h"
#include "hal_wdt.h"
#include "sub_functions.h"

#include "grp_usbc_reinit.h" /* reinit */
#include "grp_usb_mdchg.h" /* mode change */

/* expression has no effect ... */
#pragma diag_suppress=174-D /* expression has no effect */


/**** INTERNAL STRUCTURE DEFINES ****************************************************************/
grp_u8 *                                g_pucDownloadWrBuf = (grp_u8 *)WRITE_BUF_ADDR;
grp_u8 *                                g_pucDownloadRdBuf = (grp_u8 *)READ_BUF_ADDR;
/**** INTERNAL STATIC VARIABLE DEFINES **********************************************************/
#if 0
/* OS resource information */
DLOCAL grp_vos_t_task*          l_ptPCDTask;       /* Task information block pointer           */

DLOCAL grp_vos_t_queue*         l_ptConnectQue;    /* Notice connect/disconecct status Queue   */
DLOCAL grp_vos_t_queue*         l_ptCompleteQue;   /* Notice processing completion Queue       */

/* Buffers */
DLOCAL grp_u8*                  l_pucData;         /* Data buffer area                         */

/* Device information */
typedef struct {
    grp_u16                     usUsbDevId;                 /* Device identifier                */
    grp_u16                     usConfigNum;                /* Configuration number             */
    grp_u8                      ucInterfaceNum;             /* Interface number                 */
    grp_u8                      ucTotalIfNum;               /* Number of total interfaces       */
    grp_u16                     usCnfDescSize;              /* Size of configuration descriptor */
    grp_u8                      *pucDevDesc;                /* Device descriptor                */
    grp_u8                      *pucCnfDesc;                /* Configuration descriptor         */
    grp_usbdi_pipe              atPipe[16];                 /* Pipe handle information          */

} test_t_dev_info;

DLOCAL test_t_dev_info          l_tDevInfo;
#endif

/* Standard Device Request Information */
grp_usbdi_st_device_request     l_tStDevReq;
grp_usbdi_request               l_tNrmlReq;

/* Time out value */
#define TEST_TMOUT_VALUE        GRP_VOS_INFINITE/*1000*/


/**** INTERNAL FUNCTIONS PROTOTYPE DEFINES ******************************************************/
#if 0
LOCAL grp_s32 _grp_test_SmplInit(void);
LOCAL void    _grp_test_SmplTask(grp_u32);

LOCAL grp_s32 _grp_test_SmplConnect(void);
LOCAL grp_s32 _grp_test_SmplDisconnect(void);

LOCAL grp_s32 _grp_test_SmplInterfaceOpen(void);
LOCAL grp_s32 _grp_test_SmplNormalRequest(void);

LOCAL grp_s32 _grp_test_SmplNotify(grp_cnfsft_notify *ptNotify);
LOCAL grp_s32 _grp_test_SmplCmpSetConfiguration(grp_usbdi_st_device_request *ptStDevReq);
LOCAL grp_s32 _grp_test_SmplNormalRequestCb(grp_usbdi_request *ptReq);
#endif
LOCAL void _fusbhost_app_system_error(u8 fatal_err, u8 code);
LOCAL void _usbhost_app_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
/* GR-FILEの初期化に関する定義 -----------------------------------------------------------------*/
/* プールサイズ */
#define FILE_POOL_SIZE                  (1024 * 100)
grp_u32 g_aulGrFileBuf[FILE_POOL_SIZE/4] __attribute__ ((section ("TMP_NONCACHE"), aligned (4), zero_init));

/* 挿抜処理用タスク */
grp_vos_t_task      *g_ptConDconTask;
#define _CONDCON_TASK_NAME              ((grp_u8 *)"tConDcon")
#define _CONDCON_TASK_STACK             (1024)
#define _CONDCON_TASK_PRI               GRP_VOS_PRI_LOWEST
#define _CONDCON_TASK_STAT              GRP_VOS_READY

grp_vos_t_task      *g_ptAppTask;
#define _APP_TASK_NAME                  ((grp_u8 *)"tHostApp")
#define _APP_TASK_STACK                 (1024*4)
#define _APP_TASK_PRI                   GRP_VOS_PRI_LOWEST
#define _APP_TASK_STAT                  GRP_VOS_READY

/* FSIFからの通知用キュー */
extern grp_vos_t_queue          *g_ptXferQue;

#define _TEST_DATA_BUF_SIZE             (4*1024)
grp_u32                         g_aulTestWrBuf[_TEST_DATA_BUF_SIZE/4];
grp_u32                         g_aulTestRdBuf[_TEST_DATA_BUF_SIZE/4];
grp_u8 *                        g_pucTestWrBuf = (grp_u8 *)g_aulTestWrBuf;
grp_u8 *                        g_pucTestRdBuf = (grp_u8 *)g_aulTestRdBuf;

#define GRDBG_PRINT                     //

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL grp_si        _usb_test_GrFileInit(void);
LOCAL grp_si        _usb_test_AppInit(void);
LOCAL grp_s8*       _usb_test_GetMemoryArea(void);
LOCAL void          _usb_test_ConDconTask( grp_u32 ulArg);
LOCAL void          _usb_test_AppTask( grp_u32 ulArg);

/* External function prototypes */
EXTERN grp_si grp_fsif_dev_io_init(void);

/***** Complete Flag ****************************************************************************/
#if 0
#define GRP_TEST_CMP_INIT               ((grp_u32)0x00000000)
#define GRP_TEST_CMP_SETCONF            ((grp_u32)0x00000001)
#define GRP_TEST_CMP_NG                 ((grp_u32)0x80000000)

DLOCAL grp_u32                  l_ulCmpFlag = 0;

LOCAL grp_u32 _grp_test_SmplCheckFlag(void)
{
grp_si                          iCnt;

    for (iCnt=0; iCnt++; iCnt<1000) {
        if (l_ulCmpFlag) {
            break;
        }
        grp_vos_DelayTask(100);
    }

    return l_ulCmpFlag;
}
#endif

/************************************************************************************************/
/* FUNCTION   : _usb_test_GetMemoryArea                                                         */
/*                                                                                              */
/* DESCRIPTION: GR-FILE向けプール用メモリアドレスの変換                                         */
/*              ！！！ 使用する環境に併せてポーティングしてください ！！！                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : vpMemAddress                    メモリアドレス                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : 変換後のアドレス                Success                                         */
/*              USB_TEST_NULL                   Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s8* _usb_test_GetMemoryArea(void)
{
    /* GR-FILEで使用するメモリプールを非キャッシュ領域より割り当てます */
    return (grp_s8*)g_aulGrFileBuf;
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_Test_Stack_Init                                                           */
/*                                                                                              */
/* DESCRIPTION: GRUSB Stack Initialize                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBC_OK                 Success                                             */
/*              GRP_USBC_INIT_ERROR         Error                                               */
/************************************************************************************************/
int GRUSB_Test_Stack_Init(void)
{
    /* bus module */
    if (grp_bus_Init() != GRP_BUS_OK) {
        /* error */
        return GRP_USBC_INIT_ERROR;
    }

    /* Initialize GR-USB */
    if (grp_usbc_HostInit() != GRP_USBC_OK) {
        return GRP_USBC_INIT_ERROR;
    }

    /* From the device io module to MSC driver */
    if (grp_fsif_dev_io_init() != 0) {
        /* error */
        return GRP_USBC_INIT_ERROR;
    }

    /* GR-FILE */
    if (_usb_test_GrFileInit() != GRP_USBC_OK) {
        /* error */
        return GRP_USBC_INIT_ERROR;
    }
    if (_usb_test_AppInit() != GRP_USBC_OK) {
        return GRP_USBC_INIT_ERROR;
    }

#if 0
    /* Initialize Test Driver */
    if (_grp_test_SmplInit() != GRP_USBC_OK) {
        return GRP_USBC_INIT_ERROR;
    }
#endif
    /* start USB */
    if (grp_usbc_Enable() !=  GRP_USBC_OK) {
        return GRP_USBC_INIT_ERROR;
    }

    return GRP_USBC_OK;
}

/*==============================================================================================*/
/*                          Local Functions                                                     */
/*==============================================================================================*/

#if 0
/************************************************************************************************/
/* FUNCTION   : _grp_test_SmplInit                                                              */
/*                                                                                              */
/* DESCRIPTION: Printer class device driver sample initialization.                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBC_OK             Initialization is completed.                            */
/*              GRP_USBC_INIT_ERROR     Initialization is not completed.                        */
/************************************************************************************************/
LOCAL grp_s32 _grp_test_SmplInit(void)
{
grp_s32                         lResult;
grp_cnfsft_registration         tInput;

    /* Create CMEM area */
    lResult = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&l_pucData);
    if (lResult != GRP_CMEM_OK) {
        /* error */
        return GRP_USBC_INIT_ERROR;
    }

    /* Create Notice Connect and Disconecct Status Queue */
    lResult = grp_vos_CreateQueue(&l_ptConnectQue, (grp_u8*)"qTestCon", 4, 5);
    if (lResult != GRP_VOS_POS_RESULT) {
        /* error */
        return GRP_USBC_INIT_ERROR;
    }

    /* Create Notice processing completion Queue */
    lResult = grp_vos_CreateQueue(&l_ptCompleteQue, (grp_u8*)"qTestCmp", 4, 5);
    if (lResult != GRP_VOS_POS_RESULT) {
        /* error */
        return GRP_USBC_INIT_ERROR;
    }

    /* Create sample task */
    lResult = grp_vos_CreateTask(&l_ptPCDTask,
                                 (grp_u8*)"tTestSmp",
                                 _grp_test_SmplTask,
                                 2048,
                                 5,
                                 GRP_VOS_READY,
                                 0);

    if (lResult != GRP_VOS_POS_RESULT) {
        /* error */
        return GRP_USBC_INIT_ERROR;
    }

    /* Register device */
    tInput.usVendorID           = TEST_VENDOR_ID;
    tInput.usProductID          = TEST_PRODUCT_ID;
    tInput.ucInterfaceClass     = 0;
    tInput.ucInterfaceSubClass  = 0;
    tInput.ucInterfaceProtocol  = 0;
    tInput.ucDeviceClass        = 0;
    tInput.usLoadOption         = GRP_CNFSFT_VENDOR_SPECIFIED;
    tInput.pfnEventNotification = _grp_test_SmplNotify;
    tInput.pvReference          = 0;

    lResult = grp_cnfsft_SetNotification(&tInput);
    if (lResult != GRP_CNFSFT_OK) {
        /* error */
        return GRP_USBC_INIT_ERROR;
    }

    return GRP_USBC_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_test_SmplTask                                                              */
/*                                                                                              */
/* DESCRIPTION: USB printer class device driver sample task.                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulArg                   Not Use                                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _grp_test_SmplTask(grp_u32 ulArg)
{
grp_s32                         lRcvMsg;
grp_s32                         lStat;

    while (1) {
        /* Reset Queue (message clear) */
        while (grp_vos_ReceiveQueue(l_ptConnectQue,(void *)&lRcvMsg,GRP_VOS_NOWAIT) == GRP_VOS_POS_RESULT);

        /* Wait until printer class device is connected */
        if (grp_vos_ReceiveQueue(l_ptConnectQue, (void *)&lRcvMsg, GRP_VOS_INFINITE) == GRP_VOS_POS_RESULT) {
            if (lRcvMsg == TEST_CONNECT) {
                lStat = GRP_USBC_OK;
                /* Connect Process */
                if (lStat == GRP_USBC_OK) {
                    lStat = _grp_test_SmplConnect();
                }
                /* Pipe Open */
                if (lStat == GRP_USBC_OK) {
                    lStat = _grp_test_SmplInterfaceOpen();
                }
                /* Normal Request */
                if (lStat == GRP_USBC_OK) {
                    lStat = _grp_test_SmplNormalRequest();
                }
            }
            else if (lRcvMsg == TEST_DISCONNECT) {
                /* Disconnect Process */
                _grp_test_SmplDisconnect();
            }
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_test_SmplConnect                                                           */
/*                                                                                              */
/* DESCRIPTION: USB Device coonect processing.                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBC_OK             OK                                                      */
/*              GRP_USBC_ILLEGAL_ERROR  NG                                                      */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_test_SmplConnect(void)
{
grp_s32                         lStat = GRP_USBC_OK;
grp_u32                         ulFlag;
grp_usbdi_config_desc_b         *ptConfDesc = (grp_usbdi_config_desc_b *)l_tDevInfo.pucCnfDesc;

    /* 1. Set Configuration Descroptor */
    /* -- set parmeters -- */
    l_tStDevReq.usUsbDevId      = l_tDevInfo.usUsbDevId;
    l_tStDevReq.ucConfiguration = ptConfDesc->bConfigurationValue;
    l_tStDevReq.pvDescriptor    = (void *)l_tDevInfo.pucCnfDesc;
    l_tStDevReq.pfnStCbFunc     = _grp_test_SmplCmpSetConfiguration;
    /* -- flag set -- */
    l_ulCmpFlag = 0;
    /* -- request -- */
    lStat = grp_usbd_SetConfiguration(&l_tStDevReq);
    if (lStat != GRP_USBD_OK) {
        /* error */
        return GRP_USBC_ILLEGAL_ERROR;
    }
    else {
        ulFlag = _grp_test_SmplCheckFlag();
        if (ulFlag & GRP_TEST_CMP_NG) {
            /* error */
            return GRP_USBC_ILLEGAL_ERROR;
        }
    }

    return GRP_USBC_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_test_SmplDisconnect                                                        */
/*                                                                                              */
/* DESCRIPTION: USB Device disconnect processing.                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBC_OK             OK                                                      */
/*              GRP_USBC_ILLEGAL_ERROR  NG                                                      */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_test_SmplDisconnect(void)
{
grp_s32                         lStat = GRP_USBC_OK;
return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_test_SmplInterfaceOpen                                                     */
/*                                                                                              */
/* DESCRIPTION: USB/Host Interface Open Test.                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBC_OK             OK                                                      */
/*              GRP_USBC_ILLEGAL_ERROR  NG                                                      */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_test_SmplInterfaceOpen(void)
{
grp_s32                     lStat = GRP_USBC_OK;
grp_usbdi_pipe_operate      tPipeOpe;
grp_usbdi_descriptor_info   tDesInfo;
grp_usbdi_if_desc           tIfDesPtr;

    /* Analyze Interface descriptor */
    tDesInfo.ucInterface = l_tDevInfo.ucInterfaceNum;
    tDesInfo.ucAlternate = 0;
    tDesInfo.pvDesc      = l_tDevInfo.pucCnfDesc;
    tDesInfo.ulSize      = l_tDevInfo.usCnfDescSize;
    lStat = grp_usbd_SearchIfDescriptor(&tDesInfo, &tIfDesPtr);
    if (lStat == GRP_USBD_OK) {
        /* Interface Open */
        tPipeOpe.ucInterface  = l_tDevInfo.ucInterfaceNum;
        tPipeOpe.usUsbDevId   = l_tDevInfo.usUsbDevId;
        tPipeOpe.ucPipeNumber = 1;
        tPipeOpe.ptPipe       = &l_tDevInfo.atPipe[0];

//        tPipeOpe.atSelPipe[0].ucTransferMode       = GRP_USBD_BULK;
        tPipeOpe.atSelPipe[0].ucTransferMode       = GRP_USBD_INTERRUPT;
        tPipeOpe.atSelPipe[0].ucTransferDirection  = GRP_USBD_TX_IN;

        lStat = grp_usbd_InterfaceOpen(&tPipeOpe);
        if (lStat == GRP_USBD_OK) {
            /* OK */
        }
        else {
            /* error */
            lStat = GRP_USBC_ILLEGAL_ERROR;
        }
    }
    else {
        /* error */
        lStat = GRP_USBC_ILLEGAL_ERROR;
    }

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_test_SmplNormalRequest                                                     */
/*                                                                                              */
/* DESCRIPTION: USB/Host Normal Request Test.                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBC_OK             OK                                                      */
/*              GRP_USBC_ILLEGAL_ERROR  NG                                                      */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_test_SmplNormalRequest(void)
{
grp_s32             lStat;
grp_s32             lCbStat;
grp_s32             lRetStat = GRP_USBC_OK;
grp_usbdi_pipe      *ptPipe  = &l_tDevInfo.atPipe[0];
grp_u8              *pucBuf;

    lStat = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&pucBuf);
    if (lStat != GRP_USBD_OK) {
        return GRP_USBC_ILLEGAL_ERROR;
    }

    /*************************/
    /*---- TRANSFER (IN) ----*/
    /*************************/

    /* Search pipe (IN) */
    while (grp_vos_ReceiveQueue(l_ptCompleteQue, &lCbStat, GRP_VOS_NOWAIT) == GRP_VOS_POS_RESULT);

    /* Transfer request */
    l_tNrmlReq.ptPipe         = ptPipe;
    l_tNrmlReq.pucBuffer      = pucBuf;
    l_tNrmlReq.ulBufferLength = 512;
    l_tNrmlReq.iShortXferOK   = GRP_USB_TRUE;
    l_tNrmlReq.pfnNrCbFunc    = _grp_test_SmplNormalRequestCb;
    l_tNrmlReq.pvReferenceP   = (void *)0;

    lStat = grp_usbd_NormalRequest(&l_tNrmlReq);
    if (lStat == GRP_USBD_OK) {
        /* Wait notification of completion from callback */
        lStat = grp_vos_ReceiveQueue(l_ptCompleteQue, &lCbStat, TEST_TMOUT_VALUE);
        if (lStat == GRP_VOS_POS_RESULT) {

            if (lCbStat == GRP_USBD_TR_STALL) {
/*                lStat = _UsbHTest_StallRecovery(ptPipe);*/
                lRetStat = 1;
                lStat    = GRP_USBC_ILLEGAL_ERROR;
// 本来はリカバリの処理が必要だが、今回はとりあえずエラー 2020.04.28
            }
        }
        else if (lStat == GRP_VOS_NEG_NO_MESSAGE) {
            /* Cancel request */
            grp_usbd_NormalRequestCancel(&l_tNrmlReq);
            lRetStat = 9;
            lStat    = GRP_USBC_ILLEGAL_ERROR;
        }
        else {
            lRetStat = 10;
            lStat    = GRP_USBC_ILLEGAL_ERROR;
        }
    }
    else {
        lRetStat = 11;
        lStat    = GRP_USBC_ILLEGAL_ERROR;
    }

    grp_cmem_BlkRel(pucBuf);

    return lStat;
}

/*==============================================================================================*/
/*                          Callback Function                                                   */
/*==============================================================================================*/
/************************************************************************************************/
/* FUNCTION   : _grp_test_SmplNotify                                                            */
/*                                                                                              */
/* DESCRIPTION: GRUSB printer class device connect/disconnect callback.                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : *ptUsrReq               Pointer of PCD request structure.                       */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_test_SmplNotify(grp_cnfsft_notify *ptNotify)
{
grp_s32                         lSendMsg;
grp_s32                         lStat = 0;
grp_usbdi_config_desc_b*        ptCnfDesc;
grp_u16                         usCnfDescSize;

    switch (ptNotify->usEvent) {
    case GRP_CNFSFT_DEVICE_ATTACHED:
        /* Printer device connect */
        lSendMsg = TEST_CONNECT;
        /* Get configuration descriptor size */
        ptCnfDesc     = (grp_usbdi_config_desc_b *)ptNotify->pucConfDesc;
        usCnfDescSize = (grp_u16)((grp_u16)(ptCnfDesc->wTotalLength_Low)
                               | ((grp_u16)(ptCnfDesc->wTotalLength_High) << 8));

        /* Save device information */
        l_tDevInfo.usUsbDevId     = ptNotify->usUsbDevId;
        l_tDevInfo.usConfigNum    = ptNotify->usConfigIdx;
        l_tDevInfo.ucInterfaceNum = ptNotify->ucInterfaceNum;
        l_tDevInfo.ucTotalIfNum   = ptNotify->ucTotalIfNum;
        l_tDevInfo.pucDevDesc     = ptNotify->pucDevDesc;
        l_tDevInfo.pucCnfDesc     = ptNotify->pucConfDesc;
        l_tDevInfo.usCnfDescSize  = usCnfDescSize;
        break;

    case GRP_CNFSFT_DEVICE_DETACHED:
        /* Printer device disconnect */
        lSendMsg = TEST_DISCONNECT;
        break;
    }

    lStat = (grp_s32)grp_vos_SendQueue(l_ptConnectQue, (void *)&lSendMsg, GRP_VOS_NOWAIT);

    return lStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_test_SmplCmpSetConfiguration                                               */
/*                                                                                              */
/* DESCRIPTION: SET_CONFIGURATION callback function                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptStDevReq                      standard device request structure               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBC_OK             OK                                                      */
/*              GRP_USBC_ILLEGAL_ERROR  NG                                                      */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_test_SmplCmpSetConfiguration(grp_usbdi_st_device_request *ptStDevReq)
{

    if (ptStDevReq->lStatus == GRP_USBD_TR_NO_FAIL) {
        /* --- success --- */
        l_ulCmpFlag = GRP_TEST_CMP_SETCONF;
    }
    else {
        /* --- error --- */
        l_ulCmpFlag = GRP_TEST_CMP_NG;
    }

    return GRP_USBC_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_test_SmplNormalRequestCb                                                   */
/*                                                                                              */
/* DESCRIPTION: This is callback function of _grp_test_SmplNormalRequest.                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptReq                           Request structure                               */
/*                                                                                              */
/*               (Member used)                                                                  */
/*              lStatus                         Communication status                            */
/*              ulActualLength                  Actual transferred data length                  */
/*              pvReferenceP                    User pointer                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_USBD_OK                     Success                                         */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_test_SmplNormalRequestCb(grp_usbdi_request *ptReq)
{
    grp_vos_SendQueue(l_ptCompleteQue, (void *)&ptReq->lStatus, GRP_VOS_INFINITE);

    return GRP_USBD_OK;
}
#endif

/************************************************************************************************/
/* FUNCTION   : _usb_test_GrFileInit                                                            */
/*                                                                                              */
/* DESCRIPTION: Initialize of GR-FILE.                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : USB_TEST_OK                     Success                                         */
/*              USB_TEST_NG                     Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_si _usb_test_GrFileInit(void)
{
    grp_s8                          *pcPoolArea;

    /*--- GR-FILE向けメモリ管理の初期化 ---*/
    /* メモリエリアの取得 */
    pcPoolArea = _usb_test_GetMemoryArea();
    if (pcPoolArea == 0) {
        /* error */
        return USB_TEST_NG;
    }
    /* メモリ管理の初期化 */
    grp_mem_vl_init((char *)pcPoolArea, FILE_POOL_SIZE);

    /*--- GR-FILEの初期化 ---*/
    /* configurationパラメータ(必要ならば) */
        /* 本アプリケーションではデフォルト値を利用する */
    /* GR-FILEの初期化 */
    if (grp_fs_init() != 0) {
        /* error */
        return USB_TEST_NG;
    }

    /*--- フック関数の初期化処理 ---*/
    fat_interrupt_lookup = 0;

    /*--- 標準入出力関数登録(必要ならば) ---*/
        /* 本アプリケーションでは不要なので未設定 */
        /* grp_stdio_io_stdout = GRP_USB_NULL; */
        /* grp_stdio_io_stdin  = GRP_USB_NULL; */

    /*--- I/Oエラー関数登録(必要ならば) ---*/
        /* 本アプリケーションでは不要なので未設定 */

    return USB_TEST_OK;
}

/************************************************************************************************/
/* FUNCTION   : _usb_test_ConDconTask                                                           */
/*                                                                                              */
/* DESCRIPTION: MSメディアの挿抜を監視するタスク                                                */
/*              本タスクでは接続および切断の通知のみを行います。                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulArg                           reserved                                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL void _usb_test_ConDconTask( grp_u32 ulArg)
{
grp_si                          iPtn;

    /* ワーニング対策 */
    ulArg = 0;

    while (1) {
        if (grp_vos_ReceiveQueue(   g_ptXferQue,
                                    (void *)&iPtn,
                                    GRP_VOS_INFINITE) != GRP_VOS_POS_RESULT) {
            continue;
        }

        if (iPtn & GRP_FSIF_ATTACHED_MEDIA) {
            GRDBG_PRINT("--> CONNECT USB0 \r\n");

/* ----> for mode change */
            /* 切り替え監視タスクへメディア接続通知 */
            (void)grp_usb_mdchg_SendMediaState(GRP_MEDIA_ATTACHED);
/* <---- for mode change */
        }
        else {
            GRDBG_PRINT("<-- DISCONNECT USB0 \r\n");

/* ----> for mode change */
            /* 切り替え監視タスクへメディア切断通知 */
            (void)grp_usb_mdchg_SendMediaState(GRP_MEDIA_DETACHED);
/* <---- for mode change */
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : _usb_test_mount                                                                 */
/*                                                                                              */
/* DESCRIPTION: マウント処理を実行します                                                        */
/*----------------------------------------------------------------------------------------------*/
grp_si _usb_test_mount(void)
{
    grp_u8                          aucDevName[] = "usb0";
    grp_si                          iRet = 0;

    iRet = grp_fs_mount((const char *)aucDevName, "/", "fat", GRP_FS_SYNC_FL_CLOSE);
    if (iRet == 0) {
        GRDBG_PRINT("     O mount ok\r\n");
    }
    else if (iRet == GRP_FS_ERR_NEED_CHECK) {
        GRDBG_PRINT("     X mount ng - need force to mount\r\n");
    }
    else {
        GRDBG_PRINT("     X mount ng\r\n");
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_force_mount                                                           */
/*                                                                                              */
/* DESCRIPTION: マウント処理を実行します                                                        */
/*----------------------------------------------------------------------------------------------*/
grp_si _usb_test_force_mount(void)
{
    grp_u8                          aucDevName[] = "usb0";
    grp_si                          iRet = 0;

    iRet = grp_fs_mount((const char *)aucDevName, "/", "fat", (GRP_FS_SYNC_FL_CLOSE | GRP_FS_FORCE_MOUNT));
    if (iRet == 0) {
        GRDBG_PRINT("     O force mount ok\r\n");
    }
    else {
        GRDBG_PRINT("     X force mount ng\r\n");
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_unmount                                                               */
/*                                                                                              */
/* DESCRIPTION: アンマウント処理を実行します                                                    */
/*----------------------------------------------------------------------------------------------*/
grp_si _usb_test_unmount(void)
{
    grp_u8                          aucDevName[] = "usb0";
    grp_si                          iRet = 0;

    iRet = grp_fs_unmount((const char *)aucDevName, 0);
    if (iRet == 0) {
        GRDBG_PRINT("     O unmount ok\r\n");
    }
    else if (iRet == GRP_FS_ERR_BUSY) {
        GRDBG_PRINT("     X unmount ng - need force to unmount\r\n");
    }
    else {
        GRDBG_PRINT("     X unmount ng\r\n");
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_force_unmount                                                         */
/*                                                                                              */
/* DESCRIPTION: アンマウント処理を実行します                                                    */
/*----------------------------------------------------------------------------------------------*/
grp_si _usb_test_force_unmount(void)
{
    grp_u8                          aucDevName[] = "usb0";
    grp_si                          iRet = 0;

    iRet = grp_fs_unmount((const char *)aucDevName, GRP_FS_FORCE_UMOUNT);
    if (iRet == 0) {
        GRDBG_PRINT("     O force unmount ok\r\n");
    }
    else {
        GRDBG_PRINT("     X force unmount ng\r\n");
        iRet = -1;
    }
    return iRet;
}
#if 0
/************************************************************************************************/
/* FUNCTION   : _usb_test_ls                                                                    */
/*                                                                                              */
/* DESCRIPTION: リストの取得処理を実行しシリアルにファイル名を出力します                        */
/*----------------------------------------------------------------------------------------------*/
#define _USB_TEST_FILE_NAME_MAX         (16)
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_test_ls(void)
{
    grp_si                          iHdr    = 0;
    grp_si                          iRet    = 0;
    grp_fs_dir_ent_t                tDirent;
    grp_u8                          aucName[_USB_TEST_FILE_NAME_MAX*2+1];

    /* NULLストッパーの付加（） */
    aucName[_USB_TEST_FILE_NAME_MAX*2] = 0;

    /* ルートディレクトリの情報しか取らないため */
    iHdr = -1;

    GRDBG_PRINT("--- File List---\r\n");
    tDirent.pucName     = aucName;                          /* set file aucName buffer      */
    tDirent.sNameSize   = _USB_TEST_FILE_NAME_MAX*2;        /* set aucName buffer size      */
    tDirent.uiStart     = 0;                                /* start offset is 0            */
    tDirent.uiEnd       = 0;                                /* end offset is 0              */
    while ((iRet = grp_fs_get_dirent(iHdr, &tDirent)) > 0) {
        GRDBG_PRINT("%s\r\n", tDirent.pucName);
        tDirent.sNameSize = _USB_TEST_FILE_NAME_MAX*2;      /* set aucName buffer size      */
        tDirent.uiStart   = tDirent.uiEnd;                  /* set next                     */
    }
    GRDBG_PRINT("\r\n");
    return 0;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_format                                                                */
/*                                                                                              */
/* DESCRIPTION: フォーマットを行ないます                                                        */
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_test_format(void)
{
    grp_fat_format_param_t          param;
    grp_fs_media_info_t             media;
    grp_si                          iRet;
    grp_u8                          aucDevName[] = "usb0";

    memset(&param, 0, sizeof(param));
    memset(&media, 0, sizeof(media));
    iRet = grp_fat_format(aucDevName, &param, &media);
    if (iRet >= 0) {    /* ok */
        GRDBG_PRINT("     O format ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X format ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_open_file                                                             */
/*                                                                                              */
/* DESCRIPTION: ファイルをオープンします                                                        */
/*----------------------------------------------------------------------------------------------*/
grp_si                          g_iHandle1    = -1;
grp_u8                          g_aucTestFileName[] = "TEST1.TXT";
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_test_open_file(void)
{
    grp_si                          iMode       = 0;
    grp_si                          iProt       = 0;
    grp_si                          iRet;
    grp_u8                          aucFilneName[_USB_TEST_FILE_NAME_MAX*2+1];

    grp_std_strcpy(aucFilneName, g_aucTestFileName);
    iMode = GRP_FS_O_RDWR | GRP_FS_O_CREAT /*| GRP_FS_O_DIRECT_IO*/;
    iProt = GRP_FS_PROT_RWXA;                                           /* すべて許可   */
    iRet = grp_fs_open(aucFilneName, iMode, iProt);
    if (iRet >= 0) {    /* ok */
        g_iHandle1 = iRet;
        iRet = 0;
        GRDBG_PRINT("     O file open ok\r\n");
    }
    else {              /* ng */
        g_iHandle1 = -1;
        GRDBG_PRINT("     X file open ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_close_file                                                            */
/*                                                                                              */
/* DESCRIPTION: ファイルをクローズします                                                        */
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_test_close_file(void)
{
    grp_si                          iRet;

    if (g_iHandle1 == -1) {
        /* error */
        GRDBG_PRINT("     X file close ng <not open file>\r\n");
    }

    iRet = grp_fs_close(g_iHandle1);
    if (iRet == 0) {    /* ok */
        g_iHandle1 = -1;
        GRDBG_PRINT("     O file close ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file close ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_write_file                                                            */
/*                                                                                              */
/* DESCRIPTION: ファイルにデータを書き込みます                                                  */
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_test_write_file(void)
{
    grp_si                          iRet;
    grp_u8 *                        pucBuf = (grp_u8 *)g_pucTestWrBuf;
    grp_u32                         ulDataSz = _TEST_DATA_BUF_SIZE;
    grp_si                          i;
    grp_si                          j;

    /* テストデータでバッファを埋める */
    for (i=0, j=0; i<ulDataSz; i++) {
        *(pucBuf + i) = i;
        if (i%512 == 0) {
            *(pucBuf + i) = j++;
        }
    }

    if (g_iHandle1 == -1) {
        /* error */
        GRDBG_PRINT("     X file write ng <not open file>\r\n");
    }

    /* ファイルオープンで GRP_FS_O_DIRECT_IO を指定している場合は */
    /* 非キャッシュ領域のバッファを引数に渡します */
    iRet = grp_fs_write(g_iHandle1, pucBuf, ulDataSz);
    if (iRet >= 0) {    /* ok */
        iRet = 0;
        GRDBG_PRINT("     O file write ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file write ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_read_file                                                             */
/*                                                                                              */
/* DESCRIPTION: ファイルからデータを読み込みます                                                */
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_test_read_file(void)
{
    grp_si                          iRet;
    grp_u8 *                        pucBuf = (grp_u8 *)g_pucTestRdBuf;
    grp_u32                         ulDataSz = _TEST_DATA_BUF_SIZE;

    if (g_iHandle1 == -1) {
        /* error */
        GRDBG_PRINT("     X file read ng <not open file>\r\n");
    }

    /* バッファの初期化 */
    grp_std_memset(pucBuf, 0, ulDataSz);

    /* ファイルオープンで GRP_FS_O_DIRECT_IO を指定している場合は */
    /* 非キャッシュ領域のバッファを引数に渡します */
    iRet = grp_fs_read(g_iHandle1, pucBuf, ulDataSz);
    if (iRet >= 0) {    /* ok */
        iRet = 0;
        GRDBG_PRINT("     O file read ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file read ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_del_file                                                              */
/*                                                                                              */
/* DESCRIPTION: ファイルを削除します                                                            */
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_test_del_file(void)
{
    grp_si                          iRet;

    if (g_iHandle1 != -1) {
        /* error */
        GRDBG_PRINT("     X file delete ng <opened file>\r\n");
    }

    iRet = grp_fs_unlink(g_aucTestFileName);
    if (iRet == 0) {    /* ok */
        GRDBG_PRINT("     O file delete ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file delete ng\r\n");
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_check_file                                                            */
/*                                                                                              */
/* DESCRIPTION: 読み込んだファイルを書き込んだデータと比較します                                */
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_test_check_file(void)
{
    grp_si                          iRet;

    iRet = grp_std_memcmp(g_pucTestWrBuf, g_pucTestRdBuf, _TEST_DATA_BUF_SIZE);
    if (iRet == 0) {    /* ok */
        GRDBG_PRINT("     O file check ok\r\n");
    }
    else {              /* ng */
        GRDBG_PRINT("     X file check ng\r\n");
    }

    /* バッファの初期化 */
    grp_std_memset(g_pucTestRdBuf, 0, _TEST_DATA_BUF_SIZE);

    return iRet;
}
#endif
/************************************************************************************************/
/* FUNCTION   : _usb_download_mount                                                                 */
/*                                                                                              */
/* DESCRIPTION: マウント処理を実行します                                                        */
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_download_mount(void)
{
    grp_u8                          aucDevName[] = "usb0";
    grp_si                          iRet = 0;

#if 0
    iRet = grp_fs_mount((const char *)aucDevName, "/", "fat", GRP_FS_SYNC_FL_CLOSE);
#else
    iRet = grp_fs_mount((const char *)aucDevName, "/", "fat", (GRP_FS_SYNC_FL_CLOSE | GRP_FS_FORCE_MOUNT));
#endif
    if (iRet == 0) {
		osw_printf( "_usb_download_mount iRet = 0\n" );
    }
    else if (iRet == GRP_FS_ERR_NEED_CHECK) {
		osw_printf( "_usb_download_mount iRet = GRP_FS_ERR_NEED_CHECK\n" );
    }
    else if (iRet == GRP_FS_ERR_IO) {
		osw_printf( "_usb_download_mount iRet = GRP_FS_ERR_IO\n" );
    }
    else {
		osw_printf( "_usb_download_mount %08x\n", iRet );
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_download_unmount                                                               */
/*                                                                                              */
/* DESCRIPTION: アンマウント処理を実行します                                                    */
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_download_unmount(void)
{
    grp_u8                          aucDevName[] = "usb0";
    grp_si                          iRet = 0;

    iRet = grp_fs_unmount((const char *)aucDevName, 0);
    if (iRet == 0) {
    }
    else if (iRet == GRP_FS_ERR_BUSY) {
        iRet = grp_fs_unmount((const char *)aucDevName, GRP_FS_FORCE_UMOUNT);
    }
    else {
        iRet = -1;
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_download_ls                                                                */
/*                                                                                              */
/* DESCRIPTION: リストの取得処理を実行し、ファイルサイズをチェックします。                        											*/
/*----------------------------------------------------------------------------------------------*/
#define _USB_DOWNLOAD_FILE_NAME_MAX         (32)
grp_u8                          aucDownloadFileName[_USB_DOWNLOAD_FILE_NAME_MAX*2+1];
grp_si                          iHdr    = 0;
grp_fs_dir_ent_t                tDirent;
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_download_ls(void)
{
    grp_si                          iRet    = 1;

    /* NULLストッパーの付加（） */
    aucDownloadFileName[_USB_DOWNLOAD_FILE_NAME_MAX*2] = 0;

    tDirent.pucName     = aucDownloadFileName;              /* set file aucDownloadFileName buffer      */
    tDirent.sNameSize   = _USB_DOWNLOAD_FILE_NAME_MAX*2;        /* set aucDownloadFileName buffer size      */
    while ((iRet = grp_fs_get_dirent(iHdr, &tDirent)) > 0) {
        tDirent.sNameSize = _USB_DOWNLOAD_FILE_NAME_MAX*2;      /* set aucName buffer size      */
        tDirent.uiStart   = tDirent.uiEnd;                  /* set next                     */
    	if ((tDirent.ucType == GRP_FS_FILE_FILE)
    	 && (tDirent.uiSize == _DOWNLOAD_FILE_SIZE))
    	{		/* regular file & firmware file size */
			iRet    = 0;
			break;
    	}
    }
    aucDownloadFileName[_USB_DOWNLOAD_FILE_NAME_MAX*2] = 0;
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_download_open_file                                                             */
/*                                                                                              */
/* DESCRIPTION: ファイルをオープンします                                                        */
/*----------------------------------------------------------------------------------------------*/
grp_si                          g_iDownloadFileHandle    = -1;
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_download_open_file(void)
{
    grp_si                          iMode       = 0;
    grp_si                          iProt       = 0;
    grp_si                          iRet;
    grp_u8                          aucFilneName[_USB_DOWNLOAD_FILE_NAME_MAX*2+1];

    grp_std_strcpy(aucFilneName, aucDownloadFileName);
    iMode = GRP_FS_O_RDONLY | GRP_FS_O_DIRECT_IO;
    iProt = GRP_FS_PROT_RWXA;                                           /* すべて許可   */
    iRet = grp_fs_open(aucFilneName, iMode, iProt);
    if (iRet >= 0) {    /* ok */
    	g_iDownloadFileHandle = iRet;
        iRet = 0;
    }
    else {              /* ng */
    	g_iDownloadFileHandle = -1;
    }
    return iRet;
}

/************************************************************************************************/
/* FUNCTION   : _usb_download_read_file                                                         */
/*                                                                                              */
/* DESCRIPTION: ファイルからデータを読み込みます                                                                                                                                                                          */
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_download_read_file(void)
{
    grp_si                          iRet;
    grp_u8 *                        pucBuf = (grp_u8 *)g_pucDownloadRdBuf;
    grp_u32                         ulDataSz = _READ_DATA_BUF_SIZE;

    if (g_iDownloadFileHandle == -1) {
        /* error */
    }

    /* バッファの初期化 */
#if 1
    int count = ulDataSz;
	u8 *dst_byte = (u8 *) pucBuf;

	while (count--) {
		*dst_byte = 0;
		dst_byte++;
	}
#else
    grp_std_memset(pucBuf, 0, ulDataSz);
#endif
#if 0
    grp_u32                         ulReadSz = 0x00010000;
    grp_u32                         ulDataIdx = 0;
	while (ulDataIdx < ulDataSz)
	{
		_hal_feed_wdt();
	    iRet = grp_fs_read(g_iDownloadFileHandle, pucBuf[ulDataIdx], ulReadSz);
	    if (iRet >= 0) {    /* ok */
	        iRet = 0;
	    }
	    else {              /* ng */
	    }
	    ulDataIdx += ulReadSz;
	}
#else
    /* ファイルオープンで GRP_FS_O_DIRECT_IO を指定している場合は */
    /* 非キャッシュ領域のバッファを引数に渡します */
    iRet = grp_fs_read(g_iDownloadFileHandle, pucBuf, ulDataSz);
    if (iRet >= 0) {    /* ok */
        iRet = 0;
    }
    else {              /* ng */
    }
#endif
    return iRet;
}

/************************************************************************************************/
/* FUNCTION   : _usb_download_close_file                                                        */
/*                                                                                              */
/* DESCRIPTION: ファイルをクローズします                                                                                                                                                                                        */
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_download_close_file(void)
{
    grp_si                          iRet;

    if (g_iDownloadFileHandle == -1) {
    }

    iRet = grp_fs_close(g_iDownloadFileHandle);
    if (iRet == 0) {    /* ok */
    	g_iDownloadFileHandle = -1;
    }
    else {              /* ng */
    }
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _download_header_check                                                          */
/*                                                                                              */
/* DESCRIPTION: ダウンロードできるファイルか否かをチェックします                                                                                                                                                         */
/*----------------------------------------------------------------------------------------------*/
grp_si                          g_iDownloadFileStartAddress    = -1;
grp_si                          g_iDownloadFileEndAddress      = -1;
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _download_header_check(void)
{
    grp_si                          iRet = 0;

#if (defined(PRJ_IVIZION2) && (BV_UNIT_TYPE>=WS2_MODEL))
	/* 識別子文字列が "VFM20"でなければDLしない */
	if(	g_pucDownloadRdBuf[0] != 'V'
	 || g_pucDownloadRdBuf[1] != 'F'
	 || g_pucDownloadRdBuf[2] != 'M'
	 ||	g_pucDownloadRdBuf[3] != '2'
	 ||	g_pucDownloadRdBuf[4] != '0' )
	{
		return(-1);
	}
#else
	/* 識別子文字列が "VFM18"でなければDLしない */
	if(	g_pucDownloadRdBuf[0] != 'V'
	 || g_pucDownloadRdBuf[1] != 'F'
	 || g_pucDownloadRdBuf[2] != 'M'
	 ||	g_pucDownloadRdBuf[3] != '2'
	 ||	g_pucDownloadRdBuf[4] != '1' )
	{
		return(-1);
	}
#endif
	g_iDownloadFileStartAddress = (g_pucDownloadRdBuf[6] << 24)
			+ (g_pucDownloadRdBuf[7] << 16)
			+ (g_pucDownloadRdBuf[8] << 8)
			+ g_pucDownloadRdBuf[9];
	if(	g_iDownloadFileStartAddress != ROM_ALIAS_START_ADDRESS - DDR_START_ADDRESS )
	{
		return(-1);
	}
	g_iDownloadFileEndAddress = (g_pucDownloadRdBuf[10] << 24)
			+ (g_pucDownloadRdBuf[11] << 16)
			+ (g_pucDownloadRdBuf[12] << 8)
			+ g_pucDownloadRdBuf[13];
	if(	g_iDownloadFileEndAddress != ROM_ALIAS_END_ADDRESS - DDR_START_ADDRESS )
	{
		return(-1);
	}
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _download_file_crc_check                                                        */
/*                                                                                              */
/* DESCRIPTION: ダウンロードできるファイルか否かをチェックします                                                                                                                                                         */
/*----------------------------------------------------------------------------------------------*/
extern u16 _calc_crc(u8 *data, u32 length, bool erase);
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _download_file_crc_check(void)
{
#if 1
    grp_si                          iRet;
    grp_u16 calc_crc = 0;
    grp_u16 tmp_crc;
    bool erase = true;
    const grp_u32 block_size = 0x10000;
    grp_u32 total, length, index;

    if (g_iDownloadFileStartAddress == -1) {
		return(-1);
    }
    if (g_iDownloadFileEndAddress == -1) {
		return(-1);
    }

    total = g_iDownloadFileEndAddress - g_iDownloadFileStartAddress - 2 + 1;
    index = 0;
    while(total)
    {
    	if(total < block_size)
    	{
        	length = total;
    	}
    	else
    	{
        	length = block_size;
    	}
        /* CRC算出 */
        calc_crc = _calc_crc(	(u8 *)&g_pucDownloadRdBuf[index],
    							(u32)length,
    							erase );
        total -= length;
        index += length;
        erase = false;
        grp_vos_DelayTask(1);
    }
    tmp_crc = g_pucDownloadRdBuf[g_iDownloadFileEndAddress - g_iDownloadFileStartAddress - 1];
    tmp_crc = (tmp_crc << 8) & 0xff00;
    tmp_crc = tmp_crc + g_pucDownloadRdBuf[g_iDownloadFileEndAddress - g_iDownloadFileStartAddress];

    if( tmp_crc == calc_crc )
    {
    	iRet = 0;
    }
    else
    {
    	iRet = -1;
    }
#else
    grp_si                          iRet;
    grp_u16 calc_crc;
    grp_u16 tmp_crc;

    if (g_iDownloadFileStartAddress == -1) {
		return(-1);
    }
    if (g_iDownloadFileEndAddress == -1) {
		return(-1);
    }

    /* CRC算出 */
    calc_crc = _calc_crc(	(u8 *)g_pucDownloadRdBuf,
							(u32)g_iDownloadFileEndAddress-g_iDownloadFileStartAddress-2+1,
							true );
    tmp_crc = g_pucDownloadRdBuf[g_iDownloadFileEndAddress - g_iDownloadFileStartAddress - 1];
    tmp_crc = (tmp_crc << 8) & 0xff00;
    tmp_crc = tmp_crc + g_pucDownloadRdBuf[g_iDownloadFileEndAddress - g_iDownloadFileStartAddress];

    if( tmp_crc == calc_crc )
    {
    	iRet = 0;
    }
    else
    {
    	iRet = -1;
    }
#endif
    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_download_check_file                                                        */
/*                                                                                              */
/* DESCRIPTION: ダウンロードできるファイルか否かをチェックします                                                                                                                                                                                        */
/*----------------------------------------------------------------------------------------------*/
LOCAL grp_si _usb_download_check_file(void)
{
    grp_si                          iRet;

    /* ルートディレクトリの情報しか取らないため */
    iHdr = -1;
    tDirent.uiStart     = 0;                                /* start offset is 0            */
    tDirent.uiEnd       = 0;                                /* end offset is 0              */

	osw_printf( "_usb_download_mount\n" );
	if ((iRet = _usb_download_mount()) == 0) {
		osw_printf( "_usb_download_ls\n" );
		while (_usb_download_ls() == 0)
		{
			osw_printf( "_usb_download_open_file\n" );
			_hal_feed_wdt();
		    if (_usb_download_open_file() != 0)
		    {
		    	continue;
		    }
			osw_printf( "_usb_download_read_file\n" );
			_hal_feed_wdt();
		    if (_usb_download_read_file() != 0)
		    {
		    	continue;
		    }
			osw_printf( "_usb_download_close_file\n" );
			_hal_feed_wdt();
		    if (_usb_download_close_file() != 0)
		    {
		    	continue;
		    }
			osw_printf( "_download_header_check\n" );
			_hal_feed_wdt();
			/* ヘッダーチェック、ファイルCRCチェックを行う */
			iRet = _download_header_check();
			if(iRet != 0)
			{
				continue;
			}
			osw_printf( "_download_file_crc_check\n" );
			_hal_feed_wdt();
			iRet = _download_file_crc_check();
			if(iRet == 0)
			{
				break;
			}
		}
	}
	osw_printf( "_usb_download_unmount\n" );
	_usb_download_unmount();

    return iRet;
}

/************************************************************************************************/
/* FUNCTION   : _usb_WaitAttach                                                         */
/*                                                                                              */
/* DESCRIPTION: Registration of the interrupt handler                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : pulMode                     attach or not		                                */
/*                                                                                              */
/* RESULTS    : GRP_MDCHG_OK                Success                                             */
/*              GRP_MDCHG_NG                Error                                               */
/*                                                                                              */
/************************************************************************************************/
grp_vos_t_queue*         l_ptAttachQue = NULL;    /* Notice Attach Queue   */
INT _usb_WaitAttach( UINT32 *pulMode )
{
INT     iRet = GRP_MDCHG_OK;

    if (grp_vos_ReceiveQueue(l_ptAttachQue, (void *)pulMode, GRP_VOS_INFINITE) != GRP_VOS_POS_RESULT)
    {
        iRet = GRP_MDCHG_NG;
    }

    return iRet;
}
/************************************************************************************************/
/* FUNCTION   : _usb_test_AppTask                                                               */
/*                                                                                              */
/* DESCRIPTION: テストアプリケーション用のタスク                                                */
/*              本タスクではグローバル変数(g_ulTestNum)を変更することで色々なテストを実施します */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulArg                           reserved                                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
UINT32                          g_ulTestNum = 0;
/*----------------------------------------------------------------------------------------------*/
LOCAL void _usb_test_AppTask( grp_u32 ulArg)
{
    UINT32      ulMsg;
    /* ワーニング対策 */
    ulArg = 0;


    g_ulTestNum = 0;
    while (1) {
        switch (g_ulTestNum) {
        case 0:     /* アイドル */
            if (GRP_MDCHG_OK == _usb_WaitAttach( &ulMsg ))
            {
                g_ulTestNum = 1;
            }
            grp_vos_DelayTask(100);
            break;
        case 1:
            if(_usb_download_check_file() != 0)
            	g_ulTestNum = 0;
            else
            	g_ulTestNum = 2;
           	break;
        case 2:
        	_usbhost_app_send_msg(ID_DLINE_MBX, TMSG_OTG_NOTICE, TMSG_SUB_DOWNLOAD, 0, 0, 0);
            g_ulTestNum = 0;
            break;

#if 0
        case 1:     /* マウント */
            _usb_test_mount();                              g_ulTestNum = 0;        break;
        case 2:     /* アンマウント */
            _usb_test_unmount();                            g_ulTestNum = 0;        break;
        case 3:     /* ファイルリスト */
            _usb_test_ls();                                 g_ulTestNum = 0;        break;
        case 5:     /* 強制マウント */
            _usb_test_force_mount();                        g_ulTestNum = 0;        break;
        case 6:     /* 強制アンマウント */
            _usb_test_force_unmount();                      g_ulTestNum = 0;        break;
        case 7:     /* フォーマット */
            _usb_test_format();                             g_ulTestNum = 0;        break;
        case 10:    /* ファイルオープン */
            _usb_test_open_file();                          g_ulTestNum = 0;        break;
        case 11:    /* ファイルクローズ */
            _usb_test_close_file();                         g_ulTestNum = 0;        break;
        case 12:    /* ファイルライト g_pucTestWrBuf に設定しファイルへ書き込み */
            _usb_test_write_file();                         g_ulTestNum = 0;        break;
        case 13:    /* ファイルリード g_pucTestRdBuf にファイルより読みこみ */
            _usb_test_read_file();                          g_ulTestNum = 0;        break;
        case 14:    /* ファイル削除 */
            _usb_test_del_file();                           g_ulTestNum = 0;        break;
        case 15:    /* ファイル比較 g_pucTestWrBuf と g_pucTestRdBuf の比較 */
            _usb_test_check_file();                         g_ulTestNum = 0;        break;

        case 20:    /* ファイルのオープン～ライト～クローズ */
            if (_usb_test_open_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_write_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_close_file() != 0) {              g_ulTestNum = 0;        break;}
            g_ulTestNum = 0;
            break;
        case 21:    /* ファイルのオープン～リード～クローズ */
            if (_usb_test_open_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_read_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_close_file() != 0) {              g_ulTestNum = 0;        break;}
            g_ulTestNum = 0;
            break;

        case 30:     /* マウント-リスト-アンマウントの繰り返し */
            _usb_test_mount();
            _usb_test_ls();
            _usb_test_unmount();
            grp_vos_DelayTask(500);
            break;
        case 31:     /* マウント-リスト-アンマウントの繰り返し */
            _usb_test_mount();
            _usb_test_unmount();
            grp_vos_DelayTask(500);
            break;
        case 32:    /* ファイル比較連続テスト */
            if (_usb_test_open_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_write_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_close_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_open_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_read_file()  != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_close_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_check_file() != 0) {              g_ulTestNum = 0;        break;}
            if (_usb_test_del_file()   != 0) {              g_ulTestNum = 0;        break;}
            grp_vos_DelayTask(500);
            break;

        case 70:     /* 再初期化処理 */
        	grp_reinit_HostReInit();
            grp_vos_DelayTask(500);
        	g_ulTestNum = 0;
            break;
#endif
        default:    /* 未実装テスト番号 */
            g_ulTestNum = 0;                                                        break;
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : _usb_test_AppInit                                                               */
/*                                                                                              */
/* DESCRIPTION: Initialize of thie test application.                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : USB_TEST_OK                     Success                                         */
/*              USB_TEST_NG                     Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_si _usb_test_AppInit(void)
{
    /* ファイルオープンで GRP_FS_O_DIRECT_IO を指定している場合は */
    /* 非キャッシュ領域のアドレスを設定します */
    g_pucTestWrBuf = (grp_u8 *)g_aulTestWrBuf;
    g_pucTestRdBuf = (grp_u8 *)g_aulTestRdBuf;

    /*--- メディア挿抜処理タスクの作成 ---*/
    if (grp_vos_CreateTask( &g_ptConDconTask,
                            _CONDCON_TASK_NAME,
                            _usb_test_ConDconTask,
                            _CONDCON_TASK_STACK,
                            _CONDCON_TASK_PRI,
                            _CONDCON_TASK_STAT, 0) != GRP_VOS_POS_RESULT) {
        /* error */
        return USB_TEST_NG;
    }

    /*--- テストアプリケーション用タスクの作成 ---*/
    if (grp_vos_CreateTask( &g_ptAppTask,
                            _APP_TASK_NAME,
                            _usb_test_AppTask,
                            _APP_TASK_STACK,
                            _APP_TASK_PRI,
                            _APP_TASK_STAT, 0) != GRP_VOS_POS_RESULT) {
        /* error */
        return USB_TEST_NG;
    }

	/* Create queue */
    if (grp_vos_CreateQueue(&l_ptAttachQue, (grp_u8*)"qAttach", 4, 1)) {
        /* error */
        return USB_TEST_NG;
    }
    return USB_TEST_OK;
}

/*----------------------------------------------------------------------------------------------*/
/*  以下の関数はGR-FILEのライブラリをいじりたくないため、ダミーの関数として用意しておく。       */
/*  cons_getcharは通常proc_event内のアプリでしか使われていないので、このデモでは使用しないこと  */
/*  にする。                                                                                    */
/*----------------------------------------------------------------------------------------------*/

/************************************************************************************************/
/* FUNCTION   : cons_putchar                                                                    */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : mode                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
grp_si cons_putchar(grp_si iChar)
{
    return 0;
}

/************************************************************************************************/
/* FUNCTION   : cons_getchar                                                                    */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : mode                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
grp_si cons_getchar(grp_si iMode)
{
    return 0;
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
static T_MSG_BASIC usbhost_app_msg;
extern u8 ex_operating_mode;
void _fusbhost_app_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	//_usbhost_app_send_msg(ID_DISPLAY_MBX, TMSG_DISP_LED_ON, DISP_CTRL_DISPLAY_TEST, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */

#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_USB_HOST_APP_TASK, (u16)code, (u16)usbhost_app_msg.tmsg_code, (u16)usbhost_app_msg.arg1, fatal_err);
}

/*********************************************************************//**
 * @brief send task message
 * @param[in]	receiver task id
 * 				task message code
 * 				argument 1
 * 				argument 2
 * 				argument 3
 * 				argument 4
 * @return 		None
 **********************************************************************/
void _usbhost_app_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_USB_HOST_APP_TASK;
		t_msg->mpf_id = ID_MBX_MPF;
		t_msg->tmsg_code = tmsg_code;
		t_msg->arg1 = arg1;
		t_msg->arg2 = arg2;
		t_msg->arg3 = arg3;
		t_msg->arg4 = arg4;
		ercd = snd_mbx(receiver_id, (T_MSG *)t_msg);
		if (ercd != E_OK)
		{
			/* system error */
			_fusbhost_app_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_fusbhost_app_system_error(1, 2);
	}
}
