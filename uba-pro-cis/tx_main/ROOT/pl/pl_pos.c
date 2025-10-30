/*
 * pl_pos.c
 *
 *  Created on: 2019/04/22
 *      Author: yuji-kenta
 */
/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"

#include "common.h"
//#include "custom.h"

#include "pl_gpio.h"
#include "pl_motor.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

#include "pl_pos.h"

/*****************************************************************************/
/**
* Initialize da data.
*
* @param	None
*
* @return	alarm code pl spi dac.
*
* @note		None
*
******************************************************************************/
/*****************************************************************************/
/**
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_pos_init_da(void)
{
	u8 rtn = ALARM_CODE_OK;

	if(_pl_set_entrance_posi_da() != ALARM_CODE_OK)		/* DA8 入口 発光強度 */
		return ALARM_CODE_PL_SPI;
	if(_pl_set_centering_posi_da() != ALARM_CODE_OK)	/* DA9 幅寄せ 発光強度 */
		return ALARM_CODE_PL_SPI;
	if(_pl_set_apb_in_posi_da() != ALARM_CODE_OK)		/* DA1 PBIN 発光強度 */
		return ALARM_CODE_PL_SPI;
	if(_pl_set_apb_out_posi_da() != ALARM_CODE_OK)		/* DA2 PBOUT 発光強度 */
		return ALARM_CODE_PL_SPI;
	if(_pl_set_exit_posi_da() != ALARM_CODE_OK)			/* DA10 出口 発光強度 */
		return ALARM_CODE_PL_SPI;
	if(_pl_set_ent_threshold_posi_da() != ALARM_CODE_OK)	/* DA6 入口 検出閾値 */
		return ALARM_CODE_PL_SPI;
	if(_pl_set_ext_threshold_posi_da() != ALARM_CODE_OK)	/* DA7 幅寄せ/出口/PBIN/PBOUT 検出閾値, iVIZONとUBAで若干数が異なる */
		return ALARM_CODE_PL_SPI;
#if POINT_UV1_ENABLE	//FPGAへの設定なので、UBA700も設定しても問題ない
	if(_pl_set_uv_da() != ALARM_CODE_OK)	/* DA4  UV0 発光強度,   DA4  UV0(上反射),  DA3,UV1(下反射) */
		return ALARM_CODE_PL_SPI;
#endif
#if POINT_UV2_ENABLE	//FPGAへの設定なので、UBA700も設定しても問題ない
	if(_pl_set_uv1_da() != ALARM_CODE_OK)	/* DA4  UV0 発光強度,   DA4  UV0(上反射),  DA3,UV1(下反射) */
		return ALARM_CODE_PL_SPI;
#endif

	return rtn;
}

/*****************************************************************************/
/**
* re-send da data.
*
* @param	None
*
* @return	alarm code pl spi dac.
*
* @note		None
*
******************************************************************************/

/*****************************************************************************/
/**
* spi st ready.
*
* @param
* 			- none
*
* @return
*			- OK	: ALARM_CODE_OK
*			- NG	: ALARM_CODE_PL_SPI.
*
* @note		sspi waiting.
*
******************************************************************************/
u8 _pl_dac_wait_ready(void)
{
	u8 rtn = ALARM_CODE_OK;
	u32 cnt = 0;
	while(FPGA_REG.DAC_ST.BIT.BUSY)	// ビジー解除待ち
	{
		if(cnt++ > 1000000)
		{
			rtn = ALARM_CODE_PL_SPI;
			break;
		}
	}
	return rtn;
}

/*****************************************************************************/
/**
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
/*****************************************************************************/
/**
* Set entrance da.
*
* @param	None
*
* @return	alarm code pl spi.
*
* @note		None
*
******************************************************************************/
/*****************************************************************************/
/**
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_set_entrance_posi_da(void)
{
	FPGA_DAC_REG.DAC_DATA[DAC_INDEX_ENT_LED].LWORD = (u32)ex_position_da.entrance;

	return ALARM_CODE_OK;
}
/*****************************************************************************/
/**
* Set centering da.
*
* @param	None
*
* @return	alarm code pl spi.
*
* @note		None
*
******************************************************************************/
/*****************************************************************************/
/**
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_set_centering_posi_da(void)
{
	FPGA_DAC_REG.DAC_DATA[DAC_INDEX_CEN_LED].LWORD = (u32)ex_position_da.centering;

	return ALARM_CODE_OK;
}
/*****************************************************************************/
/**
* Set apb in da.
*
* @param	None
*
* @return	alarm code pl spi.
*
* @note		None
*
******************************************************************************/
/*****************************************************************************/
/**
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_set_apb_in_posi_da(void)
{
	FPGA_DAC_REG.DAC_DATA[DAC_INDEX_PBIN_LED].LWORD = (u32)ex_position_da.apb_in;

	return ALARM_CODE_OK;
}
/*****************************************************************************/
/**
* Set apb out da.
*
* @param	None
*
* @return	alarm code pl spi.
*
* @note		None
*
******************************************************************************/
/*****************************************************************************/
/**
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_set_apb_out_posi_da(void)
{
	FPGA_DAC_REG.DAC_DATA[DAC_INDEX_PBOUT_LED].LWORD = (u32)ex_position_da.apb_out;

	return ALARM_CODE_OK;
}
/*****************************************************************************/
/**
* Set exit da.
*
* @param	None
*
* @return	alarm code pl spi.
*
* @note		None
*
******************************************************************************/
/*****************************************************************************/
/**
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_set_exit_posi_da(void)
{
	FPGA_DAC_REG.DAC_DATA[DAC_INDEX_EXT_LED].LWORD = (u32)ex_position_da.exit;

	return ALARM_CODE_OK;
}
/*****************************************************************************/
/**
* Set threshold da.
*
* @param	None
*
* @return	alarm code pl spi.
*
* @note		None
*
******************************************************************************/

/*****************************************************************************/
/**
* Set entrance threshold da.
*
* @param	None
*
* @return	alarm code pl spi.
*
* @note		None
*
******************************************************************************/
/*****************************************************************************/
/**
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_set_ent_threshold_posi_da(void)
{
	FPGA_DAC_REG.DAC_DATA[DAC_INDEX_ENT_THR].LWORD = (u32)ex_position_da.ent_threshold;

	return ALARM_CODE_OK;
}

/*****************************************************************************/
/**
* Set entrance threshold da.
*
* @param	None
*
* @return	alarm code pl spi.
*
* @note		None
*
******************************************************************************/
/*****************************************************************************/
/**
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_set_ext_threshold_posi_da(void)
{
	FPGA_DAC_REG.DAC_DATA[DAC_INDEX_EXT_THR].LWORD = (u32)ex_position_da.ext_threshold;

	return ALARM_CODE_OK;
}
/*****************************************************************************/
/**
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_set_uv_da(void)
{
	FPGA_DAC_REG.DAC_DATA[DAC_INDEX_UV_LED].LWORD = (u32)ex_cis_adjustment_data.point_uv_adj.da[0];

	return ALARM_CODE_OK;
}

/*****************************************************************************/
/**
 * DAC
 * b15 b14 b13 b12 b11 b10 b9 b8 b7 b6 b5 b4 b3 b2 b1 b0
 *|   CH No.  |         8bit value       |    reserve   |
 *****************************************************************************/
u8 _pl_set_uv1_da(void)
{
	FPGA_DAC_REG.DAC_DATA[DAC_INDEX_UV1_LED].LWORD = (u32)ex_cis_adjustment_data.point_uv_adj.da[1];

	return ALARM_CODE_OK;
}

/*********************************************************************//**
 * @brief		All Sensor GAIN
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8 _pl_position_sensor_gain(u8 gain) // gain == 0が Low gain
{
	SNS_CTL_UNION sns_ctl;

	sns_ctl.LWORD = FPGA_REG.SNS_CTL.LWORD;
	sns_ctl.WORD.PSG = 0;

	if(gain & POSI_ENTRANCE)
	{
		sns_ctl.BIT.PSGD0 = 0;
	}
	else
	{
		sns_ctl.BIT.PSGD0 = 1;	/* Low gain */
	}
	if(gain & POSI_CENTERING)
	{
		sns_ctl.BIT.PSGD1 = 0;
	}
	else
	{
		sns_ctl.BIT.PSGD1 = 1;
	}
	if(gain & POSI_EXIT)
	{
		sns_ctl.BIT.PSGD2 = 0;
	}
	else
	{
		sns_ctl.BIT.PSGD2 = 1;
	}
	if(gain & POSI_APB_IN)
	{
		sns_ctl.BIT.PSGD3 = 0;
	}
	else
	{
		sns_ctl.BIT.PSGD3 = 1;
	}
	if(gain & POSI_APB_OUT)
	{
		sns_ctl.BIT.PSGD4 = 0;
	}
	else
	{
		sns_ctl.BIT.PSGD4 = 1;
	}
	if(gain & POSI_BOX_HOM)
	{
		sns_ctl.BIT.PSGD5 = 0;
	}
	else
	{
		sns_ctl.BIT.PSGD5 = 1;
	}

	FPGA_REG.SNS_CTL.LWORD = sns_ctl.LWORD;

	return ALARM_CODE_OK;
}
