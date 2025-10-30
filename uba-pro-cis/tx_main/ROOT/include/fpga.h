/*
 * fpga.h
 *
 *  Created on: 2021/06/21
 *      Author: suzuki-hiroyuki
 */

#ifndef SRC_INCLUDE_FPGA_H_
#define SRC_INCLUDE_FPGA_H_

#include "common.h"
#include "stdint.h"
#include "typedefine.h"

/****************************************************************/
/**
 * @union CAP_CMD_UNION
 * @brief 採取指示
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t SCAN_START :1;
		uint32_t :31;
	} BIT;
} CAP_CMD_UNION;

/****************************************************************/
/**
 * @union CAP_MAXBK_UNION
 * @brief 最大ブロック数
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t SCAN_MAX_BLOCK :9;
		uint32_t :23;
	} BIT;
} CAP_MAXBK_UNION;

/****************************************************************/
/**
 * @union CAP_IDX_UNION
 * @brief 採取番号
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t SCAN_NO :8;
		uint32_t :24;
	} BIT;
} CAP_IDX_UNION;

/****************************************************************/
/**
 * @union CAP_IDXMAX_UNION
 * @brief 採取番号最大値
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t SCAN_BLK_MAX :8;
		uint32_t :24;
	} BIT;
} CAP_IDXMAX_UNION;

/****************************************************************/
/**
 * @union CAP_MD_UNION
 * @brief 採取モード
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t :31;
		uint32_t CAPEN :1;
	} BIT;
} CAP_MD_UNION;

/****************************************************************/
/**
 * @union CAP_BLID_UNION
 * @brief 媒体ID
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t BILL_ID :16;
		uint32_t :16;
	} BIT;
} CAP_BLID_UNION;

/****************************************************************/
/**
 * @union CAP_CINIT_UNION
 * @brief 採取タイマーセット
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t SCAN_TIMER :11;
		uint32_t :21;
	} BIT;
} CAP_CINIT_UNION;

/****************************************************************/
/**
 * @union CAP_CCIS_UNION
 * @brief CISスタートカウント値
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CIS_START_COUNT :11;
		uint32_t :21;
	} BIT;
} CAP_CCIS_UNION;

/****************************************************************/
/**
 * @union CAP_CMG_UNION
 * @brief 磁気センサースタートカウント値
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t MAG_START_COUNT :11;
		uint32_t :21;
	} BIT;
} CAP_CMG_UNION;

enum {
	ENCODER_CYCLE = 48000-1,//min(40800 - 1),		// 搬送パルス周期、480.00us=2400*(7+1)+4800*(3+1)+2400*1+2400*3(last 予備3Seq)
	MAG_CYCLE = 11000-1,	//DBVのMAG調整と同じ周期でサンプリング 110000ns=110.00us
	POS_CYCLE = 10000-1,
};
/****************************************************************/
/**
 * @union CAP_PLS_UNION
 * @brief 搬送パルス設定
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t ITVL :16;
		uint32_t :15;
		uint32_t SEL :1;
	} BIT;
} CAP_PLS_UNION;

/****************************************************************/
/**
 * @union CAP_CUV_UNION
 * @brief 採取設定UV（蛍光）センサー
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t ST :11;	//	W/R	UVセンサースタートカウント値。
		uint32_t :18;
		uint32_t ADCTYPE :1;//	W/R ADCタイプ設定 0:iVIZION2 1:UBA700
		uint32_t N1CH :1;	//	W/R	センサーのチャネル数 0:1ch(iVIZION2) 1:2ch(UBA700)
		uint32_t MBK :1;	//	W/R	0:毎ブロックch0が動作 1:偶数ブロックにch0,奇数ブロックにch1が動作。
	} BIT;
} CAP_CUV_UNION;

/****************************************************************/
/**
 * @union CAP_LN_CIS_UNION
 * @brief CISライン数
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		int32_t CA :16;
		int32_t CB :16;
	} BIT;
} CAP_LN_CIS_UNION;

/****************************************************************/
/**
 * @union CAP_DBK_MG_UNION
 * @brief 磁気ブロック番号
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t MG :9;
		uint32_t :23;
	} BIT;
} CAP_DBK_MG_UNION;

/****************************************************************/
/**
 * @union CAP_TM0_UNION
 * @brief
 */
/****************************************************************/
typedef union
{
	uint32_t LOWRD;
	struct
	{
		uint32_t TIMER :11;
		uint32_t :21;
	} BIT;
} CAP_TM0_UNION;

/****************************************************************/
/**
 * @union CAP_DBK_UV_UNION
 * @brief
 */
/****************************************************************/
typedef union
{
	uint32_t LOWRD;
	struct
	{
		uint32_t BLK :8;
		uint32_t :24;
	} BIT;
} CAP_DBK_UV_UNION;

/****************************************************************/
/**
 * @union CIS_MD_UNION
 * @brief CISモード
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CMP :1;
		uint32_t CONT :1;
		uint32_t DARK :1;
		uint32_t :28;
		uint32_t DT_EN :1;
	} BIT;
} CIS_MD_UNION;

/****************************************************************/
/**
 * @union CIS_BKI_UNION
 * @brief CISデータブロック番号初期値
 */
/****************************************************************/
typedef union
{
	int32_t LWORD;
	struct
	{
		int32_t CISA :16;
		int32_t CISB :16;
	} BIT;
} CIS_BKI_UNION;

/****************************************************************/
/**
 * @union CIS_ST_UNION
 * @brief CIS状態
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t ALOCK :1;
		uint32_t BLOCK :1;
		uint32_t :28;
		uint32_t ATYPE :1;
		uint32_t BTYPE :1;
	} BIT;
} CIS_ST_UNION;

/****************************************************************/
/**
 * @union UV_CHK_UNION
 * @brief UV状態
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CHK0 :1;
		uint32_t CHK1 :1;
		uint32_t :30;
	} BIT;
} UV_CHK_UNION;

/****************************************************************/
/**
 * @union PW_CTL_UNION
 * @brief 電源状態
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;

	struct
	{
		uint32_t CIS :1;
		uint32_t VM :1;
		uint32_t IOEX :1;
		uint32_t :28;
		uint32_t VSNSSEL :1;
	} BIT;

} PW_CTL_UNION;

/****************************************************************/
/**
 * @union CIS_CTLB_UNION
 * @brief CIS制御信号バッファ制御
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t BUF_ON :1;		// CIS制御信号バッファ制御、1:オン, 0:オフ
		uint32_t :31;
	} BIT;
} CIS_CTLB_UNION;

typedef enum {
	DAC_ENT_LED = 0,
	DAC_CNT_LED = 1,
	DAC_PBI_LED = 2,
	DAC_PBO_LED = 3,
	DAC_EXT_LED = 4,
	DAC_TH_LED 	= 5,
	DAC_VREF_LED = 6,
	DAC_ENTTH_LED 	= 7,

	DAC_LED_MAX = 8,
} DAC_LED_T;
/****************************************************************/
/**
 * @union DAC_ST_UNION
 * @brief DAC制御部状態
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t BUSY :16;
		uint32_t :15;
		uint32_t TYPE :1;
	} BIT;
} DAC_ST_UNION;
/****************************************************************/
/**
 * @union DAC_CTL_UNION
 * @brief DAC制御設定状態
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t ZERO :16;
		uint32_t :16;
	} BIT;
} DAC_CTL_UNION;
/****************************************************************/
/**
 * @union DAC_WR_UNION
 * @brief DAC書き込み
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DAT :12;
		uint32_t CMD :4;
		uint32_t :16;
	} BIT;
} DAC_WR_UNION;

/****************************************************************/
/**
 * @union SSPI_ST_UNION
 * @brief センサーSPI状態
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t PTR :11;
		uint32_t :20;
		uint32_t BUSY :1;
	} BIT;
} SSPI_ST_UNION;

/****************************************************************/
/**
 * @union SSPI_CMD_UNION
 * @brief センサーSPI転送指示
 */
/****************************************************************/
typedef enum {
	SSPI_DN_CISA = 0,
	SSPI_DN_CISB = 1,
	SSPI_DN_MAG = 2,
	SSPI_DN_CPS = 3,
} SSPI_DN_T;
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t SZ :11;   		//転送バイト数-1
		uint32_t :5;			//予約
		uint32_t RS :1;			//受信切り替えバイト 1:1バイト送信後、受信に切り替える  0:常に送信方向。
		uint32_t :13;			//予約
		SSPI_DN_T DN :2;		//0:CISA  1:CISB  2:予約  3:予約
	} BIT;
} SSPI_CMD_UNION;

/****************************************************************/
/**
 * @union RUART_TX_UNION
 * @brief RFID UART送信バッファ状態
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t PTR :11;
		uint32_t :5;
		uint32_t SZ :11;
		uint32_t :5;
	} BIT;
} RUART_TX_UNION;

/****************************************************************/
/**
 * @union RUART_RX_UNION
 * @brief RFID UART受信バッファ状態
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t PTR :11;
		uint32_t :5;
		uint32_t INTP :11;
		uint32_t :2;
		uint32_t ERR_F :1;
		uint32_t PCLR :1;
		uint32_t EN :1;
	} BIT;
} RUART_RX_UNION;

/****************************************************************/
/**
 * @union RFID_UNION
 * @brief RFID制御
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t :29;
		uint32_t BAUDRATE :1;
		uint32_t ICB :1;
		uint32_t RST :1;
	} BIT;
} RFID_UNION;
/****************************************************************/
/**
 * @union IF_UNION
 * @brief I/F信号
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t ITTL1 :1;	//R	I-TTL1
		uint32_t ITTL2 :1;	//R	I-TTL2
		uint32_t ITTL3 :1;	//R	I-TTL3 out ベゼル点灯に使用
		uint32_t :26;		//-	予約
		uint32_t STYPE0 :1;	//R	サブ基板タイプ識別信号
		uint32_t STYPE1 :1;	//R	000:TYPE1	100:TYPE2  101:TYPE3
		uint32_t STYPE2 :1;	//R
		//TYPE1
		// STYPE0 0, STYPE1 0, STYPE2 0
		//TYPE2
		// STYPE0 0, STYPE1 0, STYPE2 1

	} BIT;
} IF_UNION;
/****************************************************************/
/**
 * @union SNS_CTL_UNION
 * @brief センサー点灯制御
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct {
		uint32_t :16;
		uint32_t PSG:16;		// 全ポジションセンサゲイン設定, 19/04/24
	} WORD;
	struct
	{
		uint32_t :8;
		uint32_t ENC_ON :1;
		uint32_t :7;
		uint32_t PSGD0 :1;
		uint32_t PSGD1 :1;
		uint32_t PSGD2 :1;
		uint32_t PSGD3 :1;
		uint32_t PSGD4 :1;
		uint32_t PSGD5 :1;
		uint32_t :10;
	} BIT;
} SNS_CTL_UNION;

/****************************************************************/
/**
 * @union PW_DET_UNION
 * @brief 電源検知
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t VM :1;
		uint32_t  :1;
		uint32_t SDOC :1;
		uint32_t :29;
	} BIT;
} PW_DET_UNION;
/****************************************************************/
/**
 * @union ENC_MSMIN_UNION
 * @brief エンコーダー周期最小値
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t ITVL :16;
		uint32_t :16;
	} BIT;
} ENC_MSMIN_UNION;

/****************************************************************/
/**
 * @union ENC_MSMAX_UNION
 * @brief エンコーダー周期最大値
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t ITVL :16;
		uint32_t :16;
	} BIT;
} ENC_MSMAX_UNION;

/****************************************************************/
/**
 * @union ENC_MS_UNION
 * @brief エンコーダー測定結果
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t ITVL :16;
		uint32_t :15;
		uint32_t DIR:1;	//R	ローラー軸エンコーダー回転方向 0:正方向 1:逆方向
	} BIT;
} ENC_MS_UNION;

/****************************************************************/
/**
 * @union ENC_MSCLR_UNION
 * @brief エンコーダー周期統計クリア
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CLR :1;
		uint32_t :31;
	} BIT;
} ENC_MSCLR_UNION;

/****************************************************************/
/**
 * @union ENC_CNT_UNION
 * @brief エンコーダーカウント
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CNT :16;
		uint32_t :16;
	} BIT;
} ENC_CNT_UNION;

/****************************************************************/
/**
 * @union ENC_DN_UNION
 * @brief エンコーダーダウンカウンター
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CNT :16;
		uint32_t :16;
	} BIT;
} ENC_DN_UNION;

/****************************************************************/
/**
 * @union CISDT_SET_UNION
 * @brief 媒体検出設定
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t TH_PX :10;	//	W/R	媒体検出で有ラインとなるための画素数
		uint32_t :6;		//	-	(予約)
		uint32_t TH_LN :3;	//	W/R	媒体検出で有確定するための連続ライン数閾値
		uint32_t :5;		//	-	(予約)
		uint32_t PREBK :5;	//	W/R	媒体検出前記録ブロック数-1
		uint32_t :3;		//	-	(予約)
	} BIT;
} CISDT_SET_UNION;

/****************************************************************/
/**
 * @union CISDT_ST_UNION
 * @brief 媒体検出情報
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t PRELN :10;		//	R	媒体検出前のライン数。サンプリング開始でクリア。2値化対象プレーンごとにカウントアップし、検出で停止する。
		uint32_t :2;			//	-	(予約)
		uint32_t LNRM :3;		//	R	連続ライン数残り(0で検知)
		uint32_t :1;			//	-	(予約)
		uint32_t OLDESTBK :5;	//	R	画像内で(媒体検出前にサイクリックに記録された)最も古いデータのブロック番号(PREBK以下の値をとる)
		uint32_t :11;			//	-	(予約)
	} BIT;
} CISDT_ST_UNION;

/****************************************************************/
/**
 * @union BIN_RNG_UNION
 * @brief 2値化対象画素範囲
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t LL :10;	//W/R	2値化対象画素番号下限
		uint32_t :6;		//-	(予約)
		uint32_t UL :10;	//W/R	2値化対象画素番号上限
		uint32_t :6;		//-	(予約)
	} BIT;
} BIN_RNG_UNION;
/****************************************************************/
/**
 * @union BIN_INTSET_UNION
 * @brief 2値化割込み設定
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t LN :9;		//W/R	割込み基準ライン
		uint32_t :21;		//-	(予約)
	} BIT;
} BIN_INTSET_UNION;
/****************************************************************/
/**
 * @union BIN_ST_UNION
 * @brief 2値化状態
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t LN :10;	//R	現在のライン番号
		uint32_t :6;		//-	(予約)
		uint32_t EXPX :8;	//R	最新の媒体有り画素数
		uint32_t :8;		//-	(予約)
	} BIT;
} BIN_ST_UNION;
/****************************************************************/
/**
 * @union MLT_TH_UNION
 * @brief 重券検出閾値
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t LTH :8;	//W/R	連続ライン数閾値
		uint32_t :24;		//-	(予約)
	} BIT;
} MLT_TH_UNION;
/****************************************************************/
/**
 * @union MLT_ST_UNION
 * @brief 重券検出状態
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t LN :10;	//R	現在のライン番号
		uint32_t :6;		//-	(予約)
		uint32_t LNRM :8;	//R	連続ライン数残り(0で検知)
		uint32_t :7;		//-	(予約)
		uint32_t DT :1;		//R	重券検知 1:検知した
	} BIT;
} MLT_ST_UNION;
/****************************************************************/
/**
 * @union HENC_MSMIN_UNION
 * @brief 搬送エンコーダー周期最小値
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CNT :16;
		uint32_t :16;
	} BIT;
} HENC_MSMIN_UNION;

/****************************************************************/
/**
 * @union HENC_MSMAX_UNION
 * @brief 搬送エンコーダー周期最大値
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CNT :16;
		uint32_t :16;
	} BIT;
} HENC_MSMAX_UNION;

/****************************************************************/
/**
 * @union HENC_MS_UNION
 * @brief エンコーダー測定結果
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t ITVL :16;
		uint32_t :16;
	} BIT;
} HENC_MS_UNION;

/****************************************************************/
/**
 * @union HENC_MSCLR_UNION
 * @brief 搬送エンコーダー周期統計クリア
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CLR :1;
		uint32_t :31;
	} BIT;
} HENC_MSCLR_UNION;

/****************************************************************/
/**
 * @union SENC_MSMIN_UNION
 * @brief 搬送エンコーダー周期最小値
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CNT :23;
		uint32_t :9;
	} BIT;
} SENC_MSMIN_UNION;

/****************************************************************/
/**
 * @union SENC_MSMAX_UNION
 * @brief 搬送エンコーダー周期最大値
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CNT :23;
		uint32_t :9;
	} BIT;
} SENC_MSMAX_UNION;

/****************************************************************/
/**
 * @union SENC_MS_UNION
 * @brief エンコーダー測定結果
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t ITVL :23;
		uint32_t :9;
	} BIT;
} SENC_MS_UNION;

/****************************************************************/
/**
 * @union SENC_MSCLR_UNION
 * @brief 搬送エンコーダー周期統計クリア
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CLR :1;
		uint32_t :31;
	} BIT;
} SENC_MSCLR_UNION;
/****************************************************************/
/**
 * @union PENC_MSMIN_UNION
 * @brief PBエンコーダー周期最小値
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CNT :23;	//R	"PBモーターエンコーダー周期測定結果の最小値。
		uint32_t :9;		//-	(予約)
	} BIT;
} PENC_MSMIN_UNION;
/****************************************************************/
/**
 * @union PENC_MSMAX_UNION
 * @brief PBエンコーダー周期最大値
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CNT :23;	//R	"PBモーターエンコーダー周期測定結果の最大値。
		uint32_t :9;		//-	(予約)
	} BIT;
} PENC_MSMAX_UNION;
/****************************************************************/
/**
 * @union PENC_MS_UNION
 * @brief PBエンコーダー測定結果
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t ITVL :23;	//R	PBモーターエンコーダー周期測定結果。10us単位。83.88607msまで測定可能。
		uint32_t :9;		//-	(予約)
	} BIT;
} PENC_MS_UNION;
/****************************************************************/
/**
 * @union PENC_MSCLR_UNION
 * @brief PBエンコーダー周期統計クリア
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CLR :1;	//W	PBモーターエンコーダー周期統計クリア。1を書きこむと、エンコーダー周期最小値、エンコーダー周期最大値がクリアされる。本ビットは自動的に0クリアされる。
		uint32_t :31;		//-	(予約)
	} BIT;
} PENC_MSCLR_UNION;
/****************************************************************/
/**
 * @union ACT_EN_UNION
 * @brief モーター動作許可
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t EN :1;
		uint32_t SLP_N :1;
		uint32_t :30;
	} BIT;
} ACT_EN_UNION;
/****************************************************************/
/**
 * @union CMOT_CTL_UNION
 * @brief 幅寄せモーター制御
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DUTY :11;
		uint32_t :19;
		MOT_CMD_T CMD:2;
	} BIT;
} CMOT_CTL_UNION;

/****************************************************************/
/**
 * @union LMOT_CTL_UNION
 * @brief レバーモーター制御
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DUTY :11;
		uint32_t :19;
		MOT_CMD_T CMD:2;
	} BIT;
} LMOT_CTL_UNION;
/****************************************************************/
/**
 * @union HMOT_CTL_UNION
 * @brief 搬送モーター制御
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DUTY :11;
		uint32_t :3;
		uint32_t ENCDAC:1;
		uint32_t :1;
		uint32_t LCL:4;
		uint32_t UCL:4;
		uint32_t PMSET:3;
		uint32_t :3;
		MOT_CMD_T CMD :2;
	} BIT;
} HMOT_CTL_UNION;

/****************************************************************/
/**
 * @union HENC_CNT_UNION
 * @brief 搬送モーターエンコーダーカウント
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CNT :16;
		uint32_t :16;
	} BIT;
} HENC_CNT_UNION;

/****************************************************************/
/**
 * @union SMOT_CTL_UNION
 * @brief 収納モーター制御
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DUTY :11;
		uint32_t :3;
		uint32_t ENCDAC:1;
		uint32_t REFSEL:1;
		uint32_t LCL:4;
		uint32_t UCL:4;
		uint32_t PMSET:3;
		uint32_t :3;
		MOT_CMD_T CMD :2;
	} BIT;
} SMOT_CTL_UNION;

/****************************************************************/
/**
 * @union SENC_CNT_UNION
 * @brief 収納モーターエンコーダーカウント
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CNT :16;
		uint32_t :16;
	} BIT;
} SENC_CNT_UNION;

/****************************************************************/
/**
 * @union SENC_DN_UNION
 * @brief 収納モーターエンコーダダウンカウント
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CNT :16;	//W/R	本レジスターに1以上の値を書き込むと収納モーターエンコーダーのパルスを0までダウンカウントする。0になると割込み(FPGA_IRQ41)発生。
		uint32_t :16;		//-		(予約)
	} BIT;
} SENC_DN_UNION;
/****************************************************************/
/**
* @union SMOT_ONTM_UNION
* @brief 収納モーターオン時間積算
*/
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t ONTM :25;	//R		収納モーターオン時間積算値(10ns単位)。SENC_DNレジスターに書き込むと0クリアされる。SENC_DNレジスターに書き込まれてから最初のエンコーダーパルスでオン時間積算をスタートし、SENC_DNレジスターが0になると積算を停止する。
		uint32_t :7;		//-		(予約)
	} BIT;
} SMOT_ONTM_UNION;

/****************************************************************/
/**
 * @union PMOT_CTL_UNION
 * @brief PBモーター制御
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DUTY :11;
		uint32_t :5;
		uint32_t LCL:4;
		uint32_t UCL:4;
		uint32_t PMSET:3;
		uint32_t :3;
		MOT_CMD_T CMD :2;
	} BIT;
} PMOT_CTL_UNION;

/****************************************************************/
/**
 * @union PENC_CNT_UNION
 * @brief PBモーターエンコーダーカウント
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t CNT :16;
		uint32_t :16;
	} BIT;
} PENC_CNT_UNION;

/****************************************************************/
/**
 * @union EVREC_LL_UNION
 * @brief イベント記録 下限アドレス
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t LL :28;
		uint32_t :4;
	} BIT;
} EVREC_LL_UNION;

/****************************************************************/
/**
 * @union EVREC_UL_UNION
 * @brief イベント記録 上限アドレス
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t UL :28;
		uint32_t :4;
	} BIT;
} EVREC_UL_UNION;

/****************************************************************/
/**
 * @union EVREC_DST_UNION
 * @brief イベント記録 上限アドレス
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DST :28;
		uint32_t :3;
		uint32_t OV:1;
	} BIT;
} EVREC_DST_UNION;

/****************************************************************/
/**
 * @union EVREC_S0_UNION
 * @brief イベント記録 上限アドレス
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t EN :1;
		uint32_t CYC :1;
		uint32_t :30;
	} BIT;
} EVREC_S0_UNION;

/****************************************************************/
/**
 * @union UCID0_UNION
 * @brief UCID0
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t UCID0 :32;	//R	ユニークチップID(64bit)の下位32bit
	} BIT;
} UCID0_UNION;

/****************************************************************/
/**
 * @union UCID1_UNION
 * @brief UCID1
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t UCID1 :32;	//R	ユニークチップID(64bit)の上位32bit
	} BIT;
} UCID1_UNION;

/****************************************************************/
/**
 * @union FVER_UNION
 * @brief FPGAバージョン
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t MI :8;
		uint32_t MA :8;
		uint32_t :16;
	} BIT;
} FVER_UNION;
/****************************************************************/
/**
 * @struct FPGA_REG_STRUCTURE
 * @brief FPGAレジスタ構造体
 */
/****************************************************************/
typedef struct
{
	CAP_CMD_UNION CAP_CMD;				//	000
	CAP_MAXBK_UNION CAP_MAXBK;			//	004
	CAP_IDX_UNION CAP_IDX;				//	008
	CAP_IDXMAX_UNION CAP_IDXMAX;		//	00C
	CAP_MD_UNION CAP_MD;				//	010
	CAP_BLID_UNION CAP_BLID;			//	014
	uint32_t RESERVED1;					//	018
	CAP_CINIT_UNION CAP_CINIT;			//	01C
	CAP_CCIS_UNION CAP_CCIS;			//	020
	uint32_t RESERVED24;				//	024
	CAP_CMG_UNION CAP_CMG;				//	028
	uint32_t RESERVED2C;				//	02C
	CAP_PLS_UNION CAP_PLS;				//	030
	CAP_CUV_UNION CAP_CUV;				//	034
	uint32_t RESERVED3[2];				//	038,03C
	CAP_LN_CIS_UNION CAP_LN_CIS;		//	040
	uint32_t RESERVED4[3];				//	044,048,04C
	CAP_TM0_UNION CAP_TM0;				//	050
	uint32_t RESERVED5[2];				//	054,058
	CAP_DBK_UV_UNION CAP_DBK_UV;		//	05C
	CIS_MD_UNION CIS_MD;				//	060
	CIS_BKI_UNION CIS_BKI;				//	064
	uint32_t RESERVED6[6];				//	068,06C,070,074,078,07C
	CIS_ST_UNION CIS_ST;				//	080
	uint32_t RESERVED7;					//	084
	UV_CHK_UNION UV_CHK;				//	088
	uint32_t RESERVED8[4];				//	08C,090,094,098
	PW_CTL_UNION PW_CTL;				//	09C
	uint32_t RESERVED9[4];				//	0A0,0A4,0A8,0AC
	DAC_ST_UNION DAC_ST;				//	0B0
	DAC_CTL_UNION DAC_CTL;				//	0B4
	SSPI_ST_UNION SSPI_ST;				//	0B8
	SSPI_CMD_UNION SSPI_CMD;			//	0BC
	uint32_t RESERVED11[4];				//	0C0,0C4,0C8,0CC
	RUART_TX_UNION RUART_TX;			//	0D0
	RUART_RX_UNION RUART_RX;			//	0D4
	RFID_UNION RFID;					//	0D8
	IF_UNION IF;						//	0DC
	uint32_t RESERVED12;				//	0E0
	SNS_CTL_UNION SNS_CTL;				//	0E4
	uint32_t RESERVED13;				//	0E8
	PW_DET_UNION PWDET;					//	0EC
	ENC_MSMIN_UNION ENC_MSMIN;			//	0F0
	ENC_MSMAX_UNION ENC_MSMAX;			//	0F4
	ENC_MS_UNION ENC_MS;				//	0F8
	ENC_MSCLR_UNION ENC_MSCLR;			//	0FC
	ENC_CNT_UNION ENC_CNT;				//	100
	uint32_t RESERVED14[3];				//	104,108,10C
	ENC_DN_UNION ENC_DN[4];				//	110,114,118,11C
	uint32_t RESERVED15[12];			//	120-14C [(80-32)/4=12]
	CISDT_SET_UNION CISDT_SET;			//	150
	CISDT_ST_UNION CISDT_ST;			//	154
	uint32_t RESERVED158[2];			//	158-15C
	BIN_RNG_UNION BIN_RNG;				//	160
	BIN_INTSET_UNION BIN_INTSET;		//	164
	BIN_ST_UNION BIN_ST;				//	168
	uint32_t RESERVED16C;				//	16C
	MLT_TH_UNION MLT_TH;				//	170
	MLT_ST_UNION MLT_ST;				//	174
	uint32_t RESERVED178[2];			//	178,17C
	HENC_MSMIN_UNION HENC_MSMIN;		//	180
	HENC_MSMAX_UNION HENC_MSMAX;		//	184
	HENC_MS_UNION HENC_MS;				//	188
	HENC_MSCLR_UNION HENC_MSCLR;		//	18C
	SENC_MSMIN_UNION SENC_MSMIN;		//	190
	SENC_MSMAX_UNION SENC_MSMAX;		//	194
	SENC_MS_UNION SENC_MS;				//	198
	SENC_MSCLR_UNION SENC_MSCLR;		//	19C
	PENC_MSMIN_UNION PENC_MSMIN;		//	1A0
	PENC_MSMAX_UNION PENC_MSMAX;		//	1A4
	PENC_MS_UNION PENC_MS;				//	1A8
	PENC_MSCLR_UNION PENC_MSCLR;		//	1AC
	uint32_t RESERVED16[20];			//	1B0-1FC [(256-176)/4=20]
	ACT_EN_UNION ACT_EN;				//	200
	uint32_t RESERVED17[3];				//	204,208,20C
	CMOT_CTL_UNION CMOT_CTL;			//	210
	uint32_t RESERVED18;				//	214
	LMOT_CTL_UNION LMOT_CTL;			//	218
	uint32_t RESERVED21C[5];			//	21C-22C
	HMOT_CTL_UNION HMOT_CTL;			//	230
	HENC_CNT_UNION HENC_CNT;			//	234
	uint32_t RESERVED19[2];				//	238,23C
	SMOT_CTL_UNION SMOT_CTL;			//	240
	SENC_CNT_UNION SENC_CNT;			//	244
	SENC_DN_UNION SENC_DN;				//	248
	SMOT_ONTM_UNION SMOT_ONTM;			//	24C
	PMOT_CTL_UNION PMOT_CTL;			//	250
	PENC_CNT_UNION PENC_CNT;			//	254
	uint32_t RESERVED20[10];			//	258-27C [(128-72)/4=10]
	EVREC_LL_UNION EVREC_LL;			//	280
	EVREC_UL_UNION EVREC_UL;			//	284
	EVREC_DST_UNION EVREC_DST;			//	288
	EVREC_S0_UNION EVREC_S0;			//	28C
	uint32_t RESERVED21[86];			//	290-3E4 [(488-144)/4=86]
	UCID0_UNION UCID0;					//	3E8
	UCID1_UNION UCID1;					//	3EC
	uint32_t TIMSTMP;					//	3F0
	uint32_t RESERVED22[2];				//	3F4,3F8
	FVER_UNION FVER;					//	3FC
} FPGA_REG_STRUCT, *PFPGA_REG_STRUCT;

/****************************************************************/
/**
 * @union DAC_DATA
 * @brief FPGAバージョン
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DAT :8;
		uint32_t :24;
	} BIT;
} DAC_DATA_UNION;
/****************************************************************/
/**
 * @struct DACDAT_REG_STRUCT
 * @brief FPGA DAC DATAレジスタ構造体
 */
/****************************************************************/
//並びがFPGAのアドレスに対応しているので、並び替え、削除 禁止
enum {
	DAC_INDEX_SIDE_LED = 0,		//糸検知 発光強度/* 0 *///not use
	DAC_INDEX_PBIN_LED,			//PBIN 発光強度  /* 1 */
	DAC_INDEX_PBOUT_LED,		//PBOUT 発光強度/* 2 */
	DAC_INDEX_UV1_LED,			//UV 下発光強度/* 3 */
	DAC_INDEX_UV_LED,			//UV 上発光強度/* 4 */
	DAC_INDEX_SMOT_CUR_FULL,	//収納モーターFull 電流制限/* 5 */// not use
	DAC_INDEX_ENT_THR,			//入口 検出閾値/* 6 *//*  ポジション入口 閾値 */
	DAC_INDEX_EXT_THR,			//幅寄せ/出口/ニアフル/BOX1/BOX2 検出閾値/* 7 *//*  ポジションその他 閾値 */
	DAC_INDEX_ENT_LED,			//入口 発光強度/* 8 */
	DAC_INDEX_CEN_LED,			//幅寄せ 発光強度/* 9 */
	DAC_INDEX_EXT_LED,			//出口 発光強度/* 10 */
	DAC_INDEX_SHUTTER_MOT_CUR,	//シャッターモーター 電流制限/* 11 */
	DAC_INDEX_PBMOT_CUR,		//PBモーター 電流制限/* 12 */
	DAC_INDEX_CMOT_CUR,			//幅寄せモーター 電流制限/* 13 */
	DAC_INDEX_SMOT_CUR,			//収納モーター 電流制限/* 14 *///not use
	DAC_INDEX_HMOT_CUR,			//搬送モーター 電流制限/* 15 *///not use
};
typedef struct
{
	DAC_DATA_UNION DAC_DATA[16];			//	0000-003C
	uint32_t RESERVED1[240];				//	003D-03FF
	uint32_t HMOT_DAC[256];					//	0400-07FF
	uint32_t SMOT_DAC0[256];				//	0800-0BFF
	uint32_t SMOT_DAC1[256];				//	0C00-0FFF
} DACDAT_REG_STRUCT, *PDACDAT_REG_STRUCT;

/****************************************************************/
/**
 * @union DUTY_UNION
 * @brief MOTOR DUTY
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DAT :11;
		uint32_t :21;
	} BIT;
} DUTY_UNION;

/****************************************************************/
/**
 * @union MOTPRM_VAL_UNION
 * @brief MOTOR PARAM VALUE
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DAT :16;
		uint32_t :16;
	} BIT;
} MOTPRM_VAL_UNION;
/****************************************************************/
/**
 * @union FB_UNION
 * @brief Feed back limit
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DAT :15;
		uint32_t :17;
	} BIT;
} FB_UNION;

/****************************************************************/
/**
 * @union UD_FRQ_UNION
 * @brief update freq
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DAT :6;
		uint32_t :26;
	} BIT;
} UD_FRQ_UNION;

/****************************************************************/
/**
 * @union CNT2NXT_UNION
 * @brief pulse count for next param
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DAT :12;
		uint32_t :20;
	} BIT;
} CNT2NXT_UNION;

/****************************************************************/
/**
 * @union DUTY_OF_UNION
 * @brief duty offset
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD;
	struct
	{
		uint32_t DAT :11;
		uint32_t :21;
	} BIT;
} DUTY_OF_UNION;
/****************************************************************/
/**
 * @union MOTPRM_UNION
 * @brief MOTOR PARAMETER
 */
/****************************************************************/
typedef union
{
	uint32_t LWORD[16];
	struct
	{
		DUTY_UNION DUTY_INIT;		//	000
		MOTPRM_VAL_UNION SPD_OB;	//	004
		MOTPRM_VAL_UNION DGAIN;		//	008
		MOTPRM_VAL_UNION PGAIN;		//	00C
		MOTPRM_VAL_UNION IDC;		//	010
		MOTPRM_VAL_UNION ISM;		//	014
		MOTPRM_VAL_UNION IGAIN;		//	018
		FB_UNION FB_LL;				//	01C
		FB_UNION FB_UL;				//	020
		UD_FRQ_UNION UD_FRQ;		//	024
		DUTY_UNION DUTY_LL;			//	028
		DUTY_UNION DUTY_UL;			//	02C
		CNT2NXT_UNION CNT2NXT;		//	030
		DUTY_OF_UNION DUTY_OF;		//	034
		uint32_t RESERVED0[2];		//	038-03C
	} BIT;
} MOTPRM_UNION;

typedef struct
{
	MOTPRM_UNION M0[8];				//	000-1C0
	MOTPRM_UNION M1[8];				//	200-3C0
	MOTPRM_UNION M2[8];				//	400-5C0
} MOTPRM_REG_STRUCT, *PMOTPRM_REG_STRUCT;

/****************************************************************/
/**
 * @union CAP_INFO_STRUCTURE
 * @brief 採取データ情報メモリ構造体
 */
/****************************************************************/
enum {
	CAP_INFO_NUM = 256,		// 採取データ情報数
};

typedef struct
{
	uint32_t BASE;		// (SDRAMベースアドレス / 4)

	union
	{
		uint32_t LWORD;
		struct
		{
			uint32_t TIMSTMP :31;		// 採取開始時のタイムスタンプ(10ns単位)
			uint32_t WP :1;		// 採取設定、0:採取許可, 1:採取禁止
		} BIT;
	} SET;

	union
	{
		uint32_t LWORD;
		struct
		{
			uint32_t PROC_NO:16;		// 媒体ID、処理番号に使用する
			uint32_t ENC_CNT:16;		// 採取開始時のエンコーダカウント(ENC_CNT)
		} BIT;
	} BLID;

	union
	{
		uint32_t LWORD;
		struct
		{
			uint32_t BLK :10;		// 採取ブロック数 - 1
			uint32_t :1;
			uint32_t IH :1;		// 再スタート禁止期間に次媒体検知
			uint32_t CP :4;		// 静電容量センサ採取状態、0:停止, 1:開始待ち, 3:採取中, 7:正常終了, D:異常(SDRAMデータ欠落), E:異常(センサースタート無効), F:異常(サブブロックパルスエラー)
			uint32_t CIS :4;	// CIS採取状態
			uint32_t TC :4;		// 厚みセンサ採取状態
			uint32_t MG :4;		// 磁気センサ採取状態
			uint32_t SP :4;		// 特殊センサ採取状態
		} BIT;
	} CAPST;
} CAP_INFO_STRUCTURE;
typedef CAP_INFO_STRUCTURE CAP_INFO_ARRAY_T[CAP_INFO_NUM];		// 採取データ情報構造体配列
#define CAP_INFO_ARRAY (*(volatile CAP_INFO_ARRAY_T *)(CAP_INFO_BASE))		// 採取データ情報配列, 19/05/23

/****************************************************************/
/**
 * @union FPGA_CISSEQ_REG_WORD0_UNION
 * @brief FPGA CISシーケンスレジスタ ワード０
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 TREP :13;							/* 読取周期（10ns単位） */
		u32  :1;
		u32 COUNT :1;							/* 0:次のエンコーダパルスまで停止 / 1:次シーケンス自動開始 */
		u32 NXT :1;								/* 0:シーエンス終了 / 1:次シーケンス有り */
		u32 V_LED:1;							/* LED 駆動電圧 0:12V 1:24V */
		u32  :15;
	} BIT;
} FPGA_CISSEQ_REG_WORD0_UNION;

/****************************************************************/
/**
 * @union FPGA_CISSEQ_REG_WORD1_UNION
 * @brief FPGA CISシーケンスレジスタ ワード1
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 BinTh :8;	//2値化閾値
		u32 :8;			//予約(0とする)
		u32 Aadd :8;	//2値化データ格納アドレス加算分/4 (下CIS格納アドレスに対して)
		u32 :5;			//予約(0とする)
		u32 Mlt :1;		//重券検知算出対象 1:対象(下CIS)
		u32 :1;			//予約(0とする)
		u32 Bin :1;		//2値化対象 1:対象(下CIS)
	} BIT;
} FPGA_CISSEQ_REG_WORD1_UNION;

/****************************************************************/
/**
 * @union FPGA_CISSEQ_REG_WORD2_UNION
 * @brief FPGA CISシーケンスレジスタ ワード２
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32  :4;
		u32 ILED :8;							/* 上ＣＩＳ発光電DAC 設定値 */
		u32 LED :4;								/* 上LED光源番号 */
		u32 T_LED :13;							/* 上ＣＩＳ点灯時間（10ns単位） */
		u32  :1;
		u32 S_LED :2;							/* 上ＣＩＳ発光電流測定抵抗選択 */
	} BIT;
} FPGA_CISSEQ_REG_WORD2_UNION;

/****************************************************************/
/**
 * @union FPGA_CISSEQ_REG_WORD3_UNION
 * @brief FPGA CISシーケンスレジスタ ワード３
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32  :4;
		u32 ILED :8;							/* 下ＣＩＳ発光電DAC 設定値 */
		u32 LED :4;								/* 下LED光源番号 */
		u32 T_LED :13;							/* 下ＣＩＳ点灯時間（10ns単位） */
		u32  :1;
		u32 S_LED :2;							/* 下ＣＩＳ発光電流測定抵抗選択 */
	} BIT;
} FPGA_CISSEQ_REG_WORD3_UNION;

/****************************************************************/
/**
 * @union FPGA_CISSEQ_REG_WORD4_UNION
 * @brief FPGA CISシーケンスレジスタ ワード４
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 BKADRS :8;							/* 上ＣＩＳ黒補正メモリ先頭アドレス/32 */
		u32 WHADRS :8;							/* 上ＣＩＳ黒補正メモリ先頭アドレス/32 */
		u32 AINC :16;							/* 上ＣＩＳ格納アドレス増分（周期）/4 */
	} BIT;
} FPGA_CISSEQ_REG_WORD4_UNION;

/****************************************************************/
/**
 * @union FPGA_CISSEQ_REG_WORD5_UNION
 * @brief FPGAシーケンスレジスタ ワード５
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 AOFS :30;							/* 上ＣＩＳ格納アドレスオフセット/4 */
		u32  :1;
		u32 DMY :1;								/* 上ＣＩＳシーケンス種別 0:通常シーケンス / 1: ダミーシーケンス（メモリ格納無し）*/
	} BIT;
} FPGA_CISSEQ_REG_WORD5_UNION;

/****************************************************************/
/**
 * @union FPGA_CISSEQ_REG_WORD6_UNION
 * @brief FPGA CISシーケンスレジスタ ワード６
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 BKADRS :8;							/* 下ＣＩＳ黒補正メモリ先頭アドレス/32 */
		u32 WHADRS :8;							/* 下ＣＩＳ黒補正メモリ先頭アドレス/32 */
		u32 AINC :16;							/* 下ＣＩＳ格納アドレス増分（周期）/4 */
	} BIT;
} FPGA_CISSEQ_REG_WORD6_UNION;

/****************************************************************/
/**
 * @union FPGA_CISSEQ_REG_WORD7_UNION
 * @brief FPGA CISシーケンスレジスタ ワード７
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 AOFS :30;							/* 上ＣＩＳ格納アドレスオフセット/4 */
		u32  :1;
		u32 DMY :1;								/* 上ＣＩＳシーケンス種別 0:通常シーケンス / 1: ダミーシーケンス（メモリ格納無し）*/
	} BIT;
} FPGA_CISSEQ_REG_WORD7_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD0_UNION
 * @brief FPGA SSシーケンスレジスタ ワード0
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32  :15;
		u32 TCVLD :1;							/* 厚みセンサー動作 0:無効 1:有効 */
		u32 TCINC :16;							/* 厚みセンサ格納アドレス周期/4 */
	} BIT;
} FPGA_SSSEQ_REG_WORD0_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD1_UNION
 * @brief FPGA SSシーケンスレジスタ ワード1
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 TCOFS :30;							/* 厚みセンサ格納アドレスオフセット/4 */
		u32  :2;
	} BIT;
} FPGA_SSSEQ_REG_WORD1_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD2_UNION
 * @brief FPGA SSシーケンスレジスタ ワード2
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32  :15;
		u32 MGVLD :1;							/* 磁気センサ動作 0:無効 1:有効 */
		u32 MGINC :16;							/* 磁気センサ格納アドレス周期/4 */
	} BIT;
} FPGA_SSSEQ_REG_WORD2_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD3_UNION
 * @brief FPGA SSシーケンスレジスタ ワード3
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 MGOFS :30;							/* 磁気センサ格納アドレスオフセット/4 */
		u32  :2;
	} BIT;
} FPGA_SSSEQ_REG_WORD3_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD4_UNION
 * @brief FPGA SSシーケンスレジスタ ワード4
 */
/****************************************************************/
typedef union 
{
	u32 LWORD;
} FPGA_SSSEQ_REG_WORD4_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD5_UNION
 * @brief FPGA SSシーケンスレジスタ ワード5
 */
/****************************************************************/
typedef union 
{
	u32 LWORD;
} FPGA_SSSEQ_REG_WORD5_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD6_UNION
 * @brief FPGA SSシーケンスレジスタ ワード6
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_SSSEQ_REG_WORD6_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD7_UNION
 * @brief FPGA SSシーケンスレジスタ ワード7
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_SSSEQ_REG_WORD7_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD8_UNION
 * @brief FPGA SSシーケンスレジスタ ワード8
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 RESERVED :15;
		u32 CPVLD :1;							/* 静電容量センサ動作 0:無効 1:有効 */
		u32 CPINC :16;							/* 静電容量センサ格納アドレス周期/4 */
	} BIT;
} FPGA_SSSEQ_REG_WORD8_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD9_UNION
 * @brief FPGA SSシーケンスレジスタ ワード9
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 CPOFS :30;							/* 静電容量センサ格納アドレスオフセット/4 */
		u32 RESERVED :2;
	} BIT;
} FPGA_SSSEQ_REG_WORD9_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD10_UNION
 * @brief FPGA SSシーケンスレジスタ ワード10
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 RESERVED :15;
		u32 UVId :1;		// UV(蛍光）センサー動作 0:無効 1:有効
		u32 UVAInc :16;		// UV(蛍光）格納アドレス増分（周期）/4
	} BIT;
} FPGA_SSSEQ_REG_WORD10_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD11_UNION
 * @brief FPGA SSシーケンスレジスタ ワード11
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 UVAOfs :30;		// UV(蛍光）格納アドレスオフセット/4
		u32 RESERVED :2;
	} BIT;
} FPGA_SSSEQ_REG_WORD11_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD12_UNION
 * @brief FPGA SSシーケンスレジスタ ワード12
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 UVDLOn :16;		// UV(蛍光）点灯タイミング（10ns単位）
		u32 UVTLOn :16;		// UV(蛍光）点灯時間（10ns単位）
	} BIT;
} FPGA_SSSEQ_REG_WORD12_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD13_UNION
 * @brief FPGA SSシーケンスレジスタ ワード13
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 UVTAD1 :16;		// UV(蛍光）AD変換1回目タイミング（10ns単位）
		u32 UVTAD2 :16;		// UV(蛍光）AD変換2回目タイミング（10ns単位）
	} BIT;
} FPGA_SSSEQ_REG_WORD13_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD14_UNION
 * @brief FPGA SSシーケンスレジスタ ワード14
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_SSSEQ_REG_WORD14_UNION;

/****************************************************************/
/**
 * @union FPGA_SSSEQ_REG_WORD15_UNION
 * @brief FPGA SSシーケンスレジスタ ワード15
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_SSSEQ_REG_WORD15_UNION;

/****************************************************************/
/**
 * @struct BV_CISSEQ
 * @brief BV CISシーケンス
 */
/****************************************************************/
enum {
	BV_CISSEQ_NUM = 64,		// 識別センサシーケンス数(CIS), 19/12/10
	BV_SSSEQ_NUM = 64,		// 識別センサシーケンス数(CIS以外), 19/12/10
};
typedef struct
{
	FPGA_CISSEQ_REG_WORD0_UNION WORD0;
	FPGA_CISSEQ_REG_WORD1_UNION WORD1;
	FPGA_CISSEQ_REG_WORD2_UNION WORD2;
	FPGA_CISSEQ_REG_WORD3_UNION WORD3;
	FPGA_CISSEQ_REG_WORD4_UNION WORD4;
	FPGA_CISSEQ_REG_WORD5_UNION WORD5;
	FPGA_CISSEQ_REG_WORD6_UNION WORD6;
	FPGA_CISSEQ_REG_WORD7_UNION WORD7;
} BV_CISSEQ_STRUCT;
typedef struct
{
	BV_CISSEQ_STRUCT BV_CISSEQ_ARRAY_T[BV_CISSEQ_NUM];		// 識別センサシーケンス構造体配列(CIS), 19/05/23
} BV_CISSEQ_ARRAY_ST;
typedef BV_CISSEQ_STRUCT BV_CISSEQ_ARRAY_T[BV_CISSEQ_NUM];		// 識別センサシーケンス構造体配列(CIS), 19/12/10


typedef struct
{
	FPGA_SSSEQ_REG_WORD0_UNION WORD0;
	FPGA_SSSEQ_REG_WORD1_UNION WORD1;
	FPGA_SSSEQ_REG_WORD2_UNION WORD2;
	FPGA_SSSEQ_REG_WORD3_UNION WORD3;
	FPGA_SSSEQ_REG_WORD4_UNION WORD4;
	FPGA_SSSEQ_REG_WORD5_UNION WORD5;
	FPGA_SSSEQ_REG_WORD6_UNION WORD6;
	FPGA_SSSEQ_REG_WORD7_UNION WORD7;
	FPGA_SSSEQ_REG_WORD8_UNION WORD8;
	FPGA_SSSEQ_REG_WORD9_UNION WORD9;
	FPGA_SSSEQ_REG_WORD10_UNION WORD10;
	FPGA_SSSEQ_REG_WORD11_UNION WORD11;
	FPGA_SSSEQ_REG_WORD12_UNION WORD12;
	FPGA_SSSEQ_REG_WORD13_UNION WORD13;
	FPGA_SSSEQ_REG_WORD14_UNION WORD14;
	FPGA_SSSEQ_REG_WORD15_UNION WORD15;
} BV_SSSEQ_STRUCT;
typedef struct
{
	BV_SSSEQ_STRUCT BV_SSSEQ_ARRAY_T[BV_SSSEQ_NUM];			// 識別センサシーケンス構造体配列(SS)
} BV_SSSEQ_ARRAY_ST;
typedef BV_CISSEQ_STRUCT BV_CISSEQ_ARRAY_T[BV_SSSEQ_NUM];	// 識別センサシーケンス構造体配列(SS)


/****************************************************************/
/**
 * @struct FPGA_REG_CISEDG_UNION
 * @brief 媒体先頭ライン
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 PIX0 :10;	/* 画素(n*2) 媒体先頭ライン */
		u32 :6;
		u32 PIX1 :10;	/* 画素(n*2+1) 媒体先頭ライン */
		u32 :6;
	} BIT;
} FPGA_REG_CISEDG_UNION;
/****************************************************************/
/**
 * @struct BV_CISEDG_ARRAY_ST
 * @brief CIS 2値化エッジ情報メモリー 割り当て
 */
/****************************************************************/
typedef struct
{
	FPGA_REG_CISEDG_UNION	PIX[360];
} BV_CISEDG_ARRAY_ST;

/****************************************************************/
/**
 * @struct FPGA_REG_VRCH_UNION
 * @brief 積算値: ライン 仮想チャネル
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 VRCH :16;	/* 積算値: ライン0 仮想チャネル */
		u32 :16;
	} BIT;
} FPGA_REG_VRCH_UNION;
typedef struct
{
	FPGA_REG_VRCH_UNION VRCH[8];
} BV_VRLINE_ARRAY_ST;
/****************************************************************/
/**
 * @struct FPGA_REG_VRPIXMIN_UNION
 * @brief 仮想チャネル0 画素番号下限
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 PIXMIN :10;	/* 仮想チャネル0 画素番号下限 */
		u32 :22;
	} BIT;
} FPGA_REG_VRPIXMIN_UNION;
/****************************************************************/
/**
 * @struct FPGA_REG_VRPIXMAX_UNION
 * @brief 仮想チャネル0 画素番号上限
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 PIXMAX :10;	/* 仮想チャネル0 画素番号上限 */
		u32 :22;
	} BIT;
} FPGA_REG_VRPIXMAX_UNION;
/****************************************************************/
/**
 * @struct FPGA_REG_VRTHR_UNION
 * @brief 仮想チャネル0 積算値閾値
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
	struct
	{
		u32 PIXTHR :16;	/* 仮想チャネル0 積算値閾値 */
		u32 :16;
	} BIT;
} FPGA_REG_VRTHR_UNION;
typedef struct
{
	FPGA_REG_VRPIXMIN_UNION VRPIXMIN;
	FPGA_REG_VRPIXMAX_UNION VRPIXMAX;
	FPGA_REG_VRTHR_UNION VRTHR;
	u32 RESERVED;
} BV_VRSET_ARRAY_ST;
/****************************************************************/
/**
 * @struct BV_CISMLT_VRSET_ST
 * @brief CIS重券検出情報・設定メモリー 割り当て
 */
/****************************************************************/
typedef struct
{
	BV_VRLINE_ARRAY_ST VRLINE[32];
	u32 RESERVED[224];
	BV_VRSET_ARRAY_ST VRCHSET[8];
} BV_CISMLT_ARRAY_ST;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_DUTY_INIT_UNION
 * @brief FPGA DUTY初期値
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_DUTY_INIT_UNION;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_SPD_OB_UNION
 * @brief FPGA 速度目標
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_SPD_OB_UNION;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_DGAIN_UNION
 * @brief FPGA 微分(D)ゲイン
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_DGAIN_UNION;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_PGAIN_UNION
 * @brief FPGA 積分(P)ゲイン
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_PGAIN_UNION;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_IDC_UNION
 * @brief FPGA I減衰係数(1-平滑化係数)
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_IDC_UNION;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_ISM_UNION
 * @brief FPGA I(対数移動平均)平滑化係数。固定小数(1.0を512で表現)。
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_ISM_UNION;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_IGAIN_UNION
 * @brief FPGA I(積分)ゲイン。固定小数(1.0を512で表現)。
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_IGAIN_UNION;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_FB_LL_UNION
 * @brief DUTY下限。
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_FB_LL_UNION;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_FB_UL_UNION
 * @brief DUTY上限。下限以上に設定すること。
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_FB_UL_UNION;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_UD_FRQ_UNION
 * @brief 更新頻度-1。0で毎回(エンコーダー立上りごとに)更新。
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_UD_FRQ_UNION;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_DUTY_LL_UNION
 * @brief DUTY下限。
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_DUTY_LL_UNION;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_CNT2NXT_UNION
 * @brief パラメーターセット切り替えカウント。0:切り替え無し。1以上:本セット内でのエンコーダーパルス回数で次のメモリーセットに切り替わる。
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_CNT2NXT_UNION;

/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_DUTY_OF_UNION
 * @brief DUTYオフセット。
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_DUTY_OF_UNION;
/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_DUMY_UNION
 * @brief ダミー
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_DUMY_UNION;
/****************************************************************/
/**
 * @union FPGA_MOTPRM_REG_DUTY_UL_UNION
 * @brief FPGA DUTY上限。64～2047の範囲で設定する。
 */
/****************************************************************/
typedef union
{
	u32 LWORD;
} FPGA_MOTPRM_REG_DUTY_UL_UNION;
/****************************************************************/
/**
 * @struct BV_MOTPRM
 * @brief BV モーター制御パラメーター
 */
/****************************************************************/
typedef struct
{
	FPGA_MOTPRM_REG_DUTY_INIT_UNION DUTY_INIT;
	FPGA_MOTPRM_REG_SPD_OB_UNION SPD_OB;
	FPGA_MOTPRM_REG_DGAIN_UNION DGAIN;
	FPGA_MOTPRM_REG_PGAIN_UNION PGAIN;
	FPGA_MOTPRM_REG_IDC_UNION IDC;
	FPGA_MOTPRM_REG_ISM_UNION ISM;
	FPGA_MOTPRM_REG_IGAIN_UNION IGAIN;
	FPGA_MOTPRM_REG_FB_LL_UNION FB_LL;
	FPGA_MOTPRM_REG_FB_UL_UNION FB_UL;
	FPGA_MOTPRM_REG_UD_FRQ_UNION UD_FRQ;
	FPGA_MOTPRM_REG_DUTY_LL_UNION DUTY_LL;
	FPGA_MOTPRM_REG_DUTY_UL_UNION DUTY_UL;
	FPGA_MOTPRM_REG_CNT2NXT_UNION CNT2NXT;
	FPGA_MOTPRM_REG_DUTY_OF_UNION DUTY_OF;
} BV_MOTPRM_STRUCT;

#define MOTPRM_REG (*(volatile BV_MOTPRM_STRUCT*)FPGA_ADDR_MOTPRM)


typedef union
{
	u32 LWORD;
	struct
	{
		u32 DATA0 :8;
		u32 DATA1 :8;
		u32 DATA2 :8;
		u32 DATA3 :8;
	} BIT;
} FPGA_REG_LWORD_UNION;

typedef struct _AFE_REGISTER
{
	u8	ADDRESS;
	u8	BANKCNTRL;
	u8	CNTRL;
	u8	SHG0;

	u8	OFFDAC00;
	u8	OFFDAC01;
	u8	OFFDAC02;
	u8	OFFDAC03;

	u8	DGAIN00;
	u8	DGAIN01;
	u8	DGAIN02;
	u8	DGAIN03;

	u8	OFFSET00_MSB;
	u8	OFFSET00_LSB;
	u8	OFFSET01_MSB;
	u8	OFFSET01_LSB;

	u8	OFFSET02_MSB;
	u8	OFFSET02_LSB;
	u8	OFFSET03_MSB;
	u8	OFFSET03_LSB;

	u8	OUTCTRL1;
	u8	OUTCTRL2;
	u8	OUTCTRL3;
	u8	OUTCTRL4;

	u8	TRIG;
	u8	TGOUT1;
	u8	TGOUT2;
	u8	TGOUT3;

	u8	PCISCKR;
	u8	PCISCKF;
	u8	PSHDF;
	u8	LEDCTRL;

	u8	COUNT0_MSB;
	u8	COUNT0_LSB;
	u8	COUNT1_MSB;
	u8	COUNT1_LSB;

	u8	COUNT2_MSB;
	u8	COUNT2_LSB;
	u8	RESERVE_25;
	u8	RESERVE_26;

	u8	RESERVE_27;
	u8	RESERVE_28;
	u8	LEDEN_RRISE1_MSB;
	u8	LEDEN_RRISE1_LSB;

	u8	LEDEN_RFALL1_MSB;
	u8	LEDEN_RFALL1_LSB;
	u8	LEDEN_RRISE2_MSB;
	u8	LEDEN_RRISE2_LSB;

	u8	LEDEN_RFALL2_MSB;
	u8	LEDEN_RFALL2_LSB;
	u8	LEDEN_RRISE3_MSB;
	u8	LEDEN_RRISE3_LSB;

	u8	LEDEN_RFALL3_MSB;
	u8	LEDEN_RFALL3_LSB;
	u8	LEDEN_RRISE4_MSB;
	u8	LEDEN_RRISE4_LSB;

	u8	LEDEN_RFALL4_MSB;
	u8	LEDEN_RFALL4_LSB;
	u8	LEDEN_GRISE1_MSB;
	u8	LEDEN_GRISE1_LSB;

	u8	LEDEN_GFALL1_MSB;
	u8	LEDEN_GFALL1_LSB;
	u8	LEDEN_GRISE2_MSB;
	u8	LEDEN_GRISE2_LSB;

	u8	LEDEN_GFALL2_MSB;
	u8	LEDEN_GFALL2_LSB;
	u8	LEDEN_GRISE3_MSB;
	u8	LEDEN_GRISE3_LSB;

	u8	LEDEN_GFALL3_MSB;
	u8	LEDEN_GFALL3_LSB;
	u8	LEDEN_GRISE4_MSB;
	u8	LEDEN_GRISE4_LSB;

	u8	LEDEN_GFALL4_MSB;
	u8	LEDEN_GFALL4_LSB;
	u8	LEDEN_BRISE1_MSB;
	u8	LEDEN_BRISE1_LSB;

	u8	LEDEN_BFALL1_MSB;
	u8	LEDEN_BFALL1_LSB;
	u8	LEDEN_BRISE2_MSB;
	u8	LEDEN_BRISE2_LSB;

	u8	LEDEN_BFALL2_MSB;
	u8	LEDEN_BFALL2_LSB;
	u8	LEDEN_BRISE3_MSB;
	u8	LEDEN_BRISE3_LSB;

	u8	LEDEN_BFALL3_MSB;
	u8	LEDEN_BFALL3_LSB;
	u8	LEDEN_BRISE4_MSB;
	u8	LEDEN_BRISE4_LSB;

	u8	LEDEN_BFALL4_MSB;
	u8	LEDEN_BFALL4_LSB;
	u8	SP2START_MSB;
	u8	SP2START_LSB;

	u8	SP2HWIDTH_MSB;
	u8	SP2HWIDTH_LSB;
	u8	SP2PERIOD_MSB;
	u8	SP2PERIOD_LSB;

	u8	SP2COUNT;
	u8	BOSRISE0_MSB;
	u8	BOSRISE0_LSB;
	u8	BOSFALL0_MSB;

	u8	BOSFALL0_LSB;
	u8	ENRISE_MSB;
	u8	ENRISE_LSB;
	u8	ENFALL_MSB;

	u8	ENFALL_LSB;
	u8	TRIGWIDTH_MSB;
	u8	TRIGWIDTH_LSB;
	u8	CISCK_MASK_START_MSB;

	u8	CISCK_MASK_START_LSB;
	u8	CISCK_MASK_END_MSB;
	u8	CISCK_MASK_END_LSB;
	u8	ISELR;

	u8	ISELG;
	u8	ISELB;
	u8	RESERVE_71;
	u8	ADCK_CONTROL;

	u8	RESERVE_73;
	u8	RESERVE_74;
	u8	RESERVE_75;
	u8	RESERVE_76;

	u8	ADCK_PH;
	u8	RESERVE_78;
	u8	RESERVE_79;
	u8	RESERVE_7A;

	u8	RESERVE_7B;
	u8	RESERVE_7C;
	u8	RESERVE_7D;
	u8	RESERVE_7E;

	u8	RESERVE_7F;
} AFE_REGISTER;


/****************************************************************/
/*						FPGA IRQ番号							*/
/****************************************************************/
#define FPGA_IRQ0				(72)
#define FPGA_IRQ1				(73)
#define FPGA_IRQ2				(74)
#define FPGA_IRQ3				(75)
#define FPGA_IRQ4				(76)
#define FPGA_IRQ5				(77)
#define FPGA_IRQ6				(78)
#define FPGA_IRQ7				(79)
#define FPGA_IRQ8				(80)
#define FPGA_IRQ9				(81)
#define FPGA_IRQ10				(82)
#define FPGA_IRQ11				(83)
#define FPGA_IRQ12				(84)
#define FPGA_IRQ13				(85)
#define FPGA_IRQ14				(86)
#define FPGA_IRQ15				(87)
#define FPGA_IRQ16				(88)
#define FPGA_IRQ17				(89)
#define FPGA_IRQ18				(90)
#define FPGA_IRQ19				(91)
#define FPGA_IRQ20				(92)
#define FPGA_IRQ21				(93)
#define FPGA_IRQ22				(94)
#define FPGA_IRQ23				(95)
#define FPGA_IRQ24				(96)
#define FPGA_IRQ25				(97)
#define FPGA_IRQ26				(98)
#define FPGA_IRQ27				(99)
#define FPGA_IRQ28				(100)
#define FPGA_IRQ29				(101)
#define FPGA_IRQ30				(102)
#define FPGA_IRQ31				(103)
#define FPGA_IRQ32				(104)
#define FPGA_IRQ33				(105)
#define FPGA_IRQ34				(106)
#define FPGA_IRQ35				(107)
#define FPGA_IRQ36				(108)
#define FPGA_IRQ37				(109)
#define FPGA_IRQ38				(110)
#define FPGA_IRQ39				(111)
#define FPGA_IRQ40				(112)
#define FPGA_IRQ41				(113)
#define FPGA_IRQ42				(114)
#define FPGA_IRQ43				(115)
#define FPGA_IRQ44				(116)
#define FPGA_IRQ45				(117)
#define FPGA_IRQ46				(118)
#define FPGA_IRQ47				(119)
#define FPGA_IRQ48				(120)
#define FPGA_IRQ49				(121)
#define FPGA_IRQ50				(122)
#define FPGA_IRQ51				(123)
#define FPGA_IRQ52				(124)
#define FPGA_IRQ53				(125)
#define FPGA_IRQ54				(126)
#define FPGA_IRQ55				(127)
#define FPGA_IRQ56				(128)
#define FPGA_IRQ57				(129)
#define FPGA_IRQ58				(130)
#define FPGA_IRQ59				(131)
#define FPGA_IRQ60				(132)
#define FPGA_IRQ61				(133)
#define FPGA_IRQ62				(134)
#define FPGA_IRQ63				(135)			/* リファレンスクロック制御用I2C */

#endif /* SRC_INCLUDE_FPGA_H_ */
