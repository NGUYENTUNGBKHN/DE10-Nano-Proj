/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      com_app_intr_cmd.h                                                      1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Communication Function Driver アプリケーション Interrupt転送テストコマンド 定義ファイル */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   M.Suzuki       2020/04/10  V1.00                                                           */
/*                              Created initial version                                         */
/*                                                                                              */
/************************************************************************************************/
#ifndef _COM_APP_INTR_CMD_H_
#define _COM_APP_INTR_CMD_H_

/**** INCLUDE FILES *****************************************************************************/


/**** INTERNAL DATA DEFINES *********************************************************************/
/* Notification開始コマンド */
#define START_CMD           "*NT_START?"                                  /* Start command      */
#define ALL_NT_CMD          "#ALL_NOTIFICATION#"                          /* All Notification   */
#define NC_CMD              "#NETWORK_CONNECTION#"                        /* NETWORK_CONNECTION */
#define RA_CMD              "#RESPONSE_AVAILABLE#"                        /* RESPONSE_AVAILABLE */
#define SS_CMD              "#SERIAL_STATE#"                              /* SERIAL_STATE       */

/* Notification開始コマンドサイズ */
#define START_CMD_LEN       (sizeof(START_CMD) - 1)                     /* Start command長      */
#define ALL_NT_CMD_LEN      (sizeof(ALL_NT_CMD) - 1)                    /* All Notification長   */
#define NC_CMD_LEN          (sizeof(NC_CMD) - 1)                        /* NETWORK_CONNECTION長 */
#define RA_CMD_LEN          (sizeof(RA_CMD) - 1)                        /* RESPONSE_AVAILABLE長 */
#define SS_CMD_LEN          (sizeof(SS_CMD) - 1)                        /* SERIAL_STATE長       */

/* コマンドレスポンス定義 */
#define CMD_RESP            " OK\r\n"                                   /* コマンドレスポンス   */
#define CMD_RESP_LEN        (sizeof(CMD_RESP) - 1)                      /* コマンドレスポンス長 */


/**** EXTERNAL FUNCTION PROTOTYPE ***************************************************************/
EXTERN VOID  com_app_NotificationInit( VOID );
EXTERN VOID  com_app_UpdateNotificationNo( VOID );
EXTERN UINT8 com_app_GetNotificationNo( VOID );
EXTERN INT   com_app_CheckNotificationCmd( UINT8* );
EXTERN VOID  com_app_SendNotification( UINT8 );
EXTERN VOID  com_app_NotificationInit2( VOID );
EXTERN VOID  com_app_UpdateNotificationNo2( VOID );
EXTERN UINT8 com_app_GetNotificationNo2( VOID );
EXTERN INT   com_app_CheckNotificationCmd2( UINT8* );
EXTERN VOID  com_app_SendNotification2( UINT8 );

#endif /* _COM_APP_INTR_CMD_H_ */
