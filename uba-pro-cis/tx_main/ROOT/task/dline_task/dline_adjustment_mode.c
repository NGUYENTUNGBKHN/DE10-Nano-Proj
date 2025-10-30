/******************************************************************************/
/*! @addtogroup Group1
    @file       dline_adjustment_mode.h
    @brief      dline adjustment mode function
    @date       2018/08/27
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan Cash Machine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/08/27 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
#include <string.h>
#include <stdio.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "custom.h"
#include "common.h"
#include "sensor.h"
#include "sensor_ad.h"
#include "struct.h"
#include "dline_test.h"
#include "dline_adjustment_mode.h"
#include "hal_spi_fram.h"
#include "pl/pl_cis.h"
#if DEBUG_ADJUSTMENT
#include "pl/pl.h"
//#include "pl_gpio.h"
#endif
#include "operation.h"
#include "sub_functions.h"

#define EXT
#include "../common/global.h"
#include "com_ram.c"
#include "usb_ram.c"
#include "cis_ram.c"

/************************** Variable declaration *****************************/
/************************** EXTERNAL VARIABLES *******************************/

u32 ex_send_offset;
/************************** Function Prototypes ******************************/

/************************** External functions *******************************/
extern void _dline_system_error(u8 fatal_err, u8 code);
extern void _dline_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
extern void set_response_1data(u8 cmd);

/******************************************************************************/
/*! @brief Set adjustment value to send buffer,
		   or get adjustment value from receive buffer.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
u8 sensor_data(void)//0x52
{
#if DEBUG_COMPERE_TIME
	u8 *p_data = NULL;
#endif
	u8 response = 0;

/*<<	初期流動のフェーズ確認		>>*/
	switch((u8)ex_front_usb.pc.mess.phase)	// 5バイト目のデータが入っている
	{
	/* get command */
	case PHASE_SENSOR_DATA_GET_PAPER_AD_MTF:	//0x01
		switch((u8)ex_front_usb.pc.mess.command)
		{
		case CMD_RUN:
			response = RES_ACK;
			ex_send_offset = 0;
			_main_set_pl_active(PL_ENABLE);

			_pl_cis_enable_set(1);

			/* DONE: check PL enable or not */
			if(get_pl_state())
			{
				ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_ONE_SHOT;
				_dline_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_ONE_SHOT, CIS_MODE_AD_PAPER, 0, 0);
			}
			else
			{
				response = RES_NG;
			}
			break;
		case CMD_ENQ:
			if(!ex_cis_adjustment_tbl.busy)
			{
				response = RES_DATA;
				_main_set_pl_active(PL_DISABLE);
				/* set data */
				ex_usb_write_size = sizeof(ROW_SENSOR_DATA);
				ex_usb_pbuf = (u8 *)&ex_row_data.tmp_10bit_ad_tbl[0];

				ex_send_offset = 0;
				ex_usb_buf_change = 1;
			}
			else
			{
				response = RES_BUSY;
			}
			break;
		default:
			response = RES_NAK;
			break;
		}
		break;
	case PHASE_SENSOR_DATA_GET_NON_PAPER_AD_MTF:	//0x02
		switch((u8)ex_front_usb.pc.mess.command)
		{
		case CMD_RUN:
			response = RES_ACK;
			ex_send_offset = 0;
			_main_set_pl_active(PL_ENABLE);

			_pl_cis_enable_set(1);

			/* DONE: check PL enable or not */
			if(get_pl_state())
			{
				ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_ONE_SHOT;
				_dline_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_ONE_SHOT, CIS_MODE_BC_NON_PAPER, 0, 0);
			}
			else
			{
				response = RES_NG;
			}
			break;
		case CMD_ENQ:
			if(!ex_cis_adjustment_tbl.busy)
			{
				response = RES_DATA;
				_main_set_pl_active(PL_DISABLE);
				/* set data */
				ex_usb_write_size = sizeof(ROW_SENSOR_DATA);
				ex_usb_pbuf = (u8 *)&ex_row_data.tmp_10bit_ad_tbl[0];

				ex_send_offset = 0;
				ex_usb_buf_change = 1;
			}
			else
			{
				response = RES_BUSY;
			}
			break;
		default:
			response = RES_NAK;
			break;
		}
		break;
	case PHASE_SENSOR_DATA_GET_PAPER_WCAD_MTF:	//0x03
		switch((u8)ex_front_usb.pc.mess.command)
		{
		case CMD_RUN:
			response = RES_ACK;
			ex_send_offset = 0;
			_main_set_pl_active(PL_ENABLE);

			_pl_cis_enable_set(1);

			/* DONE: check PL enable or not */
			if(get_pl_state())
			{
				ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_ONE_SHOT;
				_dline_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_ONE_SHOT, CIS_MODE_WC_PAPER, 0, 0);
			}
			else
			{
				response = RES_NG;
			}
			break;
		case CMD_ENQ:
			if(!ex_cis_adjustment_tbl.busy)
			{
				response = RES_DATA;
				_main_set_pl_active(PL_DISABLE);
				/* set data */
				ex_usb_write_size = sizeof(ADJ_SENSOR_DATA);
				ex_usb_pbuf = (u8 *)&ex_adj_data.tmp_ad_tbl[0];

				ex_send_offset = 0;
				ex_usb_buf_change = 1;
			}
			else
			{
				response = RES_BUSY;
			}
			break;
		default:
			response = RES_NAK;
			break;
		}
		break;
	case PHASE_SENSOR_DATA_GET_NON_PAPER_TMPAD_MTF:
		switch((u8)ex_front_usb.pc.mess.command)
		{
		case CMD_RUN:
			response = RES_ACK;
			ex_send_offset = 0;
			_main_set_pl_active(PL_ENABLE);

			_pl_cis_enable_set(1);

			/* DONE: check PL enable or not */
			if(get_pl_state())
			{
				ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_ONE_SHOT;
				_dline_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_ONE_SHOT, CIS_MODE_TMP_NON_PAPER, 0, 0);
			}
			else
			{
				response = RES_NG;
			}
			break;
		case CMD_ENQ:
			if(!ex_cis_adjustment_tbl.busy)
			{
				response = RES_DATA;
				_main_set_pl_active(PL_DISABLE);
				/* set data */
				ex_usb_write_size = sizeof(ROW_SENSOR_DATA);
				ex_usb_pbuf = (u8 *)&ex_row_data.tmp_10bit_ad_tbl[0];

				ex_send_offset = 0;
				ex_usb_buf_change = 1;
			}
			else
			{
				response = RES_BUSY;
			}
			break;
		default:
			response = RES_NAK;
			break;
		}
		break;
#if DEBUG_COMPERE_TIME
	case 0xFF:
		response = RES_DATA;
		/* set data */
		ex_usb_write_size = 6 + sizeof(DO_TIME);	// set length

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8*)&do_time;	// set send data point
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(DO_TIME));
		break;
#endif
	default:
		response = RES_NAK;
		break;
	}
	return response;
}
/******************************************************************************/
/*! @brief Set adjustment value to send buffer,
		   or get adjustment value from receive buffer.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
u8 adjustment_value(u8 type)//0x54
{
	u8 response = 0;
	u32 cnt = 0;
	u8 *p_data = NULL;
	u8 ad_value;

/*<<	調整モードのフェーズ確認		>>*/
	switch((u8)ex_front_usb.pc.mess.phase)	// 5バイト目のデータが入っている
	{
	/* get command */
	case PHASE_ADJUSTMENT_GET_ADJ_PAPER:	//0x11
		response = RES_NAK;

		break;
	case PHASE_ADJUSTMENT_GET_ADJ_NON_PAPER:	// 0x12
		response = RES_NAK;
		break;
	case PHASE_ADJUSTMENT_GET_DA:	// 0x13
		response = RES_DATA;
		/* set data */
		ex_usb_write_size = 6 + sizeof(CIS_ADJUSTMENT_DA);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_cis_adjustment_data.cis_da;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(CIS_ADJUSTMENT_DA));

		break;
	case PHASE_ADJUSTMENT_GET_TIME:	// 0x14
		response = RES_DATA;
		/* set data */
		ex_usb_write_size = 6 + sizeof(CIS_ADJUSTMENT_TIME);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_cis_adjustment_data.cis_time;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(CIS_ADJUSTMENT_TIME));

		break;
	case PHASE_ADJUSTMENT_GET_AGAIN:
		response = RES_DATA;
		/* set data */
		ex_usb_write_size = 6 + sizeof(AFE_ADJUSTMENT_AGAIN);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_cis_adjustment_data.afe_again;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(AFE_ADJUSTMENT_AGAIN));

		break;
	case PHASE_ADJUSTMENT_GET_DGAIN:
		response = RES_DATA;
		/* set data */
		ex_usb_write_size = 6 + sizeof(AFE_ADJUSTMENT_DGAIN);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_cis_adjustment_data.afe_dgain;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(AFE_ADJUSTMENT_DGAIN));

		break;
	case PHASE_ADJUSTMENT_GET_OFFSET:
		response = RES_DATA;
		/* set data */
		ex_usb_write_size = 6 + sizeof(AFE_ADJUSTMENT_AOFFSET);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_cis_adjustment_data.afe_aoffset;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(AFE_ADJUSTMENT_AOFFSET));

		break;
	case PHASE_ADJUSTMENT_GET_BC:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(BC_DATA);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		ex_usb_pbuf = (u8 *)&ex_cis_adjustment_data.cis_bc;
		ex_usb_buf_change = 1;
		break;
	case PHASE_ADJUSTMENT_GET_WC:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(WC_DATA);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		ex_usb_pbuf = (u8 *)&ex_cis_adjustment_data.cis_wc;
		ex_usb_buf_change = 1;
		break;
	case PHASE_ADJUSTMENT_GET_PGA:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(CIS_ADJUSTMENT_PGA);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_cis_adjustment_tmp.cis_pga.red_ref_pga_u;
		for(cnt = 0; cnt < sizeof(CIS_ADJUSTMENT_PGA); cnt++)
		{
			*(ex_usb_write_buffer + 6 + cnt) = *(p_data + cnt);
		}
		break;
	case PHASE_ADJUSTMENT_GET_TMP_PIXEL:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(CIS_ADJUSTMENT_ERIA);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_cis_adjustment_tmp.cis_tmp_eria;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(CIS_ADJUSTMENT_ERIA));
		break;
	case PHASE_ADJUSTMENT_GET_TMP_AD:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(CIS_ADJUSTMENT_AD);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_cis_adjustment_tmp.cis_tmp_ad;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(CIS_ADJUSTMENT_AD));
		break;
	case PHASE_ADJUSTMENT_GET_TMP_DA:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(CIS_ADJUSTMENT_DA);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_cis_adjustment_tmp.cis_tmp_da;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(CIS_ADJUSTMENT_DA));
		break;
	case PHASE_ADJUSTMENT_GET_TMP_TIME:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(CIS_ADJUSTMENT_TIME);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_cis_adjustment_tmp.cis_tmp_time;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(CIS_ADJUSTMENT_TIME));
		break;
	case PHASE_ADJUSTMENT_GET_SLED:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(CIS_ADJUSTMENT_DA);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_cis_adjustment_data.cis_sled;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(CIS_ADJUSTMENT_DA));
		break;
	case PHASE_ADJUSTMENT_GET_TMP_SLED:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(CIS_ADJUSTMENT_DA);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_cis_adjustment_tmp.cis_tmp_sled;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(CIS_ADJUSTMENT_DA));
		break;
	case PHASE_ADJUSTMENT_GET_POS_AD:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(u16);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_position_sensor;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(u16));
		break;
	case PHASE_ADJUSTMENT_GET_POS_DA:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(POSITION_SENSOR);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_position_da;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(POSITION_SENSOR));
		break;
	case PHASE_ADJUSTMENT_GET_POS_ADJ_DA:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(POSITION_SENSOR);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_position_da_adj;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(POSITION_SENSOR));
		break;
	case PHASE_ADJUSTMENT_GET_POS_GAIN:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + 1;

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;
		*(ex_usb_write_buffer + 6) = ex_position_ga;
		break;

	case PHASE_ADJUSTMENT_GET_POINT_UV_GAIN:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(ex_cis_adjustment_data.point_uv_adj.gain);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)ex_cis_adjustment_data.point_uv_adj.gain;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(ex_cis_adjustment_data.point_uv_adj.gain));
		break;
	case PHASE_ADJUSTMENT_GET_POINT_UV_DA:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(ex_cis_adjustment_data.point_uv_adj.da);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)ex_cis_adjustment_data.point_uv_adj.da;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(ex_cis_adjustment_data.point_uv_adj.da));
		break;
	case PHASE_ADJUSTMENT_GET_POINT_UV_OFFSET:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(ex_cis_adjustment_data.point_uv_adj.offset);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)ex_cis_adjustment_data.point_uv_adj.offset;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(ex_cis_adjustment_data.point_uv_adj.offset));
		break;

	case PHASE_ADJUSTMENT_GET_MAG_GAIN:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(u16)*2;

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_adjustment_data.mag_adj_value.ul_gain;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(u16)*2);
		break;
	case PHASE_ADJUSTMENT_GET_MAG_ADJ_MAX:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(u16)*2;

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_adjustment_data.mag_adj_value.ul_adj_max;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(u16)*2);
		break;
	case PHASE_ADJUSTMENT_GET_MAG_ADJ_TMP:
		response = RES_DATA;

		/* set data */
		ex_usb_write_size = 6 + sizeof(MAG_SENSOR_VAL);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_mag_adj;
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(MAG_SENSOR_VAL));
		break;
		/* set command */
	case PHASE_ADJUSTMENT_SET_ADJ_PAPER:		// 0x91	not use
	case PHASE_ADJUSTMENT_SET_ADJ_NON_PAPER:	// 0x92	not use

		response = RES_NAK;
		break;
	case PHASE_ADJUSTMENT_SET_DA:			// 0x93	use
		memcpy(&ex_cis_adjustment_data.cis_da, &ex_usb_read_buffer[6], sizeof(CIS_ADJUSTMENT_DA));

		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_TIME:			// 0x94	use
		memcpy(&ex_cis_adjustment_data.cis_time, &ex_usb_read_buffer[6], sizeof(CIS_ADJUSTMENT_TIME));

		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_AGAIN:
		memcpy(&ex_cis_adjustment_data.afe_again, &ex_usb_read_buffer[6], sizeof(AFE_ADJUSTMENT_AGAIN));

		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_DGAIN:
		memcpy(&ex_cis_adjustment_data.afe_dgain, &ex_usb_read_buffer[6], sizeof(AFE_ADJUSTMENT_DGAIN));

		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_OFFSET:
		memcpy(&ex_cis_adjustment_data.afe_aoffset, &ex_usb_read_buffer[6], sizeof(AFE_ADJUSTMENT_AOFFSET));

		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_BC:
		response = RES_ACK;
		switch((u8)ex_usb_read_buffer[6])
		{
		case BLACK_NO_1:
			memcpy(&ex_cis_adjustment_data.cis_bc.black_data1_u[0], &ex_usb_read_buffer[7], MAIN_SCAN_LINE * sizeof(u16));
			memcpy(&ex_cis_adjustment_data.cis_bc.black_data1_d[0], &ex_usb_read_buffer[MAIN_SCAN_LINE * sizeof(u16) + 7], MAIN_SCAN_LINE * sizeof(u16));
			break;
		case BLACK_NO_2:
			memcpy(&ex_cis_adjustment_data.cis_bc.black_data2_u[0], &ex_usb_read_buffer[7], MAIN_SCAN_LINE * sizeof(u16));
			memcpy(&ex_cis_adjustment_data.cis_bc.black_data2_d[0], &ex_usb_read_buffer[MAIN_SCAN_LINE * sizeof(u16) + 7], MAIN_SCAN_LINE * sizeof(u16));
			break;
		default:
			response = RES_NAK;
			break;
		}
		break;
	case PHASE_ADJUSTMENT_SET_WC:
		response = RES_ACK;

		switch((u8)ex_usb_read_buffer[6])
		{
		case SEN_NO_RLS_RED:
			memcpy(&ex_cis_adjustment_data.cis_wc.red_ref_u[0], &ex_usb_read_buffer[7], MAIN_SCAN_LINE * sizeof(u16));
			memcpy(&ex_cis_adjustment_data.cis_wc.red_ref_d[0], &ex_usb_read_buffer[MAIN_SCAN_LINE * sizeof(u16) + 7], MAIN_SCAN_LINE * sizeof(u16));
			break;
		case SEN_NO_RLS_GRE:
			memcpy(&ex_cis_adjustment_data.cis_wc.gre_ref_u[0], &ex_usb_read_buffer[7], MAIN_SCAN_LINE * sizeof(u16));
			memcpy(&ex_cis_adjustment_data.cis_wc.gre_ref_d[0], &ex_usb_read_buffer[MAIN_SCAN_LINE * sizeof(u16) + 7], MAIN_SCAN_LINE * sizeof(u16));
			break;
		case SEN_NO_RLS_BLU:
			memcpy(&ex_cis_adjustment_data.cis_wc.blu_ref_u[0], &ex_usb_read_buffer[7], MAIN_SCAN_LINE * sizeof(u16));
			memcpy(&ex_cis_adjustment_data.cis_wc.blu_ref_d[0], &ex_usb_read_buffer[MAIN_SCAN_LINE * sizeof(u16) + 7], MAIN_SCAN_LINE * sizeof(u16));
			break;
		case SEN_NO_RLS_IR1:
			memcpy(&ex_cis_adjustment_data.cis_wc.ir1_ref_u[0], &ex_usb_read_buffer[7], MAIN_SCAN_LINE * sizeof(u16));
			memcpy(&ex_cis_adjustment_data.cis_wc.ir1_ref_d[0], &ex_usb_read_buffer[MAIN_SCAN_LINE * sizeof(u16) + 7], MAIN_SCAN_LINE * sizeof(u16));
			break;
		case SEN_NO_RLS_IR2:
			memcpy(&ex_cis_adjustment_data.cis_wc.ir2_ref_u[0], &ex_usb_read_buffer[7], MAIN_SCAN_LINE * sizeof(u16));
			memcpy(&ex_cis_adjustment_data.cis_wc.ir2_ref_d[0], &ex_usb_read_buffer[MAIN_SCAN_LINE * sizeof(u16) + 7], MAIN_SCAN_LINE * sizeof(u16));
			break;
		case SEN_NO_RLS_FL:
			memcpy(&ex_cis_adjustment_data.cis_wc.fl_ref_u[0], &ex_usb_read_buffer[7], MAIN_SCAN_LINE * sizeof(u16));
			memcpy(&ex_cis_adjustment_data.cis_wc.fl_ref_d[0], &ex_usb_read_buffer[MAIN_SCAN_LINE * sizeof(u16) + 7], MAIN_SCAN_LINE * sizeof(u16));
			break;
		case SEN_NO_TLS_RED:
			memcpy(&ex_cis_adjustment_data.cis_wc.c6_led_u[0], &ex_usb_read_buffer[7], MAIN_SCAN_LINE * sizeof(u16));
			memcpy(&ex_cis_adjustment_data.cis_wc.red_pen_d[0], &ex_usb_read_buffer[MAIN_SCAN_LINE * sizeof(u16) + 7], MAIN_SCAN_LINE * sizeof(u16));
			break;
		case SEN_NO_TLS_GRE:
			memcpy(&ex_cis_adjustment_data.cis_wc.c7_led_u[0], &ex_usb_read_buffer[7], MAIN_SCAN_LINE * sizeof(u16));
			memcpy(&ex_cis_adjustment_data.cis_wc.gre_pen_d[0], &ex_usb_read_buffer[MAIN_SCAN_LINE * sizeof(u16) + 7], MAIN_SCAN_LINE * sizeof(u16));
			break;
		case SEN_NO_TLS_IR1:
			memcpy(&ex_cis_adjustment_data.cis_wc.c8_led_u[0], &ex_usb_read_buffer[7], MAIN_SCAN_LINE * sizeof(u16));
			memcpy(&ex_cis_adjustment_data.cis_wc.ir1_pen_d[0], &ex_usb_read_buffer[MAIN_SCAN_LINE * sizeof(u16) + 7], MAIN_SCAN_LINE * sizeof(u16));
			break;
		case SEN_NO_TLS_IR2:
			memcpy(&ex_cis_adjustment_data.cis_wc.c9_led_u[0], &ex_usb_read_buffer[7], MAIN_SCAN_LINE * sizeof(u16));
			memcpy(&ex_cis_adjustment_data.cis_wc.ir2_pen_d[0], &ex_usb_read_buffer[MAIN_SCAN_LINE * sizeof(u16) + 7], MAIN_SCAN_LINE * sizeof(u16));
			break;
		case SEN_NO_BLK:
		default:
			response = RES_NAK;
			break;
		}
		break;

	case PHASE_ADJUSTMENT_SET_PGA:
		memcpy(&ex_cis_adjustment_tmp.cis_pga, &ex_usb_read_buffer[6], sizeof(CIS_ADJUSTMENT_PGA));

		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_TMP_PIXEL:
		memcpy(&ex_cis_adjustment_tmp.cis_tmp_eria, &ex_usb_read_buffer[6], sizeof(CIS_ADJUSTMENT_ERIA));
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_TMP_AD:
		memcpy(&ex_cis_adjustment_tmp.cis_tmp_ad, &ex_usb_read_buffer[6], sizeof(CIS_ADJUSTMENT_AD));
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_TMP_DA:
		memcpy(&ex_cis_adjustment_tmp.cis_tmp_da, &ex_usb_read_buffer[6], sizeof(CIS_ADJUSTMENT_DA));
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_TMP_TIME:
		memcpy(&ex_cis_adjustment_tmp.cis_tmp_time, &ex_usb_read_buffer[6], sizeof(CIS_ADJUSTMENT_TIME));
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_SLED:
		memcpy(&ex_cis_adjustment_data.cis_sled, &ex_usb_read_buffer[6], sizeof(CIS_ADJUSTMENT_DA));
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_TMP_SLED:
		memcpy(&ex_cis_adjustment_tmp.cis_tmp_sled, &ex_usb_read_buffer[6], sizeof(CIS_ADJUSTMENT_DA));
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_POS_DA:
		memcpy(&ex_position_da, &ex_usb_read_buffer[6], sizeof(POSITION_SENSOR));
		set_position_da();
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_POS_ADJ_DA:
		memcpy(&ex_position_da_adj, &ex_usb_read_buffer[6], sizeof(POSITION_SENSOR));
		memcpy(&ex_position_da, &ex_usb_read_buffer[6], sizeof(POSITION_SENSOR));

		set_position_da();
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_POS_GAIN:
		ex_position_ga = ex_usb_read_buffer[6] ;
		if (0 != set_position_ga())
		{
			/* system error */
			_dline_system_error(1, 6);
		}
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_POINT_UV_GAIN:
		memcpy(ex_cis_adjustment_data.point_uv_adj.gain, &ex_usb_read_buffer[6], sizeof(ex_cis_adjustment_data.point_uv_adj.gain));
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_POINT_UV_DA:
		memcpy(ex_cis_adjustment_data.point_uv_adj.da, &ex_usb_read_buffer[6], sizeof(ex_cis_adjustment_data.point_uv_adj.da));
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_POINT_UV_OFFSET:
		memcpy(ex_cis_adjustment_data.point_uv_adj.offset, &ex_usb_read_buffer[6], sizeof(ex_cis_adjustment_data.point_uv_adj.offset));
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_MAG_GAIN:
		memcpy(&ex_adjustment_data.mag_adj_value.ul_gain, &ex_usb_read_buffer[6], sizeof(u16)*2);
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_MAG_ADJ_MAX:
		memcpy(&ex_adjustment_data.mag_adj_value.ul_adj_max, &ex_usb_read_buffer[6], sizeof(u16)*2);
		response = RES_ACK;
		break;
	case PHASE_ADJUSTMENT_SET_MAG_ADJ_TMP:
		memcpy(&ex_mag_adj, &ex_usb_read_buffer[6], sizeof(MAG_SENSOR_VAL));
		response = RES_ACK;
		break;
	default:
		response = RES_NAK;
		break;
	}

	return response;
}
/******************************************************************************/
/*! @brief Read or Write adjustment value from PC to EEPROM.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
u8 adjustment_eeprom_rw(void)	// 0x53
{
	/* Read & Write EEPROM */
	u8 response = 0;
	u8 *p_data = NULL;
	u16 sum;
/*<<	調整モードのフェーズ確認		>>*/
	switch((u8)ex_front_usb.pc.mess.phase)
	{
	case PHASE_ADJUSTMENT_GET_DATA_SERIAL:
		response = RES_DATA;

		ex_adjustment_fram_busy |= ADJ_FRAM_BUSY;
		_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_READ_REQ, FRAM_ADJ_DS, 0, 0, 0);//初期流動番号

		/* set data */
		ex_usb_write_size = 6 + sizeof(DATA_SERIAL);

		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = response;

		p_data = (u8 *)&ex_adjustment_data.data_serial; //初期流動番号
		memcpy(ex_usb_write_buffer + 6, p_data, sizeof(DATA_SERIAL));
		break;
	case PHASE_ADJUSTMENT_SET_DATA_SERIAL:
		if(is_test_mode())
		{
			memcpy(&ex_adjustment_data.data_serial, &ex_usb_read_buffer[6], sizeof(DATA_SERIAL)); //初期流動番号
			ex_adjustment_fram_busy |= ADJ_FRAM_BUSY;
			_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ADJ_DS, 0, 0, 0);//初期流動番号
			// send response
			set_response_1data(RES_ACK);
		}
		else
		{
			// send response
			set_response_1data(RES_NAK);
		}
		break;
	case PHASE_ADJUSTMENT_WRITE_FRAM_INFOMATION:
		switch(ex_front_usb.pc.mess.command)
		{
		case CMD_FACTRY:
			if(is_test_mode())
			{
				/* set data */
				ex_adjustment_fram_busy |= ADJ_FRAM_BUSY;
				memcpy(&ex_adjustment_data.factory_info, ex_usb_read_buffer + FUSB_HEADER_SIZE,sizeof(ADJUSTMENT_INFO));
				memcpy(&ex_adjustment_data.maintenance_info, ex_usb_read_buffer + FUSB_HEADER_SIZE,sizeof(ADJUSTMENT_INFO));

				_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ADJ_INF, FRAM_SETTING_CLEAR, 0, 0);
				// send response
				set_response_1data(RES_ACK);
// move to fram_task
//#ifdef _ENABLE_JDL
//				jdl_init(1);
//#endif	/* _ENABLE_JDL */
			}
			else
			{
				// send response
				set_response_1data(RES_NAK);
			}
			break;
		case CMD_MAINTENANCE:
			if(is_test_mode())
			{
				/* set data */
				ex_adjustment_fram_busy |= ADJ_FRAM_BUSY;
				memcpy(&ex_adjustment_data.maintenance_info, ex_usb_read_buffer + FUSB_HEADER_SIZE,sizeof(ADJUSTMENT_INFO));

				_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_ADJ_INF, FRAM_SETTING_KEEP, 0, 0);
				// send response
				set_response_1data(RES_ACK);
			}
			else
			{
				// send response
				set_response_1data(RES_NAK);
			}
			break;
		case CMD_READ_FACTRY:
			/* set data */
			ex_usb_write_size = sizeof(ADJUSTMENT_INFO) + 6;
			*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
			*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
			*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
			*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
			*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
			*(ex_usb_write_buffer + 5) = RES_DATA;
			memcpy(ex_usb_write_buffer + 6, &ex_adjustment_data.factory_info, sizeof(ADJUSTMENT_INFO));
			break;
		case CMD_READ_MAINTENANCE:
			/* set data */
			ex_usb_write_size = sizeof(ADJUSTMENT_INFO) + 6;
			*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
			*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
			*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
			*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
			*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
			*(ex_usb_write_buffer + 5) = RES_DATA;
			memcpy(ex_usb_write_buffer + 6, &ex_adjustment_data.maintenance_info, sizeof(ADJUSTMENT_INFO));
			break;
		case CMD_ENQ:
			// send response
			if(ex_adjustment_fram_busy & ADJ_FRAM_BUSY)
			{
				set_response_1data(RES_BUSY);
			}
			else
			{
				set_response_1data(RES_OK);
			}
			break;
		default:
			break;
		}
		break;
	case PHASE_ADJUSTMENT_WRITE_FRAM_POSITION_VALUE:
		switch(ex_front_usb.pc.mess.command)
		{
		case CMD_FACTRY:
			if((is_test_mode())
			 &&(ex_front_usb.pc.mess.length == 6 + FRAM_ADJ_POS_DA_SIZE + FRAM_ADJ_POS_GAIN_SIZE))
			{
				/* set data */
				ex_adjustment_fram_busy |= ADJ_FRAM_BUSY;
				memset(&ex_adjustment_data.factory_value, 0, sizeof(ADJUSTMENT_VALUE));
				memset(&ex_adjustment_data.maintenance_value, 0, sizeof(ADJUSTMENT_VALUE));
				memcpy(&ex_adjustment_data.factory_value, ex_usb_read_buffer + FUSB_HEADER_SIZE,FRAM_ADJ_POS_DA_SIZE + FRAM_ADJ_POS_GAIN_SIZE);
				memcpy(&ex_adjustment_data.maintenance_value, ex_usb_read_buffer + FUSB_HEADER_SIZE,FRAM_ADJ_POS_DA_SIZE + FRAM_ADJ_POS_GAIN_SIZE);

				memcpy(&ex_position_tmp.tmp_entrance, &ex_usb_read_buffer[FUSB_HEADER_SIZE], FRAM_ADJ_POS_DA_SIZE + FRAM_ADJ_POS_GAIN_SIZE);
				sum = culc_fram_postmp_sum();
				memcpy(&ex_position_tmp.tmp_sum, &sum, FRAM_ADJ_TMP_POS_SUM_SIZE);
				memcpy(&ex_position_tmp_bk.tmp_entrance, &ex_position_tmp.tmp_entrance, sizeof(POS_ADJUSTMENT_TMP));

				_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_POS, 0, 0, 0);
				// send response
				set_response_1data(RES_ACK);
			}
			else
			{
				// send response
				set_response_1data(RES_NAK);
			}
			break;
		case CMD_MAINTENANCE:
			if((is_test_mode())
			 && (ex_front_usb.pc.mess.length == 6 + FRAM_ADJ_POS_DA_SIZE + FRAM_ADJ_POS_GAIN_SIZE))
			{
			/* set data */
				ex_adjustment_fram_busy |= ADJ_FRAM_BUSY;
				memset(&ex_adjustment_data.maintenance_value, 0, sizeof(ADJUSTMENT_VALUE));
				memcpy(&ex_adjustment_data.maintenance_value, ex_usb_read_buffer + FUSB_HEADER_SIZE,FRAM_ADJ_POS_DA_SIZE + FRAM_ADJ_POS_GAIN_SIZE);

				memcpy(&ex_position_tmp.tmp_entrance, &ex_usb_read_buffer[FUSB_HEADER_SIZE], FRAM_ADJ_POS_DA_SIZE + FRAM_ADJ_POS_GAIN_SIZE);
				sum = culc_fram_postmp_sum();
				memcpy(&ex_position_tmp.tmp_sum, &sum, FRAM_ADJ_TMP_POS_SUM_SIZE);
				memcpy(&ex_position_tmp_bk.tmp_entrance, &ex_position_tmp.tmp_entrance, sizeof(POS_ADJUSTMENT_TMP));

				_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_POS, 0, 0, 0);
				// send response
				set_response_1data(RES_ACK);
			}
			else
			{
				// send response
				set_response_1data(RES_NAK);
			}
			break;
		case CMD_READ_FACTRY:
			/* set data */
			ex_usb_write_size = (FRAM_ADJ_POS_DA_SIZE + FRAM_ADJ_POS_GAIN_SIZE) + 6;
			*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
			*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
			*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
			*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
			*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
			*(ex_usb_write_buffer + 5) = RES_DATA;
			memcpy(ex_usb_write_buffer + 6, &ex_adjustment_data.factory_value, (FRAM_ADJ_POS_DA_SIZE + FRAM_ADJ_POS_GAIN_SIZE));
			break;
		case CMD_READ_MAINTENANCE:
			/* set data */
			ex_usb_write_size = (FRAM_ADJ_POS_DA_SIZE + FRAM_ADJ_POS_GAIN_SIZE) + 6;
			*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
			*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
			*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
			*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
			*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
			*(ex_usb_write_buffer + 5) = RES_DATA;
			memcpy(ex_usb_write_buffer + 6, &ex_adjustment_data.maintenance_value, (FRAM_ADJ_POS_DA_SIZE + FRAM_ADJ_POS_GAIN_SIZE));
			break;
		case CMD_ENQ:
			// send response
			if(ex_adjustment_fram_busy & ADJ_FRAM_BUSY)
			{
				set_response_1data(RES_BUSY);
			}
			else
			{
				/*ここで構造体データ送る read write共通*/
				set_response_1data(RES_OK);
			}
			break;
		default:
			break;
		}
		break;
	case PHASE_ADJUSTMENT_WRITE_FRAM_ADJUSTMENT_VALUE:
		switch(ex_front_usb.pc.mess.command)
		{
		case CMD_WRITE:
			if(is_test_mode())
			{
				// send response
				set_response_1data(RES_ACK);
				ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_EEP_ACCCESS;
				_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_CIS_ADJ, 0, 0, 0);
			}
			else
			{
				// send response
				set_response_1data(RES_NAK);
			}
			break;
		case CMD_READ:
			// send response
			set_response_1data(RES_ACK);
			ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_EEP_ACCCESS;
			_dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_READ_REQ, FRAM_CIS_ADJ, 0, 0, 0);
			break;
		case CMD_ENQ:
			// send response
			if(ex_cis_adjustment_tbl.busy)
			{
				set_response_1data(RES_BUSY);
			}
			else
			{
				/*read write共通*/
				set_response_1data(RES_OK);
			}
			break;
		default:
			break;
		}
		break;
	case PHASE_ADJUSTMENT_GET_FRAM_SUM:
		/* set data */
		ex_usb_write_size = sizeof(ex_fram_sum) + 6;
		*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
		*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
		*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
		*(ex_usb_write_buffer + 5) = RES_DATA;
		memcpy(ex_usb_write_buffer + 6, &ex_fram_sum, sizeof(ex_fram_sum));
		break;
	default:
		response = NAK;
		break;
	}
	return response;
}
/******************************************************************************/
/*! @brief Start sensor sampling, or set sampling value to send buffer.
    @par            Refer
    - 参照するグローバル変数 ex_front_usb
    @return         none
    @exception      none
******************************************************************************/
u8 adjustment_sampling(void)	// 0x55
{
	u8 response = 0;
	u32 cnt = 0;
	u8* p_data;

/*<<	調整モードのフェーズ確認		>>*/
	switch((u8)ex_front_usb.pc.mess.phase)
	{
	case PHASE_ADJUSTMENT_SAMPLING_ADJ_PAPER:	//0x55	0x01
		switch((u8)ex_front_usb.pc.mess.command)
		{
		case CMD_RUN:	// 0x55 0x01 0x01 紙有りA/D値サンプリング開始コマンド
			response = RES_ACK;
			ex_send_offset = 0;
			_main_set_pl_active(PL_ENABLE);

			_pl_cis_enable_set(1);

			/* DONE: check PL enable or not */
			if(get_pl_state())
			{
				ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_ONE_SHOT;
				_dline_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_ONE_SHOT, CIS_MODE_WC_PAPER, 0, 0);
			}
			else
			{
				response = RES_NG;
			}
			break;
		case CMD_ENQ:	// 0x55 0x01 0x05  紙有りA/D値取得コマンド
			if(!ex_cis_adjustment_tbl.busy)
			{
				response = RES_DATA;
				_main_set_pl_active(PL_DISABLE);
				/* set data */
				ex_usb_write_size = sizeof(TMP_AD_TBL) + 6;

				*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
				*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
				*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
				*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
				*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
				*(ex_usb_write_buffer + 5) = response;

				p_data = (u8 *)&ex_tmp_ad_data;
				for(cnt = 0; cnt < sizeof(TMP_AD_TBL); cnt++)
				{
					*(ex_usb_write_buffer + 6 + cnt) = *(p_data + cnt);
				}
			}
			else
			{
				response = RES_BUSY;
			}
			break;
		default:
			response = RES_NAK;
			break;
		}
		break;
	case PHASE_ADJUSTMENT_SAMPLING_ADJ_NON_PAPER:	//0x55	0x02
		switch((u8)ex_front_usb.pc.mess.command)
		{
		case CMD_RUN:	//0x55	0x02 0x01
			response = RES_ACK;
			ex_send_offset = 0;
			_main_set_pl_active(PL_ENABLE);

			_pl_cis_enable_set(1);

			/* DONE: check PL enable or not */
			if(get_pl_state())
			{
				ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_ONE_SHOT;
				_dline_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_ONE_SHOT, CIS_MODE_WC_NON_PAPER, 0, 0);
			}
			else
			{
				response = RES_NG;
			}
			break;
		case CMD_ENQ:	//0x55	0x02 0x05
			if(!ex_cis_adjustment_tbl.busy)
			{
				response = RES_DATA;
				_main_set_pl_active(PL_DISABLE);
				/* set data */
				ex_usb_write_size = sizeof(TMP_AD_TBL) + 6;

				*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
				*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
				*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
				*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
				*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
				*(ex_usb_write_buffer + 5) = response;

				p_data = (u8 *)&ex_tmp_ad_data;
				for(cnt = 0; cnt < sizeof(TMP_AD_TBL); cnt++)
				{
					*(ex_usb_write_buffer + 6 + cnt) = *(p_data + cnt);
				}
			}
			else
			{
				response = RES_BUSY;
			}
			break;
		default:
			response = RES_NAK;
			break;
		}
		break;

	case PHASE_ADJUSTMENT_SAMPLING_TEMPERATURE:		//0x55	0x06
		response = RES_NAK;
		break;

	case PHASE_ADJUSTMENT_SAMPLING_AD_PAPER:		//0x55	0x07
		switch((u8)ex_front_usb.pc.mess.command)
		{
		case CMD_RUN:
			response = RES_ACK;
			ex_send_offset = 0;
			_main_set_pl_active(PL_ENABLE);

			_pl_cis_enable_set(1);

			/* DONE: check PL enable or not */
			if(get_pl_state())
			{
				ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_ONE_SHOT;
				_dline_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_ONE_SHOT, CIS_MODE_AD_PAPER, 0, 0);
			}
			else
			{
				response = RES_NG;
			}
			break;
		case CMD_ENQ:
			if(!ex_cis_adjustment_tbl.busy)
			{
				response = RES_DATA;
				_main_set_pl_active(PL_DISABLE);
				/* set data */
				ex_usb_write_size = sizeof(TMP_10BIT_AD_TBL) + 6;

				*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
				*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
				*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
				*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
				*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
				*(ex_usb_write_buffer + 5) = response;

				p_data = (u8 *)&ex_tmp_10bit_ad_data;
				for(cnt = 0; cnt < sizeof(TMP_10BIT_AD_TBL); cnt++)
				{
					*(ex_usb_write_buffer + 6 + cnt) = *(p_data + cnt);
				}
			}
			else
			{
				response = RES_BUSY;
			}
			break;
		default:
			response = RES_NAK;
			break;
		}
		break;

	case PHASE_ADJUSTMENT_SAMPLING_AD_NON_PAPER:		//0x55	0x08
		switch((u8)ex_front_usb.pc.mess.command)
		{
		case CMD_RUN:
			response = RES_ACK;
			ex_send_offset = 0;
			_main_set_pl_active(PL_ENABLE);

			_pl_cis_enable_set(1);

			/* DONE: check PL enable or not */
			if(get_pl_state())
			{
				ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_ONE_SHOT;
				_dline_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_ONE_SHOT, CIS_MODE_AD_NON_PAPER, 0, 0);
			}
			else
			{
				response = RES_NG;
			}
			break;
		case CMD_ENQ:
			if(!ex_cis_adjustment_tbl.busy)
			{
				response = RES_DATA;
				_main_set_pl_active(PL_DISABLE);
				/* set data */
				ex_usb_write_size = sizeof(TMP_10BIT_AD_TBL) + 6;

				*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
				*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
				*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
				*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
				*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
				*(ex_usb_write_buffer + 5) = response;

				p_data = (u8 *)&ex_tmp_10bit_ad_data;
				for(cnt = 0; cnt < sizeof(TMP_10BIT_AD_TBL); cnt++)
				{
					*(ex_usb_write_buffer + 6 + cnt) = *(p_data + cnt);
				}
			}
			else
			{
				response = RES_BUSY;
			}
			break;
		default:
			response = RES_NAK;
			break;
		}
		break;

	case PHASE_ADJUSTMENT_SAMPLING_BC_PAPER:		//0x55	0x09
		switch((u8)ex_front_usb.pc.mess.command)
		{
		case CMD_RUN:
			response = RES_ACK;
			ex_send_offset = 0;
			_main_set_pl_active(PL_ENABLE);

			_pl_cis_enable_set(1);

			/* DONE: check PL enable or not */
			if(get_pl_state())
			{
				ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_ONE_SHOT;
				_dline_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_ONE_SHOT, CIS_MODE_BC_PAPER, 0, 0);
			}
			else
			{
				response = RES_NG;
			}
			break;
		case CMD_ENQ:
			if(!ex_cis_adjustment_tbl.busy)
			{
				response = RES_DATA;
				_main_set_pl_active(PL_DISABLE);
				/* set data */
				ex_usb_write_size = sizeof(TMP_10BIT_AD_TBL) + 6;

				*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
				*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
				*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
				*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
				*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
				*(ex_usb_write_buffer + 5) = response;

				p_data = (u8 *)&ex_tmp_10bit_ad_data;
				for(cnt = 0; cnt < sizeof(TMP_10BIT_AD_TBL); cnt++)
				{
					*(ex_usb_write_buffer + 6 + cnt) = *(p_data + cnt);
				}
			}
			else
			{
				response = RES_BUSY;
			}
			break;
		default:
			response = RES_NAK;
			break;
		}
		break;

	case PHASE_ADJUSTMENT_SAMPLING_BC_NON_PAPER:		//0x55	0x0A
		switch((u8)ex_front_usb.pc.mess.command)
		{
		case CMD_RUN:
			response = RES_ACK;
			ex_send_offset = 0;
			_main_set_pl_active(PL_ENABLE);

			_pl_cis_enable_set(1);

			/* DONE: check PL enable or not */
			if(get_pl_state())
			{
				ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_ONE_SHOT;
				_dline_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_ONE_SHOT, CIS_MODE_BC_NON_PAPER, 0, 0);
			}
			else
			{
				response = RES_NG;
			}
			break;
		case CMD_ENQ:
			if(!ex_cis_adjustment_tbl.busy)
			{
				response = RES_DATA;
				_main_set_pl_active(PL_DISABLE);
				/* set data */
				ex_usb_write_size = sizeof(TMP_10BIT_AD_TBL) + 6;

				*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
				*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
				*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
				*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
				*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
				*(ex_usb_write_buffer + 5) = response;

				p_data = (u8 *)&ex_tmp_10bit_ad_data;
				for(cnt = 0; cnt < sizeof(TMP_10BIT_AD_TBL); cnt++)
				{
					*(ex_usb_write_buffer + 6 + cnt) = *(p_data + cnt);
				}
			}
			else
			{
				response = RES_BUSY;
			}
			break;
		default:
			response = RES_NAK;
			break;
		}
		break;
#if MAG1_ENABLE
	case PHASE_ADJUSTMENT_SAMPLING_ADJ_MAG:		//0x55	0x10
		switch((u8)ex_front_usb.pc.mess.command)
		{
		case CMD_RUN:
			response = RES_ACK;
			ex_send_offset = 0;
			_main_set_pl_active(PL_ENABLE);

			_pl_cis_enable_set(1);

			/* DONE: check PL enable or not */
			if(get_pl_state())
			{
				ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_ONE_SHOT;
				_dline_send_msg(ID_DISCRIMINATION_MBX, TMSG_CIS_INITIALIZE_REQ, AD_MODE_MAG_ADJUSTMENT, MAG_INIT_MODE_STANDBY, 0, 0);
			}
			else
			{
				response = RES_NG;
			}
			break;
		case CMD_ENQ:
			if(!ex_cis_adjustment_tbl.busy)
			{
				response = RES_DATA;
				_main_set_pl_active(PL_DISABLE);
				/* set data */
				ex_usb_write_size = sizeof(MAG_SENSOR_DATA) + 6;

				*ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
				*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8)&0xff);
				*(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
				*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
				*(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
				*(ex_usb_write_buffer + 5) = response;

				p_data = (u8 *)&ex_tmp_mag_data;
				for(cnt = 0; cnt < sizeof(MAG_SENSOR_DATA); cnt++)
				{
					*(ex_usb_write_buffer + 6 + cnt) = *(p_data + cnt);
				}
			}
			else
			{
				response = RES_BUSY;
			}
			break;

		default:
			response = RES_NAK;
			break;
		}
		break;
#endif
	case PHASE_ADJUSTMENT_SAMPLING_PL_ENABLE:		//0x55	0xF0
		if(ex_front_usb.pc.mess.command == CMD_RUN)	//0x55	0xF0  0x01
		{
			response = RES_ACK;
			_main_set_pl_active(PL_ENABLE);

			_pl_cis_enable_set(1);

			_main_set_sensor_active(1);
		}
		else if(ex_front_usb.pc.mess.command == CMD_ENQ)	//0x55	0xF0  0x05
		{
			response = RES_ACK;
			_main_set_sensor_active(0);
			_main_set_pl_active(PL_DISABLE);
		}
		else
		{
			response = RES_NAK;
		}
		break;

	default:
		response = RES_NAK;
		break;
	}
	return response;
};

void front_usb_adjustment_request()
{
	u8 response = 0;

/*<<	調整モード及び各モード確認		>>*/
	ex_front_usb.pc.mess.serviceID = FUSB_SERVICE_ID_TESTMODE;
	switch((u8)ex_front_usb.pc.mess.modeID)
	{
	case MODE_ADJUSTMENT_SENSOR_DATA:
		/* Adjustment Proc */
		response = sensor_data();
		break;
	case MODE_ADJUSTMENT_EEPROM_RW:	// 0x53
		/* Read & Write EEPROM */
		response = adjustment_eeprom_rw();
		break;
	case MODE_ADJUSTMENT_VALUE:		// 0x54
		/* Set & Get value */
		response = adjustment_value(0);
		break;
	case MODE_ADJUSTMENT_SAMPLING:		// 0x55
		/* Start & Get Sampling */
		response = adjustment_sampling();
		break;
	case MODE_ADJUSTMENT_TEMPERATURE:
		response = RES_NAK;//adjustment_temperature();
		break;
	default:
		response = RES_NAK;
		break;
	}

	if(response != RES_DATA)
	{
		set_response_1data(response);
	}
	return;
}
