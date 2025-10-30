/******************************************************************************/
/*! @addtogroup Main
    @file       rfid_task.c
    @brief      control rfid task function
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
#include "sub_functions.h"
#include "hal.h"
#include "pl/pl.h"
#include "pl/pl_rfid.h"
#include "pl/pl_rfid_2k.h"

#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"
/************************** Function Prototypes ******************************/
void rfid_task(VP_INT exinf);
u32 address_to_block(u32 addr);
u32 size_to_block_count(u32 siz);

void rfid_clear_icb_demo(u32 address, u32 size);
void rfid_read_icb_demo(u32 address, u32 size);

/************************** External functions *******************************/
/************************** Variable declaration *****************************/
static T_MSG_BASIC rfid_msg;

/************************** PRIVATE DEFINITIONS *************************/
enum _RFID_MODE
{
	RFID_MODE_IDLE = 0,
	RFID_MODE_CONNECT,
	RFID_MODE_RESET,
	RFID_MODE_READ,
	RFID_MODE_WRITE,
};
typedef struct _RFID_SYSTEM_INFO
{
	u8 block_size;
	u8 block_num;
} RFID_SYSTEM_INFO;

enum RFID_TAG_TYPE
{
	RFID_TYPE_232 = 0,		//iviozn2
	RFID_TYPE_2000,			//UBA700
	RFID_TYPE_MAX
};
enum RFID_BAUDRATE
{
	RFID_BAUDRATE_38400 = 0,
	RFID_BAUDRATE_19200 = 1,
	RFID_BAUDRATE_8910 = 2,
	RFID_BAUDRATE_9600 = 3,
	RFID_BAUDRATE_115200 = 4,
};
const RFID_SYSTEM_INFO sysinfo_tag[RFID_TYPE_MAX] =
{
	{4,58},
	{8,250}
};
///*const*/ enum RFID_TAG_TYPE rfid_tag_type_select = RFID_TYPE_232;
/*const*/ enum RFID_TAG_TYPE rfid_tag_type_select = RFID_TYPE_2000;

/************************** PRIVATE FUNCTIONS *************************/
static ER _rfid_carrier_on_command_proc(void);
static ER _rfid_carrier_off_command_proc(void);
#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
//static ER _rfid_read_sblock_command_proc(u32 block, u32 block_count);
//static ER _rfid_read_mblock_command_proc(u32 block, u32 block_count);
#endif
//static ER _rfid_write_sblock_command_proc(void);
//static ER _rfid_write_data_proc(void);
// fast, 2k
//static ER _rfid_fast_read_mblock_command_proc(u32 block, u32 block_count);
#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
static ER _rfid_2k_read_sblock_command_proc(u32 block, u32 block_count);
static ER _rfid_2k_read_mblock_command_proc(u32 block, u32 block_count);
static ER _rfid_2k_fast_read_sblock_command_proc(u32 block, u32 block_count);
static ER _rfid_2k_fast_read_mblock_command_proc(u32 block, u32 block_count);
static ER _rfid_2k_read_mblock_unlimited_command_proc(u32 block, u32 block_count);
static ER _rfid_2k_fast_read_mblock_unlimited_command_proc(u32 block, u32 block_count);
static ER _rfid_2k_write_sblock_command_proc(void);
static ER _rfid_2k_fast_write_sblock_command_proc(void);
static ER _rfid_2k_write_data_proc(void);
static ER _rfid_2k_fast_write_data_proc(void);
#endif


void _rfid_initialize_proc(void);
void _rfid_msg_proc(void);
void _rfid_set_mode(u16 mode);

void _rfid_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4);
void _rfid_system_error(u8 fatal_err, u8 code);

//2023-11-09 #define RFID_RETRY 5 //エラーまで5s
#define RFID_RETRY 3 //2023-11-09	//エラーまで2.5s


#define IS_RFID_EVT_TXD_EMPTY(x)		((x & EVT_RFID_EMPTY) == EVT_RFID_EMPTY)
#define IS_RFID_EVT_RXD_RCV(x)		((x & EVT_RFID_RCV) == EVT_RFID_RCV)

/************************** EXTERN FUNCTIONS *************************/

// ICB data read / write message
void rfid_set_write_buffer(u32 address, u32 size)
{
	int	ii;
	u8	*sptr;
	u32 block, block_count;

	block = address_to_block(address);
	block_count = size_to_block_count(size);

	if(rfid_tag_type_select == RFID_TYPE_232)
	{
	#if 0 //RFID_TYPE_232 2024-06-19
		sptr = ex_ICB_rfid_write_data_bufer;
		if(EX_RFID_TAG_BLOCK < block_count)
		{
			program_error();
		}
		for(ii = 0; ii < block_count; ii++)
		{
			ex_rfid_wsb_set_data[ii][0] = (u8)(ii + block);
			ex_rfid_wsb_set_data[ii][1] = *sptr++;
			ex_rfid_wsb_set_data[ii][2] = *sptr++;
			ex_rfid_wsb_set_data[ii][3] = *sptr++;
			ex_rfid_wsb_set_data[ii][4] = *sptr++;
		}
		ex_rfid_wsb_write_cnt = 0;
		ex_rfid_wsb_set_cnt = block_count;
	#endif
	}
	else if(rfid_tag_type_select == RFID_TYPE_2000)
	{
		sptr = ex_rfid_2k_write_data_bufer;
		if(EX_RFID_2K_TAG_BLOCK < block_count)
		{
			program_error();
		}
		for(ii = 0; ii < block_count; ii++)
		{
			ex_rfid_2k_wsb_set_data[ii][0] = (u8)(ii + block);
			ex_rfid_2k_wsb_set_data[ii][1] = *sptr++;
			ex_rfid_2k_wsb_set_data[ii][2] = *sptr++;
			ex_rfid_2k_wsb_set_data[ii][3] = *sptr++;
			ex_rfid_2k_wsb_set_data[ii][4] = *sptr++;
			ex_rfid_2k_wsb_set_data[ii][5] = *sptr++;
			ex_rfid_2k_wsb_set_data[ii][6] = *sptr++;
			ex_rfid_2k_wsb_set_data[ii][7] = *sptr++;
			ex_rfid_2k_wsb_set_data[ii][8] = *sptr++;
		}
		ex_rfid_wsb_write_cnt = 0;
		ex_rfid_wsb_set_cnt = block_count;
	}
}

u32 get_block_num(void)
{
	switch (rfid_tag_type_select)
	{
	case RFID_TYPE_232:
	case RFID_TYPE_2000:
		return sysinfo_tag[rfid_tag_type_select].block_num;
	default:
		break;
	}
	return 0;
}
u32 address_to_block(u32 addr)
{
	u32 block = 0;
	switch (rfid_tag_type_select)
	{
	case RFID_TYPE_232:
	case RFID_TYPE_2000:
		block = addr / sysinfo_tag[rfid_tag_type_select].block_size;
		break;
	default:
		break;
	}
	return block;
}
u32 size_to_block_count(u32 siz)
{
	u32 count = 0;
	switch (rfid_tag_type_select)
	{
	case RFID_TYPE_232:
	case RFID_TYPE_2000:
		count = siz / sysinfo_tag[rfid_tag_type_select].block_size;
		if(siz % sysinfo_tag[rfid_tag_type_select].block_size)
		{
			count++;
		}
		break;
	default:
		break;
	}
	return count;
}
/*********************************************************************//**
 * @brief read Log(dummy)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rfid_read_icb(u32 address, u32 size)
{
	ER ercd = E_OK;
	UINT retry = 0;
	u32 block,block_count;
	u32 res_code = ALARM_CODE_OK;

	block = address_to_block(address);
	block_count = size_to_block_count( size);

	_pl_rfid_init();
	//_pl_rfid_txd_enable();
	_pl_rfid_rxd_enable();
	while(1)
	{
		if(retry++ > RFID_RETRY)
		{
			_rfid_carrier_off_command_proc();
			ercd = E_TMOUT;
			break;
		}
		res_code = ALARM_CODE_OK;
		/* RFID carrier on */
		if((ercd = _rfid_carrier_on_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		/* mblock */
	#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) //2023-04-18
		//Fast ReadMultipleBlocksUnlimited[0xd5]
		/* fast mblock */
		if((ercd = _rfid_2k_fast_read_mblock_unlimited_command_proc(block, block_count)) != E_OK)
	#else
		if((ercd = _rfid_fast_read_mblock_command_proc(block, block_count)) != E_OK)
	#endif
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		/* RFID carrier off */
		if((ercd = _rfid_carrier_off_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		break;
	}
	//_pl_rfid_txd_disable();
	_pl_rfid_rxd_disable();
	if(ercd == E_OK)
	{
		_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_READ_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
	}
	else
	{
		_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_READ_RSP, TMSG_SUB_ALARM, res_code, 0, 0);
	}
}
/*********************************************************************//**
 * @brief write Log(dummy)
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void rfid_write_icb(u32 address, u32 size)
{
	ER ercd = E_OK;
	UINT retry = 0;
	u32 block,block_count;
	u32 res_code = ALARM_CODE_OK;

	block = address_to_block(address);
	block_count = size_to_block_count(size);

	if(block_count > get_block_num())
	{
		program_error();
	}

	rfid_set_write_buffer(address, size); /* support 232 and 2K*/

	_pl_rfid_init();
	//_pl_rfid_txd_enable();
	_pl_rfid_rxd_enable();
	while(1)
	{
		if(retry++ > RFID_RETRY)
		{
			_rfid_carrier_off_command_proc();
			ercd = E_TMOUT;
			break;
		}
		res_code = ALARM_CODE_OK;
		/* RFID carrier on */
		if((ercd = _rfid_carrier_on_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}

		rfid_set_write_buffer(address, size); /* support 232 and 2K*/
		/* sblock */
	#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1) //2023-04-18
		//Fast WriteSingleBlock[0xd1]
		/* fast sblock */
		if((ercd = _rfid_2k_fast_write_data_proc()) != E_OK)
	#else
		if((ercd = _rfid_write_data_proc()) != E_OK) /* not supoort 2K */
	#endif
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		/* RFID carrier off */
		if((ercd = _rfid_carrier_off_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		break;
	}
	//_pl_rfid_txd_disable();
	_pl_rfid_rxd_disable();
	if(ercd == E_OK)
	{
		_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_WRITE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
	}
	else
	{
		_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_WRITE_RSP, TMSG_SUB_ALARM, res_code, 0, 0);
	}
}
/*******************************
        rfid_task
 *******************************/
void rfid_task(VP_INT exinf)
{
	T_MSG_BASIC *tmsg_pt;

	_rfid_initialize_proc();

	while(1)
	{
		if((trcv_mbx(ID_RFID_MBX, (T_MSG **)&tmsg_pt, TASK_WAIT_TIME)) == E_OK)
		{
			memcpy(&rfid_msg, tmsg_pt, sizeof(T_MSG_BASIC));
			if ((rel_mpf(rfid_msg.mpf_id, tmsg_pt)) != E_OK)
			{
				/* system error */
				_rfid_system_error(1, 3);
			}
			_rfid_msg_proc();
		}
	}

}

FLGPTN wait_rfid_isr(void)
{
	FLGPTN flag = 0;
	ER ercd;
	ercd = twai_flg(ID_RFID_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, 100);
	if(ercd == E_OK)
	{
		clr_flg(ID_RFID_FLAG, ~EVT_ALL_BIT);
		return flag;
	}
	else
	{
		return 0;
	}
}

FLGPTN wait_rfid_2k_isr(void)
{
	FLGPTN flag = 0;
	ER ercd;
	ercd = twai_flg(ID_RFID_FLAG, EVT_ALL_BIT, TWF_ORW, &flag, 2000);
	if(ercd == E_OK)
	{
		clr_flg(ID_RFID_FLAG, ~EVT_ALL_BIT);
		return flag;
	}
	else
	{
		return 0;
	}
}


void _rfid_initialize_proc(void)
{

}

/*********************************************************************//**
 * @brief process of RFID reset
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_reset_command_proc(void)
{
	ER ercd = E_OK;

	__pl_rfid_baudrate_proc(0);

	__pl_rfid_reset_on_proc();
	/* keep reset 3ms or longer */
	OSW_TSK_sleep(4);
	__pl_rfid_reset_off_proc();
#if 1//#if (_DEBUG_NRWA5_ENABLE==1)
	/* wait wakeup 18ms or longer */
	OSW_TSK_sleep(20);
#else
	/* wait wakeup 1.5ms or longer */
	OSW_TSK_sleep(2);
#endif

	return ercd;
}
/*********************************************************************//**
 * @brief process of RFID read ver
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_read_ver_command_proc(void)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;

	while(1)
	{
		/* read version */
		rfid_read_ver_proc();
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_read_ver_receive_data_proc() == 0)
			{
				break;
			}
			else
			{
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}
/*********************************************************************//**
 * @brief process of RFID CARRIER ON
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_carrier_on_command_proc(void)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;

	while(1)
	{
		/* RFID CARRIER ON */
		rfid_rf_control_proc(RFID_CARRIER_ON);
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			break;
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}
/*********************************************************************//**
 * @brief process of RFID BAUDRATE
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_baudrate_command_proc(u8 baudrate)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;

	while(1)
	{
		/* RFID BAUDRATE */
		rfid_baudrate_proc(baudrate);
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			// baudrate wait time
			// 115200 0.3ms
			//   9600 1.8ms
			OSW_TSK_sleep(4);
			break;
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}
/*********************************************************************//**
 * @brief process of RFID inventory
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_inventory_command_proc(void)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;

	while(1)
	{
		/* inventory */
		rfid_inventory_proc();
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_inventory_receive_data_proc() == 0)
			{
				break;
			}
			else
			{
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}

#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
/*********************************************************************//**
 * @brief process of RFID FAST inventory
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_fast_inventory_command_proc(void)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;

	while(1)
	{
		/* inventory */
		rfid_fast_inventory_proc();
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_inventory_receive_data_proc() == 0)
			{
				break;
			}
			else
			{
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}
#endif

/*********************************************************************//**
 * @brief process of RFID check system information
 * @param[in]	None
 * @return 		None
 **********************************************************************/
ER rfid_check_system_information(void)
{
	ER ercd = E_OK;

	if((ex_rfid_info.blocknum+1 == sysinfo_tag[RFID_TYPE_232].block_num)
	 && (ex_rfid_info.blocksize+1 == sysinfo_tag[RFID_TYPE_232].block_size ))
	{
		rfid_tag_type_select = RFID_TYPE_232;
		ercd = E_OK;
	}
	else if((ex_rfid_info.blocknum+1 == sysinfo_tag[RFID_TYPE_2000].block_num)
	 && (ex_rfid_info.blocksize+1 == sysinfo_tag[RFID_TYPE_2000].block_size ))
	{
		rfid_tag_type_select = RFID_TYPE_2000;
#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
		ercd = E_OK;
#else
		ercd = E_NOSPT;
#endif
	}
	else
	{
		ercd = E_NOSPT;
	}
	return ercd;
}
/*********************************************************************//**
 * @brief process of RFID get system information
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_get_system_information_command_proc(void)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;

	while(1)
	{
		/* inventory */
		rfid_get_system_information_proc();
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_get_system_information_receive_data_proc() == 0)
			{
				break;
			}
			else
			{
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}

//(_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
/*********************************************************************//**
 * @brief process of RFID read single block
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
static ER _rfid_read_sblock_command_proc(u32 block, u32 block_count)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;
	UINT length_rest = block_count;
	rfid_tx_rmb_no = block;
	rfid_tx_rmb_length = block_count;

	while(1)
	{
		/* read sblock */
		rfid_read_sblock_proc(block);
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_read_sblock_receive_data_proc(block) == 0)
			{
				length_rest -= 1;
				block += 1;
				if(length_rest == 0)
				{
					break;
				}
			}
			else
			{
				rfid_read_sblock_receive_err_response_proc();
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			rfid_read_sblock_receive_err_response_proc();
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}

/*********************************************************************//**
 * @brief process of RFID read multiple blocks
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_read_mblock_command_proc(u32 block, u32 block_count)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;
	u8 tmp_count;
	UINT length_rest = block_count;
	rfid_tx_rmb_no = block;
	rfid_tx_rmb_length = block_count;
	if(RFID_MAX_READ_BLOCK_LENGTH > length_rest)
	{
		tmp_count = length_rest;
	}
	else
	{
		tmp_count = RFID_MAX_READ_BLOCK_LENGTH;
	}

	while(1)
	{
		/* read mblock */
		rfid_read_mblock_proc(block, tmp_count);
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_read_mblock_receive_data_proc(block, tmp_count) == 0)
			{
				length_rest -= tmp_count;
				block += tmp_count;
				if(length_rest == 0)
				{
					break;
				}
				else if(RFID_MAX_READ_BLOCK_LENGTH > length_rest)
				{
					tmp_count = length_rest;
				}
				else
				{
					tmp_count = RFID_MAX_READ_BLOCK_LENGTH;
				}
			}
			else
			{
				rfid_read_mblock_receive_err_response_proc();
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			rfid_read_mblock_receive_err_response_proc();
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}
#endif


/*********************************************************************//**
 * @brief process of RFID FAST read multiple blocks
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
static ER _rfid_fast_read_mblock_command_proc(u32 block, u32 block_count)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;
	u8 tmp_count;
	UINT length_rest = block_count;
	rfid_tx_rmb_no = block;
	rfid_tx_rmb_length = block_count;
	if(RFID_MAX_READ_BLOCK_LENGTH > length_rest)
	{
		tmp_count = length_rest;
	}
	else
	{
		tmp_count = RFID_MAX_READ_BLOCK_LENGTH;
	}

	while(1)
	{
		/* FAST read mblock */
		rfid_fast_read_mblock_proc(block, tmp_count);
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_read_mblock_receive_data_proc(block, tmp_count) == 0)
			{
				length_rest -= tmp_count;
				block += tmp_count;
				if(length_rest == 0)
				{
					break;
				}
				else if(RFID_MAX_READ_BLOCK_LENGTH > length_rest)
				{
					tmp_count = length_rest;
				}
				else
				{
					tmp_count = RFID_MAX_READ_BLOCK_LENGTH;
				}
			}
			else
			{
				rfid_read_mblock_receive_err_response_proc();
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			rfid_read_mblock_receive_err_response_proc();
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}
#endif

/*********************************************************************//**
 * @brief process of RFID write single block
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0 //RFID_TYPE_232 2024-06-19
static ER _rfid_write_sblock_command_proc(void)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;
	while(1)
	{
		/* write single block */
		rfid_write_sblock_proc();
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_write_sblock_receive_data_proc() == 0)
			{
				break;
			}
			else
			{
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}
#endif

#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
/*********************************************************************//**
 * @brief process of RFID write single block
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_2k_write_sblock_command_proc(void)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;
	while(1)
	{
		/* write single block */
		rfid_2k_write_sblock_proc();
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_write_sblock_receive_data_proc() == 0)
			{
				break;
			}
			else
			{
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}
/*********************************************************************//**
 * @brief process of RFID 2K FAST write single block
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_2k_fast_write_sblock_command_proc(void)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;
	while(1)
	{
		/* write single block */
		rfid_2k_fast_write_sblock_proc();
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_write_sblock_receive_data_proc() == 0)
			{
				break;
			}
			else
			{
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}
#endif

/*********************************************************************//**
 * @brief process of RFID write data
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0 //RFID_TYPE_232 2024-06-19
static ER _rfid_write_data_proc(void)
{
	ER ercd = E_OK;
	UINT retry = 0;

	while(1)
	{
		/* write single block */
		if((ercd = _rfid_write_sblock_command_proc()) == E_OK)
		{
			ex_rfid_wsb_write_cnt++;
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
		if(ex_rfid_wsb_write_cnt == ex_rfid_wsb_set_cnt)
		{
			break;
		}
	}
	return ercd;
}
#endif

#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
/*********************************************************************//**
 * @brief process of 2k RFID read single block
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_2k_read_sblock_command_proc(u32 block, u32 block_count)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;
	UINT length_rest = block_count;
	rfid_tx_rmb_no = block;
	rfid_tx_rmb_length = block_count;

	while(1)
	{
		/* read sblock */
		rfid_2k_read_sblock_proc(block);
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_2k_read_sblock_receive_data_proc(block) == 0)
			{
				length_rest -= 1;
				block += 1;
				if(length_rest == 0)
				{
					break;
				}
			}
			else
			{
				rfid_read_sblock_receive_err_response_proc();
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			rfid_read_sblock_receive_err_response_proc();
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}

/*********************************************************************//**
 * @brief process of RFID 2k tag read multiple blocks
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_2k_read_mblock_command_proc(u32 block, u32 block_count)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;
	u8 tmp_count;
	UINT length_rest = block_count;
	rfid_tx_rmb_no = block;
	rfid_tx_rmb_length = block_count;
	if(RFID_2K_MAX_READ_BLOCK_LENGTH > length_rest)
	{
		tmp_count = length_rest;
	}
	else
	{
		tmp_count = RFID_2K_MAX_READ_BLOCK_LENGTH;
	}

	while(1)
	{
		/* read mblock */
		rfid_2k_read_mblock_proc(block, tmp_count);
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_2k_read_mblock_receive_data_proc(block, tmp_count) == 0)
			{
				length_rest -= tmp_count;
				block += tmp_count;
				if(length_rest == 0)
				{
					break;
				}
				else if(RFID_2K_MAX_READ_BLOCK_LENGTH > length_rest)
				{
					tmp_count = length_rest;
				}
				else
				{
					tmp_count = RFID_2K_MAX_READ_BLOCK_LENGTH;
				}
			}
			else
			{
				rfid_read_mblock_receive_err_response_proc();
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			rfid_read_mblock_receive_err_response_proc();
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}


/*********************************************************************//**
 * @brief process of 2k RFID FAST read single block
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_2k_fast_read_sblock_command_proc(u32 block, u32 block_count)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;
	UINT length_rest = block_count;
	rfid_tx_rmb_no = block;
	rfid_tx_rmb_length = block_count;

	while(1)
	{
		/* 2k FAST read sblock */
		rfid_2k_fast_read_sblock_proc(block);
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_2k_read_sblock_receive_data_proc(block) == 0)
			{
				length_rest -= 1;
				block += 1;
				if(length_rest == 0)
				{
					break;
				}
			}
			else
			{
				rfid_read_sblock_receive_err_response_proc();
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			rfid_read_sblock_receive_err_response_proc();
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}


/*********************************************************************//**
 * @brief process of RFID 2k tag FAST read multiple blocks
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_2k_fast_read_mblock_command_proc(u32 block, u32 block_count)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;
	u8 tmp_count;
	UINT length_rest = block_count;
	rfid_tx_rmb_no = block;
	rfid_tx_rmb_length = block_count;
	if(RFID_2K_MAX_READ_BLOCK_LENGTH > length_rest)
	{
		tmp_count = length_rest;
	}
	else
	{
		tmp_count = RFID_2K_MAX_READ_BLOCK_LENGTH;
	}

	while(1)
	{
		/* read mblock */
		rfid_2k_fast_read_mblock_proc(block, tmp_count);
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_2k_read_mblock_receive_data_proc(block, tmp_count) == 0)
			{
				length_rest -= tmp_count;
				block += tmp_count;
				if(length_rest == 0)
				{
					break;
				}
				else if(RFID_2K_MAX_READ_BLOCK_LENGTH > length_rest)
				{
					tmp_count = length_rest;
				}
				else
				{
					tmp_count = RFID_2K_MAX_READ_BLOCK_LENGTH;
				}
			}
			else
			{
				rfid_read_mblock_receive_err_response_proc();
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			rfid_read_mblock_receive_err_response_proc();
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}


/*********************************************************************//**
 * @brief process of RFID 2k tag FAST read multiple blocks unlimited
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_2k_read_mblock_unlimited_command_proc(u32 block, u32 block_count)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;
	u8 tmp_count;
	UINT length_rest = block_count;
	rfid_tx_rmb_no = block;
	rfid_tx_rmb_length = block_count;
	if(RFID_2K_MAX_READ_BLOCK_UNLIMITED_LENGTH > length_rest)
	{
		tmp_count = length_rest;
	}
	else
	{
		tmp_count = RFID_2K_MAX_READ_BLOCK_UNLIMITED_LENGTH;
	}

	while(1)
	{
		/* read mblock */
		rfid_2k_read_mblock_unlimited_proc(block, tmp_count);
		flag = wait_rfid_2k_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_2k_read_mblock_receive_data_proc(block, tmp_count) == 0)
			{
				length_rest -= tmp_count;
				block += tmp_count;
				if(length_rest == 0)
				{
					break;
				}
				else if(RFID_2K_MAX_READ_BLOCK_UNLIMITED_LENGTH > length_rest)
				{
					tmp_count = length_rest;
				}
				else
				{
					tmp_count = RFID_2K_MAX_READ_BLOCK_UNLIMITED_LENGTH;
				}
			}
			else
			{
				rfid_read_mblock_receive_err_response_proc();
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			rfid_read_mblock_receive_err_response_proc();
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}


/*********************************************************************//**
 * @brief process of RFID 2k tag FAST read multiple blocks unlimited
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_2k_fast_read_mblock_unlimited_command_proc(u32 block, u32 block_count)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;
	u8 tmp_count;
	UINT length_rest = block_count;
	rfid_tx_rmb_no = block;
	rfid_tx_rmb_length = block_count;
	if(RFID_2K_MAX_READ_BLOCK_UNLIMITED_LENGTH > length_rest)
	{
		tmp_count = length_rest;
	}
	else
	{
		tmp_count = RFID_2K_MAX_READ_BLOCK_UNLIMITED_LENGTH;
	}

	while(1)
	{
		/* read mblock */
		rfid_2k_fast_read_mblock_unlimited_proc(block, tmp_count);
		flag = wait_rfid_2k_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			if(rfid_2k_read_mblock_receive_data_proc(block, tmp_count) == 0)
			{
				length_rest -= tmp_count;
				block += tmp_count;
				if(length_rest == 0)
				{
					break;
				}
				else if(RFID_2K_MAX_READ_BLOCK_UNLIMITED_LENGTH > length_rest)
				{
					tmp_count = length_rest;
				}
				else
				{
					tmp_count = RFID_2K_MAX_READ_BLOCK_UNLIMITED_LENGTH;
				}
			}
			else
			{
				rfid_read_mblock_receive_err_response_proc();
				if(retry++ > RFID_RETRY)
				{
					ercd = E_TMOUT;
					break;
				}
			}
		}
		else
		{
			rfid_read_mblock_receive_err_response_proc();
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}

/*********************************************************************//**
 * @brief process of 2K RFID write data
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_2k_write_data_proc(void)
{
	ER ercd = E_OK;
	UINT retry = 0;

	while(1)
	{
		/* write single block */
		if((ercd = _rfid_2k_write_sblock_command_proc()) == E_OK)
		{
			ex_rfid_wsb_write_cnt++;
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
		if(ex_rfid_wsb_write_cnt == ex_rfid_wsb_set_cnt)
		{
			break;
		}
	}
	return ercd;
}

/*********************************************************************//**
 * @brief process of 2K RFID write data
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_2k_fast_write_data_proc(void)
{
	ER ercd = E_OK;
	UINT retry = 0;

	while(1)
	{
		/* FAST write single block */
		if((ercd = _rfid_2k_fast_write_sblock_command_proc()) == E_OK)
		{
			ex_rfid_wsb_write_cnt++;
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
		if(ex_rfid_wsb_write_cnt == ex_rfid_wsb_set_cnt)
		{
			break;
		}
	}
	return ercd;
}
#endif
/*********************************************************************//**
 * @brief process of RFID CARRIER OFF
 * @param[in]	None
 * @return 		None
 **********************************************************************/
static ER _rfid_carrier_off_command_proc(void)
{
	ER ercd = E_OK;
	FLGPTN flag;
	UINT retry = 0;

	while(1)
	{
		/* RFID CARRIER OFF */
		rfid_rf_control_proc(RFID_CARRIER_OFF);
		flag = wait_rfid_isr();
		if(IS_RFID_EVT_RXD_RCV(flag))
		{
			break;
		}
		else
		{
			if(retry++ > RFID_RETRY)
			{
				ercd = E_TMOUT;
				break;
			}
		}
	}
	return ercd;
}
/*********************************************************************//**
 * @brief process of RFID unit info message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _rfid_info_proc(void)
{
	ER ercd = E_OK;
	UINT retry = 0;

	_rfid_set_mode(RFID_MODE_CONNECT);
	_pl_rfid_init();
	_pl_rfid_rxd_enable();
	while(1)
	{
		if(retry++ > RFID_RETRY)
		{
			_rfid_carrier_off_command_proc();
			ercd = E_TMOUT;
			break;
		}
		ercd = E_OK;
		/* reset rfid module */
		if((ercd = _rfid_reset_command_proc()) != E_OK)
		{
			continue;
		}
		/* rfid version */
		if((ercd = _rfid_read_ver_command_proc()) != E_OK)
		{
			continue;
		}
	//#if 1//#if (_DEBUG_NRWA5_ENABLE==1)
		/* change baudrate */
		if((ercd = _rfid_baudrate_command_proc(RFID_BAUDRATE_115200)) != E_OK)
		{
			continue;
		}
		else
		{
			__pl_rfid_baudrate_proc(1);
		}
	//#endif
		/* RFID carrier on */
		if((ercd = _rfid_carrier_on_command_proc()) != E_OK)
		{
			continue;
		}
		/* RFID carrier off */
		if((ercd = _rfid_carrier_off_command_proc()) != E_OK)
		{
			continue;
		}
		break;
	}
	_pl_rfid_rxd_disable();
	if(ercd == E_OK)
	{
		_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_STATUS_INFO, TMSG_SUB_SUCCESS, TRUE, 0, 0);
	}
	else
	{
		_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_STATUS_INFO, TMSG_SUB_SUCCESS, FALSE, 0, 0);
	}
	_rfid_set_mode(RFID_MODE_IDLE);
}

#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
/*********************************************************************//**
 * @brief process of RFID reset message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _rfid_command_test_msg_proc(void)
{
	ER ercd = E_OK;
	UINT retry = 0;
	u32 res_code = ALARM_CODE_OK;

	u8 data_back[8];
	u8 data_back_done=0;

	_rfid_set_mode(RFID_MODE_RESET);
	_pl_rfid_init();
	_pl_rfid_rxd_enable();
	while(1)
	{
		if(retry++ > RFID_RETRY)
		{
			_rfid_carrier_off_command_proc();
			ercd = E_TMOUT;
			break;
		}
		res_code = ALARM_CODE_OK;

		/* reset rfid module */
		if((ercd = _rfid_reset_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}

		//1 ReadVer
		/* rfid version */
		if((ercd = _rfid_read_ver_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		//2 BaudRate
		/* change baudrate */
		if((ercd = _rfid_baudrate_command_proc(RFID_BAUDRATE_115200)) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		else
		{
			__pl_rfid_baudrate_proc(1);
			// baudrate wait time
			// 115200 0.3ms
			//   9600 1.8ms
			OSW_TSK_sleep(4);
		}
		//3 RFControl
		/* RFID carrier on */
		if((ercd = _rfid_carrier_on_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		//4 Inventory
		/* inventory */
		if((ercd = _rfid_inventory_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		//5 Fast Inventory
		/* Fast inventory */
		if((ercd = _rfid_fast_inventory_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		//6 GetSystemInformation
		/* get system information */
		if((ercd = _rfid_get_system_information_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		/* check system information */
		if((ercd = rfid_check_system_information()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		if(rfid_tag_type_select == RFID_TYPE_232)
		{

		#if 0//
			//ReadSingleBlock[0x20]
			/* sblock *//* 615msec */
			if((ercd = _rfid_read_sblock_command_proc(0, RFID_ALL_BLOCK_LENGTH)) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}
			//ReadMultipleBlocks[0x23]
			/* mblock *//* 105msec*/
			if((ercd = _rfid_read_mblock_command_proc(0, RFID_ALL_BLOCK_LENGTH)) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}
			//WriteSingleBlock[0x21]
			rfid_set_write_buffer(0, 232); /* support 232 and 2K*/
			/* sblock *//* 605msec*/
			if((ercd = _rfid_write_data_proc()) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}
			//Fast ReadMultipleBlocks[0xc3]
			/* mblock *//* 65msec*/
			if((ercd = _rfid_fast_read_mblock_command_proc(0, RFID_ALL_BLOCK_LENGTH)) != E_OK) /* supoort 232*/
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}
			//Read MultipleBlocksUnlimited[----]
			//Fast ReadSingleBlock[----]
			//Fast WriteSingleBlock[----]
			//Fast ReadMultipleBlocksUnlimited[----]
		#else
		//UBA700はサポート外
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;

		#endif

		}
		else if(rfid_tag_type_select == RFID_TYPE_2000)
		{
			#if (UBA_RFID_AGING == 1)
			//現状既存のメモリに影響のない、最終8バイトデータの読み書きで確認する
			if(1)
			{
			/* 書き込み前に現在のデータを読み込み */
				if((ercd = _rfid_2k_fast_read_mblock_unlimited_command_proc(249, 1)) != E_OK)
				{
					res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
					continue;
				}
			/* 現在のデータをバックアップ*/
				if(data_back_done == 0)
				{
				/* 最初の1回のみバックアップ*/
					data_back[0] = ex_rfid_rx_data[4];
					data_back[1] = ex_rfid_rx_data[5];
					data_back[2] = ex_rfid_rx_data[6];
					data_back[3] = ex_rfid_rx_data[7];
					data_back[4] = ex_rfid_rx_data[8];
					data_back[5] = ex_rfid_rx_data[9];
					data_back[6] = ex_rfid_rx_data[10];
					data_back[7] = ex_rfid_rx_data[11];
					data_back_done = 1;
				}

			/* 0x00～0x77書き込み*/

				ex_rfid_2k_write_data_bufer[0] = 0x00;
				ex_rfid_2k_write_data_bufer[1] = 0x11;
				ex_rfid_2k_write_data_bufer[2] = 0x22;
				ex_rfid_2k_write_data_bufer[3] = 0x33;
				ex_rfid_2k_write_data_bufer[4] = 0x44;
				ex_rfid_2k_write_data_bufer[5] = 0x55;
				ex_rfid_2k_write_data_bufer[6] = 0x66;
				ex_rfid_2k_write_data_bufer[7] = 0x77;

				//rfid_set_write_buffer(address, size); /* support 232 and 2K*/
				rfid_set_write_buffer(1992, 8); /* support 232 and 2K*/

				/* sblock */
				if((ercd = _rfid_2k_fast_write_data_proc()) != E_OK)
				//if((ercd = _rfid_write_data_proc()) != E_OK) //not support 2k
				{
					res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
					continue;
				}

				if((ercd = _rfid_2k_fast_read_mblock_unlimited_command_proc(249, 1)) != E_OK)
				{
					res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
					continue;
				}

				// check deata
				if
				(
				( ex_rfid_rx_data[4] == 0x00 )
				&&
				( ex_rfid_rx_data[5] == 0x11 )
				&&
				( ex_rfid_rx_data[6] == 0x22 )
				&&
				( ex_rfid_rx_data[7] == 0x33 )
				&&
				( ex_rfid_rx_data[8] == 0x44 )
				&&
				( ex_rfid_rx_data[9] == 0x55 )
				&&
				( ex_rfid_rx_data[10] == 0x66 )
				&&
				( ex_rfid_rx_data[11] == 0x77 )
				)
				{
					/* OK */
				}
				else
				{
					res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
					continue;
				}

			/* 書き込み(元々書き込まれていたデータに戻す)*/

				ex_rfid_2k_write_data_bufer[0] = data_back[0];
				ex_rfid_2k_write_data_bufer[1] = data_back[1];
				ex_rfid_2k_write_data_bufer[2] = data_back[2];
				ex_rfid_2k_write_data_bufer[3] = data_back[3];
				ex_rfid_2k_write_data_bufer[4] = data_back[4];
				ex_rfid_2k_write_data_bufer[5] = data_back[5];
				ex_rfid_2k_write_data_bufer[6] = data_back[6];
				ex_rfid_2k_write_data_bufer[7] = data_back[7];

				rfid_set_write_buffer(1992, 8); /* support 232 and 2K*/

				/* sblock */
				if((ercd = _rfid_2k_fast_write_data_proc()) != E_OK)
				{
					res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
					continue;
				}

				if((ercd = _rfid_2k_fast_read_mblock_unlimited_command_proc(249, 1)) != E_OK)
				{
					res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
					continue;
				}

				// check deata
				if
				(
				( ex_rfid_rx_data[4] == data_back[0] )
				&&
				( ex_rfid_rx_data[5] == data_back[1] )
				&&
				( ex_rfid_rx_data[6] == data_back[2] )
				&&
				( ex_rfid_rx_data[7] == data_back[3] )
				&&
				( ex_rfid_rx_data[8] == data_back[4] )
				&&
				( ex_rfid_rx_data[9] == data_back[5] )
				&&
				( ex_rfid_rx_data[10] == data_back[6] )
				&&
				( ex_rfid_rx_data[11] == data_back[7] )
				)
				{
					/* OK */
				}
				else
				{
					res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
					continue;
				}
			}
			#else
			if((ercd = _rfid_2k_fast_read_mblock_unlimited_command_proc(0, 32)) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}
			#endif //end
		}
		/* RFID carrier off */
		if((ercd = _rfid_carrier_off_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		break;
	}

	//_pl_rfid_txd_disable();
	_pl_rfid_rxd_disable();
	//#if UBA_RFID
	if(ex_dline_testmode.test_no == TEST_RFID_UBA)	//2023-03-16
	{
		if(ercd == E_OK)
		{
			_rfid_send_msg(ID_MAIN_MBX, TMSG_RFID_RESET_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		}
		else
		{
			_rfid_send_msg(ID_MAIN_MBX, TMSG_RFID_RESET_RSP, TMSG_SUB_ALARM, res_code, 0, 0);
		}
	}

	#if (UBA_RFID_AGING == 1)
	else if(ex_dline_testmode.test_no == TEST_AGING)
	{
		if(ercd == E_OK)
		{
			_rfid_send_msg(ID_MAIN_MBX, TMSG_RFID_RESET_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		}
		else
		{
			_rfid_send_msg(ID_MAIN_MBX, TMSG_RFID_RESET_RSP, TMSG_SUB_ALARM, res_code, 0, 0);
		}
	}
	#endif
	else
	{
		if(ercd == E_OK)
		{
			_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_RESET_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		}
		else
		{
			_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_RESET_RSP, TMSG_SUB_ALARM, res_code, 0, 0);
		}
	}
	_rfid_set_mode(RFID_MODE_IDLE);
}
#endif
/*********************************************************************//**
 * @brief process of RFID reset message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
#if 0
void _rfid_reset_msg_proc(void)
{
	ER ercd = E_OK;
	UINT retry = 0;
	u32 res_code = ALARM_CODE_OK;

	_rfid_set_mode(RFID_MODE_RESET);

	memset(&ex_rfid_unit, 0, sizeof(ex_rfid_unit));
	memset(&ex_rfid_info, 0, sizeof(ex_rfid_info));
	_pl_rfid_init();
	//_pl_rfid_txd_enable();
	_pl_rfid_rxd_enable();
	while(1)
	{
		if(retry++ > RFID_RETRY)
		{
			_rfid_carrier_off_command_proc();
			ercd = E_TMOUT;
			break;
		}
		res_code = ALARM_CODE_OK;

		/* reset rfid module */
		if((ercd = _rfid_reset_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		/* rfid version */
		if((ercd = _rfid_read_ver_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
#if 1//#if (_DEBUG_NRWA5_ENABLE==1)
		/* change baudrate */
		if((ercd = _rfid_baudrate_command_proc(RFID_BAUDRATE_115200)) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		else
		{
			__pl_rfid_baudrate_proc(1);
			// baudrate wait time
			// 115200 0.3ms
			//   9600 1.8ms
			OSW_TSK_sleep(4);
		}
#endif
		/* RFID carrier on */
		if((ercd = _rfid_carrier_on_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		/* inventory */
		if((ercd = _rfid_inventory_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		/* get system information */
		if((ercd = _rfid_get_system_information_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		/* check system information */
		if((ercd = rfid_check_system_information()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		/* mblock */
		if((ercd = _rfid_fast_read_mblock_command_proc(0, RFID_ALL_BLOCK_LENGTH)) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}

		/* RFID carrier off */
		if((ercd = _rfid_carrier_off_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		break;
	}

	//_pl_rfid_txd_disable();
	_pl_rfid_rxd_disable();
	//#if UBA_RFID
	//if(ex_main_task_mode1 == MODE1_TEST_STANDBY)
	if(ex_dline_testmode.test_no == TEST_AGING)
	{
		if(ercd == E_OK)
		{
			_rfid_send_msg(ID_MAIN_MBX, TMSG_RFID_RESET_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		}
		else
		{
			_rfid_send_msg(ID_MAIN_MBX, TMSG_RFID_RESET_RSP, TMSG_SUB_ALARM, res_code, 0, 0);
		}
	}
	else if(ex_dline_testmode.test_no == TEST_RFID_UBA)	//2023-03-16
	{
		if(ercd == E_OK)
		{
			_rfid_send_msg(ID_MAIN_MBX, TMSG_RFID_RESET_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		}
		else
		{
			_rfid_send_msg(ID_MAIN_MBX, TMSG_RFID_RESET_RSP, TMSG_SUB_ALARM, res_code, 0, 0);
		}
	}
	else
	{
		if(ercd == E_OK)
		{
			_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_RESET_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
		}
		else
		{
			_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_RESET_RSP, TMSG_SUB_ALARM, res_code, 0, 0);
		}
	}
	//#endif

	_rfid_set_mode(RFID_MODE_IDLE);
}
#endif

/*********************************************************************//**
 * @brief process of RFID read message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _rfid_read_msg_proc(void)
{
	_rfid_set_mode(RFID_MODE_READ);
	switch (rfid_msg.arg1)
	{
	case RFID_ICB:
		rfid_read_icb(rfid_msg.arg2, rfid_msg.arg3);
		break;
	default:
		break;
	}
	_rfid_set_mode(RFID_MODE_IDLE);
}

/*********************************************************************//**
 * @brief process of RFID write message
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _rfid_write_msg_proc(void)
{
	_rfid_set_mode(RFID_MODE_WRITE);
	switch (rfid_msg.arg1)
	{
	case RFID_ICB:
		rfid_write_icb(rfid_msg.arg2, rfid_msg.arg3);
		break;
	default:
		break;
	}
	_rfid_set_mode(RFID_MODE_IDLE);
}
/*********************************************************************//**
 * @brief MBX message procedure
 *  bezel task busy
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _rfid_msg_proc(void)
{
	switch (rfid_msg.tmsg_code)
	{
	case TMSG_RFID_STATUS_REQ:		/* rfid info */
		_rfid_info_proc();
		break;
	case TMSG_RFID_RESET_REQ:		/* reset rfid */
#if 1//#if (_DEBUG_NRWA5_2K_TAG_COMMUNICATION_TEST_ENABLE==1)
		_rfid_command_test_msg_proc();
#else
		_rfid_reset_msg_proc();
#endif
		break;
	case TMSG_RFID_READ_REQ:		/* read rfid */
		_rfid_read_msg_proc();
		break;
	case TMSG_RFID_WRITE_REQ:		/* write rfid */
		_rfid_write_msg_proc();
		break;

	//2023-04-19
	case TMSG_RFID_CLEAR_DEMO_REQ:
		//_rfid_write_msg_proc_demo();
		rfid_clear_icb_demo(rfid_msg.arg2, rfid_msg.arg3);
		break;
	case TMSG_RFID_READ_DEMO_REQ:
		//_rfid_write_msg_proc_demo();
		rfid_read_icb_demo(rfid_msg.arg2, rfid_msg.arg3);
		break;
	default:					/* other */
		/* system error ? */
		_rfid_system_error(0, 4);
		break;
	}
}

/*********************************************************************//**
 * @brief set task mode
 * @param[in]	mode : task mode
 * @return 		None
 **********************************************************************/
void _rfid_set_mode(u16 mode)
{
	ex_rfid_task_mode = mode;
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
void _rfid_send_msg(u32 receiver_id, u32 tmsg_code, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
	T_MSG_BASIC *t_msg;
	ER ercd;

	ercd = get_mpf(ID_MBX_MPF, (VP *)&t_msg);
	if (ercd == E_OK)
	{
		t_msg->sender_id = ID_RFID_TASK;
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
			_rfid_system_error(1, 1);
		}
	}
	else
	{
		/* system error */
		_rfid_system_error(1, 2);
	}
}


/*********************************************************************//**
 * @brief set system error
 * @param[in]	system error code
 * @return 		None
 **********************************************************************/
void _rfid_system_error(u8 fatal_err, u8 code)
{

#ifdef _DEBUG_SYSTEM_ERROR
	_rfid_send_msg(ID_BEZEL_MBX, TMSG_DISP_BEZEL_TEST_RUNNING, 0, 0, 0, 0);
#else  /* _DEBUG_SYSTEM_ERROR */
	//if (fatal_err)
	//{
	//	_rfid_send_msg(ID_RFID_MBX, TMSG_DISP_LED_ALARM, ALARM_CODE_TASK_AREA, 0, 0, 0);
	//}
#endif /* _DEBUG_SYSTEM_ERROR */

	_debug_system_error(ID_RFID_TASK, (u16)code, (u16)rfid_msg.tmsg_code, (u16)rfid_msg.arg1, fatal_err);
}

void rfid_clear_icb_demo(u32 address, u32 size)
{
	ER ercd = E_OK;
	UINT retry = 0;
	u32 res_code = ALARM_CODE_OK;

	_rfid_set_mode(RFID_MODE_RESET);
	_pl_rfid_init();
	_pl_rfid_rxd_enable();
	while(1)
	{
		if(retry++ > RFID_RETRY)
		{
			_rfid_carrier_off_command_proc();
			ercd = E_TMOUT;
			break;
		}
		res_code = ALARM_CODE_OK;

		/* reset rfid module */
		if((ercd = _rfid_reset_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}

		//1 ReadVer
		/* rfid version */
		if((ercd = _rfid_read_ver_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		//2 BaudRate
		/* change baudrate */
		if((ercd = _rfid_baudrate_command_proc(RFID_BAUDRATE_115200)) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		else
		{
			__pl_rfid_baudrate_proc(1);
			// baudrate wait time
			// 115200 0.3ms
			//   9600 1.8ms
			OSW_TSK_sleep(4);
		}
		//3 RFControl
		/* RFID carrier on */
		if((ercd = _rfid_carrier_on_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		//4 Inventory
		/* inventory */
		if((ercd = _rfid_inventory_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		//5 Fast Inventory
		/* Fast inventory */
		if((ercd = _rfid_fast_inventory_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		//6 GetSystemInformation
		/* get system information */
		if((ercd = _rfid_get_system_information_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		/* check system information */
		if((ercd = rfid_check_system_information()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		if(rfid_tag_type_select == RFID_TYPE_232)
		{

		}
		else if(rfid_tag_type_select == RFID_TYPE_2000)
		{

			#if 1 //最終版ではいらないかも/* sblock */
			//読み込みバッファのクリア
			memset( (void *)&ex_rfid_2k_read_data_bufer[0], 0, sizeof(ex_rfid_2k_read_data_bufer) );
			if((ercd = _rfid_2k_read_sblock_command_proc(0, RFID_2K_ALL_BLOCK_LENGTH)) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}
			#endif

			//WriteSingleBlock[0x31]
			memset( &ex_rfid_2k_write_data_bufer[0] , 0x00 , sizeof(ex_rfid_2k_write_data_bufer) );
			rfid_set_write_buffer(0, 2000);
			/* sblock */
			if((ercd = _rfid_2k_write_data_proc()) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}

			#if 1 //最終版ではいらないかも/* sblock */
			//読み込みバッファのクリア
			memset( (void *)&ex_rfid_2k_read_data_bufer[0], 0, sizeof(ex_rfid_2k_read_data_bufer) );
			if((ercd = _rfid_2k_read_sblock_command_proc(0, RFID_2K_ALL_BLOCK_LENGTH)) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}
			#endif

		}
		/* RFID carrier off */
		if((ercd = _rfid_carrier_off_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		break;
	}

	//_pl_rfid_txd_disable();
	_pl_rfid_rxd_disable();
	if(ercd == E_OK)
	{
		_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_WRITE_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
	}
	else
	{
		_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_WRITE_RSP, TMSG_SUB_ALARM, res_code, 0, 0);
	}
}




void rfid_read_icb_demo(u32 address, u32 size)	//2023-04-19
{
	ER ercd = E_OK;
	UINT retry = 0;
	u32 block,block_count;
	u32 res_code = ALARM_CODE_OK;

	block = address_to_block(address);
	block_count = size_to_block_count( size);

	_rfid_set_mode(RFID_MODE_RESET);
	_pl_rfid_init();
	_pl_rfid_rxd_enable();
	while(1)
	{
		if(retry++ > RFID_RETRY)
		{
			_rfid_carrier_off_command_proc();
			ercd = E_TMOUT;
			break;
		}
		res_code = ALARM_CODE_OK;

		/* reset rfid module */
		if((ercd = _rfid_reset_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}

		//1 ReadVer
		/* rfid version */
		if((ercd = _rfid_read_ver_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		//2 BaudRate
		/* change baudrate */
		if((ercd = _rfid_baudrate_command_proc(RFID_BAUDRATE_115200)) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		else
		{
			__pl_rfid_baudrate_proc(1);
			// baudrate wait time
			// 115200 0.3ms
			//   9600 1.8ms
			OSW_TSK_sleep(4);
		}
		//3 RFControl
		/* RFID carrier on */
		if((ercd = _rfid_carrier_on_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		//4 Inventory
		/* inventory */
		if((ercd = _rfid_inventory_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		//5 Fast Inventory
		/* Fast inventory */
		if((ercd = _rfid_fast_inventory_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		//6 GetSystemInformation
		/* get system information */
		if((ercd = _rfid_get_system_information_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}
		/* check system information */
		if((ercd = rfid_check_system_information()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
			continue;
		}

	/*-------------------------------------------------------------------*/
	/* Read start	*/
	/*-------------------------------------------------------------------*/

			//ReadSingleBlock[0x30]
			/* sblock */
			if((ercd = _rfid_2k_read_sblock_command_proc(0, RFID_2K_ALL_BLOCK_LENGTH)) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}
			//ReadMultipleBlocks[0x33]
			/* mblock */
			if((ercd = _rfid_2k_read_mblock_command_proc(0, RFID_2K_ALL_BLOCK_LENGTH)) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}
			//Fast ReadSingleBlock[0xd0]
			/* fast sblock */
			if((ercd = _rfid_2k_fast_read_sblock_command_proc(0, RFID_2K_ALL_BLOCK_LENGTH)) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}
			//Fast ReadMultipleBlocks[0xd3]
			/* fast mblock */
			if((ercd = _rfid_2k_fast_read_mblock_command_proc(0, RFID_2K_ALL_BLOCK_LENGTH)) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}
			//Read MultipleBlocksUnlimited[0xb5]
			/* fast mblock */
			if((ercd = _rfid_2k_read_mblock_unlimited_command_proc(0, RFID_2K_ALL_BLOCK_LENGTH)) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}
			//Fast ReadMultipleBlocksUnlimited[0xd5]
			/* fast mblock */
			if((ercd = _rfid_2k_fast_read_mblock_unlimited_command_proc(0, RFID_2K_ALL_BLOCK_LENGTH)) != E_OK)
			{
				res_code = ALARM_CODE_RFID_ICB_COMMUNICTION_MAIN;
				continue;
			}

	/*-------------------------------------------------------------------*/
	/* Read End */
	/*-------------------------------------------------------------------*/

		/* RFID carrier off */
		if((ercd = _rfid_carrier_off_command_proc()) != E_OK)
		{
			res_code = ALARM_CODE_RFID_UNIT_MAIN;
			continue;
		}
		break;
	}
	//_pl_rfid_txd_disable();
	_pl_rfid_rxd_disable();
	if(ercd == E_OK)
	{
		_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_READ_RSP, TMSG_SUB_SUCCESS, 0, 0, 0);
	}
	else
	{
		_rfid_send_msg(ID_ICB_MBX, TMSG_RFID_READ_RSP, TMSG_SUB_ALARM, res_code, 0, 0);
	}
}

/* EOF */
