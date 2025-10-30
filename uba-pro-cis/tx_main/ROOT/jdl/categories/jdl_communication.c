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
 * @file jdl_communication.c
 * @brief  Communication category of JCM Device Log
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/

#include "jdl.h"


#if defined(_JDL_USE_COMM)

/************************** BACKUP VARIABLES ***************************/


/************************** PRIVATE DEFINITIONS ************************/


/************************** PRIVATE FUNCTIONS **************************/
static u8 _jdl_comm_shift_head_index(u8 size, u32 *head_idx, u32 *tail_idx, u8 *buff_rnd);


/************************** PRIVATE VARIABLES **************************/
static u16 _s_jdl_comm_rev;
static u8 _s_jdl_comm_write_interval;
static u8 _s_jdl_comm_rtn; /* For Debug */

/************************** EXTERN FUNCTIONS ***************************/


/************************** EXTERN VARIABLES ***************************/



/***********************************************************************/
/**
 * @brief initialize communication category
 * @param[in]  clear : force clear setting
 * @param[in]  pid   : Protocol ID
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_comm_init(u8 clear, u8 *pid)
{
    u8 rtn = JDL_E_OK;
    u8 cnt;
    u8 lpid[JDL_SIZE_PROTOCOL_ID];
    u32 head_idx;
    u32 tail_idx;
    
    /* Load communication category revision */
    _s_jdl_comm_rtn = _jdl_load(JDL_COMM_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_comm_rev);
    
    if (_s_jdl_comm_rev != _JDL_SWAP_16(JDL_COMM_REV))
    {
        rtn = JDL_E_REVCHG;
    }
    
    /* Check protocol id */
    if ((clear == 0)
     && (rtn == JDL_E_OK)
     && (pid != JDL_NULL))
    {
        /* Load protocol id */
        _s_jdl_comm_rtn = _jdl_load(JDL_COMM_ADR_PROTOCOL_ID, JDL_SIZE_PROTOCOL_ID, &lpid[0]);
        
        for (cnt = 0; cnt < JDL_SIZE_PROTOCOL_ID; cnt++)
        {
            if (pid[cnt] != lpid[cnt])
            {
                rtn = JDL_E_IDCHG;
                break;
            }
        }
    }
    
    /* Check log index(head/tail) */
    if ((clear == 0)
     && (rtn == JDL_E_OK))
    {
        /* Load head/tail index */
        _s_jdl_comm_rtn = _jdl_load_dword(JDL_COMM_ADR_HEAD_INDEX, &head_idx);
        _s_jdl_comm_rtn = _jdl_load_dword(JDL_COMM_ADR_TAIL_INDEX, &tail_idx);
        
        if ((head_idx >= JDL_COMM_BUFFER_SIZE)
         || (tail_idx >= JDL_COMM_BUFFER_SIZE)
         || (head_idx == tail_idx))
        {
        /* Index error */
            rtn = JDL_E_DCORR;
        }
    }
    
    if ((clear != 0)
     || (rtn != JDL_E_OK))
    {
    /* Clear log */
        if (pid == JDL_NULL)
        {
            /* Load protocol id */
            _s_jdl_comm_rtn = _jdl_load(JDL_COMM_ADR_PROTOCOL_ID, JDL_SIZE_PROTOCOL_ID, &lpid[0]);
            _s_jdl_comm_rev = _JDL_SWAP_16(JDL_COMM_REV);
            _s_jdl_comm_rtn = _jdl_clear(JDL_COMM_ADR_BUFF_BASE, JDL_COMM_BUFF_TOTAL);
            _s_jdl_comm_rtn = _jdl_save(JDL_COMM_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_comm_rev);
            _s_jdl_comm_rtn = _jdl_save(JDL_COMM_ADR_PROTOCOL_ID, JDL_SIZE_PROTOCOL_ID, &lpid[0]);
        }
        else
        {
            _s_jdl_comm_rev = _JDL_SWAP_16(JDL_COMM_REV);
            _s_jdl_comm_rtn = _jdl_clear(JDL_COMM_ADR_BUFF_BASE, JDL_COMM_BUFF_TOTAL);
            _s_jdl_comm_rtn = _jdl_save(JDL_COMM_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_comm_rev);
            _s_jdl_comm_rtn = _jdl_save(JDL_COMM_ADR_PROTOCOL_ID, JDL_SIZE_PROTOCOL_ID, pid);
        }
        _jdl_event_link_clear();
    }
    
    /* Save log buffer size */
    _s_jdl_comm_rtn = _jdl_save_dword(JDL_COMM_ADR_BUFF_SIZE, JDL_COMM_BUFFER_SIZE);
    
    /* Init interval timer for communication actegory */
    _s_jdl_comm_write_interval = 0;
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief update communication category tick time
 * @return     none
 */
/***********************************************************************/
void _jdl_comm_tick(void)
{
    /* Increment timer */
    if (_s_jdl_comm_write_interval != 0xFF)
    {
        _s_jdl_comm_write_interval++;
    }
}


/***********************************************************************/
/**
 * @brief add the data to log buffer of communication category 
 * @param[in]  data    : pointer to the data to add from
 * @param[in]  size    : number of bytes to add.
 * @param[in]  tx_flag : transmission flag to add.
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_comm_add_data(u8 *data, u8 size, u8 tx_flag)
{
    u8 rtn = JDL_E_OK;
    u8 header[JDL_COMM_PKT_HEADER_SIZE];
    u8 add_size;
    u8 buff_rnd;
    u32 head_idx;
    u32 tail_idx;
    u32 cnt;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
        /* Check command size */
        if (size > 0x7F)
        {
            header[0] = 0x7F;
            rtn = JDL_E_PARAM;
        }
        else
        {
            header[0] = size;
        }
        add_size = (header[0] + JDL_COMM_PKT_HEADER_SIZE);
        
        /* Load head/tail index and buffer round */
        _s_jdl_comm_rtn = _jdl_load_dword(JDL_COMM_ADR_HEAD_INDEX, &head_idx);
        _s_jdl_comm_rtn = _jdl_load_dword(JDL_COMM_ADR_TAIL_INDEX, &tail_idx);
        _s_jdl_comm_rtn = _jdl_load(JDL_COMM_ADR_BUFF_ROUND, JDL_DATA_TYPE_SIZE_BYTE, &buff_rnd);
        
        /* Check index */
        if ((head_idx >= JDL_COMM_BUFFER_SIZE)
         || (tail_idx >= JDL_COMM_BUFFER_SIZE)
         || ((head_idx != 0) && (head_idx == tail_idx)))
        {
            /* Data corrupted */
            _s_jdl_comm_rtn = _jdl_clear(JDL_COMM_ADR_HEAD_INDEX, ((JDL_COMM_ADR_LOG_BASE + JDL_COMM_BUFFER_SIZE) - JDL_COMM_ADR_HEAD_INDEX));
            head_idx = 0;
            tail_idx = 0;
            buff_rnd = 0;
            _jdl_event_link_clear();
        }
        else if ((head_idx != 0)
              || (tail_idx != 0))
        {
            /* Shift Head index */
            rtn = _jdl_comm_shift_head_index(add_size, &head_idx, &tail_idx, &buff_rnd);
            if (rtn != JDL_E_OK)
            {
                /* Data corrupted */
                _s_jdl_comm_rtn = _jdl_clear(JDL_COMM_ADR_HEAD_INDEX, ((JDL_COMM_ADR_LOG_BASE + JDL_COMM_BUFFER_SIZE) - JDL_COMM_ADR_HEAD_INDEX));
                head_idx = 0;
                tail_idx = 0;
                buff_rnd = 0;
                _jdl_event_link_clear();
            }
        }
        
        /* Set Tx flag to Data Header */
        if (tx_flag)
        {
            header[0] |= 0x80;
        }
        header[1] = _s_jdl_comm_write_interval;
        
        /* Add Packet Data Header */
        if ((head_idx == 0)
         && (tail_idx == 0))
        {
        /* Log data is empty */
            _s_jdl_comm_rtn = _jdl_save((tail_idx + JDL_COMM_ADR_LOG_BASE), JDL_COMM_PKT_HEADER_SIZE, &header[0]);
            tail_idx = (JDL_COMM_PKT_HEADER_SIZE - 1);
            buff_rnd = 1;
        }
        else
        {
            for (cnt = 0; cnt < JDL_COMM_PKT_HEADER_SIZE; cnt++)
            {
                if ((tail_idx + 1) >= JDL_COMM_BUFFER_SIZE)
                {
                    tail_idx = 0;
                    if (buff_rnd == 0xFF)
                    {
                        buff_rnd = 1;
                    }
                    else
                    {
                        buff_rnd++;
                    }
                }
                else
                {
                    tail_idx++;
                }
                /* Save header */
                _s_jdl_comm_rtn = _jdl_save((tail_idx + JDL_COMM_ADR_LOG_BASE), JDL_DATA_TYPE_SIZE_BYTE, &header[cnt]);
            }
        }
        
        /* Add Packet Data */
        for (cnt = 0; cnt < (add_size - JDL_COMM_PKT_HEADER_SIZE); cnt++)
        {
            if ((tail_idx + 1) >= JDL_COMM_BUFFER_SIZE)
            {
                tail_idx = 0;
                if (buff_rnd == 0xFF)
                {
                    buff_rnd = 1;
                }
                else
                {
                    buff_rnd++;
                }
            }
            else
            {
                tail_idx++;
            }
            /* Save data */
            _s_jdl_comm_rtn = _jdl_save((tail_idx + JDL_COMM_ADR_LOG_BASE), JDL_DATA_TYPE_SIZE_BYTE, (data + cnt));
        }
        
        /* Save head/tail index and buffer round */
        _s_jdl_comm_rtn = _jdl_save_dword(JDL_COMM_ADR_HEAD_INDEX, head_idx);
        _s_jdl_comm_rtn = _jdl_save_dword(JDL_COMM_ADR_TAIL_INDEX, tail_idx);
        _s_jdl_comm_rtn = _jdl_save_byte(JDL_COMM_ADR_BUFF_ROUND, buff_rnd);
        
        _s_jdl_comm_write_interval = 0;
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
u8 _jdl_comm_req(u32 s_offset, u32 buff_size)
{
    u8 rtn;

	if (buff_size < 1)
    {
    /* Size error */
        rtn = JDL_E_PARAM;
    }
    else if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_COMM_SEND_TOTAL))
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
 * @brief get the data from log buffer of communication category 
 * @param[out] buff      : pointer to the buffer to get
 * @param[in]  buff_size : buffer size
 * @param[in]  s_offset  : offset address of send data for each category
 * @param[out] g_size    : size of data gotten
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_comm_get(u8 *buff, u32 buff_size, u32 s_offset, u32 *g_size)
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
    else if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_COMM_SEND_TOTAL))
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
            _jdl_memcpy((void *)&category_header[JDL_CHEAD_ADR_NAME], "COMMUNICATION", sizeof("COMMUNICATION"));
            *((u16 *)&category_header[JDL_CHEAD_ADR_ID]) = _JDL_SWAP_16(JDL_CATE_ID_COMMUNICATION);
            *((u16 *)&category_header[JDL_CHEAD_ADR_REV]) = _s_jdl_comm_rev;
            *((u32 *)&category_header[JDL_CHEAD_ADR_CSIZE]) = _JDL_SWAP_32(((u32)JDL_SIZE_CATEGORY_HEADER + (u32)JDL_COMM_SEND_TOTAL));
            
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
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_COMM_SEND_TOTAL)))
        {
        /* Log Datas Area */
            temp_offset = (w_offset - JDL_SIZE_CATEGORY_HEADER);
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_COMM_SEND_TOTAL)
            {
            /* Log Datas or less */
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_COMM_SEND_TOTAL - temp_offset);
            }
            /* Add Category base address */
            temp_offset += JDL_COMM_ADR_SEND_BASE;
            _s_jdl_comm_rtn = _jdl_load(temp_offset, temp_wsize, (buff + w_size));
            w_size += temp_wsize;
            w_offset += temp_wsize;
        }
        
        *g_size = w_size;
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief get the current index of communication log buffer
 * @param[out] index : current index
 * @param[out] round : number of round
 * @return     none
 */
/***********************************************************************/
void _jdl_comm_get_idx(u32 *index, u8 *round)
{
    _s_jdl_comm_rtn = _jdl_load_dword(JDL_COMM_ADR_TAIL_INDEX, index);
    _s_jdl_comm_rtn = _jdl_load(JDL_COMM_ADR_BUFF_ROUND, JDL_DATA_TYPE_SIZE_BYTE, round);
}


/***********************************************************************/
/**
 * @brief get the error code
 * @return     none
 */
/***********************************************************************/
u8 _jdl_comm_get_err_code(void)
{
    return _s_jdl_comm_rtn;
}


/***********************************************************************/
/**
 * @brief shift the head index
 * @param[in]  size   : data size to add
 * @return     error cord
 */
/***********************************************************************/
static u8 _jdl_comm_shift_head_index(u8 size, u32 *head_idx, u32 *tail_idx, u8 *buff_rnd)
{
    u8 rtn = JDL_E_OK;
    u32 next_head;
    u32 next_tail;
    u8 h_shift_flag = 0;
    u8 next_h_rnd_flag = 0;
    u8 next_t_rnd_flag = 0;
    u8 temp;
    u8 cnt;
    
    /* Check to need head_idx shifted */
    if (*head_idx < *tail_idx)
    {
    /* 先頭が末尾より小さい */
        if ((*tail_idx + size) >= JDL_COMM_BUFFER_SIZE)
        {
        /* Next tail_idx is JDL_COMM_BUFFER_SIZE or more */
            next_tail = ((*tail_idx + size) - JDL_COMM_BUFFER_SIZE);
            if (*head_idx <= next_tail)
            {
            /* Need head index shifted */
                h_shift_flag = 1;
            }
        }
    }
    else
    {
    /* 先頭が末尾以上 */
        if (*head_idx <= (*tail_idx + size))
        {
        /* Next tail_idx is head_idx or more */
            if ((*tail_idx + size) >= JDL_COMM_BUFFER_SIZE)
            {
            /* Next tail_idx is JDL_COMM_BUFFER_SIZE or more */
                next_tail = ((*tail_idx + size) - JDL_COMM_BUFFER_SIZE);
                next_t_rnd_flag = 1;
            }
            else
            {
                next_tail = (*tail_idx + size);
            }
            /* Need head index shifted */
            h_shift_flag = 1;
        }
    }
    
    /* Shift head index */
    if (h_shift_flag == 1)
    {
        next_head = *head_idx;
        /* 最大43回シフトする                             */
        /*   保存最大数の129byte分は必ず移動できる        */
        /*    -> 127Byte(パケット最大数)+2byte(ヘッダー)  */
        for (cnt = 0; cnt < 43; cnt++)
        {
            /* Load paket header */
            _s_jdl_comm_rtn = _jdl_load((next_head + JDL_COMM_ADR_LOG_BASE), JDL_DATA_TYPE_SIZE_BYTE, &temp);
            /* Shift paket  */
            next_head += ((temp & 0x7F) + JDL_COMM_PKT_HEADER_SIZE);
            if (next_head >= JDL_COMM_BUFFER_SIZE)
            {
                next_h_rnd_flag = 1;
                next_head = (next_head - JDL_COMM_BUFFER_SIZE);
            }
            
            if (next_t_rnd_flag == 1)
            {
            /* Next tail indexは一周する */
                if ((next_h_rnd_flag == 1)
                 && (next_head > next_tail))
                {
                    break;
                }
            }
            else
            {
            /* Next tail indexは一周しない */
                if (next_head > next_tail)
                {
                    break;
                }
            }
        }
        if (cnt >= 43)
        {
            rtn = JDL_E_DCORR;
        }
        else
        {
            *head_idx = next_head;
        }
    }
    
    return rtn;
}


#endif  /* _JDL_USE_COMM */


