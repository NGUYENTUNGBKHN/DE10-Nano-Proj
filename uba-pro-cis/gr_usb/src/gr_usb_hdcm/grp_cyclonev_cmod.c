/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2014-2019 Grape Systems, Inc.                        */
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
/*      grp_cyclonev_cmod.c                                                     1.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB HOST/DEVICE Common module for CycloneV                                           */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   T.Kobayashi    2014/12/24  V0.90                                                           */
/*                            - Created beta release version.                                   */
/*   T.Kobayashi    2015/01/21  V1.00                                                           */
/*                            - 1st release version.                                            */
/*   K.Kaneko       2019/04/26  V1.01                                                           */
/*                            - Added support for host mode.                                    */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_cyclonev_tg.h"
#include "grp_cyclonev_cmod.h"


/**** LOCAL DATA DEFINES ************************************************************************/


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL grp_s32 _grp_cyclonev_cmod_CoreReset( void );
LOCAL grp_s32 _grp_cyclonev_cmod_PhyClockReset( void );


/************************************************************************************************/
/* FUNCTION   : grp_cyclonev_cmod_CoreInit                                                      */
/*                                                                                              */
/* DESCRIPTION: Core Initialize                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iMode                           Operation mode                                  */
/*                                                  GRP_CYCLONEV_MODE_DEVICE     Device mode    */
/*                                                  GRP_CYCLONEV_MODE_HOST       Host mode      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonev_cmod_CoreInit( grp_si iMode )
{
grp_s32                 lStat;
grp_u32                 ulGAhbCfg;
grp_u32                 ulGUsbCfg;
    
    /* Check operation mode */
    if(( GRP_CYCLONEV_MODE_DEVICE != iMode ) && ( GRP_CYCLONEV_MODE_HOST != iMode ))
    {
        return GRP_COMMON_ERROR;    /* DIRECT RETURN */
    }
    
    /* Core soft reset */
    lStat = _grp_cyclonev_cmod_CoreReset();
    
    if( GRP_COMMON_OK !=lStat )
    {
        return GRP_COMMON_ERROR;    /* DIRECT RETURN */
    }
    
    /* Set operation mode */
    lStat = grp_cyclonev_cmod_SetMode( iMode );
    
    if( GRP_COMMON_OK !=lStat )
    {
        return GRP_COMMON_ERROR;    /* DIRECT RETURN */
    }
    
    /* USB turnaround time(device only) */
    if( GRP_CYCLONEV_MODE_DEVICE == iMode )
    {
    /* Read USB configuration register */
    ulGUsbCfg = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GUSBCFG );
    
    ulGUsbCfg &= ~CYCLONEVG_B04_USBTRDTIM;          /* USB turnaround time  */
    ulGUsbCfg |= CYCLONEVG_VUSBTRDTIM_8BIT_UTMIP;   /* USB turnaround time  */
    
    /* Write USB configuration register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GUSBCFG, ulGUsbCfg );
    
    grp_target_DelayMS( 50 );
    }
    
    /* Read AHB configuration register */
    ulGAhbCfg = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GAHBCFG );
    
    /* Clear burst length/type */
    ulGAhbCfg &= ~CYCLONEVG_B04_HBSTLEN;
    /* Set burst length/type */
    ulGAhbCfg |= CYCLONEVG_VHBSTLEN_WORD64ORINCR8;
    
#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
    /* Set DMA enable */
    ulGAhbCfg |= CYCLONEVG_B01_DMAEN;
#endif
    
    /* Write AHB configuration register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GAHBCFG, ulGAhbCfg );
    
    /* PHY clock reset */
    lStat = _grp_cyclonev_cmod_PhyClockReset();
    
    if( GRP_COMMON_OK !=lStat )
    {
        return GRP_COMMON_ERROR;    /* DIRECT RETURN */
    }
    
    return GRP_COMMON_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonev_cmod_EnableGlobalIntr                                              */
/*                                                                                              */
/* DESCRIPTION: Enable global interrupt                                                         */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonev_cmod_EnableGlobalIntr( void )
{
    /* Unmask the global interrupt */
    CYCLONEV_R32_ST( CYCLONEV_A32_OTG_GAHBCFG, CYCLONEVG_B01_GLBLINTRMSK );
    
    return GRP_COMMON_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonev_cmod_DisableGlobalIntr                                             */
/*                                                                                              */
/* DESCRIPTION: Disable global interrupt                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonev_cmod_DisableGlobalIntr( void )
{
    /* Mask the global interrupt */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_GAHBCFG, CYCLONEVG_B01_GLBLINTRMSK );
    
    return GRP_COMMON_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonev_cmod_ClearGlobalIntr                                               */
/*                                                                                              */
/* DESCRIPTION: Clear global interrupt                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonev_cmod_ClearGlobalIntr( void )
{
    /* Disable all interrupts */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GINTMSK, 0x00000000 );
    
    /* Clear any pending interrupts */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GINTSTS, 0xFFFFFFFF );
    
    return GRP_COMMON_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonev_cmod_EnableDeviceIntr                                              */
/*                                                                                              */
/* DESCRIPTION: Enable device mode interrupts                                                   */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonev_cmod_EnableDeviceIntr( void )
{
grp_u32                 ulGIntMsk;
    
    /* Enable interrupt bits */
    ulGIntMsk = ( CYCLONEVG_B01_WKUPINTMSK      /* Resume/remote wakeup detected interrupt          */
                | CYCLONEVG_B01_SESSREQINTMSK   /* Session request/new session detected interrupt   */
                | CYCLONEVG_B01_OEPINTMSK       /* OUT endpoints interrupt                          */
                | CYCLONEVG_B01_IEPINTMSK       /* IN endpoints interrupt                           */
                | CYCLONEVG_B01_ENUMDONEMSK     /* Enumeration done                                 */
                | CYCLONEVG_B01_USBRSTMSK       /* USB reset                                        */
                | CYCLONEVG_B01_USBSUSPMSK      /* USB suspend                                      */
                | CYCLONEVG_B01_OTGINTMSK     /* OTG interrupt                                    */
                | CYCLONEVG_B01_CONIDSTSCHNGMSK ); /* Connector ID Change interrupt            */
    
    /* Enable interrupts */
    CYCLONEV_R32_ST( CYCLONEV_A32_OTG_GINTMSK, ulGIntMsk );
    
    return GRP_COMMON_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonev_cmod_DisableDeviceIntr                                             */
/*                                                                                              */
/* DESCRIPTION: Disable device mode interrupts                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonev_cmod_DisableDeviceIntr( void )
{
grp_u32                 ulGIntMsk;
    
    /* Disable interrupt bits */
    ulGIntMsk = ( CYCLONEVG_B01_WKUPINTMSK      /* Resume/remote wakeup detected interrupt          */
                | CYCLONEVG_B01_SESSREQINTMSK   /* Session request/new session detected interrupt   */
                | CYCLONEVG_B01_OEPINTMSK       /* OUT endpoints interrupt                          */
                | CYCLONEVG_B01_IEPINTMSK       /* IN endpoints interrupt                           */
                | CYCLONEVG_B01_ENUMDONEMSK     /* Enumeration done                                 */
                | CYCLONEVG_B01_USBRSTMSK       /* USB reset                                        */
                | CYCLONEVG_B01_USBSUSPMSK      /* USB suspend                                      */
                | CYCLONEVG_B01_OTGINTMSK     /* OTG interrupt                                    */
                | CYCLONEVG_B01_CONIDSTSCHNGMSK );    /* Connector ID Change interrupt            */
    
    /* Disable interrupts */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_GINTMSK, ulGIntMsk );
    
    return GRP_COMMON_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonev_cmod_EnableHostIntr                                                */
/*                                                                                              */
/* DESCRIPTION: Enable host mode interrupts                                                     */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonev_cmod_EnableHostIntr( void )
{
grp_u32                 ulGIntMsk;
    
    /* Enable host channels interrupt bit */
    ulGIntMsk = (CYCLONEVG_B01_HCHINTMSK    /* host channels interrupt bit */
                | CYCLONEVG_B01_CONIDSTSCHNGMSK );    /* Connector ID Change interrupt */
    
    /* Enable interrupts */
    CYCLONEV_R32_ST( CYCLONEV_A32_OTG_GINTMSK, ulGIntMsk );
    
    return GRP_COMMON_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonev_cmod_DisableHostIntr                                               */
/*                                                                                              */
/* DESCRIPTION: Disable host mode interrupts                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonev_cmod_DisableHostIntr( void )
{
grp_u32                 ulGIntMsk;
    
    /* Disable host channels interrupt bit */
    ulGIntMsk = (CYCLONEVG_B01_HCHINTMSK    /* host channels interrupt bit */
                | CYCLONEVG_B01_CONIDSTSCHNGMSK );    /* Connector ID Change interrupt */
    
    /* Disable interrupts */
    CYCLONEV_R32_CR( CYCLONEV_A32_OTG_GINTMSK, ulGIntMsk );
    
    return GRP_COMMON_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonev_cmod_FlushTxFifo                                                   */
/*                                                                                              */
/* DESCRIPTION: Flush the TxFIFO                                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iFnum                           FIFO number                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonev_cmod_FlushTxFifo( grp_si iFnum )
{
grp_u32                 ulGRstCtl;
grp_si                  i;
    
    /* Set TxFIFO flush */
    ulGRstCtl = CYCLONEVG_B01_TXFFLSH;
    
    /* Set TxFIFO number */
    ulGRstCtl |= (grp_u32)( iFnum << 6 );
    
    /* Write reset register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GRSTCTL, ulGRstCtl );
        
    for( i = 0; i < GRP_CYCLONEV_RETRY_COUNT; i++ )
    {
        /* Read reset register */
        ulGRstCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GRSTCTL );
        
        /* Check TxFIFO flush */
        if( CYCLONEVG_B01_TXFFLSH != ( CYCLONEVG_B01_TXFFLSH & ulGRstCtl ) )
        {
            break;
        }
    }
    
    if( GRP_CYCLONEV_RETRY_COUNT == i )
    {
        /* Time out error */
        return GRP_COMMON_ERROR;    /* DIRECT RETURN */
    }
    
    grp_target_DelayMS( 10 );
    
    return GRP_COMMON_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonev_cmod_FlushRxFifo                                                   */
/*                                                                                              */
/* DESCRIPTION: Flush the RxFIFO                                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonev_cmod_FlushRxFifo( void )
{
grp_u32                 ulGRstCtl;
grp_si                  i;
    
    /* Set RxFIFO flush */
    ulGRstCtl = CYCLONEVG_B01_RXFFLSH;
    
    /* Write reset register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GRSTCTL, ulGRstCtl );
        
    for( i = 0; i < GRP_CYCLONEV_RETRY_COUNT; i++ )
    {
        /* Read reset register */
        ulGRstCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GRSTCTL );
        
        /* Check RxFIFO flush */
        if( CYCLONEVG_B01_RXFFLSH != ( CYCLONEVG_B01_RXFFLSH & ulGRstCtl ) )
        {
            break;
        }
    }
    
    if( GRP_CYCLONEV_RETRY_COUNT == i )
    {
        /* Time out error */
        return GRP_COMMON_ERROR;    /* DIRECT RETURN */
    }
    
    grp_target_DelayMS( 10 );
    
    return GRP_COMMON_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonev_cmod_SetMode                                                       */
/*                                                                                              */
/* DESCRIPTION: Set operation mode                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iMode                           Operation mode                                  */
/*                                                  GRP_CYCLONEV_MODE_DEVICE     Device mode    */
/*                                                  GRP_CYCLONEV_MODE_HOST       Host mode      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_cyclonev_cmod_SetMode( grp_si iMode )
{
grp_u32                 ulGUsbCfg;
    
    /* Check operation mode */
    if(( GRP_CYCLONEV_MODE_DEVICE != iMode ) && ( GRP_CYCLONEV_MODE_HOST != iMode ))
    {
        return GRP_COMMON_ERROR;    /* DIRECT RETURN */
    }
    
    /* Read USB configuration register */
    ulGUsbCfg = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GUSBCFG );
    
    if( GRP_CYCLONEV_MODE_DEVICE == iMode )
    {
        /* Clear forced host mode */
        ulGUsbCfg &= ~CYCLONEVG_B01_FORCEHSTMODE;
        /* Set forced device mode */
 //       ulGUsbCfg |= CYCLONEVG_B01_FORCEDEVMODE;
    }
    else if( GRP_CYCLONEV_MODE_HOST == iMode )
    {
        /* Clear forced device mode */
        ulGUsbCfg &= ~CYCLONEVG_B01_FORCEDEVMODE;
        /* Set forced host mode */
 //       ulGUsbCfg |= CYCLONEVG_B01_FORCEHSTMODE;
    }
    
    /* Write USB configuration register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GUSBCFG, ulGUsbCfg );
    
    return GRP_COMMON_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_cyclonev_cmod_GetMode                                                       */
/*                                                                                              */
/* DESCRIPTION: Get operation mode                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Operation mode                                                                  */
/*                                                                                              */
/************************************************************************************************/
grp_si grp_cyclonev_cmod_GetMode( void )
{
grp_u32                 ulGIntSts;
    
    /* Read core interrupt register */
    ulGIntSts = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GINTSTS );
    
    /* Return current operation mode */
    return (grp_si)( CYCLONEVG_B01_CURMOD & ulGIntSts );
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonev_cmod_CoreReset                                                    */
/*                                                                                              */
/* DESCRIPTION: Core soft reset                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonev_cmod_CoreReset( void )
{
grp_u32                 ulGRstCtl;
grp_si                  i;
    
    for( i = 0; i < GRP_CYCLONEV_RETRY_COUNT; i++ )
    {
        /* Read reset register */
        ulGRstCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GRSTCTL );
        
        /* Check AHB master idle */
        if( CYCLONEVG_B01_AHBIDLE == ( CYCLONEVG_B01_AHBIDLE & ulGRstCtl ) )
        {
            break;
        }
    }
    
    if( GRP_CYCLONEV_RETRY_COUNT == i )
    {
        /* Time out error */
        return GRP_COMMON_ERROR;    /* DIRECT RETURN */
    }
    
    /* Set core soft reset */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GRSTCTL, CYCLONEVG_B01_CSFTRST );
    
    grp_target_DelayMS( 10 );
    
    for( i = 0; i < GRP_CYCLONEV_RETRY_COUNT; i++ )
    {
        /* Read reset register */
        ulGRstCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GRSTCTL );
        
        /* Check core soft reset */
        if( CYCLONEVG_B01_CSFTRST != ( CYCLONEVG_B01_CSFTRST & ulGRstCtl ) )
        {
            break;
        }
    }
    
    if( GRP_CYCLONEV_RETRY_COUNT == i )
    {
        /* Time out error */
        return GRP_COMMON_ERROR;    /* DIRECT RETURN */
    }
    
    return GRP_COMMON_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_cyclonev_cmod_PhyClockReset                                                */
/*                                                                                              */
/* DESCRIPTION: PHY clock reset                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_COMMON_OK                   Success                                         */
/*              GRP_COMMON_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_cyclonev_cmod_PhyClockReset( void )
{
    /* Reset power and clock gating control register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_PCGCCTL, 0x00000000 );
    
    return GRP_COMMON_OK;
}

/* END OF grp_cyclonev_cmod.c */
