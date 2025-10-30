/************************************************************************************************/
/*                                                                                              */
/*                          Copyright(C) 2006-2008 Grape Systems, Inc.                          */
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
/*      grp_msc_scsi.c                                                          1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver Scsi Sub Class.                                  */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                            - Created new policy initial version                              */
/*   K.Takagi       2007/12/28  V0.90                                                           */
/*                            - Created beta version                                            */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_msc_local.h"
#include "grp_msc_scsi.h"
#include "grp_scsi_local.h"


/***** LOCAL PARAMETER DEFINITIONS **************************************************************/


/***** LOCAL FUNCTION PROTOTYPES ****************************************************************/
LOCAL grp_s32 _msc_ScsiReadSector10( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent );
LOCAL grp_s32 _msc_ScsiReadSector12( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent );
LOCAL grp_s32 _msc_ScsiWriteSector10( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent );
LOCAL grp_s32 _msc_ScsiWriteSector12( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent );
LOCAL grp_s32 _msc_ScsiModeSense06( grp_msc_cmd *ptCmd, grp_u8 ucPage, grp_u32 *pulLength, grp_u8 *pucContent );
LOCAL grp_s32 _msc_ScsiModeSense10( grp_msc_cmd *ptCmd, grp_u8 ucPage, grp_u32 *pulLength, grp_u8 *pucContent );

/************************************************************************************************/
/* FUNCTION     : grp_msc_ScsiInit                                                              */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_msc_ScsiInit( void )
{
grp_s32                         lStatus = GRP_MSC_OK;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiReadSector                                                           */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 _msc_ScsiReadSector( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
grp_s32                         lStatus;

    if( ulSectorSize > GRP_USBC_MAX_US ){
        lStatus = _msc_ScsiReadSector12( ptCmd, ulStartLba, ulSectorSize, pulLength, pucContent );
    }
    else{
        lStatus = _msc_ScsiReadSector10( ptCmd, ulStartLba, ulSectorSize, pulLength, pucContent );
    }

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiWriteSector                                                          */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 _msc_ScsiWriteSector( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
grp_s32                         lStatus;

    if( ulSectorSize > GRP_USBC_MAX_US ){
        lStatus = _msc_ScsiWriteSector12( ptCmd, ulStartLba, ulSectorSize, pulLength, pucContent );
    }
    else{
        lStatus = _msc_ScsiWriteSector10( ptCmd, ulStartLba, ulSectorSize, pulLength, pucContent );
    }

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiInquiry                                                              */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 _msc_ScsiInquiry( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
SBC_INQUIRY_STR                 *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SBC_INQUIRY_SZ;

    pucStr = (SBC_INQUIRY_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SBC_INQUIRY_CMD;
    /* Allocation Length */
    if( ptCmd->ulReqLength > GRP_USBC_OVER_UC ){
        ptCmd->ulReqLength = GRP_USBC_MAX_UC;
    }
    pucStr->ucAllocationLen = (grp_u8)ptCmd->ulReqLength;
    /* Other parameters */
    pucStr->ucCmddtEvpd     = 0;
    pucStr->ucPageOperation = 0;
    pucStr->ucControl       = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiReadFormatCapacity                                                   */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 _msc_ScsiReadFormatCapacity( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
SBC_READ_FORMAT_CAPA_STR        *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SBC_READ_FORMAT_CAPA_SZ;

    pucStr = (SBC_READ_FORMAT_CAPA_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SBC_READ_FORMAT_CAPA_CMD;
    /* Data size */
    if( ptCmd->ulReqLength > GRP_USBC_OVER_US ){
        ptCmd->ulReqLength = GRP_USBC_MAX_US;
    }
    pucStr->aucAllocationLen[0] = (grp_u8)((grp_u32)(ptCmd->ulReqLength>> 8) & 0xff);
    pucStr->aucAllocationLen[1] = (grp_u8)((grp_u32)(ptCmd->ulReqLength>> 0) & 0xff);
    /* Other parameters */
    pucStr->ucControl = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiReadCapacity                                                         */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 _msc_ScsiReadCapacity( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
SBC_READ_CAPACITY_STR           *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    ptCmd      = GRP_USB_NULL;  /* Not use on this version  */
    *pulLength = SBC_READ_CAPACITY_SZ;

    pucStr = (SBC_READ_CAPACITY_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SBC_READ_CAPACITY_CMD;
    /* Logical block address */
    pucStr->aucLba[0] = 0;
    pucStr->aucLba[1] = 0;
    pucStr->aucLba[2] = 0;
    pucStr->aucLba[3] = 0;
    /* Other parameters */
    pucStr->ucPmi     = 0;
    pucStr->ucControl = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiModeSense                                                            */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 _msc_ScsiModeSense( grp_msc_cmd *ptCmd, grp_u8 ucPage, grp_u32 *pulLength, grp_u8 *pucContent )
{
grp_s32                         lStatus = GRP_MSC_OK;

    if( ptCmd->ulReqLength > GRP_USBC_MAX_UC ){
        lStatus = _msc_ScsiModeSense10( ptCmd, ucPage, pulLength, pucContent );
    }
    else{
        lStatus = _msc_ScsiModeSense06( ptCmd, ucPage, pulLength, pucContent );
    }

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiTestUnitReady                                                        */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 _msc_ScsiTestUnitReady( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
SBC_TEST_UNIT_READY_STR         *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    ptCmd      = GRP_USB_NULL;  /* Not use on this version  */
    *pulLength = SBC_TEST_UNIT_READY_SZ;

    pucStr = (SBC_TEST_UNIT_READY_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SBC_TEST_UNIT_READY_CMD;
    /* Other parameters */
    pucStr->ucControl = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiRequestSense                                                         */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 _msc_ScsiRequestSense( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
SBC_REQUEST_SENSE_STR           *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SBC_REQUEST_SENSE_SZ;

    pucStr = (SBC_REQUEST_SENSE_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SBC_REQUEST_SENSE_CMD;
    /* Allocation Length */
    pucStr->ucAllocationLen = (grp_u8)(ptCmd->ulReqLength & 0xff);
    /* Other parameters */
    pucStr->ucControl = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiReadSector10                                                         */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _msc_ScsiReadSector10( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
SBC_READ10_STR                  *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    ptCmd      = GRP_USB_NULL;  /* Not use on this version  */
    *pulLength = SBC_READ10_SZ;

    pucStr = (SBC_READ10_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SBC_READ10_CMD;
    /* Block address */
    pucStr->aucLba[0]     = (grp_u8)((grp_u32)(ulStartLba>>24) & 0xff);
    pucStr->aucLba[1]     = (grp_u8)((grp_u32)(ulStartLba>>16) & 0xff);
    pucStr->aucLba[2]     = (grp_u8)((grp_u32)(ulStartLba>> 8) & 0xff);
    pucStr->aucLba[3]     = (grp_u8)((grp_u32)(ulStartLba>> 0) & 0xff);
    /* Reading sector size */
    pucStr->aucTransferLen[0] = (grp_u8)((grp_u32)(ulSectorSize>> 8) & 0xff);
    pucStr->aucTransferLen[1] = (grp_u8)((grp_u32)(ulSectorSize>> 0) & 0xff);
    /* Other parameters */
    pucStr->ucDpoFuaReladr = 0;
    pucStr->ucControl      = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiReadSector12                                                         */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _msc_ScsiReadSector12( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
SBC_READ12_STR                  *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    ptCmd      = GRP_USB_NULL;  /* Not use on this version  */
    *pulLength = SBC_READ12_SZ;

    pucStr = (SBC_READ12_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SBC_READ12_CMD;
    /* Block address */
    pucStr->aucLba[0] = (grp_u8)((grp_u32)(ulStartLba>>24) & 0xff);
    pucStr->aucLba[1] = (grp_u8)((grp_u32)(ulStartLba>>16) & 0xff);
    pucStr->aucLba[2] = (grp_u8)((grp_u32)(ulStartLba>> 8) & 0xff);
    pucStr->aucLba[3] = (grp_u8)((grp_u32)(ulStartLba>> 0) & 0xff);
    /* Read sector size */
    pucStr->aucTransferLen[0] = (grp_u8)((grp_u32)(ulSectorSize>>24) & 0xff);
    pucStr->aucTransferLen[1] = (grp_u8)((grp_u32)(ulSectorSize>>16) & 0xff);
    pucStr->aucTransferLen[2] = (grp_u8)((grp_u32)(ulSectorSize>> 8) & 0xff);
    pucStr->aucTransferLen[3] = (grp_u8)((grp_u32)(ulSectorSize>> 0) & 0xff);
    /* Other parameters */
    pucStr->ucStreaming = 0;
    pucStr->ucControl   = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiWriteSector10                                                        */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _msc_ScsiWriteSector10( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
SBC_WRITE10_STR                 *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    ptCmd      = GRP_USB_NULL;  /* Not use on this version  */
    *pulLength = SBC_WRITE10_SZ;

    pucStr = (SBC_WRITE10_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SBC_WRITE10_CMD;
    /* Block address */
    pucStr->aucLba[0] = (grp_u8)((grp_u32)(ulStartLba>>24) & 0xff);
    pucStr->aucLba[1] = (grp_u8)((grp_u32)(ulStartLba>>16) & 0xff);
    pucStr->aucLba[2] = (grp_u8)((grp_u32)(ulStartLba>> 8) & 0xff);
    pucStr->aucLba[3] = (grp_u8)((grp_u32)(ulStartLba>> 0) & 0xff);
    /* Reading sector size */
    pucStr->aucTransferLen[0] = (grp_u8)((grp_u32)(ulSectorSize>> 8) & 0xff);
    pucStr->aucTransferLen[1] = (grp_u8)((grp_u32)(ulSectorSize>> 0) & 0xff);
    /* Other parameters */
    pucStr->ucDpoFuaReladr = 0;
    pucStr->ucControl      = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiWriteSector                                                          */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _msc_ScsiWriteSector12( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
SBC_WRITE12_STR                 *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    ptCmd      = GRP_USB_NULL;  /* Not use on this version  */
    *pulLength = SBC_WRITE12_SZ;

    pucStr = (SBC_WRITE12_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SBC_WRITE12_CMD;
    /* Block address */
    pucStr->aucLba[0] = (grp_u8)((grp_u32)(ulStartLba>>24) & 0xff);
    pucStr->aucLba[1] = (grp_u8)((grp_u32)(ulStartLba>>16) & 0xff);
    pucStr->aucLba[2] = (grp_u8)((grp_u32)(ulStartLba>> 8) & 0xff);
    pucStr->aucLba[3] = (grp_u8)((grp_u32)(ulStartLba>> 0) & 0xff);
    /* Reading sector size */
    pucStr->aucTransferLen[0] = (grp_u8)((grp_u32)(ulSectorSize>>24) & 0xff);
    pucStr->aucTransferLen[1] = (grp_u8)((grp_u32)(ulSectorSize>>16) & 0xff);
    pucStr->aucTransferLen[2] = (grp_u8)((grp_u32)(ulSectorSize>> 8) & 0xff);
    pucStr->aucTransferLen[3] = (grp_u8)((grp_u32)(ulSectorSize>> 0) & 0xff);
    /* Other parameters */
    pucStr->ucControl = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiModeSense06                                                          */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _msc_ScsiModeSense06( grp_msc_cmd *ptCmd, grp_u8 ucPage, grp_u32 *pulLength, grp_u8 *pucContent )
{
SBC_MODE_SENSE06_STR            *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SBC_MODE_SENSE06_SZ;

    pucStr = (SBC_MODE_SENSE06_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SBC_MODE_SENSE06_CMD;
    /* Page code */
    pucStr->ucPcPage  = ucPage;                     /* Bit7-6:Page Control  */
                                                    /* Bit5-0:Page code     */
    /* Allocation Length */
    pucStr->ucAllocationLen = (grp_u8)ptCmd->ulReqLength;
    /* Other parameters */
    pucStr->ucDbd     = 0;                          /* Bit3:DBD(0/1)        */
    pucStr->ucControl = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_ScsiModeSense10                                                          */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : none                                                                          */
/* OUTPUT       : none                                                                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_ERROR                 Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _msc_ScsiModeSense10( grp_msc_cmd *ptCmd, grp_u8 ucPage, grp_u32 *pulLength, grp_u8 *pucContent )
{
SBC_MODE_SENSE10_STR            *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SBC_MODE_SENSE10_SZ;

    pucStr = (SBC_MODE_SENSE10_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SBC_MODE_SENSE10_CMD;
    /* Page code */
    pucStr->ucPcPage  = ucPage;                         /* Bit7-6:Page Control  */
                                                        /* Bit5-0:Page code     */
    /* Allocation Length */
    pucStr->aucAllocationLen[0] = (grp_u8)((grp_u32)(ptCmd->ulReqLength>> 8) & 0xff);
    pucStr->aucAllocationLen[1] = (grp_u8)((grp_u32)(ptCmd->ulReqLength>> 0) & 0xff);
    /* Other parameters */
    pucStr->ucDbd     = 0;                              /* Bit3:DBD(0/1)        */
    pucStr->ucControl = 0;

    return( lStatus );
}
