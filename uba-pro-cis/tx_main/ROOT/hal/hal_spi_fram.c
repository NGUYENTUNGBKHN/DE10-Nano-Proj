/******************************************************************************/
/*! @addtogroup Main
    @file       hal_spi_fram.c
    @date       2018/01/24
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * hal_spi_fram.c
 *
 *  Created on: 2018/01/24
 *      Author: suzuki-hiroyuki
 */
// FRAMドライバファイル。

/***************************** Include Files *********************************/
#include "string.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "js_io.h"
#include "js_oswapi.h"
#include "spi/js_spi.h"
#include "spi_fram/js_spi_fram.h"
#include "common.h"
#include "custom.h"
#include "sub_functions.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

#include "fram_drv.h"
#include "hal_spi_fram.h"
/************************** PRIVATE DEFINITIONS *************************/

/************************** EXTERN VARIABLES *************************/

/*********************************************************************//**
 * @brief initialize FRAM register
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u32 _hal_init_fram_status_register(void)
{
	u32 ret = SUCCESS;
	UINT32 retry;
	UINT8 wdata,rdata;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
	retry = 0;
	while(retry < 1000)
	{
		retry++;
		if(fram_drv_read_status(&rdata) != FRAM_DRV_SUCCESS)
		{
			ret = ERROR;
		}
		wdata = 0x40;
		if(fram_drv_write_status(wdata) != FRAM_DRV_SUCCESS)
		{
			ret = ERROR;
		}
		if(fram_drv_read_status(&rdata) != FRAM_DRV_SUCCESS)
		{
			ret = ERROR;
		}
		if(wdata == rdata)
		{
			ret = SUCCESS;
			break;
		}
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}

/*********************************************************************//**
 * @brief initialize FRAM register
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u32 _hal_set_fram_write_protect(void)
{
	u32 ret = SUCCESS;
	UINT32 retry;
	UINT8 wdata,rdata;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
	retry = 0;
	while(retry < 1000)
	{
		retry++;
		if(fram_drv_read_status(&rdata) != FRAM_DRV_SUCCESS)
		{
			ret = ERROR;
		}
		wdata = 0x4C;
		if(fram_drv_write_status(wdata) != FRAM_DRV_SUCCESS)
		{
			ret = ERROR;
		}
		if(fram_drv_read_status(&rdata) != FRAM_DRV_SUCCESS)
		{
			ret = ERROR;
		}
		if(wdata == rdata)
		{
			ret = SUCCESS;
			break;
		}
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}
/*********************************************************************//**
 * @brief		Write FRAM for sensor adjustment sum(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_write_fram_adj_sum(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > (u32)(FRAM_ADJ_SUM_SIZE) )
	{
		return ERROR;
	};

	spi_buf.buf = (u8 *)pdata;
	spi_buf.addr = address + FRAM_ADJ_SUM_ADR;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};
	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}
/*********************************************************************//**
 * @brief		Read FRAM for sensor adjustment sum(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_read_fram_adj_sum(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > (u32)(FRAM_ADJ_SUM_SIZE) )
	{
		return ERROR;
	};

	spi_buf.addr = address + FRAM_ADJ_SUM_ADR;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	spi_buf.buf = (UINT8 *)pdata;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}
/*********************************************************************//**
 * @brief		Write FRAM for sensor adjustment(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_write_fram_adj(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_ADJ_SIZE )
	{
		return ERROR;
	};
	spi_buf.buf = (u8 *)pdata;
	spi_buf.addr = address + FRAM_ADJ_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};
	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}

/*********************************************************************//**
 * @brief		Read FRAM for sensor adjustment(FM25V05)
 * @param[in]	FRAM read start address (0x0 - 0xFFFF)
 * @param[out]	destination address for FRAM data)
 * @param[in]	length to read
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_read_fram_adj(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_ADJ_SIZE )
	{
		return ERROR;
	};

	spi_buf.addr = address + FRAM_ADJ_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	spi_buf.buf = (UINT8 *)pdata;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}
/*********************************************************************//**
 * @brief		Write FRAM for power recover(FM25V05)
 * @param[in]	FRAM read start address (0x0 - 0xFFFF)
 * @param[out]	destination address for FRAM data)
 * @param[in]	length to read
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_write_fram_power_recover(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_PWR_ALL_SIZE )
	{
		return ERROR;
	};
	spi_buf.buf = (u8 *)pdata;
	spi_buf.addr = address + FRAM_PWR_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};
	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}
/*********************************************************************//**
 * @brief		Read FRAM for power recover(FM25V05)
 * @param[in]	FRAM read start address (0x0 - 0xFFFF)
 * @param[out]	destination address for FRAM data)
 * @param[in]	length to read
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_read_fram_power_recover(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_PWR_ALL_SIZE )
	{
		return ERROR;
	};

	spi_buf.addr = address + FRAM_PWR_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	spi_buf.buf = (UINT8 *)pdata;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}

/*********************************************************************//**
 * @brief		Write FRAM for cis sensor adjustment(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_write_fram_cis_adj(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > (u32)(FRAM_ADJ_CIS_SIZE) )
	{
		return ERROR;
	};

	spi_buf.buf = (u8 *)pdata;
	spi_buf.addr = address + FRAM_ADJ_CIS_ADR;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};
	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}

/*********************************************************************//**
 * @brief		Read FRAM for cis sensor adjustment(FM25V05)
 * @param[in]	FRAM read start address (0x0 - 0xFFFF)
 * @param[out]	destination address for FRAM data)
 * @param[in]	length to read
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_read_fram_cis_adj(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > (u32)(FRAM_ADJ_CIS_SIZE) )
	{
		return ERROR;
	};

	spi_buf.addr = address + FRAM_ADJ_CIS_ADR;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	spi_buf.buf = (UINT8 *)pdata;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}

/*********************************************************************//**
 * @brief		Write FRAM for pos&cis sensor tmp adjustment(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_write_fram_adj_tmp(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_ADJ_TMP_SIZE )
	{
		return ERROR;
	};

	spi_buf.buf = (u8 *)pdata;
	spi_buf.addr = address + FRAM_ADJ_TMP_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};
	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}

/*********************************************************************//**
 * @brief		Read FRAM for pos&cis sensor tmp adjustment(FM25V05)
 * @param[in]	FRAM read start address (0x0 - 0xFFFF)
 * @param[out]	destination address for FRAM data)
 * @param[in]	length to read
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_read_fram_adj_tmp(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_ADJ_TMP_SIZE )
	{
		return ERROR;
	};

	spi_buf.addr = address + FRAM_ADJ_TMP_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	spi_buf.buf = (UINT8 *)pdata;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}

/*********************************************************************//**
 * @brief		Write FRAM for mag sensor tmp adjustment(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_write_fram_mag_adj_tmp(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > (FRAM_ADJ_MAG_TMP_ADR + FRAM_ADJ_MAG_TMP_SIZE) )
	{
		return ERROR;
	};

	spi_buf.buf = (u8 *)pdata;
	spi_buf.addr = address + FRAM_ADJ_TMP_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};
	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}

/*********************************************************************//**
 * @brief		Read FRAM for mag sensor tmp adjustment(FM25V05)
 * @param[in]	FRAM read start address (0x0 - 0xFFFF)
 * @param[out]	destination address for FRAM data)
 * @param[in]	length to read
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_read_fram_mag_adj_tmp(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > (FRAM_ADJ_MAG_TMP_ADR + FRAM_ADJ_MAG_TMP_SIZE) )
	{
		return ERROR;
	};

	spi_buf.addr = address + FRAM_ADJ_TMP_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	spi_buf.buf = (UINT8 *)pdata;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}
/*********************************************************************//**
 * @brief		Write FRAM for pos&cis sensor tmp adjustment(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_write_fram_cis_adj_tmp(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_ADJ_CIS_TMP_SIZE )
	{
		return ERROR;
	};

	spi_buf.buf = (u8 *)pdata;
	spi_buf.addr = address + FRAM_ADJ_TMP_OFFSET + FRAM_ADJ_CIS_TMP_ADR;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};
	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}

/*********************************************************************//**
 * @brief		Read FRAM for pos&cis sensor tmp adjustment(FM25V05)
 * @param[in]	FRAM read start address (0x0 - 0xFFFF)
 * @param[out]	destination address for FRAM data)
 * @param[in]	length to read
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_read_fram_cis_adj_tmp(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_ADJ_CIS_TMP_SIZE )
	{
		return ERROR;
	};

	spi_buf.addr = address + FRAM_ADJ_TMP_OFFSET + FRAM_ADJ_CIS_TMP_ADR;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	spi_buf.buf = (UINT8 *)pdata;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}
/*********************************************************************//**
 * @brief		Write FRAM for JCM Device Log(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_write_fram_if(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_IF_ALL_SIZE)
	{
		return ERROR;
	};

	spi_buf.buf = (u8 *)pdata;
	spi_buf.addr = address + FRAM_IF_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}
/*********************************************************************//**
 * @brief		Read FRAM for JCM Device Log(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_read_fram_if(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_IF_ALL_SIZE)
	{
		return ERROR;
	};

	spi_buf.addr = address + FRAM_IF_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	spi_buf.buf = (UINT8 *)pdata;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}
/*********************************************************************//**
 * @brief		Write FRAM for JCM Device Log(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		None
 **********************************************************************/
u32 _hal_write_fram_log(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
#if (_ENABLE_JDL==1)
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_LOG_ALL_SIZE )
	{
		return ERROR;
	};

	spi_buf.buf = (u8 *)pdata;
	spi_buf.addr = address + FRAM_LOG_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};
	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
#endif
	return ret;
}
/*********************************************************************//**
 * @brief		Read FRAM for JCM Device Log(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_read_fram_log(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
#if (_ENABLE_JDL==1)
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_LOG_ALL_SIZE )
	{
		return ERROR;
	};

	spi_buf.addr = address + FRAM_LOG_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	spi_buf.buf = (UINT8 *)pdata;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
#endif
	return ret;
}

/*********************************************************************//**
 * @brief		Write FRAM for ICB Data(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		None
 **********************************************************************/
u32 _hal_write_fram_icb(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_ICB_ALL_SIZE)
	{
		return ERROR;
	};

	spi_buf.buf = (u8 *)pdata;
	spi_buf.addr = address + FRAM_ICB_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}
/*********************************************************************//**
 * @brief		Read FRAM for ICB Data(FM25V05)
 * @param[in]	address (0x0 - 0xFFFF)
 * @return 		SUCCESS or ERROR
 **********************************************************************/
u32 _hal_read_fram_icb(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_ICB_ALL_SIZE)
	{
		return ERROR;
	};

	spi_buf.addr = address + FRAM_ICB_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	spi_buf.buf = (UINT8 *)pdata;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}
#if (HAL_STATUS_ENABLE==1)
	if(ret == ERROR)
	{
		ex_hal_status.fram = HAL_STATUS_NG;
	}
	else
	{
		ex_hal_status.fram = HAL_STATUS_OK;
	}
#endif
	return ret;
}

#if defined(UBA_RTQ)
u32 _hal_read_fram_rtq(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_IF_ALL_SIZE)
	{
		return ERROR;
	};

	spi_buf.addr = address + FRAM_IF_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	spi_buf.buf = (UINT8 *)pdata;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_read(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	return ret;
}

u32 _hal_write_fram_rtq(u32 address, u8 *pdata, u32 length)
{
	u32 ret = SUCCESS;
	UINT32 byte;
	SPI_BUF_INFO spi_buf;
	ER ercd;

	ercd = wai_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	if( (address + length) > FRAM_IF_ALL_SIZE)
	{
		return ERROR;
	};

	spi_buf.addr = address + FRAM_IF_OFFSET;
	spi_buf.len = length;
	spi_buf.byte_count = &byte;
	spi_buf.buf = (UINT8 *)pdata;
	if( (spi_buf.addr + spi_buf.len) > FRAM_SIZE )
	{
		ret = ERROR;
	};

	if(fram_drv_write(spi_buf.addr, spi_buf.buf, spi_buf.len) != FRAM_DRV_SUCCESS)
	{
		ret = ERROR;
	}
	ercd = sig_sem(ID_SPI_SEM);
	if(ercd != E_OK)
	{
		program_error();
	}

	return ret;
}
#endif // UBA_RTQ


/* EOF */
