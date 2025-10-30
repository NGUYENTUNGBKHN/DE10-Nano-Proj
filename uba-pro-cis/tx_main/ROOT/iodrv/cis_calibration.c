/*
 * cis_calibration.c
 *
 *  Created on: 2018/03/14
 *      Author: takada-kazuki
 */

#include "systemdef.h"
#define EXTERN extern
#include "cis_calibration.h"
//#include "baule17_image_inc.h"
//#include "baule17_inc.h"
#include "fpga.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "common.h"
#define EXT
#include "../common/global.h"
#include "cis_ram.c"

//T_ADJDATA adjData;
//T_CADJ_NOPAPER_T_LEVEL cadj_nopaper_t;
unsigned long MemCapCSeq[SZ_MemCapCSeq];				// 採取シーケンス設定(CIS部) メモリーイメージ
unsigned long MemCmpBk[CADJ_N_PLN * CIS_CHPX];   		// CIS黒補正テーブル メモリーイメージ
unsigned long MemCmpWh[CADJ_N_PLN * CIS_CHPX];   		// CIS白補正テーブル メモリーイメージ
//T_ADJDATA calibra_data;

// 採取シーケンス設定(CISプレーン部)
const T_CAPCSEQP CapCSeqP[] =
{
	{1, 1, CISA_R_R, CISB_R_R},
	{1, 1, CISA_R_IR1, CISB_R_IR1},
	{1, 1, CISA_BLANK, CISB_T_G},
	{1, 1, CISA_R_B, CISB_R_B},
	{1, 1, CISA_R_IR2, CISB_R_IR2},
	{1, 1, CISA_R_G, CISB_R_G},
	{0, 1, CISA_BLANK, CISB_BLANK},

	{1, 1, CISA_R_R, CISB_R_R},
	{1, 1, CISA_R_IR1, CISB_R_IR1},
	{1, 1, CISA_BLANK, CISB_T_R},
	{1, 1, CISA_R_B, CISB_R_B},
	{1, 1, CISA_BLANK, CISB_T_IR2},
	{1, 1, CISA_R_G, CISB_R_G},
	{0, 1, CISA_BLANK, CISB_BLANK},

	{1, 1, CISA_R_R, CISB_R_R},
	{1, 1, CISA_R_IR1, CISB_R_IR1},
	{1, 1, CISA_BLANK, CISB_T_G},
	{1, 1, CISA_R_B, CISB_R_B},
	{1, 1, CISA_R_IR2, CISB_R_IR2},
	{1, 1, CISA_R_G, CISB_R_G},
	{0, 1, CISA_BLANK, CISB_BLANK},

	{1, 1, CISA_R_R, CISB_R_R},
	{1, 1, CISA_R_IR1, CISB_R_IR1},
	{1, 1, CISA_BLANK, CISB_T_IR1},
	{1, 1, CISA_R_B, CISB_R_B},
	{1, 1, CISA_R_FL, CISB_R_FL},
	{1, 1, CISA_R_G, CISB_R_G},
	{0, 0, CISA_BLANK, CISB_BLANK}
};

const T_CAPCSEQS CapCSeqSC[] = {   // 採取シーケンス設定(CIS格納アドレス部 補正後データ)
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 0 + CISA_R_R_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 0 + CISB_R_R_OFFSET)},		// CISA_R_R, CISB_R_R
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 0 + CISA_R_IR1_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 0 + CISB_R_IR1_OFFSET)},	// CISA_R_IR1, CISB_R_IR1
	{0, 0, BLOCK_BYTE_SIZE, (PERIOD_100DPI * 0 + CISB_T_G_OFFSET)},															// CISA_T_G, CISB_BLANK
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 0 + CISA_R_B_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 0 + CISB_R_B_OFFSET)},		// CISA_R_B, CISB_R_B
	{BLOCK_BYTE_SIZE, (PERIOD_100DPI * 0 + CISA_R_IR2_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_100DPI * 0 + CISB_R_IR2_OFFSET)},	// CISA_R_IR2, CISB_R_IR2
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 0 + CISA_R_G_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 0 + CISB_R_G_OFFSET)},		// CISA_R_G, CISB_R_G
	{0, 0, 0, 0},																																					// CISA_BLANK, CISB_BLANK

	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 1 + CISA_R_R_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 1 + CISB_R_R_OFFSET)},		// CISA_R_R, CISB_R_R
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 1 + CISA_R_IR1_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 1 + CISB_R_IR1_OFFSET)},	// CISA_R_IR1, CISB_R_IR1
	{0, 0, BLOCK_BYTE_SIZE, (PERIOD_50DPI * 0 + CISB_T_R_OFFSET)},															// CISA_T_R, CISB_BLANK
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 1 + CISA_R_B_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 1 + CISB_R_B_OFFSET)},		// CISA_R_B, CISB_R_B
	{0, 0, BLOCK_BYTE_SIZE, (PERIOD_50DPI * 0 + CISB_T_IR2_OFFSET)},														// CISA_T_IR2, CISB_BLANK
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 1 + CISA_R_G_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 1 + CISB_R_G_OFFSET)},		// CISA_R_G, CISB_R_G
	{0, 0, 0, 0},																											// CISA_BLANK, CISB_BLANK

	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 2 + CISA_R_R_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 2 + CISB_R_R_OFFSET)},		// CISA_R_R, CISB_R_R
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 2 + CISA_R_IR1_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 2 + CISB_R_IR1_OFFSET)},	// CISA_R_IR1, CISB_R_IR1
	{0, 0, BLOCK_BYTE_SIZE, (PERIOD_100DPI * 1 + CISB_T_G_OFFSET)},															// CISA_T_G, CISB_BLANK
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 2 + CISA_R_B_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 2 + CISB_R_B_OFFSET)},		// CISA_R_B, CISB_R_B
	{BLOCK_BYTE_SIZE, (PERIOD_100DPI * 1 + CISA_R_IR2_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_100DPI * 1 + CISB_R_IR2_OFFSET)},	// CISA_R_IR2, CISB_R_IR2
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 2 + CISA_R_G_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 2 + CISB_R_G_OFFSET)},		// CISA_R_G, CISB_R_G
	{0, 0, 0, 0},																											// CISA_BLANK, CISB_BLANK

	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 3 + CISA_R_R_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 3 + CISB_R_R_OFFSET)},		// CISA_R_R, CISB_R_R
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 3 + CISA_R_IR1_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 3 + CISB_R_IR1_OFFSET)},	// CISA_R_IR1, CISB_R_IR1
	{0, 0, BLOCK_BYTE_SIZE, (PERIOD_50DPI * 0 + CISB_T_IR1_OFFSET)},														// CISA_T_IR1, CISB_BLANK
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 3 + CISA_R_B_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 3 + CISB_R_B_OFFSET)},		// CISA_R_B, CISB_R_B
	{0, 0, 0, 0},																											// CISA_R_FL, CISB_R_FL
	{BLOCK_BYTE_SIZE, (PERIOD_200DPI * 3 + CISA_R_G_OFFSET), BLOCK_BYTE_SIZE, (PERIOD_200DPI * 3 + CISB_R_G_OFFSET)},		// CISA_R_G, CISB_R_G
	{0, 0, 0, 0},																											// CISA_BLANK, CISB_BLANK
};
T_CAPCPLN CapCPln[CADJ_N_PLND] = {   // 採取シーケンスプレーン情報(CIS部)
	{0, 2400, 0, UP, REFLECTION, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1900},		// 00:CISA_R_R CpWhObを192から185に変更、LedWhObをCpWhObの補正計数倍に変更(各プレーンも同様), 19/11/08
	{0, 2400, 0, UP, REFLECTION, 0, 0, 2, 0, 9, 9, 0, 0, 0, 1900},		// 01:CISA_R_G CpWhObを192から185に変更(185 * 2.7 = 499.5) , 19/11/08
	{0, 2400, 0, UP, REFLECTION, 0, 0, 3, 0, 18, 18, 0, 0, 0, 1900},		// 02:CISA_R_B CpWhObを192から185に変更 , 19/11/08
	{0, 2400, 0, UP, REFLECTION, 0, 0, 4, 0, 27, 27, 0, 0, 0, 1900},	// 03:CISA_R_IR1 CpWhObを192から185に変更 , 19/11/08
	{0, 4800, 0, UP, REFLECTION, 0, 0, 5, 0, 36, 36, 0, 0, 0, 4300},	// 04:CISA_R_IR2 CpWhObを192から185に変更 , 19/11/08
	{1, 4800, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 05:CISA_R_FL
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 06:CIS_ブランク
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 07:CIS_ブランク
	{0, 2400, 0, DOWN, REFLECTION, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1900},	// 08:CISB_R_R CpWhObを192から185に変更 , 19/11/08
	{0, 2400, 0, DOWN, REFLECTION, 0, 0, 2, 0, 9, 9, 0, 0, 0, 1900},	// 09:CISB_R_G CpWhObを192から185に変更 , 19/11/08
	{0, 2400, 0, DOWN, REFLECTION, 0, 0, 3, 0, 18, 18, 0, 0, 0, 1900},	// 10:CISB_R_B CpWhObを192から185に変更 , 19/11/08
	{0, 2400, 0, DOWN, REFLECTION, 0, 0, 4, 0, 27, 27, 0, 0, 0, 1900},	// 11:CISB_R_IR1 CpWhObを192から185に変更 , 19/11/08
	{0, 4800, 0, DOWN, REFLECTION, 0, 0, 5, 0, 36, 36, 0, 0, 0, 4300},	// 12:CISB_R_IR2 CpWhObを192から185に変更 , 19/11/08
	{1, 4800, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 13:CISB_R_FL
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 14:CIS_ブランク
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 15:CIS_ブランク
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 16:CIS_ブランク
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 17:CIS_ブランク
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 18:CIS_ブランク
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 19:CIS_ブランク
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 20:CIS_ブランク
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 21:CIS_ブランク
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 22:CIS_ブランク
	{0, 2400, 0, DOWN, TRANSMISSION, 0, 0, 7, 0, 54, 54, 0, 0, 0, 4250},	// 23:CISB_T_R (96 * 2.7 = 259.2)
	{0, 2400, 0, DOWN, TRANSMISSION, 0, 0, 8, 0, 63, 63, 0, 0, 0, 4250},	// 24:CISB_T_G CpWhObを96から45に変更(45 * 2.7 = 121.5) , 19/11/08
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// CIS_ブランク
	{0, 2400, 0, DOWN, TRANSMISSION, 0, 0, 9, 0, 72, 72, 0, 0, 0, 4250},	// 26:CISB_T_IR1
	{0, 4800, 0, DOWN, TRANSMISSION, 0, 0, 10, 0, 81, 81, 0, 0, 0, 4250},	// 27:CISB_T_IR2
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 28:CIS_ブランク
	{1, 2400, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},	// 29:CIS_ブランク
};


void set_seq_memory_8bit(void)
{
	// 採取シーケンスメモリーを補正後データ採取用に設定
	CapCSeq_Make(MemCapCSeq, SZ_MemCapCSeq, CapCSeqP, CapCSeqSC, SZ_CapCSeq, CapCPln, CADJ_N_PLND);
	memcpy((unsigned long *) FPGA_ADDR_CISSEQ, MemCapCSeq, sizeof(MemCapCSeq));
}

// シーケンス設定メモリー(CIS部)を生成する
int CapCSeq_Make(unsigned long *p_memSeq, unsigned int szMSeq, const T_CAPCSEQP *p_seqP, const T_CAPCSEQS *p_seqS, unsigned int szSeq, T_CAPCPLN *p_ipln, unsigned int szPln)
{
	unsigned int i;
	unsigned int idxPln;		// プレーン情報の番号
	T_CAPCPLN *p_plnA;   // CISAプレーン情報へのポインター
	T_CAPCPLN *p_plnB;   // CISBプレーン情報へのポインター
	T_CAPCPLN *p_plnLEDA;   // LEDAプレーン情報へのポインター
	T_CAPCPLN *p_plnLEDB;   // LEDBプレーン情報へのポインター
	unsigned long mImg;		// メモリーイメージ
	unsigned long *p_mSeqEd1;   // シーケンスメモリー末尾の次へのポインター
	
	p_mSeqEd1 = p_memSeq + szMSeq;
	
	for (i = 0; i < szSeq; i++) {
		idxPln = p_seqP->PlnA;
		if (idxPln >= szPln) {
			// プレーン情報の番号がサイズ以上ならエラー
			return -1;
		}
		p_plnA = p_ipln + idxPln;
		
		idxPln = p_seqP->PlnB;
		if (idxPln >= szPln) {
			// プレーン情報の番号がサイズ以上ならエラー
			return -1;
		}
		p_plnB = p_ipln + idxPln;
		
		// RT(透過/反射)はCISAの情報を採用
		if (p_plnB->RT == 0) {		// 反射なら、LEDプレーンはCISプレーンと同じ
			p_plnLEDA = p_plnA;
			p_plnLEDB = p_plnB;
		} else {		// 透過なら、LEDプレーンとして対向側のCISプレーンを採用
			p_plnLEDA = p_plnB;
			p_plnLEDB = p_plnA;
		}
		
		// 0ワード目 (VLEDとTrepはCISAの情報を採用)
		//mImg = (p_plnA->VLED) << 16;	// LED駆動電圧選択
		mImg = (p_seqP->Nxt) << 15;	// 1:次シーケンスあり
		mImg |= (p_seqP->Cont) << 14;	// 1:次シーケンス自動開始
		mImg |= p_plnA->TRep;			// 蓄積時間(トリガー信号周期)
		*p_memSeq++ = mImg;
		
		// 1ワード目
		*p_memSeq++ = 0;				// 予約(0とする)
		
		// 2ワード目 CISA
		mImg = (p_plnLEDA->SLED) << 30;		// LED電流測定抵抗選択
		mImg |= (p_plnLEDA->TLED) << 16;	// LED点灯時間
		mImg |= (p_plnLEDA->LED) << 12;		// LED番号
		mImg |= (p_plnLEDA->ILED) << 4;		// LED強度DAC設定値
		*p_memSeq++ = mImg;
		
		// 3ワード目 CISB
		mImg = (p_plnLEDB->SLED) << 30;		// LED電流測定抵抗選択
		mImg |= (p_plnLEDB->TLED) << 16;	// LED点灯時間
		mImg |= (p_plnLEDB->LED) << 12;		// LED番号
		mImg |= (p_plnLEDB->ILED) << 4;		// LED強度DAC設定値
		*p_memSeq++ = mImg;
		
		// 4ワード目 CISA
		mImg = ((p_seqS->AInc) / 4) << 16;	// CISA 格納アドレス増分(周期)/4
		mImg |= (p_plnA->WhAdr) << 8;	// 白補正メモリー先頭アドレス/32
		mImg |= p_plnA->BkAdr;			// 黒補正メモリー先頭アドレス/32
		*p_memSeq++ = mImg;
		
		// 5ワード目 CISA
		mImg = (p_seqS->AOfs) / 4;			// CISA 格納アドレスオフセット/4
		mImg |= (p_plnA->Dmy) << 31;		// 1:ダミー(メモリー格納なし)
		*p_memSeq++ = mImg;
		
		// 6ワード目 CISB
		mImg = ((p_seqS->BInc) / 4) << 16;	// CISB 格納アドレス増分(周期)/4
		mImg |= (p_plnB->WhAdr) << 8;	// 白補正メモリー先頭アドレス/32
		mImg |= p_plnB->BkAdr;			// 黒補正メモリー先頭アドレス/32
		*p_memSeq++ = mImg;
		
		// 7ワード目 CISB
		mImg = (p_seqS->BOfs) / 4;			// CISB 格納アドレスオフセット/4
		mImg |= (p_plnB->Dmy) << 31;		// 1:ダミー(メモリー格納なし)
		*p_memSeq++ = mImg;
		
		p_seqP++;
		p_seqS++;
	}
	
	do {
		*p_memSeq++ = 0;
	} while (p_memSeq < p_mSeqEd1);
	
	return 0;
}
