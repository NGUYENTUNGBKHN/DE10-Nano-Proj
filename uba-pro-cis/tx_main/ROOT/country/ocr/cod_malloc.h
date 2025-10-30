#ifndef _FAKE_MALLOC_H_
#define _FAKE_MALLOC_H_

#ifdef __cplusplus
extern "C" {
#endif

void* malloc_void(int size); 
char* malloc_char(int size);
int* malloc_int(int size);
float* malloc_float(int size);
short* malloc_short(int size);
void free_proc(void * add);

#ifdef __cplusplus
}
#endif

#endif

