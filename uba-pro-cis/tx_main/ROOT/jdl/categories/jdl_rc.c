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
 * @file jdl_rc.c
 * @brief  RC category of JCM Device Log
 * @date 2018.10.25
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/

#include "jdl.h"


#if defined(_JDL_USE_RC)

/************************** BACKUP VARIABLES ***************************/


/************************** PRIVATE DEFINITIONS ************************/


/************************** PRIVATE FUNCTIONS **************************/
static void _jdl_rc_clear(void);
static void _jdl_rc_check_sum(void);


/************************** PRIVATE VARIABLES **************************/
static u16 _s_jdl_rc_rev;


/************************** EXTERN FUNCTIONS ***************************/


/************************** EXTERN VARIABLES ***************************/



/***********************************************************************/
/**
 * @brief initialize rc category
 * @param[in]  clear : force clear setting
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_rc_init(u8 clear)
{
    u8 rtn = JDL_E_OK;
    
    /* Load acceptance category revision */
    _jdl_load(JDL_RC_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_rc_rev);
    
    if (_s_jdl_rc_rev != _JDL_SWAP_16(JDL_RC_REV))
    {
    /* Change revision */
        rtn = JDL_E_REVCHG;
    }
    
    if ((clear != 0)
     || (rtn != JDL_E_OK))
    {
    /* Clear log */
        _jdl_rc_clear();
    }
    else
    {
        /* Check the checksum */
        _jdl_rc_check_sum();
    }
    
    /* Save unit infomation settings */
    _jdl_save_word(JDL_RC_ADR_UNIT_NUM, JDL_RC_UNIT_NUM);
    _jdl_save_word(JDL_RC_ADR_UNIT_INFO_SIZE, JDL_RC_UNIFO_SIZE);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set unit infomation
 * @param[in]  unit_no      : unit number(index)
 * @param[in]  uinfo_offset : unifo offset of data to set
 * @param[in]  data         : pointer to the data to set from
 * @param[in]  size         : number of bytes to set.
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_rc_set_uinfo(u16 unit_no, u32 uinfo_offset, u8 *data, u32 size)
{
    u8 rtn = JDL_E_OK;
    u32 uinfo_base;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if ((unit_no >= JDL_RC_UNIT_NUM) || (size < 1))
    {
    /* Parameter Error */
        rtn = JDL_E_PARAM;
    }
    else if (uinfo_offset >= JDL_RC_OFS_UINFO_DINFO_BASE)
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        uinfo_base = (JDL_RC_ADR_UINFO_BASE + (JDL_RC_UNIFO_SIZE * unit_no));
        
        /* Save data */
        rtn = _jdl_save((uinfo_base + uinfo_offset), size, data);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set unit infomation for word type
 * @param[in]  unit_no      : unit number(index)
 * @param[in]  uinfo_offset : unifo offset of data to set
 * @param[in]  data         : word type data
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_rc_set_uinfo_word(u16 unit_no, u32 uinfo_offset, u16 data)
{
    u8 rtn = JDL_E_OK;
    u32 uinfo_base;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if (unit_no >= JDL_RC_UNIT_NUM)
    {
    /* Parameter Error */
        rtn = JDL_E_PARAM;
    }
    else if (uinfo_offset >= JDL_RC_OFS_UINFO_DINFO_BASE)
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        uinfo_base = (JDL_RC_ADR_UINFO_BASE + (JDL_RC_UNIFO_SIZE * unit_no));
        
        /* Save data */
        rtn = _jdl_save_word((uinfo_base + uinfo_offset), data);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set drum infomation
 * @param[in]  unit_no      : unit number(index)
 * @param[in]  drum_no      : drum number(index)
 * @param[in]  dinfo_offset : dinfo offset of data to set
 * @param[in]  data         : pointer to the data to set from
 * @param[in]  size         : number of bytes to set.
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_rc_set_dinfo(u16 unit_no, u16 drum_no, u32 dinfo_offset, u8 *data, u32 size)
{
    u8 rtn = JDL_E_OK;
    u32 uinfo_base;
    u32 dinfo_base;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if ((unit_no >= JDL_RC_UNIT_NUM) || (drum_no >= JDL_RC_DRUM_NUM) || (size < 1))
    {
    /* Parameter Error */
        rtn = JDL_E_PARAM;
    }
    else if ((dinfo_offset < (JDL_RC_OFS_DINFO_CHECKSUM + JDL_SIZE_CHECKSUM)) || (JDL_RC_OFS_DINFO_TRACK_BASE <= dinfo_offset))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        uinfo_base = (JDL_RC_ADR_UINFO_BASE + (JDL_RC_UNIFO_SIZE * unit_no));
        dinfo_base = ((uinfo_base + JDL_RC_OFS_UINFO_DINFO_BASE) + (JDL_RC_DINFO_SIZE * drum_no));
        
        /* Save data */
        rtn = _jdl_save((dinfo_base + dinfo_offset), size, data);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set drum infomation for word type
 * @param[in]  unit_no      : unit number(index)
 * @param[in]  drum_no      : drum number(index)
 * @param[in]  dinfo_offset : dinfo offset of data to set
 * @param[in]  data         : pointer to the data to set from
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_rc_set_dinfo_word(u16 unit_no, u16 drum_no, u32 dinfo_offset, u16 data)
{
    u8 rtn = JDL_E_OK;
    u32 uinfo_base;
    u32 dinfo_base;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if ((unit_no >= JDL_RC_UNIT_NUM) || (drum_no >= JDL_RC_DRUM_NUM))
    {
    /* Parameter Error */
        rtn = JDL_E_PARAM;
    }
    else if ((dinfo_offset < (JDL_RC_OFS_DINFO_CHECKSUM + JDL_SIZE_CHECKSUM)) || (JDL_RC_OFS_DINFO_TRACK_BASE <= dinfo_offset))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        uinfo_base = (JDL_RC_ADR_UINFO_BASE + (JDL_RC_UNIFO_SIZE * unit_no));
        dinfo_base = ((uinfo_base + JDL_RC_OFS_UINFO_DINFO_BASE) + (JDL_RC_DINFO_SIZE * drum_no));
        
        /* Save data */
        rtn = _jdl_save_word((dinfo_base + dinfo_offset), data);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief Increment total counter.
 * @param[in]  unit_no      : unit number (index)
 * @param[in]  drum_no      : drum number (index)
 * @param[in]  dinfo_offset : dinfo (total count) offset
 * @param[in]  type         : data type (size)
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_rc_inc_tcnt(u16 unit_no, u16 drum_no, u32 dinfo_offset, u8 type)
{
    u8 rtn = JDL_E_OK;
    u32 uinfo_base;
    u32 dinfo_base;
    u8 data[JDL_DATA_TYPE_SIZE_DWORD];
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if ((unit_no >= JDL_RC_UNIT_NUM) || (drum_no >= JDL_RC_DRUM_NUM))
    {
    /* Parameter Error */
        rtn = JDL_E_PARAM;
    }
    else if (dinfo_offset >= JDL_RC_OFS_DINFO_CHECKSUM)
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        uinfo_base = (JDL_RC_ADR_UINFO_BASE + (JDL_RC_UNIFO_SIZE * unit_no));
        dinfo_base = ((uinfo_base + JDL_RC_OFS_UINFO_DINFO_BASE) + (JDL_RC_DINFO_SIZE * drum_no));
        
        /* Select counter type and incremented */
        switch (type)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            rtn = _jdl_inc_8bit((dinfo_base + dinfo_offset), &data[0]);
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            rtn = _jdl_inc_16bit((dinfo_base + dinfo_offset), (u16 *)&data[0]);
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            rtn = _jdl_inc_32bit((dinfo_base + dinfo_offset), (u32 *)&data[0]);
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
            _jdl_save_data_checksum((dinfo_base + dinfo_offset), &data[0], type, dinfo_base, (dinfo_base + JDL_RC_OFS_DINFO_CHECKSUM));
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief add tracking infomation
 * @param[in]  unit_no      : unit number(index)
 * @param[in]  drum_no      : drum number(index)
 * @param[in]  data         : pointer to the data to set from
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_rc_add_track(u16 unit_no, u16 drum_no, u8 data)
{
    u8 rtn = JDL_E_OK;
    u32 uinfo_base;
    u32 dinfo_base;
    u32 track_base;
    u32 cur_track;
    u16 cur_num;
    u8 sdata;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if ((unit_no >= JDL_RC_UNIT_NUM) || (drum_no >= JDL_RC_DRUM_NUM))
    {
    /* Parameter Error */
        rtn = JDL_E_PARAM;
    }
    else
    {
        uinfo_base = (JDL_RC_ADR_UINFO_BASE + (JDL_RC_UNIFO_SIZE * unit_no));
        dinfo_base = ((uinfo_base + JDL_RC_OFS_UINFO_DINFO_BASE) + (JDL_RC_DINFO_SIZE * drum_no));
        track_base = (dinfo_base + JDL_RC_OFS_DINFO_TRACK_BASE);
        
        rtn = _jdl_load_word(track_base, &cur_num);
        
        if (rtn == JDL_E_OK)
        {
            if (cur_num >= JDL_RC_TRACKING_NUM)
            {
                sdata = data;
                cur_num++;
                cur_track = ((track_base + JDL_RC_OFS_TRACK_BASE) + cur_num);
                
                cur_num = _JDL_SWAP_16(cur_num);
                /* Calaculate checksum and Save data */
                rtn = _jdl_save_data_2_checksum(track_base, (u8 *)&cur_num, JDL_DATA_TYPE_SIZE_WORD, cur_track, &sdata, JDL_DATA_TYPE_SIZE_BYTE, track_base, (track_base + JDL_RC_OFS_TRACK_CHECKSUM));
            }
            else
            {
            /* Tracking data full */
                rtn = JDL_E_BFULL;
            }
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief remove tracking infomation
 * @param[in]  unit_no      : unit number(index)
 * @param[in]  drum_no      : drum number(index)
 * @param[in]  data         : pointer to the data to set from
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_rc_remove_track(u16 unit_no, u16 drum_no)
{
    u8 rtn = JDL_E_OK;
    u32 uinfo_base;
    u32 dinfo_base;
    u32 track_base;
    u32 cur_track;
    u16 cur_num;
    u8 sdata;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if ((unit_no >= JDL_RC_UNIT_NUM) || (drum_no >= JDL_RC_DRUM_NUM))
    {
    /* Parameter Error */
        rtn = JDL_E_PARAM;
    }
    else
    {
        uinfo_base = (JDL_RC_ADR_UINFO_BASE + (JDL_RC_UNIFO_SIZE * unit_no));
        dinfo_base = ((uinfo_base + JDL_RC_OFS_UINFO_DINFO_BASE) + (JDL_RC_DINFO_SIZE * drum_no));
        track_base = (dinfo_base + JDL_RC_OFS_DINFO_TRACK_BASE);
        
        rtn = _jdl_load_word(track_base, &cur_num);
        
        if (rtn == JDL_E_OK)
        {
            if (cur_num > 0)
            {
                sdata = 0;
                cur_track = ((track_base + JDL_RC_OFS_TRACK_BASE) + cur_num);
                
                cur_num--;
                cur_num = _JDL_SWAP_16(cur_num);
                /* Calaculate checksum and Save data */
                rtn = _jdl_save_data_2_checksum(track_base, (u8 *)&cur_num, JDL_DATA_TYPE_SIZE_WORD, cur_track, &sdata, JDL_DATA_TYPE_SIZE_BYTE, track_base, (track_base + JDL_RC_OFS_TRACK_CHECKSUM));
            }
            else
            {
            /* Tracking data empty */
                rtn = JDL_E_BEMPTY;
            }
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
u8 _jdl_rc_req(u32 s_offset, u32 buff_size)
{
    u8 rtn;

	if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_RC_SEND_TOTAL))
    {
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
 * @brief get the data from log buffer of RC category 
 * @param[out] buff      : pointer to the buffer to get
 * @param[in]  buff_size : buffer size
 * @param[in]  s_offset  : offset address of send data for each category
 * @param[out] g_size    : size of data gotten
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_rc_get(u8 *buff, u32 buff_size, u32 s_offset, u32 *g_size)
{
    u8 rtn = JDL_E_OK;
    u8 category_header[JDL_SIZE_CATEGORY_HEADER];
    u32 w_size;
    u32 w_offset;
    u32 temp_wsize;
    u32 temp_offset;
    //u32 temp_sdata;
    //u32 sdata_idx;
    
    
    if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_RC_SEND_TOTAL))
    {
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
            _jdl_memcpy((void *)&category_header[JDL_CHEAD_ADR_NAME], "AD_RC", sizeof("AD_RC"));
            *((u16 *)&category_header[JDL_CHEAD_ADR_ID]) = _JDL_SWAP_16(JDL_CATE_ID_AD_RC);
            *((u16 *)&category_header[JDL_CHEAD_ADR_REV]) = _s_jdl_rc_rev;
            *((u32 *)&category_header[JDL_CHEAD_ADR_CSIZE]) = _JDL_SWAP_32(((u32)JDL_SIZE_CATEGORY_HEADER + (u32)JDL_RC_SEND_TOTAL));
            
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
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_RC_SEND_TOTAL)))
        {
        /* Log Datas Area */
            temp_offset = (w_offset - JDL_SIZE_CATEGORY_HEADER);
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_RC_SEND_TOTAL)
            {
            /* Log Datas or less */
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_RC_SEND_TOTAL - temp_offset);
            }
            /* Add Category base address */
            temp_offset += JDL_RC_ADR_SEND_BASE;
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
 * @brief Clear rc category. 
 * @param[in]  non
 * @return     non
 */
/***********************************************************************/
static void _jdl_rc_clear(void)
{
    u32 uinfo_base;
    u32 dinfo_base;
    u32 track_base;
    u32 sum_addr;
    u8 ucnt;
    u8 dcnt;
    
    /* Clear Buffer */
    _jdl_clear(JDL_RC_ADR_BUFF_BASE, JDL_RC_BUFF_TOTAL);
    
    /* Category revision */
    _s_jdl_rc_rev = _JDL_SWAP_16(JDL_RC_REV);
    _jdl_save(JDL_RC_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_rc_rev);
    
    /* Unit infomation */
    for (ucnt = 0; ucnt < JDL_RC_UNIT_NUM; ucnt++)
    {
        uinfo_base = (JDL_RC_ADR_UINFO_BASE + (JDL_RC_UNIFO_SIZE * ucnt));
        
        /* Save drum infomation settings */
        _jdl_save_word((uinfo_base + JDL_RC_OFS_UINFO_DRUM_NUM), JDL_RC_DRUM_NUM);
        _jdl_save_word((uinfo_base + JDL_RC_OFS_UINFO_DINFO_SIZE), JDL_RC_DINFO_SIZE);
        
        /* Drum infomation */
        for (dcnt = 0; dcnt < JDL_RC_DRUM_NUM; dcnt++)
        {
            dinfo_base = ((uinfo_base + JDL_RC_OFS_UINFO_DINFO_BASE) + (JDL_RC_DINFO_SIZE * dcnt));
            track_base = (dinfo_base + JDL_RC_OFS_DINFO_TRACK_BASE);
            
            /* Total counter Check sum */
            sum_addr = (dinfo_base + JDL_RC_OFS_DINFO_CHECKSUM);
            _jdl_renew_checksum(dinfo_base, sum_addr);
            
            /* Save drum tracking settings */
            _jdl_save_word((dinfo_base + JDL_RC_OFS_DINFO_TRACK_NUM), JDL_RC_TRACKING_NUM);
            
            /* Tracking info Check sum */
            sum_addr = (track_base + JDL_RC_OFS_TRACK_CHECKSUM);
            _jdl_renew_checksum(track_base, sum_addr);
        }
    }
}


/***********************************************************************/
/**
 * @brief Check data checksum. 
 * @param[in]  non
 * @return     non
 */
/***********************************************************************/
static void _jdl_rc_check_sum(void)
{
    u16 sum;
    u16 prev_sum;
    u32 uinfo_base;
    u32 dinfo_base;
    u32 track_base;
    u32 sum_addr;
    u8 ucnt;
    u8 dcnt;
    
    /* Unit infomation */
    for (ucnt = 0; ucnt < JDL_RC_UNIT_NUM; ucnt++)
    {
        uinfo_base = (JDL_RC_ADR_UINFO_BASE + (JDL_RC_UNIFO_SIZE * ucnt));
        
        /* Drum infomation */
        for (dcnt = 0; dcnt < JDL_RC_DRUM_NUM; dcnt++)
        {
            dinfo_base = ((uinfo_base + JDL_RC_OFS_UINFO_DINFO_BASE) + (JDL_RC_DINFO_SIZE * dcnt));
            track_base = (dinfo_base + JDL_RC_OFS_DINFO_TRACK_BASE);
            
            /* Total counter Check sum */
            sum_addr = (dinfo_base + JDL_RC_OFS_DINFO_CHECKSUM);
            _jdl_calc_checksum(dinfo_base, sum_addr, &sum);
            _jdl_load_word(sum_addr, &prev_sum);
            if ((prev_sum & 0x8000) || (prev_sum != sum))
            {
                /* Set corrupted flag */
                sum |= 0x8000;
                _jdl_save_word(sum_addr, sum);
            }
            
            /* Tracking info Check sum */
            sum_addr = (track_base + JDL_RC_OFS_TRACK_CHECKSUM);
            _jdl_calc_checksum(track_base, sum_addr, &sum);
            _jdl_load_word(sum_addr, &prev_sum);
            if ((prev_sum & 0x8000) || (prev_sum != sum))
            {
                /* Set corrupted flag */
                sum |= 0x8000;
                _jdl_save_word(sum_addr, sum);
            }
        }
    }
}


#endif  /* _JDL_USE_RC */


