/******************************************************************************/
/*! @addtogroup Main
    @file       hal_i2c_eeprom.c
    @brief      I2C EEPROM driver
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

#include "hal_i2c_eeprom.h"

/************************** PRIVATE DEFINITIONS *************************/
/* slave address */
// iVIZION2
// EEPROM IC S23C32
#define I2C3_S_S23C32_0		0xA0	// 1010000xB	/* MGU-EEPROM */

#define EEPROM_PAGE_SIZE	0x20	/* S24C32C 32byte */
#define EEPROM_TIME_WRITE	10		/* Twr 5ms*2 */
/************************** EXTERN VARIABLES *************************/
extern I2C_HANDLE hI2c3;
/************************** EXTERN FUNCTIONS *************************/
extern u32 i2c3_reset(void);
/************************** LOCAL VARIABLES *************************/
u8 eeprom_local_buffer[EEPROM_MAX_SIZE];

/*********************************************************************//**
 * @brief		Read EEPROM temperature(EEPROM)
 * @param[in]	dst destination address
 * @param[in]	addr eeprom address
 * @param[in]	len data size
 * @return 		None
 **********************************************************************/
ER _hal_read_eeprom(u8 *dst, u16 addr, u16 len) //もともとのまま
{
	I2C_SEND_PACK pack;
	UB w[16];
	UB r[16];
	ER ercd = E_OK;

	if((dst == 0) || (len == 0))
	{
		return E_I2C_PARM;
	}
	if(addr + len > EEPROM_MAX_SIZE)
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
				ercd = i2c3_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			ercd = E_OK;
			/* I2Cスイッチ設定 */
			/* ライト動作 */
			w[0] = (u8)((addr >> 8) & 0xFF);
			w[1] = (u8)(addr & 0xFF);
			memset( (void *)&pack, 0, sizeof(pack) );
			memset( (void *)&r, 0, sizeof(r) );
			pack.address = I2C3_S_S23C32_0;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = 0;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* ライト→リピートスタート→リード動作 */
			memset( (void *)&pack, 0, sizeof(pack) );
			memset( (void *)&r, 0, sizeof(r) );
			pack.address = I2C3_S_S23C32_0;
			pack.write_dat = 0;
			pack.write_len = 0;
			pack.read_dat = &eeprom_local_buffer[addr];
			pack.read_len = len;
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
		memcpy(dst, &eeprom_local_buffer[addr], len);
	}
	else
	{
		i2c3_reset();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.eeprom = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.eeprom = HAL_STATUS_OK;
	}
#endif
	sig_sem(ID_I2C3_SEM);

	return ercd;
}
/*********************************************************************//**
 * @brief		Write EEPROM temperature(EEPROM)
 * @param[in]	src source address
 * @param[in]	addr eeprom address
 * @param[in]	len data size
 * @return 		None
 **********************************************************************/
ER _hal_write_eeprom(u8 *src, u16 addr, u16 len)
{
	I2C_SEND_PACK pack;
	UB w[16];
	UB r[16];
	ER ercd = E_OK;
	int max_page;
	int cur_page;
	u16 cur_addr;
	u8 size;

	if((src == 0) || (len == 0))
	{
		return E_I2C_PARM;
	}
	if(addr + len > EEPROM_MAX_SIZE)
	{
		return E_I2C_PARM;
	}
	ercd = wai_sem(ID_I2C3_SEM);

	max_page = len/EEPROM_PAGE_SIZE;
	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				ercd = i2c3_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			for(cur_page = 0; cur_page < max_page; cur_page++)
			{
				ercd = E_OK;
				/* I2Cスイッチ設定 */
				/* ライト動作 */
				cur_addr = addr + cur_page * EEPROM_PAGE_SIZE;
				if(cur_addr + EEPROM_PAGE_SIZE > addr + len)
				{
					size = addr + len - cur_addr;
				}
				else
				{
					size = EEPROM_PAGE_SIZE;
				}
				w[0] = (u8)((cur_addr >> 8) & 0xFF);
				w[1] = (u8)(cur_addr & 0xFF);
				memset( (void *)&pack, 0, sizeof(pack) );
				memset( (void *)&r, 0, sizeof(r) );
				pack.address = I2C3_S_S23C32_0;
				pack.write_dat = w;
				pack.write_len = 2 + size;
				pack.read_dat = 0;
				pack.read_len = 0;
				if( I2c_send( &hI2c3, &pack ) == FALSE ){
					ercd = E_I2C_SEND;
					continue;
				}
				for(int time = 0; time < EEPROM_TIME_WRITE;  )
				{
					/* ライトポーリング */
					memset( (void *)&pack, 0, sizeof(pack) );
					memset( (void *)&r, 0, sizeof(r) );
					pack.address = I2C3_S_S23C32_0;
					pack.write_dat = 0;
					pack.write_len = 0;
					pack.read_dat = r;
					pack.read_len = 1;
					if( I2c_send( &hI2c3, &pack ) == TRUE )
					{
						break;
					}
					else
					{
						time++;
						if( time== EEPROM_TIME_WRITE ){
							ercd = E_I2C_SEND;
						}
					}
				}
				if( ercd == E_I2C_SEND ){
					ercd = E_I2C_SEND;
					continue;
				}
			}
			/* leave loop */
			break;
		}
	}

	if(ercd == E_OK)
	{
	}
	else
	{
		i2c3_reset();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.eeprom = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.eeprom = HAL_STATUS_OK;
	}
#endif
	sig_sem(ID_I2C3_SEM);

	return ercd;
}

#if defined(EEPROM_TEST)
ER _hal_write_eeprom_test() //整理中
{
	//EEPROM実装確認の為、書き込み、読み込みを固定にする
	I2C_SEND_PACK pack;
	UB w[2+10];		//テスト用に10バイト固定 + 2バイト アドレス
	UB r[16];
	ER ercd = E_OK;

	UB data_w[10];
	UB data_r[10];	//書き込みデータ正常か確認する為の読み込みデータ保存先

	u8 *src;
	u8 size = 10;		//サイズ固定
	u16 addr = 0;	//アドレス0固定
	//書き込みデータ作成
	for(int i = 0; i < 40; i++)
	{
		data_w[i] = i;
	}
	src = &data_w[0];

	ercd = wai_sem(ID_I2C3_SEM);
	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				ercd = i2c3_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}

			ercd = E_OK;
			/* I2Cスイッチ設定 */
			/* ライト動作 */		
			w[0] = (u8)((addr >> 8) & 0xFF);
			w[1] = (u8)(addr & 0xFF);
			memcpy((u8 *)&w[2],(u8 *)(src + addr),size);

			memset( (void *)&pack, 0, sizeof(pack) );
			memset( (void *)&r, 0, sizeof(r) );
			pack.address = I2C3_S_S23C32_0;
			pack.write_dat = w;
			pack.write_len = 2 + size;
			pack.read_dat = 0;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
		/*-----------------------------------------------------*/
			dly_tsk(10);	//書き込み時間最大 5ms待つ必要あり
			/* Read確認 */
			w[0] = (u8)((addr >> 8) & 0xFF);
			w[1] = (u8)(addr & 0xFF);
			memset( (void *)&pack, 0, sizeof(pack) );
			memset( (void *)&r, 0, sizeof(r) );
			pack.address = I2C3_S_S23C32_0;
			pack.write_dat = w;				//読み込みアドレス指定
			pack.write_len = 2;
			pack.read_dat = 0;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}

			memset( (void *)&pack, 0, sizeof(pack) );
			memset( (void *)&r, 0, sizeof(r) );
			pack.address = I2C3_S_S23C32_0;
			pack.write_dat = 0;
			pack.write_len = 0;
			pack.read_dat = &data_r[addr];
			pack.read_len = size;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/*-----------------------------------------------------*/
			/* leave loop */
			break;
		}
	}

	if(ercd == E_OK)
	{
	}
	else
	{
		i2c3_reset();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.eeprom = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.eeprom = HAL_STATUS_OK;
	}
#endif
	sig_sem(ID_I2C3_SEM);

	return ercd;
}
#endif


/* EOF */

