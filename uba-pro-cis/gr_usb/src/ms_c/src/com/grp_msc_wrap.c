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
/*      grp_msc_wrap.c                                                          1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST# Mass Storage Class Driver.                                                 */
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

/***** INCLUDE FILES ****************************************************************************/
#include "grp_msc_local.h"


/***** LOCAL PARAMETER DEFINITIONS **************************************************************/


/***** LOCAL FUNCTION PROTOTYPES ****************************************************************/


/************************************************************************************************/
/* FUNCTION     : grp_msc_ReadSector                                                            */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptCmd                         Request command                                 */
/*                  hMscHdr                     MSC handler                                     */
/*                  ucLun                       Logical unit number                             */
/*                  pucReqBuffer                Buffer for read                                 */
/*                  ulRequestLength             Length for read                                 */
/*                  pfnCallback                 Callback function                               */
/*                  pvUserRef                   Pointer for user                                */
/*                ulStartLba                    Start LBA for read                              */
/* OUTPUT       : ptCmd                                                                         */
/*                  lStatus                     Status of read                                  */
/*                  ulActualLength              Actual length of read                           */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_FAIL                  Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
grp_s32 grp_msc_ReadSector( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize )
{
grp_msc_set                     *ptSet;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_WRAP_(0x01, 0x00, 0x00);

    grp_std_memset( ptCmd->aucCmdContent, 0, GRP_MSC_CMD_LENGTH );

    ptSet = ptCmd->hMscHdr;
    switch( ptSet->ucSubClass ){
#ifdef GRP_MSC_USE_ATAPI
    case GRP_MSC_ATAPI_CODE:
        lStatus = _msc_AtapiReadSector( ptCmd, ulStartLba, ulSectorSize, 
                                                        &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_ATAPI */

#ifdef GRP_MSC_USE_SFF8070I
    case GRP_MSC_SFF8070I_CODE:
        lStatus = _msc_Sff8070iReadSector( ptCmd, ulStartLba, ulSectorSize, 
                                                        &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SFF8070I */

#ifdef GRP_MSC_USE_SCSI
    case GRP_MSC_SCSI_CODE:
        lStatus = _msc_ScsiReadSector( ptCmd, ulStartLba, ulSectorSize, 
                                                        &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SCSI */

#ifdef GRP_MSC_USE_UFI
    case GRP_MSC_UFI_CODE:
        lStatus = _msc_UfiReadSector( ptCmd, ulStartLba, ulSectorSize, 
                                                        &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_UFI */

    default:
        lStatus = GRP_MSC_ERROR;
        break;
    }

    if( lStatus == GRP_MSC_OK ){
        ptCmd->ucDir = GRP_USBD_TX_IN;
        lStatus = grp_msc_ReqCmd( ptCmd );
    }

    _TRACE_MSC_WRAP_(0x01, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_WriteSector                                                           */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptCmd                         Request command                                 */
/*                  hMscHdr                     MSC handler                                     */
/*                  ucLun                       Logical unit number                             */
/*                  pucReqBuffer                Buffer for write                                */
/*                  ulRequestLength             Length for write                                */
/*                  pfnCallback                 Callback function                               */
/*                  pvUserRef                   Pointer for user                                */
/*                ulStartLba                    Start LBA for write                             */
/* OUTPUT       : ptCmd                                                                         */
/*                  lStatus                     Status of write                                 */
/*                  ulActualLength              Actual length of write                          */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_FAIL                  Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
grp_s32 grp_msc_WriteSector( grp_msc_cmd *ptCmd, grp_u32 ulStartLba, grp_u32 ulSectorSize )
{
grp_msc_set                     *ptSet;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_WRAP_(0x02, 0x00, 0x00);

    grp_std_memset( ptCmd->aucCmdContent, 0, GRP_MSC_CMD_LENGTH );

    ptSet = ptCmd->hMscHdr;
    switch( ptSet->ucSubClass ){
#ifdef GRP_MSC_USE_ATAPI
    case GRP_MSC_ATAPI_CODE:
        lStatus = _msc_AtapiWriteSector( ptCmd, ulStartLba, ulSectorSize, 
                                                        &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_ATAPI */

#ifdef GRP_MSC_USE_SFF8070I
    case GRP_MSC_SFF8070I_CODE:
        lStatus = _msc_Sff8070iWriteSector( ptCmd, ulStartLba, ulSectorSize, 
                                                        &ptCmd->ulCmdLength,  ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SFF8070I */

#ifdef GRP_MSC_USE_SCSI
    case GRP_MSC_SCSI_CODE:
        lStatus = _msc_ScsiWriteSector( ptCmd, ulStartLba, ulSectorSize, 
                                                        &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SCSI */

#ifdef GRP_MSC_USE_UFI
    case GRP_MSC_UFI_CODE:
        lStatus = _msc_UfiWriteSector( ptCmd, ulStartLba, ulSectorSize, 
                                                        &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_UFI */

    default:
        lStatus = GRP_MSC_ERROR;
        break;
    }

    if( lStatus == GRP_MSC_OK ){
        ptCmd->ucDir = GRP_USBD_TX_OUT;
        lStatus = grp_msc_ReqCmd( ptCmd );
    }

    _TRACE_MSC_WRAP_(0x02, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_Inquiry                                                               */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptCmd                         Request command                                 */
/*                  hMscHdr                     MSC handler                                     */
/*                  ucLun                       Logical unit number                             */
/*                  pucReqBuffer                Buffer for inquiry                              */
/*                  ulRequestLength             Length for inquiry                              */
/*                  pfnCallback                 Callback function                               */
/*                  pvUserRef                   Pointer for user                                */
/* OUTPUT       : ptCmd                                                                         */
/*                  lStatus                     Status of inquiry                               */
/*                  ulActualLength              Actual length of inquiry                        */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_FAIL                  Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
grp_s32 grp_msc_Inquiry( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptSet;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_WRAP_(0x03, 0x00, 0x00);

    grp_std_memset( ptCmd->aucCmdContent, 0, GRP_MSC_CMD_LENGTH );

    ptSet = ptCmd->hMscHdr;
    switch( ptSet->ucSubClass ){
#ifdef GRP_MSC_USE_ATAPI
    case GRP_MSC_ATAPI_CODE:
        lStatus = _msc_AtapiInquiry( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_ATAPI */

#ifdef GRP_MSC_USE_SFF8070I
    case GRP_MSC_SFF8070I_CODE:
        lStatus = _msc_Sff8070iInquiry( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SFF8070I */

#ifdef GRP_MSC_USE_SCSI
    case GRP_MSC_SCSI_CODE:
        lStatus = _msc_ScsiInquiry( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SCSI */

#ifdef GRP_MSC_USE_UFI
    case GRP_MSC_UFI_CODE:
        lStatus = _msc_UfiInquiry( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_UFI */

    default:
        lStatus = GRP_MSC_ERROR;
        break;
    }

    if( lStatus == GRP_MSC_OK ){
        ptCmd->ucDir = GRP_USBD_TX_IN;
        lStatus = grp_msc_ReqCmd( ptCmd );
    }

    _TRACE_MSC_WRAP_(0x03, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_ReadFormatCapacity                                                    */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptCmd                         Request command                                 */
/*                  hMscHdr                     MSC handler                                     */
/*                  ucLun                       Logical unit number                             */
/*                  pucReqBuffer                Buffer for format capacity                      */
/*                  ulRequestLength             Length for format capacity                      */
/*                  pfnCallback                 Callback function                               */
/*                  pvUserRef                   Pointer for user                                */
/* OUTPUT       : ptCmd                                                                         */
/*                  lStatus                     Status of format capacity                       */
/*                  ulActualLength              Actual length of format capacity                */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_FAIL                  Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
grp_s32 grp_msc_ReadFormatCapacity( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptSet;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_WRAP_(0x04, 0x00, 0x00);

    grp_std_memset( ptCmd->aucCmdContent, 0, GRP_MSC_CMD_LENGTH );

    ptSet = ptCmd->hMscHdr;
    switch( ptSet->ucSubClass ){
#ifdef GRP_MSC_USE_ATAPI
    case GRP_MSC_ATAPI_CODE:
        lStatus = _msc_AtapiReadFormatCapacity( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_ATAPI */

#ifdef GRP_MSC_USE_SFF8070I
    case GRP_MSC_SFF8070I_CODE:
        lStatus = _msc_Sff8070iReadFormatCapacity( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SFF8070I */

#ifdef GRP_MSC_USE_SCSI
    case GRP_MSC_SCSI_CODE:
        lStatus = _msc_ScsiReadFormatCapacity( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SCSI */

#ifdef GRP_MSC_USE_UFI
    case GRP_MSC_UFI_CODE:
        lStatus = _msc_UfiReadFormatCapacity( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_UFI */

    default:
        lStatus = GRP_MSC_ERROR;
        break;
    }

    if( lStatus == GRP_MSC_OK ){
        ptCmd->ucDir = GRP_USBD_TX_IN;
        lStatus = grp_msc_ReqCmd( ptCmd );
    }

    _TRACE_MSC_WRAP_(0x04, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_ReadCapacity                                                          */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptCmd                         Request command                                 */
/*                  hMscHdr                     MSC handler                                     */
/*                  ucLun                       Logical unit number                             */
/*                  pucReqBuffer                Buffer for capacity                             */
/*                  ulRequestLength             Length for capacity                             */
/*                  pfnCallback                 Callback function                               */
/*                  pvUserRef                   Pointer for user                                */
/* OUTPUT       : ptCmd                                                                         */
/*                  lStatus                     Status of capacity                              */
/*                  ulActualLength              Actual length of capacity                       */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_FAIL                  Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
grp_s32 grp_msc_ReadCapacity( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptSet;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_WRAP_(0x05, 0x00, 0x00);

    grp_std_memset( ptCmd->aucCmdContent, 0, GRP_MSC_CMD_LENGTH );

    ptSet = ptCmd->hMscHdr;
    switch( ptSet->ucSubClass ){
#ifdef GRP_MSC_USE_ATAPI
    case GRP_MSC_ATAPI_CODE:
        lStatus = _msc_AtapiReadCapacity( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_ATAPI */

#ifdef GRP_MSC_USE_SFF8070I
    case GRP_MSC_SFF8070I_CODE:
        lStatus = _msc_Sff8070iReadCapacity( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SFF8070I */

#ifdef GRP_MSC_USE_SCSI
    case GRP_MSC_SCSI_CODE:
        lStatus = _msc_ScsiReadCapacity( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SCSI */

#ifdef GRP_MSC_USE_UFI
    case GRP_MSC_UFI_CODE:
        lStatus = _msc_UfiReadCapacity( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_UFI */

    default:
        lStatus = GRP_MSC_ERROR;
        break;
    }

    if( lStatus == GRP_MSC_OK ){
        ptCmd->ucDir = GRP_USBD_TX_IN;
        lStatus = grp_msc_ReqCmd( ptCmd );
    }

    _TRACE_MSC_WRAP_(0x05, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_ModeSense                                                             */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptCmd                         Request command                                 */
/*                  hMscHdr                     MSC handler                                     */
/*                  ucLun                       Logical unit number                             */
/*                  pucReqBuffer                Buffer for mode sense                           */
/*                  ulRequestLength             Length for mode sense                           */
/*                  pfnCallback                 Callback function                               */
/*                  pvUserRef                   Pointer for user                                */
/*                ucPage                        Page code                                       */
/* OUTPUT       : ptCmd                                                                         */
/*                  lStatus                     Status                                          */
/*                  ulActualLength              Actual length                                   */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_FAIL                  Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
grp_s32 grp_msc_ModeSense( grp_msc_cmd *ptCmd, grp_u8 ucPage )
{
grp_msc_set                     *ptSet;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_WRAP_(0x06, 0x00, 0x00);

    grp_std_memset( ptCmd->aucCmdContent, 0, GRP_MSC_CMD_LENGTH );

    ptSet = ptCmd->hMscHdr;
    switch( ptSet->ucSubClass ){
#ifdef GRP_MSC_USE_ATAPI
    case GRP_MSC_ATAPI_CODE:
        lStatus = _msc_AtapiModeSense( ptCmd, ucPage, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_ATAPI */

#ifdef GRP_MSC_USE_SFF8070I
    case GRP_MSC_SFF8070I_CODE:
        lStatus = _msc_Sff8070iModeSense( ptCmd, ucPage, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SFF8070I */

#ifdef GRP_MSC_USE_SCSI
    case GRP_MSC_SCSI_CODE:
        lStatus = _msc_ScsiModeSense( ptCmd, ucPage, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SCSI */

#ifdef GRP_MSC_USE_UFI
    case GRP_MSC_UFI_CODE:
        lStatus = _msc_UfiModeSense( ptCmd, ucPage, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_UFI */

    default:
        lStatus = GRP_MSC_ERROR;
        break;
    }

    if( lStatus == GRP_MSC_OK ){
        ptCmd->ucDir = GRP_USBD_TX_IN;
        lStatus = grp_msc_ReqCmd( ptCmd );
    }

    _TRACE_MSC_WRAP_(0x06, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_TestUnitReady                                                         */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptCmd                         Request command                                 */
/*                  hMscHdr                     MSC handler                                     */
/*                  ucLun                       Logical unit number                             */
/*                  pucReqBuffer                Buffer for read                                 */
/*                  pfnCallback                 Callback function                               */
/*                  pvUserRef                   Pointer for user                                */
/* OUTPUT       : ptCmd                                                                         */
/*                  lStatus                     Status of read                                  */
/*                  ulActualLength              Actual length of read                           */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_FAIL                  Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
grp_s32 grp_msc_TestUnitReady( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptSet;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_WRAP_(0x07, 0x00, 0x00);

    grp_std_memset( ptCmd->aucCmdContent, 0, GRP_MSC_CMD_LENGTH );

    ptSet = ptCmd->hMscHdr;
    switch( ptSet->ucSubClass ){
#ifdef GRP_MSC_USE_ATAPI
    case GRP_MSC_ATAPI_CODE:
        lStatus = _msc_AtapiTestUnitReady( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_ATAPI */

#ifdef GRP_MSC_USE_SFF8070I
    case GRP_MSC_SFF8070I_CODE:
        lStatus = _msc_Sff8070iTestUnitReady( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SFF8070I */

#ifdef GRP_MSC_USE_SCSI
    case GRP_MSC_SCSI_CODE:
        lStatus = _msc_ScsiTestUnitReady( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SCSI */

#ifdef GRP_MSC_USE_UFI
    case GRP_MSC_UFI_CODE:
        lStatus = _msc_UfiTestUnitReady( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_UFI */

    default:
        lStatus = GRP_MSC_ERROR;
        break;
    }

    if( lStatus == GRP_MSC_OK ){
        ptCmd->ulReqLength = 0;
        lStatus = grp_msc_ReqCmd( ptCmd );
    }

    _TRACE_MSC_WRAP_(0x07, 0x00, _F_END);

    return( lStatus );
}

/************************************************************************************************/
/* FUNCTION     : grp_msc_RequestSense                                                          */
/*                                                                                              */
/* DESCRIPTION  :                                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT        : ptCmd                         Request command                                 */
/*                  hMscHdr                     MSC handler                                     */
/*                  ucLun                       Logical unit number                             */
/*                  pucReqBuffer                Buffer for capacity                             */
/*                  ulRequestLength             Length for capacity                             */
/*                  pfnCallback                 Callback function                               */
/*                  pvUserRef                   Pointer for user                                */
/* OUTPUT       : ptCmd                                                                         */
/*                  lStatus                     Status of capacity                              */
/*                  ulActualLength              Actual length of capacity                       */
/*                                                                                              */
/* RETURN       : GRP_MSC_OK                    Successful                                      */
/*                GRP_MSC_FAIL                  Failed                                          */
/*                                                                                              */
/* EFFECT       : none                                                                          */
/************************************************************************************************/
grp_s32 grp_msc_RequestSense( grp_msc_cmd *ptCmd )
{
grp_msc_set                     *ptSet;
grp_s32                         lStatus = GRP_MSC_OK;

    _TRACE_MSC_WRAP_(0x08, 0x00, 0x00);

    grp_std_memset( ptCmd->aucCmdContent, 0, GRP_MSC_CMD_LENGTH );

    ptSet = ptCmd->hMscHdr;
    switch( ptSet->ucSubClass ){
#ifdef GRP_MSC_USE_ATAPI
    case GRP_MSC_ATAPI_CODE:
        lStatus = _msc_AtapiRequestSense( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_ATAPI */

#ifdef GRP_MSC_USE_SFF8070I
    case GRP_MSC_SFF8070I_CODE:
        lStatus = _msc_Sff8070iRequestSense( ptCmd, &ptCmd->ulCmdLength,  ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SFF8070I */

#ifdef GRP_MSC_USE_SCSI
    case GRP_MSC_SCSI_CODE:
        lStatus = _msc_ScsiRequestSense( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_SCSI */

#ifdef GRP_MSC_USE_UFI
    case GRP_MSC_UFI_CODE:
        lStatus = _msc_UfiRequestSense( ptCmd, &ptCmd->ulCmdLength, ptCmd->aucCmdContent );
        break;
#endif /* GRP_MSC_USE_UFI */

    default:
        lStatus = GRP_MSC_ERROR;
        break;
    }

    if( lStatus == GRP_MSC_OK ){
        ptCmd->ucDir = GRP_USBD_TX_IN;
        lStatus = grp_msc_ReqCmd( ptCmd );
    }

    _TRACE_MSC_WRAP_(0x08, 0x00, _F_END);

    return( lStatus );
}
