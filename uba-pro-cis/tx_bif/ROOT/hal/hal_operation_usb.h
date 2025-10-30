/*
 * hal_usb.h
 *
 *  Created on: 2019/08/09
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_HAL_HAL_OPERATION_USB_H_
#define SRC_HAL_HAL_OPERATION_USB_H_

#include "js_oswapi_cfg.h"
#include "js_oswapi.h"
#include "js_io.h"

/* 関数プロトタイプ */
extern void reset_usb1(void);
extern VOID _OperationUsbInit( VOID );
extern VOID _OperationUsbConnStat( INT iConnStat );
extern VOID _OperationUsbSendEncapsulatedCommad( UINT32 ulSize, UINT8* pucData );
extern VOID _OperationUsbGetEncapsulatedResponse( UINT16 usLength );
extern VOID _OperationUsbSetLineCoding( UINT32 ulSize, UINT8* pucData );
extern VOID _OperationUsbGetLineCoding( UINT16 usLength );
extern VOID _OperationUsbSetControlLineState( UINT16 usValue );
extern VOID _OperationUsbResponseAvailable( VOID );
extern VOID _OperationUsbSendData( UINT32 ulSize, UINT8* pucData, VOID* pInfo );
extern VOID _OperationUsbRecvData( UINT32 ulSize, UINT8* pucData, VOID* pInfo );
extern VOID _OperationUsbReqRecvData( VOID );
extern s32 OperationUsbGetAvailableChar( VOID );
extern s32 OperationUsbGetRecvData( u8* dst, s32 size_to_read );
extern void OperationUsbReqSendData(void);
extern void OperationUSBConnect(void);
extern void RstmgrUSB1Reset(void);
extern void FrontUSBConnect(void);
extern void RearUSBConnect(void);
extern void OperationUSBDisconnect(void);
extern s32 is_usb1_connected(void);
/* EOF */


#endif /* SRC_HAL_HAL_OPERATION_USB_H_ */
