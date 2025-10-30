/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2007-2008 Grape Systems, Inc.                          */
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
/*      grp_fsif                                                                1.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Device I/O routines for GR-USB/HOST# Mass storage package.                              */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   K.Takagi       2007/07/06  V0.01                                                           */
/*                            - Created initiale version                                        */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*   K.Takagi       2008/10/31  V1.01                                                           */
/*                            - Correction by member change in structure.                       */
/*                              - grp_fsif_init_prm                                             */
/*                                  pfnEventNotification -> pfnFsifNotification                 */
/*                            - Changed parameter types.                                        */
/*                              - grp_fsif_EventNotification                                    */
/*                                                                                              */
/************************************************************************************************/

#include "grp_vos.h"
#include "grp_fs_disk_part.h"
#include "grp_fs_cfg.h"
#include "grp_fsif.h"

static grp_fs_dev_open_t                _grp_fs_grusb_open_dev;                 /* open device  */
static grp_fs_dev_close_t               _grp_fs_grusb_close_dev;                /* close device */
static grp_fs_dev_read_t                _grp_fs_grusb_read_dev;                 /* read device  */
static grp_fs_dev_write_t               _grp_fs_grusb_write_dev;                /* write device */
static grp_fs_dev_ioctl_t               _grp_fs_grusb_ioctl_dev;                /* ioctl device */

grp_fs_dev_op_t     grp_fs_dev_op_grusb = {                                     /* device       */
    _grp_fs_grusb_open_dev,                                                     /* open device  */
    _grp_fs_grusb_close_dev,                                                    /* close device */
    _grp_fs_grusb_read_dev,                                                     /* read device  */
    _grp_fs_grusb_write_dev,                                                    /* write device */
    _grp_fs_grusb_ioctl_dev                                                     /* ioctl device */
};

/*----------------------------------------------------------------------------------------------*/
/* The following codes are sample programs.                                             >>      */
/* Queue for notification to the application */
grp_vos_t_queue                         *g_ptXferQue;
#define XFER_QUENAME                    ((grp_u8 *)"qXFER")
#define XFER_QUESIZE                    4
#define XFER_QUECOUNT                   10
/* handler management table */
#define _GRP_FSIF_MAX_TBL_NUM_          5
grp_si   l_aiHdrTbl[5];
/*                                                                                      <<      */
/*----------------------------------------------------------------------------------------------*/

/************************************************************************************************/
/* FUNCTION:    _grp_fs_grusb_open_dev                                                          */
/*                                                                                              */
/* DESCRIPTION: Open device                                                                     */
/*              Note: only non extended and non CHS FAT type file is supported                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT:       iDev:                           device number                                   */
/*              iRWOpen:                        read/write open                                 */
/* OUTPUT:      piHandle:                       I/O handle                                      */
/*              puiOff:                         start offset                                    */
/*              puiSize:                        size of device                                  */
/*              piSzShift:                      block size shift                                */
/*                                                                                              */
/* RESULT:      GRP_FS_ERR_IO:                  I/O error                                       */
/*              GRP_FS_ERR_BAD_DEV:             bad device name/number                          */
/*              GRP_FS_ERR_TOO_MANY:            too many opens                                  */
/*              GRP_FS_ERR_BUSY:                device busy                                     */
/*              GRP_FS_ERR_FS:                  bad file system                                 */
/*              0:                              success                                         */
/*                                                                                              */
/************************************************************************************************/
static int _grp_fs_grusb_open_dev( int iDev, int iRWOpen, grp_int32_t *piHandle, grp_uint32_t *puiOff, grp_uint32_t *puiSize, int *piSzShift)
{
int                             iMajor;                             /* major device number      */
int                             iSubId;                             /* sub device ID            */
int                             iPart;                              /* partition number         */
int                             i;                                  /* block shift count        */
int                             iRet;                               /* return value             */
int                             iTmpRet;                            /* dumy return value        */
grp_int32_t                     iSecSize;                           /* sector size              */
grp_int32_t                     iSec;                               /* sector count             */
grp_fs_dk_part_t                *ptPart;                            /* partition information    */
grp_fsif_media_info             tMediaInfo;                         /* PC card attribute        */
grp_fs_dk_part_t                atPart[GRP_FS_PART_CNT];            /* partition information    */
unsigned char                   *pucBuf;                            /* partition data buffer    */

    iRWOpen = 0;    /* Warning measures */

    iMajor = GRP_FS_DEV_MAJOR(iDev);                                /* major device             */
    iSubId = GRP_FS_DEV_SUBID(iDev);                                /* sub device ID            */
    iPart = GRP_FS_DEV_PART(iDev);                                  /* partition number         */
    if ((iPart < 0)                                                 /* check parameters         */
     || ((iPart >= GRP_FS_PART_CNT) && (iPart != GRP_FS_DEV_RAW_PART))) {
                                                                    /* invalid device number    */
        return(GRP_FS_ERR_BAD_DEV);                                 /* return error             */
    }

    if (l_aiHdrTbl[iSubId] == 0) {
        return(GRP_FS_ERR_BAD_DEV);                                 /* return error             */
    }

    if ((grp_fsif_GetMediaInfo((void *)l_aiHdrTbl[iSubId], &tMediaInfo) != 0)
                                                                    /* failed to get attribute  */
     || (tMediaInfo.ulSectorNum == 0)                               /* not ready                */
     || (tMediaInfo.ulSectorSize == 0)) {                           /* not ready                */
        return(GRP_FS_ERR_IO);                                      /* return error             */
    }
    iSecSize = (int)tMediaInfo.ulSectorSize;                        /* set sector size          */
    iRet = GRP_FS_ERR_IO;                                           /* set default error number */
    *piHandle = 0;                                                  /* set dummy handle         */
    if (iSecSize & (iSecSize - 1)) {                        /* sector size is not power of 2    */
        goto err_ret;                                               /* return error             */
    }
    for (i = 0; iSecSize != (1 << i); i++);                          /* compute block shift     */
    *piSzShift = i;                                                  /* set block shift count   */
    if (iPart == GRP_FS_DEV_RAW_PART) {                          /* raw partition (no partition)*/
        *puiOff = 0;                                                 /* set start offset        */
        *puiSize = tMediaInfo.ulSectorNum;                           /* set block count         */
        return(0);                                                   /* return success          */
    }
    if (iSecSize > 4096) {
        goto err_ret;                                               /* return error             */
    }
    else if (iSecSize >= 512) {
        iSec = 1;                                       /* read one sector that is more 512bytes*/
    }
    else {
        iSec = 512 / iSecSize;                                  /* read sectors until 512bytes  */
    }
    if (grp_fsif_GetNonCacheBuffer( &pucBuf) != GRP_FSIF_OK) {              /* get buffer       */
        goto err_ret;
    }
    if (_grp_fs_grusb_read_dev(*piHandle, iDev, 0, pucBuf, iSec) != iSec) {
                                                                    /* failed to read 1st block */
        grp_fsif_RelNonCacheBuffer( pucBuf);                        /* release buffer           */
        goto err_ret;                                               /* return error             */
    }
    iTmpRet = grp_fs_get_part(pucBuf, atPart);
    if (grp_fsif_RelNonCacheBuffer( pucBuf) != GRP_FSIF_OK) {       /* release buffer           */
        goto err_ret;
    }

    iRet = GRP_FS_ERR_FS;                                           /* set default error number */
    switch(iTmpRet) {                                               /* get partition infomation */
    case GRP_FS_PART_VALID:                                     /* valid partition information  */
        break;                                                      /* break                    */
    case GRP_FS_PART_LESS:                                          /* partition less           */
        *puiOff = 0;                                                /* start offset             */
        *puiSize = tMediaInfo.ulSectorNum;                          /* set capacity             */
        return(0);                                                  /* return success           */
    default:                                                        /* others                   */
        goto err_ret;                                               /* return error             */
    }
    ptPart = &atPart[iPart];                                        /* target partition         */
    switch(ptPart->ucPartType) {                                    /* check partition type     */
    case GRP_FS_PART_FAT12:                                         /* FAT12                    */
    case GRP_FS_PART_FAT16_L32:                                     /* FAT16 < 32MB             */
    case GRP_FS_PART_FAT16_H32:                                     /* FAT16 >= 32MB            */
    case GRP_FS_PART_FAT32_LBA:                                     /* FAT32 LBA                */
    case GRP_FS_PART_FAT32_CHS:                                     /* FAT32 CHS                */
    case GRP_FS_PART_FAT16_LBA:                                     /* FAT16 LBA                */
    case GRP_FS_PART_FAT32_HCHS:                                    /* hidden FAT32 CHS         */
    case GRP_FS_PART_FAT32_HLBA:                                    /* hidden FAT32 LBA         */
    case GRP_FS_PART_FAT16_HLBA:                                    /* hidden FAT16 LBA         */
        break;                                                      /* success                  */
    default:
        goto err_ret;                                               /* return error             */
    }
    *puiOff = ptPart->uiStartSec;                                   /* start offset             */
    *puiSize = ptPart->uiSecCnt;                                    /* set block count          */
    return(0);                                                      /* return success           */

err_ret:
    _grp_fs_grusb_close_dev(*piHandle, iDev);                       /* close device             */
    return(iRet);                                                   /* return I/O error         */
}

/************************************************************************************************/
/* FUNCTION:    _grp_fs_grusb_close_dev                                                         */
/*                                                                                              */
/* DESCRIPTION: Close device                                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT:       iHandle,                        I/O handle                                      */
/*              iDev:                           device number                                   */
/* OUTPUT:      None                                                                            */
/*                                                                                              */
/* RESULT:      GRP_FS_ERR_IO:                  I/O error                                       */
/*              0:                              success                                         */
/*                                                                                              */
/************************************************************************************************/
static int _grp_fs_grusb_close_dev( grp_int32_t iHandle, int iDev)
{
    /* no process */

    iHandle = 0;    /* Warning measures */
    iDev    = 0;    /* Warning measures */

    return(0);                                                          /* return success       */
}

/************************************************************************************************/
/* FUNCTION:    _grp_fs_grusb_read_dev                                                          */
/*                                                                                              */
/* DESCRIPTION: Read device                                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT:       iHandle,                        I/O handle                                      */
/*              iDev:                           device number                                   */
/*              uiDevBlk:                       device block number                             */
/*              iCnt:                           I/O count                                       */
/* OUTPUT:      pucBuf:                         data read                                       */
/*                                                                                              */
/* RESULT:      GRP_FS_ERR_IO:                  I/O error                                       */
/*              0 or positive:                  read count                                      */
/*                                                                                              */
/************************************************************************************************/
static grp_int32_t _grp_fs_grusb_read_dev( grp_int32_t iHandle, int iDev, grp_uint32_t uiDevBlk, grp_uchar_t *pucBuf, grp_isize_t iCnt)
{
int                             iRet;                           /* return value from usb read   */
int                             iMajor;                         /* major device number          */
int                             iSubId;                         /* sub device ID                */
int                             iPart;                          /* partition number             */

    iHandle = 0;    /* Warning measures */

    if (iCnt <= 0) {                                                /* no data to read          */
        return(0);                                                  /* return 0                 */
    }

    iMajor = GRP_FS_DEV_MAJOR(iDev);                                /* major device             */
    iSubId = GRP_FS_DEV_SUBID(iDev);                                /* sub device ID            */
    iPart  = GRP_FS_DEV_PART(iDev);                                 /* partition number         */
    if (l_aiHdrTbl[iSubId] == 0) {
        return(GRP_FS_ERR_BAD_DEV);                                 /* return error             */
    }

    iRet = grp_fsif_ReadSector( (void *)l_aiHdrTbl[iSubId], (grp_u32)uiDevBlk, (grp_u32)iCnt, pucBuf);
    if (iRet != 0) {                                                        /* read error       */
        return(GRP_FS_ERR_IO);                                              /* I/O error        */
    }

    return(iCnt);                                                   /* return read count        */
}

/************************************************************************************************/
/* FUNCTION:    _grp_fs_grusb_write_dev                                                         */
/*                                                                                              */
/* DESCRIPTION: Write device                                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT:       iHandle,                        I/O handle                                      */
/*              iDev:                           device number                                   */
/*              uiDevBlk:                       device block number                             */
/*              pucBuf:                         data to write                                   */
/* OUTPUT:      None                                                                            */
/*                                                                                              */
/* RESULT:      GRP_FS_ERR_IO:                  I/O error                                       */
/*              0 or positive:                  written count                                   */
/*                                                                                              */
/************************************************************************************************/
static grp_int32_t _grp_fs_grusb_write_dev( grp_int32_t iHandle, int iDev, grp_uint32_t uiDevBlk, grp_uchar_t *pucBuf, grp_isize_t iCnt)
{
int                             iRet;                           /* return value from usb read   */
int                             iMajor;                         /* major device number          */
int                             iSubId;                         /* sub device ID                */
int                             iPart;                          /* partition number             */

    iHandle = 0;    /* Warning measures */

    if (iCnt <= 0) {                                                /* no data to write         */
        return(0);                                                  /* return 0                 */
    }

    iMajor = GRP_FS_DEV_MAJOR(iDev);                                /* major device             */
    iSubId = GRP_FS_DEV_SUBID(iDev);                                /* sub device ID            */
    iPart  = GRP_FS_DEV_PART(iDev);                                 /* partition number         */
    if (l_aiHdrTbl[iSubId] == 0) {
        return(GRP_FS_ERR_BAD_DEV);                                 /* return error             */
    }

    iRet = grp_fsif_WriteSector( (void *)l_aiHdrTbl[iSubId], (grp_u32)uiDevBlk, (grp_u32)iCnt, pucBuf);
    if (iRet != 0) {                                                        /* read error       */
        return(GRP_FS_ERR_IO);                                              /* I/O error        */
    }

    return(iCnt);                                                   /* return read count        */
}

/************************************************************************************************/
/* FUNCTION:    _grp_fs_grusb_ioctl_dev                                                         */
/*                                                                                              */
/* DESCRIPTION: ioctl device                                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT:       iHandle,                        I/O handle                                      */
/*              iDev:                           device number                                   */
/*              uiCmd:                          command                                         */
/*              pvParam:                        parameter (for some commands)                   */
/* OUTPUT:      pvParam:                        result data (for some commands)                 */
/*                                                                                              */
/* RESULT:      GRP_FS_ERR_NOT_SUPPORT:         not supported                                   */
/*              GRP_FS_ERR_BAD_DEV:             invalid device number                           */
/*              GRP_FS_ERR_TOO_MANY:            too many opens                                  */
/*              GRP_FS_ERR_BUSY:                device busy                                     */
/*              GRP_FS_ERR_IO:                  I/O error                                       */
/*              0:                              success                                         */
/*                                                                                              */
/************************************************************************************************/
static int _grp_fs_grusb_ioctl_dev( int iDev, grp_uint32_t uiCmd, void *pvParam)
{
    iDev    = 0;    /* Warning measures */
    uiCmd   = 0;    /* Warning measures */
    pvParam = 0;    /* Warning measures */

    /* not supported */
    return(GRP_FS_ERR_NOT_SUPPORT);                                     /* return               */
}


/*----------------------------------------------------------------------------------------------*/
/* The following codes are sample programs.                                             >>      */

/************************************************************************************************/
/* FUNCTION:    grp_fsif_EventNotification                                                      */
/*                                                                                              */
/* DESCRIPTION: Notified the event to the application                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT:       ulEvnt                          Connect or Dissconect event                     */
/*              pvHdr                           handler                                         */
/*              ucIdx                           index number                                    */
/* OUTPUT:      None                                                                            */
/*                                                                                              */
/* RESULT:      None                                                                            */
/*                                                                                              */
/************************************************************************************************/
void grp_fsif_EventNotification( grp_u32 ulEvnt, void *pvHdr, grp_u8 ucIdx)
{
grp_u32                         ulMsg = 0;
grp_s32                         lStat;
int                             i;

    ucIdx = 0;  /* Warning measures */

    if (ulEvnt == GRP_FSIF_ATTACHED_MEDIA) {
        /* connected */
        for (i=0; i<_GRP_FSIF_MAX_TBL_NUM_; i++) {
            if (l_aiHdrTbl[i] == 0) {
                l_aiHdrTbl[i] = (grp_si)pvHdr;
                break;
            }
        }
        /* error check */
        if (i == _GRP_FSIF_MAX_TBL_NUM_) {
            /* no table */
            return;
        }
        /* notified the index to the application */
        ulMsg = GRP_FSIF_ATTACHED_MEDIA;                                /* Set Connect State    */
    }
    else {
        /* disconnected */
        for (i=0; i<_GRP_FSIF_MAX_TBL_NUM_; i++) {
            if (l_aiHdrTbl[i] == (grp_si)pvHdr) {
                l_aiHdrTbl[i] = 0;
                break;
            }
        }
        /* error check */
        if (i == _GRP_FSIF_MAX_TBL_NUM_) {
            /* no table */
            return;
        }
        /* notified the index to the application */
        ulMsg = GRP_FSIF_DETACHED_MEDIA;                                /* Set Disconnect State */
    }

    ulMsg |= i & 0x000000ff;                                            /* Set Device Number    */
    lStat = grp_vos_SendQueue(g_ptXferQue, (void *)&ulMsg, GRP_VOS_NOWAIT);
    if (lStat == GRP_VOS_POS_RESULT) {
        /* error */
        return;
    }

    return;
}

/************************************************************************************************/
/* FUNCTION:    grp_fsif_dev_io_init                                                            */
/*                                                                                              */
/* DESCRIPTION: Initialized this module and the fsif module                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT:       None                                                                            */
/* OUTPUT:      None                                                                            */
/*                                                                                              */
/* RESULTS    : 0                               Success                                         */
/*              -1                              Error                                           */
/*                                                                                              */
/************************************************************************************************/
int grp_fsif_dev_io_init(void)
{
grp_fsif_init_prm               tInitPrm;
grp_s32                         lStat;
int                             i;

    /* initialize of the table */
    for (i=0; i<_GRP_FSIF_MAX_TBL_NUM_; i++) {
        l_aiHdrTbl[i] = 0;
    }

    /* create queue */
    lStat = grp_vos_CreateQueue(&g_ptXferQue, XFER_QUENAME, XFER_QUESIZE, XFER_QUECOUNT);
    if (lStat != GRP_VOS_POS_RESULT) {
        return -1;
    }

    /* initialized of fsif modules */
    tInitPrm.pfnFsifNotification = grp_fsif_EventNotification;
    lStat = grp_fsif_Init( &tInitPrm);
    if (lStat != GRP_FSIF_OK) {
        return -1;
    }

    return 0;
}

/*                                                                                      <<      */
/*----------------------------------------------------------------------------------------------*/
