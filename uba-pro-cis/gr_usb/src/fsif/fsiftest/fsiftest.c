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
/*      fsiftest.c                                                              0.11            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Test for FSIF                                                                           */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamakawa     2007/08/22  Version 0.01                                                    */
/*                              Created initial version 0.01                                    */
/*   H.Yamakawa     2007/09/04  Version 0.10                                                    */
/*                              レビュー(2007/08/28)により、以下の項目修正                      */
/*                              ・複数デバイス対応を外した                                      */
/*                              ・各種リクエスト構造体の実体をグローバルに移動                  */
/*                              ・同種のAPIは1つを残して取り除きシンプルにした                  */
/*   K.Takagi       2008/10/08  Version 0.11                                                    */
/*                              構造体のメンバ（コールバック関数名）変更による修正              */
/*                              ・_FsifTest_Initialize                                          */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "grp_std_types.h"
#include "grp_std_tools.h"

#include "grp_fsif.h"
#include "grp_msc.h"
#ifdef FSIF_TEST_USE_HUB
#include "grp_hubd.h"
#endif  /* FSIF_TEST_USE_HUB */
#include "grp_usbc.h"
#include "grp_usr_mem.h"

#include "fsiftest.h"

/**** INTERNAL DEFINITIONS **********************************************************************/
/* Error code */
#define FSIFTEST_OK             0
#define FSIFTEST_NG             -1

/* Flag */
#define FSIFTEST_FLG_ON         1
#define FSIFTEST_FLG_OFF        0

/* Delay time */
#define TEST_TIME_DELAY         100

/* Data buffer information */
#define TEST_BUF_SIZE           GRP_CMEM_BSIZE_TEST
#define TEST_START_SECNUM       100

/**** INTERNAL VALIABLES ************************************************************************/
/* Connect/Disconnect flag */
static grp_u32                  l_ulConnectFlag = FSIFTEST_FLG_OFF;

typedef struct {
    grp_u16                     usInfoStat;
    grp_u16                     usPad;
    grp_si                      iDevId;
    void                        *pvHdr;
    grp_u32                     ulSectorSize;
    grp_u32                     ulSectorNum;

} test_t_dev_info;

static test_t_dev_info          l_tDevInfo;

#ifndef MSC_TEST_NO_INIT
static grp_vos_t_task           *l_ptTsk;
#endif  /* MSC_TEST_NO_INIT */

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
static grp_s32 _FsifTest_Initialize(void);
static grp_s32 _FsifTest_GetMediaInfo(void);
static grp_s32 _FsifTest_ReadWrite(void);
static void _FsifTest_EventNotice_Callback(grp_s32 ulEvnt, void *pvHdr, grp_si iIdx);
static void _FsifTest_MakeData(grp_u8 *pucData, grp_u32 ulDataLen);
static void _FsifTest_ClearData(grp_u8 *pucData, grp_u32 ulDataLen);
static grp_s32 _FsifTest_CheckData(grp_u8 *pucData, grp_u32 ulDataLen);

/**** MAIN TEST *********************************************************************************/
/************************************************************************************************/
/* FUNCTION   : FsifTest_Main                                                                   */
/*                                                                                              */
/* DESCRIPTION: FSIF test main function                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void FsifTest_Main(void)
{
grp_s32         lStat;

    /* FSIF Initialize Test */
    lStat = _FsifTest_Initialize();
    if( lStat != FSIFTEST_OK ) {
        while( 1 );
    }

    while( 1 ) {
        /* Wait until device is connected */
        while( l_ulConnectFlag != FSIFTEST_FLG_ON ) {
            grp_vos_DelayTask(TEST_TIME_DELAY);
        }

        /* FSIF GetMediaInfo Test */
        lStat = _FsifTest_GetMediaInfo();
        if( lStat != FSIFTEST_OK ) {
            while( 1 );
        }

        /* FSIF Read/Write Test */
        lStat = _FsifTest_ReadWrite();
        if( lStat != FSIFTEST_OK ) {
            while( 1 );
        }

        /* Wait until device is connected */
        while( l_ulConnectFlag == FSIFTEST_FLG_ON ) {
            grp_vos_DelayTask(TEST_TIME_DELAY);
        }
    }
}

/**** SUB TESTS *********************************************************************************/
/************************************************************************************************/
/* FUNCTION   : _FsifTest_Initialize                                                            */
/*                                                                                              */
/* DESCRIPTION: FSIF Initialze Test.                                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : FSIFTEST_OK                     Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static grp_s32 _FsifTest_Initialize(void)
{
grp_s32             lStat;
grp_fsif_init_prm   tInitPrm;

#ifndef FSIF_TEST_NO_INIT
    /* Initialize GR-VOS */
    lStat = grp_vos_Init();
    if( lStat != GRP_VOS_POS_RESULT )
    {
        return 1;
    }

    /* Register own task to GR-VOS */
    lStat = grp_vos_RegisterTask(&l_ptTsk, (grp_u8 *)"TEST_TSK", GRP_VOS_PRI_NORMAL);
    if( lStat != GRP_VOS_POS_RESULT )
    {
        return 1;
    }

    /* Initialize GR-USB/Host */
    lStat = grp_usbc_HostInit();
    if( lStat != GRP_USBC_OK )
    {
        return 2;
    }

#ifdef FSIF_TEST_USE_HUB
    /* Initialize Hub driver */
    lStat = grp_hubd_Init();
    if( lStat != GRP_HUBD_OK )
    {
        return 3;
    }
#endif  /* FSIF_TEST_USE_HUB */
#endif  /* FSIF_TEST_NO_INIT */

    /* FSIF Initialize */
    tInitPrm.pfnFsifNotification = _FsifTest_EventNotice_Callback;
    lStat = grp_fsif_Init(&tInitPrm);
    if( lStat != GRP_FSIF_OK )
    {
        return 4;
    }

    /* Enable GR-USB/Host */
    lStat = grp_usbc_Enable();
    if( lStat != GRP_USBC_OK )
    {
        return 5;
    }

    return FSIFTEST_OK;
}

/************************************************************************************************/
/* FUNCTION   : _FsifTest_GetMediaInfo                                                          */
/*                                                                                              */
/* DESCRIPTION: FSIF GetMediaInfo Test.                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : FSIFTEST_OK                     Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static grp_s32 _FsifTest_GetMediaInfo(void)
{
grp_s32                 lStat;
grp_fsif_media_info     tMediaInfo;

    /* Get Media Info */
    lStat = grp_fsif_GetMediaInfo(l_tDevInfo.pvHdr, &tMediaInfo);
    if( lStat == GRP_FSIF_OK )
    {
        if( ( tMediaInfo.ulSectorSize % 512 ) == 0 
         && tMediaInfo.ulSectorNum > 1 )
        {
            /* OK */
            l_tDevInfo.ulSectorSize = tMediaInfo.ulSectorSize;
            l_tDevInfo.ulSectorNum  = tMediaInfo.ulSectorNum;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 2;
    }

    return FSIFTEST_OK;
}

/************************************************************************************************/
/* FUNCTION   : _FsifTest_ReadWrite                                                             */
/*                                                                                              */
/* DESCRIPTION: FSIF Read/Write Test.                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : FSIFTEST_OK                     Success                                         */
/*              Other                           Error number                                    */
/*                                                                                              */
/************************************************************************************************/
static grp_s32 _FsifTest_ReadWrite(void)
{
grp_s32             lStat;
grp_s32             lRetStat = FSIFTEST_OK;
grp_u32             ulSecCnt;
grp_u32             ulDataLen;
grp_u8              *pucSaveDataBuf;
grp_u8              *pucTestDataBuf;

    /* Check start sector number */
    if( l_tDevInfo.ulSectorNum <= TEST_START_SECNUM )
    {   /* Start sector number is out of range */
        return 6;
    }

    /* Set sector count and data length */
    ulSecCnt = ( ( TEST_BUF_SIZE / l_tDevInfo.ulSectorSize ) <= ( l_tDevInfo.ulSectorNum - TEST_START_SECNUM ) )
                        ? ( TEST_BUF_SIZE / l_tDevInfo.ulSectorSize )
                        : ( l_tDevInfo.ulSectorNum - TEST_START_SECNUM );
    ulDataLen = l_tDevInfo.ulSectorSize * ulSecCnt;

    /* Get data buffer for save */
    lStat = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&pucSaveDataBuf);
    if( lStat == GRP_CMEM_OK )
    {
        /* Read data */
        lStat = grp_fsif_ReadSector(l_tDevInfo.pvHdr, TEST_START_SECNUM, ulSecCnt, pucSaveDataBuf);
        if( lStat == GRP_FSIF_OK )
        {
            /* Get data buffer for test */
            lStat = grp_cmem_BlkGet(GRP_CMEM_ID_TEST, (void **)&pucTestDataBuf);
            if( lStat == GRP_CMEM_OK )
            {
                /* Make write data */
                _FsifTest_MakeData(pucTestDataBuf, ulDataLen);

                /* Write data */
                lStat = grp_fsif_WriteSector(l_tDevInfo.pvHdr, TEST_START_SECNUM, ulSecCnt, pucTestDataBuf);
                if( lStat == GRP_FSIF_OK )
                {
                    /* Clear data buffer */
                    _FsifTest_ClearData(pucTestDataBuf, ulDataLen);

                    /* Read data */
                    lStat = grp_fsif_ReadSector(l_tDevInfo.pvHdr, TEST_START_SECNUM, ulSecCnt, pucTestDataBuf);
                    if( lStat == GRP_FSIF_OK )
                    {
                        /* Compare between read data and write data */
                        lStat = _FsifTest_CheckData(pucTestDataBuf, ulDataLen);
                        if( lStat == FSIFTEST_OK )
                        {
                            /* Write saved data */
                            lStat = grp_fsif_WriteSector(l_tDevInfo.pvHdr, TEST_START_SECNUM, ulSecCnt, pucSaveDataBuf);
                            if( lStat == GRP_FSIF_OK )
                            {
                                /* OK */
                            }
                            else
                            {
                                lRetStat =  1;
                            }
                        }
                        else
                        {
                            lRetStat =  2;
                        }
                    }
                    else
                    {
                        lRetStat =  3;
                    }
                }
                else
                {
                    lRetStat =  4;
                }

                grp_cmem_BlkRel(pucTestDataBuf);
            }
            else
            {
                lRetStat =  5;
            }
        }
        else
        {
            lRetStat =  6;
        }

        grp_cmem_BlkRel(pucSaveDataBuf);
    }
    else
    {
        lRetStat =  7;
    }

    return lRetStat;
}

/**** CALLBACK FUNCTIONS ************************************************************************/
/************************************************************************************************/
/* FUNCTION   : _FsifTest_EventNotice_Callback                                                  */
/*                                                                                              */
/* DESCRIPTION: FSIF Event Notice Callback function.                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulEvnt                          Event                                           */
/*              pvHdr                           Media handle                                    */
/*              iIdx                            Index                                           */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
static void _FsifTest_EventNotice_Callback(grp_s32 ulEvnt, void *pvHdr, grp_si iIdx)
{
    if( ulEvnt == GRP_FSIF_ATTACHED_MEDIA )
    {   /* connect process */
        l_tDevInfo.iDevId = iIdx;
        l_tDevInfo.pvHdr  = pvHdr;
        l_ulConnectFlag   = FSIFTEST_FLG_ON;
    }
    else
    {   /* disconnect process */
        l_ulConnectFlag = FSIFTEST_FLG_OFF;
    }
}

/**** LOCAL FUNCTIONS ***************************************************************************/
/************************************************************************************************/
/* FUNCTION   : _FsifTest_MakeData                                                              */
/*                                                                                              */
/* DESCRIPTION: Make test data.                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pucData                         Data buffer area                                */
/*              ulDataLen                       Data buffer length                              */
/* OUTPUT     : pucData                         Made data                                       */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
static void _FsifTest_MakeData(grp_u8 *pucData, grp_u32 ulDataLen)
{
grp_si      iCnt;

    for( iCnt = 0; iCnt < ulDataLen; iCnt ++, pucData++ )
    {
        *pucData = iCnt % 0x100;
    }
}

/************************************************************************************************/
/* FUNCTION   : _FsifTest_ClearData                                                             */
/*                                                                                              */
/* DESCRIPTION: Clear data buffer.                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pucData                         Data buffer area                                */
/*              ulDataLen                       Data buffer length                              */
/* OUTPUT     : pucData                         Cleared data                                    */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
static void _FsifTest_ClearData(grp_u8 *pucData, grp_u32 ulDataLen)
{
grp_si      iCnt;

    for( iCnt = 0; iCnt < ulDataLen; iCnt ++, pucData++ )
    {
        *pucData = 0;
    }
}

/************************************************************************************************/
/* FUNCTION   : _FsifTest_CheckData                                                             */
/*                                                                                              */
/* DESCRIPTION: Cehck test data.                                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pucData                         Data buffer area                                */
/*              ulDataLen                       Data buffer length                              */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : FSIFTEST_OK                     Success                                         */
/*              FSIFTEST_NG                     Fail                                            */
/*                                                                                              */
/************************************************************************************************/
static grp_s32 _FsifTest_CheckData(grp_u8 *pucData, grp_u32 ulDataLen)
{
grp_s32     lStat = FSIFTEST_OK;
grp_si      iCnt;

    for( iCnt = 0; iCnt < ulDataLen; iCnt ++, pucData++ )
    {
        if( *pucData != ( iCnt % 0x100 ) )
        {
            lStat = FSIFTEST_NG;
        }
    }
    return lStat;
}

