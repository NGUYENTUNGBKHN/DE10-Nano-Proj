/******************************************************************************/
/*! @addtogroup Group2
    @file       id003_encryption.c
    @brief      id003 encryption process
    @date       2013/10/25
    @author     Development Dept at Tokyo
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2013/10/25 Development Dept at Tokyo
      -# Initial Version
******************************************************************************/
#include <string.h>
#include "common.h"
#include "id003_encryption.h"
#include "id003.h"

#define EXT
#include "../common/global.h"
#include "com_ram.c"


/************************** PRIVATE VARIABLES *************************/
#if 1//#if defined(ID003_SPECK64)
#define	USE_CBC							1

u32 rk[ROUNDS];
u32 Pt[2];
u32 Ct[2];
u32 Dt[2];

static void _id003_enc_speck64_init(const u8 *serial_num_ptr, u8 *cbc_ptr);
static void _id003_enc_speck64_encode(const u8 *input_ptr, u8 *output_ptr);
static void _id003_enc_speck64_decode(const u8 *input_ptr, u8 *output_ptr, u8 *cbc_ptr);
#endif

static const u8 sc_id003_enc_key_default[ID003_ENC_GC2_BLOCK_LEN] = {0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff};
static const u8 sc_id003_device_serial_no[8] = {'I','D','0','0','3','J','C','M'};

static u8 s_id003_enc_key[8];
static u8 s_id003_enc_number;
static u8 s_id003_enc_cbc[8];
static u8 s_id003_enc_cbc_work[8];

static u8 s_id003_enc_cbc_work_backup[8];
u8 id003_enc_mode;

/************************** PRIVATE FUNCTIONS *************************/
static void _id003_enc_gc2_init(const u8 *serial_num_ptr, u8 *cbc_ptr);
static void _id003_enc_gc2_encode(const u8 *input_ptr, u8 *pCBCInDataOut);
static void _id003_enc_gc2_decode(const u8 *input_ptr, u8 *pOutput, u8 *pCBC);


/*========================================================================*/
/*                                                                        */
/*      id003_enc_init()                                                  */
/*                                                                        */
/*========================================================================*/
/*  (内部変数の初期化)                                                    */
/*		キーとか内部変数の初期化                                          */
/*		                                                                  */
/*	ﾘｾｯﾄ時にcallしてください                                              */
/*========================================================================*/
void id003_enc_init(void)
{
	u8	cnt;
	
	/* clear encryption key */
	for (cnt = 0; cnt < ID003_ENC_GC2_BLOCK_LEN; cnt++)
	{
		s_id003_enc_key[cnt] = sc_id003_enc_key_default[cnt];
	}

	/* make CBC */
	//起動時の下記の処理が異なる為、SPECあり、なしは完全には共通化できない様な気がする
	//起動時は、まだモードがこの時点では定まっていない
	//ただし、実際に_id003_enc_gc2_init、_id003_enc_speck64_initは、
	//id003_enc_make_key(u8 *key) をコールする時
	//下記の暗号キーを受信時なので、
	//_id003_encryption_key_cmd_proc(void) or _id003_set_encrypted_keynumber_cmd_proc(void)
	//その時にSPECモードか確認しているので、ここでの初期化は、どちらでもいいきがするが、
	//SPEC対応時に _id003_enc_speck64_init をディフォルトにすればいいと思う
	#if 1//#if defined(ID003_SPECK64)	/* '22-12-23 */
	_id003_enc_speck64_init(sc_id003_device_serial_no, s_id003_enc_cbc);
	#else
	_id003_enc_gc2_init(sc_id003_device_serial_no, s_id003_enc_cbc);
	#endif

	#if 0 //2025-03-28 /* set secret number */// ランダム Random_Secret_no[] に変わったので、現状必要なし
	if ((((ex_fram_sum >> 8)&0xFF) == 0x00)
	 && ((ex_fram_sum&0xFF) == 0x00))
	{ /*-- secret number は0以外 --*/
		s_id003_secret_number[0] = 0x11;
		s_id003_secret_number[1] = 0x12;
	}
	else
	{
		s_id003_secret_number[0] = ((ex_fram_sum >> 8)&0xFF);
		s_id003_secret_number[1] = (ex_fram_sum&0xFF);
	}
	s_id003_secret_number[2] = (s_id003_secret_number[0] ^ s_id003_secret_number[1] ^ 0x5A);
	#endif

	/* clear encryption number (default:1) */
	s_id003_enc_number = 1;

	/* set encryption mode default 8-byte */
	id003_enc_mode = PAYOUT8MODE;
}

/*========================================================================*/
/*                                                                        */
/*      id003_enc_make_key()                                              */
/*                                                                        */
/*========================================================================*/
/*  (CBCの更新)                                                           */
/*		1.HOSTから与えられるKEY(8byte)                                    */
/*		2.デバイスシリアル番号(定数 8byte)                                */
/*	  これらを材料に CBCを計算する。                                      */
/*	  データ暗号化時は, このCBCを参照して暗号化する。                     */
/*	ENCRYPTION KEY( id003_enc_key )が更新された時は コールしてください    */
/*========================================================================*/
void id003_enc_make_key(u8 *key) //ok
{
	u8	cnt;
	for (cnt = 0; cnt < ID003_ENC_GC2_BLOCK_LEN; cnt++)
	{
		s_id003_enc_key[cnt] = *(key + cnt);
	}
	//SPEC対応、未対応 共用できる	
	if(id003_enc_mode != PAYOUT16MODE_SPECK)
	{
	//通常用
		_id003_enc_gc2_init(sc_id003_device_serial_no, s_id003_enc_cbc); //ok
	}
	else
	{
		//SPEC用
		_id003_enc_speck64_init(sc_id003_device_serial_no, s_id003_enc_cbc);
	}
}

/*========================================================================*/
/*                                                                        */
/*      id003_enc_set_number()                                            */
/*                                                                        */
/*========================================================================*/
/*  (encryption numberの設定)                                             */
/*========================================================================*/
void id003_enc_set_number(u8 number)
{
	s_id003_enc_number = number;
}

/*========================================================================*/
/*                                                                        */
/*      id003_enc_set_number()                                            */
/*                                                                        */
/*========================================================================*/
/*  (encryption numberの取得)                                             */
/*========================================================================*/
u8 id003_enc_get_number(void)
{
	return s_id003_enc_number;
}

/*========================================================================*/
/*                                                                        */
/*      ID003Enc_encode()                                                 */
/*                                                                        */
/*========================================================================*/
/* 	(データの暗号化)                                                      */
/* 	u8 *src 暗号化するデータ(平文) 	(8bytes)                              */
/*	u8 *dst 暗号化データ出力先		(8bytes)                              */
/*========================================================================*/
void id003_enc_encode(u8 *src, u8 *dst) //ok
{
	/* CBCデータをWORKへコピー */
	memcpy(s_id003_enc_cbc_work, s_id003_enc_cbc, 8);
	//SPECKと共有できる
	if(id003_enc_mode == PAYOUT16MODE_SPECK)
	{
		_id003_enc_speck64_encode(src, s_id003_enc_cbc_work);
	}
	else
	{
		/* CBCデータを材料に暗号化,暗号化データは CBC WORKへ */
		_id003_enc_gc2_encode(src, s_id003_enc_cbc_work);
	}

	/* 暗号化したデータを返す */
	memcpy(dst, s_id003_enc_cbc_work, 8);

	/*--------------------------------------------------------------*/
	/* 今回の暗号化データは,次回の暗号化の材料となる。 				*/
	/* この動きは 関数id003_enc_vendack_receive(void)内の動きを拝見 */
	/*--------------------------------------------------------------*/
}

/*========================================================================*/
/*                                                                        */
/*      id003_enc_update_context()                                        */
/*                                                                        */
/* コンテキスト更新                                                       */
/*========================================================================*/
void id003_enc_update_context(void)
{
	/* swap internal data */
	memcpy(s_id003_enc_cbc, s_id003_enc_cbc_work, 8);
}

/*========================================================================*/
/*                                                                        */
/*      id003_enc_vendack_receive()                                       */
/*                                                                        */
/* VENDｽﾃｰﾀｽに対するACKを受信したとき,この関数がコールされる              */
/*========================================================================*/
void id003_enc_vendack_receive(void)
{
	/* inclement encryption number (default:1) */
	s_id003_enc_number++;
	if( s_id003_enc_number == 0 ){	/* next to 0xff is 0x01 */
		s_id003_enc_number = 1;
	}
	/* swap internal data */
	memcpy(s_id003_enc_cbc, s_id003_enc_cbc_work, 8);

}
/*========================================================================*/
/*                                                                        */
/*      _id003_enc_gc2_init                                               */
/*                                                                        */
/*========================================================================*/
/*                                                                        */
/* Parameter:                                                             */
/*      serial_num_ptr		device serial number                          */
/*      cbc_ptr				pointer to the CBC init vector                */
/*                                                                        */
/* Returns:                                                               */
/*      void                                                              */
/*                                                                        */
/* Description:                                                           */
/*      Initializes the CBC context with the device serial number.        */
/*========================================================================*/
static void _id003_enc_gc2_init(const u8 *serial_num_ptr, u8 *cbc_ptr) //ok
{
	u8 cnt;
	u8 cnt2;
	
	for (cnt = 0; cnt < ID003_ENC_GC2_BLOCK_LEN; cnt++)
	{
		cbc_ptr[cnt] = 0;
		for(cnt2 = 0; cnt2 < ID003_ENC_GC2_BLOCK_LEN; cnt2++)
		{
			cbc_ptr[cnt] ^= serial_num_ptr[cnt2];
			cbc_ptr[cnt]  = ID003_ENC_GC2_BARREL_L(cbc_ptr[cnt]);
			cbc_ptr[cnt] += s_id003_enc_key[cnt];			/* HOSTから与えられるキー */
		}
	}
}

/*========================================================================*/
/*                                                                        */
/*      _id003_enc_gc2_encode                                             */
/*                                                                        */
/*========================================================================*/
/*                                                                        */
/* Parameter:                                                             */
/*      input_ptr		input data (plain)                                */
/*      cbc_data_ptr	at input holds the previous encryption buffer     */
/*                      at output holds the encrypted input_ptr           */
/*                                                                        */
/* Returns:                                                               */
/*      void                                                              */
/*                                                                        */
/* Description:                                                           */
/*      Encodes 8 bytes of data.                                          */
/*========================================================================*/
static void _id003_enc_gc2_encode(const u8 *input_ptr, u8 *cbc_data_ptr)
{
	u8 cnt;
	u8 cnt2;
	
	for (cnt = 0; cnt < ID003_ENC_GC2_BLOCK_LEN; cnt++)
	{
		cbc_data_ptr[cnt] = input_ptr[cnt] ^ cbc_data_ptr[cnt];
		for (cnt2 = 0; cnt2 < ID003_ENC_GC2_ROUNDS; cnt2++)
		{
			cbc_data_ptr[cnt] = ID003_ENC_GC2_BARREL_L(cbc_data_ptr[cnt]) + s_id003_enc_key[cnt];
		}
	}
}

/*========================================================================*/
/*                                                                        */
/*      id003_enc_decode()                                                */
/*                                                                        */
/*========================================================================*/
/* 	(データの復号化)                                                      */
/* 	u8 *src 復号するデータ(平文) 	(8bytes)                              */
/*	u8 *dst 復号化データ出力先		(8bytes)                              */
/*========================================================================*/
void id003_enc_decode(u8 *src, u8 *dst) //ok
{
	/* CBCデータをWORKへコピー */
	memcpy(s_id003_enc_cbc_work, s_id003_enc_cbc, 8);

	/* 暗号化メッセージを復号化する */
	//下記はSPECKと両立できる
	if(id003_enc_mode == PAYOUT16MODE_SPECK)
	{
		_id003_enc_speck64_decode(src, dst, s_id003_enc_cbc_work);
	}
	else
	{
		/* 暗号化メッセージを復号化する */
		_id003_enc_gc2_decode(src, dst, s_id003_enc_cbc_work);
	}
}


/*========================================================================*/
/*                                                                        */
/*      _id003_enc_gc2_decode                                             */
/*                                                                        */
/*========================================================================*/
/*                                                                        */
/* Parameter:                                                             */
/*      input_ptr          input data (cipher)                            */
/*      output_ptr         receives plain data                            */
/*      cbc_ptr            pointer to the CBC context.                    */
/*                                                                        */
/* Returns:                                                               */
/*      void                                                              */
/*                                                                        */
/* Description:                                                           */
/*      Decodes 8 bytes of data.                                          */
/*========================================================================*/
static void _id003_enc_gc2_decode(const u8 *input_ptr, u8 *output_ptr, u8 *cbc_ptr)
{
	u8 cnt;
	u8 cnt2;
	
	for (cnt = 0; cnt < ID003_ENC_GC2_BLOCK_LEN; cnt++)
	{
		output_ptr[cnt] = input_ptr[cnt];
		for(cnt2 = 0; cnt2 < ID003_ENC_GC2_ROUNDS; cnt2++)
		{
			output_ptr[cnt] -= s_id003_enc_key[cnt];
			output_ptr[cnt] = ID003_ENC_GC2_BARREL_R(output_ptr[cnt]);
		}
		output_ptr[cnt]  = output_ptr[cnt] ^ cbc_ptr[cnt];
		cbc_ptr[cnt] = input_ptr[cnt];
	}
}



/*========================================================================*/
/*                                                                        */
/*      id003_enc_cancel_update_context()                                 */
/*                                                                        */
/* コンテキスト更新のキャンセル                                           */
/*========================================================================*/
void id003_enc_cancel_update_context(void) //ok
{
	/* 暗号化有効時 */
	if( (ex_cline_status_tbl.option & ID003_OPTION_ENCRYPTION) == ID003_OPTION_ENCRYPTION )
	{
		memcpy( s_id003_enc_cbc, s_id003_enc_cbc_work_backup, 8 );
	}
}

u8 id003_decryptcmd(u8 *data) //ok
{
	u8 DecryptBuf[16];
	u8 i = 0;
    u8 crc1 = 0;
    u8 crc2 = 0;

    /* 次の8byteを複合化するためには、id003_enccbcに更新された、s_id003_enc_cbc_workをコピーする必要がある */
    /* ただし、今回のコマンドが正式なコマンドでない場合はs_id003_enc_cbcに更新をキャンセルする必要がある	*/
    /* キャンセルに備えて、現在のid003_enccbcの値をs_id003_enc_cbc_work_backupに保存する					*/
    memcpy(s_id003_enc_cbc_work_backup, s_id003_enc_cbc, 8);

    /* 暗号化メッセージをバックアップ */
    memcpy(&DecryptBuf[0], (u8 *)&data[3], 8);

    /* 復号化 */
    id003_enc_decode(&DecryptBuf[0], &data[3]); //ok

	if(((id003_enc_mode == PAYOUT16MODE_SPECK) && (data[1] == (0x07 + ID003_ADD_04) )) // if PAYOUT8 send and mode is for PAYOUT 16 SPECK
	|| ((id003_enc_mode == PAYOUT16MODE) && (data[1] == (0x07 + ID003_ADD_04) ))  // if PAYOUT8 send and mode is for PAYOUT 16
	|| ((id003_enc_mode == PAYOUT8MODE) && (data[1] == (0x0F + ID003_ADD_04) )))		 // if PAYOUT16 send and mode is for PAYOUT 8
	{
		return false;
	}

    if(data[1] == (0x0F + ID003_ADD_04))
    //if(data[1] == 0x0F) 	
    {
	//16byteモードのみ必要な様だ	
        /* 次の8バイトの複合化の為、id003_enccbcをid003_enccbc_workで更新                                   */
        id003_enc_update_context();

        /* 暗号化メッセージを保存 */
        memcpy(&DecryptBuf[8], (u8 *)&data[11], 8);
        /* 復号化結果をdata[11]へ */
        id003_enc_decode(&DecryptBuf[8], &data[11]); //ok

 		if(id003_enc_mode == PAYOUT16MODE_SPECK)
		{
			if(data[3] == 0xF0)	/* Encrypted Payout */
			{	
				if((data[11] != Random_Secret_no[0])
				|| (data[12] != Random_Secret_no[1])
				|| (data[13] != Random_Secret_no[2])
				|| (data[16] != s_id003_enc_number))
				{
					++ex_illigal_payout_command;
				    return false;
				}
			}
			else if(data[3] == 0x03) /* Encrypted Key */
			{
				if((data[13] != Random_Secret_no[0])
				|| (data[14] != Random_Secret_no[1])
				|| (data[15] != Random_Secret_no[2]))
				{
					++ex_illigal_payout_command;
				    return false;
				}
			}	
		}
		else
		{
			if((data[11] != Random_Secret_no[0]) 
			||(data[12] != Random_Secret_no[1]) 
			||(data[13] != Random_Secret_no[2]) 
			||(data[16] != s_id003_enc_number))
	        {
	            return false;
	        }
		}
    }

	//data[1] = data[1] + 4; //UBA700はすでに + 4 しているので必要なし 処理1
    /* 復号化したメッセージのCRC確認 */
    crc1 = 0;
    crc2 = 0;

	for(i = 0; i < data[1] - 2; i++)
	{
		crccal( data[i], &crc1, &crc2 );
	}

	if( data[1] == 0x13 ) //UBA500も上の処理で + 4している、その為UBA700ではここでは、 + 4 して考える必要なし
	{
	//16byte の場合の CRC位置が8バイト後ろなので
		i = 8;
	}
	else
	{
		i = 0;
	}

	//CRCの確認
	if( crc1 == data[9 + i] && crc2 == data[10 + i] )
	{
		/* CRC演算のために一時的に実Lenghtに変更していたものを */
		/* 元に戻す */
		//data[1] = data[1] - 4;	//UBA700は上の 処理1 で+4していないので　Length戻さない、

		//#if defined(ID003_SPECK64)
		if(id003_enc_mode == PAYOUT16MODE_SPECK)
		{
			_id003_sts_poll = 1;	
			ex_illigal_payout_command = 0;
		}
		//#endif
		return true;
	}
	else
	{
		/* コンテキストの更新をキャンセル */
		id003_enc_cancel_update_context();

		return false;
	}
}

/*========================================================================*//*========================================================================*/
/*                                                                        */
/*	SPECK					                                              */
/*                                                                        */
/*========================================================================*//*========================================================================*/
/*========================================================================*/
/*                                                                        */
/*	_id003_enc_speck64_Key128                                             */
/*                                                                        */
/*========================================================================*/
/*                                                                        */
/* Parameter:                                                             */
/*      input_ptr		input data (plain)                                */
/*      cbc_data_ptr	at input holds the previous encryption buffer     */
/*                      at output holds the encrypted input_ptr           */
/*                                                                        */
/* Returns:                                                               */
/*      void                                                              */
/*                                                                        */
/* Description:                                                           */
/*      K is (Context 8byte + Encryption key 8byte)                       */
/*========================================================================*/
//static void _id003_enc_speck64_Key128(u32 *key) ////#if defined(ID003_SPECK64)
static void _id003_enc_speck64_init(const u8 *serial_num_ptr, u8 *cbc_ptr) //ok
{
	u8 i, j;
	u32 tkey[4];
	u8 temp_cbc[8];

	/* Convert from u8 to u32 */
	j = 0;
	for(i = 0; i < 2; i++)
	{
		tkey[i]	 = (u32)serial_num_ptr[j] | ((u32)serial_num_ptr[j + 1] << 8) | ((u32)serial_num_ptr[j + 2] << 16) | ((u32)serial_num_ptr[j + 3] << 24);
		j += 4;
	}

	j = 0;
	for(i = 2; i < 4; i++)
	{
		tkey[i]	 = (u32)s_id003_enc_key[j] | ((u32)s_id003_enc_key[j + 1] << 8) | ((u32)s_id003_enc_key[j + 2] << 16) | ((u32)s_id003_enc_key[j + 3] << 24);
		j += 4;
	}

	/* Create key(rk) from tkey */
	for(i = 0; i < ROUNDS;)
	{
		rk[i] = tkey[0]; ER32(tkey[1], tkey[0], i++);
		rk[i] = tkey[0]; ER32(tkey[2], tkey[0], i++);
		rk[i] = tkey[0]; ER32(tkey[3], tkey[0], i++);
	}

#if defined(USE_CBC)
	/* Cretate initial Context */
	memset(&temp_cbc, 0, sizeof(temp_cbc));

	_id003_enc_speck64_encode(serial_num_ptr, temp_cbc); //ok

	for(i = 0; i < ID003_ENC_SPECK64_BLOCK_LEN; i++)
	{
        cbc_ptr[i] = (u8)(temp_cbc[i]);
	}
#endif
}


/*========================================================================*/
/*                                                                        */
/*	_id003_enc_speck64_encode                                             */
/*                                                                        */
/*========================================================================*/
/*                                                                        */
/* Parameter:                                                             */
/*      input_ptr		input data (plain)                                */
/*      output_ptr   	at input holds the previous encryption buffer     */
/*                      at output holds the encrypted input_ptr           */
/*                                                                        */
/* Returns:                                                               */
/*      void                                                              */
/*                                                                        */
/* Description:                                                           */
/*      Encodes 8 bytes of data.                                          */
/*========================================================================*/
static void _id003_enc_speck64_encode(const u8 *input_ptr, u8 *cbc_data_ptr) //ok
{
    u8 i, j;
	u8 temp[8];

#if defined(USE_CBC)
	for(i = 0; i < ID003_ENC_SPECK64_BLOCK_LEN; i++)
	{
        temp[i] = input_ptr[i] ^ cbc_data_ptr[i];
	}
#endif

	/* Convert "temp" from u8 to u32 */
	j = 0;
	for(i = 0; i < 2; i++)
	{
#if defined(USE_CBC)
		Dt[i]	 = (u32)temp[j] | ((u32)temp[j + 1] << 8) | ((u32)temp[j + 2] << 16) | ((u32)temp[j + 3] << 24);
#else
		Dt[i]	 = (u32)input_ptr[j] | ((u32)input_ptr[j + 1] << 8) | ((u32)input_ptr[j + 2] << 16) | ((u32)input_ptr[j + 3] << 24);
#endif
		j += 4;
	}

	/* Encrypted */
	for(i = 0; i < ROUNDS;)		ER32(Dt[1], Dt[0], rk[i++]);

	/* Convert Dt from u32 to u8 */
	j = 0;
	for(i = 0; i < 2; i++)
	{
		cbc_data_ptr[j] = (u8)Dt[i];
		cbc_data_ptr[j + 1] = (u8)(Dt[i] >> 8);
		cbc_data_ptr[j + 2] = (u8)(Dt[i] >> 16);
		cbc_data_ptr[j + 3] = (u8)(Dt[i] >> 24);
		j += 4;
	}
}


/*========================================================================*/
/*                                                                        */
/*	_id003_enc_speck64_decode                                             */
/*                                                                        */
/*========================================================================*/
/*                                                                        */
/* Parameter:                                                             */
/*      input_ptr          input data (cipher)                            */
/*      output_ptr         receives plain data                            */
/*      cbc_ptr            pointer to the CBC context.                    */
/*                                                                        */
/* Returns:                                                               */
/*      void                                                              */
/*                                                                        */
/* Description:                                                           */
/*      Decodes 8 bytes of data.                                          */
/*========================================================================*/
static void _id003_enc_speck64_decode(const u8 *input_ptr, u8 *output_ptr, u8 *cbc_ptr)
{
	int i;
	u8 j;
	u8 temp[8];

	/* Convert "*input_ptr" from u8 to u32 */
	j = 0;
	for(i = 0; i < 2; i++)
	{
		Dt[i]	 = (u32)input_ptr[j] | ((u32)input_ptr[j + 1] << 8) | ((u32)input_ptr[j + 2] << 16) | ((u32)input_ptr[j + 3] << 24);
		j += 4;
	}

	/* Decrypted */
	for(i = (ROUNDS - 1); i >= 0;)	DR32(Dt[1], Dt[0], rk[i--]);

	/* Convert Dt from u32 to u8 */
	j = 0;
	for(i = 0; i < 2; i++)
	{
#if defined(USE_CBC)
		temp[j] = (u8)Dt[i];
		temp[j + 1] = (u8)(Dt[i] >> 8);
		temp[j + 2] = (u8)(Dt[i] >> 16);
		temp[j + 3] = (u8)(Dt[i] >> 24);
		j += 4;
#else
		output_ptr[j] = (u8)Dt[i];
		output_ptr[j + 1] = (u8)(Dt[i] >> 8);
		output_ptr[j + 2] = (u8)(Dt[i] >> 16);
		output_ptr[j + 3] = (u8)(Dt[i] >> 24);
		j += 4;
#endif
	}

	for(i = 0; i < ID003_ENC_SPECK64_BLOCK_LEN; i++)
	{
#if defined(USE_CBC)
        output_ptr[i] = temp[i] ^ cbc_ptr[i];
#endif
		cbc_ptr[i]    = input_ptr[i];
	}
}
