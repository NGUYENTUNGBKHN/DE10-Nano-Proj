/****************************************************************************/
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2010                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
 * @file hal_led.c
 * @brief LEDの点灯／消灯のハードウェアレイヤー
 * @date 2018.01.26 Created
 */
/****************************************************************************/
#include "systemdef.h"
#include "sub_functions.h"
#include "hal_gpio.h"
#include "hal_cyclonev_rstmgr.h"
#include "common.h"			// モデル定義
#define EXTERN extern

#define EXT
#include "com_ram.c"
#include "usb_ram.c"
#include "jsl_ram.c"


/****************************************************************/
/**
 * @brief GPIO OFF<->ON
 */
/****************************************************************/
static void _Gpio_toggle( UINT16 gpio_id)
{
	if(Gpio_in(gpio_id) == 0)
	{
		Gpio_out(gpio_id, 1);
	}
	else
	{
		Gpio_out(gpio_id, 0);
	}
}
/****************************************************************/
/**
 * @brief BOX SU SIGNAL
 */
/****************************************************************/
u8 __hal_su_select_read(void)
{
	return Gpio_in(__HAL_SU_SELECT);
}
/****************************************************************/
/**
 * @brief External Reset DETECT SIGNAL
 */
/****************************************************************/
u8 __hal_reset_det_read(void)
{
	return Gpio_in(__HAL_DET_RES);
}
/****************************************************************/
/**
 * @brief Power Fail DETECT SIGNAL
 */
/****************************************************************/
u8 __hal_pf_det_read(void)
{
	return Gpio_in(GPIO_VDET);
}
/****************************************************************/
/**
 * @brief RFID Interrupt SIGNAL
 */
/****************************************************************/
u8 __hal_rfid_int_read(void)
{
#if (BV_UNIT_TYPE==ES_MODEL) && defined(PRJ_IVIZION2)
	// I2CなのでGPIOみれない
	return 0;
#else
	//return Gpio_in(__HAL_RFID_INT)
	return 0;
#endif
}
/****************************************************************/
/**
 * @brief SD Card set DETECT SIGNAL
 */
/****************************************************************/
u8 __hal_sd_card_dect_read(void)
{
	return Gpio_in(__HAL_SD_SET);
}

/****************************************************************/
/**
 * @brief [DEBUG]SD Card DETECT SIGNAL OUTPUT TOGGLE
 */
/****************************************************************/
void __hal_sd_card_dect_toggle(void)
{
	_Gpio_toggle(__HAL_SD_SET);
}

#if (BV_UNIT_TYPE == ES_MODEL) && defined(PRJ_IVIZION2)
/****************************************************************/
/**
 * @brief 24V Voltage DETECT SIGNAL
 */
/****************************************************************/
u8 __hal_24v_dect_read(void)
{
	return Gpio_in(__HAL_24V_DET);
}
#elif (BV_UNIT_TYPE >= WS_MODEL) && defined(PRJ_IVIZION2)
/****************************************************************/
/**
 * @brief Internal 5V Power work or not SIGNAL
 */
/****************************************************************/
u8 __hal_5v_dect_read(void)
{
	return Gpio_in(__HAL_5V_PG_N);
}
/****************************************************************/
/**
 * @brief RS232/PC SETTING SIGNAL
 */
/****************************************************************/
u8 __hal_if_set_read(void)
{
	return Gpio_in(__HAL_IF_SET);
}
#endif

/****************************************************************/
/**
 * @brief FUSB DETECT SIGNAL
 */
/****************************************************************/
u8 __hal_fusb_dect_read(void)
{
	return Gpio_in(__HAL_FUSB_ONOFF);
}

/****************************************************************/
/**
 * @brief FUSB DETECT SIGNAL
 */
/****************************************************************/
u8 is_fusb_dect_on(void)
{
	if(__hal_fusb_dect_read() == 0)
	{
		/* FRONT USB CONNECTED */
		return 1;
	}
	return 0;
}

/****************************************************************/
/**
 * @brief EXTERNAL INHIBIT SIGNAL
 */
/****************************************************************/
u8 __hal_external_inhibit_read(void)
{
	return Gpio_in(GPIO_TEST_TRIGGER);
}
/****************************************************************/
/**
 * @brief EXTERNAL INHIBIT SIGNAL
 */
/****************************************************************/
u8 is_external_inhibit_on(void)
{
	if(__hal_external_inhibit_read() == 0)
	{
		/* EXTERNAL INHIBIT */
		return 1;
	}
	return 0;
}

/****************************************************************/
/**
 * @brief Centering open sensor interrupt
 */
/****************************************************************/
#if defined(PRJ_IVIZION2)
void _hal_cen_open_interrupt_enable(void)
{
	GpioIsr_enable(GPIO_SEN_CEN_OPEN);
}
void _hal_cen_open_interrupt_disable(void)
{
	GpioIsr_disable(GPIO_SEN_CEN_OPEN);
}
/****************************************************************/
/**
 * @brief Centering open sensor interrupt
 */
/****************************************************************/
void _hal_cen_close_interrupt_enable(void)
{
	GpioIsr_enable(GPIO_SEN_CEN_CLOSE);
}
void _hal_cen_close_interrupt_disable(void)
{
	GpioIsr_disable(GPIO_SEN_CEN_CLOSE);
}
#endif

/****************************************************************/
/**
 * @brief UART SELECT
 * 			IF_SEL1	IF_SEL2
 * PC		H		H
 * RS232C	L		H
 * CCTALK	H		L
 * OFF		L		L
 */
/****************************************************************/
void __hal_if_select_pc(void)
{
	Gpio_out( GPIO_IF_SEL1, 1 );
	Gpio_out( GPIO_IF_SEL2, 1 );
}
void __hal_if_select_rs232c(void)
{
	Gpio_out( GPIO_IF_SEL1, 0 );
	Gpio_out( GPIO_IF_SEL2, 1 );
}
void __hal_if_select_cctalk(void)
{
	Gpio_out( GPIO_IF_SEL1, 1 );
	Gpio_out( GPIO_IF_SEL2, 0 );
}
void __hal_if_select_off(void)
{
	Gpio_out( GPIO_IF_SEL1, 0 );
	Gpio_out( GPIO_IF_SEL2, 0 );
}


/*******************************
        initialize jsl ware GPIO
 *******************************/
extern void _intr_centering_motor_close(void *arg);
extern void _intr_centering_motor_open(void *arg);
extern void _intr_stacker_motor_home(void *arg);



void set_gpio_irq(void)
{
#if 1//#if defined(PRJ_IVIZION2)
	GPIO_ISR_PARAM gpio_iprm;
	memset( (void *)&gpio_iprm, 0, sizeof(gpio_iprm) );
	gpio_iprm.cb_isr_arg = NULL;
#else
	TARGET_PROJECT_NOT_DEFINED_ERROR
#endif
}

/**
 * @brife 入出力ポート初期設定
**/
void setup_gpio(void)
{
	Gpio_init(NULL);
	RstmgrGpio0Reset();
	RstmgrGpio1Reset();
	RstmgrGpio2Reset();

#if 1//#if defined(PRJ_IVIZION2)
	// Set Initial Value for OUTPUT
#if (FIX_FRONT_USB_USE==1)
	// default
	Gpio_out(__HAL_FUSB_SELECT, USB_SELECT_FRONT);
#else
	Gpio_out(__HAL_FUSB_SELECT, USB_SELECT_REAR);
#endif
	Gpio_out(__HAL_STR_LEDONOFF, 0);
	Gpio_out(__HAL_PSLED_ONOFF, 0);
	Gpio_out( __HAL_USB0_RESET, RESET_ASSERT );
	Gpio_out( __HAL_USB1_RESET, RESET_ASSERT );
	Gpio_out(__HAL_SPI0_STB, 1);
	// Set Output Mode
	if( Gpio_mode(__HAL_USB0_RESET, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_USB1_RESET, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_FUSB_SELECT, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_IF_SEL2, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_PSLED_ONOFF, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_STR_LEDONOFF, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_IF_SEL1, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	#if defined(PRJ_IVIZION2)
	if( Gpio_mode(__HAL_IF_LED, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	#else
	if( Gpio_mode(__HAL_GPIO48, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	#endif
	// Set Input Mode
	if( Gpio_mode(__HAL_EXIT_PS, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_PBIN_PS, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_SU_SELECT, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_PBOUT_PS, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
#if defined(PRJ_IVIZION2)
	if( Gpio_mode(__HAL_N_FULL_PS, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
#endif

	if( Gpio_mode(__HAL_BOX1_PS, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
#if defined(PRJ_IVIZION2)
	if( Gpio_mode(__HAL_BOX2_PS, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
#else
	//UBA_GPIO 同じ番号で、ivizionはBox
	if( Gpio_mode(__HAL_PB_HOME, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_SHUTTER_HOME, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
#endif
	if( Gpio_mode(__HAL_CENT_PS, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_ENT_PS, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};

#if defined(PRJ_IVIZION2)
	if( Gpio_mode(__HAL_IF_SET, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};

	// Set Input Mode(Interrupt)
	if( Gpio_mode(GPIO_SEN_CEN_CLOSE, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
#endif

	if( Gpio_mode(GPIO_SEN_CEN_OPEN, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(GPIO_SEN_BOX_HOM, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_FUSB_ONOFF, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_DET_RES, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_SD_SET, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_PF, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
#if (BV_UNIT_TYPE == ES_MODEL) && defined(PRJ_IVIZION2)
	if( Gpio_mode(__HAL_24V_DET, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		_main_system_error(1, 1);
		return;
	};
#endif
	if( Gpio_mode(GPIO_SPI0_STB, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
#else
	TARGET_PROJECT_NOT_DEFINED_ERROR
#endif
	//
	set_gpio_irq();
}

/*********************************************************************//**
 * @brief		Initialize Front USB Connect detect proc
 * @param[in]	u32 high_edge
 * @return 		None
 **********************************************************************/
void init_intr_fusb_dect(u32 high_edge)
{
	GPIO_ISR_PARAM gpio_iprm;
	memset( (void *)&gpio_iprm, 0, sizeof(gpio_iprm) );
	if(high_edge)
	{
		gpio_iprm.attr = GPIO_ATTR_HIGH_EDGE;
	}
	else
	{
	gpio_iprm.attr = GPIO_ATTR_LOW_EDGE;
	}
	gpio_iprm.cb_isr_func = _intr_fusb_dect;
	gpio_iprm.cb_isr_arg = NULL;
	GpioIsr_open(__HAL_FUSB_ONOFF, &gpio_iprm);

	GpioIsr_enable(__HAL_FUSB_ONOFF);
}
void clear_intr_fusb_dect(void)
{
	GpioIsr_clear(__HAL_FUSB_ONOFF);
}


/*********************************************************************//**
 * @brief		Initialize Test Switch Connect detect proc
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_test_sw(void)
{
#if 0
	GPIO_ISR_PARAM gpio_iprm;
	memset( (void *)&gpio_iprm, 0, sizeof(gpio_iprm) );
	Gpio_out( GPIO_TEST_LED, 0 );
	// Set Output Mode
	if( Gpio_mode(GPIO_TEST_LED, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	// Set intput Mode
	if( Gpio_mode(GPIO_TEST_TRIGGER, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	gpio_iprm.attr = GPIO_ATTR_LOW_EDGE;
	gpio_iprm.cb_isr_arg = NULL;
	gpio_iprm.cb_isr_func = _intr_test_sw;
	GpioIsr_open(GPIO_TEST_TRIGGER, &gpio_iprm);
	GpioIsr_enable(GPIO_TEST_TRIGGER);
#endif
}



/*********************************************************************//**
 * @brief		Initialize Power fail detect proc
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_power_fail(void)
{
#if 0
	GPIO_ISR_PARAM gpio_iprm;
	memset( (void *)&gpio_iprm, 0, sizeof(gpio_iprm) );
	gpio_iprm.cb_isr_arg = NULL;
	/* power fail intr */
	gpio_iprm.attr = GPIO_ATTR_LOW_EDGE;
	gpio_iprm.cb_isr_func = _intr_low_voltage;
	GpioIsr_open(GPIO_VDET, &gpio_iprm);
	GpioIsr_disable(GPIO_VDET);
#endif
}
/* End of file */
