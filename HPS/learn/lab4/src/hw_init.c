#include <stdio.h>
#include <stdlib.h>
#define PRINTF_UART
#include "alt_interrupt.h"
#include "soc_cv_av/alt_clock_manager.h"
#include "js_oswapi.h"
#include "js_io.h"
#include "js_intc_reg.h"
#include "js_rstmgr_reg.h"
#include "js_i2c.h"
#include "cyclonev_sysmgr_reg_def.h"
#include "hal_clk.h"
#include "cpu_api.h"

/************************************************************************************************/
/* FUNCTION   : intc_init                                                                         */
/*                                                                                              */
/* DESCRIPTION: 初期化                                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* ObUTPUT     : none                                                                            */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void intc_init(void)
{
	int n;
	//
	//for(int i = ALT_INT_INTERRUPT_SGI0;i <= ALT_INT_INTERRUPT_WDOG1_IRQ;i++)
	//{
	//	alt_int_dist_pending_clear(ALT_INT_INTERRUPT_I2C3_IRQ);
	//}
	//
	if( get_core_id() == 0 ){
		/* INTC Reset */
		for( n = 0 ; n < 8 ; n ++ ){
			IOREG32(INTC_BASE,INTC_ICDICER(n)) = 0xFFFFFFFF;
			IOREG32(INTC_BASE,INTC_ICDICPR(n)) = 0xFFFFFFFF;
		}
		for( n = 0 ; n < 256 ; n ++ ){
			IOREG8(INTC_BASE,INTC_ICDIPR(n)) = 0xF0;
		}
		for( n = OSW_PRIVINT_NUM ; n < 256 ; n ++ ){
			IOREG8(INTC_BASE,INTC_ICDIPTR(n)) = 0;
		}
		for( n = 2 ; n < 16 ; n ++ ){
			IOREG32(INTC_BASE,INTC_ICDICFR(n)) = 0x55555555;
		}
		IOREG32(INTC_BASE,INTC_ICDDCR) = 0x1;
	}

	IOREG32(INTC_BASE,INTC_ICCPMR) = 0xF0;	/* 16 supported levels */
	IOREG32(INTC_BASE,INTC_ICCBPR) = 2;

	while( 1 ){
		n = IOREG32(INTC_BASE,INTC_ICCIAR) = 0x3FF;
		if( n == 0x3FF ) break;
		IOREG32(INTC_BASE,INTC_ICCEOIR) = n;
	};
	IOREG32(INTC_BASE,INTC_ICCICR) = 0x1;
}
/************************************************************************************************/
/* FUNCTION   : system_clk_init                                                                         */
/*                                                                                              */
/* DESCRIPTION: PLLクロック初期化                                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* ObUTPUT     : none                                                                            */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void system_clk_init(void)
{
	disable_unused_peripheral();
	alt_clk_clock_disable(ALT_CLK_QSPI);

	// 1) MAIN PLL  1600mhz to 800mhz
	set_main_pll(800*1000*1000);
	set_peri_pll(400*1000*1000);
	get_pll_info();

	// グローバル変数clock_frequencyにクロックを格納
	get_pll_info();
}
/****************************************************************/
/**
 * @brief SYSMGRレジスタセットアップ
 */
/****************************************************************/
void setup_sysmgr(void)
{
	// GPIO60:SPIM0_SS0
	IOREG32(SYSMGR_BASE,SYS_GENERALIO12) = (UINT32)0x0;
}

void RstmgrInit(void)
{
	RSTMGR_PERMODRST_T permodrst = {0};
	uint32_t read_value;
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
	permodrst.bit.qspi = 1;
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
/************************************************************************************************/
/* FUNCTION   : hw_init                                                                         */
/*                                                                                              */
/* DESCRIPTION: 初期化                                                                          */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : none                                                                            */
/* ObUTPUT     : none                                                                            */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void hw_init(void)
{
	/* clock setting */
	system_clk_init();
	/* interrupt setting */
	intc_init();
	/* mux setting */
	setup_sysmgr();
	/* rstmgr setting */
	RstmgrInit();
}
