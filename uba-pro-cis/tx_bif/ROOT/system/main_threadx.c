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
//#include "operation.h"
#include "sub_functions.h"
#include "sysmgr_setup.h"
#include "kernel_config.h"
#include "txi_data.h"
#include "hal_gpio.h"
#include "hal_wdt.h"
#include "hal_led.h"
#include "hal_clk.h"
#include "hal_cyclonev_rstmgr.h"
//#include "fram_drv.h"
#include "download.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

extern void sample_ictrl_callback_SGI1_cold_reset(uint32_t icciar, void * context);

UCHAR tx_trace_buffer[65536 * 2];		// TraceX用トレースバッファ, 19/04/04



const UINT32 CFG_I2C_SYSCLK[] = {100000000, 100000000, 100000000, 100000000};		/* ペリフェラルシステムクロック周波数(100MHz) */
//const UINT32 CFG_I2C_CLK[] = {100000, 100000, 100000, 100000};				/* I2C目標CLK周波数(100kHz) */
const UINT32 CFG_I2C_CLK[] = {200000, 200000, 200000, 200000};				/* I2C目標CLK周波数(200kHz)、400kHzを200kHzに変更, 19/06/27 */

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
#if 1
	/* clear pending interrupts */
	alt_int_dist_pending_clear(ALT_INT_INTERRUPT_I2C3_IRQ);
	alt_int_dist_priority_set(ALT_INT_INTERRUPT_I2C3_IRQ, IPL_USER_NORMAL);
#else
	T_DINH dinh = {0};
	extern void i2c_isr3(void);
	/* clear pending interrupts */
	alt_int_dist_pending_clear(ALT_INT_INTERRUPT_I2C3_IRQ);
	dinh.inhatr = TA_HLNG;
	dinh.inthdr = i2c_isr3;
	def_inh(ALT_INT_INTERRUPT_I2C3_IRQ, &dinh);
	alt_int_dist_priority_set(ALT_INT_INTERRUPT_I2C3_IRQ, IPL_KERNEL_NORMAL);		// システムタイマ割込みレベル設定, 19/05/29
#endif
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
	if( get_core_id() == 0 ){
		/* INTC Reset */
		for( n = 0 ; n < 8 ; n ++ ){
			IOREG32(INTC_BASE,INTC_ICDICER(n)) = 0xFFFFFFFF;
			IOREG32(INTC_BASE,INTC_ICDICPR(n)) = 0xFFFFFFFF;
		}
		for( n = 0 ; n < 256 ; n ++ ){
#if 1
			IOREG8(INTC_BASE,INTC_ICDIPR(n)) = 0xF0;
#else
			IOREG8(INTC_BASE,INTC_ICDIPR(n)) = 0;
#endif
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
#if defined(PRJ_IVIZION2) //いらない 将来的にはPLLのクロックは落とさないで、MPUクロックのみ落とす
#if (BV_UNIT_TYPE==ES_MODEL)
#if _DEBUG_CLOCK_DOWN
	// clockダウン対応
	get_pll_info();
	// 800mhz to 400mhz
	set_main_pll_down(2);
	get_pll_info();
#endif
#endif
#endif
#if 0 //消費電力調査
	//// 1) MAIN PLL  1600mhz to 800mhz
	set_main_pll(800*1000*1000);
	set_peri_pll(400*1000*1000);
	// MPU CLK set 200mhz
	set_mpu_clock(200*1000*1000);
	get_pll_info();
#endif
	// QSPI Download
	alt_clk_clock_enable(ALT_CLK_QSPI);
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
#if 0
	/* mux setting */
	setup_sysmgr();
	/* rstmgr setting */
	RstmgrInit();
	RSTMGR_PERMODRST_T permodrst;
	
	// Disable WatchDog Timer
	permodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST);
	permodrst.bit.l4wd0 = 1;
	IOREG32(RSTMGR_BASE, RSTMGR_PERMODRST) = permodrst.lword;
#endif
}
void _register_handler(void)
{
}
/*********************************************************************//**
 * @brief main_task select Host I/F procedure
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _main_select_protocol(void)
{
	u8 if_select;
	u8 protocol_select;

#if  defined(_PROTOCOL_ENABLE_ID003)
	ex_smrt_id = 03;
	if_select = IF_SELECT_TTL;
	protocol_select = PROTOCOL_SELECT_ID003;
#elif defined(_PROTOCOL_ENABLE_ID0G8)
	ex_smrt_id = 0xF8;
	if_select = IF_SELECT_USB;
	protocol_select = PROTOCOL_SELECT_ID0G8;
#else
	ex_smrt_id = 0;
	if_select = IF_SELECT_NONE;
	protocol_select = 0;
#endif /* _PROTOCOL_ENABLE_ID064 */
	ex_cline_status_tbl.if_select = if_select;
	ex_cline_status_tbl.protocol_select = protocol_select;
}
/*********************************************************************//**
 * @brief		initialize variables
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _init_val(void)
{
	//  変数の初期化
	memset(&ex_section_status, 0, sizeof(ex_section_status));
}

/*********************************************************************//**
 * @brief		Calculate ROM file CRC.
 * @param[in]	None
 * @return 		Succeeded or Failure
 * @retval 		true Succeeded
 * @retval 		false Failure
 **********************************************************************/
#if 0
static bool _check_program_crc(void)
{
	// ROM program start address=0x00180000, end address=0x010FFFFD
    u16 calc_crc;
    u16 tmp_crc;
    u32 start_addr;
    u32 end_addr;
    bool ret = true;

    start_addr = ROM_ALIAS_START_ADDRESS;
    end_addr = ROM_ALIAS_END_ADDRESS;

	/* DDR3に転送されたプログラムCRC算出 */
    calc_crc = _calc_crc( (u8 *)start_addr,
                                 (u32)(end_addr-start_addr-2+1),
								 true);
    /* DDR3に転送されたROM全体のCRC算出 */
    ex_rom_crc = _calc_crc( (u8 *)(end_addr - 1),
                                 (u32)2,
								 false);
    /* Flashに保存されているCRC処理 */
    tmp_crc = *(u8 *)(end_addr-1);
    tmp_crc = (tmp_crc << 8) & 0xff00;
    tmp_crc = tmp_crc + *(u8 *)end_addr;
	osw_printf("\n\r");
	osw_printf( "_check_program_crc:ex_rom_crc=0x%08x\n", (unsigned int)ex_rom_crc);
	osw_printf( "_check_program_crc:start=0x%08x\n", (unsigned int)start_addr);
	osw_printf( "_check_program_crc:end=0x%08x\n", (unsigned int)end_addr);
    /* 一致するか比較 */
    if( calc_crc != tmp_crc )
    {
    	osw_printf("_check_program_crc failed.(rom=0x%04x, calc=0x%04x) \n\r",tmp_crc,calc_crc);
        ret = false;
    }

    return( ret );
}
#endif

extern const void * Load$$BIF_DATA$$Base;
extern const void * Image$$BIF_DATA$$Base;
extern const void * Image$$BIF_DATA$$Length;
extern const void * Image$$BIF_BSS$$ZI$$Base;
extern const void * Image$$BIF_BSS$$ZI$$Length;

// 20211001 Grapesystem debug for USB/DEVICE DMA mode
extern const void * Load$$TMP_NONCACHE$$Base;
extern const void * Image$$TMP_NONCACHE$$Base;
extern const void * Image$$TMP_NONCACHE$$Length;

void _init_system_memory(void)
{
#if 0
	//debug
	void *size1,*size2;
	void *addr1,*addr2,*addr3;
	addr1 = &Image$$BIF_DATA$$Base;
	addr2 = &Image$$BIF_BSS$$ZI$$Base;
	addr3 = &Load$$BIF_DATA$$Base;
	size1 = &Image$$BIF_DATA$$Length;
	size2 = &Image$$BIF_BSS$$ZI$$Length;
#endif
#if 1 // 20211001 Grapesystem debug for USB/DEVICE DMA mode
	void *size1;
	void *addr1,*addr2;
	addr1 = &Image$$TMP_NONCACHE$$Base;
	addr2 = &Load$$TMP_NONCACHE$$Base;
	size1 = &Image$$TMP_NONCACHE$$Length;
#endif
	/* clear .BSS section */
	memset( &Image$$BIF_BSS$$ZI$$Base, 0x00, (size_t) (((UINT32)&Image$$BIF_BSS$$ZI$$Length)));
	/* copy .data section */
	memcpy( (void *)&Image$$BIF_DATA$$Base, (void *)&Load$$BIF_DATA$$Base,((UINT32)&Image$$BIF_DATA$$Length));

	// 20211001 Grapesystem debug for USB/DEVICE DMA mode
	memcpy( (void *)&Image$$TMP_NONCACHE$$Base, (void *)&Load$$TMP_NONCACHE$$Base,((UINT32)&Image$$TMP_NONCACHE$$Length));
}
void _bif_if_download(void)
{
	/* clear BSS section, Copy Initial value to DATA section */
	_init_system_memory();

	_main_select_protocol();
	ex_operating_mode = OPERATING_MODE_IF_DOWNLOAD;
    /* ダウンロード開始要求フラグ     */
	ex_cline_download_control.is_start_received = DL_START_NORMAL;

	hw_init();		// 残りのハードウェアモジュールの設定, 19/04/05

	/* Setup the Altera hardware. */
	tx_alt_initialize_low_level();

	/* Enter the ThreadX kernel.  */
	tx_kernel_enter();

/* never come here (for warning of gcc) */
    return;
};
void _bif_if_diff_download(void)
{
	/* clear BSS section, Copy Initial value to DATA section */
	_init_system_memory();

	_main_select_protocol();
	ex_operating_mode = OPERATING_MODE_IF_DIFF_DOWNLOAD;
    /* ダウンロード開始要求フラグ     */
	ex_cline_download_control.is_start_received = DL_START_DIFFERENTIAL;

	hw_init();		// 残りのハードウェアモジュールの設定, 19/04/05

	/* Setup the Altera hardware. */
	tx_alt_initialize_low_level();

	/* Enter the ThreadX kernel.  */
	tx_kernel_enter();

/* never come here (for warning of gcc) */
    return;
};
void _bif_device_usb_download(void)
{
	/* clear BSS section, Copy Initial value to DATA section */
	_init_system_memory();

	ex_operating_mode = OPERATING_MODE_USB_DOWNLOAD;

	hw_init();		// 残りのハードウェアモジュールの設定, 19/04/05

	/* Setup the Altera hardware. */
	tx_alt_initialize_low_level();

	/* Enter the ThreadX kernel.  */
	tx_kernel_enter();

/* never come here (for warning of gcc) */
    return;
};
void _bif_host_usb_download(void)
{
	/* clear BSS section, Copy Initial value to DATA section */
	_init_system_memory();

	ex_operating_mode = OPERATING_MODE_FILE_DOWNLOAD;

	hw_init();		// 残りのハードウェアモジュールの設定, 19/04/05

	/* Setup the Altera hardware. */
	tx_alt_initialize_low_level();

	/* Enter the ThreadX kernel.  */
	tx_kernel_enter();

/* never come here (for warning of gcc) */
    return;
};
void _bif_subline_usb_download(void)
{
	/* clear BSS section, Copy Initial value to DATA section */
	_init_system_memory();

	ex_operating_mode = OPERATING_MODE_SUBLINE_DOWNLOAD;

	hw_init();		// 残りのハードウェアモジュールの設定, 19/04/05

	/* Setup the Altera hardware. */
	tx_alt_initialize_low_level();

	/* Enter the ThreadX kernel.  */
	tx_kernel_enter();

/* never come here (for warning of gcc) */
    return;
};
void _bif_start(void)
{
	/* clear BSS section, Copy Initial value to DATA section */
	_init_system_memory();

	_main_select_protocol();
	ex_operating_mode = OPERATING_MODE_WAIT_REQ;

	hw_init();		// 残りのハードウェアモジュールの設定, 19/04/05

	/* Setup the Altera hardware. */
	tx_alt_initialize_low_level();

	/* Enter the ThreadX kernel.  */
	tx_kernel_enter();

	/* XXX link with alt_p2uart */
	// alt_log_done(DEFAULT_TERM);		// HWLIBの改善に伴いコメント, 19/10/25

/* never come here (for warning of gcc) */
    return;
}
#if 1
enum {
	/* FLASHメモリマップ */
	//FLASH_AREA_BIFPROGRAM_ORG 	= (0x00020000),
	//FLASH_AREA_BIFPROGRAM_SIZE 	= (0x000E0000),
	//FLASH_AREA_MAINPROGRAM_ORG 	= (0x00100000),
	//FLASH_AREA_MAINPROGRAM_SIZE = (0x00100000),


	FLASH_AREA_ROMPROGRAM_ORG 	= (0x00180000),
	FLASH_AREA_ROMPROGRAM_SIZE 	= (0x00E80000),
	/* RAMメモリマップ */
	RAM_AREA_MAIN_PROGRAM 		= (0x00280000),

	// MMU TLBアドレス
	// TLB_ADDR = 0xFFFF0000,
	TLB_ADDR = -65536,		// 0xFFFF0000
};
/*********************************************************************//**
 * @brief		Copy ROM image from QSPI to DDR3.
 * @param[in]	None
 * @return 		Succeeded or Failure
 * @retval 		true Succeeded
 * @retval 		false Failure
 **********************************************************************/
#if 0
bool _bif_rom_copy_to_ddr(void)
{
	QSPI_BUF_INFO buf_info = {0};
	UINT32 actual_size = 0;
	bool load_stat = false;
	INT32 cnt;

#if 1
#define DIV_NUM 29
	for(cnt = 0; cnt < DIV_NUM;cnt++)
	{
		/* コード領域の読出し */
		buf_info.buf = (u8 *)RAM_AREA_MAIN_PROGRAM + cnt * (FLASH_AREA_ROMPROGRAM_SIZE/DIV_NUM);
		buf_info.addr = FLASH_AREA_ROMPROGRAM_ORG + cnt * (FLASH_AREA_ROMPROGRAM_SIZE/DIV_NUM);
		buf_info.len = FLASH_AREA_ROMPROGRAM_SIZE / DIV_NUM;
		buf_info.byte_count = &actual_size;

		load_stat = QSPI_Flash_Read(&hQFlash, &buf_info);
		if(load_stat == FALSE)
		{
			break;
		}
		_hal_feed_wdt();
	}
#else
	/* コード領域の読出し */
	buf_info.buf = (u8 *)RAM_AREA_PROGRAM;
	buf_info.addr = FLASH_AREA_ROMPROGRAM_ORG;
	buf_info.len = FLASH_AREA_ROMPROGRAM_SIZE;
	buf_info.byte_count = &actual_size;

	load_stat = QSPI_Flash_Read(&hQFlash, &buf_info);
#endif

	return load_stat;
}
#endif
#endif

int bif_main(int argc, char **argv)
{
	/* clear BSS section, Copy Initial value to DATA section */
	_init_system_memory();

#if 1
	// jump to main
	/*
	 * Initialize variables
	 */
	_init_val();
	/*
	 * Register the Exception handlers
	 */
	_register_handler();
	/* WDT */
	_hal_start_wdt();
	_hal_enable_wdt();
#if 0
	// MPU CLK set 800mhz
	set_mpu_clock(800*1000*1000);
	/* Copy ROM(MAIN) to DDR3 */
	_bif_rom_copy_to_ddr();
	_hal_enable_wdt();
	// DONE:　ROM program全体のCRCチェック
	ex_section_status.rom_crc_result = _check_program_crc();
	// MPU CLK set 200mhz
	set_mpu_clock(200*1000*1000);
#else
	ex_section_status.rom_crc_result = 0;
#endif
    /* I/F領域のCRC正常な場合 */
    if( ex_section_status.rom_crc_result )
    {
    	/************************************/
    	/*		Move to	I/F process	        */
    	/************************************/
		_if_start();
    }
    else
    {
    	/************************************/
    	/*		Move to	BIF process	        */
    	/************************************/
        /* 各セクションのCRC算出し、      */
        /* どのセクションが異常がチェック */
        /* Boot I/Fだけで動作する */
    	_bif_start();
    }

	while( 1 );
#else
	hw_init();		// 残りのハードウェアモジュールの設定, 19/04/05
	
	/* Setup the Altera hardware. */
	tx_alt_initialize_low_level();
	
	/* Enter the ThreadX kernel.  */
	tx_kernel_enter();
	
	/* XXX link with alt_p2uart */
	// alt_log_done(DEFAULT_TERM);		// HWLIBの改善に伴いコメント, 19/10/25
	
#endif
	/* Never get here in a ThreadX system...  */
	return 0;
}


void tx_application_define(void *first_unused_memory)
{
#if 1
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
	/* SGI2:RESERVED(jump to BIF) */

#if 0
	// CPU1リセット解除, 19/05/30
	IOREG32(RSTMGR_BASE,RSTMGR_MPUMODRST) &= (~0x2);
#endif
#else
	UINT ret;
	RSTMGR_MPUMODRST_T mpumodrst;

	ret &= 0;
	txi_init(first_unused_memory);
	ret = tx_trace_enable(tx_trace_buffer, sizeof(tx_trace_buffer), 50);		// イベントトレース許可, 19/04/04
	tx_trace_buffer_full_notify(tx_buffer_full_callback);		// イベントトレースバッファフル通知コールバック, 19/04/08

	OSW_SYS_init(0, 0, 0, 0);		// JSLWare初期設定
	setup_gpio();		/* GPIOセットアップ */

	kernel_config();		// カーネルモジュール設定, 19/04/05

	// CPU1リセット解除, 19/05/30
	mpumodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MPUMODRST);
	mpumodrst.bit.cpu1 = 0;
	IOREG32(RSTMGR_BASE, RSTMGR_MPUMODRST) = mpumodrst.lword;
#endif
	
	alt_gpt_tmr_start(SYS_TIMER);		// システムタイマスタートをmain_taskから移動, 19/07/23
}



