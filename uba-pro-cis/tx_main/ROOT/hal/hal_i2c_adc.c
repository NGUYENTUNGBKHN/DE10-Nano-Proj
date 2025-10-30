/******************************************************************************/
/*! @addtogroup Main
    @file       hal_i2c_adc.c
    @brief      I2C A/D converter
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

#define I2C3_S_PWR_ADC		0xA8	// CH3:1010100xB	/* ADC081C021 */
/************************** EXTERN VARIABLES *************************/
// 16ch: DipSwitch1&2、8ch: ベゼル制御ポート(1-4)、磁気制御DigiPote
extern I2C_HANDLE hI2c0;
extern I2C_HANDLE hI2c3;
/************************** EXTERN FUNCTIONS *************************/
extern u32 i2c0_reset(void);
extern u32 i2c3_reset(void);

/*********************************************************************//**
 * @brief		Read Power Voltage A/D
 * @param[in]	None
 * @return 		None
 **********************************************************************/
ER _hal_voltage_ad_read(u8 *value)
{
	I2C_SEND_PACK pack;
	UB w[16];
	UB r[16];
	ER ercd = E_OK;

	ercd = wai_sem(ID_I2C3_SEM);

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < 5; retry++ )
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
			memset( (void *)&pack, 0, sizeof(pack) );
			memset( (void *)&r, 0, sizeof(r) );
			pack.address = I2C3_S_PWR_ADC;
			pack.write_dat = w;
			pack.write_len = 0;
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
		*value = (u8)((r[1]&0xF0) >> 4 | (r[0]&0xF) << 4);
	}
	else
	{
		i2c3_reset();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.i2c3 = HAL_STATUS_NG;
		ex_hal_status.power_adc = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.i2c3 = HAL_STATUS_OK;
		ex_hal_status.power_adc = HAL_STATUS_OK;
	}
#endif
	sig_sem(ID_I2C3_SEM);

	return ercd;
}
/* EOF */

