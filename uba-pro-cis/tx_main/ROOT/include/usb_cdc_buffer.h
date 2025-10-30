/*
 * usb_cdc_buffer.h
 *
 *  Created on: 2018/11/27
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_MAIN_USB_USB_CDC_BUFFER_H_
#define SRC_MAIN_USB_USB_CDC_BUFFER_H_


#ifndef USB_CDC_MEM_SIZE
#define USB_CDC_MEM_SIZE	(64 * 1024)
#endif

typedef struct {
	u8 *buffer;
	u32 length;
	u32 start;
	u32 end;
	u32 initialized;
} RingBuffer;

#define USB_CDC_BUFFER_INITIALIZED	0xaa995566

/* Function Prototypes */
s32 _usb_cdc_buffer_init(RingBuffer *rbuf, u32 size);
u32 _usb_cdc_buffer_read(RingBuffer *rbuf, u8 *buffer, u32 length);
u32 _usb_cdc_buffer_write(RingBuffer *rbuf, u8 *data, u32 length);
u32 _usb_cdc_buffer_available_bytes(RingBuffer *rbuf);
u32 _usb_cdc_buffer_available_space(RingBuffer *rbuf);

#endif /* SRC_MAIN_USB_USB_CDC_BUFFER_H_ */
