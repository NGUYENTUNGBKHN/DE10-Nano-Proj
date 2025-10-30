/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      com_app.h                                                               1.00            */
/*                                                                                              */
/* DESCRIPTION:                                                                                 */
/*                                                                                              */
/*      Communication Function Driver アプリケーション ヘッダファイル                           */
/*                                                                                              */
/* HISTORY                                                                                      */
/*                                                                                              */
/*   NAME           DATE        REMARKS                                                         */
/*                                                                                              */
/*   M.Suzuki       2013/04/02  V1.00                                                           */
/*                              Created initial version                                         */
/*                                                                                              */
/************************************************************************************************/
#ifndef _COM_APP_H_
#define _COM_APP_H_

/**** INCLUDE FILES *****************************************************************************/


/**** INTERNAL DATA DEFINES *********************************************************************/
/* エラーコード */
#define COM_APP_OK              (1)                                                  /* Success */
#define COM_APP_ERROR           (0)                                                  /* Error   */


/**** EXTERNAL FUNCTION PROTOTYPE ***************************************************************/
EXTERN INT com_app_Init( VOID );
EXTERN INT com_app_Init2( VOID );
EXTERN INT com_hid_app_Init2( VOID );

#endif /* _COM_APP_H_ */
