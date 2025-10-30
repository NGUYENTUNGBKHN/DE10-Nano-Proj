/******************************************************************************/
/*! @addtogroup Main
    @file       pl.c
    @brief      PL電源のON/OFF切り替え。ビットストリームデータの転送を行う。
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "alt_dma.h"
#include "alt_fpga_manager.h"
#include "alt_bridge_manager.h"
#include "alt_address_space.h"

#include "common.h"
//#include "custom.h"

#include "pl.h"
#include "pl_cis.h"
#include "pl_evrec.h"
#include "motor_ctrl.h"
#include "operation.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

/**** STRUCTURES ********************************************************************************/
enum {
	IH_MAGIC = 0x27051956,		/* Image Magic Number */
	IH_NMLEN = 32,				/* Image Name Length */
	FPGA_CONFIG_RETRY = 5,		// FPGAコンフィグレーションリトライ回数

	/* DDR3開始アドレス */
	DDR3_BASE = (0x00100000),
	/* FLASHメモリマップ */
	//FLASH_AREA_FPGACONFIG_ORG = (0x281000),
	FLASH_AREA_FPGACONFIG_ORG = (0x381000),
	FLASH_AREA_FPGACONFIG_SIZE = (0x3FF000),
	/* DDR3メモリマップ */
	DDR3_AREA_FPGACONFIG_ORG = (FLASH_AREA_FPGACONFIG_ORG+DDR3_BASE),
	DDR3_AREA_FPGACONFIG_SIZE = (FLASH_AREA_FPGACONFIG_SIZE),
	/* FLASHメモリデータ */
	FLASH_DATA_BLANK = -1,
};

typedef struct {
	uint8_t id[4];
	uint8_t version[64];
	void* start_address;
	uint32_t size;
	uint32_t crc32;
	uint8_t reserve[176];
} PROGRAM_INFO_T;		// ブロックヘッダ構造

typedef struct {
	uint32_t ih_magic;		/* Image Header Magic Number */
	uint32_t ih_hcrc;		/* Image Header CRC Checksum */
	uint32_t ih_time;		/* Image Creation Timestamp */
	uint32_t ih_size;		/* Image Data Size */
	uint32_t ih_load;		/* Data	 Load  Address */
	uint32_t ih_ep;		/* Entry Point Address */
	uint32_t ih_dcrc;		/* Image Data CRC Checksum */
	uint8_t ih_os;		/* Operating System */
	uint8_t ih_arch;		/* CPU architecture */
	uint8_t ih_type;		/* Image Type */
	uint8_t ih_comp;		/* Compression Type */
	uint8_t ih_name[IH_NMLEN];		/* Image Name */
} image_header_t;		// rbfヘッダ構造


typedef union {
	uint32_t lword;
	struct {
		uint32_t membl:2;
		uint32_t useeccasdata:1;
		uint32_t applycfg:1;
		uint32_t :28;
	} bit;
} SDRAM_STATICCFG_T;
typedef union {
	uint32_t lword;
	struct {
		uint32_t rom:1;
		uint32_t ocram:1;
		uint32_t sysmgr:1;
		uint32_t sysmgrcold:1;
		uint32_t fpgamgr:1;
		uint32_t acpidmap:1;
		uint32_t s2f:1;
		uint32_t s2fcold:1;
		uint32_t nrstpin:1;
		uint32_t timestampcold:1;
		uint32_t clkmgrcold:1;
		uint32_t scanmgr:1;
		uint32_t frzctrlcold:1;
		uint32_t sysdbg:1;
		uint32_t dbg:1;
		uint32_t tapcold:1;
		uint32_t sdrcold:1;
		uint32_t :15;
	} bit;
} RSTMGR_MISCMODRST_T;


extern struct _BEZEL_LED_INFO ex_bezel_led;

/**** DEFINES ***********************************************************************************/



#define SYS_ISW_HANDOFF_AXIBRIDGE			SYS_ISW_HANDOFF(0)
#define SYS_ISW_HANDOFF_L3REMAP				SYS_ISW_HANDOFF(1)
#define SYS_ISW_HANDOFF_FPGAINTF			SYS_ISW_HANDOFF(2)
#define SYS_ISW_HANDOFF_FPGA2SDR			SYS_ISW_HANDOFF(3)
#define SYS_ISW_HANDOFF_ROWBITS				SYS_ISW_HANDOFF(4)

#if (DEBUG!=1)
extern uint32_t alt_crc32(uint32_t crc, uint8_t *p, uint32_t len);
extern uint32_t calc_crc32(uint8_t *start_adr, uint32_t len, uint32_t value);
/************************************************************************************************/
/* FUNCTION   : fpga_config_write                                                               */
/*                                                                                              */
/* DESCRIPTION: FPGAコンフィグデータ書込み                                                      */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : config_address  コンフィグレーションデータ先頭アドレス                          */
/* INPUT      : size            コンフィグレーションデータサイズ                                */
/* OUTPUT     : none                                                                            */
/* RESULTS    : true            正常終了                                                        */
/*              false           エラー終了                                                      */
/*                                                                                              */
/************************************************************************************************/
static bool fpga_config_write(uint8_t* config_address, uint32_t size)
{
	ALT_STATUS_CODE status = ALT_E_SUCCESS;
	SDRAM_STATICCFG_T staticcfg = {0};
	RSTMGR_MISCMODRST_T miscmodrst = {0};		// 19/12/24
	bool ret = true;

	if (ret == true) {
		status = alt_fpga_init();
		ret = (status == ALT_E_SUCCESS ? true : false);
	}

	/* Verify power is on */
	if (ret == true) {
		ret = (alt_fpga_state_get() != ALT_FPGA_STATE_POWER_OFF ? true : false);
	}

	/* Take control of the FPGA CB */
	if (ret == true) {
		status = alt_fpga_control_enable();
		ret = (status == ALT_E_SUCCESS ? true : false);
	}

	/* Program the FPGA */
	if (ret == true) {
		/* Try the full configuration a few times. */
		uint32_t retry;
		for (retry = 0, ret = false; (retry < FPGA_CONFIG_RETRY) && (ret == false); retry++) {
			status = alt_fpga_configure(config_address, size);
			ret = (status == ALT_E_SUCCESS ? true : false);
		}
		if (ret == true) {		// FPGAコンフィグがOKのとき
			miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);
			while (miscmodrst.bit.s2f == 0) {		// s2fビットをセットする
				miscmodrst.bit.s2f = 1;
				IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST) = miscmodrst.lword;
				miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);
			}
			while (miscmodrst.bit.s2f != 0) {		// s2fビットをクリアする
				miscmodrst.bit.s2f = 0;
				IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST) = miscmodrst.lword;
				miscmodrst.lword = IOREG32(RSTMGR_BASE, RSTMGR_MISCMODRST);
			}
		}
	}

	if (ret == true) {
		/* enable signals from hps peripheral controller to fpga (based on handoff) */
		// writel(readl(ISWGRP_HANDOFF_FPGAINTF), SYSMGR_FPGAINTF_MODULE);
		IOREG32(SYS_BASE, SYS_FPGAINT_MODULE) = IOREG32(SYS_BASE, SYS_ISW_HANDOFF_FPGAINTF);

		/* enable signals from fpga to hps sdram (based on handoff) */
		// setbits_le32((SOCFPGA_SDR_ADDRESS + SDR_CTRLGRP_STATICCFG_ADDRESS), SDR_CTRLGRP_STATICCFG_APPLYCFG_MASK);
		staticcfg.lword = IOREG32(SDRC_BASE, SDRAM_STATICCFG);
		staticcfg.bit.applycfg = 1;
		IOREG32(SDRC_BASE, SDRAM_STATICCFG) = staticcfg.lword;

		// writel(readl(ISWGRP_HANDOFF_FPGA2SDR), (SOCFPGA_SDR_ADDRESS + SDR_CTRLGRP_FPGAPORTRST_ADDRESS));
		IOREG32(SDRC_BASE, SDRAM_FPGAPORTRST) = IOREG32(SYS_BASE, SYS_ISW_HANDOFF_FPGA2SDR);


		/* enable the axi bridges if FPGA programmed */
		// writel(readl(ISWGRP_HANDOFF_AXIBRIDGE), &reset_manager_base->brg_mod_reset);
		IOREG32(RSTMGR_BASE, RSTMGR_BRGMODRST) = IOREG32(SYS_BASE, SYS_ISW_HANDOFF_AXIBRIDGE);

		/* remap the enabled bridge into NIC-301 */
		// writel(readl(ISWGRP_HANDOFF_L3REMAP), SOCFPGA_L3REGS_ADDRESS);
		IOREG32(L3REGS_BASE, L3REGS_REMAP) = IOREG32(SYS_BASE, SYS_ISW_HANDOFF_L3REMAP);
	}

	if (ret == true) {
		status = alt_fpga_control_disable();
		ret = (status == ALT_E_SUCCESS ? true : false);
	}

	if (ret == true) {
		status = alt_fpga_uninit();
		ret = (status == ALT_E_SUCCESS ? true : false);
	}

	return ret;
}

/************************************************************************************************/
/* FUNCTION   : fpga_config_body                                                                */
/*                                                                                              */
/* DESCRIPTION: FPGAコンフィグレーション                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : config_address  コンフィグレーションデータ先頭アドレス                          */
/* INPUT      : size            コンフィグレーションデータ最大サイズ                            */
/* OUTPUT     : none                                                                            */
/* RESULTS    : true            正常終了                                                        */
/*              false           エラー終了                                                      */
/*                                                                                              */
/************************************************************************************************/
static bool fpga_config_body(uint32_t config_address, uint32_t max_size)
{
	image_header_t header = {0};		// rbfヘッダ
	uint32_t header_size = sizeof(header);
	uint32_t crc = 0;		// CRC値
	uint32_t value = 0;
	bool ret = true;

	// rbfヘッダ読出し
	memcpy(&header, (void *)config_address, header_size);
	// ブランクチェック
	ret = (header.ih_magic != (uint32_t)FLASH_DATA_BLANK ? true : false);		// ブランクチェック
	if (ret == true) {		// ヘッダがブランクで無い時
		crc = __rev(header.ih_hcrc);		// ヘッダCRC値(エンディアン変換)
		header.ih_hcrc = 0;		// ヘッダCRC計算のためCRC値をクリア
		value = alt_crc32(0, (uint8_t *)&header, header_size);		// ヘッダCRC値計算
		ret = (value == crc ? true : false);		// ヘッダCRC計算値比較
	}

	if (ret == true) {		// ヘッダCRC値が一致するとき
		if (__rev(header.ih_magic) == IH_MAGIC) {		// エンディアン変換でrbfマジックナンバが一致するとき
			header.ih_magic = __rev(header.ih_magic);		// エンディアン変換する
			header.ih_hcrc = __rev(header.ih_hcrc);
			header.ih_time = __rev(header.ih_time);
			header.ih_size = __rev(header.ih_size);
			header.ih_load = __rev(header.ih_load);
			header.ih_ep = __rev(header.ih_ep);
			header.ih_dcrc = __rev(header.ih_dcrc);
		}
	}

	if (ret == true) {		// コンフィグデータ読出し正常のとき
		value = alt_crc32(0, (uint8_t *)(config_address + header_size), header.ih_size);		// データCRC値計算
		ret = (value == header.ih_dcrc ? true : false);		// データCRC値比較
	}
	if (ret == true) {		// データCRC値が一致するとき
		// コンフィグ実行
		ret = fpga_config_write((uint8_t*)(config_address + header_size), header.ih_size);
	}

	return ret;
}
/************************************************************************************************/
/* FUNCTION   : fpga_config                                                                     */
/*                                                                                              */
/* DESCRIPTION: FPGAコンフィグレーション                                                        */
/*----------------------------------------------------------------------------------------------*/
/* INPUT      : config_address  コンフィグレーションデータ先頭アドレス                          */
/* INPUT      : max_size        コンフィグレーションデータ最大サイズ                            */
/* OUTPUT     : none                                                                            */
/* RESULTS    : true            正常終了                                                        */
/*              false           エラー終了                                                      */
/*                                                                                              */
/************************************************************************************************/
static bool fpga_config(uint32_t config_address, uint32_t max_size)
{
	bool ret = true;
	PROGRAM_INFO_T program_info = {0};		// ブロックヘッダ
	uint32_t value = 0;
	char config_name[] = "FPGA";

	ret = fpga_config_body(config_address, max_size);		// rbfコンフィグデータ

	if (ret == false) {		// rbfコンフィグNGのとき
		// rbfヘッダ読出し
		memcpy(&program_info, (void *)config_address, sizeof(program_info));

		ret = (memcmp(program_info.id, config_name, sizeof(program_info.id)) == 0 ? true : false);
		if (ret == true) {		// ブロックヘッダのとき
			value = calc_crc32((void *)config_address, program_info.size, 0);
			ret = (value == program_info.crc32 ? true : false);		// ヘッダCRC計算値比較
		}
		if (ret == true) {		// CRC32値が一致するとき
			ret = fpga_config_body(config_address + sizeof(program_info), max_size);		// rbfコンフィグデータ
		}
	}

	return ret;
}
#endif

/******************************************************************************
 *! @brief Check FPGA state
 * @param[in]	None
 * @return 		Succeeded or Failure
 * @retval 		true Succeeded
 * @retval 		false Failure
******************************************************************************/
int check_pl_state(void)
{
	// TOOD:PLがリセットされていないかチェック
	ALT_FPGA_STATE_t stat;
	stat = alt_fpga_state_get();

	if(stat != ALT_FPGA_STATE_USER_MODE)
		return 0;

	return 1;

}
/******************************************************************************
 *! @brief First Initialze PL
 * @param[in]	None
 * @return 		Succeeded or Failure
 * @retval 		true Succeeded
 * @retval 		false Failure
******************************************************************************/
int initialize_pl(void)
{
#if (DEBUG==1) // debug
    /* ICE-MODEではPCAP使用できない。 */
    /* Programmer (Quartus Prime 20.1)を使用する。 */
	return 1;
#else
	bool fpga_stat = false;
	bool stat = false;
    /* TODO:CIS Power ON */
	/*
	 * TODO:FPGA Configuration
	 */
	// FPGAコンフィグデータ確認
	stat = fpga_config(DDR3_AREA_FPGACONFIG_ORG, DDR3_AREA_FPGACONFIG_SIZE);
	fpga_stat = (stat == false ? false : true);		// FPGAコンフィグレーションNGのとき使用不可

	return stat;
#endif
}

#if 1//あまり意味ないように思うがのこしておく
void set_dac_ctl(void)
{
	DAC_CTL_UNION data;
	data.LWORD = FPGA_REG.DAC_CTL.LWORD;
	data.BIT.ZERO = 0;
	FPGA_REG.DAC_CTL.LWORD = data.LWORD;
}
#endif

/******************************************************************************
 *! @brief Enable/Disable PL
 * @param[in]	enable
 *              @arg 1 : Power ON
 *              @arg 0 : Power OFF
 * @return 		Succeeded or Failure
 * @retval 		true Succeeded
 * @retval 		false Failure
******************************************************************************/
int enable_pl(int enable)
{
	PW_CTL_UNION pw_ctl;

	if(!check_pl_state())
	{
		_main_system_error(1, 253);
	}
    if(enable)
    {
    	// FPGA CLK(100MHz) enable
    	alt_fpga_gpo_write(0x00000001, 0x00000001);
	#if 1//#if (_DEBUG_FPGA_CLOCK_NOT_STOP==1)
     	ex_fpga_dummy_clk = 1;
     	dly_tsk(10);
	#endif
    	while(!get_pl_state()){dly_tsk(10);};
	#if (FPGA_LOG_ENABLE==1)
		_pl_evrec_start(1);
	#endif /* FPGA_LOG_ENABLE */

    	pw_ctl.LWORD = FPGA_REG.PW_CTL.LWORD;
		pw_ctl.BIT.VM = 1;	//FPGAマニュアルと異なりUBAも1にしないと、搬送、幅よせなどの動作がすごく遅くなる
    	FPGA_REG.PW_CTL.LWORD = pw_ctl.LWORD;
    	dly_tsk(10);
	}
    else
    {
    	pw_ctl.LWORD = FPGA_REG.PW_CTL.LWORD;
    	pw_ctl.BIT.VM = 0;
    	pw_ctl.BIT.CIS = 0;		/* CIS Power OFF */
    	FPGA_REG.PW_CTL.LWORD = pw_ctl.LWORD;
		ex_cis = 0;
    	dly_tsk(10);

	#if (FPGA_LOG_ENABLE==1)
    	_pl_evrec_stop();
	#endif /* FPGA_LOG_ENABLE */
	#if 1//#if (_DEBUG_FPGA_CLOCK_NOT_STOP==1)
     	ex_fpga_dummy_clk = 0;
	#else
    	/* FPGA CLK(100MHz) disable */
    	alt_fpga_gpo_write(0x00000001, 0x00000000);
	#endif	// _DEBUG_FPGA_CLOCK_NOT_STOP
    }

	//あまり意味ないように思うがのこしておく// #if defined(UBA)
	set_dac_ctl();
	//#endif

	return 1;
}
/******************************************************************************
 *! @brief Enable/Disable PL
 * @param       None
 * @return  	enable
 *              @arg 1 : FPGA CLK ON
 *              @arg 0 : FPGA CLK OFF
******************************************************************************/
int get_pl_state(void)
{
#if 1//#if (_DEBUG_FPGA_CLOCK_NOT_STOP==1)
	return ex_fpga_dummy_clk;
#else
	if(alt_fpga_gpi_read(0x00000001))
	{
		return 1;
	}
	return 0;
#endif
}


/******************************************************************************
 *! @brief Enable/Disable IOEX
 * @param[in]	enable
 *              @arg 1 : Power ON
 *              @arg 0 : Power OFF
 * @return 		Succeeded or Failure
 * @retval 		true Succeeded
 * @retval 		false Failure
******************************************************************************/
u8 _pl_ioex_reset(u8 set)
{
	PW_CTL_UNION pw_ctl;
	//if(get_pl_state() == 0)
	//{
	//	return 0;
	//}
    if(set)
    {
		pw_ctl.LWORD = FPGA_REG.PW_CTL.LWORD;
		/* IOEX Power OFF */
		pw_ctl.BIT.IOEX = 1;
		FPGA_REG.PW_CTL.LWORD = pw_ctl.LWORD;
	}
    else
    {
    	pw_ctl.LWORD = FPGA_REG.PW_CTL.LWORD;
        /* IOEX Power ON */
    	pw_ctl.BIT.IOEX = 0;
    	FPGA_REG.PW_CTL.LWORD = pw_ctl.LWORD;
    }

	return 1;
}



u8 _pl_bezel_led_uba(u8 data)
{

	IF_UNION if_data;
	if_data.LWORD = (u32)FPGA_REG.IF.LWORD;

	if( data == 0 )
	{
		if_data.BIT.ITTL3 = 0;
		ex_bezel_led.bezel_on = 0;
	}
	else
	{
		if_data.BIT.ITTL3 = 1;
		ex_bezel_led.bezel_on = 1;		
	}

	FPGA_REG.IF.LWORD = if_data.LWORD;
	ex_bezel_led.bezel_led_tm = 0;
}

#if 0 //for debug
u8 _pl_subboard_type(u8 data)	//2025-08-18
{

	IF_UNION if_data;
	if_data.LWORD = (u32)FPGA_REG.IF.LWORD;

	ex_subboard_bit0 = if_data.BIT.STYPE0;
	ex_subboard_bit1 = if_data.BIT.STYPE1;
	ex_subboard_bit2 = if_data.BIT.STYPE2;
}
#endif

#if 1 //#if (DEBUL_PL_PLL_LOCK_IRQ==1)

/************************** Private Definitions *****************************/
#define FPGA_PLL_IRQ_INTR_ID			OSW_INT_FPGA_IRQ63
/************************** Private Variables *****************************/
OSW_ISR_HANDLE hplIsr63;


void pl_pll_reset_core(void)
{
	// FPGA CLK(100MHz) enable
	alt_fpga_gpo_write(0x00000001, 0x00000001);
}
void _pl_pll_reset(void)
{
	ALT_STATUS_CODE result;
	/* FPGA CLK(100MHz) disable */
	alt_fpga_gpo_write(0x00000001, 0x00000000);
	while(1)
	{
		result = alt_int_sgi_trigger(ALT_INT_INTERRUPT_SGI5, ALT_INT_SGI_TARGET_ALL_EXCL_SENDER, (alt_int_cpu_target_t)0, true);
		if(result == ALT_E_SUCCESS)
		{
			break;
		}
	}
	while(alt_fpga_gpi_read(0x00000001) == 0)
	{
	}
}
void _intr_pll(void)
{
	_pl_pll_reset();
	iset_flg(ID_POWER_FLAG, EVT_PLL_LOCK);
}

/*********************************************************************//**
 * @brief		FPGA PLL interrupt
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_intr_pll(void)
{
	// Disable UART interrupts
	OSW_ISR_disable(FPGA_PLL_IRQ_INTR_ID);
	_intr_pll();
	OSW_ISR_enable(FPGA_PLL_IRQ_INTR_ID);
}

/*********************************************************************//**
 * @brief		PLL Interrupt initialize
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_pll_init(void)
{
	OSW_ISR_HANDLE hSgiIsr5;
	// PLL IRQ Handler
	if(hplIsr63.hdl == 0)
	{
		OSW_ISR_create( &hplIsr63, FPGA_PLL_IRQ_INTR_ID, _pl_intr_pll);
		OSW_ISR_set_priority(FPGA_PLL_IRQ_INTR_ID, IPL_USER_HIGHEST);
		set_int_typ(FPGA_PLL_IRQ_INTR_ID, IRQ_EDGE_RISING);

		/* For Dual Core Cold Reset Interrupt */
		OSW_ISR_create( &hSgiIsr5, OSW_INT_SGI5, pl_pll_reset_core);
		OSW_ISR_enable( OSW_INT_SGI5 );
	}
	// Disable Interrupt PLL unlocked
	OSW_ISR_disable(FPGA_PLL_IRQ_INTR_ID);
}
/*********************************************************************//**
 * @brief		PLL enable
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_pll_interrupt_enable(void)
{
	// Disable Interrupt PLL
	OSW_ISR_enable(FPGA_PLL_IRQ_INTR_ID);
}
/*********************************************************************//**
 * @brief		PLL disable
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _pl_pll_interrupt_disable(void)
{
	// Disable Interrupt PLL
	OSW_ISR_disable(FPGA_PLL_IRQ_INTR_ID);
}
#endif
