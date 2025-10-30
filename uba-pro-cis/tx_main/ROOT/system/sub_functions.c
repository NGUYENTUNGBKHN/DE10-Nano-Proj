/******************************************************************************/
/*! @addtogroup Group2
    @file       if_main.c
    @brief      Main process for I/F
    @date       2018/02/26
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018-2019 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
******************************************************************************/

#include "kernel.h"
#include "kernel_inc.h"
#include "MP_SCU.h"
#include "soc_cv_av/alt_reset_manager.h"
#include "alt_interrupt.h"
#include "alt_cache.h"
#include "common.h"
#include "operation.h"
#include "sub_functions.h"
#include "hal.h"
#include "hal_gpio.h"
#include "hal_i2c_iox.h"
#include "hal_i2c_adc.h"
#include "cyclonev_sysmgr_reg_def.h"
#include "hal_cyclonev_rstmgr.h"
#include "hal_spi_fram.h"
#include "pl/pl_uv.h"
#include "pl/pl_cis.h"
#include "user/icb/icb.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "usb_ram.c"
#include "cis_ram.c"


/************************** External function *****************************/
/************************** Variable declaration *****************************/
struct _SYSTEM_ERROR
{
	u16 taskid;
	u16 errno;
	u16 tmsg;
	u16 submsg;
};
struct _SYSTEM_ERROR _debug_system_error_val;
/* reset manager */
#define MPUMODRST             0xFFD05010U
#define MPUMODRST_CPU1        (1<<1)
#define MPUMODRST_WDT         (1<<2)
#define MPUMODRST_SCU         (1<<3)
#define MPUMODRST_L2          (1<<4)
/*********************************************************************//**
 * @brief		reset mpu module
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void reset_mpu_module(void)
{
	/* cpu1 reset */
	static volatile uint32_t *const mpumodrst = (uint32_t *) MPUMODRST;
	*mpumodrst |= ( MPUMODRST_CPU1 );

	return;
}

/*********************************************************************//**
 * @brief		program_error
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void program_error(void)
{
	// エラー、リセット
	_soft_reset();
}


#define ALT_QSPI_TIMEOUT_INFINITE (0xffffffff)
#define ALT_QSPI_STIG_OPCODE_N25Q512A_RESET_EN             (0x66)
#define ALT_QSPI_STIG_OPCODE_N25Q512A_RESET_MEM            (0x99)
extern ALT_STATUS_CODE alt_qspi_stig_cmd(uint32_t opcode, uint32_t dummy, uint32_t timeout);
static ALT_STATUS_CODE qspi_software_reset(void)
{
    ALT_STATUS_CODE status = ALT_E_SUCCESS;

    /* Reset the volatile memory on the N25Q */

    if (status == ALT_E_SUCCESS)
    {
        status = alt_qspi_stig_cmd(ALT_QSPI_STIG_OPCODE_N25Q512A_RESET_EN, 0, ALT_QSPI_TIMEOUT_INFINITE);
    }

    /* Reset memory */
    if (status == ALT_E_SUCCESS)
    {
        status = alt_qspi_stig_cmd(ALT_QSPI_STIG_OPCODE_N25Q512A_RESET_MEM, 0, ALT_QSPI_TIMEOUT_INFINITE);
    }
    return status;
}
/*********************************************************************//**
 * @brief		MAIN CPU self reset
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void cold_reset_core0(void)
{
	/*
	 *  reset QSPI module before cold reset
	 *  https://rocketboards.org/foswiki/Documentation/SocBoardQspiBoot
	 */
	qspi_software_reset();
	/* CPU Cold Reset */
	alt_reset_cold_reset();
}
void cold_reset_core1(void)
{
	ALT_STATUS_CODE result;
	OSW_ISR_HANDLE hSgiIsr1;
	/* For Dual Core Cold Reset Interrupt */
	OSW_ISR_create( &hSgiIsr1, OSW_INT_SGI1, cold_reset_core0);
	OSW_ISR_enable( OSW_INT_SGI1 );
	while(1)
	{
		result = alt_int_sgi_trigger(ALT_INT_INTERRUPT_SGI1, ALT_INT_SGI_TARGET_ALL_EXCL_SENDER, (alt_int_cpu_target_t)0, true);
		if(result == ALT_E_SUCCESS)
		{
			break;
		}
	}
	go_to_sleep();
}
void _soft_reset(void)
{
	if( get_core_id() == 0 )
	{
		cold_reset_core0();
	}
	else
	{
		cold_reset_core1();
	}
	while( 1 );
}
void sample_ictrl_callback_SGI1_cold_reset(uint32_t icciar, void * context)
{
	cold_reset_core0();
}
void bif_if_download_core0(void)
{
	/* cpu1 reset */
	reset_mpu_module();
	OSW_ISR_disable( OSW_INT_SGI2 );
	alt_cache_system_disable();
	/* BIFへジャンプ */
	if(ex_download_start_flg == DL_START_NORMAL)
	{
		bif_if_download();
	}
	else
	{
		bif_if_diff_download();
	}
}
void bif_if_download_core1(void)
{
	ALT_STATUS_CODE result;
	OSW_ISR_HANDLE hSgiIsr2;
	/* For Dual Core Download Request Interrupt */
	OSW_ISR_create( &hSgiIsr2, OSW_INT_SGI2, bif_if_download_core0);
	OSW_ISR_enable( OSW_INT_SGI2 );
	while(1)
	{
		result = alt_int_sgi_trigger(ALT_INT_INTERRUPT_SGI2, ALT_INT_SGI_TARGET_ALL_EXCL_SENDER, (alt_int_cpu_target_t)0, true);
		if(result == ALT_E_SUCCESS)
		{
			break;
		}
	}
	go_to_sleep();
}
void bif_if_download_smp(void)
{
	if( get_core_id() == 0 )
	{
		bif_if_download_core0();
	}
	else
	{
		bif_if_download_core1();
	}
	while( 1 );
}
void sample_ictrl_callback_SGI2_bif_download(uint32_t icciar, void * context)
{
	bif_if_download_core0();
}

// usb device download
void bif_device_usb_download_core0(void)
{
	/* cpu1 reset */
	reset_mpu_module();
	OSW_ISR_disable( OSW_INT_SGI3 );
	alt_cache_system_disable();
	/* BIFへジャンプ */
	bif_device_usb_download();
}
void bif_device_usb_download_core1(void)
{
	ALT_STATUS_CODE result;
	OSW_ISR_HANDLE hSgiIsr3;
	/* For Dual Core Download Request Interrupt */
	OSW_ISR_create( &hSgiIsr3, OSW_INT_SGI3, bif_device_usb_download_core0);
	OSW_ISR_enable( OSW_INT_SGI3 );
	while(1)
	{
		result = alt_int_sgi_trigger(ALT_INT_INTERRUPT_SGI3, ALT_INT_SGI_TARGET_ALL_EXCL_SENDER, (alt_int_cpu_target_t)0, true);
		if(result == ALT_E_SUCCESS)
		{
			break;
		}
	}
	go_to_sleep();
}
void bif_device_usb_download_smp(void)
{
	if( get_core_id() == 0 )
	{
		bif_device_usb_download_core0();
	}
	else
	{
		bif_device_usb_download_core1();
	}
	while( 1 );
}
void sample_ictrl_callback_SGI3_bif_device_usb_download(uint32_t icciar, void * context)
{
	bif_device_usb_download_core0();
}
// usb host download
void bif_host_usb_download_core0(void)
{
	/* cpu1 reset */
	reset_mpu_module();
	OSW_ISR_disable( OSW_INT_SGI4 );
	alt_cache_system_disable();
	/* BIFへジャンプ */
	bif_host_usb_download();
}
void bif_host_usb_download_core1(void)
{
	ALT_STATUS_CODE result;
	OSW_ISR_HANDLE hSgiIsr4;
	/* For Dual Core Download Request Interrupt */
	OSW_ISR_create( &hSgiIsr4, OSW_INT_SGI4, bif_host_usb_download_core0);
	OSW_ISR_enable( OSW_INT_SGI4 );
	while(1)
	{
		result = alt_int_sgi_trigger(ALT_INT_INTERRUPT_SGI4, ALT_INT_SGI_TARGET_ALL_EXCL_SENDER, (alt_int_cpu_target_t)0, true);
		if(result == ALT_E_SUCCESS)
		{
			break;
		}
	}
	go_to_sleep();
}

void sample_ictrl_callback_SGI4_bif_host_usb_download(uint32_t icciar, void * context)
{
	bif_host_usb_download_core0();
}
void bif_host_usb_download_smp(void)
{
	if( get_core_id() == 0 )
	{
		bif_host_usb_download_core0();
	}
	else
	{
		bif_host_usb_download_core1();
	}
	while( 1 );
}


void set_test_mode(void)
{
	ex_operating_mode |= OPERATING_MODE_TEST;
}


bool is_test_mode(void)
{
	if ((ex_operating_mode & OPERATING_MODE_TEST) == OPERATING_MODE_TEST)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_ld_mode(void)
{
	if (is_test_mode())
	{
		if ((ex_operating_mode & OPERATING_MODE_TEST_LD) == OPERATING_MODE_TEST_LD)
		{
			return true;
		}
	}
	else
	{
		if ((ex_system & BIT_LD_UNIT) == BIT_LD_UNIT)
		{
			return true;
		}
	}
	return false;
}


void set_test_ld_mode(u8 set)
{
	if ((is_test_mode()) && (set))
	{
		ex_operating_mode |= OPERATING_MODE_TEST_LD;
	}
	else
	{
		ex_operating_mode &= ~(OPERATING_MODE_TEST_LD);
	}
}


void set_test_allacc_mode(u8 set)
{
	if ((is_test_mode()) && (set))
	{
		ex_operating_mode |= OPERATING_MODE_TEST_ALL_ACCEPT;
	}
	else
	{
		ex_operating_mode &= ~(OPERATING_MODE_TEST_ALL_ACCEPT);
	}
}


void set_test_allrej_mode(u8 set)
{
	if ((is_test_mode()) && (set))
	{
		ex_operating_mode |= OPERATING_MODE_TEST_ALL_REJECT;
	}
	else
	{
		ex_operating_mode &= ~(OPERATING_MODE_TEST_ALL_REJECT);
	}
}

u16 calc_check_sum(u8 *ptr, u16 length)
{
	u16	sum = 0;
	u16 cnt;

	for (cnt = 0; cnt < length; cnt++)
	{
		sum += *((u8 *)(ptr + cnt));
	}
	return (~sum);
}


u16 culc_fram_adj_sum(void) //CIS以外とCIS領域の合計sum
{
	u16 sum = 0;
	u8 *ptr;
	u32 size;

	// info
	ptr = (u8 *)&ex_adjustment_data;  //CIS以外
	size = sizeof(ADJUSTMENT_DATA);
	for(int cnt = 0; cnt < size; cnt++)
	{
		sum += *ptr;
		ptr++;
	}
	// cis data
	ptr = (u8 *)&ex_cis_adjustment_data; //CIS領域
	size = sizeof(CIS_ADJUSTMENT_DATA);
	for(int cnt = 0; cnt < size; cnt++)
	{
		sum += *ptr;
		ptr++;
	}

	return sum;
}
u16 culc_fram_tmpadj_sum(void)
{
	u16 sum = 0;
	u8 *ptr;
	u32 size;

	// pos da
	ptr = (u8 *)&ex_position_da_adj;
	size = FRAM_ADJ_TMP_POS_DA_SIZE;
	for(int cnt = 0; cnt < size; cnt++)
	{
		sum += *ptr;
		ptr++;
	}
	// pos ga
	ptr = (u8 *)&ex_position_ga;
	size = FRAM_ADJ_TMP_POS_GAIN_SIZE;
	for(int cnt = 0; cnt < size; cnt++)
	{
		sum += *ptr;
		ptr++;
	}

	return sum;
}
u16 culc_fram_postmp_sum(void)
{
	u16 sum = 0;
	u8 *ptr;
	u32 size;

	// pos da & ga
	ptr = (u8 *)&ex_position_tmp;
	size = FRAM_ADJ_TMP_POS_DA_SIZE + FRAM_ADJ_TMP_POS_GAIN_SIZE;
	for(int cnt = 0; cnt < size; cnt++)
	{
		sum += *ptr;
		ptr++;
	}

	return sum;
}
u32 write_fram_tmpadj(void) //待機時補正
{
	u32 fram_result;
	u16 sum;

	sum = culc_fram_tmpadj_sum();

	ex_position_tmp.tmp_entrance = ex_position_da_adj.entrance;
	ex_position_tmp.tmp_centering = ex_position_da_adj.centering;
	ex_position_tmp.tmp_apb_in = ex_position_da_adj.apb_in;
	ex_position_tmp.tmp_apb_out = ex_position_da_adj.apb_out;
	ex_position_tmp.tmp_exit = ex_position_da_adj.exit;
	ex_position_tmp.tmp_ga = ex_position_ga;
	memcpy(&ex_position_tmp.tmp_sum, &sum, FRAM_ADJ_TMP_POS_SUM_SIZE);
	memcpy(&ex_position_tmp_bk, &ex_position_tmp, sizeof(POS_ADJUSTMENT_TMP));

	fram_result = _hal_write_fram_adj_tmp(FRAM_ADJ_TMP_ADR, (u8 *)&ex_position_tmp, FRAM_ADJ_TMP_POS_DA_SIZE + FRAM_ADJ_TMP_POS_GAIN_SIZE + FRAM_ADJ_TMP_POS_SUM_SIZE);
	// SPI access error
	if (fram_result != SUCCESS)
	{
		// DONE:Set Error
		return ALARM_CODE_SPI;
	}
	fram_result = _hal_write_fram_adj_tmp(FRAM_ADJ_TMP_BK_ADR, (u8 *)&ex_position_tmp_bk, FRAM_ADJ_TMP_POS_BK_DA_SIZE + FRAM_ADJ_TMP_POS_BK_GAIN_SIZE + FRAM_ADJ_TMP_POS_BK_SUM_SIZE);
	// SPI access error
	if (fram_result != SUCCESS)
	{
		// DONE:Set Error
		return ALARM_CODE_SPI;
	}
	return ALARM_CODE_OK;
}
u8 check_fram_tmpadj_sum(void) //電源ON時の調整値確認
{
	u16 sum = 0;

	// check tmpadj
	ex_position_da_adj.entrance = ex_position_tmp.tmp_entrance;
	ex_position_da_adj.centering = ex_position_tmp.tmp_centering;
	ex_position_da_adj.apb_in = ex_position_tmp.tmp_apb_in;
	ex_position_da_adj.apb_out = ex_position_tmp.tmp_apb_out;
	ex_position_da_adj.exit = ex_position_tmp.tmp_exit;
	ex_position_ga = ex_position_tmp.tmp_ga;
	sum = culc_fram_tmpadj_sum();
	if (sum == ex_position_tmp.tmp_sum)
	{
		if(( ex_position_da_adj.entrance != 0 )
		 && ( ex_position_da_adj.centering != 0 )
		 && ( ex_position_da_adj.apb_in != 0 )
		 && ( ex_position_da_adj.apb_out != 0 )
		 && ( ex_position_da_adj.exit != 0 )
		){
			return ALARM_CODE_OK;
		}
	}
	// check tmpadj(backup)
	ex_position_da_adj.entrance = ex_position_tmp_bk.tmp_entrance;
	ex_position_da_adj.centering = ex_position_tmp_bk.tmp_centering;
	ex_position_da_adj.apb_in = ex_position_tmp_bk.tmp_apb_in;
	ex_position_da_adj.apb_out = ex_position_tmp_bk.tmp_apb_out;
	ex_position_da_adj.exit = ex_position_tmp_bk.tmp_exit;
	ex_position_ga = ex_position_tmp_bk.tmp_ga;
	sum = culc_fram_tmpadj_sum();
	if (sum == ex_position_tmp_bk.tmp_sum)
	{
		if(( ex_position_da_adj.entrance != 0 )
		 && ( ex_position_da_adj.centering != 0 )
		 && ( ex_position_da_adj.apb_in != 0 )
		 && ( ex_position_da_adj.apb_out != 0 )
		 && ( ex_position_da_adj.exit != 0 )
		){
			return ALARM_CODE_OK;
		}
	}
	// DONE:Set Error
	return ALARM_CODE_FRAM;
}

bool is_all_accept_mode(void)
{
	if ((is_test_mode())
	&& ((ex_operating_mode & OPERATING_MODE_TEST_ALL_ACCEPT) == OPERATING_MODE_TEST_ALL_ACCEPT))
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool is_all_reject_mode(void)
{
	if ((is_test_mode())
	&& ((ex_operating_mode & OPERATING_MODE_TEST_ALL_REJECT) == OPERATING_MODE_TEST_ALL_REJECT))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/*********************************************************************//**
 * @brief is box set
 * @param[in]	None
 * @return 		true  : set box
 * 				false : unset box
 **********************************************************************/
bool is_box_set(void)
{
	if ((is_ld_mode()) || (SENSOR_BOX))
	{
		return true;
	}
	return false;
}

/*********************************************************************//**
 * @brief is point uv set
 * @param[in]	None
 * @return 		true  : unset point uv sensor
 * 				false : set point uv sensor
 **********************************************************************/
bool is_uv_led_check(void)
{
#if 1//#if (DEBUG_DISABLE_UV_CHECK==1)
	//ivizon2は使用しているかもしれないが、UBA700は使用しない
	return false;
#else
	// A/Dサンプリング後に呼び出す必要がある
#if (POINT_UV1_ENABLE==1)
	if(!_pl_uv0_check())
	{
		return true;
	}
#endif
#if (POINT_UV2_ENABLE==1)
	if(!_pl_uv1_check())
	{
		return true;
	}
#endif
	return false;
#endif
}

/*********************************************************************//**
 * @brief is CISA LVDS lock
 * @param[in]	None
 * @return 		true  : set CIS
 * 				false : unset CIS
 **********************************************************************/
bool is_cisa_lvds_lock(void)
{
	u32 retry = 10;
	while(!_pl_cisa_lock_check())
	{
		if(retry-- == 0)
		{
			return false;
		}
		OSW_TSK_sleep(10);
	}
	return true;
}

/*********************************************************************//**
 * @brief is CISB LVDS lock
 * @param[in]	None
 * @return 		true  : set CIS
 * 				false : unset CIS
 **********************************************************************/
bool is_cisb_lvds_lock(void)
{
	u32 retry = 10;
	while(!_pl_cisb_lock_check())
	{
		if(retry-- == 0)
		{
			return false;
		}
		OSW_TSK_sleep(10);
	}
	return true;
}

/***********************************************************************
  Hardware dependent Device Initialization
 **********************************************************************/
#define I2C_ENABLE				0x6C
void __hal_i2c0_soft_reset(void)
{
	u32 i;
	// I2C0 disable
	IOREG32(I2C0_BASE,I2C_ENABLE) = 0x0;
	for( i = 0 ; i < 1000 ; i ++ ){
		if( (IOREG32(I2C0_BASE,I2C_ENABLE) & 0x1) == 0 ) break;
	}
	// GPIO55-56:FUNCTION:GPIO
	IOREG32(SYSMGR_BASE,SYS_GENERALIO7) = (UINT32)0x0;
	IOREG32(SYSMGR_BASE,SYS_GENERALIO8) = (UINT32)0x0;
	Gpio_mode(GPIO_55, GPIO_MODE_OUTPUT);
	Gpio_mode(GPIO_56, GPIO_MODE_OUTPUT);
	Gpio_out(GPIO_55, 0);
	Gpio_out(GPIO_56, 0);
	dly_tsk(1);
	Gpio_out(GPIO_55, 1);
	Gpio_out(GPIO_56, 1);
	// GPIO55-56:FUNCTION:I2C0
	IOREG32(SYSMGR_BASE,SYS_GENERALIO7) = (UINT32)0x1;
	IOREG32(SYSMGR_BASE,SYS_GENERALIO8) = (UINT32)0x1;
}
void __hal_i2c3_soft_reset(void)
{
	u32 i;
	// I2C0 disable
	IOREG32(I2C3_BASE,I2C_ENABLE) = 0x0;
	for( i = 0 ; i < 1000 ; i ++ ){
		if( (IOREG32(I2C3_BASE,I2C_ENABLE) & 0x1) == 0 ) break;
	}
	// GPIO20-21:FUNCTION:GPIO
	IOREG32(SYSMGR_BASE,SYS_MIXED1IO6) = (UINT32)0x0;
	IOREG32(SYSMGR_BASE,SYS_MIXED1IO7) = (UINT32)0x0;
	Gpio_mode(GPIO_20, GPIO_MODE_OUTPUT);
	Gpio_mode(GPIO_21, GPIO_MODE_OUTPUT);
	Gpio_out(GPIO_20, 0);
	Gpio_out(GPIO_21, 0);
	dly_tsk(1);
	Gpio_out(GPIO_20, 1);
	Gpio_out(GPIO_21, 1);
	// GPIO20-21:FUNCTION:I2C3
	IOREG32(SYSMGR_BASE,SYS_MIXED1IO6) = (UINT32)0x1;
	IOREG32(SYSMGR_BASE,SYS_MIXED1IO7) = (UINT32)0x1;
}
u32 i2c0_reset(void)
{
	__hal_i2c0_soft_reset();
	// DONE:reset module(RSTMGR)
	RstmgrI2C0Reset();
	/* ドライバクローズ */
	I2c_close( &hI2c0 );
	// DONE:reopen I2C1(JSL-Ware)
	setup_i2c0();
	return E_OK;
}
u32 i2c3_reset(void)
{
	__hal_i2c3_soft_reset();
	// DONE:reset module(RSTMGR)
	RstmgrI2C3Reset();
	/* ドライバクローズ */
	I2c_close( &hI2c3 );
	// DONE:reopen I2C3(JSL-Ware)
	setup_i2c3();

	_hal_i2c3_init_iox();
	return E_OK;
}

u32 uart1_reset(void)
{
#ifdef JSL_UART
	/* ドライバクローズ */
	Uart_close( &hUart1 );
#endif
	// DONE:reset module(RSTMGR)
	RstmgrUart1Reset();
	// TODO:reopen Uart1(JSL-Ware)
	return E_OK;
}

u8 _main_check_fram_adj_sum(void)
{
	u16 sum = 0;

	if(is_test_mode())
	{
		// not check
		return ALARM_CODE_OK;
	}

	sum = culc_fram_adj_sum(); //CIS以外とCIS領域の合計sum

	// fram checksum (big endian)
	if (sum != ex_fram_sum)
	{
		// DONE:Set Error
		return ALARM_CODE_FRAM;
	}
	return ALARM_CODE_OK;
}
/*******************************
        initialize MIO for test mode
 *******************************/
void _main_init_performance_test_mio(void)
{
#if defined(UBA_RTQ)
	// GPIO61:UART0-RXD
	//IOREG32(SYSMGR_BASE,SYS_GENERALIO13) = (UINT32)0x3;
	// GPIO62:UART0-TXD
	//IOREG32(SYSMGR_BASE,SYS_GENERALIO14) = (UINT32)0x3;
#else
	// GPIO61:UART0-RXD→GPIO
	IOREG32(SYSMGR_BASE,SYS_GENERALIO13) = (UINT32)0x0;
	// GPIO62:UART0-TXD→GPIO
	IOREG32(SYSMGR_BASE,SYS_GENERALIO14) = (UINT32)0x0;
#endif
};
/*--------------------------------------------------------------
*	check STACKER style
*---------------------------------------------------------------
*	引数	:non
*---------------------------------------------------------------
*	戻値	:non
*--------------------------------------------------------------*/
void unit_style_check(void)
{
	int	ii = 0;
	int	flag = SELECT_SS_MODE;
	do
	{
	/*<		wait 1ms	>*/
		OSW_TSK_sleep(1);
		if(SELECT_SS_MODE == __hal_su_select_read())
		{
			if(flag == SELECT_SS_MODE)
			{
				ii += 1;
			}
			else
			{
				flag = SELECT_SS_MODE;
				ii = 0;
			}
		}
		else
		{
			if(flag != SELECT_SS_MODE)
			{
				ii += 1;
			}
			else
			{
				flag = ~SELECT_SS_MODE;
				ii = 0;
			}
		}
	}while(ii < 3);
/*<--->*/
	if(flag == SELECT_SS_MODE)
	{
		ex_system = ((ex_system & ~BIT_STACKER_UNIT_FLAG) | BIT_SS_UNIT);
	}
	else
	{
		ex_system = ((ex_system & ~BIT_STACKER_UNIT_FLAG) | BIT_SU_UNIT);
	}
}

u8 unit_voltage_check_uba(void)
{
	u8 value;

	if(_hal_voltage_ad_read(&value) != E_OK)
	{
		return(0);
	}
	
	return(value);

}
#if defined(UBA_RTQ)
bool is_icb_enable(void)
{
	if( (is_test_mode() == false) && 
		(ex_rc_configuration.rfid_module == CONNECT_RFID) 
		// &&  (check_ICBflag())
		&& (ex_ICB_systemInhiStaus != INHIBIT_ICB) /* 2024-07-05 */
		)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
#else
bool is_icb_enable(void)
{
	if( (is_test_mode() == false) && (ex_ICB_systemInhiStaus != INHIBIT_ICB) && (check_ICBflag()))
	{
	/* use ICB	*/
		return true;
	}
	else
	{
		return false;
	}
}
#endif // 

bool is_legacy_mode_enable(void)
{
	if((ex_mode2_setting.legacy_mode == 0xAA)
	 && (ex_mode2_setting.legacy_mode_chk == 0x55)
	){
		return true;
	}
	else
	{
		return false;
	}
}
/*********************************************************************//**
 * @brief [debug] system error
 * @param[in]	task id
 * 				system error no
 * 				task message code
 * 				argument 1
 * @return 		None
 **********************************************************************/
void _debug_system_error(u16 task_id, u16 error_no, u16 tmsg, u16 arg1, u8 fatal_err)
{
	_debug_system_error_val.taskid = (u16)task_id;
	_debug_system_error_val.errno  = (u16)error_no;
	_debug_system_error_val.tmsg   = tmsg;
	_debug_system_error_val.submsg = arg1;


#if 1
	return;	/* 2022-01-11 */
#elif (DEBUG==1)
	return;
#endif

#ifdef _DEBUG_SYSTEM_ERROR
	if (task_id != ID_DISPLAY_TASK)
	{
		while (1)
		{
			dly_tsk(1000);
		}
	}
#else  /* _DEBUG_SYSTEM_ERROR */
	if (fatal_err)
	{
		if (task_id != ID_DISPLAY_TASK)
		{
			dly_tsk(10000);
		}
		program_error();				/* wait until reset */
	}
#endif /* _DEBUG_SYSTEM_ERROR */
}

