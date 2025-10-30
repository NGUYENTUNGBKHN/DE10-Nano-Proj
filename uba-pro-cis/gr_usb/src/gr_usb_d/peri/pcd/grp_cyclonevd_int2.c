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
/*      grp_cyclonevd_int.c                                                     1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/DEVICE for CycloneV(OTG) - Interrupt process module                              */
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
#include "perid.h"
#include "peri_prm.h"
#include "grp_cyclonev_macro.h"
#include "grp_cyclonev_bit_val.h"
#include "grp_cyclonev_reg.h"
#include "grp_cyclonevd_int.h"
#include "grp_cyclonevd_hal.h"
#include "grp_cyclonevd_cfg.h"


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL VOID _grp_cyclonevd_int_Out_EP2( VOID );
LOCAL VOID _grp_cyclonevd_int_In_EP2( VOID );
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
LOCAL VOID _grp_cyclonevd_int_RxFifoNonEmpty2( VOID );
#endif
LOCAL VOID _grp_cyclonevd_int_GlobalOutNak2( VOID );
LOCAL VOID _grp_cyclonevd_int_Resume2( VOID );
LOCAL VOID _grp_cyclonevd_int_Suspend2( VOID );
LOCAL VOID _grp_cyclonevd_int_Reset2( VOID );
LOCAL VOID _grp_cyclonevd_int_Enum_Done2( VOID );
LOCAL VOID _grp_cyclonevd_int_Session_Req2( VOID );
LOCAL VOID _grp_cyclonevd_int_OTGInt2( VOID );
LOCAL VOID _grp_cyclonevd_int_Sof2( VOID );
LOCAL VOID _grp_cyclonevd_int_Mode_Mismatch2( VOID );
LOCAL VOID _grp_cyclonevd_DisableEp2( INT );


/************************************************************************************************/
/* FUNCTION   : grp_cyclonevd_Interrupt2                                                        */
/*                                                                                              */
/* DESCRIPTION: PCD interrupt handling                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID grp_cyclonevd_Interrupt2( VOID )
{
UINT32                  ulGIntSts;
UINT32                  ulGIntMsk;
UINT32                  ulStatus;
    
    /* Get core interrupt status */
    ulGIntSts = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GINTSTS2 );    /* Read interrupt status */
    ulGIntMsk = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GINTMSK2 );    /* Read interrupt mask */
    
    /* Get enabled interrupt status */
    ulStatus = ( ulGIntSts & ulGIntMsk );
    
    while( 0 != ulStatus )
    {
        if( CYCLONEVG_B01_OEPINT == ( CYCLONEVG_B01_OEPINT & ulStatus ) )
        {
            /* Out endpoint interrupt */
            _grp_cyclonevd_int_Out_EP2();
        }
        else if( CYCLONEVG_B01_IEPINT == ( CYCLONEVG_B01_IEPINT & ulStatus ) )
        {
            /* IN endpoint interrupt */
            _grp_cyclonevd_int_In_EP2();
        }
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
        else if( CYCLONEVG_B01_RXFLVL == ( CYCLONEVG_B01_RXFLVL & ulStatus ) )
        {
            /* RxFIFO non-empty */
            _grp_cyclonevd_int_RxFifoNonEmpty2();
        }
#endif
        else if( CYCLONEVG_B01_GOUTNAKEFF == ( CYCLONEVG_B01_GOUTNAKEFF & ulStatus ) )
        {
            /* Global OUT NAK effective */
            _grp_cyclonevd_int_GlobalOutNak2();
        }
        else if( CYCLONEVG_B01_MODEMIS == ( CYCLONEVG_B01_MODEMIS & ulStatus ) )
        {
            /* Mode mismatch interrupt */
            _grp_cyclonevd_int_Mode_Mismatch2();
        }
        else if( CYCLONEVG_B01_WKUPINT == ( CYCLONEVG_B01_WKUPINT & ulStatus ) )
        {
            /* Resume/remote wakeup detected interrupt */
            _grp_cyclonevd_int_Resume2();
        }
        else if( CYCLONEVG_B01_USBSUSP == ( CYCLONEVG_B01_USBSUSP & ulStatus ) )
        {
            /* USB suspend */
            _grp_cyclonevd_int_Suspend2();
        }
        else if( CYCLONEVG_B01_SOF == ( CYCLONEVG_B01_SOF & ulStatus ) )
        {
            /* Start of frame */
            _grp_cyclonevd_int_Sof2();
        }
        else if( CYCLONEVG_B01_SESSREQINT == ( CYCLONEVG_B01_SESSREQINT & ulStatus ) )
        {
            /* Session request/new session detected interrupt */
            _grp_cyclonevd_int_Session_Req2();
        }
        else if( CYCLONEVG_B01_USBRST == ( CYCLONEVG_B01_USBRST & ulStatus ) )
        {
            /* USB reset */
            _grp_cyclonevd_int_Reset2();
        }
        else if( CYCLONEVG_B01_ENUMDONE == ( CYCLONEVG_B01_ENUMDONE & ulStatus ) )
        {
            /* Enumeration done */
            _grp_cyclonevd_int_Enum_Done2();
        }
        else if( CYCLONEVG_B01_OTGINT == ( CYCLONEVG_B01_OTGINT & ulStatus ) )
        {
            /* OTG interrupt */
            _grp_cyclonevd_int_OTGInt2();
        }
        
        /* Get core interrupt status */
        ulGIntSts = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GINTSTS2 );    /* Read interrupt status */
        ulGIntMsk = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GINTMSK2 );    /* Read interrupt mask */
        
        /* Get enabled interrupt status */
        ulStatus = ( ulGIntSts & ulGIntMsk );
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_int_Out_EP2                                                      */
/*                                                                                              */
/* DESCRIPTION: Processing when OUT endpoint interruption occurred                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_int_Out_EP2( VOID )
{
GRUSB_EndPointInfo*     ptEpInfo;
INT                     iEpNo;
UINT32                  ulDaInt;
UINT32                  ulDaIntMsk;
UINT32                  ulDaIntSts;
UINT32                  ulDoEpInt;
UINT32                  ulDoEpMsk;
UINT32                  ulEpIntSts;
    
    /* Get all endpoints interrupt status */
    ulDaInt    = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DAINT2 );         /* Read interrupt status */
    ulDaIntMsk = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DAINTMSK2 );      /* Read interrupt mask */
    
    /* Get enabled OUT endpoint interrupt status */
    ulDaIntSts = ( ( ulDaInt & ulDaIntMsk ) & CYCLONEVD_B16_OUTEPMSK );
    
    for( iEpNo = 0; iEpNo < GRP_CYCLONEVD_EP_NUM; iEpNo++ )
    {
        if( ulDaIntSts & ( CYCLONEVD_VOUTEPINT_EP( iEpNo ) ) )
        {
            /* Get specified OUT endpoint interrupt status */
            ulDoEpInt = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DOEPINT2( iEpNo ) );   /* Read interrupt status */
            ulDoEpMsk = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DOEPMSK2 );            /* Read interrupt mask */
            
            /* Get enabled interrupt status */
            ulEpIntSts = ( ulDoEpInt & ulDoEpMsk );
            
            if( CYCLONEVD_B01_XFERCOMPL == ( CYCLONEVD_B01_XFERCOMPL & ulEpIntSts ) )
            {
                /* Transfer completed interrupt */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPINT2( iEpNo ), CYCLONEVD_B01_XFERCOMPL );
                
                if( GRUSB_DEV_EP0 == iEpNo )
                {
                    /* Endpoint 0 interrupt */
                    grp_cyclonevd_hal_CtrlOutTransactionIntr2();
                }
                else
                {
                    /* Get endpoint information */
                    ptEpInfo = GRUSB_Prm_GetEndPointInfo2( iEpNo );
                    
                    if( GRUSB_NULL == ptEpInfo )
                    {
                        /* Disable to specified endpoint */
                        _grp_cyclonevd_DisableEp2( iEpNo );
                    }
                    else
                    {
                        /* Endpoint x interrupt (x=1..15) */
                        grp_cyclonevd_hal_DataOutTransactionIntr2( ptEpInfo );
                    }
                }
            }
            if( CYCLONEVD_B01_SETUP == ( CYCLONEVD_B01_SETUP & ulEpIntSts ) )
            {
                /* SETUP phase done */
                
                /* Endpoint 0 (CONTROL) only */
                if( GRUSB_DEV_EP0 == iEpNo )
                {
                    /* Setup command received */
                    grp_cyclonevd_hal_CtrlSetupIntr2();
                }
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPINT2( iEpNo ), CYCLONEVD_B01_SETUPMSK );
            }
            if( CYCLONEVD_B01_NYETINTRPT == ( CYCLONEVD_B01_NYETINTRPT & ulEpIntSts ) )
            {
                /* NYET interrupt */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPINT2( iEpNo ), CYCLONEVD_B01_NYETINTRPT );
            }
            if( CYCLONEVD_B01_BACK2BACKSETUP == ( CYCLONEVD_B01_BACK2BACKSETUP & ulEpIntSts ) )
            {
                /* Back-to-back SETUP packets received */
                
                /* Endpoint 0 (CONTROL) only */
                if( GRUSB_DEV_EP0 == iEpNo )
                {
                    /* No process */
                }
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPINT2( iEpNo ), CYCLONEVD_B01_BACK2BACKSETUP );
            }
            if( CYCLONEVD_B01_OUTTKNEPDIS == ( CYCLONEVD_B01_OUTTKNEPDIS & ulEpIntSts ) )
            {
                /* OUT token received when endpoint disabled */
                
                /* Endpoint 0 (CONTROL) only */
                if( GRUSB_DEV_EP0 == iEpNo )
                {
                    /* No process */
                }
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPINT2( iEpNo ), CYCLONEVD_B01_OUTTKNEPDIS );
            }
            if( CYCLONEVD_B01_AHBERR == ( CYCLONEVD_B01_AHBERR & ulEpIntSts ) )
            {
                /* AHB error */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPINT2( iEpNo ), CYCLONEVD_B01_AHBERR );
            }
            if( CYCLONEVD_B01_EPDISBLD == ( CYCLONEVD_B01_EPDISBLD & ulEpIntSts ) )
            {
                /* Endpoint disabled interrupt */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DOEPINT2( iEpNo ), CYCLONEVD_B01_EPDISBLD );
                
                /* Get endpoint information */
                ptEpInfo = GRUSB_Prm_GetEndPointInfo2( iEpNo );
                
                if( GRUSB_NULL == ptEpInfo )
                {
                    /* Disable to specified endpoint */
                    _grp_cyclonevd_DisableEp2( iEpNo );
                }
                else
                {
                    /* OUT endpoint disabled interrupt */
                    grp_cyclonevd_hal_OutEndpointDisabledIntr2( ptEpInfo );
                }
            }
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_int_In_EP2                                                       */
/*                                                                                              */
/* DESCRIPTION: Processing when IN endpoint interruption occurred                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_int_In_EP2( VOID )
{
GRUSB_EndPointInfo*     ptEpInfo;
INT                     iEpNo;
UINT32                  ulDaInt;
UINT32                  ulDaIntMsk;
UINT32                  ulDaIntSts;
UINT32                  ulDiEpInt;
UINT32                  ulDiEpMsk;
UINT32                  ulDiEpEmpMsk;
UINT32                  ulEpIntSts;
    
    /* Get all endpoints interrupt status */
    ulDaInt    = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DAINT2 );         /* Read interrupt status */
    ulDaIntMsk = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DAINTMSK2 );      /* Read interrupt mask */
    
    /* Get enabled IN endpoint interrupt status */
    ulDaIntSts = ( ( ulDaInt & ulDaIntMsk ) & CYCLONEVD_B16_INEPMSK );
    
    for( iEpNo = 0; iEpNo < GRP_CYCLONEVD_EP_NUM; iEpNo++ )
    {
        if( ulDaIntSts & CYCLONEVD_VINEPINT_EP( iEpNo ) )
        {
            /* Get specified IN endpoint interrupt status */
            ulDiEpInt    = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ) );    /* Read interrupt status */
            ulDiEpMsk    = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DIEPMSK2 );             /* Read interrupt mask */
            ulDiEpEmpMsk = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_DIEPEMPMSK2 );          /* Read TxFIFO empty interrupt mask */
            /* Get enabled interrupt status */
            ulEpIntSts = ( ulDiEpInt & ( ulDiEpMsk | ( ( ( ulDiEpEmpMsk >> iEpNo ) & 0x00000001 ) << 7 ) ) );
            
            if( CYCLONEVD_B01_XFERCOMPL == ( CYCLONEVD_B01_XFERCOMPL & ulEpIntSts ) )
            {
                /* Transfer completed interrupt */
                
                /* Mask TxFIFO empty interrupt */
                CYCLONEV_R32_CR( CYCLONEV_A32_OTG_DIEPEMPMSK2, CYCLONEVD_VINEPTXFEMPMSK_EP( iEpNo ) );
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), CYCLONEVD_B01_XFERCOMPL );
                
                if( GRUSB_DEV_EP0 == iEpNo )
                {
                    /* Endpoint 0 interrupt */
                    grp_cyclonevd_hal_CtrlInTransactionIntr2();
                }
                else
                {
                    /* Get endpoint information */
                    ptEpInfo = GRUSB_Prm_GetEndPointInfo2( iEpNo );
                    
                    if( GRUSB_NULL == ptEpInfo )
                    {
                        /* Disable to specified endpoint */
                        _grp_cyclonevd_DisableEp2( iEpNo );
                    }
                    else
                    {
                        /* Endpoint x interrupt (x=1..15) */
                        grp_cyclonevd_hal_DataInTransactionIntr2( ptEpInfo );
                    }
                }
            }
            if( CYCLONEVD_B01_NAKINTRPT == ( CYCLONEVD_B01_NAKINTRPT & ulEpIntSts ) )
            {
                /* MAK interrupt */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), CYCLONEVD_B01_NAKMSK );
            }
            if( CYCLONEVD_B01_BBLEERR == ( CYCLONEVD_B01_BBLEERR & ulEpIntSts ) )
            {
                /* Babble error interrupt */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), CYCLONEVD_B01_BBLEERR );
            }
            if( CYCLONEVD_B01_PKTDRPSTS == ( CYCLONEVD_B01_PKTDRPSTS & ulEpIntSts ) )
            {
                /* Packet dropped status */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), CYCLONEVD_B01_PKTDRPSTS );
            }
            if( CYCLONEVD_B01_BNAINTR == ( CYCLONEVD_B01_BNAINTR & ulEpIntSts ) )
            {
                /* Buffer not available interrupt */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), CYCLONEVD_B01_BNAINTR );
            }
            if( CYCLONEVD_B01_TXFIFOUNDRN == ( CYCLONEVD_B01_TXFIFOUNDRN & ulEpIntSts ) )
            {
                /* Transmit FIFO underrun */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), CYCLONEVD_B01_TXFIFOUNDRN );
            }
            if( CYCLONEVD_B01_TXFEMP == ( CYCLONEVD_B01_TXFEMP & ulEpIntSts ) )
            {
                /* Transmit FIFO empty */
                
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
                /* Get endpoint information */
                ptEpInfo = GRUSB_Prm_GetEndPointInfo2( iEpNo );
                
                if( GRUSB_NULL == ptEpInfo )
                {
                    /* Disable to specified endpoint */
                    _grp_cyclonevd_DisableEp2( iEpNo );
                }
                else
                {
                    /* TxFIFO empty interrupt */
                    grp_cyclonevd_hal_TxFifoEmptyIntr2( ptEpInfo );
                }
#endif
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), CYCLONEVD_B01_TXFEMP );
            }
            if( CYCLONEVD_B01_INEPNAKEFF == ( CYCLONEVD_B01_INEPNAKEFF & ulEpIntSts ) )
            {
                /* IN endpoint NAK effective */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), CYCLONEVD_B01_INEPNAKEFF );
                
                if( GRUSB_DEV_EP0 != iEpNo )
                {
                    /* IN endpoint NAK effective interrupt */
                    grp_cyclonevd_hal_InEndpointNakEffectiveIntr2( iEpNo );
                }
            }
            if( CYCLONEVD_B01_INTKNTXFEMP == ( CYCLONEVD_B01_INTKNTXFEMP & ulEpIntSts ) )
            {
                /* IN token received when TxFIFO is empty */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), CYCLONEVD_B01_INTKNTXFEMP );
            }
            if( CYCLONEVD_B01_TIMEOUT == ( CYCLONEVD_B01_TIMEOUT & ulEpIntSts ) )
            {
                /* Timeout condition */
                
                /* Endpoint 0 (CONTROL) only */
                if( GRUSB_DEV_EP0 == iEpNo )
                {
                    /* No process */
                }
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), CYCLONEVD_B01_TIMEOUT );
            }
            if( CYCLONEVD_B01_AHBERR == ( CYCLONEVD_B01_AHBERR & ulEpIntSts ) )
            {
                /* AHB error */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), CYCLONEVD_B01_AHBERR );
            }
            if( CYCLONEVD_B01_EPDISBLD == ( CYCLONEVD_B01_EPDISBLD & ulEpIntSts ) )
            {
                /* Endpoint disabled interrupt */
                
                /* Clear interrupt status */
                CYCLONEV_R32_WR( CYCLONEV_A32_OTG_DIEPINT2( iEpNo ), CYCLONEVD_B01_EPDISBLD );
                
                /* Get endpoint information */
                ptEpInfo = GRUSB_Prm_GetEndPointInfo2( iEpNo );
                
                if( GRUSB_NULL == ptEpInfo )
                {
                    /* Disable to specified endpoint */
                    _grp_cyclonevd_DisableEp2( iEpNo );
                }
                else
                {
                    /* IN endpoint disabled interrupt */
                    grp_cyclonevd_hal_InEndpointDisabledIntr2( ptEpInfo );
                }
            }
        }
    }
}

#if (GRP_CYCLONEV_FIFO_ACCESS_MODE == GRP_CYCLONEV_FIFO_PIO)
/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_int_RxFifoNonEmpty2                                              */
/*                                                                                              */
/* DESCRIPTION: Processing when RxFIFO non-empty interruption occurred                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_int_RxFifoNonEmpty2( VOID )
{
    /* Processing of RxFIFO non-empty */
    grp_cyclonevd_hal_RxFifoNonEmptyIntr2();
}
#endif

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_int_GlobalOutNak2                                                */
/*                                                                                              */
/* DESCRIPTION: Processing when Global OUT NAK effective interruption occurred                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_int_GlobalOutNak2( VOID )
{
    /* Processing of Global OUT NAK effective */
    grp_cyclonevd_hal_GlobalOutNakEffectiveIntr2();
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_int_Resume2                                                      */
/*                                                                                              */
/* DESCRIPTION: Processing when RESUME/Remote wakeup interruption occurred                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_int_Resume2( VOID )
{
    /* Processing of interruption of RESUME */
    grp_cyclonevd_hal_ResumeIntr2();
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_int_Suspend2                                                     */
/*                                                                                              */
/* DESCRIPTION: Processing when SUSPEND interruption occurred                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_int_Suspend2( VOID )
{
    /* Processing of interruption of SUSPEND */
    grp_cyclonevd_hal_SuspendIntr2();
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_int_Reset2                                                       */
/*                                                                                              */
/* DESCRIPTION: Processing when RESET interruption occurred                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_int_Reset2( VOID )
{
    /* Processing of interruption of BUS_RESET */
    grp_cyclonevd_hal_BusResetIntr2();
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_int_Enum_Done2                                                   */
/*                                                                                              */
/* DESCRIPTION: Processing when Emureration done interruption occurred                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_int_Enum_Done2( VOID )
{
    /* Processing of enumeration completion */
    grp_cyclonevd_hal_EnumerationCompletion2();
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_int_Session_Req2                                                 */
/*                                                                                              */
/* DESCRIPTION: Processing when Session request/new session detected interruption occurred      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_int_Session_Req2( VOID )
{
    /* Check VBUS Status */
    if( GRUSB_TRUE == grp_cyclonevd_hal_ChackVBusStatus2() )
    {
        /* Connect */
        GRUSB_DEV_Connect2();
    }
    
    /* Interrupt clear */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GINTSTS2, CYCLONEVG_B01_SESSREQINT );
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_int_OTGInt2                                                      */
/*                                                                                              */
/* DESCRIPTION: Processing when OTG interruption occurred                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_int_OTGInt2( VOID )
{
UINT32                  ulGOtgint;
    
    /* Read interrupt register */
    ulGOtgint = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GOTGINT2 );
    
    if( CYCLONEVG_B01_SESENDDET == ( CYCLONEVG_B01_SESENDDET & ulGOtgint ) )
    {
        /* Session end detected */
        
        /* Disconnect */
        GRUSB_DEV_Disconnect2();
    }
    
    /* Interrupt clear */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GOTGINT2, ulGOtgint );
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_int_Sof2                                                         */
/*                                                                                              */
/* DESCRIPTION: Processing when SOF interruption occurred                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_int_Sof2( VOID )
{
    /* No process */
    
    /* Interrupt clear */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GINTSTS2, CYCLONEVG_B01_SOF );
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_int_Mode_Mismatch2                                               */
/*                                                                                              */
/* DESCRIPTION: Processing when mode mismatch interruption occurred                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_int_Mode_Mismatch2( VOID )
{
    /* No process */
    
    /* Interrupt clear */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GINTSTS2, CYCLONEVG_B01_MODEMIS );
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonevd_DisableEp2                                                       */
/*                                                                                              */
/* DESCRIPTION: Disable to specified endpoint                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                           Endpoint number                                 */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
LOCAL VOID _grp_cyclonevd_DisableEp2( INT iEpNo )
{
GRUSB_EndPointInfo*     ptEpInfo;
    
    if( GRUSB_DEV_EP0 != iEpNo )
    {
        /* endpoint is not 0 */
        
        /* Get endpoint information */
        ptEpInfo = GRUSB_Prm_GetEndPointInfo2( iEpNo );
        
        if( GRUSB_NULL == ptEpInfo )
        {
            /* Send disable to specified endpoint */
            grp_cyclonevd_hal_SendDisable2( iEpNo );
            
            /* Receive disable to specified endpoint */
            grp_cyclonevd_hal_RecvDisable2( iEpNo );
        }
    }
}


/* END OF grp_cyclonevd_int.c */
