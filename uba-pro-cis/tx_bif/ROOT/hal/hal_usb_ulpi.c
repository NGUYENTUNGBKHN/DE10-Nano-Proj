/******************************************************************************/
/*! @addtogroup Main
    @file       hal_usb.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * hal_usb.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */
// Grape System社USBドライバラッパーファイル。

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "common.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "typedefine.h"
#include "memorymap.h"
#include "grp_cyclonev_macro.h"
#include "grp_cyclonev_reg.h"
#include "grp_cyclonev_bit_val.h"

#define EXT
#include "com_ram.c"
#include "usb_ram.c"
#include "jsl_ram.c"

#include "usb_cdc_buffer.h"


u8 read_ulpi(u8 addr)
{
	grp_u8 					data;
	grp_u32                 ulGPvnCtl;

	ulGPvnCtl =
			  (addr << 16)	// regaddr
			| (0x0 << 22)	// regwr(READ)
			| (0x1 << 25);	// newregreq

    /* Write PHY ULPI register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GPVNDCTL, ulGPvnCtl );
    while(1)
    {
    	ulGPvnCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GPVNDCTL );
    	if((ulGPvnCtl&(0x1 << 27)) == (0x1 << 27))
    	{
    		data = ulGPvnCtl & 0xFF;
    		break;
    	}
    };
	return data;
}
void write_ulpi(u8 addr, u8 data)
{
	grp_u32                 ulGPvnCtl;

	ulGPvnCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GPVNDCTL );

	ulGPvnCtl =
			  (addr << 16)	// regaddr
			| (0x1 << 22)	// regwr(WRITE)
			| (0x1 << 25)	// newregreq
			| (data);		// val

    /* Write PHY ULPI register */
    CYCLONEV_R32_WR( CYCLONEV_A32_OTG_GPVNDCTL, ulGPvnCtl );
    while(1)
    {
    	ulGPvnCtl = CYCLONEV_R32_RD( CYCLONEV_A32_OTG_GPVNDCTL );
    	if((ulGPvnCtl&(0x1 << 27)) == (0x1 << 27))
    	{
    		break;
    	}
    };
}
u8 ulpi_table[3][0x20];
void ulpi_setting(void)
{
    /* Write USB3340 PHY ULPI register */

	// 0x06:Function Control(clear)
	//write_ulpi(0x06,0x3);		// FUNCTION CONTROL: clear XcvrSelect 00b
	// 0x31:HS Compesation(default:00h)
	//write_ulpi(0x31,(0x7<<4)|0x3);	// PHYBoost 25.7%, VariSense 50%(0x3)
}
void ulpi_load(int evt)
{
	grp_u8 					uAddr;
	int index = 0;

	// index = 0
	// call count
	ulpi_table[evt][index] += 1;
	index++;
    /* Read USB3340 PHY ULPI register */
	// index = 1
	// 0x00:vendor id low(default:24h)
	uAddr = 0x00;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 2
	// 0x01:vendor id high(default:04h)
	uAddr = 0x01;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 3
	// 0x02:Product id low(default:09h)
	uAddr = 0x02;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 4
	// 0x03:Product id high(default:00h)
	uAddr = 0x03;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 5
	// 0x04:Function Control(default:41h)
	uAddr = 0x04;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 6
	// 0x07:Interface Control(default:00h)
	uAddr = 0x07;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 7
	// 0x0A:OTG Control(default:06h)
	uAddr = 0x0a;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 8
	// 0x0D:USB Interrupt Enable Rising(default:1Fh)
	uAddr = 0x0d;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 9
	// 0x10:USB Interrupt Enable Falling(default:1Fh)
	uAddr = 0x10;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 10
	// 0x13:USB Interrupt Status(default:00h)
	uAddr = 0x13;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 11
	// 0x14:USB Interrupt Latch(default:00h)
	uAddr = 0x14;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 12
	// 0x15:Debug(default:00h)
	uAddr = 0x15;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 13
	// 0x16:Scratch Register(default:00h)
	uAddr = 0x16;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 14
	// 0x19:Carkit Control(default:00h)
	uAddr = 0x19;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 15
	// 0x1D:Carkit Interrupt Enable(default:00h)
	uAddr = 0x1D;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 16
	// 0x20:Carkit Interrupt Status(default:00h)
	uAddr = 0x20;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 17
	// 0x21:Carkit Interrupt Latch(default:00h)
	uAddr = 0x21;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 18
	// 0x31:HS Compesation(default:00h)
	uAddr = 0x31;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 19
	// 0x32:USB-IF Charger Detection(default:00h)
	uAddr = 0x32;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
	// index = 20
	// 0x33:Headset Audio Mode(default:00h)
	uAddr = 0x33;
	ulpi_table[evt][index++] = read_ulpi(uAddr);
}
