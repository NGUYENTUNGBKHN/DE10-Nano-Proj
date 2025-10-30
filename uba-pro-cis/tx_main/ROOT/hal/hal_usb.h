/*
 * hal_usb.h
 *
 *  Created on: 2019/08/09
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_HAL_HAL_USB_H_
#define SRC_HAL_HAL_USB_H_

void reset_usb0(void);
s32 UsbGetAvailableChar( VOID );
s32 UsbGetRecvData( u8* dst, s32 size_to_read );
void UsbReqSendData(void);
void UsbReqSendSamplingData(void);

#if defined(USB_REAR_USE)
void rear_reset_usb2(void);
s32 Rear_UsbGetAvailableChar( VOID );
s32 Rear_UsbGetRecvData( u8* dst, s32 size_to_read );
void Rear_UsbReqSendData(void);
void Rear_UsbReqSendSamplingData(void);
s32 rear_is_usb2_connected(void);
void _Rear_Usb_Stop();
void _Rear_Usb_ReStart();
#endif 

#endif /* SRC_HAL_HAL_USB_H_ */
