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
	return Gpio_in(GPIO_EXT_RESET);
}

/****************************************************************/
/**
 * @brief Internal 5V Power work or not SIGNAL
 */
/****************************************************************/
u8 __hal_5v_dect_read(void) //後ろの電源を検知
{
	return Gpio_in(__HAL_5V_PG_N);
}


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

void __hal_if_select_rs232c(void)
{
	Gpio_out( GPIO_IF_SEL2, 1 ); //cc-talkへの切り替え Hihg ID-003, Low-cc-talk
}

/*******************************
        initialize jsl ware GPIO
 *******************************/



extern void _intr_apb_motor_home(void *arg);
extern void _intr_pb_encooder(void *arg);

void set_gpio_irq(void)
{

	GPIO_ISR_PARAM gpio_iprm;
	memset( (void *)&gpio_iprm, 0, sizeof(gpio_iprm) );
	gpio_iprm.cb_isr_arg = NULL;

	/* PB EncoderとHomeでは、CPU側とFPGA側で異なる可能性がある*/
	//UBA_WS1 PB encdoer割り込みがFPGA->GPIOへ 2022-09-27a
	gpio_iprm.attr = GPIO_ATTR_HIGH_EDGE;
//	gpio_iprm.cb_isr_func = _pl_intr_pb_encooder;
	gpio_iprm.cb_isr_func = _intr_pb_encooder;

	GpioIsr_open(__HAL_PB_ENC, &gpio_iprm);
	GpioIsr_disable(__HAL_PB_ENC);

	//参考　FPGA割り込みからの割り込みの時
	//OSW_ISR_create( &hplIsr38, PB_ENC_IRQ_INTR_ID, _pl_intr_pb_encooder);
	//OSW_ISR_set_priority(PB_ENC_IRQ_INTR_ID, IPL_USER_HIGHEST);
	//set_int_typ(PB_ENC_IRQ_INTR_ID, IRQ_EDGE_RISING);

}

/**
 * @brife 入出力ポート初期設定
**/
void setup_gpio(void)
{
	Gpio_init(NULL);


	// Set Initial Value for OUTPUT
#if 0//#if (FIX_FRONT_USB_USE==1)
	// default
	Gpio_out(__HAL_FUSB_SELECT, USB_SELECT_FRONT);
#else
	Gpio_out(__HAL_FUSB_SELECT, USB_SELECT_REAR);
#endif
	Gpio_out(GPIO_53, 0);	//not use 糸検知センサONだったが廃止された
	Gpio_out(__HAL_PSLED_ONOFF, 0);
	Gpio_out( __HAL_USB0_RESET, RESET_ASSERT );
	Gpio_out( __HAL_USB2_RESET, RESET_ASSERT );
	Gpio_out(__HAL_SPI0_STB, 1);
	// Set Output Mode
	if( Gpio_mode(__HAL_USB0_RESET, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_USB2_RESET, GPIO_MODE_OUTPUT) == FALSE ){
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
	if( Gpio_mode(__HAL_IF_SEL2, GPIO_MODE_OUTPUT) == FALSE ){ //cc-talkへの切り替え Hihg ID-003, Low-cc-talk
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
	if( Gpio_mode(GPIO_53, GPIO_MODE_OUTPUT) == FALSE ){	//not use 糸検知センサONだったが廃止された
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	#if 1//
	if( Gpio_mode(__HAL_GPIO48, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	#else
	if( Gpio_mode(__HAL_IF_LED, GPIO_MODE_OUTPUT) == FALSE ){
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
	if( Gpio_mode(GPIO_26, GPIO_MODE_INPUT) == FALSE ){ //RTQのReady信号　HighでReady RTQが動作できる状態
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(GPIO_66, GPIO_MODE_OUTPUT) == FALSE ){ // LowでRTQに対してReset, HighでReset解除
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_BOX1_PS, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_PB_HOME, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};

	if( Gpio_mode(__HAL_HOME_LED, GPIO_MODE_OUTPUT) == FALSE ){
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

#if 0//
	if( Gpio_mode(__HAL_IF_SET, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
#endif

	if( Gpio_mode(__HAL_CENT_OPEN_PS, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(__HAL_BOX_HOME_PS, GPIO_MODE_INPUT) == FALSE ){
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
	#if 0 //2024-01-31
	if( Gpio_mode(__HAL_SD_SET, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	#endif
	if( Gpio_mode(__HAL_PF, GPIO_MODE_INPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(GPIO_SPI0_STB, GPIO_MODE_OUTPUT) == FALSE ){
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	if( Gpio_mode(GPI_01, GPIO_MODE_INPUT) == FALSE ){	/* PB ENC	*/
		osw_printf( "Gpio_mode() - Error\n" );
		/* system error */
		program_error();
		return;
	};
	//
	set_gpio_irq();

	#if defined(UBA_RTQ)
	Gpio_out( GPIO_RC_RESET, GPIO_PIN_HIGH ); //Reset解除 (RTQに対してResetが効いたのでResetを解除) //2025-07-11
	#endif	
}

/*********************************************************************//**
 * @brief		Initialize Power fail detect proc
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_power_fail(void)
{
	GPIO_ISR_PARAM gpio_iprm;
	memset( (void *)&gpio_iprm, 0, sizeof(gpio_iprm) );
	gpio_iprm.cb_isr_arg = NULL;
	/* power fail intr */
	gpio_iprm.attr = GPIO_ATTR_LOW_EDGE;
	gpio_iprm.cb_isr_func = _intr_low_voltage;
	GpioIsr_open(GPIO_VDET, &gpio_iprm);
	GpioDeb_enable(GPIO_VDET);
	GpioIsr_enable(GPIO_VDET);
}

/*********************************************************************//**
 * @brief		Initialize External Reset detect proc
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_external_reset(void)
{
	volatile u8 value;
	GPIO_ISR_PARAM gpio_iprm;
	memset( (void *)&gpio_iprm, 0, sizeof(gpio_iprm) );
	gpio_iprm.cb_isr_arg = NULL;
	/* power fail intr */
	gpio_iprm.attr = GPIO_ATTR_LOW_EDGE;
	//gpio_iprm.attr = GPIO_ATTR_HIGH_EDGE;
	gpio_iprm.cb_isr_func = _intr_external_reset;
	GpioIsr_open(GPIO_EXT_RESET, &gpio_iprm);
	GpioIsr_enable(GPIO_EXT_RESET);

	value = __hal_reset_det_read();
}
/* End of file */
