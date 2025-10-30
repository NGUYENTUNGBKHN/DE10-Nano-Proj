/******************************************************************************/
/*! @addtogroup Group1
    @file       pl_cis.h
    @brief      cis sensor header file
    @date       2018/05/24
    @author     yuji-kenta
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/05/24 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
#pragma once

#ifndef _PL_CIS_HEADER_H		/* prevent circular inclusions */
#define _PL_CIS_HEADER_H		/* by using protection macros */

/************************** EXTERNAL VARIABLES *******************************/

/************************** Constant Definitions ****************************/
enum PL_CIS_SEQUENCE_MODE {
	PL_CIS_POWERDOWN,
	PL_CIS_POWERUP,
	PL_CIS_CLEAR,
	PL_CIS_INIT,
	PL_CIS_SRRD,
	PL_CIS_DAC,
	PL_CIS_AFE,
	PL_CIS_SCAN_ENABLE,
	PL_CIS_SCAN_DISABLE,
	PL_CIS_SCAN_IDLE,
};

#define CLK_PHASE_SHIFT_SIZE	51

/************************** Variable declaration *****************************/

/************************** External functions *******************************/
extern u32 InitGpioDirection(u16 DeviceId, unsigned Channel, u32 Port, u8 Direction);
extern u32 GpioOutputExample(u16 DeviceId, unsigned Channel, u32 DataWrite);
extern u32 GpioInputExample(u16 DeviceId, unsigned Channel, u32 Port, u32 *DataRead);
extern u32 GpioPinWrite(u16 DeviceId, unsigned Channel, u32 Mask, u32 Output);
//extern void Xil_DCacheInvalidateRange(INTPTR adr, u32 len);
/*-----------------------------------------------------------*/
void _pl_cis_cap_info(u8 type);
void _pl_sampling_ss_seq(void);
void _pl_oneshot_ss_seq(u8 type);
void _pl_oneshot_cis_seq(u8 type);
void _pl_oneshot_non_paper_cis_seq(u8 type);
void _pl_sampling_cis_seq(void);
void _pl_sampling_regist_data(void);

/* Initialize DDR */
void _pl_cis_init_ddr_memory(void);
/* Initialize fpga register */
void _pl_cis_init_fpga(void);
/* Initialize afe data */
void _pl_cis_init_afe(void);
/* Initialize ad data */
void _pl_cis_init_ad(u16 cismode);
/* cis enable */
u8 _pl_cis_enable_set(u8 set);

/* set afe data */
void sspi_wait_ready(void);
void sspi_cis_initialize(u8 side);

void _pl_cis_oneshot_sequence(u16 cismode);
void _pl_mag_adjustment_sequence(void);
u8 _pl_cis_temp_adjust_pga(void);
u8 _pl_cis_calc_bc(void);
u8 _pl_cis_calc_wc(void);
/*-----------------------------------------------------------*/
void _pl_cis_scanst(u8 set);
void _pl_cis_scan_enable(u8 set);
u32 _pl_get_cis_cap_ln(void);
/* cis check */
u32 _pl_cisa_lock_check(void);
u32 _pl_cisb_lock_check(void);
void _pl_sampling_cyclic_off(void);
u8 _pl_validation_sensor_offon(u8 on);
#endif /* _PL_CIS_HEADER_H */

/* EOF */
