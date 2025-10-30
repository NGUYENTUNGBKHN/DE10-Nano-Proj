/******************************************************************************/
/*! @addtogroup Group1
    @file       hal_sensor.h
    @brief      Sensor control header
    @date       2012/10/16
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2012/10/15 H.Suzuki
      -# Initial Version
******************************************************************************/


/******************************************************
					hal_sensor.c
******************************************************/
/* position sensor raw data select */
enum{
	SELECT_POSI_AD_ENTRANCE,	/* Entrance sensor		*/
	SELECT_POSI_AD_CENTERING,	/* Centering sensor		*/
	SELECT_POSI_AD_APB_IN,		/* APB-IN sensor		*/
	SELECT_POSI_AD_APB_OUT,		/* APB-OUT sensor		*/
	SELECT_POSI_AD_EXIT,		/* Exit sensor			*/
};

/* Public Functions ----------------------------------------------------------- */
void _hal_sen_position_LED_set(u8 set);
u8 _hal_sen_feed_encoder_status_read(void);
void _hal_sen_select_position_sensor(u8 select);
u16 _hal_sen_pos_sensor_read(void);
void _hal_position_sensor_on(void);
void _hal_position_intr_on(void);
void _hal_position_sensor_off(void);
void hal_all_led_off(void);
u32 hal_position_sensor_gain(u8 gain);



/* EOF */
