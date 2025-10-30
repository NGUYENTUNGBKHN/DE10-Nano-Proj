/******************************************************************************/
/*! @addtogroup Main
    @file       hal_wdt.c
    @date       2018/01/31
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * hal_wdt.c
 *
 *  Created on: 2018/01/31
 *      Author: suzuki-hiroyuki
 */

#include "typedefine.h"
#include "js_oswapi.h"


/************************** Constant Definitions *****************************/
/* Watchdog Timer defines */
#define WDT_ZERO_MODE_OFFSET       0x0
#define WDT_CTR_CNTRL_OFFSET       0x4
#define WDT_RESTART_OFFSET         0x8
#define WDT_STS_OFFSET             0xC

#define WDT_ENABLE                 0x1
#define WDT_RST_ENABLE             0x2
#define WDT_IRQ_ENABLE             0x4
#define WDT_ZERO_KEY               0x00ABC000
#define WDT_ZERO_ENABLE_VAL   (WDT_ENABLE | WDT_RST_ENABLE)
#define WDT_USER_SETTING_MASK      0xF

#define WDT_CTR_CNTRL_VAL          ((0x248 << 6) | 0xFFFF)
#define WDT_RESTART_VAL            0x00001999
#define WDT_RESET_VAL              0x1

/* Reboot status register defines:
 * 0xF0000000 for FSBL fallback mask to notify Boot Rom
 * 0x60000000 for FSBL to mark that FSBL has not handoff yet
 * 0x0F000000 for FSBL to record partition number to work on
 * 0x00FFFFFF for user application to use across soft reset
 */
#define FSBL_FAIL_MASK          0xF0000000
#define FSBL_IN_MASK            0x60000000
#define PARTITION_NUMBER_MASK   0x0F000000

/*
 * Watchdog related Error codes
 */
#define WDT_RESET_OCCURED		0xA501 /**< WDT Reset happened in FSBL */
#define WDT_INIT_FAIL			0xA502 /**< WDT driver INIT failed */

/*
 * SLCR Registers
 */
#define PS_RST_CTRL_REG			(XPS_SYS_CTRL_BASEADDR + 0x200)
#define FPGA_RESET_REG			(XPS_SYS_CTRL_BASEADDR + 0x240)
#define RESET_REASON_REG		(XPS_SYS_CTRL_BASEADDR + 0x250)
#define RESET_REASON_CLR		(XPS_SYS_CTRL_BASEADDR + 0x254)
#define REBOOT_STATUS_REG		(XPS_SYS_CTRL_BASEADDR + 0x258)
#define BOOT_MODE_REG			(XPS_SYS_CTRL_BASEADDR + 0x25C)
#define PS_LVL_SHFTR_EN			(XPS_SYS_CTRL_BASEADDR + 0x900)

#define RESET_REASON_SRST		0x00000020 /**< Reason for reset is SRST */
#define RESET_REASON_SWDT		0x00000001 /**< Reason for reset is SWDT */


/*********************************************************************//**
 * @brief		(Re)start WDT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _hal_start_wdt(void)
{
	// TODO:implement
	//IOREG32(WDT0_BASE, WDT_RESTART_OFFSET) = WDT_RESTART_VAL;
}

/*********************************************************************//**
 * @brief		Enable WDT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _hal_enable_wdt(void)
{
	// TODO:implement
#if 0
	u32 WdtReg = IOREG32(WDT0_BASE, WDT_ZERO_MODE_OFFSET);

	IOREG32(WDT0_BASE, WDT_CTR_CNTRL_OFFSET) = WDT_CTR_CNTRL_VAL;

	WdtReg &= ~WDT_USER_SETTING_MASK;
	IOREG32(WDT0_BASE, WDT_ZERO_MODE_OFFSET) = WdtReg | WDT_ZERO_ENABLE_VAL | WDT_ZERO_KEY;
#endif
}

/*********************************************************************//**
 * @brief		Disable WDT
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _hal_disable_wdt(void)
{
	// TODO:implement
#if 0
	u32 WdtReg = IOREG32(WDT0_BASE, WDT_ZERO_MODE_OFFSET);

	IOREG32(WDT0_BASE, WDT_ZERO_MODE_OFFSET)
	    = (WdtReg | WDT_ZERO_KEY) & ~WDT_ENABLE;
#endif
}

