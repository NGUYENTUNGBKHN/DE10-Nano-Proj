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
/*      grp_msc_sff8070i.c                                                      1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver SFF8070i Sub Class.                              */
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
#include "grp_msc_sff8070i.h"
#include "grp_sff8070i_local.h"

/***** LOCAL PARAMETER DEFINITIONS **************************************************************/


/***** LOCAL FUNCTION PROTOTYPES ****************************************************************/
LOCAL grp_s32 _msc_Sff8070iReadSector10( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent );
LOCAL grp_s32 _msc_Sff8070iReadSector12( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent );
LOCAL grp_s32 _msc_Sff8070iWriteSector10( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent );
LOCAL grp_s32 _msc_Sff8070iWriteSector12( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent );

/************************************************************************************************/
/* FUNCTION     : grp_msc_Sff8070iInit                                                          */
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
grp_s32 grp_msc_Sff8070iInit( void )
{
grp_s32                         lStatus = GRP_MSC_OK;

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_Sff8070iReadSector                                                       */
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
grp_s32 _msc_Sff8070iReadSector( grp_msc_cmd *ptCmd,grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
grp_s32                         lStatus;

    if( ulSectorSize > GRP_USBC_MAX_US ){
        lStatus = _msc_Sff8070iReadSector12( ptCmd, ulStartLba, ulSectorSize, pulLength, pucContent );
    }
    else{
        lStatus = _msc_Sff8070iReadSector10( ptCmd, ulStartLba, ulSectorSize, pulLength, pucContent );
    }

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_Sff8070iWriteSector                                                      */
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
grp_s32 _msc_Sff8070iWriteSector( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
grp_s32                         lStatus;

    if( ulSectorSize > GRP_USBC_MAX_US ){
        lStatus = _msc_Sff8070iWriteSector12( ptCmd, ulStartLba, ulSectorSize, pulLength, pucContent );
    }
    else{
        lStatus = _msc_Sff8070iWriteSector10( ptCmd, ulStartLba, ulSectorSize, pulLength, pucContent );
    }

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_Sff8070iInquiry                                                          */
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
grp_s32 _msc_Sff8070iInquiry( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
SFF8070I_INQUIRY_STR            *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SFF8070I_INQUIRY_SZ;

    pucStr = (SFF8070I_INQUIRY_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SFF8070I_INQUIRY_CMD;
    /* Allocation Length */
    if( ptCmd->ulReqLength > GRP_USBC_OVER_UC ){
        ptCmd->ulReqLength = GRP_USBC_MAX_UC;
    }
    pucStr->ucAllocationLen = (grp_u8)ptCmd->ulReqLength;
    /* Logical unit number */
    pucStr->ucLun = (grp_u8)((grp_u32)ptCmd->ucLun << 5);
    /* Other parameters */

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_Sff8070iReadFormatCapacity                                               */
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
grp_s32 _msc_Sff8070iReadFormatCapacity( grp_msc_cmd *ptCmd,grp_u32 *pulLength, grp_u8 *pucContent )
{
SFF8070I_READ_FORMAT_CAPA_STR   *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SFF8070I_READ_FORMAT_CAPA_SZ;

    pucStr = (SFF8070I_READ_FORMAT_CAPA_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SFF8070I_READ_FORMAT_CAPA_CMD;
    /* Data size */
    if( ptCmd->ulReqLength > GRP_USBC_OVER_US ){
        ptCmd->ulReqLength = GRP_USBC_MAX_US;
    }
    pucStr->aucAllocationLen[0] = (grp_u8)((grp_u32)(ptCmd->ulReqLength>> 8) & 0xff);
    pucStr->aucAllocationLen[1] = (grp_u8)((grp_u32)(ptCmd->ulReqLength>> 0) & 0xff);
    /* Logical unit number */
    pucStr->ucLun = (grp_u8)((grp_u32)ptCmd->ucLun << 5);
    /* Other parameters */

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_Sff8070iReadCapacity                                                     */
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
grp_s32 _msc_Sff8070iReadCapacity( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
SFF8070I_READ_CAPACITY_STR      *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SFF8070I_READ_CAPACITY_SZ;

    pucStr = (SFF8070I_READ_CAPACITY_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SFF8070I_READ_CAPACITY_CMD;
    /* Logical unit number */
    pucStr->ucLun = (grp_u8)((grp_u32)ptCmd->ucLun << 5);
    /* Other parameters */

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_Sff8070iModeSense                                                        */
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
grp_s32 _msc_Sff8070iModeSense( grp_msc_cmd *ptCmd, grp_u8 ucPage, grp_u32 *pulLength, grp_u8 *pucContent )
{
SFF8070I_MODE_SENSE_STR         *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SFF8070I_MODE_SENSE_SZ;

    pucStr = (SFF8070I_MODE_SENSE_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SFF8070I_MODE_SENSE_CMD;
    /* Page code */
    pucStr->ucPcPage  = ucPage;                     /* Bit7-6:Page Control  */
                                                    /* Bit5-0:Page code     */
    /* Allocation Length */
    pucStr->aucAllocationLen[0] = (grp_u8)((grp_u32)(ptCmd->ulReqLength>> 8) & 0xff);
    pucStr->aucAllocationLen[1] = (grp_u8)((grp_u32)(ptCmd->ulReqLength>> 0) & 0xff);
    /* Logical unit number */
    pucStr->ucLun = (grp_u8)((grp_u32)ptCmd->ucLun << 5);
    /* Other parameters */

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_Sff8070iTestUnitReady                                                    */
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
grp_s32 _msc_Sff8070iTestUnitReady( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
SFF8070I_TEST_UNIT_READY_STR    *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SFF8070I_TEST_UNIT_READY_SZ;

    pucStr = (SFF8070I_TEST_UNIT_READY_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SFF8070I_TEST_UNIT_READY_CMD;
    /* Logical unit number */
    pucStr->ucLun = (grp_u8)((grp_u32)ptCmd->ucLun << 5);
    /* Other parameters */

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_Sff8070iRequestSense                                                     */
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
grp_s32 _msc_Sff8070iRequestSense( grp_msc_cmd *ptCmd, grp_u32 *pulLength, grp_u8 *pucContent )
{
SFF8070I_REQUEST_SENSE_STR      *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SFF8070I_REQUEST_SENSE_SZ;

    pucStr = (SFF8070I_REQUEST_SENSE_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SFF8070I_REQUEST_SENSE_CMD;
    /* Allocation Length */
    pucStr->ucAllocationLen = (grp_u8)(ptCmd->ulReqLength & 0xff);
    /* Logical unit number */
    pucStr->ucLun = (grp_u8)((grp_u32)ptCmd->ucLun << 5);
    /* Other parameters */

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_Sff8070iReadSector10                                                     */
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
LOCAL grp_s32 _msc_Sff8070iReadSector10( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
SFF8070I_READ10_STR             *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SFF8070I_READ10_SZ;

    pucStr = (SFF8070I_READ10_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SFF8070I_READ10_CMD;
    /* Block address */
    pucStr->aucLba[0] = (grp_u8)((grp_u32)(ulStartLba>>24) & 0xff);
    pucStr->aucLba[1] = (grp_u8)((grp_u32)(ulStartLba>>16) & 0xff);
    pucStr->aucLba[2] = (grp_u8)((grp_u32)(ulStartLba>> 8) & 0xff);
    pucStr->aucLba[3] = (grp_u8)((grp_u32)(ulStartLba>> 0) & 0xff);
    /* Reading sector size */
    pucStr->aucTransferLen[0] = (grp_u8)((grp_u32)(ulSectorSize>> 8) & 0xff);
    pucStr->aucTransferLen[1] = (grp_u8)((grp_u32)(ulSectorSize>> 0) & 0xff);
    /* Logical unit number */
    pucStr->ucLun = (grp_u8)((grp_u32)ptCmd->ucLun << 5);
    /* Other parameters */

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_Sff8070iReadSector12                                                     */
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
LOCAL grp_s32 _msc_Sff8070iReadSector12( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
SFF8070I_READ12_STR             *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SFF8070I_READ12_SZ;

    pucStr = (SFF8070I_READ12_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SFF8070I_READ12_CMD;
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
    /* Logical unit number */
    pucStr->ucLun = (grp_u8)((grp_u32)ptCmd->ucLun << 5);
    /* Other parameters */

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_Sff8070iWriteSector10                                                    */
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
LOCAL grp_s32 _msc_Sff8070iWriteSector10( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
SFF8070I_WRITE10_STR            *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SFF8070I_WRITE10_SZ;

    pucStr = (SFF8070I_WRITE10_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SFF8070I_WRITE10_CMD;
    /* Block address */
    pucStr->aucLba[0] = (grp_u8)((grp_u32)(ulStartLba>>24) & 0xff);
    pucStr->aucLba[1] = (grp_u8)((grp_u32)(ulStartLba>>16) & 0xff);
    pucStr->aucLba[2] = (grp_u8)((grp_u32)(ulStartLba>> 8) & 0xff);
    pucStr->aucLba[3] = (grp_u8)((grp_u32)(ulStartLba>> 0) & 0xff);
    /* Reading sector size */
    pucStr->aucTransferLen[0] = (grp_u8)((grp_u32)(ulSectorSize>> 8) & 0xff);
    pucStr->aucTransferLen[1] = (grp_u8)((grp_u32)(ulSectorSize>> 0) & 0xff);
    /* Logical unit number */
    pucStr->ucLun = (grp_u8)((grp_u32)ptCmd->ucLun << 5);
    /* Other parameters */

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : _msc_Sff8070iWriteSector                                                      */
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
LOCAL grp_s32 _msc_Sff8070iWriteSector12( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize, grp_u32 *pulLength, grp_u8 *pucContent )
{
SFF8070I_WRITE12_STR            *pucStr;
grp_s32                         lStatus = GRP_MSC_OK;

    *pulLength = SFF8070I_WRITE12_SZ;

    pucStr = (SFF8070I_WRITE12_STR *)pucContent;
    /* Operation code */
    pucStr->ucCommand = SFF8070I_WRITE12_CMD;
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
    /* Logical unit number */
    pucStr->ucLun = (grp_u8)((grp_u32)ptCmd->ucLun << 5);
    /* Other parameters */

    return( lStatus );
}
