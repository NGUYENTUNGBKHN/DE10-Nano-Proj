#include <stdio.h>
#include <stdlib.h>

#define EXT
#include "../common/global.h"

void size_judge (ST_SJ* sj)//サイズ識別
{
	sj->res = 0;//何にも当てはまらなければエラー

	sj->note_x_size = (u16)(sj->note_x_size * 0.127f * 10);
	sj->note_y_size = (u16)(sj->note_y_size * 0.127f * 10);

	if(sj->thr_note_x_size_max >= sj->note_x_size && sj->note_x_size >= sj->thr_note_x_size_min &&
		sj->thr_note_y_size_max >= sj->note_y_size && sj->note_y_size >= sj->thr_note_y_size_min)
	{
		sj->res = 0;	//サイズ一致
	}
	else
	{
		sj->res = ERR_SIZE_MISS_MACH;//何にも当てはまらなければエラー
	}

	// ichijo
	size_limit.x_min_size = sj->thr_note_x_size_min;
	size_limit.x_max_size = sj->thr_note_x_size_max;
	size_limit.y_min_size = sj->thr_note_y_size_min;
	size_limit.y_max_size = sj->thr_note_y_size_max;
	size_limit.x_size = sj->note_x_size;
	size_limit.y_size = sj->note_y_size;
}
