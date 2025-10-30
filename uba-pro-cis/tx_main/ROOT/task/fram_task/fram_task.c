/******************************************************************************/
/*! @addtogroup Main
    @file       fram_task.c
    @brief      control fram task function
    @date       2021/04/19
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2021/04/19 Development Dept at Tokyo
      -# Initial Version
      -# Branch from Display Task
*****************************************************************************/

/***************************** Include Files *********************************/
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "operation.h"
#include "sub_functions.h"
#include "hal.h"
#include "status_tbl.h"
#include "cyc.h"	//2023-12-04

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

#include "fram_drv.h"
#include "hal_spi_fram.h"
/************************** Function Prototypes ******************************/
void fram_task(VP_INT exinf);

/************************** External functions *******************************/

/************************** Variable declaration *****************************/
static T_MSG_BASIC fram_msg;

/************************** PRIVATE DEFINITIONS *************************/
enum _FRAM_MODE
{
	FRAM_MODE_IDLE = 0,
	FRAM_MODE_READ,
	FRAM_MODE_WRITE,
};
/************************** PRIVATE FUNCTIONS *************************/
static void _fram_initialize_proc(void);
static void _fram_msg_proc(void);
void _fram_set_mode(u16 mode);

void _fram_send_rw_res(u32 task_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _fram_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _fram_system_error(u8 fatal_err, u8 code);
static u8 fram_read_if_authentication(void);
u8 fram_write_if_authentication(void);

/************************** EXTERN FUNCTIONS *************************/
#if defined(UBA_RTQ)
static u8 fram_read_rtq_soft_info();
static u8 fram_read_rtq_before_sta();
static u8 fram_read_rtq_log_if();
static u8 fram_read_rtq_mente_seri();

static u8 fram_read_rtq_edition_all();	//2024-10-16

#endif // UBA_RTQ


/*********************************************************************//**
 * @brief write temperature adjustment data from FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_write_adj_tmp(void)
{
	u8 err = SUCCESS;


	if(_hal_write_fram_adj_tmp(FRAM_ADJ_TMP_ADR, (u8 *)&ex_position_tmp, sizeof(POS_ADJUSTMENT_TMP)) != SUCCESS)
	{
		err = ERROR;
	}

	if(_hal_write_fram_adj_tmp(FRAM_ADJ_TMP_BK_ADR, (u8 *)&ex_position_tmp_bk, sizeof(POS_ADJUSTMENT_TMP)) != SUCCESS)
	{
		err = ERROR;
	}
#if MAG1_ENABLE
	if(_hal_write_fram_mag_adj_tmp(FRAM_ADJ_MAG_TMP_ADR, (u8 *)&ex_mag_adj, sizeof(MAG_SENSOR_VAL)) != SUCCESS)
	{
		err = ERROR;
	}
#endif

	return(err);
}/*********************************************************************//**
 * @brief write cis adjustment data from FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_write_cis_adj(void)
{
	u8 err = SUCCESS;

	if(_hal_write_fram_cis_adj(0, (u8 *)&ex_cis_adjustment_data, sizeof(CIS_ADJUSTMENT_DATA)) != SUCCESS)
	{
		err = ERROR;
	}
	if(_hal_write_fram_cis_adj_tmp(0, (u8 *)&ex_cis_adjustment_tmp, sizeof(CIS_ADJUSTMENT_TMP)) != SUCCESS)
	{
		err = ERROR;
	}
	ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_NONE;

	return(err);
}/*********************************************************************//**
 * @brief write cis temperature adjustment data from FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_write_cis_adj_tmp(void)
{
	u8 err = SUCCESS;

	if(_hal_write_fram_cis_adj_tmp(0, (u8 *)&ex_cis_adjustment_tmp, sizeof(CIS_ADJUSTMENT_TMP)) != SUCCESS)
	{
		err = ERROR;
	}

	return(err);
}
/*********************************************************************//**
 * @brief read adjustment data from FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_read_adj(void)
{
	u8 err = SUCCESS;

	if(_hal_read_fram_adj(0, (u8 *)&ex_adjustment_data, sizeof(ADJUSTMENT_DATA)) != SUCCESS)
	{
		err = ERROR;
	}

	if(_hal_read_fram_adj_sum(0, (u8 *)&ex_fram_sum, FRAM_ADJ_SUM_SIZE) != SUCCESS)
	{
		err = ERROR;
	}
	ex_adjustment_fram_busy &= ~ADJ_FRAM_BUSY;

	return(err);
}
/*********************************************************************//**
 * @brief read temperature adjustment data from FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_read_adj_tmp(void)
{
	u8 err = SUCCESS;

	if(_hal_read_fram_adj_tmp(FRAM_ADJ_TMP_ADR, (u8 *)&ex_position_tmp, sizeof(POS_ADJUSTMENT_TMP)) != SUCCESS)
	{
		err = ERROR;
	}
	if(_hal_read_fram_adj_tmp(FRAM_ADJ_TMP_BK_ADR, (u8 *)&ex_position_tmp_bk, sizeof(POS_ADJUSTMENT_TMP)) != SUCCESS)
	{
		err = ERROR;
	}
#if MAG1_ENABLE
	if(_hal_read_fram_mag_adj_tmp(FRAM_ADJ_MAG_TMP_ADR, (u8 *)&ex_mag_adj, sizeof(MAG_SENSOR_VAL)) != SUCCESS)
	{
		err = ERROR;
	}
#endif

	return(err);
}
/*********************************************************************//**
 * @brief read cis adjustment data from FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_read_cis_adj(void)
{
	u8 err = SUCCESS;

	if(_hal_read_fram_cis_adj(0, (u8 *)&ex_cis_adjustment_data, sizeof(CIS_ADJUSTMENT_DATA)) != SUCCESS)
	{
		err = ERROR;
	}
	if(_hal_read_fram_cis_adj_tmp(0, (u8 *)&ex_cis_adjustment_tmp, sizeof(CIS_ADJUSTMENT_TMP)) != SUCCESS)
	{
		err = ERROR;
	}
	ex_cis_adjustment_tbl.busy = CIS_ADJ_JUB_NONE;

	return(err);
}
/*********************************************************************//**
 * @brief read cis temperature adjustment data from FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_read_cis_adj_tmp(void)
{
	u8 err = SUCCESS;

	if(_hal_read_fram_cis_adj_tmp(0, (u8 *)&ex_cis_adjustment_tmp, sizeof(CIS_ADJUSTMENT_TMP)) != SUCCESS)
	{
		err = ERROR;
	}

	return(err);
}
/*********************************************************************//**
 * @brief read I/F to FRAM(dummy)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_read_if(void)
{
	u8 err = SUCCESS;

#if defined(_PROTOCOL_ENABLE_ID003)
	#if 0
	u8 *ptr;
	/* status table */
	ptr = (u8 *)&bkex_uba_eep_read[0];

	err = _hal_read_fram_if( FRAM_IF_STATUS_TBL_ADR, ptr, BKEX_UBA_LOG);
	if( !err )
	{
		err = 1;
	}
	#endif
#endif
	return(err);
}

/*********************************************************************//**
 * @brief write I/F to FRAM(dummy)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_write_if(void)
{
	u8 err = SUCCESS;

#if defined(_PROTOCOL_ENABLE_ID003)
	#if 0
	u8 *ptr;
	/* status table */
	ptr = (u8 *)&bkex_uba_eep_write[0];

	err = _hal_write_fram_if( FRAM_IF_STATUS_TBL_ADR, ptr, BKEX_UBA_LOG);
	if( !err )
	{
		err = 1;
	}
	return(err);
	#endif	//end UBA_LOG
#else
#endif

	return(err);
}
/*********************************************************************//**
 * @brief read JDL to FRAM(dummy)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
extern u8 fram_read_jdl(void);
extern u8 fram_write_jdl(void);
/*********************************************************************//**
 * @brief write Adjustment Information to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
//センサ調整Toolで設定する、CIS以外の領域とCISの領域を合わせて1つのsumで管理している
//その為、下記込み時は常に両方の領域を計算してsumを書き込む必要がある
/*----------------------------------*/
/*----------------------------------*/
/* CIS以外の領域(センサ調整Tool)	*/
/* 初期流動番号						*/
/* ID								*/
/* バージョン						*/
/* 日時								*/
/* シリアル番号						*/
/* ポジションセンサDA				*/
/* MAG,UV							*/
/*----------------------------------*/
/* CIS領域(センサ調整Tool)			*/
/*----------------------------------*/
/* CIS以外の領域+CIS領域の合計sum	*/
/*----------------------------------*/
static u8 fram_write_adj_inf(void) //CIS以外+CIS領域を含めたsum (CISは書いてない)
{
	u8 err = SUCCESS;
	u16 sum;

	if(_hal_write_fram_adj(FRAM_ADJ_OFFSET, (u8 *)&ex_adjustment_data, sizeof(ADJUSTMENT_DATA)) != SUCCESS)
	{
		err = ERROR;
	}

	sum = culc_fram_adj_sum(); //CIS以外とCIS領域の合計sum
	if(_hal_write_fram_adj_sum(0, (u8 *)&sum, FRAM_ADJ_SUM_SIZE) != SUCCESS)
	{
		err = ERROR;
	}
	ex_adjustment_fram_busy &= ~ADJ_FRAM_BUSY;

	return(err);
}

/*********************************************************************//**
 * @brief read FRAM register
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_write_protect(void)
{
	u8 err = SUCCESS;
	u8 data = 0;

	if(_hal_set_fram_write_protect() != SUCCESS)
	{
		err = ERROR;
	}

	return(err);
}

/*********************************************************************//**
 * @brief read FRAM register
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_init_status_register(void)
{
	u8 err = SUCCESS;
	u8 data = 0;

	if(_hal_init_fram_status_register() != SUCCESS)
	{
		err = ERROR;
	}

	return(err);
}
/*********************************************************************//**
 * @brief read Adjustment Information to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_read_adj_inf(void)
{
	u8 err = SUCCESS;

	if(_hal_read_fram_adj(0, (u8 *)&ex_adjustment_data, sizeof(ADJUSTMENT_DATA)) != SUCCESS)
	{
		err = ERROR;
	}
	ex_adjustment_fram_busy &= ~ADJ_FRAM_BUSY;

	return(err);
}
/*********************************************************************//**
 * @brief write Adjustment Data Serial to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_write_adj_ds(void)//初期流動番号
{
	u8 err = SUCCESS;
	u16 sum;

	if(_hal_write_fram_adj(FRAM_ADJ_OFFSET, (u8 *)&ex_adjustment_data.data_serial, sizeof(DATA_SERIAL)) != SUCCESS) //初期流動番号
	{
		err = ERROR;
	}

	sum = culc_fram_adj_sum(); //CIS以外とCIS領域の合計sum
	if(_hal_write_fram_adj_sum(0, (u8 *)&sum, FRAM_ADJ_SUM_SIZE) != SUCCESS)
	{
		err = ERROR;
	}
	ex_adjustment_fram_busy &= ~ADJ_FRAM_BUSY;

	return(err);
}
/*********************************************************************//**
 * @brief read Adjustment Data Serial Information to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_read_adj_ds(void)
{
	u8 err = SUCCESS;

	if(_hal_read_fram_adj(0, (u8 *)&ex_adjustment_data.data_serial, sizeof(DATA_SERIAL)) != SUCCESS) //初期流動番号
	{
		err = ERROR;
	}
	ex_adjustment_fram_busy &= ~ADJ_FRAM_BUSY;

	return(err);
}

/*********************************************************************//**
 * @brief write Mode2 Setting Data to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_write_mode2_setting(void)
{
	u8 err = SUCCESS;

	if(_hal_write_fram_power_recover(FRAM_MODE2_SET_ADR, (u8 *)&ex_mode2_setting, FRAM_MODE2_SET_SIZE) != SUCCESS)
	{
		err = ERROR;
	}
	ex_adjustment_fram_busy &= ~ADJ_FRAM_BUSY;

	return(err);
}
/*********************************************************************//**
 * @brief read Mode2 Setting Data to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_read_mode2_setting(void)
{
	u8 err = SUCCESS;

	if(_hal_read_fram_power_recover(FRAM_MODE2_SET_ADR, (u8 *)&ex_mode2_setting, sizeof(Mode2Setting)) != SUCCESS)
	{
		err = ERROR;
	}
	ex_adjustment_fram_busy &= ~ADJ_FRAM_BUSY;

	return(err);
}
/*********************************************************************//**
 * @brief write Power Recover Data to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_write_power_recover(void)
{
	u8 err = SUCCESS;

	return(err);
}
/*********************************************************************//**
 * @brief read Power Recover Data to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_read_power_recover(void)
{
	u8 err = SUCCESS;


	return(err);
}
/*********************************************************************//**
 * @brief write ICB Data to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_write_icb_setting(void)
{
	u8 err = SUCCESS;

	if(_hal_write_fram_icb(FRAM_ICB_SETTING_ADR, (u8 *)&ex_icb_setting, FRAM_ICB_SETTING_SIZE) != SUCCESS)
	{
		err = ERROR;
	}

	return(err);
}
/*********************************************************************//**
 * @brief read ICB Data to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_read_icb_setting(void)
{
	u8 err = SUCCESS;

	if(_hal_read_fram_icb(0, (u8 *)&ex_icb_setting, sizeof(ex_icb_setting)) != SUCCESS)
	{
		err = ERROR;
	}

	return(err);
}

/*********************************************************************//**
 * @brief write ICB Data to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_write_icb_recover(void)
{
	u8 err = SUCCESS;

	//2024-04-03
#if defined(UBA_RTQ_ICB)
	if(_hal_write_fram_icb(FRAM_ICB_RECOVER_DATA_ADR, (u8 *)&ex_icb_recovery_blank, sizeof(ex_icb_recovery_blank)) != SUCCESS)
#else
	if(_hal_write_fram_icb(FRAM_ICB_RECOVER_DATA_ADR, (u8 *)&ex_icb_recovery, FRAM_ICB_RECOVER_DATA_SIZE) != SUCCESS)
#endif
	{
		err = ERROR;
	}
	
	return(err);
}

#if defined(UBA_RTQ_ICB)
static u8 fram_write_icb_recover_rtq(void)
{
	u8 err = SUCCESS;

	//2024-04-03 書き込みはSmrtdat　で起動時の読み込みはSmrtdat_fram
	if(_hal_write_fram_icb((FRAM_ICB_RECOVER_DATA_ADR + sizeof(ex_icb_recovery_blank)), (u8 *)&Smrtdat_fram, sizeof(Smrtdat_fram)) != SUCCESS)
	{
		err = ERROR;
	}
	//2025-08-06
	if(_hal_write_fram_icb((FRAM_ICB_RECOVER_DATA_ADR + sizeof(ex_icb_recovery_blank) + sizeof(Smrtdat_fram) ), (u8 *)&Smrtdat_fram_bk, sizeof(Smrtdat_fram_bk)) != SUCCESS)
	{
		err = ERROR;
	}
	
	return(err);
}
#endif

/*********************************************************************//**
 * @brief read ICB Data to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_read_icb_recover(void)
{
	u8 err = SUCCESS;

	//2024-04-03
#if defined(UBA_RTQ_ICB)
	if(_hal_read_fram_icb(FRAM_ICB_RECOVER_DATA_ADR, (u8 *)&ex_icb_recovery_blank, sizeof(ex_icb_recovery_blank)) != SUCCESS)
#else
	if(_hal_read_fram_icb(FRAM_ICB_RECOVER_DATA_ADR, (u8 *)&ex_icb_recovery, sizeof(ex_icb_recovery)) != SUCCESS)
#endif
	{
		err = ERROR;
	}

#if defined(UBA_RTQ_ICB) //2024-04-03 書き込みはSmrtdat　で起動時の読み込みはSmrtdat_fram
	if(_hal_read_fram_icb(FRAM_ICB_RECOVER_DATA_ADR + sizeof(ex_icb_recovery_blank), (u8 *)&Smrtdat_fram, sizeof(Smrtdat_fram)) != SUCCESS)
	{
		err = ERROR;
	}

	//2025-08-06
	if(_hal_read_fram_icb(FRAM_ICB_RECOVER_DATA_ADR + sizeof(ex_icb_recovery_blank) + sizeof(Smrtdat_fram), (u8 *)&Smrtdat_fram_bk, sizeof(Smrtdat_fram_bk)) != SUCCESS)
	{
		err = ERROR;
	}

#endif

	return(err);
}
/*******************************
        fram_task
 *******************************/
#define IS_EVT_FRAM_WRITE(x)			((x & EVT_FRAM_WRITE) == EVT_FRAM_WRITE)

void fram_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;
	FLGPTN flag = 0;
	ER ercd;

	_fram_initialize_proc();

	while(1)
	{
		if((trcv_mbx(ID_FRAM_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME)) == E_OK)
		{
			memcpy(&fram_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(fram_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_fram_system_error(1, 3);
			}
			_fram_msg_proc();
		}
		ercd = twai_flg(ID_FRAM_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, 1);
		if (ercd != E_OK)
		{
			flag = 0;
		}
		else
		{
			if (IS_EVT_FRAM_WRITE(flag))
			{
				write_status_table();
			}
		}
	}
}


static void _fram_initialize_proc(void)
{
	FRAM_DRV_ER er = FRAM_DRV_SUCCESS;

	/* ドライバ初期化 */
	er = fram_drv_init();
	if( er != FRAM_DRV_SUCCESS ){
		osw_printf( "fram_drv_init() - Error\n" );
		/* system error */
		_fram_system_error(1, 25);
		return;
	}

	/* ドライバオープン */
	er = fram_drv_open(0);
	if( er != FALSE )
	{
		osw_printf( "fram_drv_open() - Error\n" );
		/* system error */
		_fram_system_error(1, 26);
		return;
	}
}


/*********************************************************************//**
 * @brief process of FRAM read all data
 * @param[in]	None
 * @return 		err
 **********************************************************************/
static u32 _fram_read_all_data_proc(void) //mode_powerの最初でコールしている
{
	u32 err = SUCCESS;

	if(fram_read_adj_inf() != SUCCESS)
	{
		err = ERROR;
	}

	if(fram_read_adj() != SUCCESS)
	{
		err = ERROR;
	}

	if(fram_read_adj_tmp() != SUCCESS)
	{
		err = ERROR;
	}

	if(fram_read_cis_adj() != SUCCESS)
	{
		err = ERROR;
	}

	if(fram_read_cis_adj_tmp() != SUCCESS)
	{
		err = ERROR;
	}

	if(fram_read_if() != SUCCESS)
	{
		err = ERROR;
	}
	
	if(fram_read_mode2_setting() != SUCCESS)
	{
		err = ERROR;
	}
	
	if(fram_read_power_recover() != SUCCESS)
	{
		err = ERROR;
	}

	if(fram_read_icb_setting() != SUCCESS)
	{
		err = ERROR;
	}
	if(fram_read_icb_recover() != SUCCESS)
	{
		err = ERROR;
	}

	if(fram_read_jdl() != SUCCESS)
	{
		err = ERROR;
	}
#if (_DEBUG_FPGA_FRAM==1) //2023-07-22
	load_phase_fram_log();
#endif

#if defined(UBA_RTQ_AZ_LOG) 
	_hal_read_fram_debug_log_uba(); //2023-09-05
#endif

#if defined(UBA_RTQ)
	if (!fram_read_rtq_soft_info())
	{
		err = ERROR;
	}

	if (!fram_read_rtq_before_sta())
	{
		err = ERROR;
	}
	
	if (!fram_read_rtq_log_if())
	{
		err = ERROR;
	}

	if (!fram_read_rtq_mente_seri())
	{
		err = ERROR;
	}

	if (!fram_read_rtq_edition_all())	//2024-10-16
	{
		err = ERROR;
	}



#endif // UBA_RTQ

	return err;
}

/*********************************************************************//**
 * @brief read I/F Authentication data to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_read_if_authentication(void)
{
	u8 err = SUCCESS;
#if 0
	u8 *ptr;
	/* status table */
	ptr = (u8 *)&ex_Authentication;

	err = _hal_read_fram_if( FRAM_IF_ID003_AUTH_STS_ADR, ptr, sizeof(ex_Authentication));
	if( !err )
	{
		err = 1;
	}
	ptr = (u8 *)&ex_authentication_sum;
	err = _hal_read_fram_if( FRAM_IF_ID003_AUTH_SUM_ADR, ptr, FRAM_IF_ID003_AUTH_SUM_SIZE);
	if( !err )
	{
		err = 1;
	}
#endif

	return(err);
}

#if defined(UBA_RTQ)
static u8 fram_read_rtq_soft_info()
{
	u32 addr = 0;
	u8 err = SUCCESS;
	/* code */
	addr = FRAM_RTQ_SOFT_INFO_ADR;

	/* model */
	err = _hal_read_fram_rtq(addr, &ex_model_bk[0], 16);
	if (!err)
	{
		_fram_system_error(1, 26);
		return ERROR;
	}
	addr += 16;
	/* protocol */
	err = _hal_read_fram_rtq(addr, &ex_protocol_bk[0], 8);
	if (!err)
	{
		_fram_system_error(1, 26);
		return ERROR;
	}
	addr += 8;
	/* Countries */
	err = _hal_read_fram_rtq(addr, &ex_country_bk[0], 8);
	if (!err)
	{
		_fram_system_error(1, 26);
		return ERROR;
	}

	return err;
}

static u8 fram_read_rtq_before_sta()
{
	u32 addr = 0;
	u8 err = SUCCESS;
	/* code */
	addr = FRAM_RTQ_BEFORE_STA_ADR;

	/* rc_before_mode */
	err = _hal_read_fram_rtq(addr, &rc_before_mode, 1);
	if (!err)
	{
		_fram_system_error(1, 26);
		return ERROR;
	}
	addr ++;
	/* rc_before_model */
	err = _hal_read_fram_rtq(addr, &rc_before_model, 1);
	if (!err)
	{
		_fram_system_error(1, 26);
		return ERROR;
	}

	return err;
}

static u8 fram_read_rtq_log_if()
{
	u32 addr = 0;
	u32 len = 0;
	u8 err = SUCCESS;
	/* code */
	addr = FRAM_RTQ_RC_LOG_IF_ADR;
	len = FRAM_RTQ_RC_LOG_IF_SIZE;

	/* RC log if */
	err = _hal_read_fram_rtq(addr, (u8*)&rcLogdatIF, len);
	if (!err)
	{
		_fram_system_error(1, 26);
		return ERROR;
	}

	return err;
}

static u8 fram_read_rtq_mente_seri()
{
	u32 addr = 0;
	u32 len = 0;
	u8 err = SUCCESS;

	addr = FRAM_RTQ_MENTE_SERI_ADR;
	len = FRAM_RTQ_MENTE_SERI_SIZE;

	/* read menten serial */
	err = _hal_read_fram_rtq(addr, (u8*)&read_mente_serailno_data_bk[0], FRAM_RTQ_MENTE_SERI_SIZE);
	if (!err)
	{
		_fram_system_error(1, 26);
		return ERROR;
	}

	return err;
}

//2024-10-16
static u8 fram_read_rtq_edition_all()
{
	u32 addr = 0;
	u32 len = 0;
	u8 err = SUCCESS;
	u8 get[2];

	addr = FRAM_RTQ_MAINTENANCE_EDITION;
	len = FRAM_RTQ_MAINTENANCE_EDITION_SIZE;

	/* read menten serial */
	err = _hal_read_fram_rtq(addr, (u8*)&get[0], 2);
	if (!err)
	{
		_fram_system_error(1, 26);
		return ERROR;
	}
	//将来Factoryを改めて呼び出したいという仕様になったらget[0]を使用する事を検討	
	read_editionno_data.head[0] = get[1];

	return err;
}

static u8 fram_read_rtq_proc()
{
	u8 err = SUCCESS;
	
	switch (fram_msg.arg2)
	{
	case FRAM_RC_SOFT_INFO:
		err = fram_read_rtq_soft_info();
		break;
	case FRAM_RC_ALL:
		err = fram_read_rtq_soft_info();
		if (!err)
		{
			return ERROR;
		}
		err = fram_read_rtq_before_sta();
		if (!err)
		{
			return ERROR;
		}
		err = fram_read_rtq_log_if();
		if (!err)
		{
			return ERROR;
		}
		err = fram_read_rtq_mente_seri();
		if (!err)
		{
			return ERROR;
		}
		break;
	case FRAM_RC_EDITION:	//2024-10-16
		err = fram_read_rtq_edition_all();
		if (!err)
		{
			return ERROR;
		}
		else
		{
			read_editionno_data.read_end &= ~(READ_HEAD_EXEC);
			read_editionno_data.read_end |= READ_HEAD_END;
		}

		break;

	default:
		_fram_system_error(1, 26);
		break;
	}

	return err;
}
#endif // UBA_RTQ

/*********************************************************************//**
 * @brief write I/F Authentication data to FRAM
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static u8 fram_write_if_authentication(void)
{
	u8 err = SUCCESS;
#if 0
	u16 sum;
	u8 *ptr;
	/* status table */
	ptr = (u8 *)&ex_Authentication;

	err = _hal_write_fram_if( FRAM_IF_ID003_AUTH_STS_ADR, ptr, sizeof(ex_Authentication));
	if( !err )
	{
		err = 1;
	}
	sum = 0;
	for(int i  = 0; i < sizeof(ex_Authentication); i++)
	{
		sum += (u8)*(ptr + i);
	}
	sum = sum ^ 0x5555;
	ptr = (u8 *)&sum;
	err = _hal_write_fram_if( FRAM_IF_ID003_AUTH_SUM_ADR, ptr, FRAM_IF_ID003_AUTH_SUM_SIZE);
	if( !err )
	{
		err = 1;
	}
#endif

	return(err);
}

#if defined(UBA_RTQ)
static u8 fram_write_rtq_proc()
{
	u8 err = SUCCESS;
	u32 addr = 0;
	u8 data[256];
	u32 len = 0;

	switch (fram_msg.arg2)
	{
	case FRAM_RC_SOFT_INFO:
		/* code */
		addr = FRAM_RTQ_SOFT_INFO_ADR;
		len = FRAM_RTQ_SOFT_INFO_SIZE;
		memcpy(&data[0], &ex_model_bk[0], 16);
		memcpy(&data[16], &ex_protocol_bk[0], 8);
		memcpy(&data[24], &ex_country_bk[0], 8);
		break;
	case FRAM_RC_BEFORE_STA:
		addr = FRAM_RTQ_BEFORE_STA_ADR;
		len = FRAM_RTQ_BEFORE_STA_SIZE;
		data[0] = rc_before_mode;
		data[1] = rc_before_model;
		break;
	case FRAM_RC_LOG_IF:
		addr = FRAM_RTQ_RC_LOG_IF_ADR;
		len = FRAM_RTQ_RC_LOG_IF_SIZE;
		memcpy(&data[0],  (u8*)&rcLogdatIF, FRAM_RTQ_RC_LOG_IF_SIZE);
		break;
	case FRAM_RC_MENTEN_SERI:
		addr = FRAM_RTQ_MENTE_SERI_ADR;
		len = FRAM_RTQ_MENTE_SERI_SIZE;
		memcpy(&data[0],  (u8*)&read_mente_serailno_data_bk[0], FRAM_RTQ_MENTE_SERI_SIZE);
	//	memcpy(&data[72],  (u8*)&write_mente_serailno_data_bk[0], 72);
		break;

	case FRAM_RC_EDITION:	//2024-10-16
		if(fram_msg.arg3 == 0)
		{
		//Factory
			addr = FRAM_RTQ_MAINTENANCE_EDITION;
			len = FRAM_RTQ_MAINTENANCE_EDITION_SIZE;
			//同じのをコピー
			data[0] = write_editionno_data.head[0];
			data[1] = write_editionno_data.head[0];
		}
		else if(fram_msg.arg3 == 1)
		{
		//Men
			addr = FRAM_RTQ_MAINTENANCE_EDITION + 1;
			len = FRAM_RTQ_MAINTENANCE_EDITION_SIZE - 1;
			//同じのをコピー
			data[0] = write_editionno_data.head[0];
		}
		break;


	default:
		/* system error */
		_fram_system_error(1, 26);
		break;
	}

	err = _hal_write_fram_rtq(addr, data, len);
	if (!err)
	{
		return ERROR;
	}

	return err;
}
#endif // UBA_RTQ

/*********************************************************************//**
 * @brief process of FRAM read message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _fram_read_proc(void)
{
	u32 err;
	_fram_set_mode(FRAM_MODE_READ);
	switch (fram_msg.arg1)
	{
	case FRAM_ALL: //mode_powerの最初でコールしている
		err = _fram_read_all_data_proc();
		break;
	case FRAM_ADJ: //not use
		break;
	case FRAM_ADJ_TMP:
		err = fram_read_adj_tmp();
		break;
	case FRAM_CIS_ADJ:
		err = fram_read_cis_adj();
		break;
	case FRAM_CIS_ADJ_TMP:
		err = fram_read_cis_adj_tmp();
		break;
	case FRAM_IF:
		err = fram_read_if();
		break;
	case FRAM_LOG:
		err = fram_read_jdl();
		break;
	case FRAM_ADJ_INF:
		err = fram_read_adj_inf();
		break;
	case FRAM_POS:
		err = fram_read_adj_inf();
		if(err == SUCCESS)
		{
			err = fram_read_adj_tmp();
		}
		break;
	case FRAM_ADJ_DS:
		err = fram_read_adj_ds();//初期流動用の番号
		break;
	case FRAM_MODE_SETTING:
		err = fram_read_mode2_setting();
		break;
	case FRAM_POWER_RECOVER:
		err = fram_read_power_recover();
		break;
	case FRAM_ICB_SETTING:
		err = fram_read_icb_setting();
		break;
	case FRAM_ICB_RECOVER:
		err = fram_read_icb_recover();
		break;
	case FRAM_IF_AUTHENTICATION:
		err = fram_read_if_authentication(); //not use
		break;
#if defined(UBA_RTQ)
	case FRAM_RTQ:
		err = fram_read_rtq_proc();
		break;
#endif // UBA_RTQ
	default:
		err = ERROR;
		break;
	}
	_fram_set_mode(FRAM_MODE_IDLE);

	if(err == SUCCESS)
	{
		_fram_send_rw_res(fram_msg.sender_id, TMSG_FRAM_READ_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
	}
	else
	{
		_fram_send_rw_res(fram_msg.sender_id, TMSG_FRAM_READ_RSP, TMSG_SUB_ALARM, ALARM_CODE_FRAM, 0, 0);
	}
}

/*********************************************************************//**
 * @brief process of FRAM write message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _fram_write_proc(void)
{
	u32 err;

	_fram_set_mode(FRAM_MODE_WRITE);
	switch (fram_msg.arg1)
	{
	case FRAM_ALL: //not use
		err = ERROR;
		break;
	case FRAM_ADJ: //not use
		break;
	case FRAM_ADJ_TMP:
		err = fram_write_adj_tmp();  //待機時補正 ポジション+MAG(調整sumの範囲外)
		break;
	case FRAM_CIS_ADJ: 				//ここだけ、調整sum範囲のCISを書き込んでいるが、sumは書き込んでいないので注意
		err = fram_write_cis_adj(); //調整 CIS(調整sumの範囲)(sumは書き込んでない)＋待機時補正 CIS 
		break;
	case FRAM_CIS_ADJ_TMP:
		err = fram_write_cis_adj_tmp();//待機時補正 CIS(調整sumの範囲外)
		break;
	case FRAM_IF:
		err = fram_write_if();	//現状未使用
		break;
	case FRAM_LOG:
		err = fram_write_jdl();
		break;
	case FRAM_ADJ_INF:
		err = fram_write_adj_inf();//調整  CIS以外+CISを含めたsum(調整sumの範囲)(sum書き込み)(CISは書いてない)
		if(fram_msg.arg2 == FRAM_SETTING_CLEAR)
		{
			// clear fram settings
			memset(&ex_icb_setting, 0xff, sizeof(ex_icb_setting));
			err = fram_write_icb_setting();
		#if defined(UBA_RTQ_ICB)
			memset(&ex_icb_recovery_blank, 0xff, sizeof(ex_icb_recovery_blank));
		#else
			memset(&ex_icb_recovery, 0xff, sizeof(ex_icb_recovery));
		#endif
			err = fram_write_icb_recover();
#if defined(_PROTOCOL_ENABLE_ID003)
			memset(&ex_Authentication, 0xff, sizeof(ex_Authentication));
			ex_authentication_sum = 0xffff;
#endif
			err = fram_write_if_authentication(); //not use
#ifdef _ENABLE_JDL
			jdl_init(1);
#endif	/* _ENABLE_JDL */
		}
		break;
	case FRAM_POS:
		err = fram_write_adj_inf();//調整 CIS以外+CISを含めたsum(調整sumの範囲)(sum書き込み)(CISは書いてない)
		if(err == SUCCESS)
		{
			err = fram_write_adj_tmp(); //待機時補正 ポジション+ MAG(調整sumの範囲外)
		}
		break;
	case FRAM_ADJ_DS:
		err = fram_write_adj_ds();//初期流動番号
		break;
	case FRAM_MODE_SETTING:
		err = fram_write_mode2_setting();
		break;
	case FRAM_POWER_RECOVER:
		err = fram_write_power_recover();
		break;
	case FRAM_ICB_SETTING:
		// clear recovery data
		err = fram_write_icb_recover();
		// write new setting
		err = fram_write_icb_setting();
		break;
	case FRAM_ICB_RECOVER:
		err = fram_write_icb_recover();
		break;
	case FRAM_IF_AUTHENTICATION:
		err = fram_write_if_authentication(); //not use
		if(err == SUCCESS)
		{
			err = fram_read_if_authentication(); //not use

		}
		break;
#if defined(UBA_RTQ)
	case FRAM_RTQ:
		err = fram_write_rtq_proc();
		break;
	#if defined(UBA_RTQ_ICB)
	case FRAM_ICB_RECOVER_RTQ:
		err = fram_write_icb_recover_rtq();
		break;
	#endif
#endif // UBA_RTQ
	default:
		err = ERROR;
		break;
	}
	_fram_set_mode(FRAM_MODE_IDLE);
	if(err == SUCCESS)
	{
		_fram_send_rw_res(fram_msg.sender_id, TMSG_FRAM_WRITE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
	}
	else
	{
		_fram_send_rw_res(fram_msg.sender_id, TMSG_FRAM_WRITE_RSP, TMSG_SUB_ALARM, ALARM_CODE_FRAM, 0, 0);
	}
}
/*********************************************************************//**
 * @brief process of Power fail FRAM write message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _fram_power_fail(void)
{
	_fram_set_mode(FRAM_MODE_WRITE);
	/* save power recovery flag */
	write_status_table();
	/* power down */
#ifdef _ENABLE_JDL
    jdl_error(ALARM_CODE_POWER_OFF, _main_conv_seq(), ex_main_task_mode1, ex_main_task_mode2, ex_position_sensor);
#endif  /* _ENABLE_JDL */
	/* cold reset */
	_soft_reset();
	_fram_set_mode(FRAM_MODE_IDLE);
}
/*********************************************************************//**
 * @brief process of external RESET FRAM write message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _fram_external_reset(void)
{
	_fram_set_mode(FRAM_MODE_WRITE);
	/* save power recovery flag */
	write_status_table();
	/* power down */
#ifdef _ENABLE_JDL
    jdl_error(ALARM_CODE_EXTERNAL_RESET, _main_conv_seq(), ex_main_task_mode1, ex_main_task_mode2, ex_position_sensor);
#endif  /* _ENABLE_JDL */
	_fram_set_mode(FRAM_MODE_IDLE);
}
/*********************************************************************//**
 * @brief process of Power fail FRAM write message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _fram_pll_lock(void)
{
_fram_set_mode(FRAM_MODE_WRITE);
	/* save power recovery flag */
	write_status_table();
	/* power down */
#ifdef _ENABLE_JDL
    jdl_error(ALARM_CODE_PLL_LOCK, _main_conv_seq(), ex_main_task_mode1, ex_main_task_mode2, ex_position_sensor);
#endif  /* _ENABLE_JDL */
	_fram_set_mode(FRAM_MODE_IDLE);
}
/*********************************************************************//**
 * @brief MBX message procedure
 *  bezel task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static void _fram_msg_proc(void)
{
	switch (fram_msg.tmsg_code)
	{
	case TMSG_FRAM_READ_REQ:		/* read fram */
		_fram_read_proc();
		break;
	case TMSG_FRAM_WRITE_REQ:		/* write fram */
		_fram_write_proc();
		break;
	case TMSG_POWER_NOTICE:			/* write fram from power fail task */
		_fram_power_fail();
		break;
	case TMSG_RESET_NOTICE:			/* write fram from power fail task */
		_fram_external_reset();
		break;
	case TMSG_PLL_LOCK:
		_fram_pll_lock();
		break;
	default:					/* other */
		/* system error ? */
		_fram_system_error(0, 4);
		break;
	}
}

/*********************************************************************//**
 * @brief set task mode
 * @param[in]	mode : task mode
 * @return 		None
 **********************************************************************/
void _fram_set_mode(u16 mode)
{
	ex_fram_task_mode = mode;
}

/*********************************************************************//**
 * @brief send task message
 * @param[in]	receiver task id
 * 				task message code
 * 				argument 1
 * 				argument 2
 * 				argument 3
 * 				argument 4
 * @return 		None
 **********************************************************************/
void _fram_send_rw_res(u32 task_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	u32 mbx_id;
	switch(task_id)
	{
	case ID_MAIN_TASK:			/* main task */
		mbx_id = ID_MAIN_MBX;
		break;
	case ID_CLINE_TASK:			/* cline task */
		mbx_id = ID_CLINE_MBX;
		break;
	case ID_DLINE_TASK:			/* dline task */
		mbx_id = ID_DLINE_MBX;
		break;
	case ID_FEED_TASK:			/* feed task */
		mbx_id = ID_FEED_MBX;
		break;
	case ID_ICB_TASK:			/* icb task */
		mbx_id = ID_ICB_MBX;
		break;
	case ID_POWER_TASK:			/* pf task */
		mbx_id = ID_POWER_MBX;
		break;
	default:				/* other */
		/* system error ? */
		_fram_system_error(0, 5);
		break;
	}
	_fram_send_msg(mbx_id, tmsg_code, arg1, arg2, arg3, arg4);
}
/*********************************************************************//**
 * @brief send task message
 * @param[in]	receiver task id
 * 				task message code
 * 				argument 1
 * 				argument 2
 * 				argument 3
 * 				argument 4
 * @return 		None
 **********************************************************************/
void _fram_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_FRAM_TASK;
		t_msg->mpf_id = ID_MBX_MPF;
		t_msg->tmsg_code = tmsg_code;
		t_msg->arg1 = arg1;
		t_msg->arg2 = arg2;
		t_msg->arg3 = arg3;
		t_msg->arg4 = arg4;
		ercd = snd_mbx(receiver_id, (T_MSG *)t_msg);
		if (ercd != E_OK)
		{
			/* system error */
			_fram_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_fram_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _fram_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	_fram_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_TEST_RUNNING, 0, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	//if (fatal_err)
	//{
	//	_fram_send_msg(ID_FRAM_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	//}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_FRAM_TASK, (u16)code, (u16)fram_msg.tmsg_code, (u16)fram_msg.arg1, fatal_err);
}


/* EOF */
