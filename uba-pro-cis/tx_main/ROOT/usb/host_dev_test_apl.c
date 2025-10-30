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
#include "debug.h"
#include "js_oswapi.h"
#include "hal_gpio.h"

#include "grp_cyclonev_tg.h"
#include "grp_cyclonev_cfg.h"
#include "com_app.h"




#include "grp_usb_mdchg.h"
#include "grp_cyclonev_cmod.h"


/**** STRUCTURE PROTOTYPES **********************************************************************/

/**** INTERNAL VARIABLE DEFINES *****************************************************************/
#define DEVICE_MODE_COM         (0)
#define DEVICE_MODE_COM_CUSTOM  (1)

/**** EXTERNAL VARIABLE DEFINES *****************************************************************/

/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/
void USB0_Mode_Change(void);    /* Host mode / Device mode */
void USB0_USBDEV_Init(void);
void USB2_USBDEV_Init(void);

/**** EXTERNAL FUNCTION PROTOTYPES **************************************************************/
extern void reset_usb0(void);
extern int host_app_Init( void );
VOID _UsbInit( VOID );
VOID _Rear_UsbInit( VOID );

/**** INTERNAL DATA DEFINES *********************************************************************/
/* for mode change test */
DLOCAL GRP_USB_MDCHG_INITPARAM g_tTestAplPrm = {
    USB0_USBDEV_Init,       /* DEVICE */
    USB0_USBDEV_Init,       /* DEVICE */
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
#if defined(_PROTOCOL_ENABLE_ID003)
    /* USB1 USB/DEVICE(COMファンクション）初期化 */
    USB2_USBDEV_Init();
#elif defined(_PROTOCOL_ENABLE_ID0G8)
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
    _UsbInit();
}

/************************************************************************************************/
/* FUNCTION   : USB2_USBDEV_Init                                                                */
/*                                                                                              */
/* DESCRIPTION: USB1(GR-USB/DEVICE + CDC + サンプルアプリケーション)初期化モジュール            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void USB2_USBDEV_Init(void)
{
    INT     iStat;
    //ER      ercd;
#if defined(USB_REAR_USE)
    rear_reset_usb2();
    OSW_TSK_sleep(100);
    _Rear_UsbInit();
#else
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

