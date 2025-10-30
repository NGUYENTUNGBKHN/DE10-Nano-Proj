

#ifndef SIMURATION
#include "kernel_inc.h"
#include "kernel_config.h"
#endif

#include "cod_malloc.h"
#include <stdio.h>
#include <stdlib.h>

#define EXT
#include "../common/global.h"
#include "cod_config.h"


#ifdef _COD_BAU_SE_

#define MAX_CHAR_HEAP 200000
//#define MAX_INT_HEAP 10000
#define MAX_INT_HEAP 200000
#define MAX_FLOAT_HEAP 50000

static char char_heap[MAX_CHAR_HEAP];
static int int_heap[MAX_INT_HEAP];
static float float_heap[MAX_FLOAT_HEAP];

static int used_char = 0;
static int used_int = 0;
static int used_float = 0;

char* malloc_char(int size) {
	char* p;
	if (used_char + size > MAX_CHAR_HEAP) {
		used_char = 0;
	}
	p = char_heap + used_char;
	used_char += size;
	return p;
}

int* malloc_int(int size) {
	int* p;
	if (used_int + size > MAX_INT_HEAP) {
		used_int = 0;
	}
	p = int_heap + used_int;
	used_int += size;
	return p;
}

float* malloc_float(int size) {
	float* p;
	if (used_float + size > MAX_FLOAT_HEAP) {
		used_float = 0;
	}
	p = float_heap + used_float;
	used_float += size;
	return p;
}

#else 

//char* malloc_char(int size) {
//	return (char*)malloc(sizeof(char) * size);
//}
//
//int* malloc_int(int size) {
//	return (int*)malloc(sizeof(int) * size);
//}
//
//float* malloc_float(int size) {
//	return (float*) malloc(sizeof(float) * size);
//}
char* malloc_char(int size)
{
#ifdef BAU_LE17
	return (char *)get_system_mpl(sizeof(char) * size);
#else
	return (char*)malloc(sizeof(char) * size);
#endif
}

short* malloc_short(int size)
{
#ifdef BAU_LE17
	return (short *)get_system_mpl(sizeof(short) * size);
#else
	return (short*)malloc(sizeof(short) * size);
#endif
}

int* malloc_int(int size)
{
#ifdef BAU_LE17
	return (int *)get_system_mpl(sizeof(int) * size);
#else
	return (int*)malloc(sizeof(int) * size);
#endif
}

float* malloc_float(int size)
{
#ifdef BAU_LE17
	return (float *)get_system_mpl(sizeof(float) * size);
#else
	return (float*)malloc(sizeof(float) * size);
#endif
}

void* malloc_void(int size)
{
#ifdef BAU_LE17
	return (void*)get_system_mpl(size);
#else
	return (void*)malloc(size);
#endif
}


void free_proc(void * add)
{
#ifdef BAU_LE17
	release_system_mpl(add);
#else
	free(add);
#endif
}
#endif
