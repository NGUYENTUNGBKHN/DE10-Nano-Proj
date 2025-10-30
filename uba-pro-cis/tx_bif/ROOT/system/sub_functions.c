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
#include "common.h"
#include "sub_functions.h"
#include "soc_cv_av/alt_reset_manager.h"
#include "alt_interrupt.h"
#include "hal_cyclonev_rstmgr.h"
#include "hal_i2c_iox.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "usb_ram.c"

/************************** Function Prototypes ******************************/
/************************** Variable declaration *****************************/
struct _SYSTEM_ERROR
{
	u16 taskid;
	u16 errno;
	u16 tmsg;
	u16 submsg;
};
struct _SYSTEM_ERROR _debug_system_error_val;

/*********************************************************************//**
 * @brief		program_error
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void program_error(void)
{
	// エラー、リセット
#if 1
	_soft_reset();
#else
	while( 1 ){dly_tsk(1000);};
#endif
}

#define ALT_QSPI_TIMEOUT_INFINITE (0xffffffff)
#define ALT_QSPI_STIG_OPCODE_N25Q512A_RESET_EN             (0x66)
#define ALT_QSPI_STIG_OPCODE_N25Q512A_RESET_MEM            (0x99)
extern ALT_STATUS_CODE alt_qspi_stig_cmd(uint32_t opcode, uint32_t dummy, uint32_t timeout);
ALT_STATUS_CODE qspi_software_reset(void)
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
#if 1
	/* clear pending interrupts */
	alt_int_dist_pending_clear(ALT_INT_INTERRUPT_SGI1);
	alt_int_dist_priority_set(ALT_INT_INTERRUPT_SGI1, IPL_USER_NORMAL);
#endif
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
		cold_reset_core0();
		//cold_reset_core1();
	}
	while(1);
}
void sample_ictrl_callback_SGI1_cold_reset(uint32_t icciar, void * context)
{
	cold_reset_core0();
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

/***********************************************************************
  Hardware dependent Device Initialization
 **********************************************************************/
u32 i2c0_reset(void)
{
#if 1
	// DONE:reset module(RSTMGR)
	RstmgrI2C0Reset();
	/* ドライバクローズ */
	I2c_close( &hI2c0 );
	// DONE:reopen I2C1(JSL-Ware)
	setup_i2c0();
#endif
	return E_OK;
}
u32 i2c3_reset(void)
{
#if 1
	// DONE:reset module(RSTMGR)
	RstmgrI2C3Reset();
	/* ドライバクローズ */
	I2c_close( &hI2c3 );
	// DONE:reopen I2C3(JSL-Ware)
	setup_i2c3();

	_hal_i2c3_init_iox();
#endif
	return E_OK;
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

//#if 1	/* '13-10-01 Event Log */
//	ivent_log_no8();
//#endif

#if !defined(PRJ_IVIZION2)//UBA_MUST
	return;	/* 2022-01-11 */
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
