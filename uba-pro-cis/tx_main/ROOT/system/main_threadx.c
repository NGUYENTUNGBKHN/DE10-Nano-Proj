/* This is a small demo of the high-performance ThreadX kernel.  It includes examples of eight
   threads of different priorities, using a message queue, semaphore, mutex, event flags group,
   byte pool, and block pool.  */

#include <kernel.h>		// kernel.h内でEXTERN宣言を操作するので、EXTERNの前でincludeする
#define EXTERN extern

#include <stdio.h>
#include <stdlib.h>
#include "tx_api.h"
#include "memorymap.h"
#define PRINTF_UART
#include "tx_alt_low_level.h"
#include "alt_interrupt.h"
#include "soc_cv_av/alt_clock_manager.h"
#include "js_oswapi.h"
#include "js_io.h"
#include "js_intc_reg.h"
#include "js_rstmgr_reg.h"
#include "js_i2c.h"
#include "operation.h"
#include "sub_functions.h"
#include "sysmgr_setup.h"
#include "kernel_config.h"
#include "txi_data.h"
#include "hal_gpio.h"
#include "hal_led.h"
#include "hal_clk.h"
#include "hal_cyclonev_rstmgr.h"
//#include "fram_drv.h"

#define EXT
#include "com_ram.c"
#include "cis_ram.c"
#include "jsl_ram.c"

extern void sample_ictrl_callback_SGI1_cold_reset(uint32_t icciar, void * context);
//extern void sample_ictrl_callback_SGI2_bif_download(uint32_t icciar, void * context);
//extern void sample_ictrl_callback_SGI3_bif_device_usb_download(uint32_t icciar, void * context);
//extern void sample_ictrl_callback_SGI4_bif_host_usb_download(uint32_t icciar, void * context);

UCHAR tx_trace_buffer[65536 * 2];		// TraceX用トレースバッファ, 19/04/04



const UINT32 CFG_I2C_SYSCLK[] = {100000000, 100000000, 100000000, 100000000};		/* ペリフェラルシステムクロック周波数(100MHz) */
#if (_DEBUG_I2C_CLK_100KHZ==1)
const UINT32 CFG_I2C_CLK[] = {100000, 100000, 100000, 100000};				/* I2C目標CLK周波数(100kHz) */
#elif (_DEBUG_I2C_CLK_400KHZ==1)
const UINT32 CFG_I2C_CLK[] = {400000, 400000, 400000, 400000};				/* ※i2c_reg_initでMST_CONをHIGH SPEEDに変更する必要あり　I2C目標CLK周波数(400kHz)  */
#else
const UINT32 CFG_I2C_CLK[] = {200000, 200000, 200000, 200000};				/* I2C目標CLK周波数(200kHz)、400kHzを200kHzに変更, 19/06/27 */
#endif

//UINT32 _end __attribute__ ((section("unused_head"), zero_init));		// 未使用メモリ領域の先頭を設定, ThreadXで使用, 19/11/21

void tx_buffer_full_callback(void *buffer)
{
	// 必要なら、トレースバッファフル時の処理を追記する
	// tx_trace_disable();		// トレース停止
}


/**** JSL Ware FUNCTIONS ************************************************************************/
/************************************************************************************************/
/* FUNCTION   : OSW_USR_logout                                                                  */
/*                                                                                              */
/* DESCRIPTION: ログ出力処理                                                                    */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : pcBuf       送信バッファ                                                        */
/*              ulLength    送信データ長                                                        */
/* OUTPUT     : none                                                                            */
/* RESULTS    : none                                                                            */
/*                                                                                              */
/************************************************************************************************/
void OSW_USR_logout(char *pcBuf, UINT32 ulLength)
{
#if 0
	UINT32 ulIsrStat;

	ulIsrStat = OSW_ISR_global_disable();

	while (ulLength--) {
		Uart_send(&l_hDbgPort, *pcBuf, 1);
		pcBuf++;
	}

	OSW_ISR_global_restore(ulIsrStat);
#endif
}

void setup_i2c0(void)
{
	I2C_PARAM i2c_prm;
	//I2C_HANDLE hI2c0;
	memset( (void *)&i2c_prm, 0, sizeof(i2c_prm) );
	i2c_prm.port = 0;
	i2c_prm.opt = I2C_OPT_MASTER;
	/* ドライバオープン */
	if( I2c_open( &hI2c0, &i2c_prm ) == FALSE ){
		program_error();
	}
}
void setup_i2c3(void)
{
	I2C_PARAM i2c_prm;
	//I2C_HANDLE hI2c3;
	memset( (void *)&i2c_prm, 0, sizeof(i2c_prm) );
	i2c_prm.port = 3;
	i2c_prm.opt = I2C_OPT_MASTER;
	/* ドライバオープン */
	if( I2c_open( &hI2c3, &i2c_prm ) == FALSE ){
		program_error();
	}
}
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

	#if defined(_PROTOCOL_ENABLE_ID0G8)
	// DFU Downloadの為、Flashアクセスを有効にする
	alt_clk_clock_enable(ALT_CLK_QSPI);
	#endif

	// グローバル変数clock_frequencyにクロックを格納
	get_pll_info();
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
extern const void * Load$$MAIN_DATA$$Base;
extern const void * Image$$MAIN_DATA$$Base;
extern const void * Image$$MAIN_DATA$$Length;
extern const void * Image$$MAIN_BSS$$ZI$$Base;
extern const void * Image$$MAIN_BSS$$ZI$$Length;
extern const void * Load$$COUNTRY_DATA$$Base;
extern const void * Image$$COUNTRY_DATA$$Base;
extern const void * Image$$COUNTRY_DATA$$Length;

// 20211001 Grapesystem debug for USB/DEVICE DMA mode
extern const void * Load$$TMP_NONCACHE$$Base;
extern const void * Image$$TMP_NONCACHE$$Base;
extern const void * Image$$TMP_NONCACHE$$Length;

int main(int argc, char **argv)
{
	// 20211001 Grapesystem debug for USB/DEVICE DMA mode
	void *size1;
	void *addr1,*addr2;
	addr1 = &Image$$TMP_NONCACHE$$Base;
	addr2 = &Load$$TMP_NONCACHE$$Base;
	size1 = &Image$$TMP_NONCACHE$$Length;
	/* ダミーアクセス */
	ADJDAT[0] = 0;
	ROWDAT[0] = 0;
	IMAGEDAT[0] = 0;
	/* clear .BSS section */
	memset( &Image$$MAIN_BSS$$ZI$$Base, 0x00, (size_t) (((UINT32)&Image$$MAIN_BSS$$ZI$$Length)));
	/* copy .data section */
	memcpy( (void *)&Image$$MAIN_DATA$$Base, (void *)&Load$$MAIN_DATA$$Base,((UINT32)&Image$$MAIN_DATA$$Length));
#if (DEBUG!=1)
	/* copy .data section */
	memcpy( (void *)&Image$$COUNTRY_DATA$$Base, (void *)&Load$$COUNTRY_DATA$$Base,((UINT32)&Image$$COUNTRY_DATA$$Length));
#endif
	// 20211001 Grapesystem debug for USB/DEVICE DMA mode
	memcpy( (void *)&Image$$TMP_NONCACHE$$Base, (void *)&Load$$TMP_NONCACHE$$Base,((UINT32)&Image$$TMP_NONCACHE$$Length));

	hw_init();		// 残りのハードウェアモジュールの設定, 19/04/05
	
	/* Setup the Altera hardware. */
	tx_alt_initialize_low_level();
	
	/* Enter the ThreadX kernel.  */
	tx_kernel_enter();
	
	/* XXX link with alt_p2uart */
	// alt_log_done(DEFAULT_TERM);		// HWLIBの改善に伴いコメント, 19/10/25
	
	/* Never get here in a ThreadX system...  */
	return 0;
}


void tx_application_define(void *first_unused_memory)
{
	UINT ret;

#ifdef TX_ENABLE_EVENT_TRACE
    tx_trace_enable(event_buffer, sizeof(event_buffer), 32);
#endif

	ret &= 0;
	txi_init(first_unused_memory);
	ret = tx_trace_enable(tx_trace_buffer, sizeof(tx_trace_buffer), 50);		// イベントトレース許可, 19/04/04
	tx_trace_buffer_full_notify(tx_buffer_full_callback);		// イベントトレースバッファフル通知コールバック, 19/04/08
	
	OSW_SYS_init(0, 0, 0, 0);		// JSLWare初期設定
	setup_gpio();		/* GPIOセットアップ */
	/* =========================================== */
	/* ===== I2Cマスタモード                    ===== */
	/* =========================================== */
	/* ドライバ初期化 */
	if( I2c_init( NULL ) == FALSE ){
		program_error();
	}
	setup_i2c0();		/* I2C0セットアップ */
	setup_i2c3();		/* I2C3セットアップ */

	//_hal_i2c3_init_iox();

	kernel_config();		// カーネルモジュール設定, 19/04/05

	//alt_int_isr_register(ALT_INT_INTERRUPT_SGI1, sample_ictrl_callback_SGI1_cold_reset, 0);/* SGI1:cold reset */
	//alt_int_isr_register(ALT_INT_INTERRUPT_SGI2, sample_ictrl_callback_SGI2_bif_download, 0);/* SGI2:jump to BIF(IF) */
	//alt_int_isr_register(ALT_INT_INTERRUPT_SGI3, sample_ictrl_callback_SGI3_bif_device_usb_download, 0);/* SGI2:jump to BIF(USB DEVICE) */
	//alt_int_isr_register(ALT_INT_INTERRUPT_SGI4, sample_ictrl_callback_SGI4_bif_host_usb_download, 0);/* SGI2:jump to BIF(USB DEVICE) */

	
	alt_gpt_tmr_start(SYS_TIMER);		// システムタイマスタートをmain_taskから移動, 19/07/23
}



