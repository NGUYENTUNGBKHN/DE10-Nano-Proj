/****************************************************************************/
/*                                                                          */
/*  COPYRIGHT (C) Japan Cash Machine Co.,Ltd. 2017                          */
/*  ALL RIGHTS RESERVED                                                     */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/* This software contains proprietary, trade secret information and is      */
/* the property of Japan Cash Machine. This software and the information    */
/* contained therein may not be disclosed, used, transferred or             */
/* copied in whole or in part without the express, prior written            */
/* consent of Japan Cash Machine.                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/*                                                                          */
/* 本ソフトウェアに含まれるソースコードには日本金銭機械株式会社固有の       */
/* 企業機密情報含んでいます。                                               */
/* 秘密保持契約無しにソフトウェアとそこに含まれる情報の全体もしくは一部を   */
/* 公開も複製も行いません。                                                 */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/
/**
 * MODEL NAME : （モデル名）
 * @file jdl_conf.c
 * @brief  JCM Device Log Configuration Source
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/

#include "jdl.h"
#include "itron.h"
#include "kernel.h"
#include "kernel_inc.h"
#include "sensor.h"
#include "status_tbl.h"
#include "hal_spi_fram.h"

#include "cyc.h"

#define EXT
#include "com_ram.c"
#include "com_ram_ncache.c"
#include "cis_ram.c"

#if defined(UBA_RTQ)
#include "feed_rc_unit.h"
#include "rc_operation.h"//2024-07-08
#endif // UBA_RTQ


#if defined(_PROTOCOL_ENABLE_ID003)
#include "id003_sram.c"
#endif

/************************** BACKUP VARIABLES ***************************/
/*---------------------------------------------------------------------*/
/* Please set each backup variable to any section.                     */
/*---------------------------------------------------------------------*/
#if defined(_JDL_IAR_EWARM) /* For EBA-40, DBV-400, DBV-500-S/R and DBV-50 */

#pragma location = "JDL_SRAM"
__root u8 _bkex_jdl_buff[JDL_BUFF_TOTAL];


#elif defined(_JDL_TI_CCS) /* For UBA-Pro */

#pragma SET_DATA_SECTION("JDL_SRAM")

u8 _bkex_jdl_buff[JDL_BUFF_TOTAL];

#pragma SET_DATA_SECTION() // JDL_SRAM


#elif defined(_JDL_XILINX_SDK) /* For RBA-40 */
u8 _bkex_jdl_buff[JDL_BUFF_TOTAL];

#if defined(_JDL_PMIO)
#endif /* _JDL_PMIO */

#elif defined(_JDL_ARMDS_SDK) /* For iVIZION2/UBA Pro CIS */
u8 _bkex_jdl_buff[JDL_BUFF_TOTAL];

#if defined(_JDL_PMIO)
#endif /* _JDL_PMIO */

#else  /*  */
#endif /* _JDL_IAR_EWARM */


/************************** PRIVATE DEFINITIONS ************************/
#define JDL_TEMP_ADJ_VALI_GAP_OFFSET 96
#define JDL_TEMP_ADJ_POSI_DA_OFFSET  (JDL_TEMP_ADJ_VALI_GAP_OFFSET + 96)
#define JDL_TEMP_ADJ_SIDE_VAL_OFFSET (JDL_TEMP_ADJ_POSI_DA_OFFSET + 96)

/************************** PRIVATE FUNCTIONS **************************/
static u8 jdl_init_sys(u8 clear);
static u8 jdl_init_stat(u8 clear);
static u8 jdl_init_sens(u8 clear);
static void jdl_get_sensor_status(u8 *buff);
static u8 jdl_bill_reject(u16 dinfo_idx, u16 rej_code);
static u8 jdl_ticket_reject(u16 dinfo_idx, u16 rej_code);
static u8 jdl_save_error_log(u32 err_code, u32 seq);
static u8 jdl_save_error_counter(u32 err_code);
static u8 jdl_comm_get_sta(void);


/************************** PRIVATE VARIABLES **************************/
static u8 s_ir_jdl_feed_pulse;
static u8 s_ir_jdl_stacker_pulse;

static u8 s_jdl_ins_flag;

static u8 s_jdl_comm_sst_skip;
static u8 s_jdl_comm_sst_cmd;
static struct
{
    u8 rx_flag;
    u8 rx_cmd;
    u8 tx_flag;
    u8 tx_rsp;
} s_jdl_comm_prev_comm, s_jdl_comm_wait_sst_rsp;
static u8 s_jdl_err_data[JDL_ERR_OFS_BLK_EVEN_IDX];
static u8 s_jdl_err_update_flag;
#ifdef _DEBUG_JDL  /* yamazaki JDL DEBUG */
static u32 debug_jdl_wait_cnt;
#endif /* _DEBUG_JDL  yamazaki JDL DEBUG */
static u8 s_jdl_rtn;

/************************** EXTERN FUNCTIONS ***************************/


/************************** EXTERN VARIABLES ***************************/
extern const UB software_ver[];


/***********************************************************************/
/**
 * @brief initialize jcm device log
 * @param[in]  clear : force clear setting
 * @return     none
 */
/***********************************************************************/
void jdl_init(u8 clear)
{
    u8 res_jdl;
    u8 res_sys;
    u8 clr_flg;
    u8 pid[4];
    JDL_TIME stime;
    JDL_TIME etime;
    
    clr_flg = clear;
    
    res_jdl = _jdl_init();
    /* JDL全体のフォーマットリビジョン変更確認 */
    /* 変更があった場合は、Statitics以外をクリアする */
    if ((res_jdl == JDL_E_REVCHG) && (clr_flg == 0))
    {
        clr_flg = 1;
    }
    res_sys = jdl_init_sys(clr_flg);
    
    /* StatiticsはJDL全体のフォーマットリブジョンが変わってもクリアしない */
    /* 引数で1を指定されたときのみクリアする */
    s_jdl_rtn = jdl_init_stat(clear);
    
    s_jdl_rtn = jdl_init_sens(clr_flg);
    
    s_jdl_rtn = _jdl_event_init(clr_flg);
    
    s_jdl_rtn = _jdl_err_init(clr_flg);

    if ((clr_flg != 0) || (res_sys == JDL_E_VERCHG))
    {
        pid[0] = 0x00;
        pid[1] = 0x00;
        pid[2] = 0x00;
        pid[3] = 0x00;
        _jdl_comm_init(1, &pid[0]);
        s_jdl_rtn = _jdl_acc_init(1);
    }
    else
    {
        s_jdl_rtn = _jdl_acc_init(0);
    }
    
    s_jdl_rtn = _jdl_posiana_init(clr_flg);
    
#if defined(_JDL_USE_RC)
    s_jdl_rtn = _jdl_rc_init(clr_flg);
#endif /* defined(_JDL_USE_RC) */
    
#if defined(UBA_RC) || defined(UBA_RTQ)
    s_jdl_rtn = _jdl_oprc_init(clr_flg);
#endif
    
    _jdl_memset(&s_jdl_err_data[0], 0, JDL_ERR_OFS_BLK_EVEN_IDX);
    s_jdl_err_update_flag = 0;
    
    s_ir_jdl_feed_pulse = 0;
    s_ir_jdl_stacker_pulse = 0;
    s_jdl_ins_flag = 0;
    
    _jdl_set_mode(JDL_MODE_ENABLE);
    
    /* UBA-ProはRTCを搭載していないため、Systemの設定時間と経過時間に  */
    /* Sensorの最終調整日時とStatiticsの通電時間を初期値として設定する */
    /*  (全カテゴリー初期化完了後) */
    stime.high = _JDL_SWAP_32(*((u32 *)&ex_adjustment_data.maintenance_info.date[0]));
    stime.low = _JDL_SWAP_32(*((u32 *)&ex_adjustment_data.maintenance_info.date[JDL_DATA_TYPE_SIZE_DWORD]));

    s_jdl_rtn = _jdl_stat_get_mov(JDL_STAT_ADR_MOV_ENER_TIME, JDL_STAT_SIZE_MOV_CNTR, &etime.low, 0);
    etime.high = 0;
    _jdl_set_time(&stime, &etime);
}


/***********************************************************************/
/**
 * @brief Set PowerUp logs
 * @param[in]  time : setting time (8byte)
 * @return     none
 */
/***********************************************************************/
void jdl_set_jdl_time(u8 *time)
{
    JDL_TIME tmp_stime;
    JDL_TIME tmp_etime;
    
    //tmp_stime.high = *((u32 *)&time[0]);
    //tmp_stime.low = *((u32 *)&time[JDL_DATA_TYPE_SIZE_DWORD]);
    tmp_stime.high = _JDL_SWAP_32(*((u32 *)&time[0]));
    tmp_stime.low = _JDL_SWAP_32(*((u32 *)&time[JDL_DATA_TYPE_SIZE_DWORD]));
    tmp_etime.high = 0;
    tmp_etime.low = 0;
    
    _jdl_set_time(&tmp_stime, &tmp_etime);
}


/***********************************************************************/
/**
 * @brief Set PowerUp logs
 * @return     none
 */
/***********************************************************************/
void jdl_powerup(void)
{
    u8 event_data[JDL_EVEN_OFS_TIME];
    
    /* Increment the Power UP counter */
    s_jdl_rtn = _jdl_stat_inc_mov(JDL_STAT_ADR_MOV_POWERUP, JDL_STAT_SIZE_MOV_CNTR);
    
    /* Set Power Up Event */
    event_data[JDL_EVEN_OFS_CORD]     = JDL_EVEN_CODE_POWERUP;
    event_data[JDL_EVEN_OFS_DATA]     = ex_dipsw1;
    event_data[(JDL_EVEN_OFS_DATA+1)] = ex_operating_mode;
    event_data[(JDL_EVEN_OFS_DATA+2)] = 0x00;
    event_data[(JDL_EVEN_OFS_DATA+3)] = 0x00;
    event_data[(JDL_EVEN_OFS_DATA+4)] = 0x00;
    event_data[(JDL_EVEN_OFS_DATA+5)] = 0x00;
    event_data[JDL_EVEN_OFS_MODE]     = ex_main_task_mode1;
    event_data[(JDL_EVEN_OFS_MODE+1)] = ex_main_task_mode2;
    jdl_get_sensor_status(&event_data[JDL_EVEN_OFS_SENS]);
    s_jdl_rtn = _jdl_event_set(event_data);
}


/***********************************************************************/
/**
 * @brief Set device reset
 * @return     none
 */
/***********************************************************************/
void jdl_dev_reset(void)
{
    u8 event_data[JDL_EVEN_OFS_TIME];
    
    /* Increment the device reset counter */
    s_jdl_rtn = _jdl_stat_inc_mov(JDL_STAT_ADR_MOV_RESET, JDL_STAT_SIZE_MOV_CNTR);
    
    /* Set Power Up Event */
    event_data[JDL_EVEN_OFS_CORD]     = JDL_EVEN_CODE_RESET;
    event_data[JDL_EVEN_OFS_DATA]     = ex_dipsw1;
    event_data[(JDL_EVEN_OFS_DATA+1)] = ex_operating_mode;
    event_data[(JDL_EVEN_OFS_DATA+2)] = 0x00;
    event_data[(JDL_EVEN_OFS_DATA+3)] = 0x00;
    event_data[(JDL_EVEN_OFS_DATA+4)] = 0x00;
    event_data[(JDL_EVEN_OFS_DATA+5)] = 0x00;
    event_data[JDL_EVEN_OFS_MODE]     = ex_main_task_mode1;
    event_data[(JDL_EVEN_OFS_MODE+1)] = ex_main_task_mode2;
    jdl_get_sensor_status(&event_data[JDL_EVEN_OFS_SENS]);
    s_jdl_rtn = _jdl_event_set(event_data);
    
    /* Update error infomation due to init counter */
    if (s_jdl_err_update_flag != 0)
    {
        s_jdl_err_update_flag = 0;
        if (s_jdl_err_data[JDL_ERR_OFS_BLK_INIT] != 0xFF)
        {
            s_jdl_err_data[JDL_ERR_OFS_BLK_INIT]++;
        }
        _jdl_err_update(s_jdl_err_data);
    }
}


/***********************************************************************/
/**
 * @brief Count entry motor
 * @param[in]  run_time : Motor run time
 * @return     none
 */
/***********************************************************************/
void jdl_move_entry(u16 run_time)
{
#if defined(UBA_RC)
    s_jdl_rtn = _jdl_stat_motor(JDL_STAT_ADR_MOV_EMOT_CNT, JDL_STAT_SIZE_MOV_CNTR, JDL_STAT_SIZE_MOV_CNTR, (u32)run_time, 0);
#endif /* UBA_RC */
}


/***********************************************************************/
/**
 * @brief Count feed motor
 * @param[in]  run_time : Motor run time
 * @return     none
 */
/***********************************************************************/
void jdl_move_feed(u16 run_time)
{
#if 1   //NEW_JDL
    s_jdl_rtn = _jdl_stat_motor_new(JDL_STAT_ADR_MOV_FMOT_CNT, JDL_STAT_SIZE_MOV_CNTR, 0, (u32)run_time, 0);
#else    
    s_jdl_rtn = _jdl_stat_motor(JDL_STAT_ADR_MOV_FMOT_CNT, JDL_STAT_SIZE_MOV_CNTR, JDL_STAT_SIZE_MOV_CNTR, (u32)run_time, 0);
#endif
}

void jdl_move_shutter(u16 run_time)   //NEW_JDL
{
    s_jdl_rtn = _jdl_stat_motor_new(JDL_STAT_ADR_MOV_LMOT_CNT, JDL_STAT_SIZE_MOV_CNTR, 0, (u32)run_time, 0);
}

/***********************************************************************/
/**
 * @brief Count stack motor
 * @param[in]  run_time : Motor run time
 * @return     none
 */
/***********************************************************************/
void jdl_move_stack(u16 run_time)
{
#if 1   //NEW_JDL
    s_jdl_rtn = _jdl_stat_motor_new(JDL_STAT_ADR_MOV_SMOT_CNT, JDL_STAT_SIZE_MOV_CNTR, 0, (u32)run_time, 0);
#else
#if defined(UBA_RC)
    s_jdl_rtn = _jdl_stat_motor(JDL_STAT_ADR_MOV_SMOT_CNT, JDL_STAT_SIZE_MOV_CNTR, JDL_STAT_SIZE_MOV_CNTR, (u32)run_time, 0);
#endif /* UBA_RC */
#endif
}


/***********************************************************************/
/**
 * @brief Count centering motor
 * @param[in]  run_time : Motor run time
 * @return     none
 */
/***********************************************************************/
void jdl_move_centering(u16 run_time)
{
#if 1   //NEW_JDL
    s_jdl_rtn = _jdl_stat_motor_new(JDL_STAT_ADR_MOV_CMOT_CNT, JDL_STAT_SIZE_MOV_CNTR, 0, (u32)run_time, 0);
#else
    s_jdl_rtn = _jdl_stat_motor(JDL_STAT_ADR_MOV_CMOT_CNT, JDL_STAT_SIZE_MOV_CNTR, JDL_STAT_SIZE_MOV_CNTR, (u32)run_time, 0);
#endif
}


/***********************************************************************/
/**
 * @brief Count apb motor
 * @param[in]  run_time : Motor run time
 * @return     none
 */
/***********************************************************************/
void jdl_move_apb(u16 run_time)
{
#if 1   //NEW_JDL
    s_jdl_rtn = _jdl_stat_motor_new(JDL_STAT_ADR_MOV_AMOT_CNT, JDL_STAT_SIZE_MOV_CNTR, 0, (u32)run_time, 0);
#else
    s_jdl_rtn = _jdl_stat_motor(JDL_STAT_ADR_MOV_AMOT_CNT, JDL_STAT_SIZE_MOV_CNTR, JDL_STAT_SIZE_MOV_CNTR, (u32)run_time, 0);
#endif
}


/***********************************************************************/
/**
 * @brief Count eeprom1 written
 * @return     none
 */
/***********************************************************************/
void jdl_write_eep(void)
{
    s_jdl_rtn = _jdl_stat_inc_mov(JDL_STAT_ADR_MOV_FRAM, JDL_STAT_SIZE_MOV_CNTR);
}


/***********************************************************************/
/**
 * @brief Add Energization time
 * @param[in]  time : Energization time to add
 * @return     none
 */
/***********************************************************************/
void jdl_ener_time(void)
{
    u32 time;
    
    _jdl_draw_ener_time(&time);
    if (time != 0)
    {
        s_jdl_rtn = _jdl_stat_add_mov(JDL_STAT_ADR_MOV_ENER_TIME, JDL_STAT_SIZE_MOV_CNTR, time, 0);
    }
}


/***********************************************************************/
/**
 * @brief Count insertion
 * @return     none
 */
/***********************************************************************/
void jdl_insert(void) //紙幣取り込み開始
{
    s_jdl_rtn = _jdl_stat_insert();
    s_jdl_rtn = _jdl_acc_insert();
    
    s_jdl_ins_flag = 1;
}


/***********************************************************************/
/**
 * @brief Save accepted log
 * @param[in]  denomi : denomination index
 * @return     none
 */
/***********************************************************************/
void jdl_accept(u16 denomi) //押し込み成功で呼び出し
{
    u8 event_data[JDL_EVEN_OFS_TIME];
    
    event_data[JDL_EVEN_OFS_CORD]     = JDL_EVEN_CODE_ACC;
    event_data[JDL_EVEN_OFS_DATA]     = (u8)(denomi & 0xFF);
    event_data[(JDL_EVEN_OFS_DATA+1)] = (u8)((denomi >> 8) & 0xFF);
    event_data[(JDL_EVEN_OFS_DATA+2)] = 0x00;
    event_data[(JDL_EVEN_OFS_DATA+3)] = 0x00;
    event_data[(JDL_EVEN_OFS_DATA+4)] = 0x00;
    event_data[(JDL_EVEN_OFS_DATA+5)] = 0x00;
    event_data[JDL_EVEN_OFS_MODE]     = ex_main_task_mode1;
    event_data[(JDL_EVEN_OFS_MODE+1)] = ex_main_task_mode2;
    jdl_get_sensor_status(&event_data[JDL_EVEN_OFS_SENS]);
    s_jdl_rtn = _jdl_event_set(event_data);
    
    if (s_jdl_ins_flag)
    {
        if (denomi != BAR_INDX)
        {
        /* Bill accepted */
            s_jdl_rtn = _jdl_stat_bill_accept();
            s_jdl_rtn = _jdl_acc_bill_accept(denomi);
        }
        else
        {
        /*Bar ticket accepted */
            s_jdl_rtn = _jdl_stat_ticket_accept();
            s_jdl_rtn = _jdl_acc_ticket_accept(0);
        }
    }
    s_jdl_ins_flag = 0;
    
    s_jdl_rtn = _jdl_posiana_set_prev();
}


/***********************************************************************/
/**
 * @brief Save rejected log
 * @param[in]  rej_code : reject code
 * @param[in]  vali_sta : validation start flag
 * @param[in]  denomi   : denomination index
 * @param[in]  seq      : sequence no
 * @param[in]  m_mode1  : main task mode1
 * @param[in]  m_mode2  : main task mode1
 * @param[in]  sens     : sensor state
 * @return     none
 */
/***********************************************************************/
void jdl_reject(u16 rej_code, u8 vali_sta, u16 denomi, u32 seq, u32 m_mode1, u32 m_mode2, u32 sens)
{
    u16 dinfo_idx;
    u8 event_data[JDL_EVEN_OFS_TIME];
    
    if (denomi == BAR_INDX)
    {
    /* Bar ticet */
        dinfo_idx = JDL_ACC_DINFO_TICKET_IDX;
    }
    else if (vali_sta == 0)
    {
    /* Unkown bill */
        dinfo_idx = JDL_ACC_DINFO_UNKNOWN_IDX;
    }
    else
    {
        dinfo_idx = denomi;
    }
    
    event_data[JDL_EVEN_OFS_CORD]     = JDL_EVEN_CODE_REJ;
    event_data[JDL_EVEN_OFS_DATA]     = (rej_code & 0xFF);
    event_data[(JDL_EVEN_OFS_DATA+1)] = 0x00;
    event_data[(JDL_EVEN_OFS_DATA+2)] = vali_sta;
    event_data[(JDL_EVEN_OFS_DATA+3)] = (denomi & 0xFF);
    event_data[(JDL_EVEN_OFS_DATA+4)] = (m_mode1 & 0xFF);
    event_data[(JDL_EVEN_OFS_DATA+5)] = (m_mode2 & 0xFF);
    event_data[JDL_EVEN_OFS_MODE]     = ((seq >> 8) & 0xFF);
    event_data[(JDL_EVEN_OFS_MODE+1)] = (seq & 0xFF);
    jdl_get_sensor_status(&event_data[JDL_EVEN_OFS_SENS]);
    s_jdl_rtn = _jdl_event_set(event_data);
    
    if (s_jdl_ins_flag)
    {
        if (dinfo_idx < JDL_ACC_DINFO_TICKET_IDX)
        {
        /* Bill */
            s_jdl_rtn = jdl_bill_reject(dinfo_idx, rej_code);
        }
        else
        {
        /* Bar ticket */
            s_jdl_rtn = jdl_ticket_reject(dinfo_idx, rej_code);
        }
    }
    s_jdl_ins_flag = 0;
    
    s_jdl_rtn = _jdl_posiana_set_prev();
}


/***********************************************************************/
/**
 * @brief Save error log
 * @param[in]  err_code : error code
 * @param[in]  seq      : sequence no
 * @param[in]  m_mode1  : main task mode1
 * @param[in]  m_mode2  : main task mode1
 * @param[in]  sens     : sensor state
 * @return     none
 */
/***********************************************************************/
void jdl_error(u32 err_code, u32 seq, u32 m_mode1, u32 m_mode2, u32 sens)
{
    u8 event_data[JDL_EVEN_OFS_TIME];
    
    /* Save event log */
    event_data[JDL_EVEN_OFS_CORD]     = JDL_EVEN_CODE_ERR;
    event_data[JDL_EVEN_OFS_DATA]     = (err_code & 0xFF);
    event_data[(JDL_EVEN_OFS_DATA+1)] = 0x00;
    event_data[(JDL_EVEN_OFS_DATA+2)] = ex_main_task_mode1;
    event_data[(JDL_EVEN_OFS_DATA+3)] = ex_main_task_mode2;
    event_data[(JDL_EVEN_OFS_DATA+4)] = (m_mode1 & 0xFF);
    event_data[(JDL_EVEN_OFS_DATA+5)] = (m_mode2 & 0xFF);
    event_data[JDL_EVEN_OFS_MODE]     = ((seq >> 8) & 0xFF);
    event_data[(JDL_EVEN_OFS_MODE+1)] = (seq & 0xFF);
    jdl_get_sensor_status(&event_data[JDL_EVEN_OFS_SENS]);
    s_jdl_rtn = _jdl_event_set(event_data);
    
    /* Save error log */
    s_jdl_rtn = jdl_save_error_log(err_code, seq);
    
    /* Save stat log */
    s_jdl_rtn = jdl_save_error_counter(err_code);
    
    s_jdl_ins_flag = 0;
}


/***********************************************************************/
/**
 * @brief Count ticket acceptance
 * @param[in]  tid    : task id
 * @param[in]  mode1  : task mode/sequence 1
 * @param[in]  mode2  : task mode/sequence 2
 * @param[in]  tdata1 : trace data 1
 * @param[in]  tdata2 : trace data 2
 * @param[in]  tdata3 : trace data 3
 * @return     none
 */
/***********************************************************************/
void jdl_add_trace(u8 tid, u8 mode1, u8 mode2, u8 tdata1, u8 tdata2, u8 tdata3)
{
    u8 trace[JDL_ERR_TRCD_SIZE];
    
    trace[JDL_ERR_OFS_TRCD_ID]       = tid;
    trace[JDL_ERR_OFS_TRCD_MODE]     = mode1;
    trace[(JDL_ERR_OFS_TRCD_MODE+1)] = mode2;
    trace[JDL_ERR_OFS_TRCD_DATA]     = tdata1;
    trace[(JDL_ERR_OFS_TRCD_DATA+1)] = tdata2;
    trace[(JDL_ERR_OFS_TRCD_DATA+2)] = tdata3;
    jdl_get_sensor_status(&trace[JDL_ERR_OFS_TRCD_SENS]);
    
    s_jdl_rtn = _jdl_err_add_trace(trace);
}


/***********************************************************************/
/**
 * @brief Initialize communication category
 * @param[in]  pid      : protocol id
 * @param[in]  sst_skip : skip status
 * @param[in]  sst_cmd  : status command
 * @return     none
 */
/***********************************************************************/
void jdl_comm_init(u8 pid, u8 sst_skip, u8 sst_cmd)
{
    u8 conv_id[4];
    
    switch (pid)
    {
    case PROTOCOL_SELECT_ID003:
        conv_id[0] = 0x00;
        conv_id[1] = 0x00;
        conv_id[2] = 0x03;
        conv_id[3] = 0x00;
        break;
    case PROTOCOL_SELECT_ID008:
        conv_id[0] = 0x00;
        conv_id[1] = 0x00;
        conv_id[2] = 0x08;
        conv_id[3] = 0x00;
        break;
    case PROTOCOL_SELECT_ID0E3:
        conv_id[0] = 0x00;
        conv_id[1] = 0x0E;
        conv_id[2] = 0x03;
        conv_id[3] = 0x00;
        break;
    case PROTOCOL_SELECT_ID064GD:
        conv_id[0] = 0x00;
        conv_id[1] = 0x06;
        conv_id[2] = 0x04;
        conv_id[3] = 0x00;
        break;
    //#if defined(_PROTOCOL_ENABLE_ID0G8)
    case PROTOCOL_SELECT_ID0G8:
        conv_id[0] = 0x00;
        conv_id[1] = 0x47;	/* 1byteなのでGは使用できないので0x47にする(アスキーの"G")*/
        conv_id[2] = 0x08;
        conv_id[3] = 0x00;
        break;
    //#endif
    default:
        s_jdl_rtn = JDL_E_PARAM;
        break;
    }
    
    if (s_jdl_rtn == JDL_E_OK)
    {
        s_jdl_comm_sst_skip = sst_skip;
        s_jdl_comm_sst_cmd = sst_cmd;
        
        s_jdl_rtn = _jdl_comm_init(0, &conv_id[0]);
    }
}


/***********************************************************************/
/**
 * @brief Save rx packet
 * @param[in]  data : rx packet data
 * @param[in]  size : data size
 * @return     none
 */
/***********************************************************************/
void jdl_comm_rx_pkt(u8 *data, u8 size)
{
    if (s_jdl_comm_sst_skip == 0)
    {
    /*  */
        /* add previous rx command */
        s_jdl_rtn = _jdl_comm_add_data(data, size, 0);
    }
    else
    {
        if (s_jdl_comm_wait_sst_rsp.rx_flag == 1)
        {
        /* previous comm was no response */
            /* add previous rx command */
            s_jdl_rtn = _jdl_comm_add_data(&s_jdl_comm_wait_sst_rsp.rx_cmd, 1, 0);
            /* add current rx command */
            s_jdl_rtn = _jdl_comm_add_data(data, size, 0);
            s_jdl_comm_prev_comm.rx_flag = 1;
            s_jdl_comm_prev_comm.rx_cmd = *data;
            s_jdl_comm_prev_comm.tx_flag = 0;
            s_jdl_comm_prev_comm.tx_rsp = 0;
            /* clear */
            s_jdl_comm_wait_sst_rsp.rx_flag = 0;
            s_jdl_comm_wait_sst_rsp.rx_cmd = 0;
        }
        else if ((*data == s_jdl_comm_sst_cmd)
         && (s_jdl_comm_prev_comm.rx_cmd == s_jdl_comm_sst_cmd))
        {
        /* need to check status */
            s_jdl_comm_wait_sst_rsp.rx_flag = 1;
            s_jdl_comm_wait_sst_rsp.rx_cmd = *data;
        }
        else
        {
        /*  */
            /* add current rx command */
            s_jdl_rtn = _jdl_comm_add_data(data, size, 0);
            s_jdl_comm_prev_comm.rx_flag = 1;
            s_jdl_comm_prev_comm.rx_cmd = *data;
            s_jdl_comm_prev_comm.tx_flag = 0;
            s_jdl_comm_prev_comm.tx_rsp = 0;
        }
    }
}


/***********************************************************************/
/**
 * @brief Save tx packet
 * @param[in]  data : tx packet data
 * @param[in]  size : data size
 * @return     none
 */
/***********************************************************************/
void jdl_comm_tx_pkt(u8 *data, u8 size)
{
    if (s_jdl_comm_sst_skip == 0)
    {
    /*  */
        /* add previous rx command */
        s_jdl_rtn = _jdl_comm_add_data(data, size, 1);
    }
    else
    {
        if (s_jdl_comm_wait_sst_rsp.rx_flag)
        {
            if (*data != s_jdl_comm_prev_comm.tx_rsp)
            {
            /* current status is different as previous status */
                /* add current rx command */
                s_jdl_rtn = _jdl_comm_add_data(&s_jdl_comm_wait_sst_rsp.rx_cmd, 1, 0);
                /* add current tx command */
                s_jdl_rtn = _jdl_comm_add_data(data, size, 1);
                s_jdl_comm_prev_comm.rx_flag = 1;
                s_jdl_comm_prev_comm.rx_cmd = s_jdl_comm_wait_sst_rsp.rx_cmd;
                s_jdl_comm_prev_comm.tx_flag = 1;
                s_jdl_comm_prev_comm.tx_rsp = *data;
            }
            s_jdl_comm_wait_sst_rsp.rx_flag = 0;
            s_jdl_comm_wait_sst_rsp.rx_cmd = 0;
        }
        else
        {
            /* add current tx command */
            s_jdl_rtn = _jdl_comm_add_data(data, size, 1);
            s_jdl_comm_prev_comm.tx_flag = 1;
            s_jdl_comm_prev_comm.tx_rsp = *data;
        }
    }
}


/***********************************************************************/
/**
 * @brief Update sensor correction value
 * @return     none
 */
/***********************************************************************/
void jdl_sens_update_cor_val(void)
{

    u8 rtn = JDL_E_OK;
    u16 cnt;
    u8 *bptr;
    u32 offset;

    s_jdl_rtn = _jdl_sens_update_cor();
    /* Position Sensor D/A*/
    offset = JDL_SENS_ADR_COR_VAL;
    bptr = &ex_position_da.entrance;
    for (cnt = 0; cnt < 5; cnt++)
    {
        s_jdl_rtn = _jdl_sens_set_val(offset, JDL_DATA_TYPE_SIZE_BYTE, (u32)(bptr[cnt]));
        offset += 1;
    }
    //2024-03-07 Posiotin sensor Gain
	rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_position_ga, sizeof(ex_position_ga));
        offset += 1;

    #if 0 //2024-03-07 有効にする場合は注意が必要 CIS関係はまともに動いてないので、ivizion2同様 コメントアウト
    //下記の処理を走らせても、CIS用のメモリを確保していないので、書き込み処理は走らない
    //逆に書き込み処理が動作したら、JDLのComunication領域に上書きされる
    /* CIS PGA */
	rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_cis_adjustment_tmp.cis_pga, sizeof(CIS_ADJUSTMENT_PGA));
	offset += sizeof(CIS_ADJUSTMENT_PGA);
    /* CIS AREA */
    rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_cis_adjustment_tmp.cis_tmp_eria, sizeof(CIS_ADJUSTMENT_ERIA));
    offset += sizeof(CIS_ADJUSTMENT_ERIA);
    /* CIS A/D no paper */
    rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_cis_adjustment_tmp.cis_tmp_ad, sizeof(CIS_ADJUSTMENT_AD));
    offset += sizeof(CIS_ADJUSTMENT_AD);
    /* CIS D/A no paper */
    rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_cis_adjustment_tmp.cis_tmp_da, sizeof(CIS_ADJUSTMENT_DA));
    offset += sizeof(CIS_ADJUSTMENT_DA);
    /* CIS TIME no paper */
    rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_cis_adjustment_tmp.cis_tmp_time, sizeof(CIS_ADJUSTMENT_TIME));
    offset += sizeof(CIS_ADJUSTMENT_TIME);
#endif

}


/***********************************************************************/
/**
 * @brief Update position analysis
 * @return     none
 */
/***********************************************************************/
void jdl_posiana_update(void)
{
    u8 sens_stat[JDL_SIZE_SENS_STAT];
    
    jdl_get_sensor_status(&sens_stat[0]);
    
    s_jdl_rtn = _jdl_posiana_update_state(&sens_stat[0]);
}


/***********************************************************************/
/**
 * @brief Set rc version (boot, firmware and firmware check sum)
 * @return     none
 */
/***********************************************************************/
void jdl_rc_set_version(void)
{
#if defined(UBA_RC) || defined(UBA_RTQ)
    /* Boot version */
    s_jdl_rtn = _jdl_rc_set_uinfo(0, JDL_RC_OFS_UINFO_BOOT_VER, &RecycleSoftInfo.BootRomid[0], JDL_SIZE_BOOT_VER);
    s_jdl_rtn = _jdl_rc_set_uinfo(1, JDL_RC_OFS_UINFO_BOOT_VER, &RecycleSoftInfo.BootRomid[0], JDL_SIZE_BOOT_VER);
    /* Firmware version */
    s_jdl_rtn = _jdl_rc_set_uinfo(0, JDL_RC_OFS_UINFO_FIRM_VER, &RecycleSoftInfo.FlashRomid[0], 28);
    s_jdl_rtn = _jdl_rc_set_uinfo(1, JDL_RC_OFS_UINFO_FIRM_VER, &RecycleSoftInfo.FlashRomid[0], 28);
    /* Firmware check sum */
    s_jdl_rtn = _jdl_rc_set_uinfo_word(0, JDL_RC_OFS_UINFO_FIRM_CRC, *((u16 *)&RecycleSoftInfo.FlashCheckSum[0]));
    s_jdl_rtn = _jdl_rc_set_uinfo_word(1, JDL_RC_OFS_UINFO_FIRM_CRC, *((u16 *)&RecycleSoftInfo.FlashCheckSum[0]));
#endif /* UBA_RC */
}


/***********************************************************************/
/**
 * @brief Set feed speed
 * @return     none
 */
/***********************************************************************/
void jdl_rc_set_speed(void)
{
#if defined(UBA_RC) || defined(UBA_RTQ)
    /* Fwd speed */
    s_jdl_rtn = _jdl_rc_set_uinfo_word(0, JDL_RC_OFS_UINFO_IN_SPEED, uba_feed_speed_fwd);
    s_jdl_rtn = _jdl_rc_set_uinfo_word(1, JDL_RC_OFS_UINFO_IN_SPEED, uba_feed_speed_fwd);
    /* Rev speed */
    s_jdl_rtn = _jdl_rc_set_uinfo_word(0, JDL_RC_OFS_UINFO_OUT_SPEED, uba_feed_speed_rev);
    s_jdl_rtn = _jdl_rc_set_uinfo_word(1, JDL_RC_OFS_UINFO_OUT_SPEED, uba_feed_speed_rev);
#endif /* UBA_RC */
}


/***********************************************************************/
/**
 * @brief Set recycle setting
 * @return     none
 */
/***********************************************************************/
void jdl_rc_set_rc_setting(void)
{
#if defined(UBA_RC) || defined(UBA_RTQ)
    u8 cnt;
    u8 unit_no;
    u8 drum_no;
    u8 temp[7];
    
    for (cnt = 0; cnt < 4; cnt++)
    {
        temp[0] = RecycleSettingInfo.DenomiInfo[cnt].Data1;
        temp[1] = RecycleSettingInfo.DenomiInfo[cnt].Data2;
        temp[2] = RecycleSettingInfo.DenomiInfo[cnt].BillInfo.Length;
        temp[3] = RecycleSettingInfo.DenomiInfo[cnt].BillInfo.Max;
        temp[4] = RecycleSettingInfo.DenomiInfo[cnt].BillInfo.Min;
        temp[5] = 0x00;
        temp[6] = RecycleSettingInfo.DenomiInfo[cnt].RecycleLimit;
        
        unit_no = (cnt / 2);
        drum_no = (cnt % 2);
        
        s_jdl_rtn = _jdl_rc_set_dinfo(unit_no, drum_no, JDL_RC_OFS_DINFO_CURRENCY, &temp[0], 7);
    }
#endif /* UBA_RC */
}


/***********************************************************************/
/**
 * @brief RC stack
 * @param[in]  unit_no  : unit number
 * @param[in]  drum_no  : drum number
 * @param[in]  denomi   : denomination index
 * @param[in]  bill_len : bill length
 * @return     none
 */
/***********************************************************************/
void jdl_rc_stack(u16 unit_no, u16 drum_no, u16 denomi, u16 bill_len) //リサイクラ収納
{
#if defined(UBA_RC) || defined(UBA_RTQ)
    u8 event_data[JDL_EVEN_OFS_TIME];
    u16 temp_unit;
    u16 temp_drum;
    
    /* For UBA-Pro */
    if ((unit_no < 1) || (4 < unit_no))
    {
    /* unit number error */
        s_jdl_rtn = JDL_E_PARAM;
    }
    else
    {
        s_jdl_rtn = _jdl_stat_bill_accept();
        s_jdl_rtn = _jdl_acc_bill_accept(denomi);
        
        s_jdl_rtn = _jdl_stat_rc(JDL_STAT_ADR_RC_STACK, JDL_STAT_SIZE_RC_CNTR);
        
        temp_unit = ((unit_no - 1) / 2);
        temp_drum = ((unit_no - 1) % 2);
        
        /* Save event log */
        event_data[JDL_EVEN_OFS_CORD]     = JDL_EVEN_CODE_RC_ACC;
        event_data[JDL_EVEN_OFS_DATA]     = (u8)(denomi & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+1)] = (u8)((denomi >> 8) & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+2)] = (u8)(temp_unit & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+3)] = (u8)(temp_drum & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+4)] = (u8)(bill_len & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+5)] = (u8)((bill_len >> 8) & 0xFF);
        event_data[JDL_EVEN_OFS_MODE]     = ex_main_task_mode1;
        event_data[(JDL_EVEN_OFS_MODE+1)] = ex_main_task_mode2;
        jdl_get_sensor_status(&event_data[JDL_EVEN_OFS_SENS]);
        s_jdl_rtn = _jdl_event_set(event_data);
        
        /* Add tracking data */
        s_jdl_rtn = _jdl_rc_add_track(temp_unit, temp_drum, denomi);
    }
    
    s_jdl_rtn = _jdl_posiana_set_prev();
#endif /* UBA_RC */
}


/***********************************************************************/
/**
 * @brief RC stack
 * @param[in]  unit_no  : unit number
 * @param[in]  drum_no  : drum number
 * @param[in]  pre_feed : pre feed settings
 * @return     none
 */
/***********************************************************************/
void jdl_rc_payout(u16 unit_no, u16 drum_no, u8 pre_feed)
{
#if defined(UBA_RC) || defined(UBA_RTQ)
    u8 event_data[JDL_EVEN_OFS_TIME];
    u16 temp_unit;
    u16 temp_drum;
    
    /* For UBA-Pro */
    if ((unit_no < 1) || (4 < unit_no))
    {
    /* unit number error */
        s_jdl_rtn = JDL_E_PARAM;
    }
    else
    {
        s_jdl_rtn = _jdl_stat_rc(JDL_STAT_ADR_RC_PAYOUT, JDL_STAT_SIZE_RC_CNTR);
        
        temp_unit = ((unit_no - 1) / 2);
        temp_drum = ((unit_no - 1) % 2);
        
        /* Save event log */
        event_data[JDL_EVEN_OFS_CORD]     = JDL_EVEN_CODE_RC_PAY;
        event_data[JDL_EVEN_OFS_DATA]     = 0x00;//(u8)(denomi & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+1)] = 0x00;//(u8)((denomi >> 8) & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+2)] = (u8)(temp_unit & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+3)] = (u8)(temp_drum & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+4)] = pre_feed;
        event_data[(JDL_EVEN_OFS_DATA+5)] = 0x00;
        event_data[JDL_EVEN_OFS_MODE]     = ex_main_task_mode1;
        event_data[(JDL_EVEN_OFS_MODE+1)] = ex_main_task_mode2;
        jdl_get_sensor_status(&event_data[JDL_EVEN_OFS_SENS]);
        s_jdl_rtn = _jdl_event_set(event_data);
        
        /* Add tracking data */
        s_jdl_rtn = _jdl_rc_remove_track(temp_unit, temp_drum);
    }
    
    s_jdl_rtn = _jdl_posiana_set_prev();
#endif /* UBA_RC */
}


/***********************************************************************/
/**
 * @brief RC stack
 * @param[in]  unit_no  : unit number
 * @param[in]  drum_no  : drum number
 * @param[in]  pre_feed : pre feed settings
 * @return     error cord
 */
/***********************************************************************/
void jdl_rc_collect(u16 unit_no, u16 drum_no, u8 pre_feed)
{
#if defined(UBA_RC) || defined(UBA_RTQ)
    u8 event_data[JDL_EVEN_OFS_TIME];
    u16 temp_unit;
    u16 temp_drum;
    
    /* For UBA-Pro */
    if ((unit_no < 1) || (4 < unit_no))
    {
    /* unit number error */
        s_jdl_rtn = JDL_E_PARAM;
    }
    else
    {
        s_jdl_rtn = _jdl_stat_rc(JDL_STAT_ADR_RC_COLLECT, JDL_STAT_SIZE_RC_CNTR);
        
        temp_unit = ((unit_no - 1) / 2);
        temp_drum = ((unit_no - 1) % 2);
        
        /* Save event log */
        event_data[JDL_EVEN_OFS_CORD]     = JDL_EVEN_CODE_RC_COL;
        event_data[JDL_EVEN_OFS_DATA]     = 0x00;//(u8)(denomi & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+1)] = 0x00;//(u8)((denomi >> 8) & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+2)] = (u8)(temp_unit & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+3)] = (u8)(temp_drum & 0xFF);
        event_data[(JDL_EVEN_OFS_DATA+4)] = pre_feed;
        event_data[(JDL_EVEN_OFS_DATA+5)] = 0x00;
        event_data[JDL_EVEN_OFS_MODE]     = ex_main_task_mode1;
        event_data[(JDL_EVEN_OFS_MODE+1)] = ex_main_task_mode2;
        jdl_get_sensor_status(&event_data[JDL_EVEN_OFS_SENS]);
        s_jdl_rtn = _jdl_event_set(event_data);
        
        /* Add tracking data */
        s_jdl_rtn = _jdl_rc_remove_track(temp_unit, temp_drum);
    }
    
    s_jdl_rtn = _jdl_posiana_set_prev();
#endif /* UBA_RC */
}

#if defined(UBA_RTQ)
/***********************************************************************/
/**
 * @brief RC stack
 * @param[in]  unit_no  : unit number
 * @param[in]  drum_no  : drum number
 * @param[in]  pre_feed : pre feed settings
 * @return     error cord
 */
/***********************************************************************/
void jdl_rc_each_count(u16 unit_no, u8 operation)
{
    u8 unit, drum;

    switch (unit_no)
    {
    case 1: /* drum1 */
        unit = 0;
        drum = 0;
        break;
    case 2: /* drum2 */
        unit = 0;
        drum = 1;
        break;
    case 3: /* drum3 */
        unit = 1;
        drum = 0;
        break;
    case 4: /* drum4 */
        unit = 1;
        drum = 1;
        break;
    default:
        operation = 0;
        break;
    }

    switch (operation)
    {
    case STACK:
        s_jdl_rtn = _jdl_rc_inc_tcnt(unit, drum, JDL_RC_OFS_DINFO_STACK, JDL_DATA_TYPE_SIZE_DWORD);
        break;
    case COLLECT:
        s_jdl_rtn = _jdl_rc_inc_tcnt(unit, drum, JDL_RC_OFS_DINFO_COLLECT, JDL_DATA_TYPE_SIZE_DWORD);
        break;
    case PAYOUT:
        s_jdl_rtn = _jdl_rc_inc_tcnt(unit, drum, JDL_RC_OFS_DINFO_PAYOUT, JDL_DATA_TYPE_SIZE_DWORD);
        break;
    case PAYREJECT:
        s_jdl_rtn = _jdl_rc_inc_tcnt(unit, drum, JDL_RC_OFS_DINFO_P_REJECT, JDL_DATA_TYPE_SIZE_WORD);
        break;
    default:
        s_jdl_rtn = JDL_E_PARAM;
        break;
    }
}
#endif

/***********************************************************************/
/**
 * @brief Start getting JDL
 * @param[out]  total : JDL total size
 * @return     error cord
 */
/***********************************************************************/
u8 jdl_get_sta(u32 *total)
{
    u8 rtn = JDL_E_OK;
    u16 cnt;
    u16 *wptr;
    u32 offset;
    u8 sens_stat[JDL_SIZE_SENS_STAT];
#if defined(UBA_RC) || defined(UBA_RTQ)
    u8 temp[6];
#endif /* UBA_RC */
    
    /* temporary remove to save log in under */
    _jdl_set_mode(JDL_MODE_DISABLE);
    
    *total = JDL_SIZE_FILE_HEADER;                                                                               /* 0 : File Header */
    *total += (JDL_SIZE_CATEGORY_HEADER + JDL_SYS_SEND_TOTAL);                                                   /* 1 : System */
    *total += (JDL_SIZE_CATEGORY_HEADER + JDL_STAT_SEND_TOTAL);                                                  /* 2 : Statistics */
    *total += (JDL_SIZE_CATEGORY_HEADER + JDL_SENS_SEND_TOTAL);                                                  /* 3 : Sensor */
    *total += (JDL_SIZE_CATEGORY_HEADER + JDL_COMM_SEND_TOTAL);                                                  /* 4 : Communication */
    *total += (JDL_SIZE_CATEGORY_HEADER + JDL_EVEN_SEND_TOTAL);                                                  /* 5 : Event */
    *total += (JDL_SIZE_CATEGORY_HEADER + JDL_ERR_SEND_TOTAL + JDL_ERR_TRCD_TOTAL);                              /* 6 : Error */
    *total += (JDL_SIZE_CATEGORY_HEADER + JDL_ACC_SIZE_DENOMI_NUM + JDL_ACC_CURRENCY_SIZE + JDL_ACC_SEND_TOTAL); /* 7 : Acceptance */
    *total += (JDL_SIZE_CATEGORY_HEADER + JDL_PANA_SEND_TOTAL + (JDL_PANA_RCD_TOTAL * 2));                       /* 8 : Position Analysis */
#if defined(_JDL_USE_RC)
    *total += (JDL_SIZE_CATEGORY_HEADER + JDL_RC_SEND_TOTAL);                                                    /* 9 : RC */
#endif /* _JDL_USE_RC */
#if defined(_JDL_USE_OP_RC)
	*total += (JDL_SIZE_CATEGORY_HEADER + JDL_OPRC_SEND_TOTAL);                                                  /* 10: OPRC */
#endif /* UBA_RC */
    
    _jdl_sys_set_tim();
    
    /*----------------------------------------------------------*/
    /* Set Current A/D                                          */
    /*----------------------------------------------------------*/
    cnt++;
    offset = 0;
    wptr = 0;
    offset = 0;
    /* Current Sensor State */
    jdl_get_sensor_status(&sens_stat[0]);
    _jdl_sens_set_cur_blk(JDL_SENS_ADR_SENS_STAT, &sens_stat[0], JDL_SIZE_SENS_STAT);

#if 0//#if defined(UBA_RTQ)
    /* Set Unit1 infomation */
    temp[0] = ex_rc_status.sst1A.byte;                  /* Unit status */
    temp[1] = ex_rc_status.sst1B.byte;                  /* Unit status */
    temp[2] = ex_rc_status.sst21A.byte;                 /* Sensor status */
    temp[3] = ex_rc_status.sst21B.byte;                 /* Sensor status */
    temp[4] = (u8)(ex_rc_error_code & 0xFF);            /* Error detail */
    temp[5] = (u8)((ex_rc_error_code >> 8) & 0xFF);     /* Error detail */
    _jdl_rc_set_uinfo(0, JDL_RC_OFS_UINFO_UNIT_STAT, &temp[0], 6);

    /* Set Unit2 infomation */
    temp[2] = ex_rc_status.sst22A.byte;                 /* Sensor status */
    temp[3] = ex_rc_status.sst22B.byte;                 /* Sensor status */
    _jdl_rc_set_uinfo(1, JDL_RC_OFS_UINFO_UNIT_STAT, &temp[0], 6);

    /* Set Unit1 drum1 and drum2 */
    temp[0] = ex_rc_status.sst31A.byte;                 /* DRUM Sensor status */
    temp[1] = ex_rc_status.sst31B.byte;                 /* DRUM Sensor status */
    _jdl_rc_set_dinfo(0, 0, JDL_RC_OFS_DINFO_SENS_STAT, &temp[0], 2);
    _jdl_rc_set_dinfo(0, 1, JDL_RC_OFS_DINFO_SENS_STAT, &temp[0], 2);

    /* Set Unit2 drum1 and drum2 */
    temp[0] = ex_rc_status.sst32A.byte;                 /* DRUM Sensor status */
    temp[1] = ex_rc_status.sst32B.byte;                 /* DRUM Sensor status */
    _jdl_rc_set_dinfo(1, 0, JDL_RC_OFS_DINFO_SENS_STAT, &temp[0], 2);
    _jdl_rc_set_dinfo(1, 1, JDL_RC_OFS_DINFO_SENS_STAT, &temp[0], 2);
#endif // UBA_RTQ
    
    jdl_comm_get_sta();
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Get JDL
 * @param[out]  buff      : Save destination buffre
 * @param[in]   buff_size : buffre size
 * @param[in]   offset    : data offset to get
 * @param[out]  g_size    : data size
 * @return     error cord
 */
/***********************************************************************/
u8 jdl_req_data(u32 offset, u32 buff_size)
{
    u8 rtn = JDL_E_OK;
    u32 each_ofs[11];
    u32 each_offset;
    
    each_ofs[0]  = JDL_SIZE_FILE_HEADER;                                                                                            /* 0 : File Header */
    each_ofs[1]  = (each_ofs[0] + JDL_SIZE_CATEGORY_HEADER + JDL_SYS_SEND_TOTAL);                                                   /* 1 : System */
    each_ofs[2]  = (each_ofs[1] + JDL_SIZE_CATEGORY_HEADER + JDL_STAT_SEND_TOTAL);                                                  /* 2 : Statistics */
    each_ofs[3]  = (each_ofs[2] + JDL_SIZE_CATEGORY_HEADER + JDL_SENS_SEND_TOTAL);                                                  /* 3 : Sensor */
    each_ofs[4]  = (each_ofs[3] + JDL_SIZE_CATEGORY_HEADER + JDL_COMM_SEND_TOTAL);                                                  /* 4 : Communication */
    each_ofs[5]  = (each_ofs[4] + JDL_SIZE_CATEGORY_HEADER + JDL_EVEN_SEND_TOTAL);                                                  /* 5 : Event */
    each_ofs[6]  = (each_ofs[5] + JDL_SIZE_CATEGORY_HEADER + JDL_ERR_SEND_TOTAL + JDL_ERR_TRCD_TOTAL);                              /* 6 : Error */
    each_ofs[7]  = (each_ofs[6] + JDL_SIZE_CATEGORY_HEADER + JDL_ACC_SIZE_DENOMI_NUM + JDL_ACC_CURRENCY_SIZE + JDL_ACC_SEND_TOTAL); /* 7 : Acceptance */
    each_ofs[8]  = (each_ofs[7] + JDL_SIZE_CATEGORY_HEADER + JDL_PANA_SEND_TOTAL + (JDL_PANA_RCD_TOTAL * 2));                       /* 8 : Position Analysis */
#if defined(_JDL_USE_RC)
    each_ofs[9]  = (each_ofs[8] + JDL_SIZE_CATEGORY_HEADER + JDL_RC_SEND_TOTAL);                                                    /* 9 : RC */
#endif /* _JDL_USE_RC */
#if defined(_JDL_USE_OP_RC)
	each_ofs[10] = (each_ofs[9] + JDL_SIZE_CATEGORY_HEADER + JDL_OPRC_SEND_TOTAL);                                                  /* 10: OPRC */
#endif /* UBA_RC */
	
    
    if (offset < each_ofs[0])
    {
    /* 0 : File Header */
		rtn = JDL_E_OK;
    }
    else if (offset < each_ofs[1])
    {
    /* 1 : System */
        each_offset = offset - each_ofs[0];
        rtn = _jdl_sys_req(each_offset, buff_size);
    }
    else if (offset < each_ofs[2])
    {
    /* 2 : Statistics */
        each_offset = offset - each_ofs[1];
        rtn = _jdl_stat_req(each_offset, buff_size);
    }
    else if (offset < each_ofs[3])
    {
    /* 3 : Sensor */
        each_offset = offset - each_ofs[2];
        rtn = _jdl_sens_req(each_offset, buff_size);
    }
    else if (offset < each_ofs[4])
    {
    /* 4 : Communication */
        each_offset = offset - each_ofs[3];
        rtn = _jdl_comm_req(each_offset, buff_size);
    }
    else if (offset < each_ofs[5])
    {
    /* 5 : Event */
        each_offset = offset - each_ofs[4];
        rtn = _jdl_event_req(each_offset, buff_size);
    }
    else if (offset < each_ofs[6])
    {
    /* 6 : Error */
        each_offset = offset - each_ofs[5];
        rtn = _jdl_err_req(each_offset, buff_size);
    }
    else if (offset < each_ofs[7])
    {
    /* 7 : Acceptance */
        each_offset = offset - each_ofs[6];
        rtn = _jdl_acc_req(each_offset, buff_size);
    }
    else if (offset < each_ofs[8])
    {
    /* 8 : Position Analysis */
        each_offset = offset - each_ofs[7];
        rtn = _jdl_posiana_req(each_offset, buff_size);
    }
#if defined(_JDL_USE_RC)
    else if (offset < each_ofs[9])
    {
    /* 9 : RC */
        each_offset = offset - each_ofs[8];
        rtn = _jdl_rc_req(each_offset, buff_size);
    }
#endif /* _JDL_USE_RC */
#if defined(_JDL_USE_OP_RC)
    else if (offset < each_ofs[10])
    {
    /* 10: OPRC */
        each_offset = offset - each_ofs[9];
        rtn = _jdl_oprc_req(each_offset, buff_size);
    }
#endif /* _JDL_USE_OP_RC */
    else
    {
        rtn = JDL_E_MACV;
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Get JDL
 * @param[out]  buff      : Save destination buffre
 * @param[in]   buff_size : buffre size
 * @param[in]   offset    : data offset to get
 * @param[out]  g_size    : data size
 * @return     error cord
 */
/***********************************************************************/
u8 jdl_get_data(u8 *buff, u32 buff_size, u32 offset, u32 *g_size)
{
    u8 rtn = JDL_E_OK;
    //JDL_FILE_HEADER header;
    u8 file_header[JDL_SIZE_FILE_HEADER];
    u32 each_ofs[11];
    u32 wsize;
    u32 each_offset;
    
    each_ofs[0] = JDL_SIZE_FILE_HEADER;                                                                                            /* 0 : File Header */
    each_ofs[1] = (each_ofs[0] + JDL_SIZE_CATEGORY_HEADER + JDL_SYS_SEND_TOTAL);                                                   /* 1 : System */
    each_ofs[2] = (each_ofs[1] + JDL_SIZE_CATEGORY_HEADER + JDL_STAT_SEND_TOTAL);                                                  /* 2 : Statistics */
    each_ofs[3] = (each_ofs[2] + JDL_SIZE_CATEGORY_HEADER + JDL_SENS_SEND_TOTAL);                                                  /* 3 : Sensor */
    each_ofs[4] = (each_ofs[3] + JDL_SIZE_CATEGORY_HEADER + JDL_COMM_SEND_TOTAL);                                                  /* 4 : Communication */
    each_ofs[5] = (each_ofs[4] + JDL_SIZE_CATEGORY_HEADER + JDL_EVEN_SEND_TOTAL);                                                  /* 5 : Event */
    each_ofs[6] = (each_ofs[5] + JDL_SIZE_CATEGORY_HEADER + JDL_ERR_SEND_TOTAL + JDL_ERR_TRCD_TOTAL);                              /* 6 : Error */
    each_ofs[7] = (each_ofs[6] + JDL_SIZE_CATEGORY_HEADER + JDL_ACC_SIZE_DENOMI_NUM + JDL_ACC_CURRENCY_SIZE + JDL_ACC_SEND_TOTAL); /* 7 : Acceptance */
    each_ofs[8] = (each_ofs[7] + JDL_SIZE_CATEGORY_HEADER + JDL_PANA_SEND_TOTAL + (JDL_PANA_RCD_TOTAL * 2));                       /* 8 : Position Analysis */
#if defined(_JDL_USE_RC)
    each_ofs[9] = (each_ofs[8] + JDL_SIZE_CATEGORY_HEADER + JDL_RC_SEND_TOTAL);                                                    /* 9 : RC */
#endif /* _JDL_USE_RC */
#if defined(_JDL_USE_OP_RC)
	each_ofs[10] = (each_ofs[9] + JDL_SIZE_CATEGORY_HEADER + JDL_OPRC_SEND_TOTAL);                                                  /* 10: OPRC */
#endif /* _JDL_USE_OP_RC */
	
    
    if (offset < each_ofs[0])
    {
    /* 0 : File Header */
        _jdl_memset((void *)&file_header[0], 0, JDL_SIZE_FILE_HEADER);
        _jdl_memcpy((void *)&file_header[JDL_FHEAD_ADR_NAME], (u8 *)"JCM DEV LOG", sizeof("JCM DEV LOG"));
        _jdl_get_rev((u16 *)&file_header[JDL_FHEAD_ADR_REV]);
#if defined(UBA_RC) || defined(UBA_RTQ)
        *((u32 *)&file_header[JDL_FHEAD_ADR_FSIZE]) = _JDL_SWAP_32(each_ofs[10]);
#else
        *((u32 *)&file_header[JDL_FHEAD_ADR_FSIZE]) = _JDL_SWAP_32(each_ofs[8]);
#endif // UBA_RC || UBA_RTQ        
        *((u16 *)&file_header[JDL_FHEAD_ADR_PID]) = _JDL_SWAP_16(JCM_PRODUCT_ID);
        if ((buff_size + offset) > each_ofs[0])
        {
            wsize = (each_ofs[0] - offset);
        }
        else
        {
            wsize = buff_size;
        }
        _jdl_memcpy(buff, &file_header[offset], wsize);
        *g_size = wsize;
    }
    else if (offset < each_ofs[1])
    {
    /* 1 : System */
        each_offset = offset - each_ofs[0];
        rtn = _jdl_sys_get(buff, buff_size, each_offset, g_size);
    }
    else if (offset < each_ofs[2])
    {
    /* 2 : Statistics */
        each_offset = offset - each_ofs[1];
        rtn = _jdl_stat_get(buff, buff_size, each_offset, g_size);
    }
    else if (offset < each_ofs[3])
    {
    /* 3 : Sensor */
        each_offset = offset - each_ofs[2];
        rtn = _jdl_sens_get(buff, buff_size, each_offset, g_size);
    }
    else if (offset < each_ofs[4])
    {
    /* 4 : Communication */
        each_offset = offset - each_ofs[3];
        rtn = _jdl_comm_get(buff, buff_size, each_offset, g_size);
    }
    else if (offset < each_ofs[5])
    {
    /* 5 : Event */
        each_offset = offset - each_ofs[4];
        rtn = _jdl_event_get(buff, buff_size, each_offset, g_size);
    }
    else if (offset < each_ofs[6])
    {
    /* 6 : Error */
        each_offset = offset - each_ofs[5];
        rtn = _jdl_err_get(buff, buff_size, each_offset, g_size);
    }
    else if (offset < each_ofs[7])
    {
    /* 7 : Acceptance */
        each_offset = offset - each_ofs[6];
        rtn = _jdl_acc_get(buff, buff_size, each_offset, g_size);
    }
    else if (offset < each_ofs[8])
    {
    /* 8 : Position Analysis */
        each_offset = offset - each_ofs[7];
        rtn = _jdl_posiana_get(buff, buff_size, each_offset, g_size);
    }
#if defined(_JDL_USE_RC)
    else if (offset < each_ofs[9])
    {
    /* 9 : RC */
        each_offset = offset - each_ofs[8];
        rtn = _jdl_rc_get(buff, buff_size, each_offset, g_size);
    }
#endif
#if defined(_JDL_USE_OP_RC)
	else if (offset < each_ofs[10])
    {
    /* 10: OPRC */
        each_offset = offset - each_ofs[9];
        rtn = _jdl_oprc_get(buff, buff_size, each_offset, g_size);
    }
#endif /* _JDL_USE_OP_RC */
    else
    {
    	*g_size = 0;
        rtn = JDL_E_MACV;
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief End getting JDL
 * @return     error cord
 */
/***********************************************************************/
u8 jdl_get_end(void)
{
    u8 rtn = JDL_E_OK;
    
    _jdl_set_mode(JDL_MODE_ENABLE);
#ifdef _DEBUG_JDL  /* yamazaki JDL DEBUG */
    debug_jdl_wait_cnt++;
#endif /* _DEBUG_JDL  yamazaki JDL DEBUG */
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief initialize jcm device log
 * @param[in]  clear : force clear setting
 * @return     error cord
 */
/***********************************************************************/
u8 jdl_category_clear(u16 cate_no)
{
    u8 rtn = JDL_E_OK;
    
    /* [TODO]クリア中の書き込み対応の改善が必要なため、対策までコメントアウトする */
    //_jdl_set_mode(JDL_MODE_DISABLE);
    switch (cate_no)
    {
    case JDL_CATE_ID_ALL:
        rtn = jdl_init_sys(1);
        rtn = jdl_init_sens(1);
        rtn = _jdl_comm_init(1, JDL_NULL);
        rtn = _jdl_event_init(1);
        rtn = _jdl_err_init(1);
        rtn = _jdl_acc_init(1);
        rtn = _jdl_posiana_init(1);
#if defined(_JDL_USE_RC)
        rtn = _jdl_rc_init(1);
#endif /* _JDL_USE_RC */
#if defined(UBA_RC) || defined(UBA_RTQ)
        rtn = _jdl_oprc_init(1);
#endif
        _jdl_memset(&s_jdl_err_data[0], 0, JDL_ERR_OFS_BLK_EVEN_IDX);
        s_jdl_err_update_flag = 0;
        s_jdl_ins_flag = 0;
        break;
    case JDL_CATE_ID_SYSTEM:
        rtn = jdl_init_sys(1);
        break;
    case JDL_CATE_ID_STATISTICS:
        rtn = jdl_init_stat(1);
        break;
    case JDL_CATE_ID_SENSOR:
        rtn = jdl_init_sens(1);
        break;
    case JDL_CATE_ID_COMMUNICATION:
        rtn = _jdl_comm_init(1, JDL_NULL);
        break;
    case JDL_CATE_ID_EVENT:
        rtn = _jdl_event_init(1);
        break;
    case JDL_CATE_ID_ERROR:
        rtn = _jdl_err_init(1);
        break;
    case JDL_CATE_ID_ACCEPTANCE:
        rtn = _jdl_acc_init(1);
        break;
    case JDL_CATE_ID_POSIANA:
        rtn = _jdl_posiana_init(1);
        break;
#if defined(_JDL_USE_RC)
    case JDL_CATE_ID_AD_RC:
        rtn = _jdl_rc_init(1);
        break;
#endif /* _JDL_USE_RC */
#if defined(UBA_RC) || defined(UBA_RTQ)
    case JDL_CATE_ID_OP_RC:
        rtn = _jdl_oprc_init(1);
        break;
#endif
    case JDL_CATE_ID_CLEAR_JDL:
        jdl_init(1);
        break;
    }
    _jdl_set_mode(JDL_MODE_ENABLE);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief initialize system category
 * @param[in]  clear  : log clear (0:disabled, 1:enabled)
 * @return     error cord
 */
/***********************************************************************/
static u8 jdl_init_sys(u8 clear)
{
    u8 rtn = JDL_E_OK;
    u8 res;
    u8 *boot_ver;
    u8 temp[4];
#ifdef _DEBUG_JDL  /* DEBUG App表示確認 */
    JDL_TIME stime;
    JDL_TIME etime;
#endif /* _DEBUG_JDL */
    
    res = _jdl_sys_cmp(JDL_SYS_ADR_FIRM_VER, (u8 *)&software_ver[0], 64);
    if (res == JDL_E_DDATA)
    {
        rtn = JDL_E_VERCHG;
    }
    
    s_jdl_rtn = _jdl_sys_init(1);

    /* Model */
    s_jdl_rtn = _jdl_sys_set(JDL_SYS_ADR_MODEL, &ex_model[0], MODEL_LENGTH);

    /* Serial No. 1 */
    s_jdl_rtn = _jdl_sys_set(JDL_SYS_ADR_SERIAL1, &ex_adjustment_data.factory_info.serial_no[0], JDL_SIZE_SERIAL_NO);

    /* Serial No. 2 */
    s_jdl_rtn = _jdl_sys_set(JDL_SYS_ADR_SERIAL2, &ex_adjustment_data.maintenance_info.serial_no[0], JDL_SIZE_SERIAL_NO);

#ifdef _DEBUG_JDL  /* DEBUG App表示確認 */
    /* Serial No. 1 */
    s_jdl_rtn = _jdl_sys_set(JDL_SYS_ADR_SERIAL1, (u8 *)"181000000001", JDL_SIZE_SERIAL_NO);

    /* Serial No. 2 */
    s_jdl_rtn = _jdl_sys_set(JDL_SYS_ADR_SERIAL2, (u8 *)"181999999999", JDL_SIZE_SERIAL_NO);
#endif /* _DEBUG_JDL */

    /* Firmware Version */
    s_jdl_rtn = _jdl_sys_set(JDL_SYS_ADR_FIRM_VER, (u8 *)&software_ver[0], 64);

    /* Boot Version */
	boot_ver = (u8 *)0x00140540;
    s_jdl_rtn = _jdl_sys_set(JDL_SYS_ADR_BOOT_VER, boot_ver, 16);

    /* Set Time */
    /*   If this model has real time clock
     *   _jdl_set_time(&time);
     */
#ifdef _DEBUG_JDL  /* DEBUG App表示確認 */
    time.high = 0x00000000;
    time.low = 0x5C2AAD80;
    _jdl_set_time(&time);
#endif /* _DEBUG_JDL */

    /* Dip Switch Setting */
    temp[0] = ex_dipsw1;
    temp[1] = 0;
#ifdef _DEBUG_JDL  /* DEBUG App表示確認 */
    temp[1] = 0xFF;
#endif /* _DEBUG_JDL */
    s_jdl_rtn = _jdl_sys_set(JDL_SYS_ADR_OPT_DIPSW, &temp[0], 2);

    /* Option Setting */
    temp[0] = 0;
    temp[1] = 0;
    temp[2] = 0;
    temp[3] = 0;
#ifdef _DEBUG_JDL  /* DEBUG App表示確認 */
    temp[0] = 1;
    temp[1] = 2;
    temp[2] = 3;
    temp[3] = 4;
#endif /* _DEBUG_JDL */

    s_jdl_rtn = _jdl_sys_set(JDL_SYS_ADR_OPT_OPTION, &temp[0], 4);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief initialize system category
 * @param[in]  clear  : log clear (0:disabled, 1:enabled)
 * @return     error cord
 */
/***********************************************************************/
static u8 jdl_init_stat(u8 clear)
{
    u8 rtn = JDL_E_OK;
    
    rtn = _jdl_stat_init(clear);
    
    //if (rtn =)
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief initialize sensor category
 * @param[in]  clear  : log clear (0:disabled, 1:enabled)
 * @return     error cord
 */
/***********************************************************************/
static u8 jdl_init_sens(u8 clear)
{
    u8 rtn = JDL_E_OK;
    u16 cnt;
    u16 *wptr;
    u32 offset;
    //u16 adj_cnt;
    u8 tool_ver[4];
    u8 date[8];
    //u16 debug_size;
    
    rtn = _jdl_sens_init(clear);
    
    /*----------------------------------------------------------*/
    /* Set Maintenance Adjustment Information                   */
    /*----------------------------------------------------------*/
    //adj_cnt = 0; //暫定
    wptr = 0;
    tool_ver[0] = 0;
    tool_ver[1] = 0;
    tool_ver[2] = ex_adjustment_data.maintenance_info.version[1];
    tool_ver[3] = ex_adjustment_data.maintenance_info.version[0];

    for (cnt = 0; cnt < 8; cnt++)
    {
        date[cnt] = ex_adjustment_data.maintenance_info.date[cnt];
    }

    /* Adjustment count */
    //rtn = _jdl_sens_set_adj_val(JDL_SENS_ADR_NUM_OF_ADJ, JDL_DATA_TYPE_SIZE_WORD, adj_cnt);

    /* Adjustment tool version */
    rtn = _jdl_sens_set_blk(JDL_SENS_ADR_APP_VER, &tool_ver[0], 4);

    /* Adjustment date */
    rtn = _jdl_sens_set_blk(JDL_SENS_ADR_ADJ_TIME, &date[0], 8);

    /* Position Sensor D/A*/
    offset = JDL_SENS_ADR_ADJ_VAL;
    rtn = _jdl_sens_set_blk(offset, &ex_adjustment_data.maintenance_value.pos_entrance_da, 5);
    offset += 5;

    /* CIS Black data */
    rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_cis_adjustment_data.cis_bc, sizeof(BC_DATA));
    offset += sizeof(BC_DATA);
    /* CIS White data */
    rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_cis_adjustment_data.cis_wc, sizeof(WC_DATA));
    offset += sizeof(WC_DATA);
    /* CIS DA */
    rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_cis_adjustment_data.cis_da, sizeof(CIS_ADJUSTMENT_DA));
    offset += sizeof(CIS_ADJUSTMENT_DA);
    /* CIS Time */
    rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_cis_adjustment_data.cis_time, sizeof(CIS_ADJUSTMENT_TIME));
    offset += sizeof(CIS_ADJUSTMENT_TIME);
    /* CIS Analog */
    rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_cis_adjustment_data.afe_again, sizeof(AFE_ADJUSTMENT_AGAIN));
    offset += sizeof(AFE_ADJUSTMENT_AGAIN);
    /* CIS Digital gain */
    rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_cis_adjustment_data.afe_dgain, sizeof(AFE_ADJUSTMENT_DGAIN));
    offset += sizeof(AFE_ADJUSTMENT_DGAIN);
    /* CIS Offset */
    rtn = _jdl_sens_set_blk(offset, (u8 *)&ex_cis_adjustment_data.afe_aoffset, sizeof(AFE_ADJUSTMENT_AOFFSET));
    offset += sizeof(AFE_ADJUSTMENT_AOFFSET);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Get sensor status
 * @return     error cord
 */
/***********************************************************************/
static void jdl_get_sensor_status(u8 *buff)
{
//SSは2byteまで使っている、仕様的には4byteなのでRTQは4byte全て使用
#if defined(UBA_RTQ) //2024-07-17
    *buff = (u8)(ex_position_sensor & 0xFF);

	if(ex_rc_status.sst1B.bit.stacker_home == 1)
	{
		*buff |= 0x80;
	}
	else
	{
		*buff &= ~(0x80);
	}

    //2nd
    buff++;
    //2byte目のbit7以外は全て、RTQ関係で埋まっている
    //SSの2byte目に使用している、CIS紙幣検知のみ残してbit7にアサイン、BOX検知、BOX Home,CIS Enableの情報はあきらめる
    //UBA500 RTQとの違いはUBA500では未使用のbit7にCIS紙幣検知をアサイン
    // *buff = ((ex_rc_status.sst21A.byte & 0x38) >> 3);
    // *buff |= (ex_rc_status.sst22A.byte & 0x38);
    if (ex_rc_status.sst1A.bit.busy)        // RC busy status
    {
        *buff |= 0x01;
    }

    if (ex_rc_status.sst1B.bit.box_detect)  // BOX_EXIST
    {
        *buff |= 0x02;
    }
    else
    {
        *buff &= ~0x02;
    }

    if(ex_position_sensor & POSI_VALIDATION) // VALIDATION
    {
        *buff |= 0x04;
    }
    else
    {
        *buff &= ~0x04;
    }
#if 1 //2025-03-07
	if (ex_rc_status.sst1A.bit.warning)        // RC warning
	{
		*buff |= 0x08;
	}

	if (ex_rc_status.sst1A.bit.error)        // RC Error
	{
		*buff |= 0x10;
	}
	//2025-07-04
	if (ex_rc_status.sst1A.bit.pause)        // RC pause
	{
		*buff |= 0x80;
	}
	*buff &= ~0x60;
#else
	// other position bit = 0
    *buff &= ~0xF8;
#endif
    /* 3rd */
    buff++;
    // pos 1 2 3
    *buff = ((ex_rc_status.sst21A.byte & 0x38) >> 3);
    // pos 4 5 6
    *buff |= (ex_rc_status.sst22A.byte & 0x38);

    if(ex_rc_status.sst4A.bit.pos_sen1)
	{
        *buff |= 0x40;
	}
	else
	{
		*buff &= ~(0x40);
	}
	if(ex_rc_status.sst4A.bit.pos_sen2)
	{
        *buff |= 0x80;
	}
	else
	{
		*buff &= ~(0x80);
	}
    /* 4th */
    buff++;

    *buff = ex_rc_status.sst31A.byte & 0x7F;

    if(ex_rc_status.sst4A.bit.pos_sen3)
	{
        *buff |= 0x80;
	}
	else
	{
		*buff &= ~(0x80);
	}
    /* 5th */
    buff++;

    *buff = ex_rc_status.sst32A.byte & 0x7F;
    if(ex_rc_status.sst4A.bit.pos_senR)
	{
        *buff |= 0x80;
	}
	else
	{
		*buff &= ~(0x80);
	}
    #if 0 //UBA_MUST
    //3rd
    *buff++;
    *buff = (ex_rc_status.sst31A.byte & 0x7F);
    if (ex_rc_status.sst31B.byte & 0x0C)
    {
        *buff |= 0x80;
    }
    //4th
    *buff++;
    *buff = (ex_rc_status.sst32A.byte & 0x7F);
    if (ex_rc_status.sst32B.byte & 0x0C)
    {
        *buff |= 0x80;
    }
    #endif
#else

    *buff++ = (u8)(ex_position_sensor & 0xFF);
    *buff = (u8)((ex_position_sensor >> 8) & 0xFF);

    //2byte
    //2024-02-26
    if(ex_position_sensor & POSI_VALIDATION)
    {
        *buff |= 0x04;
    }
    else
    {
        *buff &= ~0x04; //2024-04-18
    }
    
    if(_cyc_validation_mode == VALIDATION_CHECK_MODE_RUN)
    {
        *buff |= 0x08;
    }
    else
    {
        *buff &= ~0x08; //2024-04-18
    }

    if ((_ir_feed_motor_ctrl.mode == MOTOR_FWD)
     || (_ir_feed_motor_ctrl.mode == MOTOR_REV))
    {
        *buff |= 0x10;
    }
    if ((_ir_centering_motor_ctrl.mode == MOTOR_FWD)
     || (_ir_centering_motor_ctrl.mode == MOTOR_REV))
    {
        *buff |= 0x20;
    }
    if ((_ir_stacker_motor_ctrl.mode == MOTOR_FWD)
     || (_ir_stacker_motor_ctrl.mode == MOTOR_REV))
    {
        *buff |= 0x40;
    }
#if 1 //UBA_MUST
    *buff++ = 0x00;
    *buff++ = 0x00;
    *buff++ = 0x00;
#endif

#endif

}


/***********************************************************************/
/**
 * @brief Count bill rejected
 * @param[in]  dinfo_idx : denomination information index
 * @param[in]  rej_code  : reject code
 * @return     error cord
 */
/***********************************************************************/
static u8 jdl_bill_reject(u16 dinfo_idx, u16 rej_code)
{
    u8 rtn = JDL_E_OK;
    
    switch (rej_code)
    {

    case REJECT_CODE_SKEW:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_SKEW, JDL_STAT_OFS_STAT_REJ_FEED);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_SKEW, JDL_ACC_OFS_S1_DINFO_FEED);
        break;
    case REJECT_CODE_MAG_PATTERN:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_MAG_PATTN, JDL_STAT_OFS_STAT_REJ_MAG);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_MAG_PATTN, JDL_ACC_OFS_S1_DINFO_MAG);
    	break;
    case REJECT_CODE_MAG_AMOUNT:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_MAG_AMOUN, JDL_STAT_OFS_STAT_REJ_MAG);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_MAG_AMOUN, JDL_ACC_OFS_S1_DINFO_MAG);
    	break;
    case REJECT_CODE_ACCEPTOR_STAY_PAPER:
    case REJECT_CODE_STACKER_STAY_PAPER:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_POSI_AT, JDL_STAT_OFS_STAT_REJ_FEED);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_POSI_AT, JDL_ACC_OFS_S1_DINFO_FEED);
        break;
    case REJECT_CODE_XRATE:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_XRATE, JDL_STAT_OFS_STAT_REJ_OPT);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_XRATE, JDL_ACC_OFS_S1_DINFO_OPT);
        break;
    case REJECT_CODE_INSERT_CANCEL:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_CANCEL, JDL_STAT_OFS_STAT_REJ_FEED);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_CANCEL, JDL_ACC_OFS_S1_DINFO_FEED);
        break;
    case REJECT_CODE_FEED_SLIP:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_SLIP, JDL_STAT_OFS_STAT_REJ_FEED);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_SLIP, JDL_ACC_OFS_S1_DINFO_FEED);
        break;
    case REJECT_CODE_FEED_MOTOR_LOCK:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_FMOT_LOCK, JDL_STAT_OFS_STAT_REJ_FEED);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_FMOT_LOCK, JDL_ACC_OFS_S1_DINFO_FEED);
        break;
    case REJECT_CODE_FEED_TIMEOUT:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_FEED_TOUT, JDL_STAT_OFS_STAT_REJ_FEED);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_FEED_TOUT, JDL_ACC_OFS_S1_DINFO_FEED);
        break;
    case REJECT_CODE_APB_HOME:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_APB_HOME, JDL_STAT_OFS_STAT_REJ_FEED);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_APB_HOME, JDL_ACC_OFS_S1_DINFO_FEED);
        break;
    case REJECT_CODE_CENTERING_HOME:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_CENT_HOME, JDL_STAT_OFS_STAT_REJ_FEED);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_CENT_HOME, JDL_ACC_OFS_S1_DINFO_FEED);
        break;
    case REJECT_CODE_PRECOMP:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_PRECOMP, JDL_STAT_OFS_STAT_REJ_OPT);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_PRECOMP, JDL_ACC_OFS_S1_DINFO_OPT);
        break;
    case REJECT_CODE_PATTERN:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_PHOT_PATT, JDL_STAT_OFS_STAT_REJ_OPT);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_PHOT_PATT, JDL_ACC_OFS_S1_DINFO_OPT);
        break;
    case REJECT_CODE_PHOTO_LEVEL:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_PHOT_LEVE, JDL_STAT_OFS_STAT_REJ_OPT);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_PHOT_LEVE, JDL_ACC_OFS_S1_DINFO_OPT);
        break;
    case REJECT_CODE_INHIBIT:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_INHIBIT, JDL_STAT_OFS_STAT_REJ_RET);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_INHIBIT, JDL_ACC_OFS_S1_DINFO_RET);
        break;
    case REJECT_CODE_ESCROW_TIMEOUT: //not use
    case REJECT_CODE_RETURN:
    case REJECT_CODE_OPERATION: //not use
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_RETURN, JDL_STAT_OFS_STAT_REJ_RET);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_RETURN, JDL_ACC_OFS_S1_DINFO_RET);
        break;
    case REJECT_CODE_LENGTH:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_LENGTH, JDL_STAT_OFS_STAT_REJ_OPT);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_LENGTH, JDL_ACC_OFS_S1_DINFO_OPT);
        break;
    case REJECT_CODE_PAPER_SHORT:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_SHORT, JDL_STAT_OFS_STAT_REJ_FEED);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_SHORT, JDL_ACC_OFS_S1_DINFO_FEED);
        break;
    case REJECT_CODE_PAPER_LONG:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_LONG, JDL_STAT_OFS_STAT_REJ_FEED);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_LONG, JDL_ACC_OFS_S1_DINFO_FEED);
        break;
    case REJECT_CODE_SYNC:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_SYNC, JDL_STAT_OFS_STAT_REJ_CF);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_SYNC, JDL_ACC_OFS_S1_DINFO_CF);
    	break;
    case REJECT_CODE_DYENOTE :
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_DYE, JDL_STAT_OFS_STAT_REJ_CF);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_DYE, JDL_ACC_OFS_S1_DINFO_CF);
    	break;
    case REJECT_CODE_HOLE:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_HOLE, JDL_STAT_OFS_STAT_REJ_CF);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_HOLE, JDL_ACC_OFS_S1_DINFO_CF);
    	break;
    case REJECT_CODE_TEAR:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_TEAR, JDL_STAT_OFS_STAT_REJ_CF);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_TEAR, JDL_ACC_OFS_S1_DINFO_CF);
    	break;
    case REJECT_CODE_DOG_EAR:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_DOG_EAR, JDL_STAT_OFS_STAT_REJ_CF);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_DOG_EAR, JDL_ACC_OFS_S1_DINFO_CF);
        break;
    case REJECT_CODE_COUNTERFEIT:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_CF, JDL_STAT_OFS_STAT_REJ_CF);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_CF, JDL_ACC_OFS_S1_DINFO_CF);
        break;
    case REJECT_CODE_FAKE_MCIR:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_MCIR, JDL_STAT_OFS_STAT_REJ_CF);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_MCIR, JDL_ACC_OFS_S1_DINFO_CF);
    	break;
    case REJECT_CODE_FAKE_M3C:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_M3C, JDL_STAT_OFS_STAT_REJ_CF);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_M3C, JDL_ACC_OFS_S1_DINFO_CF);
    	break;
    case REJECT_CODE_FAKE_M4C:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_M4C, JDL_STAT_OFS_STAT_REJ_CF);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_M4C, JDL_ACC_OFS_S1_DINFO_CF);
    	break;
    case REJECT_CODE_FAKE_IR:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_IR, JDL_STAT_OFS_STAT_REJ_CF);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_IR, JDL_ACC_OFS_S1_DINFO_CF);
    	break;
    case REJECT_CODE_THREAD:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_THREAD, JDL_STAT_OFS_STAT_REJ_CF);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_THREAD, JDL_ACC_OFS_S1_DINFO_CF);
        break;
    case REJECT_CODE_LOST_BILL:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_LOST, JDL_STAT_OFS_STAT_REJ_FEED);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_LOST, JDL_ACC_OFS_S1_DINFO_FEED);
        break;
    default:
        rtn = _jdl_stat_bill_reject(JDL_STAT_ADR_REJ_RESERVED2, JDL_STAT_OFS_STAT_REJ_FEED);
        rtn = _jdl_acc_bill_reject(dinfo_idx, JDL_ACC_OFS_REJ_RESERVED2, JDL_ACC_OFS_S1_DINFO_FEED);
        break;
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @param[in]  dinfo_idx : denomination information index
 * @param[in]  rej_code  : reject code
 * @brief Count ticket rejected
 * @return     error cord
 */
/***********************************************************************/
static u8 jdl_ticket_reject(u16 dinfo_idx, u16 rej_code)
{
    u8 rtn = JDL_E_OK;
    u16 tinfo_idx;
    
    if (dinfo_idx < JDL_ACC_DINFO_TICKET_IDX)
    {
    /* Index error */
        rtn = JDL_E_PARAM;
    }
    else
    {
        /* Shift ticket infomation index */
        tinfo_idx = (dinfo_idx - JDL_ACC_DINFO_TICKET_IDX);
        
        switch (rej_code)
        {
        case REJECT_CODE_BAR_NC:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_NC, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_NC, JDL_ACC_OFS_S1_DINFO_BAR_NC);
            break;
        case REJECT_CODE_BAR_UN:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_UN, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_UN, JDL_ACC_OFS_S1_DINFO_BAR_UN);
            break;
        case REJECT_CODE_BAR_SH:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_SH, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_SH, JDL_ACC_OFS_S1_DINFO_BAR_UN);
            break;
        case REJECT_CODE_BAR_ST:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_ST, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_ST, JDL_ACC_OFS_S1_DINFO_BAR_ST);
            break;
        case REJECT_CODE_BAR_SP:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_SP, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_SP, JDL_ACC_OFS_S1_DINFO_BAR_ST);
            break;
        case REJECT_CODE_BAR_TP:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_TP, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_TP, JDL_ACC_OFS_S1_DINFO_BAR_NC);
            break;
        case REJECT_CODE_BAR_XR:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_XR, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_XR, JDL_ACC_OFS_S1_DINFO_BAR_XR);
            break;
        case REJECT_CODE_BAR_PHV:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_PHV, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_PHV, JDL_ACC_OFS_S1_DINFO_BAR_XR);
            break;
        case REJECT_CODE_BAR_DIN:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_DIN, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_DIN, JDL_ACC_OFS_S1_DINFO_BAR_XR);
            break;
        case REJECT_CODE_BAR_LG:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_LG, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_LG, JDL_ACC_OFS_S1_DINFO_BAR_LG);
            break;
        case REJECT_CODE_BAR_NG:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_NG, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_NG, JDL_ACC_OFS_S1_DINFO_BAR_LG);
            break;
        case REJECT_CODE_BAR_MC:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_MC, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_MC, JDL_ACC_OFS_S1_DINFO_BAR_MC);
            break;
//      case BER_MC:	//2019-04-25 not use
//          rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_MC, JDL_STAT_OFS_STAT_REJ_BAR);
//          rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_MC, JDL_ACC_OFS_S1_DINFO_BAR_MC);
//          break;
        default:
            rtn = _jdl_stat_ticket_reject(JDL_STAT_ADR_REJ_BAR_UN, JDL_STAT_OFS_STAT_REJ_BAR);
            rtn = _jdl_acc_ticket_reject(tinfo_idx, JDL_ACC_OFS_REJ_BAR_RESERV, JDL_ACC_OFS_S1_DINFO_BAR_UN);
            break;
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @param[in]  err_code : error code
 * @param[in]  seq      : sequence no
 * @brief Count ticket rejected
 * @return     error cord
 */
/***********************************************************************/
static u8 jdl_save_error_log(u32 err_code, u32 seq)
{
    u8 rtn = JDL_E_OK;
#ifdef _DEBUG_JDL  /* DEBUG App表示確認 */
    u8 cnt;
#endif /* _DEBUG_JDL */
    
    s_jdl_err_update_flag = 1;
    if (((s_jdl_err_data[JDL_ERR_OFS_BLK_CORD]) == (err_code & 0xFF))
     && ((s_jdl_err_data[JDL_ERR_OFS_BLK_MODE]) == ((seq >> 8) & 0xFF))
     && ((s_jdl_err_data[JDL_ERR_OFS_BLK_MODE+1]) == (seq & 0xFF)))
    {
    /* Update error infomation due to same error */
        if (s_jdl_err_data[JDL_ERR_OFS_BLK_SAME] != 0xFF)
        {
            s_jdl_err_data[JDL_ERR_OFS_BLK_SAME]++;
        }
        _jdl_err_update(s_jdl_err_data);
    }
    else
    {
        _jdl_memset(&s_jdl_err_data[0], 0, JDL_ERR_OFS_BLK_EVEN_IDX);
        
        /* Set Error code */
        s_jdl_err_data[JDL_ERR_OFS_BLK_CORD]     = (err_code & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_CORD+1)] = 0x00;
        /* Set Task ID */
        switch (seq & 0xF000)
        {
        case 0x0000:
            s_jdl_err_data[JDL_ERR_OFS_BLK_TID] = ID_FEED_TASK;
            break;
        case 0x1000:
            s_jdl_err_data[JDL_ERR_OFS_BLK_TID] = ID_STACKER_TASK;
            break;
        case 0x2000:
            s_jdl_err_data[JDL_ERR_OFS_BLK_TID] = ID_CENTERING_TASK;
            break;
        case 0x3000:
            s_jdl_err_data[JDL_ERR_OFS_BLK_TID] = ID_APB_TASK;
            break;
        default:
            s_jdl_err_data[JDL_ERR_OFS_BLK_TID] = ID_MAIN_TASK;
            break;
        }
        /* Set Task Mode */
        s_jdl_err_data[JDL_ERR_OFS_BLK_MODE]     = ((seq >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_MODE+1)] = (seq & 0xFF);
        
#ifdef _DEBUG_JDL  /* DEBUG App表示確認 */
        for (cnt = 0; cnt < (JDL_ERR_OFS_BLK_EVEN_IDX - JDL_ERR_OFS_BLK_INFO); cnt++)
        {
            s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO + cnt)] = (cnt + 1);
        }
#else  /* _DEBUG_JDL */

        s_jdl_err_data[JDL_ERR_OFS_BLK_INFO]      = ex_main_task_mode1;
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+1)]  = ex_main_task_mode2;
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+2)]  = ((ex_dline_task_mode >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+3)]  = (ex_dline_task_mode & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+4)]  = ((ex_cline_status_tbl.line_task_mode >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+5)]  = (ex_cline_status_tbl.line_task_mode & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+6)]  = ((ex_feed_task_seq >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+7)]  = (ex_feed_task_seq & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+8)] = ((ex_discrimination_task_mode >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+9)] = (ex_discrimination_task_mode & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+10)] = ((ex_sensor_task_mode >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+11)] = (ex_sensor_task_mode & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+12)] = ((ex_timer_task_mode >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+13)] = (ex_timer_task_mode & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+14)] = ((ex_centering_task_seq >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+15)] = (ex_centering_task_seq & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+16)] = ((ex_apb_task_seq >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+17)] = (ex_apb_task_seq & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+18)] = ((ex_motor_task_mode >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+19)] = (ex_motor_task_mode & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+20)] = ((ex_fram_task_mode >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+21)] = (ex_fram_task_mode & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+22)] = ((ex_otg_task_mode >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+23)] = (ex_otg_task_mode & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+24)] = ((ex_display_task_mode >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+25)] = (ex_display_task_mode & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+26)] = ((ex_bezel_task_mode >> 8) & 0xFF);
        s_jdl_err_data[(JDL_ERR_OFS_BLK_INFO+27)] = (ex_bezel_task_mode & 0xFF);
#endif /* _DEBUG_JDL */
        
        _jdl_err_set(s_jdl_err_data);
        
        _jdl_posiana_set();
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @param[in]  err_code : error code
 * @brief Count ticket rejected
 * @return     error cord
 */
/***********************************************************************/
static u8 jdl_save_error_counter(u32 err_code)
{
    u8 rtn = JDL_E_OK;
    
    switch (err_code)
    {

    case ALARM_CODE_FRAM:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_MEMORY, JDL_STAT_OFS_STAT_ERR_MEM);
        break;
    case ALARM_CODE_SPI:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_PERIPHERA, JDL_STAT_OFS_STAT_ERR_MEM);
        break;
    case ALARM_CODE_STACKER_FULL:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_SK_FULL, JDL_STAT_OFS_STAT_ERR_FULL);
        break;
    case ALARM_CODE_FEED_OTHER_SENSOR_SK:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_POSI_SK, JDL_STAT_OFS_STAT_ERR_JAMS);
        break;
    case ALARM_CODE_FEED_SLIP_SK:
    case ALARM_CODE_FEED_MOTOR_LOCK_SK: //2024-02-13
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_SLIP_SK, JDL_STAT_OFS_STAT_ERR_JAMS);
        break;
    case ALARM_CODE_FEED_TIMEOUT_SK:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_F_TOUT_SK, JDL_STAT_OFS_STAT_ERR_JAMS);
        break;
    case ALARM_CODE_FEED_LOST_BILL:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_LOST_BILL, JDL_STAT_OFS_STAT_ERR_JAMS);
        break;
    case ALARM_CODE_FEED_MOTOR_SPEED_LOW:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_F_SPEED_L, JDL_STAT_OFS_STAT_ERR_FEED);
        break;
    case ALARM_CODE_FEED_MOTOR_SPEED_HIGH:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_F_SPEED_H, JDL_STAT_OFS_STAT_ERR_FEED);
        break;
    case ALARM_CODE_FEED_MOTOR_LOCK:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_FMOT_LOCK, JDL_STAT_OFS_STAT_ERR_FEED);
        break;
    case ALARM_CODE_FEED_OTHER_SENSOR_AT:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_POSI_AT, JDL_STAT_OFS_STAT_ERR_JAMA);
        break;
    case ALARM_CODE_FEED_SLIP_AT:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_SLIP_AT, JDL_STAT_OFS_STAT_ERR_JAMA);
        break;
    case ALARM_CODE_FEED_TIMEOUT_AT:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_F_TOUT_AT, JDL_STAT_OFS_STAT_ERR_JAMA);
        break;
    case ALARM_CODE_FEED_MOTOR_LOCK_AT:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_F_LOCK_AT, JDL_STAT_OFS_STAT_ERR_JAMA);
        break;
    case ALARM_CODE_APB_TIMEOUT:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_APB_TOUT, JDL_STAT_OFS_STAT_ERR_APB);
        break;
    case ALARM_CODE_APB_HOME:
    case ALARM_CODE_APB_HOME_STOP:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_APB_HOME, JDL_STAT_OFS_STAT_ERR_APB);
        break;
    case ALARM_CODE_CHEAT:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_CHEAT, JDL_STAT_OFS_STAT_ERR_CHEA);
        break;
    case ALARM_CODE_CENTERING_TIMEOUT:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_CENT_TOUT, JDL_STAT_OFS_STAT_ERR_CENT);
        break;
    case ALARM_CODE_CENTERING_HOME_STOP:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_CENT_HOME, JDL_STAT_OFS_STAT_ERR_CENT);
        break;
    case ALARM_CODE_POWER_OFF:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RESERVED1, JDL_STAT_OFS_STAT_ERR_COMM_WDT);
        break;
    case ALARM_CODE_EXTERNAL_RESET:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RESERVED3, JDL_STAT_OFS_STAT_ERR_COMM_WDT);
        break;
    case ALARM_CODE_PLL_LOCK:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RESERVED4, JDL_STAT_OFS_STAT_ERR_COMM_WDT);
        break;
    case ALARM_CODE_CIS_TEMPERATURE:		//2025-10-03
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_ICB_RESERVED8, JDL_STAT_OFS_STAT_ERR_ICB_REV5);
        break;
		
#if defined(UBA_RTQ)
    /* [ICB] [RFID] */
    //case ALARM_CODE_RFID_UNIT:
    //    rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_ICB_NO_RESP, JDL_STAT_OFS_STAT_ERR_ICB_NO_RESP);
    //    break;
    //case ALARM_CODE_RFID_ICB_COMMUNICTION:
    //    rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_ICB_COMMU, JDL_STAT_OFS_STAT_ERR_ICB_COMMU);
    //    break;
    case ALARM_CODE_RFID_ICB_DATA:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_ICB_DATA, JDL_STAT_OFS_STAT_ERR_ICB_DATA);
        break;
    case ALARM_CODE_RFID_ICB_SETTING:
    case ALARM_CODE_RFID_ICB_NUMBER_MISMATCH:
    case ALARM_CODE_RFID_ICB_NOT_INITIALIZE:
    case ALARM_CODE_ICB_FORCED_QUIT:
    case ALARM_CODE_RFID_ICB_MC_INVALID:
		rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_ICB_RESERVED1, JDL_STAT_OFS_STAT_ERR_ICB_REV1);
		break;
    /* [RC] */
    case ALARM_CODE_RC_ERROR:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_ERR, JDL_STAT_OFS_STAT_ERR_RC_ERR);
        break;
    case ALARM_CODE_RC_ROM:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_ROM, JDL_STAT_OFS_STAT_ERR_RC_ROM_DL);
        break;
	case ALARM_CODE_RC_REMOVED:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_REMOVED, JDL_STAT_OFS_STAT_ERR_RC_RM_COM);
        break;
	case ALARM_CODE_RC_COM:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_COMMU, JDL_STAT_OFS_STAT_ERR_RC_RM_COM);
        break;
	case ALARM_CODE_RC_DWERR:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_DL_ERR, JDL_STAT_OFS_STAT_ERR_RC_ROM_DL);
        break;
	case ALARM_CODE_RC_POS:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_POS, JDL_STAT_OFS_STAT_ERR_RC_OPT);
        break;
	case ALARM_CODE_RC_TRANSPORT:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_TRANS, JDL_STAT_OFS_STAT_ERR_RC_OPT);
        break;
	case ALARM_CODE_RC_TIMEOUT:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_TIMEOUT, JDL_STAT_OFS_STAT_ERR_RC_OPT);
        break;
	case ALARM_CODE_RC_DENOMINATION:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_DENOMI, JDL_STAT_OFS_STAT_ERR_RC_DENOMI);
        break;
	case ALARM_CODE_RC_EMPTY:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_EMPTY, JDL_STAT_OFS_STAT_ERR_RC_EMPTY);
        break;
	case ALARM_CODE_RC_DOUBLE:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_DOUBLE, JDL_STAT_OFS_STAT_ERR_RC_EMPTY);
        break;
	case ALARM_CODE_RC_FULL:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_FULL, JDL_STAT_OFS_STAT_ERR_RC_EMPTY);
        break;
	case ALARM_CODE_RC_EXCHAGED:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_EXCHANGE, JDL_STAT_OFS_STAT_ERR_RC_REV);
        break;
	case ALARM_CODE_RC_FORCED_QUIT:
	#if defined(UBA_RTQ_ICB)//#if defined(NEW_RFID)
	case ALARM_CODE_RC_RFID:
	#endif
		rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RC_FORCE_QUIT, JDL_STAT_OFS_STAT_ERR_RC_REV);
        break;	
#endif // UBA_RTQ
    default:
        rtn = _jdl_stat_err(JDL_STAT_ADR_ERR_RESERVED2, JDL_STAT_OFS_STAT_ERR_FEED);
        break;
    }
    return rtn;
}


/***********************************************************************/
/**
 * @brief prepare getting communication log
 * @return     error cord
 */
/***********************************************************************/
static u8 jdl_comm_get_sta(void)
{
    u8 rtn = JDL_E_OK;
    
    s_jdl_comm_prev_comm.rx_flag = 0;
    s_jdl_comm_prev_comm.rx_cmd = 0;
    s_jdl_comm_prev_comm.tx_flag = 0;
    s_jdl_comm_prev_comm.tx_rsp = 0;
    s_jdl_comm_wait_sst_rsp.rx_flag = 0;
    s_jdl_comm_wait_sst_rsp.rx_cmd = 0;
    s_jdl_comm_wait_sst_rsp.tx_flag = 0;
    s_jdl_comm_wait_sst_rsp.tx_rsp = 0;
    
    return rtn;
}


/*==========================================================================*/
/*==========================================================================*/
/* Use inside JDL                                                           */
/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>*/
/***********************************************************************/
/**
 * @brief Load logs from backup memory
 * @param[in]   offset : log offset to load
 * @param[in]   size   : log size to load
 * @param[out]  data   : load destination buffer
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_load(u32 offset, u32 size, u8 *data)
{
    u8 rtn = JDL_E_OK;
    u32 cnt;
    
    if ((offset >= JDL_BUFF_TOTAL)
     || (size > JDL_BUFF_TOTAL)
     || ((offset + size) > JDL_BUFF_TOTAL))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        _hal_read_fram_log(offset,&_bkex_jdl_buff[offset],size);
    /* Load data */
        for (cnt = 0; cnt < size; cnt++)
        {
            *(data + cnt) = _bkex_jdl_buff[(offset + cnt)];
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Load word type logs from backup memory
 * @param[in]   offset : log offset to load
 * @param[out]  data   : load destination buffer
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_load_word(u32 offset, u16 *data)
{
    u8 rtn;
    u16 load_data;
    
    rtn = _jdl_load(offset, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&load_data);
    
    if (rtn == JDL_E_OK)
    {
        *data = _JDL_SWAP_16(load_data);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Load double word type logs from backup memory
 * @param[in]   offset : log offset to load
 * @param[out]  data   : load destination buffer
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_load_dword(u32 offset, u32 *data)
{
    u8 rtn;
    u32 load_data;
    
    rtn = _jdl_load(offset, JDL_DATA_TYPE_SIZE_DWORD, (u8 *)&load_data);
    
    if (rtn == JDL_E_OK)
    {
        *data = _JDL_SWAP_32(load_data);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Save logs to backup memory
 * @param[in]   offset : log offset to save
 * @param[in]   size   : log size to save
 * @param[out]  data   : save data
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_save(u32 offset, u32 size, u8 *data)
{
    u8 rtn = JDL_E_OK;
    u32 cnt;
    
    if ((offset >= JDL_BUFF_TOTAL)
     || (size > JDL_BUFF_TOTAL)
     || ((offset + size) > JDL_BUFF_TOTAL))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
    /* Save data */
        for (cnt = 0; cnt < size; cnt++)
        {
            _bkex_jdl_buff[(offset + cnt)] = *(data + cnt);
        }
        _hal_write_fram_log(offset,&_bkex_jdl_buff[offset],size);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Save byte type logs to backup memory
 * @param[in]   offset : log offset to save
 * @param[out]  data   : save data
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_save_byte(u32 offset, u8 data)
{
    u8 rtn;
    //u8 save_data;
    
    /* Swap data */
    //save_data = data;
    
    /* Save data */
    rtn = _jdl_save(offset, JDL_DATA_TYPE_SIZE_BYTE, &data);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Save word type logs to backup memory
 * @param[in]   offset : log offset to save
 * @param[out]  data   : save data
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_save_word(u32 offset, u16 data)
{
    u8 rtn;
    u16 save_data;
    
    /* Swap data */
    save_data = _JDL_SWAP_16(data);
    
    /* Save data */
    rtn = _jdl_save(offset, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&save_data);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Save double word type logs to backup memory
 * @param[in]   offset : log offset to save
 * @param[out]  data   : save data
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_save_dword(u32 offset, u32 data)
{
    u8 rtn;
    u32 save_data;
    
    /* Swap data */
    save_data = _JDL_SWAP_32(data);
    
    /* Save data */
    rtn = _jdl_save(offset, JDL_DATA_TYPE_SIZE_DWORD, (u8 *)&save_data);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Clear logs on backup memory
 * @param[in]   offset : log offset to clear
 * @param[out]  data   : log size to clear
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_clear(u32 offset, u32 size)
{
    u8 rtn = JDL_E_OK;
    
    if ((offset >= JDL_BUFF_TOTAL)
     || (size > JDL_BUFF_TOTAL)
     || ((offset + size) > JDL_BUFF_TOTAL))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
    /* Clear buffer */
        _jdl_memset(&_bkex_jdl_buff[offset], 0, size);
        _hal_write_fram_log(offset,&_bkex_jdl_buff[offset],size);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Copy logs
 * @param[in]   d_offset : destination log offset
 * @param[in]   s_offset : source log offset
 * @param[in]   size     : log size
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_copy(u32 d_offset, u32 s_offset, u32 size)
{
    u8 rtn = JDL_E_OK;
    
    if ((d_offset >= JDL_BUFF_TOTAL)
     || (size > JDL_BUFF_TOTAL)
     || ((d_offset + size) > JDL_BUFF_TOTAL)
     || (s_offset >= JDL_BUFF_TOTAL)
     || ((s_offset + size) > JDL_BUFF_TOTAL))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
    /* Clear buffer */
        _jdl_memcpy(&_bkex_jdl_buff[d_offset], &_bkex_jdl_buff[s_offset], size);
    #if 1 //2024-03-06 //コピーはしているが、FRAMへの書き込み処理がないので、0のままになっていた書き込み処理を追加
        _hal_write_fram_log(d_offset,&_bkex_jdl_buff[d_offset],size);
    #endif

    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief calculate check sum
 * @param[in]  sta_offset : start offset
 * @param[in]  end_offset : end offset
 * @param[out] sum        : check sum
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_calc_checksum(u32 sta_offset, u32 end_offset, u16 *sum)
{
    u8 rtn = JDL_E_OK;
    u32 offset;
    
    if (sta_offset >= end_offset)
    {
    /* Address error */
        rtn = JDL_E_PARAM;
    }
    else if ((sta_offset >= JDL_BUFF_TOTAL)
     || (end_offset >= JDL_BUFF_TOTAL))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
    /* Calculate check sum */
        *sum = 0;
        for (offset = sta_offset; offset < end_offset; offset++)
        {
            *sum += _bkex_jdl_buff[offset];
        }
        *sum = ~(*sum);
        *sum = (*sum & 0x7FFF);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Renew check sum
 * @param[in]  sta_adr : start pointer
 * @param[in]  sum_adr : checksum pointer
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_renew_checksum(u32 sta_adr, u32 sum_adr)
{
    u8 rtn = JDL_E_OK;
    u16 sum = 0;
    
    /* Calculate checksum */
    rtn = _jdl_calc_checksum(sta_adr, sum_adr, &sum);
    if (rtn == JDL_E_OK)
    {
    /* Renew check sum */
        *((u16*)&_bkex_jdl_buff[sum_adr]) = (u16)_JDL_SWAP_16(sum);
        _hal_write_fram_log(sum_adr,&_bkex_jdl_buff[sum_adr],JDL_DATA_TYPE_SIZE_WORD);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Save data after checksum calculated
 * @param[in]  offset  : save data offset
 * @param[in]  data    : save data pointer
 * @param[in]  type    : data type (size)
 * @param[in]  sta_adr : start pointer
 * @param[in]  sum_adr : checksum pointer
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_save_data_checksum(u32 offset, u8 *data, u8 type, u32 sta_adr, u32 sum_adr)
{
    u8 rtn = JDL_E_OK;
    u16 sum = 0;
    u16 prev_sum;
    u32 cnt;
    
    /* Calculate checksum */
    for (cnt = sta_adr; cnt < sum_adr; cnt++)
    {
        if ((cnt >= offset) && (cnt < (offset + type)))
        {
        /* Use the save data */
            sum += *(data + (cnt - offset));
        }
        else
        {
        /* UBA-Pro：直接参照 RBAでは異なる */
            sum += _bkex_jdl_buff[cnt];
        }
    }
    sum = (~sum);
    sum = (sum & 0x7FFF);
    
    /* Load previous checksum */
    _jdl_load_word(sum_adr, &prev_sum);
    if (prev_sum & 0x8000)
    {
    /* Set corrupted flag */
        sum |= 0x8000;
    }
    
    /* Save data */
    _jdl_save(offset, type, data);
    /* Save checksum */
    _jdl_save_word(sum_adr, sum);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Save 2 data after checksum calculated
 * @param[in]  offset1    : save 1st data offset
 * @param[in]  inc_data1  : save 1st data pointer
 * @param[in]  type1      : save 1st data type (size)
 * @param[in]  offset2    : save 2nd data offset
 * @param[in]  inc_data2  : save 2nd data pointer
 * @param[in]  type2      : save 2nddata type (size)
 * @param[in]  sta_adr    : start pointer
 * @param[in]  sum_adr    : checksum pointer
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_save_data_2_checksum(u32 offset1, u8 *inc_data1, u8 type1, u32 offset2, u8 *inc_data2, u8 type2, u32 sta_adr, u32 sum_adr)
{
    u8 rtn = JDL_E_OK;
    u16 sum = 0;
    u16 prev_sum;
    u32 cnt;
    
    /* Calculate checksum */
    for (cnt = sta_adr; cnt < sum_adr; cnt++)
    {
        if ((cnt >= offset1) && (cnt < (offset1 + type1)))
        {
        /* Use the save inc_data1 */
            sum += *(inc_data1 + (cnt - offset1));
        }
        else if ((cnt >= offset2) && (cnt < (offset2 + type2)))
        {
        /* Use the save inc_data2 */
            sum += *(inc_data2 + (cnt - offset2));
        }
        else
        {
        /* UBA-Pro：直接参照 RBAでは異なる */
            sum += _bkex_jdl_buff[cnt];
        }
    }
    sum = (~sum);
    sum = (sum & 0x7FFF);
    
    /* Load previous checksum */
    _jdl_load_word(sum_adr, &prev_sum);
    if (prev_sum & 0x8000)
    {
    /* Set corrupted flag */
        sum |= 0x8000;
    }
    
    /* Save inc_data1 */
    _jdl_save(offset1, type1, inc_data1);
    /* Save inc_data2 */
    _jdl_save(offset2, type2, inc_data2);
    /* Save checksum */
    _jdl_save_word(sum_adr, sum);
    
    return rtn;
}
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
/* Use inside JDL                                                           */
/*==========================================================================*/
/*==========================================================================*/


/*==========================================================================*/
/*==========================================================================*/
/* For JDL Debug                                                            */
/*==========================================================================*/
#ifdef _DEBUG_JDL  /* yamazaki JDL DEBUG */
void jdl_debug(void)
{
    u32 cnt;
    u8 mode = 0;
    u32 rej_code = 0;
    u32 err_code = 0;
    u8 vali_flg = 0;
    
    debug_jdl_wait_cnt = 600000;
    
    for (cnt = 0; cnt < 600000; cnt++)
    {
        while (cnt >= debug_jdl_wait_cnt)
        {
            dly_tsk(1000);
        }
        switch (mode)
        {
        case 0:
            jdl_insert();
            jdl_accept(cnt % JDL_ACC_DENOMI_NUM);
            //jdl_accept(cnt % 10);
            //jdl_accept((cnt & 0x00000003));
            break;
        case 1:
            jdl_insert();
            jdl_accept(BAR_INDX);
            break;
        case 2:
            jdl_insert();
            rej_code++;
            if (rej_code > REJECT_CODE_LOST_BILL)
            {
                rej_code = 1;
            }
            jdl_reject(rej_code, vali_flg, (cnt % JDL_ACC_DENOMI_NUM), cnt, (0xFF00 | mode), (0xFF00 | vali_flg), 0xFFFF);
            //jdl_reject(rej_code, vali_flg, (cnt % 10), cnt, (0xFF00 | mode), (0xFF00 | vali_flg), 0xFFFF);
            //jdl_reject(rej_code, vali_flg, (cnt & 0x00000003), cnt, (0xFF00 | mode), (0xFF00 | vali_flg), 0xFFFF);
            break;
        case 3:
            jdl_insert();
            rej_code++;
            if (rej_code < REJECT_CODE_BAR_NC)
            {
                rej_code = REJECT_CODE_BAR_NC;
            }
            jdl_reject(rej_code, vali_flg, BAR_INDX, cnt, (0xFF00 | mode), (0xFF00 | vali_flg), 0xFFFF);
            break;
        case 4:
            err_code++;
            jdl_error(err_code, cnt, (0xFF00 | mode), (0xFF00 | mode), 0xFFFF);
            break;
        case 5:
            jdl_init(1);
            jdl_powerup();
            cnt = 0;
            mode = 0;
            break;
        }
    }
}
#endif /* _DEBUG_JDL  yamazaki JDL DEBUG */

