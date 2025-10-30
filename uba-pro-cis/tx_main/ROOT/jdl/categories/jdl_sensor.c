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
 * @file jdl_sensor.c
 * @brief  Sensor category of JCM Device Log
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/

#include "jdl.h"


#if defined(_JDL_USE_SENS)

/************************** BACKUP VARIABLES ***************************/


/************************** PRIVATE DEFINITIONS ************************/


/************************** PRIVATE FUNCTIONS **************************/


/************************** PRIVATE VARIABLES **************************/
static u16 _s_jdl_sensor_rev;


/************************** EXTERN FUNCTIONS ***************************/


/************************** EXTERN VARIABLES ***************************/



/***********************************************************************/
/**
 * @brief initialize sensor category
 * @param[in]  clear : force clear setting
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_sens_init(u8 clear)
{
    u8 rtn = JDL_E_OK;
    
    /* Load sensor category revision */
    _jdl_load(JDL_SENS_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_sensor_rev);
    
    if (_s_jdl_sensor_rev != _JDL_SWAP_16(JDL_SENS_REV))
    {
    /* Change revision */
        rtn = JDL_E_REVCHG;
    }
    
    if ((clear != 0)
     || (rtn != JDL_E_OK))
    {
    /* Clear log */
        _s_jdl_sensor_rev = _JDL_SWAP_16(JDL_SENS_REV);
        _jdl_clear(JDL_SENS_ADR_BUFF_BASE, JDL_SENS_BUFF_TOTAL);
        _jdl_save(JDL_SENS_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_sensor_rev);
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set value
 * @param[in]  offset : Offset of value to set
 * @param[in]  type   : Data type ( size )
 * @param[in]  val    : Data to set
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_sens_set_val(u32 offset, u8 type, u32 val)
{
    u8 rtn = JDL_E_OK;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if ((offset < JDL_SENS_ADR_BUFF_BASE) || (JDL_SENS_ADR_FOR_NEXT_CATEGORY <= offset)
     || ((offset + type) > JDL_SENS_ADR_FOR_NEXT_CATEGORY))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        /* Sellect data type and save data */
        switch (type)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            rtn = _jdl_save_byte(offset, (u8)(0x000000FF & val));
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            rtn = _jdl_save_word(offset, (u16)(0x0000FFFF & val));
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            rtn = _jdl_save_dword(offset, val);
           break;
        default:
            rtn = JDL_E_PARAM;
            break;
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set block data
 * @param[in]  offset : Offset of data to set
 * @param[in]  data   : pointer to the data to set from
 * @param[in]  size   : number of bytes to set.
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_sens_set_blk(u32 offset, u8 *data, u32 size)
{
    u8 rtn = JDL_E_OK;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else if (size < 1)
    {
    /* Parameter Error */
        rtn = JDL_E_PARAM;
    }
    else if ((offset < JDL_SENS_ADR_BUFF_BASE) || (JDL_SENS_ADR_FOR_NEXT_CATEGORY <= offset)
          || (size > JDL_SENS_ADR_FOR_NEXT_CATEGORY)
          || ((offset + size) > JDL_SENS_ADR_FOR_NEXT_CATEGORY))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        /* Save data */
        rtn = _jdl_save(offset, size, data);
   }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief update correction value
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_sens_update_cor(void) /* センサ条項更新の為 ポジションセンサのDA値を前回のポジションセンサDA値にコピー*/
{
    u8 rtn = JDL_E_OK;
    JDL_TIME time;
    u32 csize;
    
    if (_jdl_get_mode() != JDL_MODE_ENABLE)
    {
    /* Disable */
        rtn = JDL_E_DISABLE;
    }
    else
    {
        /* Get JDL time (1sec) */
        _jdl_get_1s_ctime(&time);
        
        /* Size of correction area */
        csize = (JDL_SENS_ADR_PRE_COR_VAL - JDL_SENS_ADR_COR_VAL);
        
        /* Copy correction area to previous area */
        _jdl_copy(JDL_SENS_ADR_PRE_COR_VAL, JDL_SENS_ADR_COR_VAL, csize);
        /* Clear correction area */
        _jdl_clear(JDL_SENS_ADR_COR_VAL, csize);
        
        /* Set correction time */
        _jdl_save_dword(JDL_SENS_ADR_COR_TIME, time.high);
        _jdl_save_dword((JDL_SENS_ADR_COR_TIME + JDL_DATA_TYPE_SIZE_DWORD), time.low);
    }
    
    return rtn;
}

/***********************************************************************/
/**
 * @brief set current value
 * @param[in]  offset : Offset of value to set
 * @param[in]  type   : Data type ( 0:Byte, 1:Word, 2:Dword, 3;Qword )
 * @param[in]  val    : Data to set
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_sens_set_cur_val(u32 offset, u8 type, u32 val)
{
    u8 rtn = JDL_E_OK;
    
    if ((offset < JDL_SENS_ADR_BUFF_BASE) || (JDL_SENS_ADR_FOR_NEXT_CATEGORY <= offset)
     || ((offset + type) > JDL_SENS_ADR_FOR_NEXT_CATEGORY))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else
    {
        /* Sellect data type and save data */
        switch (type)
        {
        case JDL_DATA_TYPE_SIZE_BYTE:
            rtn = _jdl_save_byte(offset, (u8)(0x000000FF & val));
            break;
        case JDL_DATA_TYPE_SIZE_WORD:
            rtn = _jdl_save_word(offset, (u16)(0x0000FFFF & val));
            break;
        case JDL_DATA_TYPE_SIZE_DWORD:
            rtn = _jdl_save_dword(offset, val);
           break;
        default:
            rtn = JDL_E_PARAM;
            break;
        }
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set current block data
 * @param[in]  offset : Offset of data to set
 * @param[in]  data   : pointer to the data to set from
 * @param[in]  size   : number of bytes to set.
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_sens_set_cur_blk(u32 offset, u8 *data, u32 size)
{
    u8 rtn = JDL_E_OK;
    
    if ((offset < JDL_SENS_ADR_BUFF_BASE) || (JDL_SENS_ADR_FOR_NEXT_CATEGORY <= offset)
     || (size > JDL_SENS_ADR_FOR_NEXT_CATEGORY)
     || ((offset + size) > JDL_SENS_ADR_FOR_NEXT_CATEGORY))
    {
    /* Memory access violation */
        rtn = JDL_E_MACV;
    }
    else if (size < 1)
    {
    /* Parameter Error */
        rtn = JDL_E_PARAM;
    }
    else
    {
        /* Save data */
        rtn = _jdl_save(offset, size, data);
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
u8 _jdl_sens_req(u32 s_offset, u32 buff_size)
{
    u8 rtn;

	if (buff_size < 1)
    {
    /* Size error */
        rtn = JDL_E_PARAM;
    }
    else if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_SENS_SEND_TOTAL))
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
 * @brief get the data from log buffer of sensor category 
 * @param[out] buff      : pointer to the buffer to get
 * @param[in]  buff_size : buffer size
 * @param[in]  s_offset  : offset address of send data for each category
 * @param[out] g_size    : size of data gotten
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_sens_get(u8 *buff, u32 buff_size, u32 s_offset, u32 *g_size)
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
    else if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_SENS_SEND_TOTAL))
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
            _jdl_memcpy((void *)&category_header[JDL_CHEAD_ADR_NAME], "SENSOR", sizeof("SENSOR"));
            *((u16 *)&category_header[JDL_CHEAD_ADR_ID]) = _JDL_SWAP_16(JDL_CATE_ID_SENSOR);
            *((u16 *)&category_header[JDL_CHEAD_ADR_REV]) = _s_jdl_sensor_rev;
            *((u32 *)&category_header[JDL_CHEAD_ADR_CSIZE]) = _JDL_SWAP_32(((u32)JDL_SIZE_CATEGORY_HEADER + (u32)JDL_SENS_SEND_TOTAL));
            
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
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_SENS_SEND_TOTAL)))
        {
        /* Log Datas Area */
            temp_offset = (w_offset - JDL_SIZE_CATEGORY_HEADER);
            
            /* Check remaining size */
            if (((buff_size - w_size) + temp_offset) <= JDL_SENS_SEND_TOTAL)
            {
            /* Log Datas or less */
                temp_wsize = (buff_size - w_size);
            }
            else
            {
                temp_wsize = (JDL_SENS_SEND_TOTAL - temp_offset);
            }
            /* Add Category base address */
            temp_offset += JDL_SENS_ADR_SEND_BASE;
            _jdl_load(temp_offset, temp_wsize, (buff + w_size));
            w_size += temp_wsize;
            w_offset += temp_wsize;
        }
        
        *g_size = w_size;
    }
    
    return rtn;
}


#endif  /* _JDL_USE_SENS */


