/*
 * cis_calibration.h
 *
 *  Created on: 2018/03/14
 *      Author: takada-kazuki
 */

#ifndef IODRV_CIS_CALIBRATION_H_
#define IODRV_CIS_CALIBRATION_H_

#pragma push
#pragma pack(4)

#include "fpga.h"
#include <stdint.h>

/**************************************************
 * FPGA定義
 * todo いずれ整理する
 **************************************************/
 // FPGA定義をbaule17_fpgaio.hへ移動, 19/05/27
/**************************************************
 * FPGA定義 END
 **************************************************/
#define CIS_CHPX		288					// CIS AFE-CHごとの画素数

#define CADJ_N_PLN		28					// CIS プレーンの数
#define CADJ_N_PLND		30					// CIS プレーンの数(ダミーを含む)
#define CADJ_CAPBK		128					// CIS調整で採取するブロック数を8から128に変更, 19/01/30
#define CADJ_CAPSZ		0x1800000			// CIS調整で採取する1毎あたりのデータサイズを0x200000から0x1800000に変更, 19/01/30
#define CADJ_CPWHML		1024				// CIS 白補正テーブル計算で掛ける数
#define CADJ_CPWHSB		128					// CIS 白補正テーブル計算で引く数
#define CADJ_TLEDTR		8					// CIS調整 点灯時間調整トライ回数上限
#define CADJ_TLEDWHRTLL	0.95				// CIS調整 点灯時間調整時 白レベル目標比下限
#define CADJ_TLEDWHRTUL	1.05				// CIS調整 点灯時間調整時 白レベル目標比上

#define CIS_ADJ_BLACK_LOWER		(10)	// 黒補正値　最小
#define CIS_ADJ_BLACK_UPPER		(255)	// 黒補正値　最小

// CIS　プレーンNO
enum
{
	CISA_R_R = 0, 					// 上反射R
	CISA_R_G = 1,					// 上反射G
	CISA_R_B = 2,					// 上反射B
	CISA_R_IR1 = 3,					// 上反射IR1
	CISA_R_IR2 = 4,					// 上反射IR2
	CISA_R_FL = 5,					// 上反射FL, 19/12/03
	CISB_R_R = 8,					// 下反射R
	CISB_R_G = 9,					// 下反射G
	CISB_R_B = 10,					// 下反射B
	CISB_R_IR1 = 11,				// 下反射IR1
	CISB_R_IR2 = 12,				// 下反射IR2
	CISB_R_FL = 13,					// 下反射FL, 19/12/03
	CISB_T_R = 23,					// 透過R
	CISB_T_G = 24,					// 透過G
	CISB_T_IR1 = 26,				// 下反射IR1
	CISB_T_IR2 = 27,				// 下反射IR2
	CISA_BLANK = 28,				// 上ブランク, 19/12/03
	CISB_BLANK = 29,				// 下ブランク, 19/12/03
};

typedef struct		// 採取シーケンス設定(CISプレーン部)
{
	unsigned char Cont;		// 1:次シーケンス自動開始
	unsigned char Nxt;		// 1:次シーケンスあり
	unsigned char PlnA;		// CISAプレーン
	unsigned char PlnB;		// CISBプレーン
} T_CAPCSEQP;

typedef struct		// 採取シーケンス設定(CIS格納アドレス部)
{
	unsigned int AInc;		// CISA 格納アドレス増分(周期)/4
	unsigned int AOfs;		// CISA 格納アドレスオフセット/4
	unsigned int BInc;		// CISB 格納アドレス増分(周期)/4
	unsigned int BOfs;		// CISB 格納アドレスオフセット/4
} T_CAPCSEQS;

#define SZ_MemCapCSeq 512						// 採取シーケンス設定(CIS部) メモリーイメージのサイズ(ワード数)
#define SZ_CapCSeq (sizeof(CapCSeqP)/sizeof(CapCSeqP[0]))		// 採取シーケンス設定のサイズ(配列要素数)

typedef struct		// 採取シーケンスプレーン情報(CIS部)
{
	int Dmy;					// 1:ダミー(メモリー格納なし)
	unsigned long TRep;			// 蓄積時間(トリガー信号周期)
	unsigned long TLED;			// LED点灯時間
	unsigned char Side;			// 表裏情報
	unsigned char RT;			// 0:反射/1:透過
	unsigned char VLED;			// LED駆動電圧選択
	unsigned char ILED;			// LED強度DAC設定値
	unsigned char LED;			// LED番号
	unsigned char SLED;			// LED電流測定抵抗選択
	unsigned char BkAdr;		// 黒補正メモリー先頭アドレス/32
	unsigned char WhAdr;		// 白補正メモリー先頭アドレス/32
	unsigned char AdjSet;		// 調整セット番号(調整ダミー入れ替えごとに設定)
	unsigned short CpWhOb;		// 白補正における白目標値
	unsigned short LedWhOb;		// LED調整における黒補正後白目標値
	unsigned short TLedUl;		// LED点灯時間上限
} T_CAPCPLN;

typedef struct		// CISデータのプレーンごと格納アドレス情報
{
	unsigned int Period;		// データ周期(shortデータ単位)
	unsigned int Offset;		// データオフセット(shortデータ単位)
} T_CDATPLN;
#define FW_WORK_MEM_TOP 0x00420000

/* 静電容量センサー関連 */
#define CIS_CAP_OFFSET_DEFAULT 47			// 16->47

/* CIS紙なし2調整関係 */
enum {
	CADJ_E_OK = 0,
	CADJ_E_R_NG = -1,					// 紙なし調整　反射
	CADJ_E_T_NG = -10,					// 紙なし調整　透過 異常
	CADJ_E_T_HIG = -11,					// 紙なし調整　透過　出力高い
	CADJ_E_T_LOW = -12,					// 紙なし調整　透過　出力低い
};
#define CADJ_NOPAPER_TRY 8				// CIS紙なし調整 調整トライ回数上限

#define CIS_REFLECT_WH_EFFECT_AREA1				(19)		// 反射　白基準エリア
#define CIS_REFLECT_WH_EFFECT_AREA2				(35)		// 反射　白基準エリア
#define CIS_TRANSMISSION_WH_EFFECT_AREA1		(53)		// 透過　有効画素エリア
#define CIS_TRANSMISSION_WH_EFFECT_AREA2		(731)		// 透過　有効画素エリア

/**************************************************
 * CIS以外用
 **************************************************/
// 採取シーケンス設定(磁気)
typedef struct {
	uint32_t enable;
	uint32_t adr_inc;
	uint32_t offset;
} T_MAG_PLN;

// 採取シーケンス設定(メカ厚み)
typedef struct {
	uint32_t enable;
	uint32_t adr_inc;
	uint32_t offset;
} T_TC_PLN;

// 採取シーケンス設定(静電容量センサ)
typedef struct {
	uint32_t enable;
	uint32_t adr_inc;
	uint32_t offset;
} T_CP_PLN;

typedef struct {
	T_MAG_PLN mag;
	T_TC_PLN tc;
	T_CP_PLN cp;
} CAP_SSEQ_T;

#define CAP_SSEQ_SIZE (sizeof(cap_sseq)/sizeof(cap_sseq[0]))		// 採取シーケンス設定のサイズ(配列要素数)
#define CAP_SSEQ_RAW_SIZE (sizeof(cap_sseq_raw)/sizeof(cap_sseq_raw[0]))		// 採取シーケンス設定のサイズ(配列要素数)

void set_seq_memory_8bit(void);
int CapCSeq_Make(unsigned long *p_memSeq, unsigned int szMSeq, const T_CAPCSEQP *p_seqP, const T_CAPCSEQS *p_seqS, unsigned int szSeq, T_CAPCPLN *p_ipln, unsigned int szPln);

#pragma pop
#endif /* IODRV_CIS_CALIBRATION_H_ */
