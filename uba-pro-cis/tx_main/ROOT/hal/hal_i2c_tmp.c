/******************************************************************************/
/*! @addtogroup Main
    @file       hal_i2c_tmp.c
    @brief      I2C I/F Digital Potentiometer driver
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/

/***************************** Include Files *********************************/
#include "string.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "js_io.h"
#include "js_oswapi.h"
#include "i2c/js_i2c.h"
#include "common.h"
#include "custom.h"
#include "hal.h"

#define EXT
#include "com_ram.c"

#include "hal_i2c_tmp.h"

/************************** PRIVATE DEFINITIONS *************************/
/* slave address */
// iVIZION2
// Temperature IC TMP112
//      resolution:0.0625
// 		MAX:128(0x7FF)
// 		MIN:-55(0xC90)
#define I2C3_S_TMP112_0		0x92	// 1001001xB	/* CISA:TMP112-0 */
#define I2C3_S_TMP112_1		0x90	// 1001000xB	/* CISB:TMP112-1 */
#define MIN_TMP112			((s16)0xC90)
#define MAX_TMP112			((s16)0x7FF)

// Temperature IC
//new temp ic
#define I2C3_S_STTS22H_0		0x7E	//将来使用される温度IC
#define MAX_STTS22H			((s16)0x30D4)	// 		MAX:125(0x30D4)
//old temp ic
#define I2C3_S_S5851A_0		0x9C	// 1001110xB	/* OUT AIR:S5851A-0 *///古い温度IC

#define MIN_S5851A			((s16)0xD80)	// 		MIN:-40(0xD80)
#define MAX_S5851A			((s16)0x7D0)	// 		MAX:125(0x7D0)
#define TMP_RES				((float)0.0625)

#define TMP112_CMD_RD                0x00
#define S5851A_CMD_RD                0x00
/************************** EXTERN VARIABLES *************************/
extern I2C_HANDLE hI2c3;
/************************** EXTERN FUNCTIONS *************************/
extern u32 i2c3_reset(void);
extern void __hal_i2c3_por(void);

/*********************************************************************//**
 * @brief		Read CIS-A temperature(TMP112)
 * @param[in]	bin temperature (12bit)
 * @param[in]	cel Celsius temperature (12bit)
 * @return 		None
 **********************************************************************/
ER _hal_tmp_read_cisa(u16 *bin, s16 *cel)
{
	I2C_SEND_PACK pack;
	UB w[16];
	UB r[16];
	ER ercd = E_OK;

	if((bin == 0) || (cel == 0))
	{
		return E_I2C_PARM;
	}
	ercd = wai_sem(ID_I2C3_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				__hal_i2c3_por();
				ercd = i2c3_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			/* ライト→リピートスタート→リード動作 */
			w[0] = TMP112_CMD_RD;
			memset( (void *)&pack, 0, sizeof(pack) );
			memset( (void *)&r, 0, sizeof(r) );
			pack.address = I2C3_S_TMP112_0;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 2;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
	}

	if(ercd == E_OK)
	{
		*bin = (u16)(r[1] >> 4 | r[0] << 4);
		if( (*bin) & 0x800)
		{
		/* - */
			*cel = (s16)(0 - (((0xFFF - (*bin)) + 1) * TMP_RES));
		}
		else
		{
		/* + */
			*cel = (s16)((*bin) * TMP_RES);
		}
	}
	else
	{
		*cel = MAX_TMP112;
		__hal_i2c3_por();
		i2c3_reset();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.tmp_cisa = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.tmp_cisa = HAL_STATUS_OK;
	}
#endif
	sig_sem(ID_I2C3_SEM);

	return ercd;
}
/*********************************************************************//**
 * @brief		Read CIS-B temperature(TMP112)
 * @param[in]	ret temperature (12bit)
 * @return 		None
 **********************************************************************/
ER _hal_tmp_read_cisb(u16 *bin, s16 *cel)
{
	I2C_SEND_PACK pack;
	UB w[16];
	UB r[16];
	ER ercd = E_OK;

	if((bin == 0) || (cel == 0))
	{
		return E_I2C_PARM;
	}
	ercd = wai_sem(ID_I2C3_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				__hal_i2c3_por();
				ercd = i2c3_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			/* ライト→リピートスタート→リード動作 */
			w[0] = TMP112_CMD_RD;
			memset( (void *)&pack, 0, sizeof(pack) );
			memset( (void *)&r, 0, sizeof(r) );
			pack.address = I2C3_S_TMP112_1;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 2;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
	}

	if(ercd == E_OK)
	{
		*bin = (u16)(r[1] >> 4 | r[0] << 4);
		if( (*bin) & 0x800)
		{
		/* - */
			*cel = (s16)(0 - (((0xFFF - (*bin)) + 1) * TMP_RES));
		}
		else
		{
		/* + */
			*cel = (s16)((*bin) * TMP_RES);
		}
	}
	else
	{
		*cel = MAX_TMP112;
		__hal_i2c3_por();
		i2c3_reset();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.tmp_cisb = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.tmp_cisb = HAL_STATUS_OK;
	}
#endif
	sig_sem(ID_I2C3_SEM);

	return ercd;
}

ER _hal_tmp_read_out(u16 *bin, s16 *cel)
{

	ER ercd = E_OK;
	static u8 new_type = 0;	/* 基本は、new_type == 0, ABLIC IC  12万台程度*/

	if(new_type == 0)
	{
		if(_hal_tmp_read_out_old(bin, cel) != E_OK)
		{
			if(_hal_tmp_read_out_new(bin, cel) != E_OK)
			{
			/* NG */
				ercd = ERROR;
				return ercd;
			}
			else
			{
			/* OK */
				new_type = 1;	/* 次からはnew_type */
				return ercd;
			}
		}
		else
		{
			/* OK */
			return ercd;
		}
	}
	else
	{
		if(_hal_tmp_read_out_new(bin, cel) != E_OK)
		{
			if(_hal_tmp_read_out_old(bin, cel) != E_OK)
			{
			/* NG */
				ercd = ERROR;
				return ercd;
			}
			else
			{
			/* OK */
				new_type = 0;	/* 次からはold_type */
				return ercd;
			}
		}
		else
		{
			/* OK */
			return ercd;			
		}
	}
}


ER _hal_tmp_read_out_old(u16 *bin, s16 *cel)
{
	I2C_SEND_PACK pack;
	UB w[16];
	UB r[16];
	ER ercd = E_OK;

	if((bin == 0) || (cel == 0))
	{
		return E_I2C_PARM;
	}
	ercd = wai_sem(ID_I2C3_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				__hal_i2c3_por();
				ercd = i2c3_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			/* ライト→リピートスタート→リード動作 */
			w[0] = S5851A_CMD_RD;

			memset( (void *)&pack, 0, sizeof(pack) );
			memset( (void *)&r, 0, sizeof(r) );
			pack.address = I2C3_S_S5851A_0;

			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 2;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
	}

	if(ercd == E_OK)
	{
		*bin = (u16)(r[1] >> 4 | r[0] << 4);
		if( (*bin) & 0x800)
		{
		/* - */
			*cel = (s16)(0 - (((0xFFF - (*bin)) + 1) * TMP_RES));
		}
		else
		{
		/* + */
			*cel = (s16)((*bin) * TMP_RES);
		}
	}
	else
	{
		*cel = MAX_S5851A;
		__hal_i2c3_por();
		i2c3_reset();
	}

#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.tmp_out = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.tmp_out = HAL_STATUS_OK;
	}
#endif
	sig_sem(ID_I2C3_SEM);

	return ercd;

}

ER _hal_tmp_read_out_new(u16 *bin, s16 *cel)
{
	I2C_SEND_PACK pack;
	UB w[16];
	UB r[16];
	ER ercd = E_OK;
	UB temp[2];
	if((bin == 0) || (cel == 0))
	{
		return E_I2C_PARM;
	}
	ercd = wai_sem(ID_I2C3_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				__hal_i2c3_por();
				ercd = i2c3_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			/* ライト→リピートスタート→リード動作 */
			w[0] = 0x04;
			/* bit3:auto increase, bit2:freerun, bit1:timeout disable  */
			w[1] = 0x0E;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_STTS22H_0;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			/* ライト→リピートスタート→リード動作 */
			w[0] = 0x06; //0x07
			memset( (void *)&pack, 0, sizeof(pack) );
			memset( (void *)&r, 0, sizeof(r) );
			pack.address = I2C3_S_STTS22H_0;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 2;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
	}

	if(ercd == E_OK)
	{
		temp[0] = r[0];
		temp[1] = r[1];
		*bin = (u16)(r[1] << 8 | r[0]);
		if( (*bin) >= 0x8000)
		{
		/* - */
			*cel = (s16)(((s32)(*bin) - 0x10000) / 100);
		}
		else
		{
		/* + */
			*cel = (s16)((*bin) / 100);
		}
	}
	else
	{
		*cel = MAX_STTS22H;
		__hal_i2c3_por();
		i2c3_reset();
	}

#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.tmp_out = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.tmp_out = HAL_STATUS_OK;
	}
#endif
	sig_sem(ID_I2C3_SEM);

	return ercd;
}



/* EOF */

