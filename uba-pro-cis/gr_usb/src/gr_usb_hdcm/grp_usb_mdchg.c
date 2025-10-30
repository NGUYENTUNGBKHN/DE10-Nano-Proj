/************************************************************************************************/
/*                                                                                              */
/*                             Copyright(C) 2020 Grape Systems, Inc.                            */
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
/*      grp_usb_mdchg.c                                                         1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB DEVICE/HOST mode change module                                                   */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   H.Kato         2020/01/10  V1.00                                                           */
/*                            - Created 1st release version.                                    */
/*                                                                                              */
/************************************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "kernel.h"

#include "grp_usbc_types.h"
#include "perid.h"
#include "grp_cyclonev_reg.h"
#include "grp_usbc_reinit.h"    /* grp_reinit_HostReInit, _not_initialized */
#include "grp_usbc.h"           /* grp_usbc_Disable */
#include "grp_usb_mdchg.h"

#include "grp_vos.h"	            /* grp_vos_CreateQueue */
#include "grp_cyclonev_cmod.h" /* grp_cyclonev_cmod_GetMode */
#include "grp_cyclonevh.h"    /* grp_cyclonevh_Interrupt */


/**** DEFINES ***********************************************************************************/
/* OTGSC */
#define     OTGSC_CHECK_MASK    (0x7F0100FF)
#define     OTGSC_B_DEVICE      (0x00000100)

/* 1st Interrupt check */
#define     NOT_INTR_OCCUR      (0)
#define     INTR_OCCUR          (1)

#define  UNUSED_VARIABLE(x) (void)(x) /* for grp_mdchg_target_OtgIsr() */


/**** INTERNAL VARIABLES ************************************************************************/
LOCAL GRP_USB_MDCHG_INITPARAM   l_tInitParam    = {0, 0};

DLOCAL grp_vos_t_queue*         l_ptMdchgQue = NULL;    /* Notice Mdchg Queue   */


/**** EXTERNAL FUNCTIONS ************************************************************************/


/************************************************************************************************/
/* FUNCTION   : grp_usb_mdchg_Init                                                              */
/*                                                                                              */
/* DESCRIPTION: Module initialization                                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_MDCHG_OK                Success                                             */
/*              GRP_MDCHG_NG                Error                                               */
/*                                                                                              */
/************************************************************************************************/
INT grp_usb_mdchg_Init( VOID )
{
INT         iRet = GRP_MDCHG_OK;
grp_s32                         lResult;

	if (!_not_initialized)
	{
		return GRP_MDCHG_OK;
	}

    /* ���荞�ݖ��� */
    if (GRP_USBC_OK != grp_usbc_Disable())
    {
        iRet= GRP_MDCHG_NG;
    }
    else
    {
        /* GR-VOS */
        if (grp_vos_Init() != GRP_VOS_POS_RESULT) {
            /* error */
            return GRP_MDCHG_NG;
        }

    	/* Create queue */
        lResult = grp_vos_CreateQueue(&l_ptMdchgQue, (grp_u8*)"qMdchg", 4, 1);
        if (lResult != GRP_VOS_POS_RESULT)
        {
            /* error */
        	iRet = GRP_MDCHG_NG;
        }
    }

    return iRet;
}

/************************************************************************************************/
/* FUNCTION   : grp_usb_mdchg_SetFunc                                                              */
/*                                                                                              */
/* DESCRIPTION: Set USB Host/Device Initialize Function                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pfnUsr1stHostInit           USB host initialize function                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_MDCHG_OK                Success                                             */
/*              GRP_MDCHG_NG                Error                                               */
/*                                                                                              */
/************************************************************************************************/
INT grp_usb_mdchg_SetFunc( GRP_USB_MDCHG_INITPARAM *ptParam )
{
INT         iRet = GRP_MDCHG_OK;

    /* Save user function */
    l_tInitParam.pfn1stDevInitFunc      =   ptParam->pfn1stDevInitFunc;
    l_tInitParam.pfn2ndDevInitFunc      =   ptParam->pfn2ndDevInitFunc;
    l_tInitParam.pfn1stHostInitFunc     =   ptParam->pfn1stHostInitFunc;

    return iRet;
}

/************************************************************************************************/
/* FUNCTION   : grp_usb_mdchg_WaitMdChg                                                         */
/*                                                                                              */
/* DESCRIPTION: Registration of the interrupt handler                                           */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : pulMode                     Device or Host mode                                 */
/*                                                                                              */
/* RESULTS    : GRP_MDCHG_OK                Success                                             */
/*              GRP_MDCHG_NG                Error                                               */
/*                                                                                              */
/************************************************************************************************/
INT grp_usb_mdchg_WaitMdChg( UINT32 *pulMode )
{
INT     iRet = GRP_MDCHG_OK;

//    if (E_OK != rcv_dtq(l_iMdChgQID, ( VP_INT * )pulMode))
    if (grp_vos_ReceiveQueue(l_ptMdchgQue, (void *)pulMode, GRP_VOS_INFINITE) != GRP_VOS_POS_RESULT)
    {
        iRet = GRP_MDCHG_NG;
    }

    return iRet;
}

/************************************************************************************************/
/* FUNCTION   : grp_usb_mdchg_BootDevice                                                        */
/*                                                                                              */
/* DESCRIPTION: Boot device mode                                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_MDCHG_OK                Success                                             */
/*              GRP_MDCHG_NG                Error                                               */
/*                                                                                              */
/************************************************************************************************/
INT grp_usb_mdchg_BootDevice(UINT32 Port2nd)
{
INT     iRet = GRP_MDCHG_OK;

    /* ���荞�ݖ��� */
    if (GRP_USBC_OK != grp_usbc_Disable())
    {
        iRet= GRP_MDCHG_NG;
    }
    else
    {
        /* User function */
    	if(Port2nd)
    	{
            if (0 != l_tInitParam.pfn2ndDevInitFunc)
            {
                l_tInitParam.pfn2ndDevInitFunc();
            }
    	}
    	else
    	{
            if (0 != l_tInitParam.pfn1stDevInitFunc)
            {
                l_tInitParam.pfn1stDevInitFunc();
            }
    	}
    }

    return iRet;
} /* grp_usb_mdchg_BootDevice */


/************************************************************************************************/
/* FUNCTION   : grp_usb_mdchg_BootHost                                                          */
/*                                                                                              */
/* DESCRIPTION: Boot host mode                                                                  */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_MDCHG_OK                Success                                             */
/*              GRP_MDCHG_NG                Error                                               */
/*                                                                                              */
/************************************************************************************************/
INT grp_usb_mdchg_BootHost(VOID)
{
INT     iRet = GRP_MDCHG_OK;

    if (_not_initialized)
    {
        /* ���荞�ݖ��� */
        if (GRP_USBC_OK != grp_usbc_Disable())
        {
            iRet= GRP_MDCHG_NG;
        }

        /* User Function */
        if (0 != l_tInitParam.pfn1stHostInitFunc)
        {
            l_tInitParam.pfn1stHostInitFunc();
        }
    }
    else
    {
        /* USB�z�X�g�ď��������� */
        if (REINIT_STATUS_OK != grp_reinit_HostReInit())
        {
            /* error */
            iRet= GRP_MDCHG_NG;
        }
    }

    return iRet;
} /* grp_usb_mdchg_BootHost */

/************************************************************************************************/
/* FUNCTION   : grp_usb_mdchg_SendMediaState                                                    */
/*                                                                                              */
/* DESCRIPTION: Notify media status                                                             */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : eMedia                      GRP_MEDIA_ATTACHED      ���f�B�A�ڑ��i�z�X�g���j    */
/*                                          GRP_MEDIA_DETACHED      ���f�B�A�ؒf�i�z�X�g���j    */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : GRP_MDCHG_OK                Success                                             */
/*              GRP_MDCHG_NG                Error                                               */
/*                                                                                              */
/************************************************************************************************/
INT grp_usb_mdchg_SendMediaState(GRP_USB_MDCHG_MSG eMedia)
{
UINT32  ulMedia;
INT     iRet = GRP_MDCHG_OK;

    switch (eMedia)
    {
    case    GRP_MEDIA_ATTACHED:
    case    GRP_MEDIA_DETACHED:
        /* Send Queue */
        ulMedia = (UINT32)eMedia;
        if (grp_vos_SendQueue(l_ptMdchgQue, (void *)&ulMedia, GRP_VOS_INFINITE) != GRP_VOS_POS_RESULT)
        {
            iRet= GRP_MDCHG_NG;
        }
        break;

    default:
        iRet= GRP_MDCHG_NG;
        break;
    }

    return iRet;
} /* grp_usb_mdchg_SendMediaState */

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
void grp_usb_mdchg_ConidIsr( void )
{
grp_si                  iMode;
UINT32                ulUsbMode;

    /* Get operation mode */
    iMode = grp_cyclonev_cmod_GetMode();

    if( GRP_CYCLONEV_MODE_DEVICE == iMode )
    {
        /* Callback device mode */
        ulUsbMode = GRP_MODE_DEVICE;
    }
    else if( GRP_CYCLONEV_MODE_HOST == iMode )
    {
        /* Callback host mode */
        ulUsbMode = GRP_MODE_HOST;
    }

    /* Send Queue */
   grp_vos_SendQueue(l_ptMdchgQue, (void *)&ulUsbMode, GRP_VOS_NOWAIT);
}
/* END OF FILE */
