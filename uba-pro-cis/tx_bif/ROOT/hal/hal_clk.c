/******************************************************************************/
/*! @addtogroup Main
    @file       hal_clk.c
    @brief      clk LED driver.
    @brief      SoC動作クロック制御ドライバファイル。
    @date       2021/12/17
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2021 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * hal_clk.c
 *
 *  Created on: 2021/12/17
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#include "string.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "js_io.h"
#include "js_oswapi.h"
#include "common.h"
#include "hal_clk.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

/*********************************************************************//**
 * @brief		Control clk LED
 *				R0：BZLED1 R1：BZLED2 R2：BZLED3 R3 : BZLED4 R4: BZLED5(未使用)
 * @param[in]	data
 * @return 		E_OK:succeeded
 * 				E_I2C_SEND or other :failure
 **********************************************************************/
void set_main_pll(alt_freq_t new_freq)
{
	ALT_STATUS_CODE status;
	alt_freq_t cur_freq;
	float div;
	u32 main_multi,main_div;
	u32 cur_multi,cur_div;
	u32 new_multi,new_div;
    status = alt_clk_pll_vco_freq_get(ALT_CLK_MAIN_PLL, &cur_freq);
    if(new_freq == cur_freq)
    {
    	return;
    }
    div = (float)cur_freq / new_freq;
	//clock 1600mhz to 800mhz
	// get freq
	alt_clk_pll_vco_cfg_get(ALT_CLK_MAIN_PLL,&main_multi,&main_div);
	cur_multi = main_multi;
	cur_div = main_div;
	while(main_multi / div < cur_multi)
	{
		new_div = cur_div;
		new_multi = cur_multi * 0.8;
		if(cur_multi - new_multi < 1)
		{
			//error
			program_error();
		}
		if(new_multi < main_multi/div)
		{
			new_multi = main_multi / div;
		}
		alt_clk_pll_vco_cfg_set(ALT_CLK_MAIN_PLL,new_multi,new_div);
		while(alt_clk_pll_is_locked(ALT_CLK_MAIN_PLL) != ALT_E_TRUE){};
		alt_clk_pll_vco_cfg_get(ALT_CLK_MAIN_PLL,&cur_multi,&cur_div);
	}
	// get freq
	alt_clk_pll_vco_cfg_get(ALT_CLK_MAIN_PLL,&main_multi,&main_div);
    if (status == ALT_E_SUCCESS)
    {
    	//clock 400mhz to 400mhz
		alt_clk_divider_set(ALT_CLK_MAIN_PLL_C3, 1);
    }
}
/*********************************************************************//**
 * @brief		Control clk LED
 *				R0：BZLED1 R1：BZLED2 R2：BZLED3 R3 : BZLED4 R4: BZLED5(未使用)
 * @param[in]	data
 * @return 		E_OK:succeeded
 * 				E_I2C_SEND or other :failure
 **********************************************************************/
void set_peri_pll(alt_freq_t new_freq)
{
	ALT_STATUS_CODE status;
	alt_freq_t cur_freq;
	float div;
	u32 peri_multi,peri_div;
	u32 cur_multi,cur_div;
	u32 new_multi,new_div;
    status = alt_clk_pll_vco_freq_get(ALT_CLK_PERIPHERAL_PLL, &cur_freq);
    if(new_freq == cur_freq)
    {
    	return;
    }
    div = (float)cur_freq / new_freq;
	//clock 1000mhz to 400mhz
	// get freq
	alt_clk_pll_vco_cfg_get(ALT_CLK_PERIPHERAL_PLL,&peri_multi,&peri_div);
	cur_multi = peri_multi;
	cur_div = peri_div;
	while(peri_multi / div < cur_multi)
	{
		new_div = cur_div;
		new_multi = cur_multi * 0.8;
		if(cur_multi - new_multi < 1)
		{
			//error
			program_error();
		}
		if(new_multi < peri_multi/div)
		{
			new_multi = peri_multi / div;
		}
		alt_clk_pll_vco_cfg_set(ALT_CLK_PERIPHERAL_PLL,new_multi,new_div);
		while(alt_clk_pll_is_locked(ALT_CLK_PERIPHERAL_PLL) != ALT_E_TRUE){};
		alt_clk_pll_vco_cfg_get(ALT_CLK_PERIPHERAL_PLL,&cur_multi,&cur_div);
	}
	// get freq
	alt_clk_pll_vco_cfg_get(ALT_CLK_PERIPHERAL_PLL,&peri_multi,&peri_div);
    if (status == ALT_E_SUCCESS)
    {
    	//clock 400mhz to 200mhz
		alt_clk_divider_set(ALT_CLK_PERIPHERAL_PLL_C3, 2);
		alt_clk_divider_set(ALT_CLK_PERIPHERAL_PLL_C4, 2);
    }
}
/*********************************************************************//**
 * @brief		Control clk LED
 *				R0：BZLED1 R1：BZLED2 R2：BZLED3 R3 : BZLED4 R4: BZLED5(未使用)
 * @param[in]	data
 * @return 		E_OK:succeeded
 * 				E_I2C_SEND or other :failure
 **********************************************************************/
void disable_unused_peripheral(void)
{
	ALT_STATUS_CODE status;
	alt_freq_t freq;
	alt_freq_t osc1;
	u32 div;

	osc1 = alt_clk_ext_clk_freq_get(ALT_CLK_OSC1);
    status = alt_clk_pll_vco_freq_get(ALT_CLK_MAIN_PLL, &freq);
    if (status == ALT_E_SUCCESS)
    {
    	div = freq / osc1;
    	if(div > 512)
    	{
    		div = 512;
    	}
		// nand_sdmmc_base_clk => OSC1
		// cfg_h2f_user0_base_clk => OSC1
		//alt_clk_divider_set(ALT_CLK_MAIN_PLL_C4, div);
		alt_clk_divider_set(ALT_CLK_MAIN_PLL_C5, div);
    }
    status = alt_clk_pll_vco_freq_get(ALT_CLK_PERIPHERAL_PLL, &freq);
    if (status == ALT_E_SUCCESS)
    {
    	div = 512;
		// emac0_base_clk => base / 512
		// emac1_base_clk => base / 512
		// periph_qspi_base_clk => base / 512
		// h2f_user1_base_clk => base / 512
		alt_clk_divider_set(ALT_CLK_PERIPHERAL_PLL_C0, 512);
		alt_clk_divider_set(ALT_CLK_PERIPHERAL_PLL_C1, 512);
		alt_clk_divider_set(ALT_CLK_PERIPHERAL_PLL_C2, 512);
		alt_clk_divider_set(ALT_CLK_PERIPHERAL_PLL_C5, 512);
    }
    status = alt_clk_pll_vco_freq_get(ALT_CLK_SDRAM_PLL, &freq);
    if (status == ALT_E_SUCCESS)
    {
    	div = freq / osc1;
    	if(div > 512)
    	{
    		div = 512;
    	}
		// h2f_user2_clock
		alt_clk_divider_set(ALT_CLK_SDRAM_PLL_C5, div);
    }

	//NOTE: You cannot disable the USB or NAND clock because either would cause
	//       memory to fail
	//alt_clk_clock_disable(ALT_CLK_NAND);
	//alt_clk_clock_disable(ALT_CLK_NAND_X);
	alt_clk_clock_disable(ALT_CLK_EMAC0);
	alt_clk_clock_disable(ALT_CLK_EMAC1);
	alt_clk_clock_disable(ALT_CLK_CAN0);
	alt_clk_clock_disable(ALT_CLK_CAN1);
}
/*********************************************************************//**
 * @brief		Control clk LED
 *				R0：BZLED1 R1：BZLED2 R2：BZLED3 R3 : BZLED4 R4: BZLED5(未使用)
 * @param[in]	data
 * @return 		E_OK:succeeded
 * 				E_I2C_SEND or other :failure
 **********************************************************************/
void set_mpu_clock(alt_freq_t new_freq)
{
	ALT_STATUS_CODE status;
	uint32_t new_div;
	alt_freq_t pll_freq;

	// clear safe mode
	alt_clk_safe_mode_clear();

	// MPU clock source PLL
	status = alt_clk_pll_vco_freq_get(ALT_CLK_MAIN_PLL, &pll_freq);
    if (status == ALT_E_SUCCESS)
    {
		new_div = pll_freq / new_freq;

		status = alt_clk_divider_set(ALT_CLK_MAIN_PLL_C0, new_div);
    }
    if (status != ALT_E_SUCCESS)
    {
    	program_error();
    }
}

/*********************************************************************//**
 * @brief		Control clk LED
 *				R0：BZLED1 R1：BZLED2 R2：BZLED3 R3 : BZLED4 R4: BZLED5(未使用)
 * @param[in]	data
 * @return 		E_OK:succeeded
 * 				E_I2C_SEND or other :failure
 **********************************************************************/
void change_mpu_clock(MPU_CLOCK clock)
{
#if (_DEBUG_VARIABLE_CLOCK==1)
	u32 cur_freq;
	// clear safe mode
	alt_clk_safe_mode_clear();
	if(clock == MPU_CLOCK_LOW)
	{
		alt_clk_divider_set(ALT_CLK_MPU, _DEBUG_VARIABLE_LOW);
	}
	else if(clock == MPU_CLOCK_HIGH)
	{
		alt_clk_divider_set(ALT_CLK_MPU, _DEBUG_VARIABLE_HIGH);
	}
	else
	{
		// nothing todo
	}
#endif
}
/*********************************************************************//**
 * @brief		Control clk LED
 *				R0：BZLED1 R1：BZLED2 R2：BZLED3 R3 : BZLED4 R4: BZLED5(未使用)
 * @param[in]	data
 * @return 		E_OK:succeeded
 * 				E_I2C_SEND or other :failure
 **********************************************************************/
void set_main_pll_down(u32 div)
{
	u32 main_multi,main_div;
	u32 cur_multi,cur_div;
	u32 new_multi,new_div;
	//clock 800mhz to 400mhz
	// set freq
	alt_clk_pll_vco_cfg_get(ALT_CLK_MAIN_PLL,&main_multi,&main_div);
	cur_multi = main_multi;
	cur_div = main_div;
	while(main_multi / div < cur_multi)
	{
		new_div = cur_div;
		new_multi = cur_multi * 0.8;
		if(cur_multi - new_multi < 1)
		{
			//error
			program_error();
		}
		if(new_multi < main_multi/div)
		{
			new_multi = main_multi / div;
		}
		alt_clk_pll_vco_cfg_set(ALT_CLK_MAIN_PLL,new_multi,new_div);
		while(alt_clk_pll_is_locked(ALT_CLK_MAIN_PLL) != ALT_E_TRUE){};
		alt_clk_pll_vco_cfg_get(ALT_CLK_MAIN_PLL,&cur_multi,&cur_div);
	}
}
/*********************************************************************//**
 * @brief		Control clk LED
 *				R0：BZLED1 R1：BZLED2 R2：BZLED3 R3 : BZLED4 R4: BZLED5(未使用)
 * @param[in]	data
 * @return 		E_OK:succeeded
 * 				E_I2C_SEND or other :failure
 **********************************************************************/
void get_pll_info(void)
{
	u32 i;
	u32 main_multi,main_div;
	u32 peri_multi,peri_div;
	u32 sdram_multi,sdram_div;
	// get freq
	alt_clk_pll_vco_cfg_get(ALT_CLK_MAIN_PLL,&main_multi,&main_div);
	alt_clk_pll_vco_cfg_get(ALT_CLK_PERIPHERAL_PLL,&peri_multi,&peri_div);
	alt_clk_pll_vco_cfg_get(ALT_CLK_SDRAM_PLL,&sdram_multi,&sdram_div);
	for(i = (u32)ALT_CLK_MAIN_PLL; i <= (u32)ALT_CLK_H2F_USER2; i++)
	{
		clock_enabled[i] = alt_clk_is_enabled((ALT_CLK_t)i);
		alt_clk_freq_get((ALT_CLK_t)i, &clock_frequency[i]);
	}
	clock_frequency[ALT_CLK_OSC1] = alt_clk_ext_clk_freq_get(ALT_CLK_OSC1);
	for(i = 0; i <= 1; i++)
	{
		safe_enabled[i] = alt_clk_is_in_safe_mode((ALT_CLK_SAFE_DOMAIN_t)i);
	}
}
/* EOF */

