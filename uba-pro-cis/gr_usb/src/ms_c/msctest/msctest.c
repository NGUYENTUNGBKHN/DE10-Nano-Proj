/************************************************************************************************/
/*                                                                                              */
/*                           Copyright(C) 2007-2008 Grape Systems, Inc.                         */
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
/*      msctest.c                                                               0.11            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Test for USB/Host Mass Storage Class                                                    */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamakawa     2007/07/02  Version 0.01                                                    */
/*                              Created initial version 0.01                                    */
/*   H.Yamakawa     2007/08/29  Version 0.10                                                    */
/*                              レビュー(2007/08/28)により、以下の項目修正                      */
/*                              ・複数デバイス対応を外した                                      */
/*                              ・各種リクエスト構造体の実体をグローバルに移動                  */
/*                              ・同種のAPIは1つを残して取り除きシンプルにした                  */
/*                              #ifdefでHubを追加できるようにした                               */
/*   K.Takagi       2008/10/08  Version 0.11                                                    */
/*                              構造体のメンバ（コールバック関数名）変更による修正              */
/*                              ・_MscTest_Initialize                                           */
/*                              TestUnitReadyコマンドに対してエラーを返すケースがあるため、以下 */
/*                              の関数内でコマンドを再送するよう修正した。                      */
/*                              ・_MscTest_Command                                              */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_std_types.h"
#include "grp_std_tools.h"

#ifdef MSC_TEST_USE_HUB
#include "grp_hubd.h"
#endif  /* MSC_TEST_USE_HUB */
#include "grp_msc.h"
#include "grp_usbc.h"

#include "msctest.h"

/**** INTERNAL DEFINITIONS **********************************************************************/
/* Error code */
#define MSCTEST_OK              0
#define MSCTEST_NG              -1

/* Flag */
#define MSCTEST_FLG_ON          1
#define MSCTEST_FLG_OFF         0

/* Register device information */
#define TEST_VENDOR_ID          0x0d4b
#define TEST_PRODUCT_ID         0x3203

/* User pointer */
#define TEST_REF                (void *)0x12345678

/* MassStorage command information */
#define TEST_LBA_START               100
#define TEST_SECTOR_SIZE             512
#define TEST_NUM_OF_SECTOR           ( GRP_CMEM_BSIZE_TEST / TEST_SECTOR_SIZE )
#define TEST_INQUIRY_DATA_SIZE       36
#define TEST_RFC_DATA_SIZE           12
#define TEST_RC_DATA_SIZE            8
#define TEST_MODESENSE_DATA_SIZE     4
#define TEST_MODESENSE_PAGE          0x3F
#define TEST_TUR_DATA_SIZE           0
#define TEST_REQSENSE_DATA_SIZE      18

/* Application event flag information */
#define TEST_APLQ_NAME          (grp_u8 *)"TT_AQ"
#define TEST_APLQ_SIZE          4
#define TEST_APLQ_CNT           1

/* Delay time */
#define TEST_TIME_DELAY         100

/* Time out value */
#define TEST_TMOUT_VALUE        1000

/**** INTERNAL VALIABLES ************************************************************************/
/* Connect/Disconnect flag */
static grp_u32                  l_ulConnectFlag = MSCTEST_FLG_OFF;

typedef struct {
    grp_u32                     ulFlag;
    grp_u32                     ulSectorSize;
    grp_u32                     ulNumOfSector;

} test_t_dev_info;

static test_t_dev_info          l_tDevInfo;

/* INQUIRY  Inquiry Command Descriptor Block */
typedef struct  _test_t_sbc_inquiry
{
    grp_u8                  ucCommand;                              /* Operation code (12h)     */
    grp_u8                  ucCmddtEvpd;                            /* CmdDt - EVPD             */
    grp_u8                  ucPageOperation;                        /* Page or Operation code   */
    grp_u8                  ucReserved;                             /* Reserved                 */
    grp_u8                  ucAllocationLen;                        /* Allocation Length        */
    grp_u8                  ucControl;                              /* Control                  */
    grp_u8                  aucPadd[6];                             /* Padding                  */
} test_t_sbc_inquiry;

/* MassStorage handle */
static grp_msc_hdr              l_tMscHdr = NULL;

#ifndef MSC_TEST_NO_INIT
static grp_vos_t_task           *l_ptTsk = 0;
#endif  /* MSC_TEST_NO_INIT */
static grp_vos_t_queue          *l_ptAplQueue;

/* Request information */
static grp_msc_cmd              l_tMscCmd;

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
static int _MscTest_Initialize(void);
static int _MscTest_Open(void);
static int _MscTest_Close(void);
static int _MscTest_Command(void);
static int _MscTest_Command_ReadSector(void);
static int _MscTest_Command_WriteSector(void);
static int _MscTest_Command_Inquiry(void);
static int _MscTest_Command_Inquiry_Scsi(void);
static int _MscTest_Command_ReadFormatCapacity(void);
static int _MscTest_Command_ReadCapacity(void);
static int _MscTest_Command_ModeSense(void);
static int _MscTest_Command_TestUnitReady(void);
static int _MscTest_Command_RequestSense(void);
static grp_s32 _MscTest_ConDis_Callback(grp_msc_notify *ptMscNotify);
static grp_s32 _MscTest_Command_Callback(grp_msc_cmd *ptMscCmd);
static grp_s32 _MscTest_TestInit(void);

/************************************************************************************************/
/* MACRO      : MSCTEST_GET32                                                                   */
/*                                                                                              */
/* DESCRIPTION: Get 32bit data from data array. This data array is in little endian.            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pucArray                        Top address of data array.                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : 32bit data                                                                      */
/*                                                                                              */
/************************************************************************************************/
#define MSCTEST_GET32(pucArray)    (grp_u32)( (((grp_u32)*(pucArray+0))<<24) \
                                            | (((grp_u32)*(pucArray+1))<<16) \
                                            | (((grp_u32)*(pucArray+2))<< 8) \
                                            | (((grp_u32)*(pucArray+3))    ) \
                                            )

/**** MAIN TEST *********************************************************************************/
/************************************************************************************************/
/* FUNCTION   : MscTest_Main                                                                    */
/*                                                                                              */
/* DESCRIPTION: MSC test main function                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void MscTest_Main(void)
{
int iStat;

    /* MSC Initialize Test */
    iStat = _MscTest_Initialize();
    if( iStat != MSCTEST_OK ) {
        while( 1 );
    }

    while( 1 ) {
        /* Wait until device is connected */
        while( l_ulConnectFlag != MSCTEST_FLG_ON ) {
            grp_vos_DelayTask(TEST_TIME_DELAY);
        }

        l_tDevInfo.ulFlag = MSCTEST_FLG_OFF;

        /* MSC Open Test */
        iStat = _MscTest_Open();
        if( iStat != MSCTEST_OK ) {
            while( 1 );
        }

        /* MSC Command Test */
        iStat = _MscTest_Command();
        if( iStat != MSCTEST_OK ) {
            while( 1 );
        }

        /* MSC Close Test */
        iStat = _MscTest_Close();
        if( iStat != MSCTEST_OK ) {
            while( 1 );
        }

        /* Wait until device is connected */
        while( l_ulConnectFlag == MSCTEST_FLG_ON ) {
            grp_vos_DelayTask(TEST_TIME_DELAY);
        }
    }
}

/**** SUB TESTS *********************************************************************************/
/************************************************************************************************/
/* FUNCTION   : _MscTest_Initialize                                                             */
/*                                                                                              */
/* DESCRIPTION: MSC Initialze Test.                                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Initialize(void)
{
grp_msc_reg     tMscReg;
grp_s32         lStat;

#ifndef MSC_TEST_NO_INIT
    /* Initialize GR-VOS */
    lStat = grp_vos_Init();
    if( lStat != GRP_VOS_POS_RESULT ) {
        return 1;
    }

    /* Register own task to GR-VOS */
    lStat = grp_vos_RegisterTask(&l_ptTsk, (grp_u8 *)"MSC_TSK", GRP_VOS_PRI_NORMAL);
    if( lStat != GRP_VOS_POS_RESULT ) {
        return 1;
    }

    /* Initialize GR-USB/Host */
    lStat = grp_usbc_HostInit();
    if( lStat != GRP_USBC_OK ) {
        return 2;
    }

#ifdef MSC_TEST_USE_HUB
    /* Initialize Hub driver */
    lStat = grp_hubd_Init();
    if( lStat != GRP_HUBD_OK ) {
        return 3;
    }
#endif  /* MSC_TEST_USE_HUB */
#endif  /* MSC_TEST_NO_INIT */

    /* Initialize MSC Driver */
    lStat = grp_msc_Init(GRP_USB_NULL);
    if( lStat != GRP_MSC_OK ) {
        return 4;
    }

    /* Register Mass Storage Device Type */
    tMscReg.ucSubClass       = GRP_MSC_SFF8070I_CODE;
    tMscReg.ucProtocol       = GRP_MSC_BOT_CODE;
    tMscReg.pfnMscEvCallback = _MscTest_ConDis_Callback;
    tMscReg.pvUserRef        = GRP_USB_NULL;
    tMscReg.ucMode           = GRP_MSC_REG_PROTOCOL;
    lStat = grp_msc_Register( &tMscReg );
    if( lStat != GRP_MSC_OK ) {
        return 5;
    }

    tMscReg.ucSubClass       = GRP_MSC_SCSI_CODE;
    tMscReg.ucProtocol       = GRP_MSC_BOT_CODE;
    tMscReg.pfnMscEvCallback = _MscTest_ConDis_Callback;
    tMscReg.pvUserRef        = GRP_USB_NULL;
    tMscReg.ucMode           = GRP_MSC_REG_PROTOCOL;
    lStat = grp_msc_Register( &tMscReg );
    if( lStat != GRP_MSC_OK ) {
        return 5;
    }
/*
    tMscReg.ucSubClass       = GRP_MSC_SCSI_CODE;
    tMscReg.ucProtocol       = GRP_MSC_BOT_CODE;
    tMscReg.usVendorId       = TEST_VENDOR_ID;
    tMscReg.usProductId      = TEST_PRODUCT_ID;
    tMscReg.pfnMscEvCallback = _MscTest_ConDis_Callback;
    tMscReg.pvUserRef        = GRP_USB_NULL;
    tMscReg.ulMode           = GRP_MSC_REG_VENDOR;
    lStat = grp_msc_Register( &tMscReg );
    if( lStat != GRP_MSC_OK ) {
        return 5;
    }
*/
    /* Initialize test */
    lStat = _MscTest_TestInit();
    if( lStat != MSCTEST_OK ) {
        return 6;
    }

    /* Enable GR-USB/Host interrupt */
    lStat = grp_usbc_Enable();
    if( lStat != GRP_USBC_OK ) {
        return 7;
    }

    return MSCTEST_OK;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Open                                                                   */
/*                                                                                              */
/* DESCRIPTION: MSC Open Test.                                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Open(void)
{
grp_s32         lStat;

    l_tMscCmd.hMscHdr = l_tMscHdr;

    /* Open Device */
    lStat = grp_msc_Open(&l_tMscCmd);
    if( lStat != GRP_MSC_OK ) {
        return 1;
    }

    return MSCTEST_OK;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Close                                                                  */
/*                                                                                              */
/* DESCRIPTION: MSC Close Test.                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Close(void)
{
grp_s32         lStat;

    l_tMscCmd.hMscHdr = l_tMscHdr;

    /* Close Device */
    lStat = grp_msc_Close(&l_tMscCmd);
    if( lStat != GRP_MSC_OK ) {
        return 1;
    }

    return MSCTEST_OK;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Command                                                                */
/*                                                                                              */
/* DESCRIPTION: MSC Command Test.                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Command(void)
{
int iStat;
int iErrCode = MSCTEST_OK;
int i;

    for (i=0; i<3; i++) {

        /* Inquiry */
        iStat = _MscTest_Command_Inquiry();
        if( iStat != MSCTEST_OK ) {
            iErrCode = 1;
            continue;
        }

        /* Test Unit Ready */
        iStat = _MscTest_Command_TestUnitReady();
        if( iStat != MSCTEST_OK ) {
            iErrCode = 2;
            continue;
        }

        /* Read Capacity */
        iStat = _MscTest_Command_ReadCapacity();
        if( iStat != MSCTEST_OK ) {
            iErrCode = 3;
            continue;
        }

        /* Read Format Capacity */
        iStat = _MscTest_Command_ReadFormatCapacity();
        if( iStat != MSCTEST_OK ) {
            iErrCode = 4;
            continue;
        }

        /* Request Sense */
        iStat = _MscTest_Command_RequestSense();
        if( iStat != MSCTEST_OK ) {
            iErrCode = 5;
            continue;
        }

        /* Mode Sense */
        iStat = _MscTest_Command_ModeSense();
        if( iStat != MSCTEST_OK ) {
            iErrCode = 6;
            continue;
        }

        /* Read Sector */
        iStat = _MscTest_Command_ReadSector();
        if( iStat != MSCTEST_OK ) {
            iErrCode = 7;
            continue;
        }

        /* Write Sector */
        iStat = _MscTest_Command_WriteSector();
        if( iStat != MSCTEST_OK ) {
            iErrCode = 8;
            continue;
        }

        /* success */
        iErrCode = MSCTEST_OK;
        break;
    }

    return iErrCode;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Command_ReadSector                                                     */
/*                                                                                              */
/* DESCRIPTION: MSC Command Test (Read Sector).                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Command_ReadSector(void)
{
grp_s32         lRetStat = MSCTEST_OK;
grp_s32         lStat;
grp_msc_cmd     *ptMscCmd;
grp_u8          *pucBuf;
grp_u32         ulNumOfSector;
grp_u32         ulSectorSize;

    if( l_tDevInfo.ulFlag == MSCTEST_FLG_ON ) {
        ulSectorSize  = l_tDevInfo.ulSectorSize;
        if( l_tDevInfo.ulNumOfSector <= TEST_LBA_START ) {
            return 5;
        }
        else if( l_tDevInfo.ulNumOfSector < ( TEST_LBA_START + ( GRP_CMEM_BSIZE_TEST / ulSectorSize ) ) ) {
            ulNumOfSector = ( l_tDevInfo.ulNumOfSector - TEST_LBA_START );
        }
        else {
            ulNumOfSector = ( GRP_CMEM_BSIZE_TEST / ulSectorSize );
        }
    }
    else {
        ulNumOfSector = TEST_NUM_OF_SECTOR;
        ulSectorSize  = TEST_SECTOR_SIZE;
    }

    lStat = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&pucBuf);
    if( lStat != GRP_USBD_OK ) {
        return 6;
    }

    while( GRP_VOS_POS_RESULT == grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, GRP_VOS_NOWAIT) );

    l_tMscCmd.hMscHdr       = l_tMscHdr;
    l_tMscCmd.ucLun         = 0;
    l_tMscCmd.pucReqBuffer  = pucBuf;
    l_tMscCmd.ulReqLength   = (ulNumOfSector * ulSectorSize);
    l_tMscCmd.pfnCallback   = _MscTest_Command_Callback;
    l_tMscCmd.pvUserRef     = TEST_REF;
    /* Make Command Content */
    lStat = grp_msc_ReadSector(&l_tMscCmd, TEST_LBA_START, ulNumOfSector);
    if( lStat == GRP_MSC_OK ) {
        /* Wait notification of completion from callback */
        lStat = grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, TEST_TMOUT_VALUE);
        if( lStat == GRP_VOS_POS_RESULT ) {
            if( ptMscCmd->lStatus == GRP_MSC_NO_FAIL 
             && ptMscCmd->ulActualLength == l_tMscCmd.ulReqLength
             && ptMscCmd->pvUserRef == l_tMscCmd.pvUserRef ) {
                /* OK */
            }
            else {
                lRetStat = 1;
            }
        }
        else if( lStat == GRP_VOS_NEG_NO_MESSAGE ) {
            /* Cancel request */
            grp_msc_Cancel(&l_tMscCmd);
            lRetStat = 2;
        }
        else {
            lRetStat = 3;
        }
    }
    else {
        lRetStat = 4;
    }

    grp_cmem_BlkRel(pucBuf);

    return lRetStat;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Command_WriteSector                                                    */
/*                                                                                              */
/* DESCRIPTION: MSC Command Test (Write Sector).                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Command_WriteSector(void)
{
grp_s32         lRetStat = MSCTEST_OK;
grp_s32         lStat;
grp_msc_cmd     *ptMscCmd;
grp_u8          *pucBuf;
grp_u32         ulNumOfSector;
grp_u32         ulSectorSize;

    if( l_tDevInfo.ulFlag == MSCTEST_FLG_ON ) {
        ulSectorSize  = l_tDevInfo.ulSectorSize;
        if( l_tDevInfo.ulNumOfSector <= TEST_LBA_START ) {
            return 5;
        }
        else if( l_tDevInfo.ulNumOfSector < ( TEST_LBA_START + ( GRP_CMEM_BSIZE_TEST / ulSectorSize ) ) ) {
            ulNumOfSector = ( l_tDevInfo.ulNumOfSector - TEST_LBA_START );
        }
        else {
            ulNumOfSector = ( GRP_CMEM_BSIZE_TEST / ulSectorSize );
        }
    }
    else {
        ulNumOfSector = TEST_NUM_OF_SECTOR;
        ulSectorSize  = TEST_SECTOR_SIZE;
    }

    lStat = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&pucBuf);
    if( lStat != GRP_USBD_OK ) {
        return 6;
    }

    while( GRP_VOS_POS_RESULT == grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, GRP_VOS_NOWAIT) );

    l_tMscCmd.hMscHdr       = l_tMscHdr;
    l_tMscCmd.ucLun         = 0;
    l_tMscCmd.pucReqBuffer  = pucBuf;
    l_tMscCmd.ulReqLength   = (ulNumOfSector * ulSectorSize);
    l_tMscCmd.pfnCallback   = _MscTest_Command_Callback;
    l_tMscCmd.pvUserRef     = TEST_REF;
    /* Make Command Content */
    lStat = grp_msc_WriteSector(&l_tMscCmd, TEST_LBA_START, ulNumOfSector);
    if( lStat == GRP_MSC_OK ) {
        /* Wait notification of completion from callback */
        lStat = grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, TEST_TMOUT_VALUE);
        if( lStat == GRP_VOS_POS_RESULT ) {
            if( ptMscCmd->lStatus == GRP_MSC_NO_FAIL 
             && ptMscCmd->ulActualLength == l_tMscCmd.ulReqLength
             && ptMscCmd->pvUserRef == l_tMscCmd.pvUserRef ) {
                /* OK */
            }
            else {
                lRetStat = 1;
            }
        }
        else if( lStat == GRP_VOS_NEG_NO_MESSAGE ) {
            /* Cancel request */
            grp_msc_Cancel(&l_tMscCmd);
            lRetStat = 2;
        }
        else {
            lRetStat = 3;
        }
    }
    else {
        lRetStat = 4;
    }

    grp_cmem_BlkRel(pucBuf);

    return lRetStat;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Command_Inquiry                                                        */
/*                                                                                              */
/* DESCRIPTION: MSC Command Test (Inquiry).                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Command_Inquiry(void)
{
grp_s32         lRetStat = MSCTEST_OK;
grp_s32         lStat;
grp_msc_cmd     *ptMscCmd;
grp_u8          *pucBuf;

    lStat = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&pucBuf);
    if( lStat != GRP_USBD_OK ) {
        return 5;
    }

    while( GRP_VOS_POS_RESULT == grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, GRP_VOS_NOWAIT) );

    l_tMscCmd.hMscHdr       = l_tMscHdr;
    l_tMscCmd.ucLun         = 0;
    l_tMscCmd.pucReqBuffer  = pucBuf;
    l_tMscCmd.ulReqLength   = TEST_INQUIRY_DATA_SIZE;
    l_tMscCmd.pfnCallback   = _MscTest_Command_Callback;
    l_tMscCmd.pvUserRef     = TEST_REF;
    /* Make Command Content */
    lStat = grp_msc_Inquiry(&l_tMscCmd);
    if( lStat == GRP_MSC_OK ) {
        /* Wait notification of completion from callback */
        lStat = grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, TEST_TMOUT_VALUE);
        if( lStat == GRP_VOS_POS_RESULT ) {
            if( ptMscCmd->lStatus == GRP_MSC_NO_FAIL 
             && ptMscCmd->ulActualLength == l_tMscCmd.ulReqLength
             && ptMscCmd->pvUserRef == l_tMscCmd.pvUserRef ) {
                /* OK */
            }
            else {
                lRetStat = 1;
            }
        }
        else if( lStat == GRP_VOS_NEG_NO_MESSAGE ) {
            /* Cancel request */
            grp_msc_Cancel(&l_tMscCmd);
            lRetStat = 2;
        }
        else {
            lRetStat = 3;
        }
    }
    else {
        lRetStat = 4;
    }

    grp_cmem_BlkRel(pucBuf);

    return lRetStat;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Command_Inquiry_Scsi                                                   */
/*                                                                                              */
/* DESCRIPTION: MSC Command Test (Inquiry).                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Command_Inquiry_Scsi(void)
{
test_t_sbc_inquiry  *pucStr;
grp_s32             lRetStat = MSCTEST_OK;
grp_s32             lStat;
grp_msc_cmd         *ptMscCmd;
grp_u8              *pucBuf;

    lStat = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&pucBuf);
    if( lStat != GRP_USBD_OK ) {
        return 5;
    }

    while( GRP_VOS_POS_RESULT == grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, GRP_VOS_NOWAIT) );

    l_tMscCmd.hMscHdr       = l_tMscHdr;
    l_tMscCmd.ucLun         = 0;
    l_tMscCmd.pucReqBuffer  = pucBuf;
    l_tMscCmd.ulReqLength   = TEST_INQUIRY_DATA_SIZE;
    l_tMscCmd.pfnCallback   = _MscTest_Command_Callback;
    l_tMscCmd.pvUserRef     = TEST_REF;
    l_tMscCmd.ucDir         = GRP_USBD_TX_IN;
    l_tMscCmd.ulCmdLength   = 6; /* INQUIRY command size */
    /* Make Command Content */
    pucStr = (test_t_sbc_inquiry *)l_tMscCmd.aucCmdContent;
    grp_std_memset( pucStr, 0, GRP_MSC_CMD_LENGTH );
    pucStr->ucCommand = 0x12; /* INQUIRY command id */
    pucStr->ucAllocationLen = (grp_u8)TEST_INQUIRY_DATA_SIZE;
    pucStr->ucCmddtEvpd = 0;
    pucStr->ucPageOperation = 0;
    pucStr->ucControl = 0;

    /* Request Command */
    lStat = grp_msc_ReqCmd(&l_tMscCmd);
    if( lStat == GRP_MSC_OK ) {
        /* Wait notification of completion from callback */
        lStat = grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, TEST_TMOUT_VALUE);
        if( lStat == GRP_VOS_POS_RESULT ) {
            if( ptMscCmd->lStatus == GRP_MSC_NO_FAIL 
             && ptMscCmd->ulActualLength == l_tMscCmd.ulReqLength
             && ptMscCmd->pvUserRef == l_tMscCmd.pvUserRef ) {
                /* OK */
            }
            else {
                lRetStat = 1;
            }
        }
        else if( lStat == GRP_VOS_NEG_NO_MESSAGE ) {
            /* Cancel request */
            grp_msc_Cancel(&l_tMscCmd);
            lRetStat = 2;
        }
        else {
            lRetStat = 3;
        }
    }
    else {
        lRetStat = 4;
    }

    grp_cmem_BlkRel(pucBuf);

    return lRetStat;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Command_ReadFormatCapacity                                             */
/*                                                                                              */
/* DESCRIPTION: MSC Command Test (Read Format Capacity).                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Command_ReadFormatCapacity(void)
{
grp_s32         lRetStat = MSCTEST_OK;
grp_s32         lStat;
grp_msc_cmd     *ptMscCmd;
grp_u8          *pucBuf;

    lStat = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&pucBuf);
    if( lStat != GRP_USBD_OK ) {
        return 5;
    }

    while( GRP_VOS_POS_RESULT == grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, GRP_VOS_NOWAIT) );

    l_tMscCmd.hMscHdr       = l_tMscHdr;
    l_tMscCmd.ucLun         = 0;
    l_tMscCmd.pucReqBuffer  = pucBuf;
    l_tMscCmd.ulReqLength   = TEST_RFC_DATA_SIZE;
    l_tMscCmd.pfnCallback   = _MscTest_Command_Callback;
    l_tMscCmd.pvUserRef     = TEST_REF;
    /* Make Command Content */
    lStat = grp_msc_ReadFormatCapacity(&l_tMscCmd);
    if( lStat == GRP_MSC_OK ) {
        /* Wait notification of completion from callback */
        lStat = grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, TEST_TMOUT_VALUE);
        if( lStat == GRP_VOS_POS_RESULT ) {
            if( ptMscCmd->lStatus == GRP_MSC_NO_FAIL 
             && ptMscCmd->ulActualLength == TEST_RFC_DATA_SIZE
             && ptMscCmd->pvUserRef == l_tMscCmd.pvUserRef ) {
                /* OK */
            }
            else {
                lRetStat = 1;
            }
        }
        else if( lStat == GRP_VOS_NEG_NO_MESSAGE ) {
            /* Cancel request */
            grp_msc_Cancel(&l_tMscCmd);
            lRetStat = 2;
        }
        else {
            lRetStat = 3;
        }
    }
    else {
        lRetStat = 4;
    }

    grp_cmem_BlkRel(pucBuf);

    return lRetStat;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Command_ReadCapacity                                                   */
/*                                                                                              */
/* DESCRIPTION: MSC Command Test (Read Capacity).                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Command_ReadCapacity(void)
{
grp_s32         lRetStat = MSCTEST_OK;
grp_s32         lStat;
grp_msc_cmd     *ptMscCmd;
grp_u8          *pucBuf;

    lStat = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&pucBuf);
    if( lStat != GRP_USBD_OK ) {
        return 5;
    }

    while( GRP_VOS_POS_RESULT == grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, GRP_VOS_NOWAIT) );

    l_tMscCmd.hMscHdr       = l_tMscHdr;
    l_tMscCmd.ucLun         = 0;
    l_tMscCmd.pucReqBuffer  = pucBuf;
    l_tMscCmd.ulReqLength   = TEST_RC_DATA_SIZE;
    l_tMscCmd.pfnCallback   = _MscTest_Command_Callback;
    l_tMscCmd.pvUserRef     = TEST_REF;
    /* Make Command Content */
    lStat = grp_msc_ReadCapacity(&l_tMscCmd);
    if( lStat == GRP_MSC_OK ) {
        /* Wait notification of completion from callback */
        lStat = grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, TEST_TMOUT_VALUE);
        if( lStat == GRP_VOS_POS_RESULT ) {
            if( ptMscCmd->lStatus == GRP_MSC_NO_FAIL 
             && ptMscCmd->ulActualLength == l_tMscCmd.ulReqLength
             && ptMscCmd->pvUserRef == l_tMscCmd.pvUserRef ) {
                /* OK */
                l_tDevInfo.ulFlag        = MSCTEST_FLG_ON;
                l_tDevInfo.ulNumOfSector = MSCTEST_GET32(pucBuf) + 1;
                l_tDevInfo.ulSectorSize  = MSCTEST_GET32(pucBuf + 4);
            }
            else {
                lRetStat = 1;
            }
        }
        else if( lStat == GRP_VOS_NEG_NO_MESSAGE ) {
            /* Cancel request */
            grp_msc_Cancel(&l_tMscCmd);
            lRetStat = 2;
        }
        else {
            lRetStat = 3;
        }
    }
    else {
        lRetStat = 4;
    }

    grp_cmem_BlkRel(pucBuf);

    return lRetStat;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Command_ModeSense                                                      */
/*                                                                                              */
/* DESCRIPTION: MSC Command Test (Mode Sense).                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Command_ModeSense(void)
{
grp_s32         lRetStat = MSCTEST_OK;
grp_s32         lStat;
grp_msc_cmd     *ptMscCmd;
grp_u8          *pucBuf;

    lStat = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&pucBuf);
    if( lStat != GRP_USBD_OK ) {
        return 5;
    }

    while( GRP_VOS_POS_RESULT == grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, GRP_VOS_NOWAIT) );

    l_tMscCmd.hMscHdr       = l_tMscHdr;
    l_tMscCmd.ucLun         = 0;
    l_tMscCmd.pucReqBuffer  = pucBuf;
    l_tMscCmd.ulReqLength   = TEST_MODESENSE_DATA_SIZE;
    l_tMscCmd.pfnCallback   = _MscTest_Command_Callback;
    l_tMscCmd.pvUserRef     = TEST_REF;
    /* Make Command Content */
    lStat = grp_msc_ModeSense(&l_tMscCmd, TEST_MODESENSE_PAGE);
    if( lStat == GRP_MSC_OK ) {
        /* Wait notification of completion from callback */
        lStat = grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, TEST_TMOUT_VALUE);
        if( lStat == GRP_VOS_POS_RESULT ) {
            if( ptMscCmd->lStatus == GRP_MSC_NO_FAIL 
             && ptMscCmd->ulActualLength == TEST_MODESENSE_DATA_SIZE
             && ptMscCmd->pvUserRef == l_tMscCmd.pvUserRef ) {
                /* OK */
            }
            else {
                lRetStat = 1;
            }
        }
        else if( lStat == GRP_VOS_NEG_NO_MESSAGE ) {
            /* Cancel request */
            grp_msc_Cancel(&l_tMscCmd);
            lRetStat = 2;
        }
        else {
            lRetStat = 3;
        }
    }
    else {
        lRetStat = 4;
    }

    grp_cmem_BlkRel(pucBuf);

    return lRetStat;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Command_TestUnitReady                                                  */
/*                                                                                              */
/* DESCRIPTION: MSC Command Test (Test Unit Ready).                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Command_TestUnitReady(void)
{
grp_s32         lRetStat = MSCTEST_OK;
grp_s32         lStat;
grp_msc_cmd     *ptMscCmd;
grp_u8          *pucBuf;

    lStat = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&pucBuf);
    if( lStat != GRP_USBD_OK ) {
        return 5;
    }

    while( GRP_VOS_POS_RESULT == grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, GRP_VOS_NOWAIT) );

    l_tMscCmd.hMscHdr       = l_tMscHdr;
    l_tMscCmd.ucLun         = 0;
    l_tMscCmd.pucReqBuffer  = pucBuf;
    l_tMscCmd.ulReqLength   = TEST_TUR_DATA_SIZE;
    l_tMscCmd.pfnCallback   = _MscTest_Command_Callback;
    l_tMscCmd.pvUserRef     = TEST_REF;
    /* Make Command Content */
    lStat = grp_msc_TestUnitReady(&l_tMscCmd);
    if( lStat == GRP_MSC_OK ) {
        /* Wait notification of completion from callback */
        lStat = grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, TEST_TMOUT_VALUE);
        if( lStat == GRP_VOS_POS_RESULT ) {
            if( ptMscCmd->lStatus == GRP_MSC_NO_FAIL 
             && ptMscCmd->ulActualLength == l_tMscCmd.ulReqLength
             && ptMscCmd->pvUserRef == l_tMscCmd.pvUserRef ) {
                /* OK */
            }
            else {
                lRetStat = 1;
            }
        }
        else if( lStat == GRP_VOS_NEG_NO_MESSAGE ) {
            /* Cancel request */
            grp_msc_Cancel(&l_tMscCmd);
            lRetStat = 2;
        }
        else {
            lRetStat = 3;
        }
    }
    else {
        lRetStat = 4;
    }

    grp_cmem_BlkRel(pucBuf);

    return lRetStat;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Command_RequestSense                                                   */
/*                                                                                              */
/* DESCRIPTION: MSC Command Test (Request Sense).                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static int _MscTest_Command_RequestSense(void)
{
grp_s32         lRetStat = MSCTEST_OK;
grp_s32         lStat;
grp_msc_cmd     *ptMscCmd;
grp_u8          *pucBuf;

    lStat = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&pucBuf);
    if( lStat != GRP_USBD_OK ) {
        return 5;
    }

    while( GRP_VOS_POS_RESULT == grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, GRP_VOS_NOWAIT) );

    l_tMscCmd.hMscHdr       = l_tMscHdr;
    l_tMscCmd.ucLun         = 0;
    l_tMscCmd.pucReqBuffer  = pucBuf;
    l_tMscCmd.ulReqLength   = TEST_REQSENSE_DATA_SIZE;
    l_tMscCmd.pfnCallback   = _MscTest_Command_Callback;
    l_tMscCmd.pvUserRef     = TEST_REF;
    /* Make Command Content */
    lStat = grp_msc_RequestSense(&l_tMscCmd);
    if( lStat == GRP_MSC_OK ) {
        /* Wait notification of completion from callback */
        lStat = grp_vos_ReceiveQueue(l_ptAplQueue, &ptMscCmd, TEST_TMOUT_VALUE);
        if( lStat == GRP_VOS_POS_RESULT ) {
            if( ptMscCmd->lStatus == GRP_MSC_NO_FAIL 
             && ptMscCmd->ulActualLength == l_tMscCmd.ulReqLength
             && ptMscCmd->pvUserRef == l_tMscCmd.pvUserRef ) {
                /* OK */
            }
            else {
                lRetStat = 1;
            }
        }
        else if( lStat == GRP_VOS_NEG_NO_MESSAGE ) {
            /* Cancel request */
            grp_msc_Cancel(&l_tMscCmd);
            lRetStat = 2;
        }
        else {
            lRetStat = 3;
        }
    }
    else {
        lRetStat = 4;
    }

    grp_cmem_BlkRel(pucBuf);

    return lRetStat;
}

/**** CALLBACK FUNCTIONS ************************************************************************/
/************************************************************************************************/
/* FUNCTION   : _MscTest_ConDis_Callback                                                        */
/*                                                                                              */
/* DESCRIPTION: MSC Connect Disconnect Callback function.                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMscNotify                     Device information                              */
/*                                                                                              */
/*               (Member used)                                                                  */
/*              ucSubClass                      Sub class                                       */
/*              ucProtocol                      Protocol                                        */
/*              pvUserRef                       User pointer                                    */
/*              hMscHdr                         MassStorage handle                              */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_MSC_OK                      Success                                         */
/*                                                                                              */
/************************************************************************************************/
static grp_s32 _MscTest_ConDis_Callback(grp_msc_notify *ptMscNotify)
{
    if( ptMscNotify->iEvent == GRP_MSC_ATTACHED )
    {
        l_tMscHdr = ptMscNotify->hMscHdr;
        l_ulConnectFlag = MSCTEST_FLG_ON;
    }
    else if( ptMscNotify->iEvent == GRP_MSC_DETACHED )
    {
        l_ulConnectFlag = MSCTEST_FLG_OFF;
    }

    return GRP_MSC_OK;
}

/************************************************************************************************/
/* FUNCTION   : _MscTest_Command_Callback                                                       */
/*                                                                                              */
/* DESCRIPTION: MSC Command Test Callback function.                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptMscCmd                        Command information                             */
/*                                                                                              */
/*               (Member used)                                                                  */
/*              lStatus                         Communication status                            */
/*              ulActualLength                  Actual transferred data length                  */
/*              pvUserRef                       User pointer                                    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_MSC_OK                      Success                                         */
/*                                                                                              */
/************************************************************************************************/
static grp_s32 _MscTest_Command_Callback(grp_msc_cmd *ptMscCmd)
{
    grp_vos_SendQueue(l_ptAplQueue, (void *)&ptMscCmd, GRP_VOS_INFINITE);

    return GRP_MSC_OK;
}

/**** LOCAL FUNCTIONS ***************************************************************************/
/************************************************************************************************/
/* FUNCTION   : _MscTest_TestInit                                                               */
/*                                                                                              */
/* DESCRIPTION: MSC Test Initialze.                                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : MSCTEST_OK                      Success                                         */
/*              MSCTEST_NG                      Fail                                            */
/*                                                                                              */
/************************************************************************************************/
static grp_s32 _MscTest_TestInit(void)
{
grp_s32         lStat = MSCTEST_OK;

    /* Create queue to notify completion from callback */
    lStat = grp_vos_CreateQueue(&l_ptAplQueue, TEST_APLQ_NAME, TEST_APLQ_SIZE, TEST_APLQ_CNT);
    if( lStat != GRP_VOS_POS_RESULT )
    {
        lStat = MSCTEST_NG;
    }

    return lStat;
}

