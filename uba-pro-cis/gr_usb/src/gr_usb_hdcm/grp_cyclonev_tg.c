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

#include "grp_usb_mdchg.h"        /* for mdchg */

#define  GRP_TG_INTNO_USB0      157
#define  GRP_TG_INTNO_USB1      160

//#ifdef SWITCH_USB0_USB1
//#define  GRP_TG_INTNO_USB       GRP_TG_INTNO_USB1		// USB1:�V�݂��ꂽ�|�[�g�A�摜�]���p
//#else
#define  GRP_TG_INTNO_USB       GRP_TG_INTNO_USB0		// USB0:�d��&�V���A���[�q�ׂ̗̃|�[�g �����e�i���X�p ������MRX�Ɠ����ʒu
//#endif
//#define  IPL_USB                224         /* ���荞�ݗD��x �l���������قǍ��D�� */
#define  IPL_USB_LEVEL          IPL_USER_NORMAL		// USB�����݃��x�����w��, 18/11/06
#define  UNUSED_VARIABLE(x) (void)(x)

grp_u32	gRegIntFlag = 0;

extern void grp_cyclonevd_Interrupt( void );
extern void grp_cyclonevh_Interrupt( void );

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL grp_s32 _grp_target_RegistIntrHandler( void );
LOCAL grp_s32 _grp_target_EnableIntr( void );
LOCAL grp_s32 _grp_target_DisableIntr( void );


/************************************************************************************************/
/* FUNCTION   : grp_target_HwInit                                                               */
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
grp_s32 grp_target_HwInit( grp_si iMode )
{
grp_s32                 lStat;

    /* Registration of the interrupt handler */
    lStat = _grp_target_RegistIntrHandler();
    
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
    lStat = grp_cyclonev_cmod_CoreInit( iMode );
    
    if( GRP_COMMON_OK != lStat )
    {
        return GRP_TARGET_ERROR;
    }
    
    /* clear otg interrupt */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GOTGINT, CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GOTGINT ) );
    /* Set soft disconnect */
    CYCLONEV_R32_ST( CYCLONEV_A32_OTG_DCTL, CYCLONEVD_B01_SFTDISCON );

    return GRP_TARGET_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_target_StartIntr                                                            */
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
grp_s32 grp_target_StartIntr( grp_si iMode )
{
    /* Clear global interrupt */
    grp_cyclonev_cmod_ClearGlobalIntr();
    
    if( GRP_CYCLONEV_MODE_DEVICE == iMode )
    {
        /* Enable device mode interrupts */
        grp_cyclonev_cmod_EnableDeviceIntr();
    }
    else if( GRP_CYCLONEV_MODE_HOST == iMode )
    {
        /* Enable host mode interrupts */
        grp_cyclonev_cmod_EnableHostIntr();
// %%GR
//        return GRP_TARGET_ERROR;
    }
    
    /* Enable interrupt */
    _grp_target_EnableIntr();
    
    /* Enable global interrupt */
    grp_cyclonev_cmod_EnableGlobalIntr();
    
    return GRP_TARGET_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_target_StopIntr                                                             */
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
grp_s32 grp_target_StopIntr( grp_si iMode )
{
    /* Disable global interrupt */
    grp_cyclonev_cmod_DisableGlobalIntr();
    
    /* Disable interrupt */
    _grp_target_DisableIntr();
    
    if( GRP_CYCLONEV_MODE_DEVICE == iMode )
    {
        /* Disable device mode interrupts */
        grp_cyclonev_cmod_DisableDeviceIntr();
    }
    else if( GRP_CYCLONEV_MODE_HOST == iMode )
    {
        /* Disable host mode interrupts */
        grp_cyclonev_cmod_DisableHostIntr();
// %%GR
//        return GRP_TARGET_ERROR;
    }
    
    return GRP_TARGET_OK;
}

/************************************************************************************************/
/* FUNCTION   : grp_target_DelayMS                                                              */
/*                                                                                              */
/* DESCRIPTION: Delay time in msec                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ulMsec                          Delay time (msec)                               */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void grp_target_DelayMS( grp_u32 ulMsec )
{
	/*
	 * Make sure the interrupt priority of the system timer is
	 * higher than the USB0/1 interrupt priority.
	 * */
    dly_tsk( ulMsec );
}

/************************************************************************************************/
/* FUNCTION   : _grp_target_RegistIntrHandler                                                   */
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
LOCAL grp_s32 _grp_target_RegistIntrHandler( void )
{
	/* ���Ɋ��荞�ݓo�^�ς݂̏ꍇ */
	if (gRegIntFlag)
	{
		return GRP_TARGET_OK;
	}

#if defined(PRJ_OS_UC3)
    T_CISR cisr = {TA_HLNG, (VP_INT)0, GRP_TG_INTNO_USB, (FP)grp_target_OtgIsr, IPL_USB_LEVEL};
    
    if (E_OK > acre_isr(&cisr)) {
        return GRP_TARGET_ERROR;
    }
//#elif defined(PRJ_OS_THREADX_UITRON)
//    T_DINH dinh = {0, (FP)grp_target_OtgIsr};
//
//    if (E_OK > def_inh(GRP_TG_INTNO_USB, &dinh)) {
//        return GRP_TARGET_ERROR;
//    }
//    set_irq_priority(GRP_TG_INTNO_USB, IPL_USB_LEVEL);
#else
    OSW_ISR_HANDLE handle;

	if( OSW_ISR_create( &handle, GRP_TG_INTNO_USB, (osw_isr_func)grp_target_OtgIsr ) == FALSE )
	{
        return GRP_TARGET_ERROR;
	}
	OSW_ISR_set_priority(GRP_TG_INTNO_USB, IPL_USER_NORMAL);

#endif
        
    gRegIntFlag = 1;
    return GRP_TARGET_OK;
}

/************************************************************************************************/
/* FUNCTION   : _grp_target_EnableIntr                                                          */
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
LOCAL grp_s32 _grp_target_EnableIntr( void )
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
/* FUNCTION   : _grp_target_DisableIntr                                                         */
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
LOCAL grp_s32 _grp_target_DisableIntr( void )
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
/* FUNCTION   : grp_target_OtgIsr                                                               */
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
void grp_target_OtgIsr( grp_u32 icciar, void * context )
{
grp_si                  iMode;
/* for mdchg */
UINT32                  ulGIntSts;
UINT32                  ulGIntMsk;
UINT32                  ulStatus;

    UNUSED_VARIABLE(context);
    UNUSED_VARIABLE(icciar);

#if 1
    /* for mdchg */
    /*
     *  Connector ID ���ω����Ă���ꍇ�A���[�h�؂�ւ����������{
     *  Connector ID ���ω����Ă��Ȃ��ꍇ�A���[�h�ɑΉ��������荞�ݏ��������{
     */
    /* Get core interrupt status */
    ulGIntSts = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GINTSTS );    /* Read interrupt status */
    ulGIntMsk = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GINTMSK );    /* Read interrupt mask */

    /* Get enabled interrupt status */
    ulStatus = ( ulGIntSts & ulGIntMsk );

    /* connector ID change */
    if( CYCLONEVG_B01_CONIDSTSCHNG == ( CYCLONEVG_B01_CONIDSTSCHNG & ulStatus ) )
    {
    	grp_usb_mdchg_ConidIsr();
    	CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GINTSTS,  ulGIntSts);    /* Clear interrupt status */
    	return;
    }
#endif

    /* Get operation mode */
    iMode = grp_cyclonev_cmod_GetMode();
    
    if( GRP_CYCLONEV_MODE_DEVICE == iMode )
    {
        /* Device mode interrupt */
        
        /* Call interrupt handler of pcd driver */
        grp_cyclonevd_Interrupt();
    }
    else if( GRP_CYCLONEV_MODE_HOST == iMode )
    {
        /* Host mode interrupt */
        grp_cyclonevh_Interrupt();
// %%GR
//      grp_target_BufferAddressError(iMode);

    }
}

/************************************************************************************************/
/* FUNCTION   : grp_target_BufferAddressError                                                   */
/*                                                                                              */
/* DESCRIPTION: Buffer adress error ( Not DWORD-aligned )                                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iEpNo                        Endpoint No                                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void grp_target_BufferAddressError( grp_si iEpNo )
{
    /* If necessary, here please add the code */
    
    while(1);
}

#if (GRP_CYCLONEV_FIFO_ACCESS_MODE != GRP_CYCLONEV_FIFO_PIO)
 #if (GRP_CYCLONEV_USE_DCACHE_TYPE != GRP_CYCLONEV_DCACHE_INVALID)
/************************************************************************************************/
/* FUNCTION   : grp_target_DataCacheInvalidate                                                  */
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
grp_s32 grp_target_DataCacheInvalidate( grp_ui uiAddress, grp_ui uiLength )
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
