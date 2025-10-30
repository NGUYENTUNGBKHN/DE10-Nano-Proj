/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2014-2015 Grape Systems, Inc.                        */
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
/*      grp_cyclonevd_hal.c                                                     1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/DEVICE for CycloneV(OTG) - Hardware Abstraction Layer module                     */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   T.Kobayashi    2014/12/24  V0.90                                                           */
/*                            - Created beta release version.                                   */
/*   T.Kobayashi    2015/01/21  V1.00                                                           */
/*                            - 1st release version.                                            */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include <string.h>
#include "grp_cyclonev_cmod.h"
#include "grp_cyclonev_macro.h"
#include "grp_cyclonev_bit_val.h"
#include "grp_cyclonev_reg.h"

#include "perid.h"
#include "peri_ctl.h"
#include "peri_prm.h"
#include "peri_sts.h"
#include "peri_hal.h"

#include "grp_cyclonevd_hal.h"
#include "grp_cyclonevd_cfg.h"
#include "grp_cyclonev_tg.h"


/**** LOCAL DATA DEFINES ************************************************************************/
/* Receive information */
DLOCAL grp_cyclonevd_rcv_info  l_atRcvInf2[GRP_CYCLONEVD_EP_NUM];
/* Pull-up status */
DLOCAL BOOLEAN                  l_bPUStatus2;
/* Enumeration completion status */
DLOCAL BOOLEAN                  l_bECStatus2;
/* Tx FIFO buffer size */
DLOCAL UINT32                   l_aulTxFifoSiz2[GRP_CYCLONEVD_EP_NUM] = {
                                    GRP_CYCLONEVD_TX_FIFO_0_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_1_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_2_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_3_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_4_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_5_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_6_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_7_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_8_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_9_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_10_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_11_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_12_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_13_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_14_SIZE,
                                    GRP_CYCLONEVD_TX_FIFO_15_SIZE };
/* Test mode selector value */
DLOCAL UINT16                   l_usTestMode2;
/* CONTROL transfer state */
DLOCAL UINT16                   l_usCtrlSts2 = GRP_CYCLONEVD_CTLST_IDLE;
/* Setup packet buffer */
DLOCAL UINT8                    l_aucSetupPkt2[GRP_CYCLONEVD_BUF_SIZE];
/* CONTROL send data buffer */
DLOCAL UINT8                    l_aucCtrlData2[GRP_CYCLONEVD_TX_FIFO_0_SIZE];
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
/* CONTROL send data buffer valid flag */
DLOCAL BOOLEAN                  l_bCtrlDataValidFlag2;
#endif


/**** CALLBACK FUNCTIONS ************************************************************************/
DLOCAL GRUSB_Notice             l_pfnCbBusReset2        = GRUSB_NULL;  /* BUS_RESET              */
DLOCAL GRUSB_Notice             l_pfnCbSuspend2         = GRUSB_NULL;  /* SUSPEND                */
DLOCAL GRUSB_Notice             l_pfnCbResume2          = GRUSB_NULL;  /* RESUME                 */
DLOCAL GRUSB_Notice             l_pfnCbCmpSetIf2        = GRUSB_NULL;  /* SET_INTERFACE          */
DLOCAL GRUSB_Notice             l_pfnCbCmpSetConf2      = GRUSB_NULL;  /* SET_CONFIGURETION      */
DLOCAL GRUSB_CtrTransferCancel  l_pfnCbCtrTransferCncl2 = GRUSB_NULL;  /* CONTROL send cancel    */
DLOCAL GRUSB_CtrReceiveCancel   l_pfnCbCtrReceiveCncl2  = GRUSB_NULL;  /* CONTROL receive cancel */


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL VOID    _grp_cyclonevd_hal_ResetSetting2( VOID );
LOCAL VOID    _grp_cyclonevd_hal_SetFifoSize2( VOID );
LOCAL BOOLEAN _grp_cyclonevd_hal_EndpointActivation2( INT );
LOCAL BOOLEAN _grp_cyclonevd_hal_EndpointDeactivation2( INT );
LOCAL VOID    _grp_cyclonevd_hal_SetEP0StallState2( INT );
LOCAL VOID    _grp_cyclonevd_hal_SetEPxStallState2( INT, INT );
LOCAL BOOLEAN _grp_cyclonevd_hal_CheckBusSpeed2( VOID );
LOCAL BOOLEAN _grp_cyclonevd_hal_GetEndpointType2( INT, UINT32*, UINT8* );
LOCAL VOID    _grp_cyclonevd_hal_Attach2( VOID );
LOCAL VOID    _grp_cyclonevd_hal_Detach2( VOID );
LOCAL VOID    _grp_cyclonevd_hal_GetDeviceRequest2( DEVICE_REQUEST_INFO* );
LOCAL VOID    _grp_cyclonevd_hal_CtrlStageTransition2( GRUSB_EndPointInfo*, DEVICE_REQUEST_INFO* );
LOCAL VOID    _grp_cyclonevd_hal_CtrlDataOutStage2( GRUSB_EndPointInfo* );
LOCAL VOID    _grp_cyclonevd_hal_CtrlDataInStage2( GRUSB_EndPointInfo* );
LOCAL VOID    _grp_cyclonevd_hal_CtrlStatusOutStage2( VOID );
LOCAL VOID    _grp_cyclonevd_hal_CtrlStatusInStage2( VOID );
LOCAL UINT16  _grp_cyclonevd_hal_DataRecv2( GRUSB_EndPointInfo* );
LOCAL VOID    _grp_cyclonevd_hal_DataSend2( GRUSB_EndPointInfo* );
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
LOCAL VOID    _grp_cyclonevd_hal_ReadRecvData2( GRUSB_EndPointInfo*, UINT32 );
LOCAL VOID    _grp_cyclonevd_hal_WriteSendData2( GRUSB_EndPointInfo*, UINT32 );
LOCAL VOID    _grp_cyclonevd_hal_ReadFifo2( INT, UINT8*, UINT32 );
LOCAL VOID    _grp_cyclonevd_hal_WriteFifo2( INT, UINT8*, UINT32 );
#endif
LOCAL VOID    _grp_cyclonevd_hal_ReqRecvSetupPacket2( VOID );
LOCAL VOID    _grp_cyclonevd_hal_ReqCtrlRecvData2( UINT8*, UINT32 );
LOCAL VOID    _grp_cyclonevd_hal_ReqCtrlSendData2( UINT8*, UINT32 );
LOCAL VOID    _grp_cyclonevd_hal_ReqRecvData2( INT, UINT8*, UINT32 );
LOCAL VOID    _grp_cyclonevd_hal_ReqSendData2( INT, UINT8*, UINT32 );
LOCAL VOID    _grp_cyclonevd_hal_ClearRecvInfo2( GRUSB_EndPointInfo* );
LOCAL VOID    _grp_cyclonevd_hal_ClearSendInfo2( GRUSB_EndPointInfo* );
LOCAL UINT32  _grp_cyclonevd_hal_GetRecvDataSize2( INT );
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
LOCAL UINT32  _grp_cyclonevd_hal_GetSendDataSize2( INT );
#endif
LOCAL BOOLEAN _grp_cyclonevd_hal_StartTestMode2( VOID );

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALInit2                                                              */
/*                                                                                              */
/* DESCRIPTION: Initialize HAL (only start up)                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALInit2( VOID )
{
UINT32                  ulDThrCtl;
    
    /* Initialize test mode selector value */
    l_usTestMode2 = GRP_CYCLONEVD_TM_DISABLED;
    
#if 1  /* Added the same as USB0 */
    /* Clear device speed */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_DCFG2, CYCLONEVD_B02_DEVSPD );
#endif /* Added the same as USB0 */

    /* Set device speed */
    CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DCFG2, CYCLONEVD_VDEVSPD_USBHS20 );
    
    /* Setup the Data FIFO RAM size */
    _grp_cyclonevd_hal_SetFifoSize2();
    
    /* Reset setting */
    _grp_cyclonevd_hal_ResetSetting2();
    
    /* Read device threshold control register */
    ulDThrCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DTHRCTL2 );
    
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
    /* CONTROL send data buffer invalid */
    l_bCtrlDataValidFlag2 = GRUSB_FALSE;
    /* Disable DMA arbiter parking */
    ulDThrCtl &= ~CYCLONEVD_B01_ARBPRKEN;
#else
    /* Set nonisochronous IN endpoints threshold enable */
    ulDThrCtl |= CYCLONEVD_B01_NONISOTHREN;
    /* Clear transmit threshold length */
    ulDThrCtl &= ~CYCLONEVD_B09_TXTHRLEN;
    /* Set transmit threshold length */
    ulDThrCtl |= ( GRP_CYCLONEVD_TX_THR_LEN << 2 );
    /* Set receive threshold enable */
    ulDThrCtl |= CYCLONEVD_B01_RXTHREN;
    /* Clear receive threshold length */
    ulDThrCtl &= ~CYCLONEVD_B09_RXTHRLEN;
    /* Set receive threshold length */
    ulDThrCtl |= ( GRP_CYCLONEVD_RX_THR_LEN << 17 );
#endif
    
    /* Write device threshold control register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DTHRCTL2, ulDThrCtl );
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALConnectController2                                                 */
/*                                                                                              */
/* DESCRIPTION: Processing when the device is connected to Host                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALConnectController2( VOID )
{
    /* Check VBUS Status */
    if( GRUSB_TRUE == grp_cyclonevd_hal_ChackVBusStatus2() )
    {
        /* CONNECT */
        _grp_cyclonevd_hal_Attach2();
    }
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALDisonnectController2                                               */
/*                                                                                              */
/* DESCRIPTION: Processing when the device is disconnected to Host                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALDisonnectController2( VOID )
{
    /* DISCONNECT */
    _grp_cyclonevd_hal_Detach2();
    
    /* Reset setting */
    _grp_cyclonevd_hal_ResetSetting2();
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALDataWrite2                                                         */
/*                                                                                              */
/* DESCRIPTION: Send data to controller (not CONTROL transfer)                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALDataWrite2( GRUSB_EndPointInfo* ptEpInfo )
{
INT                     iReadP;
GRUSB_SndDataInfo*      ptSndDtInfo;
    
    /* Check buffer is exist */
    iReadP = GRLIB_Cyclic_CheckRead2( &( ptEpInfo->tCycBufInfo ) );
    
    /* Check flag of sending information */
    if( GRUSB_TRUE == ptEpInfo->uInfo.tSndInfo.iFlag )
    {
        /* If application is already requesting to send */
        /* then no process is executed                  */
    }
    else if( GRLIB_NONBUFFER == iReadP )
    {
        /* Set flag of sending information to FALSE */
        ptEpInfo->uInfo.tSndInfo.iFlag = GRUSB_FALSE;
    }
    else
    {
        /* Get send data information */
        ptSndDtInfo = &( ptEpInfo->ptTrnsInfo[iReadP].tSndDtInfo );
        
        /* Check buffer start address alignment */
        if ( (UINT32)ptSndDtInfo->pucBuf & 3 )
        {
            /* Notice of buffer address error to upper layer */
            GRP_USBD_BUFFER_ADDRESS_ERROR2(ptEpInfo->iEpNo);
            
            /* Renew buffer pointer */
            GRLIB_Cyclic_IncRead2( &( ptEpInfo->tCycBufInfo ) );
        }
        else
        {
            /* Save current buffer pointer */
            memcpy( &( ptEpInfo->tCrntTrnsInfo.tSndDtInfo ), 
                    &( ptEpInfo->ptTrnsInfo[iReadP].tSndDtInfo ), 
                    sizeof(GRUSB_SndDataInfo) );
            
            /* Renew buffer pointer */
            GRLIB_Cyclic_IncRead2( &( ptEpInfo->tCycBufInfo ) );
            
            /* Renew sending information */
            ptEpInfo->uInfo.tSndInfo.ulSndSize = 0;
            ptEpInfo->uInfo.tSndInfo.iFlag     = GRUSB_TRUE;
            
            /* Data send */
            _grp_cyclonevd_hal_DataSend2( ptEpInfo );
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALDataFIFORcvStart2                                                  */
/*                                                                                              */
/* DESCRIPTION: Start interruption to receive data (not CONTROL transfer)                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALDataFIFORcvStart2( GRUSB_EndPointInfo* ptEpInfo )
{
INT                     iReadP;
GRUSB_RcvDataInfo*      ptRcvDtInfo;
    
    /* Check receiving flag whether this process passed */
    if( GRUSB_FALSE == l_atRcvInf2[ptEpInfo->iEpNo].bRcvFlag )
    {
        /* Check buffer is exist */
        iReadP = GRLIB_Cyclic_CheckRead2( &( ptEpInfo->tCycBufInfo ) );
        
        if( GRLIB_NONBUFFER != iReadP )
        {
            /* Get receive data information */
            ptRcvDtInfo = &( ptEpInfo->ptTrnsInfo[iReadP].tRcvDtInfo );
            
            /* Check buffer start address alignment */
            if ( (UINT32)ptRcvDtInfo->pucBuf & 3 )
            {
                /* Notice of buffer address error to upper layer */
                GRP_USBD_BUFFER_ADDRESS_ERROR2(ptEpInfo->iEpNo);
                
                /* Renew buffer pointer */
                GRLIB_Cyclic_IncRead2( &( ptEpInfo->tCycBufInfo ) );
            }
        }
        
        /* Set receiving flag to TRUE */
        l_atRcvInf2[ptEpInfo->iEpNo].bRcvFlag = GRUSB_TRUE;
        /* Set receiving status to "First receive transfer" */
        l_atRcvInf2[ptEpInfo->iEpNo].ulRcvStat = GRP_CYCLONEVD_RX_FIRST;
        
        /* Data receive */
        _grp_cyclonevd_hal_DataRecv2( ptEpInfo );
    }
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALCtrlRcv2                                                           */
/*                                                                                              */
/* DESCRIPTION: Wait to receive CONTROL transfer                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALCtrlRcv2( GRUSB_EndPointInfo* ptEpInfo )
{
    /* Check CONTROL transfer state */
    if( ( GRP_CYCLONEVD_CTLST_DATA | GRP_CYCLONEVD_CTLST_OUT | GRP_CYCLONEVD_CTLST_NO_BUF_WAIT ) == l_usCtrlSts2 )
    {
        /* Clear no buffer wait option */
        l_usCtrlSts2 &= ~GRP_CYCLONEVD_CTLST_NO_BUF_WAIT;
    }
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALSetStallState2                                                     */
/*                                                                                              */
/* DESCRIPTION: Set STALL status to the controller                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/*              iStatus                         Stall status        GRUSB_TRUE  : Set STALL     */
/*                                                                  GRUSB_FALSE : Clear STALL   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALSetStallState2( INT iEpNo, INT iStatus )
{
    /* Set STALL state to peripheral common module */
    GRUSB_DEV_StateSetStall2( iEpNo, iStatus );
    
    if( GRUSB_DEV_EP0 == iEpNo )
    {
        /* Set endpoint 0 STALL status */
        _grp_cyclonevd_hal_SetEP0StallState2( iStatus );
    }
    else
    {
        /* Set endpoint x STALL status */
        _grp_cyclonevd_hal_SetEPxStallState2( iEpNo, iStatus );
    }
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALSetAddress2                                                        */
/*                                                                                              */
/* DESCRIPTION: Set address to the controller                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usAddress                       Device address                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALSetAddress2( UINT16 usAddress )
{
    /* Reset device address */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_DCFG2, CYCLONEVD_B07_DEVADDR );
    
    /* Save the device address */
    CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DCFG2, ( ( usAddress << 4 ) & CYCLONEVD_B07_DEVADDR ) );
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALCallbackBusReset2                                                  */
/*                                                                                              */
/* DESCRIPTION: Register the callback function when BUS_RESET occurred                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pFunc                           Callback function                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALCallbackBusReset2( GRUSB_Notice pFunc )
{
    /* Save the pointer of callback function */
    l_pfnCbBusReset2 = pFunc;
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALCallbackSuspend2                                                   */
/*                                                                                              */
/* DESCRIPTION: Register the callback function when SUSPEND occurred                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pFunc                           Callback function                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALCallbackSuspend2( GRUSB_Notice pFunc )
{
    /* Save the pointer of callback function */
    l_pfnCbSuspend2 = pFunc;
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALCallbackResume2                                                    */
/*                                                                                              */
/* DESCRIPTION: Register the callback function when RESUME occurred                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pFunc                           Callback function                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALCallbackResume2( GRUSB_Notice pFunc )
{
    /* Save the pointer of callback function */
    l_pfnCbResume2 = pFunc;
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALCallbackCtrTransferCancel2                                         */
/*                                                                                              */
/* DESCRIPTION: Register the callback function to notice CONTROL transfer cancel                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pFunc                           Callback function                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALCallbackCtrTransferCancel2( GRUSB_CtrTransferCancel pFunc )
{
    /* Save the pointer of callback function */
    l_pfnCbCtrTransferCncl2 = pFunc;
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALCallbackCtrReceiveCancel2                                          */
/*                                                                                              */
/* DESCRIPTION: Register the callback function to notice CONTROL receiving cancel               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pFunc                           Callback function                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALCallbackCtrReceiveCancel2( GRUSB_CtrReceiveCancel pFunc )
{
    /* Save the pointer of callback function */
    l_pfnCbCtrReceiveCncl2 = pFunc;
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALTogleClear2                                                        */
/*                                                                                              */
/* DESCRIPTION: Clear toggle                                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALTogleClear2( INT iEpNo )
{
BOOLEAN                 bStat;
UINT32                  ulDEpCtl;
UINT32                  ulDEpCtlAddr;
UINT8                   ucType;
    
    if( GRUSB_DEV_EP0 != iEpNo )
    {
        /* Get endpoint type */
        bStat = GRUSB_Prm_GetEndPointType2( iEpNo, &ucType );
        
        if( GRUSB_TRUE != bStat )
        {
            /* Invalid endpoint */
            return;
        }
        
        /* Endpoint type is BULK or INTERRUPT */
        if( ( GRUSB_DEV_MAC_IsEpTypeBLK( ucType ) ) || ( GRUSB_DEV_MAC_IsEpTypeINTR( ucType ) ) )
        {
            /* Check endpoint direction */
            if( GRUSB_DEV_MAC_IsEpTypeIN( ucType ) )
            {
                /* Get device IN endpoint x control register address */
                ulDEpCtlAddr = CYCLONEV_A32_OTG_DIEPCTL2( iEpNo );
            }
            else
            {
                /* Get device OUT endpoint x control register address */
                ulDEpCtlAddr = CYCLONEV_A32_OTG_DOEPCTL2( iEpNo );
            }
            
            /* Read endpoint x control register */
            ulDEpCtl = CYCLONEV_R32_RD( ulDEpCtlAddr );
            
            /* Set DATA0 PID */
            ulDEpCtl |= CYCLONEVD_B01_SETD0PID;
            
            /* Write endpoint x control register */
            CYCLONEV_R32_WR( ulDEpCtlAddr, ulDEpCtl );
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALRmtWkup2                                                           */
/*                                                                                              */
/* DESCRIPTION: Execute RemoteWakeup processing                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALRmtWkup2( VOID )
{
    /* Not supported */
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALSndAbort2                                                          */
/*                                                                                              */
/* DESCRIPTION: Abort transmission of Control or data sending                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALSndAbort2( GRUSB_EndPointInfo* ptEpInfo )
{
UINT32                  ulDiEpCtl;
    
    /* Check status */
    if( ( GRUSB_TRUE  == grp_cyclonevd_hal_ChackVBusStatus2() )                  /* VBUS status  */
     && ( GRUSB_DEV_STATE_CONFIGURED == GRUSB_DEV_StateGetDeviceState2() ) )     /* Device state */
    {
        /* Read device IN endpoint x control register */
        ulDiEpCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DIEPCTL2( ptEpInfo->iEpNo ) );
        
        /* Set NAK */
        ulDiEpCtl |= CYCLONEVD_B01_SNAK;
        
        /* Write device IN endpoint x control register */
        CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPCTL2( ptEpInfo->iEpNo ), ulDiEpCtl );
        
        /* Unmask IN endpoint NAK effective interrupt */
        CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DIEPMSK2, CYCLONEVD_B01_INEPNAKEFFMSK );
    }
    else
    {
        /* Clear send information */
        _grp_cyclonevd_hal_ClearSendInfo2( ptEpInfo );
    }
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALRcvAbort2                                                          */
/*                                                                                              */
/* DESCRIPTION: Abort transmission of Control or data receiving                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALRcvAbort2( GRUSB_EndPointInfo* ptEpInfo )
{
    /* Check status */
    if( ( GRUSB_TRUE  == grp_cyclonevd_hal_ChackVBusStatus2() )                  /* VBUS status  */
     && ( GRUSB_DEV_STATE_CONFIGURED == GRUSB_DEV_StateGetDeviceState2() ) )     /* Device state */
    {
        /* Set Global OUT NAK */
        CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DCTL2, CYCLONEVD_B01_SGOUTNAK );
        
        /* Unmask Global OTU NAK effective interrupt */
        CYCLONEV_R32_ST( CYCLONEV_A32_OTG_GINTMSK2, CYCLONEVG_B01_GOUTNAKEFFMSK );
    }
    else
    {
        /* Clear receive information */
        _grp_cyclonevd_hal_ClearRecvInfo2( ptEpInfo );
    }
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALCallbackCmpSetIf2                                                  */
/*                                                                                              */
/* DESCRIPTION: Register the callback function that is called SET_INTERFACE is completed        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pFunc                           Callback function                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALCallbackCmpSetIf2( GRUSB_Notice pFunc )
{
    /* Save the pointer of callback function */
    l_pfnCbCmpSetIf2 = pFunc;
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALCallbackCmpSetConfig2                                              */
/*                                                                                              */
/* DESCRIPTION: Register the callback function that is called SET_CONFIGURATION is completed    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pFunc                           Callback function                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALCallbackCmpSetConfig2( GRUSB_Notice pFunc )
{
    /* Save the pointer of callback function */
    l_pfnCbCmpSetConf2 = pFunc;
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALReqPullupRegister2                                                 */
/*                                                                                              */
/* DESCRIPTION: Set the request flag that is use to judge whether pull-up register is ON or OFF */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iReqFlg                         Request flag        GRP_USB_TRUE  : Allow       */
/*                                                                  GRP_USB_FALSE : Prohibit    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALReqPullupRegister2( INT iReqFlg )
{
    if( GRUSB_TRUE == iReqFlg )
    {
        /* Update pull-up status */
        l_bPUStatus2 = GRUSB_TRUE;
        
        if( GRUSB_TRUE == grp_cyclonevd_hal_ChackVBusStatus2() )
        {
            /* CONNECT */
            _grp_cyclonevd_hal_Attach2();
        }
        else
        {
            /* DISCONNECT */
            _grp_cyclonevd_hal_Detach2();
        }
    }
    else
    {
        /* Disconnect */
        GRUSB_DEV_Disconnect2();
        
        /* Update pull-up status */
        l_bPUStatus2 = GRUSB_FALSE;
    }
}

/************************************************************************************************/
/* FUNCTION   : GRUSB_DEV_HALSetTestMode2                                                       */
/*                                                                                              */
/* DESCRIPTION: Set test mode selectors                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : usValue                         Test mode selectors                             */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID GRUSB_DEV_HALSetTestMode2( UINT16 usValue )
{
    /* Save test mode selector value */
    l_usTestMode2 = usValue;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_EnumerationCompletion2                                        */
/*                                                                                              */
/* DESCRIPTION: Enumeration completion                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_EnumerationCompletion2( VOID )
{
BOOLEAN                 bSpeed;
    
    /* Check bus speed */
    bSpeed = _grp_cyclonevd_hal_CheckBusSpeed2();
    
    /* Bus speed notify */
    GRUSB_Prm_SetBusSpeed2( bSpeed );
    
    /* Endpoint 0 activation */
    _grp_cyclonevd_hal_EndpointActivation2( GRUSB_DEV_EP0 );
    
    /* Update enumeration completion status */
    l_bECStatus2 = GRUSB_TRUE;
    
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
    /* Unmask receive FIFO non-empty interrupt */
    CYCLONEV_R32_ST( CYCLONEV_A32_OTG_GINTMSK2, CYCLONEVG_B01_RXFLVLMSK );
#endif
    
    /* Receive enable */
    grp_cyclonevd_hal_RecvEnable2( GRUSB_DEV_EP0, GRP_CYCLONEVD_PKT_TYPE_SETUP );
    
    /* Interrupt clear */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GINTSTS2, CYCLONEVG_B01_ENUMDONE );
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_BusResetIntr2                                                 */
/*                                                                                              */
/* DESCRIPTION: Processing of interruption of BUS_RESET                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_BusResetIntr2( VOID )
{
INT                     iEpNo;
    
    /* Reset enumeration completion status */
    l_bECStatus2 = GRUSB_FALSE;
    
    /* Re-initialize of CONTROL transfer module */
    GRUSB_DEV_CtrlReInit2();
    
    /* Clear perid status */
    GRUSB_DEV_StateSuspend2( GRUSB_FALSE );
    GRUSB_DEV_StateReset2();
    
    for( iEpNo = 0; iEpNo < GRP_CYCLONEVD_EP_NUM; iEpNo++ )
    {
        /* Set receiving flag to FALSE */
        l_atRcvInf2[iEpNo].bRcvFlag = GRUSB_FALSE;
        /* Set receiving status to "Initialization completion" */
        l_atRcvInf2[iEpNo].ulRcvStat = GRP_CYCLONEVD_RX_INIT;
    }
    
    /* Clear Remote wakeup signaling */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_DCTL2, CYCLONEVD_B01_RMTWKUPSIG );
    
    /* Flush the TxFIFO */
    grp_cyclonev_cmod_FlushTxFifo2( GRUSB_DEV_EP0 );
    
    for( iEpNo = 0; iEpNo < GRP_CYCLONEVD_EP_NUM; iEpNo++ )
    {
        /* Set NAK for all OUT endpoints */
        CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DOEPCTL2( iEpNo ), CYCLONEVD_B01_SNAK );
        
        /* Clear device OUT endpoint x interrupt register */
        CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPINT( iEpNo ), 0x000000FF );
        /* Clear device IN endpoint x interrupt register */
        CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), 0x000000FF );
    }
    
    /* Clear device all endpoints interrupt mask register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DAINTMSK2, 0x00000000 );
    /* Clear device OUT endpoint common interrupt mask register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPMSK2, 0x00000000 );
    /* Clear device IN endpoint common interrupt mask register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPMSK2, 0x00000000 );
    /* Clear device IN endpoint FIFO empty interrupt mask register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPEMPMSK2, 0x00000000 );
    
    /* Unmask OUT/IN endpoint 0 (CONTROL) interrupt */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DAINTMSK2, ( CYCLONEVD_VOUTEPMSK_EP( GRUSB_DEV_EP0 ) | CYCLONEVD_VINEPMSK_EP( GRUSB_DEV_EP0 ) ) );
    /* Unmask OUT endpoint */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPMSK2, ( CYCLONEVD_B01_SETUPMSK | CYCLONEVD_B01_XFERCOMPLMSK ) );
    /* Unmask IN endpoint */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPMSK2, ( CYCLONEVD_B01_TIMEOUTMSK | CYCLONEVD_B01_XFERCOMPLMSK ) );
    
    /* Reset device address */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_DCFG2, CYCLONEVD_B07_DEVADDR );
    
    /* Setup the Data FIFO RAM size */
    _grp_cyclonevd_hal_SetFifoSize2();
    
    /* Reset Idle state */
    l_usCtrlSts2 = GRP_CYCLONEVD_CTLST_IDLE;
    
    /* Notice of BUS_RESET to upper layer */
    if( GRUSB_NULL != l_pfnCbBusReset2 )
    {
        (*l_pfnCbBusReset2)();
    }
    
    /* Request to receive of SETUP packet */
    _grp_cyclonevd_hal_ReqRecvSetupPacket2();
    
    /* Interrupt clear */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GINTSTS2, CYCLONEVG_B01_USBRST );

}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_SuspendIntr2                                                  */
/*                                                                                              */
/* DESCRIPTION: Processing of interruption of Suspend                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_SuspendIntr2( VOID )
{
    /* Set SUSPEND status */
    GRUSB_DEV_StateSuspend2( GRUSB_TRUE );
    
    /* Notice of SUSPEND to upper layer */
    if( GRUSB_NULL != l_pfnCbSuspend2 )
    {
        (*(l_pfnCbSuspend2))();
    }
    
    /* Interrupt clear */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GINTSTS2, CYCLONEVG_B01_USBSUSP );
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_ResumeIntr2                                                   */
/*                                                                                              */
/* DESCRIPTION: Processing of interruption of Resume                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_ResumeIntr2( VOID )
{
    /* Clear Remote wakeup signaling */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_DCTL2, CYCLONEVD_B01_RMTWKUPSIG );
    
    /* Clear SUSPEND status */
    GRUSB_DEV_StateSuspend2( GRUSB_FALSE );
    
    /* Notice of RESUME to upper layer */
    if( GRUSB_NULL != l_pfnCbResume2 )
    {
        (*(l_pfnCbResume2))();
    }
    
    /* Interrupt clear */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GINTSTS2, CYCLONEVG_B01_WKUPINT );
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_CtrlSetupIntr2                                                */
/*                                                                                              */
/* DESCRIPTION: Processing when device Request is received                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_CtrlSetupIntr2( VOID )
{
GRUSB_EndPointInfo*     ptEpInfo;
GRUSB_SndDataInfo*      ptSndDtInfo;
GRUSB_RcvDataInfo*      ptRcvDtInfo;
DEVICE_REQUEST_INFO     tDevReq;
    
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
 #if (GRP_CYCLONEV_USE_DCACHE_TYPE != GRP_CYCLONEV_DCACHE_INVALID)
    /* Data cache invalidate */
    grp_target_DataCacheInvalidate2( (grp_ui)l_aucSetupPkt2, (grp_ui)GRP_CYCLONEVD_BUF_SIZE );
 #endif
#endif
    
    /* Get device request */
    _grp_cyclonevd_hal_GetDeviceRequest2( &tDevReq );
    
    /* A STALL state is canceled irrespective of a previous state */
    GRUSB_DEV_StateSetStall2( GRUSB_DEV_EP0, GRUSB_FALSE );
    
    /* Get endpoint information */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo2( GRUSB_DEV_EP0 );
    
    if( GRUSB_NULL == ptEpInfo )
    {
        /* Endpoint isn't ready */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
        
        /* Set Idle state */
        l_usCtrlSts2 = GRP_CYCLONEVD_CTLST_IDLE;
        
        /* Request to receive of SETUP packet */
        _grp_cyclonevd_hal_ReqRecvSetupPacket2();
    }
    else
    {
        /* Clear receiving buffer counter */
        ptEpInfo->uInfo.tRcvInfo.ulRcvSize = 0;
        
        /* Check CONTROL transfer state */
        switch( l_usCtrlSts2 )
        {
        case ( GRP_CYCLONEVD_CTLST_DATA | GRP_CYCLONEVD_CTLST_IN | GRP_CYCLONEVD_CTLST_NO_BUF_WAIT ):   /* CONTROL data IN stage state (with no buffer wait option) */
        case ( GRP_CYCLONEVD_CTLST_STATUS | GRP_CYCLONEVD_CTLST_IN ):                                   /* CONTROL status IN stage state */
            /* Notice of send cancel to upper layer */
            if( GRUSB_NULL != l_pfnCbCtrTransferCncl2 )
            {
                (*l_pfnCbCtrTransferCncl2)();
            }
            break;
        case ( GRP_CYCLONEVD_CTLST_DATA | GRP_CYCLONEVD_CTLST_OUT | GRP_CYCLONEVD_CTLST_NO_BUF_WAIT ):  /* CONTROL data OUT stage state (with no buffer wait option) */
            /* Notice of receive cancel to upper layer */
            if( GRUSB_NULL != l_pfnCbCtrReceiveCncl2 )
            {
                (*l_pfnCbCtrReceiveCncl2)();
            }
            break;
        case ( GRP_CYCLONEVD_CTLST_DATA | GRP_CYCLONEVD_CTLST_IN ):                                     /* CONTROL data IN stage state */
            ptSndDtInfo = &( ptEpInfo->tCrntTrnsInfo.tSndDtInfo );
            
            /* Notice of send cancel to upper layer */
            if( GRUSB_NULL != ptSndDtInfo->pfnFunc )
            {
                (*(ptSndDtInfo->pfnFunc))( GRUSB_DEV_EP0, 
                                           ptSndDtInfo->pucBuf, 
                                           ptEpInfo->uInfo.tSndInfo.ulSndSize, 
                                           ptSndDtInfo->pAplInfo, 
                                           GRUSB_DEV_TRAN_CANCELED );
            }
            break;
        case ( GRP_CYCLONEVD_CTLST_DATA | GRP_CYCLONEVD_CTLST_OUT ):                                    /* CONTROL data OUT stage state */
        case ( GRP_CYCLONEVD_CTLST_STATUS | GRP_CYCLONEVD_CTLST_OUT ):                                  /* CONTROL status OUT stage state */
            /* Get receive data information */
            ptRcvDtInfo = &( ptEpInfo->tCrntTrnsInfo.tRcvDtInfo );
            
            /* Notice of receive cancel to upper layer */
            if( GRUSB_NULL != ptRcvDtInfo->pfnFunc )
            {
                (*(ptRcvDtInfo->pfnFunc))( GRUSB_DEV_EP0, 
                                           ptRcvDtInfo->pucBuf, 
                                           ptEpInfo->uInfo.tRcvInfo.ulRcvSize, 
                                           ptRcvDtInfo->pAplInfo, 
                                           GRUSB_DEV_TRAN_CANCELED );
            }
            break;
        default:
            break;
        }
        
        /* Analyze request */
        GRUSB_DEV_CtrlRcvReq2( &tDevReq );
        
        /* Stage transition */
        _grp_cyclonevd_hal_CtrlStageTransition2( ptEpInfo, &tDevReq );
    }
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_CtrlOutTransactionIntr2                                       */
/*                                                                                              */
/* DESCRIPTION: Process to OUT transaction of CONTROL transfer                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_CtrlOutTransactionIntr2( VOID )
{
GRUSB_EndPointInfo*     ptEpInfo;
    
    /* Get endpoint information */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo2( GRUSB_DEV_EP0 );
    
    if( GRUSB_NULL == ptEpInfo )
    {
        /* Endpoint isn't ready */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
        
        /* Set Idle state */
        l_usCtrlSts2 = GRP_CYCLONEVD_CTLST_IDLE;
        
        /* Request to receive of SETUP packet */
        _grp_cyclonevd_hal_ReqRecvSetupPacket2();
    }
    else
    {
        if( GRP_CYCLONEVD_CTLST_DATA == ( GRP_CYCLONEVD_CTLST_STAGE_TYPE & l_usCtrlSts2 ) )
        {
            /* CONTROL data OUT stage */
            _grp_cyclonevd_hal_CtrlDataOutStage2( ptEpInfo );
        }
        else
        {
            /* Set Idle state */
            l_usCtrlSts2 = GRP_CYCLONEVD_CTLST_IDLE;
            
            /* Request to receive of SETUP packet */
            _grp_cyclonevd_hal_ReqRecvSetupPacket2();
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_CtrlInTransactionIntr2                                        */
/*                                                                                              */
/* DESCRIPTION: Process to IN transaction of CONTROL transfer                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_CtrlInTransactionIntr2( VOID )
{
GRUSB_EndPointInfo*     ptEpInfo;
    
    /* Get endpoint information */
    ptEpInfo = GRUSB_Prm_GetEndPointInfo2( GRUSB_DEV_EP0 );
    
    if( GRUSB_NULL == ptEpInfo )
    {
        /* Endpoint isn't ready */
        GRUSB_DEV_HALSetStallState2( GRUSB_DEV_EP0, GRUSB_TRUE );
        
        /* Set Idle state */
        l_usCtrlSts2 = GRP_CYCLONEVD_CTLST_IDLE;
        
        /* Request to receive of SETUP packet */
        _grp_cyclonevd_hal_ReqRecvSetupPacket2();
    }
    else
    {
        if( GRP_CYCLONEVD_CTLST_DATA == ( GRP_CYCLONEVD_CTLST_STAGE_TYPE & l_usCtrlSts2 ) )
        {
            /* CONTROL data IN stage */
            _grp_cyclonevd_hal_CtrlDataInStage2( ptEpInfo );
        }
        else
        {
            /* Set Idle state */
            l_usCtrlSts2 = GRP_CYCLONEVD_CTLST_IDLE;
            
            /* Start test mode (When test mode selector value is set) */
            _grp_cyclonevd_hal_StartTestMode2();
            
            /* Request to receive of SETUP packet */
            _grp_cyclonevd_hal_ReqRecvSetupPacket2();
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_DataOutTransactionIntr2                                       */
/*                                                                                              */
/* DESCRIPTION: Process to OUT transaction of non-CONTROL transfer                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint Information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_DataOutTransactionIntr2( GRUSB_EndPointInfo* ptEpInfo )
{
GRUSB_RcvDataInfo*      ptRcvDtInfo;
INT                     iReadP;
UINT32                  ulRcvSize;
UINT16                  usResult;
    
    /* Data receive */
    usResult = _grp_cyclonevd_hal_DataRecv2( ptEpInfo );
    
    if( GRP_CYCLONEVD_RX_NOBUF == usResult )
    {
        /* No buffer to receive data */
        
        /* Set receiving flag to FALSE */
        l_atRcvInf2[ptEpInfo->iEpNo].bRcvFlag = GRUSB_FALSE;
        /* Set receiving status to "No buffer to receive data" */
        l_atRcvInf2[ptEpInfo->iEpNo].ulRcvStat = GRP_CYCLONEVD_RX_NOBUF;
        
        return;
    }
    else if( GRP_CYCLONEVD_RX_END == usResult )
    {
        /* End of transfer to buffer */
        
        /* Check buffer is exist */
        iReadP = GRLIB_Cyclic_CheckRead2( &( ptEpInfo->tCycBufInfo ) );
        
        if( GRLIB_NONBUFFER == iReadP )
        {
            /* No buffer */
            
            /* Set receiving flag to FALSE */
            l_atRcvInf2[ptEpInfo->iEpNo].bRcvFlag = GRUSB_FALSE;
        }
        
        /* Get receive data information */
        ptRcvDtInfo = &( ptEpInfo->tCrntTrnsInfo.tRcvDtInfo );
        
        /* Set received size */
        ulRcvSize = ptEpInfo->uInfo.tRcvInfo.ulRcvSize;
        
        /* Clear received buffer counter */
        ptEpInfo->uInfo.tRcvInfo.ulRcvSize = 0;
        
        /* Notice of receive completing to upper layer */
        if( GRUSB_NULL != ptRcvDtInfo->pfnFunc )
        {
            (*(ptRcvDtInfo->pfnFunc))( ptEpInfo->iEpNo, 
                                       ptRcvDtInfo->pucBuf, 
                                       ulRcvSize, 
                                       ptRcvDtInfo->pAplInfo, 
                                       GRUSB_DEV_SUCCESS );
        }
        
        /* Set receiving status to "End of transfer to buffer" */
        l_atRcvInf2[ptEpInfo->iEpNo].ulRcvStat = GRP_CYCLONEVD_RX_END;
    }
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_DataInTransactionIntr2                                        */
/*                                                                                              */
/* DESCRIPTION: Process to IN transaction of non-CONTROL transfer                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_DataInTransactionIntr2( GRUSB_EndPointInfo* ptEpInfo )
{
GRUSB_SndDataInfo*      ptSndDtInfo;
INT                     iReadP;
UINT16                  usMps;
UINT8                   ucPages;
    
    /* Get send data information */
    ptSndDtInfo = &( ptEpInfo->tCrntTrnsInfo.tSndDtInfo );
    
    /* Does it finish to send data for the requested buffer? */
    if( ptSndDtInfo->ulDataSz <= ptEpInfo->uInfo.tSndInfo.ulSndSize )
    {
        /* Get FIFO information */
        GRUSB_Prm_GetFIFOInfo2( ptEpInfo->iEpNo, &ucPages, &usMps );
        
        /* Add zero length packet flag is TRUE and just max packet size */
        if( ( GRUSB_TRUE == ptSndDtInfo->i0Len ) && ( 0 == ( ptSndDtInfo->ulDataSz % (UINT32)usMps ) ) )
        {
            /* Set flag of sending information to FALSE */
            ptSndDtInfo->i0Len = GRUSB_FALSE;
            
            /* Request to send zero length packet */
            _grp_cyclonevd_hal_ReqSendData2( ptEpInfo->iEpNo, GRUSB_NULL, 0 );
            
            return;
        }
        
        if( GRUSB_TRUE == ptEpInfo->uInfo.tSndInfo.iFlag )
        {
            /* Notice of send completing to upper layer */
            if( GRUSB_NULL != ptSndDtInfo->pfnFunc )
            {
                (*(ptSndDtInfo->pfnFunc))( ptEpInfo->iEpNo, 
                                           ptSndDtInfo->pucBuf, 
                                           ptEpInfo->uInfo.tSndInfo.ulSndSize, 
                                           ptSndDtInfo->pAplInfo, 
                                           GRUSB_DEV_SUCCESS );
            }
        }
        
        /* Check buffer is exist */
        iReadP = GRLIB_Cyclic_CheckRead2( &( ptEpInfo->tCycBufInfo ) );
        
        if( GRLIB_NONBUFFER == iReadP )
        {
            /* No buffer */
            
            /* Set flag of sending information to FALSE */
            ptEpInfo->uInfo.tSndInfo.iFlag     = GRUSB_FALSE;
            ptEpInfo->uInfo.tSndInfo.ulSndSize = 0;
            
            return;
        }
        else
        {
            /* Save current buffer pointer */
            memcpy( ptSndDtInfo, 
                    &( ptEpInfo->ptTrnsInfo[iReadP].tSndDtInfo ), 
                    sizeof(GRUSB_SndDataInfo) );
            
            /* Renew buffer pointer */
            GRLIB_Cyclic_IncRead2( &( ptEpInfo->tCycBufInfo ) );
            
            /* Renew sending information */
            ptEpInfo->uInfo.tSndInfo.ulSndSize = 0;
            ptEpInfo->uInfo.tSndInfo.iFlag = GRUSB_TRUE;
        }
    }
    
    /* Data send */
    _grp_cyclonevd_hal_DataSend2( ptEpInfo );
}

#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_RxFifoNonEmptyIntr2                                           */
/*                                                                                              */
/* DESCRIPTION: Process to RxFIFO non-empty                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_RxFifoNonEmptyIntr2( VOID )
{
GRUSB_EndPointInfo*     ptEpInfo;
UINT32                  ulGRcStsp;
UINT32                  ulByteCnt;
UINT32                  ulPktSts;
INT                     iEpNo;
    
    /* Mask receive FIFO non-empty interrupt */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_GINTMSK2, CYCLONEVG_B01_RXFLVLMSK );
    
    /* Read status read and pop register */
    ulGRcStsp = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GRXSTSP2 );
    
    /* Get byte count of the received data packet */
    ulByteCnt = ( ( CYCLONEVG_B11_BCNT & ulGRcStsp ) >> 4 );
    /* Get endpoint number */
    iEpNo = (INT)( CYCLONEVG_B04_CHNUM & ulGRcStsp );
    
    ulPktSts = ( CYCLONEVG_B04_PKTSTS & ulGRcStsp );
    
    /* Check status of the received packet */
    switch( ulPktSts )
    {
    case CYCLONEVG_VD_PKTSTS_GOUT_NAK:          /* Global OUT NAK */
        
        /* No process */
        break;
    case CYCLONEVG_VD_PKTSTS_OD_PKT_RCV:        /* OUT data packet received */
        
        if( 0 != ulByteCnt )
        {
            /* Get endpoint information */
            ptEpInfo = GRUSB_Prm_GetEndPointInfo2( iEpNo );
            /* Read receive data */
            _grp_cyclonevd_hal_ReadRecvData2( ptEpInfo, ulByteCnt );
        }
        break;
    case CYCLONEVG_VD_PKTSTS_OTF_CMPL:          /* OUT transfer completed */
        
        /* No process */
        break;
    case CYCLONEVG_VD_PKTSTS_STTR_CMPL:         /* SETUP transaction completed */
        
        /* No process */
        break;
    case CYCLONEVG_VD_PKTSTS_STDT_RCV:          /* SETUP data packet received */
        
        if( ( 8 == ulByteCnt ) && ( GRUSB_DEV_EP0 == iEpNo ) )
        {
            /* Read setup data packet from FIFO to buffer */
            _grp_cyclonevd_hal_ReadFifo2( GRUSB_DEV_EP0, l_aucSetupPkt2, ulByteCnt );
        }
        break;
    default:
        break;
    }
    
    /* Unmask receive FIFO non-empty interrupt */
    CYCLONEV_R32_ST( CYCLONEV_A32_OTG_GINTMSK2, CYCLONEVG_B01_RXFLVLMSK );
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_TxFifoEmptyIntr2                                              */
/*                                                                                              */
/* DESCRIPTION: Process to TxFIFO empty                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_TxFifoEmptyIntr2( GRUSB_EndPointInfo* ptEpInfo )
{
UINT32                  ulSendSize;
    
    /* Get send data size */
    ulSendSize = _grp_cyclonevd_hal_GetSendDataSize2( ptEpInfo->iEpNo );
    
    /* Write send data */
    _grp_cyclonevd_hal_WriteSendData2( ptEpInfo, ulSendSize );
}
#endif

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_GlobalOutNakEffectiveIntr2                                    */
/*                                                                                              */
/* DESCRIPTION: Process to Global OUT NAK effective                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_GlobalOutNakEffectiveIntr2( VOID )
{
INT                     iEpNo;
BOOLEAN                 bStat;
UINT32                  ulDoEpCtl;
UINT8                   ucType;
    
    /* Mask Global OUT NAK effective interrupt */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_GINTMSK2, CYCLONEVG_B01_GOUTNAKEFFMSK );
    
    /* Unmask endpoint disabled interrupt */
    CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DOEPMSK2, CYCLONEVD_B01_EPDISBLDMSK );
    
    /* OUT endpoint disable (except endpoint 0) */
    for( iEpNo = 1; iEpNo < GRP_CYCLONEVD_EP_NUM; iEpNo++ )
    {
        /* Get endpoint type */
        bStat = GRUSB_Prm_GetEndPointType2( iEpNo, &ucType );
        
        if( GRUSB_TRUE == bStat )
        {
            /* Check endpoint direction */
            if( GRUSB_DEV_MAC_IsEpTypeOUT( ucType ) )
            {
                /* Check receiving flag */
                if( GRUSB_TRUE == l_atRcvInf2[iEpNo].bRcvFlag )
                {
                    /* Read device OUT endpoint x control register */
                    ulDoEpCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DOEPCTL2( iEpNo ) );
                    
                    /* Set endpoint disable */
                    ulDoEpCtl |= CYCLONEVD_B01_EPDIS;
                    /* Set NAK */
                    ulDoEpCtl |= CYCLONEVD_B01_SNAK;
                    
                    /* Write device OUT endpoint x control register */
                    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPCTL2( iEpNo ), ulDoEpCtl );
                }
            }
        }
    }
    
    /* Clear global OUT NAK */
    CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DCTL2, CYCLONEVD_B01_CGOUTNAK );
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_InEndpointNakEffectiveIntr2                                   */
/*                                                                                              */
/* DESCRIPTION: Process to IN endpoint NAK effective                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_InEndpointNakEffectiveIntr2( INT iEpNo )
{
UINT32                  ulDiEpCtl;
    
    /* Mask IN endpoint NAK effective interrupt */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_DIEPMSK2, CYCLONEVD_B01_INEPNAKEFFMSK );
    
    /* Unmask endpoint disabled interrupt */
    CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DIEPMSK2, CYCLONEVD_B01_EPDISBLDMSK );
    
    /* Read device IN endpoint x control register */
    ulDiEpCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DIEPCTL2( iEpNo ) );
    
    /* Set endpoint disable */
    ulDiEpCtl |= CYCLONEVD_B01_EPDIS;
    /* Set NAK */
    ulDiEpCtl |= CYCLONEVD_B01_SNAK;
    
    /* Write device IN endpoint x control register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPCTL2( iEpNo ), ulDiEpCtl );
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_OutEndpointDisabledIntr2                                      */
/*                                                                                              */
/* DESCRIPTION: Process to OUT endpoint disabled                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_OutEndpointDisabledIntr2( GRUSB_EndPointInfo* ptEpInfo )
{
    /* Mask endpoint disabled interrupt */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_DOEPMSK2, CYCLONEVD_B01_EPDISBLDMSK );
    
    /* Clear receive information */
    _grp_cyclonevd_hal_ClearRecvInfo2( ptEpInfo );
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_InEndpointDisabledIntr2                                       */
/*                                                                                              */
/* DESCRIPTION: Process to IN endpoint disabled                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_InEndpointDisabledIntr2( GRUSB_EndPointInfo* ptEpInfo )
{
    /* Mask endpoint disabled interrupt */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_DIEPMSK2, CYCLONEVD_B01_EPDISBLDMSK );
    
    /* Clear send information */
    _grp_cyclonevd_hal_ClearSendInfo2( ptEpInfo );
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_RecvEnable2                                                   */
/*                                                                                              */
/* DESCRIPTION: Receive enable to specified endpoint                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/*              ucType                          Packet type                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_RecvEnable2( INT iEpNo, UINT8 ucType )
{
UINT32                  ulDoEpCtl;
    
    /* Read device OUT endpoint x control register */
    ulDoEpCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DOEPCTL2( iEpNo ) );
    
    if( GRP_CYCLONEVD_PKT_TYPE_DATA == ucType )
    {
        /* Clear NAK */
        ulDoEpCtl |= CYCLONEVD_B01_CNAK;
    }
    /* Endpoint enable */
    ulDoEpCtl |= CYCLONEVD_B01_EPENA;
    
    /* Write device OUT endpoint x control register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPCTL2( iEpNo ), ulDoEpCtl );
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_RecvDisable2                                                  */
/*                                                                                              */
/* DESCRIPTION: Receive disable to specified endpoint                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_RecvDisable2( INT iEpNo )
{
UINT32                  ulDoEpCtl;
    
    /* Read device OUT endpoint x control register */
    ulDoEpCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DOEPCTL2( iEpNo ) );
    
    /* Check endpoint enable */
    if( CYCLONEVD_B01_EPENA == ( CYCLONEVD_B01_EPENA & ulDoEpCtl ) )
    {
        /* Endpoint disable */
        ulDoEpCtl |= CYCLONEVD_B01_EPDIS;
        /* Set NAK */
        ulDoEpCtl |= CYCLONEVD_B01_SNAK;
    }
    else
    {
        ulDoEpCtl = 0;
    }
    
    /* Write device OUT endpoint x control register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPCTL2( iEpNo ), ulDoEpCtl );
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_SendEnable2                                                   */
/*                                                                                              */
/* DESCRIPTION: Send enable to specified endpoint                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_SendEnable2( INT iEpNo )
{
UINT32                  ulDiEpCtl;
    
    /* Read device IN endpoint x control register */
    ulDiEpCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DIEPCTL2( iEpNo ) );
    
    /* Clear NAK */
    ulDiEpCtl |= CYCLONEVD_B01_CNAK;
    /* Endpoint enable */
    ulDiEpCtl |= CYCLONEVD_B01_EPENA;
    
    /* Write device IN endpoint x control register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPCTL2( iEpNo ), ulDiEpCtl );
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_SendDisable2                                                  */
/*                                                                                              */
/* DESCRIPTION: Send disable to specified endpoint                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_hal_SendDisable2( INT iEpNo )
{
UINT32                  ulDiEpCtl;
    
    /* Read device IN endpoint x control register */
    ulDiEpCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DIEPCTL2( iEpNo ) );
    
    /* Check endpoint enable */
    if( CYCLONEVD_B01_EPENA == ( CYCLONEVD_B01_EPENA & ulDiEpCtl ) )
    {
        /* Set endpoint disable */
        ulDiEpCtl |= CYCLONEVD_B01_EPDIS;
        /* Set NAK */
        ulDiEpCtl |= CYCLONEVD_B01_SNAK;
    }
    else
    {
        ulDiEpCtl = 0;
    }
    
    /* Write device IN endpoint x control register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPCTL2( iEpNo ), ulDiEpCtl );
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_hal_ChackVBusStatus2                                              */
/*                                                                                              */
/* DESCRIPTION: Check VBUS Status                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : VBUS Status                     GRUSB_TRUE  : VBUS ON                           */
/*                                              GRUSB_FALSE : VBUS OFF                          */
/*                                                                                              */
/************************************************************************************************/
BOOLEAN grp_cyclonevd_hal_ChackVBusStatus2( VOID )
{
BOOLEAN                 bStat;
UINT32                  ulGOtgCtl;
    
    /* Read control and status register */
    ulGOtgCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GOTGCTL2 );
    
    if( CYCLONEVG_B01_BSESVLD == ( CYCLONEVG_B01_BSESVLD & ulGOtgCtl ) )
    {
        /* B-session is valid (VBUS ON) */
        bStat = GRUSB_TRUE;
    }
    else
    {
        /* B-session is not valid (VBUS OFF) */
        bStat = GRUSB_FALSE;
    }
    
    return bStat;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_ResetSetting2                                                */
/*                                                                                              */
/* DESCRIPTION: Reset setting                                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_ResetSetting2( VOID )
{
INT                     iEpNo;

    /* Flush the TxFIFO */
    grp_cyclonev_cmod_FlushTxFifo2( GRP_CYCLONEV_ALL_TXFIFO );
    /* Flush the RxFIFO */
    grp_cyclonev_cmod_FlushRxFifo2();
    
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
    /* Mask receive FIFO non-empty interrupt */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_GINTMSK2, CYCLONEVG_B01_RXFLVLMSK );
#endif
    /* Clear device OUT endpoint common interrupt mask register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPMSK2, 0x00000000 );
    /* Clear device IN endpoint common interrupt mask register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPMSK2, 0x00000000 );
    /* Clear device IN endpoint FIFO empty interrupt mask register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPEMPMSK2, 0x00000000 );
    /* Clear device all endpoints interrupt mask register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DAINTMSK2, 0x00000000 );
    
    for( iEpNo = 0; iEpNo < GRP_CYCLONEVD_EP_NUM; iEpNo++ )
    {
        /* Receive disable */
        grp_cyclonevd_hal_RecvDisable2( iEpNo );
        /* Clear device OUT endpoint x transfer size register */
        CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPTSIZ2( iEpNo ), 0x00000000 );
        
        /* Send disable */
        grp_cyclonevd_hal_SendDisable2( iEpNo );
        /* Clear device IN endpoint x transfer size register */
        CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPTSIZ2( iEpNo ), 0x00000000 );
        
        /* Endpoint deactivation (except endpoint 0) */
        _grp_cyclonevd_hal_EndpointDeactivation2( iEpNo );
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_SetFifoSize2                                                 */
/*                                                                                              */
/* DESCRIPTION: Setup the Data FIFO RAM size                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_SetFifoSize2( VOID )
{
UINT32                  ulTxFifoSiz;
UINT32                  ulStartAddr;
INT                     iEpNo;
    
    /* Set RxFIFO size */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GRXFSIZ2, GRP_CYCLONEVD_RX_FIFO_SIZE );
    
    /* IN endpoint 0 TxFIFO */
    /* Start address */
    ulStartAddr = GRP_CYCLONEVD_RX_FIFO_SIZE;
    ulTxFifoSiz = ulStartAddr;
    /* Size */
    ulTxFifoSiz |= ( l_aulTxFifoSiz2[GRUSB_DEV_EP0] << 16 );
    /* Set TxFIFO 0 (Non-periodic) start address and size */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GNPTXFSIZ2, ulTxFifoSiz );
    
    /* IN Endpoint x TxFIFO (x = 1..15) */
    for( iEpNo = 1; iEpNo < GRP_CYCLONEVD_EP_NUM; iEpNo++ )
    {
        ulTxFifoSiz = 0;
        
        /* Start address */
        ulStartAddr += l_aulTxFifoSiz2[iEpNo - 1];
        ulTxFifoSiz  = ulStartAddr;
        /* Size */
        ulTxFifoSiz |= ( l_aulTxFifoSiz2[iEpNo] << 16 );
        /* Set TxFIFO x start address and size (x = 1..15) */
        CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPTXF2( iEpNo ), ulTxFifoSiz );
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_EndpointActivation2                                          */
/*                                                                                              */
/* DESCRIPTION: Endpoint activation                                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Activation Status               GRUSB_TRUE  : Success (Valid endpoint)          */
/*                                              GRUSB_FALSE : Error (Invalid endpoint)          */
/*                                                                                              */
/************************************************************************************************/
LOCAL BOOLEAN _grp_cyclonevd_hal_EndpointActivation2( INT iEpNo )
{
BOOLEAN                 bStat;
UINT32                  ulDEpCtl;
UINT32                  ulDEpCtlAddr;
UINT32                  ulDaIntMsk;
UINT32                  ulEpType;
UINT16                  usMps;
UINT8                   ucPages;
UINT8                   ucEpDir;
    
    /* Get FIFO information */
    bStat = GRUSB_Prm_GetFIFOInfo2( iEpNo, &ucPages, &usMps );
    
    if( GRUSB_TRUE != bStat )
    {
        /* Invalid endpoint */
        return GRUSB_FALSE;
    }
    
    if( GRUSB_DEV_EP0 == iEpNo )
    {
        /* Endpoint 0 */
        
        /* Read device endpoint 0 control register */
        ulDEpCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DIEPCTL2( GRUSB_DEV_EP0 ) );
        
        /* Clear Max packet size */
        ulDEpCtl &= ~( CYCLONEVD_B02_EP0_MPS );
        
        /* Set max packet size */
        switch( usMps )
        {
        case 64:        /* 64 bytes */
            ulDEpCtl |= CYCLONEVD_VEP0_MPS_BYTES64;
            break;
        case 32:        /* 32 bytes */
            ulDEpCtl |= CYCLONEVD_VEP0_MPS_BYTES32;
            break;
        case 16:        /* 16 bytes */
            ulDEpCtl |= CYCLONEVD_VEP0_MPS_BYTES16;
            break;
        case 8:         /* 8 bytes */
            ulDEpCtl |= CYCLONEVD_VEP0_MPS_BYTES8;
            break;
        default:
            /* Invalid max packet size */
            return GRUSB_FALSE;
        }
        
        /* Write device endpoint 0 control register */
        CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPCTL2( GRUSB_DEV_EP0 ), ulDEpCtl );
        
        /* Clear global IN NAK */
        CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DCTL2, CYCLONEVD_B01_CGNPINNAK );
    }
    else
    {
        /* Endpoint 1-15 */
        
        /* Get endpoint type and direction */
        bStat = _grp_cyclonevd_hal_GetEndpointType2( iEpNo, &ulEpType, &ucEpDir );
        
        if( GRUSB_TRUE != bStat )
        {
            /* Invalid endpoint */
            return GRUSB_FALSE;
        }
        
        if( EPTYPE_IN == ucEpDir )
        {
            /* IN direction */
            
            /* Get device IN endpoint x control register address */
            ulDEpCtlAddr = CYCLONEV_A32_OTG_DIEPCTL2( iEpNo );
            
            /* Set IN endpoint interrupt mask */
            ulDaIntMsk = CYCLONEVD_VINEPMSK_EP( iEpNo );
        }
        else
        {
            /* OUT direction */
            
            /* Get device OUT endpoint x control register address */
            ulDEpCtlAddr = CYCLONEV_A32_OTG_DOEPCTL2( iEpNo );
            
            /* Set OUT endpoint interrupt mask */
            ulDaIntMsk = CYCLONEVD_VOUTEPMSK_EP( iEpNo );
        }
        
        /* Read endpoint x control register */
        ulDEpCtl = CYCLONEV_R32_RD( ulDEpCtlAddr );
        
        if( CYCLONEVD_B01_USBACTEP != ( CYCLONEVD_B01_USBACTEP & ulDEpCtl ) )
        {
            /* Set max packet size */
            ulDEpCtl = (UINT32)usMps;
            
            /* Set endpoint type */
            ulDEpCtl |= ulEpType;
            
            /* Set TxFIFO number (IN endpoint only) */
            if( EPTYPE_IN == ucEpDir )
            {
                ulDEpCtl |= (UINT32)(iEpNo << 22);
            }
            
            /* Set DATA0 PID */
            ulDEpCtl |= CYCLONEVD_B01_SETD0PID;
            
            /* Set USB active endpoint */
            ulDEpCtl |= CYCLONEVD_B01_USBACTEP;
            
            /* Write endpoint x control register */
            CYCLONEV_R32_WR( ulDEpCtlAddr, ulDEpCtl );
        }
        
        /* Set device all endpoints interrupt mask register */
        CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DAINTMSK2, ulDaIntMsk );
    }
    
    return GRUSB_TRUE;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_EndpointDeactivation2                                        */
/*                                                                                              */
/* DESCRIPTION: Endpoint deactivation                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Deactivation Status             GRUSB_TRUE  : Success (Valid endpoint)          */
/*                                              GRUSB_FALSE : Error (Invalid endpoint)          */
/*                                                                                              */
/************************************************************************************************/
LOCAL BOOLEAN _grp_cyclonevd_hal_EndpointDeactivation2( INT iEpNo )
{
BOOLEAN                 bStat;
UINT32                  ulDEpCtl;
UINT32                  ulDEpCtlAddr;
UINT32                  ulDaIntMsk;
UINT32                  ulEpType;
UINT8                   ucEpDir;
    
    if( GRUSB_DEV_EP0 != iEpNo )
    {
        /* Get endpoint type and direction */
        bStat = _grp_cyclonevd_hal_GetEndpointType2( iEpNo, &ulEpType, &ucEpDir );
        
        if( GRUSB_TRUE != bStat )
        {
            /* Invalid endpoint */
            return GRUSB_FALSE;
        }
        
        if( EPTYPE_IN == ucEpDir )
        {
            /* IN direction */
            
            /* Get device IN endpoint x control register address */
            ulDEpCtlAddr = CYCLONEV_A32_OTG_DIEPCTL2( iEpNo );
            
            ulDaIntMsk = CYCLONEVD_VINEPMSK_EP( iEpNo );
        }
        else
        {
            /* OUT direction */
            
            /* Get device OUT endpoint x control register address */
            ulDEpCtlAddr = CYCLONEV_A32_OTG_DOEPCTL2( iEpNo );
            
            ulDaIntMsk = CYCLONEVD_VOUTEPMSK_EP( iEpNo );
        }
        
        /* Read endpoint x control register */
        ulDEpCtl = CYCLONEV_R32_RD( ulDEpCtlAddr );
        
        if( CYCLONEVD_B01_USBACTEP == ( CYCLONEVD_B01_USBACTEP & ulDEpCtl ) )
        {
            /* Clear USB active endpoint */
            ulDEpCtl &= ~CYCLONEVD_B01_USBACTEP;
            
            /* Write endpoint x control register */
            CYCLONEV_R32_WR( ulDEpCtlAddr, ulDEpCtl );
        }
        
        /* Clear device all endpoints interrupt mask register */
        CYCLONEV_R32_CR( CYCLONEV_A32_OTG_DAINTMSK2, ulDaIntMsk );
    }
    
    return GRUSB_TRUE;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_SetEP0StallState2                                            */
/*                                                                                              */
/* DESCRIPTION: Set endpoint 0 STALL status to the controller                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iStatus                         Stall status        GRUSB_TRUE  : Set STALL     */
/*                                                                  GRUSB_FALSE : Clear STALL   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_SetEP0StallState2( INT iStatus )
{
UINT32                  ulDiEpCtl;
UINT32                  ulDoEpCtl;
    
    /* Read IN endpoint 0 control register */
    ulDiEpCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DIEPCTL2( GRUSB_DEV_EP0 ) );
    /* Read OUT endpoint 0 control register */
    ulDoEpCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DOEPCTL2( GRUSB_DEV_EP0 ) );
    
    if( GRUSB_TRUE == iStatus )
    {
        /* Set STALL */
        
        if( CYCLONEVD_B01_EPENA == ( CYCLONEVD_B01_EPENA & ulDiEpCtl ) )
        {
            /* Set endpoint disable */
            ulDiEpCtl |= CYCLONEVD_B01_EPDIS;
        }
        
        /* Set STALL handshake */
        ulDiEpCtl |= CYCLONEVD_B01_STALL;
        ulDoEpCtl |= CYCLONEVD_B01_STALL;
    }
    else
    {
        /* Clear STALL */
        
        /* Clear STALL handshake */
        ulDoEpCtl &= ~CYCLONEVD_B01_STALL;
        ulDiEpCtl &= ~CYCLONEVD_B01_STALL;
    }
    
    /* Write IN endpoint 0 control register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPCTL2( GRUSB_DEV_EP0 ), ulDiEpCtl );
    /* Write OUT endpoint 0 control register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPCTL2( GRUSB_DEV_EP0 ), ulDoEpCtl );
    
    if( GRUSB_TRUE == iStatus )
    {
        /* Set STALL condition state */
        l_usCtrlSts2 |= GRP_CYCLONEVD_CTLST_STALL;
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_SetEPxStallState2                                            */
/*                                                                                              */
/* DESCRIPTION: Set endpoint x STALL status to the controller (except endpoint 0)               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/*              iStatus                         Stall status        GRUSB_TRUE  : Set STALL     */
/*                                                                  GRUSB_FALSE : Clear STALL   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_SetEPxStallState2( INT iEpNo, INT iStatus )
{
BOOLEAN                 bStat;
UINT32                  ulDEpCtl;
UINT32                  ulDEpCtlAddr;
UINT8                   ucType;
UINT8                   ucDir;
    
    /* Get endpoint type */
    bStat = GRUSB_Prm_GetEndPointType2( iEpNo, &ucType );
    
    if( GRUSB_TRUE != bStat )
    {
        /* Invalid endpoint */
        return;
    }
    
    /* Check endpoint direction */
    if( GRUSB_DEV_MAC_IsEpTypeIN( ucType ) )
    {
        /* IN direction */
        ucDir = EPTYPE_IN;
        
        /* Get device IN endpoint x control register address */
        ulDEpCtlAddr = CYCLONEV_A32_OTG_DIEPCTL2( iEpNo );
    }
    else
    {
        /* OUT direction */
        ucDir = EPTYPE_OUT;
        
        /* Get device OUT endpoint x control register address */
        ulDEpCtlAddr = CYCLONEV_A32_OTG_DOEPCTL2( iEpNo );
    }
    
    /* Read endpoint x control register */
    ulDEpCtl = CYCLONEV_R32_RD( ulDEpCtlAddr );
    
    if( GRUSB_TRUE == iStatus )
    {
        /* Set STALL */
        
        /* Direction is IN */
        if( EPTYPE_IN == ucDir )
        {
            if( CYCLONEVD_B01_EPENA == ( CYCLONEVD_B01_EPENA & ulDEpCtl ) )
            {
                /* Set endpoint disable */
                ulDEpCtl |= CYCLONEVD_B01_EPDIS;
            }
        }
        
        /* Set STALL handshake */
        ulDEpCtl |= CYCLONEVD_B01_STALL;
    }
    else
    {
        /* Clear STALL */
        
        /* Clear STALL handshake */
        ulDEpCtl &= ~CYCLONEVD_B01_STALL;
    }
    
    /* Write endpoint x control register */
    CYCLONEV_R32_WR( ulDEpCtlAddr, ulDEpCtl );
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_CheckBusSpeed2                                               */
/*                                                                                              */
/* DESCRIPTION: Check Bus Speed                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Bus Speed                       GRUSB_TRUE  : High speed                        */
/*                                              GRUSB_FALSE : Full speed                        */
/*                                                                                              */
/************************************************************************************************/
LOCAL BOOLEAN _grp_cyclonevd_hal_CheckBusSpeed2( VOID )
{
UINT32                  ulDSts;
UINT32                  ulEnumSpd;
BOOLEAN                 bSpeed;
    
    /* Read device status register */
    ulDSts = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DSTS2 );
    
    /* Get enumerated speed */
    ulEnumSpd = ( CYCLONEVD_B02_ENUMSPD & ulDSts );
    
    /* Check enumerated speed */
    if( CYCLONEVD_VENUMSPD_HS3060 == ulEnumSpd )
    {
        /* High speed */
        bSpeed = GRUSB_TRUE;
    }
    else if(( CYCLONEVD_VENUMSPD_FS3060 == ulEnumSpd )
            || ( CYCLONEVD_VENUMSPD_FS48 == ulEnumSpd ))
    {
        /* Full speed */
        bSpeed = GRUSB_FALSE;
    }
    
    return bSpeed;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_GetEndpointType2                                             */
/*                                                                                              */
/* DESCRIPTION: Get endpoint type and direction                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/* OUTPUT     : pulEpType                       Endpoint type                                   */
/*              pucEpDir                        Endpoint direction                              */
/*                                                                                              */
/* RESULTS    : Status                          GRUSB_TRUE  : Success (Valid endpoint)          */
/*                                              GRUSB_FALSE : Error (Invalid endpoint)          */
/*                                                                                              */
/************************************************************************************************/
LOCAL BOOLEAN _grp_cyclonevd_hal_GetEndpointType2( INT iEpNo, UINT32* pulEpType, UINT8* pucEpDir )
{
BOOLEAN                 bStat;
UINT8                   ucType;
    
    /* Get endpoint type and direction */
    bStat = GRUSB_Prm_GetEndPointType2( iEpNo, &ucType );
    
    if( GRUSB_TRUE != bStat )
    {
        /* Invalid endpoint */
        return GRUSB_FALSE;
    }
    
    /* Check endpoint type */
    if( GRUSB_DEV_MAC_IsEpTypeCTRL( ucType ) )
    {
        /* CONTROL */
        *pulEpType = CYCLONEVD_VEPTYP_CONTROL;
    }
    else
    {
        if( GRUSB_DEV_MAC_IsEpTypeBLK( ucType ) )
        {
            /* BULK */
            *pulEpType = CYCLONEVD_VEPTYP_BULK;
        }
        else if( GRUSB_DEV_MAC_IsEpTypeINTR( ucType ) )
        {
            /* INTERRUPT */
            *pulEpType = CYCLONEVD_VEPTYP_INTERRUP;
        }
        else if( GRUSB_DEV_MAC_IsEpTypeISO( ucType ) )
        {
            /* ISOCHRONOUS transfer is not supported in V1.00 */
            return GRUSB_FALSE;
        }
        else
        {
            /* Invalid endpoint */
            return GRUSB_FALSE;
        }
        
        /* Check endpoint direction */
        if( GRUSB_DEV_MAC_IsEpTypeIN( ucType ) )
        {
            /* IN direction */
            *pucEpDir = EPTYPE_IN;
        }
        else
        {
            /* OUT direction */
            *pucEpDir = EPTYPE_OUT;
        }
    }
    
    return GRUSB_TRUE;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_Attach2                                                      */
/*                                                                                              */
/* DESCRIPTION: USB Attach (CONNECT)                                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_Attach2( VOID )
{
    if( GRUSB_TRUE == l_bPUStatus2 )
    {
        /* Clear soft disconnect */
        CYCLONEV_R32_CR( CYCLONEV_A32_OTG_DCTL2, CYCLONEVD_B01_SFTDISCON );
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_Detach2                                                      */
/*                                                                                              */
/* DESCRIPTION: USB Detach (DISCONNECT)                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_Detach2( VOID )
{
    if( GRUSB_TRUE == l_bPUStatus2 )
    {
        /* Set soft disconnect */
        CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DCTL2, CYCLONEVD_B01_SFTDISCON );
    }
    
    /* Reset enumeration completion status */
    l_bECStatus2 = GRUSB_FALSE;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_GetDeviceRequest2                                            */
/*                                                                                              */
/* DESCRIPTION: Get device request from buffer                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptDevReq                        Device request information                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_GetDeviceRequest2( DEVICE_REQUEST_INFO* ptDevReq )
{
    /* bmRequestType */
    ptDevReq->ucRequestType = l_aucSetupPkt2[0];
    /* bRequest */
    ptDevReq->ucRequest     = l_aucSetupPkt2[1];
    /* wValue */
    ptDevReq->usValue       = (UINT16)( ( l_aucSetupPkt2[3] << 8 ) | l_aucSetupPkt2[2] );
    /* wIndex */
    ptDevReq->usIndex       = (UINT16)( ( l_aucSetupPkt2[5] << 8 ) | l_aucSetupPkt2[4] );
    /* wLength */
    ptDevReq->usLength      = (UINT16)( ( l_aucSetupPkt2[7] << 8 ) | l_aucSetupPkt2[6] );
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_CtrlStageTransition2                                         */
/*                                                                                              */
/* DESCRIPTION: Process to transition the stage of CONTROL transfer                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : ptDevReq                        Device request information                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_CtrlStageTransition2( GRUSB_EndPointInfo* ptEpInfo, DEVICE_REQUEST_INFO* ptDevReq )
{
INT                     iRdIdx;
INT                     iEpNo;
    
    if( GRP_CYCLONEVD_CTLST_STALL == ( GRP_CYCLONEVD_CTLST_OPTION_TYPE & l_usCtrlSts2 ) )
    {
        /* Set Idle state */
        l_usCtrlSts2 = GRP_CYCLONEVD_CTLST_IDLE;
        
        /* Request to receive of SETUP packet */
        _grp_cyclonevd_hal_ReqRecvSetupPacket2();
        
        return;
    }
    
    if( GRUSB_DEV_DATA_TRNS_DIR_DH == ( ptDevReq->ucRequestType & GRUSB_DEV_DATA_TRNS_DIR ) )
    {
        /* Data transfer direction is Device to Host */
        
        /* Check buffer is exist */
        iRdIdx = GRLIB_Cyclic_CheckRead2( &( ptEpInfo->tCycBufInfo ) );
        
        if( GRLIB_NONBUFFER == iRdIdx )
        {
            /* No buffer */
            
            /* Set CONTROL data IN stage state (with no buffer wait option) */
            l_usCtrlSts2 = ( GRP_CYCLONEVD_CTLST_DATA | GRP_CYCLONEVD_CTLST_IN | GRP_CYCLONEVD_CTLST_NO_BUF_WAIT );
            return;
        }
        
        /* Save current buffer pointer */
        memcpy( &( ptEpInfo->tCrntTrnsInfo.tSndDtInfo ), 
                &( ptEpInfo->ptTrnsInfo[iRdIdx].tSndDtInfo ), 
                sizeof(GRUSB_SndDataInfo) );
        
        /* Renew buffer pointer */
        GRLIB_Cyclic_IncRead2( &( ptEpInfo->tCycBufInfo ) );
        ptEpInfo->uInfo.tSndInfo.ulSndSize = 0;
        
        /* Change endpoint type to IN */
        ptEpInfo->ucEpType = ( EPTYPE_IN | EPTYPE_CONTROL );
        
        /* Set CONTROL data IN stage state */
        l_usCtrlSts2 = ( GRP_CYCLONEVD_CTLST_DATA | GRP_CYCLONEVD_CTLST_IN );
        
        /* CONTROL data IN stage */
        _grp_cyclonevd_hal_CtrlDataInStage2( ptEpInfo );
    }
    else
    {
        /* Data transfer direction is Host to Device */
        
        /* Check wLength */
        if( 0 != ptDevReq->usLength )
        {
            /* Check buffer is exist */
            iRdIdx = GRLIB_Cyclic_CheckRead2( &( ptEpInfo->tCycBufInfo ) );
            
            if( GRLIB_NONBUFFER == iRdIdx )
            {
                /* No buffer */
                
                /* CONTROL data OUT stage state (with no buffer wait option) */
                return;
            }

            /* Save current buffer pointer */
            memcpy( &( ptEpInfo->tCrntTrnsInfo.tRcvDtInfo ), 
                    &( ptEpInfo->ptTrnsInfo[iRdIdx].tRcvDtInfo ), 
                    sizeof(GRUSB_RcvDataInfo) );
            
            /* Renew buffer pointer */
            GRLIB_Cyclic_IncRead2( &( ptEpInfo->tCycBufInfo ) );
            
            /* Change endpoint type to OUT */
            ptEpInfo->ucEpType = ( EPTYPE_OUT | EPTYPE_CONTROL );
            
            /* Set CONTROL data OUT stage state */
            l_usCtrlSts2 = ( GRP_CYCLONEVD_CTLST_DATA | GRP_CYCLONEVD_CTLST_OUT );
            
            /* CONTROL data OUT stage */
            _grp_cyclonevd_hal_CtrlDataOutStage2( ptEpInfo );
        }
        else
        {
            switch( ptDevReq->ucRequest )
            {
            case GRUSB_DEV_SET_CONFIGURATION:   /* SET_CONFIGURATION */
                if( 0 != ptDevReq->usValue )
                {
                    /* Endpoint activation (except endpoint 0) */
                    for( iEpNo = 1; iEpNo < GRP_CYCLONEVD_EP_NUM; iEpNo++ )
                    {
                        _grp_cyclonevd_hal_EndpointActivation2( iEpNo );
                    }
                    
                    /* Notice of SET_CONFIGURATION completing to upper layer */
                    if( GRUSB_NULL != l_pfnCbCmpSetConf2 )
                    {
                        (*(l_pfnCbCmpSetConf2))();
                    }
                }
                else
                {
                    /* Endpoint deactivation (except endpoint 0) */
                    for( iEpNo = 1; iEpNo < GRP_CYCLONEVD_EP_NUM; iEpNo++ )
                    {
                        _grp_cyclonevd_hal_EndpointDeactivation2( iEpNo );
                    }
                }
                break;
            case GRUSB_DEV_SET_INTERFACE:       /* SET_INTERFACE */
                /* Notice of SET_INTERFACE completing to upper layer */
                if( GRUSB_NULL != l_pfnCbCmpSetIf2 )
                {
                    (*(l_pfnCbCmpSetIf2))();
                }
                break;
            default:
                break;
            }
            
            /* Set CONTROL NODATA status stage state */
            l_usCtrlSts2 = ( GRP_CYCLONEVD_CTLST_STATUS | GRP_CYCLONEVD_CTLST_NODATA );
            
            /* CONTROL status IN stage */
            _grp_cyclonevd_hal_CtrlStatusInStage2();
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_CtrlDataOutStage2                                            */
/*                                                                                              */
/* DESCRIPTION: CONTROL data OUT stage                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_CtrlDataOutStage2( GRUSB_EndPointInfo* ptEpInfo )
{
UINT8*                  pucBuf;
GRUSB_RcvDataInfo*      ptRcvDtInfo;
GRUSB_RcvInfo*          ptRcvInfo;
UINT32                  ulRecvSize;
UINT32                  ulRemainSize;
UINT16                  usMps;
UINT8                   ucPages;
    
    if( GRP_CYCLONEVD_CTLST_OUT == ( GRP_CYCLONEVD_CTLST_TRANS_TYPE & l_usCtrlSts2 ) ) 
    {
        /* CONTROL OUT state */
        
        /* Get receive data information */
        ptRcvDtInfo = &( ptEpInfo->tCrntTrnsInfo.tRcvDtInfo );
        ptRcvInfo   = &( ptEpInfo->uInfo.tRcvInfo );
        
        /* Get FIFO information */
        GRUSB_Prm_GetFIFOInfo2( GRUSB_DEV_EP0, &ucPages, &usMps );
        
        if( GRP_CYCLONEVD_CTLST_TRANS_CONT == ( GRP_CYCLONEVD_CTLST_OPTION_TYPE & l_usCtrlSts2 ) )
        {
            /* Get received data size */
            ulRecvSize = _grp_cyclonevd_hal_GetRecvDataSize2( GRUSB_DEV_EP0 );
            
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
 #if (GRP_CYCLONEV_USE_DCACHE_TYPE != GRP_CYCLONEV_DCACHE_INVALID)
            /* Data cache invalidate */
            grp_target_DataCacheInvalidate2( (grp_ui)ptRcvDtInfo->pucBuf, (grp_ui)ulRecvSize );
 #endif
            /* Add receive size */
            ptRcvInfo->ulRcvSize += ulRecvSize;
#endif
            
            if( ( ptRcvInfo->ulRcvSize == ptRcvDtInfo->ulRcvBufferSz ) || ( ulRecvSize < (UINT32)usMps ) )
            {
                /* Complete */
                
                /* Notice of receive completing to upper layer */
                if( GRUSB_NULL != ptRcvDtInfo->pfnFunc )
                {
                    (*(ptRcvDtInfo->pfnFunc))( GRUSB_DEV_EP0, 
                                               ptRcvDtInfo->pucBuf, 
                                               ptRcvInfo->ulRcvSize, 
                                               ptRcvDtInfo->pAplInfo, 
                                               GRUSB_DEV_SUCCESS );
                }
                
                /* Set CONTROL status IN stage state */
               l_usCtrlSts2 = ( GRP_CYCLONEVD_CTLST_STATUS | GRP_CYCLONEVD_CTLST_IN );
               
               /* CONTROL status IN stage */
                _grp_cyclonevd_hal_CtrlStatusInStage2();
                
                return;
            }
        }
        
        /* Calculate remain data size */
        ulRemainSize = ( ptRcvDtInfo->ulRcvBufferSz - ptRcvInfo->ulRcvSize );
        
        if( ulRemainSize > (UINT32)usMps )
        {
            /* Max packet size */
            ulRecvSize = (UINT32)usMps;
        }
        else
        {
            /* Remain size */
            ulRecvSize = ulRemainSize;
        }
        
        /* Set buffer pointer */
        pucBuf = (UINT8*)( ptRcvDtInfo->pucBuf + ptRcvInfo->ulRcvSize );
        
        /* Request to receive data */
        _grp_cyclonevd_hal_ReqCtrlRecvData2( pucBuf, ulRecvSize );
        
        /* Set CONTROL data OUT stage state (Transfer continue) */
        l_usCtrlSts2 = ( GRP_CYCLONEVD_CTLST_DATA | GRP_CYCLONEVD_CTLST_OUT | GRP_CYCLONEVD_CTLST_TRANS_CONT );
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_CtrlDataInStage2                                             */
/*                                                                                              */
/* DESCRIPTION: CONTROL data IN stage                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_CtrlDataInStage2( GRUSB_EndPointInfo* ptEpInfo )
{
UINT8*                  pucBuf;
GRUSB_SndDataInfo*      ptSndDtInfo;
UINT32                  ulSentSize;
UINT32                  ulRemainSize;
UINT16                  usMps;
UINT8                   ucPages;

    if( GRP_CYCLONEVD_CTLST_IN == ( GRP_CYCLONEVD_CTLST_TRANS_TYPE & l_usCtrlSts2 ) )
    {
        /* CONTROL IN state */
        
        /* Set send information flag */
        ptEpInfo->uInfo.tSndInfo.iFlag = GRUSB_TRUE;
        
        /* Get send data information */
        ptSndDtInfo = &( ptEpInfo->tCrntTrnsInfo.tSndDtInfo );
        
        /* Get FIFO information */
        GRUSB_Prm_GetFIFOInfo2( GRUSB_DEV_EP0, &ucPages, &usMps );
        
        /* Calculate remain data size */
        ulRemainSize = ( ptSndDtInfo->ulDataSz - ptEpInfo->uInfo.tSndInfo.ulSndSize );
        
        /* Does it finish to send data for the requested buffer? */
        if( 0 == ulRemainSize )
        {
            /* Add zero length packet flag is TRUE and just max packet size */
            if( ( GRUSB_TRUE == ptSndDtInfo->i0Len ) && ( 0 == ( ptSndDtInfo->ulDataSz % (UINT32)usMps ) ) )
            {
                /* Set flag of sending information to FALSE */
                ptSndDtInfo->i0Len = GRUSB_FALSE;
                
                /* Request to send data */
                _grp_cyclonevd_hal_ReqCtrlSendData2( GRUSB_NULL, 0 );
            }
            else
            {
                /* Notice of send completing to upper layer */
                if( GRUSB_NULL != ptSndDtInfo->pfnFunc )
                {
                    (*(ptSndDtInfo->pfnFunc))( GRUSB_DEV_EP0, 
                                               ptSndDtInfo->pucBuf, 
                                               ptSndDtInfo->ulDataSz, 
                                               ptSndDtInfo->pAplInfo, 
                                               GRUSB_DEV_SUCCESS );
                }
                
                /* Set CONTROL status OUT stage state */
                l_usCtrlSts2 = ( GRP_CYCLONEVD_CTLST_STATUS | GRP_CYCLONEVD_CTLST_OUT );
                
                /* CONTROL status OUT stage */
                _grp_cyclonevd_hal_CtrlStatusOutStage2();
                
                return;
            }
        }
        else
        {
            /* Set buffer pointer */
            pucBuf = (UINT8*)( ptSndDtInfo->pucBuf + ptEpInfo->uInfo.tSndInfo.ulSndSize );
            
            /* Calculate send data size */
            if( ulRemainSize > (UINT32)usMps )
            {
#if 0       // USB CHAPTER9テスト グレープシステム回答によるDefaultステートでのConfiguration Descriptor(9.2～9.5)がエラーになる対応 2019/01/28
                if( GRUSB_DEV_STATE_DEFAULT == GRUSB_DEV_StateGetDeviceState2() )
                {
                    /* Change send data size */
                    ptSndDtInfo->ulDataSz = (UINT32)usMps;
                    
                    /* Set flag of sending information to FALSE */
                    ptSndDtInfo->i0Len = GRUSB_FALSE;
                }
#endif
                /* Max packet size */
                ulSentSize = (UINT32)usMps;
            }
            else
            {
                /* Remain size */
                ulSentSize = ulRemainSize;
            }
            
            /* Clear CONTROL send data buffer */
            memset( l_aucCtrlData2, 0, GRP_CYCLONEVD_TX_FIFO_0_SIZE );
            /* Copy CONTROL send data */
            memcpy( l_aucCtrlData2, pucBuf, ulSentSize );
            
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
            /* CONTROL send data buffer valid */
            l_bCtrlDataValidFlag2 = GRUSB_TRUE;
#endif
            /* Request to send data */
            _grp_cyclonevd_hal_ReqCtrlSendData2( l_aucCtrlData2, ulSentSize );
            
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
            /* Add send data size */
            ptEpInfo->uInfo.tSndInfo.ulSndSize += ulSentSize;
#endif
        }
        
        /* Set CONTROL data IN stage state (Transfer continue) */
        l_usCtrlSts2 = ( GRP_CYCLONEVD_CTLST_DATA | GRP_CYCLONEVD_CTLST_IN | GRP_CYCLONEVD_CTLST_TRANS_CONT );
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_CtrlStatusOutStage2                                          */
/*                                                                                              */
/* DESCRIPTION: CONTROL status OUT stage                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_CtrlStatusOutStage2( VOID )
{
    if( GRP_CYCLONEVD_CTLST_OUT == ( GRP_CYCLONEVD_CTLST_TRANS_TYPE & l_usCtrlSts2 ) )
    {
        /* Request to receive zero length packet */
        _grp_cyclonevd_hal_ReqCtrlRecvData2( GRUSB_NULL, 0 );
    }
    else
    {
        /* Set Idle state */
        l_usCtrlSts2 = GRP_CYCLONEVD_CTLST_IDLE;
        
        /* Request to receive of SETUP packet */
        _grp_cyclonevd_hal_ReqRecvSetupPacket2();
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_CtrlStatusInStage2                                           */
/*                                                                                              */
/* DESCRIPTION: CONTROL status IN stage                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_CtrlStatusInStage2( VOID )
{
    if( ( GRP_CYCLONEVD_CTLST_IN == ( GRP_CYCLONEVD_CTLST_TRANS_TYPE & l_usCtrlSts2 ) ) 
     || ( GRP_CYCLONEVD_CTLST_NODATA == ( GRP_CYCLONEVD_CTLST_TRANS_TYPE & l_usCtrlSts2 ) ) )
    {
        /* Request to send zero length packet */
        _grp_cyclonevd_hal_ReqCtrlSendData2( GRUSB_NULL, 0 );
    }
    else
    {
        /* Set Idle state */
        l_usCtrlSts2 = GRP_CYCLONEVD_CTLST_IDLE;
        
        /* Request to receive of SETUP packet */
        _grp_cyclonevd_hal_ReqRecvSetupPacket2();
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_DataRecv2                                                    */
/*                                                                                              */
/* DESCRIPTION: Data receive                                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Receive result                  GRP_CYCLONEVD_RX_CONT : Next data is exist      */
/*                                              GRP_CYCLONEVD_RX_END  : End of transfer to      */
/*                                                                      buffer                  */
/*                                              GRP_CYCLONEVD_RX_NOBUF: No buffer to receive    */
/*                                                                      data                    */
/*                                                                                              */
/************************************************************************************************/
LOCAL UINT16 _grp_cyclonevd_hal_DataRecv2( GRUSB_EndPointInfo* ptEpInfo )
{
UINT8*                  pucBuf;
GRUSB_RcvDataInfo*      ptRcvDtInfo;
GRUSB_RcvInfo*          ptRcvInfo;
INT                     iReadP;
UINT32                  ulRemainSize;
UINT32                  ulRecvSize;
UINT16                  usMps;
UINT8                   ucPages;
    
    /* Get receive data information */
    ptRcvDtInfo = &( ptEpInfo->tCrntTrnsInfo.tRcvDtInfo );
    ptRcvInfo   = &( ptEpInfo->uInfo.tRcvInfo );
    
    /* Get FIFO information */
    GRUSB_Prm_GetFIFOInfo2( ptEpInfo->iEpNo, &ucPages, &usMps );
    
    if( ( 0 == ptRcvInfo->ulRcvSize ) && ( GRP_CYCLONEVD_RX_FIRST == l_atRcvInf2[ptEpInfo->iEpNo].ulRcvStat ) )
    {
        /* First receive to the buffer */
        
        /* Check buffer is exist */
        iReadP = GRLIB_Cyclic_CheckRead2( &( ptEpInfo->tCycBufInfo ) );
        
        if( GRLIB_NONBUFFER == iReadP )
        {
            /* No buffer to receive data */
            
            /* Notice of error to upper layer */
            if( GRUSB_NULL != ptEpInfo->pfnTrnsErrFnc )
            {
                (*(ptEpInfo->pfnTrnsErrFnc))( ptEpInfo->iEpNo, GRUSB_DEV_TranErr_UNDERRUN );
            }
            
            return GRP_CYCLONEVD_RX_NOBUF;
        }
        
        /* Save current buffer pointer */
        memcpy( ptRcvDtInfo, 
                &( ptEpInfo->ptTrnsInfo[iReadP].tRcvDtInfo ), 
                sizeof(GRUSB_RcvDataInfo) );
        
        /* Renew buffer pointer */
        GRLIB_Cyclic_IncRead2( &( ptEpInfo->tCycBufInfo ) );
        
        /* Calculate receive data size */
        if( ptRcvDtInfo->ulRcvBufferSz > (UINT32)usMps )
        {
            /* Max packet size */
            ulRecvSize = (UINT32)usMps;
        }
        else
        {
            /* Receive size */
            ulRecvSize = ptRcvDtInfo->ulRcvBufferSz;
        }
    }
    else
    {
        /* Get received data size */
        ulRecvSize = _grp_cyclonevd_hal_GetRecvDataSize2( ptEpInfo->iEpNo );
        
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
 #if (GRP_CYCLONEV_USE_DCACHE_TYPE != GRP_CYCLONEV_DCACHE_INVALID)
        /* Data cache invalidate */
        grp_target_DataCacheInvalidate2( (grp_ui)ptRcvDtInfo->pucBuf, (grp_ui)ulRecvSize );
 #endif
        /* Add receive size */
        ptRcvInfo->ulRcvSize += ulRecvSize;
#endif
        
        if( ( ptRcvInfo->ulRcvSize == ptRcvDtInfo->ulRcvBufferSz ) || ( ulRecvSize < (UINT32)usMps ) )
        {
            /* End of transfer to buffer */
            return GRP_CYCLONEVD_RX_END;
        }
        
        /* Calculate remain data size */
        ulRemainSize = ( ptRcvDtInfo->ulRcvBufferSz - ptRcvInfo->ulRcvSize );
        
        if( ulRemainSize > (UINT32)usMps )
        {
            /* Max packet size */
            ulRecvSize = (UINT32)usMps;
        }
        else
        {
            /* Remain size */
            ulRecvSize = ulRemainSize;
        }
    }
    
    /* Set buffer pointer */
    pucBuf = (UINT8*)( ptRcvDtInfo->pucBuf + ptRcvInfo->ulRcvSize );
    
    /* Request to receive Data */
    _grp_cyclonevd_hal_ReqRecvData2( ptEpInfo->iEpNo, pucBuf, ulRecvSize );
    
    /* Set receiving status to "Next data is exist" */
    l_atRcvInf2[ptEpInfo->iEpNo].ulRcvStat = GRP_CYCLONEVD_RX_CONT;
    
    /* Next data is exist */
    return GRP_CYCLONEVD_RX_CONT;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_DataSend2                                                    */
/*                                                                                              */
/* DESCRIPTION: Data send                                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_DataSend2( GRUSB_EndPointInfo* ptEpInfo )
{
UINT8*                  pucBuf;
GRUSB_SndDataInfo*      ptSndDtInfo;
UINT32                  ulSentSize;
UINT32                  ulRemainSize;
UINT16                  usMps;
UINT8                   ucPages;
    
    /* Get send data information */
    ptSndDtInfo = &( ptEpInfo->tCrntTrnsInfo.tSndDtInfo );
    
    /* Calculate remain data size */
    ulRemainSize = ( ptSndDtInfo->ulDataSz - ptEpInfo->uInfo.tSndInfo.ulSndSize );
    
    /* Get FIFO information */
    GRUSB_Prm_GetFIFOInfo2( ptEpInfo->iEpNo, &ucPages, &usMps );
    
    /* Set buffer pointer */
    pucBuf = (UINT8*)( ptSndDtInfo->pucBuf + ptEpInfo->uInfo.tSndInfo.ulSndSize );
    
    /* Calculate send data size */
    if( ulRemainSize > (UINT32)usMps )
    {
        /* Max packet size */
        ulSentSize = (UINT32)usMps;
    }
    else
    {
        /* Remain size */
        ulSentSize = ulRemainSize;
    }
    
    /* Request to send data */
    _grp_cyclonevd_hal_ReqSendData2( ptEpInfo->iEpNo, pucBuf, ulSentSize );
    
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
    /* Add send data size */
    ptEpInfo->uInfo.tSndInfo.ulSndSize += ulSentSize;
#endif
}

#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_ReadRecvData2                                                */
/*                                                                                              */
/* DESCRIPTION: Read receive data                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/*              ulSize                          Read data size                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_ReadRecvData2( GRUSB_EndPointInfo* ptEpInfo, UINT32 ulSize )
{
GRUSB_RcvDataInfo*      ptRcvDtInfo;
GRUSB_RcvInfo*          ptRcvInfo;
UINT8*                  pucBuf;
    
    /* Get receive data information */
    ptRcvDtInfo = &( ptEpInfo->tCrntTrnsInfo.tRcvDtInfo );
    ptRcvInfo   = &( ptEpInfo->uInfo.tRcvInfo );
    
    /* Set buffer pointer */
    pucBuf = (UINT8*)( ptRcvDtInfo->pucBuf + ptRcvInfo->ulRcvSize );
    
    /* Read data from FIFO to buffer */
    _grp_cyclonevd_hal_ReadFifo2( ptEpInfo->iEpNo, pucBuf, ulSize );
    
    /* Add receive data size */
    ptRcvInfo->ulRcvSize += ulSize;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_WriteSendData2                                               */
/*                                                                                              */
/* DESCRIPTION: Write send data                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/*              ulSize                          Write data size                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_WriteSendData2( GRUSB_EndPointInfo* ptEpInfo, UINT32 ulSize )
{
GRUSB_SndDataInfo*      ptSndDtInfo;
GRUSB_SndInfo*          ptSndInfo;
UINT8*                  pucBuf;
UINT32                  ulCnt;
UINT32                  ulDTxFSts;
UINT32                  ulInEptFsAv;
    
    /* Get send data information */
    ptSndDtInfo = &( ptEpInfo->tCrntTrnsInfo.tSndDtInfo );
    ptSndInfo   = &( ptEpInfo->uInfo.tSndInfo );
    
    /* Calculate write count */
    ulCnt = ( ( ulSize + 3 ) / 4 );
    
    /* Read device IN endpoint-x transmit FIFO status register */
    ulDTxFSts = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DTXFSTS2( ptEpInfo->iEpNo ) );
    
    /* Get IN endpoint TxFIFO space available */
    ulInEptFsAv = ( CYCLONEVD_B16_INEPTXFSPCAVAIL & ulDTxFSts );
    
    if( ( ulInEptFsAv >= ulCnt ) && ( 0 != ulSize ) )
    {
        if (( l_bCtrlDataValidFlag2 == GRUSB_TRUE ) && ( ptEpInfo->iEpNo == GRUSB_DEV_EP0 ))
        {
            /* Set buffer pointer */
            pucBuf = (UINT8*)l_aucCtrlData2;
            /* CONTROL send data buffer invalid */
            l_bCtrlDataValidFlag2 = GRUSB_FALSE;
        }
        else
        {
            /* Set buffer pointer */
            pucBuf = (UINT8*)( ptSndDtInfo->pucBuf + ptSndInfo->ulSndSize );
        }
        
        /* Write data from buffer to FIFO */
        _grp_cyclonevd_hal_WriteFifo2( ptEpInfo->iEpNo, pucBuf, ulSize );
        
        /* Add send data size */
        ptSndInfo->ulSndSize += ulSize;
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_ReadFifo2                                                     */
/*                                                                                              */
/* DESCRIPTION: Read data from FIFO to buffer                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/*              pucBuf                          Buffer pointer                                  */
/*              ulSize                          Read data size                                  */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_ReadFifo2( INT iEpNo, UINT8* pucBuf, UINT32 ulSize )
{
UINT32                  ulLoop = 0;
UINT32                  ulCnt;
UINT32                  ulRemain;
UINT32                  ulBuf;
    
    /* Calculate read count */
    ulCnt = ( ulSize / 4 );
    /* Calculate fraction count */
    ulRemain = ( ulSize % 4 );
    
    for( ulLoop = 0; ulLoop < ulCnt; ulLoop++, pucBuf += 4 )
    {
        /* Read data from FIFO to buffer */
        *(UINT32*)pucBuf = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DFIFO2( iEpNo ) );
    }
    
    if( 0 != ulRemain )
    {
        /* Read fraction data from FIFO to local buffer */
        ulBuf = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DFIFO2( iEpNo ) );
        /* Copied fraction data by buffer */
        memcpy( pucBuf, &ulBuf, ulRemain );
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_WriteFifo2                                                   */
/*                                                                                              */
/* DESCRIPTION: Write data from buffer to FIFO                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/*              pucBuf                          Buffer pointer                                  */
/*              ulSize                          Write data size                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_WriteFifo2( INT iEpNo, UINT8* pucBuf, UINT32 ulSize )
{
UINT32                  ulLoop;
UINT32                  ulCnt;
    
    /* Calculate write count */
    ulCnt = ( ( ulSize + 3 ) / 4 );
    
    for( ulLoop = 0; ulLoop < ulCnt; ulLoop++, pucBuf += 4 )
    {
        /* Write data from buffer to FIFO */
        CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DFIFO2( iEpNo ), *(UINT32*)pucBuf );
    }
}
#endif

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_ReqRecvSetupPacket2                                          */
/*                                                                                              */
/* DESCRIPTION: Request to receive of SETUP packet                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_ReqRecvSetupPacket2( VOID )
{
UINT32                  ulDoEptSiz;
    
    /* Clear setup packet buffer */
    memset( l_aucSetupPkt2, 0, GRP_CYCLONEVD_BUF_SIZE );
    
    /* Set SETUP packet count */
    ulDoEptSiz = CYCLONEVD_VSUPCNT_THREEPACKET;
    /* Set packet count */
    ulDoEptSiz |= CYCLONEVD_B01_EP0_PKTCNT;
    /* Set transfer size */
    ulDoEptSiz |= GRP_CYCLONEVD_BUF_SIZE;
    /* Write device OUT endpoint 0 transfer size register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPTSIZ2( GRUSB_DEV_EP0 ), ulDoEptSiz );
    
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
    /* Set DMA address */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPDMA2( GRUSB_DEV_EP0 ), (UINT32)l_aucSetupPkt2 );
#endif
    
    if( GRUSB_TRUE == l_bECStatus2 )
    {
        /* Receive enable */
        grp_cyclonevd_hal_RecvEnable2( GRUSB_DEV_EP0, GRP_CYCLONEVD_PKT_TYPE_SETUP );
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_ReqCtrlRecvData2                                             */
/*                                                                                              */
/* DESCRIPTION: Request to receive data for CONTROL transfer                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pucBuf                          Buffer pointer                                  */
/*              ulSize                          Transfer size                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_ReqCtrlRecvData2( UINT8* pucBuf, UINT32 ulSize )
{
UINT32                  ulDoEptSiz;
UINT16                  usMps;
UINT8                   ucPages;
    
    /* Get FIFO information */
    GRUSB_Prm_GetFIFOInfo2( GRUSB_DEV_EP0, &ucPages, &usMps );
    
    /* Set transfer size */
    ulDoEptSiz = (UINT32)usMps;
    
    /* Set packet count */
    ulDoEptSiz |= CYCLONEVD_B01_EP0_PKTCNT;
    
    /* Write device OUT endpoint 0 transfer size register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPTSIZ2( GRUSB_DEV_EP0 ), ulDoEptSiz );
    
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
    /* Set DMA address */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPDMA2( GRUSB_DEV_EP0 ), pucBuf );
#endif
    
    /* Receive enable */
    grp_cyclonevd_hal_RecvEnable2( GRUSB_DEV_EP0, GRP_CYCLONEVD_PKT_TYPE_DATA );
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_ReqCtrlSendData2                                             */
/*                                                                                              */
/* DESCRIPTION: Request to send data for CONTROL transfer                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pucBuf                          Buffer pointer                                  */
/*              ulSize                          Transfer size                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_ReqCtrlSendData2( UINT8* pucBuf, UINT32 ulSize )
{
UINT32                  ulDiEptSiz;
    
    /* Set transfer size */
    ulDiEptSiz = ulSize;
    
    /* Set packet count */
    ulDiEptSiz |= CYCLONEVD_VEP0_PKTCNT_1PKT;
    
    /* Write device IN endpoint 0 transfer size register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPTSIZ2( GRUSB_DEV_EP0 ), ulDiEptSiz );
    
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
    /* Set DMA address */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPDMA2( GRUSB_DEV_EP0 ), pucBuf );
#endif
    
    /* Send enable */
    grp_cyclonevd_hal_SendEnable2( GRUSB_DEV_EP0 );
    
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
    if( 0 != ulSize )
    {
        /* Unmask TxFIFO empty interrupt */
        CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DIEPEMPMSK2, CYCLONEVD_VINEPTXFEMPMSK_EP( GRUSB_DEV_EP0 ) );
    }
#endif
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_ReqRecvData2                                                 */
/*                                                                                              */
/* DESCRIPTION: Request to receive Data                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/*              pucBuf                          Buffer pointer                                  */
/*              ulSize                          Transfer size                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_ReqRecvData2( INT iEpNo, UINT8* pucBuf, UINT32 ulSize )
{
UINT32                  ulDoEptSiz;
UINT32                  ulPktCnt = 1;
UINT16                  usMps;
UINT8                   ucPages;
    
    /* Get FIFO information */
    GRUSB_Prm_GetFIFOInfo2( iEpNo, &ucPages, &usMps );
    
    if( 0 == ulSize )
    {
        /* Set transfer size */
        ulDoEptSiz = (UINT32)usMps;
        /* Set packet count */
        ulDoEptSiz |= CYCLONEVD_VPKTCNT_1PKT;
    }
    else
    {
        /* Calculate packet count */
        ulPktCnt = ( ( ulSize + ( (UINT32)usMps - 1 ) ) / (UINT32)usMps );
        /* Set transfer size */
        ulDoEptSiz = ( ulPktCnt * (UINT32)usMps );
        /* Set packet count */
        ulDoEptSiz |= ( ulPktCnt << 19 );
    }
    
    /* Write device OUT endpoint x transfer size register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPTSIZ2( iEpNo ), ulDoEptSiz );
    
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
    /* Set DMA address */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPDMA2( iEpNo ), pucBuf );
#endif
    
    /* Receive enable */
    grp_cyclonevd_hal_RecvEnable2( iEpNo, GRP_CYCLONEVD_PKT_TYPE_DATA );
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_ReqSendData2                                                 */
/*                                                                                              */
/* DESCRIPTION: Request to send data                                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/*              pucBuf                          Buffer pointer                                  */
/*              ulSize                          Transfer size                                   */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_ReqSendData2( INT iEpNo, UINT8* pucBuf, UINT32 ulSize )
{
UINT32                  ulDiEptSiz;
UINT32                  ulPktCnt;
UINT16                  usMps;
UINT8                   ucPages;
    
    /* Set transfer size */
    ulDiEptSiz = ulSize;
    
    if( 0 == ulSize )
    {
        /* Set packet count */
        ulDiEptSiz |= CYCLONEVD_VPKTCNT_1PKT;
    }
    else
    {
        /* Get FIFO information */
        GRUSB_Prm_GetFIFOInfo2( iEpNo, &ucPages, &usMps );
        /* Calculate packet count */
        ulPktCnt = ( ( ulSize - 1 + (UINT32)usMps ) / (UINT32)usMps );
        /* Set packet count */
        ulDiEptSiz |= ( ulPktCnt << 19 );
    }
    
    /* Write device IN endpoint x transfer size register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPTSIZ2( iEpNo ), ulDiEptSiz );
    
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
    if( 0 != ulSize )
    {
        /* Unmask TxFIFO empty interrupt */
        CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DIEPEMPMSK2, CYCLONEVD_VINEPTXFEMPMSK_EP( iEpNo ) );
    }
#else
    /* Set DMA address */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPDMA2( iEpNo ), pucBuf );
#endif
    
    /* Send enable */
    grp_cyclonevd_hal_SendEnable2( iEpNo );
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_ClearRecvInfo2                                               */
/*                                                                                              */
/* DESCRIPTION: Clear receive information                                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_ClearRecvInfo2( GRUSB_EndPointInfo* ptEpInfo )
{
GRUSB_RcvDataInfo*      ptRcvDtInfo;
INT                     iReadP;
    
    /* Set receiving flag to FALSE */
    l_atRcvInf2[ptEpInfo->iEpNo].bRcvFlag = GRUSB_FALSE;
    /* Set receiving status to "End of transfer to buffer" */
    l_atRcvInf2[ptEpInfo->iEpNo].ulRcvStat = GRP_CYCLONEVD_RX_END;
        
    /* Is there any demand under present processing? */
    if( 0 != ptEpInfo->uInfo.tRcvInfo.ulRcvSize )
    {
        /* Get receive data information */
        ptRcvDtInfo = &( ptEpInfo->tCrntTrnsInfo.tRcvDtInfo );
        
        /* Notice of receive cancel to upper layer */
        if( GRUSB_NULL != ptRcvDtInfo->pfnFunc )
        {
            (*(ptRcvDtInfo->pfnFunc))( ptEpInfo->iEpNo,
                                       ptRcvDtInfo->pucBuf,
                                       ptEpInfo->uInfo.tRcvInfo.ulRcvSize,
                                       ptRcvDtInfo->pAplInfo,
                                       GRUSB_DEV_CANCELED );
        }
    }
    
    /* Clear the endpoint information */
    ptEpInfo->uInfo.tRcvInfo.ulRcvSize = 0;
    
    /* Initialize of receive information */
    memset( &( ptEpInfo->tCrntTrnsInfo.tRcvDtInfo ),
            0,
            sizeof(GRUSB_RcvDataInfo) );
    
    /* Notice of cancellation to all requests */
    while( 1 )
    {
        /* Check buffer is exist */
        iReadP = GRLIB_Cyclic_CheckRead2( &( ptEpInfo->tCycBufInfo ) );
        
        if( GRLIB_NONBUFFER == iReadP )
        {
            /* No buffer */
            break;
        }
        
        /* Get receive data information */
        ptRcvDtInfo = &(ptEpInfo->ptTrnsInfo[iReadP].tRcvDtInfo);
        
        /* Notice of receive cancel to upper layer */
        if( ptRcvDtInfo->pfnFunc != GRUSB_NULL )
        {
            (*(ptRcvDtInfo->pfnFunc))( ptEpInfo->iEpNo,
                                       ptRcvDtInfo->pucBuf,
                                       0,
                                       ptRcvDtInfo->pAplInfo,
                                       GRUSB_DEV_CANCELED );
        }
        
        /* Renew buffer pointer */
        GRLIB_Cyclic_IncRead2( &(ptEpInfo->tCycBufInfo) );
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_ClearSendInfo2                                               */
/*                                                                                              */
/* DESCRIPTION: Clear send information                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ptEpInfo                        Endpoint information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_hal_ClearSendInfo2( GRUSB_EndPointInfo* ptEpInfo )
{
GRUSB_SndDataInfo*      ptSndDtInfo;
INT                     iReadP;
    
    /* Mask TxFIFO empty interrupt */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_DIEPEMPMSK2, CYCLONEVD_VINEPTXFEMPMSK_EP( ptEpInfo->iEpNo ) );
    
    /* Is there any demand under present processing? */
    if( GRUSB_TRUE == ptEpInfo->uInfo.tSndInfo.iFlag )
    {
        if( 0 != ptEpInfo->uInfo.tSndInfo.ulSndSize )
        {
            /* Flush the TxFIFO */
            grp_cyclonev_cmod_FlushTxFifo2( ptEpInfo->iEpNo );
        }
        
        /* Get send data information */
        ptSndDtInfo = &( ptEpInfo->tCrntTrnsInfo.tSndDtInfo );
        
        /* Notice of send cancel to upper layer */
        if( GRUSB_NULL != ptSndDtInfo->pfnFunc )
        {
            (*(ptSndDtInfo->pfnFunc))( ptEpInfo->iEpNo,
                                       ptSndDtInfo->pucBuf,
                                       ptEpInfo->uInfo.tSndInfo.ulSndSize,
                                       ptSndDtInfo->pAplInfo,
                                       GRUSB_DEV_TRAN_CANCELED );
        }
    }
    
    /* Clear the endpoint information */
    ptEpInfo->uInfo.tSndInfo.ulSndSize = 0;
    ptEpInfo->uInfo.tSndInfo.iFlag     = GRUSB_FALSE;
    
    /* Initialize of send information */
    memset( &(ptEpInfo->tCrntTrnsInfo.tSndDtInfo),
            0,
            sizeof(GRUSB_SndDataInfo) );
    
    /* Notice of cancellation to all requests */
    while( 1 )
    {
        /* Check buffer is exist */
        iReadP = GRLIB_Cyclic_CheckRead2( &( ptEpInfo->tCycBufInfo ) );
        
        if( GRLIB_NONBUFFER == iReadP )
        {
            /* No buffer */
            break;
        }
        
        /* Get send data information */
        ptSndDtInfo = &(ptEpInfo->ptTrnsInfo[iReadP].tSndDtInfo);
        
        /* Notice of send cancel to upper layer */
        if( ptSndDtInfo->pfnFunc != GRUSB_NULL )
        {
            (*(ptSndDtInfo->pfnFunc))( ptEpInfo->iEpNo,
                                       ptSndDtInfo->pucBuf,
                                       0,
                                       ptSndDtInfo->pAplInfo,
                                       GRUSB_DEV_CANCELED );
        }
        
        /* Renew buffer pointer */
        GRLIB_Cyclic_IncRead2( &( ptEpInfo->tCycBufInfo ) );
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_GetRecvDataSize2                                             */
/*                                                                                              */
/* DESCRIPTION: Get received data size                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : received data size                                                              */
/*                                                                                              */
/************************************************************************************************/
LOCAL UINT32 _grp_cyclonevd_hal_GetRecvDataSize2( INT iEpNo )
{
UINT32                  ulDoEptSiz;
UINT32                  ulRecvSize;
UINT16                  usMps;
UINT8                   ucPages;
    
    /* Read device OUT endpoint x transfer size register */
    ulDoEptSiz = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DOEPTSIZ2( iEpNo ) );
    
    /* Get FIFO information */
    GRUSB_Prm_GetFIFOInfo2( iEpNo, &ucPages, &usMps );
    
    /* Calculate received data size */
    ulRecvSize = ( (UINT32)usMps - ( CYCLONEVD_B19_XFERSIZE & ulDoEptSiz ) );
    
    return ulRecvSize;
}

#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_GetSendDataSize2                                             */
/*                                                                                              */
/* DESCRIPTION: Get send data size                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : send data size                                                                  */
/*                                                                                              */
/************************************************************************************************/
LOCAL UINT32 _grp_cyclonevd_hal_GetSendDataSize2( INT iEpNo )
{
UINT32                  ulDiEptSiz;
UINT32                  ulSendSize;
    
    /* Read device IN endpoint 0 transfer size register */
    ulDiEptSiz = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DIEPTSIZ2( iEpNo ) );
    
    if( GRUSB_DEV_EP0 == iEpNo )
    {
        /* Endpoint 0 transfer size */
        ulSendSize = ( CYCLONEVD_B07_EP0_XFERSIZE & ulDiEptSiz );
    }
    else
    {
        /* Endpoint x transfer size (x = 1..15) */
        ulSendSize = ( CYCLONEVD_B19_XFERSIZE & ulDiEptSiz );
    }
    
    return ulSendSize;
}
#endif

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_hal_StartTestMode2                                               */
/*                                                                                              */
/* DESCRIPTION: Start test mode                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Test mode result                GRUSB_TRUE  : Enable test mode                  */
/*                                              GRUSB_FALSE : Disable test mode                 */
/*                                                                                              */
/************************************************************************************************/
LOCAL BOOLEAN _grp_cyclonevd_hal_StartTestMode2( VOID )
{
UINT32                  ulDCtl;
BOOLEAN                 bRet = GRUSB_FALSE;
    
    /* When test mode selector value is set */
    if( GRP_CYCLONEVD_TM_DISABLED != l_usTestMode2 )
    {
        /* Read device control register */
        ulDCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DCTL2 );
        
        /* Select test mode */
        switch( l_usTestMode2 )
        {
        case GRP_CYCLONEVD_TM_TEST_J:           /* Test_J mode */
            ulDCtl |= CYCLONEVD_VTSTCTL_TESTJ;
            break;
        case GRP_CYCLONEVD_TM_TEST_K:           /* Test_K mode */
            ulDCtl |= CYCLONEVD_VTSTCTL_TESTK;
            break;
        case GRP_CYCLONEVD_TM_TEST_SE0_NAK:     /* Test_SE0_NAK mode */
            ulDCtl |= CYCLONEVD_VTSTCTL_TESTSN;
            break;
        case GRP_CYCLONEVD_TM_TEST_PACKET:      /* Test_Packet mode */
            ulDCtl |= CYCLONEVD_VTSTCTL_TESTPM;
            break;
        case GRP_CYCLONEVD_TM_TEST_FORCE_ENA:   /* Test_Force_Enable */
            ulDCtl |= CYCLONEVD_VTSTCTL_TESTFE;
            break;
        default:
            /* DIRECT RETURN */
            return GRUSB_FALSE;
        }
        
        /* Write device control register */
        CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DCTL2, ulDCtl );
        
        bRet = GRUSB_TRUE;
    }
    
    return bRet;
}

/* END OF grp_cyclonevd_hal.c */
