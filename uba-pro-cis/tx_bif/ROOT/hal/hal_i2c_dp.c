/******************************************************************************/
/*! @addtogroup Main
    @file       hal_i2c_dp.c
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

/************************** PRIVATE DEFINITIONS *************************/
#define E_I2C_SEND    -4096          /* 0xE000: I2C送信失敗エラー                   */
/* slave address */
//iVIZION2
// 糸検知DigiPote
#define I2C0_S_STR		0x50	// 0101000xB	/* MCP4651 Digital Potentiometer */
#define MCP4651_CMD_WRITE_0            0x80
#define MCP4651_CMD_WRITE_1            0x90

// 磁気DigiPote
#define I2C1_S_MAG		0x30	// 0011000xB	/* AD5258 Digital Potentiometer */

#define AD5258_CMD_RDAC                0x00
#define AD5258_CMD_EEPROM              0x20
#define AD5258_CMD_WRITE_PROTECTION    0x80
#define AD5258_CMD_RESTORE             0xA0
#define AD5258_CMD_STORE               0xC0
#define WRITE_PROTECTION_ACTIVATE      0x01
#define WRITE_PROTECTION_DEACTIVATE    0x00
/************************** EXTERN VARIABLES *************************/
// 16ch: DipSwitch1&2、8ch: ベゼル制御ポート(1-4)、磁気制御DigiPote
extern I2C_HANDLE hI2c0;
/************************** EXTERN FUNCTIONS *************************/
extern u32 i2c0_reset(void);

/*********************************************************************//**
 * @brief		Write Side gain(MCP4651)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
ER _hal_side_gain_write(u8 tap)
{
	I2C_SEND_PACK pack;
	UB w[16];
	ER ercd = E_OK;

	ercd = wai_sem(ID_I2C0_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				ercd = i2c0_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			w[0] = MCP4651_CMD_WRITE_0;
			w[1] = tap;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C0_S_STR;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c0, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
		sig_sem(ID_I2C0_SEM);
	}

	return ercd;
}
/*********************************************************************//**
 * @brief		Write UV gain(MCP4651)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
ER _hal_uv_gain_write(u8 tap)
{
	I2C_SEND_PACK pack;
	UB w[16];
	ER ercd = E_OK;

	ercd = wai_sem(ID_I2C0_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				ercd = i2c0_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			w[0] = MCP4651_CMD_WRITE_1;
			w[1] = tap;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C0_S_STR;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c0, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
		sig_sem(ID_I2C0_SEM);
	}

	return ercd;
}
/*********************************************************************//**
 * @brief		Restoring EEPROM to RDAC
 * @param[in]	None
 * @return 		None
 **********************************************************************/
ER _hal_mag_restore(void)
{
	I2C_SEND_PACK pack;
	UB w[16];
	ER ercd = E_OK;

	ercd = wai_sem(ID_I2C0_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				ercd = i2c0_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			w[0] = AD5258_CMD_RESTORE;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C1_S_MAG;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c0, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
		sig_sem(ID_I2C0_SEM);
	}
	return ercd;
}
/*********************************************************************//**
 * @brief		Storing RDAC Value to EEPROM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
ER _hal_mag_store(void)
{
	I2C_SEND_PACK pack;
	UB w[16];
	ER ercd = E_OK;

	ercd = wai_sem(ID_I2C0_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				ercd = i2c0_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			w[0] = AD5258_CMD_STORE;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C1_S_MAG;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c0, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
		sig_sem(ID_I2C0_SEM);
	}
	return ercd;
}
/*********************************************************************//**
 * @brief		Write Mag RDAC(AD5258)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
ER _hal_mag_tap_write(u8 tap)
{
	I2C_SEND_PACK pack;
	UB w[16];
	ER ercd = E_OK;

	ercd = wai_sem(ID_I2C0_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				ercd = i2c0_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			w[0] = AD5258_CMD_RDAC;
			w[1] = tap;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C1_S_MAG;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c0, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
		sig_sem(ID_I2C0_SEM);
	}
	return ercd;
}
/*********************************************************************//**
 * @brief		Read Mag RDAC(AD5258)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
ER _hal_mag_tap_read(u8 *tap)
{
	I2C_SEND_PACK pack;
	UB w[16];
	UB r[16];
	ER ercd = E_OK;

	ercd = wai_sem(ID_I2C0_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				ercd = i2c0_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			/* ライト→リピートスタート→リード動作 */
			w[0] = AD5258_CMD_RDAC;
			memset( (void *)&pack, 0, sizeof(pack) );
			memset( (void *)&r, 0, sizeof(r) );
			pack.address = I2C1_S_MAG;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 1;
			if( I2c_send( &hI2c0, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
		sig_sem(ID_I2C0_SEM);
	}

	if(ercd == E_OK)
	{
		*tap = r[0];
	}

	return ercd;
}

/*********************************************************************//**
 * @brief		Write Mag EEPROM(AD5258)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
ER _hal_mag_eeprom_write(u8 tap)
{
	I2C_SEND_PACK pack;
	UB w[16];
	ER ercd = E_OK;

	ercd = wai_sem(ID_I2C0_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				ercd = i2c0_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			w[0] = AD5258_CMD_WRITE_PROTECTION;
			w[1] = WRITE_PROTECTION_DEACTIVATE;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C1_S_MAG;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c0, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* I2Cスイッチ設定 */
			w[0] = AD5258_CMD_EEPROM;
			w[1] = tap;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C1_S_MAG;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c0, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* I2Cスイッチ設定 */
			w[0] = AD5258_CMD_WRITE_PROTECTION;
			w[1] = WRITE_PROTECTION_ACTIVATE;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C1_S_MAG;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c0, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}

			/* leave loop */
			break;
		}
		sig_sem(ID_I2C0_SEM);
	}
	return ercd;
}
/*********************************************************************//**
 * @brief		Read Mag EEPROM(AD5258)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
ER _hal_mag_eeprom_read(u8 *tap)
{
	I2C_SEND_PACK pack;
	UB w[16];
	UB r[16];
	ER ercd = E_OK;

	ercd = wai_sem(ID_I2C0_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				ercd = i2c0_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			/* ライト→リピートスタート→リード動作 */
			w[0] = AD5258_CMD_EEPROM;
			memset( (void *)&pack, 0, sizeof(pack) );
			memset( (void *)&r, 0, sizeof(r) );
			pack.address = I2C1_S_MAG;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 1;
			if( I2c_send( &hI2c0, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
		sig_sem(ID_I2C0_SEM);
	}

	if(ercd == E_OK)
	{
		*tap = r[0];
	}

	return ercd;
}
/* EOF */

