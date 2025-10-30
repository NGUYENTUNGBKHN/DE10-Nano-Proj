//#include "systemdef.h"
#include "jcm_typedef.h"

/****************************************************************/
/**
 * @brief メモリーコピー
 * @param[out] dst  コピー先先頭アドレス
 * @param[in]  src  コピー元先頭アドレス
 * @param[in]  size コピーサイズ(byte単位)
 */
/****************************************************************/
void copy_memory(void *dst, void *src, u32 size)
{
	u32 ii;
	u8* c_dst;
	u8* c_src;
	
	c_dst = dst;
	c_src = src;
	
	for(ii=0;ii<size;ii++)
	{
		*c_dst = *c_src;
		c_dst++;
		c_src++;
	}
}


/****************************************************************/
/**
 * @brief メモリーコピー（４バイトコピー）
 * @param[out] dst  コピー先先頭アドレス
 * @param[in]  src  コピー元先頭アドレス
 * @param[in]  size コピーサイズ(byte単位)
 */
/****************************************************************/
void copy_memory4(void *dst, void *src, u32 size)
{
	u32 ii;
	u32* c_dst;
	u32* c_src;
	
	c_dst = dst;
	c_src = src;
	
	for(ii=0;ii < (size / sizeof(long));ii++)
	{
		*c_dst = *c_src;
		c_dst++;
		c_src++;
	}
}


/****************************************************************/
/**
 * @brief 指定メモリを0で初期化する。
 * @param[in] size バッファのサイズ
 * @param[out] dst 初期化するバッファの先頭アドレス
*/
/****************************************************************/
void zero_memory(void *dst, u32 size)
{
	u32 ii;
	u8* c_dst;
	
	c_dst = dst;
	
	for(ii=0;ii<size;ii++)
	{
		*c_dst = 0x00;
		c_dst++;
	}
}


/****************************************************************/
/**
 * @brief 指定データで初期化
 * @param[in] c 初期化するデータ
 * @param[in] size バッファのサイズ
 * @param[out] dst 初期化するバッファの先頭アドレス
 * @return 無し
*/
/****************************************************************/
void fill_memory(void *dst, u8 c, u32 size)
{
	u32 ii;
	u8 *c_dst;
	
	c_dst = dst;
	
	for(ii=0;ii<size;ii++)
	{
		*c_dst = c;
		c_dst++;
	}
}


/****************************************************************/
/**
 * @brief メモリーコンペア
 * @return 無し
*/
/****************************************************************/
int compare_memory(void* ps1,void* ps2, u32 size)
{
	u32 ii;
	u8* p1 = ps1;
	u8* p2 = ps2;
	
	for(ii=0;ii<size;ii++)
	{
		if(*p1 != *p2)
		{
			return false;
		}
		
		p1++;
		p2++;
	}
	
	return true;
}



/* End of file */
