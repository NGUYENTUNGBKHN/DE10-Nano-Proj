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
#include "common.h"

#define EXT
#include "com_ram.c"


#include  "kernel_inc.h" // タスクID Toolで自動的に設定された各タスク、メッセージBOXのID番号


#if defined(_JDL_USE_OP_RC)

/************************** BACKUP VARIABLES ***************************/


/************************** PRIVATE DEFINITIONS ************************/
#define	JDL_OPRC_OFS_CSV_TWIN_SENSOR_TOP			1688
#define	JDL_OPRC_OFS_CSV_QUAD_SENSOR_TOP			1888
#define	JDL_OPRC_OFS_CSV_PERFORMANCE_TOP			2088
#define	JDL_OPRC_OFS_ADJ_SENSOR_TOP					2160
#define	JDL_OPRC_OFS_CSV_ADJ_SENSOR_TOP				2288
#define	JDL_OPRC_OFS_CSV_RESERVED_TOP				2416
#define	JDL_OPRC_OFS_OPERATION_LOG_TOP				16030


#define	JDL_OPRC_OFS_CSV_TWIN_SENSOR_SIZE			200
#define	JDL_OPRC_OFS_CSV_QUAD_SENSOR_SIZE			200
#define	JDL_OPRC_OFS_CSV_PERFORMANCE_SIZE			72
#define	JDL_OPRC_OFS_ADJ_SENSOR_SIZE				128
#define	JDL_OPRC_OFS_CSV_ADJ_SENSOR_SIZE			128
#define	JDL_OPRC_OFS_CSV_RESERVED_SIZE				13614


/************************** PRIVATE FUNCTIONS **************************/
static void _jdl_oprc_clear(void);


/************************** PRIVATE VARIABLES **************************/
static u16 _s_jdl_oprc_rev;


/************************** EXTERN FUNCTIONS ***************************/


/************************** EXTERN VARIABLES ***************************/



/***********************************************************************/
/**
 * @brief initialize rc category
 * @param[in]  clear : force clear setting
 * @return     error cord
 */
/***********************************************************************/
u8 _jdl_oprc_init(u8 clear)
{
    u8 rtn = JDL_E_OK;
    
    if (_s_jdl_oprc_rev != _JDL_SWAP_16(JDL_OPRC_REV))
    {
    /* Change revision */
        rtn = JDL_E_REVCHG;
    }
    
    if ((clear != 0)
     || (rtn != JDL_E_OK))
    {
    /* Clear log */
        _jdl_oprc_clear();
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
u8 _jdl_oprc_req(u32 s_offset, u32 buff_size)
{
	u8 rtn;
	u8 ofs1;
	u8 ofs2;
	u8 ofs3;
	u8 ofs4;
	
	if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_OPRC_SEND_TOTAL))
    {
        rtn = JDL_E_MACV;
    }
    else
    {
    	if(s_offset != 0)
    	{
    		s_offset = s_offset - 32;
    	}
#if defined (_JDL_OPRC_EXCLUDE_YOBI)
    	if(s_offset >= MRAM_ADDRESS_YOBI)
		{
			s_offset += MRAM_SIZE_YOBI;
		}
#endif
#if 1	/* '25-03-05 */
		if(s_offset == JDL_OPRC_OFS_CSV_TWIN_SENSOR_TOP
		|| s_offset == JDL_OPRC_OFS_CSV_QUAD_SENSOR_TOP
		|| s_offset == JDL_OPRC_OFS_CSV_PERFORMANCE_TOP
		|| s_offset == JDL_OPRC_OFS_CSV_ADJ_SENSOR_TOP)
//		|| s_offset == JDL_OPRC_OFS_CSV_RESERVED_TOP)
		{
			/* 不要領域はRCからデータ取得しない */
	    	rtn = JDL_E_OK;
		}
		else
		{
	    	ofs1 = (u8)(s_offset & 0x000000FF);
	    	ofs2 = (u8)((s_offset & 0x0000FF00) >> 8);
	    	ofs3 = (u8)((s_offset & 0x00FF0000) >> 16);
	    	ofs4 = (u8)((s_offset & 0xFF000000) >> 24);

	    	_dline_send_msg(ID_RC_MBX, TMSG_RC_FRAM_READ_REQ, ofs1, ofs2, ofs3, ofs4);

	    	rtn = JDL_E_BUSY;
		}
#else
    	ofs1 = (u8)(s_offset & 0x000000FF);
    	ofs2 = (u8)((s_offset & 0x0000FF00) >> 8);
    	ofs3 = (u8)((s_offset & 0x00FF0000) >> 16);
    	ofs4 = (u8)((s_offset & 0xFF000000) >> 24);
    		
    	_dline_send_msg(ID_RC_MBX, TMSG_RC_FRAM_READ_REQ, ofs1, ofs2, ofs3, ofs4);
    	
    	rtn = JDL_E_BUSY;
#endif
    }
	
	return (rtn);
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
u8 _jdl_oprc_get(u8 *buff, u32 buff_size, u32 s_offset, u32 *g_size)
{
    u8 rtn = JDL_E_OK;
    u8 category_header[JDL_SIZE_CATEGORY_HEADER];
    u32 w_size;
    u32 w_offset;
    u32 temp_wsize;
    u32 temp_offset;
    
    
    if (s_offset >= (JDL_SIZE_CATEGORY_HEADER + JDL_OPRC_SEND_TOTAL))
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
            _jdl_memcpy((void *)&category_header[JDL_CHEAD_ADR_NAME], "OP_RC", sizeof("OP_RC"));
            *((u16 *)&category_header[JDL_CHEAD_ADR_ID]) = _JDL_SWAP_16(JDL_CATE_ID_OP_RC);
            *((u16 *)&category_header[JDL_CHEAD_ADR_REV]) = _s_jdl_oprc_rev;
            *((u32 *)&category_header[JDL_CHEAD_ADR_CSIZE]) = _JDL_SWAP_32(((u32)JDL_SIZE_CATEGORY_HEADER + (u32)JDL_OPRC_SEND_TOTAL));
            
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
         && (w_offset < (JDL_SIZE_CATEGORY_HEADER + JDL_OPRC_SEND_TOTAL)))
        {
#if 1	/* '25-03-05 */
			switch(s_offset)
			{
			case	JDL_OPRC_OFS_CSV_TWIN_SENSOR_TOP:
					_jdl_memset((buff + w_size), 0, JDL_OPRC_OFS_CSV_TWIN_SENSOR_SIZE);
		            w_size += JDL_OPRC_OFS_CSV_TWIN_SENSOR_SIZE;
					break;
			case	JDL_OPRC_OFS_CSV_QUAD_SENSOR_TOP:
					_jdl_memset((buff + w_size), 0, JDL_OPRC_OFS_CSV_QUAD_SENSOR_SIZE);
		            w_size += JDL_OPRC_OFS_CSV_QUAD_SENSOR_SIZE;
					break;
			case	JDL_OPRC_OFS_CSV_PERFORMANCE_TOP:
					_jdl_memset((buff + w_size), 0, JDL_OPRC_OFS_CSV_PERFORMANCE_SIZE);
		            w_size += JDL_OPRC_OFS_CSV_PERFORMANCE_SIZE;
					break;
			case	JDL_OPRC_OFS_CSV_ADJ_SENSOR_TOP:
					_jdl_memset((buff + w_size), 0, JDL_OPRC_OFS_CSV_ADJ_SENSOR_SIZE);
		            w_size += JDL_OPRC_OFS_CSV_ADJ_SENSOR_SIZE;
					break;
//			case	JDL_OPRC_OFS_CSV_RESERVED_TOP:
//					_jdl_memset((buff + w_size), 0, JDL_OPRC_OFS_CSV_RESERVED_SIZE);
//		            w_size += JDL_OPRC_OFS_CSV_RESERVED_SIZE;
//					break;
			default:
		        	_jdl_memcpy((buff + w_size), (u8 *)&rc_fram_log.read_data[0], rc_fram_log.read_length);
		            w_size += rc_fram_log.read_length;
					break;
			}
#else
        	_jdl_memcpy((buff + w_size), (u8 *)&rc_fram_log.read_data[0], rc_fram_log.read_length);
            w_size += rc_fram_log.read_length;
#endif
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
static void _jdl_oprc_clear(void)
{    
    /* Category revision */
    _s_jdl_oprc_rev = _JDL_SWAP_16(JDL_OPRC_REV);
}


#endif  /* _JDL_USE_RC */


