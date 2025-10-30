/******************************************************************************/
/*! @addtogroup Main
    @file       hal_bezel.c
    @brief      Bezel LED driver.
    @brief      ベゼルLED発光制御ドライバファイル。
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * hal_bezel.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#include "string.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "js_io.h"
#include "js_oswapi.h"
#include "i2c/js_i2c.h"
#include "common.h"
#include "cyclonev_rstmgr_reg_def.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

void RstmgrInit(void)
{
	RSTMGR_PERMODRST_T permodrst = {0};
	u32 read_value;
	permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	/* emac0 1 */
	permodrst.bit.emac0 = 1;
	/* emac1 1 */
	permodrst.bit.emac1 = 1;
	/* usb0 0 */
	permodrst.bit.usb0 = 0;
	/* usb1 0 */
	permodrst.bit.usb1 = 0;
	/* nand 1 */
	permodrst.bit.nand = 1;
	/* qspi 0 */
	permodrst.bit.qspi = 0;
	/* l4wd0 1 */
	permodrst.bit.l4wd0 = 1;
	/* l4wd1 1 */
	permodrst.bit.l4wd1 = 0;
	/* osc1timer0 0 */
	permodrst.bit.osc1timer0 = 0;
	/* osc1timer1 0 */
	permodrst.bit.osc1timer1 = 0;
	/* sptimer0 0 */
	permodrst.bit.sptimer0 = 0;
	/* sptimer1 0 */
	permodrst.bit.sptimer1 = 0;
	/* i2c0 0 */
	permodrst.bit.i2c0 = 0;
	/* i2c1 1 */
	permodrst.bit.i2c1 = 1;
	/* i2c2 1 */
	permodrst.bit.i2c2 = 1;
	/* i2c3 0 */
	permodrst.bit.i2c3 = 0;
	/* uart0 0 */
	permodrst.bit.uart0 = 0;
	/* uart1 0 */
	permodrst.bit.uart1 = 0;
	/* spim0 0 */
	permodrst.bit.spim0 = 0;
	/* spim1 1 */
	permodrst.bit.spim1 = 1;
	/* spis0 1 */
	permodrst.bit.spis0 = 1;
	/* spis1 1 */
	permodrst.bit.spis1 = 1;
	/* sdmmc 0 */
	permodrst.bit.sdmmc = 0;
	/* can0 1 */
	permodrst.bit.can0 = 1;
	/* can1 1 */
	permodrst.bit.can1 = 1;
	/* gpio0 0 */
	permodrst.bit.gpio0 = 0;
	/* gpio1 0 */
	permodrst.bit.gpio1 = 0;
	/* gpio2 0 */
	permodrst.bit.gpio2 = 0;
	/* dma 0 */
	permodrst.bit.dma = 0;
	/* sdr 0 */
	permodrst.bit.sdr = 0;

	do {
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		read_value = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (read_value != permodrst.lword);		// リセットアサート待ち
}
void RstmgrWdt1Reset(void)
{
	/* WDT1 Peripheral Reset */
	RSTMGR_PERMODRST_T permodrst = {0};
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.l4wd1 = 1;		// WDT1モジュールリセットアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.l4wd1 != 1);		// リセットアサート待ち
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.l4wd1 = 0;		// WDT1モジュールリセットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.l4wd1 != 0);		// リセットデアサート待ち
}
void RstmgrI2C1Reset(void)
{
	/* I2C1 Peripheral Reset */
	RSTMGR_PERMODRST_T permodrst = {0};
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.i2c1 = 1;		// I2C1モジュールリセットアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.i2c1 != 1);		// リセットアサート待ち
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.i2c1 = 0;		// I2C1モジュールリセットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.i2c1 != 0);		// リセットデアサート待ち
}


void RstmgrI2C0Reset(void)
{
	/* I2C0 Peripheral Reset */
	RSTMGR_PERMODRST_T permodrst = {0};
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.i2c0 = 1;		// I2C0モジュールリセットアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.i2c0 != 1);		// リセットアサート待ち
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.i2c0 = 0;		// I2C0モジュールリセットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.i2c0 != 0);		// リセットデアサート待ち
}
void RstmgrI2C3Reset(void)
{
	/* I2C3 Peripheral Reset */
	RSTMGR_PERMODRST_T permodrst = {0};
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.i2c3 = 1;		// I2C3モジュールリセットアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.i2c3 != 1);		// リセットアサート待ち
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.i2c3 = 0;		// I2C3モジュールリセットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.i2c3 != 0);		// リセットデアサート待ち
}

void RstmgrUSB0Reset(void)
{
	/* USB0 Peripheral Reset */
	RSTMGR_PERMODRST_T permodrst = {0};
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.usb0 = 1;		// USB0モジュールリセットアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.usb0 != 1);		// リセットアサート待ち
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.usb0 = 0;		// USB0モジュールリセットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.usb0 != 0);		// リセットデアサート待ち
}

void RstmgrUSB1Reset(void)
{
	/* USB1 Peripheral Reset */
	RSTMGR_PERMODRST_T permodrst = {0};
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.usb1 = 1;		// USB1モジュールリセットアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.usb1 != 1);		// リセットアサート待ち
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.usb1 = 0;		// USB1モジュールリセットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.usb1 != 0);		// リセットデアサート待ち
}
void RstmgrUart1Reset(void)
{
	/* UART1 Peripheral Reset */
	RSTMGR_PERMODRST_T permodrst = {0};
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.uart1 = 1;		// UART1モジュールリセットアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.uart1 != 1);		// リセットアサート待ち
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.uart1 = 0;		// UART1モジュールリセットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.uart1 != 0);		// リセットデアサート待ち
}
void RstmgrQspiReset(void)
{
	/* QSPI Peripheral Reset */
	RSTMGR_PERMODRST_T permodrst = {0};
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.qspi = 1;		// リセットビットセット
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.qspi != 1);		// リセットアサート待ち
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.qspi = 0;		// リセットビットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.qspi != 0);		// リセットデアサート待ち
}


void RstmgrSdmmcReset(void)
{
	/* SDMMC Peripheral Reset */
	RSTMGR_PERMODRST_T permodrst = {0};
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.sdmmc = 1;		// リセットビットセット
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.sdmmc != 1);		// リセットアサート待ち
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.sdmmc = 0;		// リセットビットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.sdmmc != 0);		// リセットデアサート待ち
}

void RstmgrGpio0Reset(void)
{
	/* GPIO0 Peripheral Reset */
	RSTMGR_PERMODRST_T permodrst = {0};
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.gpio0 = 1;		// GPIO0モジュールリセットアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.gpio0 != 1);		// リセットアサート待ち
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.gpio0 = 0;		// GPIO0モジュールリセットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.gpio0 != 0);		// リセットデアサート待ち
}

void RstmgrGpio1Reset(void)
{
	/* GPIO1 Peripheral Reset */
	RSTMGR_PERMODRST_T permodrst = {0};
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.gpio1 = 1;		// GPIO1モジュールリセットアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.gpio1 != 1);		// リセットアサート待ち
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.gpio1 = 0;		// GPIO1モジュールリセットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.gpio1 != 0);		// リセットデアサート待ち
}

void RstmgrGpio2Reset(void)
{
	/* GPIO2 Peripheral Reset */
	RSTMGR_PERMODRST_T permodrst = {0};
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.gpio2 = 1;		// GPIO2モジュールリセットアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.gpio2 != 1);		// リセットアサート待ち
	do {
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		permodrst.bit.gpio2 = 0;		// GPIO2モジュールリセットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (permodrst.bit.gpio2 != 0);		// リセットデアサート待ち
}

void RstmgrS2fReset(void)
{
	/* FPGA Logic Peripheral Reset */
	volatile MISCMODRST_REG_UNION miscmodrst = {0};
	do {
		miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		miscmodrst.bit.s2f = 1;		// リセットビットセット
		IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST) = miscmodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (miscmodrst.bit.s2f != 1);		// リセットアサート待ち
	do {
		miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		miscmodrst.bit.s2f = 0;		// リセットビットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST) = miscmodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (miscmodrst.bit.s2f != 0);		// リセットデアサート待ち
}

void RstmgrS2fColdReset(void)
{
	/* FPGA Logic Peripheral Reset */
	volatile MISCMODRST_REG_UNION miscmodrst = {0};
	do {
		miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		miscmodrst.bit.s2fcold = 1;		// リセットビットセット
		IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST) = miscmodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (miscmodrst.bit.s2fcold != 1);		// リセットアサート待ち
	do {
		miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		miscmodrst.bit.s2fcold = 0;		// リセットビットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST) = miscmodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (miscmodrst.bit.s2fcold != 0);		// リセットデアサート待ち
}
void RstmgrFpgamgrReset(void)
{
	/* FPGA Manager Peripheral Reset */
	volatile MISCMODRST_REG_UNION miscmodrst = {0};
	do {
		miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		miscmodrst.bit.fpgamgr = 1;		// リセットビットセット
		IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST) = miscmodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (miscmodrst.bit.fpgamgr != 1);		// リセットアサート待ち
	do {
		miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		miscmodrst.bit.fpgamgr = 0;		// リセットビットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST) = miscmodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (miscmodrst.bit.fpgamgr != 0);		// リセットデアサート待ち
}
void RstmgrBridgeReset(void)
{
	/* Bridge Peripheral Reset */
	volatile BRGMODRST_REG_UNION bridmodrst = {0};
	do {
		bridmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_BRGMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		bridmodrst.lword |= 0x7;		// リセットビットセット
		IOREG32(RSTMGR_BASE, RSTMGR_BRGMODRST) = bridmodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		bridmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_BRGMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (bridmodrst.lword != 0x7);		// リセットアサート待ち
	do {
		bridmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_BRGMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
		bridmodrst.lword = 0;		// リセットビットデアサート
		IOREG32(RSTMGR_BASE, RSTMGR_BRGMODRST) = bridmodrst.lword;		// 内蔵モジュールリセット制御レジスタ書込み
		__nop();
		bridmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_BRGMODRST);		// 内蔵モジュールリセット制御レジスタ読出し
	} while (bridmodrst.lword != 0);		// リセットデアサート待ち
	//L3 インタコネクトの設定
	uint32_t value;
	value = IOREG32(L3REGS_BASE, L3REGS_REMAP);
	value |= 0x18;
	IOREG32(L3REGS_BASE, L3REGS_REMAP) = value;
	__nop();
}
/* EOF */
