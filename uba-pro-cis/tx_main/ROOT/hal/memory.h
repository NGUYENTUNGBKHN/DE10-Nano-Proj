/*eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee*/
/**
 * @file memory.h
 * @brief メモリー操作関数 ヘッダファイル
 */
/*eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee*/
#if !defined(_MEMORY_H_INCLUDED_)
#define _MEMORY_H_INCLUDED_

#if defined(__cplusplus)
extern "C" {
#endif
/* } */

void copy_memory(void *dst, void *src, u32 size);
void copy_memory4(void *dst, void *src, u32 size);
void zero_memory(void *dst, u32 size);
void fill_memory(void *dst, u8 c, u32 size);
int compare_memory(void *ps1, void *ps2, u32 size);


#if defined(__cplusplus)
}
#endif



#endif

