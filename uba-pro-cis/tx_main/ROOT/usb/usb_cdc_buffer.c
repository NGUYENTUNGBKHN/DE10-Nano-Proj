#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "js_oswapi.h"
#include "js_io.h"
#include "usb_cdc_buffer.h"
#include "memorymap.h"


s32 _usb_cdc_buffer_init(RingBuffer *rbuf, u32 size)
{
	if (rbuf == NULL) {
		return RET_NG;
	}

	rbuf->length = size + 1;
	rbuf->start = 0;
	rbuf->end = 0;
	rbuf->buffer = (u8*)USB_CDC_BUF_BASE;

	rbuf->initialized = USB_CDC_BUFFER_INITIALIZED;

	return RET_OK;
}

u32 _usb_cdc_buffer_read(RingBuffer *rbuf, u8 *buffer, u32 length) {
	u32 bytes_read;
	u32 available_bytes;
	u32 remaining;

	if (rbuf == NULL) {
		return 0;
	}

	if (rbuf->initialized != USB_CDC_BUFFER_INITIALIZED) {
		return 0;
	}

	remaining = length;
	available_bytes = _usb_cdc_buffer_available_bytes(rbuf);
	if (length > available_bytes) {
		remaining = available_bytes;
	}

	if ((rbuf->start + remaining) > rbuf->length) {
		bytes_read = rbuf->length - rbuf->start;
		remaining -= bytes_read;
#ifdef USB_CDC_DEBUG
		xil_printf("  (1-1) remaining=%d \r\n",remaining);
		xil_printf("  (1-1) buffer=%d \r\n",buffer);
		xil_printf("  (1-1) rbuf->start=%d \r\n",rbuf->start);
		xil_printf("  (1-1) rbuf->buffer=%d \r\n",rbuf->buffer);
#endif
		memcpy(buffer,
			   rbuf->buffer + rbuf->start,
			   bytes_read);
		memcpy(buffer + bytes_read,
			   rbuf->buffer,
			   remaining);
		bytes_read += remaining;
	}
	else {
#ifdef USB_CDC_DEBUG
		xil_printf("  (1-2) length=%d \r\n",length);
		xil_printf("  (1-2) buffer=%d \r\n",buffer);
		xil_printf("  (1-2) rbuf->start=%d \r\n",rbuf->start);
		xil_printf("  (1-2) rbuf->buffer=%d \r\n",rbuf->buffer);
#endif
		memcpy(buffer,
			   rbuf->buffer + rbuf->start,
			   length);
		bytes_read = length;
	}

	/* Update the pointers in the buffer */
	if (0) {}
	else if((rbuf->start + bytes_read) > rbuf->length) {
#ifdef USB_CDC_DEBUG
		xil_printf("  (2-2)  \r\n");
#endif
		rbuf->start = (rbuf->start + bytes_read) % rbuf->length;
	}
	else {
#ifdef USB_CDC_DEBUG
		xil_printf("  (2-3)  \r\n");
#endif
		rbuf->start += bytes_read;
	}

	return bytes_read;
}

u32 _usb_cdc_buffer_write(RingBuffer *rbuf, u8 *data, u32 length) {
	u32 bytes_written;
	u32 available_bytes;
	u32 remaining;

	if (rbuf == NULL) {
		return 0;
	}

	if (rbuf->initialized != USB_CDC_BUFFER_INITIALIZED) {
		return 0;
	}

	remaining = length;
	available_bytes = _usb_cdc_buffer_available_space(rbuf);
	if (length > available_bytes) {
		remaining = available_bytes;
	}

	if ((rbuf->end + remaining) > rbuf->length) {
		bytes_written = rbuf->length - rbuf->end;
		remaining -= bytes_written;
		memcpy(rbuf->buffer + rbuf->end,
			   data,
			   bytes_written);
		memcpy(rbuf->buffer,
			   data + bytes_written,
			   remaining);
		bytes_written += remaining;
	}
	else {
		memcpy(rbuf->buffer + rbuf->end,
			   data,
			   length);
		bytes_written = length;
	}

	/* Update the pointers in the buffer */
	if((rbuf->end + bytes_written) > rbuf->length) {
		rbuf->end = (rbuf->end + bytes_written) % rbuf->length;
	}
	else {
		rbuf->end += bytes_written;
	}

	return bytes_written;
}

u32 _usb_cdc_buffer_available_bytes(RingBuffer *rbuf) {
	if (rbuf == NULL) {
		return 0;
	}

	if (rbuf->initialized != USB_CDC_BUFFER_INITIALIZED) {
		return 0;
	}

	if (rbuf->end < rbuf->start) {
		return rbuf->length - rbuf->start + rbuf->end;
	}
	else {
		return rbuf->end - rbuf->start;
	}

}

u32 _usb_cdc_buffer_available_space(RingBuffer *rbuf) {
	if (rbuf == NULL) {
		return 0;
	}

	if (rbuf->initialized != USB_CDC_BUFFER_INITIALIZED) {
		return 0;
	}

	if (rbuf->end < rbuf->start) {
		return rbuf->start - rbuf->end;
	}
	else {
		return rbuf->length - rbuf->end + rbuf->start;
	}

}
/* EOF */
