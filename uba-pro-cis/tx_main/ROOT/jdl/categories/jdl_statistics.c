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
 * @file jdl_statistics.c
 * @brief  Statistics category of JCM Device Log
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/

#include "jdl.h"

#define EXT
#include "com_ram.c"


/************************** BACKUP VARIABLES ***************************/


/************************** PRIVATE DEFINITIONS ************************/


/************************** PRIVATE FUNCTIONS **************************/
static void _jdl_stat_clear(void);
static void _jdl_stat_check_sum(void);
static u8 _jdl_stat_cnt(u32 offset, u8 type, u32 sta_adr, u32 sum_adr);
#if defined(_JDL_STAT_USE_ACC_CNT)
static u8 _jdl_stat_load_total_ins(void);
#if defined(_JDL_STAT_USE_ERR_CNT)
static u8 _jdl_stat_per_ins(u32 offset, u8 size);
#endif /* _JDL_STAT_USE_STAT_PER_INST */
#endif /* _JDL_STAT_USE_ACC_CNT */


/************************** PRIVATE VARIABLES **************************/
static u16 _s_jdl_stat_rev;
#if defined(_JDL_STAT_USE_ACC_CNT)
static u32 _s_jdl_stat_total_ins;
#endif  /* _JDL_STAT_USE_ACC_CNT */


/************************** EXTERN FUNCTIONS ***************************/


/************************** EXTERN VARIABLES ***************************/



/***********************************************************************/
/**
 * @brief initialize statistics category
 * @param[in]  clear : force clear setting
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_init(u8 clear)
{
    u8 rtn = JDL_E_OK;
    u8 rev_ck = 0;
    
    /* Load statistics category revision */
    _jdl_load(JDL_STAT_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_stat_rev);
    
    if (_s_jdl_stat_rev != _JDL_SWAP_16(JDL_STAT_REV))
    {
    /* Change revision */
        rev_ck = 1;
        rtn = JDL_E_REVCHG;
    }
    
    if (clear != 0)
    {
    /* Clear log */
        _jdl_stat_clear();
    }
    else
    {
        if (rev_ck != 0)
        {
            /* Save statistics category revision */
            _s_jdl_stat_rev = _JDL_SWAP_16(JDL_STAT_REV);
            _jdl_save(JDL_STAT_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_stat_rev);
        }
        /* Check the checksum */
        _jdl_stat_check_sum();
    }
    
#if defined(_JDL_STAT_USE_ACC_CNT)
    /* Load total insertion */
    _jdl_stat_load_total_ins();
#endif /*_JDL_STAT_USE_ACC_CNT*/

    return rtn;
}


/***********************************************************************/
/**
 * @brief increment the counter of movements and control related. 
 * @param[in]  offset : offset of data to increment
 * @param[in]  type   : data type (size)
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_inc_mov(u32 offset, u8 type)
{
    u8 rtn = JDL_E_OK;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if ((offset < JDL_STAT_ADR_MOV_BASE) || (JDL_STAT_ADR_MOV_CHECKSUM <= offset))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        /* Increment counter */
        rtn = _jdl_stat_cnt(offset, type, JDL_STAT_ADR_MOV_BASE, JDL_STAT_ADR_MOV_CHECKSUM);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief add the value to the counter of movements and control related.
 * @param[in]  offset : offset of data to count
 * @param[in]  type   : data type (size)
 * @param[in]  val1   : value 1 to add
 * @param[in]  val2   : value 2 to add (only use 64bit data type)
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_add_mov(u32 offset, u8 type, u32 val1, u32 val2)
{
    u8 rtn = JDL_E_OK;
    u8 data[JDL_DATA_TYPE_SIZE_DWORD];
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if ((offset < JDL_STAT_ADR_MOV_BASE) || (JDL_STAT_ADR_MOV_CHECKSUM <= offset))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        /* Sellect data type and add data */
        switch (type)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            rtn = _jdl_add_8bit(offset, (u8)(0x000000FF & val1), &data[0]);
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            rtn = _jdl_add_16bit(offset, (u16)(0x0000FFFF & val1), (u16 *)&data[0]);
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            rtn = _jdl_add_32bit(offset, val1, (u32 *)&data[0]);
            break;
        //case JDL_DATA_TYPE_QWORD:
        //    rtn = _jdl_add_64bit(offset, val1, val2, &data[03]);
        //    break;
        default:
            rtn = JDL_E_PARAM;
            break;
        }
        if ((rtn == JDL_E_OK) || (rtn == JDL_E_COVR))
        {
            /* Calaculate checksum and Save data */
            _jdl_save_data_checksum(offset, &data[0], type, JDL_STAT_ADR_MOV_BASE, JDL_STAT_ADR_MOV_CHECKSUM);
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief get the value from the counter of movements and control related.
 * @param[in]  offset : offset of counter
 * @param[in]  type   : data type (size)
 * @param[out] val1   : value 1 to add
 * @param[out] val2   : value 2 to add (only use 64bit data type)
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_get_mov(u32 offset, u8 type, u32 *val, u32 *val2)
{
    u8 rtn = JDL_E_OK;
    u8 data[JDL_DATA_TYPE_SIZE_DWORD];
    
    if ((offset < JDL_STAT_ADR_MOV_BASE) || (JDL_STAT_ADR_MOV_CHECKSUM <= offset))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        /* Sellect data type and add data */
        switch (type)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            /* Load counter */
            rtn = _jdl_load(offset, JDL_DATA_TYPE_SIZE_BYTE, &data[0]);
            if (rtn == JDL_E_OK)
            {
                *val = data[0];
            }
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            /* Load counter */
            rtn = _jdl_load_word(offset, (u16 *)&data[0]);
            if (rtn == JDL_E_OK)
            {
                *val = *((u16 *)&data[0]);
            }
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            /* Load counter */
            rtn = _jdl_load_dword(offset, (u32 *)&data[0]);
            if (rtn == JDL_E_OK)
            {
                *val = *((u32 *)&data[0]);
            }
            break;
        //case JDL_DATA_TYPE_QWORD:
        //    break;
        default:
            rtn = JDL_E_PARAM;
            break;
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief add the value to motor movements counter and movements time.
 * @param[in]  offset   : offset of data to count
 * @param[in]  cnt_type : counter Data type (size)
 * @param[in]  tim_type : time Data type (size)
 * @param[in]  add_tim1 : time 1 to add
 * @param[in]  add_tim2 : time 2 to add (only use 64bit data type)
 * @return     error cord
 */
/***********************************************************************/
#if 0 //動作時間は廃止、変わりに空いた箇所にshutterとstackerの動作回数を追加
u8 _jdl_stat_motor(u32 offset, u8 cnt_type, u8 tim_type, u32 add_tim1, u32 add_tim2)
{
    u8 rtn = JDL_E_OK;
    u8 rtn2 = JDL_E_OK;
    u32 tim_offset;
    u8 data[(JDL_DATA_TYPE_SIZE_DWORD * 2)];
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if ((offset < JDL_STAT_ADR_MOV_BASE) || (JDL_STAT_ADR_MOV_CHECKSUM <= offset))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    //else if ((cnt_type > JDL_DATA_TYPE_QWORD)
    //      || (tim_type > JDL_DATA_TYPE_QWORD))
    else if ((cnt_type > JDL_DATA_TYPE_SIZE_DWORD)
          || (tim_type > JDL_DATA_TYPE_SIZE_DWORD))
    {
    /* Data type error */
        rtn = JDL_E_PARAM;
    }
    else
    {
        /* Select counter type and incremented */
        switch (cnt_type)
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
        //    rtn = _jdl_inc_64bit(offset, (u32 *)&data[0], (u32 *)&data[JDL_DATA_TYPE_SIZE_DWORD]);
            rtn = JDL_E_PARAM;
            break;
        }
        
        tim_offset = (offset + cnt_type);
        /* Sellect time type and add time */
        switch (tim_type)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            rtn2 = _jdl_add_8bit(tim_offset, (u8)(0x000000FF & add_tim1), &data[cnt_type]);
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            rtn2 = _jdl_add_16bit(tim_offset, (u16)(0x0000FFFF & add_tim1), (u16 *)&data[cnt_type]);
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            rtn2 = _jdl_add_32bit(tim_offset, add_tim1, (u32 *)&data[cnt_type]);
            break;
        default:
        //    rtn2 = _jdl_add_64bit(tim_offset, add_tim1, add_tim2, (u32 *)&data[cnt_type], (u32 *)&data[(cnt_type + JDL_DATA_TYPE_SIZE_DWORD)]);
            rtn2 = JDL_E_PARAM;
            break;
        }
        
        /* Check rtn and rtn2 result */
        if (((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
         && ((rtn2 == JDL_E_OK) || (rtn2 == JDL_E_CMAX) || (rtn2 == JDL_E_COVR)))
        {
            /* Calaculate checksum and Save data */
            _jdl_save_data_checksum(offset, &data[0], (cnt_type + tim_type), JDL_STAT_ADR_MOV_BASE, JDL_STAT_ADR_MOV_CHECKSUM);
        }
        else if ((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
        {
            /* Replace rtn to rtn2 */
            rtn = rtn2;
        }
    }
    
    return rtn;
}
#endif

u8 _jdl_stat_motor_new(u32 offset, u8 cnt_type, u8 tim_type, u32 add_tim1, u32 add_tim2) //NEW_JDL
{
    u8 rtn = JDL_E_OK;
    u8 rtn2 = JDL_E_OK;
    u32 tim_offset;
    u8 data[(JDL_DATA_TYPE_SIZE_DWORD * 2)];
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if ((offset < JDL_STAT_ADR_MOV_BASE) || (JDL_STAT_ADR_MOV_CHECKSUM <= offset))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    //else if ((cnt_type > JDL_DATA_TYPE_QWORD)
    //      || (tim_type > JDL_DATA_TYPE_QWORD))
    else if ((cnt_type > JDL_DATA_TYPE_SIZE_DWORD)
        //2025-10-02  || (tim_type > JDL_DATA_TYPE_SIZE_DWORD) //時間使用しない
        )
    {
    /* Data type error */
        rtn = JDL_E_PARAM;
    }
    else
    {
        /* Select counter type and incremented */
        switch (cnt_type)
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
        //    rtn = _jdl_inc_64bit(offset, (u32 *)&data[0], (u32 *)&data[JDL_DATA_TYPE_SIZE_DWORD]);
            rtn = JDL_E_PARAM;
            break;
        }
    #if 0     //2025-10-02 //時間使用しない
        tim_offset = (offset + cnt_type);
        /* Sellect time type and add time */
        switch (tim_type)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            rtn2 = _jdl_add_8bit(tim_offset, (u8)(0x000000FF & add_tim1), &data[cnt_type]);
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            rtn2 = _jdl_add_16bit(tim_offset, (u16)(0x0000FFFF & add_tim1), (u16 *)&data[cnt_type]);
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            rtn2 = _jdl_add_32bit(tim_offset, add_tim1, (u32 *)&data[cnt_type]);
            break;
        default:
        //    rtn2 = _jdl_add_64bit(tim_offset, add_tim1, add_tim2, (u32 *)&data[cnt_type], (u32 *)&data[(cnt_type + JDL_DATA_TYPE_SIZE_DWORD)]);
            rtn2 = JDL_E_PARAM;
            break;
        }
    #endif    
        /* Check rtn and rtn2 result */
        if (((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
        //2025-10-02  && ((rtn2 == JDL_E_OK) || (rtn2 == JDL_E_CMAX) || (rtn2 == JDL_E_COVR))
        )
        {
            /* Calaculate checksum and Save data */
            _jdl_save_data_checksum(offset, &data[0], (cnt_type + tim_type), JDL_STAT_ADR_MOV_BASE, JDL_STAT_ADR_MOV_CHECKSUM);
        }
    #if 0 //2025-10-02    
        else if ((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
        {
            /* Replace rtn to rtn2 */
            rtn = rtn2;
        }
    #endif        
    }
    
    return rtn;
}


#if defined(_JDL_STAT_USE_ACC_CNT)
/***********************************************************************/
/**
 * @brief Increment total insertion counter.
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_insert(void) //紙幣取り込み開始
{
    u8 rtn = JDL_E_OK;
    u8 data[JDL_DATA_TYPE_SIZE_DWORD];
    u32 total_ins;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
        /* Select counter type and incremented */
        switch (JDL_STAT_SIZE_ACC_CNTR)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            rtn = _jdl_inc_8bit(JDL_STAT_ADR_ACC_INST, &data[0]);
            total_ins = data[0];
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            rtn = _jdl_inc_16bit(JDL_STAT_ADR_ACC_INST, (u16 *)&data[0]);
            total_ins = _JDL_SWAP_16(*((u16 *)&data[0]));
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            rtn = _jdl_inc_32bit(JDL_STAT_ADR_ACC_INST, (u32 *)&data[0]);
            total_ins = _JDL_SWAP_32(*((u32 *)&data[0]));
            break;
        default:
            /* Configuration error */
            rtn = JDL_E_CONF;
            break;
        }
        
        /* Update counter */
        if (rtn == JDL_E_OK)
        {
            /* Calaculate checksum and Save counter */
            _jdl_save_data_checksum(JDL_STAT_ADR_ACC_INST, &data[0], JDL_STAT_SIZE_ACC_CNTR, JDL_STAT_ADR_ACC_BASE, JDL_STAT_ADR_ACC_CHECKSUM);
            /* Update private variables */
            _s_jdl_stat_total_ins = total_ins;
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Increment bill accepted counter.
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_bill_accept(void)
{
    u8 rtn = JDL_E_OK;
    u8 rtn2 = JDL_E_OK;
    u8 data[(JDL_DATA_TYPE_SIZE_DWORD * 2)];
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
        /* Select counter type and incremented */
        switch (JDL_STAT_SIZE_ACC_CNTR)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            /* Increment bill insertion counter */
            rtn = _jdl_inc_8bit(JDL_STAT_ADR_ACC_BILL_INST, &data[0]);
            /* Increment bill accepted counter */
            rtn2 = _jdl_inc_8bit(JDL_STAT_ADR_ACC_BIIL_ACCT, &data[JDL_DATA_TYPE_SIZE_BYTE]);
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            /* Increment bill insertion counter */
            rtn = _jdl_inc_16bit(JDL_STAT_ADR_ACC_BILL_INST, (u16 *)&data[0]);
            /* Increment bill accepted counter */
            rtn2 = _jdl_inc_16bit(JDL_STAT_ADR_ACC_BIIL_ACCT, (u16 *)&data[JDL_DATA_TYPE_SIZE_WORD]);
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            /* Increment bill insertion counter */
            rtn = _jdl_inc_32bit(JDL_STAT_ADR_ACC_BILL_INST, (u32 *)&data[0]);
            /* Increment bill accepted counter */
            rtn2 = _jdl_inc_32bit(JDL_STAT_ADR_ACC_BIIL_ACCT, (u32 *)&data[JDL_DATA_TYPE_SIZE_DWORD]);
            break;
        default:
            /* Configuration error */
            rtn = JDL_E_CONF;
            break;
        }
        
        /* Check rtn and rtn2 result */
        if (((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
         && ((rtn2 == JDL_E_OK) || (rtn2 == JDL_E_CMAX)))
        {
            /* Calaculate checksum and Save data */
            rtn = _jdl_save_data_checksum(JDL_STAT_ADR_ACC_BILL_INST, &data[0], (JDL_STAT_SIZE_ACC_CNTR * 2), JDL_STAT_ADR_ACC_BASE, JDL_STAT_ADR_ACC_CHECKSUM);
#if defined(_JDL_STAT_USE_STAT_PER_INST)
            if (rtn == JDL_E_OK)
            {
                /* Statistics 1 */
                rtn = _jdl_stat_per_ins(JDL_STAT_OFS_STAT_ACCEPT, JDL_STAT_SIZE_STAT_A_CNTR);
            }
#endif /* _JDL_STAT_USE_STAT_PER_INST */
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
 * @brief Increment ticket accepted counter.
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_ticket_accept(void)
{
    u8 rtn = JDL_E_OK;
    u8 rtn2 = JDL_E_OK;
    u8 data[(JDL_DATA_TYPE_SIZE_DWORD * 2)];
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
        /* Select counter type and incremented */
        switch (JDL_STAT_SIZE_ACC_CNTR)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            /* Increment ticket insertion counter */
            rtn = _jdl_inc_8bit(JDL_STAT_ADR_ACC_TICK_INST, &data[0]);
            /* Increment ticket accepted counter */
            rtn2 = _jdl_inc_8bit(JDL_STAT_ADR_ACC_TICK_ACCT, &data[JDL_DATA_TYPE_SIZE_BYTE]);
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            /* Increment ticket insertion counter */
            rtn = _jdl_inc_16bit(JDL_STAT_ADR_ACC_TICK_INST, (u16 *)&data[0]);
            /* Increment ticket accepted counter */
            rtn2 = _jdl_inc_16bit(JDL_STAT_ADR_ACC_TICK_ACCT, (u16 *)&data[JDL_DATA_TYPE_SIZE_WORD]);
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            /* Increment ticket insertion counter */
            rtn = _jdl_inc_32bit(JDL_STAT_ADR_ACC_TICK_INST, (u32 *)&data[0]);
            /* Increment ticket accepted counter */
            rtn2 = _jdl_inc_32bit(JDL_STAT_ADR_ACC_TICK_ACCT, (u32 *)&data[JDL_DATA_TYPE_SIZE_DWORD]);
            break;
        default:
            /* Configuration error */
            rtn = JDL_E_CONF;
            break;
        }
        
        /* Check rtn and rtn2 result */
        if (((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
         && ((rtn2 == JDL_E_OK) || (rtn2 == JDL_E_CMAX)))
        {
            /* Calaculate checksum and Save data */
            rtn = _jdl_save_data_checksum(JDL_STAT_ADR_ACC_TICK_INST, &data[0], (JDL_STAT_SIZE_ACC_CNTR * 2), JDL_STAT_ADR_ACC_BASE, JDL_STAT_ADR_ACC_CHECKSUM);
#if defined(_JDL_STAT_USE_STAT_PER_INST)
            if (rtn == JDL_E_OK)
            {
                /* Statistics 1 */
                rtn = _jdl_stat_per_ins(JDL_STAT_OFS_STAT_ACC_BAR, JDL_STAT_SIZE_STAT_A_CNTR);
            }
#endif /* _JDL_STAT_USE_STAT_PER_INST */
        }
        else if ((rtn == JDL_E_OK) || (rtn == JDL_E_CMAX))
        {
            /* Replace rtn to rtn2 */
            rtn = rtn2;
        }
    }
    
    return rtn;
}
#endif  /* _JDL_STAT_USE_ACC_CNT */


#if defined(_JDL_STAT_USE_RC_CNT)
/***********************************************************************/
/**
 * @brief increment the counter of rc movements. 
 * @param[in]  offset : offset of data to increment
 * @param[in]  type   : data type (size)
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_rc(u32 offset, u8 type)
{
    u8 rtn = JDL_E_OK;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if ((offset < JDL_STAT_ADR_RC_BASE) || (JDL_STAT_ADR_RC_CHECKSUM <= offset))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        /* Increment counter */
        rtn = _jdl_stat_cnt(offset, type, JDL_STAT_ADR_RC_BASE, JDL_STAT_ADR_RC_CHECKSUM);
    }
    
    return rtn;
}
#endif /* _JDL_STAT_USE_RC_CNT */


#if defined(_JDL_STAT_USE_REJ_CNT)
/***********************************************************************/
/**
 * @brief Increment bill rejected counter.
 * @param[in]  rej_offset  : offset of data to count
 * @param[in]  stat_offset : offset of data to count per insertion
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_bill_reject(u32 rej_offset, u32 stat_offset)
{
    u8 rtn = JDL_E_OK;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if (((rej_offset < JDL_STAT_ADR_REJ_BASE) || (JDL_STAT_ADR_REJ_CHECKSUM <= rej_offset))
          || ((stat_offset < JDL_STAT_SIZE_STAT_A_CNTR) || (JDL_STAT_OFS_STAT_CHECKSUM <= stat_offset)))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        /* Bill insertion counter */
        if ((JDL_STAT_SIZE_ACC_CNTR == JDL_DATA_TYPE_SIZE_BYTE)
         || (JDL_STAT_SIZE_ACC_CNTR == JDL_DATA_TYPE_SIZE_WORD)
         || (JDL_STAT_SIZE_ACC_CNTR == JDL_DATA_TYPE_SIZE_DWORD))
        {
            /* Increment counter */
            rtn = _jdl_stat_cnt(JDL_STAT_ADR_ACC_BILL_INST, JDL_STAT_SIZE_ACC_CNTR, JDL_STAT_ADR_ACC_BASE, JDL_STAT_ADR_ACC_CHECKSUM);
        }
        else
        {
        /* Configuration error */
            rtn = JDL_E_CONF;
        }
        
        if (rtn == JDL_E_OK)
        {
            /* Bill rejected counter */
            if ((JDL_STAT_SIZE_REJ_CNTR == JDL_DATA_TYPE_SIZE_BYTE)
                || (JDL_STAT_SIZE_REJ_CNTR == JDL_DATA_TYPE_SIZE_WORD)
                || (JDL_STAT_SIZE_REJ_CNTR == JDL_DATA_TYPE_SIZE_DWORD))
            {
                /* Increment counter */
                rtn = _jdl_stat_cnt(rej_offset, JDL_STAT_SIZE_REJ_CNTR, JDL_STAT_ADR_REJ_BASE, JDL_STAT_ADR_REJ_CHECKSUM);
            }
            else
            {
            /* Configuration error */
                rtn = JDL_E_CONF;
            }
        }
        
#if defined(_JDL_STAT_USE_STAT_PER_INST)
        if (rtn == JDL_E_OK)
        {
            /* Statistics 1 */
            rtn = _jdl_stat_per_ins(stat_offset, JDL_STAT_SIZE_STAT_R_CNTR);
        }
#endif /* _JDL_STAT_USE_STAT_PER_INST */
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Increment ticket rejected counter.
 * @param[in]  rej_offset  : offset of data to count
 * @param[in]  stat_offset : offset of data to count per insertion
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_ticket_reject(u32 rej_offset, u32 stat_offset)
{
    u8 rtn = JDL_E_OK;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if (((rej_offset < JDL_STAT_ADR_REJ_BASE) || (JDL_STAT_ADR_REJ_CHECKSUM <= rej_offset))
          || ((stat_offset < JDL_STAT_SIZE_STAT_A_CNTR) || (JDL_STAT_OFS_STAT_CHECKSUM <= stat_offset)))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        /* Ticket insertion counter */
        if ((JDL_STAT_SIZE_ACC_CNTR == JDL_DATA_TYPE_SIZE_BYTE)
         || (JDL_STAT_SIZE_ACC_CNTR == JDL_DATA_TYPE_SIZE_WORD)
         || (JDL_STAT_SIZE_ACC_CNTR == JDL_DATA_TYPE_SIZE_DWORD))
        {
            /* Increment counter */
            rtn = _jdl_stat_cnt(JDL_STAT_ADR_ACC_TICK_INST, JDL_STAT_SIZE_ACC_CNTR, JDL_STAT_ADR_ACC_BASE, JDL_STAT_ADR_ACC_CHECKSUM);
        }
        else
        {
        /* Configuration error */
            rtn = JDL_E_CONF;
        }
        
        if (rtn == JDL_E_OK)
        {
            /* Ticket rejected counter */
            if ((JDL_STAT_SIZE_REJ_CNTR == JDL_DATA_TYPE_SIZE_BYTE)
                || (JDL_STAT_SIZE_REJ_CNTR == JDL_DATA_TYPE_SIZE_WORD)
                || (JDL_STAT_SIZE_REJ_CNTR == JDL_DATA_TYPE_SIZE_DWORD))
            {
                /* Increment counter */
                rtn = _jdl_stat_cnt(rej_offset, JDL_STAT_SIZE_REJ_CNTR, JDL_STAT_ADR_REJ_BASE, JDL_STAT_ADR_REJ_CHECKSUM);
            }
            else
            {
            /* Configuration error */
                rtn = JDL_E_CONF;
            }
        }
#if defined(_JDL_STAT_USE_STAT_PER_INST)
        if (rtn == JDL_E_OK)
        {
            /* Statistics 1 */
            rtn = _jdl_stat_per_ins(stat_offset, JDL_STAT_SIZE_STAT_R_CNTR);
        }
#endif /* _JDL_STAT_USE_STAT_PER_INST */
    }
    
    return rtn;
}
#endif  /* _JDL_STAT_USE_REJ_CNT */


#if defined(_JDL_STAT_USE_ERR_CNT)
/***********************************************************************/
/**
 * @brief increment the error counter.
 * @param[in]  err_offset  : offset of data to count
 * @param[in]  stat_offset : offset of data to count per insertion
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_err(u32 err_offset, u32 stat_offset)
{
    u8 rtn = JDL_E_OK;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if (((err_offset < JDL_STAT_ADR_ERR_BASE) || (JDL_STAT_ADR_ERR_CHECKSUM <= err_offset))
          || ((stat_offset < JDL_STAT_SIZE_STAT_A_CNTR) || (JDL_STAT_OFS_STAT_CHECKSUM <= stat_offset)))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        /* Error counter */
        if ((JDL_STAT_SIZE_ERR_CNTR == JDL_DATA_TYPE_SIZE_BYTE)
         || (JDL_STAT_SIZE_ERR_CNTR == JDL_DATA_TYPE_SIZE_WORD)
         || (JDL_STAT_SIZE_ERR_CNTR == JDL_DATA_TYPE_SIZE_DWORD))
        {
            /* Increment counter */
            rtn = _jdl_stat_cnt(err_offset, JDL_STAT_SIZE_ERR_CNTR, JDL_STAT_ADR_ERR_BASE, JDL_STAT_ADR_ERR_CHECKSUM);
        }
        else
        {
        /* Configuration error */
            rtn = JDL_E_CONF;
        }
        
#if defined(_JDL_STAT_USE_STAT_PER_INST)
        if (rtn == JDL_E_OK)
        {
            /* Statistics 1 */
            rtn = _jdl_stat_per_ins(stat_offset, JDL_STAT_SIZE_ERR_CNTR);
        }
#endif /* _JDL_STAT_USE_STAT_PER_INST */
    }
    
    return rtn;

}
#endif  /* _JDL_STAT_USE_ERR_CNT */



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
u8 _jdl_stat_req(u32 s_offset, u32 buff_size)
{
    u8 rtn;

	if (buff_size < 1)
    {
    /* Size error */
        rtn = JDL_E_PARAM;
    }
    else if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_STAT_SEND_TOTAL))
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
 * @brief get the data from log buffer of system category 
 * @param[out] buff      : pointer to the buffer to get
 * @param[in]  buff_size : buffer size
 * @param[in]  s_offset  : offset address of send data for each category
 * @param[out] g_size    : size of data gotten
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_get(u8 *buff, u32 buff_size, u32 s_offset, u32 *g_size)
{
    u8 rtn = JDL_E_OK;
    u8 category_header[JDL_SIZE_CATEGORY_HEADER];
    u32 w_size;
    u32 w_offset;
    u32 temp_wsize;
    u32 temp_offset;
    
    if (buff_size < 1)
    {
    /* Size error */
        rtn = JDL_E_PARAM;
    }
    else if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_STAT_SEND_TOTAL))
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
            _jdl_memcpy((void *)&category_header[JDL_CHEAD_ADR_NAME], "STATISTICS", sizeof("STATISTICS"));
            *((u16 *)&category_header[JDL_CHEAD_ADR_ID]) = _JDL_SWAP_16(JDL_CATE_ID_STATISTICS);
            *((u16 *)&category_header[JDL_CHEAD_ADR_REV]) = _s_jdl_stat_rev;
            *((u32 *)&category_header[JDL_CHEAD_ADR_CSIZE]) = _JDL_SWAP_32(((u32)JDL_SIZE_CATEGORY_HEADER + (u32)JDL_STAT_SEND_TOTAL));
            
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
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_STAT_SEND_TOTAL)))
        {
        /* Log Datas Area */
            temp_offset = (w_offset - JDL_SIZE_CATEGORY_HEADER);
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_STAT_SEND_TOTAL)
            {
            /* Log Datas or less */
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_STAT_SEND_TOTAL - temp_offset);
            }
            
            /* Add Category base address */
            temp_offset += JDL_STAT_ADR_SEND_BASE;
            _jdl_load(temp_offset, temp_wsize, (buff + w_size));
            w_size += temp_wsize;
            w_offset += temp_wsize;
        }
        
        *g_size = w_size;
    }
    
    return rtn;
}


#if 0
/***********************************************************************/
/**
 * @brief add the value to the counter of movements and control related.
 * @param[in]  offset : offset of data to count
 * @param[in]  type   : data type ( size )
 * @param[in]  val1   : value 1 to add
 * @param[in]  val2   : value 2 to add (only use 64bit data type)
 * @param[in]  sta_adr : start address for check sum calculation
 * @param[in]  sum_adr : check sum address
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_stat_add(u32 offset, u8 type, u32 val1, u32 val2, u32 sta_adr, u32 sum_adr)
{
    u8 rtn = JDL_E_OK;
    u8 data[JDL_DATA_TYPE_SIZE_DWORD];
    
    /* Sellect data type and add data */
    switch (type)
    {
    case JDL_DATA_TYPE_SIZE_BYTE:
        rtn = _jdl_add_8bit(offset, (u8)(0x000000FF & val1), &data[0]);
        break;
    case JDL_DATA_TYPE_SIZE_WORD:
        rtn = _jdl_add_16bit(offset, (u16)(0x0000FFFF & val1), (u16 *)&data[0]);
        break;
    case JDL_DATA_TYPE_SIZE_DWORD:
        rtn = _jdl_add_32bit(offset, val1, (u32 *)&data[0]);
        break;
    //case JDL_DATA_TYPE_QWORD:
    //    rtn = _jdl_add_64bit(offset, val1, val2, (u32 *)&data[0]);
    //    break;
    default:
        rtn = JDL_E_PARAM;
        break;
    }
    
    if ((rtn == JDL_E_OK) || (rtn == JDL_E_COVR))
    {
        /* Calaculate checksum and Save data */
        _jdl_save_data_checksum(offset, &data[0], type, sta_adr, sum_adr);
    }
    
    return rtn;
}
#endif


/***********************************************************************/
/**
 * @brief Clear statistics category. 
 * @param[in]  non
 * @return     non
 */
/***********************************************************************/
static void _jdl_stat_clear(void)
{
#if defined(_JDL_STAT_USE_STAT_PER_INST)
    u32 blk_base;
    u32 blk_sum;
    u8 cnt;
#endif /* _JDL_STAT_USE_STAT_PER_INST */
    
    /* Clear Buffer */
    _jdl_clear(JDL_STAT_ADR_BUFF_BASE, JDL_STAT_BUFF_TOTAL);
    
    /* Category revision */
    _s_jdl_stat_rev = _JDL_SWAP_16(JDL_STAT_REV);
    _jdl_save(JDL_STAT_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_stat_rev);
    
    /* Movement Counters Check Sum */
    _jdl_renew_checksum(JDL_STAT_ADR_MOV_BASE, JDL_STAT_ADR_MOV_CHECKSUM);
    
#if defined(_JDL_STAT_USE_ACC_CNT)
    /* Accepting Counters Check Sum */
    _jdl_renew_checksum(JDL_STAT_ADR_ACC_BASE, JDL_STAT_ADR_ACC_CHECKSUM);
#endif /* _JDL_STAT_USE_ACC_CNT */
#if defined(_JDL_STAT_USE_RC_CNT)
    /* RC Counters Check Sum */
    _jdl_renew_checksum(JDL_STAT_ADR_RC_BASE, JDL_STAT_ADR_RC_CHECKSUM);
#endif /* _JDL_STAT_USE_RC_CNT */
#if defined(_JDL_STAT_USE_REJ_CNT)
    /* Rejecting Counters Check Sum */
    _jdl_renew_checksum(JDL_STAT_ADR_REJ_BASE, JDL_STAT_ADR_REJ_CHECKSUM);
#endif /* _JDL_STAT_USE_REJ_CNT */
#if defined(_JDL_STAT_USE_REJ_CNT)
    /* Error Counters Check Sum */
    _jdl_renew_checksum(JDL_STAT_ADR_ERR_BASE, JDL_STAT_ADR_ERR_CHECKSUM);
#endif /* _JDL_STAT_USE_REJ_CNT */
#if defined(_JDL_STAT_USE_STAT_PER_INST)
    /* Set Counters settings */
    _jdl_save_word(JDL_STAT_ADR_STAT_BLK_SIZE, JDL_STAT_PER_INST_BLK_SIZE);
    _jdl_save_word(JDL_STAT_ADR_STAT_BLK_NUM, JDL_STAT_PER_INST_BLK_NUM);
    
    /* Each Block Check Sum */
    for (cnt = 0; cnt < JDL_STAT_PER_INST_BLK_NUM; cnt++)
    {
        blk_base = (JDL_STAT_ADR_STAT_BASE + (JDL_STAT_PER_INST_BLK_SIZE * cnt));
        blk_sum = (blk_base + JDL_STAT_OFS_STAT_CHECKSUM);
        _jdl_renew_checksum(blk_base, blk_sum);
    }
#endif /* _JDL_STAT_USE_STAT_PER_INST */
}


/***********************************************************************/
/**
 * @brief Check data checksum. 
 * @param[in]  non
 * @return     non
 */
/***********************************************************************/
static void _jdl_stat_check_sum(void)
{
    u16 sum;
    u16 prev_sum;
#if defined(_JDL_STAT_USE_STAT_PER_INST)
    u32 blk_base;
    u32 blk_sum;
    u16 cnt;
#endif /* _JDL_STAT_USE_STAT_PER_INST */
    
    /* Movement Counters Check Sum */
    _jdl_calc_checksum(JDL_STAT_ADR_MOV_BASE, JDL_STAT_ADR_MOV_CHECKSUM, &sum);
    _jdl_load_word(JDL_STAT_ADR_MOV_CHECKSUM, &prev_sum);
	#if 1	/* '22-08-23 */
	jdl_logdat_chksum = (u32)sum;
	#endif
	if ((prev_sum & 0x8000) || (prev_sum != sum))
    {
        /* Set corrupted flag */
        sum |= 0x8000;
        _jdl_save_word(JDL_STAT_ADR_MOV_CHECKSUM, sum);
    }
    
#if defined(_JDL_STAT_USE_ACC_CNT)
    /* Accepting Counters Check Sum */
    _jdl_calc_checksum(JDL_STAT_ADR_ACC_BASE, JDL_STAT_ADR_ACC_CHECKSUM, &sum);
    _jdl_load_word(JDL_STAT_ADR_ACC_CHECKSUM, &prev_sum);
    if ((prev_sum & 0x8000) || (prev_sum != sum))
    {
        /* Set corrupted flag */
        sum |= 0x8000;
        _jdl_save_word(JDL_STAT_ADR_ACC_CHECKSUM, sum);
    }
#endif /* _JDL_STAT_USE_ACC_CNT */
#if defined(_JDL_STAT_USE_RC_CNT)
    /* RC Counters Check Sum */
    _jdl_calc_checksum(JDL_STAT_ADR_RC_BASE, JDL_STAT_ADR_RC_CHECKSUM, &sum);
    _jdl_load_word(JDL_STAT_ADR_RC_CHECKSUM, &prev_sum);
    if ((prev_sum & 0x8000) || (prev_sum != sum))
    {
        /* Set corrupted flag */
        sum |= 0x8000;
        _jdl_save_word(JDL_STAT_ADR_RC_CHECKSUM, sum);
    }
#endif /* _JDL_STAT_USE_RC_CNT */
#if defined(_JDL_STAT_USE_REJ_CNT)
    /* Rejecting Counters Check Sum */
    _jdl_calc_checksum(JDL_STAT_ADR_REJ_BASE, JDL_STAT_ADR_REJ_CHECKSUM, &sum);
    _jdl_load_word(JDL_STAT_ADR_REJ_CHECKSUM, &prev_sum);
    if ((prev_sum & 0x8000) || (prev_sum != sum))
    {
        /* Set corrupted flag */
        sum |= 0x8000;
        _jdl_save_word(JDL_STAT_ADR_REJ_CHECKSUM, sum);
    }
#endif /* _JDL_STAT_USE_REJ_CNT */
#if defined(_JDL_STAT_USE_REJ_CNT)
    /* Error Counters Check Sum */
    _jdl_calc_checksum(JDL_STAT_ADR_ERR_BASE, JDL_STAT_ADR_ERR_CHECKSUM, &sum);
    _jdl_load_word(JDL_STAT_ADR_ERR_CHECKSUM, &prev_sum);
    if ((prev_sum & 0x8000) || (prev_sum != sum))
    {
        /* Set corrupted flag */
        sum |= 0x8000;
        _jdl_save_word(JDL_STAT_ADR_ERR_CHECKSUM, sum);
    }
#endif /* _JDL_STAT_USE_REJ_CNT */
    
#if defined(_JDL_STAT_USE_STAT_PER_INST)
    /* Statistics per Insertion Settings */
    _jdl_save_word(JDL_STAT_ADR_STAT_BLK_SIZE, JDL_STAT_PER_INST_BLK_SIZE);
    _jdl_save_word(JDL_STAT_ADR_STAT_BLK_NUM, JDL_STAT_PER_INST_BLK_NUM);
    /* Each Block Check Sum */
    for (cnt = 0; cnt < JDL_STAT_PER_INST_BLK_NUM; cnt++)
    {
        blk_base = (JDL_STAT_ADR_STAT_BASE + (JDL_STAT_PER_INST_BLK_SIZE * cnt));
        blk_sum = (blk_base + JDL_STAT_OFS_STAT_CHECKSUM);
        _jdl_calc_checksum(blk_base, blk_sum, &sum);
        _jdl_load_word(blk_sum, &prev_sum);
        if ((prev_sum & 0x8000) || (prev_sum != sum))
        {
            /* Set corrupted flag */
            sum |= 0x8000;
            _jdl_save_word(blk_sum, sum);
        }
    }
#endif /* _JDL_STAT_USE_STAT_PER_INST */
}


/***********************************************************************/
/**
 * @brief count the counter of statistics category 
 * @param[in]  offset  : offset address of counter to count.
 * @param[in]  type    : counter variable type (size)
 * @param[in]  sta_adr : start address for check sum calculation
 * @param[in]  sum_adr : check sum address
 * @return     error cord
 */
/***********************************************************************/
static u8 _jdl_stat_cnt(u32 offset, u8 type, u32 sta_adr, u32 sum_adr)
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
    //case JDL_DATA_TYPE_SIZE_QWORD:
    //    rtn = _jdl_inc_64bit(offset, (u32 *)&data[0], (u32 *)&data[JDL_DATA_TYPE_SIZE_DWORD]);
    //    break;
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


#if defined(_JDL_STAT_USE_ACC_CNT)
/***********************************************************************/
/**
 * @brief Load total insertion counter.
 * @return     error cord
 */
/***********************************************************************/
static u8 _jdl_stat_load_total_ins(void) //起動時に現在の枚数取得用
{
    u8 rtn = JDL_E_OK;
    u8 data[JDL_DATA_TYPE_SIZE_DWORD];
    
    /* Select counter type and load */
    switch (JDL_STAT_SIZE_ACC_CNTR)
    {
    case JDL_DATA_TYPE_SIZE_BYTE:
        /* Load counter */
        rtn = _jdl_load(JDL_STAT_ADR_ACC_INST, JDL_DATA_TYPE_SIZE_BYTE, &data[0]);
        if (rtn == JDL_E_OK)
        {
            _s_jdl_stat_total_ins = data[0];
        }
        break;
    case JDL_DATA_TYPE_SIZE_WORD:
        /* Load counter */
        rtn = _jdl_load_word(JDL_STAT_ADR_ACC_INST, (u16 *)&data[0]);
        if (rtn == JDL_E_OK)
        {
            _s_jdl_stat_total_ins = *((u16 *)&data[0]);
        }
        break;
    case JDL_DATA_TYPE_SIZE_DWORD:
        /* Load counter */
        rtn = _jdl_load_dword(JDL_STAT_ADR_ACC_INST, (u32 *)&data[0]);
        if (rtn == JDL_E_OK)
        {
            _s_jdl_stat_total_ins = *((u32 *)&data[0]);
        }
        break;
    default:
        /* Configuration error */
        rtn = JDL_E_CONF;
        break;
    }
    
    return rtn;
}
#endif /* _JDL_STAT_USE_ACC_CNT */

#if defined(_JDL_STAT_USE_ERR_CNT) && defined(_JDL_STAT_USE_ACC_CNT)
/***********************************************************************/
/**
 * @brief Set statistics category
 * @param[in]  offset : offset of data to count
 * @param[in]  type   : data type ( size )
 * @return     error cord
 */
/***********************************************************************/
static u8 _jdl_stat_per_ins(u32 offset, u8 size)
{
    u8 rtn = JDL_E_OK;
    u32 cur_blk;
    u32 cur_blk_base;
    u32 temp;
    
    /* Search current block base */
    temp = 0;
    if (_s_jdl_stat_total_ins != 0)
    {
        temp = _s_jdl_stat_total_ins - 1;
    }
    cur_blk = (temp / JDL_STAT_PER_INST_NUM_BLK);
    if (cur_blk >= JDL_STAT_PER_INST_BLK_NUM)
    {
        cur_blk = (JDL_STAT_PER_INST_BLK_NUM - 1);
    }
    cur_blk_base = (JDL_STAT_ADR_STAT_BASE + (JDL_STAT_PER_INST_BLK_SIZE * cur_blk));
    
    if ((size == JDL_DATA_TYPE_SIZE_BYTE)
     || (size == JDL_DATA_TYPE_SIZE_WORD)
     || (size == JDL_DATA_TYPE_SIZE_DWORD))
    {
        /* Save data */
        rtn = _jdl_stat_cnt((cur_blk_base + offset), size, cur_blk_base, (cur_blk_base + JDL_STAT_OFS_STAT_CHECKSUM));
    }
    else
    {
    /* Configuration error */
        rtn = JDL_E_CONF;
    }
    
    return rtn;
}
#endif  /* _JDL_STAT_USE_STAT_PER_INST && _JDL_STAT_USE_ACC_CNT */



