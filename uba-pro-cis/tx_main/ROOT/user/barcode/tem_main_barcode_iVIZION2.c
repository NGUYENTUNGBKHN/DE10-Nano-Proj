
/******************************************************************************/
/*! @addtogroup Group1
    @file       tem_main_barcode_RBA40C.c
    @brief      main barcode validation processing for the RBA40C product
    @date       2018/02/26
    @author     H.Suzuki
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

#define EXT
#include "com_ram.c"
#include "cis_ram.c"
#include "tem_global.c"

#include "tem_main.h"
#include "tem_main_barcode.h"
#include "tem_itf_barcode.h"
#include "tem_qr_barcode.h"


#if defined ENABLE_TITO_TICKET
void run_tito_validation_routine(const int search_type)
{
	memset(&bill_info->tito_ticket_result, 0, sizeof(BARCODE_1D_RESULT));

	switch(search_type)
	{
		case BARCODE_SEARCH_UP_ONLY:
			reset_watchdog_timer();
			set_1d_barcode_center_points_horizontal(CIS_BARCODE_UP);
			itf_barcode_search(CIS_BARCODE_UP, BARCODE_1D_ORIENATION_HORIZONTAL);

			memcpy(&bill_info->tito_ticket_result, &bill_info->itf_barcode_result, sizeof(BARCODE_1D_RESULT));

			//check for up side down barcode
			if(bill_info->tito_ticket_result.error_code != 0)
			{
				reset_watchdog_timer();
				set_1d_barcode_center_points_horizontal(CIS_BARCODE_DOWN);
				itf_barcode_search(CIS_BARCODE_DOWN, BARCODE_1D_ORIENATION_HORIZONTAL);

				if(bill_info->itf_barcode_result.error_code == 0)
				{
					bill_info->tito_ticket_result.error_code = ITF_BARCODE_RESULT_UP_SIDE_DOWN_ERROR;
				}
			}
			break;
		case BARCODE_SEARCH_DOWN_ONLY:
			reset_watchdog_timer();
			set_1d_barcode_center_points_horizontal(CIS_BARCODE_DOWN);
			itf_barcode_search(CIS_BARCODE_DOWN, BARCODE_1D_ORIENATION_HORIZONTAL);

			memcpy(&bill_info->tito_ticket_result, &bill_info->itf_barcode_result, sizeof(BARCODE_1D_RESULT));

			//check for up side down barcode
			if(bill_info->tito_ticket_result.error_code != 0)
			{
				reset_watchdog_timer();
				set_1d_barcode_center_points_horizontal(CIS_BARCODE_UP);
				itf_barcode_search(CIS_BARCODE_UP, BARCODE_1D_ORIENATION_HORIZONTAL);

				if(bill_info->itf_barcode_result.error_code == 0)
				{
					bill_info->tito_ticket_result.error_code = ITF_BARCODE_RESULT_UP_SIDE_DOWN_ERROR;
				}
			}
			break;
		case BARCODE_SEARCH_UP_AND_DOWN:
			//search up side
			reset_watchdog_timer();
			set_1d_barcode_center_points_horizontal(CIS_BARCODE_UP);
			itf_barcode_search(CIS_BARCODE_UP, BARCODE_1D_ORIENATION_HORIZONTAL);

			memcpy(&bill_info->tito_ticket_result, &bill_info->itf_barcode_result, sizeof(BARCODE_1D_RESULT));

			//search down side
			reset_watchdog_timer();
			set_1d_barcode_center_points_horizontal(CIS_BARCODE_DOWN);
			itf_barcode_search(CIS_BARCODE_DOWN, BARCODE_1D_ORIENATION_HORIZONTAL);

			if(bill_info->tito_ticket_result.error_code == 0 && bill_info->itf_barcode_result.error_code == 0) //barcode on both sides
			{
				bill_info->tito_ticket_result.error_code = ITF_BARCODE_RESULT_4WAY_ERROR;
			}
			else if(bill_info->tito_ticket_result.error_code == 0) //barcode on up side
			{
				//keep up side results
			}
			else if(bill_info->itf_barcode_result.error_code == 0) //barcode on down side
			{
				memcpy(&bill_info->tito_ticket_result, &bill_info->itf_barcode_result, sizeof(BARCODE_1D_RESULT));
			}
			else if(bill_info->tito_ticket_result.error_code < bill_info->itf_barcode_result.error_code) //both failed but down side has better result
			{
				memcpy(&bill_info->tito_ticket_result, &bill_info->itf_barcode_result, sizeof(BARCODE_1D_RESULT));
			}
			break;
		case BARCODE_TEST:
			reset_watchdog_timer();

			set_1d_barcode_center_points_horizontal(CIS_BARCODE_UP);
			itf_barcode_search_test(CIS_BARCODE_UP, BARCODE_1D_ORIENATION_HORIZONTAL);

			memcpy(&bill_info->tito_ticket_result, &bill_info->itf_barcode_result, sizeof(BARCODE_1D_RESULT));

			break;
		default:
			break;

	}

	//enforce tito specific digit lengths
	if(bill_info->tito_ticket_result.error_code == 0)
	{
		if(bill_info->tito_ticket_result.character_length < TITO_TICKET_MINIMUM_DIGIT_COUNT
			|| bill_info->tito_ticket_result.character_length > TITO_TICKET_MAXIMUM_DIGIT_COUNT)
		{
			bill_info->tito_ticket_result.error_code = ITF_BARCODE_RESULT_COUNT_ERROR;

			return;
		}
	}

	//enforce standard ticket size
	if(bill_info->tito_ticket_result.error_code == 0)
	{
		if(ticket_size_check() == FALSE)
		{
			bill_info->tito_ticket_result.error_code = ITF_BARCODE_RESULT_SIZE_ERROR;

			return;
		}
	}

	tito_ticket_ir_check();

	if(bill_info->tito_ticket_result.error_code == 0)
	{
		if(is_double_ticket() == TRUE)
		{
			bill_info->tito_ticket_result.error_code = ITF_BARCODE_RESULT_DOUBLE;

			return;
		}
	}
}
#endif

#if defined ENABLE_QR_TICKET
void run_qr_validation_routine(const int search_type)
{
	memset(&bill_info->qr_ticket_result, 0, sizeof(BARCODE_2D_RESULT));

	switch(search_type)
	{
	case BARCODE_SEARCH_UP_ONLY:
		reset_watchdog_timer();
		set_2d_barcode_search_points_middle(CIS_BARCODE_UP);
		qr_barcode_search(CIS_BARCODE_UP);

		memcpy(&bill_info->qr_ticket_result, &bill_info->qr_barcode_result, sizeof(BARCODE_2D_RESULT));
		break;
	case BARCODE_SEARCH_DOWN_ONLY:
		reset_watchdog_timer();
		set_2d_barcode_search_points_middle(CIS_BARCODE_DOWN);
		qr_barcode_search(CIS_BARCODE_DOWN);

		memcpy(&bill_info->qr_ticket_result, &bill_info->qr_barcode_result, sizeof(BARCODE_2D_RESULT));
		break;
	case BARCODE_SEARCH_UP_AND_DOWN:
		//search up side
		reset_watchdog_timer();
		set_2d_barcode_search_points_middle(CIS_BARCODE_UP);
		qr_barcode_search(CIS_BARCODE_UP);

		memcpy(&bill_info->qr_ticket_result, &bill_info->qr_barcode_result, sizeof(BARCODE_2D_RESULT));

		if(bill_info->qr_ticket_result.error_code == 0) //barcode on up side
		{
			// skip down side
			break;
		}
		//search down side
		reset_watchdog_timer();
		set_2d_barcode_search_points_middle(CIS_BARCODE_DOWN);
		qr_barcode_search(CIS_BARCODE_DOWN);

		if(bill_info->qr_ticket_result.error_code == 0 && bill_info->qr_barcode_result.error_code == 0) //barcode on both sides
		{
			bill_info->qr_ticket_result.error_code = QR_BARCODE_RESULT_4WAY_ERROR;
		}
		else if(bill_info->qr_ticket_result.error_code == 0) //barcode on up side
		{
			//keep up side results
		}
		else if(bill_info->qr_barcode_result.error_code == 0) //barcode on down side
		{
			memcpy(&bill_info->qr_ticket_result, &bill_info->qr_barcode_result, sizeof(BARCODE_2D_RESULT));
		}
		else if(bill_info->qr_ticket_result.error_code < bill_info->qr_barcode_result.error_code) //both failed but down side has better result
		{
			memcpy(&bill_info->qr_ticket_result, &bill_info->qr_barcode_result, sizeof(BARCODE_2D_RESULT));
		}
		break;
	default:
		break;
	}

	//enforce 2d barcode specific digit lengths
	if(bill_info->qr_ticket_result.error_code == 0)
	{
		if(bill_info->qr_ticket_result.character_length < BARCODE_2D_MINIMUM_DIGIT_COUNT
			|| bill_info->qr_ticket_result.character_length > BARCODE_2D_MAXIMUM_DIGIT_COUNT)
		{
			bill_info->qr_ticket_result.error_code = QR_BARCODE_RESULT_UNSUPPORTED_DIGITS_ERROR;

			return;
		}
	}

	//enforce standard ticket size
	if(bill_info->qr_ticket_result.error_code == 0)
	{
		if(ticket_size_check() == FALSE)
		{
			bill_info->qr_ticket_result.error_code = QR_BARCODE_RESULT_SIZE_ERROR;

			return;
		}
	}

	//bill_info->itf_barcode_has_ir = FALSE;

	if(bill_info->qr_ticket_result.error_code == 0)
	{
		if(is_double_ticket() == TRUE)
		{
			bill_info->qr_ticket_result.error_code = QR_BARCODE_RESULT_DOUBLE;
		}
	}
}
#endif


/* EOF */
