#include	<math.h>
#include    <string.h>
#include    <float.h>
#include <stdlib.h>

#define EXT

#include "../common/global.h"
/*
2021/12/2
	デバッグ用にCSV出力できるように

2022/03/17
	CNY用の処理変更
	基準との差は切り捨てから四捨五入に変更(template.c)
*/

double level_average[4] = { 0.289125777, 0.279554188, 0.258042956, 0.2570479 };		// 信頼レベルパラメータ_平均（共通）
double level_std[4] = { 0.035061887, 0.032332046, 0.025992389, 0.025890892 };	// 信頼レベルパラメータ_標準偏差（共通）

u8 db_maskpat[] = {// マスクパターン 25*25
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
				0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
				0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
				0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
				0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
				0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
				0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
				0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
				0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
				0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
				0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
				0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
				0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
				0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
				1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
				0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
				0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
				0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
				0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
				0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
				0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
				0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
				0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,
				0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
				0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
				0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,
				0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
				0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
				0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
				0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

u8 size_level(u16 size)
{
	s32 t_size = (s32)size;
	// サイズにおけるレベルの設定
	if (t_size >= 5)
	{
		t_size = (t_size - 5);
		t_size = 40 - t_size * 5; 
		if (t_size > 100)
		{
			t_size = 100;
		}
		return (u8)t_size;
	}
	else
	{
		t_size = (4 - t_size) / 2;
		t_size = 50 + t_size * 10;
		if (t_size < 0)
		{
			t_size = 0;
		}
		return (u8)t_size;
	}
}


// 折畳レベル計算用
// 幅と長さ共通でどちらか一方のみ入る
u8 fold_level(float diff)
{
	float result = 0;
	// しきい値よりも大きいとき、レベル60以上になる。（diffに正の数が来るので足される）このとき、欠損レベルを出力するよう遷移する
	// しきい値よりも小さいとき、レベル60未満になる。（diffに負の数が来るので引かれる）
	result = FOLDING_LEVEL + diff;
	if (result < 0)
	{
		result = 1;
	}
	else if (result > 100)
	{
		result = 100;
	}
	return (u8)result;
}

// 欠損レベル計算用
// 幅と長さ共通でどちらか一方のみ入る
u8 mutilation_level(s16 diff)
{
	s16 result = 0;
	diff = diff * CONSTANT_FOR_MUTILATION;
	if (diff == 0)
	{
		result = 100;
	}
	else
	{
		result = MUTILATION_LEVEL - diff;
	}

	if (result < 0)
	{
		result = 1;
	}
	else if (result > 100)
	{
		result = 100;
	}
	return (u8)result;
}

s32 get_folding_invalid_count(u8 buf_num, ST_FOLDING* folding, u16 x_sz, u16 y_sz)
{
	//u16 invalid_count[4] = { 0 };
	s16 tmp = 0;	// IR1のPeak一時格納
	u32 counter = 0;	// Peak検索時に使用するカウンタ
	//u8 plane = 0;	// パラ：プレーン番号
	s16 temporary = 0;
	s32 x = 0;		// x座標
	s32 xp = 0;		// xp座標
	s32 y = 0;		// y座標
	s32 yp = 0;		// yp座標
	s32 adjust = 25;	// 調整
	u16 plane_side = 0;	// パラ：プレーンのサイド
	u16 point_cnt = 0;		//パラ：カレントポイント
	u32 result = 0;
	ST_SPOINT spt;
	ST_NOTE_PARAMETER* pnote_param = &work[buf_num].note_param;
	float average[4] = { 0.0 };	 // 透過IR平均
	float reliability[4] = {0.0};	// 信頼度
	float threshold[4] = { 0.0 };
	u8 level[4] = { 100 };	// 信頼レベル
	u8 j_cnt = 0; // サイズが短くなっているときにだけ折り畳み検知が実行されるのでそのときに実行されたかどうかのフラグとして使用する
	s32 x_size = 0;
	s32 y_size = 0;
	u32 res_count = 0;
	u8 cnt_th = 0;
	s32 loss_a = 8; // 1mm短くなるにつれて8dot分だけ見るところをずらす必要がある
	s32 loss_b = 8; // 1mm短くなるにつれて8dot分だけ見るところをずらす必要がある
	float rel_tmp = 0.0;
	float sw_tmp = 0.0;
	s16 height = 0; // 紙幣を正面に見たときの高さ
	s16 width = 0;// 紙幣を正面に見たときの幅(長さ）
	s32 xx = 0;
	s32 yy = 0;
	u8 area = 15;//探索範囲　2022/03/17

#ifdef VS_DEBUG
	FILE* fp;
	if (work[buf_num].pbs->blank3 != 0)
	{
		fp = fopen("fold.csv", "a");
	}
#endif

	x_size = (s32)x_sz;
	y_size = (s32)y_sz;

	spt.way = work[buf_num].pbs->insertion_direction;		//方向設定
	spt.l_plane_tbl = tir1_plane_tbl_tir1[plane_side];				//プレーンの設定

	// 2mmくらいの小さい折り畳みでも検知をする必要がある国の時、１のフラグをつける（パラメタ調整時）
	// フラグが１のとき、入る
	if (folding->ppoint[point_cnt + 1].x_s == 1)
	{
		// LEとSEで切り替える
		if (work[buf_num].pbs->LEorSE == 0)
		{
			width = work[buf_num].pbs->note_x_size;
			height = work[buf_num].pbs->note_y_size;
		}
		else
		{
			height = work[buf_num].pbs->note_x_size;
			width = work[buf_num].pbs->note_y_size;
		}

		height = (s16)((height) * 0.5f); // 紙幣の高さ
		width = (s16)((width) * 0.5f); // 紙幣の幅（長さ）

		threshold[0] = folding->ppoint[point_cnt].threshold1;
		threshold[1] = folding->ppoint[point_cnt].threshold2;
		threshold[2] = folding->ppoint[point_cnt].threshold3;
		threshold[3] = folding->ppoint[point_cnt].threshold4;

		if (y_size != 0)
		{
			yy = y_size*8;	//ミリ単位からドットに変更　2022/03/17

			// 紙幣の正面から見て”上”のエッジ
			for (y = height - yy - area; y < height - yy + area; y++)
			{
				for (x = -width; x < width; x++)
				{
					spt.x = x;
					spt.y = y;
					new_L_Getdt(&spt, work[buf_num].pbs, pnote_param);
					if (spt.sensdt < HOLOGRAM_THRESHOLD && WINDOW_THRESHOLD < spt.sensdt)
					{
						result += (u32)spt.sensdt;
						counter++;
					}
					x++;
				}
			}

			if (counter == 0)
			{
				reliability[0] = 0; // 上の折り畳み信頼度
			}
			else
			{
				average[0] = ((float)result / (float)counter);
				reliability[0] = ((float)WINDOW_THRESHOLD) / average[0]; // 上の折り畳み信頼度
			}

			counter = 0;
			result = 0;
			j_cnt++;

			// 紙幣の正面から見て”下”のエッジ
			for (y = -height + yy - area; y < -height + yy + area; y++)
			{
				for (x = -width; x < width; x++)
				{
					spt.x = x;
					spt.y = y;
					new_L_Getdt(&spt, work[buf_num].pbs, pnote_param);

					if (spt.sensdt < HOLOGRAM_THRESHOLD && WINDOW_THRESHOLD < spt.sensdt)
					{
						result += (u32)spt.sensdt;
						counter++;
					}
					x++;
				}
			}

			if (counter == 0)
			{
				reliability[1] = 0; // 下の折り畳み信頼度
			}
			else
			{
				average[1] = ((float)result / (float)counter);
				reliability[1] = ((float)WINDOW_THRESHOLD) / average[1]; // 下の折り畳み信頼度
			}
			counter = 0;
			result = 0;
			j_cnt++;

			// 最も折り畳みの可能性が高いものを選ぶ（最後にレベル計算を行う）
			// 上が折り畳みの可能性があるとき
			if (reliability[0] > reliability[1])
			{
				if (rel_tmp < reliability[0])
				{
					rel_tmp = reliability[0];
					res_count = 0;
				}
			}
			// 下が折り畳みの可能性があるとき
			else
			{
				if (rel_tmp < reliability[1])
				{
					rel_tmp = reliability[1];
					res_count = 1;
				}
			}
		}

		if (x_size != 0)
		{
			
			xx = 8*x_size;//ミリ単位からドットに変更　2022/03/17

			// LEFT
			for (y = -height; y < height; y++)
			{
				for (x = -width + xx- area; x < -width + xx+ area; x++)
				{
					spt.x = x;
					spt.y = y;
					new_L_Getdt(&spt, work[buf_num].pbs, pnote_param);

					if (spt.sensdt < HOLOGRAM_THRESHOLD && WINDOW_THRESHOLD < spt.sensdt)
					{
						result += (u32)spt.sensdt;
						counter++;
					}
					//x++;
				}
			}
			if (counter == 0)
			{
				reliability[2] = 0; // RIGHTの折り畳み信頼度
			}
			else
			{
				average[2] = ((float)result / (float)counter);
				reliability[2] = ((float)WINDOW_THRESHOLD) / average[2]; // LEFTの折り畳み信頼度
			}
			counter = 0;
			result = 0;
			j_cnt++;

			// RIGHT
			for (y = -height; y < height; y++)
			{
				for (x = width - xx- area; x < width - xx+ area; x++)
				{
					spt.x = x;
					spt.y = y;
					new_L_Getdt(&spt, work[buf_num].pbs, pnote_param);

					if (spt.sensdt < HOLOGRAM_THRESHOLD && WINDOW_THRESHOLD < spt.sensdt)
					{
						result += (u32)spt.sensdt;
						counter++;
					}
					//x++;
				}
			}

			if (counter == 0)
			{
				reliability[3] = 0; // RIGHTの折り畳み信頼度
			}
			else
			{
				average[3] = ((float)result / (float)counter);
				reliability[3] = ((float)WINDOW_THRESHOLD) / average[3]; // RIGHTの折り畳み信頼度
			}
			counter = 0;
			result = 0;
			j_cnt++;
			// 最も折り畳みの可能性が高いものを選ぶ（最後にレベル計算を行う）
			// 左が折り畳みの可能性があるとき
			if (reliability[2] > reliability[3])
			{
				if (rel_tmp < reliability[2])
				{
					rel_tmp = reliability[2];
					res_count = 2;
				}
			}
			// 右が折り畳みの可能性があるとき
			else
			{
				if (rel_tmp < reliability[3])
				{
					rel_tmp = reliability[3];
					res_count = 3;
				}
			}
		}

		if (j_cnt > 0)
		{
			folding->folded_level[res_count] = fold_level(average[res_count] - threshold[res_count]);
			folding->tir_diff[res_count] = average[res_count] - threshold[res_count];
			// 最終結果
			if (folding->folded_level[res_count] > FOLDING_LEVEL)
			{
				if (res_count < 2)
				{
					folding->folded_level[res_count] = mutilation_level((s16)y_size);
				}
				else
				{
					folding->folded_level[res_count] = mutilation_level((s16)x_size);
				}

				folding->folded[res_count] = 4;//欠損
				folding->err_code = 4;
			}
			else
			{
				folding->folded[res_count] = 1;//折り畳み
				folding->err_code = (u8)res_count;
			}
		}

		if (folding->folded_level[res_count] > 100)
		{
			folding->folded_level[res_count] = 100;
		}
		else if (folding->folded_level[res_count] < 1)
		{
			folding->folded_level[res_count] = 1;
		}
	}
	else
	{
		//偶数が表　奇数が裏
		//plane = tir1_plane_tbl_tir1[plane_side];
		if (folding->ppoint[point_cnt].x_s > folding->ppoint[point_cnt].x_e)
		{
			temporary = folding->ppoint[point_cnt].x_e;
			folding->ppoint[point_cnt].x_e = folding->ppoint[point_cnt].x_s;
			folding->ppoint[point_cnt].x_s = temporary;
			temporary = 0;
		}
		if (folding->ppoint[point_cnt].y_s > folding->ppoint[point_cnt].y_e)
		{
			temporary = folding->ppoint[point_cnt].y_e;
			folding->ppoint[point_cnt].y_e = folding->ppoint[point_cnt].y_s;
			folding->ppoint[point_cnt].y_s = temporary;
			temporary = 0;
		}

		threshold[0] = folding->ppoint[point_cnt].threshold1;
		threshold[1] = folding->ppoint[point_cnt].threshold2;
		threshold[2] = folding->ppoint[point_cnt].threshold3;
		threshold[3] = folding->ppoint[point_cnt].threshold4;

		if (y_size != 0)
		{
			loss_a = loss_a * y_size; // 短くなっている分だけずらす
			if (loss_a > 25)
			{
				loss_a = loss_a / 25;
				if (loss_a > 1)
				{
					loss_a /= 2;
					loss_b = loss_a;
				}
				else
				{
					loss_a = -1;
					loss_b = 1;
				}
			}
			else
			{
				loss_a = -1;
				loss_b = 1;
			}
			// UP
			for (yp = folding->ppoint[point_cnt].y_e - loss_b; yp <= folding->ppoint[point_cnt].y_e - loss_b; yp++)
			{
				for (xp = folding->ppoint[point_cnt].x_s; xp < folding->ppoint[point_cnt].x_e; xp++)
				{
					for (y = yp * adjust; y < (yp * adjust) + adjust; y++)
					{
						for (x = xp * adjust; x < (xp * adjust) + adjust; x++)
						{
							spt.x = x;
							spt.y = y;
							new_L_Getdt(&spt, work[buf_num].pbs, pnote_param);
							tmp = spt.sensdt;
							if (tmp < HOLOGRAM_THRESHOLD && WINDOW_THRESHOLD < tmp)
							{
								result += (u32)tmp;
								counter++;
							}
							x++;
						}
					}
				}
			}
			if (counter == 0)
			{
				reliability[0] = 0; // RIGHTの折り畳み信頼度
			}
			else
			{
				average[0] = ((float)result / (float)counter);
				reliability[0] = ((float)WINDOW_THRESHOLD) / average[0]; // UPの折り畳み信頼度
			}
			counter = 0;
			result = 0;
			j_cnt++;

			// DOWN
			for (yp = folding->ppoint[point_cnt].y_s + loss_a; yp <= folding->ppoint[point_cnt].y_s + loss_a; yp++)
			{
				for (xp = folding->ppoint[point_cnt].x_s; xp < folding->ppoint[point_cnt].x_e; xp++)
				{
					for (y = yp * adjust; y < (yp * adjust) + adjust; y++)
					{
						for (x = xp * adjust; x < (xp * adjust) + adjust; x++)
						{
							spt.x = x;
							spt.y = y;
							new_L_Getdt(&spt, work[buf_num].pbs, pnote_param);
							tmp = spt.sensdt;
							if (tmp < HOLOGRAM_THRESHOLD && WINDOW_THRESHOLD < tmp)
							{
								result += (u32)tmp;
								counter++;
							}
							x++;
						}
					}
				}
			}
			if (counter == 0)
			{
				reliability[1] = 0; // RIGHTの折り畳み信頼度
			}
			else
			{
				average[1] = ((float)result / (float)counter);
				reliability[1] = ((float)WINDOW_THRESHOLD) / average[1]; // DOWNの折り畳み信頼度
			}
			counter = 0;
			result = 0;
			j_cnt++;
			// 最も折り畳みの可能性が高いものを選ぶ
			if (reliability[0] > reliability[1])
			{
				if (rel_tmp < reliability[0])
				{
					rel_tmp = reliability[0];
					res_count = 0;
				}
				level[0] = size_level((u16)y_size);
			}
			else
			{
				if (rel_tmp < reliability[1])
				{
					rel_tmp = reliability[1];
					res_count = 1;
				}
				level[0] = size_level((u16)y_size);
			}

			// マジックナンバーではなく初期化です。
			loss_a = 8;
			loss_b = 8;
		}

		if (x_size != 0)
		{
			loss_a = loss_a * x_size; // 短くなっている分だけずらす
			if (loss_a > 25)
			{
				loss_a = loss_a / 25;
				if (loss_a > 1)
				{
					loss_a /= 2;
					loss_b = loss_a;
				}
				else
				{
					loss_a = -1;
					loss_b = 1;
				}
			}
			else
			{
				loss_a = -1;
				loss_b = 0;
			}

			// 右側のホログラムが折畳検知に影響を与えている場合に実行する（事前にフラグを立てている）
			if (folding->ppoint[point_cnt + 1].x_s != -1 && folding->ppoint[point_cnt + 1].y_s != -1)
			{
				if (loss_a >= folding->ppoint[point_cnt + 1].x_s  && folding->ppoint[point_cnt + 1].y_s <= 4)
				{
					switch (folding->ppoint[point_cnt + 1].x_e)
					{
					case 0:
						sw_tmp = folding->ppoint[point_cnt].threshold1;
						break;
					case 1:
						sw_tmp = folding->ppoint[point_cnt].threshold2;
						break;
					case 2:
						sw_tmp = folding->ppoint[point_cnt].threshold3;
						break;
					case 3:
						sw_tmp = folding->ppoint[point_cnt].threshold4;
						break;
					default:
						break;
					}
					threshold[folding->ppoint[point_cnt + 1].x_e] =  sw_tmp * DECREASE_THRESH;
				}
			}

			// LEFT
			for (yp = folding->ppoint[point_cnt].y_s; yp < folding->ppoint[point_cnt].y_e; yp++)
			{
				for (xp = folding->ppoint[point_cnt].x_s + loss_a; xp <= folding->ppoint[point_cnt].x_s + loss_a; xp++)
				{
					for (y = yp * adjust; y < (yp * adjust) + adjust; y++)
					{
						for (x = xp * adjust; x < (xp * adjust) + adjust; x++)
						{
							spt.x = x;
							spt.y = y;
							new_L_Getdt(&spt, work[buf_num].pbs, pnote_param);
							tmp = spt.sensdt;
							if (tmp < HOLOGRAM_THRESHOLD && WINDOW_THRESHOLD < tmp)
							{
								result += (u32)tmp;
								counter++;
							}
							x++;
						}
					}
				}
			}
			if (counter == 0)
			{
				reliability[2] = 0; // RIGHTの折り畳み信頼度
			}
			else
			{
				average[2] = ((float)result / (float)counter);
				reliability[2] = ((float)WINDOW_THRESHOLD) / average[2]; // LEFTの折り畳み信頼度
			}
			counter = 0;
			result = 0;
			j_cnt++;

			// RIGHT
			for (yp = folding->ppoint[point_cnt].y_s; yp < folding->ppoint[point_cnt].y_e; yp++)
			{
				for (xp = folding->ppoint[point_cnt].x_e - loss_b; xp <= folding->ppoint[point_cnt].x_e - loss_b/* - 1*/; xp++)
				{
					for (y = yp * adjust; y < (yp * adjust) + adjust; y++)
					{
						for (x = xp * adjust; x < (xp * adjust) + adjust; x++)
						{
							spt.x = x;
							spt.y = y;
							new_L_Getdt(&spt, work[buf_num].pbs, pnote_param);
							tmp = spt.sensdt;
							if (tmp < HOLOGRAM_THRESHOLD && WINDOW_THRESHOLD < tmp)
							{
								result += (u32)tmp;
								counter++;
							}
							x++;
						}
					}
				}
			}
			if (counter == 0)
			{
				reliability[3] = 0; // RIGHTの折り畳み信頼度
			}
			else
			{
				average[3] = ((float)result / (float)counter);
				reliability[3] = ((float)WINDOW_THRESHOLD) / average[3]; // RIGHTの折り畳み信頼度
			}
			counter = 0;
			result = 0;
			j_cnt++;
			// 最も折り畳みの可能性が高いものを選ぶ
			if (reliability[2] > reliability[3])
			{
				if (rel_tmp < reliability[2])
				{
					rel_tmp = reliability[2];
					res_count = 2;
				}
				level[0] = size_level((u16)x_size);
			}
			else
			{
				if (rel_tmp < reliability[3])
				{
					rel_tmp = reliability[3];
					res_count = 3;
				}
				level[0] = size_level((u16)x_size);
			}
		}

		rel_tmp = 0.0;

		// 折り畳み信頼レベル計算 = 下のFOLDED_LEVELとカウント（信頼レベル）を比較する
		for (j_cnt = 0; j_cnt < 10; j_cnt++)
		{
			rel_tmp = (float)level_average[res_count] + (float)(j_cnt /*+ 1*/) * (float)level_std[res_count];

			if (rel_tmp > reliability[res_count])
			{
				break;
			}
			else
			{
				continue;
			}
		}

		folding->folded_level[res_count] = level[0];

		if (work[buf_num].pbs->LEorSE == LE)
		{
			//cnt_th = (u8)FOLDED_LEVEL - 1;
			cnt_th = (u8)FOLDED_LEVEL;
		}
		else
		{
			cnt_th = (u8)FOLDED_LEVEL;
		}

		// 最終結果
		if (threshold[res_count] < average[res_count])
			//if (folding->ppoint[point_cnt].threshold1 < average[res_count])
		{
			folding->folded[res_count] = 4;
			folding->err_code = 4;
		}
		else
		{
			if (j_cnt < cnt_th)
			{
				folding->folded[res_count] = 4;
				folding->err_code = 4;
			}
			else
			{
				folding->folded[res_count] = 1;
				folding->err_code = (u8)res_count;
			}
		}
	}

#ifdef VS_DEBUG
	if (work[buf_num].pbs->blank3 != 0)
	{
		fprintf(fp, "%s,%x,%d,%s,", work[buf_num].pbs->blank4, work[buf_num].pbs->mid_res_nn.result_jcm_id, work[buf_num].pbs->insertion_direction, work[buf_num].pbs->category);
		fprintf(fp, "%d,%s,%s,", work[buf_num].pbs->blank0[20], work[buf_num].pbs->ser_num1, work[buf_num].pbs->ser_num2);
		fprintf(fp, "0x%02d%d%d,", work[buf_num].pbs->spec_code.model_calibration, work[buf_num].pbs->spec_code.sensor_conf, work[buf_num].pbs->spec_code.mode);

		fprintf(fp, "%d,%d,%d,%d,", work[buf_num].pbs->note_x_size, work[buf_num].pbs->note_y_size, (u32)(work[buf_num].pbs->note_x_size*0.127 + 0.5), (u32)(work[buf_num].pbs->note_y_size*0.127 + 0.5));
		fprintf(fp, "%d,", folding->err_code);
		fprintf(fp, "%d,", res_count);
		fprintf(fp, "%d,", folding->folded_level[res_count]);
		fprintf(fp, "%f, %f, %f, %f,", average[0], average[1], average[2], average[3]);

		fprintf(fp, "\n");
		fclose(fp);
	}
#endif

	// ０～３なら折れ、４なら欠損
	return (s32)folding->err_code;
}

