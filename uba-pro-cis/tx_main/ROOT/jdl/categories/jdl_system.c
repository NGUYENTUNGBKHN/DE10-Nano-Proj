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
 * @file jdl_system.c
 * @brief  System category of JCM Device Log
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/

#include "jdl.h"



/************************** BACKUP VARIABLES ***************************/


/************************** PRIVATE DEFINITIONS ************************/


/************************** PRIVATE FUNCTIONS **************************/


/************************** PRIVATE VARIABLES **************************/
static u16 _s_jdl_sys_rev;

/************************** EXTERN FUNCTIONS ***************************/


/************************** EXTERN VARIABLES ***************************/



/***********************************************************************/
/**
 * @brief initialize system category
 * @param[in]  clear : force clear setting
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_sys_init(u8 clear)
{
    u8 rtn = JDL_E_OK;
    u8 rev_ck = 0;
    
    /* Load system category revision */
    _jdl_load(JDL_SYS_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_sys_rev);
    
    if (_s_jdl_sys_rev != _JDL_SWAP_16(JDL_SYS_REV))
    {
    /* Change revision */
        rev_ck = 1;
        rtn = JDL_E_REVCHG;
    }
    
    if ((clear != 0)
     || (rev_ck != 0))
    {
    /* Clear log */
        _s_jdl_sys_rev = _JDL_SWAP_16(JDL_SYS_REV);
        _jdl_clear(JDL_SYS_ADR_BUFF_BASE, JDL_SYS_BUFF_TOTAL);
        _jdl_save(JDL_SYS_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_sys_rev);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set the data to log buffer of system category 
 * @param[in]  offset : offset address of log buffer to set.
 * @param[in]  data   : pointer to the data to set from
 * @param[in]  size   : number of bytes to set.
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_sys_set(u32 offset, u8 *data, u32 size)
{
    u8 rtn = JDL_E_OK;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if (size < 1)
    {
    /* Size error */
        rtn = JDL_E_PARAM;
    }
    else if ((offset < JDL_SYS_ADR_BUFF_BASE) || (JDL_SYS_ADR_FOR_NEXT_CATEGORY <= offset)
          || (size > JDL_SYS_ADR_FOR_NEXT_CATEGORY)
          || ((offset + size) > JDL_SYS_ADR_FOR_NEXT_CATEGORY))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
    /* Save data */
        _jdl_save(offset, size, data);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief compare the data and log buffer of system category.
 * @param[in]  offset : offset address of log buffer to compare.
 * @param[in]  data   : pointer to the data to compare.
 * @param[in]  size   : number of bytes to compare.
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_sys_cmp(u32 offset, u8 *data, u32 size)
{
    u8 rtn = JDL_E_OK;
    u16 cnt;
    u8 ldata;
    
    if (size < 1)
    {
    /* Size error */
        rtn = JDL_E_PARAM;
    }
    else if ((offset < JDL_SYS_ADR_BUFF_BASE) || (JDL_SYS_ADR_FOR_NEXT_CATEGORY <= offset)
          || (size > JDL_SYS_ADR_FOR_NEXT_CATEGORY)
          || ((offset + size) > JDL_SYS_ADR_FOR_NEXT_CATEGORY))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        for (cnt = 0; cnt < size; cnt++)
        {
            /* Loag data and compare */
            _jdl_load((offset + cnt), JDL_DATA_TYPE_SIZE_BYTE, &ldata);
            if (ldata != data[cnt])
            {
            /* Different */
                rtn = JDL_E_DDATA;
                break;
            }
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set the data to log buffer of system category 
 * @param[in]  offset : offset address of log buffer to set to.
 * @param[in]  data   : pointer to the data to set from
 * @param[in]  size   : number of bytes to set.
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_sys_set_tim(void)
{
    u8 rtn = JDL_E_OK;
    JDL_TIME stime;
    JDL_TIME etime;
    
    /* Get JDL time */
    _jdl_get_time(&stime, &etime);
    
    /* Set time */
    _jdl_save_dword(JDL_SYS_ADR_SET_TIME, stime.high);
    _jdl_save_dword((JDL_SYS_ADR_SET_TIME + JDL_DATA_TYPE_SIZE_DWORD), stime.low);
    
    /* Elapsed time */
    _jdl_save_dword(JDL_SYS_ADR_ELAP_TIME, etime.high);
    _jdl_save_dword((JDL_SYS_ADR_ELAP_TIME + JDL_DATA_TYPE_SIZE_DWORD), etime.low);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief request  the data from log buffer of system category 
 * @param[out] buff      : pointer to the buffer to get
 * @param[in]  buff_size : buffer size
 * @param[in]  s_offset  : offset address of send data for each category
 * @param[out] g_size    : size of data gotten
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_sys_req(u32 s_offset, u32 buff_size)
{
    u8 rtn;

	if (buff_size < 1)
    {
    /* Size error */
        rtn = JDL_E_PARAM;
    }
    else if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_SYS_SEND_TOTAL))
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
u8 _jdl_sys_get(u8 *buff, u32 buff_size, u32 s_offset, u32 *g_size)
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
    else if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_SYS_SEND_TOTAL))
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
            _jdl_memcpy((void *)&category_header[JDL_CHEAD_ADR_NAME], "SYSTEM", sizeof("SYSTEM"));
            *((u16 *)&category_header[JDL_CHEAD_ADR_ID]) = _JDL_SWAP_16(JDL_CATE_ID_SYSTEM);
            *((u16 *)&category_header[JDL_CHEAD_ADR_REV]) = _s_jdl_sys_rev;
            *((u32 *)&category_header[JDL_CHEAD_ADR_CSIZE]) = _JDL_SWAP_32(((u32)JDL_SIZE_CATEGORY_HEADER + (u32)JDL_SYS_SEND_TOTAL));
            
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
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_SYS_SEND_TOTAL)))
        {
        /* Log Datas Area */
            temp_offset = (w_offset - JDL_SIZE_CATEGORY_HEADER);
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_SYS_SEND_TOTAL)
            {
            /* Log Datas or less */
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_SYS_SEND_TOTAL - temp_offset);
            }
            
            /* Add Category base address */
            temp_offset += JDL_SYS_ADR_SEND_BASE;
            _jdl_load(temp_offset, temp_wsize, (buff + w_size));
            w_size += temp_wsize;
            w_offset += temp_wsize;
        }
        
        *g_size = w_size;
    }
    
    return rtn;
}


