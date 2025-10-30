/*
 * jdl_rba40c.c
 *
 *  Created on: 2019/08/20
 *      Author: suzuki-hiroyuki
 */


/***************************** Include Files *********************************/
#include <string.h>
#include "common.h"
#include "status_tbl.h"
#include "sub_functions.h"
#include "hal.h"

#include "hal_spi_fram.h"

#define EXT
#include "com_ram.c"


/************************** Constant Definitions *****************************/
#if (_ENABLE_JDL==1)
/* FRAM ADDRESS(JDL) */
#define FRAM_JDL_SIZE                (JDL_BUFF_TOTAL)

/************************** Variable declaration *****************************/
extern u8 _bkex_jdl_buff[JDL_BUFF_TOTAL];

/*********************************************************************//**
 * @brief write JDL to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8 fram_write_jdl(void)
{
	// DONE: FRAM処理、関数名eeprom,fram変更
	u8 err = 0;
	u8 *ptr;

	ptr = (u8 *)&_bkex_jdl_buff;
#if 1
	/* log table */
	err = _hal_write_fram_log( 0, ptr, (64 - 28) * 1024);
	if( !err )
	{
		err = 1;
	}
	/* log table */
	err = _hal_write_fram_log( (64 - 28) * 1024, ptr + ((64 - 28) * 1024), FRAM_JDL_SIZE - ((64 - 28) * 1024));
	if( !err )
	{
		err = 1;
	}
#else
	/* log table */
	err = _hal_write_fram_log( 0, ptr, FRAM_JDL_SIZE);
	if( !err )
	{
		err = 1;
	}
#endif

	return(err);
}
/*********************************************************************//**
 * @brief read JDL to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8 fram_read_jdl(void)
{
	// DONE: FRAM処理、関数名eeprom,fram変更
	u8 err = 0;
	u8 *ptr;

#if 0
	u32 size[10];
	size[0] = JDL_SYS_BUFF_TOTAL;
	size[1] = JDL_STAT_BUFF_TOTAL;
	size[2] = JDL_SENS_BUFF_TOTAL;
	size[3] = JDL_COMM_BUFF_TOTAL;
	size[4] = JDL_EVEN_BUFF_TOTAL;
	size[5] = JDL_ERR_BUFF_TOTAL;
	size[6] = JDL_ACC_BUFF_TOTAL;
	size[7] = JDL_PANA_BUFF_TOTAL;
	size[8] = 0;
	size[9] = 0;
#endif
	ptr = (u8 *)&_bkex_jdl_buff;
#if 0
	/* log table */
	err = _hal_read_fram_log( 0, ptr, (64 - 28) * 1024);
	if( !err )
	{
		err = 1;
	}
	/* log table */
	err = _hal_read_fram_log( (64 - 28) * 1024, ptr + ((64 - 28) * 1024), FRAM_JDL_SIZE - ((64 - 28) * 1024));
	if( !err )
	{
		err = 1;
	}
#else
	/* log table */
	err = _hal_read_fram_log( 0, ptr, FRAM_JDL_SIZE);
	if( !err )
	{
		err = 1;
	}
#endif

	return(err);
}

/*********************************************************************//**
 * @brief clear JDL to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8 fram_clear_jdl(void)
{
	memset(_bkex_jdl_buff,0,JDL_BUFF_TOTAL);

	return fram_write_jdl();
}
#else
/*********************************************************************//**
 * @brief write JDL to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
u8 fram_write_jdl(void){return 1;};
u8 fram_read_jdl(void){return 1;};
#endif
/* EOF */
