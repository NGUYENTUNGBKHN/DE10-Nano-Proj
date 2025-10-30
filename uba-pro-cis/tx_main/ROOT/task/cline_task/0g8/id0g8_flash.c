/******************************************************************************/
/*! @addtogroup Main
    @file       dline_task.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "hal_cyclonev_rstmgr.h"


#if 1//#ifdef  FLASH_QSPI_MODE
#include "alt_qspi.h"
#endif

#define EXT


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

/******************************************************************************/
/*! @brief Front USB device initialize
    @return         none
    @exception      none
******************************************************************************/
void _dline_initialize_flash(void)
{

#if 1//#ifdef  FLASH_QSPI_MODE
	bool stat = false;
	qspi_software_reset();	//system sub_functions.c
	RstmgrQspiReset();   //hal hal_cyclonev_rstmgr.c
	/* FLASHメモリドライバ設定 cv5hwlib */

	stat = alt_qspi_init();	//別フォルダ
	if (ALT_E_SUCCESS != stat)
	{
		program_error();
	}

	/* disable mode bit */
	stat = alt_qspi_mode_bit_disable();	//別フォルダ
	if (ALT_E_SUCCESS != stat)
	{
		program_error();
	}
	stat = alt_qspi_mode_bit_config_set(0x00);	//別フォルダ
	if (ALT_E_SUCCESS != stat)
	{
		program_error();
	}

	stat = alt_qspi_enable();	//別フォルダ
	if (ALT_E_SUCCESS != stat)
	{
		program_error();
	}
	stat = alt_qspi_sram_partition_set(0x40);	//別フォルダ
	if (ALT_E_SUCCESS != stat)
	{
		program_error();
	}
#else
	/* FLASHメモリドライバ設定 JSL */
	QSPI_Flash_init(NULL);
	memset((void *)&flash_param, 0, sizeof(flash_param));
	QSPI_Flash_open(&hQFlash, &flash_param, &size);
#endif

}



/* EOF */
