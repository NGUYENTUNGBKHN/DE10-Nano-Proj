/******************************************************************************/
/*! @addtogroup Group1
    @file       hal_spi_fram.h
    @brief      FRAM control header
    @date       2018/05/16
    @author     H.Suzuki
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/05/16 H.Suzuki
      -# Initial Version
******************************************************************************/

#if (_ENABLE_JDL==1)
#include "../jdl/include/jdl_comm.h"
#include "../jdl/include/jdl_conf.h"
#endif
/* Public Defines ----------------------------------------------------------- */
#define FRAM_SIZE					(128 * 1024)

/* FRAM ADDRESS(Adjustment) */
#define FRAM_ADJ_OFFSET				(  0)
#define FRAM_ADJ_ALL_SIZE			( 35 * 1024)
#define FRAM_ADJ_SIZE				(128)
#define FRAM_ADJ_SER_ADR			(  0)
#define FRAM_ADJ_SER_SIZE			( 16)
#define FRAM_ADJ_INFO_0_ADR			( 16)
#define FRAM_ADJ_INFO_0_SIZE		( 28)
#define FRAM_ADJ_INFO_1_ADR			( 44)
#define FRAM_ADJ_INFO_1_SIZE		( 28)
#define FRAM_ADJ_VALUE_0_ADR		( 72)
#define FRAM_ADJ_VALUE_0_SIZE		(  8)
#define FRAM_ADJ_VALUE_1_ADR		( 80)
#define FRAM_ADJ_VALUE_1_SIZE		(  8)
#define FRAM_ADJ_POS_DA_SIZE		(  5)
#define FRAM_ADJ_POS_GAIN_SIZE		(  1)
#define FRAM_ADJ_MAG_ADR			( 88)
#define FRAM_ADJ_MAG_SIZE			(  8)
#define FRAM_ADJ_CIS_ADR			(128)
#define FRAM_ADJ_CIS_SIZE			(34676)
#define FRAM_ADJ_SUM_ADR			((35 * 1024)-2)
#define FRAM_ADJ_SUM_SIZE			2

/* FRAM ADDRESS(TempAdjustment) */
#define FRAM_ADJ_TMP_OFFSET				(FRAM_ADJ_OFFSET + FRAM_ADJ_ALL_SIZE)
#define FRAM_ADJ_TMP_ALL_SIZE			(512)
#define FRAM_ADJ_TMP_ADR				(  0)
#define FRAM_ADJ_TMP_SIZE				(  8*2)
#define FRAM_ADJ_TMP_POS_DA_SIZE		(  5)
#define FRAM_ADJ_TMP_POS_GAIN_SIZE		(  1)
#define FRAM_ADJ_TMP_POS_SUM_SIZE		(  2)
#define FRAM_ADJ_TMP_BK_ADR				(  8)
#define FRAM_ADJ_TMP_POS_BK_DA_SIZE		(  5)
#define FRAM_ADJ_TMP_POS_BK_GAIN_SIZE	(  1)
#define FRAM_ADJ_TMP_POS_BK_SUM_SIZE	(  2)
#define FRAM_ADJ_CIS_TMP_ADR			( 16)
#define FRAM_ADJ_CIS_TMP_SIZE			(216)
#define FRAM_BLK_TMP_SIZE				(272)
#define FRAM_ADJ_MAG_TMP_ADR			(504)
#define FRAM_ADJ_MAG_TMP_SIZE			(  8)

/* FRAM ADDRESS(PWR) */
#define FRAM_PWR_OFFSET					(FRAM_ADJ_TMP_OFFSET + FRAM_ADJ_TMP_ALL_SIZE)
#define FRAM_PWR_ALL_SIZE				(512)
#define FRAM_MODE_SET_ADR				(  0) //not use
#define FRAM_MODE_SET_SIZE				(  4) //not use
#define FRAM_LOCK_SET_ADR				(  4) //not use
#define FRAM_LOCK_SET_SIZE				( 24) //not use
#define FRAM_MODE2_SET_ADR				( 28)
#define FRAM_MODE2_SET_SIZE				( 16)
// STATUS TABLE(POWER RECOVERY TABLE)
#define FRAM_STATUS_TBL_ADR				256	/* 256 - 511 */
#define FRAM_STATUS_TBL_SIZE			256

/* FRAM ADDRESS(I/F) */
#define FRAM_IF_OFFSET								(FRAM_PWR_OFFSET + FRAM_PWR_ALL_SIZE)   //36864
#define FRAM_IF_ALL_SIZE							(  1 * 1024)
#define FRAM_IF_ADR									(  0)
#if defined(UBA_RTQ)
#define FRAM_IF_SIZE                                (30)
#define FRAM_RTQ_SOFT_INFO_ADR                      (FRAM_IF_ADR + FRAM_IF_SIZE) //30
#define FRAM_RTQ_SOFT_INFO_SIZE                     (32)
#define FRAM_RTQ_BEFORE_STA_ADR                     (FRAM_RTQ_SOFT_INFO_ADR + FRAM_RTQ_SOFT_INFO_SIZE)
#define FRAM_RTQ_BEFORE_STA_SIZE                    (2)
#define FRAM_RTQ_RC_LOG_IF_ADR                      (FRAM_RTQ_BEFORE_STA_ADR + FRAM_RTQ_BEFORE_STA_SIZE)
#define FRAM_RTQ_RC_LOG_IF_SIZE                     (48)
#define FRAM_RTQ_MENTE_SERI_ADR                     (FRAM_RTQ_RC_LOG_IF_ADR + FRAM_RTQ_RC_LOG_IF_SIZE)
//#define FRAM_RTQ_MENTE_SERI_SIZE                    (144)
#define FRAM_RTQ_MENTE_SERI_SIZE                    (72)


//2024-10-16
//        EPROM_MAINTENANCE_EDITION
//        EPROM_MAINTENANCE_EDITION_SIZE
#define FRAM_RTQ_MAINTENANCE_EDITION                     (FRAM_RTQ_MENTE_SERI_ADR + FRAM_RTQ_MENTE_SERI_SIZE)
#define FRAM_RTQ_MAINTENANCE_EDITION_SIZE                (2)

#endif // UBA_RTQ

/* FRAM ADDRESS(ICB) */
#define FRAM_ICB_OFFSET					(FRAM_IF_OFFSET + FRAM_IF_ALL_SIZE)
#define FRAM_ICB_ALL_SIZE				(  1 * 1024)
#define FRAM_ICB_ADR					(  0)
/* FRAM ADDRESS(LOG) */
#define FRAM_LOG_OFFSET					(FRAM_ICB_OFFSET + FRAM_ICB_ALL_SIZE)
#define FRAM_LOG_ALL_SIZE				( 90 * 1024)


//STATUS_TBLを除き,各I/F別のアドレスを使用する
#if 0//
    // ID-003 Area
    #define FRAM_IF_ID003_COMM_MODE_ADR			0	/* 0 - 7 */
    #define FRAM_IF_ID003_COMM_MODE_SIZE		8
    #define FRAM_IF_ID003_AUTH_STS_ADR			8	/* 8 - 11 *///not use
    #define FRAM_IF_ID003_AUTH_STS_SIZE			4
    #define FRAM_IF_ID003_AUTH_NCUS_KEY_ADR		12	/* 12 - 15 */
    #define FRAM_IF_ID003_AUTH_NCUS_KEY_SIZE	4
    #define FRAM_IF_ID003_AUTH_NUM_KEY_ADR		16	/* 16 - 23 */
    #define FRAM_IF_ID003_AUTH_NUM_KEY_SIZE		8
    #define FRAM_IF_ID003_AUTH_CODE_ADR			24	/* 24 - 27 */
    #define FRAM_IF_ID003_AUTH_CODE_SIZE		4
    #define FRAM_IF_ID003_AUTH_SUM_ADR			29	/* 28 - 29 *///not use
    #define FRAM_IF_ID003_AUTH_SUM_SIZE			2

    // ICB Recovery Info
    #define FRAM_ICB_SETTING_ADR				0	/* 0 - 63 */
    #define FRAM_ICB_SETTING_SIZE				64
    #define FRAM_ICB_RECOVER_DATA_ADR			(64)	/* 64 - 965 */
    #define FRAM_ICB_RECOVER_DATA_SIZE			(902)
#else

    #define FRAM_ICB_SETTING_ADR				0	//UBA_WS
    #define FRAM_ICB_SETTING_SIZE				64  //UBA_WS
    #define FRAM_ICB_RECOVER_DATA_ADR			(64)	/* 64 - 965 */  // 2024-04-03
    #define FRAM_ICB_RECOVER_DATA_SIZE			(902)                   // 2024-04-03

    #define FRAM_IF_IDENTIFER_ADR		0	/* 0 - 3 */
    #define FRAM_IF_IDENTIFER_SIZE		4
    #define FRAM_IF_IDENTIFER_SUM_ADR	4	/* 4 - 5 */
    #define FRAM_IF_IDENTIFER_SUM_SIZE	2
    #define FRAM_IF_FENC_MODE_ADR		6	/* 6 - 6 */
    #define FRAM_IF_FENC_MODE_SIZE		1
    #define FRAM_IF_FENC_STATUS_ADR		7	/* 7 - 7 */
    #define FRAM_IF_FENC_STATUS_SIZE	1
    #define FRAM_IF_FLEX_ENCTBL_ADR		8	/* 8 - 263 */
    #define FRAM_IF_FLEX_ENCTBL_SIZE	256
    #define FRAM_IF_FENC_SUM_ADR		264	/* 264 - 265 */
    #define FRAM_IF_FENC_SUM_SIZE		2
    #define FRAM_IF_ENC_ENABLE_ADR		266	/* 266 - 266 */
    #define FRAM_IF_ENC_ENABLE_SIZE		1
    #define FRAM_IF_VDIP_VAL_ADR		268	/* 268 - 269 */
    #define FRAM_IF_VDIP_VAL_SIZE		2
    #define FRAM_IF_VDIP_EN_ADR			270	/* 268 - 269 */
    #define FRAM_IF_VDIP_EN_SIZE		1
    #define FRAM_IF_MSPD_VAL_ADR		272	/* 272 - 273 */
    #define FRAM_IF_MSPD_VAL_SIZE		2
    #define FRAM_IF_MSPD_EN_ADR			274	/* 274 - 274 */
    #define FRAM_IF_MSPD_EN_SIZE		1
    #define FRAM_IF_STATUS_TBL_ADR				512	/* 512 - 767 *///not use
    #define FRAM_IF_STATUS_TBL_SIZE				256

#endif
/* Public Functions ----------------------------------------------------------- */
u32 _hal_write_fram_adj_sum(u32 address, u8 *pdata, u32 length);
u32 _hal_read_fram_adj_sum(u32 address, u8 *pdata, u32 length);
u32 _hal_write_fram_adj(u32 address, u8 *pdata, u32 length);
u32 _hal_read_fram_adj(u32 address, u8 *pdata, u32 length);
u32 _hal_write_fram_cis_adj(u32 address, u8 *pdata, u32 length);
u32 _hal_read_fram_cis_adj(u32 address, u8 *pdata, u32 length);
u32 _hal_write_fram_adj_tmp(u32 address, u8 *pdata, u32 length);
u32 _hal_read_fram_cis_adj_tmp(u32 address, u8 *pdata, u32 length);
u32 _hal_write_fram_cis_adj_tmp(u32 address, u8 *pdata, u32 length);
u32 _hal_read_fram_adj_tmp(u32 address, u8 *pdata, u32 length);
u32 _hal_write_fram_mag_adj_tmp(u32 address, u8 *pdata, u32 length);
u32 _hal_read_fram_mag_adj_tmp(u32 address, u8 *pdata, u32 length);
u32 _hal_read_fram_power_recover(u32 address, u8 *pdata, u32 length);
u32 _hal_write_fram_power_recover(u32 address, u8 *pdata, u32 length);
u32 _hal_write_fram_if(u32 address, u8 *pdata, u32 length);
u32 _hal_read_fram_if(u32 address, u8 *pdata, u32 length);
u32 _hal_write_fram_log(u32 address, u8 *pdata, u32 length);
u32 _hal_read_fram_log(u32 address, u8 *pdata, u32 length);
u32 _hal_write_fram_icb(u32 address, u8 *pdata, u32 length);
u32 _hal_read_fram_icb(u32 address, u8 *pdata, u32 length);
u32 _hal_set_fram_write_protect(void);
u32 _hal_init_fram_status_register(void);
#if defined(UBA_RTQ)
u32 _hal_read_fram_rtq(u32 address, u8 *pdata, u32 length);
u32 _hal_write_fram_rtq(u32 address, u8 *pdata, u32 length);
#endif // uBA_RTQ

/* EOF */
