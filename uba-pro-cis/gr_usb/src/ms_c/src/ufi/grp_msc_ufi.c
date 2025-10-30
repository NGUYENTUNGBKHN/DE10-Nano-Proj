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
/*      grp_msc_atapi.c                                                         1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver Ufi Sub Class.                                   */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Yamada       2006/11/01  V0.01                                                           */
/*                            - Created new policy initial version                              */
/*   K.Takagi       2008/07/31  V1.00                                                           */
/*                            - Created 1st release version                                     */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_msc_local.h"
#include "grp_msc_ufi.h"
#include "grp_ufi_local.h"


/***** LOCAL PARAMETER DEFINITIONS **************************************************************/


/***** LOCAL FUNCTION PROTOTYPES ****************************************************************/
LOCAL grp_s32 _msc_UfiReadSector10( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent );
LOCAL grp_s32 _msc_UfiReadSector12( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent );
LOCAL grp_s32 _msc_UfiWriteSector10( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent );
LOCAL grp_s32 _msc_UfiWriteSector12( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent );


/************************************************************************************************/
/* FUNCTION     : grp_msc_UfiInit                                                               */
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
grp_s32 grp_msc_UfiInit( void )
{
grp_s32                         lStatus = GRP_MSC_OK;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_UfiReadSector                                                            */
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
grp_s32 _msc_UfiReadSector( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *ucContent )
{
grp_s32                         lStatus;

    if( ulSectorSize > GRP_USBC_MAX_US ){
        lStatus = _msc_UfiReadSector12( ptCmd, ulStartLba, ulSectorSize, pulLength, ucContent );
    }
    else{
        lStatus = _msc_UfiReadSector10( ptCmd, ulStartLba, ulSectorSize, pulLength, ucContent );
    }

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_UfiWriteSector                                                           */
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
grp_s32 _msc_UfiWriteSector( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
grp_s32                         lStatus;

    if( ulSectorSize > GRP_USBC_MAX_US ){
        lStatus = _msc_UfiWriteSector12( ptCmd, ulStartLba, ulSectorSize, pulLength, pucContent );
    }
    else{
        lStatus = _msc_UfiWriteSector10( ptCmd, ulStartLba, ulSectorSize, pulLength, pucContent );
    }

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_UfiInquiry                                                               */
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
grp_s32 _msc_UfiInquiry( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
UFI_INQUIRY_STR                 *ptStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = UFI_INQUIRY_SZ;

    ptStr = (UFI_INQUIRY_STR *)pucContent;
    /* Operation code */
    ptStr->ucCommand = UFI_INQUIRY_CMD;
    /* Allocation Length */
    if( ptCmd->ulReqLength > GRP_USBC_OVER_UC ){
        ptCmd->ulReqLength = GRP_USBC_MAX_UC;
    }
    ptStr->ucAllocationLen = (grp_u8)ptCmd->ulReqLength;
    /* Other parameters */
    ptStr->ucCmddtEvpd     = 0;
    ptStr->ucPageOperation = 0;
    ptStr->ucControl       = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_UfiReadFormatCapacity                                                    */
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
grp_s32     _msc_UfiReadFormatCapacity( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
UFI_READ_FORMAT_CAPA_STR        *ptStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = UFI_READ_FORMAT_CAPA_SZ;

    ptStr = (UFI_READ_FORMAT_CAPA_STR *)pucContent;
    /* Operation code */
    ptStr->ucCommand = UFI_READ_FORMAT_CAPA_CMD;
    /* Data size */
    if( ptCmd->ulReqLength > GRP_USBC_OVER_US ){
        ptCmd->ulReqLength = GRP_USBC_MAX_US;
    }
    ptStr->aucAllocationLen[0] = (grp_u8)((grp_u32)(ptCmd->ulReqLength>> 8) & 0xff);
    ptStr->aucAllocationLen[1] = (grp_u8)((grp_u32)(ptCmd->ulReqLength>> 0) & 0xff);
    /* Other parameters */
    ptStr->ucControl = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_UfiReadCapacity                                                          */
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
grp_s32 _msc_UfiReadCapacity( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
UFI_READ_CAPACITY_STR           *ptStr;
grp_s32                         lStatus = GRP_MSC_OK;

    ptCmd      = GRP_USB_NULL;  /* Not use on this version  */
    *pulLength = UFI_READ_CAPACITY_SZ;

    ptStr = (UFI_READ_CAPACITY_STR *)pucContent;
    /* Operation code */
    ptStr->ucCommand = UFI_READ_CAPACITY_CMD;
    /* Other parameters */
    ptStr->ucPmi     = 0;
    ptStr->ucControl = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_UfiModeSense                                                             */
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
grp_s32 _msc_UfiModeSense( grp_msc_cmd *ptCmd, grp_u8 ucPage, grp_u32 *pulLength, grp_u8 *pucContent )
{
UFI_MODE_SENSE_STR              *ptStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = UFI_MODE_SENSE_SZ;

    ptStr = (UFI_MODE_SENSE_STR *)pucContent;
    /* Operation code */
    ptStr->ucCommand = UFI_MODE_SENSE_CMD;
    /* Page code */
    ptStr->ucPcPage  = ucPage;                          /* Bit7-6:Page Control  */
                                                        /* Bit5-0:Page code     */
    /* Allocation Length */
    ptStr->aucAllocationLen[0] = (grp_u8)((grp_u32)(ptCmd->ulReqLength>> 8) & 0xff);
    ptStr->aucAllocationLen[1] = (grp_u8)((grp_u32)(ptCmd->ulReqLength>> 0) & 0xff);
    /* Other parameters */
    ptStr->ucDbd     = 0;                               /* Bit3:DBD(0/1)        */
    ptStr->ucControl = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_UfiReadCapacity                                                          */
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
grp_s32 _msc_UfiTestUnitReady( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
UFI_TEST_UNIT_READY_STR         *ptStr;
grp_s32                         lStatus = GRP_MSC_OK;

    ptCmd      = GRP_USB_NULL;  /* Not use on this version  */
    *pulLength = UFI_TEST_UNIT_READY_SZ;

    ptStr = (UFI_TEST_UNIT_READY_STR *)pucContent;
    /* Operation code */
    ptStr->ucCommand = UFI_TEST_UNIT_READY_CMD;
    /* Other parameters */
    ptStr->ucControl = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_UfiReadCapacity                                                          */
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
grp_s32 _msc_UfiRequestSense( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
UFI_REQUEST_SENSE_STR           *ptStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = UFI_REQUEST_SENSE_SZ;

    ptStr = (UFI_REQUEST_SENSE_STR *)pucContent;
    /* Operation code */
    ptStr->ucCommand = UFI_REQUEST_SENSE_CMD;
    /* Allocation Length */
    ptStr->ucAllocationLen = (grp_u8)(ptCmd->ulReqLength & 0xff);
    /* Other parameters */
    ptStr->ucControl = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_UfiReadSector10                                                          */
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
LOCAL grp_s32 _msc_UfiReadSector10( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
UFI_READ10_STR                  *ptStr;
grp_s32                         lStatus = GRP_MSC_OK;

    ptCmd      = GRP_USB_NULL;  /* Not use on this version  */
    *pulLength = UFI_READ10_SZ;

    ptStr = (UFI_READ10_STR *)pucContent;
    /* Operation code */
    ptStr->ucCommand = UFI_READ10_CMD;
    /* Block address */
    ptStr->aucLba[0] = (grp_u8)((grp_u32)(ulStartLba>>24) & 0xff);
    ptStr->aucLba[1] = (grp_u8)((grp_u32)(ulStartLba>>16) & 0xff);
    ptStr->aucLba[2] = (grp_u8)((grp_u32)(ulStartLba>> 8) & 0xff);
    ptStr->aucLba[3] = (grp_u8)((grp_u32)(ulStartLba>> 0) & 0xff);
    /* Reading sector size */
    ptStr->aucTransferLen[0] = (grp_u8)((grp_u32)(ulSectorSize>> 8) & 0xff);
    ptStr->aucTransferLen[1] = (grp_u8)((grp_u32)(ulSectorSize>> 0) & 0xff);
    /* Other parameters */
    ptStr->ucDpoFuaReladr = 0;
    ptStr->ucControl      = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_UfiReadSector12                                                          */
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
LOCAL grp_s32 _msc_UfiReadSector12( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
UFI_READ12_STR                  *ptStr;
grp_s32                         lStatus = GRP_MSC_OK;

    ptCmd      = GRP_USB_NULL;  /* Not use on this version  */
    *pulLength = UFI_READ12_SZ;

    ptStr = (UFI_READ12_STR *)pucContent;
    /* Operation code */
    ptStr->ucCommand = UFI_READ12_CMD;
    /* Block address */
    ptStr->aucLba[0] = (grp_u8)((grp_u32)(ulStartLba>>24) & 0xff);
    ptStr->aucLba[1] = (grp_u8)((grp_u32)(ulStartLba>>16) & 0xff);
    ptStr->aucLba[2] = (grp_u8)((grp_u32)(ulStartLba>> 8) & 0xff);
    ptStr->aucLba[3] = (grp_u8)((grp_u32)(ulStartLba>> 0) & 0xff);
    /* Read sector size */
    ptStr->aucTransferLen[0] = (grp_u8)((grp_u32)(ulSectorSize>>24) & 0xff);
    ptStr->aucTransferLen[1] = (grp_u8)((grp_u32)(ulSectorSize>>16) & 0xff);
    ptStr->aucTransferLen[2] = (grp_u8)((grp_u32)(ulSectorSize>> 8) & 0xff);
    ptStr->aucTransferLen[3] = (grp_u8)((grp_u32)(ulSectorSize>> 0) & 0xff);
    /* Other parameters */
    ptStr->ucStreaming = 0;
    ptStr->ucControl   = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_UfiWriteSector10                                                         */
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
LOCAL grp_s32 _msc_UfiWriteSector10( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
UFI_WRITE10_STR                 *ptStr;
grp_s32                         lStatus = GRP_MSC_OK;

    ptCmd      = GRP_USB_NULL;  /* Not use on this version  */
    *pulLength = UFI_WRITE10_SZ;

    ptStr = (UFI_WRITE10_STR *)pucContent;
    /* Operation code */
    ptStr->ucCommand = UFI_WRITE10_CMD;
    /* Block address */
    ptStr->aucLba[0] = (grp_u8)((grp_u32)(ulStartLba>>24) & 0xff);
    ptStr->aucLba[1] = (grp_u8)((grp_u32)(ulStartLba>>16) & 0xff);
    ptStr->aucLba[2] = (grp_u8)((grp_u32)(ulStartLba>> 8) & 0xff);
    ptStr->aucLba[3] = (grp_u8)((grp_u32)(ulStartLba>> 0) & 0xff);
    /* Reading sector size */
    ptStr->aucTransferLen[0] = (grp_u8)((grp_u32)(ulSectorSize>> 8) & 0xff);
    ptStr->aucTransferLen[1] = (grp_u8)((grp_u32)(ulSectorSize>> 0) & 0xff);
    /* Other parameters */
    ptStr->ucDpoFuaEbpReladr = 0;
    ptStr->ucControl         = 0;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_UfiWriteSector                                                           */
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
LOCAL grp_s32 _msc_UfiWriteSector12( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
UFI_WRITE12_STR                 *ptStr;
grp_s32                         lStatus = GRP_MSC_OK;

    ptCmd      = GRP_USB_NULL;  /* Not use on this version  */
    *pulLength = UFI_WRITE12_SZ;

    ptStr = (UFI_WRITE12_STR *)pucContent;
    /* Operation code */
    ptStr->ucCommand = UFI_WRITE12_CMD;
    /* Block address */
    ptStr->aucLba[0] = (grp_u8)((grp_u32)(ulStartLba>>24) & 0xff);
    ptStr->aucLba[1] = (grp_u8)((grp_u32)(ulStartLba>>16) & 0xff);
    ptStr->aucLba[2] = (grp_u8)((grp_u32)(ulStartLba>> 8) & 0xff);
    ptStr->aucLba[3] = (grp_u8)((grp_u32)(ulStartLba>> 0) & 0xff);
    /* Reading sector size */
    ptStr->aucTransferLen[0] = (grp_u8)((grp_u32)(ulSectorSize>>24) & 0xff);
    ptStr->aucTransferLen[1] = (grp_u8)((grp_u32)(ulSectorSize>>16) & 0xff);
    ptStr->aucTransferLen[2] = (grp_u8)((grp_u32)(ulSectorSize>> 8) & 0xff);
    ptStr->aucTransferLen[3] = (grp_u8)((grp_u32)(ulSectorSize>> 0) & 0xff);
    /* Other parameters */
    ptStr->ucControl = 0;

    return( lStatus );
}
