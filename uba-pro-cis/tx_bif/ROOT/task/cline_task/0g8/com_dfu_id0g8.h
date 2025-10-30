/************************************************************************************************/
/*                                                                                              */
/* FILE NAME                                                                    VERSION         */
/*                                                                                              */
/*      com_dfu_id0g8.h                                                         1.00            */
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
#ifndef _COM_DFU_ID0G8_H_
#define _COM_DFU_ID0G8_H_

/**** INCLUDE FILES *****************************************************************************/
#include "grusbtyp.h"


/**** INTERNAL DATA DEFINES *********************************************************************/
/* エラーコード */
#define COM_DFU_OK              (1)                                                  /* Success */
#define COM_DFU_ERROR           (0)                                                  /* Error   */

#define COM_DFU_BUF_SIZE        (4096)                                               /* バッファサイズ */


/**** EXTERNAL FUNCTION PROTOTYPE ***************************************************************/
INT com_dfu_0g8_Init2( UINT32 ulRcvSize, UINT8 *pucRcvBuf, UINT32 ulSndSize, UINT8 *pucSndBuf );
#if 1//#if defined(ID0G8_BOOTIF)
UINT32 com_dfu_0g8_listen( void );
#endif
//UINT32 com_dfu_0g8_send( UINT32 ulSize, UINT8* pucBuf );

#endif /* _COM_DFU_ID0G8_H_ */
