/******************************************************************************/
/*! @addtogroup 
    @file       id003_signature.c
    @brief      Main process for boot I/F
    @date       2020/07/09
    @author     M. Michalek
    @par        Revision
    $Id$
    @par        Copyright (C)
    2012-2013 Japan CashMachine Co, Limited. All rights reserved.
*******************************************************************************
    @par        History
    - 2020/07/09 Development Dept at JCME
      -# Initial Version
    - 2022/06/07 Copy from EBA-40 to iVIZION2 project
******************************************************************************/

#include "common.h"
#include "hal.h"

#define EXT
#include "usb_ram.c"
#include "com_ram.c"

/************************** PRIVATE MACROS *************************/
/* SHA1回転左シフトマクロ関数 */
#define sha1_circular_shift(bits,word) (((word) << (bits)) | ((word) >> (32-(bits))))
/* 1 cycle block size */
#define SIGNATURE_BLOCK_SIZE 0x1000

/************************** PRIVATE VARIABLES *************************/
typedef struct _SHA1_CONTEXT
{
    u32 intermediate_hash[5]; 	/* 中間メッセージダイジェスト(ワード表記：5ワード)  		*/
    u32 length_low;            	/* メッセージ長(下位)(ビット単位) 							*/
    u32 length_high;           	/* メッセージ長(上位)(ビット単位) 							*/
    u16 message_block_index;	/* メッセージブロック配列インデックス 						*/
    u8 message_block[64];      	/* 512ビット長(64バイト長)のメッセージブロック配列 			*/
    u8 computed;               	/* 演算完了フラグ(0:未完、1:完了) 							*/
    u8 corrupted;             	/* メッセージダイジェスト損傷フラグ(0:損傷なし、1：損傷あり) */
	u32 mes_size;				/* メッセージサイズ(バイト単位) 							*/
	u8 *mes_addr;				/* メッセージアドレス 										*/
	u32 digest_size;			/* メッセージダイジェストのサイズ(バイト単位) 				*/
	u8 *digest;					/* SHA1メッセージダイジェスト(20バイト長)へのポインタ 		*/
	u8 phase;					/* SHA-1実行フェーズ 										*/
} SHA1_CONTEXT;
SHA1_CONTEXT ex_sha1_context;	/* SHA-1ハッシュ演算用コンテキスト情報構造の生成 */
u32 s_sha1_seed[5];
/************************** EXTERNAL VARIABLES *************************/
const u32 K[] = {       
	0x5A827999,
	0x6ED9EBA1,
	0x8F1BBCDC,
	0xCA62C1D6
};
/************************** PRIVATE FUNCTIONS *************************/
void calc_sha1_reset(u8* start, u8* end, u8* seed_value, u8* result);
bool calc_sha1_func(void);
static void sha1_reset(void);
static void sha1_result(void);
static void sha1_input(void);
static void sha1_input_under64(void);
static void sha1_procees_block(void);
static void sha1_pad_message(void);
static void sha1_byte_order_change(void);

/*------------------------------------------------------*/
/*  変数の初期化処理                                        */
/*  <引数> なし                                           */
/*  <返値> なし                                           */
/*------------------------------------------------------*/
void calc_sha1_reset(u8* start, u8* end, u8* seed_value, u8* result)
{
	memcpy( (u8*)s_sha1_seed, seed_value, sizeof(s_sha1_seed));
	
	ex_sha1_context.phase = 0;

	/* メッセージサイズの設定(バイト単位) */
	ex_sha1_context.mes_size = end - start;
	/* メッセージアドレスの設定 */
	ex_sha1_context.mes_addr = start;

	/* メッセージダイジェストのサイズの設定(バイト単位) */
	ex_sha1_context.digest_size = 20;
	/* メッセージダイジェストアドレスの設定 */
	ex_sha1_context.digest = result;
}
/*------------------------------------------------------*/
/*  シグネチャ計算	                                    */
/*  <引数> なし                                         */
/*  <返値> なし                                         */
/*------------------------------------------------------*/
bool calc_sha1_func(void)
{
	/* SHA-1実行フェーズに応じて処理 */
	switch(ex_sha1_context.phase) {
	/* リセット処理フェーズ */
	case 0:
		/* リセット処理 */
		sha1_reset();
		break;
	/* メッセージ入力フェーズ */
	case 1:
		/* メッセージ入力処理 */
		sha1_input();
		break;
	/* SHA1演算結果取得フェーズ */
	case 2:
		/* SHA1演算結果の取得理 */
		sha1_result();
		break;
	/* バイトオーダー変更処理状態 */
	case 3:
		/* バイトオーダー変更処理 */
		sha1_byte_order_change();
		break;
	}

	/* 損傷フラグの内容が"損傷あり"の場合? */
	if (ex_sha1_context.corrupted == 1)
	{
		return(true);
	}
	
	if(ex_sha1_context.computed == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}


/*
 *  SHA1リセット関数
 *
 *  説明:
 *    SHA1Context構造体の内容を初期化する。
 *
 *  入力:
 *      なし
 *  出力:
 *      なし
 *
 *  戻り値:
 *      なし
 *
 */
static void sha1_reset(void)
{
    ex_sha1_context.length_low             = 0;
    ex_sha1_context.length_high            = 0;
    ex_sha1_context.message_block_index    = 0;
    ex_sha1_context.intermediate_hash[0]   = s_sha1_seed[0];
    ex_sha1_context.intermediate_hash[1]   = s_sha1_seed[1];
    ex_sha1_context.intermediate_hash[2]   = s_sha1_seed[2];
    ex_sha1_context.intermediate_hash[3]   = s_sha1_seed[3];
    ex_sha1_context.intermediate_hash[4]   = s_sha1_seed[4];
    ex_sha1_context.computed   = 0;
    ex_sha1_context.corrupted  = 0;

	/* SHA-1実行フェーズを1に設定 */
	ex_sha1_context.phase = 1;
}

/*
 *  SHA1演算結果取得関数
 *
 *  説明:
 *      最終メッセージブロックに対してSHA1演算を行い、メッセージダイジェストを出力する。
 *      NOTE: The first octet of hash is stored in the 0th element,
 *            the last octet of hash in the 19th element.
 *
 *  入力:
 *      なし
 *      なし
 *  出力パラメータ:
 *      なし
 *
 *  戻り値:
 *      なし
 *
 */
static void sha1_result(void)
{
	/* ループカウンタ */
    UB count;

	/* SHA1計算完了フラグがOFFの場合？ */
    if (!ex_sha1_context.computed)
    {
		/* ここでは、最終メッセージブロックに対する処理行う */

		/* SHA1Context構造体へのパディング処理(最終メッセージブロックが対象) */
        sha1_pad_message();
		/* 64バイト分(16ワード分)繰り返す */
        for(count=0; count<64; ++count)
        {
            /* message may be sensitive, clear it out */
            ex_sha1_context.message_block[count] = 0;
        }
        /* and clear length */
        ex_sha1_context.length_low = 0;
        ex_sha1_context.length_high = 0;
    }

	memcpy((UB *)&ex_sha1_context.digest[0], (UB *)&ex_sha1_context.intermediate_hash[0], (u16)20);
	/* SHA-1実行フェーズを3に設定 */
	ex_sha1_context.phase = 3;
}

/*
 *  SHA1メッセージ入力関数
 *
 *  説明:
 *    SHA1演算処理部にメッセージを代入し、演算を行う。
 *    SHA1演算は64バイト長のメッセージブロック単位で行われる。
 *
 *  入力パラメータ:
 *      なし
 *  出力パラメータ:
 *      なし
 *
 *  戻り値:
 *      なし
 *
 */
void sha1_input(void)
{
	int	ii;
	u32 count = 0;
	
	/* 長さが0より大きい場合で、かつ、損傷フラグの内容が"損傷なし"の間、繰り返す */
    while (ex_sha1_context.mes_size && (ex_sha1_context.corrupted == 0))
    {
    	if(ex_sha1_context.mes_size < 64)
    	{
			sha1_input_under64();
		}
		else
		{
			/* メッセージブロック(64バイト)の各バイトにメッセージの内容を設定 */
			for(ii = 0; ii < 64; ii++)
			{
				ex_sha1_context.message_block[ii] = *(ex_sha1_context.mes_addr + ii);
			}
			/* 長さ(下位)が0xFFFFFFF8の場合(1回転する直前の場合)？ */
			if(ex_sha1_context.length_low == 0xFFFFFe00)
			{	/* 長さ(下位)を0にリセットする */
				ex_sha1_context.length_low = 0;		/* 長さ(下位)が0の場合(1回転した場合)？ */
				/* 長さ(上位)が0xFFFFFFFFの場合(1回転する直前の場合)？ */
				if(ex_sha1_context.length_high == 0xFFFFFFFF)
				{
					/* 長さ(上位)を0にリセットする */
					ex_sha1_context.length_high = 0;
					/* Message is too long */
					ex_sha1_context.corrupted = 1;		/* 長さ(上位)が0の場合(1回転した場合)？ */
				}
				else
				{
					ex_sha1_context.length_high++;
				}
			}
			else
			{
				/* 長さ(下位)を8増やす(8ビット分) * 64byte */
				ex_sha1_context.length_low += 512;
			}
			
			/* 入力メッセージ部分のアドレスをインクリメント */
			ex_sha1_context.mes_addr += 64;
			
			/* SHA1ブロック演算処理 */
			sha1_procees_block();
			
			ex_sha1_context.mes_size -= 64;
			
			count += 64;
		}
		
		/* メッセージ長が0の場合? */
		if (ex_sha1_context.mes_size == 0) {
			/* SHA-1実行フェーズを2に設定 */
			ex_sha1_context.phase = 2;
			break;
		}
		if(count >= SIGNATURE_BLOCK_SIZE)
		{
			break;
		}
    }
}

static void sha1_input_under64(void)	/* 2013-11-05	*/
{
	/* メッセージブロック(64バイト)の各バイトにメッセージの内容を設定 */

	ex_sha1_context.message_block[ex_sha1_context.message_block_index++] = *ex_sha1_context.mes_addr;

	/* 長さ(下位)が0xFFFFFFF8の場合(1回転する直前の場合)？ */
	if (ex_sha1_context.length_low == 0xFFFFFFF8) {
		/* 長さ(下位)を0にリセットする */
		ex_sha1_context.length_low = 0;		/* 長さ(下位)が0の場合(1回転した場合)？ */
		/* 長さ(上位)が0xFFFFFFFFの場合(1回転する直前の場合)？ */
		if (ex_sha1_context.length_high == 0xFFFFFFFF) {
			/* 長さ(上位)を0にリセットする */
			ex_sha1_context.length_high = 0;
			/* Message is too long */
			ex_sha1_context.corrupted = 1;		/* 長さ(上位)が0の場合(1回転した場合)？ */
		} else {
			ex_sha1_context.length_high++;
		}
	} else {
		/* 長さ(下位)を8増やす(8ビット分) */
		ex_sha1_context.length_low += 8;
	}

	/* 入力メッセージ部分のアドレスをインクリメント */
	ex_sha1_context.mes_addr++;

	/* メッセージブロックインデックスが64に達した場合(1ブロック分演算準備ができた場合)？ */
	/* なお、メッセージブロックインデックス値は、sha1_procees_block()の最後で0にリセットされる */
	if (ex_sha1_context.message_block_index == 64)
	{
		/* SHA1ブロック演算処理 */
		sha1_procees_block();
	}
	
	--ex_sha1_context.mes_size;
}


/*
 *  SHA1ブロック演算処理関数
 *
 *  説明:
 *      この関数は512ビット長(64バイト長)のメッセージブロックに対してSHA1演算を行う。
 *
 *  入力パラメータ:
 *      なし
 *  出力パラメータ:
 *      なし
 *
 *  戻り値:
 *      なし。
 *
 *  Comments:
 *      Many of the variable names in this code, especially the
 *      single character names, were used because those were the
 *      names used in the publication.
 *
 *
 */
static void sha1_procees_block(void)
{
    /* ループカウンタ */
    int     t;        
    /* テンポラリワード値 */
    u32      temp_word;
    /* ワード配列 */
    u32      W[16];
    /* ワードバッファA */
    u32      wA;     
    /* ワードバッファB */
    u32      wB;     
    /* ワードバッファC */
    u32      wC;     
    /* ワードバッファD */
    u32      wD;     
    /* ワードバッファE */
    u32      wE;     
    /* インデックスs(0から15までの数になる) */
    u8      s;     
    /* マスク値 */
    u8      mask;     

	/* マスクに0x0Fを設定 */
	mask = 0x0F;

    /*
     *  Initialize the first 16 words in the array W
     */
    /* バイト表記(message_block[])からワード表記(W[])するための処理：最初の16ワード分 */
    for(t = 0; t < 16; t++)
    {
        W[t] = ((u32)ex_sha1_context.message_block[t * 4]) << 24;
        W[t] |= ((u32)ex_sha1_context.message_block[t * 4 + 1]) << 16;
        W[t] |= ((u32)ex_sha1_context.message_block[t * 4 + 2]) << 8;
        W[t] |= ((u32)ex_sha1_context.message_block[t * 4 + 3]);
    }

	/* 中間Message Digest(ワード単位)の現在値をワードバッファA、B、C、D、Eに代入 */
	/* なお、中間Message Digestの値はこの関数が実行されるたびに更新される。 */
    wA = ex_sha1_context.intermediate_hash[0];
    wB = ex_sha1_context.intermediate_hash[1];
    wC = ex_sha1_context.intermediate_hash[2];
    wD = ex_sha1_context.intermediate_hash[3];
    wE = ex_sha1_context.intermediate_hash[4];
	
	/* ループ */
	for (t = 0; t < 80; t++)
	{
		/* インデックスtからインデックスsを求める */
		s = t & mask;

		/* ワード16から79を対象にSHA1回転シフト処理(処理内容はSHA1仕様書RFC3174.txtを参照) */
		if(t >= 16)
		{
			/* SHA1回転シフト処理(シフトは排他的論理和に左方向に1ビットシフト) */
			W[s] = sha1_circular_shift(1,W[(s+13) & mask] ^ W[(s+8) & mask] ^ W[(s+2) & mask] ^ W[s]);
		}

		/* 共通部分 */
		temp_word = sha1_circular_shift(5,wA) + wE + W[s];
		/* ループカウンタが0以上20未満の場合の処理 */
		if (t >= 0 && t < 20) {
			temp_word += ((wB & wC) | ((~wB) & wD)) + K[0];
		/* ループカウンタが20以上40未満の場合の処理(処理内容はSHA1仕様書RFC3174.txtを参照) */
		} else if (t >= 20 && t < 40) {
			temp_word += (wB ^ wC ^ wD) + K[1];
		/* ループカウンタが40以上60未満の場合の処理(処理内容はSHA1仕様書RFC3174.txtを参照) */
		} else if (t >= 40 && t < 60) {
			temp_word += ((wB & wC) | (wB & wD) | (wC & wD)) + K[2];
		/* ループカウンタが60以上80未満の場合の処理(処理内容はSHA1仕様書RFC3174.txtを参照) */
		} else {
			temp_word += (wB ^ wC ^ wD) + K[3];
		}

		/* 以降の処理はループカウンタに関係なく共通 */
		wE = wD;
		wD = wC;
		wC = sha1_circular_shift(30,wB);

		wB = wA;
		wA = temp_word;
	}

	/* 中間Message Digest(ワード単位)にワードバッファA、B、C、D、Eを加算することで更新 */
	ex_sha1_context.intermediate_hash[0] += wA;
	ex_sha1_context.intermediate_hash[1] += wB;
	ex_sha1_context.intermediate_hash[2] += wC;
	ex_sha1_context.intermediate_hash[3] += wD;
	ex_sha1_context.intermediate_hash[4] += wE;

	/* メッセージブロックインデックスを0にリセット */
    ex_sha1_context.message_block_index = 0;
}

/*
 *  SHA1メッセージパディング関数
 *
 *  説明:
 *    最終メッセージブロックに対して、パディング処理とオリジナルメッセージ長の付加
 *  を行い、最終メッセージブロックに対するSHA1ブロック演算処理を行う。
 *    この処理では、まず最初に、ブロックの残りの領域が9バイト以上あるかどうかを確認
 *  する。これは、パディング用のバイトが最低1バイトとオリジナルメッセージ長の領域が
 *  8バイト必要なためである。
 *
 *  入力パラメータ:
 *      なし。
 *  出力パラメータ:
 *      なし。
 *  戻り値:
 *      なし。
 *
 */
static void sha1_pad_message(void)
{
	/* メッセージブロックインデックスが55より大きい場合？ */
	/* (最終メッセージブロックの最後の8バイト部分がオリジナルメッセージ長の設定領域で予約され、 */
	/* さらにパディング用のバイトが最低1バイト必要なため。合計で最低9バイト必要なため。) */
    if (ex_sha1_context.message_block_index > 55)
    {
		/* メッセージ終端直後のバイトに1をパッドする */
        ex_sha1_context.message_block[ex_sha1_context.message_block_index++] = 0x80;
		/* メッセージブロックインデックスが64未満の間繰り返す */
		/* (メッセージブロックの残りの部分全てに0をパッドするため) */
        while(ex_sha1_context.message_block_index < 64)
        {
			/* 0をパッドする */
            ex_sha1_context.message_block[ex_sha1_context.message_block_index++] = 0;
        }

		/* SHA1ブロック演算処理 */
        sha1_procees_block();

 		/* メッセージブロックインデックスが56未満の間繰り返す */
		/* (オリジナルメッセージ長の領域直前まで0でパッドするため) */
        while(ex_sha1_context.message_block_index < 56)
        {
			/* 0をパッドする */
            ex_sha1_context.message_block[ex_sha1_context.message_block_index++] = 0;
        }
    }
    else
    {
		/* メッセージ終端直後のバイトに1をパッドする */
        ex_sha1_context.message_block[ex_sha1_context.message_block_index++] = 0x80;
		/* メッセージブロックインデックスが56未満の間繰り返す */
		/* (オリジナルメッセージ長の領域直前まで0でパッドするため) */
        while(ex_sha1_context.message_block_index < 56)
        {
			/* 0をパッドする */
            ex_sha1_context.message_block[ex_sha1_context.message_block_index++] = 0;
        }
    }

    /*
     *  Store the message length as the last 8 octets
     */
    ex_sha1_context.message_block[56] = (ex_sha1_context.length_high >> 24) & 0xFF;
    ex_sha1_context.message_block[57] = (ex_sha1_context.length_high >> 16) & 0xFF;
    ex_sha1_context.message_block[58] = (ex_sha1_context.length_high >> 8) & 0xFF;
    ex_sha1_context.message_block[59] = (ex_sha1_context.length_high) & 0xFF;
    ex_sha1_context.message_block[60] = (ex_sha1_context.length_low >> 24) & 0xFF;
    ex_sha1_context.message_block[61] = (ex_sha1_context.length_low >> 16) & 0xFF;
    ex_sha1_context.message_block[62] = (ex_sha1_context.length_low >> 8) & 0xFF;
    ex_sha1_context.message_block[63] = (ex_sha1_context.length_low) & 0xFF;
	/* (オリジナルメッセージ長を含む)最終ブロックに対するSHA1ブロック演算処理 */
    sha1_procees_block();
}

/*
 *  SHA1メッセージダイジェストバイトオーダー変更関数
 *
 *  説明:
 *      メッセージダイジェストのバイトオーダーを逆にする。
 *
 *  入力:
 *      なし。
 *  出力パラメータ:
 *      なし。
 *
 *  戻り値:
 *      なし。
 *
 */
static void sha1_byte_order_change(void)
{
	/* SHA1計算完了フラグをONに設定 */
    ex_sha1_context.computed = 1;
}
