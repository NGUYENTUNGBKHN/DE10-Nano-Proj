/******************************************************************************/
/*! @addtogroup Group2
    @file       dline_recycle_mode.c
    @brief      
    @date       2024/03/07
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    Japan CashMachine Co, Limited. All rights reserved.
******************************************************************************/

#if defined(UBA_RTQ)

#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"
#include "common.h"
#include "sub_functions.h"
#include "motor_ctrl.h"
#include "dline_suite.h"
#include "dline_test.h"
#include "hal.h"
#include "hal_clk.h"

#define EXT
#include "com_ram.c"
#include "com_ram_ncache.c"
#include "usb_ram.c"

#include "dline_recycle_mode.h"


#define UBA_RTQ_DEBUG_TEST

extern void set_response_1data(u8 cmd);

u8 phase_payout()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_NONE:
        if(ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
        {
            set_response_1data(ACK);
            OperationDenomi.unit = ex_usb_read_buffer[6];
            OperationDenomi.count = ex_usb_read_buffer[7];
            OperationDenomi.remain = OperationDenomi.count;
            _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_PAYOUT_REQ, 
                            OperationDenomi.unit, OperationDenomi.count, 0, 0);
        }
        else
        {
            set_response_1data(NAK);
        }
        break;
    
    default:
        set_response_1data(NAK);
        break;
    }
    return reset;
}

u8 phase_collect()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_NONE:
        if(ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
        {
            set_response_1data(ACK);
            OperationDenomi.unit = ex_usb_read_buffer[6];
			_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_COLLECT_REQ, 
                            OperationDenomi.unit, 0, 0, 0);
        }
        else
        {
            set_response_1data(NAK);
        }
        break;
    
    default:
        set_response_1data(NAK);
        break;
    }
    return reset;
}

u8 phase_set_recycle_denomi()
{
    u8 reset = 0;
    u32 cnt;
    u8 denomi[4];

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_NONE:
        if(ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
        {
            set_response_1data(ACK);

            denomi[0] = ex_usb_read_buffer[6];
            denomi[1] = ex_usb_read_buffer[10];
            denomi[2] = ex_usb_read_buffer[14];
            denomi[3] = ex_usb_read_buffer[18];

            for (cnt = 0; cnt < 4; cnt++)
            {
                is_recycle_set_usb_test_denomi(denomi[cnt], cnt);
                rc_twin_set_bill_info(cnt);
            }

            // _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_DENOMI_REQ, 0, 0, 0, 0);
        }
        else
        {
            set_response_1data(NAK);
        }
        break;
    default:
        set_response_1data(NAK);
        break;
    }
    return reset;
}

u8 phase_set_recycle_limit()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_NONE:
        if(ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
        {
            set_response_1data(ACK);
        }
        else
        {
            set_response_1data(NAK);
        }
        break;
    
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_set_recycle_length()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_NONE:
        if(ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
        {
            set_response_1data(ACK);

            RecycleSettingInfo.DenomiInfo[0].BillInfo.Length = ex_usb_read_buffer[6];
            RecycleSettingInfo.DenomiInfo[1].BillInfo.Length = ex_usb_read_buffer[7];
            RecycleSettingInfo.DenomiInfo[2].BillInfo.Length = ex_usb_read_buffer[8];
            RecycleSettingInfo.DenomiInfo[3].BillInfo.Length = ex_usb_read_buffer[9];

            RecycleSettingInfo.DenomiInfo[0].BillInfo.Max = ex_usb_read_buffer[6] + 3;
            RecycleSettingInfo.DenomiInfo[1].BillInfo.Max = ex_usb_read_buffer[7] + 3;
            RecycleSettingInfo.DenomiInfo[2].BillInfo.Max = ex_usb_read_buffer[8] + 3;
            RecycleSettingInfo.DenomiInfo[3].BillInfo.Max = ex_usb_read_buffer[9] + 3;

            RecycleSettingInfo.DenomiInfo[0].BillInfo.Min = ex_usb_read_buffer[6] - 3;
            RecycleSettingInfo.DenomiInfo[1].BillInfo.Min = ex_usb_read_buffer[7] - 3;
            RecycleSettingInfo.DenomiInfo[2].BillInfo.Min = ex_usb_read_buffer[8] - 3;
            RecycleSettingInfo.DenomiInfo[3].BillInfo.Min = ex_usb_read_buffer[9] - 3;

            // _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_DENOMI_REQ, 0, 0, 0, 0);
        }
        else
        {
            set_response_1data(NAK);
        }
        break;
    
    default:
        set_response_1data(NAK);
        break;
    }
    return reset;
}

u8 phase_set_recycle_count()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_NONE:
        if(ex_dline_task_mode == DLINE_MODE_ATEST_ENABLE)
        {
            set_response_1data(ACK);
        }
        else
        {
            set_response_1data(NAK);
        }
        break;
    
    default:
        set_response_1data(NAK);
        break;
    }
    return reset;
}

u8 phase_get_recycle_count()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_NONE:
        ex_usb_write_size = 10;

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;

        if (ex_rc_status.sst1A.bit.quad == 1)
        {
            *(ex_usb_write_buffer + 6) = RecycleSettingInfo.DenomiInfo[0].RecycleCurrent;
            *(ex_usb_write_buffer + 7) = RecycleSettingInfo.DenomiInfo[1].RecycleCurrent;
            *(ex_usb_write_buffer + 8) = RecycleSettingInfo.DenomiInfo[2].RecycleCurrent;
            *(ex_usb_write_buffer + 9) = RecycleSettingInfo.DenomiInfo[3].RecycleCurrent;
        }
        else
        {
            *(ex_usb_write_buffer + 6) = RecycleSettingInfo.DenomiInfo[0].RecycleCurrent;
            *(ex_usb_write_buffer + 7) = RecycleSettingInfo.DenomiInfo[1].RecycleCurrent;
            *(ex_usb_write_buffer + 8) = 0;
            *(ex_usb_write_buffer + 9) = 0;
        }
        break;
    
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_set_stacker_value()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_NONE:

        set_response_1data(ACK);
        break;
    
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_get_stacker_value()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_NONE:
        set_response_1data(ACK);
        break;
    
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_communication()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC1_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_COMMUNICATION, 0, 0);
        
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_rc_led_test()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC1_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_SW_LED, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_END:
        reset = 1;
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_dipsw_test()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC1_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_DIPSW, 0, 0);
        
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        
        ex_usb_write_size = (FUSB_HEADER_SIZE + 1);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)ex_rc_dip_sw;
        
        break;
    case CMD_END:
        
        reset = 1;
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        set_response_1data(ACK);
        
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_exbox_solenoid_test()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC1_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_EXBOX_SOL, 0, 0);
        
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        
        ex_usb_write_size = (FUSB_HEADER_SIZE + 1);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)ex_rc_status.sst22B.bit.ex_box_sol_state;
        
        break;
    case CMD_END:
        
        reset = 1;
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_twin_feed_motor_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_MOTOR_FWD:
        
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_TWIN_FEED_FWD, 0, 0);
        
        set_response_1data(ACK);
        break;
    case CMD_MOTOR_REV:
        
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_TWIN_FEED_REV, 0, 0);
        
        set_response_1data(ACK);
        break;
    case CMD_END:
        
        reset = 1;
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_twin_flap_motor_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:

        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC3_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_TWIN_FLAP_USB, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        ex_usb_write_size = (FUSB_HEADER_SIZE + 10);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)ex_rc_status.sst21A.bit.flap1_senA;
        *(ex_usb_write_buffer + 7) = (u8)ex_rc_status.sst21A.bit.flap1_senB;
        *(ex_usb_write_buffer + 8) = (u8)ex_rc_status.sst21A.bit.flap1_senRC1;
        *(ex_usb_write_buffer + 9) = (u8)ex_rc_status.sst21B.bit.flap1_lever;
        *(ex_usb_write_buffer + 10) = (u8)(is_flapper1_head_to_twin_pos());
        *(ex_usb_write_buffer + 11) = (u8)(is_flapper1_head_to_box_pos());
        *(ex_usb_write_buffer + 12) = (u8)(is_flapper1_twin_to_box_pos());
        *(ex_usb_write_buffer + 13) = (u8)((ex_rc_flap_test_time_send) & 0xff);
        *(ex_usb_write_buffer + 14) = (u8)(((ex_rc_flap_test_time_send) >> 8) & 0xff);
        *(ex_usb_write_buffer + 15) = (u8)ex_rc_test_status;
        break;
    case CMD_END:
        reset = 1;
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_twin_tr_pos_sens_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_TWIN_SEN1, 0, 0);
        
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        
        ex_usb_write_size = (FUSB_HEADER_SIZE + 3);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_status.sst21A.bit.pos_sen1);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_status.sst21A.bit.pos_sen2);
        *(ex_usb_write_buffer + 8) = (u8)(ex_rc_status.sst21A.bit.pos_sen3);
        
        break;
    case CMD_END:
        
        reset = 1;
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_twin_rc_pos_sens_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_TWIN_SEN2, 0, 0);
        
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        
        ex_usb_write_size = (FUSB_HEADER_SIZE + 3);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_status.sst31A.bit.pos_senA);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_status.sst31A.bit.pos_senB);
        *(ex_usb_write_buffer + 8) = (u8)(ex_rc_status.sst31A.bit.pos_senC);
        
        break;
    case CMD_END:
        
        reset = 1;
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_twin_solenoid_test()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_TWIN_SOL, 0, 0);
        
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        
        ex_usb_write_size = (FUSB_HEADER_SIZE + 2);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_status.sst31B.bit.u1_sol_state);
        *(ex_usb_write_buffer + 7) = (u8)ex_rc_test_status;
        break;
    case CMD_END:
    
        reset = 1;
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
    
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_twin_drum1_motor_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
    
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 
                        DIPSWRC_TWIN_DRUM1, 0, 0);
    
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
    
        ex_usb_write_size = (FUSB_HEADER_SIZE + 8);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_motor_speed[5][0]);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_motor_speed[5][1]);
        *(ex_usb_write_buffer + 8) = (u8)(u8)ex_rc_motor_duty[5];
        *(ex_usb_write_buffer + 9) = (u8)(ex_rc_motor_speed[6][0]);
        *(ex_usb_write_buffer + 10) = (u8)(ex_rc_motor_speed[6][1]);
        *(ex_usb_write_buffer + 11) = (u8)(u8)ex_rc_motor_duty[6];
        *(ex_usb_write_buffer + 12) = (u8)(ex_rc_test_status / 0x10);
        *(ex_usb_write_buffer + 13) = (u8)ex_rc_test_status;
    
        break;
    case CMD_END:
    
        reset = 1;
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
    
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_twin_drum2_motor_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
    
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 
                        DIPSWRC_TWIN_DRUM2, 0, 0);
    
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
    
        ex_usb_write_size = (FUSB_HEADER_SIZE + 8);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_motor_speed[5][0]);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_motor_speed[5][1]);   
        *(ex_usb_write_buffer + 8) = (u8)(u8)ex_rc_motor_duty[5];
        *(ex_usb_write_buffer + 9) = (u8)(ex_rc_motor_speed[6][0]);
        *(ex_usb_write_buffer + 10) = (u8)(ex_rc_motor_speed[6][1]);
        *(ex_usb_write_buffer + 11) = (u8)(u8)ex_rc_motor_duty[6];
        *(ex_usb_write_buffer + 12) = (u8)(ex_rc_test_status / 0x10);
        *(ex_usb_write_buffer + 13) = (u8)ex_rc_test_status;
    
        break;
    case CMD_END:
    
        reset = 1;
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
    
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_quad_feed_motor_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_MOTOR_FWD:
    
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_QUAD_FEED_FWD, 0, 0);
    
        set_response_1data(ACK);
        break;
    case CMD_MOTOR_REV:
    
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_QUAD_FEED_REV, 0, 0);
    
        set_response_1data(ACK);
        break;
    case CMD_END:
    
        reset = 1;
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
    
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_quad_flap_motor_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC3_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 
                            DIPSWRC_QUAD_FLAP_USB, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        ex_usb_write_size = (FUSB_HEADER_SIZE + 10);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)ex_rc_status.sst22A.bit.flap2_senC;
        *(ex_usb_write_buffer + 7) = (u8)ex_rc_status.sst22A.bit.flap2_senD;
        *(ex_usb_write_buffer + 8) = (u8)ex_rc_status.sst22A.bit.flap2_senRC2;
        *(ex_usb_write_buffer + 9) = (u8)ex_rc_status.sst21B.bit.flap2_lever;
        *(ex_usb_write_buffer + 10) = (u8)(is_flapper2_head_to_quad_pos());
        *(ex_usb_write_buffer + 11) = (u8)(is_flapper2_head_to_box_pos());
        *(ex_usb_write_buffer + 12) = (u8)(is_flapper2_quad_to_box_pos());
        *(ex_usb_write_buffer + 13) = (u8)((ex_rc_flap_test_time_send) & 0xff);
        *(ex_usb_write_buffer + 14) = (u8)(((ex_rc_flap_test_time_send) >> 8) & 0xff);
        *(ex_usb_write_buffer + 15) = (u8)ex_rc_test_status;
        break;
    case CMD_END:
        reset = 1;
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_quad_tr_pos_sens_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_QUAD_SEN1, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        ex_usb_write_size = (FUSB_HEADER_SIZE + 3);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_status.sst22A.bit.pos_sen4);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_status.sst22A.bit.pos_sen5);
        *(ex_usb_write_buffer + 8) = (u8)(ex_rc_status.sst22A.bit.pos_sen6);
        break;
    case CMD_END:
        reset = 1;
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_quad_rc_pos_sens_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
    
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_QUAD_SEN2, 0, 0);
    
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
    
        ex_usb_write_size = (FUSB_HEADER_SIZE + 3);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_status.sst32A.bit.pos_senD);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_status.sst32A.bit.pos_senE);
        *(ex_usb_write_buffer + 8) = (u8)(ex_rc_status.sst32A.bit.pos_senF);
    
        break;
    case CMD_END:
    
        reset = 1;
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
    
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_quad_solenoid_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ,
                         ex_dline_testmode.test_no, DIPSWRC_QUAD_SOL, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        ex_usb_write_size = (FUSB_HEADER_SIZE + 2);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_status.sst32B.bit.u2_sol_state);
        *(ex_usb_write_buffer + 7) = (u8)ex_rc_test_status;
        break;
    case CMD_END:
        reset = 1;
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_quad_drum1_motor_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 
                        DIPSWRC_QUAD_DRUM1, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        ex_usb_write_size = (FUSB_HEADER_SIZE + 8);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_motor_speed[5][0]);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_motor_speed[5][1]);
        *(ex_usb_write_buffer + 8) = (u8)(u8)ex_rc_motor_duty[5];
        *(ex_usb_write_buffer + 9) = (u8)(ex_rc_motor_speed[6][0]);
        *(ex_usb_write_buffer + 10) = (u8)(ex_rc_motor_speed[6][1]);
        *(ex_usb_write_buffer + 11) = (u8)(u8)ex_rc_motor_duty[6];
        *(ex_usb_write_buffer + 12) = (u8)(ex_rc_test_status / 0x10);
        *(ex_usb_write_buffer + 13) = (u8)ex_rc_test_status;
        break;
    case CMD_END:
        reset = 1;
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_quad_drum2_motor_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC2_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 
                        DIPSWRC_QUAD_DRUM2, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        ex_usb_write_size = (FUSB_HEADER_SIZE + 8);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_motor_speed[5][0]);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_motor_speed[5][1]);
        *(ex_usb_write_buffer + 8) = (u8)(u8)ex_rc_motor_duty[5];
        *(ex_usb_write_buffer + 9) = (u8)(ex_rc_motor_speed[6][0]);
        *(ex_usb_write_buffer + 10) = (u8)(ex_rc_motor_speed[6][1]);
        *(ex_usb_write_buffer + 11) = (u8)(u8)ex_rc_motor_duty[6];
        *(ex_usb_write_buffer + 12) = (u8)(ex_rc_test_status / 0x10);
        *(ex_usb_write_buffer + 13) = (u8)ex_rc_test_status;
        break;
    case CMD_END:
        reset = 1;
		_dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_twin_tape_sens_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
    
        ex_usb_write_size = (FUSB_HEADER_SIZE + 4);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_status.sst31A.bit.u1_d1_full);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_status.sst31A.bit.u1_d1_empty);
        *(ex_usb_write_buffer + 8) = (u8)(ex_rc_status.sst31A.bit.u1_d2_full);
        *(ex_usb_write_buffer + 9) = (u8)(ex_rc_status.sst31A.bit.u1_d2_empty);
    
        break;
    case CMD_END:
    
        reset = 1;
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
    
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_quad_tape_sens_test() //ok
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
    
        ex_usb_write_size = (FUSB_HEADER_SIZE + 4);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_status.sst32A.bit.u2_d1_full);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_status.sst32A.bit.u2_d1_empty);
        *(ex_usb_write_buffer + 8) = (u8)(ex_rc_status.sst32A.bit.u2_d2_full);
        *(ex_usb_write_buffer + 9) = (u8)(ex_rc_status.sst32A.bit.u2_d2_empty);
    
        break;
    case CMD_END:
    
        reset = 1;
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
    
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_serial_no()    /* 0x50 */
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_SERIAL_NO_WRITE_FACTORY:
        memset((u8 *)&write_serailno_data, 0, sizeof(write_serailno_data));
        write_serailno_data.fram_param = ex_usb_read_buffer[6];
        memcpy((u8 *)&write_serailno_data.version[0], &ex_usb_read_buffer[7], sizeof(write_serailno_data.version));
        memcpy((u8 *)&write_serailno_data.date[0], &ex_usb_read_buffer[9], sizeof(write_serailno_data.date));
        memcpy((u8 *)&write_serailno_data.serial_no[0], &ex_usb_read_buffer[17], sizeof(write_serailno_data.serial_no));

        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC3_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_WRITE_SERIAL_NO, 0, 0);
        set_response_1data(ACK);

        memset((u8 *)&read_serailno_data, 0, sizeof(read_serailno_data));
        break;
    case CMD_SERIAL_NO_WRITE_MAINTENANCE:
        memset((u8 *)&write_serailno_data, 0, sizeof(write_serailno_data));
        write_serailno_data.fram_param = ex_usb_read_buffer[6];
        memcpy((u8 *)&write_serailno_data.version[0], &ex_usb_read_buffer[7], sizeof(write_serailno_data.version));
        memcpy((u8 *)&write_serailno_data.date[0], &ex_usb_read_buffer[9], sizeof(write_serailno_data.date));
        memcpy((u8 *)&write_serailno_data.serial_no[0], &ex_usb_read_buffer[17], sizeof(write_serailno_data.serial_no));

        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC3_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_WRITE_SERIAL_NO, 1, 0);
        set_response_1data(ACK);

        memset((u8 *)&read_serailno_data, 0, sizeof(read_serailno_data));
        break;
    case CMD_SERIAL_NO_READ_FACTORY:
    case CMD_SERIAL_NO_READ_MAINTENANCE:
        if (read_serailno_data.read_end == READ_NONE)
        {
            if (ex_rc_status.sst1A.bit.busy == 0)
            {
                memset((u8 *)&read_serailno_data, 0, sizeof(read_serailno_data));
                read_serailno_data.fram_param = ex_usb_read_buffer[6];
                read_serailno_data.read_end |= READ_RTQ_EXEC;

                ex_dline_testmode.action = TEST_USB_CONTROL;
                ex_dline_testmode.test_no = TEST_RC3_USB;
                _dline_set_mode(DLINE_MODE_TEST_EXEC);
                if (ex_front_usb.pc.mess.command == CMD_SERIAL_NO_READ_FACTORY)
                {
                    _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_READ_SERIAL_NO, 0, 0);
                }
                else
                {
                    _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_READ_SERIAL_NO, 1, 0);
                }
            }

            set_response_1data(CMD_RUN);
        }
        else if (read_serailno_data.read_end & READ_RTQ_EXEC)
        {
            if (ex_rc_status.sst1A.bit.busy == 0)
            {
                memset((u8 *)&read_serailno_data, 0, sizeof(read_serailno_data));
                read_serailno_data.fram_param = ex_usb_read_buffer[6];
                read_serailno_data.read_end |= READ_RTQ_EXEC;

                ex_dline_testmode.action = TEST_USB_CONTROL;
                ex_dline_testmode.test_no = TEST_RC3_USB;
                _dline_set_mode(DLINE_MODE_TEST_EXEC);
                if (ex_front_usb.pc.mess.command == CMD_SERIAL_NO_READ_FACTORY)
                {
                    _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_READ_SERIAL_NO, 0, 0);
                }
                else
                {
                    _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_READ_SERIAL_NO, 1, 0);
                }
            }

            set_response_1data(CMD_RUN);
        }
        else if (read_serailno_data.read_end == READ_ERR)
        {
            set_response_1data(CMD_ERR);

            read_serailno_data.read_end = READ_NONE;
        }
        else if (read_serailno_data.read_end & READ_RTQ_END)
        {
            ex_usb_write_size = (FUSB_HEADER_SIZE + 23);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = 0x06;
            *(ex_usb_write_buffer + 6) = read_serailno_data.fram_param;
            memcpy((u8 *)&ex_usb_write_buffer[7], (u8 *)&read_serailno_data.version[0], sizeof(read_serailno_data.version));
            memcpy((u8 *)&ex_usb_write_buffer[9], (u8 *)&read_serailno_data.date[0], sizeof(read_serailno_data.date));
            memcpy((u8 *)&ex_usb_write_buffer[17], (u8 *)&read_serailno_data.serial_no[0], sizeof(read_serailno_data.serial_no));

            read_serailno_data.read_end = READ_NONE;
        }
        break;
    case CMD_EDITION_NO_WRITE_FACTORY:  /* 0x03 */
        memset((u8 *)&write_editionno_data, 0, sizeof(write_editionno_data));
        memcpy((u8 *)&write_editionno_data.head[0], &ex_usb_read_buffer[6], sizeof(write_editionno_data.head));
        memcpy((u8 *)&write_editionno_data.main[0], &ex_usb_read_buffer[7], sizeof(write_editionno_data.main));
        memcpy((u8 *)&write_editionno_data.twin[0], &ex_usb_read_buffer[8], sizeof(write_editionno_data.twin));
        memcpy((u8 *)&write_editionno_data.quad[0], &ex_usb_read_buffer[9], sizeof(write_editionno_data.quad));

        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC3_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);

        _dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_RTQ, FRAM_RC_EDITION, 0, 0); //2024-10-16
        //write_eeprom_value(EPROM_FACTORY_EDITION, EPROM_FACTORY_EDITION_SIZE, &ex_usb_read_buffer[6]);
        //write_eeprom_value(EPROM_MAINTENANCE_EDITION, EPROM_MAINTENANCE_EDITION_SIZE, &ex_usb_read_buffer[6]);

        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_WRITE_EDITION_NO, 0, 0);
        set_response_1data(ACK);
        
        memset((u8 *)&read_editionno_data, 0, sizeof(read_editionno_data));

        break;

    case CMD_EDITION_NO_WRITE_MAINTENANCE:  /* 0x21 */
        memset((u8 *)&write_editionno_data, 0, sizeof(write_editionno_data));
        memcpy((u8 *)&write_editionno_data.head[0], &ex_usb_read_buffer[6], sizeof(write_editionno_data.head));
        memcpy((u8 *)&write_editionno_data.main[0], &ex_usb_read_buffer[7], sizeof(write_editionno_data.main));
        memcpy((u8 *)&write_editionno_data.twin[0], &ex_usb_read_buffer[8], sizeof(write_editionno_data.twin));
        memcpy((u8 *)&write_editionno_data.quad[0], &ex_usb_read_buffer[9], sizeof(write_editionno_data.quad));

        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC3_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);

        _dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_WRITE_REQ, FRAM_RTQ, FRAM_RC_EDITION, 1, 0); //2024-10-16
        //write_eeprom_value(EPROM_MAINTENANCE_EDITION, EPROM_MAINTENANCE_EDITION_SIZE, &ex_usb_read_buffer[6]);

        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_WRITE_EDITION_NO, 1, 0);
        set_response_1data(ACK);

        memset((u8 *)&read_editionno_data, 0, sizeof(read_editionno_data));

        break;

    case CMD_EDITION_NO_READ_FACTORY:       /* 0x02 */
    case CMD_EDITION_NO_READ_MAINTENANCE:   /* 0x20 */
        if (read_editionno_data.read_end == READ_NONE)
    //    if (read_editionno_data.read_end == READ_NONE || 
    //        (!(read_editionno_data.read_end & READ_RTQ_HEAD_EXEC))
    //    )
        {
        //setp1 RTQへ読み込み依頼
            if (ex_rc_status.sst1A.bit.busy == 0)
            {
                memset((u8 *)&read_editionno_data, 0, sizeof(read_editionno_data));
                read_editionno_data.read_end = READ_RTQ_HEAD_EXEC;

                ex_dline_testmode.action = TEST_USB_CONTROL;
                ex_dline_testmode.test_no = TEST_RC3_USB;
                _dline_set_mode(DLINE_MODE_TEST_EXEC);
                if (ex_front_usb.pc.mess.command == CMD_EDITION_NO_READ_FACTORY)
                {
                    _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_READ_EDITION_NO, 0, 0);
                }
                else
                {
                    _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_READ_EDITION_NO, 1, 0);
                }
                _dline_send_msg(ID_FRAM_MBX, TMSG_FRAM_READ_REQ, FRAM_RTQ, FRAM_RC_EDITION, 0, 0);
            }

            set_response_1data(CMD_RUN);    /*ここのAckはまだという意味*/
        }
    //    else if (read_editionno_data.read_end == READ_EXEC)
        else if (read_editionno_data.read_end & READ_RTQ_HEAD_EXEC)       
        {
        //setp2 読み込み中
        #if 1
            //if (ex_rc_status.sst1A.bit.busy == 0)
            if (ex_rc_status.sst1A.bit.busy == 0 && !(read_editionno_data.read_end & 0x10) )
            {
                memset((u8 *)&read_editionno_data, 0, sizeof(read_editionno_data));
                read_editionno_data.read_end = READ_RTQ_EXEC;

                ex_dline_testmode.action = TEST_USB_CONTROL;
                ex_dline_testmode.test_no = TEST_RC3_USB;
                _dline_set_mode(DLINE_MODE_TEST_EXEC);
                if (ex_front_usb.pc.mess.command == CMD_EDITION_NO_READ_FACTORY)
                {
                    _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_READ_EDITION_NO, 0, 0);
                }
                else
                {
                    _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_READ_EDITION_NO, 1, 0);
                }
            }
            //Head側のリトライ
            set_response_1data(CMD_RUN);    /*ここのAckはまだという意味*/

        #else
            if (ex_rc_status.sst1A.bit.busy == 0)
            {
                memset((u8 *)&read_editionno_data, 0, sizeof(read_editionno_data));
                read_editionno_data.read_end = READ_EXEC;

                ex_dline_testmode.action = TEST_USB_CONTROL;
                ex_dline_testmode.test_no = TEST_RC3_USB;
                _dline_set_mode(DLINE_MODE_TEST_EXEC);
                if (ex_front_usb.pc.mess.command == CMD_EDITION_NO_READ_FACTORY)
                {
                    _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_READ_EDITION_NO, 0, 0);
                }
                else
                {
                    _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_READ_EDITION_NO, 1, 0);
                }
            }

            set_response_1data(CMD_RUN);    /*ここのAckはまだという意味*/

        #endif

        }
        else if (read_editionno_data.read_end == READ_ERR)
        {
            set_response_1data(CMD_ERR);

            read_editionno_data.read_end = READ_NONE;
        }
        else if (read_editionno_data.read_end == READ_RTQ_HEAD_END)
        {
            ex_usb_write_size = (FUSB_HEADER_SIZE + 4);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = 0x06;

			//最初に読み込み処理を行っているので、ここでは必要ない
            if (ex_front_usb.pc.mess.command == CMD_EDITION_NO_READ_FACTORY)
            {
                //read_eeprom_value(EPROM_FACTORY_EDITION, EPROM_FACTORY_EDITION_SIZE, ex_usb_write_buffer + 6);
            }
            else
            {
                //read_eeprom_value(EPROM_MAINTENANCE_EDITION, EPROM_MAINTENANCE_EDITION_SIZE, ex_usb_write_buffer + 6);
            }

            //2024-10-16 UBA700は毎回呼び出すので必要ない memcpy((u8 *)&read_editionno_data.head[0], (u8 *)&ex_usb_write_buffer + 6, sizeof(read_editionno_data.head));
            memcpy((u8 *)&ex_usb_write_buffer[6], (u8 *)&read_editionno_data.head[0], sizeof(read_editionno_data.head));
            memcpy((u8 *)&ex_usb_write_buffer[7], (u8 *)&read_editionno_data.main[0], sizeof(read_editionno_data.main));
            memcpy((u8 *)&ex_usb_write_buffer[8], (u8 *)&read_editionno_data.twin[0], sizeof(read_editionno_data.twin));
            memcpy((u8 *)&ex_usb_write_buffer[9], (u8 *)&read_editionno_data.quad[0], sizeof(read_editionno_data.quad));

            read_editionno_data.read_end = READ_NONE;
        }
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_sens_adj_fram()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_SENS_ADJ_FRAM_WRITE_FACTORY:
    case CMD_SENS_ADJ_FRAM_WRITE_MAINTENANCE:
        if (ex_sens_adj_fram_end == ADJ_NONE)
        {
            memset((u8 *)&ex_rc_adj_data, 0, sizeof(ex_rc_adj_data));
            ex_sens_adj_fram_end = ADJ_EXEC;
            memcpy((u8 *)&ex_rc_adj_data, (u8 *)&ex_usb_read_buffer[6], sizeof(ex_rc_adj_data));

            ex_dline_testmode.action = TEST_USB_CONTROL;
            ex_dline_testmode.test_no = TEST_RC3_USB;
            _dline_set_mode(DLINE_MODE_TEST_EXEC);
            if (ex_front_usb.pc.mess.command == CMD_SENS_ADJ_FRAM_WRITE_FACTORY)
            {
                _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 
                                        DIPSWRC_SENS_ADJ_WRITE_FRAM, 0, 0);
            }
            else
            {
                _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 
                                        DIPSWRC_SENS_ADJ_WRITE_FRAM, 1, 0);
            }
            set_response_1data(CMD_RUN);
        }
        else if (ex_sens_adj_fram_end == ADJ_ERR)
        {
            ex_sens_adj_fram_end = ADJ_NONE;
            set_response_1data(NAK);
        }
        else if (ex_sens_adj_fram_end == ADJ_EXEC)
        {
            set_response_1data(CMD_RUN);
        }
        else if (ex_sens_adj_fram_end == ADJ_WRITE_END)
        {
            ex_sens_adj_fram_end = ADJ_NONE;
            set_response_1data(ACK);
        }
        else
        {
            ex_sens_adj_fram_end = ADJ_NONE;
            set_response_1data(NAK);
        }
        break;
    case CMD_SENS_ADJ_FRAM_READ_FACTORY:
    case CMD_SENS_ADJ_FRAM_READ_MAINTENANCE:
        if (ex_sens_adj_fram_end == ADJ_NONE)
        {
            memset((u8 *)&ex_rc_adj_data, 0, sizeof(ex_rc_adj_data));
            ex_sens_adj_fram_end = ADJ_READ_EXEC;

            ex_dline_testmode.action = TEST_USB_CONTROL;
            ex_dline_testmode.test_no = TEST_RC3_USB;
            _dline_set_mode(DLINE_MODE_TEST_EXEC);
            if (ex_front_usb.pc.mess.command == CMD_SENS_ADJ_FRAM_READ_FACTORY)
            {
                _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 
                                DIPSWRC_SENS_ADJ_READ_FRAM, 0, 0);
            }
            else
            {
                _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 
                                DIPSWRC_SENS_ADJ_READ_FRAM, 1, 0);
            }

            set_response_1data(CMD_RUN);
        }
        else if (ex_sens_adj_fram_end == ADJ_ERR)
        {
            ex_sens_adj_fram_end = ADJ_NONE;
            set_response_1data(NAK);
        }
        else if (ex_sens_adj_fram_end == ADJ_READ_EXEC)
        {
            set_response_1data(CMD_RUN);
        }
        else if (ex_sens_adj_fram_end == ADJ_READ_END)
        {
            ex_sens_adj_fram_end = ADJ_NONE;

            ex_usb_write_size = (FUSB_HEADER_SIZE + 153);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = 0x06;
            memcpy((u8 *)&ex_usb_write_buffer[6], (u8 *)&ex_rc_adj_data, sizeof(ex_rc_adj_data));
        }
        else
        {
            ex_sens_adj_fram_end = ADJ_NONE;
            set_response_1data(NAK);
        }
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_perform_test_fram()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_PERFORM_TEST_FRAM_WRITE:
        if (ex_perform_test_fram_end == PTEST_NONE)
        {
            memset((u8 *)&ex_perform_test_data, 0, sizeof(ex_perform_test_data));

            ex_perform_test_fram_end = PTEST_EXEC;
            memcpy((u8 *)&ex_perform_test_data, (u8 *)&ex_usb_read_buffer[6], sizeof(ex_perform_test_data));

            ex_dline_testmode.action = TEST_USB_CONTROL;
            ex_dline_testmode.test_no = TEST_RC3_USB;
            _dline_set_mode(DLINE_MODE_TEST_EXEC);
            _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_PERFORM_TEST_WRITE_FRAM, 0, 0);
            set_response_1data(CMD_RUN);
        }
        else if (ex_perform_test_fram_end == PTEST_ERR)
        {
            ex_perform_test_fram_end = PTEST_NONE;
            set_response_1data(NAK);
        }
        else if (ex_perform_test_fram_end == PTEST_EXEC)
        {
            set_response_1data(CMD_RUN);
        }
        else if (ex_perform_test_fram_end == PTEST_WRITE_END)
        {
            ex_perform_test_fram_end = PTEST_NONE;
            set_response_1data(ACK);
        }
        else
        {
            ex_perform_test_fram_end = PTEST_NONE;
            set_response_1data(NAK);
        }
        break;
    case CMD_PERFORM_TEST_FRAM_READ:
        if (ex_perform_test_fram_end == PTEST_NONE)
        {
            memset((u8 *)&ex_perform_test_data, 0, sizeof(ex_perform_test_data));
            ex_perform_test_fram_end = PTEST_READ_EXEC;

            ex_dline_testmode.action = TEST_USB_CONTROL;
            ex_dline_testmode.test_no = TEST_RC3_USB;
            _dline_set_mode(DLINE_MODE_TEST_EXEC);
            _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_PERFORM_TEST_READ_FRAM, 0, 0);

            set_response_1data(CMD_RUN);
        }
        else if (ex_perform_test_fram_end == PTEST_ERR)
        {
            ex_perform_test_fram_end = PTEST_NONE;
            set_response_1data(NAK);
        }
        else if (ex_perform_test_fram_end == PTEST_READ_EXEC)
        {
            set_response_1data(CMD_RUN);
        }
        else if (ex_perform_test_fram_end == PTEST_READ_END)
        {
            if(ex_rc_configuration.board_type == RC_OLD_BOARD)
            {
                ex_perform_test_fram_end = PTEST_NONE;

                ex_usb_write_size = (FUSB_HEADER_SIZE + 36);

                *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
                *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
                *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
                *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
                *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
                *(ex_usb_write_buffer + 5) = 0x06;
                memcpy((u8 *)&ex_usb_write_buffer[6], (u8 *)&ex_perform_test_data, sizeof(ex_perform_test_data));
            }
            else
            {
                ex_perform_test_fram_end = PTEST_NONE;

                ex_usb_write_size = (FUSB_HEADER_SIZE + 38);

                *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
                *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
                *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
                *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
                *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
                *(ex_usb_write_buffer + 5) = 0x06;
                memcpy((u8 *)&ex_usb_write_buffer[6], (u8 *)&ex_perform_test_data, sizeof(ex_perform_test_data));
            }
        }
        else
        {
            ex_perform_test_fram_end = PTEST_NONE;
            set_response_1data(NAK);
        }
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_twin_feed_get_data()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        ex_usb_write_size = (FUSB_HEADER_SIZE + 4);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_motor_speed[6][0]);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_motor_speed[6][1]);
        *(ex_usb_write_buffer + 8) = (u8)(u8)ex_rc_motor_duty[6];
        *(ex_usb_write_buffer + 9) = (u8)ex_rc_test_status;
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_quad_feed_get_data()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        ex_usb_write_size = (FUSB_HEADER_SIZE + 4);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_motor_speed[6][0]);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_motor_speed[6][1]);
        *(ex_usb_write_buffer + 8) = (u8)(u8)ex_rc_motor_duty[6];
        *(ex_usb_write_buffer + 9) = (u8)ex_rc_test_status;
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_sens_adj_start()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_SENS_ADJ_START_FACTORY:
        ex_sens_adj_end = ADJ_EXEC;
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC3_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_START_SENS_ADJ, 0, 0);
        set_response_1data(CMD_NONE);
        break;
    case CMD_SENS_ADJ_START_MAINTENANCE:
        ex_sens_adj_end = ADJ_EXEC;
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC3_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_START_SENS_ADJ, 1, 0);
        set_response_1data(CMD_NONE);
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_sens_adj_get_val()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_SENS_ADJ_READ_FACTORY:
        if (ex_sens_adj_end == ADJ_NONE)
        {
            set_response_1data(NAK);
        }
        else if (ex_sens_adj_end == ADJ_EXEC)
        {
            set_response_1data(CMD_RUN);
        }
        else if (ex_sens_adj_end == ADJ_ERR)
        {
            u8 cnt;

            ex_usb_write_size = (FUSB_HEADER_SIZE + 158);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = CMD_ERR;
            for (cnt = 0; cnt < RC_SENSOR_MAX; cnt++)
            {
                *(ex_usb_write_buffer + (6 + (9 * cnt))) = ex_rc_adj_data[cnt].ad600da;
                *(ex_usb_write_buffer + (7 + (9 * cnt))) = ex_rc_adj_data[cnt].ad800da;
                *(ex_usb_write_buffer + (8 + (9 * cnt))) = ex_rc_adj_data[cnt].da;
                *(ex_usb_write_buffer + (9 + (9 * cnt))) = ex_rc_adj_data[cnt].ad600[0];
                *(ex_usb_write_buffer + (10 + (9 * cnt))) = ex_rc_adj_data[cnt].ad600[1];
                *(ex_usb_write_buffer + (11 + (9 * cnt))) = ex_rc_adj_data[cnt].ad800[0];
                *(ex_usb_write_buffer + (12 + (9 * cnt))) = ex_rc_adj_data[cnt].ad800[1];
                *(ex_usb_write_buffer + (13 + (9 * cnt))) = ex_rc_adj_data[cnt].ad[0];
                *(ex_usb_write_buffer + (14 + (9 * cnt))) = ex_rc_adj_data[cnt].ad[1];
            }
            *(ex_usb_write_buffer + 159) = ex_sens_adj_err_data[0];
            *(ex_usb_write_buffer + 160) = ex_sens_adj_err_data[1];
            *(ex_usb_write_buffer + 161) = ex_sens_adj_err_data[2];
            *(ex_usb_write_buffer + 162) = ex_sens_adj_err_data[3];
            *(ex_usb_write_buffer + 163) = ex_sens_adj_err_data[4];
        }
        else if (ex_sens_adj_end == ADJ_WRITE_END)
        {
            ex_sens_adj_end = ADJ_READ_EXEC;
            ex_dline_testmode.action = TEST_USB_CONTROL;
            ex_dline_testmode.test_no = TEST_RC3_USB;
            _dline_set_mode(DLINE_MODE_TEST_EXEC);
            _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                            ex_dline_testmode.test_no, DIPSWRC_GET_SENS_ADJ_DATA, 0, 0);

            set_response_1data(CMD_RUN);
        }
        else if (ex_sens_adj_end == ADJ_READ_EXEC)
        {
            set_response_1data(CMD_RUN);
        }
        else if (ex_sens_adj_end == ADJ_READ_END)
        {
            u8 cnt;

            ex_usb_write_size = (FUSB_HEADER_SIZE + 153);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = 0x06;
            for (cnt = 0; cnt < RC_SENSOR_MAX; cnt++)
            {
                *(ex_usb_write_buffer + (6 + (9 * cnt))) = ex_rc_adj_data[cnt].ad600da;
                *(ex_usb_write_buffer + (7 + (9 * cnt))) = ex_rc_adj_data[cnt].ad800da;
                *(ex_usb_write_buffer + (8 + (9 * cnt))) = ex_rc_adj_data[cnt].da;
                *(ex_usb_write_buffer + (9 + (9 * cnt))) = ex_rc_adj_data[cnt].ad600[0];
                *(ex_usb_write_buffer + (10 + (9 * cnt))) = ex_rc_adj_data[cnt].ad600[1];
                *(ex_usb_write_buffer + (11 + (9 * cnt))) = ex_rc_adj_data[cnt].ad800[0];
                *(ex_usb_write_buffer + (12 + (9 * cnt))) = ex_rc_adj_data[cnt].ad800[1];
                *(ex_usb_write_buffer + (13 + (9 * cnt))) = ex_rc_adj_data[cnt].ad[0];
                *(ex_usb_write_buffer + (14 + (9 * cnt))) = ex_rc_adj_data[cnt].ad[1];
            }
        }
        break;
    case CMD_SENS_ADJ_READ_MAINTENANCE:
        if (ex_sens_adj_end == ADJ_NONE)
        {
            set_response_1data(NAK);
        }
        else if (ex_sens_adj_end == ADJ_EXEC)
        {
            set_response_1data(CMD_RUN);
        }
        else if (ex_sens_adj_end == ADJ_ERR)
        {
            set_response_1data(CMD_ERR);
        }
        else if (ex_sens_adj_end == ADJ_WRITE_END)
        {
            ex_sens_adj_end = ADJ_READ_EXEC;
            ex_dline_testmode.action = TEST_USB_CONTROL;
            ex_dline_testmode.test_no = TEST_RC3_USB;
            _dline_set_mode(DLINE_MODE_TEST_EXEC);
            _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                            ex_dline_testmode.test_no, DIPSWRC_GET_SENS_ADJ_DATA, 0, 0);

            set_response_1data(CMD_RUN);
        }
        else if (ex_sens_adj_end == ADJ_READ_EXEC)
        {
            set_response_1data(CMD_RUN);
        }
        else if (ex_sens_adj_end == ADJ_READ_END)
        {
            u8 cnt;

            ex_usb_write_size = (FUSB_HEADER_SIZE + 153);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = 0x06;
            for (cnt = 0; cnt < RC_SENSOR_MAX; cnt++)
            {
                *(ex_usb_write_buffer + (6 + (9 * cnt))) = ex_rc_adj_data[cnt].ad600da;
                *(ex_usb_write_buffer + (7 + (9 * cnt))) = ex_rc_adj_data[cnt].ad800da;
                *(ex_usb_write_buffer + (8 + (9 * cnt))) = ex_rc_adj_data[cnt].da;
                *(ex_usb_write_buffer + (9 + (9 * cnt))) = ex_rc_adj_data[cnt].ad600[0];
                *(ex_usb_write_buffer + (10 + (9 * cnt))) = ex_rc_adj_data[cnt].ad600[1];
                *(ex_usb_write_buffer + (11 + (9 * cnt))) = ex_rc_adj_data[cnt].ad800[0];
                *(ex_usb_write_buffer + (12 + (9 * cnt))) = ex_rc_adj_data[cnt].ad800[1];
                *(ex_usb_write_buffer + (13 + (9 * cnt))) = ex_rc_adj_data[cnt].ad[0];
                *(ex_usb_write_buffer + (14 + (9 * cnt))) = ex_rc_adj_data[cnt].ad[1];
            }
        }
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_drum1_tape_pos_adj()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_test_end_flg = 0;
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC3_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_DRUM1_TAPE_POS_ADJ, 0, 0);
        set_response_1data(ACK);
        break;
    case  CMD_ENQ:
        if (ex_test_end_flg == 0)
        {
            ex_usb_write_size = (FUSB_HEADER_SIZE + 1);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = 0x06;
            *(ex_usb_write_buffer + 6) = (u8)(0xFF);
        }
        else
        {
            ex_usb_write_size = (FUSB_HEADER_SIZE + 1);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = 0x06;
            *(ex_usb_write_buffer + 6) = (u8)(0x10);
        }
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_drum2_tape_pos_adj()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_test_end_flg = 0;
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC3_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 
                        DIPSWRC_DRUM2_TAPE_POS_ADJ, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        if (ex_test_end_flg == 0)
        {
            ex_usb_write_size = (FUSB_HEADER_SIZE + 1);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = 0x06;
            *(ex_usb_write_buffer + 6) = (u8)(0xFF);
        }
        else
        {
            ex_usb_write_size = (FUSB_HEADER_SIZE + 1);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = 0x06;
            *(ex_usb_write_buffer + 6) = (u8)(0x10);
        }
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_check_fram()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_test_end_flg = 0;
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RC3_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, 
                        ex_dline_testmode.test_no, DIPSWRC_FRAM_CHECK, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        if (ex_test_end_flg == 0)
        {
            ex_usb_write_size = (FUSB_HEADER_SIZE + 1);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = 0x06;
            *(ex_usb_write_buffer + 6) = (u8)(0xFF);
        }
        else
        {
            ex_usb_write_size = (FUSB_HEADER_SIZE + 1);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = 0x06;
            *(ex_usb_write_buffer + 6) = (u8)(ex_fram_check);
        }
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

u8 phase_check_twin_or_quad() //既存のTool suteやセンサ調整Toolで使用している
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        reset = 1;	//必要ない様な気がするがUBA500に合わせる

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;

		*(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_status.sst1A.bit.quad);

        if (ex_rc_configuration.board_type == RC_OLD_BOARD)
        {
            ex_usb_write_size = (FUSB_HEADER_SIZE + 1);
        }
        else
        {
            ex_usb_write_size = (FUSB_HEADER_SIZE + 2);

            /* New Board */
            *(ex_usb_write_buffer + 7) = 0x10;

            /* Connected RFID */
            if (ex_rc_configuration.rfid_module == CONNECT_RFID)
            {
                *(ex_usb_write_buffer + 7) += 0x01;
            }

            /* Connected RS unit */
            if (is_rc_rs_unit())
            {
                *(ex_usb_write_buffer + 7) += 0x02;
            }
        }

		*(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);

		break;
    case CMD_ENQ:
        break;
    default:
        set_response_1data(NAK);
        break;
    }

    return reset;
}

//#if defined(RC_BOARD_GREEN)		/* '23-09-20 */
/******************************************************************************/
/*! @brief enable wait bill in procedure
    @return         none
    @exception      none
******************************************************************************/
u8 phase_get_rs_pos_onoff(void)
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_test_end_flg = 1;
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RS_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 
                        DIPSWRC_GET_POS_ONOFF, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        if (ex_test_end_flg == 0)
        {
            ex_usb_write_size = (FUSB_HEADER_SIZE + 3);

            *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
            *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
            *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
            *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
            *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
            *(ex_usb_write_buffer + 5) = 0x06; /* ACK */
            *(ex_usb_write_buffer + 6) = ex_rc_new_adjustment_data.pos[0];
            *(ex_usb_write_buffer + 7) = ex_rc_new_adjustment_data.pos[1];
            *(ex_usb_write_buffer + 8) = ex_rc_new_adjustment_data.pos[2];
        }
        else
        {
            set_response_1data(RC_BUSY);
        }
        break;
    default:
        set_response_1data(NAK);
        break;
    }
    return (reset);
}

/******************************************************************************/
/*! @brief enable wait bill in procedure
    @return         none
    @exception      none
******************************************************************************/
u8 phase_rs_set_pos_da(void)
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_test_end_flg = 1;
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RS_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);

        memcpy((u8 *)&ex_rc_new_adjustment_data.da[0], (u8 *)&ex_usb_read_buffer[6], 26);

        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, 
                        DIPSWRC_SET_POS_DA, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        if (ex_test_end_flg == 0)
        {
            set_response_1data(ACK);
        }
        else
        {
            set_response_1data(RC_BUSY);
        }
        break;
    default:
        set_response_1data(NAK);
        break;
    }
    return (reset);
}

/******************************************************************************/
/*! @brief enable wait bill in procedure
    @return         none
    @exception      none
******************************************************************************/
u8 phase_rs_set_pos_gain(void)
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_test_end_flg = 1;
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RS_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);

        memcpy((u8 *)&ex_rc_new_adjustment_data.gain[0], (u8 *)&ex_usb_read_buffer[6], 3);

        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no,
                     DIPSWRC_SET_POS_GAIN, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        if (ex_test_end_flg == 0)
        {
            set_response_1data(ACK);
        }
        else
        {
            set_response_1data(RC_BUSY);
        }
        break;
    default:
        set_response_1data(NAK);
        break;
    }
    return (reset);
}

//#if defined(UBA_RS)
u8 phase_rs_flap_motor_test()
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RS_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_RS_FLAP, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        ex_usb_write_size = (FUSB_HEADER_SIZE + 5);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)ex_rc_status.sst4A.bit.flap_sen1;
        *(ex_usb_write_buffer + 7) = (u8)ex_rc_status.sst4A.bit.flap_sen2;
        *(ex_usb_write_buffer + 8) = (u8)((ex_rc_flap_test_time_send) & 0xff);
        *(ex_usb_write_buffer + 9) = (u8)(((ex_rc_flap_test_time_send) >> 8) & 0xff);
        *(ex_usb_write_buffer + 10) = (u8)ex_rc_test_status;
        break;
    case CMD_END:
        reset = 1;
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }
    return (reset);
}

/******************************************************************************/
/*! @brief enable
    @return         none
    @exception      none
******************************************************************************/
u8 phase_rs_pos_sensor_test(void)
{
    u8 reset = 0;

    switch (ex_front_usb.pc.mess.command)
    {
    case CMD_RUN:
        ex_dline_testmode.action = TEST_USB_CONTROL;
        ex_dline_testmode.test_no = TEST_RS_USB;
        _dline_set_mode(DLINE_MODE_TEST_EXEC);
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_REQ, ex_dline_testmode.test_no, DIPSWRC_RS_SEN, 0, 0);
        set_response_1data(ACK);
        break;
    case CMD_ENQ:
        ex_usb_write_size = (FUSB_HEADER_SIZE + 4);

        *ex_usb_write_buffer = ex_front_usb.pc.mess.serviceID;
        *(ex_usb_write_buffer + 1) = (u8)((ex_usb_write_size >> 8) & 0xff);
        *(ex_usb_write_buffer + 2) = (u8)(ex_usb_write_size & 0xff);
        *(ex_usb_write_buffer + 3) = (u8)(u8)ex_front_usb.pc.mess.modeID;
        *(ex_usb_write_buffer + 4) = (u8)ex_front_usb.pc.mess.phase;
        *(ex_usb_write_buffer + 5) = 0x06;
        *(ex_usb_write_buffer + 6) = (u8)(ex_rc_status.sst4A.bit.pos_sen1);
        *(ex_usb_write_buffer + 7) = (u8)(ex_rc_status.sst4A.bit.pos_sen2);
        *(ex_usb_write_buffer + 8) = (u8)(ex_rc_status.sst4A.bit.pos_sen3);
        *(ex_usb_write_buffer + 9) = (u8)(ex_rc_status.sst4A.bit.pos_senR);
        break;
    case CMD_END:
        reset = 1;
        _dline_send_msg(ID_MAIN_MBX, TMSG_DLINE_TEST_FINISH_REQ, 0, 0, 0, 0);
        set_response_1data(ACK);
        break;
    default:
        set_response_1data(NAK);
        break;
    }
    return (reset);
}
//#endif	//UBA_RS 


//#endif // RC_BOARD_GREEN


void front_usb_recycle_request(void)
{
    u8 reset = 0;

    if ((u8)ex_front_usb.pc.mess.modeID == (u8)MODE_RECYCLE_REQUEST)
    {
        switch (ex_front_usb.pc.mess.phase)
        {
        case PHASE_PAYOUT:
            reset = phase_payout();
            break;
        case PHASE_COLLECT:
            reset = phase_collect();
            break;
        case PHASE_SET_RECYCLE_DENOMI:
            reset = phase_set_recycle_denomi();
            break;
        case PHASE_SET_RECYCLE_LIMIT:
            reset = phase_set_recycle_limit();
            break;
        case PHASE_SET_RECYCLE_LENGTH:
            reset = phase_set_recycle_length();
            break;
        case PHASE_SET_RECYCLE_COUNT:
            reset = phase_set_recycle_count();
            break;
        case PHASE_GET_RECYCLE_COUNT:
            reset = phase_get_recycle_count();
            break;
        case PHASE_SET_STACKER_VALUE:
            reset = phase_set_stacker_value();
            break;
        case PHASE_GET_STACKER_VALUE:
            reset = phase_get_stacker_value();
            break;
        case PHASE_COMMUNICATION:
            reset = phase_communication();
            break;
       case PHASE_RC_LED_TEST:
           reset = phase_rc_led_test();
           break;
        case PHASE_DIPSW_TEST:
            reset = phase_dipsw_test();
            break;
        case PHASE_EXBOX_SOLENOID_TEST:
            reset = phase_exbox_solenoid_test();
            break;
        case PHASE_TWIN_FEED_MOTOR_TEST: //0x03,0x00,0x06,0x70,0x34
            reset = phase_twin_feed_motor_test(); //ok
            break;
        case PHASE_QUAD_FEED_MOTOR_TEST: //0x03,0x00,0x06,0x70,0x0B
            reset = phase_quad_feed_motor_test(); //ok
            break;
        case PHASE_TWIN_FLAP_MOTOR_TEST: //0x03,0x00,0x06,0x70,0x35
            reset = phase_twin_flap_motor_test(); //ok
            break;
        case PHASE_TWIN_DRUM1_MOTOR_TEST: //0x03,0x00,0x06,0x70,0x09
            reset = phase_twin_drum1_motor_test(); //ok
            break;
        case PHASE_TWIN_DRUM2_MOTOR_TEST: //0x03,0x00,0x06,0x70,0x0A
            reset = phase_twin_drum2_motor_test(); //ok
            break;
        case PHASE_QUAD_FLAP_MOTOR_TEST: //0x03,0x00,0x06,0x70,0x0C
            reset = phase_quad_flap_motor_test(); //ok
            break;
        case PHASE_QUAD_DRUM1_MOTOR_TEST: //0x03,0x00,0x06,0x70,0x40
            reset = phase_quad_drum1_motor_test(); //ok
            break;
        case PHASE_QUAD_DRUM2_MOTOR_TEST: //0x03,0x00,0x06,0x70,0x41
            reset = phase_quad_drum2_motor_test(); //ok
            break;
        case PHASE_TWIN_TR_POS_SENSOR_TEST: //0x03,0x00,0x06,0x70,0x06
            reset = phase_twin_tr_pos_sens_test();	//ok
            break;
        case PHASE_TWIN_RC_POS_SENSOR_TEST: //0x03,0x00,0x06,0x70,0x07
            reset = phase_twin_rc_pos_sens_test(); //ok
            break;
        case PHASE_TWIN_SOLENOID_TEST: //0x03,0x00,0x06,0x70,0x08
            reset = phase_twin_solenoid_test(); //ok
            break;
        case PHASE_QUAD_TR_POS_SENSOR_TEST: //0x03,0x00,0x06,0x70,0x0D
            reset = phase_quad_tr_pos_sens_test(); //ok
            break;
        case PHASE_QUAD_RC_POS_SENSOR_TEST: //0x03,0x00,0x06,0x70,0x0E
            reset = phase_quad_rc_pos_sens_test(); //ok
            break;
        case PHASE_QUAD_SOLENOID_TEST: //0x03,0x00,0x06,0x70,0x0F
            reset = phase_quad_solenoid_test(); //ok
            break;
        case PHASE_TWIN_TAPE_SENS_TEST: //0x03,0x00,0x06,0x70,0x12
            reset = phase_twin_tape_sens_test(); //ok 使用されてないかも
            break;
        case PHASE_QUAD_TAPE_SENS_TEST: //0x03,0x00,0x06,0x70,0x13
            reset = phase_quad_tape_sens_test(); //ok 使用されてないかも
            break;
        case PHASE_SERIAL_NO: //0x03,0x00,0x06,0x70,0x50
            reset = phase_serial_no();
            break;
        case PHASE_SENS_ADJ_FRAM:
            reset = phase_sens_adj_fram();
            break;
        case PHASE_PERFORM_TEST_FRAM:
            reset = phase_perform_test_fram();
            break;
        case PHASE_TWIN_FEED_GET_DATA:
            reset = phase_twin_feed_get_data();
            break;
        case PHASE_QUAD_FEED_GET_DATA:
            reset = phase_quad_feed_get_data();
            break;
        case PHASE_SENS_ADJ_START:
            reset = phase_sens_adj_start();
            break;
        case PHASE_SENS_ADJ_GET_VAL:
            reset = phase_sens_adj_get_val();
            break;
        case PHASE_DRUM1_TAPE_POS_ADJ:
            reset = phase_drum1_tape_pos_adj();
            break;
        case PHASE_DRUM2_TAPE_POS_ADJ:
            reset = phase_drum2_tape_pos_adj();
            break;
        case PHASE_CHECK_FRAM:
            reset = phase_check_fram();
            break;
        case PHASE_CHECK_TWIN_OR_QUAD:
            reset = phase_check_twin_or_quad(); ////既存のTool suteやセンサ調整Toolで使用している
            break;
//#if defined(RC_BOARD_GREEN)		
        case PHASE_GET_RS_POS_ONOFF:
            reset = phase_get_rs_pos_onoff();
            break;
        case PHASE_SET_RS_POS_DA:
            reset = phase_rs_set_pos_da();
            break;
        case PHASE_SET_RS_POS_GAIN:
            reset = phase_rs_set_pos_gain();
            break;
 		//#if defined(UBA_RS)
        case PHASE_RS_FLAP_MOTOR_TEST:
            reset = phase_rs_flap_motor_test();
            break;
        case PHASE_RS_POS_SENSOR_TEST:
			reset = phase_rs_pos_sensor_test();
			break;
		//#endif	
//#endif // RC_BOARD_GREEN
        default:
            /*<<	clear command waiting flag	>>*/
            ex_front_usb.pc.status &= ~BIT_FUSB_COMMAND_WAITING;
            /*	*/
            set_response_1data(NAK);
        }
    }
    else
    {
        set_response_1data(NAK);
    }
    if (reset)
    {
        reset = 1;
        ex_dline_testmode.action = TEST_NON_ACTION;
        ex_dline_testmode.test_no = TEST_STANDBY;
        _dline_set_mode(DLINE_MODE_TEST_STANDBY);
    }
}


#endif // UBA_RTQ
// end
