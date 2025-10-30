/******************************************************************************/
/*! @addtogroup Main
    @file       download.c
    @date       2018/01/31
    @author     suzuki-hiroyuki
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************/
/*
 * download.c
 *
 *  Created on: 2018/01/31
 *      Author: suzuki-hiroyuki
 */

/***************************** Include Files *********************************/
#include <string.h>
#include "js_oswapi.h"
#include "struct.h"
#include "download.h"
#include "memorymap.h"
#include "crc.h"
#include "hal_wdt.h"

#ifdef  FLASH_QSPI_MODE
#include "alt_qspi.h"
#endif



#define EXT
#include "com_ram.c"
#include "jsl_ram.c"
#include "../task/cline_task/003/js_uart_id003.h"
#undef EXT


/************************** PRIVATE DEFINITIONS *************************/


/************************** EXTERNAL FUNCTIONS *************************/


/************************** EXTERNAL VARIABLES *************************/

/***************** Macros (Inline Functions) Definitions *********************/

/* Boot I/F先頭アドレス                                   */
#define D_ADRST_BOOTIF_PRG       ROM_ALIAS_START_ADDRESS
/* Boot I/F Versionまでのオフセット                       */
#define D_DOWNLOADFILE_VER_POINT 0x18
/* バッファサイズ                                         */
#define D_DOWNLOAD_BUFF_SIZE     0x01000000ul

/* セクション毎CRCチェック結果 */
#define D_SEC_CRC_OK           0
#define D_SEC_CRC_NG           1
#define D_SEC_CRC_NO_BASE      2

#define SEC_MAX_NUM            32
u8 ex_section_crc[SEC_MAX_NUM];
T_DL_SECTION_INFO s_section_info;
T_DL_SECTION_INFO s_internal_section_info;
T_DL_CHK_FLG s_dl_check_flg;

u32 s_dl_start_address;
u32 s_dl_end_address;
u32 s_dl_write_address;
u8 *s_dl_data_buff;
u32 s_dl_buff_offset;
u32 s_dl_file_offset;
u8 s_dl_end_status;


/************************** Function Prototypes ******************************/
static u8 _check_file_header( u32 offset );
static u8 _check_rom_header( void );
static u8 _check_first_section_header( void );
static u8 _check_section_header( void );
static u8 _check_bif_section_header( void );
void _write_proc( void );
void _write_proc_for_BifSection( void );

/*********************************************************************//**
 * @brief		BIF CPU self reset
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void _reset(void)
{
	/* CPU Reset */
	while( 1 );
}
/*********************************************************************//**
 * @brief		initialize variables
 * @param[in]	None
 * @return 		None
 **********************************************************************/
void init_download_variable( void )
{
    /* Signatureチェックフラグ                  */
	s_dl_check_flg.ChkSignature     = D_SIG_CHECK_DO_NOT;
	s_dl_check_flg.ChkRomHeader     = D_ROM_CHECK_DO_NOT;
	s_dl_check_flg.ChkBifSection    = D_SEC_CHECK_DO_NOT;
	s_dl_check_flg.ChkSectionHeader = D_SEC_CHECK_DO_NOT;
	s_section_info.start_adr      = 0;
	s_section_info.end_adr        = 0;
	s_section_info.size           = 0;
	s_section_info.rcv_size       = 0;
    /* BIF 2重化セクション用構造体 */
	s_internal_section_info.start_adr      = 0;
	s_internal_section_info.end_adr        = 0;
	s_internal_section_info.size           = 0;
    s_internal_section_info.rcv_size       = 0;

    /* ダウンロード受信データバッファ設定   */
	s_dl_data_buff         = (u8 *)DOWNLOAD_IMAGE_BUF;
    /* ダウンロード受信データバッファで使用 するオフセット                       */
    s_dl_buff_offset        = 0;
    /* DL先頭アドレス                       */
    s_dl_start_address      = 0;
    /* DL書き込みアドレス                   */
    s_dl_write_address      = 0;
    /* DLファイルオフセットアドレス         */
    s_dl_file_offset        = 0;

    /* ダウンロード完了状態                 */
    s_dl_end_status          = D_DL_NOT_COMPLETION;

    return;
}

/*------------------------------------------------------*/
/*  Flashメモリへの書き込み                             */
/*  <引数> なし                                         */
/*  <返値> TRUE:成功                                         */
/*  <返値> FALSE:失敗                                         */
/*------------------------------------------------------*/
static u8 _write_flash( void )
{
	ALT_STATUS_CODE stat;
	UINT32 i;
	QSPI_BUF_INFO info;
	u8 page_buffer[256];

	/* ROM Area protection */
	if( (s_dl_write_address < ROM_ALIAS_START_ADDRESS - DDR_START_ADDRESS)
	 || (s_dl_write_address + s_dl_buff_offset - 1 > ROM_ALIAS_END_ADDRESS - DDR_START_ADDRESS))
	{
		return(FALSE);
	}
	if(s_dl_write_address % 256)
	{
		info.buf = (u8 *)page_buffer;
		info.addr = s_dl_write_address - (s_dl_write_address % 256);
		info.len = 256;
		info.byte_count = &i;
#ifdef  FLASH_QSPI_MODE
		stat = alt_qspi_read((void *)info.buf
				, (uint32_t)info.addr
				, info.len);
		if(stat != ALT_E_SUCCESS)
		{
			return false;
		}
#else
		if( QSPI_Flash_Read( &hQFlash, &info ) == FALSE ){
			return(FALSE);
		};
#endif
		memcpy(&page_buffer[s_dl_write_address % 256],s_dl_data_buff,s_dl_buff_offset);
		info.buf = (u8 *)page_buffer;
		info.addr = s_dl_write_address - (s_dl_write_address % 256);
		info.len = 256;
		info.byte_count = &i;
#ifdef  FLASH_QSPI_MODE
		stat = alt_qspi_replace(info.addr
				, (u8 *)info.buf
				, info.len
				, (char *)QSPI_REPLACE_BUF
				, QSPI_REPLACE_SIZE);
		if(stat != ALT_E_SUCCESS)
		{
			return false;
		}
#else
		if( QSPI_Flash_Write( &hQFlash, &info, QSPI_FLASH_ERASE_AUTO ) == FALSE ){
			return(FALSE);
		};
#endif
	}
	else
	{
		info.buf = (u8 *)s_dl_data_buff;
		info.addr = s_dl_write_address;
		info.len = s_dl_buff_offset;
		info.byte_count = &i;
#ifdef  FLASH_QSPI_MODE
		stat = alt_qspi_replace(info.addr
				, (u8 *)info.buf
				, info.len
				, (char *)QSPI_REPLACE_BUF
				, QSPI_REPLACE_SIZE);
		if(stat != ALT_E_SUCCESS)
		{
			return false;
		}
#else
		if( QSPI_Flash_Write( &hQFlash, &info, QSPI_FLASH_ERASE_AUTO ) == FALSE ){
			return(FALSE);
		};
#endif
	}

	return(TRUE);
}

/*------------------------------------------------------*/
/*  Flashメモリへの書き込みのシーケンス処理             */
/*  <引数> なし                                         */
/*  <返値> なし                                         */
/*------------------------------------------------------*/
static void _write_flash_sequence( u32 offset )
{
    u8  result = 0;

#if defined(_PROTOCOL_ENABLE_ID003)
	while( UART_is_txd_active() == 1 );
#elif defined(_PROTOCOL_ENABLE_ID0G8)
#endif

	/* 書き込み処理 */
	result = _write_flash();

	/* 書き込み失敗の場合 */
	if( FALSE == result )
	{
		/* ダウンロード失敗状態 */
		s_dl_end_status = D_DL_FAILURE;
	}
	/* 書き込みｱﾄﾞﾚｽ進める     */
	s_dl_write_address   += s_dl_buff_offset;
	/* 最終アドレスまで到達した場合 */
	if( s_dl_write_address >= s_dl_end_address - DDR_START_ADDRESS )
	{
	   /* ダウンロードファイルoffsetアドレスを */
	   /* all 0xfに設定                        */
	   s_dl_file_offset = 0xffffffff;
	}

	/* 余りがある場合 */
	if( 0 != offset )
	{
		/* 受信バッファの先頭へ移動 */
		memcpy( s_dl_data_buff, (s_dl_data_buff + s_dl_buff_offset), offset );
		s_dl_buff_offset = offset;
	}
	else
	{
		s_dl_buff_offset = 0;
	}
}

/*********************************************************************//**
 * @brief		Verify all section crc
 * @param[in]	crc_reslut program crc check OK or not.
 * @return 		Check result.
 * @retval D_SEC_CRC_OK No CRC error
 * @retval D_SEC_CRC_NG CRC error
 * @retval D_SEC_CRC_NO_BASE Base section not found.
 **********************************************************************/
u8 _check_all_section_crc( u8 crc_result )
{
    u8  ret = D_SEC_CRC_NO_BASE;
    u32 tbl_adr = BIF_SECTION_BASE;
    u32 crc_start_adr;
    u32 crc_end_adr;
    u16 calc_crc;
    u16 flash_crc;
    u8  section_cnt = 0;
    u8  ii;

    while( 1 )
    {
        /*-------------------*/
        /* Header IDチェック */
        /*-------------------*/
    	/* 検索用VFM20 */
#if (defined(PRJ_IVIZION2) && (BV_UNIT_TYPE>=WS2_MODEL))
		if( (*(u8 *)(tbl_adr    ) != 'V') ||
			(*(u8 *)(tbl_adr + 1) != 'F') ||
			(*(u8 *)(tbl_adr + 2) != 'M') ||
			(*(u8 *)(tbl_adr + 3) != '2') ||
			(*(u8 *)(tbl_adr + 4) != '0') )
#else
		if( (*(u8 *)(tbl_adr    ) != 'V') ||
			(*(u8 *)(tbl_adr + 1) != 'F') ||
			(*(u8 *)(tbl_adr + 2) != 'M') ||
			(*(u8 *)(tbl_adr + 3) != '2') ||
			(*(u8 *)(tbl_adr + 4) != '1') )
#endif
        {
            /* HeaderID異常なら以降のデータの信頼性もなく、 */
            /* 従って次のセクションのアドレスも分からない   */
            break;
        }

        crc_start_adr = *(u8 *)(tbl_adr + D_ENTRY_TBL_OFFSET_STARTADR);
        crc_start_adr = *(u8 *)(tbl_adr + D_ENTRY_TBL_OFFSET_STARTADR + 1) + (crc_start_adr << 8);
        crc_start_adr = *(u8 *)(tbl_adr + D_ENTRY_TBL_OFFSET_STARTADR + 2) + (crc_start_adr << 8);
        crc_start_adr = *(u8 *)(tbl_adr + D_ENTRY_TBL_OFFSET_STARTADR + 3) + (crc_start_adr << 8);

        crc_end_adr   = *(u8 *)(tbl_adr + D_ENTRY_TBL_OFFSET_ENDADR);
        crc_end_adr   = *(u8 *)(tbl_adr + D_ENTRY_TBL_OFFSET_ENDADR   + 1) + (crc_end_adr   << 8);
        crc_end_adr   = *(u8 *)(tbl_adr + D_ENTRY_TBL_OFFSET_ENDADR   + 2) + (crc_end_adr   << 8);
        crc_end_adr   = *(u8 *)(tbl_adr + D_ENTRY_TBL_OFFSET_ENDADR   + 3) + (crc_end_adr   << 8);

        /* section no以降のデータを使用し、CRC16計算する */
        crc_start_adr = crc_start_adr + D_ENTRY_TBL_OFFSET_SECTION_NO + DDR_START_ADDRESS;
        crc_end_adr   = crc_end_adr + DDR_START_ADDRESS;

        /* アドレス異常チェック */
        if( (crc_start_adr < ROM_ALIAS_START_ADDRESS) || (crc_start_adr > ROM_ALIAS_END_ADDRESS) ||
            (crc_end_adr   < ROM_ALIAS_START_ADDRESS) || (crc_end_adr   > ROM_ALIAS_END_ADDRESS) ||
            (crc_start_adr >= crc_end_adr) )
        {
            /* アドレス異常なので次のセクションのアドレスも分からない */
            break;
        }

        if( D_NG == crc_result )
        {
            /* WDTクリア   */
        	_hal_feed_wdt();
            /* CRC算出 */
            calc_crc = _calc_crc( (u8 *)crc_start_adr,
                                         (u32)crc_end_adr - crc_start_adr + 1,
                                         true );
            /* WDTクリア   */
            _hal_feed_wdt();
            /* Flashに保存されているCRCの処理 */
            flash_crc = (*(u8 *)(tbl_adr + D_ENTRY_TBL_OFFSET_CRC16) << 8) +
                        (*(u8 *)(tbl_adr + D_ENTRY_TBL_OFFSET_CRC16 + 1));

            /* 一致するか比較 */
            if( calc_crc != flash_crc )
            {
                /* CRC異常        */
                ex_section_crc[section_cnt] = D_SEC_CRC_NG;
            }
            else
            {
                /* CRC正常        */
                ex_section_crc[section_cnt] = D_SEC_CRC_OK;
            }
        }
        else
        {
            /* CRC正常        */
            ex_section_crc[section_cnt] = D_SEC_CRC_OK;
        }

        section_cnt++;

        /*----------------------------------*/
        /* 最終セクションかチェック         					*/
        /* (最終セクションはCNTRYセクション) 				*/
        /*----------------------------------*/
        if( (*(u8 *)(tbl_adr + D_ENTRY_TBL_OFFSET_SECTION_NO)   == 0xFF) &&
            (*(u8 *)(tbl_adr + D_ENTRY_TBL_OFFSET_SECTION_NO+1) == 0xFF) )
        {
        	/* 最終セクションのCRCがOKだった場合 */
            if( ex_section_crc[(section_cnt-1)] == D_SEC_CRC_OK )
            {
                ret = D_SEC_CRC_OK;
            }
            else
            {
                ret = D_SEC_CRC_NG;
            }
            break;
        }
        tbl_adr = crc_end_adr + 1;
    }

    /* 以降のセクション全てNG */
    for( ii =section_cnt; ii< SEC_MAX_NUM; ii++ )
    {
        /* CRC異常            */
        ex_section_crc[ii] = D_SEC_CRC_NG;
    }

    return( ret );
}

/*------------------------------------------------------*/
/*  ダウンロードデータコピー処理 　                     */
/*  <引数> なし                                         */
/*  <返値> なし                                         */
/*------------------------------------------------------*/
void download_copy( u8* dst, u8 length )
{
    /* 受信バッファからデータ格納用バッファへコピー */
	memcpy( s_dl_data_buff + s_dl_buff_offset, dst, length );
    /* バッファオフセットアドレス更新 */
    s_dl_buff_offset = s_dl_buff_offset + length;
    /* セクション受信サイズ更新 */
    s_section_info.rcv_size = s_section_info.rcv_size + length;
}

/*********************************************************************//**
 * @brief		return crc result
 * @return 		Check result.
 * @retval      D_DL_COMPLENTION No CRC error
 * @retval      D_DL_FAILURE CRC error
 **********************************************************************/
u16 get_dl_end_status( void )
{
    return s_dl_end_status;
}
/*********************************************************************//**
 * @brief		Calculate and verify all section crc
 * @param[in]	pragram_crc result var address
 * @param[in]	file_crc program result var address
 * @return 		Check result.
 * @retval      D_DL_COMPLENTION No CRC error
 * @retval      D_DL_FAILURE CRC error
 **********************************************************************/
u16 dl_calc_crc( u16 *pragram_crc, u16 *file_crc )
{
    u16 tmp_crc;

	*pragram_crc = _calc_crc( (u8 *)s_dl_start_address, s_dl_end_address-s_dl_start_address-2+1, TRUE );
	*file_crc = _calc_crc( (u8 *)s_dl_end_address-1, 2,   FALSE );

    tmp_crc = *(u8 *)(s_dl_end_address-1);
    tmp_crc = (tmp_crc << 8) & 0xff00;
    tmp_crc = tmp_crc + *(u8 *)(s_dl_end_address);

    if( tmp_crc == *pragram_crc )
    {
        /* ダウンロード正常完了状態 */
    	s_dl_end_status = D_DL_COMPLENTION;
    }
    else
    {
        /* ダウンロード失敗状態 */
    	s_dl_end_status = D_DL_FAILURE;
    }

    return s_dl_end_status;
}

/*********************************************************************//**
 * @brief		Verify download data address
 * @param[in]	comp_address program data address.
 * @return 		Same address or not.
 * @retval      1 OK
 * @retval      0 error
 **********************************************************************/
u8 dl_check_download_address(u32 comp_address)
{
    if( comp_address == s_dl_file_offset )
    {
        return 1;
    }
    return 0;
}

/*********************************************************************//**
 * @brief		stored download data check
 * @param[in]	NONE
 * @return 		OK or NG.
 * @retval      RET_OK
 * @retval      RET_NG
 **********************************************************************/
u8 download_data_check( void )
{
    u8 ret = RET_OK;

    /* ダウンロードファイルのoffsetアドレス算出 */
    /* ROM Headerまで受信完了している場合   */
    if( D_DL_ROM_HEADER_SIZE <= s_dl_file_offset )
    {
        s_dl_file_offset = (s_dl_buff_offset + s_dl_write_address) - (s_dl_start_address - DDR_START_ADDRESS);
    }
    else
    {
    	s_dl_file_offset = s_dl_buff_offset;
    }

    /*********************/
    /* Signatureチェック */
    /*********************/
    if( s_dl_check_flg.ChkSignature == D_SIG_CHECK_DO_NOT )
    {
        /* Signatureチェック処理     */
        s_dl_check_flg.ChkSignature = _check_file_header(0);

        /* SignatureがFALSEの場合？  */
        if( s_dl_check_flg.ChkSignature == D_SIG_CHECK_NG )
        {
            /* ﾚｽﾎﾟﾝｽﾒｯｾｰｼﾞNG */
            ret = RET_NG ;
        }
    }
    /*********************/
    /* ROMヘッダチェック */
    /*********************/
    /* Signatureチェック済みでVersionチェック未実施の場合 */
    if( ( s_dl_check_flg.ChkRomHeader == D_ROM_CHECK_DO_NOT) &&
        ( s_dl_check_flg.ChkSignature == D_SIG_CHECK_OK ))
    {
        /* ROMヘッダチェック                              */
        ret = _check_rom_header();
    }
    /******************************/
    /* FirstSectionヘッダチェック */
    /******************************/
    if( (( s_dl_check_flg.ChkRomHeader    == D_ROM_CHECK_WAIT_WRITE ) ||
        ( s_dl_check_flg.ChkRomHeader     == D_ROM_CHECK_FIRST_SECTION )) &&
        ( s_dl_check_flg.ChkSignature     == D_SIG_CHECK_OK ) &&
        ( s_dl_check_flg.ChkSectionHeader == D_SEC_CHECK_DO_NOT ))
    {
        ret = _check_first_section_header();
    }
    /*************************/
    /* Sectionヘッダチェック */
    /*************************/
    if( ( s_dl_check_flg.ChkRomHeader     == D_ROM_CHECK_OK ) &&
        ( s_dl_check_flg.ChkSignature     == D_SIG_CHECK_OK ) &&
        ( s_dl_check_flg.ChkSectionHeader == D_SEC_CHECK_DO_NOT ))
    {
        ret = _check_section_header();
    }
    /*************************/
    /* BIF1ヘッダチェック */
    /*************************/
    if( ( s_dl_check_flg.ChkRomHeader     == D_ROM_CHECK_OK ) &&
        ( s_dl_check_flg.ChkSignature     == D_SIG_CHECK_OK ) &&
        ( s_dl_check_flg.ChkSectionHeader  == D_SEC_CHECK_OK ) &&
        ( s_dl_check_flg.ChkBifSection == D_BIF_CHECK_DO_NOT ))
    {
        ret = _check_bif_section_header();
    }

	return ret;
}

/*------------------------------------------------------*/
/*  FLASHとダウンロードデータが一致しているかチェック   */
/*  <引数> なし                                         */
/*  <返値> D_HEADER_CHECK_NG:不一致                     */
/*         D_HEADER_CHECK_OK:一致                       */
/*------------------------------------------------------*/
static u8 _check_header( u32 offset )
{
	u8 DucCount;
    for( DucCount = 0; DucCount < D_DL_SECTION_HEADER_SIZE; DucCount++ )
    {
		if(*(u8 *)(s_section_info.start_adr + DucCount ) != s_dl_data_buff[offset + DucCount])
		{
			return( D_HEADER_CHECK_NG );
		}
	}
	return( D_HEADER_CHECK_OK );
}


/*------------------------------------------------------*/
/*  CRC32が一致しているかチェック                       */
/*  <引数> なし                                         */
/*  <返値> D_CRC32_CHECK_NG:不一致                      */
/*         D_CRC32_CHECK_OK:一致                        */
/*------------------------------------------------------*/
static u8 _check_header_crc32( u32 offset )
{
    /* CRC32が一致しているかチェック */
    if(  *(u8 *)(s_section_info.start_adr +8 ) != s_dl_data_buff[offset + D_ENTRY_TBL_OFFSET_CRC32]    ||
         *(u8 *)(s_section_info.start_adr +9 ) != s_dl_data_buff[offset + D_ENTRY_TBL_OFFSET_CRC32 +1] ||
         *(u8 *)(s_section_info.start_adr +10) != s_dl_data_buff[offset + D_ENTRY_TBL_OFFSET_CRC32 +2] ||
         *(u8 *)(s_section_info.start_adr +11) != s_dl_data_buff[offset + D_ENTRY_TBL_OFFSET_CRC32 +3] )
    {
        return( D_CRC32_CHECK_NG );
    }
    else
    {
        return( D_CRC32_CHECK_OK );
    }
}

/*------------------------------------------------------*/
/*  ﾀﾞｳﾝﾛｰﾄﾞﾌｧｲﾙ先頭のﾍｯﾀﾞのﾁｪｯｸ                        */
/*  <引数> なし                                         */
/*  <返値> TRUE: ﾌｧｲﾙﾍｯﾀﾞﾁｪｯｸOK                         */
/*         FALSE:ﾌｧｲﾙﾍｯﾀﾞﾁｪｯｸNG                         */
/*------------------------------------------------------*/
static u8 _check_file_header( u32 offset )
{
    /* 受信したデータが5以上の場合？ */
    if( 5 <= s_dl_file_offset )
    {
    	/* 検索用VFM20 */
#if (defined(PRJ_IVIZION2) && (BV_UNIT_TYPE>=WS2_MODEL))
		if( s_dl_data_buff[offset + 0] != 'V' ||
			s_dl_data_buff[offset + 1] != 'F' ||
			s_dl_data_buff[offset + 2] != 'M' ||
			s_dl_data_buff[offset + 3] != '2' ||
			s_dl_data_buff[offset + 4] != '0' )
#else
		if( s_dl_data_buff[offset + 0] != 'V' ||
			s_dl_data_buff[offset + 1] != 'F' ||
			s_dl_data_buff[offset + 2] != 'M' ||
			s_dl_data_buff[offset + 3] != '2' ||
			s_dl_data_buff[offset + 4] != '1' )
#endif
        {
            return( D_SIG_CHECK_NG );
        }
        else
        {
            return( D_SIG_CHECK_OK );
        }
    }
    else
    {
        return( D_SIG_CHECK_DO_NOT );
    }
}

/*------------------------------------------------------*/
/*  ROMﾍｯﾀﾞのﾁｪｯｸ                                       */
/*  <引数> なし                                         */
/*  <返値> TRUE :ﾍｯﾀﾞﾁｪｯｸOK                             */
/*         FALSE:ﾍｯﾀﾞﾁｪｯｸNG                             */
/*------------------------------------------------------*/
static u8 _check_rom_header( void )
{
    u8 *pDucSrc;
    /* ループカウンタ */
    u16 DuhCount;
    u8 ret = RET_OK;

    /* ROM Headerまで受信完了している場合   */
    if( D_DL_ROM_HEADER_SIZE > s_dl_file_offset )
    {
        return( ret );
    }

    pDucSrc = (u8 *)D_ADRST_BOOTIF_PRG;

    for( DuhCount = 0; DuhCount < D_DL_ROM_HEADER_SIZE; DuhCount++ )
    {
        /* バージョンの各バイトが異なる場合? */
        if( s_dl_data_buff[DuhCount] != pDucSrc[DuhCount] )
        {
            /* ループを抜ける */
            break;
        }
    }

    if( DuhCount <= D_DOWNLOADFILE_VER_POINT +  6 )
    {
        /* チェック結果にID不一致に設定 */
        s_dl_check_flg.ChkRomHeader = D_ROM_CHECK_ID_MISMATCH;
    }
    else if( DuhCount >= D_DL_ROM_HEADER_SIZE )
    {
        /*第一セクションチェックに設定 */
        s_dl_check_flg.ChkRomHeader = D_ROM_CHECK_FIRST_SECTION;
		s_dl_check_flg.ChkSectionHeader = D_SEC_CHECK_DO_NOT;
    }
    else
    {
        s_dl_check_flg.ChkRomHeader = D_ROM_CHECK_WAIT_WRITE;
		s_dl_check_flg.ChkSectionHeader = D_SEC_CHECK_DO_NOT;
    }

    if(( s_dl_check_flg.ChkRomHeader == D_ROM_CHECK_WAIT_WRITE )
    ||( s_dl_check_flg.ChkRomHeader == D_ROM_CHECK_FIRST_SECTION ))
    {
        /* ダウンロードスタートアドレスの取得 */
        s_dl_start_address = (s_dl_data_buff[0x6] << 24) +
                            (s_dl_data_buff[0x7] << 16) +
                            (s_dl_data_buff[0x8] << 8)  +
                            (s_dl_data_buff[0x9]);
        s_dl_start_address = DDR_START_ADDRESS + s_dl_start_address;

        /* ダウンロードエンドアドレスの取得 */
        s_dl_end_address = (s_dl_data_buff[0xA] << 24) +
                          (s_dl_data_buff[0xB] << 16) +
                          (s_dl_data_buff[0xC] << 8)  +
                          (s_dl_data_buff[0xD]);
        s_dl_end_address = DDR_START_ADDRESS + s_dl_end_address;

        /* 書き込みアドレスをスタートアドレスから計算 */
        s_dl_write_address = s_dl_start_address - DDR_START_ADDRESS;
        s_section_info.sec_count  = 0;
        s_section_info.base_count = 0;
    }
    /* IDが不一致 ダウンロードファイル不正 */
    else
    {
        ret = RET_NG;
    }

    return( ret );
}



/*------------------------------------------------------*/
/*  CRCエリアのﾁｪｯｸ                        */
/*  <引数> なし                                         */
/*  <返値> D_RET_MAIN_CRC :ﾁｪｯｸOK                             */
/*         D_RET_ROM_CRC:ﾁｪｯｸNG                             */
/*         D_RET_NOT_CRC:ﾁｪｯｸNG                             */
/*------------------------------------------------------*/
static u8 _check_crc_area( u32 offset )
{
	if(((u8)(s_dl_data_buff[offset + D_ENTRY_TBL_OFFSET_SEC_NAME]    ) == 'C') &&
       ((u8)(s_dl_data_buff[offset + D_ENTRY_TBL_OFFSET_SEC_NAME + 1]) == 'T') &&
       ((u8)(s_dl_data_buff[offset + D_ENTRY_TBL_OFFSET_SEC_NAME + 2]) == 'B') &&
       ((u8)(s_dl_data_buff[offset + D_ENTRY_TBL_OFFSET_SEC_NAME + 3]) == 'L') )
    {
        return( D_RET_ROM_CRC );
    }
    return( D_RET_NOT_CRC );
}
/*------------------------------------------------------*/
/*  最初のセクションのﾍｯﾀﾞのﾁｪｯｸ                        */
/*  <引数> なし                                         */
/*  <返値> TRUE :ﾍｯﾀﾞﾁｪｯｸOK                             */
/*         FALSE:ﾍｯﾀﾞﾁｪｯｸNG                             */
/*------------------------------------------------------*/
static u8 _check_first_section_header( void )
{
    u8 ret = RET_OK;

	/* ヘッダの受信完了している場合 */
	if( D_DL_ROM_HEADER_SIZE + D_DL_SECTION_HEADER_SIZE <= s_dl_buff_offset )
	{
		/* IDなどのチェック */
		ret = _check_file_header(D_DL_ROM_HEADER_SIZE);

		if( D_SIG_CHECK_OK != ret )
		{
			return( RET_NG );
		}
		else
		{
			ret = RET_OK;
		}

		s_section_info.start_adr =  s_dl_data_buff[D_DL_ROM_HEADER_SIZE + D_ENTRY_TBL_OFFSET_STARTADR]   & 0xff;
		s_section_info.start_adr = (s_dl_data_buff[D_DL_ROM_HEADER_SIZE + D_ENTRY_TBL_OFFSET_STARTADR+1] & 0xff) + ( s_section_info.start_adr << 8);
		s_section_info.start_adr = (s_dl_data_buff[D_DL_ROM_HEADER_SIZE + D_ENTRY_TBL_OFFSET_STARTADR+2] & 0xff) + ( s_section_info.start_adr << 8);
		s_section_info.start_adr = (s_dl_data_buff[D_DL_ROM_HEADER_SIZE + D_ENTRY_TBL_OFFSET_STARTADR+3] & 0xff) + ( s_section_info.start_adr << 8);
		s_section_info.start_adr = DDR_START_ADDRESS + s_section_info.start_adr;
		s_section_info.end_adr   =  s_dl_data_buff[D_DL_ROM_HEADER_SIZE + D_ENTRY_TBL_OFFSET_ENDADR]   & 0xff;
		s_section_info.end_adr   = (s_dl_data_buff[D_DL_ROM_HEADER_SIZE + D_ENTRY_TBL_OFFSET_ENDADR+1] & 0xff) + ( s_section_info.end_adr << 8);
		s_section_info.end_adr   = (s_dl_data_buff[D_DL_ROM_HEADER_SIZE + D_ENTRY_TBL_OFFSET_ENDADR+2] & 0xff) + ( s_section_info.end_adr << 8);
		s_section_info.end_adr   = (s_dl_data_buff[D_DL_ROM_HEADER_SIZE + D_ENTRY_TBL_OFFSET_ENDADR+3] & 0xff) + ( s_section_info.end_adr << 8);
		s_section_info.end_adr = DDR_START_ADDRESS + s_section_info.end_adr;
		s_section_info.size      = s_section_info.end_adr - s_section_info.start_adr + 1;

		/* ノーマルダウンロードか、CRCのチェック結果がNGか、ダウンロードデータのCRCと異なる場合 */
		if( ( ex_section_crc[s_section_info.sec_count] == D_SEC_CRC_NG ) ||
			( D_CRC32_CHECK_OK != _check_header_crc32(D_DL_ROM_HEADER_SIZE) ) ||
			( s_dl_check_flg.ChkRomHeader == D_ROM_CHECK_WAIT_WRITE ) ||
			( ex_cline_download_control.is_start_received == DL_START_NORMAL ))
		{
			/* NGの場合ダウンロード実施 */
			s_section_info.sec_count++;
			s_dl_check_flg.ChkRomHeader = D_ROM_CHECK_WAIT_WRITE;
			s_dl_check_flg.ChkSectionHeader = D_SEC_CHECK_OK;

			/* BIFセクションチェック実施 */
			s_dl_check_flg.ChkBifSection = D_BIF_CHECK_DO_NOT;

			return( ret );
		}
		/* BIFセクションチェックOK */
		s_dl_check_flg.ChkBifSection = D_BIF_CHECK_OK;

		s_section_info.sec_count++;

		/* スキップする場合 */
		/* Section end address+1のアドレスを要求する */
		/* 最終アドレスの次のアドレス       */
		s_dl_write_address = s_section_info.end_adr - DDR_START_ADDRESS + 1 ;

		if( s_section_info.end_adr == s_dl_end_address )
		{
			s_dl_file_offset = 0xffffffff;
		}
		else
		{
			/* オフセットアドレスを更新         */
			s_dl_file_offset = s_dl_write_address - (s_dl_start_address - DDR_START_ADDRESS);
		}
		/* 当セクションの受信サイズをクリア */
		s_section_info.rcv_size = 0;
		/* バッファにある受信データ破棄     */
		s_dl_buff_offset        = 0;

		/* ROMヘッダOKに設定 */
		s_dl_check_flg.ChkRomHeader = D_ROM_CHECK_OK;
		/* 次のセクションのヘッダチェックのためチェック未実施に設定 */
		s_dl_check_flg.ChkSectionHeader = D_SEC_CHECK_DO_NOT;
	}

    return( ret );
}


/*------------------------------------------------------*/
/*  各セクションのﾍｯﾀﾞのﾁｪｯｸ                            */
/*  <引数> なし                                         */
/*  <返値> TRUE :ﾍｯﾀﾞﾁｪｯｸOK                             */
/*         FALSE:ﾍｯﾀﾞﾁｪｯｸNG                             */
/*------------------------------------------------------*/
static u8 _check_section_header( void )
{
    u8 ret = RET_OK;
    u8  result_crc_area;

	/* ヘッダの受信完了している場合 */
	if( D_DL_SECTION_HEADER_SIZE <= s_dl_buff_offset )
	{
		/* IDなどのチェック */
		ret = _check_file_header(0);

		if( D_SIG_CHECK_OK != ret )
		{
			return( RET_NG );
		}
		else
		{
			ret = RET_OK;
		}

		result_crc_area = _check_crc_area(0);

		s_section_info.start_adr =  s_dl_data_buff[D_ENTRY_TBL_OFFSET_STARTADR]   & 0xff;
		s_section_info.start_adr = (s_dl_data_buff[D_ENTRY_TBL_OFFSET_STARTADR+1] & 0xff) + ( s_section_info.start_adr << 8);
		s_section_info.start_adr = (s_dl_data_buff[D_ENTRY_TBL_OFFSET_STARTADR+2] & 0xff) + ( s_section_info.start_adr << 8);
		s_section_info.start_adr = (s_dl_data_buff[D_ENTRY_TBL_OFFSET_STARTADR+3] & 0xff) + ( s_section_info.start_adr << 8);
		s_section_info.start_adr = DDR_START_ADDRESS + s_section_info.start_adr;
		s_section_info.end_adr   =  s_dl_data_buff[D_ENTRY_TBL_OFFSET_ENDADR]   & 0xff;
		s_section_info.end_adr   = (s_dl_data_buff[D_ENTRY_TBL_OFFSET_ENDADR+1] & 0xff) + ( s_section_info.end_adr << 8);
		s_section_info.end_adr   = (s_dl_data_buff[D_ENTRY_TBL_OFFSET_ENDADR+2] & 0xff) + ( s_section_info.end_adr << 8);
		s_section_info.end_adr   = (s_dl_data_buff[D_ENTRY_TBL_OFFSET_ENDADR+3] & 0xff) + ( s_section_info.end_adr << 8);
		s_section_info.end_adr   = DDR_START_ADDRESS + s_section_info.end_adr;
		s_section_info.size      = s_section_info.end_adr - s_section_info.start_adr + 1;

#if 1 //DEBUG:強制スキップ
		/* ノーマルダウンロードか、CRCのチェック結果がNGか、ダウンロードデータのCRCと異なる場合 */
		if( ( ex_section_crc[s_section_info.sec_count] == D_SEC_CRC_NG ) ||
			( D_CRC32_CHECK_OK != _check_header_crc32(0) ) ||
			( ex_cline_download_control.is_start_received == DL_START_NORMAL ))
		{
			/* NGの場合ダウンロード実施 */
			s_section_info.sec_count++;
			s_dl_check_flg.ChkSectionHeader = D_SEC_CHECK_OK;

			/* BIFセクションチェック実施 */
			s_dl_check_flg.ChkBifSection = D_BIF_CHECK_DO_NOT;

			if( D_RET_ROM_CRC == result_crc_area )
			{
				/* 金種毎のダウンロードの際にend_adr使用するため、バックアップ */
				/* +2はROMのCRC                       */
				s_section_info.end_adr      = s_section_info.end_adr + 2;
				s_section_info.size         = D_DL_SECTION_HEADER_SIZE;
			}
			return( ret );
		}
		/* USERセクション、BASEセクションをスキップする場合 */
		/* (スキップし、かつ次のエリアがCRCの場合)          */
		else if( D_RET_ROM_CRC == result_crc_area )
		{
			s_section_info.sec_count++;
			s_dl_check_flg.ChkSectionHeader = D_SEC_CHECK_OK;
			s_section_info.start_adr = s_section_info.end_adr + 1;
			s_section_info.end_adr   = s_section_info.end_adr + 2;
			s_section_info.size      = 2;
			s_dl_write_address           = s_section_info.start_adr - DDR_START_ADDRESS;
			/* オフセットアドレスを更新         */
			s_dl_file_offset             = s_dl_write_address - (s_dl_start_address - DDR_START_ADDRESS);
			/* 当セクションの受信サイズをクリア */
			s_section_info.rcv_size      = 0;
			/* バッファにある受信データ破棄     */
			s_dl_buff_offset             = 0;
 			return( ret );
		}
#else
		if( D_RET_ROM_CRC == result_crc_area )
		{
			s_section_info.sec_count++;
			s_dl_check_flg.ChkSectionHeader = D_SEC_CHECK_OK;
			s_section_info.start_adr = s_section_info.end_adr + 1;
			s_section_info.end_adr   = s_section_info.end_adr + 2;
			s_section_info.size      = 4;
			s_dl_write_address           = s_section_info.start_adr - DDR_START_ADDRESS;
			/* オフセットアドレスを更新         */
			s_dl_file_offset             = s_dl_write_address - (s_dl_start_address - DDR_START_ADDRESS);
			/* 当セクションの受信サイズをクリア */
			s_section_info.rcv_size      = 0;
			/* バッファにある受信データ破棄     */
			s_dl_buff_offset             = 0;
			return( ret );
		}
#endif //DEBUG:強制スキップ

		s_section_info.sec_count++;

		/* スキップする場合 */
		/* Section end address+1のアドレスを要求する */
		/* 最終アドレスの次のアドレス       */
		s_dl_write_address = s_section_info.end_adr - DDR_START_ADDRESS + 1;

		if( s_section_info.end_adr == s_dl_end_address )
		{
			s_dl_file_offset = 0xffffffff;
		}
		else
		{
			/* オフセットアドレスを更新         */
			s_dl_file_offset = s_dl_write_address - (s_dl_start_address - DDR_START_ADDRESS);
		}
		/* 当セクションの受信サイズをクリア */
		s_section_info.rcv_size = 0;
		/* バッファにある受信データ破棄     */
		s_dl_buff_offset        = 0;
		/* 次のセクションのヘッダチェックのためチェック未実施に設定 */
		s_dl_check_flg.ChkSectionHeader = D_SEC_CHECK_DO_NOT;
	}

    return( ret );
}

/*------------------------------------------------------*/
/*  BIFセクションエリアのﾁｪｯｸ                           */
/*  <引数> なし                                         */
/*  <返値> TRUE :ﾍｯﾀﾞﾁｪｯｸOK                             */
/*         FALSE:ﾍｯﾀﾞﾁｪｯｸNG                             */
/*------------------------------------------------------*/
static u8 _check_bif_section_header( void )
{
    u8 ret = RET_OK;
	u8  result_file_header;
	u8  result_section_header;

	/* ２ヘッダサイズの受信完了している場合 */
	if( D_DL_SECTION_HEADER_SIZE * 2 <= s_dl_buff_offset )
	{
		/* BIF1ヘッダのチェック */
		result_file_header = _check_file_header(D_DL_SECTION_HEADER_SIZE);
		if( D_SIG_CHECK_OK != result_file_header )
		{
			/* Not Dupricated BIF Section */
			s_dl_check_flg.ChkBifSection = D_BIF_CHECK_OK;
			return( ret );
		}
		else
		{
			/* Dupricated BIF Section */
			/* データとFLASHのBIF1ヘッダー1データの比較 */
			result_section_header = _check_header(D_DL_SECTION_HEADER_SIZE);

			/* ノーマルダウンロードか、CRCのチェック結果がNGか、ダウンロードデータがBIFヘッダーBIF1ヘッダーと異なる場合 */
			if( ( ex_section_crc[s_section_info.sec_count] == D_SEC_CRC_NG ) ||
				( D_HEADER_CHECK_OK != result_section_header ) ||
				( ex_cline_download_control.is_start_received == DL_START_NORMAL ))
			{
				/* NGの場合ダウンロード実施 */
				s_section_info.sec_count++;
				/* BIF 2重化セクション用構造体 */
				s_internal_section_info.start_adr =  s_dl_data_buff[D_DL_SECTION_HEADER_SIZE + D_ENTRY_TBL_OFFSET_STARTADR]   & 0xff;
				s_internal_section_info.start_adr = (s_dl_data_buff[D_DL_SECTION_HEADER_SIZE + D_ENTRY_TBL_OFFSET_STARTADR+1] & 0xff) + ( s_internal_section_info.start_adr << 8);
				s_internal_section_info.start_adr = (s_dl_data_buff[D_DL_SECTION_HEADER_SIZE + D_ENTRY_TBL_OFFSET_STARTADR+2] & 0xff) + ( s_internal_section_info.start_adr << 8);
				s_internal_section_info.start_adr = (s_dl_data_buff[D_DL_SECTION_HEADER_SIZE + D_ENTRY_TBL_OFFSET_STARTADR+3] & 0xff) + ( s_internal_section_info.start_adr << 8);
				s_internal_section_info.end_adr   =  s_dl_data_buff[D_DL_SECTION_HEADER_SIZE + D_ENTRY_TBL_OFFSET_ENDADR]   & 0xff;
				s_internal_section_info.end_adr   = (s_dl_data_buff[D_DL_SECTION_HEADER_SIZE + D_ENTRY_TBL_OFFSET_ENDADR+1] & 0xff) + ( s_internal_section_info.end_adr << 8);
				s_internal_section_info.end_adr   = (s_dl_data_buff[D_DL_SECTION_HEADER_SIZE + D_ENTRY_TBL_OFFSET_ENDADR+2] & 0xff) + ( s_internal_section_info.end_adr << 8);
				s_internal_section_info.end_adr   = (s_dl_data_buff[D_DL_SECTION_HEADER_SIZE + D_ENTRY_TBL_OFFSET_ENDADR+3] & 0xff) + ( s_internal_section_info.end_adr << 8);
				s_internal_section_info.size      = s_internal_section_info.end_adr - s_internal_section_info.start_adr + 1;
				/* Boot I/F領域1のCRC異常な場合 */
				if( ex_section_status.bif_section1_crc_result == D_NG )
				{
					s_dl_check_flg.ChkBifSection = D_BIF_CHECK_WAIT_WRITE_BIF1;
				}
				else
				{
					s_dl_check_flg.ChkBifSection = D_BIF_CHECK_WAIT_WRITE_BIF2;
				}

				return( ret );
			}
		}
	}

    return( ret );
}


/*------------------------------------------------------*/
/*  ダウンロードデータコマンド処理                      */
/*  <引数> なし                                         */
/*  <返値> なし                                         */
/*------------------------------------------------------*/
void _download_write( void )
{
    /****************/
    /* 書き込み処理 */
    /****************/
	/* Versionチェック,Signatureチェック完了              */
	if( ( s_dl_check_flg.ChkSignature == D_SIG_CHECK_OK ) &&
		( s_dl_check_flg.ChkRomHeader == D_ROM_CHECK_OK ) &&
		( s_dl_check_flg.ChkSectionHeader == D_SEC_CHECK_OK ) &&
		( s_dl_check_flg.ChkBifSection == D_BIF_CHECK_OK ) )
	{
		_write_proc();
	}
	/* ROM Headerのみ書き込み */
	else if( (s_dl_check_flg.ChkSignature == D_SIG_CHECK_OK) &&
			 (s_dl_check_flg.ChkRomHeader == D_ROM_CHECK_WAIT_WRITE) &&
			 (s_dl_check_flg.ChkSectionHeader == D_SEC_CHECK_OK)  )
	{
		s_dl_check_flg.ChkRomHeader = D_ROM_CHECK_OK;
		_write_proc();
	}
	/* BIFセクションの書き込み */
	else if( (s_dl_check_flg.ChkSignature == D_SIG_CHECK_OK) &&
			  (s_dl_check_flg.ChkRomHeader == D_ROM_CHECK_OK) &&
			  (s_dl_check_flg.ChkSectionHeader == D_SEC_CHECK_OK) &&
			  ((s_dl_check_flg.ChkBifSection == D_BIF_CHECK_WAIT_WRITE_BIF1) ||
			   (s_dl_check_flg.ChkBifSection == D_BIF_CHECK_WAIT_WRITE_BIF2)))
	{
		_write_proc_for_BifSection();
	}
}

/*------------------------------------------------------*/
/*  書き込み処理                                        */
/*  <引数> なし                                         */
/*  <返値> なし                                         */
/*------------------------------------------------------*/
void _write_proc( void )
{
    u32 tmp_offset = 0;
    u8  write_flg = 0;

    /* セクションの最後まで受信した場合                   */
    /* セクション最終アドレスが0の場合は通常ダウンロード  */
    if( (s_section_info.end_adr - DDR_START_ADDRESS <= (s_dl_buff_offset -1 + s_dl_write_address)) &&
        (s_section_info.end_adr != 0) )
    {
        /* 書き込み後の余り計算 */
        tmp_offset = (s_dl_buff_offset -1 + s_dl_write_address) - (s_section_info.end_adr - DDR_START_ADDRESS);
        /* 書き込みサイズ */
        s_dl_buff_offset = s_dl_buff_offset - tmp_offset;
        /* ヘッダチェックを未実施に戻す */
        s_dl_check_flg.ChkSectionHeader = D_SEC_CHECK_DO_NOT;
        /* セクション受信サイズをクリア */
        s_section_info.rcv_size = 0;
        write_flg = 1;
    }
    /* 受信バッファがフルになった場合   */
    else if( D_DOWNLOAD_BUFF_SIZE <= s_dl_buff_offset )
    {
        /* 書き込み後の余り計算         */
        tmp_offset = s_dl_buff_offset - D_DOWNLOAD_BUFF_SIZE;
        /* 書き込みサイズ               */
        s_dl_buff_offset = D_DOWNLOAD_BUFF_SIZE;
        write_flg = 1;
    }
    /* ダウンロードファイルの最終アドレスまで受信した場合 */
    else if( s_dl_end_address - DDR_START_ADDRESS <= (s_dl_buff_offset -1 + s_dl_write_address) )
    {
        /* 書き込み後の余り計算 */
        tmp_offset = (s_dl_buff_offset -1 + s_dl_write_address) - (s_dl_end_address - DDR_START_ADDRESS);
        s_dl_buff_offset = s_dl_buff_offset - tmp_offset;
        /* 書き込み後の余り計算 */
        tmp_offset = 0;
        write_flg = 1;
    }

    if( 1 == write_flg )
    {
		_write_flash_sequence( tmp_offset );
    }
}

/*------------------------------------------------------*/
/*  書き込み処理                                        */
/*  <引数> なし                                         */
/*  <返値> なし                                         */
/*------------------------------------------------------*/
 void _write_proc_for_BifSection( void )
{
	u32 bk_offset = 0;
	u32 bk_address = 0;
    u32 tmp_offset = 0;
    u8  write_flg   = 0;

    /* セクションの最後まで受信した場合                   */
    /* セクション最終アドレスが0の場合は通常ダウンロード  */
    if( (s_section_info.end_adr - DDR_START_ADDRESS <= (s_dl_buff_offset -1 + s_dl_write_address)) &&
        (s_section_info.end_adr != 0) )
    {
        /* 書き込み後の余り計算 */
        tmp_offset = (s_dl_buff_offset -1 + s_dl_write_address) - (s_section_info.end_adr - DDR_START_ADDRESS);
        /* 書き込みサイズ */
        s_dl_buff_offset = s_dl_buff_offset - tmp_offset;
        /* ヘッダチェックを未実施に戻す */
        s_dl_check_flg.ChkSectionHeader = D_SEC_CHECK_DO_NOT;
        /* セクション受信サイズをクリア */
        s_section_info.rcv_size = 0;
        write_flg = 1;
    }
	/* BIF1の最終アドレスまで受信した場合 */
	else if( (s_internal_section_info.end_adr - DDR_START_ADDRESS <= (s_dl_buff_offset -1 + s_dl_write_address)) &&
        (s_internal_section_info.end_adr != 0) )
	{
		/* s_internal_section_infoとsectionヘッダ分を書き込む */
        /* 書き込み後の余り計算 */
        tmp_offset = (s_dl_buff_offset -1 + s_dl_write_address) - (s_internal_section_info.end_adr - DDR_START_ADDRESS);
        /* 書き込みサイズ */
        s_dl_buff_offset = s_dl_buff_offset - tmp_offset;
		/* BIF 2重化セクション用構造体 */
		if(s_dl_check_flg.ChkBifSection == D_BIF_CHECK_WAIT_WRITE_BIF2)
		{
			/* BIF2がCRC異常の場合はBIF2にBIF1を先に書き込む */
			/* BIF2はBIF1書き込み後再度、受信データが書き込まれる */
			/* バックアップ */
			bk_offset = s_dl_buff_offset;
			bk_address = s_dl_write_address;
			/* 書き込みアドレス */
			s_dl_write_address = s_internal_section_info.end_adr - DDR_START_ADDRESS + 1;
			_write_flash_sequence( 0 );
			/* バックアップを復帰 */
			s_dl_write_address = bk_address;
			s_dl_buff_offset = bk_offset;
		}
        /* セクション受信サイズをクリア */
        s_internal_section_info.rcv_size = 0;
        write_flg = 1;
	}
    /* 受信バッファがフルになった場合   */
    else if( D_DOWNLOAD_BUFF_SIZE <= s_dl_buff_offset )
    {
        /* 書き込み後の余り計算         */
        tmp_offset = s_dl_buff_offset - D_DOWNLOAD_BUFF_SIZE;
        /* 書き込みサイズ               */
        s_dl_buff_offset = D_DOWNLOAD_BUFF_SIZE;
        write_flg = 1;
    }
    /* ダウンロードファイルの最終アドレスまで受信した場合 */
    else if( s_dl_end_address - DDR_START_ADDRESS <= (s_dl_buff_offset -1 + s_dl_write_address) )
    {
        /* 書き込み後の余り計算 */
        tmp_offset = (s_dl_buff_offset -1 + s_dl_write_address) - (s_dl_end_address - DDR_START_ADDRESS);
        s_dl_buff_offset = s_dl_buff_offset - tmp_offset;
        /* 書き込み後の余り計算 */
        tmp_offset = 0;
        write_flg = 1;
    }

    if( 1 == write_flg )
    {
		_write_flash_sequence( tmp_offset );
        /* BIFセクションチェックをOKに戻す */
        /* 以降のデータは(_write_proc)で書き込む */
        s_dl_check_flg.ChkBifSection = D_BIF_CHECK_OK;
    }
}

INT8 _read_rom(u8 *dst, u8 *offset, u32 len)
{
	UINT32 i;
	QSPI_BUF_INFO info;

	info.buf = (u8 *)dst;
	info.addr = (u32)offset;
	info.len = len;
	info.byte_count = &i;
#ifdef  FLASH_QSPI_MODE
	if(ALT_E_ERROR == alt_qspi_read((void *)info.buf
			, info.addr
			, info.len))
	{
		return(FALSE);
	}
#else
	if( QSPI_Flash_Read( &hQFlash, &info ) == FALSE ){
		return(FALSE);
	};
#endif

	return(TRUE);
}
/*------------------------------------------------------*/
/*  ダウンロード後のQSPIからDDR3への展開                           */
/*  <引数> なし                                         */
/*  <返値> TRUE :OK                             */
/*------------------------------------------------------*/
u8 _download_copy_to_ddr3(void)
{
	u8 *dst;
	u8 *src;
	const u32 block_size = (64*1024);
	u32 copy_size;
	u32 cnt;

	copy_size = s_dl_end_address - s_dl_start_address + 1;
	dst = (u8 *)DOWNLOAD_IMAGE_BUF;
	src = (u8 *)(PROGRAM_START_ADRESS - DDR_START_ADDRESS);
	cnt = copy_size / block_size;

	while(cnt--)
	{
		if(!_read_rom((u8 *)dst, (u8 *)src, block_size))
		{
			return FALSE;
		}
		copy_size -= block_size;
		src += block_size;
		dst += block_size;
		_hal_feed_wdt();
	}

	if(copy_size % block_size)
	{
		if(!_read_rom((u8 *)dst, (u8 *)src, copy_size % block_size))
		{
			return FALSE;
		}
		copy_size -= copy_size % block_size;
		src += copy_size % block_size;
		dst += copy_size % block_size;
		_hal_feed_wdt();
	}

	return TRUE;
}

/*------------------------------------------------------*/
/*  ダウンロード後のCRCチェック                           */
/*  <引数> なし                                         */
/*  <返値> TRUE :OK                             */
/*------------------------------------------------------*/
u8 download_calc_crc(void)
{
	u32 rom_size;
	u16 tmp_crc;
	if(!_download_copy_to_ddr3())
	{
		return FALSE;
	}

	rom_size = s_dl_end_address - s_dl_start_address + 1;

	ex_cline_download_control.program_crc = _calc_crc( (u8 *)DOWNLOAD_IMAGE_BUF, rom_size - 2, TRUE );
	ex_cline_download_control.file_crc = _calc_crc( (u8 *)DOWNLOAD_IMAGE_BUF + rom_size - 2, 2,   FALSE );

    tmp_crc = *(u8 *)(DOWNLOAD_IMAGE_BUF + rom_size - 2);
    tmp_crc = (tmp_crc << 8) & 0xff00;
    tmp_crc = tmp_crc + *(u8 *)(DOWNLOAD_IMAGE_BUF + rom_size - 1);

    if( tmp_crc != ex_cline_download_control.program_crc )
    {
		return FALSE;
    }
	return TRUE;
}

/* EOF */
