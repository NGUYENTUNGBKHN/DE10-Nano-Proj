/******************************************************************************/
/*! @addtogroup Main
    @file       hal_sensor.c
    @brief      ポジションセンサー入力ポート, ON/OFF制御ドライバファイル。
    @date       2018/02/26
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
/*! @ingroup hal_sensor hal_sensor */
/* @{ */

/***************************** Include Files *********************************/
#include "string.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"

#include "hal_led.h"
#include "hal_sensor.h"
/************************** Private Definitions *****************************/

/************************** Function Prototypes ******************************/

/************************** External functions *******************************/

/************************** Variable declaration *****************************/

/*********************************************************************//**
 * @brief		Set position sensor LED
 * @param[in]	set ON or OFF
 *              @arg 0 : Position sensor LED OFF
 *              @arg 1 : Position sensor LED ON.
 * @return 		None
 **********************************************************************/
void _hal_sen_position_LED_set(u8 set)
{
	u8 set_rev;

	Gpio_out( __HAL_PSLED_ONOFF, set );

	/* 下記のセンサは論理が逆 */
	/* Low点灯、High消灯 */
	if(set == 0)
	{
		set_rev = 1;
	}
	else
	{
		set_rev = 0;
	}
	Gpio_out( __HAL_HOME_LED, set_rev );

}

/*********************************************************************//**
 * @brief		Read position sensor
 * @param[in]	Non
 * @return 		bit  0: Entrance sensor				High ->bill detect
 *                   1: Centering timing sensor
 *                   2: APB entrance sensor
 *                   3: APB exit sensor
 *                   4: Exit sensor
 *                   5: APB home sensor
 *                   6: Centering home sensor
 **********************************************************************/
u16 _hal_sen_pos_sensor_read(void)
{
	u16 sensor = 0;

	int cnt = 0;
	static const UINT16 sensor_list[] =
	{
			__HAL_ENT_PS,			// 0x0001 /* これと並びが同じようにする POSI_ENTRANCE			0x0001 */
			__HAL_CENT_PS,			// 0x0002
			__HAL_PBIN_PS,			// 0x0004
			__HAL_PBOUT_PS,			// 0x0008
			__HAL_EXIT_PS,			// 0x0010
			__HAL_PB_HOME,			// 0x0020
			__HAL_CENT_OPEN_PS,		// 0x0040
		#if defined(UBA_RTQ)
			0,						// 0x0080//RTQはポートではなくRTQからの通信で取得
		#else
			__HAL_BOX_HOME_PS,		// 0x0080
		#endif
			__HAL_SHUTTER_HOME,		// 0x0100
		#if defined(UBA_RTQ)
			0,						// 0x0200//RTQはポートではなくRTQからの通信で取得
		#else
			__HAL_BOX1_PS,			// 0x0200
		#endif
			0,						// 0x0400
			0,						// 0x0800
			0,						// 0x1000
			0,						// 0x2000
			0,						// 0x4000
			0,						// 0x8000
	};
	static const UINT16 polarity_list[] =
	{
			0,			// 0x0001
			0,			// 0x0002
			0,			// 0x0004
			0,			// 0x0008
			0,			// 0x0010 Exit
			0,			// 0x0020 PB home not use
			1,			// 0x0040 centoring	/* 論理が逆のようだ */
			1,			// 0x0080 BOX home /* 論理が逆のようだ */
			1,			// 0x0100 shutter Low Open,High close
			1,			// 0x0200 Box exist
			1,			// 0x0400 not use
			0,			// 0x0800
			0,			// 0x1000
			0,			// 0x2000
			0,			// 0x4000
			0,			// 0x8000
	};

	for(cnt = 0; cnt < 16; ++cnt)
	{
		if(sensor_list[cnt] != 0)
		{
			if(polarity_list[cnt])
			{
				if(!(Gpio_in(sensor_list[cnt])))
					sensor |= (1 << cnt);
			}
			else
			{
				if(Gpio_in(sensor_list[cnt]))
					sensor |= (1 << cnt);
			}
		}
	}

	return (sensor);
}




/*********************************************************************//**
 * @brief		Position Sensor & Home Sensor ON
 * @param[in]	set
 * @return 		None
 **********************************************************************/
void _hal_position_sensor_on(void)
{
	_hal_sen_position_LED_set(1);
}

void _hal_position_intr_on(void)
{
	GpioIsr_clear( __HAL_PB_ENC );
	GpioIsr_enable( __HAL_PB_ENC );
}

/*********************************************************************//**
 * @brief		Position Sensor & Home Sensor OFF
 * @param[in]	set
 * @return 		None
 **********************************************************************/
void _hal_position_sensor_off(void)
{
	_hal_sen_position_LED_set(0);
	GpioIsr_disable( __HAL_PB_ENC );
}

/*********************************************************************//**
 * @brief		All Sensor OFF
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void hal_all_led_off(void)
{
	_hal_sen_position_LED_set(0);
}

/* EOF */
