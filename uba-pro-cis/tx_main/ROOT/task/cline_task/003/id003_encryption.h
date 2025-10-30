/*******************************************************************************
*
*   Model        : UBA-RetroFit
*   File Name    : IF_ID003enc.h
*   Contents     : ﾒｲﾝﾌﾟﾛｸﾞﾗﾑ領域 I/F ID003
*   Version      : 1.00
*   CPU          : SH7203
*   Compiler     : SHC9.02.00
*
*    Copyright (C) 2009 Japan Cash Machine CO.,LTD. All Rights Reserved
*
*    Copyright (C) 2005 Renesas Technology Corp. All Rights Reserved
*    and Renesas Solutions Corp. All Rights Reserved
*
*    history      :2009.08.25 ver.1.00 WS model
*
********************************************************************************/

extern void id003_enc_init(void);
extern void id003_enc_make_key(u8 *key);
extern void id003_enc_set_number(u8 number);
extern u8   id003_enc_get_number(void);
extern void id003_enc_encode(u8 *src, u8 *dst);
extern void id003_enc_update_context(void);
extern void id003_enc_vendack_receive(void);
extern u8 id003_decryptcmd(u8 *data);

/*--------------------------------------*/

extern u8 id003_enc_mode;

//SPECKモードになるコマンドは2つ
//Secret numbertコマンドの拡張版 or _id003_enc_mode_cmd_proc
#define PAYOUT8MODE     	0x01
#define PAYOUT16MODE    	0x02
#define	PAYOUT16MODE_SPECK	0x04	//defined(ID003_SPECK64)


#define ID003_ENC_GC2_ROUNDS		2
#define ID003_ENC_GC2_BLOCK_LEN		8

/* barrel shifters*/
#define ID003_ENC_GC2_BARREL_R(x)	(((x) >> 3) | ((x) << 5))
#define ID003_ENC_GC2_BARREL_L(x)	(((x) << 3) | ((x) >> 5))

#define	ROTL32(x, r)	(((x) << (r)) | (x >> (32 - (r))))
#define	ROTR32(x, r)	(((x) >> (r)) | ((x) << (32 - (r))))

#define	ER32(x, y, k)	(x = ROTR32(x, 8), x += y, x ^= k, y = ROTL32(y, 3), y ^= x)
#define	DR32(x, y, k)	(y ^= x, y = ROTR32(y, 3), x ^= k, x -= y, x = ROTL32(x, 8))
#define	ROUNDS			27
#define	ID003_ENC_SPECK64_BLOCK_LEN		8

extern u8 _id003_sts_poll; //#if defined(ID003_SPECK64)
extern u8 ex_illigal_payout_command;

/*--------------------------------------*/
/* ｴﾝｸﾘﾌﾟｼｮﾝがONかどうかを判断する関数 	*/
/*	ｴﾝｸﾘﾌﾟｼｮﾝがONなら TRUEを返す 		*/
/*	ｴﾝｸﾘﾌﾟｼｮﾝがOFFなら FALSEを返す 		*/
/*	ようにする。						*/
/*--------------------------------------*/
#if 0
#pragma inline(If_encryption_is_on)
static BOOL If_encryption_is_on(void)
{
	if( id003_optional_d1 & 0x80 ){
		return(TRUE);
	}
	else{
		return(FALSE);
	}
}
#endif
