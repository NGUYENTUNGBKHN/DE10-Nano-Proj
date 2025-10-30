/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      com_app_intr_cmd2.c                                                     1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Communication Function Driver Interrupt転送テストコマンド アプリケーション 2            */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   T.Yamaguchi    2021/09/08  V1.00                                                           */
/*                              Created initial version.                                        */
/*                              Communication Function Driver アプリケーションをベースに作成    */
/*                                                                                              */
/************************************************************************************************/


/**** INCLUDE FILES *****************************************************************************/
#include <string.h>

#include "grusbtyp.h"
#include "comm_def.h"
#include "grcomm.h"
#include "com_app_intr_cmd.h"

/**** STRUCTURE PROTOTYPES **********************************************************************/


/**** INTERNAL DATA DEFINES *********************************************************************/
/* NETWORK_CONNECTION */
#define NC_DISCONNECTED         (0)                                             /* Disconnected */
#define NC_CONNECTED            (1)                                             /* Connected    */
/* SERIAL_STATE */
#define SS_DISABLE              (0)                                    /* Tx/Rx Carrier Disable */
#define SS_ENABLE               (1)                                    /* Tx/Rx Carrier Enable  */


/**** INTERNAL VARIABLE DEFINES *****************************************************************/
/* Notification一括処理フラグ */
DLOCAL BOOLEAN                  l_bAllNotification2;
/* Notification番号 */
DLOCAL UINT8                    l_ucNotificationNo2;


/**** EXTERNAL VARIABLE DEFINES *****************************************************************/


/**** INTERNAL FUNCTION PROTOTYPES **************************************************************/



/************************************************************************************************/
/* FUNCTION   : com_app_NotificationInit2                                                       */
/*                                                                                              */
/* DESCRIPTION: Notification情報の初期化処理                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID com_app_NotificationInit2( VOID )
{
    /* Notification一括処理フラグの初期化 */
    l_bAllNotification2 = GRUSB_FALSE;
    /* Notification番号の初期化 */
    l_ucNotificationNo2 = 0;
}

/************************************************************************************************/
/* FUNCTION   : com_app_UpdateNotificationNo2                                                   */
/*                                                                                              */
/* DESCRIPTION: Notification番号の更新処理                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID com_app_UpdateNotificationNo2( VOID )
{
    if( GRUSB_TRUE == l_bAllNotification2 )
    {
        /* Notification番号を更新 */
        l_ucNotificationNo2++;
    }
    else
    {
        /* Notification番号をクリア */
        l_ucNotificationNo2 = 0;
    }
}

/************************************************************************************************/
/* FUNCTION   : com_app_GetNotificationNo2                                                      */
/*                                                                                              */
/* DESCRIPTION: Notification番号の取得処理                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : Notification番号                                                                */
/*                                                                                              */
/************************************************************************************************/
UINT8 com_app_GetNotificationNo2( VOID )
{
    return l_ucNotificationNo2;
}

/************************************************************************************************/
/* FUNCTION   : _com_app_CheckNotificationCmd2                                                  */
/*                                                                                              */
/* DESCRIPTION: Notification開始コマンドチェック                                                */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
INT com_app_CheckNotificationCmd2( UINT8* pucBuf )
{
INT                     iStat;
UINT8                   ucNo;

    /* Notification開始コマンドチェック */
    iStat = strncmp( (const char*)pucBuf, START_CMD, START_CMD_LEN );

    if( 0 == iStat )
    {
        /* All notification */
        pucBuf += START_CMD_LEN;
        iStat = strncmp( (const char*)pucBuf, ALL_NT_CMD, ALL_NT_CMD_LEN );

        if( 0 == iStat )
        {
            l_bAllNotification2 = GRUSB_TRUE;
            l_ucNotificationNo2 = 1;
            /* Responseを付与 */
            pucBuf += ALL_NT_CMD_LEN;
            strncpy( (char*)pucBuf, CMD_RESP, CMD_RESP_LEN );
        }
        else
        {
            /* NETWORK_CONNECTION */
            iStat = strncmp( (const char*)pucBuf, NC_CMD, NC_CMD_LEN );

            if( 0 == iStat )
            {
                pucBuf += NC_CMD_LEN;
                ucNo   = *pucBuf;
                if( '0' == ucNo )
                {
                    /* NOTWORK_CONNECTION(Disconnected) */
                    l_ucNotificationNo2 = 1;
                }
                else if( '1' == ucNo )
                {
                    /* NETWORK_CONNECTION(Connected) */
                    l_ucNotificationNo2 = 2;
                }
                else
                {
                    return GRUSB_FALSE;
                }
                /* Responseを付与 */
                pucBuf += 1;
                strncpy( (char*)pucBuf, CMD_RESP, CMD_RESP_LEN );
            }
            else
            {
                /* RESPONSE_AVAILABLE */
                iStat = strncmp( (const char*)pucBuf, RA_CMD, RA_CMD_LEN );

                if( 0 == iStat )
                {
                    /* RESPONSE_AVAILABLE */
                    l_ucNotificationNo2 = 3;
                    /* Responseを付与 */
                    pucBuf += RA_CMD_LEN;
                    strncpy( (char*)pucBuf, CMD_RESP, CMD_RESP_LEN );
                }
                else
                {
                    /* SERIAL_STATE */
                    iStat = strncmp( (const char*)pucBuf, SS_CMD, SS_CMD_LEN );

                    if( 0 == iStat )
                    {
                        pucBuf += SS_CMD_LEN;
                        ucNo   = *pucBuf;
                        if( '1' == ucNo )
                        {
                            /* SERIAL_STATE(TxCarrier:Disable/RxCarrier:Enable) */
                            l_ucNotificationNo2 = 4;
                        }
                        else if( '2' == ucNo )
                        {
                            /* SERIAL_STATE(TxCarrier:Enable/RxCarrier:Disable) */
                            l_ucNotificationNo2 = 5;
                        }
                        else if( '3' == ucNo )
                        {
                            /* SERIAL_STATE(TxCarrier:Enable/RxCarrier:Enable) */
                            l_ucNotificationNo2 = 6;
                        }
                        else
                        {
                            return GRUSB_FALSE;
                        }
                        /* Responseを付与 */
                        pucBuf += 1;
                        strncpy( (char*)pucBuf, CMD_RESP, CMD_RESP_LEN );
                    }
                    else
                    {
                        return GRUSB_FALSE;
                    }
                }
            }
        }
    }
    else
    {
        /* Notification開始コマンドではない */
        return GRUSB_FALSE;
    }

    return GRUSB_TRUE;
}

/************************************************************************************************/
/* FUNCTION   : com_app_SendNotification2                                                       */
/*                                                                                              */
/* DESCRIPTION: COM Notification送信                                                            */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : ucNo                            通知番号                                        */
/* OUTPUT     : none                                                                            */
/*                                                                                              */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
VOID com_app_SendNotification2( UINT8 ucNo )
{
    switch( ucNo )
    {
    case 1:
        /* NOTWORK_CONNECTION(Disconnected)送信 */
        GRUSB_COMD_Notification_NetworkConnection2( NC_DISCONNECTED );
        break;
    case 2:
        /* NETWORK_CONNECTION(Connected)送信 */
        GRUSB_COMD_Notification_NetworkConnection2( NC_CONNECTED );
        break;
    case 3:
        /* RESPONSE_AVAILABLE送信 */
        GRUSB_COMD_Notification_ResponseAvailable2();
        break;
    case 4:
        /* SERIAL_STATE(TxCarrier:Disable/RxCarrier:Enable)送信 */
        GRUSB_COMD_Notification_SerialState2( ( SS_DISABLE << 1 ) | SS_ENABLE );
        break;
    case 5:
        /* SERIAL_STATE(TxCarrier:Enable/RxCarrier:Disable)送信 */
        GRUSB_COMD_Notification_SerialState2( ( SS_ENABLE << 1 ) | SS_DISABLE );
        break;
    case 6:
        /* SERIAL_STATE(TxCarrier:Enable/RxCarrier:Enable)送信 */
        GRUSB_COMD_Notification_SerialState2( ( SS_ENABLE << 1 ) | SS_ENABLE );
        break;
    default:
        if( GRUSB_TRUE == l_bAllNotification2 )
        {
            /* Notification情報の初期化 */
            com_app_NotificationInit2();
        }

        break;
    }
}
