/*
 * download.h
 *
 *  Created on: 2018/01/31
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_DOWNLOAD_H_
#define SRC_DOWNLOAD_H_

#include "typedefine.h"


#define D_OK 0
#define D_NG 1

#define DL_START_NOT           0 /* ダウンロード未開始     */
#define DL_START_NORMAL        1 /* 通常ダウンロード開始   */
#define DL_START_DIFFERENTIAL  2 /* 差分ダウンロード開始   */


/* セクション毎CRCチェック結果 */
#define D_SEC_CRC_OK           0
#define D_SEC_CRC_NG           1
#define D_SEC_CRC_NO_BASE      2

/* BASEデータ毎CRCチェック結果 */
#define D_BASE_CRC_OK          0
#define D_BASE_CRC_NG          1


#define D_MAX_DWL_DENOMI         300
#define D_RET_NOT_CRC            0
#define D_RET_ROM_CRC            1

#define D_HEADER_CHECK_OK        0
#define D_HEADER_CHECK_NG        1

#define D_CRC32_CHECK_OK         0
#define D_CRC32_CHECK_NG         1

#define D_DL_NOT_COMPLETION      0 /* 未完了                 */
#define D_DL_COMPLENTION         1 /* 完了                   */
#define D_DL_FAILURE             2 /* Failure                */

#define D_DL_SECTION_HEADER_SIZE 0x50 //0x28
#define D_DL_ROM_HEADER_SIZE     0x100
#define D_DL_BASE_HEADER_SIZE    0x4

#define D_DL_COMMON_SIZE     0xFFF00

#define D_DL_BASE_SIZE           0x7384

#define D_SKIP_OFF               0
#define D_SKIP_ON                1

#define RET_OK	0
#define RET_NG	1


extern u32 s_dl_start_address;
extern u32 s_dl_end_address;
extern u32 s_dl_write_address;
extern u8 *s_dl_data_buff;
extern u32 s_dl_buff_offset;
extern u32 s_dl_file_offset;
extern u8 s_dl_end_status;

extern void init_download_variable( void );

extern u8 _check_all_section_crc( u8 crc_result );
#if defined(_PROTOCOL_ENABLE_ID0G8)

#else
extern void _download_write( void );
#endif

extern void download_copy( u8* dst, u8 length );
extern u16 dl_calc_crc( u16 *pragram_crc, u16 *file_crc );
extern u8 download_data_check( void );
extern u8 download_calc_crc(void);
extern u16 get_dl_end_status( void );

#endif /* SRC_DOWNLOAD_H_ */
