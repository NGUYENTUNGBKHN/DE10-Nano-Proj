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
 * @file jdl_position_analysis.c
 * @brief  Position Analysis category of JCM Device Log
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/

#include "jdl.h"


#if defined(_JDL_USE_POSIANA)

/************************** BACKUP VARIABLES ***************************/


/************************** PRIVATE DEFINITIONS ************************/


/************************** PRIVATE FUNCTIONS **************************/


/************************** PRIVATE VARIABLES **************************/
static u16 _s_jdl_posi_ana_rev;
static u8 _s_jdl_posiana_recode[JDL_PANA_RCD_TOTAL];
static u8 _s_jdl_posiana_prev[JDL_PANA_RCD_TOTAL];
static u16 _s_jdl_posiana_cur;
static u8 _s_jdl_posiana_rtn; /* For Debug */
static u8 _s_jdl_posiana_bk_stat[JDL_PANA_RCD_SIZE];
static u16 _s_jdl_posiana_etime;


/************************** EXTERN FUNCTIONS ***************************/


/************************** EXTERN VARIABLES ***************************/



/***********************************************************************/
/**
 * @brief initialize position analysis category
 * @param[in]  clear : force clear setting
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_posiana_init(u8 clear)
{
    u8 rtn = JDL_E_OK;
    
    /* Load acceptance category revision */
    _s_jdl_posiana_rtn = _jdl_load(JDL_PANA_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_posi_ana_rev);
    
    if (_s_jdl_posi_ana_rev != _JDL_SWAP_16(JDL_PANA_REV))
    {
    /* Change revision */
        rtn = JDL_E_REVCHG;
    }
    
    if ((clear != 0)
     || (rtn != JDL_E_OK))
    {
    /* Clear log */
        _s_jdl_posi_ana_rev = _JDL_SWAP_16(JDL_PANA_REV);
        _s_jdl_posiana_rtn = _jdl_clear(JDL_PANA_ADR_BUFF_BASE, JDL_PANA_BUFF_TOTAL);
        _s_jdl_posiana_rtn = _jdl_save(JDL_PANA_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_posi_ana_rev);
    }
    
    /* Save log buffer size */
    _s_jdl_posiana_rtn = _jdl_save_word(JDL_PANA_ADR_BLK_SIZE, JDL_PANA_BLK_SIZE);
    _s_jdl_posiana_rtn = _jdl_save_word(JDL_PANA_ADR_BLK_NUM, JDL_PANA_BLK_NUM);
    
    _s_jdl_posiana_rtn = _jdl_save_word(JDL_PANA_ADR_SDATA_SIZE, JDL_PANA_RCD_SIZE);
    _s_jdl_posiana_rtn = _jdl_save_word(JDL_PANA_ADR_SDATA_NUM, JDL_PANA_RCD_NUM);
    
    /* Init current position analysis record variables */
    _jdl_memset(&_s_jdl_posiana_recode[0], 0, JDL_PANA_RCD_TOTAL);
    _jdl_memset(&_s_jdl_posiana_prev[0], 0, JDL_PANA_RCD_TOTAL);
    _s_jdl_posiana_cur = 0;
    
    /* Init backup sensor state variables */
    _jdl_memset(&_s_jdl_posiana_bk_stat[0], 0, JDL_PANA_RCD_SIZE);
    _s_jdl_posiana_etime = 0;
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set the data to log buffer of analysis category
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_posiana_set(void)
{
    u8 rtn = JDL_E_OK;
    u8 blk_rnd;
    u16 cur_blk;
    u32 cur_blk_idx;
    u32 w_idx;
    u32 sdata_idx;
    u32 temp;
    u16 err_blk;
    u8 err_rnd;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
        /* Get error category log index and buffer round */
        _jdl_err_get_idx(&err_blk, &err_rnd);
        
        /* Load current block index and buffer round */
        _s_jdl_posiana_rtn = _jdl_load_word(JDL_PANA_ADR_CUR_BLK, &cur_blk);
        _s_jdl_posiana_rtn = _jdl_load(JDL_PANA_ADR_BLK_RND, JDL_DATA_TYPE_SIZE_BYTE, &blk_rnd);
        
        /* Shift current block index and buffer round */
        if ((cur_blk > 0)
         && (cur_blk < JDL_PANA_BLK_NUM))
        {
            cur_blk++;
        }
        else
        {
            cur_blk = 1;
            if (blk_rnd == 0xFF)
            {
                blk_rnd = 1;
            }
            else
            {
                blk_rnd++;
            }
        }
        cur_blk_idx = JDL_PANA_ADR_BLK_BASE;
        cur_blk_idx += (JDL_PANA_BLK_SIZE * (cur_blk - 1));
        
        /* Save current block index and buffer round */
        _s_jdl_posiana_rtn = _jdl_save_word(JDL_PANA_ADR_CUR_BLK, cur_blk);
        _s_jdl_posiana_rtn = _jdl_save_byte(JDL_PANA_ADR_BLK_RND, blk_rnd);
        
        /* Save position analysis infomation data */
        _s_jdl_posiana_rtn = _jdl_save_word(cur_blk_idx, err_blk);
        _s_jdl_posiana_rtn = _jdl_save_byte((cur_blk_idx + JDL_PANA_OFS_BLK_ERR_RND), err_rnd);
        
        /* Save position analysis record */
        w_idx = (cur_blk_idx + JDL_PANA_OFS_BLK_SDATA);
        if ((_s_jdl_posiana_cur > 0) && (_s_jdl_posiana_cur < JDL_PANA_RCD_NUM))
        {
            sdata_idx = (_s_jdl_posiana_cur * JDL_PANA_RCD_SIZE);
            temp = (JDL_PANA_RCD_TOTAL - sdata_idx);
            _s_jdl_posiana_rtn = _jdl_save(w_idx, temp, &_s_jdl_posiana_recode[sdata_idx]);
            w_idx += temp;
            _s_jdl_posiana_rtn = _jdl_save(w_idx, sdata_idx, &_s_jdl_posiana_recode[0]);
        }
        else
        {
            _s_jdl_posiana_rtn = _jdl_save(w_idx, JDL_PANA_RCD_TOTAL, &_s_jdl_posiana_recode[0]);
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set the data to local buffer last operation
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_posiana_set_prev(void)
{
    u8 rtn = JDL_E_OK;
    u32 sdata_idx;
    u32 temp;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
        /* Set position analysis record */
        if ((_s_jdl_posiana_cur > 0) && (_s_jdl_posiana_cur < JDL_PANA_RCD_NUM))
        {
            sdata_idx = (_s_jdl_posiana_cur * JDL_PANA_RCD_SIZE);
            temp = (JDL_PANA_RCD_TOTAL - sdata_idx);
            _jdl_memcpy(&_s_jdl_posiana_prev[0], &_s_jdl_posiana_recode[sdata_idx], temp);
            _jdl_memcpy(&_s_jdl_posiana_prev[temp], &_s_jdl_posiana_recode[0], sdata_idx);
        }
        else
        {
            _jdl_memcpy(&_s_jdl_posiana_prev[0], &_s_jdl_posiana_recode[0], JDL_PANA_RCD_TOTAL);
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief add the position sensor data to log buffer
 * @param[in]  data   : pointer to the position sensor data to add
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_posiana_update_state(u8 *data)
{
    u8 rtn = JDL_E_OK;
    u32 idx;
    u8 cnt;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
        for (cnt = 0; cnt < JDL_PANA_RCD_SENS_SIZE; cnt++)
        {
            if (_s_jdl_posiana_bk_stat[cnt] != data[cnt])
            {
                break;
            }
        }
        if ((cnt < JDL_PANA_RCD_SENS_SIZE) || (_s_jdl_posiana_etime == 0xFFFF))
        {
        /* Sensor/Actuator state is changed or Elapsed time is maximum */
        /* Shift current index */
            if ((_s_jdl_posiana_cur > 0) && (_s_jdl_posiana_cur < JDL_PANA_RCD_NUM))
        {
            _s_jdl_posiana_cur++;
        }
        else
        {
            _s_jdl_posiana_cur = 1;
        }
            idx = ((_s_jdl_posiana_cur - 1) * JDL_PANA_RCD_SIZE);
            
            /* Backup Sensor/Actuator state */
            _jdl_memcpy(&_s_jdl_posiana_bk_stat[0], data, JDL_PANA_RCD_SDATE_SIZE);
            *((u16 *)&_s_jdl_posiana_bk_stat[JDL_PANA_RCD_SDATE_SIZE]) = _JDL_SWAP_16(_s_jdl_posiana_etime);
            /* Clear elapsed time */
            _s_jdl_posiana_etime = 0;
        
        /* Set sampling data */
            _jdl_memcpy(&_s_jdl_posiana_recode[idx], &_s_jdl_posiana_bk_stat[0], JDL_PANA_RCD_SIZE);
        }
        else
        {
        /* Increment elapsed time */
            _s_jdl_posiana_etime++;
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
u8 _jdl_posiana_req(u32 s_offset, u32 buff_size)
{
    u8 rtn;

	if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_PANA_SEND_TOTAL + (JDL_PANA_RCD_TOTAL * 2)))
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
 * @brief get the data from log buffer of position analysis category 
 * @param[out] buff      : pointer to the buffer to get
 * @param[in]  buff_size : buffer size
 * @param[in]  s_offset  : offset address of send data for each category
 * @param[out] g_size    : size of data gotten
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_posiana_get(u8 *buff, u32 buff_size, u32 s_offset, u32 *g_size)
{
    u8 rtn = JDL_E_OK;
    u8 category_header[JDL_SIZE_CATEGORY_HEADER];
    u32 w_size;
    u32 w_offset;
    u32 temp_wsize;
    u32 temp_offset;
    u32 temp_sdata;
    u32 sdata_idx;
    
    
    if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_PANA_SEND_TOTAL + (JDL_PANA_RCD_TOTAL * 2)))
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
            _jdl_memcpy((void *)&category_header[JDL_CHEAD_ADR_NAME], "POSITION ANALYS", sizeof("POSITION ANALYS"));
            *((u16 *)&category_header[JDL_CHEAD_ADR_ID]) = _JDL_SWAP_16(JDL_CATE_ID_POSIANA);
            *((u16 *)&category_header[JDL_CHEAD_ADR_REV]) = _s_jdl_posi_ana_rev;
            *((u32 *)&category_header[JDL_CHEAD_ADR_CSIZE]) = _JDL_SWAP_32(((u32)JDL_SIZE_CATEGORY_HEADER + (u32)JDL_PANA_SEND_TOTAL + (u32)(JDL_PANA_RCD_TOTAL * 2)));
            
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
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_PANA_SEND_TOTAL)))
        {
        /* Log Datas Area */
            temp_offset = (w_offset - JDL_SIZE_CATEGORY_HEADER);
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_PANA_SEND_TOTAL)
            {
            /* Log Datas or less */
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_PANA_SEND_TOTAL - temp_offset);
            }
            /* Add Category base address */
            temp_offset += JDL_PANA_ADR_SEND_BASE;
            _s_jdl_posiana_rtn = _jdl_load(temp_offset, temp_wsize, (buff + w_size));
            w_size += temp_wsize;
            w_offset += temp_wsize;
        }
        
        if ((w_size < buff_size)
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_PANA_SEND_TOTAL + JDL_PANA_RCD_TOTAL)))
        {
        /* Current Sensor Datas Area */
            temp_offset = (w_offset - (JDL_SIZE_CATEGORY_HEADER + JDL_PANA_SEND_TOTAL));
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_PANA_RCD_TOTAL)
            {
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_PANA_RCD_TOTAL - temp_offset);
            }
            
            /* Sort the sensor data from oldest to newest */
            if ((_s_jdl_posiana_cur > 0) && (_s_jdl_posiana_cur < JDL_PANA_RCD_NUM))
            {
                sdata_idx = (_s_jdl_posiana_cur * JDL_PANA_RCD_SIZE);
                temp_sdata = (JDL_PANA_RCD_TOTAL - sdata_idx);
                if (temp_sdata < temp_wsize)
                {
                    _jdl_memcpy((buff + w_size), &_s_jdl_posiana_recode[sdata_idx], temp_sdata);
                    w_size += temp_sdata;
                    _jdl_memcpy((buff + w_size), &_s_jdl_posiana_recode[0], (temp_wsize - temp_sdata));
                    w_size += (temp_wsize - temp_sdata);
                }
                else
                {
                    _jdl_memcpy((buff + w_size), &_s_jdl_posiana_recode[sdata_idx], temp_wsize);
                    w_size += temp_wsize;
                }
            }
            else
            {
                _jdl_memcpy((buff + w_size), &_s_jdl_posiana_recode[0], temp_wsize);
                w_size += temp_wsize;
            }
            
            w_offset += temp_wsize;
        }
        
        if ((w_size < buff_size)
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_PANA_SEND_TOTAL + (JDL_PANA_RCD_TOTAL * 2))))
        {
        /* Previous operation Sensor Datas Area */
            temp_offset = (w_offset - (JDL_SIZE_CATEGORY_HEADER + JDL_PANA_SEND_TOTAL + JDL_PANA_RCD_TOTAL));
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_PANA_RCD_TOTAL)
            {
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_PANA_RCD_TOTAL - temp_offset);
            }
            _jdl_memcpy((buff + w_size), &_s_jdl_posiana_prev[temp_offset], temp_wsize);
            w_size += temp_wsize;
            w_offset += temp_wsize;
        }
        
        *g_size = w_size;
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief clear link
 * @return     none
 */
/***********************************************************************/
void _jdl_posiana_link_clear(void)
{
    u16 cnt;
    u32 rnd_offset;
    
    rnd_offset = (JDL_PANA_ADR_BLK_BASE + JDL_PANA_OFS_BLK_ERR_RND);
    for (cnt = 0; cnt < JDL_PANA_BLK_NUM; cnt++)
    {
        _s_jdl_posiana_rtn = _jdl_save_byte(rnd_offset, 0x00);
        rnd_offset += JDL_PANA_BLK_SIZE;
    }
}


/***********************************************************************/
/**
 * @brief get the error code
 * @return     none
 */
/***********************************************************************/
u8 _jdl_posiana_get_err_code(void)
{
    return _s_jdl_posiana_rtn;
}


#endif  /* _JDL_USE_POSIANA */


