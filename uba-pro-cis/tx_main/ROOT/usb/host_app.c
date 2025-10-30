/************************************************************************************************/
/*                                                                                              */
/*                            Copyright(C) 2014-2019 Grape Systems, Inc.                        */
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
/*      host_app.c                                                            1.01              */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB HOST/DEVICE Common Modules sample application                                    */
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
#include "host_app.h"
#include "host_apl.h"
#include "grp_cyclonev_tg.h"
#include "grp_cyclonev_cmod.h"
#include "grp_cyclonev_types.h"


/**** INTERNAL DATA DEFINES *********************************************************************/


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
LOCAL grp_si _host_app_Otg_Init( grp_si );


/************************************************************************************************/
/* FUNCTION   : host_app_Init                                                                 */
/*                                                                                              */
/* DESCRIPTION: Initialize of sample application.                                               */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : HOST_APP_OK                   Success                                         */
/*              HOST_APP_HW_INIT_NG           H/W Init Error                                  */
/*              HOST_APP_GRUSB_INIT_NG        GR-USB Init Error                               */
/*              HOST_APP_START_INTR_NG        Start Interrupt Error                           */
/*                                                                                              */
/************************************************************************************************/
int host_app_Init( void )
{
grp_si              iStat;

    /*--- Initialize several modules ---*/
    /* OTG core init */
    iStat = _host_app_Otg_Init( GRP_CYCLONEV_MODE_HOST );

    if( HOST_APP_OK != iStat )
    {
        /* Error */
        return iStat;
    }

    /* Success */
    return HOST_APP_OK;
}

/************************************************************************************************/
/* FUNCTION   : _host_app_Otg_Init                                                            */
/*                                                                                              */
/* DESCRIPTION: Initialize of OTG.                                                              */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : iMode                           Operation mode                                  */
/*                                                  GRP_CYCLONEV_MODE_DEVICE     Device mode    */
/*                                                  GRP_CYCLONEV_MODE_HOST       Host mode      */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : HOST_APP_OK                   Success or Core unused                          */
/*              HOST_APP_HW_INIT_NG           H/W Init Error                                  */
/*              HOST_APP_GRUSB_INIT_NG        GR-USB Init Error                               */
/*              HOST_APP_START_INTR_NG        Start Interrupt Error                           */
/*                                                                                              */
/************************************************************************************************/
LOCAL grp_si _host_app_Otg_Init( grp_si iMode )
{
grp_s32             lStat;

    /* H/W init */
    lStat = grp_target_HwInit( iMode );

    if( GRP_TARGET_OK != lStat )
    {
        /* H/W Init Error */
        return HOST_APP_HW_INIT_NG;
    }

    /* GR-USB init */
    if( GRP_CYCLONEV_MODE_DEVICE == iMode )
    {
        /* GR-USB/DEVICE init */
//        ※ここにGR-USB/DEVICEの初期化処理を追加して下さい
//        ※初期化処理については、ファンクションドライバ付属のサンプルプログラム、
//        ※または『ペリフェラルコントローラドライバ ポーティングマニュアル』の
//        ※「4.2 ユーザアプリケーションにて行って頂く処理について」を参照下さい。
//
//        ※初期化処理でエラーとなった場合は、HOST_APP_GRUSB_INIT_NGを返して下さい。
    }
    else if( GRP_CYCLONEV_MODE_HOST == iMode )
    {
        /* GR-USB/HOST# init */
        GRUSB_Test_Stack_Init();
    }

    /* Start interrupt */
    lStat = grp_target_StartIntr( iMode );

    if ( GRP_TARGET_OK != lStat )
    {
        /* Start Interrupt Error */
        return HOST_APP_START_INTR_NG;
    }

    return HOST_APP_OK;
}
