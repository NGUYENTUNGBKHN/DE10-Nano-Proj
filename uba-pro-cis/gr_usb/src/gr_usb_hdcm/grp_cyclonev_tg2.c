/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2014-2015 Grape Systems, Inc.                        */
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
/*      grp_cyclonev_tg.c                                                       1.01            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB HOST/DEVICE Common module for CycloneV target function                           */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   T.Kobayashi    2014/12/24  V0.90                                                           */
/*                            - Created beta release version.                                   */
/*   T.Kobayashi    2015/01/21  V1.00                                                           */
/*                            - 1st release version.                                            */
/*   K.Jinnouchi    2017/11/21  V1.01                                                           */
/*                            - Porting to the uC3 and the Cyclone V reference bord.            */
/*                                                                                              */
/************************************************************************************************/
/* �r���h�G���[�΍�*/
#define ADDR ADDR

/**** INCLUDE FILES *****************************************************************************/
#include "kernel.h"
#include "kernel_config.h"
#include "grp_cyclonev_tg.h"
#include "grp_cyclonev_cmod.h"
#include "grp_cyclonev_macro.h"
#include "grp_cyclonev_reg.h"

#define  GRP_TG_INTNO_USB0      157
#define  GRP_TG_INTNO_USB1      160
//#ifdef SWITCH_USB0_USB1
//#define  GRP_TG_INTNO_USB       GRP_TG_INTNO_USB0		// USB0:�d��&�V���A���[�q�ׂ̗̃|�[�g �����e�i���X�p ������MRX�Ɠ����ʒu
//#else
#define  GRP_TG_INTNO_USB       GRP_TG_INTNO_USB1		// USB1:�V�݂��ꂽ�|�[�g�A�摜�]���p
//#endif
//#define  IPL_USB                224         /* ���荞�ݗD��x �l���������قǍ��D�� */
#define  IPL_USB_LEVEL          IPL_USER_NORMAL		// USB�����݃��x�����w��, 18/11/06
#define  UNUSED_VARIABLE(x) (void)(x)

extern void grp_cyclonevd_Interrupt2( void );


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL grp_s32 _grp_target_RegistIntrHandler2( void );
LOCAL grp_s32 _grp_target_EnableIntr2( void );
LOCAL grp_s32 _grp_target_DisableIntr2( void );


/************************************************************************************************/
/* FUNCTION   : grp_target_HwInit2                                                              */
/*                                                                                              */
/* DESCRIPTION: Target Initialize                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iMode                           Operation mode                                  */
/*                                                  GRP_CYCLONEV_MODE_DEVICE     Device mode    */
/*                                                  GRP_CYCLONEV_MODE_HOST       Host mode (*1) */
/*                                                  ( *1 : Not supported )                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_TARGET_OK                   Success                                         */
/*              GRP_TARGET_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_target_HwInit2( grp_si iMode )
{
grp_s32                 lStat;

    /* Registration of the interrupt handler */
    lStat = _grp_target_RegistIntrHandler2();
    
    if( GRP_TARGET_OK != lStat )
    {
        return GRP_TARGET_ERROR;
    }
    
    /* Following settings for using OTG core    */
    /*  - GPIO setting                          */
    /*  - Clock enable                          */
    /*  - USB OTG reset                         */
    /*  - Other setting (If necessary...)       */
    
    /* Core Initialize */
    lStat = grp_cyclonev_cmod_CoreInit2( iMode );
    
    if( GRP_COMMON_OK != lStat )
    {
        return GRP_TARGET_ERROR;
    }
    
    /* clear otg interrupt */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GOTGINT2, CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GOTGINT2 ) );
    /* Set soft disconnect */
    CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DCTL2, CYCLONEVD_B01_SFTDISCON );

    return GRP_TARGET_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_target_StartIntr2                                                           */
/*                                                                                              */
/* DESCRIPTION: Start Interrupt                                                                 */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iMode                           Operation mode                                  */
/*                                                  GRP_CYCLONEV_MODE_DEVICE     Device mode    */
/*                                                  GRP_CYCLONEV_MODE_HOST       Host mode (*1) */
/*                                                  ( *1 : Not supported )                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_TARGET_OK                   Success                                         */
/*              GRP_TARGET_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_target_StartIntr2( grp_si iMode )
{
    /* Clear global interrupt */
    grp_cyclonev_cmod_ClearGlobalIntr2();
    
    if( GRP_CYCLONEV_MODE_DEVICE == iMode )
    {
        /* Enable device mode interrupts */
        grp_cyclonev_cmod_EnableDeviceIntr2();
    }
    else if( GRP_CYCLONEV_MODE_HOST == iMode )
    {
        /* Enable host mode interrupts */
        return GRP_TARGET_ERROR;
    }
    
    /* Enable interrupt */
    _grp_target_EnableIntr2();
    
    /* Enable global interrupt */
    grp_cyclonev_cmod_EnableGlobalIntr2();
    
    return GRP_TARGET_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_target_StopIntr2                                                            */
/*                                                                                              */
/* DESCRIPTION: Stop Interrupt                                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iMode                           Operation mode                                  */
/*                                                  GRP_CYCLONEV_MODE_DEVICE     Device mode    */
/*                                                  GRP_CYCLONEV_MODE_HOST       Host mode (*1) */
/*                                                  ( *1 : Not supported )                      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_TARGET_OK                   Success                                         */
/*              GRP_TARGET_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_target_StopIntr2( grp_si iMode )
{
    /* Disable global interrupt */
    grp_cyclonev_cmod_DisableGlobalIntr2();
    
    /* Disable interrupt */
    _grp_target_DisableIntr2();
    
    if( GRP_CYCLONEV_MODE_DEVICE == iMode )
    {
        /* Disable device mode interrupts */
        grp_cyclonev_cmod_DisableDeviceIntr2();
    }
    else if( GRP_CYCLONEV_MODE_HOST == iMode )
    {
        /* Disable host mode interrupts */
        return GRP_TARGET_ERROR;
    }
    
    return GRP_TARGET_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_target_DelayMS2                                                             */
/*                                                                                              */
/* DESCRIPTION: Delay time in msec                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulMsec                          Delay time (msec)                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void grp_target_DelayMS2( grp_u32 ulMsec )
{
    dly_tsk( ulMsec );
}

/************************************************************************************************/
/* FUNCTION   : _grp_target_RegistIntrHandler2                                                  */
/*                                                                                              */
/* DESCRIPTION: Registration of the interrupt handler                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_TARGET_OK                   Success                                         */
/*              GRP_TARGET_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_target_RegistIntrHandler2( void )
{
#if defined(PRJ_OS_UC3)
    T_CISR cisr = {TA_HLNG, (VP_INT)0, GRP_TG_INTNO_USB, (FP)grp_target_OtgIsr2, IPL_USB_LEVEL};
    
    if (E_OK > acre_isr(&cisr)) {
        return GRP_TARGET_ERROR;
    }
//#elif defined(PRJ_OS_THREADX_UITRON)
//    T_DINH dinh = {0, (FP)grp_target_OtgIsr2};
//
//    if (E_OK > def_inh(GRP_TG_INTNO_USB, &dinh)) {
//        return GRP_TARGET_ERROR;
//    }
//    set_irq_priority(GRP_TG_INTNO_USB, IPL_USB_LEVEL);
#else
    OSW_ISR_HANDLE handle;

	if( OSW_ISR_create( &handle, GRP_TG_INTNO_USB, (osw_isr_func)grp_target_OtgIsr2 ) == FALSE )
	{
        return GRP_TARGET_ERROR;
	}
#if 1  /* ID-0G8 */
	OSW_ISR_set_priority(GRP_TG_INTNO_USB, IPL_KERNEL_HIGH);
#else  /* ID-0G8 */
	OSW_ISR_set_priority(GRP_TG_INTNO_USB, IPL_USER_NORMAL);
#endif /* ID-0G8 */
#endif
        
    return GRP_TARGET_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_target_EnableIntr2                                                         */
/*                                                                                              */
/* DESCRIPTION: Enable interrupt                                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_TARGET_OK                   Success                                         */
/*              GRP_TARGET_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_target_EnableIntr2( void )
{
#if defined(PRJ_OS_UC3)
    ena_int(GRP_TG_INTNO_USB);
//#elif defined(PRJ_OS_THREADX_UITRON)
//    enable_irq_id(GRP_TG_INTNO_USB);
#else
    OSW_ISR_enable(GRP_TG_INTNO_USB);
#endif
    
    return GRP_TARGET_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_target_DisableIntr2                                                        */
/*                                                                                              */
/* DESCRIPTION: Disable interrupt                                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_TARGET_OK                   Success                                         */
/*              GRP_TARGET_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_s32 _grp_target_DisableIntr2( void )
{
#if defined(PRJ_OS_UC3)
    dis_int(GRP_TG_INTNO_USB);
//#elif defined(PRJ_OS_THREADX_UITRON)
//    disable_irq_id(GRP_TG_INTNO_USB);
#else
    OSW_ISR_disable(GRP_TG_INTNO_USB);
#endif
    
    return GRP_TARGET_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_target_OtgIsr2                                                              */
/*                                                                                              */
/* DESCRIPTION: Isr function for OTG                                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : icciar                          ICCIAR register information                     */
/*              context                         Extended information                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void grp_target_OtgIsr2( grp_u32 icciar, void * context )
{
grp_si                  iMode;

    UNUSED_VARIABLE(context);
    UNUSED_VARIABLE(icciar);

    /* Get operation mode */
    iMode = grp_cyclonev_cmod_GetMode2();
    
    if( GRP_CYCLONEV_MODE_DEVICE == iMode )
    {
        /* Device mode interrupt */
        
        /* Call interrupt handler of pcd driver */
        grp_cyclonevd_Interrupt2();
    }
    else if( GRP_CYCLONEV_MODE_HOST == iMode )
    {
        /* Host mode interrupt */
        
    }
}

/************************************************************************************************/
/* FUNCTION   : grp_target_BufferAddressError2                                                  */
/*                                                                                              */
/* DESCRIPTION: Buffer adress error ( Not DWORD-aligned )                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                        Endpoint No                                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void grp_target_BufferAddressError2( grp_si iEpNo )
{
    /* If necessary, here please add the code */
    
    while(1);
}

#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
 #if (GRP_CYCLONEV_USE_DCACHE_TYPE != GRP_CYCLONEV_DCACHE_INVALID)
/************************************************************************************************/
/* FUNCTION   : grp_target_DataCacheInvalidate2                                                 */
/*                                                                                              */
/* DESCRIPTION: Data cache invalidate                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : uiAddress                       Data start address                              */
/*              uiLength                        Data length                                     */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_TARGET_OK                   Success                                         */
/*              GRP_TARGET_ERROR                Error                                           */
/*                                                                                              */
/************************************************************************************************/
grp_s32 grp_target_DataCacheInvalidate2( grp_ui uiAddress, grp_ui uiLength )
{
#if 0
    ���������������Ă�������
    
    return GRP_TARGET_OK;
#else
	tx_alt_cache_purge_all();
	
	return GRP_TARGET_OK;
#endif
}
 #endif
#endif

/* END OF grp_cyclonev_tg.c */
