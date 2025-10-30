/******************************************************************************/
/*! @addtogroup Group1
    @file       barcode.c
    @brief      validate a barcode ticket
    @date       2018/02/26
    @author     T.Yokoyama
    @par        Revision
    $Id$
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/02/26 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "custom.h"
#include "bv_errcode.h"
#include <string.h>
#include "sub_functions.h"

#define EXT
#include "com_ram.c"
#include "cis_ram.c"
#include "tem_global.c"

#include "tem_main.h"
#include "tem_main_barcode.h"

static u16 s_BARreject_code;

extern void interface_get_bar_info(u8 *type, u8 *length, u8 *length2, u8 *length3,u8 *length4,u8 *inhibit);

bool is_barcode_ticket_disabled(void);
void set_ticket_rej_code(void);
void tito_ticket_check(int* result);
void qr_ticket_check(int* result);

#if (DEBUG_1D_TICKET_RETURN_EXCEPT_SPECIFIC_NUMBER==1)
bool is_barcode_exept_specific_number(void)
{
	bool result = false;
	int index;

	for(index = 0; index < ex_bar_length[0]; index++)
	{
		if((index % 2) == 0)
		{
			/* check even number */
			if(ex_barcode[0] != ex_barcode[index])
			{
				result = true;
			}
		}
		else
		{
			/* check odd number */
			if(index == 1)
			{
				if(ex_barcode[1] != '0')
				{
					result = true;
				}
			}
			else
			{
				if(ex_barcode[index] == '0')
				{
					if(ex_barcode[index] - ex_barcode[index - 2] != 9)
					{
						result = true;
					}
				}
				else if(ex_barcode[index] - ex_barcode[index - 2] != 1)
				{
					result = true;
				}
			}
		}
	}
	return result;
}
#endif
UB bar_check(void)
{
	int result = 0;

#if DEBUG_VALIDATION_RESULT
	ex_barcode_charactor_count = 0;
#endif

	if (is_test_mode())
	{
		set_bill_information(&_bill_info);
		for(int type = BARCODE_TYPE_ICB; type < BARCODE_TYPE_COUNT; type++)
		{
			s_BARreject_code = 0;
			switch(type)
			{
			case BARCODE_TYPE_TITO:
				ex_Barcom.enable  = 0x03;
				ex_Barcom.type    = BARCODE_TYPE_TITO;
				ex_Barcom.length  = 0x28;
				ex_Barcom.length2 = 0x28;
				//ex_Barcom.length2  = 0x10;
				//ex_Barcom.length2  = 0x14;
				//ex_Barcom.length2  = 0x16;
				ex_Barcom.side = BARCODE_SIDE_BOTH;
				tito_ticket_check(&result);
				break;
			case BARCODE_TYPE_QR:
				ex_Barcom.enable  = 0x03;
				ex_Barcom.type    = BARCODE_TYPE_QR;
				ex_Barcom.length  = 0x28;
				ex_Barcom.length2 = 0x28;
				//ex_Barcom.length2  = 0x10;
				//ex_Barcom.length2  = 0x14;
				//ex_Barcom.length2  = 0x16;
				ex_Barcom.side = BARCODE_SIDE_BOTH;
				qr_ticket_check(&result);
				break;
			default:
				break;
			};

			if(result == TRUE)
			{
#if (DEBUG_1D_TICKET_RETURN_EXCEPT_SPECIFIC_NUMBER==1)
				if(is_barcode_exept_specific_number())
				{
					/* ナンバーインヒビット ivizionで使用していないキャラ数返却(緑3回）を使用 */
					ex_bar_reject_code = REJECT_CODE_BAR_SH;
				}
				else
				{
					ex_bar_reject_code = 0;
				}
#else
				ex_bar_reject_code = 0;
#endif
				break;
			}
			else
			{
				set_ticket_rej_code();
				if((ex_bar_reject_code == REJECT_CODE_BAR_DIN)
					|| (ex_bar_reject_code == REJECT_CODE_INHIBIT)
					|| (ex_bar_reject_code == REJECT_CODE_BAR_PHV))
				{
					break;
				}
			}
		}
	}
	else
	{
		s_BARreject_code = 0;
		ex_bar_type = BARCODE_TYPE_INVALID;
		interface_get_bar_info(	&ex_Barcom.type,
		 						&ex_Barcom.length, 
								&ex_Barcom.length2, 
								&ex_Barcom.length3, 
								&ex_Barcom.length4, 
								&ex_Barcom.enable);

	#if(_DEBUG_BARCODE_IF_MODE_4WAY==1)
		ex_Barcom.side = BARCODE_SIDE_BOTH;
	#else
		//#if(COUNTCOD==USA_CRNCY) //この定義だと、右辺側が不定でも有効になるようだ 2024-04-03
		#if 1
		if((ex_system & BIT_SU_UNIT) == BIT_SU_UNIT)
		{
			ex_Barcom.side = BARCODE_SIDE_DOWN;
		}
		else
		{
			ex_Barcom.side = BARCODE_SIDE_UP;
		}
		#else
		ex_Barcom.side = BARCODE_SIDE_BOTH;
		#endif
	#endif
		set_bill_information(&_bill_info);
		switch(ex_Barcom.type)
		{
		case BARCODE_TYPE_TITO:
			tito_ticket_check(&result);
			break;
		
		#if 0 //2024-04-03 インタフェースモードは2D受け取り禁止、テストモードは上記で許可
		case BARCODE_TYPE_QR:
			qr_ticket_check(&result);
			break;
		#endif

		default:
			break;
		};
		if(result == TRUE)
		{
			if(is_barcode_ticket_disabled())
			{
				/* 紙幣inhibit */
				ex_bar_reject_code = REJECT_CODE_INHIBIT;
			}
#if (DEBUG_1D_TICKET_RETURN_EXCEPT_SPECIFIC_NUMBER==1)
			else if(is_barcode_exept_specific_number())
			{
				/* ナンバーインヒビット ivizionで使用していないキャラ数返却(緑3回）を使用 */
				ex_bar_reject_code = REJECT_CODE_BAR_SH;
			}
#endif
			else
			{
				ex_bar_reject_code = 0;
			}
		}
		else
		{
			set_ticket_rej_code();
		}
	}

	return (result);
}

bool is_barcode_ticket_disabled(void)
{
	if(ex_Barcom.enable & 0x02)
	{
		return false;
	}
	return true;
}

bool is_banknote_disabled(void)
{
#if defined(_DEBUG_TICKET_ONLY)
	return true;
#endif
	if(ex_Barcom.enable & 0x01)
	{
		return false;
	}
	return true;
}

bool is_barcode_ticket_inserted(void)
{
	return false;
}

//barcode tickets
u16	check_number_length(void)
{
	u16	result = REJECT_CODE_OK;
	/*---- check charcter length ---*/
	if( ex_Barcom.length == ex_Barcom.length2 )
	{
#if 0//
		/* FFhの場合、16, 18と24のみ受け取る */
		if( ((ex_bar_length[1]*2) != 16)
		&& 	((ex_bar_length[1]*2) != 18)
		&& 	((ex_bar_length[1]*2) != 24)
		){
			return(REJECT_CODE_BAR_SH);			/* short miss read */
		}
#else
		/* 両方が同じ設定値の場合,その設定値を制限値とする。	*/
		/* そのｷｬﾗ長以下なら受け取る */
		//  u8 m_bar5(void)
		//マルチチケットは18char～28char
		if(((ex_bar_length[1]*2) > ex_Barcom.length)
           || (((ex_bar_length[1]*2) < 18)
		   && ( ex_Barcom.length == 28))
		   ){
			return(REJECT_CODE_BAR_SH);			/* short miss read */
		}
#endif
	}
	else{
		if( ((ex_bar_length[1]*2) != ex_Barcom.length )
		&& 	((ex_bar_length[1]*2) != ex_Barcom.length2)
		&& 	((ex_bar_length[1]*2) != ex_Barcom.length3)
		&& 	((ex_bar_length[1]*2) != ex_Barcom.length4)
		){
			return(REJECT_CODE_BAR_SH);			/* short miss read */
		}
	}
	return(result);
}
void tito_ticket_check(int* result)
{
	int character_count = 0;

	checked_ticket_types[TICKET_TYPE_TITO] = TRUE;

#if defined ENABLE_TITO_TICKET
	switch(ex_Barcom.side)
	{
	case BARCODE_SIDE_BOTH:
		run_tito_validation_routine(BARCODE_SEARCH_UP_AND_DOWN);
		break;
	case BARCODE_SIDE_UP:
		run_tito_validation_routine(BARCODE_SEARCH_UP_ONLY);
		break;
	case BARCODE_SIDE_DOWN:
		run_tito_validation_routine(BARCODE_SEARCH_DOWN_ONLY);
		break;
	default:
		break;
	};

	if(_bill_info.tito_ticket_result.error_code == 0) //successful barcode detection
	{
		//save barcode results to RBA-40C structure
		ex_bar_length[1] = (unsigned char)_bill_info.tito_ticket_result.character_length / 2;

		if(check_number_length() == REJECT_CODE_BAR_SH)
		{
		#if 1 // 対象外のキャラクタ数の場合 Reject93
			s_BARreject_code = BAR_SH;
			*result = FALSE;
		#else
			s_BARreject_code = REJ_INH;
			*result = REJ_INH;
		#endif
		}
		else
		{
			memset(ex_barcode, 0, sizeof(ex_barcode));
			ex_bar_type = BARCODE_TYPE_TITO;
			for(character_count = 0; character_count < _bill_info.tito_ticket_result.character_length; character_count++)
			{
				ex_barcode[character_count] = (unsigned char)_bill_info.tito_ticket_result.characters[character_count];
			}
		#if 1	//2023-12-12 
			memo_copy( (u8*)&ex_cline_status_tbl.ex_Barcode_recovery_icb[0], &ex_barcode[0], 28);
		#endif
			ex_bar_length[0] = (unsigned char)_bill_info.tito_ticket_result.character_length;

			s_BARreject_code = BAR_TRUE;
			*result = TRUE;

			previous_ticket_type = TICKET_TYPE_TITO;
		}
#if DEBUG_VALIDATION_RESULT
		/* Update  */
		if(bill_info->tito_ticket_result.character_length <= sizeof(ex_validation.barcode_result.barcode_1d_characters))
		{
			memcpy(&ex_validation.barcode_result.barcode_1d_characters, &bill_info->tito_ticket_result.characters, sizeof(int) * bill_info->tito_ticket_result.character_length);
			ex_validation.barcode_result.barcode_1d_character_length = bill_info->tito_ticket_result.character_length;
		}
#endif
	}
	else
	{
		s_BARreject_code = 0x100F + (_bill_info.tito_ticket_result.error_code << 8);
		*result = 0x100F + (_bill_info.tito_ticket_result.error_code << 8);
	}
#endif
}

void qr_ticket_check(int* result)
{
	int character_count = 0;

	checked_ticket_types[TICKET_TYPE_QR] = TRUE;

#if defined ENABLE_QR_TICKET
	switch(ex_Barcom.side)
	{
	case BARCODE_SIDE_BOTH:
		run_qr_validation_routine(BARCODE_SEARCH_UP_AND_DOWN);
		break;
	case BARCODE_SIDE_UP:
		run_qr_validation_routine(BARCODE_SEARCH_UP_ONLY);
		break;
	case BARCODE_SIDE_DOWN:
		run_qr_validation_routine(BARCODE_SEARCH_DOWN_ONLY);
		break;
	default:
		break;
	};

	if(_bill_info.qr_ticket_result.error_code == 0) //successful barcode detection
	{
		//save barcode results to iVIZION structure
		ex_bar_length[1] = (unsigned char)_bill_info.qr_ticket_result.character_length / 2;

		memset(ex_barcode, 0, sizeof(ex_barcode));
		ex_bar_type = BARCODE_TYPE_QR;
		for(character_count = 0; character_count < _bill_info.qr_ticket_result.character_length; character_count++)
		{
			ex_barcode[character_count] = (unsigned char)_bill_info.qr_ticket_result.characters[character_count];
		}

		ex_bar_length[0] = (unsigned char)_bill_info.qr_ticket_result.character_length;

		s_BARreject_code = BAR_TRUE;
		*result = TRUE;
	}
	else
	{
		s_BARreject_code = 0x100F + (_bill_info.qr_ticket_result.error_code << 8);
		*result = 0x100F + (_bill_info.qr_ticket_result.error_code << 8);
	}
#endif
}

void set_ticket_rej_code(void)
{
	switch(s_BARreject_code)
	{
	case BAR_TRUE:
		ex_bar_reject_code = 0;
		break;
	case  REJ_INH:
		ex_bar_reject_code = REJECT_CODE_INHIBIT;
		break;
	case BAR_NC:
		ex_bar_reject_code = REJECT_CODE_BAR_NC;
		break;
	case BAR_UN:
		ex_bar_reject_code = REJECT_CODE_BAR_UN;
		break;
	case  BAR_SH:
		ex_bar_reject_code = REJECT_CODE_BAR_SH;
		break;
	case  BAR_ST:
		ex_bar_reject_code = REJECT_CODE_BAR_ST;
		break;
	case  BAR_SP:
		ex_bar_reject_code = REJECT_CODE_BAR_SP;
		break;
	case  BAR_TP:
		ex_bar_reject_code = REJECT_CODE_BAR_TP;
		break;
	case  BAR_LG:
		ex_bar_reject_code = REJECT_CODE_BAR_LG;
		break;
	case  BAR_NG:
		ex_bar_reject_code = REJECT_CODE_BAR_NG;
		break;
	case  BAR_LIMIT_WIDE_NARROW:
		/* reserved */
		ex_bar_reject_code = REJECT_CODE_BAR_UN;
		break;
	case  BAR_LIMIT_LENGTH_DIFF:
		/* reserved */
		ex_bar_reject_code = REJECT_CODE_BAR_UN;
		break;
	case  BAR_MC:
		ex_bar_reject_code = REJECT_CODE_BAR_MC;
		break;
	case  BAR_PHV:
		ex_bar_reject_code = REJECT_CODE_BAR_PHV;
		break;
	case  BAR_DIR_MISS:
		ex_bar_reject_code = REJECT_CODE_BAR_DIN;
		break;
	default:
		/* reserved */
		ex_bar_reject_code = REJECT_CODE_BAR_UN;
		break;
	}
}

/* End of file */
