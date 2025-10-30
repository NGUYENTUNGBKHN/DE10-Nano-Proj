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
 * @file jdl_acceptance.c
 * @brief  Acceptance category of JCM Device Log
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/

#include "jdl.h"


#if defined(_JDL_USE_ACC)

/************************** BACKUP VARIABLES ***************************/


/************************** PRIVATE DEFINITIONS ************************/


/************************** PRIVATE FUNCTIONS **************************/
static void _jdl_acc_clear(void);
static void _jdl_acc_check_sum(void);
static u8 _jdl_acc_load_total_ins(void);
static u8 _jdl_acc_cnt(u32 offset, u8 type, u32 sta_adr, u32 sum_adr);
static u8 _jdl_acc_stat1_cnt(u16 dinfo_idx, u32 offset, u8 size);
static u8 _jdl_acc_stat2_cnt(u16 dinfo_idx, u32 offset, u8 size);


/************************** PRIVATE VARIABLES **************************/
static u16 _s_jdl_acc_rev;
static u32 _s_jdl_acc_total_ins;


/************************** EXTERN FUNCTIONS ***************************/


/************************** EXTERN VARIABLES ***************************/
extern const u8 currency_info[JDL_ACC_DENOMI_NUM][JDL_ACC_CURRENCY_EACH];


/***********************************************************************/
/**
 * @brief initialize acceptance category
 * @param[in]  clear : force clear setting
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_acc_init(u8 clear)
{
    u8 rtn = JDL_E_OK;
    
    /* Load acceptance category revision */
    _jdl_load(JDL_ACC_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_acc_rev);
    
    if (_s_jdl_acc_rev != _JDL_SWAP_16(JDL_ACC_REV))
    {
    /* Change revision */
        rtn = JDL_E_REVCHG;
    }
    
    if ((clear != 0)
     || (rtn != JDL_E_OK))
    {
    /* Clear log */
        _jdl_acc_clear();
    }
    else
    {
        /* Check the checksum */
        _jdl_acc_check_sum();
    }
    
    /* Load total insertion */
    _jdl_acc_load_total_ins();
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief increment the insertion counter.
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_acc_insert(void)
{
    u8 rtn = JDL_E_OK;
    u8 data[JDL_DATA_TYPE_SIZE_DWORD];
    u32 total_ins;
    JDL_TIME time;
    u32 temp;
    u32 cur_blk;
    u32 cur_blk_base;
    u8 gettime = 0;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
        /* Select counter type and incremented */
        switch (JDL_ACC_SIZE_ACC_CNTR)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            rtn = _jdl_inc_8bit(JDL_ACC_ADR_TOTAL_INS, &data[0]);
            total_ins = data[0];
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            rtn = _jdl_inc_16bit(JDL_ACC_ADR_TOTAL_INS, (u16 *)&data[0]);
            total_ins = _JDL_SWAP_16(*((u16 *)&data[0]));
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            rtn = _jdl_inc_32bit(JDL_ACC_ADR_TOTAL_INS, (u32 *)&data[0]);
            total_ins = _JDL_SWAP_32(*((u32 *)&data[0]));
            break;
        default:
            rtn = JDL_E_CONF;
            break;
        }
        
        /* Update counter */
        if (rtn == JDL_E_OK)
        {
            /* Calaculate checksum and Save counter */
            _jdl_save_data_checksum(JDL_ACC_ADR_TOTAL_INS, &data[0], JDL_ACC_SIZE_ACC_CNTR, JDL_ACC_ADR_STA_DATE, JDL_ACC_ADR_ACC_CHECKSUM);
            /* Update private variables */
            _s_jdl_acc_total_ins = total_ins;
            
            /* Check update for Statistics1 per Insertion*/
            temp = (_s_jdl_acc_total_ins % (JDL_ACC_STAT1_PINS_NUM_BLK * JDL_ACC_STAT1_BLK_NUM));
#if 1  /* yamazaki JDL Modify */
            if ((temp % JDL_ACC_STAT1_PINS_NUM_BLK) == 1)
#else  /* yamazaki JDL Modify */
            if ((temp % JDL_ACC_STAT1_PINS_NUM_BLK) == 0)
#endif /* yamazaki JDL Modify */
            {
                /* Get JDL time (1sec) */
                _jdl_get_1s_ctime(&time);
                gettime = 1;
                
                /* Search current block base */
                cur_blk = (temp / JDL_ACC_STAT1_PINS_NUM_BLK);
                cur_blk_base = (JDL_ACC_ADR_STAT1_BLK_BASE + (JDL_ACC_STAT1_BLK_SIZE * cur_blk));
                
                /* Clear Buffer */
                _jdl_clear(cur_blk_base, JDL_ACC_STAT1_BLK_SIZE);
                
                /* Set start time */
                _jdl_save_dword((cur_blk_base + JDL_ACC_OFS_S1_BLK_STIME), time.high);
                _jdl_save_dword((cur_blk_base + JDL_ACC_OFS_S1_BLK_STIME + JDL_DATA_TYPE_SIZE_DWORD), time.low);
                
                /* Renew checksum */
                _jdl_renew_checksum(cur_blk_base, (cur_blk_base + JDL_ACC_OFS_S1_BLK_CSUM));
            }
            
            /* Check update for Statistics2 per Insertion*/
            temp = (_s_jdl_acc_total_ins % (JDL_ACC_STAT2_PINS_NUM_BLK * JDL_ACC_STAT2_BLK_NUM));
#if 1  /* yamazaki JDL Modify */
            if ((temp % JDL_ACC_STAT2_PINS_NUM_BLK) == 1)
#else  /* yamazaki JDL Modify */
            if ((temp % JDL_ACC_STAT2_PINS_NUM_BLK) == 0)
#endif /* yamazaki JDL Modify */
            {
                /* Get JDL time (1sec) */
                if (gettime == 0)
                {
                    _jdl_get_1s_ctime(&time);
                }
                
                /* Search current block base */
                cur_blk = (temp / JDL_ACC_STAT2_PINS_NUM_BLK);
                cur_blk_base = (JDL_ACC_ADR_STAT2_BLK_BASE + (JDL_ACC_STAT2_BLK_SIZE * cur_blk));
                
                /* Clear Buffer */
                _jdl_clear(cur_blk_base, JDL_ACC_STAT2_BLK_SIZE);
                
                
                /* Set start time */
                _jdl_save_dword((cur_blk_base + JDL_ACC_OFS_S2_BLK_STIME), time.high);
                _jdl_save_dword((cur_blk_base + JDL_ACC_OFS_S2_BLK_STIME + JDL_DATA_TYPE_SIZE_DWORD), time.low);
                
                /* Renew checksum */
                _jdl_renew_checksum(cur_blk_base, (cur_blk_base + JDL_ACC_OFS_S2_BLK_CSUM));
            }
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief increment bill accepted counter.
 * @param[in]  denomi_idx : denomination index
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_acc_bill_accept(u16 dinfo_idx)
{
    u8 rtn = JDL_E_OK;
    u8 rtn2 = JDL_E_OK;
    u8 inc_data1[JDL_DATA_TYPE_SIZE_DWORD];
    u8 inc_data2[JDL_DATA_TYPE_SIZE_DWORD];
    u32 dinfo_base;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if (dinfo_idx >= JDL_ACC_DENOMI_NUM)
    {
    /* Denomi index error */
        rtn = JDL_E_PARAM;
    }
    else
    {
        /* Search denomi info base */
        dinfo_base = (JDL_ACC_ADR_ACC_BASE + (dinfo_idx * JDL_ACC_DINFO_SIZE));
        
        /* Select counter type and incremented */
        switch (JDL_ACC_SIZE_ACC_CNTR)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            /* Increment bill insertion counter */
            rtn = _jdl_inc_8bit(JDL_ACC_ADR_BILL_INS, &inc_data1[0]);
            /* Increment bill accepted counter */
            rtn2 = _jdl_inc_8bit(dinfo_base, &inc_data2[0]);
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            /* Increment bill insertion counter */
            rtn = _jdl_inc_16bit(JDL_ACC_ADR_BILL_INS, (u16 *)&inc_data1[0]);
            /* Increment bill accepted counter */
            rtn2 = _jdl_inc_16bit(dinfo_base, (u16 *)&inc_data2[0]);
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            /* Increment bill insertion counter */
            rtn = _jdl_inc_32bit(JDL_ACC_ADR_BILL_INS, (u32 *)&inc_data1[0]);
            /* Increment bill accepted counter */
            rtn2 = _jdl_inc_32bit(dinfo_base, (u32 *)&inc_data2[0]);
            break;
        default:
            rtn = JDL_E_CONF;
            break;
        }
        
        /* Check rtn and rtn2 result */
        if (((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
         && ((rtn2 == JDL_E_OK) || (rtn2 == JDL_E_CMAX)))
        {
            /* Calaculate checksum and Save data */
            rtn = _jdl_save_data_2_checksum(JDL_ACC_ADR_BILL_INS, &inc_data1[0], JDL_ACC_SIZE_ACC_CNTR, dinfo_base, &inc_data2[0], JDL_ACC_SIZE_ACC_CNTR, JDL_ACC_ADR_STA_DATE, JDL_ACC_ADR_ACC_CHECKSUM);
            if (rtn == JDL_E_OK)
            {
                /* Statistics 1 */
                rtn = _jdl_acc_stat1_cnt(dinfo_idx, JDL_ACC_OFS_S1_DINFO_ACC, JDL_ACC_SIZE_STAT_A_CNTR);
                /* Statistics 2 */
                rtn2 = _jdl_acc_stat2_cnt(dinfo_idx, JDL_ACC_OFS_S2_DINFO_ACC, JDL_ACC_SIZE_STAT_A_CNTR);
                
                if ((rtn == JDL_E_OK) && (rtn2 != JDL_E_OK))
                {
                    /* Replace rtn to rtn2 */
                    rtn = rtn2;
                }
            }
        }
        else if ((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
        {
            /* Replace rtn to rtn2 */
            rtn = rtn2;
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief increment ticket accepted counter.
 * @param[in]  tinfo_idx : ticket index
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_acc_ticket_accept(u16 tinfo_idx)
{
    u8 rtn = JDL_E_OK;
    u8 rtn2 = JDL_E_OK;
    u8 inc_data1[JDL_DATA_TYPE_SIZE_DWORD];
    u8 inc_data2[JDL_DATA_TYPE_SIZE_DWORD];
    u32 dinfo_base;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if (JDL_ACC_TICKET_NUM == 0)
    {
    /* Unsupported ticket */
        rtn = JDL_E_CONF;
    }
    else if (tinfo_idx >= JDL_ACC_TICKET_NUM)
    {
    /* Denomi(Ticket) index error */
        rtn = JDL_E_PARAM;
    }
    else
    {
        /* Search Ticket info base */
        dinfo_base = (JDL_ACC_ADR_ACC_BASE + ((JDL_ACC_DINFO_TICKET_IDX + tinfo_idx) * JDL_ACC_DINFO_SIZE));
        
        /* Select counter type and incremented */
        switch (JDL_ACC_SIZE_ACC_CNTR)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            /* Increment ticket insertion counter */
            rtn = _jdl_inc_8bit(JDL_ACC_ADR_TICKET_INS, &inc_data1[0]);
            /* Increment ticket accepted counter */
            rtn2 = _jdl_inc_8bit(dinfo_base, &inc_data2[0]);
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            /* Increment ticket insertion counter */
            rtn = _jdl_inc_16bit(JDL_ACC_ADR_TICKET_INS, (u16 *)&inc_data1[0]);
            /* Increment ticket accepted counter */
            rtn2 = _jdl_inc_16bit(dinfo_base, (u16 *)&inc_data2[0]);
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            /* Increment ticket insertion counter */
            rtn = _jdl_inc_32bit(JDL_ACC_ADR_TICKET_INS, (u32 *)&inc_data1[0]);
            /* Increment ticket accepted counter */
            rtn2 = _jdl_inc_32bit(dinfo_base, (u32 *)&inc_data2[0]);
            break;
        default:
            rtn = JDL_E_CONF;
            break;
        }
        
        /* Check rtn and rtn2 result */
        if (((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
         && ((rtn2 == JDL_E_OK) || (rtn2 == JDL_E_CMAX)))
        {
            /* Calaculate checksum and Save data */
            rtn = _jdl_save_data_2_checksum(JDL_ACC_ADR_TICKET_INS, &inc_data1[0], JDL_ACC_SIZE_ACC_CNTR, dinfo_base, &inc_data2[0], JDL_ACC_SIZE_ACC_CNTR, JDL_ACC_ADR_STA_DATE, JDL_ACC_ADR_ACC_CHECKSUM);
            if (rtn == JDL_E_OK)
            {
                /* Statistics 1 */
                rtn = _jdl_acc_stat1_cnt((JDL_ACC_DINFO_TICKET_IDX + tinfo_idx), JDL_ACC_OFS_S1_DINFO_ACC, JDL_ACC_SIZE_STAT_A_CNTR);
                /* Statistics 2 */
                rtn2 = _jdl_acc_stat2_cnt((JDL_ACC_DINFO_TICKET_IDX + tinfo_idx), JDL_ACC_OFS_S2_DINFO_ACC, JDL_ACC_SIZE_STAT_A_CNTR);
                
                if ((rtn == JDL_E_OK) && (rtn2 != JDL_E_OK))
                {
                    /* Replace rtn to rtn2 */
                    rtn = rtn2;
                }
            }
        }
        else if ((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
        {
            /* Replace rtn to rtn2 */
            rtn = rtn2;
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief increment bill rejected counter.
 * @param[in]  dinfo_idx   : denomination infomation index
 * @param[in]  rej_offset  : offset of data to count
 * @param[in]  stat_offset : offset of data to count per insertion
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_acc_bill_reject(u16 dinfo_idx, u32 rej_offset, u32 stat_offset)
{
    u8 rtn = JDL_E_OK;
    u8 rtn2 = JDL_E_OK;
    u8 inc_data1[JDL_DATA_TYPE_SIZE_DWORD];
    u8 inc_data2[JDL_DATA_TYPE_SIZE_DWORD];
    u32 dinfo_base;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if (dinfo_idx > JDL_ACC_DINFO_UNKNOWN_IDX)
    {
    /* Denomi index error */
        rtn = JDL_E_PARAM;
    }
    else if (((rej_offset < JDL_ACC_SIZE_ACC_CNTR) || (JDL_ACC_DINFO_SIZE <= rej_offset))
          || ((stat_offset < JDL_ACC_SIZE_STAT_A_CNTR) || (JDL_ACC_OFS_S2_BLK_STIME <= stat_offset)))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        /* Search denomi info base */
        dinfo_base = (JDL_ACC_ADR_ACC_BASE + (dinfo_idx * JDL_ACC_DINFO_SIZE));
        
        /* Select counter type and incremented */
        switch (JDL_ACC_SIZE_ACC_CNTR)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            /* Increment bill insertion counter */
            rtn = _jdl_inc_8bit(JDL_ACC_ADR_BILL_INS, &inc_data1[0]);
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            /* Increment bill insertion counter */
            rtn = _jdl_inc_16bit(JDL_ACC_ADR_BILL_INS, (u16 *)&inc_data1[0]);
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            /* Increment bill insertion counter */
            rtn = _jdl_inc_32bit(JDL_ACC_ADR_BILL_INS, (u32 *)&inc_data1[0]);
            break;
        default:
            rtn = JDL_E_CONF;
            break;
        }
        
        /* Select counter type and incremented */
        switch (JDL_ACC_SIZE_REJ_CNTR)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            /* Increment bill accepted counter */
            rtn2 = _jdl_inc_8bit((dinfo_base + rej_offset), &inc_data2[0]);
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            /* Increment bill accepted counter */
            rtn2 = _jdl_inc_16bit((dinfo_base + rej_offset), (u16 *)&inc_data2[0]);
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            /* Increment bill accepted counter */
            rtn2 = _jdl_inc_32bit((dinfo_base + rej_offset), (u32 *)&inc_data2[0]);
            break;
        default:
            rtn = JDL_E_CONF;
            break;
        }
        
        /* Check rtn and rtn2 result */
        if (((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
         && ((rtn2 == JDL_E_OK) || (rtn2 == JDL_E_CMAX)))
        {
            /* Calaculate checksum and Save data */
            rtn = _jdl_save_data_2_checksum(JDL_ACC_ADR_BILL_INS, &inc_data1[0], JDL_ACC_SIZE_ACC_CNTR, (dinfo_base + rej_offset), &inc_data2[0], JDL_ACC_SIZE_REJ_CNTR, JDL_ACC_ADR_STA_DATE, JDL_ACC_ADR_ACC_CHECKSUM);
            if (rtn == JDL_E_OK)
            {
                /* Statistics 1 */
                rtn = _jdl_acc_stat1_cnt(dinfo_idx, stat_offset, JDL_ACC_SIZE_STAT_R_CNTR);
                /* Statistics 2 */
                rtn2 = _jdl_acc_stat2_cnt(dinfo_idx, stat_offset, JDL_ACC_SIZE_STAT_R_CNTR);
                
                if ((rtn == JDL_E_OK) && (rtn2 != JDL_E_OK))
                {
                    /* Replace rtn to rtn2 */
                    rtn = rtn2;
                }
            }
        }
        else if ((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
        {
            /* Replace rtn to rtn2 */
            rtn = rtn2;
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief increment ticket rejected counter.
 * @param[in]  tinfo_idx : ticket index
 * @param[in]  rej_offset  : offset of data to count
 * @param[in]  stat_offset : offset of data to count per insertion
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_acc_ticket_reject(u16 tinfo_idx, u32 rej_offset, u32 stat_offset)
{
    u8 rtn = JDL_E_OK;
    u8 rtn2 = JDL_E_OK;
    u8 inc_data1[JDL_DATA_TYPE_SIZE_DWORD];
    u8 inc_data2[JDL_DATA_TYPE_SIZE_DWORD];
    u32 dinfo_base;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if (JDL_ACC_TICKET_NUM == 0)
    {
    /* Unsupported ticket */
        rtn = JDL_E_CONF;
    }
    else if (tinfo_idx >= JDL_ACC_TICKET_NUM)
    {
    /* Denomi(Ticket) index error */
        rtn = JDL_E_PARAM;
    }
    else if (((rej_offset < JDL_ACC_SIZE_ACC_CNTR) || (JDL_ACC_DINFO_SIZE <= rej_offset))
          || ((stat_offset < JDL_ACC_SIZE_STAT_A_CNTR) || (JDL_ACC_OFS_S2_BLK_STIME <= stat_offset)))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        /* Search Ticket info base */
        dinfo_base = (JDL_ACC_ADR_ACC_BASE + ((JDL_ACC_DINFO_TICKET_IDX + tinfo_idx) * JDL_ACC_DINFO_SIZE));
        
        /* Select counter type and incremented */
        switch (JDL_ACC_SIZE_ACC_CNTR)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            /* Increment ticket insertion counter */
            rtn = _jdl_inc_8bit(JDL_ACC_ADR_TICKET_INS, &inc_data1[0]);
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            /* Increment ticket insertion counter */
            rtn = _jdl_inc_16bit(JDL_ACC_ADR_TICKET_INS, (u16 *)&inc_data1[0]);
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            /* Increment ticket insertion counter */
            rtn = _jdl_inc_32bit(JDL_ACC_ADR_TICKET_INS, (u32 *)&inc_data1[0]);
            break;
        default:
            rtn = JDL_E_CONF;
            break;
        }
        
        /* Select counter type and incremented */
        switch (JDL_ACC_SIZE_REJ_CNTR)
        {
        case 1:
            /* Increment bill accepted counter */
            rtn2 = _jdl_inc_8bit((dinfo_base + rej_offset), &inc_data2[0]);
            break;
        case 2:
            /* Increment bill accepted counter */
            rtn2 = _jdl_inc_16bit((dinfo_base + rej_offset), (u16 *)&inc_data2[0]);
            break;
        case 4:
            /* Increment bill accepted counter */
            rtn2 = _jdl_inc_32bit((dinfo_base + rej_offset), (u32 *)&inc_data2[0]);
            break;
        default:
            rtn = JDL_E_CONF;
            break;
        }
        
        /* Check rtn and rtn2 result */
        if (((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
         && ((rtn2 == JDL_E_OK) || (rtn2 == JDL_E_CMAX)))
        {
            /* Calaculate checksum and Save data */
            rtn = _jdl_save_data_2_checksum(JDL_ACC_ADR_TICKET_INS, &inc_data1[0], JDL_ACC_SIZE_ACC_CNTR, (dinfo_base + rej_offset), &inc_data2[0], JDL_ACC_SIZE_REJ_CNTR, JDL_ACC_ADR_STA_DATE, JDL_ACC_ADR_ACC_CHECKSUM);
            if (rtn == JDL_E_OK)
            {
                /* Statistics 1 */
                rtn = _jdl_acc_stat1_cnt((JDL_ACC_DINFO_TICKET_IDX + tinfo_idx), stat_offset, JDL_ACC_SIZE_STAT_R_CNTR);
                /* Statistics 2 */
                rtn2 = _jdl_acc_stat2_cnt((JDL_ACC_DINFO_TICKET_IDX + tinfo_idx), (stat_offset + JDL_ACC_STAT2_BLK_REJ_SIZE), JDL_ACC_SIZE_STAT_R_CNTR);
                
                if ((rtn == JDL_E_OK) && (rtn2 != JDL_E_OK))
                {
                    /* Replace rtn to rtn2 */
                    rtn = rtn2;
                }
            }
        }
        else if ((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
        {
            /* Replace rtn to rtn2 */
            rtn = rtn2;
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief request the data from log buffer of system category 
 * @param[out] buff      : pointer to the buffer to get
 * @param[in]  buff_size : buffer size
 * @param[in]  s_offset  : offset address of send data for each category
 * @param[out] g_size    : size of data gotten
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_acc_req(u32 s_offset, u32 buff_size)
{
    u8 rtn;

	if (buff_size < 1)
    {
    /* Size error */
        rtn = JDL_E_PARAM;
    }
    else if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_ACC_SIZE_DENOMI_NUM + JDL_ACC_CURRENCY_SIZE + JDL_ACC_SEND_TOTAL))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
    	rtn = JDL_E_OK;
    }
	
	return rtn;
}


/***********************************************************************/
/**
 * @brief get the data from log buffer of acceptance category 
 * @param[out] buff      : pointer to the buffer to get
 * @param[in]  buff_size : buffer size
 * @param[in]  s_offset  : offset address of send data for each category
 * @param[out] g_size    : size of data gotten
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_acc_get(u8 *buff, u32 buff_size, u32 s_offset, u32 *g_size)
{
    u8 rtn = JDL_E_OK;
    u8 category_header[JDL_SIZE_CATEGORY_HEADER];
    u8 denomi_num[JDL_ACC_SIZE_DENOMI_NUM];
    u32 w_size;
    u32 w_offset;
    u32 temp_wsize;
    u32 temp_offset;
    
    if (buff_size < 1)
    {
    /* Size error */
        rtn = JDL_E_PARAM;
    }
    else if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_ACC_SIZE_DENOMI_NUM + JDL_ACC_CURRENCY_SIZE + JDL_ACC_SEND_TOTAL))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        w_size = 0;
        w_offset = s_offset;
        
        if ((w_size < buff_size)
         && (w_offset < JDL_SIZE_CATEGORY_HEADER))
        {
        /* Header Area */
            _jdl_memset((void *)&category_header[0], 0, JDL_SIZE_CATEGORY_HEADER);
            _jdl_memcpy((void *)&category_header[JDL_CHEAD_ADR_NAME], "ACCEPTANCE", sizeof("ACCEPTANCE"));
            *((u16 *)&category_header[JDL_CHEAD_ADR_ID]) = _JDL_SWAP_16(JDL_CATE_ID_ACCEPTANCE);
            *((u16 *)&category_header[JDL_CHEAD_ADR_REV]) = _s_jdl_acc_rev;
            *((u32 *)&category_header[JDL_CHEAD_ADR_CSIZE]) = _JDL_SWAP_32(((u32)JDL_SIZE_CATEGORY_HEADER + (u32)JDL_ACC_SIZE_DENOMI_NUM + (u32)JDL_ACC_CURRENCY_SIZE + (u32)JDL_ACC_SEND_TOTAL));
            
            /* Check remaining size */
            if ((buff_size + w_offset) <= JDL_SIZE_CATEGORY_HEADER)
            {
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_SIZE_CATEGORY_HEADER - w_offset);
            }
            _jdl_memcpy((buff + w_size), &category_header[w_offset], temp_wsize);
            w_size += temp_wsize;
            w_offset += temp_wsize;
        }
        
        if ((w_size < buff_size)
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_ACC_SIZE_DENOMI_NUM)))
        {
        /* Number of Denomination */
            temp_offset = (w_offset - JDL_SIZE_CATEGORY_HEADER);
            
            denomi_num[0] = (u8)((JDL_ACC_DENOMI_NUM >> 8) & 0xFF);
            denomi_num[1] = (u8)((JDL_ACC_DENOMI_NUM) & 0xFF);
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_ACC_SIZE_DENOMI_NUM)
            {
            /* Log Datas or less */
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_ACC_SIZE_DENOMI_NUM - temp_offset);
            }
            _jdl_memcpy((buff + w_size), &denomi_num[temp_offset], temp_wsize);
            w_size += temp_wsize;
            w_offset += temp_wsize;
        }
        
        
        if ((w_size < buff_size)
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_ACC_SIZE_DENOMI_NUM + JDL_ACC_CURRENCY_SIZE)))
        {
        /* Log Datas Area */
            temp_offset = (w_offset - (JDL_SIZE_CATEGORY_HEADER + JDL_ACC_SIZE_DENOMI_NUM));
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_ACC_CURRENCY_SIZE)
            {
            /* Log Datas or less */
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_ACC_CURRENCY_SIZE - temp_offset);
            }
            _jdl_memcpy((buff + w_size), ((u8 *)&currency_info + temp_offset), temp_wsize);
            w_size += temp_wsize;
            w_offset += temp_wsize;
        }
        
        if ((w_size < buff_size)
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_ACC_SIZE_DENOMI_NUM + JDL_ACC_CURRENCY_SIZE + JDL_ACC_SEND_TOTAL)))
        {
        /* Log Datas Area */
            temp_offset = (w_offset - (JDL_SIZE_CATEGORY_HEADER + JDL_ACC_SIZE_DENOMI_NUM + JDL_ACC_CURRENCY_SIZE));
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_ACC_SEND_TOTAL)
            {
            /* Log Datas or less */
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_ACC_SEND_TOTAL - temp_offset);
            }
            /* Add Category base address */
            temp_offset += JDL_ACC_ADR_SEND_BASE;
            _jdl_load(temp_offset, temp_wsize, (buff + w_size));
            w_size += temp_wsize;
            w_offset += temp_wsize;
        }
        
        *g_size = w_size;
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Clear acceptance category. 
 * @param[in]  non
 * @return     non
 */
/***********************************************************************/
static void _jdl_acc_clear(void)
{
    u32 blk_base;
    u32 blk_sum;
    u32 prev_insert;
    JDL_TIME time;
    u8 cnt;
    
    /* Get time */
    _jdl_get_1s_ctime(&time);
    
    /* Store number of inserted notes at the previous version */
    _jdl_load_dword(JDL_ACC_ADR_BILL_INS, &prev_insert);
    
    /* Clear Buffer */
    _jdl_clear(JDL_ACC_ADR_BUFF_BASE, JDL_ACC_BUFF_TOTAL);
    
    /* Category revision */
    _s_jdl_acc_rev = _JDL_SWAP_16(JDL_ACC_REV);
    _jdl_save(JDL_ACC_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_acc_rev);
    
    /* Save log settings */
    _jdl_save_word(JDL_ACC_ADR_OTHER_NUM, JDL_ACC_TICKET_NUM);
    _jdl_save_word(JDL_ACC_ADR_INFO_SIZE, JDL_ACC_DINFO_SIZE);
    
    _jdl_save_word(JDL_ACC_ADR_STAT1_BLK_SIZE, JDL_ACC_STAT1_BLK_SIZE);
    _jdl_save_word(JDL_ACC_ADR_STAT1_BLK_NUM, JDL_ACC_STAT1_BLK_NUM);
    _jdl_save_word(JDL_ACC_ADR_STAT1_DINFO_SIZE, JDL_ACC_S1_DINFO_SIZE);
    
    _jdl_save_word(JDL_ACC_ADR_STAT2_BLK_SIZE, JDL_ACC_STAT2_BLK_SIZE);
    _jdl_save_word(JDL_ACC_ADR_STAT2_BLK_NUM, JDL_ACC_STAT2_BLK_NUM);
    _jdl_save_word(JDL_ACC_ADR_STAT2_DINFO_SIZE, JDL_ACC_S2_DINFO_SIZE);
    _jdl_save_word(JDL_ACC_ADR_STAT2_REJ_SIZE, JDL_ACC_STAT2_BLK_REJ_SIZE);
    
    /* Save date */
    _jdl_save_dword(JDL_ACC_ADR_STA_DATE, time.high);
    _jdl_save_dword((JDL_ACC_ADR_STA_DATE + JDL_DATA_TYPE_SIZE_DWORD), time.low);
    
    /* Accepting Counters Check Sum */
    _jdl_renew_checksum(JDL_ACC_ADR_STA_DATE, JDL_ACC_ADR_ACC_CHECKSUM);
    
    /* Stat1 Each Block Check Sum */
    for (cnt = 0; cnt < JDL_ACC_STAT1_BLK_NUM; cnt++)
    {
        blk_base = (JDL_ACC_ADR_STAT1_BLK_BASE + (JDL_ACC_STAT1_BLK_SIZE * cnt));
        blk_sum = (blk_base + JDL_ACC_OFS_S1_BLK_CSUM);
        _jdl_renew_checksum(blk_base, blk_sum);
    }
    
    /* Stat2 Each Block Check Sum */
    for (cnt = 0; cnt < JDL_ACC_STAT2_BLK_NUM; cnt++)
    {
        blk_base = (JDL_ACC_ADR_STAT2_BLK_BASE + (JDL_ACC_STAT2_BLK_SIZE * cnt));
        blk_sum = (blk_base + JDL_ACC_OFS_S2_BLK_CSUM);
        _jdl_renew_checksum(blk_base, blk_sum);
    }
    
    /* Set number of inserted notes at the previous version */
    _jdl_save_dword(JDL_ACC_ADR_PREV_VER_INS, prev_insert);
}


/***********************************************************************/
/**
 * @brief Check data checksum. 
 * @param[in]  non
 * @return     non
 */
/***********************************************************************/
static void _jdl_acc_check_sum(void)
{
    u16 sum;
    u16 prev_sum;
    u32 blk_base;
    u32 blk_sum;
    u8 cnt;
    
    /* Accepting Counters Check Sum */
    _jdl_calc_checksum(JDL_ACC_ADR_STA_DATE, JDL_ACC_ADR_ACC_CHECKSUM, &sum);
    _jdl_load_word(JDL_ACC_ADR_ACC_CHECKSUM, &prev_sum);
    if ((prev_sum & 0x8000) || (prev_sum != sum))
    {
        /* Set corrupted flag */
        sum |= 0x8000;
        _jdl_save_word(JDL_ACC_ADR_ACC_CHECKSUM, sum);
    }
    
    /* Stat1 Each Block Check Sum */
    for (cnt = 0; cnt < JDL_ACC_STAT1_BLK_NUM; cnt++)
    {
        blk_base = (JDL_ACC_ADR_STAT1_BLK_BASE + (JDL_ACC_STAT1_BLK_SIZE * cnt));
        blk_sum = (blk_base + JDL_ACC_OFS_S1_BLK_CSUM);
        _jdl_calc_checksum(blk_base, blk_sum, &sum);
        _jdl_load_word(blk_sum, &prev_sum);
        if ((prev_sum & 0x8000) || (prev_sum != sum))
        {
            /* Set corrupted flag */
            sum |= 0x8000;
            _jdl_save_word(blk_sum, sum);
        }
    }
    
    /* Stat2 Each Block Check Sum */
    for (cnt = 0; cnt < JDL_ACC_STAT2_BLK_NUM; cnt++)
    {
        blk_base = (JDL_ACC_ADR_STAT2_BLK_BASE + (JDL_ACC_STAT2_BLK_SIZE * cnt));
        blk_sum = (blk_base + JDL_ACC_OFS_S2_BLK_CSUM);
        _jdl_calc_checksum(blk_base, blk_sum, &sum);
        _jdl_load_word(blk_sum, &prev_sum);
        if ((prev_sum & 0x8000) || (prev_sum != sum))
        {
            /* Set corrupted flag */
            sum |= 0x8000;
            _jdl_save_word(blk_sum, sum);
        }
    }
}


/***********************************************************************/
/**
 * @brief Load total insertion counter.
 * @return     error cord
 */
/***********************************************************************/
static u8 _jdl_acc_load_total_ins(void)
{
    u8 rtn = JDL_E_OK;
    u8 data[JDL_DATA_TYPE_SIZE_DWORD];
    
    /* Select counter type and load */
    switch (JDL_ACC_SIZE_ACC_CNTR)
    {
    case JDL_DATA_TYPE_SIZE_BYTE:
        /* Load counter */
        rtn = _jdl_load(JDL_ACC_ADR_TOTAL_INS, JDL_DATA_TYPE_SIZE_BYTE, &data[0]);
        if (rtn == JDL_E_OK)
        {
            _s_jdl_acc_total_ins = data[0];
        }
        break;
    case JDL_DATA_TYPE_SIZE_WORD:
        /* Load counter */
        rtn = _jdl_load_word(JDL_ACC_ADR_TOTAL_INS, (u16 *)&data[0]);
        if (rtn == JDL_E_OK)
        {
            _s_jdl_acc_total_ins = *((u16 *)&data[0]);
        }
        break;
    case JDL_DATA_TYPE_SIZE_DWORD:
        /* Load counter */
        rtn = _jdl_load_dword(JDL_ACC_ADR_TOTAL_INS, (u32 *)&data[0]);
        if (rtn == JDL_E_OK)
        {
            _s_jdl_acc_total_ins = *((u32 *)&data[0]);
        }
        break;
    default:
        /* Configuration error */
        rtn = JDL_E_CONF;
        break;
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief increment the counter.
 * @param[in]  offset  : offset address of counter to count.
 * @param[in]  type    : counter variable type
 * @param[in]  sta_adr : start address for check sum calculation
 * @param[in]  sum_adr : check sum address
 * @return     error cord
 */
/***********************************************************************/
static u8 _jdl_acc_cnt(u32 offset, u8 type, u32 sta_adr, u32 sum_adr)
{
    u8 rtn = JDL_E_OK;
    u8 data[JDL_DATA_TYPE_SIZE_DWORD];
    
    /* Select counter type and incremented */
    switch (type)
    {
    case JDL_DATA_TYPE_SIZE_BYTE:
        rtn = _jdl_inc_8bit(offset, &data[0]);
        break;
    case JDL_DATA_TYPE_SIZE_WORD:
        rtn = _jdl_inc_16bit(offset, (u16 *)&data[0]);
        break;
    case JDL_DATA_TYPE_SIZE_DWORD:
        rtn = _jdl_inc_32bit(offset, (u32 *)&data[0]);
        break;
    default:
        rtn = JDL_E_PARAM;
        break;
    }
    
    if (rtn == JDL_E_OK)
    {
        /* Calaculate checksum and Save data */
        _jdl_save_data_checksum(offset, &data[0], type, sta_adr, sum_adr);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief increment the counter of statistics1 per insertion.
 * @param[in]  dinfo_idx : denomination infomation index
 * @param[in]  offset    : offset address of log buffer
 * @param[in]  size      : counter data size
 * @return     error cord
 */
/***********************************************************************/
static u8 _jdl_acc_stat1_cnt(u16 dinfo_idx, u32 offset, u8 size)
{
    u8 rtn = JDL_E_OK;
    u32 temp;
    u32 cur_blk;
    u32 cur_blk_base;
    u32 dinfo_base;
    
    /* Search current block base */
#if 1  /* yamazaki JDL Modify */
    temp = 0;
    if (_s_jdl_acc_total_ins != 0)
    {
        temp = ((_s_jdl_acc_total_ins - 1) % (JDL_ACC_STAT1_PINS_NUM_BLK * JDL_ACC_STAT1_BLK_NUM));
    }
#else  /* yamazaki JDL Modify */
    temp = (_s_jdl_acc_total_ins % (JDL_ACC_STAT1_PINS_NUM_BLK * JDL_ACC_STAT1_BLK_NUM));
#endif /* yamazaki JDL Modify */
    cur_blk = (temp / JDL_ACC_STAT1_PINS_NUM_BLK);
    cur_blk_base = (JDL_ACC_ADR_STAT1_BLK_BASE + (JDL_ACC_STAT1_BLK_SIZE * cur_blk));
    dinfo_base = (cur_blk_base + (dinfo_idx * JDL_ACC_S1_DINFO_SIZE));
    
    if ((size == JDL_DATA_TYPE_SIZE_BYTE)
     || (size == JDL_DATA_TYPE_SIZE_WORD)
     || (size == JDL_DATA_TYPE_SIZE_DWORD))
    {
        /* Save data */
        rtn = _jdl_acc_cnt((dinfo_base + offset), size, cur_blk_base, (cur_blk_base + JDL_ACC_OFS_S1_BLK_CSUM));
    }
    else
    {
    /* Configuration error */
        rtn = JDL_E_CONF;
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief increment the counter of statistics2 per insertion.
 * @param[in]  dinfo_idx : denomination infomation index
 * @param[in]  offset    : offset address of log buffer
 * @param[in]  size      : counter data size
 * @return     error cord
 */
/***********************************************************************/
static u8 _jdl_acc_stat2_cnt(u16 dinfo_idx, u32 offset, u8 size)
{
    u8 rtn = JDL_E_OK;
    u8 rtn2 = JDL_E_OK;
    u8 inc_data1[JDL_DATA_TYPE_SIZE_DWORD];
    u8 inc_data2[JDL_DATA_TYPE_SIZE_DWORD];
    u32 temp;
    u32 cur_blk;
    u32 cur_blk_base;
    u32 dinfo_base;
    u32 conv_offset;
    
    /* Search current block base */
#if 1  /* yamazaki JDL Modify */
    temp = 0;
    if (_s_jdl_acc_total_ins != 0)
    {
        temp = ((_s_jdl_acc_total_ins - 1) % (JDL_ACC_STAT2_PINS_NUM_BLK * JDL_ACC_STAT2_BLK_NUM));
    }
#else  /* yamazaki JDL Modify */
    temp = (_s_jdl_acc_total_ins % (JDL_ACC_STAT2_PINS_NUM_BLK * JDL_ACC_STAT2_BLK_NUM));
#endif /* yamazaki JDL Modify */
    cur_blk = (temp / JDL_ACC_STAT2_PINS_NUM_BLK);
    cur_blk_base = (JDL_ACC_ADR_STAT2_BLK_BASE + (JDL_ACC_STAT2_BLK_SIZE * cur_blk));
    dinfo_base = (cur_blk_base + (dinfo_idx * JDL_ACC_S2_DINFO_SIZE));
    
     if (offset < JDL_ACC_SIZE_STAT_A_CNTR)
    {
    /* Accept */
        if ((size == JDL_DATA_TYPE_SIZE_BYTE)
            || (size == JDL_DATA_TYPE_SIZE_WORD)
            || (size == JDL_DATA_TYPE_SIZE_DWORD))
        {
            rtn = _jdl_acc_cnt(dinfo_base, size, cur_blk_base, (cur_blk_base + JDL_ACC_OFS_S2_BLK_CSUM));
        }
        else
        {
        /* Configuration error */
            rtn = JDL_E_CONF;
        }
    }
    else
    {
    /* Reject */
        /* Convet offset for statistics2 */
        conv_offset = (JDL_ACC_S2_DINFO_TOTAL + (offset - JDL_ACC_SIZE_STAT_A_CNTR));
        
        /* Select counter type and incremented */
        switch (size)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            /* Increment reject counter of each denomi */
            rtn = _jdl_inc_8bit((dinfo_base + JDL_ACC_OFS_S2_DINFO_REJ), &inc_data1[0]);
            /* Increment reject counter of all denomi */
            rtn2 = _jdl_inc_8bit((cur_blk_base + conv_offset), &inc_data2[0]);
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            /* Increment reject counter of each denomi */
            rtn = _jdl_inc_16bit((dinfo_base + JDL_ACC_OFS_S2_DINFO_REJ), (u16 *)&inc_data1[0]);
            /* Increment reject counter of all denomi */
            rtn2 = _jdl_inc_16bit((cur_blk_base + conv_offset), (u16 *)&inc_data2[0]);
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            /* Increment reject counter of each denomi */
            rtn = _jdl_inc_32bit((dinfo_base + JDL_ACC_OFS_S2_DINFO_REJ), (u32 *)&inc_data1[0]);
            /* Increment reject counter of all denomi */
            rtn2 = _jdl_inc_32bit((cur_blk_base + conv_offset), (u32 *)&inc_data2[0]);
            break;
        default:
            rtn = JDL_E_CONF;
            break;
        }
        
        /* Check rtn and rtn2 result */
        if (((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
         && ((rtn2 == JDL_E_OK) || (rtn2 == JDL_E_CMAX)))
        {
            /* Calaculate checksum and Save data */
            rtn = _jdl_save_data_2_checksum((dinfo_base + JDL_ACC_OFS_S2_DINFO_REJ), &inc_data1[0], size, (cur_blk_base + conv_offset), &inc_data2[0], size, cur_blk_base, (cur_blk_base + JDL_ACC_OFS_S2_BLK_CSUM));
        }
        else if ((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
        {
            /* Replace rtn to rtn2 */
            rtn = rtn2;
        }
    }
    
    return rtn;
}


#endif  /* _JDL_USE_ACC */


