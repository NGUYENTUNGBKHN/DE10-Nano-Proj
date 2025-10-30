/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      host_dev_test_apl.c                                                     1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      GR-USB/HOST - GR-USB/DEVICE 切り替え対応サンプルテストアプリケーション	                            */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*      NAME        DATE        REMARKS                                                         */
/*                                                                                              */
/*      T.Yamaguchi 2021/10/05  V1.00                                                           */
/*                              Created initial version                                         */
/*                                                                                              */
/************************************************************************************************/

/**** COMPILE OPTIONS ***************************************************************************/

/**** INCLUDE FILES *****************************************************************************/
#include "grp_vos.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "debug.h"
#include "js_oswapi.h"

#include "grp_cyclonev_tg.h"
#include "grp_cyclonev_cfg.h"
#include "com_app.h"
#if (_DEBUG_USB_CUSTOM==1)
#include "com_custom_app.h"
#endif

#include "grp_usb_mdchg.h"
#include "grp_cyclonev_cmod.h"


/**** STRUCTURE PROTOTYPES **********************************************************************/

/**** INTERNAL VARIABLE DEFINES *****************************************************************/
/* Device or Host */
#if 0
#define GRP_MODE_HOST           (0)
#define GRP_MODE_DEVICE         (1)
#endif

#define DEVICE_MODE_COM         (0)
#define DEVICE_MODE_COM_CUSTOM  (1)

/**** EXTERNAL VARIABLE DEFINES *****************************************************************/

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
void USB0_Mode_Change(void);    /* Host mode / Device mode */
void USB0_USBDEV_Init(void);
void USB1_USBDEV_Init(void);

/**** EXTERNAL FUNCTION PROTOTYPES **************************************************************/
extern void reset_usb0(void);
extern void USBConnect(void);
extern int host_app_Init( void );
extern void _otg_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void _otg_system_error(u8 fatal_err, u8 code);

/**** INTERNAL DATA DEFINES *********************************************************************/
/* for mode change test */
DLOCAL GRP_USB_MDCHG_INITPARAM g_tTestAplPrm = {
    USB0_USBDEV_Init,       /* DEVICE */
    USB0_USBDEV_Init,       /* DEVICE(dummy) */
    host_app_Init         /* HOST   */
};

DLOCAL INT l_iUsbMode;

/************************************************************************************************/
/* FUNCTION   : USB0_Mode_Change                                                                */
/*                                                                                              */
/* DESCRIPTION: モード切り替えモジュール                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void usb_host_dev_test_init(void)
{
#if 0  /* ID-0G8 */
    /* USB1 USB/DEVICE(COMファンクション）初期化 */
    USB1_USBDEV_Init();
#endif /* ID-0G8 */
#if defined(USB_REAR_USE)
    USB1_USBDEV_Init();
#endif 
    /* USB0 モード切替テスト */
    USB0_Mode_Change();
}

/************************************************************************************************/
/* FUNCTION   : USB0_Mode_Change                                                                */
/*                                                                                              */
/* DESCRIPTION: モード切り替えモジュール                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
extern grp_vos_t_queue*         l_ptAttachQue;    /* Notice Attach Queue   */
void USB0_Mode_Change(void)
{
    UINT32      ulMsg;
    UINT32      ulPort;
    UINT32      ulMode;
    UINT32      ulMedia;
    UINT32      ulGIntMsk;
    UINT32      ulGInt;
    INT32       lStat = 0;
    INT32       lSendMsg;

    /* モード切り替え処理初期化 */
    reset_usb0();

    grp_usb_mdchg_Init();

    /* 初期化関数登録 */
    grp_usb_mdchg_SetFunc(&g_tTestAplPrm);

    /* デバイスモードで起動 */
    USB0_USBDEV_Init();
    /* 初期モード設定 */
    ulPort      = is_fusb_dect_on();
    l_iUsbMode  = GRP_MODE_DEVICE;
    ulMode      = GRP_MODE_DEVICE;
    ulMedia     = GRP_MEDIA_DETACHED;
    ulGInt = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GINTSTS );    /* Read interrupt mask 2021.11.22 */
    if (CYCLONEVG_B01_CURMOD & ulGInt)
    {
        /* USBホスト起動 */
        grp_usb_mdchg_BootHost();
        l_iUsbMode  = GRP_MODE_HOST;
        ulMode      = GRP_MODE_HOST;
    }

    ulGIntMsk = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GINTMSK );    /* Read interrupt mask */
    ulGIntMsk |= CYCLONEVG_B01_CONIDSTSCHNGMSK;
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GINTMSK, ulGIntMsk );

    /* 切り替え通知待ち */
    while(1)
    {
        if (GRP_MDCHG_OK == grp_usb_mdchg_WaitMdChg( &ulMsg ))
        {
            switch ((GRP_USB_MDCHG_MSG)ulMsg)
            {
            case    GRP_MODE_HOST:                              /* ホスト起動                   */
                /* モード変更確認 */
                if (GRP_MODE_HOST != l_iUsbMode)
                {
                    /* USBホスト起動 */
                    grp_usb_mdchg_BootHost();
                    /* モード変更 */
                    l_iUsbMode  = GRP_MODE_HOST;
                }
                ulMode  = GRP_MODE_HOST;
                break;

            case    GRP_MODE_DEVICE:                            /* デバイス起動                 */
                /* モード変更 ＆ メディア切断確認 */
                if (((GRP_MODE_DEVICE != l_iUsbMode) && (GRP_MEDIA_DETACHED == ulMedia))
                 ||	((GRP_MODE_DEVICE == l_iUsbMode) && (ulPort != is_fusb_dect_on()))
                ){
                    /* メディア切断まで終了してからデバイスに切り替える */

                    /* USBデバイス起動 */
                    grp_usb_mdchg_BootDevice(ulPort);
                    /* モード変更 */
                    l_iUsbMode  = GRP_MODE_DEVICE;
                    ulPort      = is_fusb_dect_on();
                }
                ulMode  = GRP_MODE_DEVICE;
                break;

            case    GRP_MEDIA_ATTACHED:                         /* メディア接続（ホスト時）     */
                ulMedia = GRP_MEDIA_ATTACHED;
                lStat = (grp_s32)grp_vos_SendQueue(l_ptAttachQue, (void *)&lSendMsg, GRP_VOS_NOWAIT);
                _otg_send_msg(ID_DLINE_MBX, TMSG_OTG_NOTICE, TMSG_SUB_CONNECT, GRUSB_TRUE, 0, 0);
                break;

            case    GRP_MEDIA_DETACHED:                         /* メディア切断（ホスト時）     */
                /* モード変更 ＆ デバイス起動確認 */
                if (((GRP_MODE_DEVICE != l_iUsbMode) && (GRP_MEDIA_DETACHED == ulMedia))
                 ||	((GRP_MODE_DEVICE == l_iUsbMode) && (ulPort != is_fusb_dect_on()))
                ){
                    /* メディア切断まで終了してからデバイスに切り替える */

                    /* USBデバイス起動 */
                    grp_usb_mdchg_BootDevice(ulPort);
                    /* モード変更 */
                    l_iUsbMode  = GRP_MODE_DEVICE;
                    ulPort      = is_fusb_dect_on();
                }
                ulMedia = GRP_MEDIA_DETACHED;
                _otg_send_msg(ID_DLINE_MBX, TMSG_OTG_NOTICE, TMSG_SUB_CONNECT, GRUSB_FALSE, 0, 0);
                break;

            default :
                break;
            }
        }
        else
        {
            OSW_TSK_sleep(100 * 1000);
        }
    }
}

/************************************************************************************************/
/* FUNCTION   : USB0_USBDEV_Init                                                                */
/*                                                                                              */
/* DESCRIPTION: USB0(GR-USB/DEVICE + CDC or CDCカスタム）初期化モジュール                       */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void USB0_USBDEV_Init(void)
{
#if 1
    _UsbInit();
#else
#if (_DEBUG_USB_CUSTOM==1)
    INT             iStat;
    //ER              ercd;
    static UINT     uiCommMode = DEVICE_MODE_COM;
    //static UINT     uiCommMode = DEVICE_MODE_COM_CUSTOM;

    /* H/W init */
    if ( GRP_TARGET_OK != grp_target_HwInit( GRP_CYCLONEV_MODE_DEVICE ) )
    {
        /* H/W Init Error */
        return;
    }

    if (uiCommMode == DEVICE_MODE_COM)
    {
        /* COM test */
        iStat = com_app_Init();
        if (iStat != COM_APP_OK)
        {
            /* error */
            return;
        }

        uiCommMode = DEVICE_MODE_COM_CUSTOM;
    }
    else
    {
        /* COM Custom test */
        iStat = com_custom_app_Init();
        if (iStat != COM_CUSTOM_APP_OK)
        {
            /* error */
            return;
        }

        uiCommMode = DEVICE_MODE_COM;
    }
#else
    INT             iStat;

    /* H/W init */
    if ( GRP_TARGET_OK != grp_target_HwInit( GRP_CYCLONEV_MODE_DEVICE ) )
    {
        /* H/W Init Error */
        return;
    }

    /* COM test */
    iStat = com_app_Init();
    if (iStat != COM_APP_OK)
    {
        /* error */
        return;
    }
#endif

    /* Enable of D+ pull up is done in com_app_Init/com_custom_app_Init. */

    /* Start interrupt */
    if ( GRP_TARGET_OK != grp_target_StartIntr( GRP_CYCLONEV_MODE_DEVICE ) )
    {
        /* Start Interrupt Error */
        return ;
    }
#endif
}

/************************************************************************************************/
/* FUNCTION   : USB1_USBDEV_Init                                                                */
/*                                                                                              */
/* DESCRIPTION: USB1(GR-USB/DEVICE + CDC + サンプルアプリケーション)初期化モジュール            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void USB1_USBDEV_Init(void)
{
#if defined(USB_REAR_USE)
    rear_reset_usb1();
    _Rear_UsbInit();
#else
    INT     iStat;
    //ER      ercd;

    /* H/W init */
    if ( GRP_TARGET_OK != grp_target_HwInit2( GRP_CYCLONEV_MODE_DEVICE ) )
    {
        /* H/W Init Error */
        return;
    }

    /* COM test */
    iStat = com_app_Init2();

    if (iStat != COM_APP_OK)
    {
        /* error */
        return;
    }

    /* Enable of D+ pull up is done in com_app_Init2. */

    /* Start interrupt */
    if ( GRP_TARGET_OK != grp_target_StartIntr2( GRP_CYCLONEV_MODE_DEVICE ) )
    {
        /* Start Interrupt Error */
        return;
    }
#endif 
}

