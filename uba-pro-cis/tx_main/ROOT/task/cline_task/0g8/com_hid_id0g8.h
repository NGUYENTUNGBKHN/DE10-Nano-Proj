/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      com_hid_id0g8.h                                                         1.00            */
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
#ifndef _COM_HID_ID0G8_H_
#define _COM_HID_ID0G8_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grusbtyp.h"


/**** INTERNAL DATA DEFINES *********************************************************************/
/* エラーコード */
#define COM_HID_OK              (1)                                                  /* Success */
#define COM_HID_ERROR           (0)                                                  /* Error   */

#define COM_HID_BUF_SIZE        (512)                                                /* バッファサイズ */


/**** EXTERNAL FUNCTION PROTOTYPE ***************************************************************/
//INT com_hid_0g8_Init2( VOID );
INT com_hid_0g8_Init2( u8 flag );
VOID com_hid_0g8_detach( VOID );
UINT32 com_hid_0g8_listen( UINT8 *pucBuf );
UINT32 com_hid_0g8_send( UINT32 ulSize, UINT8* pucBuf );

#endif /* _COM_DFU_ID0G8_H_ */
