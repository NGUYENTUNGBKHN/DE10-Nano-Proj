/******************************************************************************/
/*! @addtogroup Group1
    @file       cis_sensor.h
    @brief      cis sensor function
    @file       cis_sensor.c
    @date       2018/05/24
    @author     yuji-kenta
    @par        Revision
    @par        Copyright (C)
    2018 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2018/05/24 Development Dept at Tokyo
      -# Initial Version
      -# Copy from EBA-40 project
*****************************************************************************/
// CISドライバファイル。
//CISを制御する
//CISに関するレジスタの初期化を行う
//調整補正値を書き込む
/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "kernel.h"
#include "kernel_inc.h"

#include "common.h"
//#include "custom.h"
#include "sensor.h"
#include "sub_functions.h"

#include "pl.h"
#include "pl_cis.h"

#define EXT
#include "../common/global.h"
#include "com_ram.c"
#include "jsl_ram.c"
#include "cis_ram.c"

/************************** Function Prototypes ******************************/
extern T_CAPCPLN CapCPln[CADJ_N_PLND];   // 採取シーケンスプレーン情報(CIS部)
#if MAG1_ENABLE
u8 set_mag_gain(void);
u8 set_mag_adj(void);
extern INT32 _hal_i2c3_write_mag_cal(UINT8 data);
extern INT32 _hal_i2c3_write_mag_cntl(UINT8 data);
#endif
/************************** External functions *******************************/

/************************** Variable declaration *****************************/

/************************** PRIVATE DEFINITIONS ******************************/
void _pl_cis_cap_info(u8 type)
{
	int cnt = 0;
	u8* p_Data;
	uint32_t address;
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbs = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS* pbs = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif

	p_Data = (u8 *)FPGA_ADDR_CAPINFO;
	for(cnt = 0; cnt < sizeof(CAP_INFO_STRUCTURE); cnt++)
	{
		*(p_Data + cnt) = 0;
	}
	switch(type)
	{
	case CAP_SAMPLING:
		address = (uint32_t)&pbs->sens_dt[0];
		address /= 4;
		CAP_INFO_ARRAY[0].BASE = address;
		CAP_INFO_ARRAY[0].SET.BIT.WP = 0;
		CAP_INFO_ARRAY[0].CAPST.BIT.BLK = 0;
		break;
	case CAP_ONE_SHOT_8BIT:
		address = (uint32_t)&ex_adj_data;
		address /= 4;
		CAP_INFO_ARRAY[0].BASE = address;
		CAP_INFO_ARRAY[0].SET.BIT.WP = 0;
		CAP_INFO_ARRAY[0].CAPST.BIT.BLK = 0;
		break;
	case CAP_ONE_SHOT_10BIT:
		address = (uint32_t)&ex_row_data;
		address /= 4;
		CAP_INFO_ARRAY[0].BASE = address;
		CAP_INFO_ARRAY[0].SET.BIT.WP = 0;
		CAP_INFO_ARRAY[0].CAPST.BIT.BLK = 0;
		break;
	case CAP_ONE_SHOT_MAG:
		address = (uint32_t)&ex_tmp_mag_data;
		address /= 4;
		CAP_INFO_ARRAY[0].BASE = address;
		CAP_INFO_ARRAY[0].SET.BIT.WP = 0;
		CAP_INFO_ARRAY[0].CAPST.BIT.BLK = 0;
		break;
	case CAP_ONE_SHOT_CHECK_CIS:
		address = (uint32_t)&ex_tmp_validation_data;
		address /= 4;
		CAP_INFO_ARRAY[0].BASE = address;
		CAP_INFO_ARRAY[0].SET.BIT.WP = 0;
		CAP_INFO_ARRAY[0].CAPST.BIT.BLK = 0;
		break;
	}
}

/*****************************************************************************/
/**
* Initialize sampling Other Sensor sequence. (8bit or 16bit)
*
* @param	None
*
* @return	None
*
* @note		None
*
******************************************************************************/
void _pl_sampling_ss_seq(void)
{
	int seq = 0;
	u8* p_Data;

	FPGA_SSSEQ_REG_WORD2_UNION ssseq_word2 = {0};
	FPGA_SSSEQ_REG_WORD3_UNION ssseq_word3 = {0};
	FPGA_SSSEQ_REG_WORD10_UNION ssseq_word10 = {0};
	FPGA_SSSEQ_REG_WORD11_UNION ssseq_word11 = {0};
	FPGA_SSSEQ_REG_WORD12_UNION ssseq_word12 = {0};
	FPGA_SSSEQ_REG_WORD13_UNION ssseq_word13 = {0};

	p_Data = (u8 *)FPGA_ADDR_SSSEQ;
	for(seq = 0; seq < sizeof(BV_SSSEQ_ARRAY_ST); seq++)
	{
		*(p_Data + seq) = 0;
	}
#if POINT_UV1_ENABLE || POINT_UV2_ENABLE
	if(ex_uba710 == 1)
	{
		ssseq_word10.BIT.UVId = 1;
	}
	else
	{
		ssseq_word10.BIT.UVId = 0;
	}
#else
	ssseq_word10.BIT.UVId = 0;
#endif
	ssseq_word10.BIT.UVAInc = (BLOCK_BYTE_SIZE)/4;
	ssseq_word11.BIT.UVAOfs = CISA_R_UV_OFFSET/4;
	ssseq_word12.BIT.UVDLOn = 0;
	ssseq_word12.BIT.UVTLOn = 4700;
	ssseq_word13.BIT.UVTAD1 = 4000;
	ssseq_word13.BIT.UVTAD2 = 0;

#if MAG1_ENABLE
	if(ex_uba710 == 1)
	{
		ssseq_word2.BIT.MGVLD = 1;
	}
	else
	{
		ssseq_word2.BIT.MGVLD = 0;
	}
#else
	ssseq_word2.BIT.MGVLD = 0;
#endif
	ssseq_word2.BIT.MGINC = (BLOCK_BYTE_SIZE)/4;
	ssseq_word3.BIT.MGOFS = CISA_R_MAG1_OFFSET/4;


	seq = 25;
	BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD2.LWORD = ssseq_word2.LWORD;
	BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD3.LWORD = ssseq_word3.LWORD;
	BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD10.LWORD = ssseq_word10.LWORD;
	BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD11.LWORD = ssseq_word11.LWORD;
	BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD12.LWORD = ssseq_word12.LWORD;
	BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD13.LWORD = ssseq_word13.LWORD;
}

/*****************************************************************************/
/**
* Initialize oneshot Other Sensor sequence. (8bit or 16bit)
*
* @param	- u8 type : sampling data bit type.
*
* @return	None
*
* @note		None
*
******************************************************************************/
void _pl_oneshot_ss_seq(u8 type)
{
	int seq = 0;
	u8* p_Data;

#if MAG1_ENABLE
	FPGA_SSSEQ_REG_WORD2_UNION ssseq_word2 = {0};
	FPGA_SSSEQ_REG_WORD3_UNION ssseq_word3 = {0};
#endif
	FPGA_SSSEQ_REG_WORD10_UNION ssseq_word10 = {0};
	FPGA_SSSEQ_REG_WORD11_UNION ssseq_word11 = {0};
	FPGA_SSSEQ_REG_WORD12_UNION ssseq_word12 = {0};
	FPGA_SSSEQ_REG_WORD13_UNION ssseq_word13 = {0};

	p_Data = (u8 *)FPGA_ADDR_SSSEQ;
	for(seq = 0; seq < sizeof(BV_SSSEQ_ARRAY_ST); seq++)
	{
		*(p_Data + seq) = 0;
	}

	if(type == CAP_ONE_SHOT_8BIT)
	{//CIS FLのタイミング
		seq = 9;
#if POINT_UV1_ENABLE && POINT_UV2_ENABLE
		if(ex_uba710 == 1)
		{
			ssseq_word10.BIT.UVId = 1;
			ssseq_word10.BIT.UVAInc = (sizeof(TMP_AD_TBL)/2)/4;				//cis uvと同じとこ（暫定）
			ssseq_word11.BIT.UVAOfs = (ONESHOT_ADDRESS_OFFSET_UP_R_FL)/4;	//cis uvと同じとこ（暫定）
		}	
#elif POINT_UV1_ENABLE && !POINT_UV2_ENABLE
	ssseq_word10.BIT.UVId = 1;
		ssseq_word10.BIT.UVAInc = (sizeof(TMP_AD_TBL))/4;				//cis uvと同じとこ（暫定）
		ssseq_word11.BIT.UVAOfs = (ONESHOT_ADDRESS_OFFSET_UP_R_FL)/4;	//cis uvと同じとこ（暫定）
#endif
		ssseq_word12.BIT.UVDLOn = 0;
		ssseq_word12.BIT.UVTLOn = 4700;
		ssseq_word13.BIT.UVTAD1 = 4000;
		ssseq_word13.BIT.UVTAD2 = 0;

		BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD10.LWORD = ssseq_word10.LWORD;
		BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD11.LWORD = ssseq_word11.LWORD;
		BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD12.LWORD = ssseq_word12.LWORD;
		BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD13.LWORD = ssseq_word13.LWORD;
	}
	else if(type == CAP_ONE_SHOT_10BIT)
	{//CIS FLのタイミング
		seq = 9;
#if POINT_UV1_ENABLE && POINT_UV2_ENABLE
		if(ex_uba710 == 1)
		{
			ssseq_word10.BIT.UVId = 1;
			ssseq_word10.BIT.UVAInc = (sizeof(TMP_10BIT_AD_TBL)/2)/4;				//cis uvと同じとこ（暫定）
			ssseq_word11.BIT.UVAOfs = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_FL)/4;	//cis uvと同じとこ（暫定）
		}
#elif POINT_UV1_ENABLE && !POINT_UV2_ENABLE
		ssseq_word10.BIT.UVId = 1;
		ssseq_word10.BIT.UVAInc = (sizeof(TMP_10BIT_AD_TBL))/4;				//cis uvと同じとこ（暫定）
		ssseq_word11.BIT.UVAOfs = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_FL)/4;	//cis uvと同じとこ（暫定）
#else
	ssseq_word10.BIT.UVId = 0;
#endif
	}
	else if(type == CAP_ONE_SHOT_MAG)
	{
#if MAG1_ENABLE
		if(ex_uba710 == 1)
		{
			seq = 0;
			ssseq_word2.BIT.MGVLD = 1;
			ssseq_word2.BIT.MGINC = (sizeof(TMP_MAG_AD_TBL))/4;
			ssseq_word3.BIT.MGOFS = ONESHOT_ADDRESS_MAG_OFFSET_MAG;
			BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD2.LWORD = ssseq_word2.LWORD;
			BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD3.LWORD = ssseq_word3.LWORD;
		}
#endif
	}

	ssseq_word12.BIT.UVDLOn = 0;
	ssseq_word12.BIT.UVTLOn = 4700;
	ssseq_word13.BIT.UVTAD1 = 4000;
	ssseq_word13.BIT.UVTAD2 = 0;

	BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD10.LWORD = ssseq_word10.LWORD;
	BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD11.LWORD = ssseq_word11.LWORD;
	BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD12.LWORD = ssseq_word12.LWORD;
	BV_SSSEQ_ARRAY.BV_SSSEQ_ARRAY_T[seq].WORD13.LWORD = ssseq_word13.LWORD;
}
/*****************************************************************************/
/**
* Initialize oneshot cis sequence. (8bit or 16bit)
*
* @param	- u8 type : sampling data bit type.
*
* @return	None
*
* @note		None
*
******************************************************************************/
void _pl_oneshot_cis_seq(u8 type)
{
	/* シーケンス初期化 */
	/*
		-S0-	-S1-	-S2-	-S3-	-S4-	-S5-	-S6-	-S7-	-S8-	-S9-
		R_RED	R_GRE	R_BLU	R_IR1	R_IR2	T_RED	T_GRE	T_IR1	T_IR2	R_FL
		return
	 */
	int cnt = 0;
	u8* p_Data;
	u16 time;
	u32 plane_a,plane_b;

	FPGA_CISSEQ_REG_WORD0_UNION cisseq_word0 = {0};
	FPGA_CISSEQ_REG_WORD1_UNION cisseq_word1 = {0};
	FPGA_CISSEQ_REG_WORD2_UNION cisseq_word2 = {0};
	FPGA_CISSEQ_REG_WORD3_UNION cisseq_word3 = {0};
	FPGA_CISSEQ_REG_WORD4_UNION cisseq_word4 = {0};
	FPGA_CISSEQ_REG_WORD5_UNION cisseq_word5 = {0};
	FPGA_CISSEQ_REG_WORD6_UNION cisseq_word6 = {0};
	FPGA_CISSEQ_REG_WORD7_UNION cisseq_word7 = {0};

	p_Data = (u8 *)FPGA_ADDR_CISSEQ;
	for(cnt = 0; cnt < sizeof(BV_CISSEQ_ARRAY_ST); cnt++)
	{
		*(p_Data + cnt) = 0;
	}
	/* 1 採取データ情報初期化 */
	_pl_cis_cap_info(type);

	if(type == CAP_ONE_SHOT_8BIT)
	{
		//SEQ 0(1-1)
		plane_a = UP_R_R;
		plane_b = DOWN_R_R;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.red_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.red_ref_time_u * ex_cis_adjustment_tmp.cis_pga.red_ref_pga_u);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.red_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.red_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.red_ref_time_d * ex_cis_adjustment_tmp.cis_pga.red_ref_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.red_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_UP_R_R)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_R_R)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 1(1-2)
		plane_a = UP_R_G;
		plane_b = DOWN_R_G;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.gre_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.gre_ref_time_u * ex_cis_adjustment_tmp.cis_pga.gre_ref_pga_u);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.gre_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.gre_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.gre_ref_time_d * ex_cis_adjustment_tmp.cis_pga.gre_ref_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.gre_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_UP_R_G)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_R_G)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD7.LWORD = cisseq_word7.LWORD;


		//SEQ 2(1-2)
		plane_a = UP_R_B;
		plane_b = DOWN_R_B;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.blu_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.blu_ref_time_u * ex_cis_adjustment_tmp.cis_pga.blu_ref_pga_u);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.blu_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.blu_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.blu_ref_time_d * ex_cis_adjustment_tmp.cis_pga.blu_ref_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word3.BIT.T_LED = time;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.blu_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_UP_R_B)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_R_B)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 3(1-3)
		plane_a = UP_R_IR1;
		plane_b = DOWN_R_IR1;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir1_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.ir1_ref_time_u * ex_cis_adjustment_tmp.cis_pga.ir1_ref_pga_u);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir1_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.ir1_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.ir1_ref_time_d * ex_cis_adjustment_tmp.cis_pga.ir1_ref_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir1_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_UP_R_IR1)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_R_IR1)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 4(1-4)
		plane_a = UP_R_IR2;
		plane_b = DOWN_R_IR2;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir2_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.ir2_ref_time_u * ex_cis_adjustment_tmp.cis_pga.ir2_ref_pga_u);
		if(time > LED_TIME_MAX2)
		{
			time = LED_TIME_MAX2;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir2_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.ir2_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.ir2_ref_time_d * ex_cis_adjustment_tmp.cis_pga.ir2_ref_pga_d);
		if(time > LED_TIME_MAX2)
		{
			time = LED_TIME_MAX2;
		}
		cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir2_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_UP_R_IR2)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_R_IR2)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 5(1-5)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_R;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.red_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.red_pen_time_u * ex_cis_adjustment_tmp.cis_pga.red_pen_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.red_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_T_R)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 6(1-6)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_G;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.gre_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.gre_pen_time_u * ex_cis_adjustment_tmp.cis_pga.gre_pen_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.gre_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_T_G)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 7(1-7)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_IR1;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir1_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.ir1_pen_time_u * ex_cis_adjustment_tmp.cis_pga.ir1_pen_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir1_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_T_IR1)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 8(1-8)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_IR2;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir2_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.ir2_pen_time_u * ex_cis_adjustment_tmp.cis_pga.ir2_pen_pga_d);
		if(time > LED_TIME_MAX2)
		{
			time = LED_TIME_MAX2;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir2_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_T_IR2)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 9(1-9)
		plane_a = UP_R_FL;
		plane_b = DOWN_R_FL;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
#if !CIS_UV_ENABLE
		cisseq_word2.BIT.ILED = 0;
#else
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.fl_ref_da_u;
#endif		
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.fl_ref_time_u * ex_cis_adjustment_tmp.cis_pga.fl_ref_pga_u);
		if(time > LED_TIME_MAX2)
		{
			time = LED_TIME_MAX2;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.fl_ref_da_u;
#if !CIS_UV_ENABLE
		cisseq_word3.BIT.ILED = 0;
#else
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.fl_ref_da_d;
#endif
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_data.cis_time.fl_ref_time_d * ex_cis_adjustment_tmp.cis_pga.fl_ref_pga_d);
		if(time > LED_TIME_MAX2)
		{
			time = LED_TIME_MAX2;
		}
		cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.fl_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_UP_R_FL)/4;
#if !CIS_UV_ENABLE
		cisseq_word5.BIT.DMY = 1;
#else
		cisseq_word5.BIT.DMY = 0;
#endif
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_R_FL)/4;
#if !CIS_UV_ENABLE
		cisseq_word7.BIT.DMY = 1;
#else
		cisseq_word7.BIT.DMY = 0;
#endif
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 10(1-10)
		plane_a = P_Reserve1;
		plane_b = P_Reserve4;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 0;
		cisseq_word0.BIT.NXT = 0;
		cisseq_word2.LWORD = 0;
		cisseq_word3.LWORD = 0;
		cisseq_word4.LWORD = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.LWORD = 0;
		cisseq_word7.BIT.AOFS = 0;
		cisseq_word7.BIT.DMY = 1;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD7.LWORD = cisseq_word7.LWORD;
	}
	else if(type == CAP_ONE_SHOT_10BIT)
	{
		//SEQ 0(1-1)
		plane_a = UP_R_R;
		plane_b = DOWN_R_R;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.red_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_data.cis_time.red_ref_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.red_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.red_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word3.BIT.T_LED = ex_cis_adjustment_data.cis_time.red_ref_time_d;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.red_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_R)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_R_R)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 1(1-2)
		plane_a = UP_R_G;
		plane_b = DOWN_R_G;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.gre_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_data.cis_time.gre_ref_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.gre_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.gre_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word3.BIT.T_LED = ex_cis_adjustment_data.cis_time.gre_ref_time_d;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.gre_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_G)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_R_G)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 2(1-2)
		plane_a = UP_R_B;
		plane_b = DOWN_R_B;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.blu_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_data.cis_time.blu_ref_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.blu_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.blu_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word3.BIT.T_LED = ex_cis_adjustment_data.cis_time.blu_ref_time_d;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.blu_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_B)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_R_B)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 3(1-3)
		plane_a = UP_R_IR1;
		plane_b = DOWN_R_IR1;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir1_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_data.cis_time.ir1_ref_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir1_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.ir1_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word3.BIT.T_LED = ex_cis_adjustment_data.cis_time.ir1_ref_time_d;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir1_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_IR1)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_R_IR1)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 4(1-4)
		plane_a = UP_R_IR2;
		plane_b = DOWN_R_IR2;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir2_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_data.cis_time.ir2_ref_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir2_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.ir2_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word3.BIT.T_LED = ex_cis_adjustment_data.cis_time.ir2_ref_time_d;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir2_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_IR2)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_R_IR2)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 5(1-5)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_R;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.red_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_data.cis_time.red_pen_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.red_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_T_R)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 6(1-6)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_G;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.gre_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_data.cis_time.gre_pen_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.gre_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_T_G)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 7(1-7)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_IR1;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir1_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_data.cis_time.ir1_pen_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir1_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_T_IR1)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 8(1-8)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_IR2;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir2_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_data.cis_time.ir2_pen_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir2_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_T_IR2)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 9(1-9)
		plane_a = UP_R_FL;
		plane_b = DOWN_R_FL;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
#if !CIS_UV_ENABLE
		cisseq_word2.BIT.ILED = 0;
#else
		cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.fl_ref_da_u;
#endif
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_data.cis_time.fl_ref_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.fl_ref_da_u;
#if !CIS_UV_ENABLE
		cisseq_word3.BIT.ILED = 0;
#else
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.fl_ref_da_d;
#endif
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word3.BIT.T_LED = ex_cis_adjustment_data.cis_time.fl_ref_time_d;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.fl_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_FL)/4;
#if !CIS_UV_ENABLE
		cisseq_word5.BIT.DMY = 1;
#else
		cisseq_word5.BIT.DMY = 0;
#endif
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_R_FL)/4;
#if !CIS_UV_ENABLE
		cisseq_word7.BIT.DMY = 1;
#else
		cisseq_word7.BIT.DMY = 0;
#endif
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 10(1-10)
		plane_a = UP_R_R;//黒レベル取得 無点灯赤反射で代用
		plane_b = DOWN_R_R;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = 0;
		cisseq_word2.BIT.LED = 0;
		cisseq_word2.BIT.T_LED = 0; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = 0;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = 0;
		cisseq_word3.BIT.T_LED = 0; //10000ns (10.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_RESERVE1)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = 0;
		cisseq_word6.BIT.WHADRS = 0;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_RESERVE2)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 11(1-11)
		plane_a = UP_R_FL;//黒レベル2種類目取得 無点灯UV反射で代用　TREP違いの黒レベル
		plane_b = DOWN_R_FL;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = 0;
		cisseq_word2.BIT.LED = 0;
		cisseq_word2.BIT.T_LED = 0;
		cisseq_word2.BIT.S_LED = 0;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = 0;
		cisseq_word3.BIT.T_LED = 0;
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_RESERVE4)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = 0;
		cisseq_word6.BIT.WHADRS = 0;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_RESERVE5)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 12(1-12)
		plane_a = P_Reserve1;
		plane_b = P_Reserve4;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 0;
		cisseq_word0.BIT.NXT = 0;
		cisseq_word2.LWORD = 0;
		cisseq_word3.LWORD = 0;
		cisseq_word4.LWORD = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.LWORD = 0;
		cisseq_word7.BIT.AOFS = 0;
		cisseq_word7.BIT.DMY = 1;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD7.LWORD = cisseq_word7.LWORD;
	}
	else if(type == CAP_ONE_SHOT_MAG)
	{
#if MAG1_ENABLE
		if(ex_uba710 == 1)
		{
			//SEQ 0(1-1)	MAGとるために仮設定
			plane_a = UP_R_R;
			plane_b = DOWN_R_R;
			cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
			cisseq_word0.BIT.COUNT = 1;
			cisseq_word0.BIT.NXT = 1;
			cisseq_word2.BIT.ILED = 0;
			cisseq_word2.BIT.LED = 0;
			cisseq_word2.BIT.T_LED = 0;
			cisseq_word2.BIT.S_LED = 0;
			cisseq_word3.BIT.ILED = 0;
			cisseq_word3.BIT.LED = 0;
			cisseq_word3.BIT.T_LED = 0;
			cisseq_word3.BIT.S_LED = 0;
			cisseq_word4.BIT.BKADRS = 0;
			cisseq_word4.BIT.WHADRS = 0;
			cisseq_word4.BIT.AINC = 0;
			cisseq_word5.BIT.AOFS = 0;
			cisseq_word5.BIT.DMY = 1;
			cisseq_word6.BIT.BKADRS = 0;
			cisseq_word6.BIT.WHADRS = 0;
			cisseq_word6.BIT.AINC = 0;
			cisseq_word7.BIT.AOFS = 0;
			cisseq_word7.BIT.DMY = 1;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD0.LWORD = cisseq_word0.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD1.LWORD = cisseq_word1.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD2.LWORD = cisseq_word2.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD3.LWORD = cisseq_word3.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD4.LWORD = cisseq_word4.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD5.LWORD = cisseq_word5.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD6.LWORD = cisseq_word6.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD7.LWORD = cisseq_word7.LWORD;
			//SEQ 1(1-2)
			plane_a = P_Reserve1;
			plane_b = P_Reserve4;
			cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
			cisseq_word0.BIT.COUNT = 0;
			cisseq_word0.BIT.NXT = 0;
			cisseq_word2.LWORD = 0;
			cisseq_word3.LWORD = 0;
			cisseq_word4.LWORD = 0;
			cisseq_word5.BIT.AOFS = 0;
			cisseq_word5.BIT.DMY = 1;
			cisseq_word6.LWORD = 0;
			cisseq_word7.BIT.AOFS = 0;
			cisseq_word7.BIT.DMY = 1;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD0.LWORD = cisseq_word0.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD1.LWORD = cisseq_word1.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD2.LWORD = cisseq_word2.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD3.LWORD = cisseq_word3.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD4.LWORD = cisseq_word4.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD5.LWORD = cisseq_word5.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD6.LWORD = cisseq_word6.LWORD;
			BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD7.LWORD = cisseq_word7.LWORD;
		}
#endif
	}
}
/*****************************************************************************/
/**
* Initialize oneshot non paper cis sequence. (8bit or 16bit)
*
* @param	- u8 type : sampling data bit type.
*
* @return	None
*
* @note		None
*
******************************************************************************/
void _pl_oneshot_non_paper_cis_seq(u8 type)
{
	/* シーケンス初期化 */
	/*
		-S0-	-S1-	-S2-	-S3-	-S4-	-S5-	-S6-	-S7-	-S8-	-S9-
		R_RED	R_IR1	R_GRE	R_BLU	R_IR2	T_GRE	T_RED	T_IR1	T_IR2	R_FL
		return
	 */
	int cnt = 0;
	u8* p_Data;
	u16 time;
	u32 plane_a,plane_b;

	FPGA_CISSEQ_REG_WORD0_UNION cisseq_word0 = {0};
	FPGA_CISSEQ_REG_WORD1_UNION cisseq_word1 = {0};
	FPGA_CISSEQ_REG_WORD2_UNION cisseq_word2 = {0};
	FPGA_CISSEQ_REG_WORD3_UNION cisseq_word3 = {0};
	FPGA_CISSEQ_REG_WORD4_UNION cisseq_word4 = {0};
	FPGA_CISSEQ_REG_WORD5_UNION cisseq_word5 = {0};
	FPGA_CISSEQ_REG_WORD6_UNION cisseq_word6 = {0};
	FPGA_CISSEQ_REG_WORD7_UNION cisseq_word7 = {0};

	p_Data = (u8 *)FPGA_ADDR_CISSEQ;
	for(cnt = 0; cnt < sizeof(BV_CISSEQ_ARRAY_ST); cnt++)
	{
		*(p_Data + cnt) = 0;
	}
	/* 1 採取データ情報初期化 */
	_pl_cis_cap_info(type);

	if(type == CAP_ONE_SHOT_8BIT)
	{
		//SEQ 0(1-1)
		plane_a = UP_R_R;
		plane_b = DOWN_R_R;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.red_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.red_ref_time_u * ex_cis_adjustment_tmp.cis_pga.red_ref_pga_u);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.red_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.red_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.red_ref_time_d * ex_cis_adjustment_tmp.cis_pga.red_ref_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.red_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_UP_R_R)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_R_R)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 1(1-2)
		plane_a = UP_R_G;
		plane_b = DOWN_R_G;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.gre_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.gre_ref_time_u * ex_cis_adjustment_tmp.cis_pga.gre_ref_pga_u);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.gre_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.gre_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.gre_ref_time_d * ex_cis_adjustment_tmp.cis_pga.gre_ref_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.gre_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_UP_R_G)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_R_G)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD7.LWORD = cisseq_word7.LWORD;


		//SEQ 2(1-2)
		plane_a = UP_R_B;
		plane_b = DOWN_R_B;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.blu_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.blu_ref_time_u * ex_cis_adjustment_tmp.cis_pga.blu_ref_pga_u);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.blu_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.blu_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.blu_ref_time_d * ex_cis_adjustment_tmp.cis_pga.blu_ref_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word3.BIT.T_LED = time;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.blu_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_UP_R_B)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_R_B)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 3(1-3)
		plane_a = UP_R_IR1;
		plane_b = DOWN_R_IR1;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.ir1_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.ir1_ref_time_u * ex_cis_adjustment_tmp.cis_pga.ir1_ref_pga_u);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.ir1_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.ir1_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.ir1_ref_time_d * ex_cis_adjustment_tmp.cis_pga.ir1_ref_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.ir1_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_UP_R_IR1)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_R_IR1)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 4(1-4)
		plane_a = UP_R_IR2;
		plane_b = DOWN_R_IR2;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.ir2_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.ir2_ref_time_u * ex_cis_adjustment_tmp.cis_pga.ir2_ref_pga_u);
		if(time > LED_TIME_MAX2)
		{
			time = LED_TIME_MAX2;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.ir2_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.ir2_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.ir2_ref_time_d * ex_cis_adjustment_tmp.cis_pga.ir2_ref_pga_d);
		if(time > LED_TIME_MAX2)
		{
			time = LED_TIME_MAX2;
		}
		cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.ir2_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_UP_R_IR2)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_R_IR2)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 5(1-5)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_R;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.red_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.red_pen_time_u * ex_cis_adjustment_tmp.cis_pga.red_pen_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.red_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_T_R)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 6(1-6)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_G;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.gre_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.gre_pen_time_u * ex_cis_adjustment_tmp.cis_pga.gre_pen_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.gre_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_T_G)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 7(1-7)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_IR1;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.ir1_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.ir1_pen_time_u * ex_cis_adjustment_tmp.cis_pga.ir1_pen_pga_d);
		if(time > LED_TIME_MAX)
		{
			time = LED_TIME_MAX;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.ir1_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_T_IR1)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 8(1-8)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_IR2;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.ir2_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.ir2_pen_time_u * ex_cis_adjustment_tmp.cis_pga.ir2_pen_pga_d);
		if(time > LED_TIME_MAX2)
		{
			time = LED_TIME_MAX2;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.ir2_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_T_IR2)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 9(1-9)
		plane_a = UP_R_FL;
		plane_b = DOWN_R_FL;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
#if !CIS_UV_ENABLE
		cisseq_word2.BIT.ILED = 0;
#else
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.fl_ref_da_u;
#endif
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.fl_ref_time_u * ex_cis_adjustment_tmp.cis_pga.fl_ref_pga_u);
		if(time > LED_TIME_MAX2)
		{
			time = LED_TIME_MAX2;
		}
		cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.fl_ref_da_u;
#if !CIS_UV_ENABLE
		cisseq_word3.BIT.ILED = 0;
#else
		cisseq_word3.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.fl_ref_da_d;
#endif
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		time = (u16)(ex_cis_adjustment_tmp.cis_tmp_time.fl_ref_time_d * ex_cis_adjustment_tmp.cis_pga.fl_ref_pga_d);
		if(time > LED_TIME_MAX2)
		{
			time = LED_TIME_MAX2;
		}
		cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.fl_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_UP_R_FL)/4;
#if !CIS_UV_ENABLE
		cisseq_word5.BIT.DMY = 1;
#else
		cisseq_word5.BIT.DMY = 0;
#endif
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_OFFSET_DOWN_R_FL)/4;
#if !CIS_UV_ENABLE
		cisseq_word7.BIT.DMY = 1;
#else
		cisseq_word7.BIT.DMY = 0;
#endif
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 10(1-10)
		plane_a = P_Reserve1;
		plane_b = P_Reserve4;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 0;
		cisseq_word0.BIT.NXT = 0;
		cisseq_word2.LWORD = 0;
		cisseq_word3.LWORD = 0;
		cisseq_word4.LWORD = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.LWORD = 0;
		cisseq_word7.BIT.AOFS = 0;
		cisseq_word7.BIT.DMY = 1;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD7.LWORD = cisseq_word7.LWORD;
	}
	else if(type == CAP_ONE_SHOT_10BIT)
	{
		//SEQ 0(1-1)
		plane_a = UP_R_R;
		plane_b = DOWN_R_R;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.red_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.red_ref_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.red_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.red_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word3.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.red_ref_time_d;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.red_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_R)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_R_R)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 1(1-2)
		plane_a = UP_R_G;
		plane_b = DOWN_R_G;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.gre_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.gre_ref_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.gre_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.gre_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word3.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.gre_ref_time_d;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.gre_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_G)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_R_G)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 2(1-2)
		plane_a = UP_R_B;
		plane_b = DOWN_R_B;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.blu_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.blu_ref_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.blu_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.blu_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word3.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.blu_ref_time_d;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.blu_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_B)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_R_B)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 3(1-3)
		plane_a = UP_R_IR1;
		plane_b = DOWN_R_IR1;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.ir1_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.ir1_ref_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.ir1_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.ir1_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word3.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.ir1_ref_time_d;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.ir1_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_IR1)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_R_IR1)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[3].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 4(1-4)
		plane_a = UP_R_IR2;
		plane_b = DOWN_R_IR2;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.ir2_ref_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.ir2_ref_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.ir2_ref_da_u;
		cisseq_word3.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.ir2_ref_da_d;
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word3.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.ir2_ref_time_d;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.ir2_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_IR2)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_R_IR2)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[4].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 5(1-5)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_R;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.red_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.red_pen_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.red_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_T_R)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[5].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 6(1-6)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_G;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.gre_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.gre_pen_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.gre_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_T_G)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[6].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 7(1-7)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_IR1;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.ir1_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.ir1_pen_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.ir1_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_T_IR1)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[7].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 8(1-8)
		plane_a = P_Reserve1;
		plane_b = DOWN_T_IR2;
		cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.ir2_pen_da_u;
		cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.ir2_pen_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.ir2_pen_da_u;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = LED_BLK;
		cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_T_IR2)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[8].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 9(1-9)
		plane_a = UP_R_FL;
		plane_b = DOWN_R_FL;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
#if !CIS_UV_ENABLE
		cisseq_word2.BIT.ILED = 0;
#else
		cisseq_word2.BIT.ILED = ex_cis_adjustment_tmp.cis_tmp_da.fl_ref_da_u;
#endif
		cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
		cisseq_word2.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.fl_ref_time_u;
		cisseq_word2.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_sled.fl_ref_da_u;
#if !CIS_UV_ENABLE
		cisseq_word3.BIT.ILED = 0;
#else
		cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.fl_ref_da_d;
#endif
		cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
		cisseq_word3.BIT.T_LED = ex_cis_adjustment_tmp.cis_tmp_time.fl_ref_time_d;
		cisseq_word3.BIT.S_LED = ex_cis_adjustment_tmp.cis_tmp_da.fl_ref_da_d;
		cisseq_word4.BIT.BKADRS = CapCPln[plane_a].BkAdr;
		cisseq_word4.BIT.WHADRS = CapCPln[plane_a].WhAdr;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_UP_R_FL)/4;
#if !CIS_UV_ENABLE
		cisseq_word5.BIT.DMY = 1;
#else
		cisseq_word5.BIT.DMY = 0;
#endif
		cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
		cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_DOWN_R_FL)/4;
#if !CIS_UV_ENABLE
		cisseq_word7.BIT.DMY = 1;
#else
		cisseq_word7.BIT.DMY = 0;
#endif
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[9].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 10(1-10)
		plane_a = UP_R_R;//黒レベル取得 無点灯赤反射で代用
		plane_b = DOWN_R_R;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = 0;
		cisseq_word2.BIT.LED = 0;
		cisseq_word2.BIT.T_LED = 0; //10000ns (10.0us) X/10
		cisseq_word2.BIT.S_LED = 0;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = 0;
		cisseq_word3.BIT.T_LED = 0; //10000ns (10.0us) X/10
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_RESERVE1)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = 0;
		cisseq_word6.BIT.WHADRS = 0;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_RESERVE2)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[10].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 11(1-11)
		plane_a = UP_R_FL;//黒レベル2種類目取得 無点灯UV反射で代用　TREP違いの黒レベル
		plane_b = DOWN_R_FL;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 1;
		cisseq_word0.BIT.NXT = 1;
		cisseq_word2.BIT.ILED = 0;
		cisseq_word2.BIT.LED = 0;
		cisseq_word2.BIT.T_LED = 0;
		cisseq_word2.BIT.S_LED = 0;
		cisseq_word3.BIT.ILED = 0;
		cisseq_word3.BIT.LED = 0;
		cisseq_word3.BIT.T_LED = 0;
		cisseq_word3.BIT.S_LED = 0;
		cisseq_word4.BIT.BKADRS = 0;
		cisseq_word4.BIT.WHADRS = 0;
		cisseq_word4.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word5.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_RESERVE4)/4;
		cisseq_word5.BIT.DMY = 0;
		cisseq_word6.BIT.BKADRS = 0;
		cisseq_word6.BIT.WHADRS = 0;
		cisseq_word6.BIT.AINC = (sizeof(TMP_10BIT_AD_TBL))/4;
		cisseq_word7.BIT.AOFS = (ONESHOT_ADDRESS_10BIT_OFFSET_RESERVE5)/4;
		cisseq_word7.BIT.DMY = 0;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[11].WORD7.LWORD = cisseq_word7.LWORD;

		//SEQ 12(1-12)
		plane_a = P_Reserve1;
		plane_b = P_Reserve4;
		cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
		cisseq_word0.BIT.COUNT = 0;
		cisseq_word0.BIT.NXT = 0;
		cisseq_word2.LWORD = 0;
		cisseq_word3.LWORD = 0;
		cisseq_word4.LWORD = 0;
		cisseq_word5.BIT.AOFS = 0;
		cisseq_word5.BIT.DMY = 1;
		cisseq_word6.LWORD = 0;
		cisseq_word7.BIT.AOFS = 0;
		cisseq_word7.BIT.DMY = 1;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD0.LWORD = cisseq_word0.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD1.LWORD = cisseq_word1.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD2.LWORD = cisseq_word2.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD3.LWORD = cisseq_word3.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD4.LWORD = cisseq_word4.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD5.LWORD = cisseq_word5.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD6.LWORD = cisseq_word6.LWORD;
		BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[12].WORD7.LWORD = cisseq_word7.LWORD;
	}
}
/*****************************************************************************/
/**
* Initialize sampling cis sequence.
*
* @param	None
*
* @return	None
*
* @note		None
*
******************************************************************************/
void _pl_sampling_cis_seq(void)
{
	/* シーケンス初期化 */
	/*
	SBLK0	-S0-	-S1-	-S2-	-S3-		-S4-		-S5-	-SX-
	UP		R_RED	R_IR1	T_GRE	R_GRE	R_IR2	-----	R_BLU	-----
	DN		R_RED	R_IR1	-----	R_GRE	R_IR2	-----	R_BLU	-----
	SBLK1	-S0-	-S1-	-S2-	-S3-		-S4-		-S5-	-SX-
	UP		R_RED	R_IR1	T_RED	R_GRE	-----	-----	R_BLU	-----
	DN		R_RED	R_IR1	-----	R_GRE	T_IR2	-----	R_BLU	-----
	SBLK2	-S0-	-S1-	-S2-	-S3-		-S4-		-S5-	-SX-
	UP		R_RED	R_IR1	T_GRE	R_GRE	R_IR2	-----	R_BLU	-----
	DN		R_RED	R_IR1	-----	R_GRE	R_IR2	-----	R_BLU	-----
	SBLK3	-S0-	-S1-	-S2-	-S3-		-S4-		-S5-	-SX-
	UP		R_RED	R_IR1	T_IR1	R_GRE	R_FL	-----	R_BLU	-----
	DN		R_RED	R_IR1	-----	R_GRE	R_FL	-----	R_BLU	-----
	 */
	int cnt = 0;
	int seq = 0;
	u8* p_Data;
	u16 time;
	u32 plane_a,plane_b;
	p_Data = (u8 *)FPGA_ADDR_CISSEQ;
	for(cnt = 0; cnt < sizeof(BV_CISSEQ_ARRAY_ST); cnt++)
	{
		*(p_Data + cnt) = 0;
	}
	/* 1 採取データ情報初期化 */
	_pl_cis_cap_info(CAP_SAMPLING);
	// 採取シーケンスメモリーを補正後データ採取用に設定
	set_seq_memory_8bit();

	//FPGA_CISSEQ_REG_WORD0_UNION cisseq_word0 = {0};
	FPGA_CISSEQ_REG_WORD1_UNION cisseq_word1 = {0};
	FPGA_CISSEQ_REG_WORD2_UNION cisseq_word2 = {0};
	FPGA_CISSEQ_REG_WORD3_UNION cisseq_word3 = {0};
	//FPGA_CISSEQ_REG_WORD4_UNION cisseq_word4 = {0};
	//FPGA_CISSEQ_REG_WORD5_UNION cisseq_word5 = {0};
	//FPGA_CISSEQ_REG_WORD6_UNION cisseq_word6 = {0};
	//FPGA_CISSEQ_REG_WORD7_UNION cisseq_word7 = {0};

/*****************************************************************************************/
	//SEQ 0(0-0)
	//SEQ 7(1-0)
	//SEQ 14(2-0)
	//SEQ 21(3-0)
	plane_a = UP_R_R;
	plane_b = DOWN_R_R;
	cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.red_ref_da_u;
	cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.red_ref_time_u * ex_cis_adjustment_tmp.cis_pga.red_ref_pga_u);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.red_ref_da_u;
	cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.red_ref_da_d;
	cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.red_ref_time_d * ex_cis_adjustment_tmp.cis_pga.red_ref_pga_d);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.red_ref_da_d;
	seq = 0;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 7;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 14;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 21;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;

	//SEQ 1(0-1)
	//SEQ 8(1-1)
	//SEQ 15(2-1)
	//SEQ 22(3-1)
	plane_a = UP_R_IR1;
	plane_b = DOWN_R_IR1;
	cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir1_ref_da_u;
	cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.ir1_ref_time_u * ex_cis_adjustment_tmp.cis_pga.ir1_ref_pga_u);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir1_ref_da_u;
	cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.ir1_ref_da_d;
	cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.ir1_ref_time_d * ex_cis_adjustment_tmp.cis_pga.ir1_ref_pga_d);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir1_ref_da_d;
	seq = 1;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 8;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 15;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 22;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;

	//SEQ 2(0-2)
	//SEQ 16(2-2)
	plane_a = P_Reserve1;
	plane_b = DOWN_T_G;

//2023-10-31	cisseq_word1.BIT.BinTh = 240;
	cisseq_word1.BIT.BinTh = 160;

	cisseq_word1.BIT.Aadd = 188;
	cisseq_word1.BIT.Mlt = 0;
	cisseq_word1.BIT.Bin = 1;
	cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.gre_pen_da_u;
	cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.gre_pen_time_u * ex_cis_adjustment_tmp.cis_pga.gre_pen_pga_d);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.gre_pen_da_u;
	cisseq_word3.BIT.ILED = 0;
	cisseq_word3.BIT.LED = LED_BLK;
	cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
	cisseq_word3.BIT.S_LED = 0;
	seq = 2;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD1.LWORD = cisseq_word1.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 16;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD1.LWORD = cisseq_word1.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;

	//SEQ 5(0-3)
	//SEQ 12(1-3)
	//SEQ 19(2-3)
	//SEQ 26(3-3)
	plane_a = UP_R_G;
	plane_b = DOWN_R_G;
	cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.gre_ref_da_u;
	cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.gre_ref_time_u * ex_cis_adjustment_tmp.cis_pga.gre_ref_pga_u);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.gre_ref_da_u;
	cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.gre_ref_da_d;
	cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.gre_ref_time_d * ex_cis_adjustment_tmp.cis_pga.gre_ref_pga_d);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.gre_ref_da_d;
	seq = 5;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 12;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 19;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 26;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;

	//SEQ 4(0-4)
	//SEQ 18(2-4)
	plane_a = UP_R_IR2;
	plane_b = DOWN_R_IR2;
	cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir2_ref_da_u;
	cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.ir2_ref_time_u * ex_cis_adjustment_tmp.cis_pga.ir2_ref_pga_u);
	if(time > LED_TIME_MAX2)
	{
		time = LED_TIME_MAX2;
	}
	cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir2_ref_da_u;
	cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.ir2_ref_da_d;
	cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.ir2_ref_time_d * ex_cis_adjustment_tmp.cis_pga.ir2_ref_pga_d);
	if(time > LED_TIME_MAX2)
	{
		time = LED_TIME_MAX2;
	}
	cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir2_ref_da_d;
	seq = 4;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 18;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;

	//SEQ 3(0-5)
	//SEQ 10(1-5)
	//SEQ 17(2-5)
	//SEQ 24(3-5)
	plane_a = UP_R_B;
	plane_b = DOWN_R_B;
	cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.blu_ref_da_u;
	cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.blu_ref_time_u * ex_cis_adjustment_tmp.cis_pga.blu_ref_pga_u);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.blu_ref_da_u;
	cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.blu_ref_da_d;
	cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.blu_ref_time_d * ex_cis_adjustment_tmp.cis_pga.blu_ref_pga_d);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.blu_ref_da_d;
	seq = 3;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 10;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 17;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
	seq = 24;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;

	//SEQ 9(1-2)
	plane_a = P_Reserve1;
	plane_b = DOWN_T_R;
	cisseq_word1.BIT.BinTh = 0;
	cisseq_word1.BIT.Aadd = 0;
	cisseq_word1.BIT.Mlt = 1;
	cisseq_word1.BIT.Bin = 0;
	cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.red_pen_da_u;
	cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.red_pen_time_u * ex_cis_adjustment_tmp.cis_pga.red_pen_pga_d);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.red_pen_da_u;
	cisseq_word3.BIT.ILED = 0;
	cisseq_word3.BIT.LED = LED_BLK;
	cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
	cisseq_word3.BIT.S_LED = 0;
	seq = 9;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD1.LWORD = cisseq_word1.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;

	//SEQ 23(1-4)
	plane_a = P_Reserve1;
	plane_b = DOWN_T_IR1;
	cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir1_pen_da_u;
	cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.ir1_pen_time_u * ex_cis_adjustment_tmp.cis_pga.ir1_pen_pga_d);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir1_pen_da_u;
	cisseq_word3.BIT.ILED = 0;
	cisseq_word3.BIT.LED = LED_BLK;
	cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
	cisseq_word3.BIT.S_LED = 0;
	seq = 23;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;

	//SEQ 11(3-2)
	plane_a = P_Reserve1;
	plane_b = DOWN_T_IR2;
	cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir2_pen_da_u;
	cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.ir2_pen_time_u * ex_cis_adjustment_tmp.cis_pga.ir2_pen_pga_d);
	if(time > LED_TIME_MAX2)
	{
		time = LED_TIME_MAX2;
	}
	cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir2_pen_da_u;
	cisseq_word3.BIT.ILED = 0;
	cisseq_word3.BIT.LED = LED_BLK;
	cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
	cisseq_word3.BIT.S_LED = 0;
	seq = 11;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;

	//SEQ 25(3-4)
	plane_a = UP_R_FL;
	plane_b = DOWN_R_FL;
	cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.fl_ref_da_u;
	cisseq_word2.BIT.LED = CapCPln[plane_a].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.fl_ref_time_u * ex_cis_adjustment_tmp.cis_pga.fl_ref_pga_u);
	if(time > LED_TIME_MAX2)
	{
		time = LED_TIME_MAX2;
	}
	cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.fl_ref_da_u;
	cisseq_word3.BIT.ILED = ex_cis_adjustment_data.cis_da.fl_ref_da_d;
	cisseq_word3.BIT.LED = CapCPln[plane_b].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.blu_ref_time_d * ex_cis_adjustment_tmp.cis_pga.fl_ref_pga_d);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word3.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word3.BIT.S_LED = ex_cis_adjustment_data.cis_sled.fl_ref_da_d;
	seq = 25;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[seq].WORD3.LWORD = cisseq_word3.LWORD;
}
/*****************************************************************************/
/**
* Initialize fpga register.
*
* @param	None
*
* @return	None
*
* @note		None
*
******************************************************************************/
void _pl_cis_init_fpga(void)
{
	int cnt = 0;

	/* FPGA共通初期化処理 */
	CAP_MAXBK_UNION cap_maxbk = {0};
	CAP_IDX_UNION cap_idx = {0};
	CAP_IDXMAX_UNION cap_idxmax = {0};
	CAP_MD_UNION cap_md = {0};
	CAP_BLID_UNION cap_blid = {0};
	CAP_CINIT_UNION cap_cinit = {0};
	CAP_CCIS_UNION cap_ccis = {0};
	CAP_CMG_UNION cap_cmg = {0};
	CAP_PLS_UNION cap_pls = {0};
	CIS_MD_UNION cis_md = {0};
	CIS_BKI_UNION cis_bki = {0};
	BIN_RNG_UNION bin_rng = {0};
	BIN_INTSET_UNION bin_intset = {0};
	MLT_TH_UNION mlt_th = {0};

	cap_maxbk.BIT.SCAN_MAX_BLOCK = (SCAN_BLOCK_SIZE-1);	/* W/R	最大ブロック数-1 */
	cap_idx.BIT.SCAN_NO = 0;							/* W/R	採取番号 */
	cap_idxmax.BIT.SCAN_BLK_MAX = 0;					/* W/R	採取番号最大値 */
	cap_md.BIT.CAPEN = 0;								/* W/R	採取モード */
	cap_blid.BIT.BILL_ID = 0;							/* W/R	媒体ID */
	cap_cinit.BIT.SCAN_TIMER = 1;						/* W/R	採取タイマーセット値 */
	cap_ccis.BIT.CIS_START_COUNT = 1;					/* W/R	CISスタートカウント値 */
#if MAG1_ENABLE
	if(ex_uba710 == 1)
	{
		cap_cmg.BIT.MAG_START_COUNT = 1;					/* W/R	磁気センサスタート */
	}
	else
	{
		cap_cmg.BIT.MAG_START_COUNT = 0;					/* W/R	磁気センサスタート */
	}
#else
	cap_cmg.BIT.MAG_START_COUNT = 0;					/* W/R	磁気センサスタート */
#endif
	cap_pls.BIT.ITVL = ENCODER_CYCLE;					/* W/R	疑似搬送パルス周期-1 (10ns) */
	cap_pls.BIT.SEL = 0;								/* W/R	搬送パルス選択(エンコーダパルス) */
	cis_md.BIT.CMP = 1;									/* W/R	CISモード */
	cis_md.BIT.CONT = 0;								/* W/R	CISモード */
	cis_md.BIT.DARK = 0;								/* W/R	CISモード */
	cis_bki.BIT.CISA = -18;								/* W/R	CISデータブロック番号初期値 */
	cis_bki.BIT.CISB = -1;								/* W/R	CISデータブロック番号初期値 */

	FPGA_REG.CAP_MAXBK.LWORD = cap_maxbk.LWORD;
	FPGA_REG.CAP_IDX.LWORD = cap_idx.LWORD;
	FPGA_REG.CAP_IDXMAX.LWORD = cap_idxmax.LWORD;
	FPGA_REG.CAP_MD.LWORD = cap_md.LWORD;
	FPGA_REG.CAP_BLID.LWORD = cap_blid.LWORD;
	FPGA_REG.CAP_CINIT.LWORD = cap_cinit.LWORD;
	FPGA_REG.CAP_CCIS.LWORD = cap_ccis.LWORD;
	FPGA_REG.CAP_CMG.LWORD = cap_cmg.LWORD;
	FPGA_REG.CAP_PLS.LWORD = cap_pls.LWORD;
	FPGA_REG.CIS_MD.LWORD = cis_md.LWORD;
	FPGA_REG.CIS_BKI.LWORD = cis_bki.LWORD;
	FPGA_REG.BIN_RNG.LWORD = bin_rng.LWORD;
	FPGA_REG.BIN_INTSET.LWORD = bin_intset.LWORD;
	FPGA_REG.MLT_TH.LWORD = mlt_th.LWORD;

	for(cnt = 0; cnt < 288; cnt++)
	{
		/* outside */
		//R_RED
		CISCMPAB_REGISTER.TBL0[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data1_u[0 + cnt];
		CISCMPAB_REGISTER.TBL0[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data1_u[288 + cnt];
		CISCMPAW_REGISTER.TBL0[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.red_ref_u[0 + cnt];
		CISCMPAW_REGISTER.TBL0[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.red_ref_u[288 + cnt];
		//R_GRE
		CISCMPAB_REGISTER.TBL1[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data1_u[0 + cnt];
		CISCMPAB_REGISTER.TBL1[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data1_u[288 + cnt];
		CISCMPAW_REGISTER.TBL1[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.gre_ref_u[0 + cnt];
		CISCMPAW_REGISTER.TBL1[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.gre_ref_u[288 + cnt];
		//R_BLU
		CISCMPAB_REGISTER.TBL2[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data1_u[0 + cnt];
		CISCMPAB_REGISTER.TBL2[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data1_u[288 + cnt];
		CISCMPAW_REGISTER.TBL2[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.blu_ref_u[0 + cnt];
		CISCMPAW_REGISTER.TBL2[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.blu_ref_u[288 + cnt];
		//R_IR1
		CISCMPAB_REGISTER.TBL3[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data1_u[0 + cnt];
		CISCMPAB_REGISTER.TBL3[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data1_u[288 + cnt];
		CISCMPAW_REGISTER.TBL3[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.ir1_ref_u[0 + cnt];
		CISCMPAW_REGISTER.TBL3[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.ir1_ref_u[288 + cnt];
		//R_IR2
		CISCMPAB_REGISTER.TBL4[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data2_u[0 + cnt];
		CISCMPAB_REGISTER.TBL4[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data2_u[288 + cnt];
		CISCMPAW_REGISTER.TBL4[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.ir2_ref_u[0 + cnt];
		CISCMPAW_REGISTER.TBL4[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.ir2_ref_u[288 + cnt];
		//R_FL
		CISCMPAB_REGISTER.TBL5[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data2_u[0 + cnt];
		CISCMPAB_REGISTER.TBL5[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data2_u[288 + cnt];
		CISCMPAW_REGISTER.TBL5[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.fl_ref_u[0 + cnt];
		CISCMPAW_REGISTER.TBL5[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.fl_ref_u[288 + cnt];

		/* inside */
		//R_RED
		CISCMPBB_REGISTER.TBL0[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data1_d[0 + cnt];
		CISCMPBB_REGISTER.TBL0[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data1_d[288 + cnt];
		CISCMPBW_REGISTER.TBL0[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.red_ref_d[0 + cnt];
		CISCMPBW_REGISTER.TBL0[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.red_ref_d[288 + cnt];
		//R_GRE
		CISCMPBB_REGISTER.TBL1[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data1_d[0 + cnt];
		CISCMPBB_REGISTER.TBL1[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data1_d[288 + cnt];
		CISCMPBW_REGISTER.TBL1[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.gre_ref_d[0 + cnt];
		CISCMPBW_REGISTER.TBL1[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.gre_ref_d[288 + cnt];
		//R_BLU
		CISCMPBB_REGISTER.TBL2[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data1_d[0 + cnt];
		CISCMPBB_REGISTER.TBL2[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data1_d[288 + cnt];
		CISCMPBW_REGISTER.TBL2[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.blu_ref_d[0 + cnt];
		CISCMPBW_REGISTER.TBL2[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.blu_ref_d[288 + cnt];
		//R_IR1
		CISCMPBB_REGISTER.TBL3[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data1_d[0 + cnt];
		CISCMPBB_REGISTER.TBL3[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data1_d[288 + cnt];
		CISCMPBW_REGISTER.TBL3[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.ir1_ref_d[0 + cnt];
		CISCMPBW_REGISTER.TBL3[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.ir1_ref_d[288 + cnt];
		//R_IR2
		CISCMPBB_REGISTER.TBL4[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data2_d[0 + cnt];
		CISCMPBB_REGISTER.TBL4[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data2_d[288 + cnt];
		CISCMPBW_REGISTER.TBL4[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.ir2_ref_d[0 + cnt];
		CISCMPBW_REGISTER.TBL4[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.ir2_ref_d[288 + cnt];
		//R_FL
		CISCMPBB_REGISTER.TBL5[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data2_d[0 + cnt];
		CISCMPBB_REGISTER.TBL5[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data2_d[288 + cnt];
		CISCMPBW_REGISTER.TBL5[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.fl_ref_d[0 + cnt];
		CISCMPBW_REGISTER.TBL5[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.fl_ref_d[288 + cnt];
		//T_RED
		CISCMPBB_REGISTER.TBL6[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data1_d[0 + cnt];
		CISCMPBB_REGISTER.TBL6[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data1_d[288 + cnt];
		CISCMPBW_REGISTER.TBL6[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.red_pen_d[0 + cnt];
		CISCMPBW_REGISTER.TBL6[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.red_pen_d[288 + cnt];
		//T_GRE
		CISCMPBB_REGISTER.TBL7[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data1_d[0 + cnt];
		CISCMPBB_REGISTER.TBL7[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data1_d[288 + cnt];
		CISCMPBW_REGISTER.TBL7[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.gre_pen_d[0 + cnt];
		CISCMPBW_REGISTER.TBL7[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.gre_pen_d[288 + cnt];
		//T_IR1
		CISCMPBB_REGISTER.TBL8[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data1_d[0 + cnt];
		CISCMPBB_REGISTER.TBL8[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data1_d[288 + cnt];
		CISCMPBW_REGISTER.TBL8[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.ir1_pen_d[0 + cnt];
		CISCMPBW_REGISTER.TBL8[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.ir1_pen_d[288 + cnt];
		//T_IR2
		CISCMPBB_REGISTER.TBL9[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_bc.black_data2_d[0 + cnt];
		CISCMPBB_REGISTER.TBL9[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_bc.black_data2_d[288 + cnt];
		CISCMPBW_REGISTER.TBL9[cnt].BIT.CH0 = ex_cis_adjustment_data.cis_wc.ir2_pen_d[0 + cnt];
		CISCMPBW_REGISTER.TBL9[cnt].BIT.CH1 = ex_cis_adjustment_data.cis_wc.ir2_pen_d[288 + cnt];
	}
	for(cnt = 0; cnt < 144; cnt++)
	{
		//R_RED
		CISCMPAB_REGISTER.TBL0[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data1_u[576 + cnt];
		CISCMPAW_REGISTER.TBL0[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.red_ref_u[576 + cnt];
		//R_GRE
		CISCMPAB_REGISTER.TBL1[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data1_u[576 + cnt];
		CISCMPAW_REGISTER.TBL1[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.gre_ref_u[576 + cnt];
		//R_BLU
		CISCMPAB_REGISTER.TBL2[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data1_u[576 + cnt];
		CISCMPAW_REGISTER.TBL2[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.blu_ref_u[576 + cnt];
		//R_IR1
		CISCMPAB_REGISTER.TBL3[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data1_u[576 + cnt];
		CISCMPAW_REGISTER.TBL3[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.ir1_ref_u[576 + cnt];
		//R_IR2
		CISCMPAB_REGISTER.TBL4[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data2_u[576 + cnt];
		CISCMPAW_REGISTER.TBL4[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.ir2_ref_u[576 + cnt];
		//R_FL
		CISCMPAB_REGISTER.TBL5[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data2_u[576 + cnt];
		CISCMPAW_REGISTER.TBL5[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.fl_ref_u[576 + cnt];
		//R_RED
		CISCMPBB_REGISTER.TBL0[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data1_d[576 + cnt];
		CISCMPBW_REGISTER.TBL0[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.red_ref_d[576 + cnt];
		//R_GRE
		CISCMPBB_REGISTER.TBL1[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data1_d[576 + cnt];
		CISCMPBW_REGISTER.TBL1[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.gre_ref_d[576 + cnt];
		//R_BLU
		CISCMPBB_REGISTER.TBL2[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data1_d[576 + cnt];
		CISCMPBW_REGISTER.TBL2[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.blu_ref_d[576 + cnt];
		//R_IR1
		CISCMPBB_REGISTER.TBL3[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data1_d[576 + cnt];
		CISCMPBW_REGISTER.TBL3[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.ir1_ref_d[576 + cnt];
		//R_IR2
		CISCMPBB_REGISTER.TBL4[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data2_d[576 + cnt];
		CISCMPBW_REGISTER.TBL4[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.ir2_ref_d[576 + cnt];
		//R_FL
		CISCMPBB_REGISTER.TBL5[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data2_d[576 + cnt];
		CISCMPBW_REGISTER.TBL5[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.fl_ref_d[576 + cnt];
		//T_RED
		CISCMPBB_REGISTER.TBL6[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data1_d[576 + cnt];
		CISCMPBW_REGISTER.TBL6[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.red_pen_d[576 + cnt];
		//T_GRE
		CISCMPBB_REGISTER.TBL7[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data1_d[576 + cnt];
		CISCMPBW_REGISTER.TBL7[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.gre_pen_d[576 + cnt];
		//T_IR1
		CISCMPBB_REGISTER.TBL8[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data1_d[576 + cnt];
		CISCMPBW_REGISTER.TBL8[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.ir1_pen_d[576 + cnt];
		//T_IR2
		CISCMPBB_REGISTER.TBL9[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_bc.black_data2_d[576 + cnt];
		CISCMPBW_REGISTER.TBL9[cnt].BIT.CH2 = ex_cis_adjustment_data.cis_wc.ir2_pen_d[576 + cnt];
	}
}
void sspi_wait_test_ready(u8 add)
{
	SSPI_ST_UNION sspi_st = {0};
	sspi_st.BIT.PTR = add;

	do {
		sspi_st.LWORD = FPGA_REG.SSPI_ST.LWORD;
	} while (sspi_st.BIT.BUSY != 0);		// ビジー解除待ち
}
void sspi_cis_test_initialize(u8 side, u8 add)
{
	SSPI_CMD_UNION sspi_cmd = {0};

	sspi_wait_test_ready(add);
	sspi_cmd.BIT.SZ = 2 - 1;		// (転送数 - 1)を設定
	sspi_cmd.BIT.DN = (SSPI_DN_T)side;
	FPGA_REG.SSPI_CMD.LWORD = sspi_cmd.LWORD;
	sspi_wait_test_ready(add);
}
/*****************************************************************************/
/**
* Initialize afe data.
*
* @param	None
*
* @return	None
*
* @note		None
*
******************************************************************************/
void _pl_cis_init_afe(void)
{
	u32 offset = 0;
	u32 *p_Word;
	FPGA_REG_LWORD_UNION lword = {0};

	p_Word = (u32 *)FPGA_ADDR_SSPITDAT;
	offset = 0;

	lword.BIT.DATA0 = 0x00;					//-1h
	lword.BIT.DATA1 = BANKCNTRL_VALUE;		//00h
	lword.BIT.DATA2 = CNTRL_VALUE;			//01h
	lword.BIT.DATA3 = (u8)((ex_cis_adjustment_data.afe_again.shg0_u[3] << 3)
						+ (ex_cis_adjustment_data.afe_again.shg0_u[2] << 2)
						+ (ex_cis_adjustment_data.afe_again.shg0_u[1] << 1)
						+ (ex_cis_adjustment_data.afe_again.shg0_u[0] << 0));		//02h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = ex_cis_adjustment_data.afe_aoffset.offdac_u[0];		//03h
	lword.BIT.DATA1 = ex_cis_adjustment_data.afe_aoffset.offdac_u[1];		//04h
	lword.BIT.DATA2 = ex_cis_adjustment_data.afe_aoffset.offdac_u[2];		//05h
	lword.BIT.DATA3 = ex_cis_adjustment_data.afe_aoffset.offdac_u[3];		//06h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = ex_cis_adjustment_data.afe_dgain.dgain_u[0];		//07h
	lword.BIT.DATA1 = ex_cis_adjustment_data.afe_dgain.dgain_u[1];		//08h
	lword.BIT.DATA2 = ex_cis_adjustment_data.afe_dgain.dgain_u[2];		//09h
	lword.BIT.DATA3 = ex_cis_adjustment_data.afe_dgain.dgain_u[3];		//0Ah
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = OFFSET00_MSB_VALUE;		//0Bh
	lword.BIT.DATA1 = OFFSET00_LSB_VALUE;		//0Ch
	lword.BIT.DATA2 = OFFSET01_MSB_VALUE;		//0Dh
	lword.BIT.DATA3 = OFFSET01_LSB_VALUE;		//0Eh
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = OFFSET02_MSB_VALUE;		//0Fh
	lword.BIT.DATA1 = OFFSET02_LSB_VALUE;		//10h
	lword.BIT.DATA2 = OFFSET03_MSB_VALUE;		//11h
	lword.BIT.DATA3 = OFFSET03_LSB_VALUE;		//12h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = OUTCTRL1_VALUE;			//13h
	lword.BIT.DATA1 = OUTCTRL2_VALUE;			//14h
	lword.BIT.DATA2 = OUTCTRL3_VALUE;			//15h
	lword.BIT.DATA3 = OUTCTRL4_VALUE;			//16h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = TRIG_VALUE;				//17h
	lword.BIT.DATA1 = TGOUT1_VALUE;				//18h
	lword.BIT.DATA2 = TGOUT2_VALUE;				//19h
	lword.BIT.DATA3 = TGOUT3_VALUE;				//1Ah
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = PCISCKR_VALUE;			//1Bh
	lword.BIT.DATA1 = PCISCKF_VALUE;			//1Ch
	lword.BIT.DATA2 = PSHDF_VALUE;				//1Dh
	lword.BIT.DATA3 = LEDCTRL_VALUE;			//1Eh
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = COUNT0_MSB_VALUE;			//1Fh
	lword.BIT.DATA1 = COUNT0_LSB_VALUE;			//20h
	lword.BIT.DATA2 = COUNT1_MSB_VALUE;			//21h
	lword.BIT.DATA3 = COUNT1_LSB_VALUE;			//22h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = COUNT2_MSB_VALUE;			//23h
	lword.BIT.DATA1 = COUNT2_LSB_VALUE;			//24h
	lword.BIT.DATA2 = 0;//RESERVE_25_VALUE;		//25h
	lword.BIT.DATA3 = 0;//RESERVE_26_VALUE;		//26h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = 0;//RESERVE_27_VALUE;		//27h
	lword.BIT.DATA1 = 0;//RESERVE_28_VALUE;		//28h
	lword.BIT.DATA2 = LEDEN_RRISE1_MSB_VALUE;	//29h
	lword.BIT.DATA3 = LEDEN_RRISE1_LSB_VALUE;	//2Ah
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = LEDEN_RFALL1_MSB_VALUE;	//2Bh
	lword.BIT.DATA1 = LEDEN_RFALL1_LSB_VALUE;	//2Ch
	lword.BIT.DATA2 = LEDEN_RRISE2_MSB_VALUE;	//2Dh
	lword.BIT.DATA3 = LEDEN_RRISE2_LSB_VALUE;	//2Eh
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = LEDEN_RFALL2_MSB_VALUE;	//2Fh
	lword.BIT.DATA1 = LEDEN_RFALL2_LSB_VALUE;	//30h
	lword.BIT.DATA2 = LEDEN_RRISE3_MSB_VALUE;	//31h
	lword.BIT.DATA3 = LEDEN_RRISE3_LSB_VALUE;	//32h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = LEDEN_RFALL3_MSB_VALUE;	//33h
	lword.BIT.DATA1 = LEDEN_RFALL3_LSB_VALUE;	//34h
	lword.BIT.DATA2 = LEDEN_RRISE4_MSB_VALUE;	//35h
	lword.BIT.DATA3 = LEDEN_RRISE4_LSB_VALUE;	//36h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = LEDEN_RFALL4_MSB_VALUE;	//37h
	lword.BIT.DATA1 = LEDEN_RFALL4_LSB_VALUE;	//38h
	lword.BIT.DATA2 = LEDEN_GRISE1_MSB_VALUE;	//39h
	lword.BIT.DATA3 = LEDEN_GRISE1_LSB_VALUE;	//3Ah
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = LEDEN_GFALL1_MSB_VALUE;	//3Bh
	lword.BIT.DATA1 = LEDEN_GFALL1_LSB_VALUE;	//3Ch
	lword.BIT.DATA2 = LEDEN_GRISE2_MSB_VALUE;	//3Dh
	lword.BIT.DATA3 = LEDEN_GRISE2_LSB_VALUE;	//3Eh
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = LEDEN_GFALL2_MSB_VALUE;	//3Fh
	lword.BIT.DATA1 = LEDEN_GFALL2_LSB_VALUE;	//40h
	lword.BIT.DATA2 = LEDEN_GRISE3_MSB_VALUE;	//41h
	lword.BIT.DATA3 = LEDEN_GRISE3_LSB_VALUE;	//42h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = LEDEN_GFALL3_MSB_VALUE;	//43h
	lword.BIT.DATA1 = LEDEN_GFALL3_LSB_VALUE;	//44h
	lword.BIT.DATA2 = LEDEN_GRISE4_MSB_VALUE;	//45h
	lword.BIT.DATA3 = LEDEN_GRISE4_LSB_VALUE;	//46h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = LEDEN_GFALL4_MSB_VALUE;	//47h
	lword.BIT.DATA1 = LEDEN_GFALL4_LSB_VALUE;	//48h
	lword.BIT.DATA2 = LEDEN_BRISE1_MSB_VALUE;	//49h
	lword.BIT.DATA3 = LEDEN_BRISE1_LSB_VALUE;	//4Ah
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = LEDEN_BFALL1_MSB_VALUE;	//4Bh
	lword.BIT.DATA1 = LEDEN_BFALL1_LSB_VALUE;	//4Ch
	lword.BIT.DATA2 = LEDEN_BRISE2_MSB_VALUE;	//4Dh
	lword.BIT.DATA3 = LEDEN_BRISE2_LSB_VALUE;	//4Eh
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = LEDEN_BFALL2_MSB_VALUE;	//4Fh
	lword.BIT.DATA1 = LEDEN_BFALL2_LSB_VALUE;	//50h
	lword.BIT.DATA2 = LEDEN_BRISE3_MSB_VALUE;	//51h
	lword.BIT.DATA3 = LEDEN_BRISE3_LSB_VALUE;	//52h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = LEDEN_BFALL3_MSB_VALUE;	//53h
	lword.BIT.DATA1 = LEDEN_BFALL3_LSB_VALUE;	//54h
	lword.BIT.DATA2 = LEDEN_BRISE4_MSB_VALUE;	//55h
	lword.BIT.DATA3 = LEDEN_BRISE4_LSB_VALUE;	//56h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = LEDEN_BFALL4_MSB_VALUE;	//57h
	lword.BIT.DATA1 = LEDEN_BFALL4_LSB_VALUE;	//58h
	lword.BIT.DATA2 = SP2START_MSB_VALUE;		//59h
	lword.BIT.DATA3 = SP2START_LSB_VALUE;		//5Ah
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = SP2HWIDTH_MSB_VALUE;		//5Bh
	lword.BIT.DATA1 = SP2HWIDTH_LSB_VALUE;		//5Ch
	lword.BIT.DATA2 = SP2PERIOD_MSB_VALUE;		//5Dh
	lword.BIT.DATA3 = SP2PERIOD_LSB_VALUE;		//5Eh
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = SP2COUNT_VALUE;			//5Fh
	lword.BIT.DATA1 = BOSRISE0_MSB_VALUE;		//60h
	lword.BIT.DATA2 = BOSRISE0_LSB_VALUE;		//61h
	lword.BIT.DATA3 = BOSFALL0_MSB_VALUE;		//62h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = BOSFALL0_LSB_VALUE;		//63h
	lword.BIT.DATA1 = ENRISE_MSB_VALUE;			//64h
	lword.BIT.DATA2 = ENRISE_LSB_VALUE;			//65h
	lword.BIT.DATA3 = ENFALL_MSB_VALUE;			//66h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = ENFALL_LSB_VALUE;			//67h
	lword.BIT.DATA1 = TRIGWIDTH_MSB_VALUE;		//68h
	lword.BIT.DATA2 = TRIGWIDTH_LSB_VALUE;		//69h
	lword.BIT.DATA3 = CISCK_MASK_START_MSB_VALUE;//6Ah
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = CISCK_MASK_START_LSB_VALUE;//6Bh
	lword.BIT.DATA1 = CISCK_MASK_END_MSB_VALUE;	//6Ch
	lword.BIT.DATA2 = CISCK_MASK_END_LSB_VALUE;	//6Dh
	lword.BIT.DATA3 = ISELR_VALUE;				//6Eh
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = ISELG_VALUE;				//6Fh
	lword.BIT.DATA1 = ISELB_VALUE;				//70h
	lword.BIT.DATA2 = 0;//RESERVE_71_VALUE;		//71h
	lword.BIT.DATA3 = ADCK_CONTROL_VALUE;		//72h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = 0;//RESERVE_73_VALUE;		//73h
	lword.BIT.DATA1 = 0;//RESERVE_74_VALUE;		//74h
	lword.BIT.DATA2 = 0;//RESERVE_75_VALUE;		//75h
	lword.BIT.DATA3 = 0;//RESERVE_76_VALUE;		//76h
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = ADCK_PH_VALUE;			//77h
	lword.BIT.DATA1 = 0;//RESERVE_78_VALUE;		//78h
	lword.BIT.DATA2 = 0;//RESERVE_79_VALUE;		//79h
	lword.BIT.DATA3 = 0;//RESERVE_7A_VALUE;		//7Ah
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = 0;//RESERVE_7B_VALUE;		//7Bh
	lword.BIT.DATA1 = 0;//RESERVE_7C_VALUE;		//7Ch
	lword.BIT.DATA2 = 0;//RESERVE_7D_VALUE;		//7Dh
	lword.BIT.DATA3 = 0;//RESERVE_7E_VALUE;		//7Eh
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = 0;//RESERVE_7F_VALUE;		//7Fh
	lword.BIT.DATA1 = 0;
	lword.BIT.DATA2 = 0;
	lword.BIT.DATA3 = 0;
	*(p_Word + offset++) = lword.LWORD;

	//afe initialize
	sspi_cis_initialize(SSPI_DN_CISA);

	offset = 0;
	lword.BIT.DATA0 = 0x00;
	lword.BIT.DATA1 = BANKCNTRL_VALUE;
	lword.BIT.DATA2 = CNTRL_VALUE;
	lword.BIT.DATA3 = (u8)((ex_cis_adjustment_data.afe_again.shg0_d[3] << 3)
						+ (ex_cis_adjustment_data.afe_again.shg0_d[2] << 2)
						+ (ex_cis_adjustment_data.afe_again.shg0_d[1] << 1)
						+ (ex_cis_adjustment_data.afe_again.shg0_d[0] << 0));
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = ex_cis_adjustment_data.afe_aoffset.offdac_d[0];
	lword.BIT.DATA1 = ex_cis_adjustment_data.afe_aoffset.offdac_d[1];
	lword.BIT.DATA2 = ex_cis_adjustment_data.afe_aoffset.offdac_d[2];
	lword.BIT.DATA3 = ex_cis_adjustment_data.afe_aoffset.offdac_d[3];
	*(p_Word + offset++) = lword.LWORD;
	lword.BIT.DATA0 = ex_cis_adjustment_data.afe_dgain.dgain_d[0];
	lword.BIT.DATA1 = ex_cis_adjustment_data.afe_dgain.dgain_d[1];
	lword.BIT.DATA2 = ex_cis_adjustment_data.afe_dgain.dgain_d[2];
	lword.BIT.DATA3 = ex_cis_adjustment_data.afe_dgain.dgain_d[3];
	*(p_Word + offset++) = lword.LWORD;
	/* 以下AB共通 */

	//afe initialize
	sspi_cis_initialize(SSPI_DN_CISB);
}

/*****************************************************************************/
/**
* spi st ready.
*
* @param
* 			- none
*
* @return
*			- none
*
* @note		sspi waiting.
*
******************************************************************************/
void sspi_wait_ready(void)
{
	SSPI_ST_UNION sspi_st = {0};

	do {
		sspi_st.LWORD = FPGA_REG.SSPI_ST.LWORD;
	} while (sspi_st.BIT.BUSY != 0);		// ビジー解除待ち
}
/*****************************************************************************/
/**
* initialize cis afe.
*
* @param
* 			- u8 side : send cis side.
* 						0 - outside
* 						1 - inside
* @return
*			- none
*
* @note		before set afe data.
*
******************************************************************************/
void sspi_cis_initialize(u8 side)
{
	SSPI_CMD_UNION sspi_cmd = {0};

	sspi_wait_ready();
	sspi_cmd.BIT.SZ = AFE_REGISTER_SIZE - 1;		// (転送数 - 1)を設定
	sspi_cmd.BIT.DN = (SSPI_DN_T)side;
	FPGA_REG.SSPI_CMD.LWORD = sspi_cmd.LWORD;
	sspi_wait_ready();
}
/*****************************************************************************/
/**
* initialize cis scan data.
*
* @param	- u16 cismode : sampling mode(AD, BC, ADJ / PAPER, NONE PAPER).
*
* @return	None
*
* @note		None
*
******************************************************************************/
void _pl_cis_init_ad(u16 cismode)
{
	u8 *p_Data;
#if (_DEBUG_CIS_MULTI_IMAGE==1)
	ST_BS *pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
#else
	ST_BS* pbill_data = (ST_BS *)BILL_NOTE_IMAGE_TOP;		// イメージデータの先頭アドレス
#endif

	switch(cismode)
	{
	case CIS_MODE_ADJ_PAPER:
#if (_DEBUG_CIS_MULTI_IMAGE==1)
		for(int i = 0; i < BILL_NOTE_IMAGE_MAX_COUNT;i++)
		{
			if(ex_cis_image_control.image_status[(ex_cis_image_control.current + i)%BILL_NOTE_IMAGE_MAX_COUNT] == CIS_IMAGE_INITIALIZED)
			{
				ex_cis_image_control.current = (ex_cis_image_control.current + i)%BILL_NOTE_IMAGE_MAX_COUNT;
				break;
			}
			if(i == BILL_NOTE_IMAGE_MAX_COUNT - 1)
			{
				p_Data = (u8 *)&pbill_data->proc_num;
				for(int i = 0; i < RESULT_INFO_SIZE/sizeof(u32) ; i++)
				{
					*(u32 *)p_Data = (u32)0;
					p_Data += sizeof(u32);
				}
				p_Data = (u8 *)&pbill_data->sens_dt[0];
				for(int i = 0; i < sizeof(pbill_data->sens_dt)/sizeof(u32) ; i++)
				{
					*(u32 *)p_Data = (u32)0xFFFFFFFF;
					p_Data += sizeof(u32);
				}
				ex_cis_image_control.image_status[ex_cis_image_control.current] = CIS_IMAGE_INITIALIZED;
			}
		}
		pbill_data = &((ST_BS *)BILL_NOTE_IMAGE_TOP)[ex_cis_image_control.current%BILL_NOTE_IMAGE_MAX_COUNT];
		parameter_set(pbill_data);
#else
		p_Data = (u8 *)&pbill_data->proc_num;
		for(int i = 0; i < RESULT_INFO_SIZE/sizeof(u32) ; i++)
		{
			*(u32 *)p_Data = (u32)0;
			p_Data += sizeof(u32);
		}
		p_Data = (u8 *)&pbill_data->sens_dt[0];
		for(int i = 0; i < sizeof(pbill_data->sens_dt)/sizeof(u32) ; i++)
		{
			*(u32 *)p_Data = (u32)0xFFFFFFFF;
			p_Data += sizeof(u32);
		}
#endif
		break;
	case CIS_MODE_ADJ_NON_PAPER:
		p_Data = (u8 *)&ex_tmp_ad_data;
		memset(p_Data, 0xFF, sizeof(TMP_AD_TBL));
		p_Data = (u8 *)&ex_adj_data;
		memset(p_Data, 0xFF, sizeof(ADJ_SENSOR_DATA));
		break;
	case CIS_MODE_AD_PAPER:
	case CIS_MODE_AD_NON_PAPER:
	case CIS_MODE_BC_PAPER:
	case CIS_MODE_BC_NON_PAPER:
		p_Data = (u8 *)&ex_tmp_10bit_ad_data;
		memset(p_Data, 0xFF, sizeof(TMP_10BIT_AD_TBL));
		p_Data = (u8 *)&ex_row_data;
		memset(p_Data, 0xFF, sizeof(ROW_SENSOR_DATA));
		break;
	case CIS_MODE_WC_PAPER:
	case CIS_MODE_WC_NON_PAPER:
		p_Data = (u8 *)&ex_tmp_ad_data;
		memset(p_Data, 0xFF, sizeof(TMP_AD_TBL));
		p_Data = (u8 *)&ex_adj_data;
		memset(p_Data, 0xFF, sizeof(ADJ_SENSOR_DATA));
		break;
#if MAG1_ENABLE
	case MAG_MODE_ADJUSTMENT:
		if(ex_uba710 == 1)
		{	
			p_Data = (u8 *)&s_sens_tempadj_ul_mag;
			memset(p_Data, 0x00, sizeof(MAG_SENS_TEMP_ADJ_INFO));
			p_Data = (u8 *)&s_sens_tempadj_ur_mag;
			memset(p_Data, 0x00, sizeof(MAG_SENS_TEMP_ADJ_INFO));
			p_Data = (u8 *)&ex_tmp_mag_data;
			memset(p_Data, 0xFF, sizeof(MAG_SENSOR_DATA));
		}
		break;
#endif
	case CIS_MODE_VALIDATION_CHECK:
	//#if (_DEBUG_CIS_AS_A_POSITION==1)
		p_Data = (u8 *)&ex_tmp_validation_data;
		memset(p_Data, 0x00, sizeof(TMP_VALIDATION_SENSOR_DATA));
	//#endif
		break;
	default:
		break;
	}
}
/*****************************************************************************/
/**
* cis scan start. (after cis scan flag enable.)
*
* @param	set :	start/stop
* 					0 - scan stop
*					1 - scan start
* @return	None
*
* @note		None
*
******************************************************************************/
void _pl_cis_scanst(u8 set)
{
	u8 timer = 5;
	CAP_CMD_UNION cap_cmd;
	cap_cmd.LWORD = 0;
	cap_cmd.BIT.SCAN_START = set;
	FPGA_REG.CAP_CMD.LWORD = cap_cmd.LWORD;							//a スキャン開始
	while(FPGA_REG.CAP_CMD.BIT.SCAN_START != set)
	{
		FPGA_REG.CAP_CMD.BIT.SCAN_START = set;
		timer--;
		if(!timer)
		{
			FPGA_REG.CAP_CMD.BIT.SCAN_START = 0;
			break;
		}
	}
}
/*****************************************************************************/
/**
* cis scan enable. (cis sampling start.)
*
* @param	set :	enable/disable
* 					0 - disable
*					1 - enable
* @return	None
*
* @note		None
*
******************************************************************************/
void _pl_cis_scan_enable(u8 set)
{
	u8 timer = 5;
	CAP_MD_UNION cap_md;
	cap_md.LWORD = 0;

	cap_md.BIT.CAPEN = set;							/* W/R	採取指示 */
	FPGA_REG.CAP_MD.LWORD = cap_md.LWORD;
	while(FPGA_REG.CAP_MD.BIT.CAPEN != set)
	{
		FPGA_REG.CAP_MD.BIT.CAPEN = set;
		timer--;
		if(!timer)
		{// タイムアップ時はどちらにしても停止
			FPGA_REG.CAP_MD.BIT.CAPEN = 0;
			break;
		}
	}
}
/*****************************************************************************/
/**
* sampling fpga register set.
*
* @param	None
*
* @return	None
*
* @note		None
*
******************************************************************************/
void _pl_sampling_regist_data(void)
{
	CIS_MD_UNION cis_md = {0};
	CAP_PLS_UNION cap_pls = {0};
	CAP_MAXBK_UNION cap_maxbk = {0};
	CAP_IDX_UNION cap_idx = {0};
	CAP_IDXMAX_UNION cap_idxmax = {0};
	CAP_BLID_UNION cap_blid = {0};
	CAP_CINIT_UNION cap_cinit = {0};
	CAP_CCIS_UNION cap_ccis = {0};
	CAP_CMG_UNION cap_cmg = {0};
	CAP_MD_UNION cap_md = {0};
	CAP_CUV_UNION cap_cuv = {0};
	BIN_RNG_UNION bin_rng = {0};
	BIN_INTSET_UNION bin_intset = {0};
	MLT_TH_UNION mlt_th = {0};
	CISDT_SET_UNION cisdt_set = {0};

	cap_pls.BIT.SEL = 0;					// 搬送パルス設定:エンコーダパルス
	cap_pls.BIT.ITVL = ENCODER_CYCLE;					/* W/R	疑似搬送パルス周期-1 (10ns) */
	cap_maxbk.BIT.SCAN_MAX_BLOCK = (BLOCK_MAX);// 調整時採取ブロック数 - 1
	cap_idx.BIT.SCAN_NO = 0;				// 採取番号
	cap_idxmax.BIT.SCAN_BLK_MAX = 0;		// 採取番号最大値
	cap_blid.BIT.BILL_ID = 0;				// 採取ID
	cap_cinit.BIT.SCAN_TIMER = 105;			// 採取タイマーセット値
	cap_ccis.BIT.CIS_START_COUNT = 105;		// CISスタートカウント値
	cis_md.BIT.CMP = 1;						// CISモード:補正モード*********8bit
	cis_md.BIT.CONT = 0;					// CISモード:採取時のみ動作
	cis_md.BIT.DARK = 0;					// CISモード:通常点灯
	cis_md.BIT.DT_EN = 1;					// CISモード:媒体検出・先頭サイクリック記録許可
	cap_md.BIT.CAPEN = 1;					// 採取モード:採取動作許可
#if MAG1_ENABLE
	if(ex_uba710 == 1)
	{
		cap_cmg.BIT.MAG_START_COUNT = MAG_SAMPLING_DELAY;	/* W/R	磁気センサスタート */
	}
	else
	{
		cap_cmg.BIT.MAG_START_COUNT = 0;					/* W/R	磁気センサスタート */
	}
#else
	cap_cmg.BIT.MAG_START_COUNT = 0;					/* W/R	磁気センサスタート */
#endif

#if POINT_UV2_ENABLE
	if(ex_uba710 == 1)
	{
		cap_cuv.BIT.ST = 12;						// UVセンサスタートカウント値
		cap_cuv.BIT.ADCTYPE = 1;
		cap_cuv.BIT.N1CH = 1;
		cap_cuv.BIT.MBK = 1;
	}
	else
	{
		cap_cuv.BIT.ST = 0;						// UVセンサスタートカウント値
		cap_cuv.BIT.N1CH = 0;
		cap_cuv.BIT.MBK = 0;
	}
#elif POINT_UV1_ENABLE
	#if 0//#if defined(PRJ_IVIZION2)
	cap_cuv.BIT.ST = 62;						// UVセンサスタートカウント値
	#else
	cap_cuv.BIT.ST = 12;						// UVセンサスタートカウント値
	#endif
	cap_cuv.BIT.ADCTYPE = 0;
	cap_cuv.BIT.N1CH = 0;
	cap_cuv.BIT.MBK = 0;
#else
	cap_cuv.BIT.ST = 0;						// UVセンサスタートカウント値
	cap_cuv.BIT.N1CH = 0;
	cap_cuv.BIT.MBK = 0;
#endif

#if (BANKNOTE_EDGE_SKEW_ENABLE) || (BANKNOTE_EDGE_LENGTH_ENABLE)
	//Skew Paper Check
	bin_rng.BIT.LL = CISA_AREA_MIN;//搬送路全域チェック
	bin_rng.BIT.UL = CISA_AREA_MAX;
	//bin_intset.BIT.LN = 50;
	bin_intset.BIT.LN = 120;
#endif
#if BANKNOTE_MLT_PAPER_ENABLE
	//MLT Paper Check
	BV_CISMLT_ARRAY.VRCHSET[0].VRPIXMIN.BIT.PIXMIN = EDGE_PAPER_CHECK_PIX_MIN;
	BV_CISMLT_ARRAY.VRCHSET[0].VRPIXMAX.BIT.PIXMAX = EDGE_PAPER_CHECK_PIX_MAX;
	BV_CISMLT_ARRAY.VRCHSET[0].VRTHR.BIT.PIXTHR = EDGE_PAPER_CHECK_MLT_THR;
	mlt_th.BIT.LTH = 5;
#endif
#if BANKNOTE_CYCLIC_ENABLE
	cisdt_set.BIT.TH_PX = 80;
	cisdt_set.BIT.TH_LN = 3;
	cisdt_set.BIT.PREBK = 21;
#endif
	FPGA_REG.CAP_PLS.LWORD = cap_pls.LWORD;
	FPGA_REG.CAP_MAXBK.LWORD = cap_maxbk.LWORD;
	FPGA_REG.CAP_IDX.LWORD = cap_idx.LWORD;
	FPGA_REG.CAP_IDXMAX.LWORD = cap_idxmax.LWORD;
	FPGA_REG.CAP_BLID.LWORD = cap_blid.LWORD;
	FPGA_REG.CAP_CINIT.LWORD = cap_cinit.LWORD;
	FPGA_REG.CAP_CCIS.LWORD = cap_ccis.LWORD;
	FPGA_REG.CAP_CMG.LWORD = cap_cmg.LWORD;
	FPGA_REG.CAP_MD.LWORD = cap_md.LWORD;
	FPGA_REG.CIS_MD.LWORD = cis_md.LWORD;
	FPGA_REG.CAP_CUV.LWORD = cap_cuv.LWORD;
	FPGA_REG.BIN_RNG.LWORD = bin_rng.LWORD;
	FPGA_REG.BIN_INTSET.LWORD = bin_intset.LWORD;
	FPGA_REG.MLT_TH.LWORD = mlt_th.LWORD;
	FPGA_REG.CISDT_SET.LWORD = cisdt_set.LWORD;
}

/*****************************************************************************/
/**
* cis adjustment, one shot data sampling. (64line sampling average)
*
* @param	- u16 cismode : sampling mode(AD, BC, ADJ / PAPER, NONE PAPER).
*
* @note		before sampled cis sensor data.
* 			(use all color data of write adjustment chip area.)
*
******************************************************************************/
void _pl_cis_oneshot_sequence(u16 cismode)
{
	u32 pix = 0;
	u32 line = 0;
	u32 tmpdata[20];

	CIS_MD_UNION cis_md = {0};
	CAP_PLS_UNION cap_pls = {0};
	CAP_MAXBK_UNION cap_maxbk = {0};
	CAP_IDX_UNION cap_idx = {0};
	CAP_IDXMAX_UNION cap_idxmax = {0};
	CAP_BLID_UNION cap_blid = {0};
	CAP_CINIT_UNION cap_cinit = {0};
	CAP_CCIS_UNION cap_ccis = {0};
	CAP_CMG_UNION cap_cmg = {0};
	CAP_MD_UNION cap_md = {0};
	CAP_CUV_UNION cap_cuv = {0};

	switch(ex_cis_adjustment_tbl.sequence)
	{
	case ADJUSTMENT_PL_INIT:
		// Scan flag init
		_pl_cis_scan_enable(0);
		// scanst init
		_pl_cis_scanst(0);

		ex_cis_adjustment_tbl.sequence++;
		ex_cis_adjustment_tbl.weit = 1;
		break;
	case ADJUSTMENT_CIS_INIT:
		/* mdst(sr register) initialize */
		_pl_cis_init_fpga();
	#if POINT_UV1_ENABLE || POINT_UV2_ENABLE
		// UV DA,GAINセット
		if(ex_uba710 == 1)
		{
			set_uv_adj();
		}
	#endif	

	#if MAG1_ENABLE
		if(ex_uba710 == 1)
		{
			set_mag_gain();
		}
	#endif
		switch(cismode)
		{
		case CIS_MODE_ADJ_NON_PAPER:
		case CIS_MODE_WC_NON_PAPER:
			cis_md.BIT.CMP = 1;				// CISモード:補正データ
			_pl_oneshot_non_paper_cis_seq(CAP_ONE_SHOT_8BIT);
			_pl_oneshot_ss_seq(CAP_ONE_SHOT_8BIT);	//その他センサシーケンス設定
			break;
		case CIS_MODE_BC_NON_PAPER:
		case CIS_MODE_AD_NON_PAPER:
			cis_md.BIT.CMP = 0;				// CISモード:生データ
			_pl_oneshot_non_paper_cis_seq(CAP_ONE_SHOT_10BIT);
			_pl_oneshot_ss_seq(CAP_ONE_SHOT_10BIT);	//その他センサシーケンス設定
			break;
		case CIS_MODE_ADJ_PAPER:
		case CIS_MODE_WC_PAPER:
			cis_md.BIT.CMP = 1;				// CISモード:補正データ
			_pl_oneshot_cis_seq(CAP_ONE_SHOT_8BIT);
			_pl_oneshot_ss_seq(CAP_ONE_SHOT_8BIT);	//その他センサシーケンス設定
			break;
		case CIS_MODE_BC_PAPER:
		case CIS_MODE_AD_PAPER:
			cis_md.BIT.CMP = 0;				// CISモード:生データ
			_pl_oneshot_cis_seq(CAP_ONE_SHOT_10BIT);
			_pl_oneshot_ss_seq(CAP_ONE_SHOT_10BIT);	//その他センサシーケンス設定
			break;
		default:
			/* error:reset */
			program_error();
		}

		//cis_ctlb.BIT.BUF_ON = 1;
		cap_pls.BIT.SEL = 1;					// 搬送パルス設定:疑似搬送パルス
		cap_pls.BIT.ITVL = ENCODER_CYCLE;					/* W/R	疑似搬送パルス周期-1 (10ns) */
		cap_maxbk.BIT.SCAN_MAX_BLOCK = (SUB_ADJ_LINE - 1);// 調整時採取ブロック数 - 1
		cap_idx.BIT.SCAN_NO = 0;				// 採取番号
		cap_idxmax.BIT.SCAN_BLK_MAX = 0;		// 採取番号最大値
		cap_blid.BIT.BILL_ID = 0;				// 採取ID
		cap_cinit.BIT.SCAN_TIMER = 1;			// 採取タイマーセット値
		cap_ccis.BIT.CIS_START_COUNT = 1;		// CISスタートカウント値
		cap_cmg.BIT.MAG_START_COUNT = 0;		// 磁気センサースタートカウント値(動作させない設定値)
		cis_md.BIT.CONT = 0;					// CISモード:採取時のみ動作
		cis_md.BIT.DARK = 0;					// CISモード:通常点灯
		cap_md.BIT.CAPEN = 1;					// 採取モード:採取動作許可
#if POINT_UV2_ENABLE
		if(ex_uba710 == 1)
		{
			cap_cuv.BIT.ST = 1;
			cap_cuv.BIT.ADCTYPE = 1;
			cap_cuv.BIT.N1CH = 1;
			cap_cuv.BIT.MBK = 1;
		}
		else
		{
			cap_cuv.BIT.ST = 0;
			cap_cuv.BIT.ADCTYPE = 0;
			cap_cuv.BIT.N1CH = 0;
			cap_cuv.BIT.MBK = 0;
		}
#elif POINT_UV1_ENABLE
		cap_cuv.BIT.ST = 1;
		cap_cuv.BIT.ADCTYPE = 0;
		cap_cuv.BIT.N1CH = 0;
		cap_cuv.BIT.MBK = 0;
#else
		cap_cuv.BIT.ST = 0;
		cap_cuv.BIT.ADCTYPE = 0;
		cap_cuv.BIT.N1CH = 0;
		cap_cuv.BIT.MBK = 0;
#endif
		FPGA_REG.CAP_CUV.LWORD = cap_cuv.LWORD;

		//FPGA_REG.CIS_CTLB.LWORD = cis_ctlb.LWORD;
		FPGA_REG.CAP_PLS.LWORD = cap_pls.LWORD;
		FPGA_REG.CAP_MAXBK.LWORD = cap_maxbk.LWORD;
		FPGA_REG.CAP_IDX.LWORD = cap_idx.LWORD;
		FPGA_REG.CAP_IDXMAX.LWORD = cap_idxmax.LWORD;
		FPGA_REG.CAP_BLID.LWORD = cap_blid.LWORD;
		FPGA_REG.CAP_CINIT.LWORD = cap_cinit.LWORD;
		FPGA_REG.CAP_CCIS.LWORD = cap_ccis.LWORD;
		FPGA_REG.CAP_CMG.LWORD = cap_cmg.LWORD;
		FPGA_REG.CAP_MD.LWORD = cap_md.LWORD;
		FPGA_REG.CIS_MD.LWORD = cis_md.LWORD;


		ex_cis_adjustment_tbl.data_count = 0;
		ex_cis_adjustment_tbl.sequence++;
		ex_cis_adjustment_tbl.weit = 50;//CIS電源ON後50ms以上待機
		break;
	case ADJUSTMENT_REGSTER_RESET:
		/* afe initialize */
		_pl_cis_init_afe();

		_pl_cis_init_ad(cismode);
		_pl_cis_scan_enable(1);
		ex_cis_adjustment_tbl.scan_count = 0;
		ex_cis_adjustment_tbl.sequence++;
		ex_cis_adjustment_tbl.weit = 5;
		break;
	case ADJUSTMENT_SAMPLING:
		_pl_cis_scanst(1);
		ex_cis_adjustment_tbl.sequence++;
		ex_cis_adjustment_tbl.weit = 50;//480us*64line=30.72ms
		break;
	case ADJUSTMENT_DATA_CALCRATION:
		memset(&tmpdata, 0, sizeof(tmpdata));
		switch(cismode)
		{
		case CIS_MODE_ADJ_PAPER:
		case CIS_MODE_ADJ_NON_PAPER:
		case CIS_MODE_WC_PAPER:
		case CIS_MODE_WC_NON_PAPER:
			memcpy(&ex_tmp_ad_data.cis_red_ref_u, &ex_adj_data.tmp_ad_tbl[0].cis_red_ref_u, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_gre_ref_u, &ex_adj_data.tmp_ad_tbl[0].cis_gre_ref_u, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_blu_ref_u, &ex_adj_data.tmp_ad_tbl[0].cis_blu_ref_u, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_ir1_ref_u, &ex_adj_data.tmp_ad_tbl[0].cis_ir1_ref_u, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_ir2_ref_u, &ex_adj_data.tmp_ad_tbl[0].cis_ir2_ref_u, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_fl_ref_u, &ex_adj_data.tmp_ad_tbl[0].cis_fl_ref_u, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.c6_led_u, &ex_adj_data.tmp_ad_tbl[0].c6_led_u, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.c7_led_u, &ex_adj_data.tmp_ad_tbl[0].c7_led_u, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.c8_led_u, &ex_adj_data.tmp_ad_tbl[0].c8_led_u, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.c9_led_u, &ex_adj_data.tmp_ad_tbl[0].c9_led_u, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_red_ref_d, &ex_adj_data.tmp_ad_tbl[0].cis_red_ref_d, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_gre_ref_d, &ex_adj_data.tmp_ad_tbl[0].cis_gre_ref_d, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_blu_ref_d, &ex_adj_data.tmp_ad_tbl[0].cis_blu_ref_d, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_ir1_ref_d, &ex_adj_data.tmp_ad_tbl[0].cis_ir1_ref_d, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_ir2_ref_d, &ex_adj_data.tmp_ad_tbl[0].cis_ir2_ref_d, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_fl_ref_d, &ex_adj_data.tmp_ad_tbl[0].cis_fl_ref_d, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_red_pen_d, &ex_adj_data.tmp_ad_tbl[0].cis_red_pen_d, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_gre_pen_d, &ex_adj_data.tmp_ad_tbl[0].cis_gre_pen_d, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_ir1_pen_d, &ex_adj_data.tmp_ad_tbl[0].cis_ir1_pen_d, HDRTBL_SIZE);
			memcpy(&ex_tmp_ad_data.cis_ir2_pen_d, &ex_adj_data.tmp_ad_tbl[0].cis_ir2_pen_d, HDRTBL_SIZE);

			for(pix = 0; pix < MAIN_SCAN_LINE; pix++)
			{
				memset(&tmpdata, 0, sizeof(tmpdata));
				for(line = 0; line < SUB_ADJ_LINE; line++)
				{
					tmpdata[0] += ex_adj_data.tmp_ad_tbl[line].cis_red_ref_u[HDRTBL_SIZE + pix];
					tmpdata[1] += ex_adj_data.tmp_ad_tbl[line].cis_gre_ref_u[HDRTBL_SIZE + pix];
					tmpdata[2] += ex_adj_data.tmp_ad_tbl[line].cis_blu_ref_u[HDRTBL_SIZE + pix];
					tmpdata[3] += ex_adj_data.tmp_ad_tbl[line].cis_ir1_ref_u[HDRTBL_SIZE + pix];
					tmpdata[4] += ex_adj_data.tmp_ad_tbl[line].cis_ir2_ref_u[HDRTBL_SIZE + pix];
					tmpdata[6] += ex_adj_data.tmp_ad_tbl[line].c6_led_u[HDRTBL_SIZE + pix];
					tmpdata[7] += ex_adj_data.tmp_ad_tbl[line].c7_led_u[HDRTBL_SIZE + pix];
					tmpdata[8] += ex_adj_data.tmp_ad_tbl[line].c8_led_u[HDRTBL_SIZE + pix];//reserve1
					tmpdata[9] += ex_adj_data.tmp_ad_tbl[line].c9_led_u[HDRTBL_SIZE + pix];//reserve4
					tmpdata[10] += ex_adj_data.tmp_ad_tbl[line].cis_red_ref_d[HDRTBL_SIZE + pix];
					tmpdata[11] += ex_adj_data.tmp_ad_tbl[line].cis_gre_ref_d[HDRTBL_SIZE + pix];
					tmpdata[12] += ex_adj_data.tmp_ad_tbl[line].cis_blu_ref_d[HDRTBL_SIZE + pix];
					tmpdata[13] += ex_adj_data.tmp_ad_tbl[line].cis_ir1_ref_d[HDRTBL_SIZE + pix];
					tmpdata[14] += ex_adj_data.tmp_ad_tbl[line].cis_ir2_ref_d[HDRTBL_SIZE + pix];
					tmpdata[16] += ex_adj_data.tmp_ad_tbl[line].cis_red_pen_d[HDRTBL_SIZE + pix];
					tmpdata[17] += ex_adj_data.tmp_ad_tbl[line].cis_gre_pen_d[HDRTBL_SIZE + pix];
					tmpdata[18] += ex_adj_data.tmp_ad_tbl[line].cis_ir1_pen_d[HDRTBL_SIZE + pix];
					tmpdata[19] += ex_adj_data.tmp_ad_tbl[line].cis_ir2_pen_d[HDRTBL_SIZE + pix];
				}

				ex_tmp_ad_data.cis_red_ref_u[HDRTBL_SIZE + pix] = (u8)(tmpdata[0] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_gre_ref_u[HDRTBL_SIZE + pix] = (u8)(tmpdata[1] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_blu_ref_u[HDRTBL_SIZE + pix] = (u8)(tmpdata[2] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_ir1_ref_u[HDRTBL_SIZE + pix] = (u8)(tmpdata[3] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_ir2_ref_u[HDRTBL_SIZE + pix] = (u8)(tmpdata[4] / SUB_ADJ_LINE);
				ex_tmp_ad_data.c6_led_u[HDRTBL_SIZE + pix] = (u8)(tmpdata[6] / SUB_ADJ_LINE);
				ex_tmp_ad_data.c7_led_u[HDRTBL_SIZE + pix] = (u8)(tmpdata[7] / SUB_ADJ_LINE);
				ex_tmp_ad_data.c8_led_u[HDRTBL_SIZE + pix] = (u8)(tmpdata[8] / SUB_ADJ_LINE);
				ex_tmp_ad_data.c9_led_u[HDRTBL_SIZE + pix] = (u8)(tmpdata[9] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_red_ref_d[HDRTBL_SIZE + pix] = (u8)(tmpdata[10] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_gre_ref_d[HDRTBL_SIZE + pix] = (u8)(tmpdata[11] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_blu_ref_d[HDRTBL_SIZE + pix] = (u8)(tmpdata[12] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_ir1_ref_d[HDRTBL_SIZE + pix] = (u8)(tmpdata[13] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_ir2_ref_d[HDRTBL_SIZE + pix] = (u8)(tmpdata[14] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_red_pen_d[HDRTBL_SIZE + pix] = (u8)(tmpdata[16] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_gre_pen_d[HDRTBL_SIZE + pix] = (u8)(tmpdata[17] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_ir1_pen_d[HDRTBL_SIZE + pix] = (u8)(tmpdata[18] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_ir2_pen_d[HDRTBL_SIZE + pix] = (u8)(tmpdata[19] / SUB_ADJ_LINE);

				//UV
#if POINT_UV1_ENABLE && POINT_UV2_ENABLE
				if(ex_uba710 == 1)
				{
					for(line = 0; line < (SUB_ADJ_LINE/2); line++)
					{
						tmpdata[5] += ex_adj_data.tmp_ad_tbl[line].cis_fl_ref_u[HDRTBL_SIZE + pix];
						tmpdata[15] += ex_adj_data.tmp_ad_tbl[line].cis_fl_ref_d[HDRTBL_SIZE + pix];
					}
					ex_tmp_ad_data.cis_fl_ref_u[HDRTBL_SIZE + pix] = (u8)(tmpdata[5] / (SUB_ADJ_LINE/2));
					ex_tmp_ad_data.cis_fl_ref_d[HDRTBL_SIZE + pix] = (u8)(tmpdata[15] / (SUB_ADJ_LINE/2));
				}
				else
				{
					for(line = 0; line < SUB_ADJ_LINE; line++)
					{
						tmpdata[5] += ex_adj_data.tmp_ad_tbl[line].cis_fl_ref_u[HDRTBL_SIZE + pix];
						tmpdata[15] += ex_adj_data.tmp_ad_tbl[line].cis_fl_ref_d[HDRTBL_SIZE + pix];
					}
					ex_tmp_ad_data.cis_fl_ref_u[HDRTBL_SIZE + pix] = (u8)(tmpdata[5] / SUB_ADJ_LINE);
					ex_tmp_ad_data.cis_fl_ref_d[HDRTBL_SIZE + pix] = (u8)(tmpdata[15] / SUB_ADJ_LINE);
				}
#else
				for(line = 0; line < SUB_ADJ_LINE; line++)
				{
					tmpdata[5] += ex_adj_data.tmp_ad_tbl[line].cis_fl_ref_u[HDRTBL_SIZE + pix];
					tmpdata[15] += ex_adj_data.tmp_ad_tbl[line].cis_fl_ref_d[HDRTBL_SIZE + pix];
				}
				ex_tmp_ad_data.cis_fl_ref_u[HDRTBL_SIZE + pix] = (u8)(tmpdata[5] / SUB_ADJ_LINE);
				ex_tmp_ad_data.cis_fl_ref_d[HDRTBL_SIZE + pix] = (u8)(tmpdata[15] / SUB_ADJ_LINE);
#endif
			}
			break;
		case CIS_MODE_BC_PAPER:
		case CIS_MODE_BC_NON_PAPER:
		case CIS_MODE_AD_PAPER:
		case CIS_MODE_AD_NON_PAPER:
			memcpy(&ex_tmp_10bit_ad_data.cis_red_ref_u, &ex_row_data.tmp_10bit_ad_tbl[0].cis_red_ref_u, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_gre_ref_u, &ex_row_data.tmp_10bit_ad_tbl[0].cis_gre_ref_u, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_blu_ref_u, &ex_row_data.tmp_10bit_ad_tbl[0].cis_blu_ref_u, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_ir1_ref_u, &ex_row_data.tmp_10bit_ad_tbl[0].cis_ir1_ref_u, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_ir2_ref_u, &ex_row_data.tmp_10bit_ad_tbl[0].cis_ir2_ref_u, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_fl_ref_u, &ex_row_data.tmp_10bit_ad_tbl[0].cis_fl_ref_u, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.c6_led_u, &ex_row_data.tmp_10bit_ad_tbl[0].c6_led_u, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.c7_led_u, &ex_row_data.tmp_10bit_ad_tbl[0].c7_led_u, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.c8_led_u, &ex_row_data.tmp_10bit_ad_tbl[0].c8_led_u, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.c9_led_u, &ex_row_data.tmp_10bit_ad_tbl[0].c9_led_u, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_red_ref_d, &ex_row_data.tmp_10bit_ad_tbl[0].cis_red_ref_d, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_gre_ref_d, &ex_row_data.tmp_10bit_ad_tbl[0].cis_gre_ref_d, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_blu_ref_d, &ex_row_data.tmp_10bit_ad_tbl[0].cis_blu_ref_d, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_ir1_ref_d, &ex_row_data.tmp_10bit_ad_tbl[0].cis_ir1_ref_d, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_ir2_ref_d, &ex_row_data.tmp_10bit_ad_tbl[0].cis_ir2_ref_d, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_fl_ref_d, &ex_row_data.tmp_10bit_ad_tbl[0].cis_fl_ref_d, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_red_pen_d, &ex_row_data.tmp_10bit_ad_tbl[0].cis_red_pen_d, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_gre_pen_d, &ex_row_data.tmp_10bit_ad_tbl[0].cis_gre_pen_d, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_ir1_pen_d, &ex_row_data.tmp_10bit_ad_tbl[0].cis_ir1_pen_d, HDRTBL_SIZE_10BIT);
			memcpy(&ex_tmp_10bit_ad_data.cis_ir2_pen_d, &ex_row_data.tmp_10bit_ad_tbl[0].cis_ir2_pen_d, HDRTBL_SIZE_10BIT);

			for(pix = 0; pix < MAIN_SCAN_LINE; pix++)
			{
				memset(&tmpdata, 0, sizeof(tmpdata));
				for(line = 0; line < SUB_ADJ_LINE; line++)
				{
					tmpdata[0] += ex_row_data.tmp_10bit_ad_tbl[line].cis_red_ref_u[HDRTBL_SIZE_10BIT + pix];
					tmpdata[1] += ex_row_data.tmp_10bit_ad_tbl[line].cis_gre_ref_u[HDRTBL_SIZE_10BIT + pix];
					tmpdata[2] += ex_row_data.tmp_10bit_ad_tbl[line].cis_blu_ref_u[HDRTBL_SIZE_10BIT + pix];
					tmpdata[3] += ex_row_data.tmp_10bit_ad_tbl[line].cis_ir1_ref_u[HDRTBL_SIZE_10BIT + pix];
					tmpdata[4] += ex_row_data.tmp_10bit_ad_tbl[line].cis_ir2_ref_u[HDRTBL_SIZE_10BIT + pix];
					tmpdata[6] += ex_row_data.tmp_10bit_ad_tbl[line].c6_led_u[HDRTBL_SIZE_10BIT + pix];
					tmpdata[7] += ex_row_data.tmp_10bit_ad_tbl[line].c7_led_u[HDRTBL_SIZE_10BIT + pix];
					tmpdata[8] += ex_row_data.tmp_10bit_ad_tbl[line].c8_led_u[HDRTBL_SIZE_10BIT + pix];
					tmpdata[9] += ex_row_data.tmp_10bit_ad_tbl[line].c9_led_u[HDRTBL_SIZE_10BIT + pix];
					tmpdata[10] += ex_row_data.tmp_10bit_ad_tbl[line].cis_red_ref_d[HDRTBL_SIZE_10BIT + pix];
					tmpdata[11] += ex_row_data.tmp_10bit_ad_tbl[line].cis_gre_ref_d[HDRTBL_SIZE_10BIT + pix];
					tmpdata[12] += ex_row_data.tmp_10bit_ad_tbl[line].cis_blu_ref_d[HDRTBL_SIZE_10BIT + pix];
					tmpdata[13] += ex_row_data.tmp_10bit_ad_tbl[line].cis_ir1_ref_d[HDRTBL_SIZE_10BIT + pix];
					tmpdata[14] += ex_row_data.tmp_10bit_ad_tbl[line].cis_ir2_ref_d[HDRTBL_SIZE_10BIT + pix];
					tmpdata[16] += ex_row_data.tmp_10bit_ad_tbl[line].cis_red_pen_d[HDRTBL_SIZE_10BIT + pix];
					tmpdata[17] += ex_row_data.tmp_10bit_ad_tbl[line].cis_gre_pen_d[HDRTBL_SIZE_10BIT + pix];
					tmpdata[18] += ex_row_data.tmp_10bit_ad_tbl[line].cis_ir1_pen_d[HDRTBL_SIZE_10BIT + pix];
					tmpdata[19] += ex_row_data.tmp_10bit_ad_tbl[line].cis_ir2_pen_d[HDRTBL_SIZE_10BIT + pix];
				}

				ex_tmp_10bit_ad_data.cis_red_ref_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[0] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_gre_ref_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[1] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_blu_ref_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[2] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_ir1_ref_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[3] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_ir2_ref_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[4] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_fl_ref_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[5] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.c6_led_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[6] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.c7_led_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[7] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.c8_led_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[8] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.c9_led_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[9] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_red_ref_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[10] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_gre_ref_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[11] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_blu_ref_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[12] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_ir1_ref_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[13] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_ir2_ref_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[14] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_fl_ref_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[15] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_red_pen_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[16] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_gre_pen_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[17] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_ir1_pen_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[18] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_ir2_pen_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[19] / SUB_ADJ_LINE);

				//UV
#if POINT_UV1_ENABLE && POINT_UV2_ENABLE
				if(ex_uba710 == 1)
				{
					for(line = 0; line < (SUB_ADJ_LINE/2); line++)
					{
						tmpdata[5] += ex_row_data.tmp_10bit_ad_tbl[line].cis_fl_ref_u[HDRTBL_SIZE_10BIT + pix];
						tmpdata[15] += ex_row_data.tmp_10bit_ad_tbl[line].cis_fl_ref_d[HDRTBL_SIZE_10BIT + pix];
					}
					ex_tmp_10bit_ad_data.cis_fl_ref_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[5] / (SUB_ADJ_LINE/2));
					ex_tmp_10bit_ad_data.cis_fl_ref_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[15] / (SUB_ADJ_LINE/2));
				}
				else
				{
					for(line = 0; line < SUB_ADJ_LINE; line++)
					{
						tmpdata[5] += ex_row_data.tmp_10bit_ad_tbl[line].cis_fl_ref_u[HDRTBL_SIZE_10BIT + pix];
						tmpdata[15] += ex_row_data.tmp_10bit_ad_tbl[line].cis_fl_ref_d[HDRTBL_SIZE_10BIT + pix];
					}
					ex_tmp_10bit_ad_data.cis_fl_ref_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[5] / SUB_ADJ_LINE);
					ex_tmp_10bit_ad_data.cis_fl_ref_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[15] / SUB_ADJ_LINE);
				}
#else
				for(line = 0; line < SUB_ADJ_LINE; line++)
				{
					tmpdata[5] += ex_row_data.tmp_10bit_ad_tbl[line].cis_fl_ref_u[HDRTBL_SIZE_10BIT + pix];
					tmpdata[15] += ex_row_data.tmp_10bit_ad_tbl[line].cis_fl_ref_d[HDRTBL_SIZE_10BIT + pix];
				}
				ex_tmp_10bit_ad_data.cis_fl_ref_u[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[5] / SUB_ADJ_LINE);
				ex_tmp_10bit_ad_data.cis_fl_ref_d[HDRTBL_SIZE_10BIT + pix] = (u16)(tmpdata[15] / SUB_ADJ_LINE);
#endif
				ex_calc_bc.black_data1_u[pix] = ex_tmp_10bit_ad_data.c6_led_u[HDRTBL_SIZE_10BIT + pix];
				ex_calc_bc.black_data1_d[pix] = ex_tmp_10bit_ad_data.c7_led_u[HDRTBL_SIZE_10BIT + pix];
				ex_calc_bc.black_data2_u[pix] = ex_tmp_10bit_ad_data.c8_led_u[HDRTBL_SIZE_10BIT + pix];
				ex_calc_bc.black_data2_d[pix] = ex_tmp_10bit_ad_data.c9_led_u[HDRTBL_SIZE_10BIT + pix];
			}
			break;
		default:
			/* error:reset */
			program_error();
		}
		_pl_cis_scanst(0);
		_pl_cis_scan_enable(0);
		ex_cis_adjustment_tbl.sequence = ADJUSTMENT_SEQUENCE_END;
		break;
	case ADJUSTMENT_SEQUENCE_END:
		break;
	default:
		/* error:reset */
		program_error();
	}
}

/*****************************************************************************/
/**
* mag max value check, three top data average calculation.
*
* @param	None.
*
* @note		before top mag data buff transfer.
*
******************************************************************************/
void check_mag_max_data(void)
{
	u32 total = 0;
	u16 average = 0;

	memset(&ex_mag_adj.ul_adj_max, 0, sizeof(u16)*2);

	//UL MAG
	for(int line = 1; line < MAG_SCAN_LINE - 1; line++)
	{
		if(ex_tmp_mag_data.tmp_mag_ad_tbl[line].mag[3] != 0xFFFF)//Block No
		{
			total = ex_tmp_mag_data.tmp_mag_ad_tbl[line-1].mag[UL_MAG_BYTE] + ex_tmp_mag_data.tmp_mag_ad_tbl[line].mag[UL_MAG_BYTE] + ex_tmp_mag_data.tmp_mag_ad_tbl[line+1].mag[UL_MAG_BYTE];
			average = total / 3;

			if(ex_mag_adj.ul_adj_max < average)
			{
				ex_mag_adj.ul_adj_max = average;
			}
		}
	}
	//UR MAG
	for(int line = 1; line < MAG_SCAN_LINE - 1; line++)
	{
		if(ex_tmp_mag_data.tmp_mag_ad_tbl[line].mag[3] != 0xFFFF)//Block No
		{
			total = ex_tmp_mag_data.tmp_mag_ad_tbl[line-1].mag[UR_MAG_BYTE] + ex_tmp_mag_data.tmp_mag_ad_tbl[line].mag[UR_MAG_BYTE] + ex_tmp_mag_data.tmp_mag_ad_tbl[line+1].mag[UR_MAG_BYTE];
			average = total / 3;

			if(ex_mag_adj.ur_adj_max < average)
			{
				ex_mag_adj.ur_adj_max = average;
			}
		}
	}
}
/*****************************************************************************/
/**
* mag adjustment, one shot data sampling. (90line sampling data)
*
* @param	None.
*
* @note		before sampled mag sensor adjustment.
*
******************************************************************************/
void _pl_mag_adjustment_sequence(void)
{
	CIS_MD_UNION cis_md = {0};
	CAP_PLS_UNION cap_pls = {0};
	CAP_MAXBK_UNION cap_maxbk = {0};
	CAP_IDX_UNION cap_idx = {0};
	CAP_IDXMAX_UNION cap_idxmax = {0};
	CAP_BLID_UNION cap_blid = {0};
	CAP_CINIT_UNION cap_cinit = {0};
	CAP_CCIS_UNION cap_ccis = {0};
	CAP_CMG_UNION cap_cmg = {0};
	CAP_MD_UNION cap_md = {0};

	switch(ex_cis_adjustment_tbl.sequence)
	{
	case ADJUSTMENT_PL_INIT:
		// Scan flag init
		_pl_cis_scan_enable(0);
		// scanst init
		_pl_cis_scanst(0);

		ex_cis_adjustment_tbl.sequence++;
		ex_cis_adjustment_tbl.weit = 1;
		break;
	case ADJUSTMENT_CIS_INIT:
		/* mdst(sr register) initialize */
		_pl_cis_init_fpga();
#if MAG1_ENABLE
		if(ex_uba710 == 1)
		{
			/* mag ctrl high */
			_hal_i2c3_write_mag_cntl(1);
		}
#endif
		cis_md.BIT.CMP = 0;									// CISモード:生データ
		_pl_oneshot_cis_seq(CAP_ONE_SHOT_MAG);				// CISシーケンス設定
		_pl_oneshot_ss_seq(CAP_ONE_SHOT_MAG);				// その他センサシーケンス設定

		cap_pls.BIT.SEL = 1;								// 搬送パルス設定:疑似搬送パルス
		cap_pls.BIT.ITVL = MAG_CYCLE;						// 疑似搬送パルス周期-1 (10ns)
		cap_maxbk.BIT.SCAN_MAX_BLOCK = (MAG_SCAN_LINE - 1);	// 調整時採取ブロック数 - 1
		cap_idx.BIT.SCAN_NO = 0;							// 採取番号
		cap_idxmax.BIT.SCAN_BLK_MAX = 0;					// 採取番号最大値
		cap_blid.BIT.BILL_ID = 0;							// 採取ID
		cap_cinit.BIT.SCAN_TIMER = 1;						// 採取タイマーセット値
		cap_ccis.BIT.CIS_START_COUNT = 1;					// CISスタートカウント値
		cap_cmg.BIT.MAG_START_COUNT = 1;					// 磁気センサースタートカウント値(動作設定)
		cis_md.BIT.CONT = 0;								// CISモード:採取時のみ動作
		cis_md.BIT.DARK = 0;								// CISモード:通常点灯
		cap_md.BIT.CAPEN = 1;								// 採取モード:採取動作許可

		FPGA_REG.CAP_PLS.LWORD = cap_pls.LWORD;
		FPGA_REG.CAP_MAXBK.LWORD = cap_maxbk.LWORD;
		FPGA_REG.CAP_IDX.LWORD = cap_idx.LWORD;
		FPGA_REG.CAP_IDXMAX.LWORD = cap_idxmax.LWORD;
		FPGA_REG.CAP_BLID.LWORD = cap_blid.LWORD;
		FPGA_REG.CAP_CINIT.LWORD = cap_cinit.LWORD;
		FPGA_REG.CAP_CCIS.LWORD = cap_ccis.LWORD;
		FPGA_REG.CAP_CMG.LWORD = cap_cmg.LWORD;
		FPGA_REG.CIS_MD.LWORD = cis_md.LWORD;
		FPGA_REG.CAP_MD.LWORD = cap_md.LWORD;

		ex_cis_adjustment_tbl.data_count = 0;
		ex_cis_adjustment_tbl.sequence++;
		ex_cis_adjustment_tbl.weit = 50;//CIS電源ON後50ms以上待機
		break;
	case ADJUSTMENT_REGSTER_RESET://補正はここから
		/* afe initialize */
		_pl_cis_init_afe();

		_pl_cis_init_ad(MAG_MODE_ADJUSTMENT);
#if MAG1_ENABLE
		if(ex_uba710 == 1)
		{
			/* set mag gain */
			set_mag_gain();
		}
#endif
		ex_cis_adjustment_tbl.sequence++;
		ex_cis_adjustment_tbl.weit = 5;
		break;
	case ADJUSTMENT_SAMPLING:
		_pl_cis_scan_enable(1);
		_pl_cis_scanst(1);
#if MAG1_ENABLE
		if(ex_uba710 == 1)
		{
			/* high mag cal port */
			_hal_i2c3_write_mag_cal(1);
		}
#endif
		ex_cis_adjustment_tbl.sequence++;
		ex_cis_adjustment_tbl.weit = 15;//min 10ms wait
		break;
	case ADJUSTMENT_DATA_CALCRATION:
#if MAG1_ENABLE
		if(ex_uba710 == 1)
		{
			_hal_i2c3_write_mag_cal(0);
		}
#endif
		/* check ex_tmp_mag_data */
		check_mag_max_data();

		_pl_cis_scanst(0);
		_pl_cis_scan_enable(0);
#if MAG1_ENABLE		//この関数内がすでに、700,710の判別済みなので、切り分け必要ない
		if(set_mag_adj() >= 2)//OK
		{
			/* mag ctrl low */
			//_hal_i2c3_write_mag_cntl(0);
			ex_cis_adjustment_tbl.sequence = ADJUSTMENT_SEQUENCE_END;
		}
		else//retry
		{
			ex_cis_adjustment_tbl.sequence = ADJUSTMENT_SAMPLING;
		}
		ex_cis_adjustment_tbl.weit = 5;
#endif
		break;
	case ADJUSTMENT_SEQUENCE_END://補正はここまでをぐるぐる回す　前のシーケンスでENDに入ったら終わり
		break;
	default:
		/* error:reset */
		program_error();
	}
}

/*****************************************************************************/
/**
* calcration pga adjustment value.
*
* @param	None
*
* @return
*		- TRUE if successful
*		- FALSE if unsuccessful
*
* @note		before sampled cis sensor data.
* 			(use all color data of write adjustment chip area.)
*
******************************************************************************/
u8 _cis_temp_adjust_pga(void)
{
	u8 led = 0;
	u8 side = 0;
	u16 pix = 0;
	u16 *p_data;
	u16 checkad = 0;
	u16 adjad = 0;
	float *pga = 0;
	u16 datacnt = 0;
	u32 sumad = 0;
	u16 tmpad = 0;
	float ratio = 0.0f;
	u16 *bc;

	// rls
	for(side = OUTSIDE; side <= INSIDE; side++)
	{
		for(led = RLS_RED; led <= RLS_IR2; led++)
		{
#if !CIS_UV_ENABLE
			if(led == RLS_FL)
			{
				continue;
			}
#endif
			datacnt = 0;
			sumad = 0;
			if(side == OUTSIDE)
			{
				switch(led)
				{
				case RLS_RED:
					p_data = &ex_tmp_10bit_ad_data.cis_red_ref_u[HDRTBL_SIZE_10BIT];
					adjad = ex_cis_adjustment_tmp.cis_tmp_ad.red_ref_ad_u;
					pga = &ex_calc_pga.red_ref_pga_u;
					bc = ex_calc_bc.black_data1_u;
					break;
				case RLS_GRE:
					p_data = &ex_tmp_10bit_ad_data.cis_gre_ref_u[HDRTBL_SIZE_10BIT];
					adjad = ex_cis_adjustment_tmp.cis_tmp_ad.gre_ref_ad_u;
					pga = &ex_calc_pga.gre_ref_pga_u;
					bc = ex_calc_bc.black_data1_u;
					break;
				case RLS_BLU:
					p_data = &ex_tmp_10bit_ad_data.cis_blu_ref_u[HDRTBL_SIZE_10BIT];
					adjad = ex_cis_adjustment_tmp.cis_tmp_ad.blu_ref_ad_u;
					pga = &ex_calc_pga.blu_ref_pga_u;
					bc = ex_calc_bc.black_data1_u;
					break;
				case RLS_IR1:
					p_data = &ex_tmp_10bit_ad_data.cis_ir1_ref_u[HDRTBL_SIZE_10BIT];
					adjad = ex_cis_adjustment_tmp.cis_tmp_ad.ir1_ref_ad_u;
					pga = &ex_calc_pga.ir1_ref_pga_u;
					bc = ex_calc_bc.black_data1_u;
					break;
				case RLS_IR2:
					p_data = &ex_tmp_10bit_ad_data.cis_ir2_ref_u[HDRTBL_SIZE_10BIT];
					adjad = ex_cis_adjustment_tmp.cis_tmp_ad.ir2_ref_ad_u;
					pga = &ex_calc_pga.ir2_ref_pga_u;
					bc = ex_calc_bc.black_data2_u;
					break;
				case RLS_FL:
					p_data = &ex_tmp_10bit_ad_data.cis_fl_ref_u[HDRTBL_SIZE_10BIT];
					adjad = ex_cis_adjustment_tmp.cis_tmp_ad.fl_ref_ad_u;
					pga = &ex_calc_pga.fl_ref_pga_u;
					bc = ex_calc_bc.black_data2_u;
					break;
				default:
					break;
				}
				for(pix = ex_cis_adjustment_tmp.cis_tmp_eria.left_pix_u[0]; pix < ex_cis_adjustment_tmp.cis_tmp_eria.left_pix_u[1]; pix++)
				{
					checkad = *(p_data + pix) - *(bc + pix);
					if(checkad >= WHITE_CHIP_MIN_TMP_AD)
					{
						sumad += checkad;
						datacnt++;
					}
				}
			}
			else if(side == INSIDE)
			{
				switch(led)
				{
				case RLS_RED:
					p_data = &ex_tmp_10bit_ad_data.cis_red_ref_d[HDRTBL_SIZE_10BIT];
					adjad = ex_cis_adjustment_tmp.cis_tmp_ad.red_ref_ad_d;
					pga = &ex_calc_pga.red_ref_pga_d;
					bc = ex_calc_bc.black_data1_d;
					break;
				case RLS_GRE:
					p_data = &ex_tmp_10bit_ad_data.cis_gre_ref_d[HDRTBL_SIZE_10BIT];
					adjad = ex_cis_adjustment_tmp.cis_tmp_ad.gre_ref_ad_d;
					pga = &ex_calc_pga.gre_ref_pga_d;
					bc = ex_calc_bc.black_data1_d;
					break;
				case RLS_BLU:
					p_data = &ex_tmp_10bit_ad_data.cis_blu_ref_d[HDRTBL_SIZE_10BIT];
					adjad = ex_cis_adjustment_tmp.cis_tmp_ad.blu_ref_ad_d;
					pga = &ex_calc_pga.blu_ref_pga_d;
					bc = ex_calc_bc.black_data1_d;
					break;
				case RLS_IR1:
					p_data = &ex_tmp_10bit_ad_data.cis_ir1_ref_d[HDRTBL_SIZE_10BIT];
					adjad = ex_cis_adjustment_tmp.cis_tmp_ad.ir1_ref_ad_d;
					pga = &ex_calc_pga.ir1_ref_pga_d;
					bc = ex_calc_bc.black_data1_d;
					break;
				case RLS_IR2:
					p_data = &ex_tmp_10bit_ad_data.cis_ir2_ref_d[HDRTBL_SIZE_10BIT];
					adjad = ex_cis_adjustment_tmp.cis_tmp_ad.ir2_ref_ad_d;
					pga = &ex_calc_pga.ir2_ref_pga_d;
					bc = ex_calc_bc.black_data2_d;
					break;
				case RLS_FL:
					p_data = &ex_tmp_10bit_ad_data.cis_fl_ref_d[HDRTBL_SIZE_10BIT];
					adjad = ex_cis_adjustment_tmp.cis_tmp_ad.fl_ref_ad_d;
					pga = &ex_calc_pga.fl_ref_pga_d;
					bc = ex_calc_bc.black_data2_d;
					break;
				default:
					break;
				}
				for(pix = ex_cis_adjustment_tmp.cis_tmp_eria.left_pix_d[0]; pix < ex_cis_adjustment_tmp.cis_tmp_eria.left_pix_d[1]; pix++)
				{
					checkad = *(p_data + pix) - *(bc + pix);
					if(checkad >= WHITE_CHIP_MIN_TMP_AD)
					{
						sumad += checkad;
						datacnt++;
					}
				}
			}

			if(datacnt == 0)
			{
				return 0;
			}
			tmpad = sumad / datacnt;
			ratio = (float)adjad / (float)tmpad;
			if((ratio < TMP_ADJUSTMENT_RATIO_MIN) || (ratio > TMP_ADJUSTMENT_RATIO_MAX))
			{
				return 0;
			}
			*pga = ratio;
		}
	}
	//tls
	for(led = TLS_RED; led < LED_END; led++)
	{
		datacnt = 0;
		sumad = 0;
		switch(led)
		{
		case TLS_RED:
			p_data = &ex_tmp_10bit_ad_data.cis_red_pen_d[HDRTBL_SIZE_10BIT];
			adjad = ex_cis_adjustment_tmp.cis_tmp_ad.red_pen_ad_d;
			pga = &ex_calc_pga.red_pen_pga_d;
			bc = ex_calc_bc.black_data1_d;
			break;
		case TLS_GRE:
			p_data = &ex_tmp_10bit_ad_data.cis_gre_pen_d[HDRTBL_SIZE_10BIT];
			adjad = ex_cis_adjustment_tmp.cis_tmp_ad.gre_pen_ad_d;
			pga = &ex_calc_pga.gre_pen_pga_d;
			bc = ex_calc_bc.black_data1_d;
			break;
		case TLS_IR1:
			p_data = &ex_tmp_10bit_ad_data.cis_ir1_pen_d[HDRTBL_SIZE_10BIT];
			adjad = ex_cis_adjustment_tmp.cis_tmp_ad.ir1_pen_ad_d;
			pga = &ex_calc_pga.ir1_pen_pga_d;
			bc = ex_calc_bc.black_data1_d;
			break;
		case TLS_IR2:
			p_data = &ex_tmp_10bit_ad_data.cis_ir2_pen_d[HDRTBL_SIZE_10BIT];
			adjad = ex_cis_adjustment_tmp.cis_tmp_ad.ir2_pen_ad_d;
			pga = &ex_calc_pga.ir2_pen_pga_d;
			bc = ex_calc_bc.black_data2_d;
			break;
		default:
			break;
		}
		for(pix = TLS_PIX_SARCH_MIN; pix < TLS_PIX_SARCH_MAX; pix++)
		{
			if(((pix >= 138) && (pix <= 149)) || ((pix >= 282) && (pix <= 233))
			|| ((pix >= 426) && (pix <= 437)) || ((pix >= 570) && (pix <= 581)))
			{
				continue;
			}
			sumad += *(p_data + pix) - *(bc + pix);
			datacnt++;
		}
		if(datacnt == 0)
		{
			return 0;
		}
		tmpad = sumad / datacnt;
		ratio = (float)adjad / (float)tmpad;
		if((ratio < TMP_ADJUSTMENT_RATIO_MIN) || (ratio > TMP_ADJUSTMENT_RATIO_MAX))
		{
			return 0;
		}
		*pga = ratio;
	}
	return 1;
}

/******************************************************************************
 *! @brief Enable/Disable CIS
 * @param[in]	enable
 *              @arg 1 : Power ON
 *              @arg 0 : Power OFF
 * @return 		Succeeded or Failure
 * @retval 		true Succeeded
 * @retval 		false Failure
******************************************************************************/
u8 _pl_cis_enable_set(u8 set)
{
	ER ercd = E_OK;
	PW_CTL_UNION pw_ctl;
	if(get_pl_state() == 0)
	{
		return 0;
	}
	/* I2C通信エラー対策 */
	ercd = wai_sem(ID_I2C3_SEM);
    if(set)
    {
		pw_ctl.LWORD = FPGA_REG.PW_CTL.LWORD;
		/* CIS Power ON */
		pw_ctl.BIT.CIS = 1;
		FPGA_REG.PW_CTL.LWORD = pw_ctl.LWORD;
		ex_cis = 1;
	}
    else
    {
    	pw_ctl.LWORD = FPGA_REG.PW_CTL.LWORD;
        /* CIS Power OFF */
    	pw_ctl.BIT.CIS = 0;
    	FPGA_REG.PW_CTL.LWORD = pw_ctl.LWORD;
		ex_cis = 0;
    }
	/* I2C通信エラー対策 wait 2ms after CIS ON/OFF */
	OSW_TSK_sleep(2);
	sig_sem(ID_I2C3_SEM);

	return 1;
}


/******************************************************************************
 *! @brief CIS Capture data line
 * @param[in]	None
 * @return 		Data Line number
******************************************************************************/
u32 _pl_get_cis_cap_ln(void)
{
	CAP_LN_CIS_UNION cap_ln_cis;
	u32 cisa,cisb;
	if(get_pl_state() == 0)
	{
		return 0;
	}
	cap_ln_cis.LWORD = FPGA_REG.CAP_LN_CIS.LWORD;
	cisa = cap_ln_cis.BIT.CA;
	cisb = cap_ln_cis.BIT.CB;

	if(cisa > cisb)
	{
		return cisb;
	}

	return cisa;
}


/******************************************************************************
 *! @brief CISA LVDS lock status
 * @return 		1: Normal
 * @return		0: Abnormal
******************************************************************************/
u32 _pl_cisa_lock_check(void)
{
	CIS_ST_UNION cis_st;

	if(get_pl_state() == 0)
	{
		return 0;
	}

	cis_st.LWORD = FPGA_REG.CIS_ST.LWORD;
	if(cis_st.BIT.ALOCK)
	{
		return 1;
	}
	return 0;
}


/******************************************************************************
 *! @brief CISB LVDS lock status
 * @return 		1: Normal
 * @return		0: Abnormal
******************************************************************************/
u32 _pl_cisb_lock_check(void)
{
	CIS_ST_UNION cis_st;

	if(get_pl_state() == 0)
	{
		return 0;
	}

	cis_st.LWORD = FPGA_REG.CIS_ST.LWORD;
	if(cis_st.BIT.BLOCK)
	{
		return 1;
	}
	return 0;
}

#if (_DEBUG_EMI_IMAGE_CHECK==1)
void _pl_sampling_cyclic_off(void)
{
	CIS_MD_UNION cis_md = {0};
	cis_md.BIT.CMP = 1;						// CISモード:補正モード*********8bit
	cis_md.BIT.CONT = 0;					// CISモード:採取時のみ動作
	cis_md.BIT.DARK = 0;					// CISモード:通常点灯
	cis_md.BIT.DT_EN = 0;					// CISモード:媒体検出・先頭サイクリック記録停止

	FPGA_REG.CIS_MD.LWORD = cis_md.LWORD;
}
#endif
/*****************************************************************************/
/**
* cis as a position ss sequence. (8bit only)
*
* @param	None
*
* @return	None
*
* @note		All clear
*
******************************************************************************/
void _pl_validation_sensor_ss_seq(void)
{
#if 0//レジスタで採取未許可で対応
	int cnt = 0;
	u8* p_Data;

	p_Data = (u8 *)FPGA_ADDR_SSSEQ;
	for(cnt = 0; cnt < sizeof(BV_SSSEQ_ARRAY_ST); cnt++)
	{
		*(p_Data + cnt) = 0;
	}
#endif
}
/*****************************************************************************/
/**
* cis as a position sequence. (8bit only)
*
* @param	None
*
* @return	None
*
* @note		None
*
******************************************************************************/
void _pl_validation_sensor_seq(void)
{
/* シーケンス初期化 */
	int cnt = 0;
	u8* p_Data;
	u16 time;
	u32 plane_a,plane_b;

	FPGA_CISSEQ_REG_WORD0_UNION cisseq_word0 = {0};
	FPGA_CISSEQ_REG_WORD1_UNION cisseq_word1 = {0};
	FPGA_CISSEQ_REG_WORD2_UNION cisseq_word2 = {0};
	FPGA_CISSEQ_REG_WORD3_UNION cisseq_word3 = {0};
	FPGA_CISSEQ_REG_WORD4_UNION cisseq_word4 = {0};
	FPGA_CISSEQ_REG_WORD5_UNION cisseq_word5 = {0};
	FPGA_CISSEQ_REG_WORD6_UNION cisseq_word6 = {0};
	FPGA_CISSEQ_REG_WORD7_UNION cisseq_word7 = {0};

	p_Data = (u8 *)FPGA_ADDR_CISSEQ;
	for(cnt = 0; cnt < sizeof(BV_CISSEQ_ARRAY_ST); cnt++)
	{
		*(p_Data + cnt) = 0;
	}
	/* 1 採取データ情報初期化 */
	_pl_cis_cap_info(CAP_ONE_SHOT_CHECK_CIS);

	//SEQ 0(1-1)
	plane_a = P_Reserve1;
	plane_b = DOWN_T_IR1;
	cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
	cisseq_word0.BIT.COUNT = 1;
	cisseq_word0.BIT.NXT = 1;
	cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.ir1_pen_da_u;
	cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.ir1_pen_time_u * ex_cis_adjustment_tmp.cis_pga.ir1_pen_pga_d);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.ir1_pen_da_u;
	cisseq_word3.BIT.ILED = 0;
	cisseq_word3.BIT.LED = LED_BLK;
	cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
	cisseq_word3.BIT.S_LED = 0;
	cisseq_word4.BIT.BKADRS = 0;
	cisseq_word4.BIT.WHADRS = 0;
	cisseq_word4.BIT.AINC = 0;
	cisseq_word5.BIT.AOFS = 0;
	cisseq_word5.BIT.DMY = 1;
	cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
	cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
	cisseq_word6.BIT.AINC = (sizeof(TMP_VALIDATION_SENSOR_TBL))/4;
	cisseq_word7.BIT.AOFS = (CHECK_CIS_ADDRESS_OFFSET_1ST)/4;
	cisseq_word7.BIT.DMY = 0;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD0.LWORD = cisseq_word0.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD1.LWORD = cisseq_word1.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD3.LWORD = cisseq_word3.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD4.LWORD = cisseq_word4.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD5.LWORD = cisseq_word5.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD6.LWORD = cisseq_word6.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[0].WORD7.LWORD = cisseq_word7.LWORD;

	//SEQ 1(1-2)
	plane_a = P_Reserve1;
	plane_b = DOWN_T_G;
	cisseq_word0.BIT.TREP = CapCPln[plane_b].TRep;
	cisseq_word0.BIT.COUNT = 1;
	cisseq_word0.BIT.NXT = 1;
	cisseq_word2.BIT.ILED = ex_cis_adjustment_data.cis_da.gre_pen_da_u;
	cisseq_word2.BIT.LED = CapCPln[plane_b].LED;
	time = (u16)(ex_cis_adjustment_data.cis_time.gre_pen_time_u * ex_cis_adjustment_tmp.cis_pga.gre_pen_pga_d);
	if(time > LED_TIME_MAX)
	{
		time = LED_TIME_MAX;
	}
	cisseq_word2.BIT.T_LED = time; //10000ns (10.0us) X/10
	cisseq_word2.BIT.S_LED = ex_cis_adjustment_data.cis_sled.gre_pen_da_u;
	cisseq_word3.BIT.ILED = 0;
	cisseq_word3.BIT.LED = LED_BLK;
	cisseq_word3.BIT.T_LED = 0; //0ns (0.0us) X/10
	cisseq_word3.BIT.S_LED = 0;
	cisseq_word4.BIT.BKADRS = 0;
	cisseq_word4.BIT.WHADRS = 0;
	cisseq_word4.BIT.AINC = 0;
	cisseq_word5.BIT.AOFS = 0;
	cisseq_word5.BIT.DMY = 1;
	cisseq_word6.BIT.BKADRS = CapCPln[plane_b].BkAdr;
	cisseq_word6.BIT.WHADRS = CapCPln[plane_b].WhAdr;
	cisseq_word6.BIT.AINC = (sizeof(TMP_VALIDATION_SENSOR_TBL))/4;
	cisseq_word7.BIT.AOFS = (CHECK_CIS_ADDRESS_OFFSET_2ED)/4;
	cisseq_word7.BIT.DMY = 0;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD0.LWORD = cisseq_word0.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD1.LWORD = cisseq_word1.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD3.LWORD = cisseq_word3.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD4.LWORD = cisseq_word4.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD5.LWORD = cisseq_word5.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD6.LWORD = cisseq_word6.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[1].WORD7.LWORD = cisseq_word7.LWORD;

	//SEQ Last
	plane_a = P_Reserve1;
	plane_b = P_Reserve4;
	cisseq_word0.BIT.TREP = CapCPln[plane_a].TRep;
	cisseq_word0.BIT.COUNT = 0;
	cisseq_word0.BIT.NXT = 0;
	cisseq_word2.LWORD = 0;
	cisseq_word3.LWORD = 0;
	cisseq_word4.LWORD = 0;
	cisseq_word5.BIT.AOFS = 0;
	cisseq_word5.BIT.DMY = 1;
	cisseq_word6.LWORD = 0;
	cisseq_word7.BIT.AOFS = 0;
	cisseq_word7.BIT.DMY = 1;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD0.LWORD = cisseq_word0.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD1.LWORD = cisseq_word1.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD2.LWORD = cisseq_word2.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD3.LWORD = cisseq_word3.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD4.LWORD = cisseq_word4.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD5.LWORD = cisseq_word5.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD6.LWORD = cisseq_word6.LWORD;
	BV_CISSEQ_ARRAY.BV_CISSEQ_ARRAY_T[2].WORD7.LWORD = cisseq_word7.LWORD;
}
void _pl_validation_sensor_regist_data(void)
{
	CIS_MD_UNION cis_md = {0};
	CAP_PLS_UNION cap_pls = {0};
	CAP_MAXBK_UNION cap_maxbk = {0};
	CAP_IDX_UNION cap_idx = {0};
	CAP_IDXMAX_UNION cap_idxmax = {0};
	CAP_BLID_UNION cap_blid = {0};
	CAP_CINIT_UNION cap_cinit = {0};
	CAP_CCIS_UNION cap_ccis = {0};
	CAP_CMG_UNION cap_cmg = {0};
	CAP_MD_UNION cap_md = {0};
	CAP_CUV_UNION cap_cuv = {0};
	CIS_BKI_UNION cis_bki = {0};
	BIN_RNG_UNION bin_rng = {0};
	BIN_INTSET_UNION bin_intset = {0};
	MLT_TH_UNION mlt_th = {0};
	CISDT_SET_UNION cisdt_set = {0};

	//cis_ctlb.BIT.BUF_ON = 1;
	cap_pls.BIT.SEL = 1;					// 搬送パルス設定:疑似搬送パルス
	cap_pls.BIT.ITVL = POS_CYCLE;			/* W/R	疑似搬送パルス周期-1 (10ns) */
	cap_maxbk.BIT.SCAN_MAX_BLOCK = (CHECK_VALIDATION_SUB_LINE - 1);// 調整時採取ブロック数 - 1
	cap_idx.BIT.SCAN_NO = 0;				// 採取番号
	cap_idxmax.BIT.SCAN_BLK_MAX = 0;		// 採取番号最大値
	cap_blid.BIT.BILL_ID = 0;				// 採取ID
	cap_cinit.BIT.SCAN_TIMER = 1;			// 採取タイマーセット値
	cap_ccis.BIT.CIS_START_COUNT = 1;		// CISスタートカウント値
	cap_cmg.BIT.MAG_START_COUNT = 0;		// 磁気センサースタートカウント値(動作させない設定値)
	cis_md.BIT.CMP = 1;						// CISモード:補正データ
	cis_md.BIT.CONT = 0;					// CISモード:採取時のみ動作
	cis_md.BIT.DARK = 0;					// CISモード:通常点灯
	cap_md.BIT.CAPEN = 1;					// 採取モード:採取動作許可
	cap_cuv.BIT.ST = 0;
	cap_cuv.BIT.ADCTYPE = 0;
	cap_cuv.BIT.N1CH = 0;
	cap_cuv.BIT.MBK = 0;
	cis_bki.BIT.CISA = 0;								/* W/R	CISデータブロック番号初期値 */
	cis_bki.BIT.CISB = 0;								/* W/R	CISデータブロック番号初期値 */


	FPGA_REG.CAP_CUV.LWORD = cap_cuv.LWORD;
	//FPGA_REG.CIS_CTLB.LWORD = cis_ctlb.LWORD;
	FPGA_REG.CAP_PLS.LWORD = cap_pls.LWORD;
	FPGA_REG.CAP_MAXBK.LWORD = cap_maxbk.LWORD;
	FPGA_REG.CAP_IDX.LWORD = cap_idx.LWORD;
	FPGA_REG.CAP_IDXMAX.LWORD = cap_idxmax.LWORD;
	FPGA_REG.CAP_BLID.LWORD = cap_blid.LWORD;
	FPGA_REG.CAP_CINIT.LWORD = cap_cinit.LWORD;
	FPGA_REG.CAP_CCIS.LWORD = cap_ccis.LWORD;
	FPGA_REG.CAP_CMG.LWORD = cap_cmg.LWORD;
	FPGA_REG.CAP_MD.LWORD = cap_md.LWORD;
	FPGA_REG.CIS_MD.LWORD = cis_md.LWORD;
	FPGA_REG.CIS_BKI.LWORD = cis_bki.LWORD;
}
/******************************************************************************
 *! @brief validation sensor on
 * @return 		1: scan start.
 * @return		0: cannot pl access.
******************************************************************************/
u8 _pl_validation_sensor_offon(u8 on)
{
	u8 result = 0;

	if(get_pl_state())
	{
		if(on)
		{
			_pl_cis_scan_enable(1);
			_pl_cis_scanst(1);
		}
		else
		{
			_pl_cis_scanst(0);
			_pl_cis_scan_enable(0);
		}
		result = 1;
	}
	return result;
}
/* EOF */
