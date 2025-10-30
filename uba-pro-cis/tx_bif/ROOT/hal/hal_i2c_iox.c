/******************************************************************************/
/*! @addtogroup Main
    @file       hal_i2c_iox.c
    @brief      I2C I/O expander, analog mux driver.
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
*****************************************************************************/

/***************************** Include Files *********************************/
#include "string.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "hal_i2c_iox.h"
#include "hal_rstmgr.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

/************************** PRIVATE DEFINITIONS *************************/
#define E_I2C_PARM    -8192          /* 0xF000: 引数エラー                   */
/************************** PRIVATE DEFINITIONS *************************/
#define I2C_RETRY_COUNT		10
/* slave address */
#define	I2C3_S_TCA9535_0	0x40	// 0100000xB	/* tca9535	I/O Expander */

/* I/O expanders direction data */
#define DIR_OUT	0
#define DIR_IN	1
#define TCA9535_0_DIR0		0x83	// 10000011B	/* 1:in/0:out */
#define TCA9535_0_DIR1		0xFF	// 11111111B	/* 1:in/0:out */

/* I/O expanders direction data */
#define TCA9535_0_POL0		0x83	// 10000011B	/* 1:inverted/0:retained */
#define TCA9535_0_POL1		0xFF	// 11111111B	/* 1:inverted/0:retained */

#define TCA9535_CMD_INPUT0     0x00
#define TCA9535_CMD_INPUT1     0x01
#define TCA9535_CMD_OUTPUT0    0x02
#define TCA9535_CMD_OUTPUT1    0x03
#define TCA9535_CMD_POLARITY0  0x04
#define TCA9535_CMD_POLARITY1  0x05
#define TCA9535_CMD_CONFIG0    0x06
#define TCA9535_CMD_CONFIG1    0x07

#define BIT_C_MSENS		0x01
#define BIT_RFID_INT	0x02
#define BIT_C_MR		0x04
#define BIT_F_LED_RED	0x08
#define BIT_F_LED_GREEN	0x10
#define BIT_F_LED_BLUE	0x20
#define BIT_F_LED_C		0x40
#define BIT_DSW2		0x80
/************************** PRIVATE VARIABLES *************************/

/************************** PRIVATE FUNCTIONS *************************/
/************************** EXTERN VARIABLES *************************/
/************************** EXTERN FUNCTIONS *************************/

/*********************************************************************//**
 * @brief		Initialize io-expander
 * @param[in]	None
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
INT32 _hal_i2c3_init_iox(void)
{
	INT32 ercd;

	if((ercd = _hal_i2c3_init_tca9535(0)) != E_OK)
	{
		return ercd;
	}
	return E_OK;
}
/*********************************************************************//**
 * @brief		Initialize tca9535
 * @param[in]	None
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
INT32 _hal_i2c3_init_tca9535(u8 devno)
{
	I2C_SEND_PACK pack;
	UB w[16];
	UINT8 addr;
	UINT8 dir0,dir1,pol0,pol1;
	ER ercd = E_OK;

	if(devno == 0)
	{
		addr = I2C3_S_TCA9535_0;
		dir0 = TCA9535_0_DIR0;
		dir1 = TCA9535_0_DIR1;
		pol0 = TCA9535_0_POL0;
		pol1 = TCA9535_0_POL1;
	}
	else
	{
		return E_ARG;
	}

	if (ercd == E_OK)
	{
		for(int retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
#if 0
				// DONE:reset module(RSTMGR)
				RstmgrI2C3Reset();
				/* ドライバクローズ */
				I2c_close( &hI2c3 );
				// DONE:reopen I2C1(JSL-Ware)
				setup_i2c3();
#endif
			}
			ercd = E_OK;
			/* I/O direction 0 */
			/* ライト動作 */
			w[0] = TCA9535_CMD_CONFIG0;
			w[1] = dir0;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = addr;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* I/O direction 1 */
			/* ライト動作 */
			w[0] = TCA9535_CMD_CONFIG1;
			w[1] = dir1;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = addr;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* I/O polarity 0 */
			/* ライト動作 */
			w[0] = TCA9535_CMD_POLARITY0;
			w[1] = pol0;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = addr;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* I/O polarity 1 */
			/* ライト動作 */
			w[0] = TCA9535_CMD_POLARITY1;
			w[1] = pol1;
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = addr;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
	}

	if(ercd != E_OK)
	{
#if 0
		// DONE:reset module(RSTMGR)
		RstmgrI2C3Reset();
		/* ドライバクローズ */
		I2c_close( &hI2c3 );
		// DONE:reopen I2C1(JSL-Ware)
		setup_i2c3();
#endif
	}
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.i2c3 = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.i2c3 = HAL_STATUS_OK;
	}
#endif
	return ercd;
}
/*********************************************************************//**
 * @brief		Write tca9535 Register
 * @param[in]	None
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
static INT32 _hal_i2c3_write_tca9535(u16 data)
{
	I2C_SEND_PACK pack;
	u8 w[16];
	INT32 ercd= E_OK;
	int retry;

	ercd = wai_sem(ID_I2C3_SEM);

	if (ercd == E_OK)
	{
		for(retry = 0; retry < I2C_RETRY_COUNT; retry++ )
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
			/* ライト動作 */
			w[0] = TCA9535_CMD_OUTPUT1;
			w[1] = (u8)((data >> 8) & 0xFF);
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_TCA9535_0;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* ライト動作 */
			w[0] = TCA9535_CMD_OUTPUT0;
			w[1] = (u8)(data & 0xFF);
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_TCA9535_0;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
	}

	if(ercd != E_OK)
	{
		i2c3_reset();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.i2c3 = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.i2c3 = HAL_STATUS_OK;
	}
#endif

	sig_sem(ID_I2C3_SEM);
 	return ercd;
}
/*********************************************************************//**
 * @brief		Read Dip switch Value(tca9535)
 * @param[in]	dipsw_no DIPSW1 or DIPSW2
 * @param[out]	dipsw_value dipsw value
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
INT32 _hal_i2c3_read_dipsw(u8 dipsw_no, u8 *dipsw_value)
{
	I2C_SEND_PACK pack;
	u8 value = 0xFF;
	u8 dipsw = 0xFF;
	UB w[16];
	UB r[16];
	ER ercd = E_OK;

	if(dipsw_no == DIPSW1)
	{
		// Input Port 1
		w[0] = TCA9535_CMD_INPUT1;
	}
	else if(dipsw_no == DIPSW2)
	{
		// Input Port 0
		w[0] = TCA9535_CMD_INPUT0;
	}
	else
	{
		*dipsw_value = 0x00;
		return E_PAR;
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
			/* ライト→リピートスタート→リード動作 */
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_TCA9535_0;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 1;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}

	}

	value = r[0];

	if(ercd == E_OK)
	{
		dipsw = value;
		*dipsw_value = ~dipsw;
	}
	else
	{
		*dipsw_value = 0x00;
		i2c3_reset();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.i2c3 = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.i2c3 = HAL_STATUS_OK;
	}
#endif
	sig_sem(ID_I2C3_SEM);

	return ercd;
}
/*********************************************************************//**
 * @brief		Read tca9535 Register
 * @param[in]	None
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
static INT32 _hal_i2c3_tca9535_read_dir(u16 *value)
{
	I2C_SEND_PACK pack;
	u8 w[16];
	u8 r[16];
	u8 data[2];
	INT32 ercd = E_OK;
	int retry;

	if(value == 0)
	{
		*value = 0x00;
		return E_PAR;
	}
	ercd = wai_sem(ID_I2C3_SEM);

	if (ercd == E_OK)
	{
		for( retry = 0; retry < I2C_RETRY_COUNT; retry++ )
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
			w[0] = TCA9535_CMD_CONFIG1;
			/* I2Cスイッチ設定 */
			/* ライト→リピートスタート→リード動作 */
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_TCA9535_0;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 1;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			data[1] = r[0];
			w[0] = TCA9535_CMD_CONFIG0;
			/* I2Cスイッチ設定 */
			/* ライト→リピートスタート→リード動作 */
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_TCA9535_0;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 1;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			data[0] = r[0];
			/* leave loop */
			break;
		}
	}


	if(ercd == E_OK)
	{
		*value = (u16)(data[1] << 8 | data[0]);
	}
	else
	{
		*value = 0x00;
		i2c3_reset();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.i2c3 = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.i2c3 = HAL_STATUS_OK;
	}
#endif
	sig_sem(ID_I2C3_SEM);

	return ercd;
}
/*********************************************************************//**
 * @brief		Read tca9535 Register
 * @param[in]	None
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
static INT32 _hal_i2c3_tca9535_write_dir(u16 data)
{
	I2C_SEND_PACK pack;
	u8 w[16];
	INT32 ercd = E_OK;
	int retry;

	ercd = wai_sem(ID_I2C3_SEM);

	if (ercd == E_OK)
	{
		for( retry = 0; retry < I2C_RETRY_COUNT; retry++ )
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
			w[0] = TCA9535_CMD_CONFIG1;
			w[1] = (u8)((data >> 8) & 0xFF);
			/* ライト動作 */
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_TCA9535_0;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* ライト動作 */
			w[0] = TCA9535_CMD_CONFIG0;
			w[1] = (u8)(data & 0xFF);
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_TCA9535_0;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
	}
	if(ercd != E_OK)
	{
		i2c3_reset();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.i2c3 = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.i2c3 = HAL_STATUS_OK;
	}
#endif
	sig_sem(ID_I2C3_SEM);

	return ercd;
}
/*********************************************************************//**
 * @brief		Change Direction tca9535 Register
 * @param[in]	data
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
static INT32 _hal_i2c3_tca9535_set_dir(UINT8 dir, UINT16 data)
{
	u32 ercd = E_OK;
	u16 rvalue;
	u16 svalue;

	ercd = _hal_i2c3_tca9535_read_dir(&rvalue);
	if(ercd == E_OK)
	{
		if(dir == DIR_IN)
		{
			svalue = rvalue | data;
		}
		else
		{
			svalue = rvalue & ~data;
		}

		ercd = _hal_i2c3_tca9535_write_dir(svalue);
	}
#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.i2c3 = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.i2c3 = HAL_STATUS_OK;
	}
#endif
	return ercd;
}
u8 swapbit(u8 in)
{
	u8 res = 0;
	int len = 8;
	while (len--)
	{
		res <<= 1;
		res |= (in & 1);
		in >>= 1;
	}
	return res;
}
/*********************************************************************//**
 * @brief		Read Dip switch Value(tca9535)
 * @param[in]	dipsw_no DIPSW1 or DIPSW2
 * @param[out]	dipsw_value dipsw value
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
INT32 _hal_i2c3_read_dipsw1(UINT8 *dipsw_value)
{
	I2C_SEND_PACK pack;
	UINT8 value = 0xFF;
	UINT8 dipsw = 0xFF;
	UINT8 w[16];
	UINT8 r[16];
	INT32 ercd = E_OK;
	int retry;


	if(dipsw_value == 0)
	{
		return E_I2C_PARM;
	}
	ercd = wai_sem(ID_I2C3_SEM);

	if (ercd == E_OK)
	{
		for( retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				ercd = i2c3_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}
			// Input Port 1
			w[0] = 1;

			/* I2Cスイッチ設定 */
			/* ライト→リピートスタート→リード動作 */
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_TCA9535_0;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 1;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			value = r[0];
			/* leave loop */
			break;
		}
	}

	if(ercd == E_OK)
	{
		dipsw = value;
		#if defined(PRJ_IVIZION2)
		*dipsw_value = swapbit(dipsw);
		#else
		*dipsw_value = dipsw;
		#endif
	}
	else
	{
		*dipsw_value = 0x00;
	}
	sig_sem(ID_I2C3_SEM);

	return ercd;
}
/*********************************************************************//**
 * @brief		Read Dip switch Value(tca9535)
 * @param[in]	dipsw_no DIPSW1 or DIPSW2
 * @param[out]	dipsw_value dipsw value
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
INT32 _hal_i2c3_read_dipsw2(UINT8 *dipsw_value)
{
	I2C_SEND_PACK pack;
	UINT8 value = 0xFF;
	UINT8 w[16];
	UINT8 r[16];
	INT32 ercd = E_OK;
	int retry;

	if(dipsw_value == 0)
	{
		return E_I2C_PARM;
	}
	ercd = wai_sem(ID_I2C3_SEM);

	if (ercd == E_OK)
	{
		for( retry = 0; retry < I2C_RETRY_COUNT; retry++ )
		{
			if(ercd != E_OK)
			{
				ercd = i2c3_reset();
				if(ercd != E_OK)
				{
					continue;
				}
			}

			// Input Port 0-bit7
			w[0] = 0;

			/* I2Cスイッチ設定 */
			/* ライト→リピートスタート→リード動作 */
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_TCA9535_0;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 1;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			value = r[0];
			/* leave loop */
			break;
		}
	}

	if(ercd == E_OK)
	{
		if((value & BIT_DSW2))
		{
			*dipsw_value = 1;
		}
		else
		{
			*dipsw_value = 0;
		}
	}
	else
	{
		*dipsw_value = 0x00;
		i2c3_reset();
	}
	sig_sem(ID_I2C3_SEM);

	return ercd;
}

/*********************************************************************//**
 * @brief		Control bezel LED
 * @param[in]	data
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
INT32 _hal_i2c3_write_led_bezel(u16 data)
{
	INT32 ercd = E_OK;
	//status ledはDIR INの場合があるので0固定
	#if 1 //2022-11-14
	ercd = _hal_i2c3_tca9535_set_dir(DIR_OUT, BIT_F_LED_C);
	if(ercd == E_OK)
	{
		if(data == 1)
		{
			ercd = _hal_i2c3_write_tca9535(BIT_F_LED_C);
		}
		else
		{
			ercd = _hal_i2c3_write_tca9535(0);
		}
	}
	return ercd;
	#else
	if(data == 1)
	{
		ercd = _hal_i2c3_write_tca9535(BIT_F_LED_C);
	}
	else
	{
		ercd = _hal_i2c3_write_tca9535(0);
	}
	return ercd;
	#endif
}
/*********************************************************************//**
 * @brief		Control LED
 * @param[in]	data
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
INT32 _hal_i2c3_write_led_red(UINT8 data)
{
	INT32 ercd = E_OK;

	ercd = _hal_i2c3_tca9535_set_dir(data, BIT_F_LED_RED);

#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.led_red = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.led_red = HAL_STATUS_OK;
	}
#endif
	return ercd;
}

/*********************************************************************//**
 * @brief		Control LED
 * @param[in]	data
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
INT32 _hal_i2c3_write_led_green(UINT8 data)
{
	INT32 ercd = E_OK;

	ercd = _hal_i2c3_tca9535_set_dir(data, BIT_F_LED_GREEN);

#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.led_green = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.led_green = HAL_STATUS_OK;
	}
#endif
	return ercd;
}

/*********************************************************************//**
 * @brief		Control LED
 * @param[in]	data
 * @return      Succeeded or failed
 * @retval      E_OK succeeded
 * @retval      OTHER failed
 **********************************************************************/
INT32 _hal_i2c3_write_led_blue(UINT8 data)
{
	INT32 ercd = E_OK;

	ercd = _hal_i2c3_tca9535_set_dir(data, BIT_F_LED_BLUE);

#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.led_blue = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.led_blue = HAL_STATUS_OK;
	}
#endif
	return ercd;
}


//2023-12-22
INT32 _hal_i2c3_read_write_p0(u8 data, u8 change_bit)
{
	I2C_SEND_PACK pack;
	u8 value = 0xFF;
	u8 dipsw = 0xFF;
	UB w[16];
	UB r[16];
	ER ercd = E_OK;
	int retry;

	// Read Port 0
	w[0] = TCA9535_CMD_INPUT0;

	ercd = wai_sem(ID_I2C3_SEM);

	if (ercd == E_OK)
	{
		for( retry = 0; retry < I2C_RETRY_COUNT; retry++ )
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
			/* ライト→リピートスタート→リード動作 */
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_TCA9535_0;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 1;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}

	}

	value = r[0];

	if(ercd == E_OK)
	{
		/* 読み込んだデータを保持して、変更したい部分を変更して書き込み */
		if(1 == data)
		{
			data = value | change_bit;
			#if 1	//2022-12-05a LEDは常にLowで点灯できるようにする
			data = data & 0xC7;
			#endif
		}
		else
		{
			/* 対象のビットを 0 */
			data = value & ~change_bit;
			#if 1	//2022-12-05a LEDは常にLowで点灯できるようにする
			data = data & 0xC7;
			#endif
		}

		for( retry = 0; retry < I2C_RETRY_COUNT; retry++ )
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
			/* ライト動作 */
			w[0] = TCA9535_CMD_OUTPUT0;
			w[1] = (u8)(data & 0xFF);
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_TCA9535_0;
			pack.write_dat = w;
			pack.write_len = 2;
			pack.read_dat = NULL;
			pack.read_len = 0;
			if( I2c_send( &hI2c3, &pack ) == FALSE )
			{
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}
	}
	else
	{
		i2c3_reset();
	}

#if (HAL_STATUS_ENABLE==1)
	if(ercd != E_OK)
	{
		ex_hal_status.i2c3 = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.i2c3 = HAL_STATUS_OK;
	}
#endif
	sig_sem(ID_I2C3_SEM);

	return ercd;
}

int _hal_i2c3_for_led_tca9535(u8 data, u8 change_bit)
{
	I2C_SEND_PACK pack;
	u8 value = 0xFF;
	u8 dipsw = 0xFF;
	UB w[16];
	UB r[16];
	ER ercd = E_OK;
	int retry;

	ercd = wai_sem(ID_I2C3_SEM);

	// Read Configu register
	w[0] = TCA9535_CMD_CONFIG0;;


	if (ercd == E_OK)
	{
		for( retry = 0; retry < I2C_RETRY_COUNT; retry++ )
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
			/* ライト→リピートスタート→リード動作 */
			memset( (void *)&pack, 0, sizeof(pack) );
			pack.address = I2C3_S_TCA9535_0;
			pack.write_dat = w;
			pack.write_len = 1;
			pack.read_dat = r;
			pack.read_len = 1;
			if( I2c_send( &hI2c3, &pack ) == FALSE ){
				ercd = E_I2C_SEND;
				continue;
			}
			/* leave loop */
			break;
		}

	}

	/* ポートは点灯 Low*/
	/* ポートは消灯 High*/
	/* Configは点灯, Low */
	/* Configは消灯, High*/

	value = r[0];

	if(1 == data)
	{
	/* Configは点灯, Low *//* 対象のビットを 0 */
		data = value & ~change_bit;
	}
	else
	{
	/* Configは消灯, High*/
		data = value | change_bit;
	}


	w[0] = TCA9535_CMD_CONFIG0;
	w[1] = data;
	memset( (void *)&pack, 0, sizeof(pack) );
	pack.address = I2C3_S_TCA9535_0;
	pack.write_dat = w;
	pack.write_len = 2;
	pack.read_dat = NULL;
	pack.read_len = 0;
	if( I2c_send( &hI2c3, &pack ) == FALSE ){
		ercd = E_I2C_SEND;
//		continue;
	}

	sig_sem(ID_I2C3_SEM);

	return ercd;
}




/*EOF*/
