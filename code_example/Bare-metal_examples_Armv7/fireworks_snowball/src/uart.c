/*
** Copyright (c) 2011 Arm Limited (or its affiliates). All rights reserved.
** Use, modification and redistribution of this file is subject to your possession of a
** valid End User License Agreement for the Arm Product of which these examples are part of 
** and your compliance with all applicable terms and conditions of such licence agreement.
*/

/* Simple polled UART driver */

// This code assumes baud rate, etc has already been setup by X-Loader or U-Boot
// Snowball's UART3 is connected to the Serial port

#define UART2_BASE 0x80007000
#define UART2_DR (volatile unsigned char *)(UART2_BASE)
#define UART2_FR (volatile unsigned char *)(UART2_BASE + 0x18)

#define TX_FIFO_E 1<<7
#define RX_FIFO_E 1<<4


void uart_putc_polled(char c)
{
    while ( !(*UART2_FR & TX_FIFO_E));
    *UART2_DR = c;
}

char uart_getchar_polled(void)
{
    while ( !(*UART2_FR & RX_FIFO_E));
    return *UART2_DR;
}
