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
 * @file jdl_error.c
 * @brief  Error category of JCM Device Log
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/

#include "jdl.h"



/************************** BACKUP VARIABLES ***************************/


/************************** PRIVATE DEFINITIONS ************************/


/************************** PRIVATE FUNCTIONS **************************/


/************************** PRIVATE VARIABLES **************************/
static u16 _s_jdl_error_rev;
static u8 _s_jdl_error_trace_rcd[JDL_ERR_TRCD_TOTAL];
static u16 _s_jdl_error_tracd_cur;
static u8 _s_jdl_error_rtn; /* For Debug */



/************************** EXTERN FUNCTIONS ***************************/


/************************** EXTERN VARIABLES ***************************/



/***********************************************************************/
/**
 * @brief initialize error category
 * @param[in]  clear : force clear setting
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_err_init(u8 clear)
{
    u8 rtn = JDL_E_OK;
    
    /* Load sensor category revision */
    _s_jdl_error_rtn = _jdl_load(JDL_ERR_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_error_rev);
    
    if (_s_jdl_error_rev != _JDL_SWAP_16(JDL_EER_REV))
    {
    /* Change revision */
        rtn = JDL_E_REVCHG;
    }
    
    if ((clear != 0)
     || (rtn != JDL_E_OK))
    {
    /* Clear log */
        _s_jdl_error_rev = _JDL_SWAP_16(JDL_EER_REV);
        _s_jdl_error_rtn = _jdl_clear(JDL_ERR_ADR_BUFF_BASE, JDL_ERR_BUFF_TOTAL);
        _s_jdl_error_rtn = _jdl_save(JDL_ERR_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_error_rev);
#if defined(_JDL_USE_POSIANA)
        _jdl_posiana_link_clear();
#endif  /* _JDL_USE_POSIANA */
    }
    
    /* Save log buffer size */
    _s_jdl_error_rtn = _jdl_save_word(JDL_ERR_ADR_BLK_SIZE, JDL_ERR_BLK_SIZE);
    _s_jdl_error_rtn = _jdl_save_word(JDL_ERR_ADR_BLK_NUM, JDL_ERR_BLK_NUM);
    _s_jdl_error_rtn = _jdl_save_word(JDL_ERR_ADR_TRCD_SIZE, JDL_ERR_TRCD_SIZE);
    _s_jdl_error_rtn = _jdl_save_word(JDL_ERR_ADR_TRCD_NUM, JDL_ERR_TRCD_NUM);
    
    /* Init current trace record variables */
    _jdl_memset(&_s_jdl_error_trace_rcd[0], 0, JDL_ERR_TRCD_TOTAL);
    _s_jdl_error_tracd_cur = 0;
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set the error data to log buffer of error category 
 * @param[in]  data   : pointer to the error data to set
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_err_set(u8 *data)
{
    u8 rtn = JDL_E_OK;
    u8 blk_rnd;
    u16 cur_blk;
    u32 cur_blk_idx;
    u32 w_idx;
    u32 trcd_idx;
    u32 temp;
    u16 event_idx;
    u8 event_rnd;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
        /* Get event category log index and buffer round */
        _jdl_event_get_idx(&event_idx, &event_rnd);
        
        /* Load current block index and buffer round */
        _s_jdl_error_rtn = _jdl_load_word(JDL_ERR_ADR_CUR_BLK, &cur_blk);
        _s_jdl_error_rtn = _jdl_load(JDL_ERR_ADR_BLK_RND, JDL_DATA_TYPE_SIZE_BYTE, &blk_rnd);
        
        /* Check index */
        if (cur_blk > JDL_ERR_BLK_NUM)
        {
        /* Clear the buffer because current index is faile */
            _s_jdl_error_rtn = _jdl_clear(JDL_ERR_ADR_CUR_BLK, ((JDL_ERR_ADR_BLK_BASE + JDL_ERR_BLK_TOTAL) - JDL_ERR_ADR_CUR_BLK));
            cur_blk = 0;
            blk_rnd = 0;
#if defined(_JDL_USE_POSIANA)
            _jdl_posiana_link_clear();
#endif  /* _JDL_USE_POSIANA */
        }
        
        /* Shift current block index and buffer round */
        if (cur_blk < JDL_ERR_BLK_NUM)
        {
            cur_blk++;
            if (blk_rnd == 0)
            {
                blk_rnd = 1;
            }
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
        cur_blk_idx = JDL_ERR_ADR_BLK_BASE;
        cur_blk_idx += (JDL_ERR_BLK_SIZE * (cur_blk - 1));
        
        /* Save current block index and buffer round */
        _s_jdl_error_rtn = _jdl_save_word(JDL_ERR_ADR_CUR_BLK, cur_blk);
        _s_jdl_error_rtn = _jdl_save_byte(JDL_ERR_ADR_BLK_RND, blk_rnd);
        
        /* Save error infomation data */
        _s_jdl_error_rtn = _jdl_save(cur_blk_idx, JDL_ERR_OFS_BLK_EVEN_IDX, data);
        _s_jdl_error_rtn = _jdl_save_word((cur_blk_idx + JDL_ERR_OFS_BLK_EVEN_IDX), event_idx);
        _s_jdl_error_rtn = _jdl_save_byte((cur_blk_idx + JDL_ERR_OFS_BLK_EVEN_RND), event_rnd);
        
        /* Save trace record */
        w_idx = (cur_blk_idx + JDL_ERR_OFS_BLK_TRCD_BASE);
        if ((_s_jdl_error_tracd_cur > 0) && (_s_jdl_error_tracd_cur < JDL_ERR_TRCD_NUM))
        {
            trcd_idx = (_s_jdl_error_tracd_cur * JDL_ERR_TRCD_SIZE);
            temp = (JDL_ERR_TRCD_TOTAL - trcd_idx);
            _s_jdl_error_rtn = _jdl_save(w_idx, temp, &_s_jdl_error_trace_rcd[trcd_idx]);
            w_idx += temp;
            _s_jdl_error_rtn = _jdl_save(w_idx, trcd_idx, &_s_jdl_error_trace_rcd[0]);
        }
        else
        {
            _s_jdl_error_rtn = _jdl_save(w_idx, JDL_ERR_TRCD_TOTAL, &_s_jdl_error_trace_rcd[0]);
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief update the error data to log buffer of error category
 * @param[in]  data   : pointer to the error data to update
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_err_update(u8 *data)
{
    u8 rtn = JDL_E_OK;
    u16 cur_blk;
    u32 cur_blk_idx;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
        /* Load current block index */
        _s_jdl_error_rtn = _jdl_load_word(JDL_ERR_ADR_CUR_BLK, &cur_blk);
        
        if ((cur_blk < 1)
         || (cur_blk > JDL_ERR_BLK_NUM))
        {
            rtn = JDL_E_DCORR;
        }
        else
        {
        /* Updata error infomation data */
            cur_blk_idx = JDL_ERR_ADR_BLK_BASE;
            cur_blk_idx += (JDL_ERR_BLK_SIZE * (cur_blk - 1));
            
            _s_jdl_error_rtn = _jdl_save(cur_blk_idx, JDL_ERR_OFS_BLK_EVEN_IDX, data);
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief add the trace data to log buffer of error category 
 * @param[in]  data   : pointer to the trace data to add
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_err_add_trace(void *data)
{
    u8 rtn = JDL_E_OK;
    u32 idx;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
        /* Shift current trace record index */
        if ((_s_jdl_error_tracd_cur > 0) && (_s_jdl_error_tracd_cur < JDL_ERR_TRCD_NUM))
        {
            _s_jdl_error_tracd_cur++;
        }
        else
        {
            _s_jdl_error_tracd_cur = 1;
        }
        idx = ((_s_jdl_error_tracd_cur - 1) * JDL_ERR_TRCD_SIZE);
        
        /* Add trace record */
        _jdl_memcpy(&_s_jdl_error_trace_rcd[idx], data, JDL_ERR_TRCD_SIZE);
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
u8 _jdl_err_req(u32 s_offset, u32 buff_size)
{
    u8 rtn;

	if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_ERR_SEND_TOTAL + JDL_ERR_TRCD_TOTAL))
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
 * @brief get the data from log buffer of error category 
 * @param[out] buff      : pointer to the buffer to get
 * @param[in]  buff_size : buffer size
 * @param[in]  s_offset  : offset address of send data for each category
 * @param[out] g_size    : size of data gotten
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_err_get(u8 *buff, u32 buff_size, u32 s_offset, u32 *g_size)
{
    u8 rtn = JDL_E_OK;
    u8 category_header[JDL_SIZE_CATEGORY_HEADER];
    u32 w_size;
    u32 w_offset;
    u32 temp_wsize;
    u32 temp_offset;
    u32 temp_trcd;
    u32 trcd_idx;
    
    
    if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_ERR_SEND_TOTAL + JDL_ERR_TRCD_TOTAL))
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
            _jdl_memcpy((void *)&category_header[JDL_CHEAD_ADR_NAME], "ERROR", sizeof("ERROR"));
            *((u16 *)&category_header[JDL_CHEAD_ADR_ID]) = _JDL_SWAP_16(JDL_CATE_ID_ERROR);
            *((u16 *)&category_header[JDL_CHEAD_ADR_REV]) = _s_jdl_error_rev;
            *((u32 *)&category_header[JDL_CHEAD_ADR_CSIZE]) = _JDL_SWAP_32(((u32)JDL_SIZE_CATEGORY_HEADER + (u32)JDL_ERR_SEND_TOTAL + (u32)JDL_ERR_TRCD_TOTAL));
            
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
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_ERR_SEND_TOTAL)))
        {
        /* Log Datas Area */
            temp_offset = (w_offset - JDL_SIZE_CATEGORY_HEADER);
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_ERR_SEND_TOTAL)
            {
            /* Log Datas or less */
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_ERR_SEND_TOTAL - temp_offset);
            }
            /* Add Category base address */
            temp_offset += JDL_ERR_ADR_SEND_BASE;
            _s_jdl_error_rtn = _jdl_load(temp_offset, temp_wsize, (buff + w_size));
            w_size += temp_wsize;
            w_offset += temp_wsize;
        }
        
        if ((w_size < buff_size)
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_ERR_SEND_TOTAL + JDL_ERR_TRCD_TOTAL)))
        {
        /* Current Sensor Datas Area */
            temp_offset = (w_offset - (JDL_SIZE_CATEGORY_HEADER + JDL_ERR_SEND_TOTAL));
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_ERR_TRCD_TOTAL)
            {
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_ERR_TRCD_TOTAL - temp_offset);
            }
            
            /* Sort the trace data from oldest to newest */
            if ((_s_jdl_error_tracd_cur > 0) && (_s_jdl_error_tracd_cur < JDL_ERR_TRCD_NUM))
            {
                trcd_idx = (_s_jdl_error_tracd_cur * JDL_ERR_TRCD_SIZE);
                temp_trcd = (JDL_ERR_TRCD_TOTAL - trcd_idx);
                
                /* Check remaining size */
                if (temp_trcd < temp_wsize)
                {
                    _jdl_memcpy((buff + w_size), &_s_jdl_error_trace_rcd[trcd_idx], temp_trcd);
                    w_size += temp_trcd;
                    _jdl_memcpy((buff + w_size), &_s_jdl_error_trace_rcd[0], (temp_wsize - temp_trcd));
                    w_size += (temp_wsize - temp_trcd);
                }
                else
                {
                    _jdl_memcpy((buff + w_size), &_s_jdl_error_trace_rcd[trcd_idx], temp_wsize);
                    w_size += temp_wsize;
                }
            }
            else
            {
                _jdl_memcpy((buff + w_size), &_s_jdl_error_trace_rcd[0], temp_wsize);
                w_size += temp_wsize;
            }
            
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
void _jdl_err_link_clear(void)
{
    u16 cnt;
    u32 rnd_offset;
    
    rnd_offset = (JDL_ERR_ADR_BLK_BASE + JDL_ERR_OFS_BLK_EVEN_RND);
    for (cnt = 0; cnt < JDL_ERR_BLK_NUM; cnt++)
    {
        _s_jdl_error_rtn = _jdl_save_byte(rnd_offset, 0x00);
        rnd_offset += JDL_ERR_BLK_SIZE;
    }
}


/***********************************************************************/
/**
 * @brief get the current index of error log buffer
 * @param[out] index : current index
 * @param[out] round : number of round
 * @return     none
 */
/***********************************************************************/
void _jdl_err_get_idx(u16 *index, u8 *round)
{
    _s_jdl_error_rtn = _jdl_load_word(JDL_ERR_ADR_CUR_BLK, index);
    _s_jdl_error_rtn = _jdl_load(JDL_ERR_ADR_BLK_RND, JDL_DATA_TYPE_SIZE_BYTE, round);
}


/***********************************************************************/
/**
 * @brief get the error code
 * @return     none
 */
/***********************************************************************/
u8 _jdl_err_get_err_code(void)
{
    return _s_jdl_error_rtn;
}


