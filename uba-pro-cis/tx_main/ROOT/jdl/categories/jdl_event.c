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
 * @file jdl_event.c
 * @brief  Event category of JCM Device Log
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/

#include "jdl.h"



/************************** BACKUP VARIABLES ***************************/


/************************** PRIVATE DEFINITIONS ************************/


/************************** PRIVATE FUNCTIONS **************************/


/************************** PRIVATE VARIABLES **************************/
static u16 _s_jdl_event_rev;
static u8 _s_jdl_event_rtn; /* For Debug */

/************************** EXTERN FUNCTIONS ***************************/


/************************** EXTERN VARIABLES ***************************/



/***********************************************************************/
/**
 * @brief initialize event category
 * @param[in]  clear : force clear setting
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_event_init(u8 clear)
{
    u8 rtn = JDL_E_OK;
    
    /* Load sensor category revision */
    _s_jdl_event_rtn = _jdl_load(JDL_EVEN_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_event_rev);
    
    if (_s_jdl_event_rev != _JDL_SWAP_16(JDL_EVEN_REV))
    {
    /* Change revision */
        rtn = JDL_E_REVCHG;
    }
    
    if ((clear != 0)
     || (rtn != JDL_E_OK))
    {
    /* Clear log */
        _s_jdl_event_rev = _JDL_SWAP_16(JDL_EVEN_REV);
        _s_jdl_event_rtn = _jdl_clear(JDL_EVEN_ADR_BUFF_BASE, JDL_EVEN_BUFF_TOTAL);
        _s_jdl_event_rtn = _jdl_save(JDL_EVEN_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_event_rev);
        _jdl_err_link_clear();
    }
    
    /* Save log buffer size */
    _s_jdl_event_rtn = _jdl_save_word(JDL_EVEN_ADR_RCD_SIZE, JDL_EVEN_RCD_SIZE);
    _s_jdl_event_rtn = _jdl_save_word(JDL_EVEN_ADR_RCD_NUM, JDL_EVEN_RCD_NUM);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set the event data to log buffer of event category 
 * @param[in]  data   : pointer to the event data to set
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_event_set(u8 *data)
{
    u8 rtn = JDL_E_OK;
    u8 rcd_rnd;
    u16 cur_rcd;
    u32 cur_rcd_idx;
    u32 time;
    u32 comm_idx = 0;
    u8 comm_rnd = 0;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
#if defined(_JDL_USE_COMM)
        /* Get communication category log index and buffer round */
        _jdl_comm_get_idx(&comm_idx, &comm_rnd);
#endif  /* _JDL_USE_COMM */
        /* Get elapsed time (100msec) */
        _jdl_get_100m_etime(&time);
        
        /* Load current record index and buffer round */
        _s_jdl_event_rtn = _jdl_load_word(JDL_EVEN_ADR_CUR_RCD, &cur_rcd);
        _s_jdl_event_rtn = _jdl_load(JDL_EVEN_ADR_RCD_RND, JDL_DATA_TYPE_SIZE_BYTE, &rcd_rnd);
        
        /* Check index */
        if (cur_rcd > JDL_EVEN_RCD_NUM)
        {
        /* Clear the buffer because current index is faile */
            _s_jdl_event_rtn = _jdl_clear(JDL_EVEN_ADR_CUR_RCD, ((JDL_EVEN_ADR_RCD_BASE + JDL_EVEN_RCD_TOTAL) - JDL_EVEN_ADR_CUR_RCD));
            cur_rcd = 0;
            rcd_rnd = 0;
            _jdl_err_link_clear();
        }
        
        /* Shift current record index and buffer round */
        if (cur_rcd < JDL_EVEN_RCD_NUM)
        {
            cur_rcd++;
            if (rcd_rnd == 0)
            {
                rcd_rnd = 1;
            }
        }
        else
        {
            cur_rcd = 1;
            if (rcd_rnd == 0xFF)
            {
                rcd_rnd = 1;
            }
            else
            {
                rcd_rnd++;
            }
        }
        cur_rcd_idx = JDL_EVEN_ADR_RCD_BASE;
        cur_rcd_idx += (JDL_EVEN_RCD_SIZE * (cur_rcd - 1));
        
        /* Save current record index and buffer round */
        _s_jdl_event_rtn = _jdl_save_word(JDL_EVEN_ADR_CUR_RCD, cur_rcd);
        _s_jdl_event_rtn = _jdl_save_byte(JDL_EVEN_ADR_RCD_RND, rcd_rnd);
        
        /* Save record data */
        _s_jdl_event_rtn = _jdl_save(cur_rcd_idx, JDL_EVEN_OFS_TIME, data);
        _s_jdl_event_rtn = _jdl_save_dword((cur_rcd_idx + JDL_EVEN_OFS_TIME), time);
        _s_jdl_event_rtn = _jdl_save_dword((cur_rcd_idx + JDL_EVEN_OFS_COMM_IDX), comm_idx);
        _s_jdl_event_rtn = _jdl_save_byte((cur_rcd_idx + JDL_EVEN_OFS_COMM_RND), comm_rnd);
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
u8 _jdl_event_req(u32 s_offset, u32 buff_size)
{
    u8 rtn;

	if (buff_size < 1)
    {
    /* Size error */
        rtn = JDL_E_PARAM;
    }
    else if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_EVEN_SEND_TOTAL))
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
 * @brief get the data from log buffer of event category 
 * @param[out] buff      : pointer to the buffer to get
 * @param[in]  buff_size : buffer size
 * @param[in]  s_offset  : offset address of send data for each category
 * @param[out] g_size    : size of data gotten
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_event_get(u8 *buff, u32 buff_size, u32 s_offset, u32 *g_size)
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
    else if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_EVEN_SEND_TOTAL))
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
            _jdl_memcpy((void *)&category_header[JDL_CHEAD_ADR_NAME], "EVENT", sizeof("EVENT"));
            *((u16 *)&category_header[JDL_CHEAD_ADR_ID]) = _JDL_SWAP_16(JDL_CATE_ID_EVENT);
            *((u16 *)&category_header[JDL_CHEAD_ADR_REV]) = _s_jdl_event_rev;
            *((u32 *)&category_header[JDL_CHEAD_ADR_CSIZE]) = _JDL_SWAP_32(((u32)JDL_SIZE_CATEGORY_HEADER + (u32)JDL_EVEN_SEND_TOTAL));
            
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
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_EVEN_SEND_TOTAL)))
        {
        /* Log Datas Area */
            temp_offset = (w_offset - JDL_SIZE_CATEGORY_HEADER);
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_EVEN_SEND_TOTAL)
            {
            /* Log Datas or less */
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_EVEN_SEND_TOTAL - temp_offset);
            }
            /* Add Category base address */
            temp_offset += JDL_EVEN_ADR_SEND_BASE;
            _s_jdl_event_rtn = _jdl_load(temp_offset, temp_wsize, (buff + w_size));
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
void _jdl_event_link_clear(void)
{
    u16 cnt;
    u32 rnd_offset;
    
    rnd_offset = (JDL_EVEN_ADR_RCD_BASE + JDL_EVEN_OFS_COMM_RND);
    for (cnt = 0; cnt < JDL_EVEN_RCD_NUM; cnt++)
    {
        _s_jdl_event_rtn = _jdl_save_byte(rnd_offset, 0x00);
        rnd_offset += JDL_EVEN_RCD_SIZE;
    }
}


/***********************************************************************/
/**
 * @brief get the current index of event log buffer
 * @param[out] index : current index
 * @param[out] round : number of round
 * @return     none
 */
/***********************************************************************/
void _jdl_event_get_idx(u16 *index, u8 *round)
{
    _s_jdl_event_rtn = _jdl_load_word(JDL_EVEN_ADR_CUR_RCD, index);
    _s_jdl_event_rtn = _jdl_load(JDL_EVEN_ADR_RCD_RND, JDL_DATA_TYPE_SIZE_BYTE, round);
}


/***********************************************************************/
/**
 * @brief get the error code
 * @return     none
 */
/***********************************************************************/
u8 _jdl_event_get_err_code(void)
{
    return _s_jdl_event_rtn;
}


