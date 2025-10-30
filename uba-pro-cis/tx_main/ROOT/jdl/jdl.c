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
 * @file jdl.c
 * @brief  JCM Device Log Common
 * @date 2017.09.20
 * @author JCM. TOKYO R&D SECTION. SOFTWARE DEVEROPMENT GROUP.
 */
/****************************************************************************/

#include "jdl.h"



/************************** BACKUP VARIABLES ***************************/


/************************** PRIVATE DEFINITIONS ************************/


/************************** PRIVATE FUNCTIONS **************************/


/************************** PRIVATE VARIABLES **************************/
static u16 _s_jdl_rev;
static u8 _s_jdl_mode;
static u8 _s_jdl_tick_cnt_10m;
static u8 _s_jdl_tick_cnt_100m;
static u8 _s_jdl_tick_cnt_1s;
static u32 _s_jdl_10m_time;
static u32 _s_jdl_100m_time;
static u32 _s_jdl_ener_time;
static JDL_TIME _s_jdl_set_time;
static JDL_TIME _s_jdl_elap_time;


/************************** EXTERN FUNCTIONS ***************************/


/************************** EXTERN VARIABLES ***************************/



/***********************************************************************/
/**
 * @brief update JDL tick time
 * @return     none
 */
/***********************************************************************/
void jdl_tick(void)
{
    _s_jdl_tick_cnt_10m++;
    if (_s_jdl_tick_cnt_10m >= JDL_NUM_OF_10MSEC_TICK)
    {
    /* Update 10msec timer */
        _s_jdl_tick_cnt_10m = 0;
        if (_s_jdl_10m_time == 0xFFFFFFFF)
        {
            _s_jdl_10m_time = 0;
        }
        else
        {
            _s_jdl_10m_time++;
        };
        
        _s_jdl_tick_cnt_100m++;
        if (_s_jdl_tick_cnt_100m >= 10)
        {
        /* Update 100msec timer */
            _s_jdl_tick_cnt_100m = 0;
            if (_s_jdl_100m_time == 0xFFFFFFFF)
            {
                _s_jdl_100m_time = 0;
            }
            else
            {
                _s_jdl_100m_time++;
            }
            
            _s_jdl_tick_cnt_1s++;
            if (_s_jdl_tick_cnt_1s >= 10)
            {
            /* Update 1sec timer */
                _s_jdl_tick_cnt_1s = 0;
                if (_s_jdl_elap_time.low == 0xFFFFFFFF)
                {
                    _s_jdl_elap_time.low = 0;
                    if (_s_jdl_elap_time.high != 0xFFFFFFFF)
                    {
                        _s_jdl_elap_time.high++;
                    }
                }
                else
                {
                    _s_jdl_elap_time.low++;
                }
                if (_s_jdl_ener_time != 0xFFFFFFFF)
                {
                    _s_jdl_ener_time++;
                }
            }
        }
        
#if defined(_JDL_USE_COMM)
        if (_s_jdl_mode == JDL_MODE_ENABLE)
        {
            _jdl_comm_tick();
        }
#endif  /* _JDL_USE_COMM */
    }
}


/***********************************************************************/
/**
 * @brief initialize JDL
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_init(void)
{
    u8 rtn = JDL_E_OK;
    
    //_s_jdl_mode = JDL_MODE_DISABLE;
    _s_jdl_mode = JDL_MODE_ENABLE;
    
    /* Load JDL revision */
    _jdl_load(JDL_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_rev);
    
    if (_s_jdl_rev != _JDL_SWAP_16(JDL_FORMAT_REV))
    {
    /* Change revision */
        rtn = JDL_E_REVCHG;
        /* Save JDL revision */
        _s_jdl_rev = _JDL_SWAP_16(JDL_FORMAT_REV);
        _jdl_save(JDL_ADR_REV, JDL_DATA_TYPE_SIZE_WORD, (u8 *)&_s_jdl_rev);
    }
    
    
    /* Init timer variables */
    _s_jdl_tick_cnt_10m = 0;
    _s_jdl_tick_cnt_100m = 0;
    _s_jdl_tick_cnt_1s = 0;
    
    _s_jdl_10m_time = 0;
    _s_jdl_100m_time = 0;
    _s_jdl_ener_time = 0; /* For Energization time */
    
    _s_jdl_set_time.high = 0;
    _s_jdl_set_time.low = 0;
    _s_jdl_elap_time.high = 0;
    _s_jdl_elap_time.low = 0;
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief set JDL mode
 * @param[in]  mode : JDL mode
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_set_mode(u8 mode)
{
    u8 rtn = JDL_E_OK;
    
    if ((mode == JDL_MODE_DISABLE) || (mode == JDL_MODE_ENABLE))
    {
    /* Set JDL mode */
        _s_jdl_mode = mode;
    }
    else
    {
        rtn = JDL_E_PARAM;
    }
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief get JDL mode
 * @return     JDL mode
 */
/***********************************************************************/
u8 _jdl_get_mode(void)
{
    return _s_jdl_mode;
}


/***********************************************************************/
/**
 * @brief get JDL revision
 * @param[out] rev : JDL revision
 * @return     none
 */
/***********************************************************************/
void _jdl_get_rev(u16 *rev)
{
    *rev = _s_jdl_rev;
}


/***********************************************************************/
/**
 * @brief get elapsed time (10msec)
 * @param[out] time : elapsed time
 * @return     none
 */
/***********************************************************************/
void _jdl_get_10m_etime(u32 *time)
{
    *time = _s_jdl_10m_time;
}


/***********************************************************************/
/**
 * @brief get elapsed time (100msec)
 * @param[out] time : elapsed time
 * @return     none
 */
/***********************************************************************/
void _jdl_get_100m_etime(u32 *time)
{
    *time = _s_jdl_100m_time;
}


/***********************************************************************/
/**
 * @brief get current time (1sec)
 * @param[out] ctime : current time
 * @return     none
 */
/***********************************************************************/
void _jdl_get_1s_ctime(JDL_TIME *ctime)
{
    if ((_s_jdl_set_time.high > (0xFFFFFFFF - _s_jdl_elap_time.high))
     || ((_s_jdl_set_time.high == (0xFFFFFFFF - _s_jdl_elap_time.high))
      && (_s_jdl_set_time.low > (0xFFFFFFFF - _s_jdl_elap_time.low))))
    {
        ctime->high = 0xFFFFFFFF;
        ctime->low = 0xFFFFFFFF;
    }
    else
    {
        ctime->high = (_s_jdl_set_time.high + _s_jdl_elap_time.high);
        if (_s_jdl_set_time.low > (0xFFFFFFFF - _s_jdl_elap_time.low))
        {
            ctime->high += 1;
            ctime->low = (_s_jdl_set_time.low - (0xFFFFFFFF - _s_jdl_elap_time.low));
        }
        else
        {
            ctime->low = (_s_jdl_set_time.low + _s_jdl_elap_time.low);
        }
    }
}


/***********************************************************************/
/**
 * @brief get time (1sec)
 * @param[out] stime : set time
 * @param[out] etime : elapsed time
 * @return     none
 */
/***********************************************************************/
void _jdl_get_time(JDL_TIME *stime, JDL_TIME *etime)
{
    stime->high = _s_jdl_set_time.high;
    stime->low = _s_jdl_set_time.low;
    etime->high = _s_jdl_elap_time.high;
    etime->low = _s_jdl_elap_time.low;
}


/***********************************************************************/
/**
 * @brief set time (1sec)
 * @param[in]  stime : set time
 * @param[in]  etime : elapsed time
 * @return     none
 */
/***********************************************************************/
void _jdl_set_time(JDL_TIME *stime, JDL_TIME *etime)
{
    _s_jdl_tick_cnt_10m = 0;
    _s_jdl_tick_cnt_100m = 0;
    _s_jdl_tick_cnt_1s = 0;
    
    _s_jdl_10m_time = 0;
    _s_jdl_100m_time = 0;
    
    _s_jdl_set_time.high = stime->high;
    _s_jdl_set_time.low = stime->low;
    _s_jdl_elap_time.high = etime->high;
    _s_jdl_elap_time.low = etime->low;
}


/***********************************************************************/
/**
 * @brief draw energization time (1sec)
 * @param[in]  time : set time
 * @return     none
 */
/***********************************************************************/
void _jdl_draw_ener_time(u32 *time)
{
    *time = _s_jdl_ener_time;
    _s_jdl_ener_time = 0;
}


/***********************************************************************/
/**
 * @brief increment the log data for BYTE(8bit) type
 * @param[in]  offset : offset of data to increment
 * @param[in]  *data : pointer of data to store
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_inc_8bit(u32 offset, u8 *data)
{
    u8 rtn = JDL_E_OK;
    
    /* Load data */
    rtn = _jdl_load(offset, JDL_DATA_TYPE_SIZE_BYTE, data);
    if (rtn == JDL_E_OK)
    {
        if (*data == 0xFF)
        {
        /* Counter Max */
            rtn = JDL_E_CMAX;
        }
        else
        {
            (*data)++;
        }
    }
    /* Save data */
    rtn = _jdl_save(offset, JDL_DATA_TYPE_SIZE_BYTE, data);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief increment the log data for WORD(16bit) type
 * @param[in]  offset : offset of data to increment
 * @param[in]  *data : pointer of data to store
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_inc_16bit(u32 offset, u16 *data)
{
    u8 rtn = JDL_E_OK;
    u16 load_data;
    
    /* Load data */
    rtn = _jdl_load_word(offset, &load_data);
    if (rtn == JDL_E_OK)
    {
        if (load_data == 0xFFFF)
        {
        /* Counter Max */
            rtn = JDL_E_CMAX;
        }
        else
        {
            load_data++;
            *data = _JDL_SWAP_16(load_data);
        }
    }
    /* Save data */
    rtn = _jdl_save_word(offset, load_data);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief increment the log data for DWORD(32bit) type
 * @param[in]  offset : offset of data to increment
 * @param[in]  *data : pointer of data to store
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_inc_32bit(u32 offset, u32 *data)
{
    u8 rtn = JDL_E_OK;
    u32 load_data;
    
    /* Load data */
    rtn = _jdl_load_dword(offset, &load_data);
    if (rtn == JDL_E_OK)
    {
        if (load_data == 0xFFFFFFFF)
        {
        /* Counter Max */
            rtn = JDL_E_CMAX;
        }
        else
        {
            load_data++;
            *data = _JDL_SWAP_32(load_data);
        }
    }
    /* Save data */
    rtn = _jdl_save_dword(offset, load_data);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief increment the log data for DWORD(32bit) type
 * @param[in]  offset : offset of data to increment
 * @param[in]  *data : pointer of data to store
 * @param[in]  *data_2 : pointer of data to store
 * @return     error cord
 */
/***********************************************************************/
#if 0
u8 _jdl_inc_64bit(u32 offset, u32 *data, u32 *data_2)
{
    u8 rtn = JDL_E_OK;
    u32 load_data;
    
    /* Load data */
    rtn = _jdl_load_dword(offset, &load_data, JDL_DATA_TYPE_SIZE_BYTE);
    if (rtn == JDL_E_OK)
    {
        if (load_data == 0xFFFFFFFF)
        {
        /* Counter Max */
            rtn = JDL_E_CMAX;
        }
        else
        {
            load_data++;
            *data = _JDL_SWAP_32(load_data);
        }
    }
    
    return rtn;
}
#endif


/***********************************************************************/
/**
 * @brief add value to the log data for BYTE(8bit) type.
 * @param[in]  offset : offset of data to add
 * @param[in]  add_data : data to add
 * @param[out] sum_data : pointer of sum data to store
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_add_8bit(u32 offset, u8 add_data, u8 *sum_data)
{
    u8 rtn = JDL_E_OK;
    
    /* Load data */
    rtn = _jdl_load(offset, JDL_DATA_TYPE_SIZE_BYTE, sum_data);
    if (rtn == JDL_E_OK)
    {
        if (*sum_data == 0xFF)
        {
        /* Counter Max */
            rtn = JDL_E_CMAX;
        }
        else if (add_data > (0xFF - *sum_data))
        {
        /* Overflow due to addition (Set max value) */
            *sum_data = 0xFF;
            rtn = JDL_E_COVR;
        }
        else
        {
            *sum_data += add_data;
        }
    }
    /* Save data */
    rtn = _jdl_save(offset, JDL_DATA_TYPE_SIZE_BYTE, sum_data);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief add value to the log data for WORD(16bit) type.
 * @param[in]  offset : offset of data to add
 * @param[in]  add_data : data to add
 * @param[out] sum_data : pointer of sum data to store
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_add_16bit(u32 offset, u16 add_data, u16 *sum_data)
{
    u8 rtn = JDL_E_OK;
    u16 load_data;
    
    /* Load data */
    rtn = _jdl_load_word(offset, &load_data);
    if (rtn == JDL_E_OK)
    {
        if (load_data == 0xFFFF)
        {
        /* Counter Max */
            rtn = JDL_E_CMAX;
        }
        else if (add_data > (0xFFFF - load_data))
        {
        /* Overflow due to addition (Set max value) */
            *sum_data = 0xFFFF;
            rtn = JDL_E_COVR;
        }
        else
        {
            load_data += add_data;
            *sum_data = _JDL_SWAP_16(load_data);
        }
    }
    /* Save data */
    rtn = _jdl_save_word(offset, load_data);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief add value to the log data for WORD(16bit) type.
 * @param[in]  offset : offset of data to add
 * @param[in]  add_data : data to add
 * @param[out] sum_data : pointer of sum data to store
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_add_32bit(u32 offset, u32 add_data, u32 *sum_data)
{
    u8 rtn = JDL_E_OK;
    u32 load_data;
    
    /* Load data */
    rtn = _jdl_load_dword(offset, &load_data);
    if (rtn == JDL_E_OK)
    {
        if (load_data == 0xFFFFFFFF)
        {
        /* Counter Max */
            rtn = JDL_E_CMAX;
        }
        else if (add_data > (0xFFFFFFFF - load_data))
        {
        /* Overflow due to addition (Set max value) */
            *sum_data = 0xFFFFFFFF;
            rtn = JDL_E_COVR;
        }
        else
        {
            load_data += add_data;
            *sum_data = _JDL_SWAP_32(load_data);
        }
    }
    /* Save data */
    rtn = _jdl_save_dword(offset, load_data);
    
    return rtn;
}


/***********************************************************************/
/**
 * @brief add value to the log data for WORD(16bit) type.
 * @param[in]  offset : offset of data to add
 * @param[in]  add_data : data to add
 * @param[out] sum_data : pointer of sum data to store
 * @return     error cord
 */
/***********************************************************************/
#if 0
u8 _jdl_add_64bit(u32 offset, u32 add_data1, u32 add_data2, u32 *sum_data1, u32 *sum_data2)
{
    u8 rtn = JDL_E_OK;
    u8 rtn2 = JDL_E_OK;
    u32 cnt;
    
    /* Load data */
    rtn = _jdl_load_dword(offset, sum_data1);
    rtn2 = _jdl_load_dword((offset + JDL_DATA_TYPE_SIZE_DWORD), sum_data2);
    if ((rtn == JDL_E_OK) && (rtn == JDL_E_OK))
    {
        if ((*sum_data1 == 0xFFFFFFFF)
         && (*sum_data2 == 0xFFFFFFFF))
        {
            rtn = JDL_E_CMAX;
        }
        else if ((add_data1 > (0xFFFFFFFF - *sum_data1))
            || ((add_data1 == (0xFFFFFFFF - *sum_data1))
            && (add_data2 > (0xFFFFFFFF - *sum_data2))))
        {
            *sum_data1 = 0xFFFFFFFF;
            *sum_data2 = 0xFFFFFFFF;
            rtn = JDL_E_COVR;
        }
        else
        {
            *sum_data1 += add_data1;
            if (add_data2 > (0xFFFFFFFF - *sum_data2))
            {
                *sum_data1 += 1;
                //cnt_low = ((cnt_low + val_low) - 0xFFFFFFFF);
                *sum_data2 = (add_data2 - (0xFFFFFFFF - *sum_data2));
            }
            else
            {
                *sum_data2 += add_data2;
            }
        }
    }
    
    return rtn;
}
#endif


/***********************************************************************/
/**
 * @brief fill memory with a constant byte
 * @param[out] ptr  : pointer to the block to fill
 * @param[in]  data : fill byte
 * @param[in]  size : number of bytes to fill
 * @return     none
 */
/***********************************************************************/
void _jdl_memset(u8 *ptr, u8 data, u32 size)
{
    while (size--)
    {
        *ptr++ = data;
    }
}


/***********************************************************************/
/**
 * @brief copy memory area
 * @param[out] dptr : pointer to the block to copy to
 * @param[in]  sptr : pointer to the block to copy from
 * @param[in]  size : number of bytes to copy
 * @return     none
 */
/***********************************************************************/
void _jdl_memcpy(u8 *dptr, u8 *sptr, u32 size)
{
    while (size--)
    {
        *dptr++ = *sptr++;
    }
}


#if 0
/***********************************************************************/
/**
 * @brief compare two areas of memory
 * @param[in]  cs : One area of memory
 * @param[in]  ct : Another area of memory
 * @param[in]  size : The size of the area
 * @return     none
 */
/***********************************************************************/
void _jdl_memcmp(u8 *cs, u8 *ct, u32 size)
{
    while (size--)
    {
        *dptr++ = *sptr++;
    }
}
#endif


