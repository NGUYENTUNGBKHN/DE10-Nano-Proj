
#ifndef __DEBUG_H_INCLUDED__
#define __DEBUG_H_INCLUDED__

#define _DEBUG_SYSTEM_ERROR 1

#define FPGA_LOG_ENABLE 1
#define HAL_STATUS_ENABLE 1


#define FPGA_LOG_SIZE 0x40000							// 256Kバイト
#define DEBUG_VALIDATION_RESULT		1

// 20210721 ES基板 待機時FPGAのクロック供給停止
// 有効にすると
// ・待機中のFPGAメモリの参照可能
// ・ただし、消費電力増大
// 20220303 WS基板 新FPGAではクロック止めると基板温度が上がるので常時供給に変更
//#define _DEBUG_FPGA_CLOCK_NOT_STOP	1	//ディフォルト化
#define BANKNOTE_EDGE_SKEW_ENABLE	1
#define BANKNOTE_EDGE_LENGTH_ENABLE	1
#define BANKNOTE_MLT_PAPER_ENABLE	0
#define BANKNOTE_CYCLIC_ENABLE		1

//2021-10-05 additional sensor enable
#define CIS_UV_ENABLE			0
#define OTHER4_ENABLE			0
#define OTHER5_ENABLE			0
#define OTHER6_ENABLE			0
#define POSITION_ENABLE			1

#define POINT_UV1_ENABLE		1
#define MAG1_ENABLE				1
#if POINT_UV1_ENABLE
	#define POINT_UV2_ENABLE	1
#else
	#define POINT_UV2_ENABLE	0
#endif		


//#define DEBUG_DISABLE_UV_CHECK 1	// 20220407 デバッグ イニシャルFPGA時のUVセンサー接続確認スキップ ディフォルト化
#define _DEBUG_I2C_CLK_100KHZ 1
#define _DEBUG_I2C_CLK_400KHZ 0
#define _DEBUG_CIS_MULTI_IMAGE	1	// 20211019 ES基板 並列動作②複数画像スタック

/*---------------------------------------------------------------------------*/
/* For Debug */
/*---------------------------------------------------------------------------*/
//#define DATA_COLLECTION_DEBUG		1
//#define FIX_FRONT_USB_USE 1
//#define DEBUL_PL_PLL_LOCK_IRQ 1
// 温度補正無効
//#define DEBUG_ADJUSTMENT_DISABLE 1
//#define DEBUG_POS_SENSOR_ADJUSTMENT_DISABLE 1
//#define DEBUG_VALIDATION_DISABLE 1
//#define _DEBUG_FPGA_FRAM 1 //2023-06-29 2023-07-22
//2024-04-03 #define TWO_D_EXTERNED_BARCODE_TICKET 1 // 無効にしてもいいかも


#if 0 //SS用
//#define UBA_RFID_AGING 1 //テスト用 //2023-09-28
#define _DEBUG_EMI_IMAGE_CHECK 1 //2023-09-28 // CISのノイズ（黒ライン）チェック
#define _DEBUG_EMI_MAG_CHECK 1
#define _DEBUG_UART_LOOPBACK 1
#define LOOPBACK_UBA 1  //テスト用
#endif
//#define EEPROM_TEST		//2025-08-07

//UBA500の暗号化定義の対応な流れはおそらく下記
// 1 EXTEND_ENCRYPTION_PAY で16byte対応 PAYOUT16MODE
// 2 ID003_SPECK64 で PAYOUT16MODE_SPECK
//#define RTQ_FACTORY		//札長などの設定もあるので (rcLength_dt1)、生産用ソフト作成時はEURの国フォルダを使用する事

#define RTQ_ENABLE_RESET
#define USB_REAR_USE
#if defined(UBA_RTQ)

	//#define DIS_ICB
	//#define RTQ_ENABLE_RESET
	#define RC_ENCRYPTION	// 
	//#define UBA_ID003_ENC
	#define UBA_RTQ_ICB
	//#define UBA_RTQ_AZ_LOG
	//#define SKEW_TEST //テスト用なので最終的には外す

/*---------------*/
	//下記はどちらか1つを使用、両方使用はNG
	//ソフト1
	//インターバル30s
	//RFID有効
	//CISのディレイ有効でエラーは無効
	//ループバッグなし
	//#define QA_TEST_SAFE				// 製品安全評価用
/*---------------*/
	//ソフト2
	//#define QA_TEST_EMC_EMI			//MEC,EMI評価用
	//#define LOOPBACK_UBA 1  			//テスト用
	//#define _DEBUG_UART_LOOPBACK 1
/*---------------*/
	// QAエージングソフト
	// QA_TEST_SAFE を使用してRFID書き込み有効
	// インターバルは15s
	// CISの温度によるディレイ処理無効
	//#define QA_TEST_AZ			// QA内でのエージングテスト用
/*---------------*/
#endif

#endif /*__DEBUG_H_INCLUDED__*/
