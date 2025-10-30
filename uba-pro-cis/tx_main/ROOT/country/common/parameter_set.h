// parameter_set.h
#ifndef	_PARASET_H
#define	_PARASET_H

/*ファイル情報*/
#define FILE_FORMAT_VERSION		0x00000005	//ファイルフォーマットバージョン
#define TRANS_CORRECT_PITCH	1.0f												// ピッチ
#define SPEC_CODE					0x0100												// 仕様コード(仕様コード一覧参照)
#define BLOCK_SIZE 				SUB_BLOCK_NUM  										// 1ブロックあたりのサブブロック数
#define SUB_BLOCK_DPI			200													// サブブロックのdpi

/*固定プレーン情報*/
#define SUB_PITCH_200DPI			1			// 副走査素子ピッチ
#define SUB_PITCH_100DPI			2			// 副走査素子ピッチ
#define SUB_PITCH_50DPI			4			// 副走査素子ピッチ
#define SUB_PITCH_25DPI			8			// 副走査素子ピッチ
#define PERIOD_BASE				(BLOCK_BYTE_SIZE / SUB_BLOCK_NUM)			// 副走査基本周期、(55872 / 8) = 6984, 19/05/24
#define PERIOD_200DPI				(PERIOD_BASE * SUB_PITCH_200DPI)			// 200dpiのピリオド、(6984 * 1) = 6984, 19/05/24
#define PERIOD_100DPI				(PERIOD_BASE * SUB_PITCH_100DPI)			// 100dpiのピリオド、(6984 * 2) = 13968, 19/05/24
#define PERIOD_50DPI				(PERIOD_BASE * SUB_PITCH_50DPI)				// 50dpiのピリオド、(6984 * 4) = 27936, 19/05/24
#define PERIOD_25DPI				(PERIOD_BASE * SUB_PITCH_25DPI)				// 25dpiのピリオド、(6984 * 8) = 55872, 19/05/24

/*CIS関係ステータス*/
#define PLANE_MAIN_OFFSET		728
#define CISA_MAIN_OFFSET		PLANE_MAIN_OFFSET			// CISA主走査オフセット量
#define CISB_MAIN_OFFSET		PLANE_MAIN_OFFSET			// CISB主走査オフセット量
#define CISA_R_R_SUB_OFFSET	0			// CISA反射赤の副走査オフセット, 19/12/03
#define CISB_R_R_SUB_OFFSET		0			// CISB反射赤の副走査オフセット, 19/12/03
#define CISA_R_G_SUB_OFFSET	0			// CISA反射緑の副走査オフセット, 19/12/03
#define CISB_R_G_SUB_OFFSET	0			// CISB反射緑の副走査オフセット, 19/12/03
#define CISA_R_B_SUB_OFFSET	0			// CISA反射青の副走査オフセット, 19/12/03
#define CISB_R_B_SUB_OFFSET	0			// CISB反射青の副走査オフセット, 19/12/03
#define CISA_R_IR1_SUB_OFFSET	0			// CISA反射赤外1(810nm)の副走査オフセット, 19/12/03
#define CISB_R_IR1_SUB_OFFSET	0			// CISB反射赤外1(810nm)の副走査オフセット, 19/12/03
#define CISA_R_IR2_SUB_OFFSET	0			// CISA反射赤外2(940nm)の副走査オフセット, 19/12/03
#define CISB_R_IR2_SUB_OFFSET	0			// CISB反射赤外2(940nm)の副走査オフセット, 19/12/03
#define CISA_R_FL_SUB_OFFSET	0			// CISA反射蛍光の副走査オフセット, 19/12/03
#define CISB_R_FL_SUB_OFFSET	0			// CISB反射蛍光の副走査オフセット, 19/12/03
#define CISB_T_R_SUB_OFFSET		0			// CISB透過赤の副走査オフセット
#define CISB_T_G_SUB_OFFSET	0			// CISB透過緑の副走査オフセット
#define CISB_T_IR1_SUB_OFFSET	0			// CISB透過赤外1(810nm)の副走査オフセット
#define CISB_T_IR2_SUB_OFFSET	0			// CISB透過赤外2(810nm)の副走査オフセット
#define CISA_AREA_MAX			(695+8)		// CISA有効範囲　大(数値指定703をシンボルに変更, 20/04/24)
#define CISA_AREA_MIN			(26+8)		// CISA有効範囲　小(数値指定34をシンボルに変更, 20/04/24)
#define CISB_AREA_MAX			(695+8)		// CISB有効範囲　大(数値指定703をシンボルに変更, 20/04/24)
#define CISB_AREA_MIN			(26+8)		// CISB有効範囲　小(数値指定34をシンボルに変更, 20/04/24)
#define CIS_MAIN_PITCH			0.127f		// 主走査素子ピッチ
#define CIS_MAIN_ALL_PIXS		(728)		// 主走査最大画素数
#define CIS_DATA_TYPE				8			// データ長
#define CISA_R_R_OFFSET			(0)			// CISA反射赤のオフセット, 19/05/13
#define CISB_R_R_OFFSET			(728)		// CISB反射赤のオフセット, 19/05/13
#define CISA_R_IR1_OFFSET		(1456)		// CISA反射赤外1(810nm)のオフセット, 19/05/13
#define CISB_R_IR1_OFFSET			(2184)		// CISB反射赤外1(810nm)のオフセット, 19/05/13
#define CISA_T_G_OFFSET			(2912)		// CISA透過緑のオフセット, 19/05/13 not used
#define CISB_T_G_OFFSET			(3640)		// CISB透過緑のオフセット, 19/05/13
#define CISA_R_G_OFFSET			(4368)		// CISA反射緑のオフセット, 19/05/13
#define CISB_R_G_OFFSET			(5096)		// CISB反射緑のオフセット, 19/05/13
#define CISA_R_IR2_OFFSET		(5824)		// CISA反射赤外2(940nm)のオフセット, 19/05/13
#define CISB_R_IR2_OFFSET			(6552)		// CISB反射赤外2(940nm)のオフセット, 19/05/13
#define CISA_R_B_OFFSET			(7280)		// CISA反射青のオフセット, 19/05/13
#define CISB_R_B_OFFSET			(8008)		// CISB反射青のオフセット, 19/05/13
#define CISA_T_R_OFFSET			(11792)		// CISA透過赤のオフセット, 19/05/13 not used
#define CISB_T_R_OFFSET			(12520)		// CISB透過赤のオフセット, 19/05/13
#define CISA_T_IR1_OFFSET		(14704)		// CISA透過赤外1(810nm)のオフセット, 19/05/13 not used
#define CISB_T_IR1_OFFSET		(15432)		// CISB透過赤外1(810nm)のオフセット, 19/05/13
#define CISA_T_IR2_OFFSET		(29552)		// CISA透過赤外2(940nm)のオフセット, 19/05/13 19/05/13 not used
#define CISB_T_IR2_OFFSET		(30280)		// CISB透過赤外2(940nm)のオフセット, 19/05/13 19/05/13
#define CISA_R_FL_OFFSET		(32464)		// CISA反射蛍光のオフセット, 19/05/13
#define CISB_R_FL_OFFSET		(33192)		// CISB反射蛍光のオフセット, 19/05/13

#define CISA_R_UV_OFFSET		(8736)		// UP Point-UV
#define CISB_R_UV_OFFSET		(44256)		// DOWN Point-UV
#define CISA_R_MAG1_OFFSET		(8768)		// Point-MAG1
#define CISA_R_MAG2_OFFSET		(8768)		// Point-MAG2
#define CISA_R_BLANK_OFFSET		(8816)		// blank1
#define CISB_R_BLANK_OFFSET		(8848)		// blank2

/*MAG1関係ステータス*/
#define MAG1_PIXEL					200			// 磁気センサ画素数, 20/03/16
#define MAG1_DEBUG_INFO 		4			// 磁気センサデバッグ情報4ワード(8バイト), 20/03/16
#define MAG1_MAIN_OFFSET		(148)		// 主走査オフセット量
#define MAG1_SUB_OFFSET		(-40)		// 副走査オフセット量
#define MAG1_AREA_MAX			198			// 主走査有効範囲　大
#define MAG1_AREA_MIN			11			// 主走査有効範囲　小
#define MAG1_MAIN_PITCH		0.5f		// 主走査素子ピッチ
#define MAG1_MAIN_ALL_PIXS	(MAG1_DEBUG_INFO + MAG1_PIXEL)		// 主走査最大画素数204, 20/03/16
#define MAG1_DATA_TYPE			10			// データ長
#define MAG1_OFFSET				0		// 磁気センサのオフセット, 19/05/13

/*TC関係ステータス*/
#define TC1_PIXEL					1			// 厚みセンサ画素数, 20/03/16
#define TC1_DEBUG_INFO 			4			// 厚みセンサデバッグ情報4ワード(8バイト), 20/03/16
#define TC1_MAIN_OFFSET			(-3)		// 主走査オフセット量
#define TC1_SUB_OFFSET			0			// 副走査オフセット量
#define TC1_AREA_MAX				4			// 主走査有効範囲　大
#define TC1_AREA_MIN				4			// 主走査有効範囲　小
#define TC1_MAIN_PITCH			5.5f		// 主走査素子ピッチ
#define TC1_MAIN_ALL_PIXS		(TC1_DEBUG_INFO + TC1_PIXEL)		// 主走査最大画素数5, 20/03/16
#define TC1_DATA_TYPE				12			// データ長
#define TC1_OFFSET					0		// 厚みセンサデータオフセット,

#define TC2_PIXEL					1			// 厚みセンサ基準データ画素数, 20/03/16
#define TC2_DEBUG_INFO 			0			// 厚みセンサ基準データデバッグ情報0ワード(0バイト), 20/03/16
#define TC2_MAIN_OFFSET			(-3)		// 主走査オフセット量
#define TC2_SUB_OFFSET			0			// 副走査オフセット量
#define TC2_AREA_MAX				0			// 主走査有効範囲　大
#define TC2_AREA_MIN				0			// 主走査有効範囲　小
#define TC2_MAIN_PITCH			5.0f		// 主走査素子ピッチ
#define TC2_MAIN_ALL_PIXS		(TC2_DEBUG_INFO + TC2_PIXEL)		// 主走査最大画素数1, 20/03/16
#define TC2_DATA_TYPE				12			// データ長
#define TC2_OFFSET					0		// 厚みセンサ基準データオフセット, 19/05/14

/*CAP関係ステータス*/
#define CAP1_PIXEL					16			// 静電容量センサ画素数, 20/03/16
#define CAP1_DEBUG_INFO 		4			// 磁気センサデバッグ情報4ワード(8バイト), 20/03/16
#define CAP1_MAIN_OFFSET		(28)		// 主走査オフセット量
#define CAP1_SUB_OFFSET			0			// 副走査オフセット量
#define CAP1_AREA_MAX			19			// 主走査有効範囲　大
#define CAP1_AREA_MIN			4			// 主走査有効範囲　小
#define CAP1_MAIN_PITCH			5.5f		// 主走査素子ピッチ
#define CAP1_MAIN_ALL_PIXS		(CAP1_DEBUG_INFO + CAP1_PIXEL)		// 主走査最大画素数20, 20/03/16
#define CAP1_DATA_TYPE			10			// データ長
#define CAP1_OFFSET				0		// 静電容量センサオフセット

/* ----------------------------- */
/*
 * CISスキャン用生出力取得用定義
 */
/* ----------------------------- */
/*固定プレーン情報(生出力用)*/
#define PERIOD_BASE_RAW			(BLOCK_BYTE_SIZE_RAW / SUB_BLOCK_NUM)		// 副走査基本周期、(111168 / 8) = 13896, 19/12/03
#define PERIOD_200DPI_RAW		(PERIOD_BASE_RAW * SUB_PITCH_200DPI)		// 200dpiのピリオド、(13896 * 1) = 13896, 19/12/03
#define PERIOD_100DPI_RAW		(PERIOD_BASE_RAW * SUB_PITCH_100DPI)		// 100dpiのピリオド、(13896 * 2) = 27792, 19/12/03
#define PERIOD_50DPI_RAW			(PERIOD_BASE_RAW * SUB_PITCH_50DPI)			// 50dpiのピリオド、(13896 * 4) = 55584, 19/12/03
#define PERIOD_25DPI_RAW			(PERIOD_BASE_RAW * SUB_PITCH_25DPI)			// 25dpiのピリオド、(13896 * 8) = 111168, 19/12/03

/*CIS関係ステータス*/
#define CIS_MAIN_ALL_PIXS_RAW	(8 + 1440)			// 主走査最大画素数, 20/04/24
#define CISA_MAIN_OFFSET_RAW	PLANE_MAIN_OFFSET							// CISA主走査オフセット量, 20/04/24
#define CISB_MAIN_OFFSET_RAW	PLANE_MAIN_OFFSET							// CISB主走査オフセット量, 20/04/24
#define CISA_AREA_MAX_RAW		(695-16)				// CISA有効範囲　大, 20/04/24
#define CISA_AREA_MIN_RAW		(26+16)				// CISA有効範囲　小, 20/04/24
#define CISB_AREA_MAX_RAW		(965-16)				// CISB有効範囲　大, 20/04/24
#define CISB_AREA_MIN_RAW		(26+16)				// CISB有効範囲　小, 20/04/24
#define CIS_DATA_TYPE_RAW		10			// データ長, 19/12/03
#define CISA_R_R_OFFSET_RAW	0		// CISA反射赤のオフセット, 19/05/13
#define CISB_R_R_OFFSET_RAW	728		// CISB反射赤のオフセット, 19/05/13
#define CISA_R_IR1_OFFSET_RAW	1456	// CISA反射赤外1(810nm)のオフセット, 19/05/13
#define CISB_R_IR1_OFFSET_RAW	2184	// CISB反射赤外1(810nm)のオフセット, 19/05/13
#define CISA_R_G_OFFSET_RAW	2912	// CISA反射緑のオフセット, 19/05/13
#define CISB_R_G_OFFSET_RAW	3640	// CISB反射緑のオフセット, 19/05/13
#define CISA_R_B_OFFSET_RAW	4368	// CISA反射青のオフセット, 19/05/13
#define CISB_R_B_OFFSET_RAW	5096	// CISB反射青のオフセット, 19/05/13
#define CISA_R_IR2_OFFSET_RAW	5824	// CISA反射赤外2(940nm)のオフセット, 19/05/13
#define CISB_R_IR2_OFFSET_RAW	6552	// CISB反射赤外2(940nm)のオフセット, 19/05/13
#define CISB_T_G_OFFSET_RAW	7280	// CISB透過緑のオフセット, 19/05/13
#define CISB_T_R_OFFSET_RAW		15448	// CISA透過赤のオフセット, 19/05/13
#define CISB_T_IR1_OFFSET_RAW	14720	// CISA透過赤外1(810nm)のオフセット, 19/05/13
#define CISB_T_IR2_OFFSET_RAW	31784	// CISA透過赤外2(940nm)のオフセット, 19/05/13
#define CISA_R_FL_OFFSET_RAW	30328	// CISA反射蛍光のオフセット, 19/05/13
#define CISB_R_FL_OFFSET_RAW	31056	// CISB反射蛍光のオフセット, 19/05/13

/*MAG1関係ステータス*/
#define MAG1_OFFSET_RAW		0		// 磁気センサのオフセット, 19/05/13

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	void parameter_set(ST_BS *tmp_pbs);
	void parameter_set_raw_cis(ST_BS *tmp_pbs);
	//void bill_sampling_parameter_set(ST_BS *tmp_pbs, T_ADJDATA* adj);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _PARASET_H */
